
DEPDIR = .deps

CFLAGS_EXTRA := -I. -DPATH_RADVD_CONF=\"/etc/radvd.conf\" \
    -DPATH_RADVD_LOG=\"/var/log/radvd.log\" -DLOG_FACILITY=LOG_DAEMON \
    -DPATH_RADVD_PID=\"/var/run/radvd.pid\" -DVERSION=\"1.5\" -DINET6=1 \
    -Wall -Wpointer-arith  -Wcast-qual -Wcast-align -Wstrict-prototypes \
    -Wmissing-prototypes -Wmissing-declarations

# Added by Mason Yu. for ipv6
LDFLAGS += -L$(ROOTDIR)/lib/libflex

.PHONY: clean romfs
all_objs = device-common.o interface.o process.o recv.o socket.o \
        device.o log.o radvd.o scanner.o timer.o gram.o privsep.o \
        radvdump.o send.o util.o

all: radvd radvdump

radvd: $(all_objs)
	@-rm -f radvd
	$(CC) $(LDFLAGS) -o radvd log.o socket.o recv.o util.o radvd.o timer.o send.o process.o interface.o device.o device-common.o privsep.o gram.o scanner.o -lfl -lc
	

radvdump: $(all_objs)
	@-rm -f radvdump
	$(CC) $(LDFLAGS) -o radvdump log.o socket.o recv.o util.o radvdump.o  -lc 

clean:
	-rm -f *.o radvd radvdump

romfs:
	$(ROMFSINST) -e CONFIG_USER_RADVD /bin/radvd
	$(ROMFSINST) -e CONFIG_USER_RADVD /bin/radvdump
	# Mason Yu. for ipv6
	$(ROMFSINST) -e CONFIG_USER_RADVD radvd.conf.example /etc/radvd.conf	

$(all_objs):%.o:%.c
	$(CC) $(CFLAGS_EXTRA) $(CFLAGS) -o $@ -c $< 

include ./$(DEPDIR)/device-bsd44.Po
include ./$(DEPDIR)/device-common.Po
include ./$(DEPDIR)/device-linux.Po
include ./$(DEPDIR)/device.Po
include ./$(DEPDIR)/gram.Po
include ./$(DEPDIR)/interface.Po
include ./$(DEPDIR)/log.Po
include ./$(DEPDIR)/privsep-bsd44.Po
include ./$(DEPDIR)/privsep-linux.Po
include ./$(DEPDIR)/privsep.Po
include ./$(DEPDIR)/process.Po
include ./$(DEPDIR)/radvd.Po
include ./$(DEPDIR)/radvdump.Po
include ./$(DEPDIR)/recv.Po
include ./$(DEPDIR)/scanner.Po
include ./$(DEPDIR)/send.Po
include ./$(DEPDIR)/socket.Po
include ./$(DEPDIR)/timer.Po
include ./$(DEPDIR)/util.Po
