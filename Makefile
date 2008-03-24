# echinus wm - a window manager
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# © 2008 Alexander Polakov

include config.mk

HEADERS = ewmh.h parse.h config.h
PIXMAPS = close.xbm iconify.xbm max.xbm 
SRC = echinus.c
OBJ = ${SRC:.c=.o}
HOM = `echo ${HOME}|sed 's.\/.\\\/.g'`
CONF = ${HOME}/.echinus/

all: options echinus ${HEADERS}

options:
	@echo echinus build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "CONFIG   = ${CONF}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

echinus: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f echinus ${OBJ} echinus-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p echinus-${VERSION}
	@cp -R LICENSE Makefile README config.mk \
		echinus.1 echinusrc ${SRC} ${HEADERS} ${PIXMAPS} echinus-${VERSION}
	@tar -cf echinus-${VERSION}.tar echinus-${VERSION}
	@gzip echinus-${VERSION}.tar
	@rm -rf echinus-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f echinus ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/echinus

installconfig:
	@echo installing configuration file and pixmaps to ${CONF}
	@mkdir ${CONF}
	@cp echinusrc ${CONF}
	@cp ${PIXMAPS} ${CONF}

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/echinus
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/echinus.1

.PHONY: all options clean dist install uninstall
