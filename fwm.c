/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * Calls to fetch an X event from the event queue are blocking.  Due reading
 * status text from standard input, a select()-driven main loop has been
 * implemented which selects for reads on the X connection and STDIN_FILENO to
 * handle all data smoothly. The event handlers of fwm are organized in an
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
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
#define LENGTH(x)		(sizeof x / sizeof x[0])
#define RESNAME                        "fwm"
#define RESCLASS               "Fwm"

/* enums */
enum { BarTop, BarBot, BarOff };			/* bar position */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };		/* color */
enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
};

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	Drawable drawable;
	GC gc;
} DC; /* draw context */

typedef struct {
	unsigned long mod;
	KeySym keysym;
	void (*func)(const char *arg);
	const char *arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(void);
} Layout;

typedef struct {
	const char *prop;
	const char *tags;
	Bool isfloating;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

/* function declarations */
int applyrules(Client *c);
void arrange(void);
void attach(Client *c);
void attachstack(Client *c);
void ban(Client *c);
void buttonpress(XEvent *e);
void checkotherwm(void);
void cleanup(void);
void compileregs(void);
void configure(Client *c);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
void destroynotify(XEvent *e);
void detach(Client *c);
void detachstack(Client *c);
void drawbar(void);
void *emallocz(unsigned int size);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void floating(void); /* default floating layout */
void focus(Client *c);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w);
unsigned long getcolor(const char *colstr);
char *getresource(const char *resource, char *defval);
long getstate(Window w);
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(Client *c, Bool focused);
unsigned int idxoftag(const char *tag);
Bool isoccupied(unsigned int t);
Bool isprotodel(Client *c);
Bool isvisible(Client *c);
void keypress(XEvent *e);
void killclient(const char *arg);
void leavenotify(XEvent *e);
void manage(Window w, XWindowAttributes *wa);
void mappingnotify(XEvent *e);
void monocle(void);
void maprequest(XEvent *e);
void movemouse(Client *c);
Client *nexttiled(Client *c);
Client *prevtiled(Client *c);
void propertynotify(XEvent *e);
void quit(const char *arg);
void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
void resizemouse(Client *c);
void restack(void);
void run(void);
void scan(void);
void setclientstate(Client *c, long state);
void setlayout(const char *arg);
void setmwfact(const char *arg);
void setnmaster(const char *arg);
void setup(void);
void spawn(const char *arg);
void tag(const char *arg);
unsigned int textnw(const char *text, unsigned int len);
unsigned int textw(const char *text);
void tile(void);
void togglebar(const char *arg);
void togglefloating(const char *arg);
void togglemax(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void focusview(const char *arg);
void unban(Client *c);
void unmanage(Client *c);
void unmapnotify(XEvent *e);
void updatebarpos(void);
void updatesizehints(Client *c);
void updatetitle(Client *c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dsply, XErrorEvent *ee);
int xerrorstart(Display *dsply, XErrorEvent *ee);
void zoom(const char *arg);

/* variables */
char **cargv;
char **environ;
Bool wasfloating = True;
double mwfact;
int screen, sx, sy, sw, sh, wax, way, waw, wah;
int borderpx;
int cx, cy, cw, ch;
unsigned int nmaster;
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int bh, bpos;
unsigned int blw = 0;
unsigned int numlockmask = 0;
void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[LeaveNotify] = leavenotify,
	[Expose] = expose,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
Atom wmatom[WMLast], netatom[NetLast];
Bool domwfact = True;
Bool dozoom = True;
Bool otherwm;
Bool running = True;
Bool selscreen = True;
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
Layout *layout = NULL;
Window root;
Regs *regs = NULL;
XrmDatabase xrdb = NULL;
/* configuration, allows nested code to access above variables */
#include "config.h"

Bool prevtags[LENGTH(tags)];

/* function implementations */
int
applyrules(Client *c) {
	static char buf[512];
	unsigned int i, j;
	regmatch_t tmp;
	Bool matched = False;
	XClassHint ch = { 0 };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	snprintf(buf, sizeof buf, "%s:%s:%s",
			ch.res_class ? ch.res_class : "",
			ch.res_name ? ch.res_name : "", c->name);
	for(i = 0; i < LENGTH(rules); i++)
		if(regs[i].propregex && !regexec(regs[i].propregex, buf, 1, &tmp, 0)) {
			c->isfloating = rules[i].isfloating;
			for(j = 0; regs[i].tagregex && j < LENGTH(tags); j++) {
				if(!regexec(regs[i].tagregex, tags[j], 1, &tmp, 0)) {
					matched = True;
					c->tags[j] = True;
				}
			}
		}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	if(!matched) {
		memcpy(c->tags, seltags, sizeof seltags);
        return 0;
    }
    else
        return 1;
}

void
arrange(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c))
			unban(c);
		else
			ban(c);
	layout->arrange();
	focus(NULL);
	restack();
}

void
attach(Client *c) {
	if(clients)
		clients->prev = c;
	c->next = clients;
	clients = c;
}

void
attachstack(Client *c) {
	c->snext = stack;
	stack = c;
}

void
ban(Client *c) {
	if(c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	c->isbanned = True;
}
void
drawmouse(XEvent *e) {
	int x1, y1, ocx, ocy, di, nx, ny, rx, ry, rx1, ry1;
	unsigned int dui;
	Window dummy;
	XButtonPressedEvent *ev = &e->xbutton;
    XEvent ee;

	ocx = nx = ev->x;
	ocy = ny = ev->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK, &ee);
		switch (ee.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
            spawn(TERMINAL);
            cx=ev->x;
            cy=ev->y;
            cw=abs(nx-ev->x);
            ch=abs(ny-ev->y);
            XClearArea(dpy, root, ev->x-4, ev->y-4, rx1+4, ry1+4, False);
			return;
		case MotionNotify:
			XSync(dpy, False);
			nx = ocx + (ee.xmotion.x - x1);
			ny = ocy + (ee.xmotion.y - y1);
            XSetLineAttributes(dpy, dc.gc, 4, LineSolid, CapNotLast, JoinMiter);
            if(rx > nx)
              rx1 = rx;
            else
              rx1 = nx;
            if(ry > ny)
              ry1 = ry;
            else
              ry1 = ny;
            XClearArea(dpy, root, ev->x-4, ev->y-4, rx1+4, ry1+4, False);
            XDrawRectangle(dpy, root, dc.gc, ev->x, ev->y, abs(nx-ev->x), abs(ny-ev->y));
            rx = nx;
            ry = ny;
            XSync(dpy, False);
			break;
		}
	}
}
void
buttonpress(XEvent *e) {
	Client *c;
	XButtonPressedEvent *ev = &e->xbutton;
    int i;

    if(ev->window == root) {
            switch(ev->button) {
                case Button3:
                    XSetForeground(dpy, dc.gc, dc.sel[ColBorder]);
                    drawmouse(e);
                    break;
                case Button4:
                    for(i=0; i <= LENGTH(tags); i++) {
                        if(i && seltags[i]) {
                                view(tags[i-1]);
                                break;
                        }
                    }
                    break;
                case Button5:
                    for(i=0; i < LENGTH(tags); i++) {
                        if(seltags[i]) {
                            view(tags[i+1]);
                            break;
                        }
                    }
                    break;
            }
            return;
    }
    if((c = getclient(ev->window))) {
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
		focus(c);
        if(CLEANMASK(ev->state) != MODKEY)
           return;
		if(ev->button == Button1) {
			if((layout->arrange == floating) || c->isfloating)
				restack();
			movemouse(c);
		}
		else if(ev->button == Button2) {
			if((floating != layout->arrange) && c->isfloating)
				togglefloating(NULL);
			else
				zoom(NULL);
		}
		else if(ev->button == Button3 && !c->isfixed) {
			if((floating == layout->arrange) || c->isfloating)
				restack();
			else
				togglefloating(NULL);
			resizemouse(c);
		}
	}
}

void
checkotherwm(void) {
	otherwm = False;
	XSetErrorHandler(xerrorstart);

	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XSync(dpy, False);
	if(otherwm)
		eprint("fwm: another window manager is already running\n");
	XSync(dpy, False);
	XSetErrorHandler(NULL);
	xerrorxlib = XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	while(stack) {
		unban(stack);
		unmanage(stack);
	}
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreePixmap(dpy, dc.drawable);
	XFreeGC(dpy, dc.gc);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XSync(dpy, False);
}

void
compileregs(void) {
	unsigned int i;
	regex_t *reg;

	if(regs)
		return;
	regs = emallocz(LENGTH(rules) * sizeof(Regs));
	for(i = 0; i < LENGTH(rules); i++) {
		if(rules[i].prop) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].prop, REG_EXTENDED))
				free(reg);
			else
				regs[i].propregex = reg;
		}
		if(rules[i].tags) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, rules[i].tags, REG_EXTENDED))
				free(reg);
			else
				regs[i].tagregex = reg;
		}
	}
}

void
configure(Client *c) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->border;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e) {
	XConfigureEvent *ev = &e->xconfigure;

	if(ev->window == root && (ev->width != sw || ev->height != sh)) {
		sw = ev->width;
		sh = ev->height;
		XFreePixmap(dpy, dc.drawable);
		dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
		updatebarpos();
		arrange();
	}
}

void
configurerequest(XEvent *e) {
	Client *c;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = getclient(ev->window))) {
		c->ismax = False;
		if(ev->value_mask & CWBorderWidth)
			c->border = ev->border_width;
		if(c->isfixed || c->isfloating || (floating == layout->arrange)) {
			if(ev->value_mask & CWX)
				c->x = ev->x;
			if(ev->value_mask & CWY)
				c->y = ev->y;
			if(ev->value_mask & CWWidth)
				c->w = ev->width;
			if(ev->value_mask & CWHeight)
				c->h = ev->height;
			if((c->x + c->w) > sw && c->isfloating)
				c->x = sw / 2 - c->w / 2; /* center in x direction */
			if((c->y + c->h) > sh && c->isfloating)
				c->y = sh / 2 - c->h / 2; /* center in y direction */
			if((ev->value_mask & (CWX | CWY))
			&& !(ev->value_mask & (CWWidth | CWHeight)))
				configure(c);
			if(isvisible(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		}
		else
			configure(c);
	}
	else {
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
destroynotify(XEvent *e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = getclient(ev->window)))
		unmanage(c);
}

void
detach(Client *c) {
	if(c->prev)
		c->prev->next = c->next;
	if(c->next)
		c->next->prev = c->prev;
	if(c == clients)
		clients = c->next;
	c->next = c->prev = NULL;
}

void
detachstack(Client *c) {
	Client **tc;

	for(tc=&stack; *tc && *tc != c; tc=&(*tc)->snext);
	*tc = c->snext;
}

void drawbar()
{
    if(sel)
    {
        write(STDOUT_FILENO, sel->name, 100);
        write(STDOUT_FILENO, "\n", 1);
    }
}

void *
emallocz(unsigned int size) {
	void *res = calloc(1, size);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
}

void
enternotify(XEvent *e) {
	XCrossingEvent *ev = &e->xcrossing;
    Client *c;

	if(ev->mode != NotifyNormal || ev->detail == NotifyInferior)
		return;
    if((c = getclient(ev->window)))
        XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
	else if(ev->window == root) {
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
expose(XEvent *e) {
	/*XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0) {
		if(ev->window == barwin)
			drawbar();
	}
    */
}

void
floating(void) { /* default floating layout */
	Client *c;

	domwfact = dozoom = False;
	for(c = clients; c; c = c->next)
 		if(isvisible(c))
 		{
 			if(!c->isfloating && !wasfloating)
 				/*restore last known float dimensions*/
 				resize(c, c->sfx, c->sfy, c->sfw, c->sfh, False);
 			else
 				resize(c, c->x, c->y, c->w, c->h, False);
 		}
    wasfloating = True;
}

void
focus(Client *c) {
	if((!c && selscreen) || (c && !isvisible(c)))
		for(c = stack; c && !isvisible(c); c = c->snext);
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
	}
	sel = c;
	drawbar();
	if(!selscreen)
		return;
	if(c) {
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);

}

void
focusnext(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->next; c && !isvisible(c); c = c->next);
	if(!c)
		for(c = clients; c && !isvisible(c); c = c->next);
	if(c) {
		focus(c);
		restack();
	}
}

void
focusprev(const char *arg) {
	Client *c;

	if(!sel)
		return;
	for(c = sel->prev; c && !isvisible(c); c = c->prev);
	if(!c) {
		for(c = clients; c && c->next; c = c->next);
		for(; c && !isvisible(c); c = c->prev);
	}
	if(c) {
		focus(c);
		restack();
	}
}

Client *
getclient(Window w) {
	Client *c;

	for(c = clients; c && c->win != w; c = c->next);
	return c;
}

unsigned long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		eprint("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

long
getstate(Window w) {
	int format, status;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	status = XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
			&real, &format, &n, &extra, (unsigned char **)&p);
	if(status != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}

char *
getresource(const char *resource, char *defval) {
       static char name[256], class[256], *type;
       XrmValue value;
       snprintf(name, sizeof(name), "%s.%s", RESNAME, resource);
       snprintf(class, sizeof(class), "%s.%s", RESCLASS, resource);
       XrmGetResource(xrdb, name, class, &type, &value);
       if(value.addr)
               return value.addr;
       return defval;
}

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!text || size == 0)
		return False;
	text[0] = '\0';
	XGetTextProperty(dpy, w, &name, atom);
	if(!name.nitems)
		return False;
	if(name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
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
grabbuttons(Client *c, Bool focused) {
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	if(focused) {
		XGrabButton(dpy, Button1, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button1, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button2, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button2, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);

		XGrabButton(dpy, Button3, MODKEY, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
		XGrabButton(dpy, Button3, MODKEY | numlockmask | LockMask, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
	}
	else
		XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
				GrabModeAsync, GrabModeSync, None, None);
}

unsigned int
idxoftag(const char *tag) {
	unsigned int i;

	for(i = 0; (i < LENGTH(tags)) && (tags[i] != tag); i++);
	return (i < LENGTH(tags)) ? i : 0;
}

Bool
isoccupied(unsigned int t) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(c->tags[t])
			return True;
	return False;
}

Bool
isprotodel(Client *c) {
	int i, n;
	Atom *protocols;
	Bool ret = False;

	if(XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		for(i = 0; !ret && i < n; i++)
			if(protocols[i] == wmatom[WMDelete])
				ret = True;
		XFree(protocols);
	}
	return ret;
}

Bool
isvisible(Client *c) {
	unsigned int i;

	for(i = 0; i < LENGTH(tags); i++)
		if(c->tags[i] && seltags[i])
			return True;
	return False;
}

void
keypress(XEvent *e) {
	KEYS
	unsigned int i;
	KeyCode code;
	KeySym keysym;
	XKeyEvent *ev;

	if(!e) { /* grabkeys */
		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for(i = 0; i < LENGTH(keys); i++) {
			code = XKeysymToKeycode(dpy, keys[i].keysym);
			XGrabKey(dpy, code, keys[i].mod, root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[i].mod | LockMask, root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[i].mod | numlockmask, root, True,
					GrabModeAsync, GrabModeAsync);
			XGrabKey(dpy, code, keys[i].mod | numlockmask | LockMask, root, True,
					GrabModeAsync, GrabModeAsync);
		}
		return;
	}
	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < LENGTH(keys); i++)
		if(keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state))
		{
			if(keys[i].func)
				keys[i].func(keys[i].arg);
		}
}

void
killclient(const char *arg) {
	XEvent ev;

	if(!sel)
		return;
	if(isprotodel(sel)) {
		ev.type = ClientMessage;
		ev.xclient.window = sel->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = wmatom[WMDelete];
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, sel->win, False, NoEventMask, &ev);
	}
	else
		XKillClient(dpy, sel->win);
}

void
leavenotify(XEvent *e) {
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->window == root) && !ev->same_screen) {
		selscreen = False;
		focus(NULL);
	}
}

void
manage(Window w, XWindowAttributes *wa) {
	Client *c, *t = NULL;
	Window trans;
	Status rettrans;
	XWindowChanges wc;
    Bool r = False;

	c = emallocz(sizeof(Client));
	c->tags = emallocz(sizeof seltags);
	c->win = w;
	if(cx && cy && cw && ch) {
		c->x = wa->x = cx;
		c->y = wa->y = cy;
		c->w = wa->width = cw;
		c->h = wa->height = ch;
        cx = cy = cw = ch = 0;
    }
	else
    {
	c->x = wa->x;
	c->y = wa->y;
	c->w = wa->width;
	c->h = wa->height;
    }
	c->oldborder = wa->border_width;
	if(c->w == sw && c->h == sh) {
		c->x = sx;
		c->y = sy;
		c->border = wa->border_width;
	}
	else {
		if(c->x + c->w + 2 * c->border > wax + waw)
			c->x = wax + waw - c->w - 2 * c->border;
		if(c->y + c->h + 2 * c->border > way + wah)
			c->y = way + wah - c->h - 2 * c->border;
		if(c->x < wax)
			c->x = wax;
		if(c->y < way)
			c->y = way;
		c->border = borderpx;
	}
	wc.border_width = c->border;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);
	c->sfx = c->x;
	c->sfy = c->y;
	c->sfw = c->w;
	c->sfh = c->h;
	XSelectInput(dpy, w,
		StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	grabbuttons(c, False);
	updatetitle(c);
	if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
		for(t = clients; t && t->win != trans; t = t->next);
	if(t)
		memcpy(c->tags, t->tags, sizeof seltags);
	r=applyrules(c);
	if(!c->isfloating)
		c->isfloating = (rettrans == Success) || c->isfixed;
	attach(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h); /* some windows require this */
	ban(c);
	XMapWindow(dpy, c->win);
	setclientstate(c, NormalState);
	arrange();
}

void
mappingnotify(XEvent *e) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		keypress(NULL);
}

void
maprequest(XEvent *e) {
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if(wa.override_redirect)
		return;
	if(!getclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(void) {
	Client *c;

	for(c = clients; c; c = c->next)
		if(isvisible(c)) {
			unban(c);
			if (c->isfloating)
				continue;
            if(bpos == BarOff) 
                resize(c, wax-c->border, way-c->border, waw, wah, False);
            else 
                resize(c, wax, 0, waw - 2*c->border, wah-1, False);
		}
		else 
			ban(c);
	focus(NULL);
	restack();
}

void
movemouse(Client *c) {
	int x1, y1, ocx, ocy, di, nx, ny;
	unsigned int dui;
	Window dummy;
	XEvent ev;

	ocx = nx = c->x;
	ocy = ny = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
	XQueryPointer(dpy, root, &dummy, &dummy, &x1, &y1, &di, &di, &dui);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ButtonRelease:
			XUngrabPointer(dpy, CurrentTime);
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			nx = ocx + (ev.xmotion.x - x1);
			ny = ocy + (ev.xmotion.y - y1);
			if(abs(wax + nx) < SNAP)
				nx = wax;
			else if(abs((wax + waw) - (nx + c->w + 2 * c->border)) < SNAP)
				nx = wax + waw - c->w - 2 * c->border;
			if(abs(way - ny) < SNAP)
				ny = way;
			else if(abs((way + wah) - (ny + c->h + 2 * c->border)) < SNAP)
				ny = way + wah - c->h - 2 * c->border;
			resize(c, nx, ny, c->w, c->h, False);
			break;
		}
	}
}




Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !isvisible(c)); c = c->next);
	return c;
}

Client *
prevtiled(Client *c) {
	for(; c && (c->isfloating || !isvisible(c)); c = c->prev);
	return c;
}
void
propertynotify(XEvent *e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if(ev->state == PropertyDelete)
		return; /* ignore */
	if((c = getclient(ev->window))) {
		switch (ev->atom) {
			default: break;
			case XA_WM_TRANSIENT_FOR:
				XGetTransientForHint(dpy, c->win, &trans);
				if(!c->isfloating && (c->isfloating = (getclient(trans) != NULL)))
					arrange();
				break;
			case XA_WM_NORMAL_HINTS:
				updatesizehints(c);
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if(c == sel)
				drawbar();
		}
	}
}

void
quit(const char *arg) {
	running = False;
    spawn("fwm");
}


void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
	XWindowChanges wc;

	if(sizehints) {
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
		if(c->incw)
			w -= w % c->incw;
		if(c->inch)
			h -= h % c->inch;

		/* restore base dimensions */
		w += c->basew;
		h += c->baseh;

		if(c->minw > 0 && w < c->minw)
			w = c->minw;
		if(c->minh > 0 && h < c->minh)
			h = c->minh;
		if(c->maxw > 0 && w > c->maxw)
			w = c->maxw;
		if(c->maxh > 0 && h > c->maxh)
			h = c->maxh;
	}
	if(w <= 0 || h <= 0)
		return;
	/* offscreen appearance fixes */
	if(x > sw)
		x = sw - w - 2 * c->border;
	if(y > sh)
		y = sh - h - 2 * c->border;
	if(x + w + 2 * c->border < sx)
		x = sx;
	if(y + h + 2 * c->border < sy)
		y = sy;
	if(c->x != x || c->y != y || c->w != w || c->h != h) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = w;
		c->h = wc.height = h;
		wc.border_width = c->border;
		XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
		configure(c);
		XSync(dpy, False);
	}
}

void
resizemouse(Client *c) {
	int ocx, ocy;
	int nw, nh;
	XEvent ev;

	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
			None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	c->ismax = False;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
	for(;;) {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask , &ev);
		switch(ev.type) {
		case ButtonRelease:
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
					c->w + c->border - 1, c->h + c->border - 1);
			XUngrabPointer(dpy, CurrentTime);
			while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
			return;
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			XSync(dpy, False);
			if((nw = ev.xmotion.x - ocx - 2 * c->border + 1) <= 0)
				nw = 1;
			if((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
				nh = 1;
			resize(c, c->x, c->y, nw, nh, True);
			break;
		}
	}
}

void
restack(void) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar();
	if(!sel)
		return;
	if(sel->isfloating || (layout->arrange == floating))
		XRaiseWindow(dpy, sel->win);
	if(layout->arrange != floating) {
		wc.stack_mode = Below;
		if(!sel->isfloating) {
			XConfigureWindow(dpy, sel->win, CWSibling | CWStackMode, &wc);
			wc.sibling = sel->win;
		}
		for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
			if(c == sel)
				continue;
			XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
			wc.sibling = c->win;
		}
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
	//char *p;
	fd_set rd;
	int xfd;
	XEvent ev;

	/* main event loop, also reads status text from stdin */
	XSync(dpy, False);
	xfd = ConnectionNumber(dpy);
	while(running) {
        FD_ZERO(&rd);
		FD_SET(xfd, &rd);
		if(select(xfd + 1, &rd, NULL, NULL, NULL) == -1) {
			if(errno == EINTR)
				continue;
			eprint("select failed\n");
		}
		while(XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if(handler[ev.type])
				(handler[ev.type])(&ev); /* call handler */
		}
	}
}

void
scan(void) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	wins = NULL;
	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
	}
	if(wins)
		XFree(wins);
}

void
setclientstate(Client *c, long state) {
	long data[] = {state, None};

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

void
setlayout(const char *arg) {
	unsigned int i;

	if(!arg) {
		if(++layout == &layouts[LENGTH(layouts)])
			layout = &layouts[0];
	}
	else {
		for(i = 0; i < LENGTH(layouts); i++)
			if(!strcmp(arg, layouts[i].symbol))
				break;
		if(i == LENGTH(layouts))
			return;
		layout = &layouts[i];
	}
	if(sel)
		arrange();
	else
		drawbar();
}

void
setmwfact(const char *arg) {
	double delta;

	if(!domwfact)
		return;
	/* arg handling, manipulate mwfact */
	if(arg == NULL)
		mwfact = MWFACT;
	else if(sscanf(arg, "%lf", &delta) == 1) {
		if(arg[0] == '+' || arg[0] == '-')
			mwfact += delta;
		else
			mwfact = delta;
		if(mwfact < 0.1)
			mwfact = 0.1;
		else if(mwfact > 0.9)
			mwfact = 0.9;
	}
	arrange();
}

void
setnmaster(const char *arg) {
	int i;

	if(layout->arrange != tile)
		return;
	if(!arg)
		nmaster = NMASTER;
	else {
		i = atoi(arg);
		if((nmaster + i) < 1 || wah / (nmaster + i) <= 2 * borderpx)
			return;
		nmaster += i;
	}
	if(sel)
		arrange();
	else
		drawbar();
}

void
setup(void) {
	int d;
	unsigned int i, j, mask;
    char *s;
	Window w;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMName] = XInternAtom(dpy, "WM_NAME", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);

	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

	/* init geometry */
	sx = sy = 0;
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);

	cx = cy = cw = ch = 0;
	/* init modifier map */
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
			== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	XFreeModifiermap(modmap);

	/* select for events */
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
		| EnterWindowMask | LeaveWindowMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask;
	wa.cursor = cursor[CurNormal];
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);

    /* init resource database */
    XrmInitialize();
    s = XResourceManagerString(dpy);
    if(s) {
        xrdb = XrmGetStringDatabase(s);
        free(s);
    }

	/* grab keys */
	keypress(NULL);

	/* init tags */
	compileregs();

	/* init appearance */
    dc.norm[ColBorder] = getcolor(getresource("normal.border",NORMBORDERCOLOR));
    dc.sel[ColBorder] = getcolor(getresource("selected.border", SELBORDERCOLOR));
    borderpx = atoi(getresource("border", BORDERPX));

    dc.h = bh = BARHEIGHT;

	/* init layouts */
	mwfact = MWFACT;
	nmaster = NMASTER;
	layout = &layouts[0];

	/* init bar */
	bpos = BARPOS;
	updatebarpos();
	dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);

    /* free resource database */
    XrmDestroyDatabase(xrdb);

	/* multihead support */
	selscreen = XQueryPointer(dpy, root, &w, &w, &d, &d, &d, &d, &mask);
}

void
spawn(const char *arg) {
	static char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	if(!arg)
		return;
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			if(dpy)
				close(ConnectionNumber(dpy));
			setsid();
			execl(shell, shell, "-c", arg, (char *)NULL);
			fprintf(stderr, "fwm: execl '%s -c %s'", shell, arg);
			perror(" failed");
		}
		exit(0);
	}
	wait(0);
}

void
tag(const char *arg) {
	unsigned int i;

	if(!sel)
		return;
	for(i = 0; i < LENGTH(tags); i++)
		sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
	arrange();
}

void
tile(void) {
	unsigned int i, n, nx, ny, nw, nh, mw, mh, th;
	Client *c, *mc;

    wasfloating = False;

	domwfact = dozoom = True;
	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	/* window geoms */
	mh = (n <= nmaster) ? wah / (n > 0 ? n : 1) : wah / nmaster;
	mw = (n <= nmaster) ? waw : mwfact * waw;
	th = (n > nmaster) ? wah / (n - nmaster) : 0;
	if(n > nmaster && th < bh)
		th = wah;

	nx = wax;
	ny = way;
	nw = 0; /* gcc stupidity requires this */
	for(i = 0, c = mc = nexttiled(clients); c; c = nexttiled(c->next), i++) {
		c->ismax = False;
        c->sfx = c->x;
        c->sfy = c->y;
        c->sfw = c->w;
        c->sfh = c->h;
		if(i < nmaster) { /* master */
			ny = way + i * mh;
			nw = mw - 2 * c->border;
			nh = wah - 2 * c->border;
			nh = mh;
			if(i + 1 == (n < nmaster ? n : nmaster)) /* remainder */
				nh = wah - mh * i;
			nh -= 2 * c->border;
		}
		else {  /* tile window */
			if(i == nmaster) {
				ny = way;
				nx += mc->w + 2 * mc->border;
				nw = waw - nx - 2 * c->border;
			}
			if(i + 1 == n) /* remainder */
				nh = (way + wah) - ny - 2 * c->border;
			else
				nh = th - 2 * c->border;
		}
		resize(c, nx, ny, nw, nh, RESIZEHINTS);
		if((RESIZEHINTS) && ((c->h < bh) || (c->h > nh) || (c->w < bh) || (c->w > nw)))
			/* client doesn't accept size constraints */
			resize(c, nx, ny, nw, nh, False);
		if(n > nmaster && th != wah)
			ny = c->y + c->h + 2 * c->border;
	}
}

void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarBot : BARPOS;
	else
		bpos = BarOff;
	updatebarpos();
	arrange();
}

void
togglefloating(const char *arg) {
	if(!sel)
		return;
	sel->isfloating = !sel->isfloating;
	if(sel->isfloating)
		/*restore last known float dimensions*/
		resize(sel, sel->sfx, sel->sfy, sel->sfw, sel->sfh, True);
	else {
		/*save last known float dimensions*/
		sel->sfx = sel->x;
		sel->sfy = sel->y;
		sel->sfw = sel->w;
		sel->sfh = sel->h;
	}
	arrange();
}

void
togglemax(const char *arg) {
	XEvent ev;

	if(!sel || sel->isfixed)
		return;
	if((sel->ismax = !sel->ismax)) {
		if((layout->arrange == floating) || sel->isfloating)
			sel->wasfloating = True;
		else {
			togglefloating(NULL);
			sel->wasfloating = False;
		}
		sel->rx = sel->x;
		sel->ry = sel->y;
		sel->rw = sel->w;
		sel->rh = sel->h;
		resize(sel, wax, way, waw - 2 * sel->border, sh - 2 * sel->border, False);
	}
	else {
		resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, False);
		if(!sel->wasfloating)
			togglefloating(NULL);
	}
	drawbar();
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
toggletag(const char *arg) {
	unsigned int i, j;

	if(!sel)
		return;
	i = idxoftag(arg);
	sel->tags[i] = !sel->tags[i];
	for(j = 0; j < LENGTH(tags) && !sel->tags[j]; j++);
	if(j == LENGTH(tags))
		sel->tags[i] = True; /* at least one tag must be enabled */
	arrange();
}

void
toggleview(const char *arg) {
	unsigned int i, j;

	i = idxoftag(arg);
	seltags[i] = !seltags[i];
	for(j = 0; j < LENGTH(tags) && !seltags[j]; j++);
	if(j == LENGTH(tags))
		seltags[i] = True; /* at least one tag must be viewed */
	arrange();
}
void
focusview(const char *arg) {
	Client *c;
	int i;
	toggleview(arg);
	i = idxoftag(arg);
	if (!seltags[i])
		return;
	for(c = clients; c; c = c->next)
    if (c->tags[i]) {
            focus(c);
			if((layout->arrange == floating) || c->isfloating)
                arrange();
    }

}
void
unban(Client *c) {
	if(!c->isbanned)
		return;
	XMoveWindow(dpy, c->win, c->x, c->y);
	c->isbanned = False;
}

void
unmanage(Client *c) {
	XWindowChanges wc;

	wc.border_width = c->oldborder;
	/* The server grab construct avoids race conditions. */
	XGrabServer(dpy);
	XSetErrorHandler(xerrordummy);
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
	detach(c);
	detachstack(c);
	if(sel == c)
		focus(NULL);
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	setclientstate(c, WithdrawnState);
	free(c->tags);
	free(c);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
	arrange();
}

void
unmapnotify(XEvent *e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = getclient(ev->window)))
		unmanage(c);
}

void
updatebarpos(void) {
	XEvent ev;

	wax = sx;
	way = sy;
	wah = sh;
	waw = sw;
	switch(bpos) {
	default:
		wah -= bh;
		way += bh;
		break;
	case BarBot:
		wah -= bh;
		break;
	case BarOff:
		break;
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
updatesizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if(c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else if(c->flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	}
	else
		c->basew = c->baseh = 0;
	if(c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else if(c->flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	}
	else
		c->minw = c->minh = 0;
	if(c->flags & PAspect) {
		c->minax = size.min_aspect.x;
		c->maxax = size.max_aspect.x;
		c->minay = size.min_aspect.y;
		c->maxay = size.max_aspect.y;
	}
	else
		c->minax = c->maxax = c->minay = c->maxay = 0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
			&& c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client *c) {
	if(!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, wmatom[WMName], c->name, sizeof c->name);
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which may call exit.  */
int
xerror(Display *dpy, XErrorEvent *ee) {
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "fwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dsply, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dsply, XErrorEvent *ee) {
	otherwm = True;
	return -1;
}

void
view(const char *arg) {
	unsigned int i;

	memcpy(prevtags, seltags, sizeof seltags);
	for(i = 0; i < LENGTH(tags); i++)
		seltags[i] = (NULL == arg);
	seltags[idxoftag(arg)] = True;
	arrange();
}

void
viewprevtag(const char *arg) {
	static Bool tmptags[sizeof tags / sizeof tags[0]];

	memcpy(tmptags, seltags, sizeof seltags);
	memcpy(seltags, prevtags, sizeof seltags);
	memcpy(prevtags, tmptags, sizeof seltags);
	arrange();
}

void
zoom(const char *arg) {
	Client *c;

	if(!sel || !dozoom || sel->isfloating)
		return;
	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next)))
			return;
	detach(c);
	attach(c);
	focus(c);
	arrange();
}

int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-v", argv[1]))
		eprint("fwm-"VERSION", © 2006-2007 Anselm R. Garbe, Sander van Dijk, "
		       "Jukka Salmi, Premysl Hruby, Szabolcs Nagy, Alexander Polakov\n");
	else if(argc != 1)
		eprint("usage: fwm [-v]\n");

	setlocale(LC_CTYPE, "");
	if(!(dpy = XOpenDisplay(0)))
		eprint("fwm: cannot open display\n");
        cargv = argv;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	checkotherwm();
	setup();
	drawbar();
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
