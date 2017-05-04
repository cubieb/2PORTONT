#
# Makefile for FT daemon for the Linux OS
#

RTL_FT_IFLAGS = -I../wsc/portingssl/crypto/include
RTL_FT_LIBS = -L../wsc/portingssl -lcrypto
RTL_FT_LIBS += -L$(ROOTDIR)/lib/librtk $(LIBMD5) -DUSE_RTK_LIB

CFLAGS += $(RTL_FT_IFLAGS) $(RTL_FT_LIBS) $(RTL_FT_CFLAG)


all: ftd

ft_main.o : ft_main.c ft_main.h

ftd: ft_main.o
	$(CC) -o ftd ft_main.o $(CFLAGS)


clean:
	rm -f *.o ftd

romfs:
	$(ROMFSINST) ftd /bin/ftd
	$(ROMFSINST) ft.conf /etc/ft.conf

