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

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtGlobal>

#ifdef Q_OS_WIN32
    #include <windows.h>
#else
    #include <X11/Xlib.h>
    #include "fixx11h.h"
#endif

namespace Platform
{

#ifdef Q_OS_WIN32
    typedef HWND WindowId;
    typedef DWORD ResourceId;
#else
    typedef Window WindowId;
    typedef int ResourceId;
#endif

}

#endif // PLATFORM_H
