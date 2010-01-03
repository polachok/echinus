# echinus wm version
VERSION = 0.3.9

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
CONF = /share/examples/echinus

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} `pkg-config --cflags xft`
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 `pkg-config --libs xft`

# flags
CFLAGS = -Os ${INCS} -DVERSION=\"${VERSION}\" -DSYSCONFPATH=\"${PREFIX}/${CONF}\"
LDFLAGS = -s ${LIBS}
CFLAGS = -g3 -ggdb3 -std=c99 -pedantic -O0 ${INCS} -DVERSION=\"${VERSION}\" -DSYSCONFPATH=\"${PREFIX}/${CONF}\"
LDFLAGS = -g3 -ggdb3 ${LIBS}

# DEBUG: Show warnings (if any). Comment out to disable.
CFLAGS += -Wall -W -Wcast-qual -Wshadow -Wwrite-strings -Wconversion
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
