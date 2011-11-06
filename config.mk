# echinus wm version
VERSION = 0.4.9

# Customize below to fit your system

# paths
PREFIX?= /usr/local
BINPREFIX?= ${PREFIX}/bin
MANPREFIX?= ${PREFIX}/share/man
CONFPREFIX?= ${PREFIX}/share/examples
DOCPREFIX?= ${PREFIX}/share/doc
CONF?= ${CONFPREFIX}

# includes and libs
INCS = -I. `pkg-config --cflags x11 xft`
LIBS = `pkg-config --libs x11 xft`

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
