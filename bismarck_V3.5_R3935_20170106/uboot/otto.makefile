OTTO_INC_DIR := $(OTTO_RELEASE_DIR)/../src/include
OTTO_IP_DIR := $(OTTO_RELEASE_DIR)/../src/soc

OTTO_XENV_H := include/asm/arch/cross_env.h
OTTO_RMU_H  := include/asm/arch/reg_map_util.h

ifeq ($(CONFIG_SPI_NOR_FLASH), y)
	OTTO_UBOOT_SNOF_DRV_NAME := nor_spi_gen$(CONFIG_SPI_NOR_FLASH_GEN)
	OTTO_UBOOT_SNOF_DRV_PATH := arch/otto/lib/$(OTTO_UBOOT_SNOF_DRV_NAME)
	export OTTO_UBOOT_SNOF_DRV_NAME OTTO_UBOOT_SNOF_DRV_PATH
endif

ifeq ($(CONFIG_SPI_NAND_FLASH), y)
	OTTO_UBOOT_SNAF_DRV_NAME := spi_nand_gen$(shell echo $(CONFIG_SPI_NAND_FLASH_GEN))
	OTTO_UBOOT_SNAF_DRV_PATH := arch/otto/lib/$(OTTO_UBOOT_SNAF_DRV_NAME)
	export OTTO_UBOOT_SNAF_DRV_NAME OTTO_UBOOT_SNAF_DRV_PATH
endif

ifeq ($(CONFIG_ECC_ENGINE), y)
	OTTO_UBOOT_ECC_DRV_NAME := ecc_gen$(shell echo $(CONFIG_ECC_ENGINE_GEN))
	OTTO_UBOOT_ECC_DRV_PATH := arch/otto/lib/$(OTTO_UBOOT_ECC_DRV_NAME)
	export OTTO_UBOOT_ECC_DRV_NAME OTTO_UBOOT_ECC_DRV_PATH
endif

.PHONY: $(OTTO_SOC_H) .otto.v

$(OTTO_SOC_H): otto_arch_sync
	@if [ ! -e $(OTTO_RELEASE_DIR)/conf.h ]; then \
		if [ ! -e $@ ]; then \
			echo "EE: Missing $@" ; \
			exit 1; \
		fi \
	else \
		cmp $@ $(OTTO_RELEASE_DIR)/conf.h > /dev/null 2>&1 ; \
		if [ "$$?" -ne 0 ]; then \
			echo -n "[MISC] Generating $@... " ; \
			sed -e '/^#define USE_ON_FLASH_SECTION$$/d' -e '/^#define .\+SECTION.\+/d' $(OTTO_RELEASE_DIR)/conf.h > $@ ; \
			echo "done" ; \
		fi \
	fi

otto_arch_sync:
	@$(call get_otto_file, $(OTTO_XENV_H), $(OTTO_INC_DIR)/cross_env.h)
	@$(call get_otto_file, $(OTTO_RMU_H),  $(OTTO_INC_DIR)/reg_map_util.h)
ifeq ($(CONFIG_SPI_NOR_FLASH), y)
	@$(call get_otto_dir,  $(OTTO_UBOOT_SNOF_DRV_PATH), $(OTTO_IP_DIR)/$(OTTO_UBOOT_SNOF_DRV_NAME))
	@cd ./arch/otto/include/asm/arch && rm -f nor_spi && ln -s ../../../lib/$(OTTO_UBOOT_SNOF_DRV_NAME) nor_spi
endif
ifeq ($(CONFIG_SPI_NAND_FLASH), y)
	@$(call get_otto_dir,  $(OTTO_UBOOT_SNAF_DRV_PATH), $(OTTO_IP_DIR)/$(OTTO_UBOOT_SNAF_DRV_NAME))
	@cd ./arch/otto/include/asm/arch && rm -f spi_nand && ln -s ../../../lib/$(OTTO_UBOOT_SNAF_DRV_NAME) spi_nand
endif
ifdef OTTO_ONFI
	$(if $(CONFIG_ONFI_GEN),,$(error missing CONFIG_ONFI_GEN))
	$(if $(CONFIG_ECC_GEN),,$(error missing CONFIG_ECC_GEN))
	@$(call get_otto_dir,  $(OTTO_ONFI), $(OTTO_IP_DIR)/$(shell echo $(CONFIG_ONFI_GEN)))
	@$(call get_otto_dir,  $(OTTO_ECC), $(OTTO_IP_DIR)/$(shell echo $(CONFIG_ECC_GEN)))
	@cd ./arch/otto/lib && rm -rf onfi && ln -s ./$(CONFIG_ONFI_GEN) onfi
	@cd ./arch/otto/include/asm/arch && rm -f ecc onfi && ln -s ../../../lib/$(CONFIG_ECC_GEN) ecc && ln -s ../../../lib/$(CONFIG_ONFI_GEN) onfi
endif
ifeq ($(CONFIG_ECC_ENGINE), y)
	@$(call get_otto_dir,  $(OTTO_UBOOT_ECC_DRV_PATH), $(OTTO_IP_DIR)/$(OTTO_UBOOT_ECC_DRV_NAME))
	@cd ./arch/otto/include/asm/arch && rm -f ecc && ln -s ../../../lib/$(OTTO_UBOOT_ECC_DRV_NAME) ecc 
endif


DISTCLEAN_FILE += $(shell find ./arch/otto/include/asm -name "soc.h" -o -name "reg_map_util.h" -o -name "cross_env.h" -o -name "9601b_pad_ctrl.h" -o -name "otto_cg_dev_freq.h" -o -name "timer.h" -o -name "cpu.h")
DISTCLEAN_FILE += $(shell find ./arch/otto/lib -name "nor_spif_proj_dep.c")
DISTCLEAN_FOLDER += $(shell find ./arch/otto/include/asm -name "nor_spi" -o -name "spi_nand" -o -name "ecc" -o -name "onfi")
DISTCLEAN_FOLDER += $(shell find ./arch/otto/lib -name "$(OTTO_UBOOT_SNOF_DRV_NAME)" -o -name "spi_nand*" -o -name "ecc_gen1" -o -name "onfi*")

otto_distclean:
	@-rm -f $(DISTCLEAN_FILE) $(OTTO_SOC_H) $(OTTO_XENV_H) $(OTTO_RMU_H) .otto.E .otto.mk arch/otto/lib/plr_flash.c 
	@-rm -rf $(CPUDIR)/$(SOC)/preloader $(DISTCLEAN_FOLDER)


# script to get version from SVN and GIT
SVN_VCODE := svn info | grep 'Revision' | sed -e 's|Revision: ||'
SVN_MCODE := svn status | grep '^M' > /dev/null 2>&1 && echo 'M' || echo ''
GIT_VCODE := git log | head -c 15 | sed -e 's|commit ||'
GIT_MCODE := git status | grep 'modified:' > /dev/null 2>&1 && echo 'M' || echo ''

# Version of U-Boot
ifeq ($(wildcard ./.svn),./.svn)
UB_VER := $(shell $(SVN_VCODE))
UB_VER := $(UB_VER)$(shell $(SVN_MCODE))
else ifeq ($(wildcard ./.git),./.git)
UB_VER := $(shell $(GIT_VCODE))
UB_VER := $(UB_VER)$(shell $(GIT_MCODE))
else
UB_VER := NA
endif

# Version of Preloader
ifeq ($(wildcard $(OTTO_RELEASE_DIR)/../.svn),$(OTTO_RELEASE_DIR)/../.svn)
PL_VER := $(shell cd $(OTTO_RELEASE_DIR)/.. && $(SVN_VCODE))
PL_VER := $(PL_VER)$(shell cd $(OTTO_RELEASE_DIR)/.. && $(SVN_MCODE))
else ifeq ($(wildcard $(OTTO_RELEASE_DIR)/../.git),$(OTTO_RELEASE_DIR)/../.git)
PL_VER := $(shell cd $(OTTO_RELEASE_DIR)/.. && $(GIT_VCODE))
PL_VER := $(PL_VER)$(shell cd $(OTTO_RELEASE_DIR)/.. && $(GIT_MCODE))
else
PL_VER := NA
endif

.otto.v:
	@echo -n "[MISC] Generating $@... "
	@echo UB_VER := $(UB_VER) >  $@
	@echo PL_VER := $(PL_VER) >> $@
	@echo "done"

OTTO_DATE  := $(shell date +%y%m%d%H%M)
OTTO_RLZ_FN:= otto_uboot.$(OTTO_DATE).u$(UB_VER).p$(PL_VER).tgz
release: clobber
	@echo -n "[MISC] Generating tarball $(OTTO_RLZ_FN)... "
	@cp toolkit_path.in.sample toolkit_path.bak
	@cp config.in config.in.bak
	@sed '/^OTTO_RELEASE_DIR.*/d' toolkit_path.bak > toolkit_path.in.sample
	@sed -e "s|bool 'Standalone U-Boot'|define_bool 'Standalone U-Boot'|" \
		-e "s|CONFIG_STANDALONE_UBOOT|CONFIG_STANDALONE_UBOOT y|" config.in.bak > config.in
	@cd ../ && tar czf $(TOPDIR)/release/$(OTTO_RLZ_FN) $(notdir $(TOPDIR)) \
		--exclude=*.git* --exclude=otto.makefile --exclude=toolkit_path.in --exclude=OTTO_README \
		--exclude=toolkit_path.bak --exclude=config.in.bak --exclude=release
	@mv toolkit_path.bak toolkit_path.in.sample
	@mv config.in.bak config.in
	@echo "done"
