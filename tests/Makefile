include config.mk

SRC = ewmhpanel.c util.c
OBJ = ${SRC:.c=.o}

all: options tests

options:
	@echo echinus tests build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

ewmhpanel: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

tests: ewmhpanel

clean:
	@echo cleaning
	@rm -f ewmhpanel
	@rm -f *.o

.PHONY: all options clean dist install uninstall
