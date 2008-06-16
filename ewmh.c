/*
 *    EWMH atom support. initial implementation borrowed from
 *    awesome wm, then partially reworked.
 *
 *    Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *    Copyright © 2008 Alexander Polakov <polachok@gmail.com>
 *
 */

#include <X11/Xatom.h>

enum { ClientList, ActiveWindow, WindowDesk,
      NumberOfDesk, DeskNames, CurDesk, ELayout,
      ClientListStacking, WindowOpacity, WindowType,
      WindowTypeDesk, WindowTypeDock, StrutPartial, ESelTags,
      WindowName, WindowState, WindowStateFs, Utf8String, Supported, NATOMS };

Atom atom[NATOMS];

#define LASTAtom ClientListStacking

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1

char* atomnames[NATOMS][1] = {
    { "_NET_CLIENT_LIST" },
    { "_NET_ACTIVE_WINDOW" },
    { "_NET_WM_DESKTOP" },
    { "_NET_NUMBER_OF_DESKTOPS" },
    { "_NET_DESKTOP_NAMES" },
    { "_NET_CURRENT_DESKTOP" },
    { "_ECHINUS_LAYOUT" },
    { "_NET_CLIENT_LIST_STACKING" },
    { "_NET_WM_WINDOW_OPACITY" },
    { "_NET_WM_WINDOW_TYPE" },
    { "_NET_WM_WINDOW_TYPE_DESKTOP" },
    { "_NET_WM_WINDOW_TYPE_DOCK" },
    { "_NET_WM_STRUT_PARTIAL" },
    { "_ECHINUS_SELTAGS" },
    { "_NET_WM_NAME" },
    { "_NET_WM_STATE" },
    { "_NET_WM_STATE_FULLSCREEN" },
    { "UTF8_STRING" }, 
    { "_NET_SUPPORTED" },
};

void
initatom(void) {
    int i;
    for(i = 0; i < NATOMS; i++){
        atom[i] = XInternAtom(dpy, atomnames[i][0], False);
    }
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[Supported], XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) atom, NATOMS);
}

void 
update_echinus_layout_name(Client *c){
	XChangeProperty(dpy, root, atom[ELayout], 
                XA_STRING, 8, PropModeReplace, 
                (unsigned char *) layouts[ltidxs[curtag]].symbol, 1L);
}

void
ewmh_update_net_client_list() {
    Window *wins;
    Client *c;
    int n = 0;

    for(c = clients; c; c = c->next)
            n++;
    for(c = bastards; c; c = c->next)
            n++;

    wins = malloc(sizeof(Window*)*n);

    for(n = 0, c = clients; c; c = c->next)
            wins[n++] = c->win;

    for(c = bastards; c; c = c->next)
            wins[n++] = c->win;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[ClientList], XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[ClientListStacking], XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    /* free wins here */
    free(wins);
    XFlush(dpy);
}

void
ewmh_update_net_number_of_desktops() {
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[NumberOfDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &ntags, 1);
}

void
ewmh_update_net_current_desktop() {
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[ESelTags], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) seltags, ntags);
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[CurDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &curtag, 1);
    update_echinus_layout_name(NULL);
}

void
ewmh_update_net_window_desktop(Client *c) {
    int i;

    for(i = 0; i < ntags && !c->tags[i]; i++);

    XChangeProperty(dpy, c->win,
                    atom[WindowDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &i, 1);
}

void
ewmh_update_net_desktop_names() {
    char buf[1024], *pos;
    int i;
    int len = 0;

    pos = buf;
    for(i = 0; i < ntags; i++) {
        sprintf(pos, "%s", tags[i]); 
        pos += (strlen(tags[i])+1);
    }
    len = pos - buf;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[DeskNames], atom[Utf8String], 8, PropModeReplace, (unsigned char *) buf, len);
}

void
ewmh_update_net_active_window() {
    Window win;

    win = sel ? sel->win : None;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    atom[ActiveWindow], XA_WINDOW, 32,  PropModeReplace, (unsigned char *) &win, 1);
}

void
ewmh_process_state_atom(Client *c, Atom state, int set) {
    if(state == atom[WindowStateFs]) {
        if(set == _NET_WM_STATE_ADD) {
            c->wasfloating = c->isfloating;
            c->isfloating = True;
            togglemax(NULL);
        }
        else if(set == _NET_WM_STATE_REMOVE) {
            c->isfloating = c->wasfloating;
            c->wasfloating = True;
            togglemax(NULL);
        }
        arrange();
        drawfloating();
    }
}

void
clientmessage(XEvent *e) {
    XClientMessageEvent *ev = &e->xclient;
    Client *c;

    if(ev->message_type == atom[ActiveWindow]) {
        focus(getclient(ev->window));
        restack();
    }
    else if(ev->message_type == atom[CurDesk]) {
        view(tags[ev->data.l[0]]);
    }
    if(ev->message_type == atom[WindowState]){ 
        if((c = getclient(ev->window))){   
            ewmh_process_state_atom(c, (Atom) ev->data.l[1], ev->data.l[0]);
            if(ev->data.l[2])
                    ewmh_process_state_atom(c, (Atom) ev->data.l[2], ev->data.l[0]);
            }
    }
}

void 
setopacity(Client *c, unsigned int opacity) {
    if (opacity == OPAQUE)
        XDeleteProperty (dpy, c->win, atom[WindowOpacity]);
    else
        XChangeProperty(dpy, c->win, atom[WindowOpacity], 
                XA_CARDINAL, 32, PropModeReplace, 
                (unsigned char *) &opacity, 1L);
}


static int
isbastard(Window win){
    Atom real, *state;
    int format;
    unsigned char *data = NULL;
    unsigned long i, n, extra;
    if(XGetWindowProperty(dpy, win, atom[WindowType], 0L, LONG_MAX, False,
                          XA_ATOM, &real, &format, &n, &extra,
                          (unsigned char **) &data) == Success && data)
        state = (Atom *) data;
        for(i = 0; i < n; i++){
            if(state[i] == atom[WindowTypeDock] || state[i] == atom[WindowTypeDesk])
                            return 1;
        }
    return 0;
}

static int
updatestruts(Window win){
    Atom real, *state;
    int format, i;
    unsigned char *data = NULL;
    unsigned long n, extra;
    if(XGetWindowProperty(dpy, win, atom[StrutPartial], 0L, LONG_MAX, False,
                          XA_CARDINAL, &real, &format, &n, &extra,
                          (unsigned char **) &data) == Success && data){
        state = (Atom *) data;
        if(n){
            for(i = LeftStrut; i < LastStrut; i++)
                struts[i] = (state[i] > struts[i]) ? state[i] : struts[i];
            updategeom();
            return 1;
        }
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
    [ELayout] = update_echinus_layout_name,
};
