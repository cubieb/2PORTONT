include ../.linux_config

EXES-1-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST) += wait_dsp 
OBJS-1-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST) += wait_dsp.o

EXES-2-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST) += voip_def_init
OBJS-2-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST) += voip_def_init.o

CFLAGS = -g -Wall -I../includes 
LDFLAGS = -L../voip_manager -lvoip_manager 

EXES-1 = $(EXES-1-y)
OBJS-1 = $(OBJS-1-y)

EXES-2 = $(EXES-2-y)
OBJS-2 = $(OBJS-2-y)

DEPENDENCY-1	= $(OBJS-1:.o=.d)
DEPENDENCY-2	= $(OBJS-2:.o=.d)

all: $(EXES-1) $(EXES-2)

$(EXES-1): $(OBJS-1)
	$(CC) -o $@ $(LDFLAGS) $^ 

$(EXES-2): $(OBJS-2)
	$(CC) -o $@ $(LDFLAGS) $^ 

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

romfs:
	[ ! -f wait_dsp ] || $(ROMFSINST) wait_dsp /bin/wait_dsp
	[ ! -f voip_def_init ] || $(ROMFSINST) voip_def_init /bin/voip_def_init

clean:
	rm -f $(LIBS) $(OBJS-1) $(OBJS-2) *.d

-include $(DEPENDENCY-1) $(DEPENDENCY-2)
