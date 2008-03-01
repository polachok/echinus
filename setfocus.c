/*
 *     written by Alexander Polakov <polachok@gmail.com> at 01.03.2008 20:57:12 MSK
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Atom netwmname;
static Atom dwmconfig;
static Atom dwmfocus;
static Display *dpy;
static Window root;
char buf[256];

static XID 
parse_id(char *s)
{
    XID retval = None;
    char *fmt = "%ld";			/* since XID is long */

    if (s) {
	if (*s == '0') s++, fmt = "%lo";
	if (*s == 'x' || *s == 'X') s++, fmt = "%lx";
	sscanf (s, fmt, &retval);
    }
    return (retval);
}


static void
getname(Window w) {
    XChangeProperty(dpy, w, dwmfocus, XA_STRING, 8,
                PropModeReplace, (unsigned char *)'1', 0);
    XSync(dpy, False);
    return;

	char **list = NULL;
	int n;
	XTextProperty prop;
	XTextProperty name;

    name.nitems = 0;
    if(XGetTextProperty(dpy, w, &name, dwmfocus)){
        printf("this one focused");
    }
    name.nitems = 0;
    XGetTextProperty(dpy, w, &name, dwmconfig);
	if(name.nitems && name.encoding == XA_STRING) {
			strncpy(buf, (char *)name.value, sizeof buf - 1);
			buf[sizeof buf - 1] = '\0';
			XFree(name.value);
            puts(buf);
            for(n = 0;  n < sizeof buf - 2 && buf[n] != '\0' && buf[n] != '|'; n++){
                if(buf[n] == '1')
                    printf("tag[%d]", n);
            }
            n++;
            if(buf[n++] == '1')
                printf("v");
            if(buf[n++] == '1')
                printf("f");
            printf("\n");

    }
    bzero(buf,1024);
	prop.nitems = 0;
	buf[0] = 0;
	XGetTextProperty(dpy, w, &prop, netwmname);
	if(!prop.nitems)
		XGetWMName(dpy, w, &prop);
	if(!prop.nitems)
		return;
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
}

int
main(int argc, char *argv[]) {
	unsigned int i, num;
	Window *wins, d1, d2;
	XWindowAttributes wa;

	if((argc > 1) && !strncmp(argv[1], "-v", 3)) {
		fputs("focus, (C)opyright MMVI Anselm R. Garbe\n", stdout);
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
    getname(parse_id(argv[1]));
}
