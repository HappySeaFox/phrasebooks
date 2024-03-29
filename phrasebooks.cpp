/*
 * This file is part of phrasebooks.
 *
 * phrasebooks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phrasebooks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with phrasebooks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QWhatsThisClickedEvent>
#include <QMutableListIterator>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDropEvent>
#include <QShortcut>
#include <QDateTime>
#include <QMimeData>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QDate>
#include <QUrl>

#include <windows.h>
#include <winnt.h>
#include <psapi.h>

#include "qtsingleapplication.h"
#include "phrasebooks.h"
#include "settings.h"
// TODO
//#include "options.h"
#include "about.h"
#include "utils.h"

#include "ui_phrasebooks.h"

static const int PHRASEBOOKS_WINDOW_STARTUP_TIMEOUT = 1200;

/***************************************/

Phrasebooks::Phrasebooks()
    : QWidget(0,
              Qt::Window
              | Qt::WindowMinimizeButtonHint
              | Qt::WindowCloseButtonHint
              | Qt::CustomizeWindowHint)
    , ui(new Ui::Phrasebooks)
    , m_running(false)
    , m_locked(false)
    , m_lastActiveOurWindow(nullptr)
    , m_drawnWindow(0)
    , m_linksChanged(false)
    , m_justTitle(false)
{
    ui->setupUi(this);

    connect(static_cast<QtSingleApplication *>(qApp), &QtSingleApplication::messageReceived, this, &Phrasebooks::slotMessageReceived);

    setAcceptDrops(true);

    QIcon icon_quit(":/images/quit.png");
    QIcon icon_phrasebooks(":/images/phrasebooks.ico");

    // context menu
    QShortcut *help_shortcut = new QShortcut(QKeySequence::HelpContents, this, SLOT(slotAbout()));
    QShortcut *quit_shortcut = new QShortcut(Qt::CTRL+Qt::Key_Q, this, SLOT(slotQuit()));

    m_menu = new QMenu(this);
    m_menu->addAction(QIcon(":/images/options.png"), tr("Options..."), this, SLOT(slotOptions()));
    m_menu->addSeparator();

    //: This is the label on a menu item that user clicks to issue the command
    m_menu->addAction(tr("Clear links"), this, SLOT(slotClearLinks()));
    m_menu->addSeparator();

    m_menu->addAction(icon_phrasebooks,
                      Utils::aboutPhrasebooks()
                      + "...\t"
                      + help_shortcut->key().toString(),
                      this,
                      SLOT(slotAbout()));

    //: Qt is a C++ crossplatform toolkit http://qt-project.org
    m_menu->addAction(tr("About Qt..."), this, SLOT(slotAboutQt()));
    m_menu->addSeparator();
    m_menu->addAction(icon_quit, tr("Quit") + '\t' + quit_shortcut->key().toString(), this, SLOT(slotQuit()));

    m_timerCheckActive = new QTimer(this);
    m_timerCheckActive->setSingleShot(true);
    m_timerCheckActive->setInterval(50);
    connect(m_timerCheckActive, &QTimer::timeout, this, &Phrasebooks::slotCheckActive);

    m_timerLoadToNextWindow = new QTimer(this);
    m_timerLoadToNextWindow->setSingleShot(true);
    m_timerLoadToNextWindow->setInterval(10);
    connect(m_timerLoadToNextWindow, &QTimer::timeout, this, &Phrasebooks::slotLoadToNextWindow);

    connect(ui->list, &List::loadText, this, &Phrasebooks::slotLoadText);

    // restore geometry
    QSize sz = SETTINGS_GET_SIZE(SETTING_SIZE);

    if(sz.isValid())
        resize(sz);

    Utils::moveWindow(this, SETTINGS_GET_POINT(SETTING_POSITION));

    checkWindows();

    // lock links
    new QShortcut(Qt::CTRL+Qt::Key_L, this, SLOT(slotLockLinks()));

    if(!SETTINGS_GET_BOOL(SETTING_FOOLSDAY_SEEN))
        QTimer::singleShot(0, this, &Phrasebooks::slotFoolsDay);

    // watch for QWhatsThisClickedEvent
    qApp->installEventFilter(this);
}

Phrasebooks::~Phrasebooks()
{
    QSize size(width(), height());

    SETTINGS_SET_SIZE(SETTING_SIZE, size, Settings::NoSync);
    SETTINGS_SET_POINT(SETTING_POSITION, pos());

    delete ui;
}

void Phrasebooks::contextMenuEvent(QContextMenuEvent *event)
{
    QApplication::restoreOverrideCursor();

    event->accept();
    m_menu->exec(event->globalPos());
}

bool Phrasebooks::eventFilter(QObject *o, QEvent *e)
{
    QEvent::Type type = e->type();

    if(type == QEvent::WhatsThisClicked)
    {
        QWhatsThisClickedEvent *ce = static_cast<QWhatsThisClickedEvent *>(e);

        if(ce)
        {
            QUrl url(ce->href());

            if(url.isValid())
                QDesktopServices::openUrl(url);
        }

        return true;
    }

    return QObject::eventFilter(o, e);
}

void Phrasebooks::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasFormat("text/plain"))
    {
        qDebug("Accepting dragged MIME in main window");
        e->acceptProposedAction();
    }
    else
        e->ignore();
}

void Phrasebooks::dragMoveEvent(QDragMoveEvent *e)
{
    if(e->mimeData()->hasFormat("text/plain"))
        e->acceptProposedAction();
    else
        e->ignore();
}

void Phrasebooks::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();
}

void Phrasebooks::dropEvent(QDropEvent *e)
{
    e->acceptProposedAction();
    ui->list->addLines(e->mimeData()->text().trimmed().split(QRegExp("\\s+"), QString::SkipEmptyParts));
}

void Phrasebooks::sendKey(int key, bool extended)
{
    KEYBDINPUT kbInput = {0, 0, 0, 0, 0};
    INPUT input[4];

    memset(input, 0, sizeof(input));

    int nelem = 2;
    int index = 0;

    // do we need the SHIFT key? Don't check all the cases, we don't need all of them
    bool shift = (key >= '!' && key <= '+') || (key >= 'A' && key <= 'Z');

    // send SHIFT down
    if(shift)
    {
        nelem += 2;

        kbInput.dwFlags = 0;
        kbInput.wVk = VK_SHIFT;
        kbInput.wScan = MapVirtualKey(VK_SHIFT, 0);

        input[index].type = INPUT_KEYBOARD;
        input[index].ki = kbInput;

        ++index;
    }

    // key down
    SHORT vkey = VkKeyScan(key);

    if(extended)
        kbInput.dwFlags = KEYEVENTF_EXTENDEDKEY;

    kbInput.wVk = vkey;
    kbInput.wScan = MapVirtualKey(vkey, 0);

    input[index].type = INPUT_KEYBOARD;
    input[index].ki = kbInput;
    ++index;

    // key up
    kbInput.dwFlags = KEYEVENTF_KEYUP;

    if(extended)
        kbInput.dwFlags |= KEYEVENTF_EXTENDEDKEY;

    input[index].type = INPUT_KEYBOARD;
    input[index].ki = kbInput;
    ++index;

    // SHIFT up
    if(shift)
    {
        kbInput.dwFlags = KEYEVENTF_KEYUP;
        kbInput.wVk = VK_SHIFT;
        kbInput.wScan = MapVirtualKey(VK_SHIFT, 0);

        input[index].type = INPUT_KEYBOARD;
        input[index].ki = kbInput;
    }

    // send both combinations
    SendInput(nelem, input, sizeof(INPUT));
}

void Phrasebooks::sendString(const QString &text)
{
    if(text.isEmpty())
    {
        qDebug("Text is empty, nothing to send");
        return;
    }

    for(int i = 0;i < text.length();i++)
        sendKey(text.at(i).toLatin1());

    // TODO
    Sleep(10);

    sendKey(VK_RETURN);
}

void Phrasebooks::checkWindow(Link *link)
{
    if(!link)
        return;

    link->threadId = GetWindowThreadProcessId(link->hwnd, &link->processId);

    qDebug("Window under cursor is %p", Utils::pointerToVoidPointer(link->hwnd));
}

Phrasebooks::Link Phrasebooks::checkTargetWindow(const QPoint &p, bool allowThisWindow)
{
    POINT pnt = {0, 0};

    pnt.x = p.x();
    pnt.y = p.y();

    HWND hwnd = RealChildWindowFromPoint(GetDesktopWindow(), pnt);

    if(!IsWindow(hwnd))
    {
        qDebug("Cannot find window under cursor %d,%d", p.x(), p.y());
        return Link();
    }

    // this window
    if(!allowThisWindow && hwnd == reinterpret_cast<HWND>(winId()))
    {
        qDebug("Ignoring ourselves");
        return Link();
    }

    // desktop
    if(Utils::isDesktop(hwnd))
    {
        qDebug("Ignoring desktop window");
        return Link();
    }

    Link link = Link(hwnd);

    // get and check subcontrol
    link.subControl = Utils::RealWindowFromPoint(pnt);

    if(link.subControl == link.hwnd)
    {
        qDebug("Subcontrol is the same top-level window, setting to 0");
        link.subControl = 0;
    }
    else if(GetAncestor(link.subControl, GA_ROOT) != hwnd)
    {
        qDebug("Subcontrol belongs to another top-level window, setting to 0");
        link.subControl = 0;
    }

    // already linked?
    QList<Link>::iterator itEnd = m_windows.end();

    for(QList<Link>::iterator it = m_windows.begin();it != itEnd;++it)
    {
        if(link.subControl)
        {
            if((*it).subControl == link.subControl)
            {
                qDebug("Window %p/%p is already linked", Utils::pointerToVoidPointer(hwnd),
                                                         Utils::pointerToVoidPointer(link.subControl));

                return Link();
            }
        }
        else if(!(*it).subControl && (*it).hwnd == link.hwnd)
        {
            qDebug("Window %p is already linked", Utils::pointerToVoidPointer(hwnd));
            return Link();
        }
    }

    if(link.subControl)
    {
        TCHAR name[MAX_PATH];

        if(!GetClassName(link.subControl, name, PHRASEBOOKS_SIZE_OF_ARRAY(name)))
            qWarning("Cannot get a class name for subcontrol %p (%ld)", Utils::pointerToVoidPointer(link.subControl), GetLastError());
        else if(!lstrcmp(name, TEXT("Edit")) || !lstrcmp(name, TEXT("TEdit")))
            link.subControlSupportsClearing = true;
    }

    qDebug("Subcontrol: %p", Utils::pointerToVoidPointer(link.subControl));

    return link;
}

void Phrasebooks::checkWindows()
{
    QString tooltip = "<table>";

    QList<Link>::iterator itEnd = m_windows.end();

    for(QList<Link>::iterator it = m_windows.begin();it != itEnd;)
    {
        // remove dead windows
        if(!IsWindow((*it).hwnd))
        {
            qDebug("Window id %p is not valid, removing", Utils::pointerToVoidPointer((*it).hwnd));
            it = m_windows.erase(it);
            itEnd = m_windows.end();
        }
        else
            ++it;
    }

    if(m_windows.isEmpty())
    {
        ui->target->setNumberOfLinks(0);
        return;
    }

    ui->target->setNumberOfLinks(m_windows.size());
}

void Phrasebooks::nextLoadableWindowIndex(int delta)
{
    m_currentWindow += delta;
    qDebug("Found index %d", m_currentWindow);
}

void Phrasebooks::loadNextWindow()
{
    nextLoadableWindowIndex(+1);

    if(m_currentWindow >= m_windows.size())
    {
        qDebug("Done for all windows");

        busy(false);
        activate();

        m_running = false;
    }
    else
        m_timerLoadToNextWindow->start();
}

void Phrasebooks::busy(bool b)
{
    ui->list->setIgnoreInput(b);
    ui->stackBusy->setCurrentIndex(b);
}

void Phrasebooks::loadText(const QString &text)
{
    qDebug("Loading text \"%s\"", qPrintable(text));

    if(isBusy() || text.isEmpty())
        return;

    checkWindows();

    if(m_windows.isEmpty())
    {
        qDebug("No windows configured");
        activate();
        return;
    }

    m_text = text;
    m_currentWindow = 0;

    nextLoadableWindowIndex();

    if(m_currentWindow >= m_windows.size())
    {
        qDebug("Cannot find where to load the ticker");
        activate();
        return;
    }

    m_running = true;
    m_startupTime = QDateTime::currentMSecsSinceEpoch();

    busy(true);

    m_lastActiveOurWindow = qApp->activeWindow();
    m_timerLoadToNextWindow->start();
}

void Phrasebooks::activate()
{
    qDebug("Activating");

    Utils::raiseWindow(this);

    if(m_lastActiveOurWindow)
        m_lastActiveOurWindow->activateWindow();
    else
        activateWindow();

    m_lastActiveOurWindow = 0;
}

void Phrasebooks::slotCheckActive()
{
    if(m_windows.isEmpty())
    {
        qDebug("Window list is empty");
        m_running = false;
        return;
    }

    const Link &link = m_windows[m_currentWindow];

    if(GetForegroundWindow() == link.hwnd)
    {
        qDebug("Found window, sending data");

        bool okToLoad = false;

        if(link.subControl)
        {
            if(setForeignFocus(link))
                okToLoad = true;
        }
        else
            okToLoad = true;

        // real ticker to load
        if(okToLoad)
            sendString(m_text);

        loadNextWindow();
    }
    else
    {
        if(QDateTime::currentMSecsSinceEpoch() - m_startupTime > PHRASEBOOKS_WINDOW_STARTUP_TIMEOUT)
        {
            qDebug("Failed to wait the window due to timeout");
            loadNextWindow();
        }
        else
        {
            qDebug("Cannot find window, continuing to search");
            m_timerLoadToNextWindow->start();
        }
    }
}

void Phrasebooks::slotAbout()
{
    About about(this);
    about.exec();
}

void Phrasebooks::slotAboutQt()
{
    QMessageBox::aboutQt(this);
}

void Phrasebooks::slotQuit()
{
    qApp->quit();
}

void Phrasebooks::slotOptions()
{
    // TODO
    /*
    Options opt(this);

    if(opt.exec() == QDialog::Accepted)
    {
        opt.saveSettings();
    }
    */
}

void Phrasebooks::slotLoadText(const QString &text)
{
    if(isBusy())
        return;

    loadText(text);
}

void Phrasebooks::slotLoadToNextWindow()
{
    HWND window = m_windows.at(m_currentWindow).hwnd;

    qDebug("Trying window %p", Utils::pointerToVoidPointer(window));

    bringToFront(window);

    m_timerCheckActive->start();
}

void Phrasebooks::slotClearLinks()
{
    if(isBusy())
        return;

    if(m_windows.isEmpty())
        return;

    qDebug("Clear links");

    MessageBeep(MB_OK);

    m_windows.clear();

    checkWindows();

    m_linksChanged = true;
}

void Phrasebooks::slotLockLinks()
{
    m_locked = !m_locked;

    ui->stackBusy->setCurrentIndex(m_locked);
    ui->target->locked(m_locked);
}

void Phrasebooks::drawWindowMarker()
{
    if(!IsWindow(m_drawnWindow))
        return;

    static HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

    HDC dc;
    HGDIOBJ oldPen, oldBrush;
    RECT rect;

    GetWindowRect(m_drawnWindow, &rect);

    dc = GetWindowDC(m_drawnWindow);

    if(!dc)
        return;

    oldPen = SelectObject(dc, pen);
    oldBrush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));

    Rectangle(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);

    ReleaseDC(m_drawnWindow, dc);
}

void Phrasebooks::removeWindowMarker()
{
    if(!IsWindow(m_drawnWindow))
        return;

    InvalidateRect(m_drawnWindow, 0, TRUE);
    UpdateWindow(m_drawnWindow);
    RedrawWindow(m_drawnWindow, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    m_drawnWindow = 0;
}

void Phrasebooks::bringToFront(HWND window)
{
    qDebug("Bring to front %p", Utils::pointerToVoidPointer(window));

    // window flags to set
    int flags = SW_SHOWNORMAL;

    if(IsZoomed(window))
        flags |= SW_SHOWMAXIMIZED;

    // try to switch to this window
    ShowWindow(window, flags);
    SetForegroundWindow(window);
}

bool Phrasebooks::isBusy() const
{
    if(m_locked)
    {
        qDebug("Locked");
        return true;
    }

    if(m_running)
    {
        qDebug("In progress, won't load a new ticker");
        return true;
    }

    return false;
}

bool Phrasebooks::detectForegroundWindowAndActivate()
{
    DWORD foregroundProcessId;

    const HWND foregroundWindow = GetForegroundWindow();
    const DWORD foregroundThreadId = GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId);

    if(GetCurrentProcessId() != foregroundProcessId)
    {
        const DWORD currentThreadId = GetCurrentThreadId();

        if(!AttachThreadInput(foregroundThreadId, currentThreadId, TRUE))
        {
            qWarning("Cannot attach to the thread %ld (%ld)", foregroundThreadId, GetLastError());
            return false;
        }

        activate();
        SetForegroundWindow(reinterpret_cast<HWND>(winId()));

        AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
    }
    else
        activate();

    return true;
}

void Phrasebooks::targetDropped(const QPoint &p, bool beep)
{
    if(isBusy())
        return;

    removeWindowMarker();

    Link link = checkTargetWindow(p, false);

    if(!link.hwnd)
        return;

    checkWindow(&link);

    link.dropPoint = p;

    // beep
    if(beep)
        MessageBeep(MB_OK);

    m_windows.append(link);

    checkWindows();

    m_linksChanged = true;
}

void Phrasebooks::slotTargetMoving(const QPoint &pt)
{
    POINT pnt = {0, 0};

    pnt.x = pt.x();
    pnt.y = pt.y();

    HWND rnewHwnd = RealChildWindowFromPoint(GetDesktopWindow(), pnt);
    HWND newHwnd = Utils::RealWindowFromPoint(pnt);

    if(m_drawnWindow != newHwnd)
        removeWindowMarker();

    if(!IsWindow(newHwnd) || rnewHwnd == reinterpret_cast<HWND>(winId()) || Utils::isDesktop(rnewHwnd))
        return;

    m_drawnWindow = newHwnd;

    drawWindowMarker();
}

void Phrasebooks::slotTargetCancelled()
{
    removeWindowMarker();
}

void Phrasebooks::slotMessageReceived(const QString &msg)
{
    qDebug("Got message \"%s\"", qPrintable(msg));

    if(msg == "activate-window")
    {
        detectForegroundWindowAndActivate();
    }
}

void Phrasebooks::slotFoolsDay()
{
    QDate day = QDate::currentDate();

    // April, 1
    if(day.day() == 1 && day.month() == 4)
    {
        QMessageBox::information(this,
                                 QString(),
                                 QString("<p>%1</p><p align=right><i>%2</i></p>")
                                        //: Message displayed to the user in the April Fool's day. Preferably copy the translation from "The Little Golden Calf" by I.Ilf, E.Petrov, Chapter 10 "A Telegram from the Brothers Karamazov" (http://en.wikipedia.org/wiki/The_Little_Golden_Calf). If you don't have the book or its online translation then translate as usual
                                        .arg(tr("All large contemporary fortunes were acquired<br>in the most dishonorable way."))
                                        //: See http://en.wikipedia.org/wiki/The_Little_Golden_Calf
                                        .arg(tr("\"The Little Golden Calf\" I.Ilf, E.Petrov")));

        SETTINGS_SET_BOOL(SETTING_FOOLSDAY_SEEN, true);
    }
}

bool Phrasebooks::setForeignFocus(const Link &link)
{
    const DWORD currentThreadId = GetCurrentThreadId();

    if(!AttachThreadInput(link.threadId, currentThreadId, TRUE))
    {
        qWarning("Cannot attach to the thread %ld (%ld)", link.threadId, GetLastError());
        return false;
    }

    HWND hwnd = SetFocus(link.subControl);

    if(link.subControlSupportsClearing)
        SendMessage(link.subControl, WM_SETTEXT, 0, 0);

    SendMessage(link.hwnd, WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_SYSCHARSET, 0x04090409);

    AttachThreadInput(link.threadId, currentThreadId, FALSE);

    if(!hwnd)
    {
        qWarning("Cannot set focus to the window %p (%ld)", Utils::pointerToVoidPointer(link.subControl), GetLastError());
        return false;
    }
    else
        qDebug("Set focus to the window %p", Utils::pointerToVoidPointer(link.subControl));

    return true;
}
