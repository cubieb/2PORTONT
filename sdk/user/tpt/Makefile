# $Header: /usr/local/dslrepos/uClinux-dist/user/tpt/Makefile,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $

CFLAGS+=-Wall -Wshadow

OFILES = tpt.o util.o build.o dump.o massage.o

tpt: $(OFILES)

romfs:
	$(ROMFSINST) /bin/tpt

$(OFILES): tpt.h

clean:
	rm -f $(OFILES) tpt
