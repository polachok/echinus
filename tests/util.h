/* forward declarations */
char* estrdup(const char *str);
int xerrordummy(Display *dpy, XErrorEvent *ee);
void* emalloc(unsigned int size);
void eprint(const char *errstr, ...);
unsigned long getcolor(const char *colstr);
char *getresource(const char *resource, char *defval, XrmDatabase xrdb);
void* getatom(Window w, Atom atom, unsigned long *n);
#if 0
char** getutf8prop(Window win, Atom atom, int *count);
#endif
Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
unsigned int textnw(const char *text, unsigned int len);
unsigned int textw(const char *text);
#ifndef RESNAME
#define RESNAME ""
#endif

#ifndef RESCLASS
#define RESCLASS ""
#endif

extern Display *dpy;
extern int screen;
extern Window root;
extern void *emallocz(unsigned int size);
