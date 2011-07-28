# echinus wm - a window manager
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# © 2008 Alexander Polakov

include config.mk

PIXMAPS = close.xbm iconify.xbm max.xbm 
FILES = draw.c parse.c ewmh.c config.h
SRC = draw.c echinus.c ewmh.c parse.c
HEADERS = config.h echinus.h
OBJ = ${SRC:.c=.o}

all: options echinus ${HEADERS}

options:
	@echo echinus build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk ${HEADERS}

echinus: ${OBJ} ${FILES}
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
	@echo installing executable file to ${DESTDIR}${BINPREFIX}
	@mkdir -p ${DESTDIR}${BINPREFIX}
	@cp -f echinus ${DESTDIR}${BINPREFIX}
	@chmod 755 ${DESTDIR}${BINPREFIX}/echinus
	@echo installing configuration file and pixmaps to ${DESTDIR}${CONFPREFIX}/echinus
	@mkdir -p ${DESTDIR}${CONFPREFIX}/echinus
	@cp echinusrc ${DESTDIR}${CONFPREFIX}/echinus
	@cp ${PIXMAPS} ${DESTDIR}${CONFPREFIX}/echinus
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g;s|CONFDIR|${DESTDIR}${CONF}|g" < echinus.1 > ${DESTDIR}${MANPREFIX}/man1/echinus.1
	@echo installing README to ${DESTDIR}${DOCPREFIX}/echinus
	@mkdir -p ${DESTDIR}${DOCPREFIX}/echinus
	@sed "s|CONFDIR|${CONF}|" < README > ${DESTDIR}${DOCPREFIX}/echinus/README

uninstall:
	@echo removing executable file from ${DESTDIR}${BINPREFIX}/bin
	@rm -f ${DESTDIR}${BINPREFIX}/bin/echinus
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/echinus.1
	@echo removing configuration file and pixmaps from ${DESTDIR}${CONFPREFIX}
	@rm -rf ${DESTDIR}${CONFPREFIX}

.PHONY: all options clean dist install uninstall
