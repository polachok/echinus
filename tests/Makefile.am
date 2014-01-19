
AM_CPPFLAGS = -include ../autoconf.h
AM_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS)

noinst_PROGRAMS = ewmhpanel

ewmhpanel_SOURCES = ewmhpanel.c util.c util.h
ewmhpanel_LDADD = $(X11_LIBS) $(XFT_LIBS)
