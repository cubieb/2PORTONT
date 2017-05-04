# $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:12 kaohj Exp $
#
# Makefile for SHA wrapper
#
# Gene Kim
# Purdue University
# August 10, 1993
#

all:	sha.o shawrapper.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -rf *.o .pure

sha.o: ../../include/byteorder.h
shawrapper.o: ../../include/byteorder.h
