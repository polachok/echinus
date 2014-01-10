
SUBDIRS = tests

AM_CPPFLAGS = -include autoconf.h -DSYSCONFPATH=\"$(pkgconfdir)\"
AM_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS) $(XRANDR_CFLAGS) $(XSYNC_CFLAGS) $(XINERAMA_CFLAGS)

bin_PROGRAMS = echinus

echinus_SOURCES = draw.c echinus.c ewmh.c parse.c config.h echinus.h
echinus_LDADD = $(XINERAMA_LIBS) $(XSYNC_LIBS) $(XRANDR_LIBS) $(XFT_LIBS) $(X11_LIBS)

echinus.man: echinus.1
	/bin/cp -f $< $@

README.echinus: README
	sed 's,CONFDIR,$(pkgconfdir),' $< >$@

nodist_man1_MANS = echinus.man

dist_pkgdata_DATA = close.xbm echinusrc iconify.xbm max.xbm
nodist_doc_DATA = README.echinus

EXTRA_DIST = LICENSE echinus.1 README TODO ChangeLog \
	     debian/changelog \
	     debian/compat \
	     debian/control \
	     debian/copyright \
	     debian/docs \
	     debian/menu \
	     debian/rules \
	     debian/watch

CLEANFILES = echinus.man README.echinus

