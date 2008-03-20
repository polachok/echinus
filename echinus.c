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
 * handle all data smoothly. The event handlers of echinus are organized in an
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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
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
#include <X11/Xft/Xft.h>

/* macros */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)		(mask & ~(numlockmask | LockMask))
#define MOUSEMASK		(BUTTONMASK | PointerMotionMask)
#define LENGTH(x)		(sizeof x / sizeof x[0])
#define RESNAME                        "echinus"
#define RESCLASS               "Echinus"
#define OPAQUE	0xffffffff
#define ROWS 4
#define COLS 5
#define INITCOLSROWS { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }

/* enums */
enum { BarTop, BarBot, BarOff };			/* bar position */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColButton, ColLast };		/* color */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast };/* default atoms */

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int tx, ty, tw, th; /* title window */
	int rx, ry, rw, rh; /* revert geometry */
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating, isicon, hastitle, hadtitle;
        Bool isplaced;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
	Window title;
};

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
        XftColor *xftnorm;
        XftColor *xftsel;
	Drawable drawable;
        XftDraw *xftdrawable;
	GC gc;
        struct {
           XftFont *xftfont;
           XGlyphInfo *extents;
	   int height;
           int width;
	} font;
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
	Bool hastitle;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

typedef struct {
    Pixmap pm;
    void (*action)(char);
} button;


int px, py;
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
void drawclient(Client *c);
void drawtext(const char *text, unsigned long col[ColLast], Bool center);
void *emallocz(unsigned int size);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void floating(void); /* default floating layout */
void ifloating(void); /* intellectual floating layout */
void focus(Client *c);
void focusin(XEvent *e);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w);
Client *gettitle(Window w);
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
void resize(Client *c, int x, int y, int w, int h, Bool offscreen);
void resizemouse(Client *c);
void resizetitle(Client *c);
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
void saveconfig(Client *c);
void unban(Client *c);
void unmanage(Client *c);
void updatebarpos(void);
void unmapnotify(XEvent *e);
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
float uf_opacity;
int screen, sx, sy, sw, sh, wax, way, waw, wah;
int borderpx;
int cx, cy, cw, ch;
unsigned int nmaster;
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int bh, bpos;
unsigned int blw = 0;
unsigned int numlockmask = 0;
Atom wmatom[WMLast];
Bool domwfact = True;
Bool dozoom = True;
Bool otherwm;
int toph = 0;
int both = 0;
Bool running = True;
Bool selscreen = True;
Bool notitles = False;
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
Display *dpy;
DC dc = {0};
button bleft = {0};
button bcenter = {0};
button bright = {0};
Layout *layout = NULL;
Window root;
Regs *regs = NULL;
XrmDatabase xrdb = NULL;

char terminal[255];

/* configuration, allows nested code to access above variables */
#include "config.h"
#include "ewmh.h"

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
	[FocusIn] = focusin,
	[UnmapNotify] = unmapnotify,
        [ClientMessage] = ewmh_process_client_message,
};


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
			c->hadtitle = rules[i].hastitle;
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

	for(c = clients; c; c = c->next){
		if(isvisible(c) && !c->isicon)
			unban(c);
		else
			ban(c);
        }
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
        XUnmapWindow(dpy, c->win);
        XUnmapWindow(dpy, c->title);
        XMoveWindow(dpy, c->title, c->x + 2 * sw, c->y);
        setclientstate(c, IconicState);
	c->isbanned = True;
}

void
iconify(Client *c) {
        ban(c);
        setclientstate(c, IconicState);
        c->isicon = True;
}

void
drawmouse(XEvent *e) {
    /* it's ugly, i know */
    /* TODO: look at 9wm */
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
                    spawn(terminal);
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
focusin(XEvent *e) {
    Client *c;
    XFocusChangeEvent *ev = &e->xfocus;
    /* TODO: fix this */
#ifdef PYPANELHACK
    if (ev->type == FocusIn){
        c = getclient(ev->window);
        if(c!=sel && c){
            focus(c);
            restack();
        }
    }
    XSync(dpy, False);
#endif
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
        focus(c);
        restack();
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
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
    else if((c = gettitle(ev->window))) {

        if((ev->x > c->tw-3*c->th) && (ev->x < c->tw-2*c->th)){
            /* min */
            iconify(c);
            return;
        }
        focus(c);
        if((ev->x > c->tw-2*c->th) && (ev->x < c->tw-c->th)){
            /* max */
            setlayout(NULL);
            return;
        }
        if((ev->x > c->tw-c->th) && (ev->x < c->tw)){
            /* close */
            killclient(NULL);
            return;
        }
        if(ev->button == Button1) {
            if((layout->arrange == floating) || c->isfloating)
                restack();
            movemouse(c);
            drawclient(c);
        }
        else if(ev->button == Button3 && !c->isfixed) {
            resizemouse(c);
            drawclient(c);
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
		eprint("echinus: another window manager is already running\n");
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
		dc.drawable = XCreatePixmap(dpy, root, sw, dc.h, DefaultDepth(dpy, screen));
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
			if(isvisible(c)){
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
                                drawclient(c);
                        }
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

void
drawtext(const char *text, unsigned long col[ColLast], Bool center) {
	int x, y, w, h;
	static char buf[256];
	unsigned int len, olen;
	XRectangle r = { dc.x, 0, dc.w, dc.h };

	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
	if(!text)
		return;
	w = 0;
	olen = len = strlen(text);
	if(len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = dc.h;
	y = dc.h-dc.font.height/2+1;
	x = dc.x+h/2;
		/* shorten text if necessary */
	while(len && (w = textnw(buf, len)) > dc.w)
		buf[--len] = 0;
	if(len < olen) {
		if(len > 1)
			buf[len - 1] = '.';
		if(len > 2)
			buf[len - 2] = '.';
		if(len > 3)
			buf[len - 3] = '.';
	}
	if(w > dc.w)
		return; /* too long */
        if(center)
                x = dc.x + dc.w/2 - w/2;
        else 
                x = dc.x + h/2;
        while(x <= 0)
                x = dc.x++;
        XftDrawStringUtf8(dc.xftdrawable, (col==dc.norm) ? dc.xftnorm : dc.xftsel,
                dc.font.xftfont, x, y, (unsigned char*)buf, len);
        dc.x = x + w;
}

Pixmap
initpixmap(const char *file) {
    Pixmap pmap;
    pmap = XCreatePixmap(dpy, root, 20, 20, 1);
    unsigned int pw, ph;
    if(BitmapSuccess == XReadBitmapFile(dpy, root, file, &pw, &ph, &pmap, &px, &py))
        return pmap;
    else
        eprint("echinus: cannot load button pixmaps, check your ~/.echinusrc\n");
    return 0;
}

void
initbuttons() {
    XSetForeground(dpy, dc.gc, dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, dc.norm[ColBG]);
    bleft.pm = initpixmap(getresource("button.left.pixmap", BLEFTPIXMAP));
    bright.pm = initpixmap(getresource("button.right.pixmap", BRIGHTPIXMAP));
    bcenter.pm = initpixmap(getresource("button.center.pixmap", BCENTERPIXMAP));
}

void
drawbuttons(Client *c) {
    XSetForeground(dpy, dc.gc, (c == sel) ? dc.sel[ColButton] : dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, (c == sel) ? dc.sel[ColBG] : dc.norm[ColBG]);
    XCopyPlane(dpy, bright.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-c->th, 0, 1);
    XCopyPlane(dpy, bleft.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-3*c->th, 0, 1);
    XCopyPlane(dpy, bcenter.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-2*c->th, 0, 1);
}

void
drawclient(Client *c) {
    unsigned int opacity;
    if(!isvisible(c))
        return;
    resizetitle(c);
    XSetForeground(dpy, dc.gc, dc.norm[ColBG]);
    XFillRectangle(dpy, c->title, dc.gc, 0, 0, c->tw, c->th);
    dc.x = dc.y = 0;
    dc.w = c->w+borderpx;
    dc.h = c->th;
    drawtext(c->name, c == sel ? dc.sel : dc.norm, False);
    if(c->tw>=6*c->th && dc.x <= c->tw-6*c->th)
        drawbuttons(c);
    XCopyArea(dpy, dc.drawable, c->title, dc.gc,
			0, 0, c->tw, c->th+2*borderpx, 0, 0);
    if (c==sel)
      opacity = OPAQUE;
    else
      opacity = (unsigned int) (uf_opacity * OPAQUE);
    ewmh_set_window_opacity(c, opacity);
    XMapWindow(dpy, c->title);
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
    if((c = getclient(ev->window))){
	if(c->isfloating || (layout->arrange == floating))
            focus(c);
        XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                    BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
    }
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
    XExposeEvent *ev = &e->xexpose;
    Client *c;
    if((c=gettitle(ev->window)))
        drawclient(c);
}

static void
initfont(const char *fontstr) {
    dc.font.xftfont = XftFontOpenXlfd(dpy,screen,fontstr);
    if(!dc.font.xftfont)
         dc.font.xftfont = XftFontOpenName(dpy,screen,fontstr);
    if(!dc.font.xftfont)
         eprint("error, cannot load font: '%s'\n", fontstr);
    dc.font.extents = malloc(sizeof(XGlyphInfo));
    XftTextExtentsUtf8(dpy,dc.font.xftfont,(unsigned char*)fontstr, strlen(fontstr), dc.font.extents);
    dc.font.height = dc.font.extents->y+dc.font.extents->yOff;
    dc.font.width = (dc.font.extents->width)/strlen(fontstr);
}

void
floating(void) { /* default floating layout */
	Client *c;
        notitles = False;

	domwfact = dozoom = False;
	for(c = clients; c; c = c->next){
            c->isplaced = False;
            if(isvisible(c) && !c->isicon) {
                    c->hastitle=c->hadtitle;
                    drawclient(c);
                    if(!c->isfloating && !wasfloating)
                            /*restore last known float dimensions*/
                            resize(c, c->sfx, c->sfy, c->sfw, c->sfh, False);
 			else
                            resize(c, c->x, c->y, c->w, c->h, False);
            }
        }
    wasfloating = True;
}

void
focus(Client *c) {
        Client *o;
        o = sel;
	if((!c && selscreen) || (c && !isvisible(c)))
		for(c = stack; c && !isvisible(c); c = c->snext);
	if(sel && sel != c) {
		grabbuttons(sel, False);
		XSetWindowBorder(dpy, sel->win, dc.norm[ColBorder]);
		XSetWindowBorder(dpy, sel->title, dc.norm[ColBorder]);
	}
	if(c) {
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
                unban(c);
	}
	sel = c;
	if(!selscreen)
		return;
	if(c) {
                setclientstate(c, NormalState);
                drawclient(c);
                ewmh_update_net_active_window();
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		XSetWindowBorder(dpy, c->title, dc.sel[ColBorder]);
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        if(o){
            drawclient(o);
        }
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

void
incnmaster(const char *arg) {
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
}

Client *
getclient(Window w) {
	Client *c;

	for(c = clients; c && c->win != w; c = c->next);
	return c;
}

Client *
gettitle(Window w) {
	Client *c;

	for(c = clients; c && c->title != w; c = c->next);
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

	for(c = clients; c; c = c->next){
		if(c->tags[t])
			return True;
        }
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
	XSetWindowAttributes twa;

        if(!ewmh_process_window_type_atom(w)){ // check for a dock window
            XMapWindow(dpy, w); // let it manage itself
            return;
        }
	c = emallocz(sizeof(Client));
	c->tags = emallocz(sizeof seltags);
	c->win = w;
        c->isicon = False;
        c->hastitle = True;
        c->hadtitle = True;

	if(cx && cy && cw && ch) {
		c->x = wa->x = cx;
		c->y = wa->y = cy;
		c->w = wa->width = cw;
		c->h = wa->height = ch;
                cx = cy = cw = ch = 0;
        }
	else {
                c->w = wa->width;
                c->h = wa->height;
                c->x = wa->x;
                c->y = wa->y;
	}
        if(wa->x && wa->y)
            c->isplaced = True;
    
        c->th = dc.h;
	c->tx = c->x = wa->x;
	c->ty = c->y - c->th;
	c->tw = c->w = wa->width;

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
        //ewmh_process_window_type_atom(c);
	wc.border_width = c->border;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	c->sfx = c->x;
	c->sfy = c->y;
	c->sfw = c->w;
	c->sfh = c->h;
	XSelectInput(dpy, w,
		StructureNotifyMask | PropertyChangeMask | EnterWindowMask | FocusChangeMask);
	grabbuttons(c, False);
        twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask | MOUSEMASK;
	//twa.border_width = borderpx;

	c->title = XCreateWindow(dpy, root, c->tx, c->ty, c->tw, c->th,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);

	XConfigureWindow(dpy, c->title, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->title, dc.norm[ColBorder]);

	updatetitle(c);
	if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
		for(t = clients; t && t->win != trans; t = t->next);
	if(t)
		memcpy(c->tags, t->tags, sizeof seltags);
	applyrules(c);
	if(!c->isfloating)
		c->isfloating = (rettrans == Success) || c->isfixed;
	attach(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h); /* some windows require this */
	XMapWindow(dpy, c->win);
	setclientstate(c, IconicState);
        drawclient(c);
        ewmh_update_net_client_list();
        ewmh_update_net_window_desktop(c);
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
        arrange();
}

void
ifloating(void) {
    Client *c;
    int rw, rh;
    int x,y;
    int i,n,k;
    rw = waw/COLS;
    rh = wah/ROWS;
    int cr = 0;
    Bool region[COLS*ROWS]=INITCOLSROWS;
    for(i=0; i<LENGTH(region) && region[i]; i++);
        if(i==LENGTH(region)){
            for(i=LENGTH(region)-1; i>=0; i--){
                region[i] = False;
            }
        }
    for(c = clients; c; c = c->next){
        if(isvisible(c)){
                if(c->isplaced){
                    y = c->y/rh;
                    x = c->x/rw;
                    n = c->w/rw;
                    k = c->h/rh;
                    cr = COLS*y+x;
                    if(n >= (COLS/2+1) && k >= (ROWS/2+1))
                        continue;
                    region[cr]=True;
                    for(i=0; i <= n && cr+i < LENGTH(region); i++){
                        region[cr+i] = True;
                    }
                }
            drawclient(c);
        }
    }
    for(c = clients; c; c = c->next){
        if(isvisible(c)){
            if(!c->isplaced) {
                if(!c->isfloating && !wasfloating){
                            /*restore last known float dimensions*/
                    c->isplaced = True;
                    resize(c, c->sfx, c->sfy, c->sfw, c->sfh, False);
                    drawclient(c);
                    continue;
                }
                if(region[0]){
                    for(cr=LENGTH(region)/2-COLS/2-2; cr < LENGTH(region) && region[cr]; cr++);
                }
                else {
                    for(cr=LENGTH(region)/2-COLS/2-2; cr >= 0 && region[cr]; cr--);
                }
                    /* put in center if it's free */
                    if(cr==LENGTH(region)){
                        region[LENGTH(region)/2]=True;
                        resize(c, sw/2-c->w/2+rand()%4*c->th, sh/2-c->h/2+rand()%4*c->th+c->th,
                                c->w, c->h, False);
                        c->isplaced = True;
                        drawclient(c);
                        continue;
                    }
                    y = c->h/rh;
                    x = c->w/rw;
                    if(y >= (ROWS/2+1) && x >= (COLS/2+1)){  // too big for us
                        c->isplaced = True;
                        drawclient(c);
                        continue;
                    }
                    region[cr] = True;
                    resize(c, wax+(cr%COLS)*rw+rand()%4*c->th, way+(cr/COLS)*rh+rand()%4*c->th+c->th,
                            c->w, c->h, False);
                    c->isplaced = True;
                    }
            drawclient(c);
            }
        }
        focus(NULL);
	restack();
}

void
monocle(void) {
	Client *c;
        wasfloating = False;
	for(c = clients; c; c = c->next){
            c->isplaced = False;
            if(isvisible(c)) {
                if(!c->isfloating){
                    c->sfx = c->x;
                    c->sfy = c->y;
                    c->sfw = c->w;
                    c->sfh = c->h;
                    c->hastitle=False;
                    unban(c);
                }
                else
                    continue;
                if(bpos == BarOff) 
                    resize(c, sx, sy, sw, sh, False);
                else {
                    resize(c, wax-c->border, way, waw, wah, False);
                }
            }
        }
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
				break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == net_wm_name) {
			updatetitle(c);
		}
    }
}

void
quit(const char *arg) {
    if(arg){
	execvp(cargv[0], cargv);
	eprint("Can't exec: %s\n", strerror(errno));
    }
    running = False;
}

void
resize(Client *c, int x, int y, int w, int h, Bool offscreen) {
	XWindowChanges wc;

	if(w <= 0 || h <= 0)
		return;
	/* offscreen appearance fixes */
	if(x > sw)
		x = sw - w - 2 * c->border;
	if(y > sh)
		y = sh - h - 2 * c->border;
        if (offscreen){
            if(x + w > sw)
                    x = sw - w - 2 * c->border;
            if(y + w> sh)
                    y = sh - h - 2 * c->border;
        }
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
    resizetitle(c);
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
	//XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->border - 1, c->h + c->border - 1);
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
				nw = MINWIDTH;
			if((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
				nh = MINHEIGHT;
			resize(c, c->x, c->y, nw, nh, False);
			break;
		}
	}
}

void
resizetitle(Client *c) {
        if(c->isicon)
            return;
	c->tx = c->x;
	c->ty = c->y-c->th;
        c->tw = c->w;
        if(!c->hastitle){
            XMoveWindow(dpy, c->title, c->x + 2 * sw, c->y);
            return;
        }
        XMoveResizeWindow(dpy, c->title, c->tx, c->ty, c->tw, c->th);
}

void
restack(void) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	if(!sel)
		return;
	if(sel->isfloating || (layout->arrange == floating) || (layout->arrange == ifloating)){
		XRaiseWindow(dpy, sel->win);
		XRaiseWindow(dpy, sel->title);
        }
	if(layout->arrange != floating && layout->arrange != ifloating) {
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
        ewmh_update_net_active_window();
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
			if(wa.map_state == IsViewable || getstate(wins[i]) == IconicState || getstate(wins[i]) == NormalState)
				manage(wins[i], &wa);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState || getstate(wins[i]) == NormalState ))
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
        if(state != IconicState)
            c->isicon = False;
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
}

void
setup(void) {
	int d;
	unsigned int i, j, mask;
	Window w;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

	/* init atoms */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMName] = XInternAtom(dpy, "WM_NAME", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
        /* init EWMH atoms */
        ewmh_init_atoms();
        ewmh_set_supported_hints();
        ewmh_update_net_numbers_of_desktop();
        ewmh_update_net_desktop_names();
        ewmh_update_net_current_desktop();
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
        char conf[255];
        sprintf(conf, "%s/%s",getenv("HOME"),"/.echinus");
        chdir(conf);
        xrdb = XrmGetFileDatabase("echinusrc");
        if(!xrdb)
            eprint("echinus: cannot open configuration file\n");

	/* grab keys */
	keypress(NULL);

	/* init tags */
	compileregs();

	/* init appearance */
	dc.norm[ColBorder] = getcolor(getresource("normal.border",NORMBORDERCOLOR));
	dc.norm[ColBG] = getcolor(getresource("normal.bg",NORMBGCOLOR));
	dc.norm[ColFG] = getcolor(getresource("normal.fg",NORMFGCOLOR));
	dc.norm[ColButton] = getcolor(getresource("normal.button",NORMBUTTONCOLOR));

        dc.sel[ColBorder] = getcolor(getresource("selected.border", SELBORDERCOLOR));
	dc.sel[ColBG] = getcolor(getresource("selected.bg", SELBGCOLOR));
	dc.sel[ColFG] = getcolor(getresource("selected.fg", SELFGCOLOR));
	dc.sel[ColButton] = getcolor(getresource("selected.button", SELBUTTONCOLOR));

        dc.xftsel=malloc(sizeof(XftColor));
        dc.xftnorm=malloc(sizeof(XftColor));
        XftColorAllocName(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen),getresource("selected.fg", SELFGCOLOR), dc.xftsel);
        XftColorAllocName(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen),getresource("normal.fg", SELFGCOLOR), dc.xftnorm);
        if(!dc.xftnorm || !dc.xftnorm)
             eprint("error, cannot allocate colors\n");
        initfont(getresource("font",FONT));
        borderpx = atoi(getresource("border", BORDERPX));
        uf_opacity = strtof(getresource("opacity", NF_OPACITY),NULL);

        strncpy(terminal, getresource("terminal", TERMINAL), 255);

        dc.h = atoi(getresource("title", TITLEHEIGHT));
        toph = atoi(getresource("space.top", BARHEIGHT));
        both = atoi(getresource("space.bottom", BARHEIGHT));

	/* init layouts */
	mwfact = MWFACT;
	nmaster = NMASTER;
	layout = &layouts[0];

	/* init bar */
	bpos = BARPOS;
	updatebarpos();
	dc.drawable = XCreatePixmap(dpy, root, sw, dc.h, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);

        /* buttons */
        initbuttons();
        chdir(getenv("HOME"));

        /* free resource database */
        XrmDestroyDatabase(xrdb);
        dc.xftdrawable = XftDrawCreate(dpy, dc.drawable, DefaultVisual(dpy,screen),DefaultColormap(dpy,screen));
        if(!dc.xftdrawable)
             eprint("error, cannot create drawable\n");

	/* multihead support */
	selscreen = XQueryPointer(dpy, root, &w, &w, &d, &d, &d, &d, &mask);

}

void
spawn(const char *arg) {
	static char *shell = NULL;

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

	if(!sel)
		return;
	for(i = 0; i < LENGTH(tags); i++)
		sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
	arrange();
        ewmh_update_net_window_desktop(sel);
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
                c->hastitle = False;
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
                                nx += mc->w + mc->border;
                                nw = waw - nx - c->border;
                        }
                        if(i + 1 == n) /* remainder */
                                nh = (way + wah) - ny - 2 * c->border;
                        else
                                nh = th - 2 * c->border;
                }
                resize(c, nx, ny, nw, nh, False);
                if(n > nmaster && th != wah)
                        ny = c->y + c->h + 2 * c->border;
        }
}

unsigned int
textnw(const char *text, unsigned int len) {
    XftTextExtentsUtf8(dpy,dc.font.xftfont,(unsigned char*)text, strlen(text), dc.font.extents);
    /*if(dc.font.extents->height > dc.font.height)
         dc.font.height = dc.font.extents->height;*/
    if(dc.font.extents->width/len > dc.font.width)
         dc.font.width = dc.font.extents->width/len;
    return dc.font.extents->width;
}

unsigned int
textw(const char *text) {
	return textnw(text, strlen(text)) + 2*dc.font.width;
}

void
togglebar(const char *arg) {
	if(bpos == BarOff)
		bpos = (BARPOS == BarOff) ? BarTop : BARPOS;
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
		resize(sel, sel->sfx, sel->sfy, sel->sfw, sel->sfh, False);
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
	for(c = clients; c; c = c->next){
        if (c->tags[i]) {
                focus(c);
                if((layout->arrange == floating) || c->isfloating)
                    arrange();
                return;
        }
    }

}
void
unban(Client *c) {
	if(!c->isbanned)
		return;
	XMapWindow(dpy, c->win);
	XMapWindow(dpy, c->title);
        setclientstate(c, NormalState);
	c->isbanned = False;

}

void
unmanage(Client *c) {
	XWindowChanges wc;
        XDestroyWindow(dpy, c->title);
        XDestroyWindow(dpy, c->win);
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
        ewmh_update_net_client_list();
}

void
updatebarpos(void) {
	XEvent ev;

	wax = sx;
	way = sy;
	wah = sh;
	waw = sw;
        switch(bpos){
	default:
		wah -= toph;
		way += toph;
		wah -= both;
		break;
	case BarOff:
		break;
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
unmapnotify(XEvent *e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;
        XWindowAttributes wa;

        if((c = getclient(ev->window))) {
            XGetWindowAttributes(dpy, ev->window, &wa);
            if(wa.map_state == IsUnmapped){
                    XGetWindowAttributes(dpy, c->title, &wa);
                    if(wa.map_state == IsViewable)
                        unmanage(c);
            }
        }
}

void
updatetitle(Client *c) {
	if(!gettextprop(c->win, net_wm_name, c->name, sizeof c->name))
		gettextprop(c->win, wmatom[WMName], c->name, sizeof c->name);
        drawclient(c);
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
	fprintf(stderr, "echinus: fatal error: request code=%d, error code=%d\n",
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
        ewmh_update_net_current_desktop();
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
		eprint("echinus-"VERSION", © 2006-2008 Anselm R. Garbe, Sander van Dijk, "
		       "Jukka Salmi, Premysl Hruby, Szabolcs Nagy, Alexander Polakov\n");
	else if(argc != 1)
		eprint("usage: echinus [-v]\n");

	setlocale(LC_CTYPE, "");
	if(!(dpy = XOpenDisplay(0)))
		eprint("echinus: cannot open display\n");
        cargv = argv;
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	checkotherwm();
	setup();
	scan();
	run();
	cleanup();

	XCloseDisplay(dpy);
	return 0;
}
