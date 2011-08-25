/* enums */

enum { ClientList, ActiveWindow, WindowDesk,
	NumberOfDesk, DeskNames, CurDesk, ELayout, WorkArea,
	ClientListStacking, WindowOpacity, WindowType,
	WindowTypeDesk, WindowTypeDock, WindowTypeDialog, StrutPartial,
	ESelTags,
	WindowName, WindowState, WindowStateFs, WindowStateModal,
	WindowStateHidden, WMCheck,
	Utf8String, Supported, WMProto, WMDelete, WMName, WMState, WMChangeState,
	WMTakeFocus, MWMHints, NATOMS
}; /* keep in sync with atomnames[][] in ewmh.c */

enum { LeftStrut, RightStrut, TopStrut, BotStrut, LastStrut }; /* ewmh struts */
enum { ColFG, ColBG, ColBorder, ColButton, ColLast };	/* colors */
enum { ClientWindow, ClientTitle, ClientFrame };	/* client parts */
enum { Iconify, Maximize, Close, LastBtn }; /* window buttons */

/* typedefs */
typedef struct Monitor Monitor;
struct Monitor {
	int sx, sy, sw, sh, wax, way, waw, wah;
	unsigned long struts[LastStrut];
	Bool *seltags;
	Bool *prevtags;
	Monitor *next;
	unsigned int curtag;
};

typedef struct {
	void (*arrange) (Monitor * m);
	char symbol;
	int features;
#define BIT(_i)	(1 << (_i))
#define MWFACT	BIT(0)
#define NMASTER	BIT(1)
#define	ZOOM	BIT(2)
#define	OVERLAP	BIT(3)
} Layout;

#define FEATURES(_layout, _which) (!(!((_layout)->features & (_which))))
#define M2LT(_mon) (views[(_mon)->curtag].layout)
#define MFEATURES(_monitor, _which) (FEATURES(M2LT(_monitor), (_which)))

typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh;	/* revert geometry */
	int th;			/* title height */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	int ignoreunmap;
	long flags;
	int border, oldborder;
	Bool isbanned, ismax, isfloating, wasfloating;
	Bool isplaced, isicon, isfill;
	Bool isfixed, isbastard, isfocusable;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
	Window title;
	Window frame;
	Pixmap drawable;
	XftDraw *xftdraw;
};

typedef struct View {
	int barpos;
	int nmaster;
	double mwfact;
	Layout *layout;
} View; /* per-tag settings */

typedef struct {
	Pixmap pm;
	int px, py;
	unsigned int pw, ph;
	int x;
	int pressed;
	void (*action) (const char *arg);
} Button; /* window buttons */

typedef struct {
	unsigned int border;
	unsigned int outline;
	unsigned int titleheight;
	unsigned int opacity;
	char titlelayout[32];
	struct {
		unsigned long norm[ColLast];
		unsigned long sel[ColLast];
		XftColor *font[2];
	} color;
	XftFont *font;
} Style;

typedef struct {
	unsigned long mod;
	KeySym keysym;
	void (*func) (const char *arg);
	const char *arg;
} Key; /* keyboard shortcuts */

typedef struct {
	char *prop;
	char *tags;
	Bool isfloating;
	Bool hastitle;
	regex_t *propregex;
	regex_t *tagregex;
} Rule; /* window matching rules */

/* ewmh.c */
Bool checkatom(Window win, Atom bigatom, Atom smallatom);
void clientmessage(XEvent * e);
void ewmh_process_state_atom(Client * c, Atom state, int set);
void *getatom(Window win, Atom atom, unsigned long *nitems);
void initewmh(void);
void mwm_process_atom(Client * c);
void setopacity(Client * c, unsigned int opacity);
extern void (*updateatom[]) (Client *);
int updatestruts(Client * c);

/* main */
void arrange(Monitor * m);
Monitor *clientmonitor(Client * c);
Monitor *curmonitor();
void *emallocz(unsigned int size);
void eprint(const char *errstr, ...);
const char *getresource(const char *resource, const char *defval);
Client *getclient(Window w, Client * list, int part);
Monitor *getmonitor(int x, int y);
void iconify(const char *arg);
void incnmaster(const char *arg);
Bool isvisible(Client * c, Monitor * m);
void focus(Client * c);
void focusicon(const char *arg);
void focusnext(const char *arg);
void focusprev(const char *arg);
void focusview(const char *arg);
void killclient(const char *arg);
void moveresizekb(const char *arg);
void quit(const char *arg);
void restart(const char *arg);
void setmwfact(const char *arg);
void setlayout(const char *arg);
void spawn(const char *arg);
void tag(const char *arg);
void togglestruts(const char *arg);
void togglefloating(const char *arg);
void togglefill(const char *arg);
void togglemax(const char *arg);
void togglemonitor(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void view(const char *arg);
void viewlefttag(const char *arg);
void viewprevtag(const char *arg);
void viewrighttag(const char *arg);
void zoom(const char *arg);

/* parse.c */
void initrules();
int initkeys();

/* draw.c */
void drawclient(Client * c);
void deinitstyle();
void initstyle();

/* XXX: this block of defines must die */
#define curseltags curmonitor()->seltags
#define curprevtags curmonitor()->prevtags
#define cursx curmonitor()->sx
#define cursy curmonitor()->sy
#define cursh curmonitor()->sh
#define cursw curmonitor()->sw
#define curwax curmonitor()->wax
#define curway curmonitor()->way
#define curwaw curmonitor()->waw
#define curwah curmonitor()->wah
#define curmontag curmonitor()->curtag
#define curstruts curmonitor()->struts
#define curlayout views[curmontag].layout

#define LENGTH(x)		(sizeof(x) / sizeof x[0])
#ifdef DEBUG
#define DPRINT			fprintf(stderr, "%s: %s() %d\n",__FILE__,__func__, __LINE__);
#define DPRINTF(format, ...)	fprintf(stderr, "%s %s():%d " format, __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define DPRINT			;
#define DPRINTF(format, ...)
#endif
#define DPRINTCLIENT(c) DPRINTF("%s: x: %d y: %d w: %d h: %d th: %d f: %d b: %d m: %d\n", \
				    c->name, c->x, c->y, c->w, c->h, c->th, c->isfloating, c->isbastard, c->ismax)

#define OPAQUE			0xffffffff
#define RESNAME		       "echinus"
#define RESCLASS	       "Echinus"
#define STR(_s)			TOSTR(_s)
#define TOSTR(_s)		#_s
#define min(_a, _b)		((_a) < (_b) ? (_a) : (_b))
#define max(_a, _b)		((_a) > (_b) ? (_a) : (_b))

/* globals */
extern Atom atom[NATOMS];
extern Display *dpy;
extern Window root;
extern Client *clients;
extern Monitor *monitors;
extern Client *sel;
extern Client *stack;
extern unsigned int ntags;
extern unsigned int nkeys;
extern unsigned int nrules;
extern int screen;
extern Style style;
extern Button button[LastBtn];
extern char **tags;
extern Key **keys;
extern Rule **rules;
extern Layout layouts[];
extern unsigned int modkey;
extern View *views;
