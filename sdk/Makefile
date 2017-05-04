############################################################################
#
# Makefile -- Top level uClinux makefile.
#
# Copyright (c) 2001-2002, SnapGear (www.snapgear.com)
# Copyright (c) 2001, Lineo
#

############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#

ifeq (.config,$(wildcard .config))
include .config

ifeq ($(CONFIG_PRODUCT),luna)
all: busybox_check experiment subdirs romfs rcX modules modules_install image vmimg
else
all: subdirs romfs modules modules_install image vmimg
endif

else
all: config_error
endif

############################################################################
#
# Software Version
#
ifeq ($(CONFIG_LINUXDIR),linux-2.6.x)
VERSIONPKG = 2.0.1
else
VERSIONPKG = 1.4.1-RC
endif
#VERSIONSTR = $(CONFIG_VENDOR)/$(CONFIG_PRODUCT) Version $(VERSIONPKG)
VERSIONSTR = $(VERSIONPKG)


############################################################################
#
# Get the core stuff worked out
#

LINUXDIR = $(CONFIG_LINUXDIR)
ifndef CONFIG_USE_RSDK_WRAPPER
LIBCDIR  = $(CONFIG_LIBCDIR)
endif #CONFIG_USE_RSDK_WRAPPER
ROOTDIR  = $(shell pwd)
PATH	 := $(PATH):$(ROOTDIR)/tools
HOSTCC   = unset GCC_EXEC_PREFIX; cc
IMAGEDIR = $(ROOTDIR)/images
ROMFSDIR = $(ROOTDIR)/romfs
ROMFSINST= romfs-inst.sh
SCRIPTSDIR = $(ROOTDIR)/config/scripts
TFTPDIR    = /tftpboot
#Added by Mason Yu. for ipv6
BUSYBOX_DEFAULT = busybox-1.12.4
BUSYBOXDIR = $(ROOTDIR)/user/$(BUSYBOX_DEFAULT)
#############################################################
ifeq ($(MAKECMDGOALS:preconfig2630_%=preconfig2630),preconfig2630)
	ifeq ($(CONFIG_PRODUCT),luna)
		PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf2630/$(MAKECMDGOALS:preconfig2630_%=%)
	else
		PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf2630
	endif
else ifeq ($(MAKECMDGOALS:preconfig318_%=preconfig318),preconfig318)
	ifeq ($(CONFIG_PRODUCT),luna)
		PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf318/$(MAKECMDGOALS:preconfig318_%=%)
	else
		PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf318
	endif
else
	CONFIG_PRODUCT ?=luna
	ifeq ($(CONFIG_PRODUCT),luna)
		CONFIG_PRD_SIGNATURE ?= DL8696RG
		CONFIG_PRJ_SIGNATURE ?= demo
		PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf2630/$(CONFIG_PRD_SIGNATURE)_$(CONFIG_PRJ_SIGNATURE)
	else
	PRECFGDIR  := $(ROOTDIR)/vendors/Realtek/$(CONFIG_PRODUCT)/conf2630
	endif
endif
export PRECFGDIR
###########################################################################

CFG = cfg_$(CONFIG_PRD_SIGNATURE)_$(CONFIG_PRJ_SIGNATURE).tgz
LINUX_CONFIG  = $(ROOTDIR)/$(LINUXDIR)/.config
CONFIG_CONFIG = $(ROOTDIR)/config/.config
MODULES_CONFIG = $(ROOTDIR)/modules/.config

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

ifeq (config.arch,$(wildcard config.arch))
include config.arch
ARCH_CONFIG = $(ROOTDIR)/config.arch
export ARCH_CONFIG
endif


ifdef CONFIG_USE_RSDK_WRAPPER
	MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
  ifdef CONFIG_KERNEL_2_6_30
		MAKEARCH_KERNEL = $(MAKE) ARCH=rlx CROSS_COMPILE=$(CROSS_KERNEL_COMPILE)
  else #CONFIG_KERNEL_2_6_30
		MAKEARCH_KERNEL = $(MAKEARCH)
  endif #CONFIG_KERNEL_2_6_30
	PATH := $(PATH):$(RSDK)/bin
else #CONFIG_USE_RSDK_WRAPPER
ifneq ($(SUBARCH),)
# Using UML, so make the kernel and non-kernel with different ARCHs
MAKEARCH = $(MAKE) ARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE) ARCH=$(ARCH) SUBARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
else
MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKEARCH)
endif
endif #CONFIG_USE_RSDK_WRAPPER



DIRS    = lib user2 user 
VENDDIR = $(ROOTDIR)/vendors/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.
KDIR=$(ROOTDIR)/linux-2.6.x
NETDIR=$(KDIR)/drivers/net
APOLLODIR=$(NETDIR)/rtl86900
SDKDIR=$(APOLLODIR)/sdk
KERNEL_DIR=$(KDIR)

LUNA_EXTRA_CHK ?= 0
MULTI_LANG_DL ?= 1
export LUNA_EXTRA_CHK
export MULTI_LANG_DL
export KDIR APOLLODIR SDKDIR KERNEL_DIR
export VENDOR PRODUCT ROOTDIR LINUXDIR HOSTCC CONFIG_SHELL
export CONFIG_CONFIG LINUX_CONFIG ROMFSDIR SCRIPTSDIR
export VERSIONPKG VERSIONSTR ROMFSINST PATH IMAGEDIR TFTPDIR

include $(APOLLODIR)/Compiler_Flag

ifeq ($(CONFIG_USE_PRELOADER_PARAMETERS), y)
ifeq ($(wildcard ./toolkit_path.in), ./toolkit_path.in)
    include ./toolkit_path.in
endif
export PLR_INC
endif

.PHONY: soc_h_prepare
soc_h_prepare:
ifeq ($(CONFIG_USE_PRELOADER_PARAMETERS), y)
	./tools/update_soc_h.sh
endif
############################################################################

#
# Config stuff,  we recall ourselves to load the new config.arch before
# running the kernel and other config scripts
#

.PHONY: config.tk config.in

config.in:
	config/mkconfig > config.in

config.tk: config.in
	$(MAKE) -C $(SCRIPTSDIR) tkparse
	ARCH=dummy $(SCRIPTSDIR)/tkparse < config.in > config.tmp
	@if [ -f /usr/local/bin/wish ];	then \
		echo '#!'"/usr/local/bin/wish -f" > config.tk; \
	else \
		echo '#!'"/usr/bin/wish -f" > config.tk; \
	fi
	cat $(SCRIPTSDIR)/header.tk >> ./config.tk
	cat config.tmp >> config.tk
	rm -f config.tmp
	echo "set defaults \"/dev/null\"" >> config.tk
	echo "set help_file \"config/Configure.help\"" >> config.tk
	cat $(SCRIPTSDIR)/tail.tk >> config.tk
	chmod 755 config.tk

.PHONY: xconfig
xconfig: config.tk
	@wish -f config.tk
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_xconfig; \
	 fi
	@config/setconfig final

.PHONY: config
config: config.in
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Configure config.in
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_config; \
	 fi
	@config/setconfig final

.PHONY: chk_ker_debug
chk_ker_debug:
ifeq ($(RTK_KERN_DEBUG),y)
	./tools/luna_build_kern_debug.sh
endif

.PHONY: menuconfig
menuconfig: 
	$(MAKE) menuconfig_phase1
	$(MAKE) menuconfig_phase2
	$(MAKE) menuconfig_phase3
	@config/setconfig final

.PHONY: menuconfig_phase1
menuconfig_phase1: config.in voip_prepare
	$(MAKE) -C $(SCRIPTSDIR)/lxdialog all
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Menuconfig config.in
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults

.PHONY: menuconfig_phase2
menuconfig_phase2:  chk_ker_debug
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_menuconfig; \
	fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_menuconfig; \
	fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_menuconfig; \
	fi

.PHONY: menuconfig_phase3
menuconfig_phase3: busybox_check
	$(eval BUSYBOXDIR_DEST := $(ROOTDIR)/$(shell $(ROOTDIR)/config/config_bb_ver.sh dir $(PRECFGDIR) $(ROOTDIR) $(CONFIG_CONFIG) $(BUSYBOX_DEFAULT)))
	@echo BUSYBOXDIR_DEST=$(BUSYBOXDIR_DEST)
	@if egrep "^CONFIG_MCONF_BZBOX=y" .config > /dev/null; then \
		$(MAKEARCH) -C $(BUSYBOXDIR_DEST) menuconfig; \
	fi
	@if egrep "^CONFIG_USE_PRELOADER_PARAMETERS=y" .config > /dev/null; then \
		$(MAKE) soc_h_prepare; \
	fi

menuconfig_dead: config.in voip_prepare
	$(MAKE) -C $(SCRIPTSDIR)/lxdialog all
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Menuconfig config.in
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_LUNA_DUAL_LINUX=y" .config > /dev/null; then \
		$(MAKE) -f LDLMakefile dual_linux_menuconfig ; \
	else \
		$(MAKE) single_linux_menuconfig ; \
	fi
	@if egrep "^CONFIG_USE_PRELOADER_PARAMETERS=y" .config > /dev/null; then \
		$(MAKE) soc_h_prepare; \
	fi
	@config/setconfig final

.PHONY: oldconfig
oldconfig:
	@$(MAKE) oldconfig_linux
	@$(MAKE) oldconfig_modules
	@$(MAKE) oldconfig_config
	@config/setconfig final

.PHONY: modules
modules:
	. $(LINUX_CONFIG); if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(LINUXDIR)/modules ] || mkdir $(LINUXDIR)/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) modules; \
	fi

.PHONY: modules_install
modules_install:
	. $(LINUX_CONFIG); if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=true modules_install; \
		rm -f $(ROMFSDIR)/lib/modules/*/build; \
	fi

linux_xconfig:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) xconfig
linux_menuconfig: voip_prepare
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) menuconfig
linux_config:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) config
modules_xconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules xconfig
modules_menuconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules menuconfig
modules_config:
	[ ! -d modules ] || $(MAKEARCH) -C modules config
modules_clean:
	-[ ! -d modules ] || $(MAKEARCH) -C modules clean
config_xconfig:
	$(MAKEARCH) -C config xconfig
config_menuconfig:
	$(MAKEARCH) -C config menuconfig
config_config:
	$(MAKEARCH) -C config config
oldconfig_config:
	$(MAKEARCH) -C config oldconfig
oldconfig_modules:
	[ ! -d modules ] || $(MAKEARCH) -C modules oldconfig
oldconfig_linux:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) oldconfig

preconfig_%:	
	@if [ -f $(VENDDIR)/conf/config_kernel_$(@:preconfig_%=%) ] ; then \
		cp $(VENDDIR)/conf/config .config && \
		cp $(VENDDIR)/conf/config_kernel_$(@:preconfig_%=%) $(LINUXDIR)/.config && \
		cp $(VENDDIR)/conf/config_user_$(@:preconfig_%=%) config/.config && \
		if [ $(@:preconfig_%=%) -eq 10 ] ; then \
			cp $(VENDDIR)/conf/config_uClibc_$(@:preconfig_%=%) uClibc/.config ; \
		fi; \
		echo "Configuration $(@:preconfig_%=%) loaded." ; \
		rm *_loaded ; \
		touch $@_loaded ; \
	else \
		echo "No configuration found" ; \
	fi

preconfig318_%:
	@if [ -f *_loaded ] &&  [ ! -f $@_loaded ] ; then \
		$(MAKE) clean; \
		echo "Change to $@. Make distclean first!" ; \
		rm *_loaded; \
	fi
	@if [ -d $(VENDDIR)/conf318/$* ] ; then \
		cp $(VENDDIR)/conf318/$*/config .config && \
		cp $(VENDDIR)/conf318/$*/config_kernel linux-3.18.x/.config && \
		cp $(VENDDIR)/conf318/$*/config_user $(CONFIG_CONFIG) && \
		if [ -f $(ROOTDIR)/config/config_bb_ver.sh ]; then \
			$(ROOTDIR)/config/config_bb_ver.sh "prepare" $(PRECFGDIR) $(ROOTDIR) $(CONFIG_CONFIG) $(BUSYBOX_DEFAULT); \
		fi; \
		if [ -f $(VENDDIR)/conf318/$*/soc.h ]; then \
			cp $(VENDDIR)/conf318/$*/soc.h linux-3.18.x/include/soc/soc.h ; \
		fi; \
		if [ -f $(VENDDIR)/conf318/$*/luna_cfg.h ]; then \
			cp $(VENDDIR)/conf318/$*/luna_cfg.h linux-3.18.x/include/soc/luna_cfg.h ; \
		else \
			echo '/* dummy file */' > linux-3.18.x/include/soc/luna_cfg.h ; \
		fi; \
		if [ -f $(VENDDIR)/conf318/$*/board.c ]; then \
			cp $(VENDDIR)/conf318/$*/board.c linux-3.18.x/arch/mips/bsp_rtl8686/board.c ; \
		fi; \
		[ -f config/config_default.xml ] && mv config/config_default.xml config/config_default.xml.backup; \
		[ -f config/config_default_hs.xml ] && mv config/config_default_hs.xml config/config_default_hs.xml.backup; \
		cp $(VENDDIR)/conf318/$*/config_default.xml config/config_default.xml ; \
		cp $(VENDDIR)/conf318/$*/config_default_hs.xml config/config_default_hs.xml ; \
		echo "Configuration318 $* loaded." ; \
		touch $@_loaded ; \
	else \
		echo "No $(CONFIG_PRODUCT) configuration found" ; \
	fi

preconfig2630_%:
ifeq ($(CONFIG_PRODUCT),luna)
	@if [ -f *_loaded ] &&  [ ! -f $@_loaded ] ; then \
		$(MAKE) clean; \
		echo "Change to $@. Make distclean first!" ; \
		rm *_loaded; \
	fi	
	@if [ -d $(VENDDIR)/conf2630/$* ] ; then \
		cp $(VENDDIR)/conf2630/$*/config .config && \
		cp $(VENDDIR)/conf2630/$*/config_kernel $(LINUX_CONFIG) && \
		cp $(VENDDIR)/conf2630/$*/config_user $(CONFIG_CONFIG) && \
		if [ -f $(ROOTDIR)/config/config_bb_ver.sh ]; then \
			$(ROOTDIR)/config/config_bb_ver.sh "prepare" $(PRECFGDIR) $(ROOTDIR) $(CONFIG_CONFIG) $(BUSYBOX_DEFAULT); \
		fi; \
		if [ -f $(VENDDIR)/conf2630/$*/soc.h ]; then \
			cp $(VENDDIR)/conf2630/$*/soc.h $(LINUXDIR)/include/soc/soc.h ; \
		fi; \
		if [ -f $(VENDDIR)/conf2630/$*/luna_cfg.h ]; then \
			cp $(VENDDIR)/conf2630/$*/luna_cfg.h $(LINUXDIR)/include/soc/luna_cfg.h ; \
		else \
			echo '/* dummy file */' > $(LINUXDIR)/include/soc/luna_cfg.h ; \
		fi; \
		if [ -f $(VENDDIR)/conf2630/$*/board.c ]; then \
			cp $(VENDDIR)/conf2630/$*/board.c $(LINUXDIR)/arch/rlx/bsp_rtl8686/board.c ; \
		fi; \
		[ -f config/config_default.xml ] && mv config/config_default.xml config/config_default.xml.backup; \
		[ -f config/config_default_hs.xml ] && mv config/config_default_hs.xml config/config_default_hs.xml.backup; \
		cp $(VENDDIR)/conf2630/$*/config_default.xml config/config_default.xml ; \
		cp $(VENDDIR)/conf2630/$*/config_default_hs.xml config/config_default_hs.xml ; \
		echo "Configuration2630 $* loaded." ; \
                touch $@_loaded ; \
        else \
	        echo "No $(CONFIG_PRODUCT) configuration found" ; \
	fi

else
	@if [ -f $(VENDDIR)/conf2630/config_kernel_$* ] ; then \
		cp $(VENDDIR)/conf2630/config_$* .config && \
		cp $(VENDDIR)/conf2630/config_kernel_$* $(LINUX_CONFIG) && \
		cp $(VENDDIR)/conf2630/config_user_$* $(CONFIG_CONFIG) && \
		if [ -f $(VENDDIR)/conf2630/config_busybox_$* ]; then \
			cp $(VENDDIR)/conf2630/config_busybox_$* $(BUSYBOXDIR)/.config ; \
		fi; \
		if [ -f $(VENDDIR)/conf2630/soc_$*.h ]; then \
			cp -a $(VENDDIR)/conf2630/soc_$*.h $(LINUXDIR)/include/soc/soc.h ; \
		fi; \
		echo "Configuration2630 $* loaded." ; \
		rm *_loaded ; \
		touch $@_loaded ; \
	else \
		echo "No $(CONFIG_PRODUCT) configuration found" ; \
	fi
endif 

ifdef CONFIG_USE_RSDK_WRAPPER
LSTRIP = $(CROSS_COMPILE)lstrip
LSTRIPPED = $(ROMFSDIR)/.lstripped
endif #CONFIG_USE_RSDK_WRAPPER

############################################################################
#
# normal make targets
#

.PHONY: romfs
romfs:
  ifdef CONFIG_USE_RSDK_WRAPPER
	$(MAKEARCH) -C $(VENDDIR) mkromfsdir
  endif #CONFIG_USE_RSDK_WRAPPER
	for dir in $(DIRS) ; do $(MAKEARCH) -C $$dir romfs STRIP=echo || exit 1 ; done
	-find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf
	-find $(ROMFSDIR)/. -name .svn -type d | xargs -r rm -rf
  ifdef CONFIG_USE_RSDK_WRAPPER
	cp -R $(RSDK)/lib/*.so $(ROMFSDIR)/lib
	cp -R $(RSDK)/lib/*.so.* $(ROMFSDIR)/lib
	if [ "$(shell ls -A $(ROOTDIR)/lib/librtk/*.so)" ]; then \
		cp -R $(ROOTDIR)/lib/librtk/*.so $(ROMFSDIR)/lib; \
	fi
	chmod 755 $(ROMFSDIR)/lib/*.so
	rm -f $(LSTRIPPED)
  endif #CONFIG_USE_RSDK_WRAPPER
	if [ -d luna_patch ]; then \
		cp -a luna_patch $(ROMFSDIR) || exit 1 ; \
	fi

.PHONY: rcX
rcX: rcX_clean
	for dir in $(VENDDIR) user user2 ; do \
		if [ "`grep "^rcX:" $$dir/Makefile`" != "" ]; then \
			$(MAKEARCH) -C $$dir rcX || exit 1 ; \
		fi \
	done		
	for rcfile in $(ROMFSDIR)/etc/init.d/rc[0-9]*; do \
		chmod -f 755 $$rcfile; \
	done

.PHONY: rcX_clean
rcX_clean:
	rm -rf $(ROMFSDIR)/etc/init.d/rc[0-9]*
	
ifeq (experiment_setting,$(wildcard experiment_setting))
EXPER_MODE = y
else
EXPER_MODE = n
endif
export EXPER_MODE

.PHONY: busybox_check
busybox_check:
	$(eval BUSYBOXDIR_DEST := $(shell $(ROOTDIR)/config/config_bb_ver.sh check $(PRECFGDIR) $(ROOTDIR) $(CONFIG_CONFIG) $(BUSYBOX_DEFAULT)))
	@if [ "$(BUSYBOXDIR_DEST)" = "ERROR" ]; then \
			echo "******* $(BUSYBOXDIR_DEST) *******"; \
			echo "RSDK version >= 4.4,7, you need to use new busybox version >= 1.22!" ;\
			exit 1 ; \
	fi

.PHONY: experiment
experiment:
	@if [ "$(EXPER_MODE)" != "y" ] && [ "$(CONFIG_RTL_MULTI_LAN_DEV)" = "y" ] && [ "$(CONFIG_RTK_RG_INIT)" = "y" ]; then \
		echo "Not ready for 'CONFIG_RTL_MULTI_LAN_DEV=y' and 'CONFIG_RTK_RG_INIT=y'"; exit 1;\
	fi	
	
.PHONY: dev
dev:
	$(MAKEARCH) -C user dev

.PHONY: image
sinclude $(LINUX_CONFIG)
sinclude $(CONFIG_CONFIG)


ifdef CONFIG_SQUASHFS
RUN_SQUASHFS_SCRIPT=y
else
ifdef CONFIG_BLK_DEV_INITRD
RUN_SQUASHFS_SCRIPT=y
endif
endif


FWU_VER_FILE:=$(LINUXDIR)/fwu_ver
ifdef OMCI_VERSION
VERSIONPKG := $(OMCI_VERSION)
else
stamp:=$(shell date +%y%m%d)
ver:=$(shell cat $(LINUXDIR)/luna_sdk_ver | tr -s "[\t]" " " | cut -d " " -f4)
VERSIONPKG := $(ver)-$(stamp)
endif

image:
	@echo VERSIONPKG=$(VERSIONPKG)
	@echo $(VERSIONPKG) > $(FWU_VER_FILE)

ifdef CONFIG_ROMFS_FS
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	$(MAKEARCH) -C $(VENDDIR) image
endif
ifdef CONFIG_CRAMFS
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	./mkcramfs $(ROMFSDIR) rootfs
endif
ifdef RUN_SQUASHFS_SCRIPT
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
  ifdef CONFIG_USE_RSDK_WRAPPER
    ifeq ($(findstring 1.3.6,$(RSDK)),1.3.6)
	$(LSTRIP) $(ROMFSDIR) $(ROOTDIR)/lib/librtk
    else
	@if [ -f $(LSTRIPPED) ]; then \
		echo; \
		echo "Cannot lstrip twice on romfs/, please run make romfs image or make all"; \
		echo; \
		exit 1; \
	 fi
	$(LSTRIP) $(ROMFSDIR)
	touch $(LSTRIPPED)
    endif
    
    # Copy the language lib again. (for general multi-lingual dynamic loading)
    ifndef CONFIG_E8B
        ifeq ($(MULTI_LANG_DL),1)
            ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_EN),y)
		cp $(ROOTDIR)/lib/librtk/libmultilang_en.so $(ROMFSDIR)/lib 
            endif
            ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_RU),y)
		cp $(ROOTDIR)/lib/librtk/libmultilang_ru.so $(ROMFSDIR)/lib 
            endif
            ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW),y)
		cp $(ROOTDIR)/lib/librtk/libmultilang_tw.so $(ROMFSDIR)/lib 
            endif
            ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN),y)
		cp $(ROOTDIR)/lib/librtk/libmultilang_cn.so $(ROMFSDIR)/lib 
            endif
        endif
    endif

  else  #CONFIG_USE_RSDK_WRAPPER
	./tools/libstrip/libstrip
  endif #CONFIG_USE_RSDK_WRAPPER
  ifdef CONFIG_KERNEL_2_6_19
	./mksquashfs-lzma $(ROMFSDIR) rootfs -be -noappend
  endif
  ifdef CONFIG_KERNEL_2_6_30
	$(LINUXDIR)/mksquashfs $(ROMFSDIR) rootfs -comp lzma  -noappend -pf $(VENDDIR)/squashfs-pf.list -always-use-fragments
	cp rootfs $(IMAGEDIR)
  endif
  ifdef CONFIG_DEFAULTS_KERNEL_3_18
	$(LINUXDIR)/mksquashfs $(ROMFSDIR) rootfs -comp gzip -noappend -pf $(VENDDIR)/squashfs-pf.list -always-use-fragments
	cp rootfs $(IMAGEDIR)
  endif
  ifeq ($(CONFIG_DEFAULT_WEB),y)
	@printf "Now the image you are compiling is  for:\33[0;31mdefault\n\33[0m"
  endif
  ifeq ($(CONFIG_GUI_WEB),y)
	@printf "Now the image you are compiling is  for:\33[0;31mGUI menu\n\33[0m"
  endif
  ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
	@printf "Now the image you are compiling is  for:\33[0;31mCTC E8B CH VER!\n\33[0m"
  endif
  ifndef CONFIG_USER_BOA_SRC_BOA
	@printf "Now the image you are compiling is  for:\33[0;31mNo BOA\n\33[0m"
  endif
endif
#
# fancy target that allows a vendor to have other top level
# make targets,  for example "make vendor_flash" will run the
# vendor_flash target in the vendors directory
#

vendor_%:
	$(MAKEARCH) -C $(VENDDIR) $@

.PHONY: linux
linux linux_only: soc_h_prepare
ifdef CONFIG_BLK_DEV_INITRD
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
endif #CONFIG_BLK_DEV_INITRD
	@if [ $(LINUXDIR) != linux-3.18.x -a != linux-2.5.x -a $(LINUXDIR) != linux-2.6.x -a ! -f $(LINUXDIR)/.depend ] ; then \
		echo "ERROR: you need to do a 'make dep' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -j 5 -C $(LINUXDIR) $(LINUXTARGET) || exit 1
	if [ -f $(LINUXDIR)/vmlinux ]; then \
		ln -f $(LINUXDIR)/vmlinux $(LINUXDIR)/linux ; \
	fi
  ifdef CONFIG_USE_RSDK_WRAPPER
	[ -d $(ROOTDIR)/include ] || mkdir -p $(ROOTDIR)/include
	[ -d $(ROOTDIR)/include/linux ] || mkdir -p $(ROOTDIR)/include/linux
	ln -fs $(ROOTDIR)/$(LINUXDIR)/include/linux/config.h include/linux/config.h
	ln -fs $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h include/linux/autoconf.h
  endif #CONFIG_USE_RSDK_WRAPPER

.PHONY: voip_prepare
voip_prepare:
	@if [ -d $(ROOTDIR)/rtk_voip ] ; then \
		cd $(LINUXDIR)/drivers/char ; ln -fsn ../../../rtk_voip/aipc_char  aipc     ; cd - ; \
		cd $(LINUXDIR)              ; ln -fsn ../rtk_voip/kernel           rtk_voip ; cd - ; \
		cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/maserati_2.0     rtk_voip ; cd - ; \
		cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/aipc_util        aipc_util; cd - ; \
	else \
		cd $(LINUXDIR)/drivers/char ; ln -fsn ../../dummy aipc ; \
		cd $(LINUXDIR)              ; ln -fsn dummy rtk_voip   ; \
	fi
#ifeq ($(CONFIG_VOIP_IPC_DSP_ARCHITECTURE) , y)
#	@cd $(LINUXDIR)              ; ln -fsn ../rtk_voip/kernel           rtk_voip   ; cd -
#	@cd $(LINUXDIR)/drivers/char ; ln -fsn ../../../rtk_voip/aipc_char  aipc       ; cd -
#	@cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/maserati_2.0        rtk_voip   ; cd -
#	#@cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/solar_1.6        rtk_voip   ; cd -
#	@cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/aipc_util        aipc_util  ; cd -
#else
#	@if [ -d $(ROOTDIR)/rtk_voip ] ; then \
#		cd $(LINUXDIR)/drivers/char ; ln -fsn ../../../rtk_voip/aipc_char  aipc     ; cd - ; \
#		cd $(LINUXDIR)              ; ln -fsn ../rtk_voip/kernel           rtk_voip ; cd - ; \
#		cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/maserati_2.0     rtk_voip ; cd - ; \
#		cd $(ROOTDIR)/user          ; ln -fsn ../rtk_voip/aipc_util        aipc_util; cd - ; \
#	else \
#		cd $(LINUXDIR)/drivers/char ; ln -fsn ../../dummy aipc ; \
#		cd $(LINUXDIR)              ; ln -fsn dummy rtk_voip   ; \
#	fi
#endif

kernel_aipc_clean:
	@[ -d $(ROOTDIR)/rtk_voip/aipc_char ] && find $(ROOTDIR)/rtk_voip/aipc_char \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name 'Module.markers' -o -name '.tmp_*.o.*' \) \
		-exec rm -f {} \;
kernel_voip_clean:
	@[ -d $(ROOTDIR)/rtk_voip/kernel ] && find $(ROOTDIR)/rtk_voip/kernel \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name 'Module.markers' -o -name '.tmp_*.o.*' \) \
		-exec rm -f {} \;

.PHONY: wifi_prepare
wifi_prepare:
	@if egrep "^CONFIG_RTL8192E_DRIVER=y" $(LINUX_CONFIG) > /dev/null; then \
		ln -fsn rtl8192cd_wlan $(LINUXDIR)/drivers/net/wireless/rtl8192cd_wifi;\
	elif egrep "^CONFIG_RTL8812_DRIVER=y" $(LINUX_CONFIG) > /dev/null; then \
		ln -fsn rtl8192cd_wlan $(LINUXDIR)/drivers/net/wireless/rtl8192cd_wifi;\
        else\
		ln -fsn rtl8192cd $(LINUXDIR)/drivers/net/wireless/rtl8192cd_wifi;\
	fi

.PHONY: subdirs
subdirs: linux
ifdef CONFIG_KERNEL_2_6_30
	for dir in $(DIRS) ; do $(MAKEARCH) -C $$dir || exit 1 ; done
else  #CONFIG_KERNEL_2_6_30
	for dir in $(DIRS) ; do $(MAKEARCH_KERNEL) -C $$dir || exit 1 ; done
endif #CONFIG_KERNEL_2_6_30

dep:
	@if [ ! -f $(LINUX_CONFIG) ] ; then \
		echo "ERROR: you need to do a 'make config' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) dep

# This one removes all executables from the tree and forces their relinking
.PHONY: relink
relink:
	find user -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f
	find $(VENDDIR) -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f

# This one build a rom image including boot code and run-time image
.PHONY: rom vmimg
ifeq ($(CONFIG_SINGLE_IMAGE),y)
ROOTFS_OFFSET := 0x40000
endif
ifeq ($(CONFIG_DOUBLE_IMAGE),y)
ROOTFS_OFFSET := 0x100000
endif
rom: rom.script
	@if [ ! -f dummy.S ] ; then \
		touch dummy.S; \
	fi
	$(CC) $(CFLAG) -c dummy.S -o dummy.o
	$(OBJCOPY) --add-section .boot=boot.bin dummy.o
	$(OBJCOPY) --add-section .pad=pad_file dummy.o
	$(OBJCOPY) --add-section .run=linux-2.6.x/vmbin.img dummy.o
	$(OBJCOPY) --remove-section=.text --remove-section=.bss --remove-section=.reginfo --remove-section=.pdr dummy.o
	$(LD) -nostdlib -Trom.script dummy.o -o rom.elf
	$(OBJCOPY) -Obinary rom.elf rombin.img
	./tools/packimg/genhead  -i rombin.img -o rom.hdr -k 0xa0000003 -f 0xbd000000  -a 0x80000000 -e 0x80000000
	cat rom.hdr rombin.img > rom.img

rom.script: rom.script.in
	sed -e 's/@@ROOTFS_OFFSET@@/$(ROOTFS_OFFSET)/' <$< >$@

vmimg:
  ifdef CONFIG_KERNEL_2_6_30
    ifdef CONFIG_BOA_WEB_E8B_CH
			$(MAKEARCH_KERNEL) -C $(LINUXDIR) vmimg_e8b
    else
      ifdef CONFIG_BLK_DEV_INITRD
				make linux
				$(MAKEARCH_KERNEL) -C $(LINUXDIR) vmimg_kernel
      else #CONFIG_BLK_DEV_INITRD
				$(MAKEARCH_KERNEL) -C $(LINUXDIR) vmimg
      endif #CONFIG_BLK_DEV_INITRD
    endif
  else #CONFIG_KERNEL_2_6_30
		$(MAKEARCH) -C $(LINUXDIR) vmimg
  endif #CONFIG_KERNEL_2_6_30

dep26:
	find ./user/boa/ -type l | xargs rm
	rm user/boa/default_web_id

clean: modules_clean
	#for dir in $(LINUXDIR) $(DIRS); do $(MAKEARCH) -C $$dir clean ; done
	for dir in $(DIRS); do $(MAKEARCH) -C $$dir clean ; done
	-rm -rf $(ROMFSDIR)/*
	rm -rf $(IMAGEDIR)/*
	rm -f config.tk
	#rm -f $(LINUXDIR)/linux
	rm -rf $(LINUXDIR)/net/ipsec/alg/libaes $(LINUXDIR)/net/ipsec/alg/perlasm
  ifdef CONFIG_USE_RSDK_WRAPPER
	rm -rf $(ROOTDIR)/include
  endif #CONFIG_USE_RSDK_WRAPPER
	make -C config/scripts/lxdialog/ clean
	make -C $(LINUXDIR) clean
	make kernel_aipc_clean kernel_voip_clean

linux_clean: modules_clean
	make -C config/scripts/lxdialog/ clean
	make -C $(LINUXDIR) clean
	make kernel_aipc_clean kernel_voip_clean

real_clean mrproper: clean
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) mrproper
	-$(MAKEARCH) -C config clean
	-rm -rf romfs config.in config.arch config.tk images
	rm -f modules/config.tk
	rm -rf .config .config.old .oldconfig autoconf.h

distclean: mrproper
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) distclean

%_menuconfig:
	[ ! -d "$(@:_menuconfig=)" ] || $(MAKEARCH) -C $(@:_menuconfig=) menuconfig

%_only:
	if [ -d "$(@:_only=)" ]; then \
		if [ -n "$(findstring user/,$@)" ]; then \
			$(MAKEARCH) -C user $(@:user/%=%); \
		else \
			if [ ! -f $(@:_only=)/Makefile ]; then \
				cd $(@:_only=) && ./configure --host=mips-linux --target=mips-linux && cd -; \
			fi; \
			$(MAKEARCH) -C $(@:_only=); \
		fi; \
	fi

%_romfs:
	[ ! -d "$(@:_romfs=)" ] || $(MAKEARCH) -C $(@:_romfs=) romfs

ifdef CONFIG_KERNEL_2_6_30
linux%_clean:
	[ ! -d "$(@:_clean=)" ] || $(MAKEARCH_KERNEL) -C $(@:_clean=) clean
	make kernel_aipc_clean kernel_voip_clean
endif #CONFIG_KERNEL_2_6_30

%_clean:
	[ ! -d "$(@:_clean=)" ] || $(MAKEARCH) -C $(@:_clean=) clean

.PHONY: config_backup
config_backup:
	$(eval BUSYBOXDIR_DEST := $(ROOTDIR)/$(shell $(ROOTDIR)/config/config_bb_ver.sh dir $(PRECFGDIR) $(ROOTDIR) $(CONFIG_CONFIG) $(BUSYBOX_DEFAULT) ))
	@echo BUSYBOXDIR_DEST=$(BUSYBOXDIR_DEST)
	@tar czf $(CFG) \
	.config \
       	$(patsubst $(ROOTDIR)/%, % ,$(LINUX_CONFIG)) \
       	$(patsubst $(ROOTDIR)/%, % ,$(CONFIG_CONFIG)) \
       	$(patsubst $(ROOTDIR)/%, %/.config ,$(BUSYBOXDIR_DEST)) \
       	config/autoconf.h
	@echo "Backup the configs in $(CFG) done"

.PHONY: config_restore
config_restore:
	@if [ -f $(CFG) ]; then \
		tar xzfm $(CFG) -C $(ROOTDIR); \
               	echo "Restore $(CFG) done!" ; \
	else \
        	echo "No backup config file: $(CFG)"; \
	fi

config_error:
	@echo "*************************************************"
	@echo "You have not run make config."
	@echo "The build sequence for this source tree is:"
	@echo "1. 'make config' or 'make xconfig'"
	@echo "2. 'make dep'"
	@echo "3. 'make'"
	@echo "*************************************************"
	@exit 1

prune:
	$(MAKE) -C user prune

dist-prep:
	-find $(ROOTDIR) -name 'Makefile*.bin' | while read t; do \
		$(MAKEARCH) -C `dirname $$t` -f `basename $$t` $@; \
	 done


sdk.shell_tree:
	$(MAKE) -C user/diagshell shell_tree
sdk.diag:
	$(MAKE) -C user/diagshell all
sdk.install:
	if [ -d $(SDKDIR)/rtk.etc ]; then \
		cp -af $(SDKDIR)/rtk.etc/*.sh $(ROMFSDIR)/etc/ ; \
		echo "/etc/runsdk.sh" >> $(ROMFSDIR)/etc/rc ; \
	fi;
