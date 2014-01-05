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
#include "echinus.h"
#include "config.h"

Atom atom[NATOMS];

/* keep in sync with enum in echinus.h */
char *atomnames[NATOMS] = {
	"MANAGER",
	"UTF8_STRING",
	"WM_PROTOCOLS",
	"WM_DELETE_WINDOW",
	"WM_NAME",
	"WM_STATE",
	"WM_CHANGE_STATE",
	"WM_TAKE_FOCUS",
	"_MOTIF_WM_HINTS",
	"_ECHINUS_LAYOUT",
	"_ECHINUS_SELTAGS",
	"_NET_WM_FULLSCREEN_MONITORS",		/* TODO */
	"_NET_DESKTOP_GEOMETRY",		/* TODO */
	"_NET_DESKTOP_VIEWPORT",		/* TODO */
	"_NET_SHOWING_DESKTOP",			/* TODO */
	"_NET_RESTART",				/* TODO */
	"_NET_SHUTDOWN",			/* TODO */
	"_NET_REQUEST_FRAME_EXTENTS",		/* TODO */
	"_NET_RESTACK_WINDOW",			/* TODO */
	"_NET_STARTUP_INFO_BEGIN",
	"_NET_STARTUP_INFO",
	"_NET_DESKTOP_LAYOUT",			/* TODO */
	"_NET_WM_USER_TIME",			/* TODO */
	"_NET_WM_USER_TIME_WINDOW",		/* TODO */
	"_NET_WM_VISIBLE_NAME",			/* TODO */
	"_NET_WM_ICON_NAME",			/* TODO */
	"_NET_WM_VISIBLE_ICON_NAME",		/* TODO */
	"_NET_WM_SYNC_REQUEST_COUNTER",
	"_NET_WM_HANDLED_ICONS",
	"_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
	/* _NET_SUPPORTED following */
	"_NET_CLIENT_LIST",
	"_NET_ACTIVE_WINDOW",
	"_NET_WM_DESKTOP",
	"_NET_WM_DESKTOP_MASK",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_DESKTOP_NAMES",
	"_NET_CURRENT_DESKTOP",
	"_NET_WORKAREA",

	"_NET_DESKTOP_MODES",
	"_NET_DESKTOP_MODE_FLOATING",
	"_NET_DESKTOP_MODE_TILED",
	"_NET_DESKTOP_MODE_BOTTOM_TILED",
	"_NET_DESKTOP_MODE_MONOCLE",

	"_NET_CLIENT_LIST_STACKING",
	"_NET_WM_WINDOW_OPACITY",
	"_NET_MOVERESIZE_WINDOW",
	"_NET_WM_MOVERESIZE",
	"_NET_FRAME_EXTENTS",

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

	"_NET_WM_STATE",
	"_NET_WM_STATE_MODAL",
	"_NET_WM_STATE_STICKY",
	"_NET_WM_STATE_MAXIMIZED_VERT",		/* TODO */
	"_NET_WM_STATE_MAXIMIZED_HORZ",		/* TODO */
	"_NET_WM_STATE_SHADED",			/* TODO */
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
	"_NET_WM_ACTION_MAXIMIZE_HORZ",		/* TODO */
	"_NET_WM_ACTION_MAXIMIZE_VERT",		/* TODO */
	"_NET_WM_ACTION_MINIMIZE",
	"_NET_WM_ACTION_MOVE",
	"_NET_WM_ACTION_RESIZE",
	"_NET_WM_ACTION_SHADE",			/* TODO */
	"_NET_WM_ACTION_STICK",
	"_NET_WM_ACTION_FLOAT",
	"_NET_WM_ACTION_FILL",

	"_NET_SUPPORTING_WM_CHECK",
	"_NET_CLOSE_WINDOW",
	"_NET_SUPPORTED"
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
	Client **cl = NULL;
	int i, j, n;

	DPRINTF("%s\n", "Updating _NET_CLIENT_LIST_STACKING");
	for (n = 0, c = stack; c; c = c->snext)
		n++;
	if (n) {
		wl = ecalloc(n, sizeof(Window));
		cl = ecalloc(n, sizeof(Client *));
	}
	for (i = 0, c = stack; c && i < n; c = c->snext)
		cl[i++] = c;

	i = 0;
	/* focused windows having state _NET_WM_STATE_FULLSCREEN */
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if ((c = cl[j]) && sel == c && c->ismax) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if ((WTCHECK(c, WindowTypeDock) && !c->isbelow) || c->isabove) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	/* windows of type _NET_WM_TYPE_DOCK (unless they have state _NET_WM_STATE_BELOW) 
	   and windows having state _NET_WM_STATE_ABOVE */
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if ((WTCHECK(c, WindowTypeDock) && !c->isbelow) || c->isabove) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	/* windows not belonging in any other layer (but we put floating above special
	   above tiled) */
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if (!c->isbastard && c->isfloating && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if (c->isbastard && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if (!c->isbastard && !c->isfloating && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	/* windows having state _NET_WM_STATE_BELOW */
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if (c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	/* windows of type _NET_WM_TYPE_DESKTOP */
	for (j = 0; j < n && i < n; j++) {
		if (!(c = cl[j]))
			continue;
		if (WTCHECK(c, WindowTypeDesk)) {
			wl[i++] = c->win;
			cl[j] = NULL;
		}
	}
	assert(i == n);
	XChangeProperty(dpy, root, atom[ClientListStacking], XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) wl, n);
	if (n) {
		free(wl);
		free(cl);
	}
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

void
ewmh_update_net_current_desktop(Client * c) {
	Monitor *m;
	long *seltags, data;
	unsigned int i;

	DPRINTF("%s\n", "Updating _ECHINUS_SELTAGS");
	seltags = ecalloc(ntags, sizeof(seltags[0]));
	for (m = monitors; m != NULL; m = m->next)
		for (i = 0; i < ntags; i++)
			if (m->seltags[i])
				seltags[i] = True;
	XChangeProperty(dpy, root, atom[ESelTags], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) seltags, ntags);

	DPRINTF("%s\n", "Updating _NET_CURRENT_DESKTOP");
	data = curmontag;
	XChangeProperty(dpy, root, atom[CurDesk], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);

	update_echinus_layout_name(NULL);
	free(seltags);
}

static Bool
issticky(Client *c) {
	unsigned int i;

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

void
ewmh_update_net_window_desktop(Client *c) {
	long i = -1;

	DPRINTF("Updating _NET_WM_DESKTOP for 0x%lx\n", c->win);
	if (!issticky(c))
		for (i = 0; i < ntags && !c->tags[i]; i++);
	XChangeProperty(dpy, c->win, atom[WindowDesk], XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) &i, 1);
}

void
ewmh_update_net_window_desktop_mask(Client *c) {
	unsigned int longs = (ntags+31)>>5;
	unsigned long data[longs];
	unsigned int i, j, k, l;

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
	Monitor *m = monitors;
	int i, j, x, y, w, h;

	DPRINTF("%s\n", "Updating _NET_WORKAREA");
	geoms = ecalloc(longs, sizeof(geoms[0]));
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

void
ewmh_update_net_desktop_names(Client * c) {
	char *buf, *pos;
	unsigned int i, len, slen;

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
mwm_process_atom(Client * c) {
	Atom real;
	int format;
	long *hint = NULL;
	unsigned long n, extra;

#define MWM_HINTS_ELEMENTS 5
#define MWM_DECOR_ALL(x) ((x) & (1L << 0))
#define MWM_DECOR_TITLE(x) ((x) & (1L << 3))
#define MWM_DECOR_BORDER(x) ((x) & (1L << 1))
#define MWM_HINTS_DECOR(x) ((x) & (1L << 1))
	if (XGetWindowProperty(dpy, c->win, atom[MWMHints], 0L, 20L, False,
			       atom[MWMHints], &real, &format, &n, &extra,
			       (unsigned char **) &hint) == Success && n >= MWM_HINTS_ELEMENTS) {
		if (MWM_HINTS_DECOR(hint[0]) && !(MWM_DECOR_ALL(hint[2]))) {
			c->title = MWM_DECOR_TITLE(hint[2]) ? root : (Window) NULL;
			c->border = MWM_DECOR_BORDER(hint[2]) ? style.border : 0;
		}
	}
	if (hint)
		XFree(hint);
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
	if (issticky(c))
		winstate[states++] = atom[WindowStateSticky];
	if (c->notaskbar)
		winstate[states++] = atom[WindowStateNoTaskbar];
	if (c->nopager)
		winstate[states++] = atom[WindowStateNoPager];
	if (c->isicon)
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
			// action[actions++] = atom[WindowActionMaxH];
			// action[actions++] = atom[WindowActionMaxV];
		}
		action[actions++] = atom[WindowActionMin];
		if (c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			action[actions++] = atom[WindowActionMove];
			if (!c->isfixed)
				action[actions++] = atom[WindowActionResize];
		}
		action[actions++] = atom[WindowActionStick];
		action[actions++] = atom[WindowActionFloat];
		if (c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			// action[actions++] = atom[WindowActionShade];
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
		if ((set == _NET_WM_STATE_ADD || set == _NET_WM_STATE_TOGGLE)
				&& !issticky(c)) {
			tag(c, -1);
		} else
		if ((set == _NET_WM_STATE_REMOVE || set == _NET_WM_STATE_TOGGLE)
				&& issticky(c)) {
			static char buf[512];
			unsigned int i, j;
			int matched = 0;
			regmatch_t tmp;
			XClassHint ch = { 0, };


			/* reapply rules */
			XGetClassHint(dpy, c->win, &ch);
			snprintf(buf, sizeof(buf), "%s:%s:%s",
			    ch.res_class ? ch.res_class : "", ch.res_name ? ch.res_name : "", c->name);
			buf[LENGTH(buf)-1] = 0;
			for (i = 0; i < nrules; i++) {
				if (rules[i]->propregex && !regexec(rules[i]->propregex, buf, 1, &tmp, 0)) {
					for (j = 0; rules[i]->tagregex && j < ntags; j++) {
						if (!regexec(rules[i]->tagregex, tags[j], 1, &tmp, 0)) {
							matched = j;
							c->tags[j] = True;
						}
					}
				}
			}
			if (ch.res_class)
				XFree(ch.res_class);
			if (ch.res_name)
				XFree(ch.res_name);
			tag(c, matched);
		}

	} else if (state == atom[WindowStateMaxV]) {
		/* TODO */
	} else if (state == atom[WindowStateMaxH]) {
		/* TODO */
	} else if (state == atom[WindowStateShaded]) {
		/* TODO */
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
		}
	} else if (state == atom[WindowStateBelow]) {
		if ((set == _NET_WM_STATE_ADD && !c->isbelow) ||
		    (set == _NET_WM_STATE_REMOVE && c->isbelow) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isbelow = !c->isbelow;
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
	c->ismanaged = True;
	ewmh_update_net_window_state(c);
}

void
clientmessage(XEvent *e) {
	XClientMessageEvent *ev = &e->xclient;
	Client *c;
	Atom message_type = ev->message_type;

	c = getclient(ev->window, clients, ClientWindow);

	if (c) {
		if (0) {
		} else if (message_type == atom[CloseWindow]) {
			killclient(c);
		} else if (message_type == atom[ActiveWindow]) {
			if (c->isicon)
				c->isicon = False;
			focus(c);
			arrange(curmonitor());
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
			XConfigureRequestEvent cre;
			unsigned flags = (unsigned) ev->data.l[0];
			unsigned source = ((flags >> 12) & 0x0f);

			/* echinus is (unfortunately) ignoring gravity */
			/* unsigned gravity = flags & 0xff; */

			if (!(c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)))
				return;
			if (source != 0 && source != 2)
				return;

			cre.type = ConfigureRequest;
			cre.send_event = False;
			cre.display = dpy;
			cre.parent = None;
			cre.window = c->win;
			cre.value_mask = 0;

			if (flags & (1 << 8)) {
				cre.value_mask |= CWX;
				cre.x = ev->data.l[1];
			}
			if (flags & (1 << 9)) {
				cre.value_mask |= CWY;
				cre.y = ev->data.l[2];
			}
			if (flags & (1 << 10)) {
				cre.value_mask |= CWWidth;
				cre.width = ev->data.l[3];
			}
			if (flags & (1 << 11)) {
				cre.value_mask |= CWHeight;
				cre.height = ev->data.l[4];
			}
			configurerequest((XEvent *)&cre);
		} else if (message_type == atom[WindowMoveResize]) {
			int x_root = (int) ev->data.l[0];
			int y_root = (int) ev->data.l[1];
			int direct = (int) ev->data.l[2];
			int button = (int) ev->data.l[3];
			int source = (int) ev->data.l[4];

			(void) y_root;
			(void) x_root;
			(void) button;
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
				/* this just warps the pointer to the bottom
				   right corner of the window */
				mouseresize(c);
				break;
			case 8: /* _NET_WM_MOVERESIZE_MOVE */
				mousemove(c);
				break;
			case 9: /* _NET_WM_MOVERESIZE_SIZE_KEYBOARD */
				/* TODO */
				break;
			case 10: /* _NET_WM_MOVERESIZE_MOVE_KEYBOARD */
				/* TODO */
				break;
			case 11: /* _NET_WM_MOVERESIZE_CANCEL */
				break;
			}
		}
	} else if (ev->window == root) {
		if (0) {
		} else if (message_type == atom[NumberOfDesk]) {
			/* TODO */
		} else if (message_type == atom[DeskGeometry]) {
			/* TODO */
		} else if (message_type == atom[DeskViewport]) {
			/* TODO */
		} else if (message_type == atom[CurDesk]) {
			int tag = ev->data.l[0];
			if (0 <= tag && tag < ntags)
				view(tag);
		} else if (message_type == atom[ShowingDesktop]) {
			/* TODO */
		} else if (message_type == atom[WMRestart]) {
			/* TODO */
		} else if (message_type == atom[WMShutdown]) {
			/* TODO */
		} else if (message_type == atom[Manager]) {
			/* TODO */
		} else if (message_type == atom[RequestFrameExt]) {
			/* TODO */
		} else if (message_type == atom[StartupInfoBegin]) {
			/* TODO */
		} else if (message_type == atom[StartupInfo]) {
			/* TODO */
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

int
getstrut(Client *c, Atom strut) {
	long *state;
	int ret = 0;
	Monitor *m;
	unsigned long i, n;

	if (!(m = clientmonitor(c)))
		return ret;

	state = getcard(c->win, strut, &n);
	if (n) {
		for (i = LeftStrut; i < LastStrut; i++)
			m->struts[i] = max(state[i], m->struts[i]);
		ret = 1;
	}
	if (state)
		XFree(state);
	return ret;
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
	[DeskNames] = ewmh_update_net_desktop_names,
	[CurDesk] = ewmh_update_net_current_desktop,
	[ELayout] = update_echinus_layout_name,
	[WorkArea] = ewmh_update_net_work_area,
	[DeskModes] = ewmh_update_net_desktop_modes,
	[WindowState] = ewmh_update_net_window_state,
	[WindowActions] = ewmh_update_net_window_actions,
	[WindowExtents] = ewmh_update_net_window_extents,
};
