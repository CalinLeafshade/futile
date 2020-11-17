# futile - window manager

include config.mk

SRC = futile.c monitor.c client.c layout.c log.c
OBJ = ${SRC:.c=.o}

VPATH=src

all: options futile

options:
	@echo futile build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

futile: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f futile ${OBJ} futile-${VERSION}.tar.gz

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f futile ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/futile
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < futile.1 > ${DESTDIR}${MANPREFIX}/man1/futile.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/futile.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/futile\
		${DESTDIR}${MANPREFIX}/man1/futile.1

.PHONY: all options clean dist install uninstall
