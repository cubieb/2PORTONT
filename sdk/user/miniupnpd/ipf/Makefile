# $Id: Makefile,v 1.1 2008/05/23 09:06:59 adsmt Exp $
CC=gcc
CFLAGS=-Wall -g -I.

all:	testipfrdr

clean:
	rm *.o testipfrdr

testipfrdr:	testipfrdr.o ipfrdr.o
	$(CC) -o $@ $^

ipfrdr.o:	ipfrdr.c

testipfrdr.o:	testipfrdr.c

