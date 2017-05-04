include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o
COBJS := $(BOARD).o board_cmd.o bus_traffic_generator.o intl_test_util.o
SOBJS :=

ifdef CONFIG_CMD_DRAM_TEST
COBJS += memctl_dram.o
endif

ifdef CONFIG_CMD_FLASH_TEST
COBJS += memctl_flash.o
CFLAGS += '-DFLASHBASE=0x9D000000'
endif

.PHONY : $(subdir)

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(LIBOBJS) $(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

#CFLAGS_board/Realtek/rtl9601b := -mlong-calls

all:	$(subdir) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

$(subdir):
	$(MAKE) -C $@ all

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
