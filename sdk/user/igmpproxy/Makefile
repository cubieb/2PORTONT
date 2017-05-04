
EXEC = igmpproxy
OBJS = mroute-api.o lib.o ifvc.o syslog.o timeout.o main.o

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#CONFIG_IP_IGMPV3=y
#endif
ifdef CONFIG_BOA_WEB_E8B_CH
CONFIG_IP_IGMPV3=n
endif
#ifeq ($(CONFIG_IP_IGMPV3),y)
CFLAGS += -D_SUPPORT_IGMPV3_
OBJS   += igmpv3.o
DEPH   += igmpproxy.h
#endif

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o *.bak

