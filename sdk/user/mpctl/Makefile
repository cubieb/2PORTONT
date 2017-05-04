
EXEC = mpctl
OBJS = mpctl.o
IPATH = ../rtk_voip/includes/
LINUX_PATH = $(ROOTDIR)/linux-2.6.x/ 
#ln -nfs $(LINUX_PATH)/rtk_voip/include/voip_params.h voip_params.h
#ln -nfs $(ROOTDIR)/rtk_voip/voip_manager_1.6/includes/voip_manager.h voip_manager.h

all: $(EXEC)
CFLAGS +=  -I$(IPATH)

$(EXEC): $(OBJS)
ifndef CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) $(CFLGAS) -o $@ $(OBJS) $(LDLIBS)
else  #CONFIG_USER_BOA_SRC_BOA
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LIBMIB) $(LIBVOIP)
endif #CONFIG_USER_BOA_SRC_BOA

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

