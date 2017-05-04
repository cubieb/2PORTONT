#-include ../../config/.config

TARGET = tcp2dev
OBJS = main.o connect.o

#PREFIX := /share/rlx/rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-110915/bin/rsdk-linux-
#CC := $(PREFIX)gcc
#STRIP := $(PREFIX)strip
CFLAGS += -Wall -Os -mips16 
#CFLAGS += -Wall -O0

ALL_C = $(patsubst %.o,%.c,$(OBJS))
ALL_DEP = $(patsubst %.o,%.d,$(OBJS))

RC_LEVEL=/etc/init.d/rc30

all: $(TARGET) 

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ 

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

ifneq ($(MAKECMDGOALS),clean)
include $(ALL_DEP)
endif

$(ALL_DEP) : %.d : %.c
	$(CC) $(CFLAGS) -MM $< > $@

romfs:
	$(ROMFSINST) $(TARGET)   /bin/$(TARGET)

.PHONY: rcX
rcX:
ifeq ($(CONFIG_USER_TCP2DEV),y)
	$(ROMFSINST) rc_dsp_console                          /etc/scripts/rc_dsp_console
	$(ROMFSINST) -a "/etc/scripts/rc_dsp_console"         $(RC_LEVEL)
	$(ROMFSINST) -a "tcp2dev /dev/dsp_console:5432 &"    $(RC_LEVEL)
endif

clean :
	-rm -rf $(TARGET) $(OBJS)
	-make clean_dep

clean_dep :
	-rm -f $(ALL_DEP)

.PHONY: clean clean_dep
