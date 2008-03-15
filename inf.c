/* (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <locale.h>

static char buf[1024];
static Atom netwmname;
static Atom dwmconfig;
static Atom dwmfocus;

static Display *dpy;
static Window root;

static int 
getname(Window t) {
	char **list = NULL;
	unsigned long n,b;
	unsigned char *win;
	Atom nname;
        Window w;
        int f = 8;

        XTextProperty prop;
        XTextProperty name;

        name.nitems = 0;

        XGetWindowProperty(dpy, root, dwmfocus, 0, 1, False, AnyPropertyType, &nname, &f, &n, &b, &win);
        if(!n){
            puts("fuck");
            return 0;
        }
        else {
            memcpy(&w, win, n*f);
        }
        
        bzero(buf,1024);
	prop.nitems = 0;
	buf[0] = 0;
	XGetTextProperty(dpy, w, &prop, netwmname);
	if(!prop.nitems)
		XGetWMName(dpy, w, &prop);
	if(!prop.nitems)
		return 0;
	if(prop.encoding == XA_STRING){
            puts("b| ?");
		strncpy(buf, (char *)prop.value, sizeof(buf));
        }
	else {
		if(XmbTextPropertyToTextList(dpy, &prop, &list, &n) >= Success
				&& n > 0 && *list)
		{
			strncpy(buf, *list, sizeof(buf));
			XFreeStringList(list);
		}
	}
	XFree(prop.value);
    return 1;
}

int
main(int argc, char *argv[]) {
	unsigned int i, num;

	setlocale(LC_CTYPE, "");
	if((argc > 1) && !strncmp(argv[1], "-v", 3)) {
		fputs("lsw (C)opyright MMVI Anselm R. Garbe\n", stdout);
		exit(EXIT_SUCCESS);
	}
	if(!(dpy = XOpenDisplay(0))) {
		fputs("lsw: cannot open display\n", stderr);
		exit(EXIT_FAILURE);
	}
	root = RootWindow(dpy, DefaultScreen(dpy));
	netwmname = XInternAtom(dpy, "_NET_WM_NAME", False);
	dwmfocus = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
        if(getname(NULL)){
                    fprintf(stdout, "%s\n", buf);
        }
	XCloseDisplay(dpy);
	return 0;
}
