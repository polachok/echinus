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
#include <ctype.h>
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
#include <X11/XF86keysym.h>
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
#define DPRINT fprintf(stderr, "%s: %s() %d\n",__FILE__,__func__, __LINE__);

/* enums */
enum { LeftStrut, RightStrut, TopStrut, BotStrut, LastStrut };
enum { StrutsOn, StrutsOff, StrutsHide };			/* struts position */
enum { TitleLeft, TitleCenter, TitleRight };			/* title position */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColButton, ColLast };		/* color */
enum { WMProtocols, WMDelete, WMName, WMState, WMLast }; /* default atom */

/* typedefs */
typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int th; /* title window */
	int rx, ry, rw, rh; /* revert geometry */
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	unsigned int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating, isicon, hastitle;
        Bool isplaced, isbastard;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
	Window title;
        Window frame;
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
    void (*action)(const char *arg);
} button;


int px, py;
/* function declarations */
void applyrules(Client *c);
void arrange(void);
void attach(Client *c);
void attachstack(Client *c);
void ban(Client *c);
void buttonpress(XEvent *e);
void bstack(void);
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
void drawtext(const char *text, unsigned long col[ColLast], unsigned int position);
void *emallocz(unsigned int size);
void enternotify(XEvent *e);
void eprint(const char *errstr, ...);
void expose(XEvent *e);
void floating(void); /* default floating layout */
void ifloating(void); /* intellectual floating layout try */
void iconifyit(const char *arg);
void incnmaster(const char *arg);
void focus(Client *c);
void focusnext(const char *arg);
void focusprev(const char *arg);
Client *getclient(Window w, Client *list, Bool title);
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
void restart(const char *arg);
void resize(Client *c, int x, int y, int w, int h, Bool sizehints);
void resizemouse(Client *c);
void restack(void);
void run(void);
void scan(void);
void setclientstate(Client *c, long state);
void setlayout(const char *arg);
void setmwfact(const char *arg);
void setup(void);
void spawn(const char *arg);
void tag(const char *arg);
unsigned int textnw(const char *text, unsigned int len);
unsigned int textw(const char *text);
void tile(void);
void togglestruts(const char *arg);
void togglefloating(const char *arg);
void togglemax(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void focusview(const char *arg);
void saveconfig(Client *c);
void unban(Client *c);
void unmanage(Client *c);
void updategeom(void);
void unmapnotify(XEvent *e);
void updatesizehints(Client *c);
void updatetitle(Client *c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
void viewlefttag(const char *arg);
void viewrighttag(const char *arg);
int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dsply, XErrorEvent *ee);
int xerrorstart(Display *dsply, XErrorEvent *ee);
void zoom(const char *arg);

/* variables */
char **cargv;
char **environ;
Bool wasfloating = True;
float uf_opacity;
int screen, sx, sy, sw, sh, wax, way, waw, wah;
int borderpx;
int cx, cy, cw, ch;
unsigned int *nmasters;
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int bh, tpos, tbpos;
unsigned int blw = 0;
unsigned int curtag = 0;
unsigned int numlockmask = 0;
Atom wmatom[WMLast];
Bool domwfact = True;
Bool dozoom = True;
Bool otherwm;
Bool running = True;
Bool selscreen = True;
Bool notitles = False;
Client *clients = NULL;
Client *sel = NULL;
Client *stack = NULL;
Cursor cursor[CurLast];
unsigned long struts[LastStrut];
Display *dpy;
DC dc = {0};
button bleft = {0};
button bcenter = {0};
button bright = {0};
Window root;
Regs *regs = NULL;
XrmDatabase xrdb = NULL;

unsigned int *bpos;
unsigned int *ltidxs;
double *mwfacts;

char terminal[255];
char **tags;
Key **keys;
Rule **rules;
Bool *seltags;
int ntags = 0;
int nkeys = 0;
int nrules = 0;
Bool dectiled = 0;
Bool hidebastards = 0;
/* configuration, allows nested code to access above variables */
#include "config.h"
#include "ewmh.c"
#include "parse.c"
#include "draw.c"

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
	[UnmapNotify] = unmapnotify,
        [ClientMessage] = clientmessage,
};


Bool *prevtags;

/* function implementations */
void
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
    for(i = 0; i < nrules; i++)
            if(regs[i].propregex && !regexec(regs[i].propregex, buf, 1, &tmp, 0)) {
                    c->isfloating = rules[i]->isfloating;
                    c->hastitle = rules[i]->hastitle;
                    for(j = 0; regs[i].tagregex && j < ntags; j++) {
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
            memcpy(c->tags, seltags, ntags*(sizeof seltags));
    }
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

    if(bpos[curtag] == StrutsOn) {
        for(c = clients ; c; c = c->next)
            if(c->isbastard)
                unban(c);
    } else if(bpos[curtag] == StrutsHide) {
        for(c = clients ; c; c = c->next)
            if(c->isbastard)
                ban(c);
    }

    layouts[ltidxs[curtag]].arrange();
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
    Client *t;
    if(stack && c->isbastard){
        for(t = stack; t->snext; t = t->snext);
        t->snext = c;
        c->snext = NULL;
        return;
    }
    c->snext = stack;
    stack = c;
}

void
ban(Client *c) {
    if(c->isbanned)
            return;
    XMoveResizeWindow(dpy, c->frame, c->x+2*sw, c->y, c->w, c->h);
    XUnmapWindow(dpy, c->frame);
    c->isbanned = True;
}

void
iconify(Client *c) {
    ban(c);
    setclientstate(c, IconicState);
    c->isicon = True;
    detach(c);
    attach(c);
}

void
iconifyit(const char *arg) {
    if(!sel)
        return;
    iconify(sel);
    arrange();
    focusnext(NULL);
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
    rx = ry = 0;
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
buttonpress(XEvent *e) {
    Client *c;
    XButtonPressedEvent *ev = &e->xbutton;

    if(ev->window == root) {
            switch(ev->button) {
                case Button3:
                    XSetForeground(dpy, dc.gc, dc.sel[ColBorder]);
                    drawmouse(e);
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
    if((c = getclient(ev->window, clients, False))) {
        focus(c);
        restack();
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        if(CLEANMASK(ev->state) != MODKEY)
           return;
        if(ev->button == Button1) {
                if((layouts[ltidxs[curtag]].arrange == floating) || c->isfloating)
                        restack();
                movemouse(c);
        }
        else if(ev->button == Button2) {
                if((floating != layouts[ltidxs[curtag]].arrange) && c->isfloating)
                        togglefloating(NULL);
                else
                        zoom(NULL);
        }
        else if(ev->button == Button3 && !c->isfixed) {
                if((floating == layouts[ltidxs[curtag]].arrange) || c->isfloating)
                        restack();
                else
                        togglefloating(NULL);
                resizemouse(c);
        }
    }
    else if((c = getclient(ev->window, clients, True))) {
        focus(c);
        if(tpos != TitleRight){
            if((ev->x > c->w-2*c->border-3*c->th) && (ev->x < c->w-2*c->border-2*c->th)){
                /* min */
                bleft.action(NULL);
                return;
            }
            if((ev->x > c->w-2*c->border-2*c->th) && (ev->x < c->w-2*c->border-c->th)){
                /* max */
                bcenter.action(NULL);
                return;
            }
            if((ev->x > c->w-2*c->border-c->th) && (ev->x < c->w-2*c->border)){
                /* close */
                bright.action(NULL);
                return;
            }
        }
        if(ev->button == Button1) {
            if((layouts[ltidxs[curtag]].arrange == floating) || (layouts[ltidxs[curtag]].arrange == ifloating) || c->isfloating)
                restack();
            movemouse(c);
        }
        else if(ev->button == Button3 && !c->isfixed) {
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
    free(tags);
    free(keys);
    /* free resource database */
    XrmDestroyDatabase(xrdb);
    /* free colors */
    XftColorFree(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen), dc.xftnorm);
    XftColorFree(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen), dc.xftsel);
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
    regs = emallocz(nrules * sizeof(Regs));
    for(i = 0; i < nrules; i++) {
            if(rules[i]->prop) {
                    reg = emallocz(sizeof(regex_t));
                    if(regcomp(reg, rules[i]->prop, REG_EXTENDED))
                            free(reg);
                    else
                            regs[i].propregex = reg;
            }
            if(rules[i]->tags) {
                    reg = emallocz(sizeof(regex_t));
                    if(regcomp(reg, rules[i]->tags, REG_EXTENDED))
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
    ce.x = 0;
    ce.y = c->th;
    ce.width = c->w - 2*c->border;
    ce.height = c->h - 2*c->border - c->th;
    ce.border_width = 0;
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
            updategeom();
            arrange();
    }
}

void
configurerequest(XEvent *e) {
    Client *c;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

    if((c = getclient(ev->window, clients, False))) {
            c->ismax = False;
            if(ev->value_mask & CWBorderWidth)
                    c->border = ev->border_width;
            if(c->isfixed || c->isfloating || (floating == layouts[ltidxs[curtag]].arrange)) {
                    if(ev->value_mask & CWX)
                            c->x = ev->x;
                    if(ev->value_mask & CWY)
                            c->y = ev->y;
                    if(ev->value_mask & CWWidth)
                            c->w = ev->width + 2*c->border;
                    if(ev->value_mask & CWHeight)
                            c->h = ev->height + 2*c->border + c->th;
                    if((c->x + c->w) > sw && c->isfloating)
                            c->x = sw / 2 - c->w / 2; /* center in x direction */
                    if((c->y + c->h) > sh && c->isfloating)
                            c->y = sh / 2 - c->h / 2; /* center in y direction */
                    if((ev->value_mask & (CWX | CWY))
                    && !(ev->value_mask & (CWWidth | CWHeight)))
                            configure(c);
                    if(isvisible(c)){
                            XMoveResizeWindow(dpy, c->frame, c->x, c->y, c->w, c->h);
                            XMoveResizeWindow(dpy, c->win, 0, c->th, ev->width, ev->height);
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

    if((c = getclient(ev->window, clients, False)))
        unmanage(c);
    struts[RightStrut] = struts[LeftStrut] = struts[TopStrut] = struts[BotStrut] = 0;
    for(c = clients; c ; c = c->next){
        if(c->isbastard){
            if(ev->window != c->win)
                updatestruts(c->win);
            else {
                detach(c);
                free(c);
            }
        }
    }
    updategeom();
    arrange();
    updateatom[ClientList](NULL);
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
    if((c = getclient(ev->window, clients, False))){
	if((c->isfloating && !c->isbastard) || (layouts[ltidxs[curtag]].arrange == floating) || (layouts[ltidxs[curtag]].arrange == ifloating)  )
            focus(c);
	else
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
    if((c = getclient(ev->window, clients, True)))
        drawclient(c);
}

void
floating(void) { /* default floating layout */
    Client *c;
    notitles = False;

    domwfact = dozoom = False;
    for(c = clients; c; c = c->next){
        if(isvisible(c) && !c->isicon) {
                c->hastitle = c->title ? True : False;
                drawclient(c);
                if(!c->isfloating)
                        /*restore last known float dimensions*/
                        resize(c, c->sfx, c->sfy, c->sfw, c->sfh, True);
                    else
                        resize(c, c->x, c->y, c->w, c->h, True);
        }
    }
    wasfloating = True;
}

void
focus(Client *c) {
    Client *o;
    o = sel;
    if((!c && selscreen) || (c && (c->isbastard || !isvisible(c))))
            for(c = stack; c && (c->isbastard || !isvisible(c)); c = c->snext);
    if(sel && sel != c) {
            grabbuttons(sel, False);
            XSetWindowBorder(dpy, sel->frame, dc.norm[ColBorder]);
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
            XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
            XSetWindowBorder(dpy, sel->frame, dc.sel[ColBorder]);
            drawclient(c);
    }
    else
            XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
    if(o)
        drawclient(o);
    updateatom[ActiveWindow](sel);
}

void
focusnext(const char *arg) {
    Client *c;

    if(!sel)
            return;
    for(c = sel->next; c && (c->isbastard || !isvisible(c)); c = c->next);
    if(!c)
            for(c = clients; c && (c->isbastard || !isvisible(c)); c = c->next);
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
    for(c = sel->prev; c && (c->isbastard || !isvisible(c)); c = c->prev);
    if(!c) {
            for(c = clients; c && c->next; c = c->next);
            for(; c && (c->isbastard || !isvisible(c)); c = c->prev);
    }
    if(c) {
            focus(c);
            restack();
    }
}

void
incnmaster(const char *arg) {
    int i;
    if(layouts[ltidxs[curtag]].arrange != tile)
            return;
    if(!arg)
            nmasters[curtag] = NMASTER;
    else {
            i = atoi(arg);
            if((nmasters[curtag] + i) < 1 || wah / (nmasters[curtag] + i) <= 2 * borderpx)
                    return;
            nmasters[curtag] += i;
    }
    if(sel)
            arrange();
}

Client *
getclient(Window w, Client *list, Bool title) {
    Client *c;

    for(c = list; c && (title ? c->title : c->win) != w; c = c->next);
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
    unsigned int Buttons[] = {Button1, Button2, Button3};            
    unsigned int Modifiers[] = {MODKEY, MODKEY|LockMask,
               MODKEY|numlockmask, MODKEY|numlockmask|LockMask};                                                                                              
    int i, j;   
    XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

    if(focused) {
           for (i = 0; i < LENGTH(Buttons); i++)                                                                                                          
                   for (j = 0; j < LENGTH(Modifiers); j++)                                                                                                
                           XGrabButton(dpy, Buttons[i], Modifiers[j], c->win, False,                                                                      
                                   BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);    
    }
    else
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK,
                            GrabModeAsync, GrabModeSync, None, None);
}

unsigned int
idxoftag(const char *tag) {
    unsigned int i;

    for(i = 0; (i < ntags) && strcmp(tag, tags[i]); i++);
    return (i < ntags) ? i : 0;
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

    for(i = 0; i < ntags; i++)
            if(c->tags[i] && seltags[i])
                    return True;
    return False;
}

void
keypress(XEvent *e) {
    unsigned int i;
    KeyCode code;
    KeySym keysym;
    XKeyEvent *ev;

    if(!e) { /* grabkeys */
            XUngrabKey(dpy, AnyKey, AnyModifier, root);
            for(i = 0; i < nkeys; i++) {
                    code = XKeysymToKeycode(dpy, keys[i]->keysym);
                    XGrabKey(dpy, code, keys[i]->mod, root, True,
                                    GrabModeAsync, GrabModeAsync);
                    XGrabKey(dpy, code, keys[i]->mod | LockMask, root, True,
                                    GrabModeAsync, GrabModeAsync);
                    XGrabKey(dpy, code, keys[i]->mod | numlockmask, root, True,
                                    GrabModeAsync, GrabModeAsync);
                    XGrabKey(dpy, code, keys[i]->mod | numlockmask | LockMask, root, True,
                                    GrabModeAsync, GrabModeAsync);
            }
            return;
    }
    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for(i = 0; i < nkeys; i++)
            if(keysym == keys[i]->keysym
            && CLEANMASK(keys[i]->mod) == CLEANMASK(ev->state))
            {
                    if(keys[i]->func)
                            keys[i]->func(keys[i]->arg);
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
    int i = 0;

    c = emallocz(sizeof(Client));
    c->win = w;

    if(checkatom(c->win, atom[WindowType], atom[WindowTypeDesk]) ||
            checkatom(c->win, atom[WindowType], atom[WindowTypeDock])){
        c->isbastard = True;
        c->isfloating = True;
        c->isfixed = True;
    }

    c->isicon = False;
    c->hastitle = c->isbastard ? False : True;
    c->tags = emallocz(ntags*(sizeof seltags));

    c->th = c->isbastard ? 0 : dc.h;
    c->border = c->isbastard ? 0 : borderpx;

    if(cx && cy && cw && ch) {
            c->x = wa->x = cx;
            c->y = wa->y = cy;
            c->w = wa->width = cw + 2 * c->border;
            c->h = wa->height = ch + 2 * c->border + c->th;
            cx = cy = cw = ch = 0;
    }
    else {
            c->x = wa->x;
            c->y = wa->y;
            c->w = wa->width + 2 * c->border;
            c->h = wa->height + 2 * c->border + c->th;
    }

    if(wa->x && wa->y)
        c->isplaced = True;
    else
        if(!c->isbastard)
            c->isplaced = False;

    c->oldborder = c->isbastard ? 0 : wa->border_width;
    if(c->w == sw && c->h == sh) {
            c->x = sx;
            c->y = sy;
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
    }
    wc.border_width = c->border;
    if(!c->isbastard){
        XConfigureWindow(dpy, w, CWBorderWidth, &wc);
        XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
        configure(c); /* propagates border_width, if size doesn't change */
        updatesizehints(c);
    }
    c->sfx = c->x;
    c->sfy = c->y;
    c->sfw = c->w;
    c->sfh = c->h;
    if(c->isbastard)
        XSelectInput(dpy, w, PropertyChangeMask);
    else
        XSelectInput(dpy, w,
            StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
    grabbuttons(c, False);
    twa.override_redirect = 1;
    twa.background_pixmap = ParentRelative;
    twa.event_mask = ExposureMask | MOUSEMASK | SubstructureRedirectMask | SubstructureNotifyMask | PointerMotionMask | EnterWindowMask;
    //twa.border_width = borderpx;
    //
    c->frame = XCreateWindow(dpy, root, c->x, c->y - c->th, c->w, c->h,
                    c->border, DefaultDepth(dpy, screen), CopyFromParent,
                    DefaultVisual(dpy, screen),
                    CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);
 
    XConfigureWindow(dpy, c->frame, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, c->frame, dc.norm[ColBorder]);

    twa.event_mask = ExposureMask | MOUSEMASK;
    if(!c->isbastard){
        c->title = XCreateWindow(dpy, c->frame, 0, 0, c->w, c->th,
                        0, DefaultDepth(dpy, screen), CopyFromParent,
                        DefaultVisual(dpy, screen),
                        CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);
    }
    else
        c->title = (Window*)NULL;

    updatetitle(c);
    if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
            for(t = clients; t && t->win != trans; t = t->next);
    if(t)
            memcpy(c->tags, t->tags, ntags*(sizeof seltags));
    applyrules(c);
    if(c->isbastard)
        for(i = 0; i < ntags; i++)
            c->tags[i] = True;
    if(!c->isfloating)
            c->isfloating = (rettrans == Success) || c->isfixed;
    attach(c);
    attachstack(c);
    twa.event_mask = EnterWindowMask |
                        PropertyChangeMask | FocusChangeMask;
    twa.win_gravity = StaticGravity;
    twa.do_not_propagate_mask = MOUSEMASK;
    XChangeWindowAttributes(dpy, c->win,
                        CWEventMask | CWWinGravity | CWDontPropagate, &twa);
    XReparentWindow(dpy, c->win, c->frame, 0, c->th);
    XMoveResizeWindow(dpy, c->win, 0, c->th, c->w - 2 * c->border, c->h - 2 * c->border - c->th); /* some windows require this */
    if(!c->isbastard)
        XMoveResizeWindow(dpy, c->frame, c->x+2*sw, c->y, c->w, c->h);
    if(checkatom(c->win, atom[WindowState], atom[WindowStateFs]))
        ewmh_process_state_atom(c, atom[WindowStateFs], 1);
    XMapWindow(dpy, c->frame);
    XMapWindow(dpy, c->win);
    if(c->title)
        XMapWindow(dpy, c->title);
    drawclient(c);
    updateatom[ClientList](NULL);
    updateatom[WindowDesk](c);
    updatestruts(c->win);
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
    if(!getclient(ev->window, clients, False))
            manage(ev->window, &wa);
    arrange();
}

int
smartcheckarea(int x, int y, int w, int h){
    /* this one is called for *every*(!) client */
    Client *c;
    int n = 0;
    for(c = clients; c; c = c->next){ 
        if(isvisible(c) && !c->isicon && c->isplaced){
            /* A Kind Of Magic */
            if((c->y + c->h >= y && c->y + c->h <= y + h
            && c->x+c->w >= x && c->x+c->w <= x+w)
            || (c->x >= x && c->x <= x+w
                 && c->y + c->h >= y && c->y + c->h <= y + h)
            || (c->x >= x && c->x <= x+w
                 && c->y >= y && c->y <= y+h)
            || (c->x+c->w >= x && c->x+c->w <= x+w
                 && c->y >= y && c->y <= y+h))
                n++;
        }
    }
    return n;
}

void
ifloating(void){
    Client *c;
    int x, y, f;
    for(c = clients; c; c = c->next){ 
        if(isvisible(c) && !c->isicon && !c->isbastard){
                for(f = 0; !c->isplaced; f++){ 
                    if(c->w > sw/2 && c->h > sw/2){
                        /* too big to deal with */
                        c->isplaced = True; 
                    }
                    /* i dunno if c->h/4 & c->w/8 are optimal */
                        for(y = way; y+c->h <= wah && !c->isplaced ; y+=c->h/4){
                            for(x = wax; x+c->w <= waw && !c->isplaced; x+=c->w/8){
                                /* are you wondering about 0.9 & 0.8 ? */
                            if(smartcheckarea(x, y, 0.8*c->w, 0.8*c->h)<=f){
                                /* got it! a big chunk of "free" space */
                                resize(c, x+c->th*(rand()%3), y+c->th+c->th*(rand()%3), c->w, c->h, True);
				c->isplaced = True;
                            }
                        }
                    }
                }
            c->hastitle = c->title ? True : False;
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
        if(isvisible(c) && !c->isicon && !c->isbastard) {
	    c->isplaced = False;
            if(!c->isfloating){
                c->sfx = c->x;
                c->sfy = c->y;
                c->sfw = c->w;
                c->sfh = c->h;
                c->hastitle = False;
                unban(c);
            }
            else
                continue;
            if(bpos[curtag] != StrutsOn) 
                resize(c, sx-c->border, sy-c->border, sw, sh, False);
            else {
                resize(c, wax, way, waw-2*c->border, wah-2*c->border, False);
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
    for(; c && (c->isfloating || !isvisible(c) || c->isbastard || c->isicon); c = c->next);
    return c;
}

Client *
prevtiled(Client *c) {
    for(; c && (c->isfloating || !isvisible(c) || c->isbastard || c->isicon); c = c->prev);
    return c;
}
void
propertynotify(XEvent *e) {
    Client *c;
    Window trans;
    XPropertyEvent *ev = &e->xproperty;

    if(ev->state == PropertyDelete)
            return; /* ignore */
    if(ev->atom == atom[StrutPartial]){
        updatestruts(ev->window);
        arrange();
    }
    if((c = getclient(ev->window, clients, False))) {
            switch (ev->atom) {
                    default: break;
                    case XA_WM_TRANSIENT_FOR:
                            XGetTransientForHint(dpy, c->win, &trans);
                            if(!c->isfloating && (c->isfloating = (getclient(trans, clients, False) != NULL)))
                                    arrange();
                            break;
                    case XA_WM_NORMAL_HINTS:
			    updatesizehints(c);
                            break;
            }
            if(ev->atom == XA_WM_NAME || ev->atom == atom[WindowName]) {
                    updatetitle(c);
            }
    }
}

void
quit(const char *arg) {
    cleanup();
    running = False;
    if(arg){
	execvp(cargv[0], cargv);
	eprint("Can't exec: %s\n", strerror(errno));
    }
}

void
resize(Client *c, int x, int y, int w, int h, Bool sizehints) {
    XWindowChanges wc;

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
	    if(c->isfloating || (layouts[ltidxs[curtag]].arrange == floating) || (layouts[ltidxs[curtag]].arrange == ifloating)) {
		    c->sfx = x;
		    c->sfy = y;
		    c->sfw = w;
		    c->sfh = h;
		    c->isplaced = True;
	    }
            c->x = x;
            c->y = y;
            c->w = w;
            c->h = h;
            XMoveResizeWindow(dpy, c->frame, c->x, c->y, c->w, c->h);
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
            c->th = c->hastitle ? dc.h : 0;
            wc.x = 0;
            wc.y = c->th;
            wc.width = w;
            wc.height = h;
            wc.border_width = c->border;
            XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
            XMoveResizeWindow(dpy, c->win, 0, c->th, w, h);
            if(c->title)
                XMoveResizeWindow(dpy, c->title, 0, 0, c->w, c->hastitle ? c->th: 1);
            //XConfigureWindow(dpy, c->win, CWWidth | CWHeight | CWBorderWidth, &wc);
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
                            nw = MINWIDTH;
                    if((nh = ev.xmotion.y - ocy - 2 * c->border + 1) <= 0)
                            nh = MINHEIGHT;
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
    Window *wl;
    int i,n;

    if(!sel)
            return;

    for(n = 0, c = stack; c; c = c->snext){
        if(isvisible(c)){
                n++;
        }
    }
    wl = malloc(sizeof(Window)*n);
    for(i = 0, c = stack; c && i<n; c = c->snext)
        if(isvisible(c)){
            wl[i++] = c->frame;
        }
    XRestackWindows(dpy, wl, i);
    XSync(dpy, False);
    while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
    fd_set rd;
    int xfd;
    XEvent ev;

    /* main event loop */
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
                    if(!XGetWindowAttributes(dpy, wins[i], &wa) || 
                    wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
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
    if(c->title)
        XChangeProperty(dpy, c->title, wmatom[WMState], wmatom[WMState], 32,
                        PropModeReplace, (unsigned char *)data, 2);
    if(state == NormalState)
        c->isicon = False;
}

void
setlayout(const char *arg) {
    unsigned int i;

    if(!arg) {
            if(&layouts[++ltidxs[curtag]] == &layouts[LENGTH(layouts)])
                    ltidxs[curtag] = 0;
    }
    else {
            for(i = 0; i < LENGTH(layouts); i++)
                    if(!strcmp(arg, layouts[i].symbol))
                            break;
            if(i == LENGTH(layouts))
                    return;
            ltidxs[curtag] = i;
    }
    if(sel)
            arrange();
    updateatom[ELayout](NULL);
}

void
setmwfact(const char *arg) {
    double delta;

    if(!domwfact)
            return;
    /* arg handling, manipulate mwfact */
    if(arg == NULL)
            mwfacts[curtag] = MWFACT;
    else if(sscanf(arg, "%lf", &delta) == 1) {
            if(arg[0] == '+' || arg[0] == '-')
                    mwfacts[curtag] += delta;
            else
                    mwfacts[curtag] = delta;
            if(mwfacts[curtag] < 0.1)
                    mwfacts[curtag] = 0.1;
            else if(mwfacts[curtag] > 0.9)
                    mwfacts[curtag] = 0.9;
    }
    arrange();
}

void
initlayouts(){
    int i,j;
    char conf[32], xres[256];

    /* init layouts */
    nmasters = (unsigned int*)emallocz(sizeof(unsigned int) * ntags);
    ltidxs = (unsigned int*)emallocz(sizeof(unsigned int) * ntags);
    mwfacts = (double*)emallocz(sizeof(double) * ntags);
    for(i = 0; i < ntags; i++) {
        ltidxs[i] = 0;
        snprintf(conf, 31, "tags.layout%d", i);
        strncpy(xres, getresource(conf, getresource("deflayout", "i")), 255);
        for (j = 0; j < LENGTH(layouts); j++) {
            if (!strcmp(layouts[j].symbol, xres)) {
                    ltidxs[i] = j;
                    break;
            }
        }
        mwfacts[i] = MWFACT;
        nmasters[i] = NMASTER;
    }

    /* init struts */
    bpos = (unsigned int*)emallocz(sizeof(unsigned int) * ntags);
    for(i = 0; i < ntags; i++)
            bpos[i] = BARPOS;
    updateatom[ELayout](NULL);
}

void
inittags(){
    int i;
    char tmp[25]="\0";
    ntags = atoi(getresource("tags.number", "5"));
    tags = emallocz(ntags*sizeof(char*));
    prevtags = emallocz(ntags*sizeof(Bool));
    seltags = emallocz(ntags*sizeof(Bool));
    seltags[0] = True;
    for(i = 0; i < ntags; i++){
        tags[i] = emallocz(25*sizeof(char));
        snprintf(tmp, 24, "tags.name%d", i);
        snprintf(tags[i], 24, "%s", getresource(tmp, "null"));
    }
}

void
setup(void) {
	int d;
	unsigned int i, j, mask;
	Window w;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

	/* init atom */
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMName] = XInternAtom(dpy, "WM_NAME", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);

        /* init EWMH atom */
        initatom();
        
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
        char conf[256];
        snprintf(conf, 255, "%s/%s", getenv("HOME"), "/.echinus");
        chdir(conf);
        xrdb = XrmGetFileDatabase("echinusrc");
        if(!xrdb)
            eprint("echinus: cannot open configuration file\n");

	/* init tags */
        inittags();
        initrules();
        initkeys();
        initlayouts();
        updateatom[NumberOfDesk](NULL);
        updateatom[DeskNames](NULL);
        updateatom[CurDesk](NULL);

	compileregs();

        /* grab keys */
	keypress(NULL);

	/* init appearance */
	dc.norm[ColBorder] = getcolor(getresource("normal.border", NORMBORDERCOLOR));
	dc.norm[ColBG] = getcolor(getresource("normal.bg", NORMBGCOLOR));
	dc.norm[ColFG] = getcolor(getresource("normal.fg", NORMFGCOLOR));
	dc.norm[ColButton] = getcolor(getresource("normal.button", NORMBUTTONCOLOR));

        dc.sel[ColBorder] = getcolor(getresource("selected.border", SELBORDERCOLOR));
	dc.sel[ColBG] = getcolor(getresource("selected.bg", SELBGCOLOR));
	dc.sel[ColFG] = getcolor(getresource("selected.fg", SELFGCOLOR));
	dc.sel[ColButton] = getcolor(getresource("selected.button", SELBUTTONCOLOR));

        dc.xftsel = emallocz(sizeof(XftColor));
        dc.xftnorm = emallocz(sizeof(XftColor));
        XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), getresource("selected.fg", SELFGCOLOR), dc.xftsel);
        XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), getresource("normal.fg", SELFGCOLOR), dc.xftnorm);
        if(!dc.xftnorm || !dc.xftnorm)
             eprint("error, cannot allocate colors\n");
        initfont(getresource("font", FONT));
        borderpx = atoi(getresource("border", BORDERPX));
        uf_opacity = atof(getresource("opacity", NF_OPACITY));

        strncpy(terminal, getresource("terminal", TERMINAL), 255);

        dc.h = atoi(getresource("title", TITLEHEIGHT));
        if(!dc.h){
            NOTITLES = 1;
            dc.h = 1;
        }
        dectiled = atoi(getresource("decoratetiled", DECORATETILED));
        hidebastards = atoi(getresource("hidebastards", "0"));
        tpos = atoi(getresource("titleposition", TITLEPOSITION));
        tbpos = atoi(getresource("tagbar", TAGBAR));

	struts[RightStrut] = struts[LeftStrut] = struts[TopStrut] = struts[BotStrut] = 0;
        updategeom();

	dc.drawable = XCreatePixmap(dpy, root, sw, dc.h, DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, 0);

        /* buttons */
        initbuttons();
        chdir(getenv("HOME"));

        /* free resource database */
        dc.xftdrawable = XftDrawCreate(dpy, dc.drawable, DefaultVisual(dpy,screen),DefaultColormap(dpy,screen));
        if(!dc.xftdrawable)
             eprint("error, cannot create drawable\n");

	/* multihead support */
	selscreen = XQueryPointer(dpy, root, &w, &w, &d, &d, &d, &d, &mask);
}

void
spawn(const char *arg) {
	static char shell[] = "/bin/sh";

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
	for(i = 0; i < ntags; i++)
		sel->tags[i] = (NULL == arg);
	sel->tags[idxoftag(arg)] = True;
        updateatom[WindowDesk](sel);
	arrange();
}

void
bstack(void) {
    unsigned int i, n, nx, ny, nw, nh, mh, tw;
    Client *c, *mc;

    domwfact = dozoom = True;
    for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
        n++;

    mh = (n == 1) ? wah : mwfacts[curtag] * wah;
    tw = (n > 1) ? waw / (n - 1) : 0;

    nx = wax;
    ny = way;
    nh = 0;
    for(i = 0, c = mc = nexttiled(clients); c; c = nexttiled(c->next), i++) {
        c->hastitle = (dectiled ? (c->title ? True : False) : False);
        c->ismax = False;
        if(i == 0) {
            nh = mh - 2 * c->border;
            nw = waw - 2 * c->border;
            nx = wax;
            if(dectiled){
                ny+=dc.h + c->border;
                nh-=dc.h + c->border;
            }
        }
        else {
            if(i == 1) {
                nx = wax;
                ny += mc->h+c->border;
                if(dectiled)
                    ny += dc.h + c->border;
                nh = (way + wah) - ny - 2 * c->border;
            }
            if(i + 1 == n)
                nw = (wax + waw) - nx - 2 * c->border;
            else
                nw = tw - c->border;
        }
        resize(c, nx, ny, nw, nh, False);
        if(n > 1 && tw != waw)
            nx = c->x + c->w + c->border;
    }
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
	mh = (n <= nmasters[curtag]) ? wah / (n > 0 ? n : 1) : wah / nmasters[curtag];
	mw = (n <= nmasters[curtag]) ? waw : mwfacts[curtag] * waw;
	th = (n > nmasters[curtag]) ? wah / (n - nmasters[curtag]) : 0;
	if(n > nmasters[curtag] && th < bh)
		th = wah;

	nx = wax;
	ny = way;
	nw = 0; /* gcc stupidity requires this */
	for(i = 0, c = mc = nexttiled(clients); c; c = nexttiled(c->next), i++) {
                c->hastitle = (dectiled ? (c->title ? True : False) : False);
		c->ismax = False;
                c->sfx = c->x;
                c->sfy = c->y;
                c->sfw = c->w;
                c->sfh = c->h;
                if(i < nmasters[curtag]) { /* master */
                        ny = way + i * (mh - c->border);
                        nw = mw - 2 * c->border;
                        nh = mh;
                        if(i + 1 == (n < nmasters[curtag] ? n : nmasters[curtag])) /* remainder */
                                nh = way + wah - ny;
                        if(dectiled){
                            ny+=dc.h+c->border;
                            nh-=dc.h+c->border;
                        }
                        nh -= 2 * c->border;
                }
                else {  /* tile window */
                        if(i == nmasters[curtag]) {
                                ny = way;
                                if(dectiled)
                                    ny+=dc.h+c->border;
                                nx += mc->w + mc->border;
                                nw = waw - nx - 2*c->border;
                        }
                        else 
                            ny -= c->border;
                        if(i + 1 == n) /* remainder */
                                nh = (way + wah) - ny - 2 * c->border;
                        else
                                nh = th - 2 * c->border;
                }
                resize(c, nx, ny, nw, nh, False);
                drawclient(c);
                if(n > nmasters[curtag] && th != wah){
                        ny = c->y + c->h + 2 * c->border;
                        if(dectiled)
                            ny += dc.h+c->border;
                }
        }
}

void
togglestruts(const char *arg) {
    bpos[curtag] = (bpos[curtag] == StrutsOn) ? (hidebastards ? StrutsHide : StrutsOff) : StrutsOn;
    updategeom();
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
togglemax(const char *arg) {
    XEvent ev;

    if(!sel || sel->isfixed)
            return;
    if((sel->ismax = !sel->ismax)) {
            sel->rx = sel->x;
            sel->ry = sel->y;
            sel->rw = sel->w;
            sel->rh = sel->h;
            resize(sel, sx - sel->border, sy - sel->border, sw + 2*sel->border, sh + 2*sel->border + sel->th, False);
    }
    else 
        resize(sel, sel->rx, sel->ry, sel->rw, sel->rh, True);
    while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
toggletag(const char *arg) {
    unsigned int i, j;

    if(!sel)
            return;
    i = idxoftag(arg);
    sel->tags[i] = !sel->tags[i];
    for(j = 0; j < ntags && !sel->tags[j]; j++);
    if(j == ntags)
            sel->tags[i] = True; /* at least one tag must be enabled */
    arrange();
}

void
toggleview(const char *arg) {
    unsigned int i, j;

    memcpy(prevtags, seltags, ntags*(sizeof seltags));
    i = idxoftag(arg);
    seltags[i] = !seltags[i];
    for(j = 0; j < ntags && !seltags[j]; j++);
    if(j == ntags) {
        seltags[i] = True; /* at least one tag must be viewed */
        j = i;
    }
    if(curtag == i)
        curtag = j;
    arrange();
    updateatom[CurDesk](NULL);
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
        if (c->tags[i] && !c->isbastard) {
                focus(c);
                c->isplaced = True;

        }
    }
    restack();
}
void
unban(Client *c) {
    if(!c->isbanned)
            return;
    XMoveResizeWindow(dpy, c->frame, c->x, c->y, c->w, c->h);
    XMapWindow(dpy, c->frame);
    setclientstate(c, NormalState);
    c->isbanned = False;
}

void
unmanage(Client *c) {
    XWindowChanges wc;
    if(c->title)
        XDestroyWindow(dpy, c->title);
    XReparentWindow(dpy, c->win, root, c->x, c->y);
    XMoveWindow(dpy, c->win, c->x, c->y);
    XDestroyWindow(dpy, c->frame);
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
    updateatom[ClientList](NULL);
}

void
updategeom(void) {
    XEvent ev;

    wax = sx;
    way = sy;
    wah = sh;
    waw = sw;
    switch(bpos[curtag]){
    default:
        wax += struts[LeftStrut];
        waw = sw - wax - struts[RightStrut];
        way += struts[TopStrut];
        wah = sh - way - struts[BotStrut];
        break;
    case StrutsHide:
    case StrutsOff:
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

    if((c = getclient(ev->window, clients, False))) {
        if(c->isicon)
            return;
        XGetWindowAttributes(dpy, ev->window, &wa);
        if(wa.map_state == IsUnmapped && c->title){
                XGetWindowAttributes(dpy, c->title, &wa);
                if(wa.map_state == IsViewable)
                        unmanage(c);
        }
    }
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
    if(!gettextprop(c->win, atom[WindowName], c->name, sizeof c->name))
            gettextprop(c->win, wmatom[WMName], c->name, sizeof c->name);
    drawclient(c);
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (ebastardly on UnmapNotify's).  Other types of errors call Xlibs
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
    unsigned int i, prevcurtag;
    memcpy(prevtags, seltags, ntags*(sizeof seltags));
    for(i = 0; i < ntags; i++)
            seltags[i] = (NULL == arg);
    seltags[idxoftag(arg)] = True;
    prevcurtag = curtag;
    curtag = idxoftag(arg);
    if (bpos[prevcurtag] != bpos[curtag])
        updategeom();
    arrange();
    updateatom[CurDesk](NULL);
}

void
viewprevtag(const char *arg) {
    Bool tmptags[ntags];
    unsigned int i, prevcurtag;
 
    i = 0;
    while(i < ntags-1 && !prevtags[i]) i++;
    prevcurtag = curtag;
    curtag = i;

    memcpy(tmptags, seltags, ntags*(sizeof seltags));
    memcpy(seltags, prevtags, ntags*(sizeof seltags));
    memcpy(prevtags, tmptags, ntags*(sizeof seltags));
    if (bpos[prevcurtag] != bpos[curtag])
	updategeom();
    arrange();
    updateatom[CurDesk](NULL);
}

void
viewlefttag(const char *arg) {
    int i;
    for(i = 0; i < ntags; i++) {
        if(i && seltags[i]) {
                view(tags[i-1]);
                break;
        }
    }
}

void
viewrighttag(const char *arg) {
    int i;
    for(i = 0; i < ntags-1; i++) {
        if(seltags[i]) {
            view(tags[i+1]);
            break;
        }
    }
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
    arrange();
    focus(c);
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
