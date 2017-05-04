# $Id: Makefile,v 1.1 2008/05/23 09:06:59 adsmt Exp $
CFLAGS=-Wall -g

all:	iptcrdr.o testiptcrdr

clean:
	$(RM) *.o testiptcrdr

testiptcrdr:	testiptcrdr.o iptcrdr.o -liptc

iptcrdr.o:	iptcrdr.c iptcrdr.h

