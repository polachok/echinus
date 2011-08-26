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
#define CLIENTMASK	        (PropertyChangeMask | EnterWindowMask | FocusChangeMask)
#define FRAMEMASK               (MOUSEMASK | SubstructureRedirectMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask)

/* function-like macros */
#define save(_c) { (_c)->rx = (_c)->x; \
	       	(_c)->ry = (_c)->y; \
		(_c)->rw = (_c)->w; \
		(_c)->rh = (_c)->h; }

/* enums */
enum { StrutsOn, StrutsOff, StrutsHide };		    /* struts position */
enum { CurNormal, CurResize, CurMove, CurLast };	    /* cursor */
enum { Clk2Focus, SloppyFloat, AllSloppy, SloppyRaise };    /* focus model */

/* function declarations */
void applyrules(Client * c);
void arrange(Monitor * m);
void attach(Client * c);
void attachstack(Client * c);
void ban(Client * c);
void buttonpress(XEvent * e);
void bstack(Monitor * m);
void checkotherwm(void);
void cleanup(void);
void compileregs(void);
void configure(Client * c);
void configurenotify(XEvent * e);
void configurerequest(XEvent * e);
void destroynotify(XEvent * e);
void detach(Client * c);
void detachstack(Client * c);
void *emallocz(unsigned int size);
void enternotify(XEvent * e);
void eprint(const char *errstr, ...);
void expose(XEvent * e);
void floating(Monitor * m);	/* default floating layout */
void ifloating(Monitor * m);	/* intellectual floating layout try */
void iconify(const char *arg);
void incnmaster(const char *arg);
void focus(Client * c);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w, Client * list, int part);
const char *getresource(const char *resource, const char *defval);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(Client * c, Bool focused);
void getpointer(int *x, int *y);
Monitor *getmonitor(int x, int y);
Monitor *curmonitor();
Monitor *clientmonitor(Client * c);
int idxoftag(const char *tag);
Bool isvisible(Client * c, Monitor * m);
void initmonitors(XEvent * e);
void keypress(XEvent * e);
void killclient(const char *arg);
void leavenotify(XEvent * e);
void focusin(XEvent * e);
void manage(Window w, XWindowAttributes * wa);
void mappingnotify(XEvent * e);
void monocle(Monitor * m);
void maprequest(XEvent * e);
void mousemove(Client * c);
void mouseresize(Client * c);
void moveresizekb(const char *arg);
Client *nexttiled(Client * c, Monitor * m);
Client *prevtiled(Client * c, Monitor * m);
void propertynotify(XEvent * e);
void reparentnotify(XEvent * e);
void quit(const char *arg);
void restart(const char *arg);
void resize(Client * c, Monitor * m, int x, int y, int w, int h, Bool sizehints);
void restack(Monitor * m);
void run(void);
void scan(void);
void setclientstate(Client * c, long state);
void setlayout(const char *arg);
void setmwfact(const char *arg);
static int smartcheckarea(Monitor * m, int x, int y, int w, int h);
void setup(char *);
void spawn(const char *arg);
void tag(const char *arg);
void tile(Monitor * m);
void togglestruts(const char *arg);
void togglefloating(const char *arg);
void togglemax(const char *arg);
void togglefill(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void togglemonitor(const char *arg);
void focusview(const char *arg);
void unban(Client * c);
void unmanage(Client * c);
void updategeom(Monitor * m);
void unmapnotify(XEvent * e);
void updatesizehints(Client * c);
void updateframe(Client * c);
void updatetitle(Client * c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
void viewlefttag(const char *arg);
void viewrighttag(const char *arg);
int xerror(Display * dpy, XErrorEvent * ee);
int xerrordummy(Display * dsply, XErrorEvent * ee);
int xerrorstart(Display * dsply, XErrorEvent * ee);
int (*xerrorxlib) (Display *, XErrorEvent *);
void zoom(const char *arg);

/* variables */
char **cargv;
Display *dpy;
int screen;
Window root;
XrmDatabase xrdb;
Bool otherwm;
Bool running = True;
Bool selscreen = True;
Monitor *monitors;
Client *clients;
Client *sel;
Client *stack;
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
/* configuration, allows nested code to access above variables */
#include "config.h"

struct {
	Bool dectiled;
	Bool hidebastards;
	int focus;
	int snap;
	char command[255];
} options;

Layout layouts[] = {
	/* function	symbol	features */
	{  ifloating,	'i',	OVERLAP },
	{  tile,	't',	MWFACT | NMASTER | ZOOM },
	{  bstack,	'b',	MWFACT | ZOOM },
	{  monocle,	'm',	0 },
	{  floating,	'f',	OVERLAP },
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
#ifdef XRANDR
	[RRScreenChangeNotify] = initmonitors,
#endif
};

/* function implementations */
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
	if (!matched) {
		memcpy(c->tags, curseltags, ntags * sizeof(curseltags[0]));
	}
}

void
arrangefloats(Monitor * m) {
	Client *c;

	for(c = stack; c; c = c->snext) {
		if(isvisible(c, m) && !c->isbastard &&
			       	c->isfloating && !c->ismax && !c->isicon)
			resize(c, m, c->x, c->y, c->w, c->h, True);
	}
}

void
arrangemon(Monitor * m) {
	Client *c;

	views[m->curtag].layout->arrange(m);
	arrangefloats(m);
	restack(m);
	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == m) &&
		    ((!c->isbastard && isvisible(c, m) && !c->isicon) ||
			(c->isbastard && views[m->curtag].barpos == StrutsOn))) {
			unban(c);
#if 0
			if (c->isbastard)
				c->isicon = False;
#endif
		}
	}

	for (c = stack; c; c = c->snext) {
		if ((clientmonitor(c) == m) &&
		    ((!c->isbastard && (!isvisible(c, m) || c->isicon)) ||
			(c->isbastard && views[m->curtag].barpos == StrutsHide))) {
			ban(c);
#if 0
			if (c->isbastard)
				c->isicon = True;
#endif
		}
	}
}

void
arrange(Monitor * m) {
	Monitor *i;

	if (!m) {
		for (i = monitors; i; i = i->next)
			arrangemon(i);
	} else
		arrangemon(m);
}

void
attach(Client * c) {
	if (clients)
		clients->prev = c;
	c->next = clients;
	clients = c;
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
	if ((c = getclient(ev->window, clients, ClientTitle))) {
		DPRINTF("TITLE %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		for (i = 0; i < LastBtn; i++) {
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
		for (i = 0; i < LastBtn; i++) {
			button[i].pressed = 0;
			drawclient(c);
		}
		if (ev->type == ButtonRelease)
			return;
		if (FEATURES(curlayout, OVERLAP) || c->isfloating)
			XRaiseWindow(dpy, c->frame);
		if (ev->button == Button1)
			mousemove(c);
		else if (ev->button == Button3)
			mouseresize(c);
	} else if ((c = getclient(ev->window, clients, ClientFrame))) {
		DPRINTF("FRAME %s: 0x%x\n", c->name, (int) ev->window);
		focus(c);
		if (FEATURES(curlayout, OVERLAP) || c->isfloating)
			XRaiseWindow(dpy, c->frame);
		if (CLEANMASK(ev->state) != modkey) {
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
			return;
		}
		if (ev->button == Button1) {
			if (!FEATURES(curlayout, OVERLAP) && !c->isfloating)
				togglefloating(NULL);
			if (c->ismax)
				togglemax(NULL);
			mousemove(c);
		} else if (ev->button == Button2) {
			if (!FEATURES(curlayout, OVERLAP) && c->isfloating)
				togglefloating(NULL);
			else
				zoom(NULL);
		} else if (ev->button == Button3) {
			if (!FEATURES(curlayout, OVERLAP) && !c->isfloating)
				togglefloating(NULL);
			if (c->ismax)
				togglemax(NULL);
			mouseresize(c);
		} else		/* don't know what to do? pass it on */
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
	}
}

void
checkotherwm(void) {
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
		unmanage(stack);
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
configure(Client * c) {
	XConfigureEvent ce;
	Monitor *m = clientmonitor(c);

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x + m->sx;
	ce.y = c->y + m->sy;
	ce.width = c->w;
	ce.height = c->h - c->th;
	ce.border_width = 0;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *) & ce);
}

void
configurenotify(XEvent * e) {
	XConfigureEvent *ev = &e->xconfigure;
	Monitor *m;

	if (ev->window == root) {
#ifdef XRANDR
		if (XRRUpdateConfiguration((XEvent *) ev)) {
#endif
			initmonitors(e);
			for (m = monitors; m; m = m->next)
				updategeom(m);
			arrange(NULL);
#ifdef XRANDR
		}
#endif
	}
}

void
configurerequest(XEvent * e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	Monitor *cm;
	int x, y, w, h;

	if ((c = getclient(ev->window, clients, ClientWindow))) {
		c->ismax = False;
		cm = clientmonitor(c);
		if (ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
		if (c->isfixed || c->isfloating || MFEATURES(clientmonitor(c), OVERLAP)) {
#if 0
			if (c->isbastard) {
				if (ev->value_mask & CWX)
					c->x = ev->x;
				if (ev->value_mask & CWY)
					c->y = ev->y;
			}
#endif
#if 0
			if (ev->value_mask & CWWidth)
				c->w = ev->width;
			if (ev->value_mask & CWHeight)
				c->h = ev->height + c->th;
			if (!(ev->value_mask & (CWX | CWY | CWWidth | CWHeight)))
				return;
#endif
#if 0
			if ((c->x + c->w) > (curwax + cursw) && c->isfloating)
				c->x = cursw / 2 - c->w / 2;	/* center in x direction */
			if ((c->y + c->h) > (curway + cursh) && c->isfloating)
				c->y = cursh / 2 - c->h / 2;	/* center in y direction */
#endif
			/* get x,y, convert to relative */
			if (ev->value_mask & CWX)
				x = ev->x - cm->sx;
			if (ev->value_mask & CWY)
				y = ev->y - cm->sy;
			if (ev->value_mask & CWWidth)
				w = ev->width;
			if (ev->value_mask & CWHeight)
				h = ev->height + c->th;
			cm = getmonitor(ev->x, ev->y);
			if (!(ev->value_mask & (CWX | CWY)) /* resize request */
			    && (ev->value_mask & (CWWidth | CWHeight))) {
				DPRINTF("RESIZE %s (%d,%d)->(%d,%d)\n", c->name, c->w, c->h, w, h);
				resize(c, clientmonitor(c), c->x, c->y, w, h, True);
			} else if ((ev->value_mask & (CWX | CWY)) /* move request */
			    && !(ev->value_mask & (CWWidth | CWHeight))) {
				DPRINTF("MOVE %s (%d,%d)->(%d,%d)\n", c->name, c->x, c->y, x, y);
				resize(c, cm, x, y, c->w, c->h, True);
#if 0
				XMoveWindow(dpy, c->frame, ev->x, ev->y);
				c->x = ev->x;
				c->y = ev->y;
				configure(c);
				arrange(cm);
#endif
			} else if ((ev->value_mask & (CWX | CWY)) /* move and resize request */
			    && (ev->value_mask & (CWWidth | CWHeight))) {
				DPRINTF("MOVE&RESIZE(MOVE) %s (%d,%d)->(%d,%d)\n", c->name, c->x, c->y, ev->x, ev->y);
				DPRINTF("MOVE&RESIZE(RESIZE) %s (%d,%d)->(%d,%d)\n", c->name, c->w, c->h, ev->width, ev->height);
				resize(c, cm, x, y, w, h, True);
			} else if ((ev->value_mask & CWStackMode)) {
				DPRINTF("RESTACK %s ignoring\n", c->name);
				configure(c);
			}
#if 0
			if (isvisible(c, NULL)) {
				DPRINTF("%s %d %d => %d %d\n", c->name, c->x,
				    c->y, ev->x, ev->y);
				XMoveResizeWindow(dpy, c->frame, c->x, c->y, c->w, c->h);
				if (c->title) {
					XMoveResizeWindow(dpy, c->title, 0, 0,
					    c->w, c->th);
					XFreePixmap(dpy, c->drawable);
					c->drawable =
					    XCreatePixmap(dpy, root, c->w,
					    c->th, DefaultDepth(dpy, screen));
				}
				XMoveResizeWindow(dpy, c->win, 0, c->th,
				    ev->width, ev->height);
				drawclient(c);
				arrange(NULL);
			}
#endif
		} else {
			configure(c);
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
	}
	XSync(dpy, False);
}

void
destroynotify(XEvent * e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if (!(c = getclient(ev->window, clients, ClientWindow)))
		return;
	unmanage(c);
	updateatom[ClientList] (NULL);
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
emallocz(unsigned int size) {
	void *res = calloc(1, size);

	if (!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
}

void
enternotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;

	if (ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;
	if ((c = getclient(ev->window, clients, ClientFrame))) {
		if (!isvisible(sel, curmonitor()))
			focus(c);
		if (c->isbastard) {
			grabbuttons(c, True);
			return;
		}
		switch (options.focus) {
		case Clk2Focus:
			XGrabButton(dpy, AnyButton, AnyModifier, c->frame,
			    False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
			break;
		case SloppyFloat:
			if (FEATURES(curlayout, OVERLAP) || c->isfloating)
				focus(c);
			XGrabButton(dpy, AnyButton, AnyModifier, c->frame,
			    False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
			break;
		case AllSloppy:
			focus(c);
			break;
		case SloppyRaise:
			focus(c);
			restack(curmonitor());
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

	if (sel && (ev->window != sel->win))
		XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
}

void
expose(XEvent * e) {
	XExposeEvent *ev = &e->xexpose;
	XEvent tmp;
	Client *c;

	while (XCheckWindowEvent(dpy, ev->window, ExposureMask, &tmp));
	if((c = getclient(ev->window, clients, ClientTitle)))
		drawclient(c);
}

void
floating(Monitor * m) {		/* default floating layout */
	Client *c;

	for (c = clients; c; c = c->next) {
		if (isvisible(c, m) && !c->isicon) {
			if (!c->isfloating)
				/* restore last known float dimensions */
				resize(c, m, c->rx, c->ry, c->rw, c->rh, True);
			else
				resize(c, m, c->x, c->y, c->w, c->h, True);
		}
	}
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
		    c && (c->isbastard || c->isicon || !isvisible(c, curmonitor())); c = c->snext);
	if (sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->frame, style.color.norm[ColBorder]);
	}
	if (c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
		/* unban(c); */
	}
	sel = c;
	if (!selscreen)
		return;
	if (c) {
		setclientstate(c, NormalState);
		if (c->isfocusable) {
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			givefocus(c);
		}
		XSetWindowBorder(dpy, sel->frame, style.color.sel[ColBorder]);
		drawclient(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}
	if (o)
		drawclient(o);
	updateatom[ActiveWindow] (sel);
	updateatom[ClientList] (NULL);
	updateatom[CurDesk] (NULL);
}

void
focusicon(const char *arg) {
	Client *c;

	for(c = clients; c && (!c->isicon || !isvisible(c, curmonitor())); c = c->next);
	if (!c)
		return;
	c->isicon = False;
	focus(c);
	arrange(curmonitor());
}

void
focusnext(const char *arg) {
	Client *c;

	if (!sel)
		return;
	for (c = sel->next;
	    c && (c->isbastard || c->isicon || !isvisible(c, curmonitor())); c = c->next);
	if (!c)
		for (c = clients;
		    c && (c->isbastard || c->isicon
			|| !isvisible(c, curmonitor())); c = c->next);
	if (c) {
		focus(c);
		restack(curmonitor());
	}
}

void
focusprev(const char *arg) {
	Client *c;

	if (!sel)
		return;
	for (c = sel->prev;
	    c && (c->isbastard || c->isicon || !isvisible(c, curmonitor())); c = c->prev);
	if (!c) {
		for (c = clients; c && c->next; c = c->next);
		for (;
		    c && (c->isbastard || c->isicon
			|| !isvisible(c, curmonitor())); c = c->prev);
	}
	if (c) {
		focus(c);
		restack(curmonitor());
	}
}

void
iconify(const char *arg) {
	Client *c;
	if (!sel)
		return;
	c = sel;
	focusnext(NULL);
	ban(c);
	c->isicon = True;
	arrange(curmonitor());
}

void
ifloating(Monitor * m) {
	Client *c;
	int x, y, f;

	for (c = clients; c; c = c->next) {
		if (isvisible(c, m) && !c->isicon && !c->isbastard) {
			if (c->isplaced)
				resize(c, m, c->x, c->y, c->w, c->h, True);
			for (f = 0; !c->isplaced; f++) {
				if ((c->w > m->sw / 2 && c->h > m->sw / 2)
				    || c->h < 4) {
					/* too big to deal with */
					c->isplaced = True;
					resize(c, m, c->x, c->y, c->w, c->h, True);
				}
				for (y = m->way;
				    y + c->h <= m->way + m->wah
				    && !c->isplaced; y += c->h / 4) {
					for (x = m->wax;
					    x + c->w <= m->wax + m->waw
					    && !c->isplaced; x += c->w / 8) {
						if (smartcheckarea(m, x, y,
							0.8 * c->w, 0.8 * c->h) <= f) {
							resize(c, m,
							    x +
							    c->th * (rand() %
								3),
							    y + c->th +
							    c->th * (rand() %
								3), c->w, c->h, True);
							c->isplaced = True;
						}
					}
				}
			}
		}
	}
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
getclient(Window w, Client * list, int part) {
	Client *c;

#define ClientPart(_c, _part) (((_part) == ClientWindow) ? (_c)->win : \
			       ((_part) == ClientTitle) ? (_c)->title : \
			       ((_part) == ClientFrame) ? (_c)->frame : 0)

	for (c = list; c && (ClientPart(c, part)) != w; c = c->next);

	return c;
}

long
getstate(Window w) {
	long ret = -1;
	long *p = NULL;
	unsigned long n;

	p = (long*)getatom(w, atom[WMState], &n);
	if (n != 0)
		ret = *p;
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
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return False;
	text[0] = '\0';
	XGetTextProperty(dpy, w, &name, atom);
	if (!name.nitems)
		return False;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *) name.value, size - 1);
	} else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
		    && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return True;
}

void
grabbuttons(Client * c, Bool focused) {
	unsigned int Buttons[] = { Button1, Button2, Button3 };
	unsigned int Modifiers[] = { modkey, modkey | LockMask,
		modkey | numlockmask, modkey | numlockmask | LockMask
	};
	unsigned int i, j;
	XUngrabButton(dpy, AnyButton, AnyModifier, c->frame);

	if (focused) {
		for (i = 0; i < LENGTH(Buttons); i++)
			for (j = 0; j < LENGTH(Modifiers); j++)
				XGrabButton(dpy, Buttons[i], Modifiers[j],
				    c->frame, False, BUTTONMASK, GrabModeAsync,
				    GrabModeSync, None, None);
	} else {
		XGrabButton(dpy, AnyButton, AnyModifier, c->frame, False,
		    BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

int
idxoftag(const char *tag) {
	unsigned int i;

	for (i = 0; (i < ntags) && strcmp(tag, tags[i]); i++);
	return (i < ntags) ? i : 0;
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

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode) ev->keycode, 0);
	for (i = 0; i < nkeys; i++)
		if (keysym == keys[i]->keysym
		    && CLEANMASK(keys[i]->mod) == CLEANMASK(ev->state)) {
			if (keys[i]->func)
				keys[i]->func(keys[i]->arg);
			XUngrabKeyboard(dpy, CurrentTime);
		}
}

void
killclient(const char *arg) {
	XEvent ev;

	if (!sel)
		return;
	if (checkatom(sel->win, atom[WMProto], atom[WMDelete])) {
		ev.type = ClientMessage;
		ev.xclient.window = sel->win;
		ev.xclient.message_type = atom[WMProto];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = atom[WMDelete];
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, sel->win, False, NoEventMask, &ev);
	} else {
		XKillClient(dpy, sel->win);
	}
}

void
leavenotify(XEvent * e) {
	XCrossingEvent *ev = &e->xcrossing;
	Client *c;

	if ((ev->window == root) && !ev->same_screen) {
		selscreen = False;
		focus(NULL);
	}
	if ((c = getclient(ev->window, clients, ClientFrame)))
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
}

void
manage(Window w, XWindowAttributes * wa) {
	Client *c, *t = NULL;
	Monitor *cm;
	Window trans;
	Status rettrans;
	XWindowChanges wc;
	XSetWindowAttributes twa;
	XWMHints *wmh;
	unsigned long mask = 0;

	cm = curmonitor();
	c = emallocz(sizeof(Client));
	c->win = w;
	if (checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]) ||
	    checkatom(c->win, atom[WindowType], atom[WindowTypeDock])) {
		c->isbastard = True;
		c->isfloating = True;
		c->isfixed = True;
	}
	if (checkatom(c->win, atom[WindowType], atom[WindowTypeDialog])) {
		c->isfloating = True;
		c->isfixed = True;
	}

	c->isicon = False;
	c->title = c->isbastard ? (Window) NULL : 1;
	c->tags = emallocz(ntags * sizeof(curseltags[0]));
	c->isfocusable = c->isbastard ? False : True;
	c->border = c->isbastard ? 0 : style.border;
	/*  XReparentWindow() unmaps *mapped* windows */
	c->ignoreunmap = wa->map_state == IsViewable ? 1 : 0;
	mwm_process_atom(c);
	updatesizehints(c);

	if ((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
		for (t = clients; t && t->win != trans; t = t->next);
	if (t)
		memcpy(c->tags, t->tags, ntags * sizeof(curseltags[0]));

	updatetitle(c);
	applyrules(c);

	c->th = c->title ? style.titleheight : 0;

	if (!c->isfloating)
		c->isfloating = (rettrans == Success) || c->isfixed;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		c->isfocusable = !(wmh->flags & InputHint) || wmh->input;
		XFree(wmh);
	}

	c->x = c->rx = wa->x % cm->sw;
	c->y = c->ry = wa->y % cm->sh;
	c->w = c->rw = wa->width;
	c->h = c->rh = wa->height + c->th;

	if (wa->x && wa->y) {
		c->isplaced = True;
	} else if (!c->isbastard) {
		getpointer(&c->x, &c->y);
		c->rx = c->x = (c->x - c->w/2) % cm->sw;
		c->ry = c->x = (c->y - c->h/2) % cm->sh;
	}
	if (c->isbastard) {
		c->x = wa->x;
		c->y = wa->y;
	}
	cm = c->isbastard ? getmonitor(c->x, c->y) : clientmonitor(c);
	c->oldborder = c->isbastard ? 0 : wa->border_width;
	if (c->w == cursw && c->h == cursh) {
		c->x = 0;
		c->y = 0;
	} else if (!c->isbastard) {
		if (c->x + c->w > curwax + curwaw)
			c->x = curwaw - c->w;
		if (c->y + c->h > curway + curwah)
			c->y = curwah - c->h;
		if (c->x < curwax)
			c->x = 0;
		if (c->y < curway)
			c->y = 0;
	}

	grabbuttons(c, False);
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
	    XCreateWindow(dpy, root, cm->sx + c->x, cm->sy + c->y, c->w,
	    c->h, c->border, wa->depth == 32 ? 32 : DefaultDepth(dpy, screen),
	    InputOutput, wa->depth == 32 ? wa->visual : DefaultVisual(dpy,
		screen), mask, &twa);

	wc.border_width = c->border;
	XConfigureWindow(dpy, c->frame, CWBorderWidth, &wc);
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
	} else {
		c->title = (Window) NULL;
	}

	if (c->isbastard) {
		free(c->tags);
		c->tags = cm->seltags;
	}
	attach(c);
	attachstack(c);
	XReparentWindow(dpy, c->win, c->frame, 0, c->th);
	XReparentWindow(dpy, c->title, c->frame, 0, 0);
	XAddToSaveSet(dpy, c->win);
	XMapWindow(dpy, c->win);
	wc.border_width = 0;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	configure(c);	/* propagates border_width, if size doesn't change */
	if (checkatom(c->win, atom[WindowState], atom[WindowStateFs]))
		ewmh_process_state_atom(c, atom[WindowStateFs], 1);
	if (c->isbastard)
		XSelectInput(dpy, w, PropertyChangeMask);
	else
		XSelectInput(dpy, w, CLIENTMASK);
	ban(c);
	updateatom[ClientList] (NULL);
	updateatom[WindowDesk] (c);
	updateframe(c);
	if (c->isbastard) {
		updatestruts(c);
		updategeom(clientmonitor(c));
	}
	arrange(clientmonitor(c));
	if (!checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]))
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
	if ((c = getclient(ev->window, clients, ClientWindow))) {
#if 0
		unban(c);
		arrange(curmonitor());
#endif
	} else
		manage(ev->window, &wa);
}

void
monocle(Monitor * m) {
	Client *c;

	for (c = nexttiled(clients, m); c; c = nexttiled(c->next, m)) {
			if (views[m->curtag].barpos != StrutsOn)
				resize(c, m, m->wax - c->border,
						m->way - c->border, m->waw, m->wah, False);
			else
				resize(c, m, m->wax, m->way,
						m->waw - 2 * c->border,
						m->wah - 2 * c->border, False);
	}
}

void
moveresizekb(const char *arg) {
	int dw, dh, dx, dy;

	dw = dh = dx = dy = 0;
	if (!sel)
		return;
	if (!sel->isfloating)
		return;
	sscanf(arg, "%d %d %d %d", &dx, &dy, &dw, &dh);
	if (dw && (dw < sel->incw))
		dw = (dw / abs(dw)) * sel->incw;
	if (dh && (dh < sel->inch))
		dh = (dh / abs(dh)) * sel->inch;
	resize(sel, curmonitor(), sel->x + dx, sel->y + dy, sel->w + dw,
	    sel->h + dh, True);
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
		if ((x >= m->sx && x < m->sx + m->sw)) {
			break;
		}
	}
	return m ? m : monitors;
}

Monitor *
clientmonitor(Client * c) {
	Monitor *m;
	unsigned int i;
	if (c) {
		if (c->isbastard)
			return getmonitor(c->x, c->y);
		for (m = monitors; m; m = m->next) {
			for (i = 0; i < ntags; i++)
				if (c->tags[i] & m->seltags[i])
					return m;
		}
	}
	return curmonitor();
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
	unsigned int i;
	XEvent ev;
	Monitor *m, *nm;

	if (c->isbastard)
		return;
	m = curmonitor();
	ocx = c->x + m->sx;
	ocy = c->y + m->sy;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	getpointer(&x1, &y1);
	for (;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type] (&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			/* we are probably moving to a different monitor */
			nm = curmonitor();
			nx = ocx + (ev.xmotion.x - x1) - nm->sx;
			ny = ocy + (ev.xmotion.y - y1) - nm->sy;
			if (abs(nm->wax + nx) < options.snap)
				nx = nm->wax;
			else if (abs((nm->wax + nm->waw) - (nx + c->w +
				    2 * c->border)) < options.snap)
				nx = nm->wax + nm->waw - c->w - 2 * c->border;
			if (abs(nm->way - ny) < options.snap)
				ny = nm->way;
			else if (abs((nm->way + nm->wah) - (ny + c->h +
				    2 * c->border)) < options.snap)
				ny = nm->way + nm->wah - c->h - 2 * c->border;
			resize(c, nm, nx, ny, c->w, c->h, False);
			save(c);
			if (m != nm) {
				for (i = 0; i < ntags; i++)
					c->tags[i] = nm->seltags[i];
				updateatom[WindowDesk] (c);
				drawclient(c);
				arrange(NULL);
			}
			break;
		}
	}
}

void
mouseresize(Client * c) {
	int ocx, ocy, nw, nh;
	Monitor *cm;
	XEvent ev;

	if (c->isbastard || c->isfixed)
		return;
	cm = curmonitor();

	ocx = c->x + cm->sx;
	ocy = c->y + cm->sy;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync,
		GrabModeAsync, None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
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
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type] (&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			if ((nw = ev.xmotion.x - ocx - 2 * c->border + 1) <= 0)
				nw = MINWIDTH;
			if ((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
				nh = MINHEIGHT;
			resize(c, cm, c->x, c->y, nw, nh, True);
			save(c);
			break;
		}
	}
}

Client *
nexttiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard
		|| c->isicon); c = c->next);
	return c;
}

Client *
prevtiled(Client * c, Monitor * m) {
	for (; c && (c->isfloating || !isvisible(c, m) || c->isbastard
		|| c->isicon); c = c->prev);
	return c;
}

void
reparentnotify(XEvent * e) {
	Client *c;
	XReparentEvent *ev = &e->xreparent;

	if ((c = getclient(ev->window, clients, ClientWindow)))
		if (ev->parent != c->frame)
			unmanage(c);
}

void
propertynotify(XEvent * e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if (ev->state == PropertyDelete) 
		return;		/* XXX: ignoring, probably shouldn't */
	if ((c = getclient(ev->window, clients, ClientWindow))) {
		switch (ev->atom) {
		case XA_WM_TRANSIENT_FOR:
			XGetTransientForHint(dpy, c->win, &trans);
			if (!c->isfloating
			    && (c->isfloating =
				(getclient(trans, clients, ClientWindow) != NULL)))
				arrange(clientmonitor(c));
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_NAME:
			updatetitle(c);
			break;
		}
		if (ev->atom == atom[StrutPartial]) {
			updatestruts(c);
			updategeom(clientmonitor(c));
			arrange(clientmonitor(c));
		} else if (ev->atom == atom[WindowName])
			updatetitle(c);
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

void
resize(Client * c, Monitor * m, int x, int y, int w, int h, Bool sizehints) {
	XWindowChanges wc;

	if (sizehints) {
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
		if (c->minh > 0 && h - c->th < c->minh)
			h = c->minh + c->th;
		if (c->maxw > 0 && w > c->maxw)
			w = c->maxw;
		if (c->maxh > 0 && h - c->th > c->maxh)
			h = c->maxh + c->th;
		h += c->th;
	}
	if (w <= 0 || h <= 0)
		return;
	/* offscreen appearance fixes */
	if (x > m->wax + m->sw)
		x = m->sw - w - 2 * c->border;
	if (y > m->way + m->sh)
		y = m->sh - h - 2 * c->border;
#if 0
	if (x + w + 2 * c->border < m->sx)
		x = m->wax;
	if (y + h + 2 * c->border < m->sy)
		y = m->way;
#endif
	if (w != c->w && c->th) {
		XMoveResizeWindow(dpy, c->title, 0, 0, w, c->th);
		XFreePixmap(dpy, c->drawable);
		c->drawable =
			XCreatePixmap(dpy, root, w, c->th, DefaultDepth(dpy, screen));
		drawclient(c);
	}
	if (c->x != x || c->y != y || c->w != w || c->h != h /* || sizehints */) {
		if (c->isfloating || MFEATURES(m, OVERLAP))
			c->isplaced = True;
		c->x = x;
		c->y = y;
		c->w = w;
		c->h = h;
		XMoveResizeWindow(dpy, c->frame, m->sx + c->x, m->sy + c->y, c->w, c->h);
		XMoveResizeWindow(dpy, c->win, 0, c->th, c->w, c->h - c->th);
		configure(c);
#if 0
		wc.x = 0;
		wc.y = c->th;
		wc.width = c->w;
		wc.height = c->h - c->th;
		wc.border_width = 0;
		XConfigureWindow(dpy, c->win,
		    CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
#endif
		XSync(dpy, False);
	}
}

void
restack(Monitor * m) {
	Client *c;
	XEvent ev;
	Window *wl;
	int i, n;

	if (!sel)
		return;
#if 0
	if (MFEATURES(m, OVERLAP)) {
		XRaiseWindow(dpy, sel->frame);
		goto end;
	}
#endif
	for (n = 0, c = stack; c; c = c->snext) {
		if (isvisible(c, m) && !c->isicon) {
			n++;
		}
	}
	if (!n)
		return;
	wl = malloc(sizeof(Window) * n);
	for (i = 0, c = stack; c && i < n; c = c->snext) {
		if (isvisible(c, m) && !c->isicon && c->isbastard &&
		    !checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]))
			wl[i++] = c->frame;
	}
	for (c = stack; c && i < n; c = c->snext)
		if (isvisible(c, m) && !c->isicon) {
			if (!c->isbastard && c->isfloating)
				wl[i++] = c->frame;
		}
	for (c = stack; c && i < n; c = c->snext) {
		if (isvisible(c, m) && !c->isicon)
			if (!c->isfloating && !c->isbastard)
				wl[i++] = c->frame;
	}
	XRestackWindows(dpy, wl, n);
	free(wl);
	for (c = stack; c; c = c->snext)
		if (checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]))
			XLowerWindow(dpy, c->frame);
      end:
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
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
scan(void) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	wins = NULL;
	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa) ||
			    wa.override_redirect
			    || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable
			    || getstate(wins[i]) == IconicState
			    || getstate(wins[i]) == NormalState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) {	/* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
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
	long data[] = { state, None };
	long winstate[2];

	XChangeProperty(dpy, c->win, atom[WMState], atom[WMState], 32,
	    PropModeReplace, (unsigned char *) data, 2);
	if (state == NormalState) {
		c->isicon = False;
		XDeleteProperty(dpy, c->win, atom[WindowState]);
	} else {
		winstate[0] = atom[WindowStateHidden];
		XChangeProperty(dpy, c->win, atom[WindowState], XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) winstate, 1);
	}
}

void
setlayout(const char *arg) {
	unsigned int i;
	Client *c;
	Bool wasfloat;

	wasfloat = FEATURES(curlayout, OVERLAP);

	if (!arg) {
#if 0
		if (&layouts[++ltidxs[curmontag]] == &layouts[LENGTH(layouts)])
			ltidxs[curmontag] = 0;
#endif
	} else {
		for (i = 0; i < LENGTH(layouts); i++)
			if (*arg == layouts[i].symbol)
				break;
		if (i == LENGTH(layouts))
			return;
		views[curmontag].layout = &layouts[i];
	}
	if (sel) {
		for (c = clients; c; c = c->next) {
			if(isvisible(c, curmonitor())) {
				if (wasfloat)
					save(c);
				if (wasfloat != FEATURES(curlayout, OVERLAP))
					updateframe(c);
			}
		}
		arrange(curmonitor());
	}
	updateatom[ELayout] (NULL);
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

/* ifloating() subroutine */
static int
smartcheckarea(Monitor * m, int x, int y, int w, int h) {
	Client *c;
	int n = 0;
	for (c = clients; c; c = c->next) {
		if (isvisible(c, m) && !c->isicon && c->isplaced) {
			/* A Kind Of Magic */
			if ((c->y + c->h >= y && c->y + c->h <= y + h
				&& c->x + c->w >= x && c->x + c->w <= x + w)
			    || (c->x >= x && c->x <= x + w
				&& c->y + c->h >= y && c->y + c->h <= y + h)
			    || (c->x >= x && c->x <= x + w && c->y >= y && c->y <= y + h)
			    || (c->x + c->w >= x && c->x + c->w <= x + w
				&& c->y >= y && c->y <= y + h))
				n++;
		}
	}
	return n;
}

void
initlayouts() {
	unsigned int i, j;
	char conf[32], ltname;
	float mwfact;
	int nmaster;
	const char *deflayout;

	/* init layouts */
	mwfact = atof(getresource("mwfact", STR(DEFMWFACT)));
	nmaster = atoi(getresource("nmaster", STR(DEFNMASTER)));
	deflayout = getresource("deflayout", "i");
	if (!nmaster)
		nmaster = 1;
	for (i = 0; i < ntags; i++) {
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
	updateatom[ELayout] (NULL);
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
			free(m);
			m = t;
		} while (m);
		monitors = NULL;
	}
	if(!running)
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
			m->curtag = n;
			m->prevtags = emallocz(ntags * sizeof(Bool));
			m->seltags = emallocz(ntags * sizeof(Bool));
			m->seltags[n] = True;
			m->next = monitors;
			monitors = m;
			n++;
		}
		XRRFreeCrtcInfo(ci);
	}
	XRRFreeScreenResources(sr);
	updateatom[WorkArea](NULL);;
	return;
      no_xrandr:
#endif
	m = emallocz(sizeof(Monitor));
	m->sx = m->wax = 0;
	m->sy = m->way = 0;
	m->sw = m->waw = DisplayWidth(dpy, screen);
	m->sh = m->wah = DisplayHeight(dpy, screen);
	m->curtag = 0;
	m->prevtags = emallocz(ntags * sizeof(Bool));
	m->seltags = emallocz(ntags * sizeof(Bool));
	m->seltags[0] = True;
	m->next = NULL;
	monitors = m;
	updateatom[WorkArea](NULL);;
}

void
inittags() {
	unsigned int i;
	char tmp[25] = "\0";

	ntags = atoi(getresource("tags.number", "5"));
	views = emallocz(ntags * sizeof(View));
	tags = emallocz(ntags * sizeof(char *));
	for (i = 0; i < ntags; i++) {
		tags[i] = emallocz(25);
		snprintf(tmp, sizeof(tmp), "tags.name%d", i);
		snprintf(tags[i], sizeof(tags[i]), "%s", getresource(tmp,
		    "null"));
	}
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
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
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
		chdir(path);
		xrdb = XrmGetFileDatabase(conf);
		/* configuration file loaded successfully; break out */
		if (xrdb)
			break;
		fprintf(stderr, "echinus: cannot open configuration file %s\n", conf);
	}
	if (confs[i] == NULL)
		eprint("echinus: couldn't open a configuration file\n");

	/* init EWMH atom */
	initewmh();

	/* init tags */
	inittags();
	/* init geometry */
	initmonitors(NULL);

	/* init modkey */
	initrules();
	initkeys();
	initlayouts();
	updateatom[NumberOfDesk] (NULL);
	updateatom[DeskNames] (NULL);
	updateatom[CurDesk] (NULL);

	grabkeys();

	/* init appearance */
	initstyle();
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

	chdir(oldcwd);

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

void
tag(const char *arg) {
	unsigned int i;

	if (!sel)
		return;
	for (i = 0; i < ntags; i++)
		sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
	updateatom[WindowDesk] (sel);
	updateframe(sel);
	arrange(NULL);
	focus(NULL);
}

void
bstack(Monitor * m) {
	int i, n, nx, ny, nw, nh, mh, tw;
	Client *c, *mc;

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	mh = (n == 1) ? m->wah : views[m->curtag].mwfact * m->wah;
	tw = (n > 1) ? m->waw / (n - 1) : 0;

	nx = m->wax;
	ny = m->way;
	nh = 0;
	for (i = 0, c = mc = nexttiled(clients, m); c; c = nexttiled(c->next, m), i++) {
		c->ismax = False;
		if (i == 0) {
			nh = mh - 2 * c->border;
			nw = m->waw - 2 * c->border;
			nx = m->wax;
		} else {
			if (i == 1) {
				nx = m->wax;
				ny += mc->h + c->border;
				nh = (m->way + m->wah) - ny - 2 * c->border;
			}
			if (i + 1 == n)
				nw = (m->wax + m->waw) - nx - 2 * c->border;
			else
				nw = tw - c->border;
		}
		resize(c, m, nx, ny, nw, nh, False);
		if (n > 1 && tw != curwaw)
			nx = c->x + c->w + c->border;
	}
}

void
tile(Monitor * m) {
	int nx, ny, nw, nh, mw, mh;
	unsigned int i, n, th;
	Client *c, *mc;

	for (n = 0, c = nexttiled(clients, m); c; c = nexttiled(c->next, m))
		n++;

	/* window geoms */
	mh = (n <= views[m->curtag].nmaster) ? m->wah / (n >
	    0 ? n : 1) : m->wah / (views[m->curtag].nmaster ? views[m->curtag].nmaster : 1);
	mw = (n <= views[m->curtag].nmaster) ? m->waw : views[m->curtag].mwfact * m->waw;
	th = (n > views[m->curtag].nmaster) ? m->wah / (n - views[m->curtag].nmaster) : 0;
	if (n > views[m->curtag].nmaster && th < style.titleheight)
		th = m->wah;

	nx = m->wax;
	ny = m->way;
	nw = 0;
	for (i = 0, c = mc = nexttiled(clients, m); c; c = nexttiled(c->next, m), i++) {
		c->ismax = False;
		if (i < views[m->curtag].nmaster) {	/* master */
			ny = m->way + i * (mh - c->border);
			nw = mw - 2 * c->border;
			nh = mh;
			if (i + 1 == (n < views[m->curtag].nmaster ? n : views[m->curtag].nmaster))	/* remainder */
				nh = m->way + m->wah - ny;
			nh -= 2 * c->border;
		} else {	/* tile window */
			if (i == views[m->curtag].nmaster) {
				ny = m->way;
				nx += mc->w + mc->border;
				nw = m->waw - nx - 2 * c->border + m->wax;
			} else
				ny -= c->border;
			if (i + 1 == n)	/* remainder */
				nh = (m->way + m->wah) - ny - 2 * c->border;
			else
				nh = th - 2 * c->border;
		}
		resize(c, m, nx, ny, nw, nh, False);
		if (n > views[m->curtag].nmaster && th != (unsigned int)m->wah) {
			ny = c->y + c->h + 2 * c->border;
		}
	}
}

void
togglestruts(const char *arg) {
	views[curmontag].barpos =
	    (views[curmontag].barpos ==
	    StrutsOn) ? (options.hidebastards ? StrutsHide : StrutsOff) : StrutsOn;
	updategeom(curmonitor());
	arrange(curmonitor());
}

void
togglefloating(const char *arg) {
	if (!sel)
		return;

	if (FEATURES(curlayout, OVERLAP))
		return;

	sel->isfloating = !sel->isfloating;
	updateframe(sel);
	if (sel->isfloating) {
		/* restore last known float dimensions */
		resize(sel, curmonitor(), sel->rx, sel->ry, sel->rw, sel->rh, False);
	} else {
		/* save last known float dimensions */
		save(sel);
	}
	arrange(curmonitor());
}

void
togglefill(const char *arg) {
	XEvent ev;
	Monitor *m = curmonitor();
	Client *c;
	int x1, x2, y1, y2, w, h;

	x1 = m->wax;
	x2 = m->sw;
	y1 = m->way;
	y2 = m->sh;

	if (!sel || sel->isfixed)
		return;
	for(c = clients; c; c = c->next) {
		if(isvisible(c, m) && (c != sel) && !c->isbastard
			       	&& (c->isfloating || MFEATURES(m, OVERLAP))) {
			if(c->y + c->h > sel->y && c->y < sel->y + sel->h) {
				if(c->x < sel->x)
					x1 = max(x1, c->x + c->w + style.border);
				else
					x2 = min(x2, c->x - style.border);
			}
			if(c->x + c->w > sel->x && c->x < sel->x + sel->w) {
				if(c->y < sel->y)
					y1 = max(y1, c->y + c->h + style.border);
				else
					y2 = max(y2, c->y - style.border);
			}
		}
		DPRINTF("x1 = %d x2 = %d y1 = %d y2 = %d\n", x1, x2, y1, y2);
	}
	w = x2 - x1;
	h = y2 - y1;
	DPRINTF("x1 = %d w = %d y1 = %d h = %d\n", x1, w, y1, h);
	if((w < sel->w) || (h < sel->h))
		return;

	if ((sel->isfill = !sel->isfill)) {
		save(sel);
		resize(sel, m, x1, y1, w, h, True);
	} else {
		resize(sel, m, sel->rx, sel->ry, sel->rw, sel->rh, True);
	}
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglemax(const char *arg) {
	XEvent ev;
	Monitor *m = curmonitor();

	if (!sel || sel->isfixed)
		return;
	sel->ismax = !sel->ismax;
	updateframe(sel);
	if (sel->ismax) {
		save(sel);
		resize(sel, m, m->wax - sel->border,
		    m->way - sel->border - sel->th, m->waw, m->wah + sel->th, False);
	} else {
		resize(sel, m, sel->rx, sel->ry, sel->rw, sel->rh, True);
	}
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
toggletag(const char *arg) {
	unsigned int i, j;

	if (!sel)
		return;
	i = idxoftag(arg);
	sel->tags[i] = !sel->tags[i];
	for (j = 0; j < ntags && !sel->tags[j]; j++);
	if (j == ntags)
		sel->tags[i] = True;	/* at least one tag must be enabled */
	drawclient(sel);
	arrange(NULL);
}

void
togglemonitor(const char *arg) {
	Monitor *m, *cm;
	int x, y;

	getpointer(&x, &y);
	for (cm = curmonitor(), m = monitors; m == cm && m && m->next; m = m->next);
	if (!m)
		return;
	XWarpPointer(dpy, None, root, 0, 0, 0, 0, m->sx + x % m->sw, m->sy + y % m->sh);
	focus(NULL);
}

void
toggleview(const char *arg) {
	unsigned int i, j;
	Monitor *m, *cm;

	i = idxoftag(arg);
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
}

void
focusview(const char *arg) {
	Client *c;
	int i;

	toggleview(arg);
	i = idxoftag(arg);
	if (!curseltags[i])
		return;
	for (c = stack; c; c = c->snext) {
		if (c->tags[i] && !c->isbastard) {
			focus(c);
			break;
		}
	}
	restack(curmonitor());
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
unmanage(Client * c) {
	Monitor *m;
	XWindowChanges wc;
	Bool doarrange, dostruts;
	Window trans;

	m = clientmonitor(c);
	doarrange = !(c->isfloating || c->isfixed
	    || XGetTransientForHint(dpy, c->win, &trans)) || c->isbastard;
	dostruts = c->isbastard;
	/* The server grab construct avoids race conditions. */
	XGrabServer(dpy);
	XSelectInput(dpy, c->frame, NoEventMask);
	XUnmapWindow(dpy, c->frame);
	XSetErrorHandler(xerrordummy);
	if (c->title) {
		XftDrawDestroy(c->xftdraw);
		XFreePixmap(dpy, c->drawable);
		XDestroyWindow(dpy, c->title);
		c->title = (Window) NULL;
	}
	XSelectInput(dpy, c->win, CLIENTMASK & ~(StructureNotifyMask | EnterWindowMask));
	XReparentWindow(dpy, c->win, root, c->x, c->y);
	XMoveWindow(dpy, c->win, c->x, c->y);
	if (!running)
		XMapWindow(dpy, c->win);
	wc.border_width = c->oldborder;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);	/* restore border */
	detach(c);
	detachstack(c);
	if (sel == c)
		focus(NULL);
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	setclientstate(c, WithdrawnState);
	XDestroyWindow(dpy, c->frame);
	/* c->tags points to monitor */
	if (!c->isbastard)
		free(c->tags);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	if (dostruts) {
		m->struts[RightStrut] = m->struts[LeftStrut] = m->struts[TopStrut] =
			m->struts[BotStrut] = 0;
		for(c = clients; c; c = c->next)
			if (c->isbastard)
				updatestruts(c);
		updategeom(m);
	}
	if (doarrange) 
		arrange(m);
	updateatom[ClientList] (NULL);
}

void
updategeom(Monitor * m) {
	m->wax = 0;
	m->way = 0;
	m->wah = m->sh;
	m->waw = m->sw;
	switch (views[m->curtag].barpos) {
	default:
		m->wax += m->struts[LeftStrut];
		m->waw -= (m->wax + m->struts[RightStrut]);
		m->way += m->struts[TopStrut];
		m->wah =
		    (int)(m->struts[BotStrut]) ? DisplayHeight(dpy, screen)
		    - m->struts[BotStrut] - m->way : m->sh - m->way;
		break;
	case StrutsHide:
	case StrutsOff:
		break;
	}
}

void
unmapnotify(XEvent * e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = getclient(ev->window, clients, ClientWindow)) /* && ev->send_event */) {
		if (c->ignoreunmap--)
			return;
		DPRINTF("killing self-unmapped window (%s)\n", c->name);
		unmanage(c);
	}
}

void
updateframe(Client * c) {

	if (!c->title)
		return;

	c->th = !c->ismax && (c->isfloating || options.dectiled ||
			MFEATURES(clientmonitor(c), OVERLAP)) ?
			style.titleheight : 0;
	if (!c->th)
		XUnmapWindow(dpy, c->title);
	else
		XMapRaised(dpy, c->title);
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
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
	    && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client * c) {
	if (!gettextprop(c->win, atom[WindowName], c->name, sizeof(c->name)))
		gettextprop(c->win, atom[WMName], c->name, sizeof(c->name));
	drawclient(c);
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
view(const char *arg) {
	int i, j;
	Monitor *m, *cm;

	i = idxoftag(arg);
	cm = curmonitor();

	if (cm->seltags[i])
		return;

	memcpy(cm->prevtags, cm->seltags, ntags * sizeof(cm->seltags[0]));

	for (j = 0; j < ntags; j++)
		cm->seltags[j] = (arg == NULL);
	cm->seltags[i] = True;
	cm->curtag = i;
	for (m = monitors; m; m = m->next) {
		if (m->seltags[i] && m != cm) {
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
}

void
viewlefttag(const char *arg) {
	unsigned int i;

	for (i = 0; i < ntags; i++) {
		if (i && curseltags[i]) {
			view(tags[i - 1]);
			break;
		}
	}
}

void
viewrighttag(const char *arg) {
	unsigned int i;

	for (i = 0; i < ntags - 1; i++) {
		if (curseltags[i]) {
			view(tags[i + 1]);
			break;
		}
	}
}

void
zoom(const char *arg) {
	Client *c;

	if (!sel || !FEATURES(curlayout, ZOOM) || sel->isfloating)
		return;
	if ((c = sel) == nexttiled(clients, curmonitor()))
		if (!(c = nexttiled(c->next, curmonitor())))
			return;
	detach(c);
	attach(c);
	arrange(curmonitor());
	focus(c);
}

int
main(int argc, char *argv[]) {
	char conf[256] = "\0";

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
	cargv = argv;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	checkotherwm();
	setup(conf);
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
