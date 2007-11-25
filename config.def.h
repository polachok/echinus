/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
/* appearance */
#define BARPOS			BarOff /* BarTop, BarOff */

/* border width */
#define BORDERPX		"4"

/* You can use
 * Dwm.normal.border: #cccccc
 * Dwm.selected.border: #ff0000
 * Dwm.border: 4
 * in Xresources instead
 */

#define SELBORDERCOLOR		"#55aaaa"
#define NORMBORDERCOLOR		"#9eeeee"

#define BARHEIGHT 13

#define NMASTER 1
#define TERMINAL "urxvt -e screen"
/* tagging */
const char *tags[] = { "0", "1", "2", "3", "ff", "ncmpc", "gossip", "gimp", "htop", "mutt", "acme" };
Bool seltags[LENGTH(tags)] = {[0] = True};
Rule rules[] = {
	/* class:instance:title regex	tags regex	isfloating */
	{ "Firefox",			"(ff|1)",		True },
    { "Password Gorilla",    NULL,   True },
	{ "Gimp",			"gimp",		True },
	{ "MPlayer",			NULL,		True },
	{ "Acroread",			NULL,		True },
	{ "ncmpc",			"ncmpc",		True },
	{ "acme",			"acme",		True },
	{ "gossip",			"gossip",		True },
	{ "gajim",			"gossip",		True },
	{ "htop",			"htop",		True },
	{ "mutt",			"(mutt|3)",		True },
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
    { MODKEY,           XK_p,       spawn,                      "pmenu" }, \
    { MODKEY,		    XK_t,	spawn, "exec urxvt -e screen" }, \
	{ MODKEY,		    XK_y,	spawn,		            "ymenu" }, \
	{ MODKEY,		    XK_h,	spawn,		            "ssh-ui" }, \
	{ 0,                XF86XK_AudioNext,          spawn,       "exec `player-control -f`" }, \
    { 0,                XF86XK_AudioPrev,          spawn,       "exec `player-control -r`" }, \
    { 0,                XF86XK_AudioPlay,          spawn,       "exec `player-control -t`" }, \
	{ MODKEY,			XK_space,	setlayout,	NULL }, \
	{ MODKEY,			XK_b,		togglebar,	NULL }, \
	{ MODKEY,			XK_j,		focusnext,	NULL }, \
	{ MODKEY,			XK_k,		focusprev,	NULL }, \
    { MODKEY,           XK_minus,           setmwfact,         "-0.05" },\
    { MODKEY,           XK_equal,           setmwfact,         "+0.05" }, \
    { MODKEY,			XK_m,		togglemax,	NULL }, \
	{ MODKEY,			XK_Return,	zoom,		NULL }, \
	{ MODKEY,			XK_Tab,		viewprevtag,	NULL }, \
	{ MODKEY|ShiftMask,		XK_space,	togglefloating,	NULL }, \
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL }, \
	{ 0,			        XK_F12,		view,		NULL }, \
    { MODKEY,           XK_d,       setnmaster,    "-1" }, \
    { MODKEY,           XK_i,       setnmaster,    "1" }, \
	{ MODKEY,			XK_F1,		view,		tags[0] }, \
	{ MODKEY,			XK_F2,		view,		tags[1] }, \
	{ MODKEY,			XK_F3,		view,		tags[2] }, \
	{ MODKEY,			XK_F4,		view,		tags[3] }, \
	{ MODKEY,		        XK_w,		focusview,	tags[4] }, \
	{ MODKEY,	        	XK_n,		focusview,	tags[5] }, \
    { MODKEY,		        XK_o,		focusview,	tags[6] }, \
	{ MODKEY,		        XK_g,		focusview,	tags[7] }, \
	{ MODKEY,		        XK_s,		focusview,	tags[8] }, \
	{ MODKEY,		        XK_u,		focusview,	tags[9] }, \
    { MODKEY,		        XK_a,		focusview,	tags[10] }, \
	{ MODKEY|ShiftMask,		XK_0,		tag,		NULL }, \
	{ MODKEY|ShiftMask,		XK_1,		toggletag,		tags[0] }, \
	{ MODKEY|ShiftMask,		XK_2,		toggletag,		tags[1] }, \
	{ MODKEY|ShiftMask,		XK_3,		toggletag,		tags[2] }, \
	{ MODKEY|ShiftMask,		XK_4,		toggletag,		tags[3] }, \
	{ MODKEY|ShiftMask,		XK_q,		quit,		NULL }, \
};
