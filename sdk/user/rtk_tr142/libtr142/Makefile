LIB = libtr142

CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API

LDFLAGS += -L$(APOLLODIR)/object/src/app/lib \
		-L$(ROOTDIR)/lib/librtk \
		$(LIBMIB)

libtr142.o: libtr142.c
	$(CC) -Os -c -o $@ $(CFLAGS) -fPIC $?

all: libtr142.o
	$(CC) -shared -o $(LIB).so -L$(ROOTDIR)/lib/librtk -L$(APOLLODIR)/object/src/app/lib $(LIBMIB) $?

clean:
	rm -rf $(LIB).so
	rm -rf *.o

romfs:
	$(ROMFSINST) /lib/features/libtr142.so
