# $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $
#
# Makefile for MD4 wrapper
#
# Gene Kim
# Purdue University
# October 18, 1992
#

all:	md4.o md4wrapper.o 

.c.o:	
	$(CC) $(CFLAGS) -c $<
	
clean:
	-rm -rf md4.o md4wrapper.o  .pure

md4.o: ../../include/byteorder.h
md4wrapper.o: ../../include/byteorder.h
