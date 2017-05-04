# $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $
#
# Makefile for MD2 wrapper
#
# Gene Kim
# Purdue University
# October 18, 1992
#

all:	md2wrapper.o md2.o

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -rf *.o .pure

md2.o: ../../include/byteorder.h
md2wrapper.o: ../../include/byteorder.h
