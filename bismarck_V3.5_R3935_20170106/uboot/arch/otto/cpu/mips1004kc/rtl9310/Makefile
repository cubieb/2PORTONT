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

COBJS	=
SOBJS =

OTTO_PRJ_DIR          := $(TOPDIR)/../project/9300_nor_fpga
OTTO_9300_NORSF_DEP_C := $(TOPDIR)/arch/otto/lib/nor_spif_proj_dep.c
OTTO_PLAT_DIR         := $(TOPDIR)/../src/platform/9300/
OTTO_CG_FREQ_H        := $(TOPDIR)/arch/otto/include/asm/otto_cg_dev_freq.h

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(LIBOBJS) $(SOBJS) $(COBJS))

all: otto_soc_sync $(obj).depend $(LIB)

otto_soc_sync:
	@$(call get_otto_file, $(OTTO_9300_NORSF_DEP_C), $(OTTO_PRJ_DIR)/ab_nor_spif_proj_dep.c)
	@$(call get_otto_file, $(OTTO_CG_FREQ_H), $(OTTO_PLAT_DIR)/cg/cg_dev_freq.h)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
