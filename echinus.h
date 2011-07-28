/* enums */

enum { ClientList, ActiveWindow, WindowDesk,
	NumberOfDesk, DeskNames, CurDesk, ELayout, WorkArea,
	ClientListStacking, WindowOpacity, WindowType,
	WindowTypeDesk, WindowTypeDock, WindowTypeDialog, StrutPartial,
	ESelTags,
	WindowName, WindowState, WindowStateFs, WindowStateModal,
	WindowStateHidden,
	Utf8String, Supported, WMProto, WMDelete, WMName, WMState, WMTakeFocus,
	MWMHints, NATOMS
};

enum { LeftStrut, RightStrut, TopStrut, BotStrut, LastStrut };
enum { StrutsOn, StrutsOff, StrutsHide };	/* struts position */
enum { AlignLeft, AlignCenter, AlignRight };	/* title position */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */
enum { ColBorder, ColFG, ColBG, ColButton, ColLast };	/* color */
enum { Clk2Focus, SloppyFloat, AllSloppy, SloppyRaise };	/* focus model */
enum { ClientWindow, ClientTitle, ClientFrame };	/* client parts */
enum { Iconify, Maximize, Close, LastBtn };

/* typedefs */
typedef struct Monitor Monitor;
struct Monitor {
	int sx, sy, sw, sh, wax, way, waw, wah;
	unsigned int curtag;
	unsigned long struts[LastStrut];
	Bool *seltags;
	Bool *prevtags;
	Monitor *next;
};

typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int th;			/* title window */
	int rx, ry, rw, rh;	/* revert geometry */
	int sfx, sfy, sfw, sfh;	/* stored float geometry, used on mode revert */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay;
	long flags;
	int border, oldborder;
	Bool isbanned, isfixed, ismax, isfloating, wasfloating, isicon;
	Bool isplaced, isbastard, isfocusable;
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

typedef struct {
	Pixmap pm;
	int px, py;
	unsigned int pw, ph;
	int x;
	void (*action) (const char *arg);
} Button;

typedef struct {
	unsigned int borderpx;
	unsigned int drawoutline;
	float uf_opacity;
	char titlelayout[32];
	Button button[LastBtn];
} Look;

typedef struct {
	unsigned int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
	XftColor *xftnorm;
	XftColor *xftsel;
	GC gc;
	struct {
		XftFont *xftfont;
		XGlyphInfo *extents;
		int ascent;
		int descent;
		int height;
		int width;
	} font;
} DC;				/* draw context */

typedef struct {
	unsigned long mod;
	KeySym keysym;
	void (*func) (const char *arg);
	const char *arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange) (Monitor * m);
} Layout;

typedef struct {
	char *prop;
	char *tags;
	Bool isfloating;
	Bool hastitle;
} Rule;

typedef struct {
	regex_t *propregex;
	regex_t *tagregex;
} Regs;

/* ewmh.c */
void clientmessage(XEvent * e);
void initatom(void);
int updatestruts(Window win);
void setopacity(Client * c, unsigned int opacity);
void mwm_process_atom(Client * c);
void ewmh_process_state_atom(Client * c, Atom state, int set);
extern void (*updateatom[]) (Client *);
Bool checkatom(Window win, Atom bigatom, Atom smallatom);

/* main */
void eprint(const char *errstr, ...);
unsigned int textnw(const char *text, unsigned int len);
unsigned int textw(const char *text);
Bool isvisible(Client * c, Monitor * m);
void *emallocz(unsigned int size);
Monitor *clientmonitor(Client * c);
void iconifyit(const char *arg);
const char *getresource(const char *resource, const char *defval);
void togglemax(const char *arg);
void killclient(const char *arg);
void floating(Monitor * m);	/* default floating layout */
void ifloating(Monitor * m);	/* intellectual floating layout try */
Monitor *curmonitor();
Client *getclient(Window w, Client * list, int part);
void focus(Client * c);
void view(const char *arg);
void viewprevtag(const char *arg);	/* views previous selected tags */
void viewlefttag(const char *arg);
void arrange(Monitor * m);
Monitor *getmonitor(int x, int y);
void setlayout(const char *arg);
void incnmaster(const char *arg);
void focusnext(const char *arg);
void focusprev(const char *arg);
void bstack(Monitor * m);
void moveresizekb(const char *arg);
unsigned long getcolor(const char *colstr);
void monocle(Monitor * m);
void quit(const char *arg);
void restart(const char *arg);
void setmwfact(const char *arg);
void spawn(const char *arg);
void tag(const char *arg);
void tile(Monitor * m);
void togglestruts(const char *arg);
void togglefloating(const char *arg);
void togglefill(const char *arg);
void toggletag(const char *arg);
void toggleview(const char *arg);
void togglemonitor(const char *arg);
void focusview(const char *arg);
void viewrighttag(const char *arg);
void zoom(const char *arg);

/* parse.c */
void initrules();
int initkeys();

/* draw.c */
void initfont(const char *fontstr);
void initbuttons();

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

#define ISLTFLOATING(m) (m && ((layouts[ltidxs[m->curtag]].arrange == floating) || (layouts[ltidxs[m->curtag]].arrange == ifloating)))

#define OPAQUE			0xffffffff

/* globals */
extern Atom atom[NATOMS];
extern Display *dpy;
extern Window root;
extern unsigned int *ltidxs;
extern Client *clients;
extern Monitor *monitors;
extern Client *sel;
extern Client *stack;
extern unsigned int ntags;
extern unsigned int nkeys;
extern unsigned int nrules;
extern int screen;
extern Look look;
extern char **tags;
extern Key **keys;
extern Rule **rules;
extern Layout layouts[];
extern DC dc;
extern Bool dectiled;
extern unsigned int modkey;
