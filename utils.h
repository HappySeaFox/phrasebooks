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

#include <QString>
#include <QRegExp>

#include <windows.h>

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
    static HWND RealWindowFromPoint(POINT pt);

    /*
     *  Is the specified window desktop (or background shell window)?
     */
    static bool isDesktop(HWND hwnd);

    template<typename T>
    static void *pointerToVoidPointer(const T *p);

    /*
     *  Replace all whitespaces with &nbsp;
     */
    static QString nonBreakable(const QString &str);

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
     *  Returns translated version of "Open YouTube tutorial"
     */
    static inline QString openYoutubeTutorialTitle();

    /*
     *  Invalid QPoint
     */
    static const QPoint invalidQPoint;

private:
    Utils();

    static HWND FindBestChild(HWND hwndFound, POINT pt);
    static BOOL CALLBACK FindBestChildProc(HWND hwnd, LPARAM lParam);
};

template<typename T>
void *Utils::pointerToVoidPointer(const T *p)
{
    return reinterpret_cast<void *>(const_cast<T *>(p));
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

inline
QString Utils::openYoutubeTutorialTitle()
{
    //: This is the label on a menu item that user clicks to issue the command. Means "Click to open a video tutorial on YouTube"
    return QObject::tr("Open YouTube tutorial");
}

#define PHRASEBOOKS_SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

#endif // UTILS_H
