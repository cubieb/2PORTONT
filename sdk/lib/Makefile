#
#	Makefile -- Master makefile for all libraries.
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_LIB=1
include $(LINUX_CONFIG)
include $(CONFIG_CONFIG)
include $(ARCH_CONFIG)

#
# always build these dir_
#
ifdef CONFIG_USE_RSDK_WRAPPER
dir_y  = libnet libcrypt_old
dir_n  =
dir_   =
LN     = ln
else #CONFIG_USE_RSDK_WRAPPER
dir_y  = $(LIBCDIR) libnet libcrypt_old
dir_n  =
dir_   =

ifeq ($(LIBCDIR),libc)
dir_y += libm
endif
endif #CONFIG_USE_RSDK_WRAPPER


#
# and build these if someone needs them
#
dir_$(CONFIG_LIB_SNAPGEAR)     += libsnapgear
dir_$(CONFIG_LIB_LIBAES)       += libaes
dir_$(CONFIG_LIB_LIBAES_FORCE) += libaes
dir_$(CONFIG_LIB_LIBDES)       += libdes
dir_$(CONFIG_LIB_LIBDES_FORCE) += libdes
dir_$(CONFIG_LIB_LIBSSL)       += libssl
dir_$(CONFIG_LIB_LIBSSL_FORCE) += libssl
dir_$(CONFIG_LIB_LIBMATRIXSSL) += libmatrixssl
dir_$(CONFIG_LIB_LIBGMP)       += libgmp
dir_$(CONFIG_LIB_LIBGMP_FORCE) += libgmp
dir_$(CONFIG_LIB_LIBG)         += libg
dir_$(CONFIG_LIB_LIBG_FORCE)   += libg
dir_$(CONFIG_LIB_LIBPAM)       += libpam
dir_$(CONFIG_LIB_LIBPAM_FORCE) += libpam
dir_$(CONFIG_LIB_LIBPCAP)      += libpcap
dir_$(CONFIG_LIB_LIBPCAP_FORCE)+= libpcap
dir_$(CONFIG_LIB_ZLIB)         += zlib
dir_$(CONFIG_LIB_ZLIB_FORCE)   += zlib
dir_$(CONFIG_LIB_LIBATM)       += libatm
dir_$(CONFIG_LIB_LIBATM_FORCE) += libatm
dir_$(CONFIG_LIB_LIBPNG)       += libpng
dir_$(CONFIG_LIB_LIBPNG_FORCE) += libpng
dir_$(CONFIG_LIB_LIBJPEG)      += libjpeg
dir_$(CONFIG_LIB_LIBJPEG_FORCE)+= libjpeg
dir_$(CONFIG_LIB_LIBSNAPGEARPP) += libsnapgear++
dir_$(CONFIG_LIB_LIBCCMALLOC)  += libccmalloc
dir_$(CONFIG_LIB_LIBMD5_FORCE) += libmd5
# Mason Yu. for ipv6
dir_$(CONFIG_LIB_FLEX) += libflex
dir_$(CONFIG_LIB_NCURSES)      += ncurses
dir_$(CONFIG_LIB_NCURSES_FORCE)+= ncurses
dir_$(CONFIG_LIB_LIBTERMCAP)      += libtermcap
dir_$(CONFIG_LIB_LIBTERMCAP_FORCE)+= libtermcap

.PHONY: all
all: pre_compile
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	[ -d $(LIBCDIR) ] || mkdir $(LIBCDIR)
	if [ ! -f $(LIBCDIR)/config.cache ]; then \
		cd $(LIBCDIR); \
		export CFLAGS="$(VENDOR_CFLAGS) -DEMBED -O2"; \
		unset LDFLAGS; \
		../configure  --prefix= \
			$(GLIBC_TARGET_OPTS) --build=i386-redhat-linux \
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include \
			--without-cvs --without-gd \
			--disable-profile --disable-force-install \
			--enable-add-ons ; \
	fi
endif
ifdef CONFIG_USE_RSDK_WRAPPER
	[ -d $(ROOTDIR)/lib/librtk ] || mkdir -p $(ROOTDIR)/lib/librtk
	[ -d $(ROOTDIR)/include/rtk ] || mkdir -p $(ROOTDIR)/include/rtk
  ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/options.h $(ROOTDIR)/include/rtk/options.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/mib.h $(ROOTDIR)/include/rtk/mib.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/mibtbl.h $(ROOTDIR)/include/rtk/mibtbl.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/sysconfig.h $(ROOTDIR)/include/rtk/sysconfig.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/utility.h $(ROOTDIR)/include/rtk/utility.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/adslif.h $(ROOTDIR)/include/rtk/adslif.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/adsl_drv.h $(ROOTDIR)/include/rtk/adsl_drv.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/wireless.h $(ROOTDIR)/include/rtk/wireless.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/LINUX/subr_dhcpv6.h $(ROOTDIR)/include/rtk/subr_dhcpv6.h
	$(LN) -fs $(ROOTDIR)/user/boa/src-e8b-new/msgq.h $(ROOTDIR)/include/rtk/msgq.h
  else
        ifndef CONFIG_E8B
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/options.h $(ROOTDIR)/include/rtk/options.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/mib.h $(ROOTDIR)/include/rtk/mib.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/mibtbl.h $(ROOTDIR)/include/rtk/mibtbl.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/sysconfig.h $(ROOTDIR)/include/rtk/sysconfig.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/utility.h $(ROOTDIR)/include/rtk/utility.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/adslif.h $(ROOTDIR)/include/rtk/adslif.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/adsl_drv.h $(ROOTDIR)/include/rtk/adsl_drv.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/subr_dhcpv6.h $(ROOTDIR)/include/rtk/subr_dhcpv6.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/subr_wlan.h $(ROOTDIR)/include/rtk/subr_wlan.h
        ifdef CONFIG_USER_XDSL_SLAVE
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/subr_nfbi_api.h $(ROOTDIR)/include/rtk/subr_nfbi_api.h
        endif #CONFIG_USER_XDSL_SLAVE
	$(LN) -fs $(ROOTDIR)/user/boa/src/msgq.h $(ROOTDIR)/include/rtk/msgq.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/tz.h $(ROOTDIR)/include/rtk/tz.h
        ifdef CONFIG_LUNA
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/rtusr_rg_api.h $(ROOTDIR)/include/rtk/rtusr_rg_api.h
	$(LN) -fs $(ROOTDIR)/user/boa/src/LINUX/chip_deps.h $(ROOTDIR)/include/rtk/chip_deps.h
        endif
        else
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/options.h $(ROOTDIR)/include/rtk/options.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/mib.h $(ROOTDIR)/include/rtk/mib.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/mibtbl.h $(ROOTDIR)/include/rtk/mibtbl.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/sysconfig.h $(ROOTDIR)/include/rtk/sysconfig.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/utility.h $(ROOTDIR)/include/rtk/utility.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/adslif.h $(ROOTDIR)/include/rtk/adslif.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/adsl_drv.h $(ROOTDIR)/include/rtk/adsl_drv.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/subr_dhcpv6.h $(ROOTDIR)/include/rtk/subr_dhcpv6.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/ipv6_info.h $(ROOTDIR)/include/rtk/ipv6_info.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/subr_wlan.h $(ROOTDIR)/include/rtk/subr_wlan.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/msgq.h $(ROOTDIR)/include/rtk/msgq.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/tz.h $(ROOTDIR)/include/rtk/tz.h
        ifdef CONFIG_LUNA
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/rtusr_rg_api.h $(ROOTDIR)/include/rtk/rtusr_rg_api.h
	$(LN) -fs $(ROOTDIR)/user/boa_e8/src/LINUX/chip_deps.h $(ROOTDIR)/include/rtk/chip_deps.h
        endif
        endif
endif
ifeq ($(CONFIG_MIDDLEWARE),y)
	$(LN) -fs $(ROOTDIR)/user/midware_interface/midwaredefs.h $(ROOTDIR)/include/rtk/midwaredefs.h
endif
endif #CONFIG_USE_RSDK_WRAPPER
	for i in $(dir_y) ; do \
		if [ -d "$$i" ]; then \
			if [ ! -f $$i/Makefile ]; then \
				cd $$i && ./configure --host=mips-linux --target=mips-linux && cd -; \
			fi; \
			make -C $$i; \
		fi \
	done

romfs:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || make -C $$i romfs || exit $$? ; \
	done
	-$(STRIP) $(ROMFSDIR)/lib/*

%_menuconfig:
	[ ! -d "$(@:_menuconfig=)" ] || make -C $(@:_menuconfig=) menuconfig

%_only:
	if [ -d "$(@:_only=)" ]; then \
		if [ ! -f $(@:_only=)/Makefile ]; then \
			cd $(@:_only=) && ./configure --host=mips-linux --target=mips-linux && cd -; \
		fi; \
		make -C $(@:_only=); \
	fi

%_romfs:
	[ ! -d "$(@:_romfs=)" ] || make -C $(@:_romfs=) romfs

%_clean:
	[ ! -d "$(@:_clean=)" ] || make -C $(@:_clean=) clean

clean:
ifdef CONFIG_USE_RSDK_WRAPPER
	-rm -rf $(ROOTDIR)/lib/librtk
endif #CONFIG_USE_RSDK_WRAPPER
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	-rm -rf $(LIBCDIR)
endif
	-for i in $(dir_y) $(dir_n) $(dir_); do \
		[ ! -d $$i ] || make -C $$i clean ; \
	done

pre_compile:
ifeq ($(CONFIG_LIB_LIBSSL),y)
	if [ ! -f libssl/config_done ]; then \
	cd libssl;./Configure no-zlib no-asm no-hw no-dso shared no-comp no-engine no-ssl2 no-ssl3 no-sse2 no-aes no-bf no-cast no-dh  no-dsa no-ec no-ecdh no-ecdsa no-idea no-md2 no-md4 no-mdc2 no-rc2 no-rc5 no-ripemd no-rmd160 no-krb5 uClinux-dist; \
	make depend;touch config_done; \
	fi
endif
	@echo "finished pre_compile"

#cd libssl;./Configure no-zlib no-asm no-hw no-dso no-comp no-engine shared no-ssl2 no-ssl3 no-sse2 no-aes no-bf no-cast no-dh  no-dsa no-ec no-ecdh no-ecdsa no-idea no-md2 no-md4 no-mdc2 no-rc2 no-rc5 no-ripemd no-rmd160 no-krb5  mips-linux;make depend;touch config_done; \
