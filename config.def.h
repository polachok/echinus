/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
/* appearance */
#define BARPOS			BarOff /* BarTop, BarOff */

/* border width */
#define BORDERPX		"1"
/* You can use
 * Dwm.normal.border: #cccccc
 * Dwm.selected.border: #ff0000
 * Dwm.border: 4
 * in Xresources instead
 */
#define FONT            "-*-snap-*-*-*-*-11-*-*-*-*-*-iso10646-*"
#if 0
#define NORMBORDERCOLOR		"#262626"
#define NORMBGCOLOR		"#262626"
#define NORMFGCOLOR		"#d3d7cf"
//#define SELBORDERCOLOR		"#555753"
//#define SELBORDERCOLOR		"#000000"
#define SELBORDERCOLOR		"#161616"
#define SELBGCOLOR		"#262626"
#define SELFGCOLOR		"#d3d7cf"
#endif
#define NORMBORDERCOLOR		"#cccccc"
#define NORMBGCOLOR		"#cccccc"
#define NORMFGCOLOR		"#000000"
#define SELBORDERCOLOR		"#0066ff"
#define SELBGCOLOR		"#0066ff"
#define SELFGCOLOR		"#ffffff"

#define BLEFTPIXMAP "min.xbm"
#define BRIGHTPIXMAP "min.xbm"
#define BCENTERPIXMAP "min.xbm"

#define BARHEIGHT 12
#define TITLEBARHEIGHT 17

#define NMASTER 1
#define TERMINAL "urxvt -e screen"
/* tagging */
const char *tags[] = { "main", "web", "irc", "mail", "dev", "misc", "im", "gfx" };
Bool seltags[LENGTH(tags)] = {[0] = True};
Rule rules[] = { \
	/* class:instance:title regex	tags regex	isfloat */ \
    { "Firefox.*",          "web",      False }, \
    { "Gajim*",            "im",       True }, \
    { "Gossip.*",            "im",       True }, \
    { "WeeChat.*",            "irc",       False }, \
    { "Firefox-bin.*",          "web",      False }, \
    { "Flock-bin.*",            "web",      False }, \
    { "MPlayer.*",          NULL,       True}, \
    { "gl2.*",          NULL,       True}, \
    { "Sylpheed.*",      "mail", False }, \
    { "mutt.*",      "mail", False }, \
    { "Gimp.*",         "gfx",      True}, \
    { "feh.*",         NULL,       True}, \
    { "htop.*",     "misc",     True}, \
    { "ncmpc.*",     "misc",     True}, \
};

/* layout(s) */
#define MWFACT			0.6	/* master width factor [0.1 .. 0.9] */
#define RESIZEHINTS		False	/* False - respect size hints in tiled resizals */
#define SNAP			5	/* snap pixel */
Layout layouts[] = {
	/* symbol		function */
	{ "=",		monocle }, /* first entry is default */
	{ "~",		floating },
	{ "#",		tile }, 
};
#define NMASTER 1
/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
Key keys[] = { \
	/* modifier			key		function	arguments */ \
	/* { MODKEY,		    XK_t,	spawn,		            "osdexec urxvt" }, \ */ \
	{ MODKEY,		    XK_t,	spawn,		            "osdexec urxvt -e screen -U" }, \
	{ MODKEY,		    XK_n,	spawn,	                    "exec np" }, \
	{ MODKEY,		    XK_b,	togglebar,	            NULL }, \
	{ 0,                        XF86XK_AudioNext,          spawn,       "exec `player-control -f`" }, \
        { 0,                        XF86XK_AudioPrev,          spawn,       "exec `player-control -r`" }, \
        { 0,                        XF86XK_AudioPlay,          spawn,       "exec `player-control -t`" }, \
	{ MODKEY,		    XK_l,	spawn,		            "sleep 3 ; slock" }, \
	{ MODKEY,		    XK_u,	spawn,		            "killall unclutter||unclutter -idle 1" }, \
	{ MODKEY|ControlMask,	    XK_Delete,	spawn,		            "exec sudo /sbin/reboot" }, \
	{ MODKEY,		    XK_y,	spawn,		            "ymenu" }, \
	{ MODKEY,		    XK_h,	spawn,		            "ssh-ui" }, \
    { MODKEY,                   XK_p,       spawn,                      "pmenu" }, \
    { MODKEY,                   XK_w,       spawn,                      "swarp 1280 900" }, \
    { MODKEY,                   XK_r,       spawn,                      "gajim-remote toggle_roster_appearance" }, \
	{ MODKEY,		    XK_f,	setlayout,	"~" }, \
	{ MODKEY,		    XK_m,	setlayout,	"=" }, \
	{ MODKEY,		    XK_r,	setlayout,	"#" }, \
    { MODKEY,		    XK_j,		focusnext,	    NULL  }, \
	{ MODKEY,		    XK_k,		focusprev,	    NULL }, \
	{ MODKEY,		    XK_Return,   	zoom,		    NULL }, \
        { MODKEY,                   XK_minus,           setmwfact,         "-0.05" },\
        { MODKEY,                   XK_equal,           setmwfact,         "+0.05" }, \
	{ MODKEY,		XK_d,		incnmaster,	"-1" }, \
	{ MODKEY,		XK_i,		incnmaster,	"1" }, \
	{ MODKEY|ShiftMask,	    XK_c,		killclient,	 NULL  }, \
        { MODKEY,                   XK_space,           togglefloating,  NULL, }, \
        { MODKEY|ShiftMask,	    XK_q,		quit,		 NULL  }, \
	{ MODKEY|ShiftMask,	    XK_1,		tag,		  tags[0] }, \
	{ MODKEY|ShiftMask,	    XK_2,		tag,		  tags[1] }, \
	{ MODKEY|ShiftMask,	    XK_3,		tag,		  tags[2] }, \
	{ MODKEY|ShiftMask,	    XK_4,		tag,		  tags[3] }, \
	{ MODKEY|ShiftMask,	    XK_5,		tag,		  tags[4] }, \
    { MODKEY|ControlMask,	    XK_1,		toggletag,	  tags[0] }, \
	{ MODKEY|ControlMask,	    XK_2,		toggletag,	  tags[1] }, \
	{ MODKEY|ControlMask,	    XK_3,		toggletag,	  tags[2] }, \
	{ MODKEY|ControlMask,	    XK_4,		toggletag,	  tags[3] }, \
	{ MODKEY|ControlMask,	    XK_5,		toggletag,	  tags[4] }, \
	{ MODKEY,	    	    XK_F1,		view,		  tags[0] }, \
	{ MODKEY,   		    XK_F2,		view,		  tags[1] }, \
	{ MODKEY,		    XK_F3,		view,		  tags[2] }, \
	{ MODKEY,	    	    XK_F4,		view,		  tags[3] }, \
	{ MODKEY,	    	    XK_F5,		view,		  tags[4] }, \
	{ MODKEY,	    	    XK_F6,		view,		  tags[7] }, \
	{ MODKEY|ShiftMask,	    XK_F1,		toggleview,	  tags[0] }, \
	{ MODKEY|ShiftMask,	    XK_F2,		toggleview,	  tags[1] }, \
	{ MODKEY|ShiftMask,	    XK_F3,		toggleview,	  tags[2] }, \
	{ MODKEY|ShiftMask,	    XK_F4,		toggleview,	  tags[3] }, \
	{ MODKEY|ShiftMask,	    XK_F5,		toggleview,	  tags[4] }, \
	{ MODKEY,       	    XK_s,		focusview,	  tags[5] }, \
	{ MODKEY,       	    XK_o,		focusview,	  tags[6] }, \
};


