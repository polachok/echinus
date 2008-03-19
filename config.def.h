/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
/* appearance */
#define BARPOS			BarTop /* BarTop, BarOff */

/* border width */
#define BORDERPX		"3"

#define NOTITLES 0
#define NF_OPACITY "0.9"
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
#define NORMBUTTONCOLOR		"#cccccc"
#define NORMBGCOLOR		"#cccccc"
#define NORMFGCOLOR		"#000000"
#define SELBORDERCOLOR		"#0066ff"
#define SELBUTTONCOLOR		"#0066ff"
#define SELBGCOLOR		"#0066ff"
#define SELFGCOLOR		"#ffffff"

#define BLEFTPIXMAP "min.xbm"
#define BRIGHTPIXMAP "min.xbm"
#define BCENTERPIXMAP "min.xbm"

#define MINWIDTH 12
#define MINHEIGHT 12


#define BARHEIGHT "12"
#define TITLEHEIGHT "12"

#define NMASTER 1
#define TERMINAL "urxvt"
/* tagging */
const char *tags[] = { "main", "web", "irc", "mail", "dev", "gfx", "misc" };
Bool seltags[LENGTH(tags)] = {[0] = True};
Rule rules[] = { \
    /* class:instance:title regex	tags regex	floating titlebar */ \
    { "Firefox.*",          "web",      False, True }, \
    { "WeeChat.*",          "irc",      False, True }, \
    { "Firefox-bin.*",      "web",      False, True }, \
    { "Flock-bin.*",        "web",      False, True }, \
    { "MPlayer.*",          NULL,       True, False}, \
    { "gl2.*",              NULL,       True, False }, \
    { "QEMU.*",             NULL,       True, True  }, \
    { "Sylpheed.*",         "mail",     False, True }, \
    { "mutt.*",             "mail",     False, True }, \
    { "Gimp.*",             "gfx",      True, True}, \
    { "feh.*",              NULL,       True, True}, \
    { "htop.*",             "misc",     True, True}, \
    { "ncmpc.*",            "misc",     True, True}, \
    { ".*ager.*",            ".*",      True, False}, \
};

/* layout(s) */
#define MWFACT			0.6	/* master width factor [0.1 .. 0.9] */
#define RESIZEHINTS		False	/* False - respect size hints in tiled resizals */
#define SNAP			5	/* snap pixel */
Layout layouts[] = {
	/* symbol		function */
	{ "%",		ifloating },
	{ "~",		floating },
	{ "=",		monocle }, /* first entry is default */
	{ "#",		tile }, 
};
#define NMASTER 1
/* key definitions */
#define MODKEY			Mod1Mask
#define KEYS \
Key keys[] = { \
	/* modifier			key		function	arguments */ \
	/* { MODKEY,		    XK_t,	spawn,		            "exec urxvt" }, \ */ \
	{ MODKEY,		    XK_t,	spawn,		            "exec urxvt -e screen -U" }, \
	{ MODKEY,		    XK_n,	spawn,	                    "exec np" }, \
	{ MODKEY,		    XK_b,	togglebar,	            NULL }, \
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
	{ MODKEY,		    XK_F12,	setlayout,	"." }, \
        { MODKEY,		    XK_j,		focusnext,	    NULL  }, \
	{ MODKEY,		    XK_k,		focusprev,	    NULL }, \
	{ MODKEY,		    XK_Return,   	zoom,		    NULL }, \
        { MODKEY,                   XK_minus,           setmwfact,         "-0.05" },\
        { MODKEY,                   XK_equal,           setmwfact,         "+0.05" }, \
	{ MODKEY,		    XK_d,		incnmaster,	"-1" }, \
	{ MODKEY,		    XK_i,		incnmaster,	"1" }, \
	{ MODKEY|ShiftMask,	    XK_c,		killclient,	 NULL  }, \
        { MODKEY,                   XK_space,           togglefloating,  NULL, }, \
        { MODKEY|ShiftMask,	    XK_q,		quit,		 "fwm"  }, \
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
	{ MODKEY,       	    XK_s,		focusview,	  tags[6] }, \
};


