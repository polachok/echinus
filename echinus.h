/* enums */

#ifdef STARTUP_NOTIFICATION
#define SN_API_NOT_YET_FROZEN
#include <libsn/sn.h>
#endif

enum {
	Manager, Utf8String, WMProto, WMDelete, WMState, WMChangeState,
	WMTakeFocus, MWMHints, ELayout, ESelTags,
	WMRestart, WMShutdown,
	DeskLayout,
	/* _WIN_PROTOCOLS following */
	WinAppState, WinAreaCount, WinArea, WinClientList, WinClientMoving,
	WinButtonProxy, WinExpandedSize, WinFocus, WinHints, WinIcons, WinLayer, WinMaxGeom,
	WinProtocols, WinResize, WinState, WinCheck, WinWorkarea, WinWorkCount, WinWorkNames,
	WinWorkspace, WinWorkspaces, SwmVroot,
	/* _NET_SUPPORTED following */
	ClientList, ActiveWindow, WindowDesk, WindowDeskMask, NumberOfDesk, DeskNames,
	CurDesk, WorkArea, DeskViewport, ShowingDesktop, DeskGeometry, DesksVisible,
	MonitorGeometry,
	DeskModes, DeskModeFloating, DeskModeTiled, DeskModeBottomTiled, DeskModeMonocle,
	DeskModeTopTiled, DeskModeLeftTiled,
	ClientListStacking, WindowOpacity, MoveResizeWindow, RestackWindow, WindowMoveResize,
	WindowExtents, HandledIcons, RequestFrameExt, VirtualRoots,
	WindowType, WindowTypeDesk, WindowTypeDock, WindowTypeToolbar, WindowTypeMenu,
	WindowTypeUtil, WindowTypeSplash, WindowTypeDialog, WindowTypeDrop,
	WindowTypePopup, WindowTypeTooltip, WindowTypeNotify, WindowTypeCombo,
	WindowTypeDnd, WindowTypeNormal,
	StrutPartial, Strut, WindowPid, WindowName, WindowNameVisible, WindowIconName,
	WindowIconNameVisible, WindowUserTime, UserTimeWindow, NetStartupId,
	StartupInfo, StartupInfoBegin,
	WindowSync, WindowCounter, WindowFsMonitors,
	WindowState, WindowStateModal, WindowStateSticky, WindowStateMaxV,
	WindowStateMaxH, WindowStateShaded, WindowStateNoTaskbar, WindowStateNoPager,
	WindowStateHidden, WindowStateFs, WindowStateAbove, WindowStateBelow,
	WindowStateAttn, WindowStateFocused, WindowStateFixed, WindowStateFloating,
	WindowStateFilled,
	WindowActions, WindowActionAbove, WindowActionBelow, WindowActionChangeDesk,
	WindowActionClose, WindowActionFs, WindowActionMaxH, WindowActionMaxV,
	WindowActionMin, WindowActionMove, WindowActionResize, WindowActionShade,
	WindowActionStick, WindowActionFloat, WindowActionFill,
	WMCheck, CloseWindow, WindowPing, Supported,
	SystemTrayWindows, WindowFrameStrut, WindowForSysTray, WindowTypeOverride,
	KdeSplashProgress, WindowChangeState,
	NATOMS
}; /* keep in sync with atomnames[] in ewmh.c */

#define WTFLAG(_type) (1<<((_type)-WindowTypeDesk))
#define WTTEST(_wintype, _type) (((_wintype) & WTFLAG(_type)) ? True : False)
#define WTCHECK(_client, _type) WTTEST((_client)->wintype, (_type))

#define _XA_MANAGER				atom[Manager]
#define _XA_UTF8_STRING				atom[Utf8String]
#define _XA_WM_PROTOCOLS			atom[WMProto]
#define _XA_WM_DELETE_WINDOW			atom[WMDelete]
#define _XA_WM_STATE				atom[WMState]
#define _XA_WM_CHANGE_STATE			atom[WMChangeState]
#define _XA_WM_TAKE_FOCUS			atom[WMTakeFocus]
#define _XA_MOTIF_WM_HINTS			atom[MWMHints]
#define _XA_ECHINUS_LAYOUT			atom[ELayout]
#define _XA_ECHINUS_SELTAGS			atom[ESelTags]
#define _XA_NET_RESTART				atom[WMRestart]
#define _XA_NET_SHUTDOWN			atom[WMShutdown]
#define _XA_NET_DESKTOP_LAYOUT			atom[DeskLayout]
/* _WIN_PROTOCOLS following */
#define _XA_WIN_APP_STATE			atom[WinAppState]
#define _XA_WIN_AREA_COUNT			atom[WinAreaCount]
#define _XA_WIN_AREA				atom[WinArea]
#define _XA_WIN_CLIENT_LIST			atom[WinClientList]
#define _XA_WIN_CLIENT_MOVING			atom[WinClientMoving]
#define _XA_WIN_DESKTOP_BUTTON_PROXY		atom[WinButtonProxy]
#define _XA_WIN_EXPANDED_SIZE			atom[WinExpandedSize]
#define _XA_WIN_FOCUS				atom[WinFocus]
#define _XA_WIN_HINTS				atom[WinHints]
#define _XA_WIN_ICONS				atom[WinIcons]
#define _XA_WIN_LAYER				atom[WinLayer]
#define _XA_WIN_MAXIMIZED_GEOMETRY		atom[WinMaxGeom]
#define _XA_WIN_PROTOCOLS			atom[WinProtocols]
#define _XA_WIN_RESIZE				atom[WinResize]
#define _XA_WIN_STATE				atom[WinState]
#define _XA_WIN_SUPPORTING_WM_CHECK		atom[WinCheck]
#define _XA_WIN_WORKAREA			atom[WinWorkarea]
#define _XA_WIN_WORKSPACE_COUNT			atom[WinWorkCount]
#define _XA_WIN_WORKSPACE_NAMES			atom[WinWorkNames]
#define _XA_WIN_WORKSPACE			atom[WinWorkspace]
#define _XA_WIN_WORKSPACES			atom[WinWorkspaces]
#define _XA_SWM_VROOT				atom[SwmVroot]
/* _NET_SUPPORTED following */
#define _XA_NET_CLIENT_LIST			atom[ClientList]
#define _XA_NET_ACTIVE_WINDOW			atom[ActiveWindow]
#define _XA_NET_WM_DESKTOP			atom[WindowDesk]
#define _XA_NET_WM_DESKTOP_MASK			atom[WindowDeskMask]
#define _XA_NET_NUMBER_OF_DESKTOPS		atom[NumberOfDesk]
#define _XA_NET_DESKTOP_NAMES			atom[DeskNames]
#define _XA_NET_CURRENT_DESKTOP			atom[CurDesk]
#define _XA_NET_WORKAREA			atom[WorkArea]
#define _XA_NET_DESKTOP_VIEWPORT		atom[DeskViewport]
#define _XA_NET_SHOWING_DESKTOP			atom[ShowingDesktop]
#define _XA_NET_DESKTOP_GEOMETRY		atom[DeskGeometry]
#define _XA_NET_VISIBLE_DESKTOPS		atom[DesksVisible]
#define _XA_NET_MONITOR_GEOMETRY		atom[MonitorGeometry]

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
#define _XA_NET_RESTACK_WINDOW			atom[RestackWindow]
#define _XA_NET_WM_MOVERESIZE			atom[WindowMoveResize]
#define _XA_NET_FRAME_EXTENTS			atom[WindowExtents]
#define _XA_NET_WM_HANDLED_ICONS		atom[HandledIcons]
#define _XA_NET_REQUEST_FRAME_EXTENTS		atom[RequestFrameExt]
#define _XA_NET_VIRTUAL_ROOTS			atom[VirtualRoots]

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
#define _XA_NET_WM_VISIBLE_NAME			atom[WindowNameVisible]
#define _XA_NET_WM_ICON_NAME			atom[WindowIconName]
#define _XA_NET_WM_VISIBLE_ICON_NAME		atom[WindowIconNameVisible]
#define _XA_NET_WM_USER_TIME			atom[WindowUserTime]
#define _XA_NET_WM_USER_TIME_WINDOW		atom[UserTimeWindow]
#define _XA_NET_STARTUP_ID			atom[NetStartupId]
#define _XA_NET_STARTUP_INFO			atom[StartupInfo]
#define _XA_NET_STARTUP_INFO_BEGIN		atom[StartupInfoBegin]
#define _XA_NET_WM_SYNC_REQUEST			atom[WindowSync]
#define _XA_NET_WM_SYNC_REQUEST_COUNTER		atom[WindowCounter]
#define _XA_NET_WM_FULLSCREEN_MONITORS		atom[WindowFsMonitors]

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
#define _XA_NET_WM_PING				atom[WindowPing]
#define _XA_NET_SUPPORTED			atom[Supported]

#define _XA_KDE_NET_SYSTEM_TRAY_WINDOWS		atom[SystemTrayWindows]
#define _XA_KDE_NET_WM_FRAME_STRUT		atom[WindowFrameStrut]
#define _XA_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR	atom[WindowForSysTray]
#define _XA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE	atom[WindowTypeOverride]
#define _XA_KDE_SPLASH_PROGRESS			atom[KdeSplashProgress]
#define _XA_KDE_WM_CHANGE_STATE			atom[WindowChangeState]

enum { LeftStrut, RightStrut, TopStrut, BotStrut, LastStrut }; /* ewmh struts */
enum { ColFG, ColBG, ColBorder, ColButton, ColLast };	/* colors */
enum { ClientWindow, ClientTitle, ClientFrame, ClientTimeWindow, ClientGroup,
       ClientTransFor, ClientLeader, ClientAny, SysTrayWindows, ClientPing,
       ClientDead, PartLast };	/* client parts */
enum { Iconify, Maximize, Close, LastBtn }; /* window buttons */
typedef enum { CauseDestroyed, CauseUnmapped, CauseReparented,
	       CauseQuitting, CauseSwitching, CauseRestarting } WithdrawCause;

typedef struct {
	int x, y, w, h, b;
} Geometry;

typedef struct {
	int x, y, w, h;
} Workarea;

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
	unsigned int num, index;
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
	char *name;
	char *icon_name;
	char *startup_id;
	int x, y, w, h, border;
	int rx, ry, rw, rh, rb;	/* restore geometry */
	int sx, sy, sw, sh, sb; /* static geometry */
	int th;			/* title height */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, maxax, minay, maxay, gravity;
	int ignoreunmap;
	long flags;
	int wintype;
	Bool isbanned, ismax, isfloating, wasfloating, ismaxv, ismaxh, isshade;
	Bool isicon, isfill, ismodal, isabove, isbelow, isattn, issticky, ishidden;
	Bool isfixed, isbastard, isfocusable, hasstruts, hastime;
	Bool notaskbar, nopager, ismanaged;
	Bool *tags;
	Client *next;
	Client *prev;
	Client *snext;
	Client *cnext;
	Window win;
	Window title;
	Window frame;
	Window time_window;
	Window leader;
	Window transfor;
	Time user_time;
	Pixmap drawable;
	XftDraw *xftdraw;
	XID sync;
#ifdef STARTUP_NOTIFICATION
	SnStartupSequence *seq;
#endif
};

typedef struct Group Group;
struct Group {
	Window *members;
	unsigned int count;
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

#ifdef STARTUP_NOTIFICATION
typedef struct Notify Notify;
struct Notify {
	Notify *next;
	SnStartupSequence *seq;
	Bool assigned;
};
#endif

/* ewmh.c */
Bool checkatom(Window win, Atom bigatom, Atom smallatom);
unsigned int getwintype(Window win);
Bool checkwintype(Window win, int wintype);
void clientmessage(XEvent * e);
void ewmh_process_net_window_state(Client *c);
void ewmh_process_net_desktop_names(void);
void ewmh_process_net_showing_desktop(void);
unsigned int ewmh_process_net_number_of_desktops(void);
void ewmh_release_user_time_window(Client *c);
Atom *getatom(Window win, Atom atom, unsigned long *nitems);
long *getcard(Window win, Atom atom, unsigned long *nitems);
void initewmh(Window w);
void exitewmh(WithdrawCause cause);
void ewmh_add_client(Client *c);
void ewmh_del_client(Client *c, WithdrawCause cause);
void mwm_process_atom(Client * c);
void setopacity(Client * c, unsigned int opacity);
extern void (*updateatom[]) (Client *);
int getstruts(Client * c);

/* main */
void inittag(unsigned int i);
void arrange(Monitor * m);
Monitor *clientmonitor(Client * c);
Monitor *curmonitor();
void *ecalloc(size_t nmemb, size_t size);
void *emallocz(size_t size);
void *erealloc(void *ptr, size_t size);
void eprint(const char *errstr, ...);
const char *getresource(const char *resource, const char *defval);
Client *getclient(Window w, int part);
Monitor *getmonitor(int x, int y);
Bool gettextprop(Window w, Atom atom, char **text);
void iconify(Client *c);
void incnmaster(const char *arg);
Bool isvisible(Client * c, Monitor * m);
Monitor *findmonbynum(int num);
void focus(Client * c);
void focusicon(const char *arg);
void focusnext(Client *c);
void focusprev(Client *c);
void focusview(int index);
void killclient(Client *c);
void applygravity(Client *c, int *xp, int *yp, int *wp, int *hp, int bw, int gravity);
void resize(Client * c, int x, int y, int w, int h, int b);
void restack(void);
void restack_client(Client *c, int stack_mode, Client *sibling);
void configurerequest(XEvent * e);
void moveresizekb(Client *c, int dx, int dy, int dw, int dh);
void mousemove(Client *c, unsigned int button, int x_root, int y_root);
void mouseresize_from(Client *c, int from, unsigned int button, int x_root, int y_root);
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
void togglemaxv(Client *c);
void togglemaxh(Client *c);
void toggleshade(Client *c);
void togglemonitor(const char *arg);
void toggletag(Client *c, int index);
void toggleview(int index);
void toggleshowing(void);
void togglehidden(Client *c);
void updateframe(Client *c);
void view(int index);
void viewlefttag(const char *arg);
void viewprevtag(const char *arg);
void viewrighttag(const char *arg);
void zoom(Client *c);
void selectionclear(XEvent *e);
void appendtag(const char *arg);
void rmlasttag(const char *arg);
void settags(unsigned int numtags);

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
extern Client *clist;
extern unsigned int ntags;
extern unsigned int nkeys;
extern unsigned int nrules;
extern Bool showing_desktop;
extern int screen;
extern Style style;
extern Button button[LastBtn];
extern char **tags;
extern Key **keys;
extern Rule **rules;
extern Layout layouts[];
extern unsigned int modkey;
extern View *views;
extern XContext context[];
extern Time user_time;
#ifdef STARTUP_NOTIFICATION
extern SnDisplay *sn_dpy;
extern SnMonitorContext *sn_ctx;
extern Notify *notifies;
#endif
