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

LIB	= $(obj)lib$(ARCH).o
FOO 	= $(OTTO_UBOOT_SNAF_DRV_NAME)
SOBJS-y	+=
COBJS-y	+= board.o bootm.o ashldi3.o ashrdi3.o lshrdi3.o
COBJS-y += otto_uart.o otto_time.o otto_sym.o otto_cg.o

COBJS-$(CONFIG_CMD_SF) += otto_nor_spi_flash.o

COBJS-$(CONFIG_SPI_NOR_FLASH) += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_core.o

ifeq ($(CONFIG_SPI_NOR_FLASH), y)
	ifeq ($(CONFIG_SPI_NOR_FLASH_GEN), 3)
		COBJS-$(CONFIG_SNOF_MICRON)    += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_micron_rev.o
#		COBJS-$(CONFIG_SNOF_MXIC)      += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_mxic_4b_cmd.o
#		COBJS-$(CONFIG_SNOF_MXIC)      += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_mxic_3b_mode.o
		COBJS-$(CONFIG_SNOF_MXIC)      += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_mxic.o
		COBJS-$(CONFIG_SNOF_WINBOND)   += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_winbond.o
		COBJS-$(CONFIG_SNOF_GD)        += $(OTTO_UBOOT_SNOF_DRV_NAME)/nor_spif_gd.o
	endif
endif

COBJS-$(CONFIG_OTTO_FLASH_LAYOUT) += otto_flash_layout.o

ifeq ($(CONFIG_SPI_NAND_FLASH),y)
	COBJS-y += otto_spi_nand_flash.o
	COBJS-y += otto_spi_nand_bb_util.o
	OTTO_SPI_NAND_GEN := $(shell echo $(CONFIG_SPI_NAND_FLASH_GEN))
	#ifneq ($(OTTO_SPI_NAND_GEN),1)
	ifneq ($(OTTO_SPI_NAND_GEN),0)
	#ifneq ($(CONFIG_SPI_NAND_FLASH_GEN),1)
		ifeq ($(CONFIG_SPI_NAND_FLASH_INIT_FIRST),y)
			COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_common.o
			COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_ctrl.o
			COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_util.o
			
		endif
		COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_mxic.o
		COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_gd.o
		COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_zentel.o
		COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_winbond.o
		#COBJS-y += $(OTTO_UBOOT_SNAF_DRV_NAME)/spi_nand_toshiba.o
	endif
	CPPFLAGS_arch/otto/lib/otto_spi_nand_flash.o := -DOTTO_SPI_NAND_GEN=$(OTTO_SPI_NAND_GEN)
	#CPPFLAGS_arch/otto/lib/otto_spi_nand_flash.o := -DOTTO_SPI_NAND_GEN=$(subst spi_nand_gen,,$(shell echo $(CONFIG_SPI_NAND_FLASH_GEN)))
endif

ifeq ($(CONFIG_CMD_ONFI),y)
COBJS-y += otto_onfi_flash.o
COBJS-y += otto_onfi_bb_util.o
ifneq ($(CONFIG_ONFI_GEN), "onfi_gen1")
#COBJS-y += 
endif
CPPFLAGS_arch/otto/lib/otto_onfi_flash.o := -DOTTO_ONFI_GEN=$(subst onfi_gen,,$(shell echo $(CONFIG_ONFI_GEN)))
endif

ifeq ($(CONFIG_ECC_ENGINE),y)
	ifeq ($(CONFIG_ECC_ENGINE_INIT),y)
		COBJS-y += $(OTTO_UBOOT_ECC_DRV_NAME)/ecc_ctrl.o
	endif
endif

CPPFLAGS_arch/otto/lib/otto_sym.o := -DSECTION_SYMACCESS=

SRCS := $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS := $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

	@echo "########"
	@echo $(FOO)
	@echo $(CONFIG_SPI_NAND_FLASH)
	@echo $(OTTO_SPI_NAND_GEN)
	@echo $(CONFIG_SPI_NAND_FLASH_INIT_FIRST)
	@echo $(CONFIG_SPI_NAND_FLASH_GEN)
	@echo $(shell echo $(CONFIG_SPI_NAND_FLASH_GEN))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
