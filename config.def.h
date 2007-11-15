/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
/* appearance */
#define BARPOS			BarOff /* BarBot, BarOff */
#define BORDERPX		4
#define FONT			"-artwiz-snap-*-*-*-*-10-*-*-*-*-*-koi8-r"
//#define NORMBORDERCOLOR		"#555753"
#define NORMBGCOLOR		"#2e3436"
#define NORMFGCOLOR		"#d3d7cf"
//#define SELBORDERCOLOR		"#555753"
//#define SELBORDERCOLOR		"#000000"
//#define SELBORDERCOLOR		"#676365"
#define SELBGCOLOR		"#3e4446"
#define SELFGCOLOR		"#d3d7cf"

//#define NORMBORDERCOLOR		"#9eeeee"
#define SELBORDERCOLOR		"#55aaaa"
#define NORMBORDERCOLOR		"#4E7878"

#define NMASTER 1
#define NCOLS 2
#define NROWS 1
#define TERMINAL "aterm"
#define ISTILE  isarrange(tile) || isarrange(ntile) || isarrange(dntile) || isarrange(tilecols)
/* tagging */
const char *tags[] = { "term", "www", "ncmpc", "gossip", "acme", "htop", "mutt", "8", "9" };
Bool seltags[LENGTH(tags)] = {[0] = True};
Rule rules[] = {
	/* class:instance:title regex	tags regex	isfloating */
	{ "Firefox",			"www",		True },
	{ "Gimp",			NULL,		True },
	{ "MPlayer",			NULL,		True },
	{ "Acroread",			NULL,		True },
	{ "ncmpc",			"ncmpc",		True },
	{ "acme",			"acme",		True },
	{ "gossip",			"gossip",		True },
	{ "htop",			"htop",		True },
	{ "mutt",			"mutt",		True },
	{ "stats",			"*",		True },
};

/* layout(s) */
#define MWFACT			0.6	/* master width factor [0.1 .. 0.9] */
#define RESIZEHINTS		False	/* False - respect size hints in tiled resizals */
#define SNAP			5	/* snap pixel */
Layout layouts[] = {
	/* symbol		function */
	{ "><>",		floating },
	{ "[]=",		tile }, /* first entry is default */
};

/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
Key keys[] = { \
	/* modifier			key		function	argument */ \
        { MODKEY,                       XK_p,       spawn,                      "pmenu" }, \
        { MODKEY,		        XK_t,	spawn, "exec aterm" }, \
	{ MODKEY,		        XK_y,	spawn,		            "ymenu" }, \
	{ MODKEY,		        XK_h,	spawn,		            "ssh-ui" }, \
	{ 0,                            XF86XK_AudioNext,          spawn,       "exec `player-control -f`" }, \
        { 0,                            XF86XK_AudioPrev,          spawn,       "exec `player-control -r`" }, \
        { 0,                            XF86XK_AudioPlay,          spawn,       "exec `player-control -t`" }, \
	{ MODKEY,			XK_space,	setlayout,	NULL }, \
	{ MODKEY,			XK_b,		togglebar,	NULL }, \
	{ MODKEY,			XK_j,		focusnext,	NULL }, \
	{ MODKEY,			XK_k,		focusprev,	NULL }, \
        { MODKEY,                       XK_minus,           setmwfact,         "-0.05" },\
        { MODKEY,                       XK_equal,           setmwfact,         "+0.05" }, \
        { MODKEY,			XK_m,		togglemax,	NULL }, \
	{ MODKEY,			XK_Return,	zoom,		NULL }, \
	{ MODKEY,			XK_Tab,		viewprevtag,	NULL }, \
	{ MODKEY|ShiftMask,		XK_space,	togglefloating,	NULL }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL }, \
	{ 0,			        XK_F12,		view,		NULL }, \
        { MODKEY,                       XK_d,       setnmaster,    "-1" }, \
        { MODKEY,                       XK_i,       setnmaster,    "1" }, \
	{ MODKEY,			XK_F1,		view,		tags[0] }, \
	{ MODKEY,		        XK_w,		focusview,	tags[1] }, \
	{ MODKEY,	        	XK_n,		focusview,	tags[2] }, \
        { MODKEY,		        XK_o,		focusview,	tags[3] }, \
	{ MODKEY,		        XK_a,		focusview,	tags[4] }, \
	{ MODKEY,		        XK_s,		focusview,	tags[5] }, \
	{ MODKEY,		        XK_u,		focusview,	tags[6] }, \
	{ MODKEY|ControlMask,		XK_8,		toggleview,	tags[7] }, \
	{ MODKEY|ControlMask,		XK_9,		toggleview,	tags[8] }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		NULL }, \
	{ MODKEY|ShiftMask,		XK_1,		tag,		tags[0] }, \
	{ MODKEY|ShiftMask,		XK_2,		tag,		tags[1] }, \
	{ MODKEY|ShiftMask,		XK_3,		tag,		tags[2] }, \
	{ MODKEY|ShiftMask,		XK_4,		tag,		tags[3] }, \
	{ MODKEY|ShiftMask,		XK_5,		tag,		tags[4] }, \
	{ MODKEY|ShiftMask,		XK_6,		tag,		tags[5] }, \
	{ MODKEY|ShiftMask,		XK_7,		tag,		tags[6] }, \
	{ MODKEY|ShiftMask,		XK_8,		tag,		tags[7] }, \
	{ MODKEY|ShiftMask,		XK_9,		tag,		tags[8] }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		NULL }, \
};
