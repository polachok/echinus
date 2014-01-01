#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xft/Xft.h>

#include "util.h"

int screen;
Display *dpy;
unsigned int mw = 0;
unsigned int mh = 0;
unsigned int mx = 0;
unsigned int my = 0;
unsigned int numlockmask = 0;
Bool running = True;
Bool bottom = False;
Window root, win;
enum { Left, Right, Top, Bottom, LeftStartY, LeftEndY, RightStartY, RightEndY, TopStartX, TopEndX, BotStartX, BotEndX };

void
setstruts(Bool autohide) {
    int *struts;
    int x, y;
    unsigned w, h, b, d;
    Atom net_wm_strut_partial;
    Atom net_wm_window_type;
    Atom net_wm_window_type_dock;
    XWindowAttributes wa;

    net_wm_strut_partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
    net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

    struts = emallocz(12*sizeof(int));
    XGetWindowAttributes(dpy, win, &wa);
    XGetGeometry(dpy, win, &root, &x, &y, &w, &h, &b, &d);
    fprintf(stderr, "%d %d\n", x, y);
    fprintf(stderr, "x:%d y:%d ht: %d\n", wa.x, wa.y, DisplayHeight(dpy, screen));
    if(bottom){
        struts[Bottom] = DisplayHeight(dpy, screen) - my + mh;
	fprintf(stderr, "BotStrut=%d\n", struts[Bottom]);
        struts[BotStartX] = mx;
        struts[BotEndX] = mx+mw;
    }
    else {
        struts[Top] = mh + my;
        struts[TopStartX] = mx;
        struts[TopEndX] = mx+mw;
    }
    XChangeProperty(dpy, win, net_wm_strut_partial, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)struts, 12);
    XChangeProperty(dpy, win, net_wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&net_wm_window_type_dock, 1);
    free(struts);
}

void
setup() {
	unsigned int i, j;
	XModifierKeymap *modmap;
	XSetWindowAttributes wa;

	/* init modifier map */
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++) {
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
			== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
		}
	XFreeModifiermap(modmap);
	wa.override_redirect = 0;
	wa.background_pixmap = ParentRelative;
	wa.event_mask = ExposureMask | ButtonPressMask | KeyPressMask;
	mw = DisplayWidth(dpy, screen)-mx;
	my = bottom ? DisplayHeight(dpy, screen) - mh : 0;
	mh = 20;
	win = XCreateWindow(dpy, root, mx,
			my, mw, mh, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
        setstruts(0);
	XMapRaised(dpy, win);
        XMoveWindow(dpy, win, mx, my);
}

void
drawme() {
	GC gc;
	gc = XCreateGC(dpy, root, 0, 0);
	XRectangle r = { 0, 0, mw, mh };
	XSetForeground(dpy, gc, BlackPixel(dpy, screen));
	XSetBackground(dpy, gc, WhitePixel(dpy, screen));
	XFillRectangles(dpy, win, gc, &r, 1);
        setstruts(0);
}

void
run(void) {
	XEvent ev;

        XSetErrorHandler(xerrordummy);
	
        /* main event loop */
	XSelectInput(dpy, root, PropertyChangeMask);
	while(running && !XNextEvent(dpy, &ev))
		switch (ev.type) {
		default:	/* ignore all crap */
			break;
		case Expose:
			if(ev.xexpose.count == 0)
				drawme();
			break;
		}
}

int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-b", argv[1]))
	    bottom = True;
	dpy = XOpenDisplay(0);
	if(!dpy)
		eprint("ewmhpanel: cannot open display\n");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	setup(bottom);
	XSync(dpy, False);
	run();
	XCloseDisplay(dpy);
	return 0;
}
