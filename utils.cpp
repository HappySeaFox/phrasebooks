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

#include <QDesktopWidget>
#include <QApplication>
#include <QDateTime>
#include <QThread>
#include <QWidget>
#include <QDebug>
#include <QPoint>
#include <QRect>

#include <climits>
#include <cstring>

#include "utils.h"

#ifdef Q_OS_UNIX
    #include <QX11Info>

    #include <X11/Intrinsic.h>
    #include <X11/extensions/XTest.h>

    #include "x11.h"
#endif

const QPoint Utils::invalidQPoint(INT_MIN, INT_MIN);

#ifdef Q_OS_WIN32
class UtilsPrivate
{
public:
    static HWND hwndGlobal;
    static DWORD dwArea;
};

HWND UtilsPrivate::hwndGlobal;
DWORD UtilsPrivate::dwArea;
#endif

/*****************************/

QString Utils::currentTime()
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
}

void Utils::raiseWindow(QWidget *w)
{
    if(!w)
        return;

    w->show();
    w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
    w->raise();

#ifdef Q_OS_UNIX
    X11::netwmSetActiveWindow(static_cast<Platform::WindowId>(w->winId()));
#endif
}

void Utils::moveWindow(QWidget *w, const QPoint &pt)
{
    if(!w)
        return;

    const int sn = qApp->desktop()->screenCount();
    const QRect headGeometry = QRect(pt, QSize(w->width(), 20));

    for(int screen = 0;screen < sn;screen++)
    {
        QRect dr = qApp->desktop()->availableGeometry(screen);

        // move to a valid position
        if(dr.contains(headGeometry.topLeft()) || dr.contains(headGeometry.bottomRight()))
        {
            w->move(pt);
            break;
        }
    }
}

Platform::WindowId Utils::winIdToNativeWindow(WId wid)
{
#ifdef Q_OS_WIN32
    return reinterpret_cast<Platform::WindowId>(wid);
#else
    return static_cast<Platform::WindowId>(wid);
#endif
}

/*
 * FindBestChildProc
 * FindBestChild
 * RealWindowFromPoint
 *
 * are written by
 *
 * Copyright (c) 2002 by J Brown
 *
 * License: Freeware
 */
#ifdef Q_OS_WIN32
BOOL CALLBACK Utils::FindBestChildProc(HWND hwnd, LPARAM lParam)
{
    RECT rect;
    POINT pt;

    pt.x = (short)LOWORD(lParam);
    pt.y = (short)HIWORD(lParam);

    GetWindowRect(hwnd, &rect);

    if(PtInRect(&rect, pt))
    {
        DWORD a = (rect.right - rect.left) * (rect.bottom - rect.top);

        if(a < UtilsPrivate::dwArea && IsWindowVisible(hwnd))
        {
            UtilsPrivate::dwArea = a;
            UtilsPrivate::hwndGlobal = hwnd;
        }
    }

    return TRUE;
}

HWND Utils::FindBestChild(HWND hwndFound, POINT pt)
{
    HWND hwnd;
    DWORD dwStyle;

    UtilsPrivate::dwArea = -1;
    UtilsPrivate::hwndGlobal = 0;

    hwnd = GetParent(hwndFound);

    dwStyle = GetWindowLong(hwndFound, GWL_STYLE);

    if(!hwnd || (dwStyle & WS_POPUP))
        hwnd = hwndFound;

    EnumChildWindows(hwnd, FindBestChildProc, MAKELPARAM(pt.x, pt.y));

    if(!UtilsPrivate::hwndGlobal)
        UtilsPrivate::hwndGlobal = hwnd;

    return UtilsPrivate::hwndGlobal;
}

Platform::WindowId Utils::RealWindowFromPoint(const QPoint &qtpt)
{
    const POINT pt = { qtpt.x(), qtpt.y() };

    HWND hWndPoint = WindowFromPoint(pt);

    if(!hWndPoint)
        return 0;

    hWndPoint = FindBestChild(hWndPoint, pt);

    while(hWndPoint && !IsWindowVisible(hWndPoint))
        hWndPoint = GetParent(hWndPoint);

    return hWndPoint;
}
#endif

Platform::WindowId Utils::topLevelWindowFromPoint(const QPoint &qtpt)
{
#ifdef Q_OS_WIN32
    const POINT pt = { qtpt.x(), qtpt.y() };
    return RealChildWindowFromPoint(GetDesktopWindow(), pt);
#else
    Platform::WindowId found = 0;

    const QList<Window> windows = X11::netwmWindowList();

    for(int i = windows.size()-1;i >= 0;i--)
    {
        Window window = windows.at(i);

        if(!X11::netwmIsWindowNormal(window))
            continue;

        Window wunused;
        int x, y;
        unsigned int width, height, rest;

        if(!XGetGeometry(QX11Info::display(), window, &wunused, &x, &y, &width, &height, &rest, &rest))
            continue;

        XTranslateCoordinates(QX11Info::display(), window, QX11Info::appRootWindow(), 0, 0, &x, &y, &wunused);

        if(QRect(x, y, width, height).contains(qtpt))
        {
            found = window;
            break;
        }
    }

    return found;
#endif
}

Platform::WindowId Utils::activeWindow()
{
#ifdef Q_OS_WIN32
    return GetForegroundWindow();
#else
    return X11::netwmActiveWindow();
#endif
}

void Utils::bringToFront(Platform::WindowId window)
{
#ifdef Q_OS_WIN32
    qDebug("Bring to front 0x%lx", Utils::windowHandleToLong(window));

    // window flags to set
    int flags = SW_SHOWNORMAL;

    if(IsZoomed(window))
        flags |= SW_SHOWMAXIMIZED;

    // try to switch to this window
    ShowWindow(window, flags);
    SetForegroundWindow(window);
#else
    X11::netwmSetActiveWindow(window);
#endif
}

bool Utils::isWindow(const Platform::WindowId &id)
{
#ifdef Q_OS_WIN32
    return IsWindow(id);
#else
    XWindowAttributes attr;
    return XGetWindowAttributes(QX11Info::display(), id, &attr);
#endif
}

void Utils::beep()
{
#ifdef Q_OS_WIN32
    MessageBeep(MB_OK);
#endif
}

class Sleeper : private QThread
{
public:
    static void sleep(const int ms)
    {
        QThread::msleep(ms);
    }
};

void Utils::sleep(const int ms)
{
    Sleeper::sleep(ms);
}

bool Utils::isDesktop(Platform::WindowId hwnd)
{
#ifdef Q_OS_WIN32
    return IsWindow(hwnd) && (hwnd == GetDesktopWindow() || hwnd == GetShellWindow());
#else
    Q_UNUSED(hwnd)
    return false;
#endif
}

void Utils::startSendingKeys()
{
#ifndef Q_OS_WIN32
    XSync(QX11Info::display(), False);
    XTestGrabControl(QX11Info::display(), True);
#endif
}

void Utils::sendKey(int key, bool extended)
{
    // do we need the SHIFT key?
    static const QString shiftChars = "!@#$%^&*()_+~<>?:\"|{}ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const bool shift = shiftChars.indexOf(key) >= 0;

#ifdef Q_OS_WIN32
    KEYBDINPUT kbInput = {0, 0, 0, 0, 0};
    INPUT input[4];

    memset(input, 0, sizeof(input));

    int nelem = 2;
    int index = 0;

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
#else
    Q_UNUSED(extended)

    KeyCode keycode = XKeysymToKeycode(QX11Info::display(), key);
    KeyCode modcode = 0;

    if(!keycode)
        return;

    if(shift)
    {
        modcode = XKeysymToKeycode(QX11Info::display(), XK_Shift_L);
        XTestFakeKeyEvent(QX11Info::display(), modcode, True, 0);
    }

    XTestFakeKeyEvent(QX11Info::display(), keycode, True, 0);
    XTestFakeKeyEvent(QX11Info::display(), keycode, False, 0);

    if(shift)
        XTestFakeKeyEvent(QX11Info::display(), modcode, False, 0);
#endif
}

void Utils::stopSendingKeys()
{
#ifndef Q_OS_WIN32
    XSync(QX11Info::display(), False);
    XTestGrabControl(QX11Info::display(), False);
    sleep(250);
#endif
}

void Utils::sendReturn()
{
#ifdef Q_OS_WIN32
    sendKey(VK_RETURN);
#else
    sendKey(XK_Return);
#endif
}
