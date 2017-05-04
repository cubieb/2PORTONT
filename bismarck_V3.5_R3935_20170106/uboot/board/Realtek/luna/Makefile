
include $(TOPDIR)/config.mk

subdir = rtk_soc

LIB	= $(obj)lib$(BOARD).o
LIBOBJS = rtk_soc/rtk_soc.o

#COBJS	:= $(BOARD).o flash.o \
	board_mem_diag.o board_cmd.o

#COBJS	:= $(BOARD).o flash_spi.o \
	board_mem_diag.o board_cmd.o

COBJS  := $(BOARD).o \
        board_mem_diag.o board_cmd.o

SOBJS   = measur_cpu_clk.o

.PHONY : $(subdir)

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(LIBOBJS) $(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

CFLAGS_board/Realtek/luna := -mlong-calls

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
