
include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o
COBJS	:= $(BOARD).o board_cmd.o
SOBJS 	:=

ifdef CONFIG_CMD_DRAM_TEST
COBJS	+= memctl_dram.o
endif

ifdef CONFIG_CMD_FLASH_TEST
COBJS	+= memctl_flash.o 
CFLAGS  += '-DFLASHBASE=0xB4000000'
endif

ifdef CONFIG_CMD_CONCURENT_TEST
COBJS	+= gdma_driver.o concur_pattern.o concur_test.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

all:	$(subdir) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS)) 

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
