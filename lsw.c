/* (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static char buf[1024];
static Atom netwmname;
static Atom dwmconfig;
static Atom dwmfocus;
static Display *dpy;
static Window root;

static int 
getname(Window w) {
	char **list = NULL;
	int n;
	XTextProperty prop;
	XTextProperty name;

    name.nitems = 0;

    XGetTextProperty(dpy, w, &name, dwmfocus);
    name.nitems = 0;
    XGetTextProperty(dpy, w, &name, dwmconfig);
	if(name.nitems && name.encoding == XA_STRING) {
			strncpy(buf, (char *)name.value, sizeof buf - 1);
			buf[sizeof buf - 1] = '\0';
			XFree(name.value);
            for(n = 0;  n < sizeof buf - 2 && buf[n] != '\0' && buf[n] != '|'; n++){
              //  if(buf[n] == '1')
               //     printf("tag[%d]", n);
            }
            n++;
            if(buf[n++] != '1')
                return 0;
            /*printf("v");
            printf("\n");*/

    }
    bzero(buf,1024);
	prop.nitems = 0;
	buf[0] = 0;
	XGetTextProperty(dpy, w, &prop, netwmname);
	if(!prop.nitems)
		XGetWMName(dpy, w, &prop);
	if(!prop.nitems)
		return 0;
	if(prop.encoding == XA_STRING)
		strncpy(buf, (char *)prop.value, sizeof(buf));
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
	Window *wins, d1, d2;
	XWindowAttributes wa;

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
	dwmconfig = XInternAtom(dpy, "_DWM_CONFIG", False);
	dwmfocus = XInternAtom(dpy, "_DWM_FOCUS", False);
	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(wa.override_redirect)
				continue;
			if(getname(wins[i])){
			if(buf[0])
                printf("0x%0x ", wins[i]);
				fprintf(stdout, "%s\n", buf);
            }
		}
	}
	if(wins)
		XFree(wins);
	XCloseDisplay(dpy);
	return 0;
}
