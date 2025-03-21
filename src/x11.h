#ifndef X11_H
#define X11_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "fixx11h.h"

#include <QProcess>
#include <QList>

class X11
{

public:
    static QList<Window> netwmWindowList();
    static bool netwmIsWindowNormal(Window win);

    static bool netwmSetActiveWindow(Window window);
    static Window netwmActiveWindow();

    // atoms
    static Atom NET_ACTIVE_WINDOW;
    static Atom NET_WM_WINDOW_TYPE;
    static Atom NET_WM_WINDOW_TYPE_NORMAL;
    static Atom NET_CLIENT_LIST_STACKING;

private:
    static void* property(Window win, Atom prop, Atom type, int *nitems = 0, bool *ok = 0);

    static void checkInit();
    static void init();
};

#endif
