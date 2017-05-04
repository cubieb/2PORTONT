CFLAGS += -I../../include -I.. -Itacc/include -Wall

.PHONY: tacacs
tacacs: tacacs.o
	make -C tacc/lib libtac.a


.PHONY: pppoe
pppoe: libpppoe.a

libpppoe.a:
	make -f Makefile.linux -C pppoe libpppoe.a

.PHONY: pppoa
pppoa: libpppoa.a

libpppoa.a:
	make -f Makefile.linux -C pppoa libpppoa.a

clean:
	make -C tacc/lib clean
	make -f Makefile.linux -C pppoe clean
	make -f Makefile.linux -C pppoa clean
	-rm -f *.o
