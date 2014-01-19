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
	"_NET_RESTART",				/* TODO */
	"_NET_SHUTDOWN",			/* TODO */
	"_NET_DESKTOP_LAYOUT",			/* TODO */
	/* _WIN_PROTOCOLS following */
	"_WIN_APP_STATE",
	"_WIN_AREA_COUNT",
	"_WIN_AREA",
	"_WIN_CLIENT_LIST",
	"_WIN_CLIENT_MOVING",
	"_WIN_DESKTOP_BUTTON_PROXY",
	"_WIN_EXPANDED_SIZE",
	"_WIN_FOCUS",
	"_WIN_HINTS",
	"_WIN_ICONS",
	"_WIN_LAYER",
	"_WIN_MAXIMIZED_GEOMETRY",
	"_WIN_PROTOCOLS",
	"_WIN_RESIZE",				/* TODO */
	"_WIN_STATE",
	"_WIN_SUPPORTING_WM_CHECK",
	"_WIN_WORKAREA",
	"_WIN_WORKSPACE_COUNT",
	"_WIN_WORKSPACE_NAMES",
	"_WIN_WORKSPACE",
	"_WIN_WORKSPACES",
	"_SWM_VROOT",
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
	"_NET_VISIBLE_DESKTOPS",
	"_NET_MONITOR_GEOMETRY",

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
	"_NET_STARTUP_INFO",
	"_NET_STARTUP_INFO_BEGIN",
	"_NET_WM_SYNC_REQUEST",
	"_NET_WM_SYNC_REQUEST_COUNTER",
	"_NET_WM_FULLSCREEN_MONITORS",

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
	"_NET_WM_PING",
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
initewmh(Window win)
{
	char name[] = "echinus";
	long data;

	XInternAtoms(dpy, atomnames, NATOMS, False, atom);
	XChangeProperty(dpy, root, _XA_NET_SUPPORTED, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &_XA_NET_CLIENT_LIST,
			NATOMS - ClientList);

	XChangeProperty(dpy, win, _XA_NET_WM_NAME, _XA_UTF8_STRING, 8,
			PropModeReplace, (unsigned char *) name, strlen(name));
	data = getpid();
	XChangeProperty(dpy, win, _XA_NET_WM_PID, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);
	XChangeProperty(dpy, root, _XA_NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &win, 1);
	XChangeProperty(dpy, win, _XA_NET_SUPPORTING_WM_CHECK, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &win, 1);

	XChangeProperty(dpy, root, _XA_WIN_PROTOCOLS, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &_XA_WIN_APP_STATE,
			ClientList - WinAppState);
	XChangeProperty(dpy, win, _XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &win, 1);
	XChangeProperty(dpy, root, _XA_WIN_SUPPORTING_WM_CHECK, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &win, 1);
	XChangeProperty(dpy, win, _XA_WIN_DESKTOP_BUTTON_PROXY, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &win, 1);
	XChangeProperty(dpy, root, _XA_WIN_DESKTOP_BUTTON_PROXY, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &win, 1);

	ewmh_update_net_client_list();
}

void
exitewmh(WithdrawCause cause)
{
	XDeleteProperty(dpy, root, _XA_WIN_SUPPORTING_WM_CHECK);
	XDeleteProperty(dpy, root, _XA_NET_SUPPORTING_WM_CHECK);
	XDeleteProperty(dpy, root, _XA_WIN_DESKTOP_BUTTON_PROXY);

	XDeleteProperty(dpy, root, _XA_WIN_PROTOCOLS);
	XDeleteProperty(dpy, root, _XA_NET_SUPPORTED);

	switch (cause) {
		int i;

	default:
	case CauseQuitting:
	{
		Atom props[] = {
			_XA_WIN_AREA_COUNT,
			_XA_WIN_AREA,
			_XA_WIN_CLIENT_LIST,
			_XA_WIN_FOCUS,
			_XA_WIN_WORKSPACE_COUNT,
			_XA_WIN_WORKSPACE_NAMES,
			_XA_WIN_WORKSPACE,
			_XA_WIN_WORKSPACES,

			_XA_NET_ACTIVE_WINDOW,
			_XA_NET_CLIENT_LIST,
			_XA_NET_CLIENT_LIST_STACKING,
			_XA_NET_NUMBER_OF_DESKTOPS,
			_XA_NET_DESKTOP_NAMES,
			_XA_NET_CURRENT_DESKTOP,
			_XA_NET_DESKTOP_VIEWPORT,
			_XA_NET_SHOWING_DESKTOP,
			_XA_NET_VISIBLE_DESKTOPS,
			_XA_NET_MONITOR_GEOMETRY,
			_XA_NET_DESKTOP_MODES,
			/* _XA_NET_DESKTOP_LAYOUT, */
			/* _XA_NET_DESKTOP_STRUTS, */
			_XA_NET_RESTART,
			None
		};

		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		/* fall through */
	}
	case CauseSwitching:
	{
		Atom props[] = {
			_XA_ECHINUS_LAYOUT,
			_XA_ECHINUS_SELTAGS,

			_XA_WIN_WORKAREA,
			_XA_NET_WORKAREA,

			None
		};

		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		/* fall through */
	}
	case CauseRestarting:
	{
		Atom props[] = {
			_XA_SWM_VROOT,
			_XA_NET_VIRTUAL_ROOTS,
			_XA_NET_DESKTOP_GEOMETRY,
			_XA_NET_RESTART,

			None
		};

		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		break;
	}
	}
}

void
ewmh_add_client(Client *c)
{
}

void
ewmh_del_client(Client * c, WithdrawCause cause)
{
	switch (cause) {
		int i;

	default:
	case CauseDestroyed:
	case CauseReparented:
	case CauseUnmapped:
	{
		ewmh_update_net_client_list();
		if (sel == c) {
			focus(NULL);
			ewmh_update_net_active_window();
		}
		/* fall through */
	}
	case CauseQuitting:
	{
		Atom props[] = {
			_XA_WM_STATE,

			_XA_WIN_MAXIMIZED_GEOMETRY,

			_XA_NET_WM_DESKTOP,
			_XA_NET_WM_STATE,
			_XA_NET_WM_ALLOWED_ACTIONS,

			None
		};
		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		/* fall through */
	}
	case CauseSwitching:
	{
		Atom props[] = {
			_XA_NET_WM_DESKTOP_MASK,
			_XA_NET_FRAME_EXTENTS,
			_XA_NET_WM_VISIBLE_NAME,
			_XA_NET_WM_VISIBLE_ICON_NAME,
			_XA_NET_WM_FULLSCREEN_MONITORS,

			None
		};
		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		/* fall through */
	}
	case CauseRestarting:
	{
		Atom props[] = {
			_XA_KDE_NET_WM_FRAME_STRUT,

			None
		};
		for (i = 0; props[i]; i++)
			XDeleteProperty(dpy, root, props[i]);
		/* fall through */
		break;
	}
	}
}

void
ewmh_update_echinus_layout_name() {
	XChangeProperty(dpy, root, _XA_ECHINUS_LAYOUT, XA_STRING, 8, PropModeReplace,
			(const unsigned char *) &views[curmontag].layout->symbol, 1L);
}

void
ewmh_update_net_client_list_stacking() {
	Client *c;
	Window *wl = NULL;
	int i, n;

	DPRINTF("%s\n", "Updating _NET_CLIENT_LIST_STACKING");
	for (n = 0, c = stack; c; n++, c = c->snext) ;
	if (n && (wl = ecalloc(n, sizeof(Window)))) {
		for (i = 0, c = stack; c && i < n; c = c->snext)
			wl[i++] = c->win;
		assert(i == n);
	}
	XChangeProperty(dpy, root, _XA_NET_CLIENT_LIST_STACKING, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) wl, n);
	free(wl);
	XFlush(dpy);		/* XXX: caller's responsibility */
}

void
ewmh_update_net_client_list() {
	Client *c;
	Window *wl = NULL;
	int i, n;

	DPRINTF("%s\n", "Updating _NET_CLIENT_LIST");
	for (n = 0, c = clist; c; n++, c = c->cnext) ;
	if (n && (wl = calloc(n, sizeof(Window)))) {
		for (i = 0, c = clist; c && i < n; c = c->cnext)
			wl[i++] = c->win;
		assert(i == n);
	}
	XChangeProperty(dpy, root, _XA_NET_CLIENT_LIST, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) wl, n);
	XChangeProperty(dpy, root, _XA_WIN_CLIENT_LIST, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) wl, n);
	free(wl);
	ewmh_update_net_client_list_stacking();
	XFlush(dpy);		/* XXX: caller's responsibility */
}

void
ewmh_update_net_desktop_viewport() {
	long *data;

	data = ecalloc(ntags * 2, sizeof(data[0]));
	XChangeProperty(dpy, root, _XA_NET_DESKTOP_VIEWPORT, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, ntags * 2);
	XChangeProperty(dpy, root, _XA_WIN_AREA, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, 2);
}

Bool names_synced = False;

void
ewmh_update_net_desktop_names() {
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
	XChangeProperty(dpy, root, _XA_NET_DESKTOP_NAMES, _XA_UTF8_STRING, 8,
			PropModeReplace, (unsigned char *) buf, len);
	XChangeProperty(dpy, root, _XA_WIN_WORKSPACE_NAMES, XA_STRING, 8,
			PropModeReplace, (unsigned char *) buf, len);
	free(buf);
	names_synced = True;
}

void
ewmh_update_echinus_seltags() {
	Monitor *m;
	long *seltags;
	unsigned int i;

	DPRINTF("%s\n", "Updating _ECHINUS_SELTAGS");
	seltags = ecalloc(ntags, sizeof(seltags[0]));
	for (m = monitors; m != NULL; m = m->next)
		for (i = 0; i < ntags; i++)
			if (m->seltags[i])
				seltags[i] = True;
	XChangeProperty(dpy, root, _XA_ECHINUS_SELTAGS, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) seltags, ntags);
	free(seltags);
}

void
ewmh_update_net_number_of_desktops() {
	long data = ntags;

	DPRINTF("%s\n", "Updating _NET_NUMBER_OF_DESKTOPS");
	XChangeProperty(dpy, root, _XA_NET_NUMBER_OF_DESKTOPS, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);
	XChangeProperty(dpy, root, _XA_WIN_WORKSPACE_COUNT, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 1);
	ewmh_update_net_desktop_modes();
	ewmh_update_net_desktop_viewport();
	ewmh_update_net_desktop_names();
	ewmh_update_echinus_seltags();
}

void
ewmh_process_net_number_of_desktops() {
	long *card;
	unsigned long n = 0;

	card = getcard(root, _XA_NET_NUMBER_OF_DESKTOPS, &n);
	if (n > 0) {
		ntags = card[0];
		XFree(card);
	}
	if (1 > ntags || ntags > 64)
		ntags = atoi(getresource("tags.number", "5"));
}

void
ewmh_process_net_showing_desktop() {
	long *card;
	unsigned long n = 0;

	DPRINTF("%s\n", "Processing _NET_SHOWING_DESKTOP");
	card = getcard(root, _XA_NET_SHOWING_DESKTOP, &n);
	if (n > 0) {
		Bool show = card[0];

		XFree(card);
		if (!show != !showing_desktop)
			toggleshowing();
	}
}


void
ewmh_update_net_showing_desktop() {
	long data = showing_desktop ? 1 : 0;

	DPRINTF("%s\n", "Updating _NET_SHOWING_DESKTOP");
	XChangeProperty(dpy, root, _XA_NET_SHOWING_DESKTOP, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&data, 1);
}

void
ewmh_update_net_desktop_geometry() {
	long data[2];

	DPRINTF("%s\n", "Updating _NET_DESKTOP_GEOMETRY");
	data[0] = DisplayWidth(dpy, screen);
	data[1] = DisplayHeight(dpy, screen);
	XChangeProperty(dpy, root, _XA_NET_DESKTOP_GEOMETRY, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, 2);
	data[0] = 1;
	data[1] = 1;
	XChangeProperty(dpy, root, _XA_WIN_AREA_COUNT, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&data, 2);
}

void
ewmh_update_net_monitor_geometry(Client *c) {
	Monitor *m;
	unsigned int i, n;
	long *data;

	for (n = 0, m = monitors; m; m = m->next, n++) ;
	data = ecalloc(4 * n, sizeof(*data));
	for (i = 0, m = monitors; m; m = m->next) {
		data[i++] = m->sx;
		data[i++] = m->sy;
		data[i++] = m->sw;
		data[i++] = m->sh;
	}
	XChangeProperty(dpy, root, _XA_NET_MONITOR_GEOMETRY, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)data, i);
}

void
ewmh_update_net_virtual_roots() {
	XDeleteProperty(dpy, root, _XA_NET_VIRTUAL_ROOTS);
}

void
ewmh_update_net_current_desktop() {
	long *data;
	unsigned int i, n;
	Monitor *m;

	DPRINTF("%s\n", "Updating _NET_CURRENT_DESKTOP");
	for (n = 0, m = monitors; m; m = m->next, n++) ;
	data = ecalloc(n, sizeof(*data));
	for (i = 0, m = monitors; m; data[i++] = m->curtag, m = m->next) ;
	XChangeProperty(dpy, root, _XA_NET_CURRENT_DESKTOP, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, n);
	XChangeProperty(dpy, root, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, n);
	free(data);
	ewmh_update_echinus_layout_name();
	ewmh_update_echinus_seltags();
}

void
ewmh_update_net_visible_desktops() {
	long *data;
	unsigned int i, n;
	Monitor *m;

	DPRINTF("%s\n", "Updating _NET_VISIBLE_DESKTOPS");
	for (n = 0, m = monitors; m; m = m->next, n++) ;
	data = ecalloc(n, sizeof(*data));
	for (i = 0, m = monitors; m; data[i++] = m->curtag, m = m->next) ;
	XChangeProperty(dpy, root, _XA_NET_VISIBLE_DESKTOPS, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, n);
	free(data);
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

	desktops = getcard(c->win, _XA_NET_WM_DESKTOP, &n);
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
	} else {
		if (desktops)
			XFree(desktops);
		desktops = getcard(c->win, _XA_WIN_WORKSPACE, &n);
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
	}
	if (desktops)
		XFree(desktops);
	return False;
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
	XChangeProperty(dpy, c->win, _XA_WIN_WORKSPACES, XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *)data, longs);
	XChangeProperty(dpy, c->win, _XA_NET_WM_DESKTOP_MASK, XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *)data, longs);
}

void
ewmh_update_net_window_desktop(Client *c) {
	DPRINTF("Updating _NET_WM_DESKTOP for 0x%lx\n", c->win);
	if (isomni(c)) {
		long i = -1;

		XChangeProperty(dpy, c->win, _XA_NET_WM_DESKTOP, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &i, 1);
		XChangeProperty(dpy, c->win, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &i, 1);
	} else {
		unsigned int i, j, n;
		long *data;

		for (n = 0, i = 0; i < ntags; i++)
			if (c->tags[i])
				n++;
		data = calloc(n, sizeof(*data));
		for (j = 0, i = 0; i < ntags; i++)
			if (c->tags[i])
				data[j++] = i;
		XChangeProperty(dpy, c->win, _XA_NET_WM_DESKTOP, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, n);
		XChangeProperty(dpy, c->win, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, n);
	}
	ewmh_update_net_window_desktop_mask(c);
	ewmh_update_net_window_state(c);
}

Bool
ewmh_process_net_window_desktop_mask(Client *c) {
	unsigned int i, j, k, l;
	long *desktops = NULL;
	unsigned long n = 0;
	Bool *prev;

	desktops = getcard(c->win, _XA_WIN_WORKSPACES, &n);
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
	} else {
		if (desktops)
			XFree(desktops);
		desktops = getcard(c->win, _XA_NET_WM_DESKTOP_MASK, &n);
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
	}
	if (desktops)
		XFree(desktops);
	return False;
}

void
ewmh_update_net_work_area() {
	long *geoms, longs = ntags * 4;
	int i, j, x, y, w, h, l = 0, r = 0, t = 0, b = 0;
	Monitor *m;
	long workarea[4];

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
	XChangeProperty(dpy, root, _XA_NET_WORKAREA, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) geoms, longs);
	free(geoms);
	workarea[0] = x;
	workarea[1] = y;
	workarea[2] = x + w;
	workarea[3] = y + h;
	XChangeProperty(dpy, root, _XA_WIN_WORKAREA, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) workarea, 4L);
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
		status = XGetWindowProperty(dpy, root, _XA_NET_DESKTOP_NAMES, 0L, bytes, False,
				       _XA_UTF8_STRING, &real, &format, &nitems, &extra,
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
	ewmh_update_net_desktop_names();
}

void
ewmh_update_net_active_window() {
	Window win;

	DPRINTF("%s\n", "Updating _NET_ACTIVE_WINDOW");
	win = sel ? sel->win : None;
	XChangeProperty(dpy, root, _XA_NET_ACTIVE_WINDOW, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &win, 1);
	XChangeProperty(dpy, root, _XA_WIN_FOCUS, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &win, 1);
}

void
ewmh_update_net_desktop_modes() {
	long *data;
	unsigned int i;

	DPRINTF("%s\n", "Updating _NET_DESKTOP_MODES");
	data = ecalloc(ntags, sizeof(data[0]));
	for (i = 0; i < ntags; i++) {
		switch (views[i].layout->symbol) {
		case 'i':
		case 'f':
			data[i] = _XA_NET_DESKTOP_MODE_FLOATING;
			break;
		case 't':
			data[i] = _XA_NET_DESKTOP_MODE_TILED;
			break;
		case 'b':
			data[i] = _XA_NET_DESKTOP_MODE_BOTTOM_TILED;
			break;
		case 'm':
			data[i] = _XA_NET_DESKTOP_MODE_MONOCLE;
			break;
		case 'u':
			data[i] = _XA_NET_DESKTOP_MODE_TOP_TILED;
			break;
		case 'l':
			data[i] = _XA_NET_DESKTOP_MODE_LEFT_TILED;
			break;
		default:
			data[i] = None;
			break;
		}
	}
	XChangeProperty(dpy, root, _XA_NET_DESKTOP_MODES, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) data, ntags);
	free(data);
	ewmh_update_echinus_layout_name();
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
	if (XGetWindowProperty(dpy, win, _XA_MOTIF_WM_HINTS, 0L, 20L, False,
			       _XA_MOTIF_WM_HINTS, &real, &format, &n, &extra,
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

#define WIN_LAYER_DESKTOP      0
#define WIN_LAYER_BELOW        2
#define WIN_LAYER_NORMAL       4
#define WIN_LAYER_ONTOP        6
#define WIN_LAYER_DOCK         8
#define WIN_LAYER_ABOVE_DOCK  10
#define WIN_LAYER_MENU        12

void
wmh_update_win_layer(Client * c)
{
	unsigned long layer = WIN_LAYER_NORMAL;

	if (WTCHECK(c, WindowTypeDesk))
		layer = WIN_LAYER_DESKTOP;
	else if (WTCHECK(c, WindowTypeMenu))
		layer = WIN_LAYER_MENU;
	else if (c->isbelow)
		layer = WIN_LAYER_BELOW;
	else if (WTCHECK(c, WindowTypeDock) && c->isabove)
		layer = WIN_LAYER_ABOVE_DOCK;
	else if (WTCHECK(c, WindowTypeDock) && !c->isabove)
		layer = WIN_LAYER_DOCK;
	else if (c->wintype & ~(WTFLAG(WindowTypeNormal) | WTFLAG(WindowTypeDesk) |
				WTFLAG(WindowTypeDock) | WTFLAG(WindowTypeMenu)))
		layer = 13;
	else
		layer = WIN_LAYER_NORMAL;
	XChangeProperty(dpy, c->win, _XA_WIN_LAYER, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &layer, 1L);
}

void
ewmh_update_net_window_actions(Client *c) {
	long action[32];
	int actions = 0;

	DPRINTF("Updating _NET_WM_ALLOWED_ACTIONS for 0x%lx\n", c->win);
	action[actions++] = _XA_NET_WM_ACTION_ABOVE;
	action[actions++] = _XA_NET_WM_ACTION_BELOW;
	if (!c->isbastard) {
		action[actions++] = _XA_NET_WM_ACTION_CHANGE_DESKTOP;
	}
	action[actions++] = _XA_NET_WM_ACTION_CLOSE;
	if (!c->isbastard) {
		if (c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			action[actions++] = _XA_NET_WM_ACTION_FULLSCREEN;
			action[actions++] = _XA_NET_WM_ACTION_MAXIMIZE_HORZ;
			action[actions++] = _XA_NET_WM_ACTION_MAXIMIZE_VERT;
		}
		action[actions++] = _XA_NET_WM_ACTION_MINIMIZE;
		if (c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			action[actions++] = _XA_NET_WM_ACTION_MOVE;
			if (!c->isfixed)
				action[actions++] = _XA_NET_WM_ACTION_RESIZE;
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
		action[actions++] = _XA_NET_WM_ACTION_STICK;
		action[actions++] = _XA_NET_WM_ACTION_FLOAT;
		if (c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
			if (c->title)
				action[actions++] = _XA_NET_WM_ACTION_SHADE;
			action[actions++] = _XA_NET_WM_ACTION_FILL;
		}
	}

	XChangeProperty(dpy, c->win, _XA_NET_WM_ALLOWED_ACTIONS, XA_ATOM, 32,
		PropModeReplace, (unsigned char *) action, actions);
}

#define WIN_STATE_STICKY          (1<<0) /* everyone knows sticky */
#define WIN_STATE_MINIMIZED       (1<<1) /* Reserved - definition is unclear */
#define WIN_STATE_MAXIMIZED_VERT  (1<<2) /* window in maximized V state */
#define WIN_STATE_MAXIMIZED_HORIZ (1<<3) /* window in maximized H state */
#define WIN_STATE_HIDDEN          (1<<4) /* not on taskbar but window visible */
#define WIN_STATE_SHADED          (1<<5) /* shaded (MacOS / Afterstep style) */
#define WIN_STATE_HID_WORKSPACE   (1<<6) /* not on current desktop */
#define WIN_STATE_HID_TRANSIENT   (1<<7) /* owner of transient is hidden */
#define WIN_STATE_FIXED_POSITION  (1<<8) /* window is fixed in position even */
#define WIN_STATE_ARRANGE_IGNORE  (1<<9) /* ignore for auto arranging */

void
ewmh_update_net_window_state(Client *c) {
	long winstate[16];
	int states = 0;
	unsigned long state = 0;
	Monitor *m;

	/* do not update until we have finished reading it */
	if (!c->ismanaged)
		return;

	DPRINTF("Updating _NET_WM_STATE for 0x%lx\n", c->win);
	if (c->ismodal)
		winstate[states++] = _XA_NET_WM_STATE_MODAL;
	if (c->issticky) {
		winstate[states++] = _XA_NET_WM_STATE_STICKY;
		state |= WIN_STATE_STICKY;
	}
	if (c->notaskbar)
		winstate[states++] = _XA_NET_WM_STATE_SKIP_TASKBAR;
	if (c->nopager)
		winstate[states++] = _XA_NET_WM_STATE_SKIP_PAGER;
	if (c->isicon || c->ishidden)
		winstate[states++] = _XA_NET_WM_STATE_HIDDEN;
	if (c->isicon)
		state |= WIN_STATE_MINIMIZED;
	if (c->isicon && c->transfor)
		state |= WIN_STATE_HID_TRANSIENT;
	if (c->ishidden)
		state |= WIN_STATE_HIDDEN;
	if (c->isfixed)
		state |= WIN_STATE_FIXED_POSITION;
	for (m = monitors; m && isvisible(c, m); m = m->next) ;
	if (!m)
		state |= WIN_STATE_HID_WORKSPACE;
	if (c->ismax)
		winstate[states++] = _XA_NET_WM_STATE_FULLSCREEN;
	if (c->isabove)
		winstate[states++] = _XA_NET_WM_STATE_ABOVE;
	if (c->isbelow)
		winstate[states++] = _XA_NET_WM_STATE_BELOW;
	if (c->isattn)
		winstate[states++] = _XA_NET_WM_STATE_DEMANDS_ATTENTION;
	if (c == sel)
		winstate[states++] = _XA_NET_WM_STATE_FOCUSED;
	if (c->isfixed)
		winstate[states++] = _XA_NET_WM_STATE_FIXED;
	if (c->isfloating) {
		winstate[states++] = _XA_NET_WM_STATE_FLOATING;
		state |= WIN_STATE_ARRANGE_IGNORE;
	}
	if (c->isfill)
		winstate[states++] = _XA_NET_WM_STATE_FILLED;
	if (c->ismaxv) {
		winstate[states++] = _XA_NET_WM_STATE_MAXIMIZED_VERT;
		state |= WIN_STATE_MAXIMIZED_VERT;
	}
	if (c->ismaxh) {
		winstate[states++] = _XA_NET_WM_STATE_MAXIMIZED_HORZ;
		state |= WIN_STATE_MAXIMIZED_HORIZ;
	}

	XChangeProperty(dpy, c->win, _XA_NET_WM_STATE, XA_ATOM, 32,
		PropModeReplace, (unsigned char *) winstate, states);
	XChangeProperty(dpy, c->win, _XA_WIN_STATE, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &state, 1);
	wmh_update_win_layer(c);
	ewmh_update_net_window_actions(c);
}

void
wmh_process_state_mask(Client *c, unsigned int mask, unsigned int change)
{
	if (mask & WIN_STATE_STICKY)
		if (((change & WIN_STATE_STICKY) && !c->issticky) ||
		    (!(change & WIN_STATE_STICKY) && c->issticky)) {
			if ((c->issticky = !c->issticky))
				tag(c, -1);
			else
				tag(c, curmontag);
		}
	if (mask & WIN_STATE_MINIMIZED)
		if (((change & WIN_STATE_MINIMIZED) && !c->isicon) ||
		    (!(change & WIN_STATE_MINIMIZED) && c->isicon)) {
			if (c->isicon) {
				c->isicon = False;
				c->ishidden = False;
				focus(c);
				arrange(clientmonitor(c));
			} else {
				iconify(c);
			}
		}
	if (mask & WIN_STATE_MAXIMIZED_VERT)
		if (((change & WIN_STATE_MAXIMIZED_VERT) && !c->ismaxv) ||
		    (!(change & WIN_STATE_MAXIMIZED_VERT) && c->ismaxv))
			togglemaxv(c);
	if (mask & WIN_STATE_MAXIMIZED_HORIZ)
		if (((change & WIN_STATE_MAXIMIZED_HORIZ) && !c->ismaxh) ||
		    (!(change & WIN_STATE_MAXIMIZED_HORIZ) && c->ismaxh))
			togglemaxh(c);
	if (mask & WIN_STATE_HIDDEN)
		if (((change & WIN_STATE_HIDDEN) && !c->ishidden) ||
		    (!(change & WIN_STATE_HIDDEN) && c->ishidden))
			togglehidden(c);
	if (mask & WIN_STATE_SHADED)
		if (((change & WIN_STATE_SHADED) && !c->isshade) ||
		    (!(change & WIN_STATE_SHADED) && c->isshade))
			toggleshade(c);
	if (mask & WIN_STATE_HID_WORKSPACE) {
		/* read-only */
	}
	if (mask & WIN_STATE_HID_TRANSIENT) {
		/* read-only */
	}
	if (mask & WIN_STATE_FIXED_POSITION)
		if (((change & WIN_STATE_FIXED_POSITION) && !c->isfixed) ||
		    (!(change & WIN_STATE_FIXED_POSITION) && c->isfixed)) {
			c->isfixed = !c->isfixed;
			arrange(NULL);
		}
	if (mask & WIN_STATE_ARRANGE_IGNORE)
		if (((change & WIN_STATE_ARRANGE_IGNORE) && !c->isfloating) ||
		    (!(change & WIN_STATE_ARRANGE_IGNORE) && c->isfloating))
			togglefloating(c);
}

void
ewmh_process_state_atom(Client *c, Atom state, int set) {
	if (0) {
	} else if (state == _XA_NET_WM_STATE_MODAL) {
		if ((set == _NET_WM_STATE_ADD && !c->ismodal) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismodal) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->ismodal = !c->ismodal;
			if (c->ismodal)
				focus(c);
		}
	} else if (state == _XA_NET_WM_STATE_STICKY) {
		if ((set == _NET_WM_STATE_ADD && !c->issticky) ||
		    (set == _NET_WM_STATE_REMOVE && c->issticky) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->issticky = !c->issticky;
			if (c->issticky)
				tag(c, -1);
			else
				tag(c, curmontag);
		}
	} else if (state == _XA_NET_WM_STATE_MAXIMIZED_VERT) {
		if ((set == _NET_WM_STATE_ADD && !c->ismaxv) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismaxv) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglemaxv(c);
	} else if (state == _XA_NET_WM_STATE_MAXIMIZED_HORZ) {
		if ((set == _NET_WM_STATE_ADD && !c->ismaxh) ||
		    (set == _NET_WM_STATE_REMOVE && c->ismaxh) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglemaxh(c);
	} else if (state == _XA_NET_WM_STATE_SHADED) {
		if ((set == _NET_WM_STATE_ADD && !c->isshade) ||
		    (set == _NET_WM_STATE_REMOVE && c->isshade) ||
		    (set == _NET_WM_STATE_TOGGLE))
			toggleshade(c);
	} else if (state == _XA_NET_WM_STATE_SKIP_TASKBAR) {
		if ((set == _NET_WM_STATE_ADD && !c->notaskbar) ||
		    (set == _NET_WM_STATE_REMOVE && c->notaskbar) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->notaskbar = !c->notaskbar;
		}
	} else if (state == _XA_NET_WM_STATE_SKIP_PAGER) {
		if ((set == _NET_WM_STATE_ADD && !c->nopager) ||
		    (set == _NET_WM_STATE_REMOVE && c->nopager) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->nopager = !c->nopager;
		}
	} else if (state == _XA_NET_WM_STATE_HIDDEN) {
		/* Implementation note: if an Application asks to toggle
		   _NET_WM_STATE_HIDDEN the Window Manager should probably just ignore
		   the request, since _NET_WM_STATE_HIDDEN is a function of some other
		   aspect of the window such as minimization, rather than an independent
		   state. */
		if ((set == _NET_WM_STATE_ADD && !c->ishidden) ||
		    (set == _NET_WM_STATE_REMOVE && c->ishidden) ||
		    (set == _NET_WM_STATE_TOGGLE))
			togglehidden(c);
	} else if (state == _XA_NET_WM_STATE_FULLSCREEN) {
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
	} else if (state == _XA_NET_WM_STATE_ABOVE) {
		if ((set == _NET_WM_STATE_ADD && !c->isabove) ||
		    (set == _NET_WM_STATE_REMOVE && c->isabove) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isabove = !c->isabove;
			restack();
		}
	} else if (state == _XA_NET_WM_STATE_BELOW) {
		if ((set == _NET_WM_STATE_ADD && !c->isbelow) ||
		    (set == _NET_WM_STATE_REMOVE && c->isbelow) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isbelow = !c->isbelow;
			restack();
		}
	} else if (state == _XA_NET_WM_STATE_DEMANDS_ATTENTION) {
		if ((set == _NET_WM_STATE_ADD && !c->isattn) ||
		    (set == _NET_WM_STATE_REMOVE && c->isattn) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			c->isattn = !c->isattn;
		}
	} else if (state == _XA_NET_WM_STATE_FOCUSED) {
		/* _NET_WM_STATE_FOCUSED indicates whether the window's decorations are
		   drawn in an active state.  Clients MUST regard it as a read-only hint. 
		   It cannot be set at map time or changed via a _NET_WM_STATE client
		   message. */
	} else if (state == _XA_NET_WM_STATE_FIXED) {
		/* _NET_WM_STATE_FIXED is a read-only state. */
	} else if (state == _XA_NET_WM_STATE_FLOATING) {
		if ((set == _NET_WM_STATE_ADD && !c->isfloating) ||
		    (set == _NET_WM_STATE_REMOVE && c->isfloating) ||
		    (set == _NET_WM_STATE_TOGGLE)) {
			togglefloating(c);
		}
	} else if (state == _XA_NET_WM_STATE_FILLED) {
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
	XChangeProperty(dpy, c->win, _XA_NET_FRAME_EXTENTS, XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) data, 4L);
	DPRINTF("Updating _KDE_NET_WM_FRAME_STRUT for 0x%lx\n", c->win);
	XChangeProperty(dpy, c->win, _XA_KDE_NET_WM_FRAME_STRUT, XA_CARDINAL, 32,
		PropModeReplace, (unsigned char *) data, 4L);
}

void
wmh_update_win_maximized_geometry(Client *c)
{
	long data[4] = { c->x, c->y, c->w, c->h };

	XChangeProperty(dpy, c->win, _XA_WIN_MAXIMIZED_GEOMETRY, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 4L);
}

void
ewmh_update_net_window_visible_name(Client *c) {
	XTextProperty prop;

	if (c->name) {
		if (XmbTextListToTextProperty(dpy, &c->name, 1, XUTF8StringStyle, &prop) == Success)
			XSetTextProperty(dpy, c->win, &prop, _XA_NET_WM_VISIBLE_NAME);
	} else {
		XDeleteProperty(dpy, c->win, _XA_NET_WM_VISIBLE_NAME);
	}
}

void
ewmh_update_net_window_visible_icon_name(Client *c) {
	XTextProperty prop;

	if (c->icon_name) {
		if (XmbTextListToTextProperty(dpy, &c->icon_name, 1, XUTF8StringStyle, &prop) == Success)
			XSetTextProperty(dpy, c->win, &prop, _XA_NET_WM_VISIBLE_ICON_NAME);
	} else {
		XDeleteProperty(dpy, c->win, _XA_NET_WM_VISIBLE_ICON_NAME);
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
ewmh_process_net_startup_id(Client * c) {
#ifdef STARTUP_NOTIFICATION
	SnStartupSequence *seq;
#endif
	if (c->startup_id)
		return;
	if (!gettextprop(c->win, _XA_NET_STARTUP_ID, &c->startup_id))
		if (c->leader != None && c->leader != c->win)
			gettextprop(c->leader, _XA_NET_STARTUP_ID, &c->startup_id);
	if (c->startup_id) {
		char *pos;

		if ((pos = strstr(c->startup_id, "_TIME")))
			push_client_time(c, atol(pos + 5));
		/*
		 * TODO: there are three other things to get from the ID:
		 *   launcher: argv[0] of launcher
		 *   launchee: argv[0] of launchee
		 *   pid:      pid of launchee
		 */
	}
#ifdef STARTUP_NOTIFICATION
	if ((seq = find_startup_seq(c))) {
		int i, workspace;

		if (!c->startup_id) {
			c->startup_id = strdup(sn_startup_sequence_get_id(seq));
			XChangeProperty(dpy, c->win, _XA_NET_STARTUP_ID,
					_XA_UTF8_STRING, 8, PropModeReplace,
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
ewmh_process_net_window_user_time(Client *c) {
	long *time = NULL;
	unsigned long n = 0;
	Window win;

	if ((win = c->time_window) == None)
		win = c->win;

	time = getcard(win, _XA_NET_WM_USER_TIME, &n);
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
ewmh_process_net_window_user_time_window(Client * c) {
	Window *wins = NULL, win = None;
	unsigned long n = 0;

	wins = getwind(c->win, _XA_NET_WM_USER_TIME_WINDOW, &n);
	if (n > 0) {
		Window other = wins[0];

		XFree(wins);
		/* check recursive */
		wins = getwind(other, _XA_NET_WM_USER_TIME_WINDOW, &n);
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

	ewmh_process_net_window_user_time(c);
}

Atom *getatom(Window win, Atom atom, unsigned long *nitems);

#define WIN_HINTS_SKIP_FOCUS      (1<<0) /* "alt-tab" skips this win */
#define WIN_HINTS_SKIP_WINLIST    (1<<1) /* do not show in window list */
#define WIN_HINTS_SKIP_TASKBAR    (1<<2) /* do not show on taskbar */
#define WIN_HINTS_GROUP_TRANSIENT (1<<3) /* Reserved - definition is unclear */
#define WIN_HINTS_FOCUS_ON_CLICK  (1<<4) /* app only accepts focus if clicked */

void
wmh_process_win_window_hints(Client * c) {
	long *state;
	unsigned long n = 0;

	state = getcard(c->win, _XA_WIN_HINTS, &n);
	if (n > 0) {
		c->isfocusable = (state[0] & WIN_HINTS_SKIP_FOCUS) ? False : True;
		c->nopager = (state[0] & WIN_HINTS_SKIP_WINLIST) ? True : False;
		c->notaskbar = (state[0] & WIN_HINTS_SKIP_TASKBAR) ? True : False;
#if 0
		/* Handled by ICCCM 2.0 by setting WM_TRANSIENT_FOR hint to None or root */
		if (state[0] & WIN_HINTS_GROUP_TRANSIENT) {
		} else {
		}
#endif
		if (state[0] & WIN_HINTS_FOCUS_ON_CLICK) {
			/* TODO */
		} else {
			/* TODO */
		}
	}
}

void
wmh_process_layer(Client * c, unsigned int layer)
{
	switch (layer) {
	case WIN_LAYER_DESKTOP:
		c->wintype |= WTFLAG(WindowTypeDesk);
		break;
	case 1:
	case WIN_LAYER_BELOW:
		c->isbelow = True;
		break;
	case 3:
	case WIN_LAYER_NORMAL:
		c->wintype |= WTFLAG(WindowTypeNormal);
		break;
	case 5:
	case WIN_LAYER_ONTOP:
		c->isabove = True;
		break;
	case 7:
	case WIN_LAYER_DOCK:
		c->wintype |= WTFLAG(WindowTypeDock);
		break;
	case 9:
	case WIN_LAYER_ABOVE_DOCK:
		c->wintype |= WTFLAG(WindowTypeDock);
		c->isabove = True;
		break;
	case 11:
	case WIN_LAYER_MENU:
		c->wintype |= WTFLAG(WindowTypeMenu);
		break;
	case 13:
	default:
		c->wintype |= WTFLAG(WindowTypePopup);
		break;
	}
}

void
wmh_process_win_layer(Client * c)
{
	long *layer;
	unsigned long n;

	layer = getcard(c->win, _XA_WIN_LAYER, &n);
	if (n)
		wmh_process_layer(c, layer[0]);
	if (layer)
		XFree(layer);
}

void
wmh_process_win_state(Client *c) {
	long *state = NULL;
	unsigned long n = 0;

	state = getcard(c->win, _XA_WIN_STATE, &n);
	if (n)
		wmh_process_state_mask(c, 0xffffffff, state[0]);
	if (state)
		XFree(state);
}

void
wmh_process_win_window_state(Client *c)
{
	wmh_process_win_window_hints(c);
	wmh_process_win_layer(c);
	wmh_process_win_state(c);
}

void
ewmh_process_net_window_state(Client *c) {
	Atom *state;
	unsigned long i, n = 0;

	wmh_process_win_window_state(c);

	state = getatom(c->win, _XA_NET_WM_STATE, &n);
	for (i = 0; i < n; i++)
		ewmh_process_state_atom(c, state[i], _NET_WM_STATE_ADD);
	if (state)
		XFree(state);
}

void
ewmh_process_net_window_sync_request_counter(Client *c) {
	int format, status;
	long *data = NULL;
	unsigned long extra, nitems = 0;
	Atom real;
	

	if (!checkatom(c->win, _XA_WM_PROTOCOLS, _XA_NET_WM_SYNC_REQUEST))
		return;
	status = XGetWindowProperty(dpy, c->win, _XA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE, 0L, 1L,
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
ewmh_update_net_window_fs_monitors(Client *c) {
	Monitor *m;
	long mons[4] = { 0, };

	if (!c->ismax)
		return;
	for (m = monitors; m && c->y != m->sy; m = m->next) ;
	if (!m)
		return;
	mons[0] = m->num;
	for (m = monitors; m && c->y + c->h != m->sy + m->sh; m = m->next) ;
	if (!m)
		return;
	mons[1] = m->num;
	for (m = monitors; m && c->x != m->sx; m = m->next) ;
	if (!m)
		return;
	mons[2] = m->num;
	for (m = monitors; m && c->x + c->w != m->sx + m->sw; m = m->next) ;
	if (!m)
		return;
	mons[3] = m->num;
	XChangeProperty(dpy, c->win, _XA_NET_WM_FULLSCREEN_MONITORS, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)mons, 4L);
}

void
ewmh_process_kde_net_window_type_override(Client *c) {
	int format, status;
	long *data = NULL;
	unsigned long extra, nitems = 0;
	Atom real;

	status = XGetWindowProperty(dpy, c->win, _XA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE, 0L, 1L,
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
ewmh_update_kde_splash_progress() {
	XEvent ev;

	ev.xclient.display = dpy;
	ev.xclient.type = ClientMessage;
	ev.xclient.window = root;
	ev.xclient.message_type = _XA_KDE_SPLASH_PROGRESS;
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
		} else if (message_type == _XA_NET_CLOSE_WINDOW) {
			killclient(c);
		} else if (message_type == _XA_NET_ACTIVE_WINDOW ||
			   message_type == _XA_WIN_FOCUS) {
			c->isicon = False;
			c->ishidden = False;
			focus(c);
			arrange(clientmonitor(c));
		} else if (message_type == _XA_NET_WM_STATE) {
			ewmh_process_state_atom(c, (Atom) ev->data.l[1], ev->data.l[0]);
			if (ev->data.l[2])
				ewmh_process_state_atom(c,
				    (Atom) ev->data.l[2], ev->data.l[0]);
			ewmh_update_net_window_state(c);
		} else if (message_type == _XA_WIN_LAYER) {
			wmh_process_layer(c, ev->data.l[0]);
			ewmh_update_net_window_state(c);
		} else if (message_type == _XA_WIN_STATE) {
			wmh_process_state_mask(c, ev->data.l[0], ev->data.l[1]);
			ewmh_update_net_window_state(c);
		} else if (message_type == _XA_WM_CHANGE_STATE) {
			if (ev->data.l[0] == IconicState)
				iconify(c);
		} else if (message_type == _XA_NET_WM_DESKTOP ||
			   message_type == _XA_WIN_WORKSPACE) {
			int index = ev->data.l[0];

			if (-1 <= index && index < ntags)
				tag(c, index);
		} else if (message_type == _XA_NET_WM_DESKTOP_MASK ||
			   message_type == _XA_WIN_WORKSPACES) {
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
				ewmh_update_net_window_desktop(c);
				drawclient(c);
				arrange(NULL);
			}
			/* TODO */
		} else if (message_type == _XA_WM_PROTOCOLS) {
			/* TODO */
		} else if (message_type == _XA_NET_WM_FULLSCREEN_MONITORS) {
			if (c->ismax) {
				Monitor *mt, *mb, *ml, *mr;
				int t, b, l, r;

				mt = findmonbynum(ev->data.l[0]);
				mb = findmonbynum(ev->data.l[1]);
				ml = findmonbynum(ev->data.l[2]);
				mr = findmonbynum(ev->data.l[3]);
				if (!mt || !mb || !ml || !mr) {
					ewmh_update_net_window_fs_monitors(c);
					return;
				}
				t = mt->sy;
				b = mb->sy + mb->sh;
				l = ml->sx;
				r = mr->sx + mr->sw;
				if (t >= b || r >= l) {
					ewmh_update_net_window_fs_monitors(c);
					return;
				}
				resize(c, l, t, r - l, b - t, 0);
			}
			XChangeProperty(dpy, c->win, _XA_NET_WM_FULLSCREEN_MONITORS, XA_CARDINAL, 32,
					PropModeReplace, (unsigned char *)&ev->data.l[0], 4L);
		} else if (message_type == _XA_NET_MOVERESIZE_WINDOW) {
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
		} else if (message_type == _XA_NET_WM_MOVERESIZE) {
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
		} else if (message_type == _XA_NET_RESTACK_WINDOW) {
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
		} else if (message_type == _XA_NET_REQUEST_FRAME_EXTENTS) {
			ewmh_update_net_window_extents(c);
		}
	} else if (ev->window == root) {
		if (0) {
		} else if (message_type == _XA_NET_NUMBER_OF_DESKTOPS) {
			settags(ev->data.l[0]);
		} else if (message_type == _XA_NET_DESKTOP_GEOMETRY ||
			   message_type == _XA_WIN_AREA_COUNT) {
			/* Ignore desktop geometry changes but change the property in response */
			ewmh_update_net_desktop_geometry();
		} else if (message_type == _XA_NET_DESKTOP_VIEWPORT ||
			   message_type == _XA_WIN_AREA) {
			/* Ignore viewport changes but change the property in response. */
			ewmh_update_net_desktop_viewport();
		} else if (message_type == _XA_NET_CURRENT_DESKTOP ||
			   message_type == _XA_WIN_WORKSPACE) {
			int tag = ev->data.l[0];
			if (0 <= tag && tag < ntags)
				view(tag);
		} else if (message_type == _XA_NET_SHOWING_DESKTOP) {
			if (!ev->data.l[0] != !showing_desktop)
				toggleshowing();
		} else if (message_type == _XA_NET_RESTART) {
			/* TODO */
		} else if (message_type == _XA_NET_SHUTDOWN) {
			/* TODO */
		} else if (message_type == _XA_MANAGER) {
			/* TODO */
		} else if (message_type == _XA_NET_STARTUP_INFO_BEGIN) {
#ifdef STARTUP_NOTIFICATION
			sn_display_process_event(sn_dpy, e);
#endif
		} else if (message_type == _XA_NET_STARTUP_INFO) {
#ifdef STARTUP_NOTIFICATION
			sn_display_process_event(sn_dpy, e);
#endif
		} else if (message_type == _XA_WM_PROTOCOLS) {
			if (0) {
			} else if (ev->data.l[0] == _XA_NET_WM_PING) {
				if ((c = getclient(ev->data.l[2], ClientPing)))
					XDeleteContext(dpy, c->win, context[ClientPing]);
				if ((c = getclient(ev->data.l[2], ClientDead)))
					XDeleteContext(dpy, c->win, context[ClientDead]);
			}
		}
	} else {
		if (0) {
		} else if (message_type == _XA_NET_REQUEST_FRAME_EXTENTS) {
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
			XChangeProperty(dpy, win, _XA_NET_FRAME_EXTENTS, XA_CARDINAL, 32,
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
		XDeleteProperty(dpy, c->win, _XA_NET_WM_WINDOW_OPACITY);
		XDeleteProperty(dpy, c->frame, _XA_NET_WM_WINDOW_OPACITY);
	} else {
		long data = opacity;

		XChangeProperty(dpy, c->win, _XA_NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *) &data, 1L);
		XChangeProperty(dpy, c->frame, _XA_NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
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
	long *layer;

	state = getatom(win, _XA_NET_WM_WINDOW_TYPE, &n);
	for (i = 0; i < n; i++)
		for (j = WindowTypeDesk; j <= WindowTypeNormal; j++)
			if (state[i] == atom[j])
				ret |= WTFLAG(j);
	layer = getcard(win, _XA_WIN_LAYER, &n);
	if (n) {
		switch ((unsigned int)layer[0]) {
		case WIN_LAYER_DESKTOP:
			ret |= WTFLAG(WindowTypeDesk);
			break;
		case 1:
		case WIN_LAYER_BELOW:
		case 3:
		case WIN_LAYER_NORMAL:
		case 5:
		case WIN_LAYER_ONTOP:
			ret |= WTFLAG(WindowTypeNormal);
			break;
		case 7:
		case WIN_LAYER_DOCK:
		case 9:
		case WIN_LAYER_ABOVE_DOCK:
			ret |= WTFLAG(WindowTypeDock);
			break;
		case 11:
		case WIN_LAYER_MENU:
			ret |= WTFLAG(WindowTypeMenu);
			break;
		case 13:
		default:
			ret |= WTFLAG(WindowTypePopup);
			break;
		}
	}
	if (ret == 0)
		ret = WTFLAG(WindowTypeNormal);
	if (state)
		XFree(state);
	if (layer)
		XFree(layer);
	return ret;
}

Bool
checkwintype(Window win, int wintype) {
	Atom *state;
	unsigned long i, n = 0;
	Bool ret = False;

	state = getatom(win, _XA_NET_WM_WINDOW_TYPE, &n);
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
	return (getstrut(c, _XA_NET_WM_STRUT_PARTIAL) || getstrut(c, _XA_NET_WM_STRUT));
}

