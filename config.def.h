/* See LICENSE file for copyright and license details. */

/* appearance */
#define BARPOS			BarTop /* BarTop, BarOff */
#define BORDERPX		"3"
#define NOTITLES 0
#define NF_OPACITY "0.9"
#define FONT            "-*-snap-*-*-*-*-11-*-*-*-*-*-iso10646-*"
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
#define TERMINAL "xterm"
/* tagging */
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
	{ "i",		ifloating },
	{ "f",		floating },
	{ "m",		monocle }, /* first entry is default */
	{ "t",		tile }, 
};
#define NMASTER 1
