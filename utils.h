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

#ifndef UTILS_H
#define UTILS_H

#include "platform.h"

#include <QString>
#include <QObject>

class QWidget;
class QPoint;

class Utils
{
public:
    static QString currentTime();

    /*
     *  Show and raise the specified window
     */
    static void raiseWindow(QWidget *);

    /*
     *  Move the window to the specified point
     */
    static void moveWindow(QWidget *w, const QPoint &pt);

    /*
     *  RealWindowFromPoint() from WIN32 API with fixes
     */
#ifdef Q_OS_WIN32
    static Platform::WindowId RealWindowFromPoint(const QPoint &qtpt);
#endif

    /*
     *  Top-level window under the point
     */
    static Platform::WindowId topLevelWindowFromPoint(const QPoint &qtpt);

    static Platform::WindowId activeWindow();

    static void bringToFront(Platform::WindowId window);

    static bool isWindow(const Platform::WindowId &id);

    static void beep();

    static void sleep(const int ms);

    /*
     *  Is the specified window desktop (or background shell window)?
     */
    static bool isDesktop(Platform::WindowId hwnd);

    static unsigned long windowHandleToLong(const Platform::WindowId &id);

    /*
     *  Returns translated version of "About Phrasebooks"
     */
    static inline QString aboutPhrasebooks();

    /*
     *  Returns translated version of "OK"
     */
    static inline QString oKTitle();

    /*
     *  Returns translated version of "Error"
     */
    static inline QString errorTitle();

    /*
     *  Returns translated version of "Network error #%1"
     */
    static inline QString networkErrorTitle();

    /*
     *  Simulate user input
     */
    static void sendKey(int key, bool extended = false);

    static void sendReturn();

    /*
     *  Invalid QPoint
     */
    static const QPoint invalidQPoint;

private:
    Utils();

#ifdef Q_OS_WIN32
    static HWND FindBestChild(HWND hwndFound, POINT pt);
    static BOOL CALLBACK FindBestChildProc(HWND hwnd, LPARAM lParam);
#endif
};

inline
unsigned long Utils::windowHandleToLong(const Platform::WindowId &id)
{
    return reinterpret_cast<unsigned long>(id);
}

inline
QString Utils::aboutPhrasebooks()
{
    //: "Phrasebooks" is the name of the application
    return QObject::tr("About Phrasebooks");
}

inline
QString Utils::oKTitle()
{
    return QObject::tr("OK");
}

inline
QString Utils::errorTitle()
{
    return QObject::tr("Error");
}

inline
QString Utils::networkErrorTitle()
{
    //: %1 will be replaced with the error code by the application. It will look like "Network error #16"
    return QObject::tr("Network error #%1");
}

#define PHRASEBOOKS_SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

#endif // UTILS_H
