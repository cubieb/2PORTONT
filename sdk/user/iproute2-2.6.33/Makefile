
DIRS=lib ip tc 
#misc netem genl
THISDIR=$(shell pwd)
CFLAGS += -DRESOLVE_HOSTNAMES -I$(THISDIR)/include
LIBNETLINK=../lib/libnetlink.a ../lib/libutil.a

LDLIBS += -L$(THISDIR)/lib -lnetlink -lutil

ADDLIB=
#options for decnet
ADDLIB+=dnet_ntop.o dnet_pton.o
#options for ipx
ADDLIB+=ipx_ntop.o ipx_pton.o

export LIBNETLINK ADDLIB

all:
	for i in $(DIRS); do make -C $$i $@ || exit $?; done
	
clean:
	for i in $(DIRS); do make -C $$i clean; done
	
romfs:
	for i in $(DIRS); do make -C $$i romfs; done