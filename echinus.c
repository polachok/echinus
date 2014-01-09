/* See LICENSE file for copyright and license details.
 *
 * echinus window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of echinus are organized in an
 * array which is accessed whenever a new event has been fetched. This allows
 * event dispatching in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a global
 * doubly-linked client list, the focus history is remembered through a global
 * stack list. Each client contains an array of Bools of the same size as the
 * global tags array to indicate the tags of a client.	
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <regex.h>
#include <signal.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/XF86keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xft/Xft.h>
#ifdef XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>
#endif
#include "echinus.h"

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
#define CLIENTMASK	        (PropertyChangeMask | StructureNotifyMask | FocusChangeMask)
#define CLIENTNOPROPAGATEMASK 	(BUTTONMASK | ButtonMotionMask)
#define FRAMEMASK               (MOUSEMASK | SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask)


/* enums */
enum { StrutsOn, StrutsOff, StrutsHide };		    /* struts position */
enum { CurNormal, CurResize, CurMove, CurLast };	    /* cursor */
enum { Clk2Focus, SloppyFloat, AllSloppy, SloppyRaise };    /* focus model */

/* function declarations */
void applyatoms(Client * c);
void applyrules(Client * c);
void arrange(Monitor * m);
void attach(Client * c, Bool attachaside);
void attachstack(Client * c);
void ban(Client * c);
void buttonpress(XEvent * e);
void bstack(Monitor * m);
void tstack(Monitor * m);
void checkotherwm(void);
void cleanup(void);
void compileregs(void);
void configure(Client * c, Window above);
void configurenotify(XEvent * e);
void configurerequest(XEvent * e);
void destroynotify(XEvent * e);
void detach(Client * c);
void detachstack(Client * c);
void *ecalloc(size_t nmemb, size_t size);
void *emallocz(size_t size);
void *erealloc(void *ptr, size_t size);
void enternotify(XEvent * e);
void eprint(const char *errstr, ...);
void expose(XEvent * e);
void iconify(Client *c);
void incnmaster(const char *arg);
void focus(Client * c);
void focusnext(Client *c);
void focusprev(Client *c);
Client *getclient(Window w, int part);
const char *getresource(const char *resource, const char *defval);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char **text);
void getpointer(int *x, int *y);
Monitor *getmonitor(int x, int y);
Monitor *curmonitor();
Monitor *clientmonitor(Client * c);
Bool isvisible(Client * c, Monitor * m);
void initmonitors(XEvent * e);
void keypress(XEvent * e);
void killclient(Client *c);
void leavenotify(XEvent * e);
void focusin(XEvent * e);
void manage(Window w, XWindowAttributes * wa);
void mappingnotify(XEvent * e);
void monocle(Monitor * m);
void maprequest(XEvent * e);
void mousemove(Client * c);
void mouseresize(Client * c);
void moveresizekb(Client *c, int dx, int dy, int dw, int dh);
Client *nexttiled(Client * c, Monitor * m);
Client *prevtiled(Client * c, Monitor * m);
void place(Client *c);
void propertynotify(XEvent * e);
void reparentnotify(XEvent * e);
void quit(const char *arg);
void restart(const char *arg);
Bool constrain(Client *c, int *wp, int *hp);
void resize(Client * c, int x, int y, int w, int h, int b);
void restack(void);
void restack_client(Client *c, int stack_mode, Client *sibling);
void run(void);
void save(Client * c);
void save_float(Client *c);
void restore(Client *c);
void restore_float(Client *c);
void scan(void);
void setclientstate(Client * c, long state);
void setlayout(const char *arg);
void setmwfact(const char *arg);
void setup(char *);
void spawn(const char *arg);
void tag(Client *c, int index);
void rtile(Monitor * m);
void ltile(Monitor * m);
void togglestruts(const char *arg);
void togglefloating(Client *c);
void togglemax(Client *c);
void togglemaxv(Client *c);
void togglemaxh(Client *c);
void togglefill(Client *c);
void toggleshade(Client *c);
void toggletag(Client *c, int index);
void toggleview(int index);
void togglemonitor(const char *arg);
void toggleshowing(void);
void togglehidden(Client *c);
void focusview(int index);
void unban(Client * c);
void unmanage(Client * c, Bool reparented, Bool destroyed);
void updategeom(Monitor * m);
void updatestruts(void);
void unmapnotify(XEvent * e);
void updatesizehints(Client * c);
void updateframe(Client * c);
void updatetitle(Client * c);
void updategroup(Client * c, Window leader, int group);
Window *getgroup(Client *c, Window leader, int group, unsigned int *count);
void removegroup(Client *c, Window leader, int group);
void updateiconname(Client * c);
void view(int index);
void viewprevtag(const char *arg);	/* views previous selected tags */
void viewlefttag(const char *arg);
void viewrighttag(const char *arg);
int xerror(Display * dpy, XErrorEvent * ee);
int xerrordummy(Display * dsply, XErrorEvent * ee);
int xerrorstart(Display * dsply, XErrorEvent * ee);
int (*xerrorxlib) (Display *, XErrorEvent *);
void zoom(Client *c);

void addtag(void);
void appendtag(const char *arg);
void deltag(void);
void rmlasttag(const char *arg);
void settags(unsigned int numtags);

Bool isdockapp(Window win);
Bool issystray(Window win);
void delsystray(Window win);

/* variables */
int cargc;
char **cargv;
Display *dpy;
int screen;
Window root;
Window selwin;
XrmDatabase xrdb;
Bool otherwm;
Bool running = True;
Bool selscreen = True;
Monitor *monitors;
Client *clients;
Client *sel;
Client *stack;
Group window_stack = { NULL, 0 };
XContext context[PartLast];
Cursor cursor[CurLast];
Style style;
Button button[LastBtn];
View *views;
Key **keys;
Rule **rules;
char **tags;
unsigned int ntags;
unsigned int nkeys;
unsigned int nrules;
unsigned int modkey;
unsigned int numlockmask;
Bool showing_desktop = False;
Time user_time = CurrentTime;
Group systray = { NULL, 0 };
/* configuration, allows nested code to access above variables */
#include "config.h"

struct {
	Bool attachaside;
	Bool dectiled;
	Bool hidebastards;
	int focus;
	int snap;
	char command[255];
} options;

Layout layouts[] = {
	/* function	symbol	features */
	{  NULL,	'i',	OVERLAP },
	{  rtile,	't',	MWFACT | NMASTER | ZOOM },
	{  bstack,	'b',	MWFACT | NMASTER | ZOOM },
	{  tstack,	'u',	MWFACT | NMASTER | ZOOM },
	{  ltile,	'l',	MWFACT | NMASTER | ZOOM },
	{  monocle,	'm',	0 },
	{  NULL,	'f',	OVERLAP },
	{  NULL,	'\0',	0 },
};

void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = buttonpress,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[FocusIn] = focusin,
	[Expose] = expose,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[ReparentNotify] = reparentnotify,
	[UnmapNotify] = unmapnotify,
	[ClientMessage] = clientmessage,
	[SelectionClear] = selectionclear,
#ifdef XRANDR
	[RRScreenChangeNotify] = initmonitors,
#endif
};

/* function implementations */
void
applyatoms(Client * c) {
	long *t;
	unsigned long n;
	unsigned int i, tag;

	/* restore tag number from atom */
	t = getcard(c->win, atom[WindowDesk], &n);
	if (n > 0) {
		tag = *t;
		XFree(t);
		if (tag >= ntags)
			return;
		for (i = 0; i < ntags; i++)
			c->tags[i] = (i == tag) ? 1 : 0;
	}
}

void
applyrules(Client * c) {
	static char buf[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(buf, sizeof(buf), "%s:%s:%s",
	    ch.res_class ? ch.res_class : "", ch.res_name ? ch.res_name : "", c->name);
	buf[LENGTH(buf)-1] = 0;
	for (i = 0; i < nrules; i++)
		if (rules[i]->propregex && !regexec(rules[i]->propregex, buf, 1, &tmp, 0)) {
			c->isfloating = rules[i]->isfloating;
			c->title = rules[i]->hastitle;
			for (j = 0; rules[i]->tagregex && j < ntags; j++) {
				if (!regexec(rules[i]->tagregex, tags[j], 1, &tmp, 0)) {
					matched = True;
					c->tags[j] = True;
				}
			}
		}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	if (!matched)
		memcpy(c->tags, curseltags, ntags * sizeof(curseltags[0]));
}

void
arrangefloats(Monitor * m) {
	Client *c;
	Monitor *om;
	int dx, dy, w, h;

	for (c = stack; c; c = c->snext) {
		if (isvisible(c, m) && !c->isbastard &&
				(c->isfloating || MFEATURES(m, OVERLAP))
				&& !c->ismax && !(c->isicon || c->ishidden)) {
			DPRINTF("%d %d\n", c->fx, c->fy);
			if (!(om = getmonitor(c->fx + c->fw/2,
					c->fy + c->fh/2)))
				continue;
			dx = om->sx + om->sw - c->fx;
			dy = om->sy + om->sh - c->fy;
			if (dx > m->sw) 
				dx = m->sw;
			if (dy > m->sh) 
				dy = m->sh;
			w = c->fw;
			h = c->fh;
			constrain(c, &w, &h);
			resize(c, m->sx + m->sw - dx, m->sy + m->sh - dy, w, h, c->fb);
			save(c);
		}
	}
}

static void
arrangemon(Monitor * m) {
	Client *c;

	if (views[m->curtag].layout->arrange)
		views[m->curtag].layout->arrange(m);
	arrangefloats(m);
	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == m) && ((!c->isbastard && !(c->isicon || c->ishidden)) ||
			(c->isbastard && views[m->curtag].barpos == StrutsOn))) {
			unban(c);
		}
	}

	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == NULL) || (!c->isbastard && (c->isicon || c->ishidden)) ||
			(c->isbastard && views[m->curtag].barpos == StrutsHide)) {
			ban(c);
		}
	}
	for (c = stack; c; c = c->snext) {
		updateatom[WindowState] (c); /* XXX: really necessary? */
		updateatom[WindowActions](c);
	}
}

void
arrange(Monitor * m) {
	if (!m)
		for (m = monitors; m; m = m->next)
			arrangemon(m);
	else
		arrangemon(m);
	restack();
}

void
attach(Client * c, Bool attachaside) {
	if (attachaside) {
		if (clients) {
			Client * lastClient = clients;
			while (lastClient->next)
				lastClient = lastClient->next;
			c->prev = lastClient;
			lastClient->next = c;
		}
		else
			clients = c;
	}
	else {
		if (clients)
			clients->prev = c;
		c->next = clients;
		clients = c;
	}
}

void
attachstack(Client * c) {
	c->snext = stack;
	stack = c;
}

void
ban(Client * c) {
	if (c->isbanned)
		return;
	c->ignoreunmap++;
	setclientstate(c, IconicState);
	XSelectInput(dpy, c->win, CLIENTMASK & ~(StructureNotifyMask | EnterWindowMask));
	XSelectInput(dpy, c->frame, NoEventMask);
	XUnmapWindow(dpy, c->frame);
	XUnmapWindow(dpy, c->win);
	XSelectInput(dpy, c->win, CLIENTMASK);
	XSelectInput(dpy, c->frame, FRAMEMASK);
	c->isbanned = True;
}

void
buttonpress(XEvent * e) {
	Client *c;
	int i;
	XButtonPressedEvent *ev = &e->xbutton;

	if (ev->window == root) {
		if (ev->type != ButtonRelease)
			return;
		switch (ev->button) {
		case Button3:
			spawn(options.command);
			break;
		case Button4:
			viewlefttag(NULL);
			break;
		case Button5:
			viewrighttag(NULL);
			break;
		}
		return;
	}
	if ((c = getclient(ev->window, ClientTitle))) {
		DPRINTF("TITLE %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		for (i = 0; i < LastBtn; i++) {
			if (button[i].action == NULL)
				continue;
			if ((ev->x > button[i].x)
			    && ((int)ev->x < (int)(button[i].x + style.titleheight))
			    && (button[i].x != -1) && (int)ev->y < style.titleheight) {
				if (ev->type == ButtonPress) {
					DPRINTF("BUTTON %d PRESSED\n", i);
					button[i].pressed = 1;
				} else {
					DPRINTF("BUTTON %d RELEASED\n", i);
					button[i].pressed = 0;
					button[i].action(NULL);
				}
				drawclient(c);
				return;
			}
		}
		for (i = 0; i < LastBtn; i++)
			button[i].pressed = 0;
		drawclient(c);
		if (ev->type == ButtonRelease)
			return;
		restack();
		if (ev->button == Button1)
			mousemove(c);
		else if (ev->button == Button3)
			mouseresize(c);
	} else if ((c = getclient(ev->window, ClientWindow))) {
		DPRINTF("WINDOW %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		restack();
		if (CLEANMASK(ev->state) != modkey) {
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
			return;
		}
		if (ev->button == Button1) {
			if (!FEATURES(curlayout, OVERLAP) && !c->isfloating)
				togglefloating(c);
			if (c->ismax)
				togglemax(c);
			mousemove(c);
		} else if (ev->button == Button2) {
			if (!FEATURES(curlayout, OVERLAP) && c->isfloating)
				togglefloating(c);
			else
				zoom(c);
		} else if (ev->button == Button3) {
			if (!FEATURES(curlayout, OVERLAP) && !c->isfloating)
				togglefloating(c);
			if (c->ismax)
				togglemax(c);
			mouseresize(c);
		}
	} else if ((c = getclient(ev->window, ClientFrame))) {
		DPRINTF("FRAME %s: 0x%x\n", c->name, (int) ev->window);
		/* Not supposed to happen */
	}
}

static Bool selectionreleased(Display *display, XEvent *event, XPointer arg) {
	if (event->type == DestroyNotify) {
		if (event->xdestroywindow.window == (Window)arg) {
			return True;
		}
	}
	return False;
}

void
selectionclear(XEvent * e) {
	char *name = XGetAtomName(dpy, e->xselectionclear.selection);

	if (name != NULL && strncmp(name, "WM_S", 4) == 0)
		/* lost WM selection - must exit */
		quit(NULL);
	if (name)
		XFree(name);
}

void
checkotherwm(void) {
	Atom wm_sn, wm_protocols, manager;
	Window wm_sn_owner;
	XTextProperty hname;
	XClassHint class_hint;
	XClientMessageEvent manager_event;
	char name[32], hostname[64] = { 0, };
	char *names[25] = {
		"WM_COMMAND",
		"WM_HINTS",
		"WM_CLIENT_MACHINE",
		"WM_ICON_NAME",
		"WM_NAME",
		"WM_NORMAL_HINTS",
		"WM_SIZE_HINTS",
		"WM_ZOOM_HINTS",
		"WM_CLASS",
		"WM_TRANSIENT_FOR",
		"WM_CLIENT_LEADER",
		"WM_DELETE_WINDOW",
		"WM_LOCALE_NAME",
		"WM_PROTOCOLS",
		"WM_TAKE_FOCUS",
		"WM_WINDOW_ROLE",
		"WM_STATE",
		"WM_CHANGE_STATE",
		"WM_SAVE_YOURSELF",
		"SM_CLIENT_ID",
		"_MOTIF_WM_HINTS",
		"_MOTIF_WM_MESSAGES",
		"_MOTIF_WM_OFFSET",
		"_MOTIF_WM_INFO",
		"__SWM_ROOT"
	};
	Atom atoms[25] = { None, };

	snprintf(name, 32, "WM_S%d", screen);
	wm_sn = XInternAtom(dpy, name, False);
	wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	manager = XInternAtom(dpy, "MANAGER", False);
	selwin = XCreateSimpleWindow(dpy, root, DisplayWidth(dpy, screen),
			DisplayHeight(dpy, screen), 1, 1, 0, 0L, 0L);
	XGrabServer(dpy);
	wm_sn_owner = XGetSelectionOwner(dpy, wm_sn);
	if (wm_sn_owner != None) {
		XSelectInput(dpy, wm_sn_owner, StructureNotifyMask);
		XSync(dpy, False);
	}
	XUngrabServer(dpy);

	XSetSelectionOwner(dpy, wm_sn, selwin, CurrentTime);

	if (wm_sn_owner != None) {
		XEvent event_return;
		XIfEvent(dpy, &event_return, &selectionreleased,
				(XPointer) wm_sn_owner);
		wm_sn_owner = None;
	}
	gethostname(hostname, 64);
	hname.value = (unsigned char *) hostname;
	hname.encoding = XA_STRING;
	hname.format = 8;
	hname.nitems = strnlen(hostname, 64);

	class_hint.res_name = NULL;
	class_hint.res_class = "Echinus";

	Xutf8SetWMProperties(dpy, selwin, "Echinus version: " VERSION,
			"echinus " VERSION, cargv, cargc, NULL, NULL,
			&class_hint);
	XSetWMClientMachine(dpy, selwin, &hname);
	XInternAtoms(dpy, names, 25, False, atoms);
	XChangeProperty(dpy, selwin, wm_protocols, XA_ATOM, 32,
			PropModeReplace, (unsigned char *)atoms,
			sizeof(atoms)/sizeof(atoms[0]));

	manager_event.display = dpy;
	manager_event.type = ClientMessage;
	manager_event.window = root;
	manager_event.message_type = manager;
	manager_event.format = 32;
	manager_event.data.l[0] = CurrentTime; /* FIXME: timestamp */
	manager_event.data.l[1] = wm_sn;
	manager_event.data.l[2] = selwin;
	manager_event.data.l[3] = 2;
	manager_event.data.l[4] = 0;
	XSendEvent(dpy, root, False, StructureNotifyMask, (XEvent *)&manager_event);
	XSync(dpy, False);

	otherwm = False;
	XSetErrorHandler(xerrorstart);

	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XSync(dpy, False);
	if (otherwm)
		eprint("echinus: another window manager is already running\n");
	XSync(dpy, False);
	XSetErrorHandler(NULL);
	xerrorxlib = XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	while (stack) {
		unban(stack);
		unmanage(stack, False, False);
	}
	free(tags);
	free(keys);
	initmonitors(NULL);
	/* free resource database */
	XrmDestroyDatabase(xrdb);
	deinitstyle();
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XSync(dpy, False);
}

void
configure(Client * c, Window above) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h - c->th;
	ce.border_width = c->border; /* ICCCM 2.0 4.1.5 */
	ce.above = above;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *) & ce);
}

void
configurenotify(XEvent * e) {
	XConfigureEvent *ev = &e->xconfigure;
	Monitor *m;
	Client *c;

	if (ev->window == root) {
#ifdef XRANDR
		if (XRRUpdateConfiguration((XEvent *) ev)) {
#endif
			initmonitors(e);
			for (c = clients; c; c = c->next) {
				if (c->isbastard) {
					m = getmonitor(c->x + c->w/2, c->y);
					memcpy(c->tags, m->seltags, ntags * sizeof(c->tags[0]));
				}
			}
			updatestruts();
#ifdef XRANDR
		}
#endif
	}
}

void
applygravity(Client * c, int *xp, int *yp, int *wp, int *hp, int bw, int gravity) {
	int xr, yr;

	switch (gravity) {
	case UnmapGravity:
	case NorthWestGravity:
	default:
		xr = *xp;
		yr = *yp;
		break;
	case NorthGravity:
		xr = *xp + bw + *wp / 2;
		yr = *yp;
		break;
	case NorthEastGravity:
		xr = *xp + 2 * bw + *wp;
		yr = *yp;
		break;
	case WestGravity:
		xr = *xp;
		yr = *yp + bw + *hp / 2;
		break;
	case CenterGravity:
		xr = *xp + bw + *wp / 2;
		yr = *yp + bw + *hp / 2;
		break;
	case EastGravity:
		xr = *xp + 2 * bw + *wp;
		yr = *yp + bw + *hp / 2;
		break;
	case SouthWestGravity:
		xr = *xp;
		yr = *yp + 2 * bw + *hp;
		break;
	case SouthGravity:
		xr = *xp + bw + *wp / 2;
		yr = *yp + 2 * bw + *hp;
		break;
	case SouthEastGravity:
		xr = *xp + 2 * bw + *wp;
		yr = *yp + 2 * bw + *hp;
		break;
	case StaticGravity:
		xr = c->sx + c->sb;
		yr = c->sy + c->sb;
		*wp = c->sw;
		*hp = c->sh;
		break;
	}
	*hp += c->th;
	if (gravity != StaticGravity)
		constrain(c, wp, hp);
	switch (gravity) {
	case UnmapGravity:
	case NorthWestGravity:
	default:
		*xp = xr;
		*yp = yr;
		break;
	case NorthGravity:
		*xp = xr - bw - *wp / 2;
		*yp = yr;
		break;
	case NorthEastGravity:
		*xp = xr - 2 * bw - *wp;
		*yp = yr;
		break;
	case WestGravity:
		*xp = xr;
		*yp = yr - bw - *hp / 2;
		break;
	case CenterGravity:
		*xp = xr - bw - *wp / 2;
		*yp = yr - bw - *hp / 2;
		break;
	case EastGravity:
		*xp = xr - 2 * bw - *wp;
		*yp = yr - bw - *hp / 2;
		break;
	case SouthWestGravity:
		*xp = xr;
		*yp = yr - 2 * bw - *hp;
		break;
	case SouthGravity:
		*xp = xr - bw - *wp / 2;
		*yp = yr - 2 * bw - *hp;
		break;
	case SouthEastGravity:
		*xp = xr - 2 * bw - *wp;
		*yp = yr - 2 * bw - *hp;
		break;
	case StaticGravity:
		*xp = xr - bw;
		*yp = yr - bw;
		break;
	}
}

void
configurerequest(XEvent * e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = getclient(ev->window, ClientWindow))) {
		Monitor *cm;

		if (!(cm = clientmonitor(c)))
			if (!(cm = curmonitor()))
				cm = monitors;
		if (c->isfixed || c->isfloating || MFEATURES(cm, OVERLAP)) {
			int x = (ev->value_mask & CWX) ? ev->x : c->x;
			int y = (ev->value_mask & CWY) ? ev->y : c->y;
			int w = (ev->value_mask & CWWidth) ? ev->width : c->w;
			int h = (ev->value_mask & CWHeight) ? ev->height : c->h - c->th;
			int b = (ev->value_mask & CWBorderWidth) ? ev->border_width : c->border;

			applygravity(c, &x, &y, &w, &h, b, c->gravity);
			resize(c, x, y, w, h, b);
			if (ev->value_mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth))
				save(c);
		} else {
			int b = (ev->value_mask & CWBorderWidth) ? ev->border_width : c->border;

			resize(c, c->x, c->y, c->w, c->h, b);
			if (ev->value_mask & (CWBorderWidth))
				c->sb = b;
		}
		if (ev->value_mask & CWStackMode) {
			Client *s = NULL;

			if (ev->value_mask & CWSibling)
				if (!(s = getclient(ev->above, ClientAny)))
					return;
			/* might want to make this optional */
			restack_client(c, ev->detail, s);
		}
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
		XSync(dpy, False);
	}
}

void
destroynotify(XEvent * e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = getclient(ev->window, ClientWindow))) {
		DPRINTF("unmanage destroyed window (%s)\n", c->name);
		unmanage(c, False, True);
		return;
	}
	if (XFindContext(dpy, ev->window, context[SysTrayWindows],
				(XPointer *)&c) == Success) {
		XDeleteContext(dpy, ev->window, context[SysTrayWindows]);
		delsystray(ev->window);
	}
}

void
detach(Client * c) {
	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;
	if (c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
detachstack(Client * c) {
	Client **tc;

	for (tc = &stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;
}

void *
ecalloc(size_t nmemb, size_t size) {
	void *res = calloc(nmemb, size);

	if (!res)
		eprint("fatal: could not calloc() %z x %z bytes\n", nmemb, size);
	return res;
}

void *
emallocz(size_t size) {
	return ecalloc(1, size);
}

void *
erealloc(void *ptr, size_t size) {
	void *res = realloc(ptr, size);

	if (!res)
		eprint("fatal: could not realloc() %z bytes\n", size);
	return res;
}

void
enternotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;

	if (ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;
	/* FIXME: pointer could be in dead zone? */
	if (!curmonitor())
		return;
	if ((c = getclient(ev->window, ClientFrame))) {
		if (c->isbastard)
			return;
		/* focus when switching monitors */
		if (!isvisible(sel, curmonitor()))
			focus(c);
		switch (options.focus) {
		case Clk2Focus:
			break;
		case SloppyFloat:
			if (FEATURES(curlayout, OVERLAP) || c->isfloating)
				focus(c);
			break;
		case AllSloppy:
			focus(c);
			break;
		case SloppyRaise:
			focus(c);
			restack();
			break;
		}
	} else if (ev->window == root) {
		selscreen = True;
		focus(NULL);
	}
}

void
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
focusin(XEvent * e) {
	XFocusChangeEvent *ev = &e->xfocus;
	Client *c;

	c = getclient(ev->window, ClientWindow);
	if (sel && c != sel)
		XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
	else if (!c)
		fprintf(stderr, "Caught FOCUSIN for unknown window 0x%lx\n", ev->window);
}

void
expose(XEvent * e) {
	XExposeEvent *ev = &e->xexpose;
	XEvent tmp;
	Client *c;

	while (XCheckWindowEvent(dpy, ev->window, ExposureMask, &tmp));
	if ((c = getclient(ev->window, ClientTitle)))
		drawclient(c);
}

void
givefocus(Client * c) {
	XEvent ce;
	if (checkatom(c->win, atom[WMProto], atom[WMTakeFocus])) {
		ce.xclient.type = ClientMessage;
		ce.xclient.message_type = atom[WMProto];
		ce.xclient.display = dpy;
		ce.xclient.window = c->win;
		ce.xclient.format = 32;
		ce.xclient.data.l[0] = atom[WMTakeFocus];
		ce.xclient.data.l[1] = CurrentTime;	/* incorrect */
		ce.xclient.data.l[2] = 0l;
		ce.xclient.data.l[3] = 0l;
		ce.xclient.data.l[4] = 0l;
		XSendEvent(dpy, c->win, False, NoEventMask, &ce);
	}
}

void
focus(Client * c) {
	Client *o;

	o = sel;
	if ((!c && selscreen) || (c && (c->isbastard || !isvisible(c, curmonitor()))))
		for (c = stack;
		    c && (c->isbastard || (c->isicon || c->ishidden) || !isvisible(c, curmonitor())); c = c->snext);
	if (sel && sel != c) {
		XSetWindowBorder(dpy, sel->frame, style.color.norm[ColBorder]);
	}
	if (c) {
		detachstack(c);
		attachstack(c);
		/* unban(c); */
	}
	sel = c;
	if (!selscreen)
		return;
	if (c) {
		if (c->isattn)
			c->isattn = False;
		setclientstate(c, NormalState);
		if (c->isfocusable) {
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			givefocus(c);
		}
		XSetWindowBorder(dpy, sel->frame, style.color.sel[ColBorder]);
		drawclient(c);
		updateatom[WindowState](c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}
	if (o)
		drawclient(o);
	updateatom[ActiveWindow] (sel);
	updateatom[CurDesk] (NULL);
	updateatom[ELayout] (NULL);
	updateatom[ESelTags] (NULL);
}

void
focusicon(const char *arg) {
	Client *c;

	for (c = clients; c && (!c->isicon || !isvisible(c, curmonitor())); c = c->next);
	if (!c)
		return;
	if (c->isicon) {
		c->isicon = False;
		c->ishidden = False;
		updateatom[WindowState](c);
	}
	focus(c);
	arrange(curmonitor());
}

void
focusnext(Client *c) {
	if (!c)
		return;
	for (c = c->next;
	    c && (c->isbastard || (c->isicon || c->ishidden) || !isvisible(c, curmonitor())); c = c->next);
	if (!c)
		for (c = clients;
		    c && (c->isbastard || (c->isicon || c->ishidden)
			|| !isvisible(c, curmonitor())); c = c->next);
	if (c) {
		focus(c);
		restack();
	}
}

void
focusprev(Client *c) {
	if (!c)
		return;
	for (c = c->prev;
	    c && (c->isbastard || (c->isicon || c->ishidden) || !isvisible(c, curmonitor())); c = c->prev);
	if (!c) {
		for (c = clients; c && c->next; c = c->next);
		for (;
		    c && (c->isbastard || (c->isicon || c->ishidden)
			|| !isvisible(c, curmonitor())); c = c->prev);
	}
	if (c) {
		focus(c);
		restack();
	}
}

static void
_iconify(Client *c) {
	if (c == sel)
		focusnext(c);
	ban(c);
	if (!c->isicon) {
		c->isicon = True;
		updateatom[WindowState](c);
	}
	arrange(clientmonitor(c));
}

void
iconify(Client *c) {
	Client *t;
	Window *g;
	unsigned int i, m = 0;

	if (!c)
		return;
	if ((g = getgroup(c, c->win, ClientTransFor, &m)))
		for (i = 0; i < m; i++)
			if ((t = getclient(g[i], ClientWindow)) && t != c)
				_iconify(t);
	_iconify(c);
}

static void
_hide(Client *c) {
	if (c->ishidden || c->isbastard || WTCHECK(c, WindowTypeDock)
			|| WTCHECK(c, WindowTypeDesk))
		return;
	if (c == sel)
		focusnext(c);
	c->ishidden = True;
	save_float(c);
#if 0
	if (!c->isbanned)
		ban(c);
#endif
	updateatom[WindowState](c);
}

void
hide(Client *c) {
	Client *t;
	Window *g;
	unsigned int i, m = 0;

	if (!c)
		return;
	if ((g = getgroup(c, c->win, ClientTransFor, &m)))
		for (i = 0; i < m; i++)
			if ((t = getclient(g[i], ClientWindow)) && t != c)
				_hide(t);
	_hide(c);
}


static void
_show(Client *c) {
	if (!c->ishidden)
		return;
	c->ishidden = False;
	if (!c->isicon)
		unban(c);
	updateatom[WindowState](c);
}

void
show(Client *c) {
	Client *t;
	Window *g;
	unsigned int i, m = 0;

	if (!c)
		return;
	if ((g = getgroup(c, c->win, ClientTransFor, &m)))
		for (i = 0; i < m; i++)
			if ((t = getclient(g[i], ClientWindow)) && t != c)
				_show(t);
	_show(c);
}

void
togglehidden(Client *c) {
	if (c->ishidden)
		show(c);
	else
		hide(c);
	arrange(clientmonitor(c));
}

void
hideall() {
	Client *c;

	for (c = clients; c; c = c->next)
		_hide(c);
	arrange(NULL);
}

void
showall() {
	Client *c;

	for (c = clients; c; c = c->next)
		_show(c);
	arrange(NULL);
}

void
toggleshowing() {
	if ((showing_desktop = showing_desktop ? False : True))
		hideall();
	else
		showall();
	updateatom[ShowingDesktop] (NULL);
}

void
incnmaster(const char *arg) {
	unsigned int i;

	if (!FEATURES(curlayout, NMASTER))
		return;
	if (!arg) {
		views[curmontag].nmaster = DEFNMASTER;
	} else {
		i = atoi(arg);
		if ((views[curmontag].nmaster + i) < 1
		    || curwah / (views[curmontag].nmaster + i) <= 2 * style.border)
			return;
		views[curmontag].nmaster += i;
	}
	if (sel)
		arrange(curmonitor());
}

Client *
getclient(Window w, int part) {
	Client *c = NULL;

	XFindContext(dpy, w, context[part], (XPointer *) & c);
	return c;
}

long
getstate(Window w) {
	long ret = -1;
	long *p = NULL;
	unsigned long n;

	p = getcard(w, atom[WMState], &n);
	if (n != 0)
		ret = *p;
	if (p)
		XFree(p);
	return ret;
}

const char *
getresource(const char *resource, const char *defval) {
	static char name[256], class[256], *type;
	XrmValue value;

	snprintf(name, sizeof(name), "%s.%s", RESNAME, resource);
	snprintf(class, sizeof(class), "%s.%s", RESCLASS, resource);
	XrmGetResource(xrdb, name, class, &type, &value);
	if (value.addr)
		return value.addr;
	return defval;
}

Bool
gettextprop(Window w, Atom atom, char **text) {
	char **list = NULL, *str;
	int n;
	XTextProperty name;

	XGetTextProperty(dpy, w, &name, atom);
	if (!name.nitems)
		return False;
	if (name.encoding == XA_STRING) {
		if ((str = strdup((char *)name.value))) {
			free(*text);
			*text = str;
		}
	} else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
		    && n > 0 && *list) {
			if ((str = strdup(*list))) {
				free(*text);
				*text = str;
			}
			XFreeStringList(list);
		}
	}
	if (name.value)
		XFree(name.value);
	return True;
}

Bool
isvisible(Client * c, Monitor * m) {
	unsigned int i;

	if (!c)
		return False;
	if (!m) {
		for (m = monitors; m; m = m->next) {
			for (i = 0; i < ntags; i++)
				if (c->tags[i] && m->seltags[i])
					return True;
		}
	} else {
		for (i = 0; i < ntags; i++)
			if (c->tags[i] && m->seltags[i])
				return True;
	}
	return False;
}

void
grabkeys(void) {
	unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
	unsigned int i, j;
	KeyCode code;
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (i = 0; i < nkeys; i++) {
		if ((code = XKeysymToKeycode(dpy, keys[i]->keysym))) {
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(dpy, code, keys[i]->mod | modifiers[j], root,
					 True, GrabModeAsync, GrabModeAsync);
		}
    }
}

void
keypress(XEvent * e) {
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	if (!curmonitor())
		return;
	ev = &e->xkey;
	keysym = XkbKeycodeToKeysym(dpy, (KeyCode) ev->keycode, 0, 0);
	for (i = 0; i < nkeys; i++)
		if (keysym == keys[i]->keysym
		    && CLEANMASK(keys[i]->mod) == CLEANMASK(ev->state)) {
			if (keys[i]->func)
				keys[i]->func(keys[i]->arg);
			XUngrabKeyboard(dpy, CurrentTime);
		}
}

void
killclient(Client *c) {
	XEvent ev;

	if (!c)
		return;
	if (checkatom(c->win, atom[WMProto], atom[WMDelete])) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = atom[WMProto];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = atom[WMDelete];
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	} else {
		XKillClient(dpy, c->win);
	}
}

void
leavenotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->window == root) && !ev->same_screen) {
		selscreen = False;
		focus(NULL);
	}
}

void
manage(Window w, XWindowAttributes * wa) {
	Client *c, *t = NULL;
	Window trans;
	XWindowChanges wc;
	XSetWindowAttributes twa;
	XWMHints *wmh;
	unsigned long mask = 0;
	Bool focusnew = True;

	c = emallocz(sizeof(Client));
	c->win = w;
	c->ismanaged = False;
	c->name = ecalloc(1, 1);
	c->icon_name = ecalloc(1, 1);
	XSaveContext(dpy, c->win, context[ClientWindow], (XPointer)c);
	XSaveContext(dpy, c->win, context[ClientAny],    (XPointer)c);
	c->wintype = getwintype(c->win);
	if (!WTCHECK(c, WindowTypeNormal)) {
		if (WTCHECK(c, WindowTypeDesk) ||
		    WTCHECK(c, WindowTypeDock) ||
		    WTCHECK(c, WindowTypeSplash) ||
		    WTCHECK(c, WindowTypeDrop) ||
		    WTCHECK(c, WindowTypePopup) ||
		    WTCHECK(c, WindowTypeTooltip) ||
		    WTCHECK(c, WindowTypeNotify) ||
		    WTCHECK(c, WindowTypeCombo) ||
		    WTCHECK(c, WindowTypeDnd)) {
			c->isbastard = True;
			c->isfloating = True;
			c->isfixed = True;
		}
		if (WTCHECK(c, WindowTypeDialog) ||
		    WTCHECK(c, WindowTypeMenu)) {
			c->isfloating = True;
			c->isfixed = True;
		}
		if (WTCHECK(c, WindowTypeToolbar) ||
		    WTCHECK(c, WindowTypeUtil)) {
			c->isfloating = True;
		}
	}
	updateatom[WindowTypeOverride] (c);

	c->isicon = False;
	c->ishidden = False;
	c->title = c->isbastard ? None : 1;
	c->tags = ecalloc(ntags, sizeof(*c->tags));
	c->isfocusable = c->isbastard ? False : True;
	c->rb = c->fb = c->border = c->isbastard ? 0 : style.border;
	/*  XReparentWindow() unmaps *mapped* windows */
	c->ignoreunmap = wa->map_state == IsViewable ? 1 : 0;
	mwm_process_atom(c);
	updatesizehints(c);

	updatetitle(c);
	updateiconname(c);
	applyrules(c);
	applyatoms(c);

	updateatom[UserTimeWindow] (c);
	updateatom[WindowUserTime] (c);
	updateatom[NetStartupId] (c);

	if ((c->hastime) &&
	    (c->user_time == CurrentTime ||
	     (int)((int)c->user_time - (int)user_time) < 0))
		focusnew = False;

	c->th = c->title ? style.titleheight : 0;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		c->isfocusable = !(wmh->flags & InputHint) || wmh->input;
		c->isattn = (wmh->flags & XUrgencyHint) ? True : False;
		if ((wmh->flags & WindowGroupHint) &&
		    (c->leader = wmh->window_group) != None) {
			updategroup(c, c->leader, ClientGroup);
		}
		XFree(wmh);
	}

	if (c->isattn)
		focusnew = True;
	if (!c->isfocusable)
		focusnew = False;

	if (XGetTransientForHint(dpy, w, &trans)) {
		if (trans == None || trans == root)
			trans = c->leader;
		if ((c->transfor = trans) != None) {
			updategroup(c, c->transfor, ClientTransFor);
			if (!(t = getclient(trans, ClientWindow)) && trans == c->leader) {
				Window *group;
				unsigned int i, m = 0;

				if ((group = getgroup(c, c->leader, ClientGroup, &m))) {
					for (i = 0; i < m; i++) {
						trans = group[i];
						if ((t = getclient(trans, ClientWindow)))
							break;
					}
				}
			}
			if (t)
				memcpy(c->tags, t->tags, ntags * sizeof(*c->tags));
		}
		c->isfloating = True;
	}

	if (!c->isfloating)
		c->isfloating = c->isfixed;

	c->x = c->rx = c->fx = wa->x;
	c->y = c->ry = c->fy = wa->y;
	c->w = c->rw = c->fw = wa->width;
	c->h = c->rh = c->fh = wa->height + c->th;

	c->sx = wa->x;
	c->sy = wa->y;
	c->sw = wa->width;
	c->sh = wa->height;
	c->sb = wa->border_width;

	if (!wa->x && !wa->y && !c->isbastard)
		place(c);

	c->hasstruts = getstruts(c);

	if (c->isbastard) {
		Monitor *cm;

		/* TODO: refit with startup notification SCREEN= */
		if (!(cm = getmonitor(wa->x, wa->y)))
			if (!(cm = curmonitor()))
				cm = monitors;
		memcpy(c->tags, cm->seltags, ntags * sizeof(*c->tags));
	}

#if 0
	if (c->w == cm->sw && c->h == cm->sh) {
		c->x = 0;
		c->y = 0;
	} else if (!c->isbastard) {
		if (c->x + c->w > cm->wax + cm->waw)
			c->x = cm->waw - c->w;
		if (c->y + c->h > cm->way + cm->wah)
			c->y = cm->wah - c->h;
		if (c->x < cm->wax)
			c->x = cm->wax;
		if (c->y < cm->way)
			c->y = cm->way;
	}
#endif

	XGrabButton(dpy, AnyButton, AnyModifier, c->win, True,
			ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
	twa.override_redirect = True;
	twa.event_mask = FRAMEMASK;
	mask = CWOverrideRedirect | CWEventMask;
	if (wa->depth == 32) {
		mask |= CWColormap | CWBorderPixel | CWBackPixel;
		twa.colormap = XCreateColormap(dpy, root, wa->visual, AllocNone);
		twa.background_pixel = BlackPixel(dpy, screen);
		twa.border_pixel = BlackPixel(dpy, screen);
	}
	c->frame =
	    XCreateWindow(dpy, root, c->x, c->y, c->w,
	    c->h, c->border, wa->depth == 32 ? 32 : DefaultDepth(dpy, screen),
	    InputOutput, wa->depth == 32 ? wa->visual : DefaultVisual(dpy,
		screen), mask, &twa);
	XSaveContext(dpy, c->frame, context[ClientFrame], (XPointer)c);
	XSaveContext(dpy, c->frame, context[ClientAny],   (XPointer)c);

	wc.border_width = c->border;
	XConfigureWindow(dpy, c->frame, CWBorderWidth, &wc);
	configure(c, None);
	XSetWindowBorder(dpy, c->frame, style.color.norm[ColBorder]);

	twa.event_mask = ExposureMask | MOUSEMASK;
	/* we create title as root's child as a workaround for 32bit visuals */
	if (c->title) {
		c->title = XCreateWindow(dpy, root, 0, 0, c->w, c->th,
		    0, DefaultDepth(dpy, screen), CopyFromParent,
		    DefaultVisual(dpy, screen), CWEventMask, &twa);
		c->drawable =
		    XCreatePixmap(dpy, root, c->w, c->th, DefaultDepth(dpy, screen));
		c->xftdraw =
		    XftDrawCreate(dpy, c->drawable, DefaultVisual(dpy, screen),
		    DefaultColormap(dpy, screen));
		XSaveContext(dpy, c->title, context[ClientTitle], (XPointer) c);
		XSaveContext(dpy, c->title, context[ClientAny], (XPointer) c);
	} else {
		c->title = None;
	}

	attach(c, options.attachaside);
	updateatom[ClientList] (NULL);
	attachstack(c);

	twa.event_mask = CLIENTMASK;
	twa.do_not_propagate_mask = CLIENTNOPROPAGATEMASK;
	XChangeWindowAttributes(dpy, c->win, CWEventMask|CWDontPropagate, &twa);
	XSelectInput(dpy, c->win, CLIENTMASK);

	XReparentWindow(dpy, c->win, c->frame, 0, c->th);
	XReparentWindow(dpy, c->title, c->frame, 0, 0);
	XAddToSaveSet(dpy, c->win);
	XMapWindow(dpy, c->win);
	wc.border_width = 0;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	ban(c);
	updateatom[WindowDesk] (c);
	updateatom[WindowDeskMask] (c);
	ewmh_process_net_window_state(c);
	c->ismanaged = True;
	updateatom[WindowState](c);
	updateatom[WindowActions](c);
	updateframe(c);

	if (c->hasstruts) {
		updateatom[WorkArea] (NULL);
		updategeom(NULL);
	}
	arrange(NULL);
	if (!WTCHECK(c, WindowTypeDesk) && focusnew)
		focus(NULL);
}

void
mappingnotify(XEvent * e) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent * e) {
	static XWindowAttributes wa;
	Client *c;
	XMapRequestEvent *ev = &e->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (wa.override_redirect)
		return;
	if (issystray(ev->window))
		return;
	if (isdockapp(ev->window))
		return;
	if (!(c = getclient(ev->window, ClientWindow)))
		manage(ev->window, &wa);
}

void
getworkarea(Monitor * m, int *wx, int *wy, int *ww, int *wh) {
	*wx = max(m->wax, 1);
	*wy = max(m->way, 1);
	*ww = min(m->wax + m->waw, DisplayWidth(dpy, screen) - 1) - *wx;
	*wh = min(m->way + m->wah, DisplayHeight(dpy, screen) - 1) - *wy;
}

void
monocle(Monitor * m) {
	Client *c;
	int wx, wy, ww, wh;

	getworkarea(m, &wx, &wy, &ww, &wh);
	for (c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		resize(c, wx, wy, ww - 2 * c->border, wh - 2 * c->border, c->border);
}

void
moveresizekb(Client *c, int dx, int dy, int dw, int dh) {
	int wasmax, wasmaxv, wasmaxh, wasfill, wasshade;

	if (!c || !c->isfloating || !(dx || dy || dw || dh))
		return;
	if ((wasmax = c->ismax)) {
		c->ismax = False;
		updateframe(c);
	}
	if ((wasmaxv = c->ismaxv))
		c->ismaxv = False;
	if ((wasmaxh = c->ismaxh))
		c->ismaxh = False;
	if ((wasfill = c->isfill))
		c->isfill = False;
	if ((wasshade = c->isshade)) {
		c->isshade = False;
		resize(c, c->x, c->y, c->w, c->h, c->border);
	}
	if (dw || dh) {
		int w, h;

		if (dw && (dw < c->incw))
			dw = (dw / abs(dw)) * c->incw;
		if (dh && (dh < c->inch))
			dh = (dh / abs(dh)) * c->inch;
		w = c->w + dw;
		h = c->h + dh;
		constrain(c, &w, &h);
		resize(c, c->x + dx, c->y + dy, w, h, c->border);
		save(c);

	} else {
		if (wasmax || wasmaxv || wasmaxh || wasfill)
			restore(c);

		resize(c, c->x + dx, c->y + dy, c->w, c->h, c->border);

		if (wasmax)
			togglemax(c);
		if (wasmaxv)
			togglemaxv(c);
		if (wasmaxh)
			togglemaxh(c);
		if (wasfill)
			togglefill(c);
	}
	if (wasshade)
		toggleshade(c);
	updateatom[WindowState] (c);
}

void
getpointer(int *x, int *y) {
	int di;
	unsigned int dui;
	Window dummy;

	XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

Monitor *
getmonitor(int x, int y) {
	Monitor *m;

	for (m = monitors; m; m = m->next) {
		if ((x >= m->sx && x <= m->sx + m->sw) &&
		    (y >= m->sy && y <= m->sy + m->sh))
			return m;
	}
	return NULL;
}

Monitor *
clientmonitor(Client * c) {
	Monitor *m;

	assert(c != NULL);
	for (m = monitors; m; m = m->next)
		if (isvisible(c, m))
			return m;
	return NULL;
}

Monitor *
curmonitor() {
	int x, y;
	getpointer(&x, &y);
	return getmonitor(x, y);
}

void
mousemove(Client * c) {
	int x1, y1, ocx, ocy, nx, ny;
	int wasmax, wasmaxv, wasmaxh, wasfill, wasshade;
	unsigned int i;
	XEvent ev;
	Monitor *m, *nm;

	if (c->isbastard)
		return;
	m = curmonitor();
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	getpointer(&x1, &y1);
	if ((wasmax = c->ismax)) {
		c->ismax = False;
		updateframe(c);
	}
	if ((wasmaxv = c->ismaxv))
		c->ismaxv = False;
	if ((wasmaxh = c->ismaxh))
		c->ismaxh = False;
	if ((wasfill = c->isfill))
		c->isfill = False;
	if ((wasshade = c->isshade)) {
		c->isshade = False;
		resize(c, c->x, c->y, c->w, c->h, c->border);
	}
	if (wasmax || wasmaxv || wasmaxh || wasfill)
		restore(c);
	for (;;) {
		int wx, wy, ww, wh;

		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			break;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type] (&ev);
			continue;
		case MotionNotify:
			XSync(dpy, False);
			/* we are probably moving to a different monitor */
			if (!(nm = curmonitor()))
				continue;
			getworkarea(m, &wx, &wy, &ww, &wh);
			nx = ocx + (ev.xmotion.x_root - x1);
			ny = ocy + (ev.xmotion.y_root - y1);
			if (abs(nx - wx) < options.snap)
				nx = wx;
			else if (abs((wx + ww) - (nx + c->w +
				    2 * c->border)) < options.snap)
				nx = wx + ww - c->w - 2 * c->border;
			if (abs(ny - wy) < options.snap)
				ny = wy;
			else if (abs((wy + wh) - (ny + c->h +
				    2 * c->border)) < options.snap)
				ny = wy + wh - c->h - 2 * c->border;
			resize(c, nx, ny, c->w, c->h, c->border);
			save(c);
			if (m != nm) {
				for (i = 0; i < ntags; i++)
					c->tags[i] = nm->seltags[i];
				updateatom[WindowDesk] (c);
				updateatom[WindowDeskMask] (c);
				updateatom[WindowState] (c);
				drawclient(c);
				arrange(NULL);
				m = nm;
			}
			continue;
		default:
			continue;
		}
		break;
	}
	if (wasmax)
		togglemax(c);
	if (wasmaxv)
		togglemaxv(c);
	if (wasmaxh)
		togglemaxh(c);
	if (wasfill)
		togglefill(c);
	if (wasshade)
		toggleshade(c);
	updateatom[WindowState] (c);
}

void
mouseresize(Client * c) {
	int ocx, ocy, nw, nh;
	int wasmax, wasmaxv, wasmaxh, wasshade, wasfill;
	/* Monitor *cm; */
	XEvent ev;

	if (c->isbastard || c->isfixed)
		return;
	/* cm = curmonitor(); */

	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	if ((wasmax = c->ismax)) {
		c->ismax = False;
		updateframe(c);
	}
	if ((wasmaxv = c->ismaxv))
		c->ismaxv = False;
	if ((wasmaxh = c->ismaxh))
		c->ismaxh = False;
	if ((wasfill = c->isfill))
		c->isfill = False;
	if ((wasshade = c->isshade)) {
		c->isshade = False;
		resize(c, c->x, c->y, c->w, c->h, c->border);
	}
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1,
	    c->h + c->border - 1);
	for (;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
			    c->w + c->border - 1, c->h + c->border - 1);
			XUngrabPointer(dpy, CurrentTime);
			while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
			break;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type] (&ev);
			continue;
		case MotionNotify:
			XSync(dpy, False);
			if ((nw = ev.xmotion.x - ocx - 2 * c->border + 1) <= 0)
				nw = MINWIDTH;
			if ((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
				nh = MINHEIGHT;
			constrain(c, &nw, &nh);
			resize(c, c->x, c->y, nw, nh, c->border);
			save(c);
			continue;
		default:
			continue;
		}
		break;
	}
	if (wasshade)
		toggleshade(c);
	updateatom[WindowState] (c);
}

Client *
nexttiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard
		|| (c->isicon || c->ishidden)); c = c->next);
	return c;
}

Client *
prevtiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard
		|| (c->isicon || c->ishidden)); c = c->prev);
	return c;
}

void
reparentnotify(XEvent * e) {
	Client *c;
	XReparentEvent *ev = &e->xreparent;

	if ((c = getclient(ev->window, ClientWindow)))
		if (ev->parent != c->frame) {
			DPRINTF("unmanage reparented window (%s)\n", c->name);
			unmanage(c, True, False);
		}
}

void
place(Client *c) {
	int x, y;
	Monitor *m;
	int d = style.titleheight;
	int wx, wy, ww, wh;


	/* XXX: do something better */
	getpointer(&x, &y);
	DPRINTF("%d %d\n", x, y);
	m = getmonitor(x, y);
	getworkarea(m, &wx, &wy, &ww, &wh);
	x = x + rand()%d - c->w/2;
	y = y + rand()%d - c->h/2;
	if (x < wx)
		x = wx;
	DPRINTF("%d %d\n", x, y);
	if (y < wy)
		y = wy;
	DPRINTF("%d+%d > %d+%d\n", x, c->w, wx, ww);
	if (x + c->w > wx + ww)
		x = wx + ww - c->w - rand()%d;
	DPRINTF("%d %d\n", x, y);
	if (y + c->h > wy + wh)
		y = wy + wh - c->h - rand()%d;
	DPRINTF("%d %d\n", x, y);

	c->rx = c->fx = c->x = x;
	c->ry = c->fy = c->y = y;
}

void
propertynotify(XEvent * e) {
	Client *c;
	Window trans = None;
	XPropertyEvent *ev = &e->xproperty;

	if ((c = getclient(ev->window, ClientWindow))) {
		if (ev->atom == atom[StrutPartial] || ev->atom == atom[Strut]) {
			c->hasstruts = getstruts(c);
			updatestruts();
		}
		if (ev->state == PropertyDelete) 
			return;
		if (ev->atom <= XA_LAST_PREDEFINED) {
			switch (ev->atom) {
			case XA_WM_TRANSIENT_FOR:
				if (XGetTransientForHint(dpy, c->win, &trans) &&
				    trans == None)
					trans = root;
				if (!c->isfloating &&
				    (c->isfloating = (trans != None))) {
					arrange(NULL);
					updateatom[WindowState] (c);
					updateatom[WindowActions] (c);
				}
				return;
			case XA_WM_NORMAL_HINTS:
				updatesizehints(c);
				return;
			case XA_WM_NAME:
				updatetitle(c);
				drawclient(c);
				return;
			case XA_WM_ICON_NAME:
				updateiconname(c);
				return;
			}
		} else {
			if (0) {
			} else if (ev->atom == atom[WindowName]) {
				updatetitle(c);
				drawclient(c);
			} else if (ev->atom == atom[WindowIconName]) {
				updateiconname(c);
			} else if (ev->atom == atom[WindowType]) {
				/* TODO */
			} else if (ev->atom == atom[WindowUserTime]) {
				updateatom[WindowUserTime] (c);
			} else if (ev->atom == atom[UserTimeWindow]) {
				updateatom[UserTimeWindow] (c);
				updateatom[WindowUserTime] (c);
			} else if (ev->atom == atom[WindowCounter]) {
				/* TODO */
			}
		}
	} else if ((c = getclient(ev->window, ClientTimeWindow))) {
		if (ev->atom > XA_LAST_PREDEFINED) {
			if (0) {
			} else if (ev->atom == atom[WindowUserTime]) {
				updateatom[WindowUserTime] (c);
			} else if (ev->atom == atom[UserTimeWindow]) {
				updateatom[UserTimeWindow] (c);
				updateatom[WindowUserTime] (c);
			}
		}
	} else if (ev->window == root) {
		if (ev->atom > XA_LAST_PREDEFINED) {
			if (0) {
			} else if (ev->atom == atom[DeskNames]) {
				ewmh_process_net_desktop_names();
				updateatom[DeskNames] (NULL);
			} else if (ev->atom == atom[DeskLayout]) {
				/* TODO */
			}
		}
	}
}

void
quit(const char *arg) {
	running = False;
	if (arg) {
		cleanup();
		execvp(cargv[0], cargv);
		eprint("Can't exec: %s\n", strerror(errno));
	}
}

Bool
constrain(Client * c, int *wp, int *hp) {
	int w = *wp, h = *hp;
	Bool ret = False;

	/* remove decoration */
	h -= c->th;

	/* set minimum possible */
	if (w < 1)
		w = 1;
	if (h < 1)
		h = 1;

	/* temporarily remove base dimensions */
	w -= c->basew;
	h -= c->baseh;

	/* adjust for aspect limits */
	if (c->minay > 0 && c->maxay > 0 && c->minax > 0 && c->maxax > 0) {
		if (w * c->maxay > h * c->maxax)
			w = h * c->maxax / c->maxay;
		else if (w * c->minay < h * c->minax)
			h = w * c->minay / c->minax;
	}

	/* adjust for increment value */
	if (c->incw)
		w -= w % c->incw;
	if (c->inch)
		h -= h % c->inch;

	/* restore base dimensions */
	w += c->basew;
	h += c->baseh;

	if (c->minw > 0 && w < c->minw)
		w = c->minw;
	if (c->minh > 0 && h < c->minh)
		h = c->minh;
	if (c->maxw > 0 && w > c->maxw)
		w = c->maxw;
	if (c->maxh > 0 && h > c->maxh)
		h = c->maxh;

	/* restore decoration */
	h += c->th;

	if (w <= 0 || h <= 0)
		return ret;
	if (w != *wp) {
		*wp = w;
		ret = True;
	}
	if (h != *hp) {
		*hp = h;
		ret = True;
	}
	return ret;
}

void
resize(Client * c, int x, int y, int w, int h, int b) {
	XWindowChanges wc;
	unsigned mask;

	if (w <= 0 || h <= 0)
		return;
	/* offscreen appearance fixes */
	if (x > DisplayWidth(dpy, screen))
		x = DisplayWidth(dpy, screen) - w - 2 * b;
	if (y > DisplayHeight(dpy, screen))
		y = DisplayHeight(dpy, screen) - h - 2 * b;
	if (w != c->w && c->th) {
		XMoveResizeWindow(dpy, c->title, 0, 0, w, c->th);
		XFreePixmap(dpy, c->drawable);
		c->drawable = XCreatePixmap(dpy, root, w, c->th, DefaultDepth(dpy, screen));
		drawclient(c);
	}
	DPRINTF("x = %d y = %d w = %d h = %d b = %d\n", x, y, w, h, b);
	mask = 0;
	if (c->x != x) {
		c->x = x;
		wc.x = x;
		mask |= CWX;
	}
	if (c->y != y) {
		c->y = y;
		wc.y = y;
		mask |= CWY;
	}
	if (c->w != w) {
		c->w = w;
		wc.width = w;
		mask |= CWWidth;
	}
	if (c->h != h) {
		c->h = h;
		wc.height = h;
		mask |= CWHeight;
	}
	if (c->border != b) {
		c->border = b;
		wc.border_width = b;
		mask |= CWBorderWidth;
		updateatom[WindowExtents] (c);
	}
	if (mask)
		XConfigureWindow(dpy, c->frame, mask, &wc);
	/* ICCCM 2.0 4.1.5 */
	XMoveResizeWindow(dpy, c->win, 0, c->th, w, h - c->th);
	if (!(mask & (CWWidth | CWHeight))) configure(c, None);
	XSync(dpy, False);
}

static Bool
wind_overlap(int min1, int max1, int min2, int max2) {
	int tmp;

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

static Bool
client_overlap(Client *c, Client *o)
{
	if (wind_overlap(c->x, c->x + c->w, o->x, o->x + o->w) &&
	    wind_overlap(c->y, c->y + c->h, o->y, o->y + o->h))
		return True;
	return False;
}

static Bool
client_occludes(Client *c, Client *o)
{
	Client *s;

	if (!client_overlap(c, o))
		return False;
	for (s = c->snext; s && s != o; s = s->snext) ;
	return (s ? True : False);
}

static Bool
client_occludes_any(Client *c)
{
	Client *s;

	for (s = c->snext; s; s = s->snext)
		if (client_overlap(c, s))
			return True;
	return False;
}

static Bool
client_occluded_any(Client *c)
{
	Client *s;

	for (s = stack; s && s != c; s = s->next)
		if (client_overlap(c, s))
			return True;
	return False;
}

void
restack_client(Client *c, int stack_mode, Client *o)
{
	Client *s, **cp, **op, **lp;

	for (lp = &stack, s = stack; s; lp = &s->snext, s = *lp) ;
	for (cp = &stack, s = stack; s && s != c; cp = &s->snext, s = *cp) ;
	assert(s == c);
	for (op = &stack, s = stack; s && s != o; op = &s->snext, s = *op) ;
	assert(s == o);

	switch (stack_mode) {
	case Above:
		if (o) {
			/* just above sibling */
			*cp = c->snext;
			*op = c;
			c->snext = o;
		} else {
			/* top of stack */
			*cp = c->snext;
			c->snext = stack;
			stack = c;
		}
		break;
	case Below:
		if (o) {
			/* just below sibling */
			*cp = c->snext;
			c->snext = o->snext;
			o->snext = c;
		} else {
			/* bottom of stack */
			*cp = c->snext;
			c->snext = *lp;
			*lp = c;
		}
		break;
	case TopIf:
		if (o) {
			if (!client_occludes(o, c))
				return;
		} else {
			if (!client_occluded_any(c))
				return;
		}
		/* top of stack */
		*cp = c->snext;
		c->snext = stack;
		stack = c;
		break;
	case BottomIf:
		if (o) {
			if (!client_occludes(c, o))
				return;
		} else {
			if (!client_occludes_any(c))
				return;
		}
		/* bottom of stack */
		*cp = c->snext;
		c->snext = *lp;
		*lp = c;
		break;
	case Opposite:
		if (o) {
			if (client_occludes(o, c)) {
				/* top of stack */
				*cp = c->snext;
				c->snext = stack;
				stack = c;
			} else if (client_occludes(c, o)) {
				/* bottom of stack */
				*cp = c->snext;
				c->snext = *lp;
				*lp = c;
			} else
				return;
		} else {
			if (client_occluded_any(c)) {
				/* top of stack */
				*cp = c->snext;
				c->snext = stack;
				stack = c;
			} else if (client_occludes_any(c)) {
				/* bottom of stack */
				*cp = c->snext;
				c->snext = *lp;
				*lp = c;
			} else
				return;
		}
		break;
	default:
		return;
	}
	restack();
}

void
restack()
{
	Client *c, **ol, **cl, **sl;
	XEvent ev;
	Window *wl;
	int i, j, n;

	if (!sel)
		return;
	for (n = 0, c = stack; c; c = c->snext, n++) ;
	if (!n) {
		updateatom[ClientListStacking] (NULL);
		return;
	}
	wl = ecalloc(n, sizeof(*wl));
	ol = ecalloc(n, sizeof(*ol));
	cl = ecalloc(n, sizeof(*cl));
	sl = ecalloc(n, sizeof(*sl));
	/* 
	 * EWMH WM SPEC 1.5 Draft 1:
	 *
	 * Stacking order
	 *
	 * To obtain good interoperability betweeen different Desktop
	 * Environments, the following layerd stacking order is
	 * recommended, from the bottom:
	 *
	 * - windows of type _NET_WM_TYPE_DESKTOP
	 * - windows having state _NET_WM_STATE_BELOW
	 * - windows not belonging in any other layer
	 * - windows of type _NET_WM_TYPE_DOCK (unless they have state
	 *   _NET_WM_TYPE_BELOW) and windows having state
	 *   _NET_WM_STATE_ABOVE
	 * - focused windows having state _NET_WM_STATE_FULLSCREEN
	 */
	for (i = 0, j = 0, c = stack; c; ol[i] = cl[i] = c, i++, c = c->snext) ;
	/* focused windows having state _NET_WM_STATE_FULLSCREEN */
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (sel == c && c->ismax) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	/* windows of type _NET_WM_TYPE_DOCK (unless they have state _NET_WM_STATE_BELOW) and
	   windows having state _NET_WM_STATE_ABOVE. */
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if ((WTCHECK(c, WindowTypeDock) && !c->isbelow) || c->isabove) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	/* windows not belonging in any other layer (but we put floating above special above tiled) 
	 */
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (!c->isbastard && c->isfloating && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (c->isbastard && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (!c->isbastard && !c->isfloating && !c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	/* windows having state _NET_WM_STATE_BELOW */
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (c->isbelow && !WTCHECK(c, WindowTypeDesk)) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	/* windows of type _NET_WM_TYPE_DESKTOP */
	for (i = 0; i < n; i++) {
		if (!(c = cl[i]))
			continue;
		if (WTCHECK(c, WindowTypeDesk)) {
			cl[i] = NULL; wl[j] = c->frame; sl[j] = c; j++;
		}
	}
	assert(j == n);
	free(cl);

	if (bcmp(ol, sl, n * sizeof(*ol))) {
		stack = sl[0];
		for (i = 0; i < n - 1; i++)
			sl[i]->snext = sl[i + 1];
		sl[i]->snext = NULL;
	}
	free(ol);
	free(sl);

	if (!window_stack.members || (window_stack.count != n) ||
			bcmp(window_stack.members, wl, n * sizeof(*wl))) {
		free(window_stack.members);
		window_stack.members = wl;
		window_stack.count = n;

		XRestackWindows(dpy, wl, n);
		XFlush(dpy);

		updateatom[ClientListStacking] (NULL);

		XSync(dpy, False);
		while (XCheckMaskEvent(dpy, EnterWindowMask, &ev)) ;
	} else
		free(wl);
}

void
run(void) {
	fd_set rd;
	int xfd;
	XEvent ev;

	/* main event loop */
	XSync(dpy, False);
	xfd = ConnectionNumber(dpy);
	while (running) {
		FD_ZERO(&rd);
		FD_SET(xfd, &rd);
		if (select(xfd + 1, &rd, NULL, NULL, NULL) == -1) {
			if (errno == EINTR)
				continue;
			eprint("select failed\n");
		}
		while (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (handler[ev.type])
				(handler[ev.type]) (&ev);	/* call handler */
		}
	}
}

void
save(Client *c) {
	memcpy(&c->rx, &c->x, 5 * sizeof(int));
}

void
save_float(Client *c) {
	memcpy(&c->fx, &c->x, 5 * sizeof(int));
}

void
restore(Client *c) {
	int w, h;

	w = c->rw;
	h = c->rh;
	constrain(c, &w, &h);
	resize(c, c->rx, c->ry, w, h, c->rb);
}

void
restore_float(Client *c) {
	resize(c, c->fx, c->fy, c->fw, c->fh, c->fb);
}

void
delsystray(Window win) {
	unsigned int i, j;

	for (i = 0, j = 0; i < systray.count; i++) {
		if (systray.members[i] == win)
			continue;
		systray.members[i] = systray.members[j++];
	}
	if (j < systray.count) {
		systray.count = j;
		XChangeProperty(dpy, root, atom[SystemTrayWindows], XA_WINDOW, 32,
				PropModeReplace, (unsigned char *) systray.members,
				systray.count);
	}
}

void
setwmstate(Window win, long state) {
	long data[] = { state, None };
	XEvent ev;

	ev.xclient.display = dpy;
	ev.xclient.type = ClientMessage;
	ev.xclient.window = win;
	ev.xclient.message_type = atom[WindowChangeState];
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = state;
	ev.xclient.data.l[1] = ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = ev.xclient.data.l[4] = 0;

	XSendEvent(dpy, root, False,
		SubstructureRedirectMask | SubstructureNotifyMask, &ev);

	XChangeProperty(dpy, win, atom[WMState], atom[WMState], 32,
	    PropModeReplace, (unsigned char *) data, 2);
}

Bool
isdockapp(Window win) {
	XWMHints *wmh;
	Bool ret;

	if ((ret = ((wmh = XGetWMHints(dpy, win)) &&
		(wmh->flags & StateHint) && (wmh->initial_state == WithdrawnState))))
		setwmstate(win, WithdrawnState);
	return ret;
}

Bool
issystray(Window win) {
	int format, status;
	long *data = NULL;
	unsigned long extra, nitems = 0;
	Atom real;
	Bool ret;
	unsigned int i;

	status =
	    XGetWindowProperty(dpy, win, atom[WindowForSysTray], 0L, 1L, False,
			       AnyPropertyType, &real, &format, &nitems, &extra,
			       (unsigned char **) &data);
	if ((ret = (status == Success && real != None))) {
		for (i = 0; i < systray.count && systray.members[i] != win; i++) ;
		if (i == systray.count) {
			XSelectInput(dpy, win, StructureNotifyMask);
			XSaveContext(dpy, win, context[SysTrayWindows], (XPointer)&systray);

			systray.members =
			    erealloc(systray.members, (i + 1) * sizeof(Window));
			systray.members[i] = win;
			systray.count++;
			XChangeProperty(dpy, root, atom[SysTrayWindows], XA_WINDOW,
					32, PropModeReplace,
					(unsigned char *) systray.members,
					systray.count);
		}
	} else
		delsystray(win);
	if (data)
		XFree(data);
	return ret;
}

void
scan(void) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;
	XWMHints *wmh;

	wins = NULL;
	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa) ||
			    wa.override_redirect || issystray(wins[i] ||
			    isdockapp(wins[i]))
			    || XGetTransientForHint(dpy, wins[i], &d1) ||
			    ((wmh = XGetWMHints(dpy, wins[i])) &&
			     (wmh->flags & WindowGroupHint) &&
			     (wmh->window_group != wins[i])))
				continue;
			if (wa.map_state == IsViewable
			    || getstate(wins[i]) == IconicState
			    || getstate(wins[i]) == NormalState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) {
			/* now the transients and group members */
			if (!XGetWindowAttributes(dpy, wins[i], &wa) ||
			    wa.override_redirect || issystray(wins[i]) ||
			    isdockapp(wins[i]))
				continue;
			if ((XGetTransientForHint(dpy, wins[i], &d1) ||
			     ((wmh = XGetWMHints(dpy, wins[i])) &&
			      (wmh->flags & WindowGroupHint) &&
			      (wmh->window_group != wins[i])))
			    && (wa.map_state == IsViewable
				|| getstate(wins[i]) == IconicState
				|| getstate(wins[i]) == NormalState))
				manage(wins[i], &wa);
		}
	}
	if (wins)
		XFree(wins);
}

void
setclientstate(Client * c, long state) {

	setwmstate(c->win, state);

	if (state == NormalState && (c->isicon || c->ishidden)) {
		c->isicon = False;
		c->ishidden = False;
		updateatom[WindowState](c);
		updateatom[WindowActions](c);
	}
}

void
setlayout(const char *arg) {
	unsigned int i;
	Client *c;
	Bool wasfloat, nowfloat;

	wasfloat = FEATURES(curlayout, OVERLAP);

	if (arg) {
		for (i = 0; i < LENGTH(layouts); i++)
			if (*arg == layouts[i].symbol)
				break;
		if (i == LENGTH(layouts))
			return;
		views[curmontag].layout = &layouts[i];
	}

	nowfloat = FEATURES(curlayout, OVERLAP);

	if (sel) {
		for (c = clients; c; c = c->next) {
			if (isvisible(c, curmonitor())) {
				if (wasfloat)
					save_float(c);
				if (wasfloat != nowfloat)
					updateframe(c);
#if 0
				/* why not this? */
				if (nowfloat)
					restore_float(c);
				/* arrange floats below does it? */
#endif
			}
		}
		arrange(curmonitor());
	}
	updateatom[ELayout] (NULL);
	updateatom[DeskModes] (NULL);
}

void
setmwfact(const char *arg) {
	double delta;

	if (!FEATURES(curlayout, MWFACT))
		return;
	/* arg handling, manipulate mwfact */
	if (arg == NULL)
		views[curmontag].mwfact = DEFMWFACT;
	else if (sscanf(arg, "%lf", &delta) == 1) {
		if (arg[0] == '+' || arg[0] == '-')
			views[curmontag].mwfact += delta;
		else
			views[curmontag].mwfact = delta;
		if (views[curmontag].mwfact < 0.1)
			views[curmontag].mwfact = 0.1;
		else if (views[curmontag].mwfact > 0.9)
			views[curmontag].mwfact = 0.9;
	}
	arrange(curmonitor());
}

void
initview(unsigned int i, float mwfact, int nmaster, const char *deflayout) {
	unsigned int j;
	char conf[32], ltname;

	views[i].layout = &layouts[0];
	snprintf(conf, sizeof(conf), "tags.layout%d", i);
	strncpy(&ltname, getresource(conf, deflayout), 1);
	for (j = 0; j < LENGTH(layouts); j++) {
		if (layouts[j].symbol == ltname) {
			views[i].layout = &layouts[j];
			break;
		}
	}
	views[i].mwfact = mwfact;
	views[i].nmaster = nmaster;
	views[i].barpos = StrutsOn;
}

void
newview(unsigned int i) {
	float mwfact;
	int nmaster;
	const char *deflayout;

	mwfact = atof(getresource("mwfact", STR(DEFMWFACT)));
	nmaster = atoi(getresource("nmaster", STR(DEFNMASTER)));
	deflayout = getresource("deflayout", "i");
	if (!nmaster)
		nmaster = 1;
	initview(i, mwfact, nmaster, deflayout);
}

void
initlayouts() {
	unsigned int i;
	float mwfact;
	int nmaster;
	const char *deflayout;

	/* init layouts */
	mwfact = atof(getresource("mwfact", STR(DEFMWFACT)));
	nmaster = atoi(getresource("nmaster", STR(DEFNMASTER)));
	deflayout = getresource("deflayout", "i");
	if (!nmaster)
		nmaster = 1;
	for (i = 0; i < ntags; i++)
		initview(i, mwfact, nmaster, deflayout);
	updateatom[ELayout] (NULL);
	updateatom[DeskModes] (NULL);
}

void
initmonitors(XEvent * e) {
	Monitor *m;
#ifdef XRANDR
	Monitor *t;
	XRRCrtcInfo *ci;
	XRRScreenResources *sr;
	int c, n;
	int ncrtc = 0;
	int dummy1, dummy2, major, minor;

	/* free */
	if (monitors) {
		m = monitors;
		do {
			t = m->next;
			free(m->seltags);
			free(m->prevtags);
			free(m);
			m = t;
		} while (m);
		monitors = NULL;
	}
	if (!running)
	    return;
	/* initial Xrandr setup */
	if (XRRQueryExtension(dpy, &dummy1, &dummy2))
		if (XRRQueryVersion(dpy, &major, &minor) && major < 1)
			goto no_xrandr;

	/* map virtual screens onto physical screens */
	sr = XRRGetScreenResources(dpy, root);
	if (sr == NULL)
		goto no_xrandr;
	else
		ncrtc = sr->ncrtc;

	for (c = 0, n = 0, ci = NULL; c < ncrtc; c++) {
		ci = XRRGetCrtcInfo(dpy, sr, sr->crtcs[c]);
		if (ci->noutput == 0)
			continue;

		if (ci != NULL && ci->mode == None)
			fprintf(stderr, "???\n");
		else {
			/* If monitor is a mirror, we don't care about it */
			if (n && ci->x == monitors->sx && ci->y == monitors->sy)
				continue;
			m = emallocz(sizeof(Monitor));
			m->sx = m->wax = ci->x;
			m->sy = m->way = ci->y;
			m->sw = m->waw = ci->width;
			m->sh = m->wah = ci->height;
			m->mx = m->sx + m->sw/2;
			m->my = m->sy + m->sh/2;
			m->curtag = n;
			m->prevtags = ecalloc(ntags, sizeof(Bool));
			m->seltags = ecalloc(ntags, sizeof(Bool));
			m->seltags[n] = True;
			m->next = monitors;
			monitors = m;
			n++;
		}
		DPRINTF("There are %d monitors.\n", n);
		XRRFreeCrtcInfo(ci);
	}
	XRRFreeScreenResources(sr);
	updateatom[DeskGeometry] (NULL);
	return;
      no_xrandr:
#endif
	m = emallocz(sizeof(Monitor));
	m->sx = m->wax = 0;
	m->sy = m->way = 0;
	m->sw = m->waw = DisplayWidth(dpy, screen);
	m->sh = m->wah = DisplayHeight(dpy, screen);
	m->mx = m->sx + m->sw/2;
	m->my = m->sy + m->sh/2;
	m->curtag = 0;
	m->prevtags = ecalloc(ntags, sizeof(Bool));
	m->seltags = ecalloc(ntags, sizeof(Bool));
	m->seltags[0] = True;
	m->next = NULL;
	monitors = m;
	updateatom[DeskGeometry] (NULL);
}

void
inittag(unsigned int i) {
	char tmp[25] = "", def[8] = "";

	tags[i] = emallocz(sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "tags.name%d", i);
	snprintf(def, sizeof(def), "%u", i);
	snprintf(tags[i], sizeof(tmp), "%s", getresource(tmp, def));
	DPRINTF("Assigned name '%s' to tag %u\n", tags[i], i);
}

void
newtag(unsigned int i) {
	inittag(i);
}

void
inittags() {
	ntags = ewmh_process_net_number_of_desktops();
	views = ecalloc(ntags, sizeof(View));
	tags = ecalloc(ntags, sizeof(char *));
	ewmh_process_net_desktop_names();
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

void
deltag() {
	Client *c;
	unsigned int i, last;

	if (ntags < 2)
		return;
	last = ntags - 1;

	/* move off the desktop being deleted */
	if (curmontag == last)
		view(last - 1);

	/* move windows off the desktop being deleted */
	for (c = clients; c; c = c->next) {
		if (isomni(c)) {
			c->tags[last] = False;
			continue;
		}
		for (i = 0; i < last; i++)
			if (c->tags[i])
				break;
		if (i == last)
			tag(c, last - 1);
		else if (c->tags[last])
			c->tags[last] = False;
	}

	free(tags[last]);
	tags[last] = NULL;

	--ntags;
#if 0
	/* caller's responsibility */
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskModes] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[ESelTags] (NULL);
#endif

}

void
rmlasttag(const char *arg) {
	deltag();
	ewmh_process_net_desktop_names();
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskModes] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[ESelTags] (NULL);
}

void
addtag() {
	Client *c;
	Monitor *m;
	unsigned int i, n;

	if (ntags >= 64)
		return; /* stop the insanity, go organic */

	n = ntags + 1;
	views = erealloc(views, n * sizeof(views[ntags]));
	newview(ntags);
	tags = erealloc(tags, n * sizeof(tags[ntags]));
	newtag(ntags);

	for (c = clients; c; c = c->next) {
		c->tags = erealloc(c->tags, n * sizeof(c->tags[ntags]));
		for (i = 0; i < ntags && c->tags[i]; i++);
		c->tags[ntags] = (i == ntags || c->issticky) ? True : False;
	}

	for (m = monitors; m; m = m->next) {
		m->prevtags = erealloc(m->prevtags, n * sizeof(m->prevtags[0]));
		m->prevtags[ntags] = False;
		m->seltags  = erealloc(m->seltags,  n * sizeof(m->seltags [0]));
		m->seltags [ntags] = False;
	}

	ntags++;
#if 0
	/* caller's responsibility */
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskModes] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[ESelTags] (NULL);
	updateatom[DeskNames] (NULL);
#endif
}

void
appendtag(const char *arg) {
	addtag();
	ewmh_process_net_desktop_names();
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskModes] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[ESelTags] (NULL);
	updateatom[DeskNames] (NULL);
}

void
settags(unsigned int numtags) {
	if (1 > numtags || numtags > 64)
		return;
	while (ntags < numtags) { addtag(); }
	while (ntags > numtags) { deltag(); }
	ewmh_process_net_desktop_names();
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskModes] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[ESelTags] (NULL);
	updateatom[DeskNames] (NULL);
}

void
sighandler(int signum) {
	if (signum == SIGHUP)
		quit("HUP!");
	else
		quit(NULL);
}

void
setup(char *conf) {
	int d;
	int i, j;
	unsigned int mask;
	Window w;
	Monitor *m;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;
	char oldcwd[256], path[256] = "/";
	char *home, *slash;
	/* configuration files to open (%s gets converted to $HOME) */
	const char *confs[] = {
		conf,
		"%s/.echinus/echinusrc",
		SYSCONFPATH "/echinusrc",
		NULL
	};

	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_bottom_right_corner);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	/* init modifier map */
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++) {
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
			    == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	XFreeModifiermap(modmap);

	/* select for events */
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
	    | EnterWindowMask | LeaveWindowMask | StructureNotifyMask |
	    ButtonPressMask | ButtonReleaseMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);

	/* init resource database */
	XrmInitialize();

	home = getenv("HOME");
	if (!home)
		*home = '/';
	if (!getcwd(oldcwd, sizeof(oldcwd)))
		eprint("echinus: getcwd error: %s\n", strerror(errno));

	for (i = 0; confs[i] != NULL; i++) {
		if (*confs[i] == '\0')
			continue;
		snprintf(conf, 255, confs[i], home);
		/* retrieve path to chdir(2) to it */
		slash = strrchr(conf, '/');
		if (slash)
			snprintf(path, slash - conf + 1, "%s", conf);
		if (chdir(path) != 0)
			fprintf(stderr, "echinus: cannot change directory\n");
		xrdb = XrmGetFileDatabase(conf);
		/* configuration file loaded successfully; break out */
		if (xrdb)
			break;
	}
	if (!xrdb)
		fprintf(stderr, "echinus: no configuration file found, using defaults\n");

	/* init EWMH atom */
	initewmh(selwin);

	updateatom[ClientList] (NULL);
	updateatom[ClientListStacking] (NULL);

	/* init tags */
	inittags();
	/* init geometry */
	initmonitors(NULL);

	/* init modkey */
	initrules();
	initkeys();
	initlayouts();
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskViewport] (NULL);
	updateatom[DeskNames] (NULL);
	updateatom[CurDesk] (NULL);
	updateatom[ELayout] (NULL);
	updateatom[ESelTags] (NULL);
	updateatom[VirtualRoots] (NULL);

	grabkeys();

	/* init appearance */
	initstyle();
	options.attachaside = atoi(getresource("attachaside", "1"));
	strncpy(options.command, getresource("command", COMMAND), LENGTH(options.command));
	options.command[LENGTH(options.command) - 1] = '\0';
	options.dectiled = atoi(getresource("decoratetiled", STR(DECORATETILED)));
	options.hidebastards = atoi(getresource("hidebastards", "0"));
	options.focus = atoi(getresource("sloppy", "0"));
	options.snap = atoi(getresource("snap", STR(SNAP)));

	for (m = monitors; m; m = m->next) {
		m->struts[RightStrut] = m->struts[LeftStrut] =
		    m->struts[TopStrut] = m->struts[BotStrut] = 0;
		updategeom(m);
	}
	updateatom[WorkArea] (NULL);
	ewmh_process_net_showing_desktop();
	updateatom[ShowingDesktop] (NULL);

	if (chdir(oldcwd) != 0)
		fprintf(stderr, "echinus: cannot change directory\n");

	/* multihead support */
	selscreen = XQueryPointer(dpy, root, &w, &w, &d, &d, &d, &d, &mask);
}

void
spawn(const char *arg) {
	static char shell[] = "/bin/sh";

	if (!arg)
		return;
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if (fork() == 0) {
		if (fork() == 0) {
			if (dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", arg, (char *) NULL);
			fprintf(stderr, "echinus: execl '%s -c %s'", shell, arg);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}

static void
_tag(Client *c, int index) {
	unsigned int i;

	for (i = 0; i < ntags; i++)
		c->tags[i] = (index == -1);
	i = (index == -1) ? 0 : index;
	c->tags[i] = True;
	updateatom[WindowDesk] (c);
	updateatom[WindowDeskMask] (c);
	updateatom[WindowState] (c);
	updateframe(c);
	arrange(NULL);
	focus(NULL);
}

void
tag(Client *c, int index) {
	Client *t;
	Window *g;
	unsigned int i, m = 0;

	if (!c)
		return;
	if ((g = getgroup(c, c->win, ClientTransFor, &m)))
		for (i = 0; i < m; i++)
			if ((t = getclient(g[i], ClientWindow)) && t != c)
				_tag(t, index);
	_tag(c, index);
}

void
bstack(Monitor * m) {
	int nx, ny, nw, nh, nb;
	int mx, my, mw, mh, mb, mn;
	int tx, ty, tw, th, tb, tn;
	unsigned int i, n;
	Client *c, *mc;
	int wx, wy, ww, wh;

	getworkarea(m, &wx, &wy, &ww, &wh);

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	/* window geoms */

	/* master & tile number */
	mn = (n > views[m->curtag].nmaster) ? views[m->curtag].nmaster : n;
	tn = (mn < n) ? n - mn : 0;
	/* master & tile width */
	mw = (mn > 0) ? ww / mn : ww;
	tw = (tn > 0) ? ww / tn : 0;
	/* master & tile height */
	mh = (tn > 0) ? views[m->curtag].mwfact * wh : wh;
	th = (tn > 0) ? wh - mh : 0;
	if (tn > 0 && th < style.titleheight)
		th = wh;

	/* top left corner of master area */
	mx = wx;
	my = wy;

	/* top left corner of tiled area */
	tx = wx;
	ty = wy + mh;

	mb = wh;
	nb = 0;

	for (i = 0, c = mc = nexttiled(clients, m); c && i < n; c = nexttiled(c->next, m), i++) {
		if (c->ismax) {
			c->ismax = False;
			updateatom[WindowState] (c);
		}
		nb = min(nb, c->border);
		if (i < mn) {
			/* master */
			nx = mx - nb;
			ny = my;
			nw = mw + nb;
			nh = mh;
			if (i == (mn - 1)) {
				nw = ww - nx + wx;
				nb = 0;
			} else
				nb = c->border;
			mx = nx + nw;
			mb = min(mb, c->border);
		} else {
			/* tile */
			tb = min(mb, c->border);
			nx = tx - nb;
			ny = ty - tb;
			nw = tw + nb;
			nh = th + tb;
			if (i == (n - 1)) {
				nw = ww - nx + wx;
				nb = 0;
			} else
				nb = c->border;
			tx = nx + nw;
		}
	      nw -= 2 * c->border;
	      nh -= 2 * c->border;
	      resize(c, nx, ny, nw, nh, c->border);
	}
}

void
tstack(Monitor * m) {
	int nx, ny, nw, nh, nb;
	int mx, my, mw, mh, mb, mn;
	int tx, ty, tw, th, tb, tn;
	unsigned int i, n;
	Client *c, *mc;
	int wx, wy, ww, wh;

	getworkarea(m, &wx, &wy, &ww, &wh);

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	/* window geoms */

	/* master & tile number */
	mn = (n > views[m->curtag].nmaster) ? views[m->curtag].nmaster : n;
	tn = (mn < n) ? n - mn : 0;
	/* master & tile width */
	mw = (mn > 0) ? ww / mn : ww;
	tw = (tn > 0) ? ww / tn : 0;
	/* master & tile height */
	mh = (tn > 0) ? views[m->curtag].mwfact * wh : wh;
	th = (tn > 0) ? wh - mh : 0;
	if (tn > 0 && th < style.titleheight)
		th = wh;

	/* top left corner of master area */
	mx = wx;
	my = wy + wh - mh;

	/* top left corner of tiled area */
	tx = wx;
	ty = wy;

	mb = wh;
	nb = 0;

	for (i = 0, c = mc = nexttiled(clients, m); c && i < n; c = nexttiled(c->next, m), i++) {
		if (c->ismax) {
			c->ismax = False;
			updateatom[WindowState] (c);
		}
		nb = min(nb, c->border);
		if (i < mn) {
			/* master */
			nx = mx - nb;
			ny = my;
			nw = mw + nb;
			nh = mh;
			if (i == (mn - 1)) {
				nw = ww - nx + wx;
				nb = 0;
			} else
				nb = c->border;
			mx = nx + nw;
			mb = min(mb, c->border);
		} else {
			/* tile */
			tb = min(mb, c->border);
			nx = tx - nb;
			ny = ty;
			nw = tw + nb;
			nh = th + tb;
			if (i == (n - 1)) {
				nw = ww - nx + wx;
				nb = 0;
			} else
				nb = c->border;
			tx = nx + nw;
		}
	      nw -= 2 * c->border;
	      nh -= 2 * c->border;
	      resize(c, nx, ny, nw, nh, c->border);
	}
}

/* tiles to right, master to left, variable number of masters */

void
rtile(Monitor * m) {
	int nx, ny, nw, nh, nb;
	int mx, my, mw, mh, mb, mn;
	int tx, ty, tw, th, tb, tn;
	unsigned int i, n;
	Client *c, *mc;
	int wx, wy, ww, wh;

	getworkarea(m, &wx, &wy, &ww, &wh);

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	/* window geoms */

	/* master & tile number */
	mn = (n > views[m->curtag].nmaster) ? views[m->curtag].nmaster : n;
	tn = (mn < n) ? n - mn : 0;
	/* master & tile height */
	mh = (mn > 0) ? wh / mn : wh;
	th = (tn > 0) ? wh / tn : 0;
	if (tn > 0 && th < style.titleheight)
		th = wh;
	/* master & tile width */
	mw = (tn > 0) ? views[m->curtag].mwfact * ww : ww;
	tw = (tn > 0) ? ww - mw : 0;

	/* top left corner of master area */
	mx = wx;
	my = wy;

	/* top left corner of tiled area */
	tx = wx + ww - tw;
	ty = wy;

	mb = ww;
	nb = 0;

	for (i = 0, c = mc = nexttiled(clients, m); c && i < n; c = nexttiled(c->next, m), i++) {
		if (c->ismax) {
			c->ismax = False;
			updateatom[WindowState] (c);
		}
		nb = min(nb, c->border);
		if (i < mn) {
			/* master */
			nx = mx;
			ny = my - nb;
			nw = mw;
			nh = mh + nb;
			if (i == (mn - 1)) {
				nh = wh - ny + wy;
				nb = 0;
			} else
				nb = c->border;
			my = ny + nh;
			mb = min(mb, c->border);
		} else {
			/* tile window */
			tb = min(mb, c->border);
			nx = tx - tb;
			ny = ty - nb;
			nw = tw + tb;
			nh = th + nb;
			if (i == (n - 1)) {
				nh = wh - ny + wy;
				nb = 0;
			} else
				nb = c->border;
			ty = ny + nh;
		}
		nw -= 2 * c->border;
		nh -= 2 * c->border;
		resize(c, nx, ny, nw, nh, c->border);
	}
}

/* tiles to left, master to right, variable number of masters */

void
ltile(Monitor * m) {
	int nx, ny, nw, nh, nb;
	int mx, my, mw, mh, mb, mn;
	int tx, ty, tw, th, tb, tn;
	unsigned int i, n;
	Client *c, *mc;
	int wx, wy, ww, wh;

	getworkarea(m, &wx, &wy, &ww, &wh);

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	/* window geoms */

	/* master & tile number */
	mn = (n > views[m->curtag].nmaster) ? views[m->curtag].nmaster : n;
	tn = (mn < n) ? n - mn : 0;
	/* master & tile height */
	mh = (mn > 0) ? wh / mn : wh;
	th = (tn > 0) ? wh / tn : 0;
	if (tn > 0 && th < style.titleheight)
		th = wh;
	/* master & tile width */
	mw = (tn > 0) ? views[m->curtag].mwfact * ww : ww;
	tw = (tn > 0) ? ww - mw : 0;

	/* top left corner of master area */
	mx = wx + ww - mw;
	my = wy;

	/* top left corner of tiled area */
	tx = wx;
	ty = wy;

	mb = ww;
	nb = 0;

	for (i = 0, c = mc = nexttiled(clients, m); c && i < n; c = nexttiled(c->next, m), i++) {
		if (c->ismax) {
			c->ismax = False;
			updateatom[WindowState] (c);
		}
		nb = min(nb, c->border);
		if (i < mn) {
			/* master */
			nx = mx;
			ny = my - nb;
			nw = mw;
			nh = mh + nb;
			if (i == (mn - 1)) {
				nh = wh - ny + wy;
				nb = 0;
			} else
				nb = c->border;
			my = ny + nh;
			mb = min(mb, c->border);
		} else {
			/* tile window */
			tb = min(mb, c->border);
			nx = tx;
			ny = ty - nb;
			nw = tw + tb;
			nh = th + nb;
			if (i == (n - 1)) {
				nh = wh - ny + wy;
				nb = 0;
			} else
				nb = c->border;
			ty = ny + nh;
		}
		nw -= 2 * c->border;
		nh -= 2 * c->border;
		resize(c, nx, ny, nw, nh, c->border);
	}
}

void
togglestruts(const char *arg) {
	views[curmontag].barpos = (views[curmontag].barpos == StrutsOn)
	    ? (options.hidebastards ? StrutsHide : StrutsOff) : StrutsOn;
	updategeom(curmonitor());
	arrange(curmonitor());
}

void
togglefloating(Client *c) {
	if (!c)
		return;

	if (FEATURES(curlayout, OVERLAP))
		return;

	c->isfloating = !c->isfloating;
	updateframe(c);
	if (c->isfloating) {
		/* restore last known float dimensions */
		restore_float(c);
	} else {
		/* save last known float dimensions */
		save_float(c);
	}
	arrange(curmonitor());
	updateatom[WindowState](c);
	updateatom[WindowActions](c);
}

Monitor *
canresize(Client *c) {
	Monitor *m;

	if (!c)
		return NULL;
	if (!(m = clientmonitor(c)))
		if (!(m = curmonitor()))
			m = monitors;
	if (c->isfixed || !(c->isfloating || MFEATURES(m, OVERLAP)))
		return NULL;
	return m;
}


void
togglefill(Client * c) {
	XEvent ev;
	Monitor *m;
	Client *o;

	if (!(m = canresize(c)))
		return;

	if ((c->isfill = !c->isfill)) {
		int x1, x2, y1, y2, w, h, b;
		int wx, wy, ww, wh;

		getworkarea(m, &wx, &wy, &ww, &wh);

		x1 = wx;
		x2 = wx + ww;
		y1 = wy;
		y2 = wy + wh;

		for (o = clients; o; o = o->next) {
			if (!isvisible(o, m) || (o == c) || o->isbastard
			    || !(o->isfloating || MFEATURES(m, OVERLAP)))
				continue;
			if (o->y + o->h > c->y && o->y < c->y + c->h) {
				if (o->x < c->x)
					x1 = max(x1, o->x + o->w + style.border);
				else
					x2 = min(x2, o->x - style.border);
			}
			if (o->x + o->w > c->x && o->x < c->x + c->w) {
				if (o->y < c->y)
					y1 = max(y1, o->y + o->h + style.border);
				else
					y2 = max(y2, o->y - style.border);
			}
			DPRINTF("x1 = %d x2 = %d y1 = %d y2 = %d\n", x1, x2, y1, y2);
		}
		w = x2 - x1;
		h = y2 - y1;
		DPRINTF("x1 = %d w = %d y1 = %d h = %d\n", x1, w, y1, h);
		if ((w < c->w) || (h < c->h)) {
			c->isfill = False;
			return;
		}
		if (!c->ismax && !c->ismaxv && !c->ismaxh)
			save(c);
		b = c->border;
		if (c->ismax) {
			c->ismax = False;
			updateframe(c);
			b = c->rb;
		}
		c->ismax = c->ismaxv = c->ismaxh = False;
		constrain(c, &w, &h);
		resize(c, x1, y1, w, h, b);
	} else
		restore(c);
	updateatom[WindowState] (c);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev)) ;	/* XXX */
}

void
togglemax(Client * c) {
	XEvent ev;
	Monitor *m;

	if (!(m = canresize(c)))
		return;

	c->ismax = !c->ismax;
	updateframe(c);
	if (c->ismax) {
		int wx, wy, ww, wh;

		if (!c->ismaxh && !c->ismaxv && !c->isfill)
			save(c);
		c->isfill  = c->ismaxh = c->ismaxv = False;
		getworkarea(m, &wx, &wy, &ww, &wh);
		resize(c, wx, wy, ww, wh, 0);
	} else
		restore(c);
	updateatom[WindowState] (c);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev)) ; /* XXX */
}

void
togglemaxv(Client * c) {
	Monitor *m;

	if (!(m = canresize(c)))
		return;

	if ((c->ismaxv = !c->ismaxv)) {
		/* maximize vertical */
		int wx, wy, ww, wh;
		int x, y, w, h, b;

		getworkarea(m, &wx, &wy, &ww, &wh);
		b = c->ismax ? c->rb : c->border;
		x = (c->isfill || c->ismax) ? c->rx : c->x;
		y = wy;
		w = (c->isfill || c->ismax) ? c->rw : c->w;
		h = wh - 2 * b;

		if (!c->ismaxh && !c->isfill && !c->ismax)
			save(c);
		if (c->ismax) {
			c->ismax = False;
			updateframe(c);
		}
		c->isfill = False;
		constrain(c, &w, &h);
		resize(c, x, y, w, h, b);
	} else {
		if (c->ismaxh)
			/* demaximized vertical, leave maximize horizontal */
			resize(c, c->x, c->ry, c->w, c->rh, c->border);
		else
			restore(c);
	}
	updateatom[WindowState] (c);
}

void
toggleshade(Client * c) {
	Monitor *m;
	XWindowChanges wc;

	if (!c->title || !(m = canresize(c)))
		return;
	if (c->ismax)
		togglemax(c);
	if ((c->isshade = !c->isshade)) {
		wc.width = c->w;
		wc.height = c->th;
		wc.border_width = c->border;
	} else {
		wc.width = c->w;
		wc.height = c->h;
		wc.border_width = c->border;
	}
	XConfigureWindow(dpy, c->frame, CWWidth | CWHeight | CWBorderWidth, &wc);
	XSync(dpy, False);
	updateatom[WindowState] (c);
}

void
togglemaxh(Client *c) {
	Monitor *m;

	if (!(m = canresize(c)))
		return;

	if ((c->ismaxh = !c->ismaxh)) {
		/* maximize horizontal */
		int wx, wy, ww, wh;
		int x, y, w, h, b;

		getworkarea(m, &wx, &wy, &ww, &wh);
		b = c->ismax ? c->rb : c->border;
		x = wx;
		y = (c->isfill || c->ismax) ? c->ry : c->y;
		w = ww - 2 * b;
		h = (c->isfill || c->ismax) ? c->rh : c->h;

		if (!c->ismaxv && !c->isfill && !c->ismax)
			save(c);
		if (c->ismax) {
			c->ismax = False;
			updateframe(c);
		}
		c->isfill = False;
		constrain(c, &w, &h);
		resize(c, x, y, w, h, b);
	} else {
		if (c->ismaxv)
			/* demaximize horizontal, leave maximize vertical */
			resize(c, c->rx, c->y, c->rw, c->h, c->border);
		else
			/* demaximize */
			restore(c);
	}
	updateatom[WindowState] (c);
}

void
toggletag(Client *c, int index) {
	unsigned int i, j;

	if (!c)
		return;
	i = (index == -1) ? 0 : index;
	c->tags[i] = !c->tags[i];
	for (j = 0; j < ntags && !c->tags[j]; j++);
	if (j == ntags)
		c->tags[i] = True;	/* at least one tag must be enabled */
	updateatom[WindowDesk] (c);
	updateatom[WindowDeskMask] (c);
	updateatom[WindowState] (c);
	drawclient(c);
	arrange(NULL);
}

void
togglemonitor(const char *arg) {
	Monitor *m, *cm;
	int x, y;

	getpointer(&x, &y);
	if (!(cm = getmonitor(x, y)))
		return;
	cm->mx = x;
	cm->my = y;
	for (m = monitors; m == cm && m && m->next; m = m->next);
	if (!m)
		return;
	XWarpPointer(dpy, None, root, 0, 0, 0, 0, m->mx, m->my);
	focus(NULL);
}

void
toggleview(int index) {
	unsigned int i, j;
	Monitor *m, *cm;

	i = (index == -1) ? 0 : index;
	cm = curmonitor();

	memcpy(cm->prevtags, cm->seltags, ntags * sizeof(cm->seltags[0]));
	cm->seltags[i] = !cm->seltags[i];
	for (m = monitors; m; m = m->next) {
		if (m->seltags[i] && m != cm) {
			memcpy(m->prevtags, m->seltags, ntags * sizeof(m->seltags[0]));
			m->seltags[i] = False;
			for (j = 0; j < ntags && !m->seltags[j]; j++);
			if (j == ntags) {
				m->seltags[i] = True;	/* at least one tag must be viewed */
				cm->seltags[i] = False; /* can't toggle */
				j = i;
			}
			if (m->curtag == i)
				m->curtag = j;
			arrange(m);
		}
	}
	arrange(cm);
	focus(NULL);
	updateatom[CurDesk] (NULL);
	updateatom[ELayout] (NULL);
	updateatom[ESelTags] (NULL);
}

void
focusview(int index) {
	unsigned int i;
	Client *c;

	i = (index == -1) ? 0 : index;
	toggleview(i);
	if (!curseltags[i])
		return;
	for (c = stack; c; c = c->snext) {
		if (c->tags[i] && !c->isbastard) {
			focus(c);
			break;
		}
	}
	restack();
}

void
unban(Client * c) {
	if (!c->isbanned)
		return;
	XSelectInput(dpy, c->win, CLIENTMASK & ~(StructureNotifyMask | EnterWindowMask));
	XSelectInput(dpy, c->frame, NoEventMask);
	XMapWindow(dpy, c->win);
	XMapWindow(dpy, c->frame);
	XSelectInput(dpy, c->win, CLIENTMASK);
	XSelectInput(dpy, c->frame, FRAMEMASK);
	setclientstate(c, NormalState);
	c->isbanned = False;
}

void
unmanage(Client * c, Bool reparented, Bool destroyed) {
	XWindowChanges wc;
	Bool doarrange, dostruts;
	Window trans = None;

	doarrange = !(c->isfloating || c->isfixed ||
		(!destroyed && XGetTransientForHint(dpy, c->win, &trans))) ||
		c->isbastard;
	dostruts = c->hasstruts;
	/* The server grab construct avoids race conditions. */
	XGrabServer(dpy);
	XSelectInput(dpy, c->frame, NoEventMask);
	XUnmapWindow(dpy, c->frame);
	XSetErrorHandler(xerrordummy);
	if (c->title) {
		XftDrawDestroy(c->xftdraw);
		XFreePixmap(dpy, c->drawable);
		XDestroyWindow(dpy, c->title);
		XDeleteContext(dpy, c->title, context[ClientTitle]);
		XDeleteContext(dpy, c->title, context[ClientAny]);
		c->title = None;
	}
	if (!destroyed) {
		XSelectInput(dpy, c->win, CLIENTMASK & ~(StructureNotifyMask | EnterWindowMask));
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!reparented) {
			if (c->gravity == StaticGravity) {
				/* restore static geometry */
				wc.x = c->sx;
				wc.y = c->sy;
				wc.width = c->sw;
				wc.height = c->sh;
			} else {
				/* restore geometry */
				wc.x = c->rx;
				wc.y = c->ry;
				wc.width = c->rw;
				wc.height = c->rh;
			}
			wc.border_width = c->sb;
			XReparentWindow(dpy, c->win, root, wc.x, wc.y);
			XMoveWindow(dpy, c->win, wc.x, wc.y);
			XConfigureWindow(dpy, c->win,
				(CWX|CWY|CWWidth|CWHeight|CWBorderWidth), &wc);
			if (!running)
				XMapWindow(dpy, c->win);
		}
	}
	detach(c);
	updateatom[ClientList] (NULL);
	detachstack(c);
	if (sel == c)
		focus(NULL);
	if (!destroyed) {
		setclientstate(c, WithdrawnState);
		/* sm-proxy thinks it needs to be restarted unless removed */
		XDeleteProperty(dpy, c->win, atom[WMState]);
	}
	XDestroyWindow(dpy, c->frame);
	XDeleteContext(dpy, c->frame, context[ClientFrame]);
	XDeleteContext(dpy, c->frame, context[ClientAny]);
	XDeleteContext(dpy, c->win, context[ClientWindow]);
	XDeleteContext(dpy, c->win, context[ClientAny]);
	ewmh_release_user_time_window(c);
	removegroup(c, c->leader, ClientGroup);
	removegroup(c, c->transfor, ClientTransFor);
	free(c->name);
	free(c->icon_name);
	free(c->startup_id);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	if (dostruts)
		updatestruts();
	else if (doarrange) 
		arrange(NULL);
}

static void
updategeommon(Monitor * m) {
	m->wax = m->sx;
	m->way = m->sy;
	m->waw = m->sw;
	m->wah = m->sh;
	switch (views[m->curtag].barpos) {
	default:
		m->wax += m->struts[LeftStrut];
		m->way += m->struts[TopStrut];
		m->waw -= m->struts[LeftStrut] + m->struts[RightStrut];
		m->wah -= m->struts[TopStrut] + m->struts[BotStrut];
		break;
	case StrutsHide:
	case StrutsOff:
		break;
	}
}

void
updategeom(Monitor *m) {
	if (!m)
		for (m = monitors; m; m = m->next)
			updategeommon(m);
	else
		updategeommon(m);
}

void
updatestruts() {
	Client *c;
	Monitor *m;

	for (m = monitors; m; m = m->next)
		m->struts[RightStrut]
			= m->struts[LeftStrut]
			= m->struts[TopStrut]
			= m->struts[BotStrut] = 0;
	for (c = clients; c; c = c->next)
		if (c->hasstruts)
			getstruts(c);
	updateatom[WorkArea] (NULL);
	updategeom(NULL);
	arrange(NULL);
}

void
unmapnotify(XEvent * e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = getclient(ev->window, ClientWindow)) /* && ev->send_event */) {
		if (c->ignoreunmap--)
			return;
		DPRINTF("unmanage self-unmapped window (%s)\n", c->name);
		unmanage(c, False, False);
	}
}

void
updateframe(Client * c) {
	int i, f = 0;

	if (!c->title)
		return;

	for (i = 0; i < ntags; i++)
		if (c->tags[i])
			f += FEATURES(views[i].layout, OVERLAP);

	c->th = !c->ismax && (c->isfloating || options.dectiled || f) ?
				style.titleheight : 0;
	if (!c->th)
		XUnmapWindow(dpy, c->title);
	else
		XMapRaised(dpy, c->title);
	updateatom[WindowExtents](c);
}

void
updatesizehints(Client * c) {
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if (c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (c->flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (c->flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (c->flags & PAspect) {
		c->minax = size.min_aspect.x;
		c->maxax = size.max_aspect.x;
		c->minay = size.min_aspect.y;
		c->maxay = size.max_aspect.y;
	} else
		c->minax = c->maxax = c->minay = c->maxay = 0;
	if (c->flags & PWinGravity)
		c->gravity = size.win_gravity;
	else
		c->gravity = NorthWestGravity;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
	    && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client * c) {
	if (!gettextprop(c->win, atom[WindowName], &c->name))
		gettextprop(c->win, XA_WM_NAME, &c->name);
	updateatom[WindowNameVisible] (c);
}

void
updateiconname(Client *c) {
	if (!gettextprop(c->win, atom[WindowIconName], &c->icon_name))
		gettextprop(c->win, XA_WM_ICON_NAME, &c->icon_name);
	updateatom[WindowIconNameVisible] (c);
}

void
updategroup(Client *c, Window leader, int group) {
	Group *g = NULL;

	if (leader == None || leader == c->win)
		return;
	XFindContext(dpy, leader, context[group], (XPointer *)&g);
	if (!g) {
		g = emallocz(sizeof(*g));
		g->members = ecalloc(8, sizeof(g->members[0]));
		g->count = 0;
		XSaveContext(dpy, leader, context[group], (XPointer)g);
	} else
		g->members = erealloc(g->members, (g->count + 1) * sizeof(g->members[0]));
	g->members[g->count] = c->win;
	g->count++;
}

Window *
getgroup(Client *c, Window leader, int group, unsigned int *count) {
	Group *g = NULL;

	if (leader == None) {
		*count = 0;
		return NULL;
	}
	XFindContext(dpy, leader, context[group], (XPointer *)&g);
	if (!g) {
		*count = 0;
		return NULL;
	}
	*count = g->count;
	return g->members;
}

void
removegroup(Client *c, Window leader, int group) {
	Group *g = NULL;

	if (leader == None || leader == c->win)
		return;
	XFindContext(dpy, leader, context[group], (XPointer *)&g);
	if (g) {
		Window *list;
		unsigned int i, j;

		list = ecalloc(g->count, sizeof(*list));
		for (i = 0, j = 0; i < g->count; i++)
			if (g->members[i] != c->win)
				list[j++] = g->members[i];
		if (j == 0) {
			free(list);
			free(g->members);
			free(g);
			XDeleteContext(dpy, leader, context[group]);
		} else {
			free(g->members);
			g->members = list;
			g->count = j;
		}
	}
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (ebastardly on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which may call exit.	*/
int
xerror(Display * dsply, XErrorEvent * ee) {
	if (ee->error_code == BadWindow
	    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr,
	    "echinus: fatal error: request code=%d, error code=%d\n",
	    ee->request_code, ee->error_code);
	return xerrorxlib(dsply, ee);	/* may call exit */
}

int
xerrordummy(Display * dsply, XErrorEvent * ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display * dsply, XErrorEvent * ee) {
	otherwm = True;
	return -1;
}

void
view(int index) {
	int i, j;
	Monitor *m, *cm;
	int prevtag;

	i = (index == -1) ? 0 : index;
	cm = curmonitor();

	if (cm->seltags[i])
		return;

	memcpy(cm->prevtags, cm->seltags, ntags * sizeof(cm->seltags[0]));

	for (j = 0; j < ntags; j++)
		cm->seltags[j] = 0;
	cm->seltags[i] = True;
	prevtag = cm->curtag;
	cm->curtag = i;
	for (m = monitors; m; m = m->next) {
		if (m->seltags[i] && m != cm) {
			m->curtag = prevtag;
			memcpy(m->prevtags, m->seltags, ntags * sizeof(m->seltags[0]));
			memcpy(m->seltags, cm->prevtags, ntags * sizeof(cm->seltags[0]));
			updategeom(m);
			arrange(m);
		}
	}
	updategeom(cm);
	arrange(cm);
	focus(NULL);
	updateatom[CurDesk] (NULL);
	updateatom[ELayout] (NULL);
	updateatom[ESelTags] (NULL);
}

void
viewprevtag(const char *arg) {
	Bool tmptags[ntags];
	unsigned int i = 0;
	int prevcurtag;

	while (i < ntags - 1 && !curprevtags[i])
		i++;
	prevcurtag = curmontag;
	curmontag = i;

	memcpy(tmptags, curseltags, ntags * sizeof(curseltags[0]));
	memcpy(curseltags, curprevtags, ntags * sizeof(curseltags[0]));
	memcpy(curprevtags, tmptags, ntags * sizeof(curseltags[0]));
	if (views[prevcurtag].barpos != views[curmontag].barpos)
		updategeom(curmonitor());
	arrange(NULL);
	focus(NULL);
	updateatom[CurDesk] (NULL);
	updateatom[ELayout] (NULL);
	updateatom[ESelTags] (NULL);
}

void
viewlefttag(const char *arg) {
	unsigned int i;

	/* wrap around: TODO: do full _NET_DESKTOP_LAYOUT */
	if (curseltags[0]) {
		view(ntags - 1);
		return;
	}
	for (i = 1; i < ntags; i++) {
		if (curseltags[i]) {
			view(i - 1);
			return;
		}
	}
}

void
viewrighttag(const char *arg) {
	unsigned int i;

	for (i = 0; i < ntags - 1; i++) {
		if (curseltags[i]) {
			view(i + 1);
			return;
		}
	}
	/* wrap around: TODO: do full _NET_DESKTOP_LAYOUT */
	if (i == ntags - 1) {
		view(0);
		return;
	}
}

void
zoom(Client *c) {
	if (!c || !FEATURES(curlayout, ZOOM) || c->isfloating)
		return;
	if (c == nexttiled(clients, curmonitor()))
		if (!(c = nexttiled(c->next, curmonitor())))
			return;
	detach(c);
	attach(c, False);
	updateatom[ClientList] (NULL);
	arrange(curmonitor());
	focus(c);
}

int
main(int argc, char *argv[]) {
	char conf[256] = "";
	int i;

	if (argc == 3 && !strcmp("-f", argv[1]))
		snprintf(conf, sizeof(conf), "%s", argv[2]);
	else if (argc == 2 && !strcmp("-v", argv[1]))
		eprint("echinus-" VERSION " (c) 2011 Alexander Polakov\n");
	else if (argc != 1)
		eprint("usage: echinus [-v] [-f conf]\n");

	setlocale(LC_CTYPE, "");
	if (!(dpy = XOpenDisplay(0)))
		eprint("echinus: cannot open display\n");
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	cargc = argc;
	cargv = argv;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	for (i = 0; i < PartLast; i++)
		context[i] = XUniqueContext();
	checkotherwm();
	setup(conf);
	updateatom[KdeSplashProgress] (NULL);
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
