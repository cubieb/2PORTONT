
#CC = mips-uclibc-gcc
#CFLAGS = -O2  -fomit-frame-pointer   -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -fpic -fPIC -embedded-relocs -mno-abicalls -I/home/casey/uClinux-dist/lib/uClibc-0.9.15/include -I/home/casey/uClinux-dist/lib/libm -I/home/casey/uClinux-dist/lib/libcrypt_old -I/home/casey/uClinux-dist -fno-builtin -nostartfiles -I/home/casey/uClinux-dist/linux-2.4.x/include

EXEC = spppd
EXEC2 = spppctl
OBJS = timeout.o arpping.o socket.o md5.o pppoe.o if_spppsubr.o main.o sys-linux.o \
		text2atm.o text2qos.o ans.o misc.o pppoatm.o chap.o chap_ms.o md4.o extra_crypto.o sha1dgst.o
	
#OBJS2 = text2atm.o text2qos.o ans.o misc.o chap.o chap_ms.o md4.o extra_crypto.o sha1dgst.o spppctl.o
OBJS2 = text2atm.o text2qos.o ans.o misc.o spppctl.o arpping.o socket.o

ifeq ($(CONFIG_PPP_MPPE_MPPC),y)
CFLAGS += -DMPPE
OBJS += mppe.o
endif

EXEC3 = chat
OBJS3 = chat.o
CHAT_EXEC =
ifeq ($(CONFIG_USER_PPPOMODEM),y)
OBJS +=  pppomodem.o
CFLAGS += -DCONFIG_USER_PPPOMODEM
CHAT_EXEC = $(EXEC3)
endif
ifeq ($(CONFIG_USER_PPTP_CLIENT_PPTP),y)
OBJS += pptp_ctrl.o pptp_callmgr.o pptp.o vector.o 
endif
ifeq ($(CONFIG_USER_L2TPD_L2TPD),y)
SUBDIRS = usl
OBJS += l2tp_avp.o l2tp_common.o l2tp_network.o \
		l2tp_packet.o l2tp_peer.o l2tp_session.o l2tp_transport.o l2tp_tunnel.o \
		l2tp_main.o ppp_unix.o
LDFLAGS += -L./usl -lusl
CFLAGS += -I./usl
endif
ifeq ($(CONFIG_USER_PPTPD_PPTPD),y)
OBJS += pptpmanager.o ctrlpacket.o pptpgre.o pptpd.o
endif
LDFLAGS += -lcrypt


all: subdirs $(EXEC) $(EXEC2) $(CHAT_EXEC)

subdirs:
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) ); done

$(OBJS): $(ROOTDIR)/include/linux/config.h

#$(EXEC): $(OBJS)
#	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@))) -L../boa/src-project/LINUX $(LIBMIB) $(LIBMD5)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@))) $(LDFLAGS) -L../boa/src-project/LINUX $(LIBMIB) $(LIBMD5) $(LIBVOIP)

$(EXEC2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LDLIBS$(LDLIBS-$(@)))

$(EXEC3): $(OBJS3)
	$(CC) $(LDFLAGS) -o $@ $(OBJS3) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/$(EXEC2)
ifeq ($(CONFIG_USER_PPPOMODEM),y)
	$(ROMFSINST) /bin/$(CHAT_EXEC)
endif

clean:
	-rm -f $(EXEC) $(EXEC2) $(CHAT_EXEC) *.elf *.o


