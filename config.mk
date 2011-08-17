# echinus wm version
VERSION = 0.4.6

# Customize below to fit your system

# paths
PREFIX?= /usr/local
BINPREFIX?= ${PREFIX}/bin
MANPREFIX?= ${PREFIX}/share/man
CONFPREFIX?= ${PREFIX}/share/examples
DOCPREFIX?= ${PREFIX}/share/doc
CONF?= ${CONFPREFIX}

X11INC?= /usr/X11R6/include
X11LIB?= /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} `pkg-config --cflags xft`
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 `pkg-config --libs xft`

DEFS = -DVERSION=\"${VERSION}\" -DSYSCONFPATH=\"${CONF}\"

# flags
CFLAGS = -Os ${INCS} ${DEFS}
LDFLAGS = -s ${LIBS}
# debug flags
CFLAGS = -g3 -ggdb3 -std=c99 -pedantic -O0 ${INCS} -DDEBUG ${DEFS}
LDFLAGS = -g3 -ggdb3 ${LIBS}

# DEBUG: Show warnings (if any). Comment out to disable.
#CFLAGS += -Wall -Wpadded
# mostly useless warnings
#CFLAGS += -W -Wcast-qual -Wshadow -Wwrite-strings
#CFLAGS += -Werror        # Treat warnings as errors.
#CFLAGS += -save-temps    # Keep precompiler output (great for debugging).

# XRandr (multihead support). Comment out to disable.
CFLAGS += -DXRANDR=1
LIBS += -lXrandr

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}
#CFLAGS += -xtarget=ultra

# compiler and linker
#CC = cc
