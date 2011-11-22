/*
 *    EWMH atom support. initial implementation borrowed from
 *    awesome wm, then partially reworked.
 *
 *    Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *    Copyright © 2008 Alexander Polakov <polachok@gmail.com>
 *
 */

#include <regex.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include "echinus.h"
#include "config.h"

Atom atom[NATOMS];

/* keep in sync with enum in echinus.h */
const char *atomnames[NATOMS][1] = {
	{ "_NET_CLIENT_LIST"		},
	{ "_NET_ACTIVE_WINDOW"		},
	{ "_NET_WM_DESKTOP"		},
	{ "_NET_NUMBER_OF_DESKTOPS"	},
	{ "_NET_DESKTOP_NAMES"		},
	{ "_NET_CURRENT_DESKTOP"	},
	{ "_ECHINUS_LAYOUT"		},
	{ "_NET_WORKAREA"		},
	{ "_NET_CLIENT_LIST_STACKING"	},
	{ "_NET_WM_WINDOW_OPACITY"	},
	{ "_NET_WM_WINDOW_TYPE"		},
	{ "_NET_WM_WINDOW_TYPE_DESKTOP"	},
	{ "_NET_WM_WINDOW_TYPE_DOCK"	},
	{ "_NET_WM_WINDOW_TYPE_DIALOG"	},
	{ "_NET_WM_STRUT_PARTIAL"	},
	{ "_ECHINUS_SELTAGS"		},
	{ "_NET_WM_NAME"		},
	{ "_NET_WM_STATE"		},
	{ "_NET_WM_STATE_FULLSCREEN"	},
	{ "_NET_WM_STATE_MODAL"		},
	{ "_NET_WM_STATE_HIDDEN"	},
	{ "_NET_SUPPORTING_WM_CHECK"	},
	{ "UTF8_STRING"			},
	{ "_NET_SUPPORTED"		},
	{ "WM_PROTOCOLS"		},
	{ "WM_DELETE_WINDOW"		},
	{ "WM_NAME"			},
	{ "WM_STATE"			},
	{ "WM_CHANGE_STATE"		},
	{ "WM_TAKE_FOCUS"		},
	{ "_MOTIF_WM_HINTS"		},
};

void
initewmh(void) {
	int i;
	char name[] = "echinus";
	XSetWindowAttributes wa;
	Window win;

	for (i = 0; i < NATOMS; i++)
		atom[i] = XInternAtom(dpy, atomnames[i][0], False);
	XChangeProperty(dpy, root,
	    atom[Supported], XA_ATOM, 32,
	    PropModeReplace, (unsigned char *) atom, NATOMS);

	wa.override_redirect = True;
	win = XCreateWindow(dpy, root, -100, 0, 1, 1,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), CWOverrideRedirect, &wa);
	XChangeProperty(dpy, win, atom[WindowName], atom[Utf8String], 8,
		       	PropModeReplace, (unsigned char*)name, strlen(name));
	XChangeProperty(dpy, root, atom[WMCheck], XA_WINDOW, 32,
		       	PropModeReplace, (unsigned char*)&win, 1);
}

void
update_echinus_layout_name() {
	XChangeProperty(dpy, root, atom[ELayout],
	    XA_STRING, 8, PropModeReplace,
	    (const unsigned char *) &views[curmontag].layout->symbol, 1L);
}

void
ewmh_update_net_client_list() {
	Window *wins = NULL;
	Client *c;
	int i, n = 0;

	for (c = stack; c; c = c->snext)
		n++;
	if (!n) {
		XChangeProperty(dpy, root, atom[ClientList], XA_WINDOW, 32,
			       	PropModeReplace, (unsigned char *) wins, n);
		XChangeProperty(dpy, root, atom[ClientListStacking], XA_WINDOW,
			       	32, PropModeReplace, (unsigned char *) wins, n);
		return;
	}
	wins = malloc(sizeof(Window) * n);
	for (i = 0, c = stack; c; c = c->snext)
		wins[i++] = c->win;
	XChangeProperty(dpy, root,
	    atom[ClientListStacking], XA_WINDOW, 32, PropModeReplace,
	    (unsigned char *) wins, n);
	for (i = 0, c = clients; c; c = c->next)
		wins[i++] = c->win;
	XChangeProperty(dpy, root,
	    atom[ClientList], XA_WINDOW, 32, PropModeReplace, (unsigned char *) wins, n);
	free(wins);
	XFlush(dpy);
}

void
ewmh_update_net_number_of_desktops() {
	XChangeProperty(dpy, root,
	    atom[NumberOfDesk], XA_CARDINAL, 32, PropModeReplace,
	    (unsigned char *) &ntags, 1);
}

void
ewmh_update_net_current_desktop() {
	Monitor *m;
	unsigned long *seltags;
	unsigned int i;

	seltags = emallocz(ntags * sizeof(unsigned long));
	for (m = monitors; m != NULL; m = m->next) {
		for (i = 0; i < ntags; i++)
			seltags[i] |= m->seltags[i];
	}
	XChangeProperty(dpy, root,
	    atom[ESelTags], XA_CARDINAL, 32, PropModeReplace,
	    (unsigned char *) seltags, ntags);
	XChangeProperty(dpy, root, atom[CurDesk], XA_CARDINAL, 32,
	    PropModeReplace, (unsigned char *) &curmontag, 1);
	update_echinus_layout_name(NULL);
	free(seltags);
}

void
ewmh_update_net_window_desktop(void *p) {
	unsigned int i;
	Client *c = (Client *)p;

	for (i = 0; i < ntags && !c->tags[i]; i++);
	XChangeProperty(dpy, c->win,
	    atom[WindowDesk], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &i, 1);
}

void
ewmh_update_net_work_area() {
	unsigned long *geoms;
	Monitor *m = monitors;
	int i, x, y, w, h;

	geoms = malloc(sizeof(unsigned long)*4*ntags);
	x = m->wax - m->sx;
	y = m->way - m->sy;
	w = m->waw;
	h = m->wah;
	for (m = m->next; m != NULL; m = m->next) {
		x = max(x, m->wax - m->sx);
		y = max(y, m->way - m->sy);
		w = min(w, m->waw);
		h = min(h, m->wah);
	}
	for (i = 0; i < ntags; i++) {
		geoms[i*4] = x;
		geoms[i*4+1] = y;
		geoms[i*4+2] = w;
		geoms[i*4+3] = h;
	}
	XChangeProperty(dpy, root,
	    atom[WorkArea], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) geoms, ntags*4);
	free(geoms);
}

void
ewmh_update_net_desktop_names() {
	char buf[1024], *pos;
	unsigned int i;
	int len = 0;

	pos = buf;
	for (i = 0; i < ntags; i++) {
		snprintf(pos, strlen(tags[i]) + 1, "%s", tags[i]);
		pos += (strlen(tags[i]) + 1);
	}
	len = pos - buf;

	XChangeProperty(dpy, root,
	    atom[DeskNames], atom[Utf8String], 8, PropModeReplace,
	    (unsigned char *) buf, len);
}

void
ewmh_update_net_active_window() {
	Window win;

	win = sel ? sel->win : None;
	XChangeProperty(dpy, root,
	    atom[ActiveWindow], XA_WINDOW, 32, PropModeReplace,
	    (unsigned char *) &win, 1);
}

void
mwm_process_atom(Client *c) {
	Atom real;
	int format;
	unsigned char *data = NULL;
	CARD32 *hint;
	unsigned long n, extra;
#define MWM_HINTS_ELEMENTS 5
#define MWM_DECOR_ALL(x) ((x) & (1L << 0))
#define MWM_DECOR_TITLE(x) ((x) & (1L << 3))
#define MWM_DECOR_BORDER(x) ((x) & (1L << 1))
#define MWM_HINTS_DECOR(x) ((x) & (1L << 1))
	if (XGetWindowProperty(dpy, c->win, atom[MWMHints], 0L, 20L, False,
		atom[MWMHints], &real, &format, &n, &extra,
		(unsigned char **) &data) == Success && n >= MWM_HINTS_ELEMENTS) {
		hint = (CARD32 *) data;
		if (MWM_HINTS_DECOR(hint[0]) && !(MWM_DECOR_ALL(hint[2]))) {
			c->title = MWM_DECOR_TITLE(hint[2]) ? root : (Window) NULL;
			c->border = MWM_DECOR_BORDER(hint[2]) ? style.border : 0;
		}
	}
	XFree(data);
}

void
ewmh_process_state_atom(Client *c, Atom state, int set) {
	CARD32 data[2];

	data[1] = None;
	if (state == atom[WindowStateFs]) {
		focus(c);
		if (set && !c->ismax) {
			c->wasfloating = c->isfloating;
			c->isfloating = True;
			data[0] = state;
		} else {
			c->isfloating = c->wasfloating;
			c->wasfloating = True;
			data[0] = None;
		}
		XChangeProperty(dpy, c->win, atom[WindowState], XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) data, 2);
		DPRINT;
		togglemax(NULL);
		arrange(curmonitor());
		DPRINTF("%s: x%d y%d w%d h%d\n", c->name, c->x, c->y, c->w, c->h);
	}
	if (state == atom[WindowStateModal])
		focus(c);
}

void
clientmessage(XEvent *e) {
	XClientMessageEvent *ev = &e->xclient;
	Client *c;

	if (ev->message_type == atom[ActiveWindow]) {
		if ((c = getclient(ev->window, clients, ClientWindow))) {
				c->isicon = False;
				focus(c);
				arrange(curmonitor());
		}
	} else if (ev->message_type == atom[CurDesk]) {
		view(tags[ev->data.l[0]]);
	} else if (ev->message_type == atom[WindowState]) {
		if ((c = getclient(ev->window, clients, ClientWindow))) {
			ewmh_process_state_atom(c, (Atom) ev->data.l[1], ev->data.l[0]);
			if (ev->data.l[2])
				ewmh_process_state_atom(c,
				    (Atom) ev->data.l[2], ev->data.l[0]);
		}
	} else if (ev->message_type == atom[WMChangeState]) {
		if ((c = getclient(ev->window, clients, ClientWindow))) {
			if (ev->data.l[0] == IconicState) {
				focus(c);
				iconify(NULL);
			}
		}
	}
}

void
setopacity(Client *c, unsigned int opacity) {
	if (opacity == OPAQUE) {
		XDeleteProperty(dpy, c->win, atom[WindowOpacity]);
		XDeleteProperty(dpy, c->frame, atom[WindowOpacity]);
	} else {
		XChangeProperty(dpy, c->win, atom[WindowOpacity],
		    XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &opacity, 1L);
		XChangeProperty(dpy, c->frame, atom[WindowOpacity],
		    XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &opacity, 1L);

	}
}

void *
getatom(Window win, Atom atom, unsigned long *nitems) {
	int format, status;
	unsigned char *ret = NULL;
	unsigned long extra;
	Atom real;

	status = XGetWindowProperty(dpy, win, atom, 0L, 64L, False, AnyPropertyType,
			&real, &format, nitems, &extra, (unsigned char **)&ret);
	if (status != Success) {
		*nitems = 0;
		return NULL;
	}

	return ret;
}

Bool
checkatom(Window win, Atom bigatom, Atom smallatom) {
	Atom *state;
	unsigned long i, n;
	Bool ret = False;

	state = (Atom*)getatom(win, bigatom, &n);
	for (i = 0; i < n; i++) {
		if (state[i] == smallatom)
			ret = True;
	}
	XFree(state);
	return ret;
}

int
getstruts(Client *c) {
	unsigned long *state;
	int ret = 0;
	Monitor *m;
	unsigned long i, n;

	if (!(m = clientmonitor(c)))
		return ret;

	state = (unsigned long*)getatom(c->win, atom[StrutPartial], &n);
	if (n) {
		for (i = LeftStrut; i < LastStrut; i++)
			m->struts[i] = max(state[i], m->struts[i]);
		ret = 1;
	}
	XFree(state);
	return ret;
}

void (*updateatom[]) (void *) = {
	[ClientList] = ewmh_update_net_client_list,
	[ActiveWindow] = ewmh_update_net_active_window,
	[WindowDesk] = ewmh_update_net_window_desktop,
	[NumberOfDesk] = ewmh_update_net_number_of_desktops,
	[DeskNames] = ewmh_update_net_desktop_names,
	[CurDesk] = ewmh_update_net_current_desktop,
	[ELayout] = update_echinus_layout_name,
	[WorkArea] = ewmh_update_net_work_area,
};
