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
unsigned int numlockmask = 0;
Bool running = True;
Window root, win;

void
setstruts(Bool bottom) {
    int *struts;
    Atom net_wm_strut_partial;
    Atom net_wm_window_type;
    Atom net_wm_window_type_dock;

    net_wm_strut_partial = XInternAtom(dpy, "_NET_WM_STRUT_PARTIAL", False);
    net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    net_wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

    struts = emallocz(12*sizeof(int));
    if(bottom){
        struts[3] = mh-1;
        struts[10] = mx;
        struts[11] = mx+mw;
    }
    else {
        struts[2] = mh-1;
        struts[8] = mx;
        struts[9] = mx+mw;
    }
    XChangeProperty(dpy, win, net_wm_strut_partial, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)struts, 12);
    XChangeProperty(dpy, win, net_wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&net_wm_window_type_dock, 1);
    /* set atom */
    XChangeProperty(dpy, root, XInternAtom(dpy, "_OURICO_WINDOW", False), XA_WINDOW, 32, PropModeReplace, (unsigned char*)&win, 1);
    free(struts);
}

void
setup(Bool bottom) {
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
	mh = 20;
	win = XCreateWindow(dpy, root, mx,
			bottom ? DisplayHeight(dpy, screen) - mh : 0, mw, mh, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
        setstruts(bottom);
	XMapRaised(dpy, win);
        XMoveWindow(dpy, win, mx, bottom ? DisplayHeight(dpy, screen) - mh : 0);
}

void
drawme() {
	GC gc;
	gc = XCreateGC(dpy, root, 0, 0);
	XRectangle r = { 0, 0, mw, mh };
	XSetForeground(dpy, gc, BlackPixel(dpy, screen));
	XSetBackground(dpy, gc, WhitePixel(dpy, screen));
	XFillRectangles(dpy, win, gc, &r, 1);
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
	Bool bottom = False;

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
