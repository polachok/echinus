/*
 *    EWMH atoms support. borrowed from awesome wm
 *
 *    Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *
 */

#include <X11/Xatom.h>

enum { ClientList, ActiveWindow, WindowDesk,
      NumberOfDesk, DeskNames, CurDesk, ClientListStacking,
      WindowOpacity, WindowType, WindowTypeDesk,
      WindowTypeDock, StrutPartial, ESelTags,
      WmName, Utf8String, Supported, NATOMS };

Atom atoms[NATOMS];

#define LASTAtom ClientListStacking

char* atomnames[NATOMS][1] = {
    { "_NET_CLIENT_LIST" },
    { "_NET_ACTIVE_WINDOW" },
    { "_NET_WM_DESKTOP" },
    { "_NET_NUMBER_OF_DESKTOPS" },
    { "_NET_DESKTOP_NAMES" },
    { "_NET_CURRENT_DESKTOP" },
    { "_NET_CLIENT_LIST_STACKING" },
    { "_NET_WM_WINDOW_OPACITY" },
    { "_NET_WM_WINDOW_TYPE" },
    { "_NET_WM_WINDOW_TYPE_DESKTOP" },
    { "_NET_WM_WINDOW_TYPE_DOCK" },
    { "_NET_WM_STRUT_PARTIAL" },
    { "_ECHINUS_SELTAGS" },
    { "_NET_WM_NAME" },
    { "UTF8_STRING" }, 
    { "_NET_SUPPORTED" },
};

void
initatoms(void) {
    int i;
    for(i = 0; i < NATOMS; i++){
        atoms[i] = XInternAtom(dpy, atomnames[i][0], False);
    }
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[Supported], XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) atoms, NATOMS);
}

void
ewmh_update_net_client_list() {
    Window *wins;
    Client *c;
    int n = 0;

    for(c = clients; c; c = c->next)
            n++;

    wins = malloc(sizeof(Window*)*n);

    for(n = 0, c = clients; c; c = c->next, n++)
            wins[n] = c->win;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[ClientList], XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[ClientListStacking], XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    /* free wins here */
    XFlush(dpy);
}

void
ewmh_update_net_number_of_desktops() {
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[NumberOfDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ntags, 1);
}

void
ewmh_update_net_current_desktop() {
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[ESelTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) seltags, ntags);
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[CurDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &curtag, 1);
}

void
ewmh_update_net_window_desktop(Client *c) {
    CARD32 count = 0;
    int i;

    for(i = 0; i < ntags; i++){
        if(c->tags[i]){
            count=i;
            break;
        }
    }

    XChangeProperty(dpy, c->win,
                    atoms[WindowDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &count, 1);
}

void
ewmh_update_net_desktop_names() {
    char buf[1024], *pos;
    ssize_t len, curr_size;
    int i;

    pos = buf;
    len = 0;
    for(i = 0; i < ntags; i++) {
        curr_size = strlen(tags[i]);
        strcpy(pos, tags[i]);
        pos += curr_size;
        strcpy(pos, "\0");
        pos++;
    }
    len = pos - buf;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[DeskNames], atoms[Utf8String], 8, PropModeReplace, (unsigned char *) buf, len);
}

void
ewmh_update_net_active_window() {
    Window win;

    win = sel ? sel->win : None;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atoms[ActiveWindow], XA_WINDOW, 32,  PropModeReplace, (unsigned char *) &win, 1);
}

void
clientmessage(XEvent *e) {
    XClientMessageEvent *ev = &e->xclient;

    if(ev->message_type == atoms[ActiveWindow]) {
        focus(getclient(ev->window));
        restack();
    }
    if(ev->message_type == atoms[CurDesk]) {
        view(tags[ev->data.l[0]]);
    }
}

void 
setopacity(Client *c, unsigned int opacity) {
    if (opacity == OPAQUE)
        XDeleteProperty (dpy, c->win, atoms[WindowOpacity]);
    else
        XChangeProperty(dpy, c->win, atoms[WindowOpacity], 
                XA_CARDINAL, 32, PropModeReplace, 
                (unsigned char *) &opacity, 1L);
}


static int
isspecial(Window win){
    Atom real, *state;
    int format;
    unsigned char *data = NULL;
    unsigned long i, n, extra;
    if(XGetWindowProperty(dpy, win, atoms[WindowType], 0L, LONG_MAX, False,
                          XA_ATOM, &real, &format, &n, &extra,
                          (unsigned char **) &data) == Success && data)
        state = (Atom *) data;
        for(i = 0; i < n; i++){
            if(state[i] == atoms[WindowTypeDock] || state[i] == atoms[WindowTypeDesk])
                            return 1;
        }
    return 0;
}

static int
updatestruts(Window win){
    Atom real, *state;
    int format;
    unsigned char *data = NULL;
    unsigned long n, extra;
    if(XGetWindowProperty(dpy, win, atoms[StrutPartial], 0L, LONG_MAX, False,
                          XA_CARDINAL, &real, &format, &n, &extra,
                          (unsigned char **) &data) == Success && data)
        state = (Atom *) data;
        if(n){
            struts[LeftStrut] = state[0] > LeftStrut ? state[0] : struts[LeftStrut];
            struts[RightStrut] = state[1] > RightStrut ? state[1] : struts[RightStrut];
            struts[TopStrut] = state[2] > TopStrut ? state[2] : struts[TopStrut];
            struts[BotStrut] = state[3] > BotStrut ? state[3] : struts[BotStrut];
            updategeom();
            return 1;
        }
    return 0;
}

void (*updateatom[LASTAtom]) (Client *) = {
    [ClientList] = ewmh_update_net_client_list,
    [ActiveWindow] = ewmh_update_net_active_window,
    [WindowDesk] = ewmh_update_net_window_desktop,
    [NumberOfDesk] = ewmh_update_net_number_of_desktops,
    [DeskNames] = ewmh_update_net_desktop_names,
    [CurDesk] = ewmh_update_net_current_desktop,
};
