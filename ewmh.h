/*
 *    EWMH atoms support. borrowed from awesome wm
 *
 *    Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *
 */

#include <X11/Xatom.h>

static Atom net_supported;
static Atom net_client_list;
static Atom net_client_list_stacking;
static Atom net_number_of_desktops;
static Atom net_current_desktop;
static Atom net_desktop_names;
static Atom net_active_window;
static Atom net_window_opacity;
static Atom net_wm_window_type;
static Atom net_wm_window_type_dock;
Atom net_wm_name;
Atom net_window_desktop;
static Atom utf8_string;

typedef struct
{
    const char *name;
    Atom *atom;
} AtomItem;

static AtomItem AtomNames[] =
{
    { "_NET_SUPPORTED", &net_supported },
    { "_NET_CLIENT_LIST", &net_client_list },
    { "_NET_CLIENT_LIST_STACKING", &net_client_list_stacking },
    { "_NET_NUMBER_OF_DESKTOPS", &net_number_of_desktops },
    { "_NET_CURRENT_DESKTOP", &net_current_desktop },
    { "_NET_DESKTOP_NAMES", &net_desktop_names },
    { "_NET_ACTIVE_WINDOW", &net_active_window },
    { "_NET_WM_NAME", &net_wm_name },
    { "_NET_WM_DESKTOP", &net_window_desktop },
    { "_NET_WM_WINDOW_OPACITY", &net_window_opacity },
    { "_NET_WM_WINDOW_TYPE", &net_wm_window_type },
    { "_NET_WM_WINDOW_TYPE_DOCK", &net_wm_window_type_dock },
    { "UTF8_STRING", &utf8_string },
};

#define ATOM_NUMBER (sizeof(AtomNames)/sizeof(AtomItem)) 
#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
#define _NET_WM_STATE_TOGGLE 2

void
ewmh_init_atoms(void) {
    unsigned int i;
    char *names[ATOM_NUMBER];
    Atom atoms[ATOM_NUMBER];
    
    for(i = 0; i < ATOM_NUMBER; i++)
        names[i] = (char *) AtomNames[i].name;
    XInternAtoms(dpy, names, ATOM_NUMBER, False, atoms);
    for(i = 0; i < ATOM_NUMBER; i++)
        *AtomNames[i].atom = atoms[i];
}

void
ewmh_set_supported_hints() {
    Atom atom[ATOM_NUMBER];
    int i = 0;

    atom[i++] = net_supported;
    atom[i++] = net_client_list;
    atom[i++] = net_client_list_stacking;
    atom[i++] = net_number_of_desktops;
    atom[i++] = net_current_desktop;
    atom[i++] = net_desktop_names;
    atom[i++] = net_active_window;
    atom[i++] = net_wm_name;
    atom[i++] = net_wm_window_type;
    atom[i++] = net_wm_window_type_dock;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_supported, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) atom, i);

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
                    net_client_list, XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_client_list_stacking, XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
    /* free wins here */
    XFlush(dpy);
}

void
ewmh_update_net_numbers_of_desktop() {
    CARD32 count = 0;

    count=ndtags;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_number_of_desktops, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &count, 1);
}

void
ewmh_update_net_current_desktop() {
    CARD32 count = 0;
    int i;

    for(i = 0; i < ndtags; i++)
        if(seltags[i]){
            count=i;
            break;
        }

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_current_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &count, 1);
}

void
ewmh_update_net_window_desktop(Client *c) {
    CARD32 count = 0;
    int i;

    for(i = 0; i < ndtags; i++){
        if(c->tags[i]){
            count=i;
            break;
        }
    }

    XChangeProperty(dpy, c->win,
                    net_window_desktop, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &count, 1);
}

void
ewmh_update_net_desktop_names() {
    char buf[1024], *pos;
    ssize_t len, curr_size;
    int i;

    pos = buf;
    len = 0;
    for(i = 0; i < ndtags; i++)
    {
        curr_size = strlen(tags[i]);
        strcpy(pos, tags[i]);
        pos += curr_size;
        strcpy(pos, "\0");
        pos++;
    }
    len = pos - buf;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_desktop_names, utf8_string, 8, PropModeReplace, (unsigned char *) buf, len);
}

void
ewmh_update_net_active_window() {
    Window win;

    win = sel ? sel->win : None;

    XChangeProperty(dpy, RootWindow(dpy, screen),
                    net_active_window, XA_WINDOW, 32,  PropModeReplace, (unsigned char *) &win, 1);
}

void
ewmh_process_client_message(XEvent *e) {
    XClientMessageEvent *ev = &e->xclient;

    if(ev->message_type == net_active_window) {
        focus(getclient(ev->window));
        restack();
    }
    if(ev->message_type == net_current_desktop) {
        view(tags[ev->data.l[0]]);
    }
}

void 
ewmh_set_window_opacity(Client *c, unsigned int opacity) {
    if (opacity == OPAQUE)
        XDeleteProperty (dpy, c->win, net_window_opacity);
    else
        XChangeProperty(dpy, c->win, net_window_opacity, 
                XA_CARDINAL, 32, PropModeReplace, 
                (unsigned char *) &opacity, 1L);
}


static int
ewmh_process_window_type_atom(Window win)
{
    Atom real, *state;
    int format;
    unsigned char *data = NULL;
    unsigned long i, n, extra;
    if(XGetWindowProperty(dpy, win, net_wm_window_type, 0L, LONG_MAX, False,
                          XA_ATOM, &real, &format, &n, &extra,
                          (unsigned char **) &data) == Success && data)
        state = (Atom *) data;
        for(i = 0; i < n; i++){
            if(state[i] == net_wm_window_type_dock)
                            return 0;
        }
    return 1;
}
