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

#include <QMutableListIterator>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QKeySequence>
#include <QInputDialog>
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

#ifdef Q_OS_WIN32
    #include <windows.h>
    #include <winnt.h>
    #include <psapi.h>

    #include "windowmarker.h"
#endif

#include "qtsingleapplication.h"
#include "englishvalidator.h"
#include "phrasebooks.h"
#include "settings.h"
#include "options.h"
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
    , m_startupTime(0)
    , m_locked(false)
#ifdef Q_OS_WIN32
    , m_drawnWindow(0)
#endif
    , m_justTitle(false)
{
    ui->setupUi(this);

    connect(static_cast<QtSingleApplication *>(qApp), &QtSingleApplication::messageReceived,
            this, &Phrasebooks::slotMessageReceived);

    setAcceptDrops(true);

    // context menu
    QShortcut *shortcutHelp = new QShortcut(QKeySequence::HelpContents, this, SLOT(slotAbout()));
    QShortcut *shortcutQuit = new QShortcut(Qt::CTRL+Qt::Key_Q, this, SLOT(slotQuit()));
    QShortcut *shortcutLock = new QShortcut(Qt::CTRL+Qt::Key_L, this, SLOT(slotLockLinks()));

    m_menu = new QMenu(this);
    m_menu->addAction(QIcon(":/images/options.png"), tr("Options..."), this, SLOT(slotOptions()));
    m_menu->addSeparator();

    //: This is the label on a menu item that user clicks to issue the command
    m_menu->addAction(tr("Restore last links"), this, SLOT(slotRestoreLastLinks()));
    //: This is the label on a menu item that user clicks to issue the command
    m_menu->addAction(tr("Clear links"), this, SLOT(slotClearLinks()));
    //: This is the label on a menu item that user clicks to issue the command
    m_menu->addAction(tr("Lock links") + '\t' + shortcutLock->key().toString(), this, SLOT(slotLockLinks()));
    m_menu->addSeparator();

    m_menu->addAction(QIcon(":/images/phrasebooks.ico"),
                      Utils::aboutPhrasebooks()
                        + "...\t"
                        + shortcutHelp->key().toString(),
                      this,
                      SLOT(slotAbout()));

    //: Qt is a C++ crossplatform toolkit http://qt-project.org
    m_menu->addAction(tr("About Qt..."), this, SLOT(slotAboutQt()));
    m_menu->addSeparator();
    m_menu->addAction(QIcon::fromTheme("application-exit", QIcon(":/images/quit.png")),
                      tr("Quit") + '\t' + shortcutQuit->key().toString(), this, SLOT(slotQuit()));

    m_timerCheckActive = new QTimer(this);
    m_timerCheckActive->setSingleShot(true);
    m_timerCheckActive->setInterval(50);
    connect(m_timerCheckActive, &QTimer::timeout, this, &Phrasebooks::slotCheckActive);

    m_timerLoadToNextWindow = new QTimer(this);
    m_timerLoadToNextWindow->setSingleShot(true);
    m_timerLoadToNextWindow->setInterval(10);
    connect(m_timerLoadToNextWindow, &QTimer::timeout, this, &Phrasebooks::slotLoadToNextWindow);

    connect(ui->list, &List::loadText,            this, &Phrasebooks::slotLoadText);
    connect(ui->list, &List::currentIndexChanged, this, &Phrasebooks::slotCurrentIndexChanged);

    // restore geometry
    QSize sz = SETTINGS_GET_SIZE(SETTING_SIZE);

    if(sz.isValid())
        resize(sz);

    if(SETTINGS_CONTAINS(SETTING_POSITION))
        Utils::moveWindow(this, SETTINGS_GET_POINT(SETTING_POSITION));

    checkWindows();

    if(!SETTINGS_GET_BOOL(SETTING_FOOLSDAY_SEEN))
        QTimer::singleShot(0, this, SLOT(slotFoolsDay()));

    connect(ui->chapter, &BooksAndChapters::selectorClosed, this, &Phrasebooks::slotSelectorClosed);

    // load the last chapter
    const QString lastChapter = SETTINGS_GET_STRING(SETTING_LAST_CHAPTER);

    if(!lastChapter.isEmpty())
    {
        if(ui->list->setCurrentChapterPath(ui->chapter->chapterFullPath(lastChapter)))
            ui->chapter->setChapter(lastChapter);
    }
}

Phrasebooks::~Phrasebooks()
{
    SETTINGS_SET_SIZE(SETTING_SIZE, size(), Settings::NoSync);
    SETTINGS_SET_POINT(SETTING_POSITION, pos());

    delete ui;
}

void Phrasebooks::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    m_menu->exec(event->globalPos());
}

void Phrasebooks::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasText())
        e->acceptProposedAction();
    else
        e->ignore();
}

void Phrasebooks::dragMoveEvent(QDragMoveEvent *e)
{
    if(e->mimeData()->hasText())
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

    QString text = e->mimeData()->text().trimmed();

    EnglishValidator validator;
    int pos = 0;

    static QRegExp rx("[\\r]?\\n");

    if(validator.validate(text, pos) == QValidator::Acceptable)
        ui->list->addLines(text.split(rx, QString::SkipEmptyParts));
}

void Phrasebooks::sendString(const QString &text)
{
    if(text.isEmpty())
    {
        qDebug("Text is empty, nothing to send");
        return;
    }

    Utils::startSendingKeys();

    for(int i = 0;i < text.length();i++)
        Utils::sendKey(text.at(i).toLatin1());

#ifdef Q_OS_WIN32
    Utils::sleep(75);
#endif

    Utils::sendReturn();
    Utils::stopSendingKeys();
}

Phrasebooks::Link Phrasebooks::checkTargetWindow(const QPoint &p)
{
    Platform::WindowId hwnd = Utils::topLevelWindowFromPoint(p);

    if(!Utils::isWindow(hwnd))
    {
        qDebug("Cannot find window under cursor %d,%d", p.x(), p.y());
        return Link();
    }

    // this window
    if(hwnd == Utils::winIdToNativeWindow(winId()))
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

    // already linked?
    QList<Link>::iterator itEnd = m_windows.end();

    for(QList<Link>::iterator it = m_windows.begin();it != itEnd;++it)
    {
        if(link.subControl)
        {
            if((*it).subControl == link.subControl)
            {
                qDebug("Window 0x%lx/0x%lx is already linked", Utils::windowHandleToLong(hwnd),
                                                                Utils::windowHandleToLong(link.subControl));

                return Link();
            }
        }
        else if(!(*it).subControl && (*it).hwnd == link.hwnd)
        {
            qDebug("Window 0x%lx is already linked", Utils::windowHandleToLong(hwnd));
            return Link();
        }
    }

    // get and check subcontrol
#ifdef Q_OS_WIN32
    link.subControl = Utils::RealWindowFromPoint(p);

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

    if(link.subControl)
    {
        TCHAR name[MAX_PATH];

        if(!GetClassName(link.subControl, name, PHRASEBOOKS_SIZE_OF_ARRAY(name)))
            qWarning("Cannot get a class name for subcontrol 0x%lx (0x%lx)", Utils::windowHandleToLong(link.subControl), GetLastError());
        else if(!lstrcmp(name, TEXT("Edit")) || !lstrcmp(name, TEXT("TEdit")))
            link.subControlSupportsClearing = true;
    }

    qDebug("Subcontrol: 0x%lx", Utils::windowHandleToLong(link.subControl));
#endif

    return link;
}

void Phrasebooks::checkWindows()
{
    QList<Link>::iterator itEnd = m_windows.end();

    for(QList<Link>::iterator it = m_windows.begin();it != itEnd;)
    {
        // remove dead windows
        if(!Utils::isWindow((*it).hwnd))
        {
            qDebug("Window id 0x%lx is not valid, removing", Utils::windowHandleToLong((*it).hwnd));
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

#ifdef Q_OS_WIN32
        LockSetForegroundWindow(LSFW_LOCK);
#endif

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
        ui->target->blink();
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

    m_timerLoadToNextWindow->start();
}

void Phrasebooks::activate()
{
    qDebug("Activating");

    Utils::raiseWindow(this);
    activateWindow();
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

    if(Utils::activeWindow() == link.hwnd)
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
    Options opt(this);

    if(opt.exec() == Options::Accepted)
        opt.saveSettings();
}

void Phrasebooks::slotRestoreLastLinks()
{
    qDebug("Restoring last links");

    slotClearLinks();

    const QList<QPoint> points = SETTINGS_GET_POINTS(SETTING_LAST_LINK_POINTS);

    foreach(const QPoint &point, points)
    {
        qDebug("Restoring link point %dx%d", point.x(), point.y());
        targetDropped(point);
    }

}

void Phrasebooks::slotLoadText(const QString &text)
{
    if(isBusy())
        return;

    loadText(text);
}

void Phrasebooks::slotCurrentIndexChanged(int current, int total)
{
    ui->labelCurrentLine->setText(QString("%1/%2").arg(current+1).arg(total));
}

void Phrasebooks::slotLoadToNextWindow()
{
    Platform::WindowId window = m_windows.at(m_currentWindow).hwnd;

    qDebug("Trying window 0x%lx", Utils::windowHandleToLong(window));

    Utils::bringToFront(window);

    m_timerCheckActive->start();
}

void Phrasebooks::slotClearLinks()
{
    if(isBusy())
        return;

    if(m_windows.isEmpty())
        return;

    qDebug("Clear links");

    Utils::beep();

    m_windows.clear();

    checkWindows();
}

void Phrasebooks::slotLockLinks()
{
    m_locked = !m_locked;

    ui->stackBusy->setCurrentIndex(m_locked);
    ui->target->locked(m_locked);
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
#ifdef Q_OS_WIN32
    DWORD foregroundProcessId;

    const HWND foregroundWindow = GetForegroundWindow();
    const DWORD foregroundThreadId = GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId);

    if(GetCurrentProcessId() != foregroundProcessId)
    {
        const DWORD currentThreadId = GetCurrentThreadId();

        if(!AttachThreadInput(foregroundThreadId, currentThreadId, TRUE))
        {
            qWarning("Cannot attach to the thread 0x%lx (0x%lx)", foregroundThreadId, GetLastError());
            return false;
        }

        activate();
        SetForegroundWindow(reinterpret_cast<HWND>(winId()));

        AttachThreadInput(foregroundThreadId, currentThreadId, FALSE);
    }
    else
#endif
        activate();

    return true;
}

void Phrasebooks::targetDropped(const QPoint &p)
{
    if(isBusy())
        return;

#ifdef Q_OS_WIN32
    WindowMarker::remove(&m_drawnWindow);
#endif

    Link link = checkTargetWindow(p);

    if(!link.hwnd)
        return;

#ifdef Q_OS_WIN32
    link.threadId = GetWindowThreadProcessId(link.hwnd, &link.processId);
#endif

    link.dropPoint = p;

    qDebug("Window under cursor is 0x%lx", Utils::windowHandleToLong(link.hwnd));

    Utils::beep();

    m_windows.append(link);

    // save
    QList<QPoint> points;

    foreach(const Link &link, m_windows)
        points.append(link.dropPoint);

    SETTINGS_SET_POINTS(SETTING_LAST_LINK_POINTS, points);

    checkWindows();
}

void Phrasebooks::slotTargetMoving(const QPoint &pt)
{
#ifdef Q_OS_WIN32
    POINT pnt = { pt.x(), pt.y() };

    HWND rnewHwnd = RealChildWindowFromPoint(GetDesktopWindow(), pnt);
    HWND newHwnd = Utils::RealWindowFromPoint(pt);

    if(m_drawnWindow != newHwnd)
        WindowMarker::remove(&m_drawnWindow);

    if(!IsWindow(newHwnd) || rnewHwnd == reinterpret_cast<HWND>(winId()) || Utils::isDesktop(rnewHwnd))
        return;

    m_drawnWindow = newHwnd;

    WindowMarker::draw(m_drawnWindow);
#else
    Q_UNUSED(pt)
#endif
}

void Phrasebooks::slotTargetCancelled()
{
#ifdef Q_OS_WIN32
    WindowMarker::remove(&m_drawnWindow);
#endif
}

void Phrasebooks::slotMessageReceived(const QString &msg)
{
    qDebug("Got message \"%s\"", qPrintable(msg));

    if(msg == "activate-window")
        detectForegroundWindowAndActivate();
}

void Phrasebooks::slotFoolsDay()
{
    const QDate day = QDate::currentDate();

    // April, 1
    if(day.day() == 1 && day.month() == 4)
    {
        QMessageBox::information(this,
                                 tr("April, 1"),
                                 QString("<p>%1</p><p align=right><i>%2</i></p>")
                                        //: Message displayed to the user in the April Fool's day
                                        .arg(tr("The limits of my language are the limits of my world."))
                                        //: See http://en.wikipedia.org/wiki/The_Little_Golden_Calf
                                        .arg(tr("Ludwig Wittgenstein")));

        SETTINGS_SET_BOOL(SETTING_FOOLSDAY_SEEN, true);
    }
}

void Phrasebooks::slotSelectorClosed(const QString &bookAndChapter)
{
    if(!bookAndChapter.isEmpty())
    {
        qDebug("Loading chapter %s", qPrintable(bookAndChapter));

        if(ui->list->setCurrentChapterPath(ui->chapter->chapterFullPath(bookAndChapter)))
        {
            ui->chapter->setChapter(bookAndChapter);
            SETTINGS_SET_STRING(SETTING_LAST_CHAPTER, bookAndChapter);
        }
    }
    else if(!ui->list->currentChapterPath().isEmpty())
    {
        if(!QFile::exists(ui->list->currentChapterPath()))
        {
            qDebug("Current chapter has been deleted");

            ui->list->reset();
            ui->chapter->setChapter(QString());

            SETTINGS_SET_STRING(SETTING_LAST_CHAPTER, QString());
        }
        else
            ui->list->maybeUpdateCurrentChapter();
    }
}

bool Phrasebooks::setForeignFocus(const Link &link)
{
#ifdef Q_OS_WIN32
    const DWORD currentThreadId = GetCurrentThreadId();

    if(!AttachThreadInput(link.threadId, currentThreadId, TRUE))
    {
        qWarning("Cannot attach to the thread 0x%lx (0x%lx)", link.threadId, GetLastError());
        return false;
    }

    HWND hwnd = SetFocus(link.subControl);

    if(link.subControlSupportsClearing)
        SendMessage(link.subControl, WM_SETTEXT, 0, 0);

    SendMessage(link.hwnd, WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_SYSCHARSET, 0x04090409);

    AttachThreadInput(link.threadId, currentThreadId, FALSE);

    if(!hwnd)
    {
        qWarning("Cannot set focus to the subcontrol 0x%lx (0x%lx)", Utils::windowHandleToLong(link.subControl), GetLastError());
        return false;
    }
    else
        qDebug("Subcontrol 0x%lx has been focused", Utils::windowHandleToLong(link.subControl));
#else
    Q_UNUSED(link)
#endif

    return true;
}
