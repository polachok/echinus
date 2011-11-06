# echinus wm - a window manager
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# © 2008 Alexander Polakov

include config.mk

PIXMAPS = close.xbm iconify.xbm max.xbm 
SRC = draw.c echinus.c ewmh.c parse.c
HEADERS = config.h echinus.h
OBJ = ${SRC:.c=.o}

all: options echinus ${HEADERS}

options:
	@echo echinus build options:
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "LIBS     = ${LIBS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	${CC} ${CPPFLAGS} -c ${CFLAGS} $<

${OBJ}: config.mk ${HEADERS}

echinus: ${OBJ} ${SRC} ${HEADERS}
	@echo CC -o $@
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ} ${LIBS}

clean:
	@echo cleaning
	rm -f echinus ${OBJ} echinus-${VERSION}.tar.gz *~

dist: clean
	@echo creating dist tarball
	mkdir -p echinus-${VERSION}
	cp -R LICENSE Makefile README config.mk \
		echinus.1 echinusrc ${SRC} ${HEADERS} ${PIXMAPS} echinus-${VERSION}
	tar -cf echinus-${VERSION}.tar echinus-${VERSION}
	gzip echinus-${VERSION}.tar
	rm -rf echinus-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${BINPREFIX}
	$(INSTALL) -D -m 755 echinus ${DESTDIR}${BINPREFIX}/echinus
	@echo installing configuration file and pixmaps to ${DESTDIR}${CONFPREFIX}
	$(INSTALL) -D -m 644 echinusrc ${DESTDIR}${CONFPREFIX}/echinusrc
	for file in $(PIXMAPS); do \
		$(INSTALL) -m 644 $${file} ${DESTDIR}${CONFPREFIX}/$${file} ; \
	done ;
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	sed -i -e "s/VERSION/${VERSION}/g;s|CONFDIR|${DESTDIR}${CONF}|g" echinus.1
	$(INSTALL) -D -m 644 echinus.1 ${DESTDIR}${MANPREFIX}/man1/echinus.1
	@echo installing README to ${DESTDIR}${DOCPREFIX}/echinus-${VERSION}
	sed -i -e "s|CONFDIR|${CONF}|" README
	$(INSTALL) -D -m 644 README ${DESTDIR}${DOCPREFIX}/echinus-${VERSION}/README

uninstall:
	@echo removing executable file from ${DESTDIR}${BINPREFIX}/bin
	rm -f ${DESTDIR}${BINPREFIX}/bin/echinus
	echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	rm -f ${DESTDIR}${MANPREFIX}/man1/echinus.1
	echo removing configuration file and pixmaps from ${DESTDIR}${CONFPREFIX}
	rm -rf ${DESTDIR}${CONFPREFIX}

.PHONY: all options clean dist install uninstall
