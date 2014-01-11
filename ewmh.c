/*
 *    EWMH atom support. initial implementation borrowed from
 *    awesome wm, then partially reworked.
 *
 *    Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *    Copyright © 2008 Alexander Polakov <polachok@gmail.com>
 *
 */

#include <assert.h>
#include <unistd.h>
#include <regex.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#ifdef XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>
#endif
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#ifdef SYNC
#include <X11/extensions/sync.h>
#endif
#include "echinus.h"
#include "config.h"

Atom atom[NATOMS];

/* keep in sync with enum in echinus.h */
char *atomnames[NATOMS] = {
	"MANAGER",
	"UTF8_STRING",
	"WM_PROTOCOLS",
	"WM_DELETE_WINDOW",
	"WM_STATE",
	"WM_CHANGE_STATE",
	"WM_TAKE_FOCUS",
	"_MOTIF_WM_HINTS",
	"_ECHINUS_LAYOUT",
	"_ECHINUS_SELTAGS",
	"_NET_WM_FULLSCREEN_MONITORS",		/* TODO */
	"_NET_RESTART",				/* TODO */
	"_NET_SHUTDOWN",			/* TODO */
	"_NET_STARTUP_INFO_BEGIN",
	"_NET_STARTUP_INFO",
	"_NET_DESKTOP_LAYOUT",			/* TODO */
	/* _NET_SUPPORTED following */
	"_NET_CLIENT_LIST",
	"_NET_ACTIVE_WINDOW",
	"_NET_WM_DESKTOP",
	"_NET_WM_DESKTOP_MASK",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_DESKTOP_NAMES",
	"_NET_CURRENT_DESKTOP",
	"_NET_WORKAREA",
	"_NET_DESKTOP_VIEWPORT",
	"_NET_SHOWING_DESKTOP",
	"_NET_DESKTOP_GEOMETRY",

	"_NET_DESKTOP_MODES",
	"_NET_DESKTOP_MODE_FLOATING",
	"_NET_DESKTOP_MODE_TILED",
	"_NET_DESKTOP_MODE_BOTTOM_TILED",
	"_NET_DESKTOP_MODE_MONOCLE",
	"_NET_DESKTOP_MODE_TOP_TILED",
	"_NET_DESKTOP_MODE_LEFT_TILED",

	"_NET_CLIENT_LIST_STACKING",
	"_NET_WM_WINDOW_OPACITY",
	"_NET_MOVERESIZE_WINDOW",
	"_NET_RESTACK_WINDOW",
	"_NET_WM_MOVERESIZE",
	"_NET_FRAME_EXTENTS",
	"_NET_WM_HANDLED_ICONS",
	"_NET_REQUEST_FRAME_EXTENTS",
	"_NET_VIRTUAL_ROOTS",

	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DOCK",
	"_NET_WM_WINDOW_TYPE_TOOLBAR",
	"_NET_WM_WINDOW_TYPE_MENU",
	"_NET_WM_WINDOW_TYPE_UTILITY",
	"_NET_WM_WINDOW_TYPE_SPLASH",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
	"_NET_WM_WINDOW_TYPE_POPUP_MENU",
	"_NET_WM_WINDOW_TYPE_TOOLTIP",
	"_NET_WM_WINDOW_TYPE_NOTIFICATION",
	"_NET_WM_WINDOW_TYPE_COMBO",
	"_NET_WM_WINDOW_TYPE_DND",
	"_NET_WM_WINDOW_TYPE_NORMAL",

	"_NET_WM_STRUT_PARTIAL",
	"_NET_WM_STRUT",
	"_NET_WM_PID",
	"_NET_WM_NAME",
	"_NET_WM_VISIBLE_NAME",
	"_NET_WM_ICON_NAME",
	"_NET_WM_VISIBLE_ICON_NAME",
	"_NET_WM_USER_TIME",
	"_NET_WM_USER_TIME_WINDOW",
	"_NET_STARTUP_ID",
	"_NET_WM_SYNC_REQUEST",
	"_NET_WM_SYNC_REQUEST_COUNTER",

	"_NET_WM_STATE",
	"_NET_WM_STATE_MODAL",
	"_NET_WM_STATE_STICKY",
	"_NET_WM_STATE_MAXIMIZED_VERT",
	"_NET_WM_STATE_MAXIMIZED_HORZ",
	"_NET_WM_STATE_SHADED",
	"_NET_WM_STATE_SKIP_TASKBAR",
	"_NET_WM_STATE_SKIP_PAGER",
	"_NET_WM_STATE_HIDDEN",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_STATE_ABOVE",
	"_NET_WM_STATE_BELOW",
	"_NET_WM_STATE_DEMANDS_ATTENTION",
	"_NET_WM_STATE_FOCUSED",
	"_NET_WM_STATE_FIXED",
	"_NET_WM_STATE_FLOATING",
	"_NET_WM_STATE_FILLED",

	"_NET_WM_ALLOWED_ACTIONS",
	"_NET_WM_ACTION_ABOVE",
	"_NET_WM_ACTION_BELOW",
	"_NET_WM_ACTION_CHANGE_DESKTOP",
	"_NET_WM_ACTION_CLOSE",
	"_NET_WM_ACTION_FULLSCREEN",
	"_NET_WM_ACTION_MAXIMIZE_HORZ",
	"_NET_WM_ACTION_MAXIMIZE_VERT",
	"_NET_WM_ACTION_MINIMIZE",
	"_NET_WM_ACTION_MOVE",
	"_NET_WM_ACTION_RESIZE",
	"_NET_WM_ACTION_SHADE",
	"_NET_WM_ACTION_STICK",
	"_NET_WM_ACTION_FLOAT",
	"_NET_WM_ACTION_FILL",

	"_NET_SUPPORTING_WM_CHECK",
	"_NET_CLOSE_WINDOW",
	"_NET_SUPPORTED",

	"_KDE_NET_SYSTEM_TRAY_WINDOWS",
	"_KDE_NET_WM_FRAME_STRUT",
	"_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR",
	"_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
	"_KDE_SPLASH_PROGRESS",
	"_KDE_WM_CHANGE_STATE"
};

#define _NET_WM_STATE_REMOVE	0
#define _NET_WM_STATE_ADD	1
#define _NET_WM_STATE_TOGGLE	2

void
initewmh(Window win) {
	char name[] = "echinus";
	long data;

	XInternAtoms(dpy, atomnames, NATOMS, False, atom);
	XChangeProperty(dpy, root, atom[Supported], XA_ATOM, 32,
	    PropModeReplace, (unsigned char *) &atom[ClientList],
	    NATOMS - ClientList);

	XChangeProperty(dpy, win, atom[WindowName], atom[Utf8String], 8,
		       	PropModeReplace, (unsigned char*)name, strlen(name));
	data = getpid();
	XChangeProperty(dpy, win, atom[WindowPid], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char*)&data, 1);
	XChangeProperty(dpy, root, atom[WMCheck], XA_WINDOW, 32,
		       	PropModeReplace, (unsigned char*)&win, 1);
	XChangeProperty(dpy, win, atom[WMCheck], XA_WINDOW, 32,
			PropModeReplace, (unsigned char*)&win, 1);
}

void
update_echinus_layout_name(Client * c) {
	XChangeProperty(dpy, root, atom[ELayout], XA_STRING, 8, PropModeReplace,
			(const unsigned char *) &views[curmontag].layout->symbol, 1L);
}

void
ewmh_update_net_client_list_stacking(Client * c) {
	Window *wl = NULL;
	int i, n;

	DPRINTF("%s\n", "Updating _NET_CLIENT_LIST_STACKING");
	for (n = 0, c = stack; c; n++, c = c->snext) ;
	if (n)
		wl = ecalloc(n, sizeof(Window));
	for (i = 0, c = stack; c && i < n; c = c->snext)
		wl[i++] = c->win;
	assert(i == n);
	XChangeProperty(dpy, root, atom[ClientListStacking], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) wl, n);
	free(wl);
	XFlush(dpy);		/* XXX: caller's responsibility */
}

void
ewmh_update_net_client_list(Client * c) {
	Window *wl = NULL;
	int i, n;

	DPRINTF("%s\n", "Updating _NET_CLIENT_LIST");
	for (n = 0, c = clients; c; c = c->next)
		n++;
	if (n && (wl = calloc(n, sizeof(Window))))
		for (i = 0, c = clients; c; c = c->next)
			wl[i++] = c->win;
	XChangeProperty(dpy, root, atom[ClientList], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) wl, n);
	free(wl);
	XFlush(dpy); /* XXX: caller's responsibility */
}

void
ewmh_update_net_number_of_desktops(Client * c) {
	long data = ntags;

	DPRINTF("%s\n", "Updating _NET_NUMBER_OF_DESKTOPS");
	XChangeProperty(dpy, root, atom[NumberOfDesk], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);
}

unsigned int
ewmh_process_net_number_of_desktops() {
	long *card;
	unsigned long n = 0;
	long numb = 0;

	card = getcard(root, atom[NumberOfDesk], &n);
	if (n > 0) {
		numb = card[0];
		XFree(card);
	}
	if (1 > numb || numb > 64)
		numb = atoi(getresource("tags.number", "5"));
	return (unsigned int) numb;
}

void
ewmh_update_net_desktop_viewport(Client *c) {
	long *data;

	data = ecalloc(ntags * 2, sizeof(data[0]));
	XChangeProperty(dpy, root, atom[DeskViewport], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, ntags * 2);
}

void
ewmh_process_net_showing_desktop() {
	long *card;
	unsigned long n = 0;

	DPRINTF("%s\n", "Processing _NET_SHOWING_DESKTOP");
	card = getcard(root, atom[ShowingDesktop], &n);
	if (n > 0) {
		Bool show = card[0];

		XFree(card);
		if (!show != !showing_desktop)
			toggleshowing();
	}
}


void
ewmh_update_net_showing_desktop(Client *c) {
	long data = showing_desktop ? 1 : 0;

	DPRINTF("%s\n", "Updating _NET_SHOWING_DESKTOP");
	XChangeProperty(dpy, root, atom[ShowingDesktop], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&data, 1);
}

void
ewmh_update_net_desktop_geometry(Client *c) {
	Monitor *m;
	long data[2] = { 0, 0 };

	DPRINTF("%s\n", "Updating _NET_DESKTOP_GEOMETRY");
	for (m = monitors; m; m = m->next) {
		data[0] = max(data[0], m->sx + m->sw);
		data[1] = max(data[1], m->sy + m->sh);
	}
	XChangeProperty(dpy, root, atom[DeskGeometry], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, 2);
}

void
ewmh_update_net_virtual_roots(Client *c) {
	XDeleteProperty(dpy, root, atom[VirtualRoots]);
}

void
ewmh_update_echinus_seltags(Client *c) {
	Monitor *m;
	long *seltags;
	unsigned int i;

	DPRINTF("%s\n", "Updating _ECHINUS_SELTAGS");
	seltags = ecalloc(ntags, sizeof(seltags[0]));
	for (m = monitors; m != NULL; m = m->next)
		for (i = 0; i < ntags; i++)
			if (m->seltags[i])
				seltags[i] = True;
	XChangeProperty(dpy, root, atom[ESelTags], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) seltags, ntags);
	free(seltags);
}

void
ewmh_update_net_current_desktop(Client * c) {
	long data = curmontag;

	DPRINTF("%s\n", "Updating _NET_CURRENT_DESKTOP");
	XChangeProperty(dpy, root, atom[CurDesk], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);
}

static Bool
isomni(Client *c) {
	unsigned int i;

	if (!c->issticky)
		for (i = 0; i < ntags; i++)
			if (!c->tags[i])
				return False;
	return True;
}

static Bool
islost(Client *c) {
	unsigned int i;
	for (i = 0; i < ntags; i++)
		if (c->tags[i])
			return False;
	return True;
}

Bool
ewmh_process_net_window_desktop(Client * c) {
	long desktop, *desktops = NULL;
	unsigned long n = 0;
	unsigned int i;

	desktops = getcard(c->win, atom[WindowDesk], &n);
	if (n > 0) {
		desktop = desktops[0];
		if ((desktop & 0xffffffff) == 0xffffffff) {
			for (i = 0; i < ntags; i++)
				c->tags[i] = True;
			return True;
		} else if (desktop > 0 && desktop <= ntags) {
			for (i = 0; i < ntags; i++)
				c->tags[i] = False;
			c->tags[desktop] = True;
			return True;
		}
	}
	return False;
}

void
ewmh_update_net_window_desktop(Client *c) {
	long i = -1;

	if (!c->ismanaged)
		if (ewmh_process_net_window_desktop(c))
			return;
	DPRINTF("Updating _NET_WM_DESKTOP for 0x%lx\n", c->win);
	if (!isomni(c))
		for (i = 0; i < ntags && !c->tags[i]; i++);
	XChangeProperty(dpy, c->win, atom[WindowDesk], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) &i, 1);
}

Bool
ewmh_process_net_window_desktop_mask(Client *c) {
	unsigned int i, j, k, l;
	long *desktops = NULL;
	unsigned long n = 0;
	Bool *prev;

	desktops = getcard(c->win, atom[WindowDeskMask], &n);
	if (n > 0) {
		prev = ecalloc(ntags, sizeof(*prev));
		memcpy(prev, c->tags, ntags * sizeof(*prev));
		for (j = 0, k = 0; j < n; j++, k += 32)
			for (i = 0, l = k; i < 32 && l < ntags; i++, l++)
				if (desktops[j] & (1<<i))
					c->tags[l] = True;
		for (i = 0; i < ntags && !c->tags[i]; i++) ;
		if (i < ntags) {
			free(prev);
			return True;
		}
		memcpy(c->tags, prev, ntags * sizeof(*prev));
		free(prev);
	}
	return False;
}

void
ewmh_update_net_window_desktop_mask(Client *c) {
	unsigned int longs = (ntags+31)>>5;
	unsigned long data[longs];
	unsigned int i, j, k, l;

	if (!c->ismanaged)
		if (ewmh_process_net_window_desktop_mask(c))
			return;
	DPRINTF("Updating _NET_WM_DESKTOP_MASK for 0x%lx\n", c->win);
	for (j = 0, k = 0; j < longs; j++, k += 32)
		for (i = 0, l = k, data[j] = 0; i < 32 && l < ntags; i++, l++)
			if (c->tags[l])
				data[j] |= (1<<i);
	XChangeProperty(dpy, c->win, atom[WindowDeskMask], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *)data, longs);
}

void
ewmh_update_net_work_area(Client * c) {
	long *geoms, longs = ntags * 4;
	int i, j, x, y, w, h, l = 0, r = 0, t = 0, b = 0;
	Monitor *m;

	DPRINTF("%s\n", "Updating _NET_WORKAREA");
	geoms = ecalloc(longs, sizeof(geoms[0]));

	for (m = monitors; m; m = m->next) {
		l = max(l, m->struts[LeftStrut]);
		r = max(r, m->struts[RightStrut]);
		t = max(t, m->struts[TopStrut]);
		b = max(b, m->struts[BotStrut]);
	}
	x = l;
	y = t;
	w = DisplayWidth(dpy, screen) - (l + r);
	h = DisplayHeight(dpy, screen) - (t + b);
	for (i = 0, j = 0; i < ntags; i++) {
		geoms[j++] = x;
		geoms[j++] = y;
		geoms[j++] = w;
		geoms[j++] = h;
	}
	XChangeProperty(dpy, root, atom[WorkArea], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) geoms, longs);
	free(geoms);
}

Bool names_synced = False;

void
ewmh_update_net_desktop_names(Client * c) {
	char *buf, *pos;
	unsigned int i, len, slen;

	if (names_synced) {
		DPRINTF("%s\n", "Updating _NET_DESKTOP_NAMES: NOT!");
		return;
	}
	DPRINTF("%s\n", "Updating _NET_DESKTOP_NAMES");
	for (len = 0, i = 0; i < ntags; i++)
		if (tags[i])
			len += strlen(tags[i]);
	len += ntags;
	pos = buf = ecalloc(len, 1);
	for (i = 0; i < ntags; i++) {
		if (tags[i]) {
			slen = strlen(tags[i]);
			memcpy(pos, tags[i], slen);
			pos += slen;
		}
		*(pos++) = '\0';
	}
	XChangeProperty(dpy, root, atom[DeskNames], atom[Utf8String], 8,
			PropModeReplace, (unsigned char *) buf, len);
	free(buf);
	names_synced = True;
}

void
ewmh_process_net_desktop_names() {
	int format, status;
	Atom real;
	unsigned long nitems = 0, extra = 0, bytes = 1;
	char *ret = NULL, *pos, *str;
	unsigned int i, len;

	names_synced = True;
	do {
		if (ret)
			XFree((unsigned char *) ret);
		bytes += 4 * extra;
		status = XGetWindowProperty(dpy, root, atom[DeskNames], 0L, bytes, False,
				       atom[Utf8String], &real, &format, &nitems, &extra,
				       (unsigned char **) &ret);
		if (status != Success) {
			names_synced = False;
			break;
		}
	} while (extra && bytes == 1);

	for (pos = ret, i = 0; nitems && i < ntags; i++) {
		if ((len = strnlen(pos, nitems))) {
			if ((str = strndup(pos, nitems))) {
				DPRINTF("Assigning name '%s' to tag %u\n", str, i);
				free(tags[i]);
				tags[i] = str;
			}
		} else {
			free(tags[i]);
			inittag(i);
		}
		nitems -= len;
		pos += len;
		if (nitems) {
			--nitems;
			++pos;
		}
	}
	for (; i < ntags; i++) {
		free(tags[i]);
		inittag(i);
		names_synced = False;
	}
	if (ret)
		XFree(ret);
}

void
ewmh_update_net_active_window(Client * c) {
	Window win;

	DPRINTF("%s\n", "Updating _NET_ACTIVE_WINDOW");
	win = sel ? sel->win : None;
	XChangeProperty(dpy, root, atom[ActiveWindow], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &win, 1);
}

void
ewmh_update_net_desktop_modes(Client * c) {
	long *data;
	unsigned int i;

	DPRINTF("%s\n", "Updating _NET_DESKTOP_MODES");
	data = ecalloc(ntags, sizeof(data[0]));
	for (i = 0; i < ntags; i++) {
		switch (views[i].layout->symbol) {
		case 'i':
		case 'f':
			data[i] = atom[DeskModeFloating];
			break;
		case 't':
			data[i] = atom[DeskModeTiled];
			break;
		case 'b':
			data[i] = atom[DeskModeBottomTiled];
			break;
		case 'm':
			data[i] = atom[DeskModeMonocle];
			break;
		case 'u':
			data[i] = atom[DeskModeTopTiled];
			break;
		case 'l':
			data[i] = atom[DeskModeLeftTiled];
			break;
		default:
			data[i] = None;
			break;
		}
	}
	XChangeProperty(dpy, root, atom[DeskModes], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) data, ntags);
	free(data);
}

void
getmwmhints(Window win, Window *title, int *border) {
	Atom real;
	int format;
	long *hint = NULL;
	unsigned long n, extra;

#define MWM_HINTS_ELEMENTS 5
#define MWM_DECOR_ALL(x) ((x) & (1L << 0))
#define MWM_DECOR_TITLE(x) ((x) & (1L << 3))
#define MWM_DECOR_BORDER(x) ((x) & (1L << 1))
#define MWM_HINTS_DECOR(x) ((x) & (1L << 1))
	if (XGetWindowProperty(dpy, win, atom[MWMHints], 0L, 20L, False,
			       atom[MWMHints], &real, &format, &n, &extra,
			       (unsigned char **) &hint) == Success && n >= MWM_HINTS_ELEMENTS) {
		if (MWM_HINTS_DECOR(hint[0]) && !(MWM_DECOR_ALL(hint[2]))) {
			*title = MWM_DECOR_TITLE(hint[2]) ? 1 : None;
			*border = MWM_DECOR_BORDER(hint[2]) ? style.border : 0;
		}
	}
	if (hint)
		XFree(hint);
}


void
mwm_process_atom(Client * c) {
	getmwmhints(c->win, &c->title, &c->border);
}

void
ewmh_update_net_window_state(Client *c) {
	long winstate[16];
	int states = 0;

	/* do not update until we have finished reading it */
	if (!c->ismanaged)
		return;

	DPRINTF("Updating _NET_WM_STATE for 0x%lx\n", c->win);
	if (c->ismodal)
		winstate[states++] = atom[WindowStateModal];
	if (c->issticky)
		winstate[states++] = atom[WindowStateSticky];
	if (c->notaskbar)
		winstate[states++] = atom[WindowStateNoTaskbar];
	if (c->nopager)
		winstate[states++] = atom[WindowStateNoPager];
	if (c->isicon || c->ishidden)
		winstate[states++] = atom[WindowStateHidden];
	if (c->ismax)
		winstate[states++] = atom[WindowStateFs];
	if (c->isabove)
		winstate[states++] = atom[WindowStateAbove];
	if (c->isbelow)
		winstate[states++] = atom[WindowStateBelow];
	if (c->isattn)
		winstate[states++] = atom[WindowStateAttn];
	if (c == sel)
		winstate[states++] = atom[WindowStateFocused];
	if (c->isfixed)
		winstate[states++] = atom[WindowStateFixed];
	if (c->isfloating)
		winstate[states++] = atom[WindowStateFloating];
	if (c->isfill)
		winstate[states++] = atom[WindowStateFilled];
	if (c->ismaxv)
		winstate[states++] = atom[WindowStateMaxV];
	if (c->ismaxh)
		winstate[states++] = atom[WindowStateMaxH];

	XChangeProperty(dpy, c->win, atom[WindowState], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) winstate, states);
}

void
ewmh_update_net_window_actions(Client *c) {
	long action[32];
	int actions = 0;

	DPRINTF("Updating _NET_WM_ALLOWED_ACTIONS for 0x%lx\n", c->win);
	action[actions++] = atom[WindowActionAbove];
	action[actions++] = atom[WindowActionBelow];
	if (!c->isbastard) {
		action[actions++] = atom[WindowActionChangeDesk];
	}
	action[actions++] = atom[WindowActionClose];
	if (!c->isbastard) {
		if (c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			action[actions++] = atom[WindowActionFs];
			action[actions++] = atom[WindowActionMaxH];
			action[actions++] = atom[WindowActionMaxV];
		}
		action[actions++] = atom[WindowActionMin];
		if (c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			action[actions++] = atom[WindowActionMove];
			if (!c->isfixed)
				action[actions++] = atom[WindowActionResize];
		} else {
			if (!c->isfixed) {
				DPRINTF("Cannot moveresize 0x%lx: %s\n", c->win, "not fixed");
			}
			if (!c->isfloating) {
				DPRINTF("Cannot moveresize 0x%lx: %s\n", c->win, "not floating");
			}
			if (!MFEATURES(clientmonitor(c), OVERLAP)) {
				DPRINTF("Cannot moveresize 0x%lx: %s\n", c->win, "not in overlap mode");
			}
		}
		action[actions++] = atom[WindowActionStick];
		action[actions++] = atom[WindowActionFloat];
		if (c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			if (c->title)
				action[actions++] = atom[WindowActionShade];
			action[actions++] = atom[WindowActionFill];
		}
	}

	XChangeProperty(dpy, c->win, atom[WindowActions], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) action, actions);
}

void
ewmh_process_state_atom(Client *c, Atom state, int set) {
	if (0) {
	} else if (state == atom[WindowStateModal]) {
		if ((set == _NET_WM_STATE_ADD && !c->ismodal) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismodal) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->ismodal = !c->ismodal;
			if (c->ismodal)
				focus(c);
		}
	} else if (state == atom[WindowStateSticky]) {
		if ((set == _NET_WM_STATE_ADD && !c->issticky) ||
		    (set == _NET_WM_STATE_REMOVE && c->issticky) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->issticky = !c->issticky;
			if (c->issticky)
				tag(c, -1);
			else
				tag(c, curmontag);
		}
	} else if (state == atom[WindowStateMaxV]) {
		if ((set == _NET_WM_STATE_ADD && !c->ismaxv) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismaxv) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglemaxv(c);
	} else if (state == atom[WindowStateMaxH]) {
		if ((set == _NET_WM_STATE_ADD && !c->ismaxh) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismaxh) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglemaxh(c);
	} else if (state == atom[WindowStateShaded]) {
		if ((set == _NET_WM_STATE_ADD && !c->isshade) ||
		    (set == _NET_WM_STATE_REMOVE && c->isshade) ||
		    (set == _NET_WM_STATE_TOGGLE))
			toggleshade(c);
	} else if (state == atom[WindowStateNoTaskbar]) {
		if ((set == _NET_WM_STATE_ADD && !c->notaskbar) ||
		    (set == _NET_WM_STATE_REMOVE && c->notaskbar) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->notaskbar = !c->notaskbar;
		}
	} else if (state == atom[WindowStateNoPager]) {
		if ((set == _NET_WM_STATE_ADD && !c->nopager) ||
		    (set == _NET_WM_STATE_REMOVE && c->nopager) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->nopager = !c->nopager;
		}
	} else if (state == atom[WindowStateHidden]) {
		/* Implementation note: if an Application asks to toggle
		   _NET_WM_STATE_HIDDEN the Window Manager should probably just ignore
		   the request, since _NET_WM_STATE_HIDDEN is a function of some other
		   aspect of the window such as minimization, rather than an independent
		   state. */
		if ((set == _NET_WM_STATE_ADD && !c->ishidden) ||
		    (set == _NET_WM_STATE_REMOVE && c->ishidden) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglehidden(c);
	} else if (state == atom[WindowStateFs]) {
		if ((set == _NET_WM_STATE_ADD || set == _NET_WM_STATE_TOGGLE)
				&& !c->ismax) {
			c->wasfloating = c->isfloating;
			if (!c->isfloating)
				togglefloating(c);
			togglemax(c);
		} else if ((set == _NET_WM_STATE_REMOVE ||
				set == _NET_WM_STATE_TOGGLE) && c->ismax) {
			togglemax(c);
			if (!c->wasfloating)
				togglefloating(c);
		}
		DPRINT;
		arrange(curmonitor());
		DPRINTF("%s: x%d y%d w%d h%d\n", c->name, c->x, c->y, c->w, c->h);
	} else if (state == atom[WindowStateAbove]) {
		if ((set == _NET_WM_STATE_ADD && !c->isabove) ||
		    (set == _NET_WM_STATE_REMOVE && c->isabove) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isabove = !c->isabove;
			restack();
		}
	} else if (state == atom[WindowStateBelow]) {
		if ((set == _NET_WM_STATE_ADD && !c->isbelow) ||
		    (set == _NET_WM_STATE_REMOVE && c->isbelow) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isbelow = !c->isbelow;
			restack();
		}
	} else if (state == atom[WindowStateAttn]) {
		if ((set == _NET_WM_STATE_ADD && !c->isattn) ||
		    (set == _NET_WM_STATE_REMOVE && c->isattn) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isattn = !c->isattn;
		}
	} else if (state == atom[WindowStateFocused]) {
		/* _NET_WM_STATE_FOCUSED indicates whether the window's decorations are
		   drawn in an active state.  Clients MUST regard it as a read-only hint. 
		   It cannot be set at map time or changed via a _NET_WM_STATE client
		   message. */
	} else if (state == atom[WindowStateFixed]) {
		/* _NET_WM_STATE_FIXED is a read-only state. */
	} else if (state == atom[WindowStateFloating]) {
		if ((set == _NET_WM_STATE_ADD && !c->isfloating) ||
		    (set == _NET_WM_STATE_REMOVE && c->isfloating) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			togglefloating(c);
		}
	} else if (state == atom[WindowStateFilled]) {
		if ((set == _NET_WM_STATE_ADD && !c->isfill) ||
		    (set == _NET_WM_STATE_REMOVE && c->isfill) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			togglefill(c);
		}
	}
}

void
ewmh_update_net_window_extents(Client *c) {
	long data[4] = {
		c->border, /* left */
		c->border, /* right */
		c->border + c->th, /* top */
		c->border, /* bottom */
	};

	DPRINTF("Updating _NET_WM_FRAME_EXTENTS for 0x%lx\n", c->win);
	XChangeProperty(dpy, c->win, atom[WindowExtents], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) &data, 4L);
	DPRINTF("Updating _KDE_NET_WM_FRAME_STRUT for 0x%lx\n", c->win);
	XChangeProperty(dpy, c->win, atom[WindowFrameStrut], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) &data, 4L);
}

void
ewmh_update_net_window_visible_name(Client *c) {
	XTextProperty prop;

	if (c->name) {
		if (XmbTextListToTextProperty(dpy, &c->name, 1, XUTF8StringStyle, &prop) == Success)
			XSetTextProperty(dpy, c->win, &prop, atom[WindowNameVisible]);
	} else {
		XDeleteProperty(dpy, c->win, atom[WindowNameVisible]);
	}
}

void
ewmh_update_net_window_visible_icon_name(Client *c) {
	XTextProperty prop;

	if (c->icon_name) {
		if (XmbTextListToTextProperty(dpy, &c->icon_name, 1, XUTF8StringStyle, &prop) == Success)
			XSetTextProperty(dpy, c->win, &prop, atom[WindowIconNameVisible]);
	} else {
		XDeleteProperty(dpy, c->win, atom[WindowIconNameVisible]);
	}
}

#ifdef STARTUP_NOTIFICATION
SnStartupSequence *
find_startup_seq(Client *c) {
	Notify *n = NULL;
	SnStartupSequence *seq = NULL;
	XClassHint ch;
	char **argv;
	int argc;
	const char *binary, *wmclass;


	if ((seq = c->seq))
		return seq;
	if (c->startup_id) {
		for (n = notifies; n; n = n->next)
			if (!strcmp(c->startup_id, sn_startup_sequence_get_id(n->seq)))
				break;
		if (!n) {
			DPRINTF("Cannot find startup id '%s'!\n", c->startup_id);
			return NULL;
		} 
		goto found_it;
	}
	if (XGetClassHint(dpy, c->win, &ch)) {
		for (n = notifies; n; n = n->next) {
			if (n->assigned)
				continue;
			if (sn_startup_sequence_get_completed(n->seq))
				continue;
			if ((wmclass = sn_startup_sequence_get_wmclass(n->seq))) {
				if (ch.res_name && !strcmp(wmclass, ch.res_name))
					break;
				if (ch.res_class && !strcmp(wmclass, ch.res_class))
					break;
			}
		}
		if (ch.res_class)
			XFree(ch.res_class);
		if (ch.res_name)
			XFree(ch.res_name);
		if (n)
			goto found_it;
	}
	if (XGetCommand(dpy, c->win, &argv, &argc)) {
		for (n = notifies; n; n = n->next) {
			if (n->assigned)
				continue;
			if (sn_startup_sequence_get_completed(n->seq))
				continue;
			if ((binary = sn_startup_sequence_get_binary_name(n->seq))) {
				if (argv[0] && !strcmp(binary, argv[0]))
					break;
			}
		}
		if (argv)
			XFreeStringList(argv);
		if (n)
			goto found_it;
	}
	if (XGetClassHint(dpy, c->win, &ch)) {
		/* try again, case insensitive */
		for (n = notifies; n; n = n->next) {
			if (n->assigned)
				continue;
			if (sn_startup_sequence_get_completed(n->seq))
				continue;
			if ((wmclass = sn_startup_sequence_get_wmclass(n->seq))) {
				if (ch.res_name && !strcasecmp(wmclass, ch.res_name))
					break;
				if (ch.res_class && !strcasecmp(wmclass, ch.res_class))
					break;
			}
		}
		if (ch.res_class)
			XFree(ch.res_class);
		if (ch.res_name)
			XFree(ch.res_name);
		if (n)
			goto found_it;
	}
	return NULL;
found_it:
	n->assigned = True;
	seq = n->seq;
	sn_startup_sequence_ref(seq);
	sn_startup_sequence_complete(seq);
	c->seq = seq;
	return seq;
}
#endif

void
push_client_time(Client *c, Time time)
{
	if (!time)
		return;
	if (c->hastime) {
		if ((c->user_time != CurrentTime) &&
		    (int) ((int) time - (int) c->user_time) > 0)
			c->user_time = time;
	} else {
		c->user_time = time;
		c->hastime = True;
	}
	if ((user_time == CurrentTime) ||
	    ((int) ((int) time - (int) user_time) > 0))
		user_time = time;
}

void
ewmh_update_net_startup_id(Client * c) {
#ifdef STARTUP_NOTIFICATION
	SnStartupSequence *seq;
#endif
	if (c->startup_id)
		return;
	if (!gettextprop(c->win, atom[NetStartupId], &c->startup_id))
		if (c->leader != None && c->leader != c->win)
			gettextprop(c->leader, atom[NetStartupId], &c->startup_id);
	if (c->startup_id) {
		char *pos;

		if ((pos = strstr(c->startup_id, "_TIME")))
			push_client_time(c, atol(pos + 5));
	}
#ifdef STARTUP_NOTIFICATION
	if (c->ismanaged)
		return;
	if ((seq = find_startup_seq(c))) {
		int i, workspace;

		if (!c->startup_id) {
			c->startup_id = strdup(sn_startup_sequence_get_id(seq));
			XChangeProperty(dpy, c->win, atom[NetStartupId],
					atom[Utf8String], 8, PropModeReplace,
					(unsigned char *) c->startup_id,
					strlen(c->startup_id)+1);
		}
		push_client_time(c, sn_startup_sequence_get_timestamp(seq));
		/* NOTE: should not override _NET_WM_DESKTOP */
		workspace = sn_startup_sequence_get_workspace(seq);
		if (0 <= workspace && workspace < ntags) {
			for (i = 0; i < ntags; i++)
				c->tags[i] = False;
			c->tags[workspace] = True;
		}
		/* TODO: use screen number to select monitor */
	}
#endif
}

void
ewmh_update_net_window_user_time(Client *c) {
	long *time = NULL;
	unsigned long n = 0;
	Window win;

	if ((win = c->time_window) == None)
		win = c->win;

	time = getcard(win, atom[WindowUserTime], &n);
	if (n > 0) {
		c->hastime = True;
		c->user_time = time[0];
		if (user_time == CurrentTime) {
			user_time = c->user_time;
		} else if (c->user_time != CurrentTime) {
			if ((int)((int)c->user_time - (int)user_time) > 0)
				user_time = c->user_time;
		}
		XFree(time);
	} else {
		c->hastime = False;
		c->user_time = CurrentTime;
	}
}

void
ewmh_release_user_time_window(Client *c) {
	Window win;
	
	if ((win = c->time_window) == None)
		return;

	c->time_window = None;
	XSelectInput(dpy, win, NoEventMask);
	XDeleteContext(dpy, win, context[ClientTimeWindow]);
	XDeleteContext(dpy, win, context[ClientAny]);
}

Window *getwind(Window win, Atom atom, unsigned long *nitems);

void
ewmh_update_net_window_user_time_window(Client * c) {
	Window *wins = NULL, win = None;
	unsigned long n = 0;

	wins = getwind(c->win, atom[UserTimeWindow], &n);
	if (n > 0) {
		Window other = wins[0];

		XFree(wins);
		/* check recursive */
		wins = getwind(other, atom[UserTimeWindow], &n);
		if (n > 0) {
			Window again = wins[0];

			XFree(wins);
			win = (other == again) ? again : None;
		}
	}
	if (win == c->time_window)
		return;
	ewmh_release_user_time_window(c);
	if (win == None)
		return;

	c->time_window = win;
	XSelectInput(dpy, win, PropertyChangeMask);
	XSaveContext(dpy, win, context[ClientTimeWindow], (XPointer)c);
	XSaveContext(dpy, win, context[ClientAny], (XPointer)c);
}

Atom *getatom(Window win, Atom atom, unsigned long *nitems);

void
ewmh_process_net_window_state(Client *c) {
	Atom *state;
	unsigned long i, n = 0;

	state = getatom(c->win, atom[WindowState], &n);
	for (i = 0; i < n; i++)
		ewmh_process_state_atom(c, state[i], _NET_WM_STATE_ADD);
	if (state)
		XFree(state);
}

void
ewmh_update_net_window_sync_request_counter(Client *c) {
	int format, status;
	long *data = NULL;
	unsigned long extra, nitems = 0;
	Atom real;
	

	if (!checkatom(c->win, atom[WMProto], atom[WindowSync]))
		return;
	status = XGetWindowProperty(dpy, c->win, atom[WindowTypeOverride], 0L, 1L,
			False, AnyPropertyType, &real, &format, &nitems, &extra,
			(unsigned char **)&data);
	if (status == Success && nitems > 0) {
#ifdef SYNC
		XSyncValue val;

		XSyncIntToValue(&val, 0);
		XSyncSetCounter(dpy, c->sync, val);
#endif
		c->sync = data[0];
	}
	if (data)
		XFree(data);
}

void
ewmh_update_kde_net_window_type_override(Client *c) {
	int format, status;
	long *data = NULL;
	unsigned long extra, nitems = 0;
	Atom real;

	status = XGetWindowProperty(dpy, c->win, atom[WindowTypeOverride], 0L, 1L,
			False, AnyPropertyType, &real, &format, &nitems, &extra,
			(unsigned char **)&data);
	if (status == Success && real != None) {
		/* no decorations or functionality */
		c->isbastard = True;
		c->isfloating = True;
		c->isfixed = True;
	}
	if (data)
		XFree(data);
}

void
ewmh_update_kde_splash_progress(Client *c) {
	XEvent ev;

	ev.xclient.display = dpy;
	ev.xclient.type = ClientMessage;
	ev.xclient.window = root;
	ev.xclient.message_type = atom[KdeSplashProgress];
	ev.xclient.format = 8;
	strcpy(ev.xclient.data.b, "wm started");
	XSendEvent(dpy, root, False, SubstructureNotifyMask, &ev);
}

void
clientmessage(XEvent *e) {
	XClientMessageEvent *ev = &e->xclient;
	Client *c;
	Atom message_type = ev->message_type;

	if ((c = getclient(ev->window, ClientWindow))) {
		if (0) {
		} else if (message_type == atom[CloseWindow]) {
			killclient(c);
		} else if (message_type == atom[ActiveWindow]) {
			if (c->isicon || c->ishidden) {
				c->isicon = False;
				c->ishidden = False;
				ewmh_update_net_window_state(c);
			}
			focus(c);
			arrange(clientmonitor(c));
		} else if (message_type == atom[WindowState]) {
			ewmh_process_state_atom(c, (Atom) ev->data.l[1], ev->data.l[0]);
			if (ev->data.l[2])
				ewmh_process_state_atom(c,
				    (Atom) ev->data.l[2], ev->data.l[0]);
			ewmh_update_net_window_state(c);
		} else if (message_type == atom[WMChangeState]) {
			if (ev->data.l[0] == IconicState)
				iconify(c);
		} else if (message_type == atom[WindowDesk]) {
			int index = ev->data.l[0];

			if (-1 <= index && index < ntags)
				tag(c, index);
		} else if (message_type == atom[WindowDeskMask]) {
			unsigned index = ev->data.l[0];
			unsigned mask = ev->data.l[1], oldmask = 0;
			unsigned num = (ntags+31)>>5;
			unsigned i, j;

			if (0 > index || index >= num)
				return;
			for (i = 0, j = index<<5; j<ntags; i++, j++) {
				if (c->tags[j])
					oldmask |= (1<<i);
				c->tags[j] = (mask & (1<<i)) ? True : False;
			}
			if (islost(c))
				for (i = 0, j = index<<5; j<ntags; i++, j++)
					c->tags[j] = (oldmask & (1<<i)) ? True : False;
			else {
				/* what toggletag does */
				updateatom[WindowDesk] (c);
				updateatom[WindowDeskMask] (c);
				updateatom[WindowState] (c);
				drawclient(c);
				arrange(NULL);
			}
			/* TODO */
		} else if (message_type == atom[WMProto]) {
			/* TODO */
		} else if (message_type == atom[WindowFsMonitors]) {
			/* TODO */
		} else if (message_type == atom[MoveResizeWindow]) {
			unsigned flags = (unsigned) ev->data.l[0];
			unsigned source = ((flags >> 12) & 0x0f);
			unsigned gravity = flags & 0xff;
			int x, y, w, h;

			if (!(c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)))
				return;
			if (source != 0 && source != 2)
				return;

			x = (flags & (1 << 8)) ? ev->data.l[1] : c->x;
			y = (flags & (1 << 9)) ? ev->data.l[2] : c->y;
			w = (flags & (1 << 10)) ? ev->data.l[3] : c->w;
			h = (flags & (1 << 11)) ? ev->data.l[4] : c->h;
			if (gravity == 0)
				gravity = c->gravity;
			applygravity(c, &x, &y, &w, &h, c->border, gravity);
			/* FIXME: this just resizes and moves the window, it does
			 * handle changing monitors */
			resize(c, x, y, w, h, c->border);
		} else if (message_type == atom[WindowMoveResize]) {
			int x_root = (int) ev->data.l[0];
			int y_root = (int) ev->data.l[1];
			int direct = (int) ev->data.l[2];
			int button = (int) ev->data.l[3];
			int source = (int) ev->data.l[4];

			if (source != 0 && source != 1 && source != 2)
				return;
			if (direct < 0 || direct > 11)
				return;
			switch (direct) {
			case 0: /* _NET_WM_MOVERESIZE_SIZE_TOPLEFT */
			case 1: /* _NET_WM_MOVERESIZE_SIZE_TOP */
			case 2: /* _NET_WM_MOVERESIZE_SIZE_TOPRIGHT */
			case 3: /* _NET_WM_MOVERESIZE_SIZE_RIGHT */
			case 4: /* _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT */
			case 5: /* _NET_WM_MOVERESIZE_SIZE_BOTTOM */
			case 6: /* _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT */
			case 7: /* _NET_WM_MOVERESIZE_SIZE_LEFT */
				mouseresize_from(c, direct, button, x_root, y_root);
				break;
			case 8: /* _NET_WM_MOVERESIZE_MOVE */
				mousemove(c, button, x_root, y_root);
				break;
			case 9: /* _NET_WM_MOVERESIZE_SIZE_KEYBOARD */
				/* TODO */
				break;
			case 10: /* _NET_WM_MOVERESIZE_MOVE_KEYBOARD */
				/* TODO */
				break;
			case 11: /* _NET_WM_MOVERESIZE_CANCEL */
				/* intercepted while moving or resizing */
				break;
			}
		} else if (message_type == atom[RestackWindow]) {
			unsigned source = ev->data.l[0];
			Window sibling = ev->data.l[1];
			unsigned detail = ev->data.l[2];
			Client *o = NULL;
			if (sibling)
				if (!(o = getclient(sibling, ClientAny)))
					return;
			if (source == 1)
				return;
			restack_client(c, detail, o);
		} else if (message_type == atom[RequestFrameExt]) {
			ewmh_update_net_window_extents(c);
		}
	} else if (ev->window == root) {
		if (0) {
		} else if (message_type == atom[NumberOfDesk]) {
			settags(ev->data.l[0]);
		} else if (message_type == atom[DeskGeometry]) {
			/* Ignore desktop geometry changes but change the property in response */
			updateatom[DeskGeometry] (NULL);
		} else if (message_type == atom[DeskViewport]) {
			/* Ignore viewport changes but change the property in response. */
			updateatom[DeskViewport] (NULL);
		} else if (message_type == atom[CurDesk]) {
			int tag = ev->data.l[0];
			if (0 <= tag && tag < ntags)
				view(tag);
		} else if (message_type == atom[ShowingDesktop]) {
			if (!ev->data.l[0] != !showing_desktop)
				toggleshowing();
		} else if (message_type == atom[WMRestart]) {
			/* TODO */
		} else if (message_type == atom[WMShutdown]) {
			/* TODO */
		} else if (message_type == atom[Manager]) {
			/* TODO */
		} else if (message_type == atom[StartupInfoBegin]) {
#ifdef STARTUP_NOTIFICATION
			sn_display_process_event(sn_dpy, e);
#endif
		} else if (message_type == atom[StartupInfo]) {
#ifdef STARTUP_NOTIFICATION
			sn_display_process_event(sn_dpy, e);
#endif
		}
	} else {
		if (0) {
		} else if (message_type == atom[RequestFrameExt]) {
			Window win = ev->window;
			unsigned int wintype;
			Window title = 1;
			int th, border = style.border;
			long data[4];

			if (win == None)
				return;
			wintype = getwintype(win);
			if (!(WTTEST(wintype, WindowTypeNormal)))
				if (WTTEST(wintype, WindowTypeDesk) ||
				    WTTEST(wintype, WindowTypeDock) ||
				    WTTEST(wintype, WindowTypeDrop) ||
				    WTTEST(wintype, WindowTypePopup) ||
				    WTTEST(wintype, WindowTypeTooltip) ||
				    WTTEST(wintype, WindowTypeNotify) ||
				    WTTEST(wintype, WindowTypeCombo) ||
				    WTTEST(wintype, WindowTypeDnd)) {
					title = None;
					border = 0;
				}
			getmwmhints(win, &title, &border);
			th = title ? style.titleheight : 0;
			data[0] = border;
			data[1] = border;
			data[2] = border + th;
			data[3] = border;
			XChangeProperty(dpy, win, atom[WindowExtents], XA_CARDINAL, 32,
					PropModeReplace, (unsigned char *) &data, 4L);
		}
	}
}

void
setopacity(Client * c, unsigned int opacity) {
	/* TODO: This is not quite right: the client is responsible for setting 
	   opacity on the client window, the WM should only propagate that
	   opacity to the frame. */
	if (opacity == OPAQUE) {
		XDeleteProperty(dpy, c->win, atom[WindowOpacity]);
		XDeleteProperty(dpy, c->frame, atom[WindowOpacity]);
	} else {
		long data = opacity;

		XChangeProperty(dpy, c->win, atom[WindowOpacity], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *) &data, 1L);
		XChangeProperty(dpy, c->frame, atom[WindowOpacity], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *) &data, 1L);
	}
}

Atom *
getatom(Window win, Atom atom, unsigned long *nitems) {
	int format, status;
	Atom *ret = NULL;
	unsigned long extra;
	Atom real;

	status = XGetWindowProperty(dpy, win, atom, 0L, 64L, False, XA_ATOM,
			&real, &format, nitems, &extra, (unsigned char **)&ret);
	if (status != Success) {
		*nitems = 0;
		return NULL;
	}
	return ret;
}

long *
getcard(Window win, Atom atom, unsigned long *nitems) {
	int format, status;
	long *ret = NULL;
	unsigned long extra;
	Atom real;

	status = XGetWindowProperty(dpy, win, atom, 0L, 64L, False, XA_CARDINAL,
			&real, &format, nitems, &extra, (unsigned char **)&ret);
	if (status != Success) {
		*nitems = 0;
		return NULL;
	}
	return ret;
}

Window *
getwind(Window win, Atom atom, unsigned long *nitems) {
	int format, status;
	Window *ret = NULL;
	unsigned long extra;
	Atom real;

	status = XGetWindowProperty(dpy, win, atom, 0L, 64L, False, XA_WINDOW,
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

	state = getatom(win, bigatom, &n);
	for (i = 0; i < n; i++) {
		if (state[i] == smallatom) {
			ret = True;
			break;
		}
	}
	if (state)
		XFree(state);
	return ret;
}

unsigned int
getwintype(Window win) {
	Atom *state;
	unsigned long i, j, n = 0;
	unsigned int ret = 0;

	state = getatom(win, atom[WindowType], &n);
	for (i = 0; i < n; i++)
		for (j = WindowTypeDesk; j <= WindowTypeNormal; j++)
			if (state[i] == atom[j])
				ret |= (1<<(j-WindowTypeDesk));
	if (ret == 0)
		ret = WTFLAG(WindowTypeNormal);
	if (state)
		XFree(state);
	return ret;
}

Bool
checkwintype(Window win, int wintype) {
	Atom *state;
	unsigned long i, n = 0;
	Bool ret = False;

	state = getatom(win, atom[WindowType], &n);
	if (n == 0) {
		if (wintype == WindowTypeNormal)
			ret = True;
	} else {
		for (i = 0; i < n; i++) {
			if (state[i] == atom[wintype]) {
				ret = True;
				break;
			}
		}
	}
	if (state)
		XFree(state);
	return ret;
}

static Bool
strut_overlap(long min1, long max1, long min2, long max2) {
	long tmp;

	if (min1 > max1) { tmp = min1; min1 = max1; max1 = tmp; }
	if (min2 > max2) { tmp = min2; min2 = max2; max2 = tmp; }
	if (min1 <= min2 && max1 >= min2)
		return True;
	if (min1 <= max2 && max1 >= max2)
		return True;
	if (min2 <= min1 && max2 >= min1)
		return True;
	if (min2 <= max1 && max2 >= max1)
		return True;
	return False;
}

int
getstrut(Client * c, Atom atom) {
	long *prop, dw, dh, strut;
	Monitor *m;
	unsigned long n = 0;
	struct {
		long l, r, t, b, ly1, ly2, ry1, ry2, tx1, tx2, bx1, bx2;
	} s;

	dw = DisplayWidth(dpy, screen);
	dh = DisplayHeight(dpy, screen);

	s.l = s.r = s.t = s.b = 0;
	s.ly1 = s.ry1 = 0;
	s.ly2 = s.ry2 = dh;
	s.tx1 = s.bx1 = 0;
	s.tx2 = s.bx2 = dw;

	prop = getcard(c->win, atom, &n);
	if (n == 0)
		return n;

	memcpy(&s, prop, n * sizeof(long));
	XFree(prop);

	for (m = monitors; m; m = m->next) {
		if ((strut = s.l - m->sx) > 0)
			if (strut_overlap(m->sy, m->sy + m->sh, s.ly1, s.ly2))
				m->struts[LeftStrut] = max(strut, m->struts[LeftStrut]);
		if ((strut = m->sx + m->sw - dw + s.r) > 0)
			if (strut_overlap(m->sy, m->sy + m->sh, s.ry1, s.ry2))
				m->struts[RightStrut] = max(strut, m->struts[RightStrut]);
		if ((strut = s.t - m->sy) > 0)
			if (strut_overlap(m->sx, m->sx + m->sw, s.tx1, s.tx2))
				m->struts[TopStrut] = max(strut, m->struts[TopStrut]);
		if ((strut = m->sy + m->sh - dh + s.b) > 0)
			if (strut_overlap(m->sx, m->sx + m->sw, s.bx1, s.bx2))
				m->struts[BotStrut] = max(strut, m->struts[BotStrut]);
	}
	return n;
}

int getstruts(Client *c) {
	return (getstrut(c, atom[StrutPartial]) || getstrut(c, atom[Strut]));
}

void (*updateatom[]) (Client *) = {
	[ClientList] = ewmh_update_net_client_list,
	[ClientListStacking] = ewmh_update_net_client_list_stacking,
	[ActiveWindow] = ewmh_update_net_active_window,
	[WindowDesk] = ewmh_update_net_window_desktop,
	[WindowDeskMask] = ewmh_update_net_window_desktop_mask,
	[NumberOfDesk] = ewmh_update_net_number_of_desktops,
	[DeskViewport] = ewmh_update_net_desktop_viewport,
	[ShowingDesktop] = ewmh_update_net_showing_desktop,
	[DeskGeometry] = ewmh_update_net_desktop_geometry,
	[VirtualRoots] = ewmh_update_net_virtual_roots,
	[DeskNames] = ewmh_update_net_desktop_names,
	[CurDesk] = ewmh_update_net_current_desktop,
	[ELayout] = update_echinus_layout_name,
	[ESelTags] = ewmh_update_echinus_seltags,
	[WorkArea] = ewmh_update_net_work_area,
	[DeskModes] = ewmh_update_net_desktop_modes,
	[WindowState] = ewmh_update_net_window_state,
	[WindowActions] = ewmh_update_net_window_actions,
	[WindowExtents] = ewmh_update_net_window_extents,
	[WindowNameVisible] = ewmh_update_net_window_visible_name,
	[WindowIconNameVisible] = ewmh_update_net_window_visible_icon_name,
	[WindowUserTime] = ewmh_update_net_window_user_time,
	[UserTimeWindow] = ewmh_update_net_window_user_time_window,
	[NetStartupId] = ewmh_update_net_startup_id,
	[WindowTypeOverride] = ewmh_update_kde_net_window_type_override,
	[KdeSplashProgress] = ewmh_update_kde_splash_progress,
	[WindowCounter] = ewmh_update_net_window_sync_request_counter,
};
