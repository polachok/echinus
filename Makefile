# echinus wm - a window manager
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# © 2008 Alexander Polakov

include config.mk

PIXMAPS = close.xbm iconify.xbm max.xbm 
FILES = draw.c parse.c ewmh.c config.h
SRC = echinus.c
OBJ = ${SRC:.c=.o}
CONF = /share/examples/echinus

all: options echinus ${HEADERS}

options:
	@echo echinus build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

echinus: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f echinus ${OBJ} echinus-${VERSION}.tar.gz *~

dist: clean
	@echo creating dist tarball
	@mkdir -p echinus-${VERSION}
	@cp -R LICENSE Makefile README config.mk \
		echinus.1 echinusrc ${SRC} ${FILES} ${PIXMAPS} echinus-${VERSION}
	@tar -cf echinus-${VERSION}.tar echinus-${VERSION}
	@gzip echinus-${VERSION}.tar
	@rm -rf echinus-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f echinus ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/echinus
	@echo installing configuration file and pixmaps to ${DESTDIR}${PREFIX}${CONF}
	@mkdir -p ${DESTDIR}${PREFIX}${CONF}
	@cp echinusrc ${DESTDIR}${PREFIX}${CONF}
	@cp ${PIXMAPS} ${DESTDIR}${PREFIX}${CONF}

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/echinus
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/echinus.1
	@echo removing configuration file and pixmaps from ${DESTDIR}${PREFIX}${CONF}
	@rm -rf ${DESTDIR}${PREFIX}${CONF}

.PHONY: all options clean dist install uninstall
