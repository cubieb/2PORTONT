
#test
#for _TEST_ON_ONE_PC_ setting
#	server <-----> client 
#	 59512  -----> 3784 (periodic bfd)
#	 59513  -----> 3784 (bfd: poll response)
#	 13784 <-----  49512 (periodic bfd)
#	 13784 <-----  49513 (bfd: poll response)
#	 13785 x-----x 3785 (echo)
#CFLAGS += -D_TEST_ON_ONE_PC_
#CFLAGS += -D_HANDLE_CONSOLE_
#CFLAGS += -D_LOCALWANTEST_
CFLAGS += -D_CHECK_TTL_


BFD_IN=bfdsocket.h bfdlib.h bfdtimeout.h libsha1.h
BFD_OB=bfdsocket.o bfdlib.o bfdtimeout.o libsha1.o bfdecho.o bfdcore.o bfdconsole.o

ifdef CONFIG_USER_WT_146
BFD_BIN=bfdctl bfdmain
LDFLAGS+= $(LIBMIB)
else
BFD_BIN=bfdserver bfdclient bfdctl bfdmain
endif

ifdef CONFIG_LIB_LIBMD5_FORCE
LDFLAGS+= $(LIBMD5)
else
BFD_IN += libmd5.h
BFD_OB += libmd5.o
endif

RC_LEVEL=/etc/init.d/rc4

all: $(BFD_BIN)

bfdconsole.o: bfdconsole.c bfdlib.h

libsha1.o: libsha1.c libsha1.h

libmd5.o: libmd5.c libmd5.h

bfdtimeout.o: bfdtimeout.c bfdtimeout.h

bfdcore.o: bfdcore.c $(BFD_IN)

bfdlib.o: bfdlib.c bfdlib.h

bfdsocket.o: bfdsocket.c bfdsocket.h

bfdecho.o: bfdecho.c bfdlib.h

bfdserver.o: bfdserver.c $(BFD_IN)

bfdclient.o: bfdclient.c $(BFD_IN)

bfdserver: bfdserver.o $(BFD_OB)

bfdclient: bfdclient.o $(BFD_OB)

bfdctl.o: bfdctl.c

bfdctl: bfdctl.o

bfdutil.o : bfdutil.c bfdutil.h

bfdmain.o : bfdmain.c $(BFD_IN) bfdutil.h

bfdmain: bfdmain.o bfdutil.o $(BFD_OB)

clean:
	rm -f *.o
	rm -f bfdserver bfdclient bfdmain bfdctl

romfs:
	$(ROMFSINST) /bin/bfdmain
	$(ROMFSINST) /bin/bfdctl

.PHONY: rcX
rcX:
	$(ROMFSINST) -a "mkdir -p /var/bfd/"  $(RC_LEVEL)