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
#include <QWidget>
#include <QRegExp>
#include <QObject>
#include <QPoint>
#include <QRect>

#include <climits>

#include "utils.h"

const QPoint Utils::invalidQPoint(INT_MIN, INT_MIN);

class UtilsPrivate
{
public:
    static HWND hwndGlobal;
    static DWORD dwArea;
};

HWND UtilsPrivate::hwndGlobal;
DWORD UtilsPrivate::dwArea;

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

HWND Utils::RealWindowFromPoint(POINT pt)
{
    HWND hWndPoint = WindowFromPoint(pt);

    if(!hWndPoint)
        return 0;

    hWndPoint = FindBestChild(hWndPoint, pt);

    while(hWndPoint && !IsWindowVisible(hWndPoint))
        hWndPoint = GetParent(hWndPoint);

    return hWndPoint;
}

bool Utils::isDesktop(HWND hwnd)
{
    return IsWindow(hwnd) && (hwnd == GetDesktopWindow() || hwnd == GetShellWindow());
}

QString Utils::nonBreakable(const QString &str)
{
    static QRegExp rxSpace("\\s");
    return QString(str).replace(rxSpace, "&nbsp;");
}
