# $Id: Makefile,v 1.1 2008/05/23 09:06:59 adsmt Exp $
# made for GNU Make
CFLAGS = -Wall -g
EXECUTABLES = testobsdrdr

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr:	testobsdrdr.o obsdrdr.o

obsdrdr.o:	obsdrdr.c obsdrdr.h

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

