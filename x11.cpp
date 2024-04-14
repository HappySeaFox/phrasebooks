#include <QX11Info>

#include <climits>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <strings.h>

#include <unistd.h>

#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>

#include "x11.h"

Atom X11::NET_ACTIVE_WINDOW = 0;
Atom X11::NET_WM_WINDOW_TYPE = 0;
Atom X11::NET_WM_WINDOW_TYPE_NORMAL = 0;
Atom X11::NET_CLIENT_LIST_STACKING = 0;

/**********************************************************/
                
void X11::init()
{
    Display *dpy = QX11Info::display();

    X11::NET_ACTIVE_WINDOW         = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    X11::NET_WM_WINDOW_TYPE        = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    X11::NET_WM_WINDOW_TYPE_NORMAL = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    X11::NET_CLIENT_LIST_STACKING  = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
}

QList<Window> X11::netwmWindowList()
{
    X11::checkInit();

    QList<Window> list;
    int num;

    Window *win = reinterpret_cast<Window *>(X11::property(QX11Info::appRootWindow(), X11::NET_CLIENT_LIST_STACKING, XA_WINDOW, &num));

    if(!win)
    {
        qDebug("X11: Cannot get window list");
        return list;
    }

    for(int i = 0;i < num;i++)
        list.append(win[i]);

    XFree(win);

    return list;
}

bool X11::netwmIsWindowNormal(Window win)
{
    X11::checkInit();

    bool normal = false;
    Atom *state;
    int num3;
    bool ok;

    if(!(state = (Atom *)X11::property(win, X11::NET_WM_WINDOW_TYPE, XA_ATOM, &num3, &ok)))
        return false;

    while(--num3 >= 0)
    {
        if(state[num3] == X11::NET_WM_WINDOW_TYPE_NORMAL)
        {
            normal = true;
            break;
        }
    }

    XFree(state);

    return normal;
}

bool X11::netwmSetActiveWindow(Window window)
{
    X11::checkInit();

    XEvent event;

    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = QX11Info::display();
    event.xclient.window = window;
    event.xclient.message_type = X11::NET_ACTIVE_WINDOW;

    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;
    event.xclient.data.l[1] = CurrentTime;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    return XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, mask, &event);
}

Window X11::netwmActiveWindow()
{
    X11::checkInit();

    Window *result = reinterpret_cast<Window*>(X11::property(QX11Info::appRootWindow(), X11::NET_ACTIVE_WINDOW, XA_WINDOW, 0));
    Window window = *result;

    XFree(result);

    return window;
}

void* X11::property(Window win, Atom prop, Atom type, int *nitems, bool *ok)
{
    X11::checkInit();

    Atom type_ret;
    int format_ret;
    unsigned long items_ret;
    unsigned long after_ret;
    unsigned char *prop_data = 0;

    if(XGetWindowProperty(QX11Info::display(),
                            win,
                            prop,
                            0,
                            0x7fffffff,
                            False,
                            type,
                            &type_ret,
                            &format_ret,
                            &items_ret,
                            &after_ret,
                            &prop_data) != Success)
    {
        if(ok)
            *ok = false;

        return 0;
    }

    if(nitems)
        *nitems = items_ret;

    if(ok)
        *ok = true;

    return prop_data;
}

void X11::checkInit()
{
    if(!X11::NET_ACTIVE_WINDOW)
        X11::init();
}
