# fwm - dynamic window manager
# © 2006-2007 Anselm R. Garbe, Sander van Dijk

include config.mk

SRC = fwm.c
TOOLS = lsw. setfocus.c
TOOLSOBJ = ${TOOLS:.c=.o}
OBJ = ${SRC:.c=.o}
HOM = `echo ${HOME}|sed 's.\/.\\\/.g'`

all: clean options fwm tools

options:
	@echo fwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "CONFIG   = ${HOME}/.fwmrc"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@
	@echo creating dot_fwmrc from dot_fwmrc_def
	@cat dot_fwmrc_def|sed "s/HOME/${HOM}/" > dot_fwmrc

fwm: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

lsw: 
	@echo CC -o $@
	@${CC} -o $@ lsw.c ${LDFLAGS}

setfocus:
	@echo CC -o $@
	@${CC} -o $@ setfocus.c ${LDFLAGS}

tools: lsw setfocus

clean:
	@echo cleaning
	@rm -f fwm setfocus lsw ${OBJ} dwm-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p fwm-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk \
		fwm.1 ${SRC} dwm-${VERSION}
	@tar -cf fwm-${VERSION}.tar dwm-${VERSION}
	@gzip fwm-${VERSION}.tar
	@rm -rf fwm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f fwm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/fwm

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/fwm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/fwm.1

.PHONY: all options clean dist install uninstall
