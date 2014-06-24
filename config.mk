# echinus wm version
VERSION = 0.4.9

# Customize below to fit your system
INSTALL?=/usr/bin/install

# paths
PREFIX?= /usr/local
BINPREFIX?= ${PREFIX}/bin
MANPREFIX?= ${PREFIX}/share/man
CONFPREFIX?= ${PREFIX}/share/echinus
DOCPREFIX?= ${PREFIX}/share/doc
CONF?= ${CONFPREFIX}

# includes and libs
CFLAGS += -I. `pkg-config --cflags x11 xft`
LIBS += `pkg-config --libs x11 xft`
CPPFLAGS += -DVERSION=\"${VERSION}\" -DSYSCONFPATH=\"${CONF}\"

# debug flags
ifdef DEBUG
CFLAGS += -g3 -ggdb3 -std=c99 -pedantic -O0 ${INCS} -DDEBUG ${DEFS}
LDFLAGS += -g3 -ggdb3
# DEBUG: Show warnings (if any). Comment out to disable.
#CFLAGS += -Wall -Wpadded
# mostly useless warnings
#CFLAGS += -W -Wcast-qual -Wshadow -Wwrite-strings
#CFLAGS += -Werror        # Treat warnings as errors.
#CFLAGS += -save-temps    # Keep precompiler output (great for debugging).
endif

# XRandr (multihead support). Comment out to disable.
ifdef MULTIHEAD
CPPFLAGS += -DXRANDR=1
LIBS += $(shell pkg-config --libs xrandr)
CFLAGS += $(shell pkg-config --cflags xrandr)
endif

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}
#CFLAGS += -xtarget=ultra

# compiler and linker
#CC = cc
