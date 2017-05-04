#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS	= luna_eth.o re8670poll.o swCore.o nor_spif_proj_dep.o
SOBJS	= lowlevel_init.o cp0_util.o rlx5281_ctr.o

OTTO_PRJ_DIR           := $(TOPDIR)/../project/9601b_demo
OTTO_TPL_DIR           := $(TOPDIR)/../src/template/9601b
OTTO_9601B_PC_H        := $(TOPDIR)/include/asm/arch/9601b_pad_ctrl.h
OTTO_9601B_NORSF_DEP_C := $(TOPDIR)/arch/otto/cpu/rlx5281/rtl9601b/nor_spif_proj_dep.c
OTTO_PLAT_DIR          := $(TOPDIR)/../src/platform/9601b/
OTTO_CG_FREQ_H         := $(TOPDIR)/arch/otto/include/asm/otto_cg_dev_freq.h

.PHONY : $(subdir)

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(LIBOBJS) $(SOBJS) $(COBJS))

all: .otto_soc_sync $(subdir) $(obj).depend $(LIB)
	@rm -f .otto_soc_sync

.otto_soc_sync:
	@$(call get_otto_file, $(OTTO_9601B_PC_H), $(OTTO_TPL_DIR)/9601b_pad_ctrl.h)
	@$(call get_otto_file, $(OTTO_9601B_NORSF_DEP_C), $(OTTO_PRJ_DIR)/ab_nor_spif_proj_dep.c)
	@$(call get_otto_file, $(OTTO_CG_FREQ_H), $(OTTO_PLAT_DIR)/cg/cg_dev_freq.h)
	@date > $@

nor_spif_proj_dep.c: .otto_soc_sync

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
