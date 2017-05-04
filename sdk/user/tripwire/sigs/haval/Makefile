# $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $
#
# Makefile for crc32 wrapper
#
# Gene Kim
# Purdue University (now University of Arizona)
# July 10, 1994
#

all:	haval.o havalwrapper.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o .pure

haval.o: ../../include/byteorder.h
haval.o: haval.h
haval.o: havalapp.h
