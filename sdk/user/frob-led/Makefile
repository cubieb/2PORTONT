# $Id: Makefile,v 1.1.1.1 2003/08/18 05:41:36 kaohj Exp $

CFLAGS	+=  -MD

PROG	:= frob-led
HEAD	:= keywords.h
SCRIPT	:= keywords.pl

LEDMAN	:= ${ROOTDIR}/${LINUXDIR}/include/linux/ledman.h

all: ${HEAD} ${PROG}

clean:
	rm -f ${PROG} *.o *.d ${HEAD}

${PROG}: $(patsubst %.c, %.o, $(wildcard *.c))
	${CC} ${LDFLAGS} -o $@ $^ ${LDLIBS}

-include *.d

${HEAD}: ${LEDMAN} ${SCRIPT}
	perl ${SCRIPT} <${LEDMAN} >$@

romfs:
	$(ROMFSINST) /bin/frob-led
