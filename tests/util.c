#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include "util.h"

char *
estrdup(const char *str) {
	char *res = strdup(str);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", strlen(str));
	return res;
}

int
xerrordummy(Display *dpy, XErrorEvent *ee) {
    return 0;
}

void *
emalloc(unsigned int size) {
	void *res = malloc(size);

	if(!res)
		eprint("fatal: could not malloc() %u bytes\n", size);
	return res;
}

void *
emallocz(unsigned int size) {
    void *res = calloc(1, size);

    if(!res)
            eprint("fatal: could not malloc() %u bytes\n", size);
    return res;
}

void
eprint(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

unsigned long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		eprint("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

char *
getresource(const char *resource, char *defval, XrmDatabase xrdb) {
   static char name[256], class[256], *type;
   XrmValue value;
   snprintf(name, sizeof(name), "%s.%s", RESNAME, resource);
   snprintf(class, sizeof(class), "%s.%s", RESCLASS, resource);
   XrmGetResource(xrdb, name, class, &type, &value);
   if(value.addr)
           return value.addr;
   return defval;
}

void*
getatom(Window w, Atom atom, unsigned long *n) {
	int format, status;
	unsigned char *p = NULL;
	unsigned long tn, extra;
	Atom real;

	status = XGetWindowProperty(dpy, w, atom, 0L, 64L, False, AnyPropertyType,
			&real, &format, &tn, &extra, (unsigned char **)&p);
	if(status == BadWindow)
		return NULL;
        if(n!=NULL)
            *n = tn;
	return p;
}
#if 0
char **
getutf8prop(Window win, Atom atom, int *count) {
    Atom type;
    int format, i;
    unsigned long nitems;
    unsigned long bytes_after;
    char *s, **retval = NULL;
    int result;
    unsigned char *tmp = NULL;

    *count = 0;
    result = XGetWindowProperty(dpy, win, atom, 0, 64L, False,
          utf8_string, &type, &format, &nitems,
          &bytes_after, &tmp);
    if (result != Success || type != utf8_string || tmp == NULL)
        return NULL;

    if (nitems) {
        char *val = (char *) tmp;
        for (i = 0; i < nitems; i++) {
            if (!val[i])
                (*count)++;
        }
        retval = emalloc((*count + 2)*sizeof(char*));
        for (i = 0, s = val; i < *count; i++, s = s +  strlen (s) + 1) {
            retval[i] = estrdup(s);
        }
        if (val[nitems-1]) {
            result = nitems - (s - val);
            memmove(s - 1, s, result);
            val[nitems-1] = 0;
            retval[i] = estrdup(s - 1);
            (*count)++;
        }
    }
    XFree (tmp);

    return retval;

}
#endif

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!text || size == 0)
		return False;
	text[0] = '\0';
	if (BadWindow == XGetTextProperty(dpy, w, &name, atom))
            return False;
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
