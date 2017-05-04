-include $(ROOTDIR)/config/.config 
-include $(ROOTDIR)/$(LINUXDIR)/.config

AIPC_EXEC = aipc_util
AIPC_OBJS = aipc_util.o

CFLAGS += -Wall 
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char/aipc/include

all: $(AIPC_EXEC)

$(AIPC_EXEC): $(AIPC_OBJS)
	$(CC) $(LDFLAGS) $(AIPC_OBJS) -o $@ 

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) -Wa,-a,-ad $< > $(@:.o=.lst)
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

romfs:
	$(ROMFSINST) $(AIPC_EXEC)                        /bin/$(AIPC_EXEC)
ifeq ($(CONFIG_RTL8686_IPC_DUAL_LINUX),y)
	$(ROMFSINST) aipc/rc_boot_dual_linux            /etc/rc_boot_dual_linux
#	$(ROMFSINST) -a "/etc/rc_boot_dual_linux"       /etc/rc
#	$(ROMFSINST) aipc/luna_slave.img                /image/luna_slave.img
endif
ifeq ($(CONFIG_USER_RTK_VOIP),y)
ifneq ($(CONFIG_RTL8686_IPC_DUAL_LINUX),y)
ifneq ($(CONFIG_RTK_VOIP_STANDALONE_ARCH),y)
	$(ROMFSINST) aipc/rc_boot_dsp                    /etc/rc_boot_dsp
	$(ROMFSINST) aipc/appimg.bin                     /image/dsp.img
endif
endif
	$(ROMFSINST) aipc/netcat-mips                    /bin/netcat
endif
ifeq ($(CONFIG_RTL8686_IPC_TEST_THREAD),y)
	$(ROMFSINST) aipc/run_all.sh                     /etc/run_all.sh
	$(ROMFSINST) aipc/run_mbox.sh                    /etc/run_mbox.sh
	$(ROMFSINST) aipc/run_ctrl.sh                    /etc/run_ctrl.sh
	$(ROMFSINST) aipc/mbox_2dsp.sh                   /etc/mbox_2dsp.sh
	$(ROMFSINST) aipc/mbox_2cpu.sh                   /etc/mbox_2cpu.sh
	$(ROMFSINST) aipc/ctrl_2dsp.sh                   /etc/ctrl_2dsp.sh
	$(ROMFSINST) aipc/ctrl_2cpu.sh                   /etc/ctrl_2cpu.sh
endif

clean:
	rm -f $(AIPC_EXEC) *.o *.d *.lst

.PHONY: all clean

