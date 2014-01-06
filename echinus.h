/* enums */

enum {
	Manager, Utf8String, WMProto, WMDelete, WMName, WMState, WMChangeState,
	WMTakeFocus, MWMHints, ELayout, ESelTags, WindowFsMonitors, DeskGeometry,
	DeskViewport, ShowingDesktop, WMRestart, WMShutdown, RequestFrameExt,
	RestackWindow, StartupInfoBegin, StartupInfo, DeskLayout, WindowUserTime,
	UserTimeWindow, WindowNameVisible, WindowIconName, WindowIconNameVisible,
	WindowCounter, HandledIcons, WindowTypeOverride,
	/* _NET_SUPPORTED following */
	ClientList, ActiveWindow, WindowDesk, WindowDeskMask, NumberOfDesk, DeskNames,
	CurDesk, WorkArea,
	DeskModes, DeskModeFloating, DeskModeTiled, DeskModeBottomTiled, DeskModeMonocle,
	DeskModeTopTiled, DeskModeLeftTiled,
	ClientListStacking, WindowOpacity, MoveResizeWindow, WindowMoveResize,
	WindowExtents,
	WindowType, WindowTypeDesk, WindowTypeDock, WindowTypeToolbar, WindowTypeMenu,
	WindowTypeUtil, WindowTypeSplash, WindowTypeDialog, WindowTypeDrop,
	WindowTypePopup, WindowTypeTooltip, WindowTypeNotify, WindowTypeCombo,
	WindowTypeDnd, WindowTypeNormal,
	StrutPartial, Strut, WindowPid, WindowName,
	WindowState, WindowStateModal, WindowStateSticky, WindowStateMaxV,
	WindowStateMaxH, WindowStateShaded, WindowStateNoTaskbar, WindowStateNoPager,
	WindowStateHidden, WindowStateFs, WindowStateAbove, WindowStateBelow,
	WindowStateAttn, WindowStateFocused, WindowStateFixed, WindowStateFloating,
	WindowStateFilled,
	WindowActions, WindowActionAbove, WindowActionBelow, WindowActionChangeDesk,
	WindowActionClose, WindowActionFs, WindowActionMaxH, WindowActionMaxV,
	WindowActionMin, WindowActionMove, WindowActionResize, WindowActionShade,
	WindowActionStick, WindowActionFloat, WindowActionFill,
	WMCheck, CloseWindow, Supported,
	NATOMS
}; /* keep in sync with atomnames[] in ewmh.c */

#define WTFLAG(_type) (1<<((_type)-WindowTypeDesk))
#define WTCHECK(_client, _type) ((((_client)->wintype) & WTFLAG(_type)) ? True : False)

#define _XA_MANAGER				atom[Manager]
#define _XA_UTF8_STRING				atom[Utf8String]
#define _XA_WM_PROTOCOLS			atom[WMProto]
#define _XA_WM_DELETE_WINDOW			atom[WMDelete]
#define _XA_WM_NAME				atom[WMName]
#define _XA_WM_STATE				atom[WMState]
#define _XA_WM_CHANGE_STATE			atom[WMChangeState]
#define _XA_WM_TAKE_FOCUS			atom[WMTakeFocus]
#define _XA_MOTIF_WM_HINTS			atom[MWMHints]
#define _XA_ECHINUS_LAYOUT			atom[ELayout]
#define _XA_ECHINUS_SELTAGS			atom[ESelTags]
#define _XA_NET_WM_FULLSCREEN_MONITORS		atom[WindowFsMonitors]
#define _XA_NET_DESKTOP_GEOMETRY		atom[DeskGeometry]
#define _XA_NET_DESKTOP_VIEWPORT		atom[DeskViewport]
#define _XA_NET_SHOWING_DESKTOP			atom[ShowingDesktop]
#define _XA_NET_RESTART				atom[WMRestart]
#define _XA_NET_SHUTDOWN			atom[WMShutdown]
#define _XA_NET_REQUEST_FRAME_EXTENTS		atom[RequestFrameExt]
#define _XA_NET_RESTACK_WINDOW			atom[RestackWindow]
#define _XA_NET_STARTUP_INFO_BEGIN		atom[StartupInfoBegin]
#define _XA_NET_STARTUP_INFO			atom[StartupInfo]
#define _XA_NET_DESKTOP_LAYOUT			atom[DeskLayout]
#define _XA_NET_WM_USER_TIME			atom[WindowUserTime]
#define _XA_NET_WM_USER_TIME_WINDOW		atom[UserTimeWindow]
#define _XA_NET_WM_VISIBLE_NAME			atom[WindowNameVisible]
#define _XA_NET_WM_ICON_NAME			atom[WindowIconName]
#define _XA_NET_WM_VISIBLE_ICON_NAME		atom[WindowIconNameVisible]
#define _XA_NET_WM_SYNC_REQUEST_COUNTER		atom[WindowCounter]
#define _XA_NET_WM_HANDLED_ICONS		atom[HandledIcons]
#define _XA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE	atom[WindowTypeOverride]
/* _NET_SUPPORTED following */
#define _XA_NET_CLIENT_LIST			atom[ClientList]
#define _XA_NET_ACTIVE_WINDOW			atom[ActiveWindow]
#define _XA_NET_WM_DESKTOP			atom[WindowDesk]
#define _XA_NET_WM_DESKTOP_MASK			atom[WindowDeskMask]
#define _XA_NET_NUMBER_OF_DESKTOPS		atom[NumberOfDesk]
#define _XA_NET_DESKTOP_NAMES			atom[DeskNames]
#define _XA_NET_CURRENT_DESKTOP			atom[CurDesk]
#define _XA_NET_WORKAREA			atom[WorkArea]

#define _XA_NET_DESKTOP_MODES			atom[DeskModes]
#define _XA_NET_DESKTOP_MODE_FLOATING		atom[DeskModeFloating]
#define _XA_NET_DESKTOP_MODE_TILED		atom[DeskModeTiled]
#define _XA_NET_DESKTOP_MODE_BOTTOM_TILED	atom[DeskModeBottomTiled]
#define _XA_NET_DESKTOP_MODE_MONOCLE		atom[DeskModeMonocle]
#define _XA_NET_DESKTOP_MODE_TOP_TILED		atom[DeskModeTopTiled]
#define _XA_NET_DESKTOP_MODE_LEFT_TILED		atom[DeskModeLeftTiled]

#define _XA_NET_CLIENT_LIST_STACKING		atom[ClientListStacking]
#define _XA_NET_WM_WINDOW_OPACITY		atom[WindowOpacity]
#define _XA_NET_MOVERESIZE_WINDOW		atom[MoveResizeWindow]
#define _XA_NET_WM_MOVERESIZE			atom[WindowMoveResize]
#define _XA_NET_FRAME_EXTENTS			atom[WindowExtents]

#define _XA_NET_WM_WINDOW_TYPE			atom[WindowType]
#define _XA_NET_WM_WINDOW_TYPE_DESKTOP		atom[WindowTypeDesk]
#define _XA_NET_WM_WINDOW_TYPE_DOCK		atom[WindowTypeDock]
#define _XA_NET_WM_WINDOW_TYPE_TOOLBAR		atom[WindowTypeToolbar]
#define _XA_NET_WM_WINDOW_TYPE_MENU		atom[WindowTypeMenu]
#define _XA_NET_WM_WINDOW_TYPE_UTILITY		atom[WindowTypeUtil]
#define _XA_NET_WM_WINDOW_TYPE_SPLASH		atom[WindowTypeSplash]
#define _XA_NET_WM_WINDOW_TYPE_DIALOG		atom[WindowTypeDialog]
#define _XA_NET_WM_WINDOW_TYPE_DROPDOWN_MENU	atom[WindowTypeDrop]
#define _XA_NET_WM_WINDOW_TYPE_POPUP_MENU	atom[WindowTypePopup]
#define _XA_NET_WM_WINDOW_TYPE_TOOLTIP		atom[WindowTypeTooltip]
#define _XA_NET_WM_WINDOW_TYPE_NOTIFICATION	atom[WindowTypeNotify]
#define _XA_NET_WM_WINDOW_TYPE_COMBO		atom[WindowTypeCombo]
#define _XA_NET_WM_WINDOW_TYPE_DND		atom[WindowTypeDnd]
#define _XA_NET_WM_WINDOW_TYPE_NORMAL		atom[WindowTypeNormal]

#define _XA_NET_WM_STRUT_PARTIAL		atom[StrutPartial]
#define _XA_NET_WM_STRUT			atom[Strut]
#define _XA_NET_WM_PID				atom[WindowPid]
#define _XA_NET_WM_NAME				atom[WindowName]

#define _XA_NET_WM_STATE			atom[WindowState]
#define _XA_NET_WM_STATE_MODAL			atom[WindowStateModal]
#define _XA_NET_WM_STATE_STICKY			atom[WindowStateSticky]
#define _XA_NET_WM_STATE_MAXIMIZED_VERT		atom[WindowStateMaxV]
#define _XA_NET_WM_STATE_MAXIMIZED_HORZ		atom[WindowStateMaxH]
#define _XA_NET_WM_STATE_SHADED			atom[WindowStateShaded]
#define _XA_NET_WM_STATE_SKIP_TASKBAR		atom[WindowStateNoTaskbar]
#define _XA_NET_WM_STATE_SKIP_PAGER		atom[WindowStateNoPager]
#define _XA_NET_WM_STATE_HIDDEN			atom[WindowStateHidden]
#define _XA_NET_WM_STATE_FULLSCREEN		atom[WindowStateFs]
#define _XA_NET_WM_STATE_ABOVE			atom[WindowStateAbove]
#define _XA_NET_WM_STATE_BELOW			atom[WindowStateBelow]
#define _XA_NET_WM_STATE_DEMANDS_ATTENTION	atom[WindowStateAttn]
#define _XA_NET_WM_STATE_FOCUSED		atom[WindowStateFocused]
#define _XA_NET_WM_STATE_FIXED			atom[WindowStateFixed]
#define _XA_NET_WM_STATE_FLOATING		atom[WindowStateFloating]
#define _XA_NET_WM_STATE_FILLED			atom[WindowStateFilled]

#define _XA_NET_WM_ALLOWED_ACTIONS		atom[WindowActions]
#define _XA_NET_WM_ACTION_ABOVE			atom[WindowActionAbove]
#define _XA_NET_WM_ACTION_BELOW			atom[WindowActionBelow]
#define _XA_NET_WM_ACTION_CHANGE_DESKTOP	atom[WindowActionChangeDesk]
#define _XA_NET_WM_ACTION_CLOSE			atom[WindowActionClose]
#define _XA_NET_WM_ACTION_FULLSCREEN		atom[WindowActionFs]
#define _XA_NET_WM_ACTION_MAXIMIZE_HORZ		atom[WindowActionMaxH]
#define _XA_NET_WM_ACTION_MAXIMIZE_VERT		atom[WindowActionMaxV]
#define _XA_NET_WM_ACTION_MINIMIZE		atom[WindowActionMin]
#define _XA_NET_WM_ACTION_MOVE			atom[WindowActionMove]
#define _XA_NET_WM_ACTION_RESIZE		atom[WindowActionResize]
#define _XA_NET_WM_ACTION_SHADE			atom[WindowActionShade]
#define _XA_NET_WM_ACTION_STICK			atom[WindowActionStick]
#define _XA_NET_WM_ACTION_FLOAT			atom[WindowActionFloat]
#define _XA_NET_WM_ACTION_FILL			atom[WindowActionFill]

#define _XA_NET_SUPPORTING_WM_CHECK		atom[WMCheck]
#define _XA_NET_CLOSE_WINDOW			atom[CloseWindow]
#define _XA_NET_SUPPORTED			atom[Supported]

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
	int mx, my;
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
#define MFEATURES(_monitor, _which) ((_monitor) && FEATURES(M2LT(_monitor), (_which)))

typedef struct Client Client;
struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh, rb;	/* revert geometry */
	int sx, sy; /* static geometry */
	unsigned int sw, sh, sb;
	int th;			/* title height */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay, gravity;
	int ignoreunmap;
	long flags;
	int border, oldborder;
	int wintype;
	Bool isbanned, ismax, isfloating, wasfloating;
	Bool isicon, isfill, ismodal, isabove, isbelow, isattn;
	Bool isfixed, isbastard, isfocusable, hasstruts;
	Bool notaskbar, nopager, ismanaged;
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
unsigned int getwintype(Window win);
Bool checkwintype(Window win, int wintype);
void clientmessage(XEvent * e);
void ewmh_process_net_window_state(Client *c);
Atom *getatom(Window win, Atom atom, unsigned long *nitems);
long *getcard(Window win, Atom atom, unsigned long *nitems);
void initewmh(Window w);
void mwm_process_atom(Client * c);
void setopacity(Client * c, unsigned int opacity);
extern void (*updateatom[]) (Client *);
int getstruts(Client * c);

/* main */
void arrange(Monitor * m);
Monitor *clientmonitor(Client * c);
Monitor *curmonitor();
void *ecalloc(size_t nmemb, size_t size);
void *emallocz(size_t size);
void eprint(const char *errstr, ...);
const char *getresource(const char *resource, const char *defval);
Client *getclient(Window w, Client * list, int part);
Monitor *getmonitor(int x, int y);
void iconify(Client *c);
void incnmaster(const char *arg);
Bool isvisible(Client * c, Monitor * m);
void focus(Client * c);
void focusicon(const char *arg);
void focusnext(Client *c);
void focusprev(Client *c);
void focusview(int index);
void killclient(Client *c);
void applygravity(Client *c, int *xp, int *yp, int *wp, int *hp, int bw, int gravity);
void resize(Client * c, int x, int y, int w, int h, int b);
void configurerequest(XEvent * e);
void moveresizekb(Client *c, int dx, int dy, int dw, int dh);
void mousemove(Client *c);
void mouseresize(Client *c);
void quit(const char *arg);
void restart(const char *arg);
void setmwfact(const char *arg);
void setlayout(const char *arg);
void spawn(const char *arg);
void tag(Client *c, int index);
void togglestruts(const char *arg);
void togglefloating(Client *c);
void togglefill(Client *c);
void togglemax(Client *c);
void togglemonitor(const char *arg);
void toggletag(Client *c, int index);
void toggleview(int index);
void updateframe(Client *c);
void view(int index);
void viewlefttag(const char *arg);
void viewprevtag(const char *arg);
void viewrighttag(const char *arg);
void zoom(Client *c);
void selectionclear(XEvent *e);

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
