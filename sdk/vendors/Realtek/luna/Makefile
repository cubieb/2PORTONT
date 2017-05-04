#
#	Makefile -- Build instructions for Realtek/luna
#
ifdef CONFIG_BOA_WEB_E8B_CH
CONFIGPATH=$(shell pwd)/e8b
else
CONFIGPATH=$(shell pwd)
endif
include $(ROOTDIR)/$(LINUXDIR)/.config
ROMFSIMG = $(IMAGEDIR)/romfs.img
IMAGE    = $(IMAGEDIR)/image.bin
ifdef CONFIG_ROMFS_FS
	DEVCMD = for i in $(DEVICES); do \
			touch $(ROMFSDIR)/dev/@$$i; \
		done
	ROMFS_DIRS = bin dev etc home lib mnt proc usr var etc/scripts 
else
	DEVCMD =
	ROMFS_DIRS = bin etc home lib mnt proc usr var etc/scripts 

ifdef CONFIG_USER_FLATFSD_XXX
	ROMFS_DIRS += etc/default
endif

endif

ifeq ($(CONFIG_RTL8686_IPC_DEV),y)
ROMFS_DIRS = bin dev etc home lib proc usr var etc/scripts usr/local image 
else
ROMFS_DIRS = bin dev etc home lib proc usr var etc/scripts usr/local
endif

ROMFS_DIRS += sys

DEVICES = \
	tty,c,5,0      console,c,5,1      cua0,c,5,64      cua1,c,5,65  \
	\
	mem,c,1,1      kmem,c,1,2         null,c,1,3       ram0,b,1,0 \
	ram1,b,1,1 \
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	rom0,b,31,0    rom1,b,31,1        rom2,b,31,2      rom3,b,31,3 \
	rom4,b,31,4    rom5,b,31,5        rom6,b,31,6      rom7,b,31,7 \
	rom8,b,31,8    rom9,b,31,9 \
	\
	tty0,c,4,0     tty1,c,4,1         tty2,c,4,2       tty3,c,4,3 \
	ttyS0,c,4,64   ttyS1,c,4,65 \
	\
	ttyp0,c,3,0    ttyp1,c,3,1        ttyp2,c,3,2      ttyp3,c,3,3 \
	ttyp4,c,3,4    ttyp5,c,3,5        ttyp6,c,3,6      ttyp7,c,3,7 \
	ttyp8,c,3,8    ttyp9,c,3,9        ttypa,c,3,10     ttypb,c,3,11 \
	ttypc,c,3,12   ttypd,c,3,13       ttype,c,3,14     ttypf,c,3,15 \
	\
	mtd0,c,90,0    mtd1,c,90,1        mtd2,c,90,2      mtd3,c,90,3 \
	\
	zero,c,1,5     random,c,1,8       urandom,c,1,9    ppp,c,108,0 \
	\
	adsl0,c,100,0  wlchr,c,13,0 \
	\
	aipc_dev,c,254,0  dsp_console,c,253,0



clean:


mkromfsdir:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(DEVCMD)
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /bin /sbin
	$(ROMFSINST) -s /var/ppp /etc/ppp
	$(ROMFSINST) -s /var/config /etc/config
	$(ROMFSINST) -s /var/passwd /etc/passwd
	$(ROMFSINST) -s /var/TZ /etc/TZ
ifdef CONFIG_E8B
	$(ROMFSINST) -s /var/config /usr/local/ct
endif
	$(ROMFSINST) mdev.conf /etc/mdev.conf
	$(ROMFSINST) /etc/inittab
ifeq ($(CONFIG_USER_LOGIN_LOGIN),y)
	echo "ttyS0::respawn:/bin/login -p" >> $(ROMFSDIR)/etc/inittab
else
	echo "ttyS0::askfirst:-/bin/sh" >> $(ROMFSDIR)/etc/inittab
endif
	$(ROMFSINST) /etc/shells
ifeq ($(CONFIG_USER_UDHCP099PRE2),y)
	$(ROMFSINST) scripts/udhcpc2.sh /etc/scripts/udhcpc.sh
else
	$(ROMFSINST) scripts/udhcpc.sh /etc/scripts/udhcpc.sh
endif
	$(ROMFSINST) scripts/udhcpc.deconfig /etc/scripts/udhcpc.deconfig
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
#luke: for rg callback we should use these scripts!
	$(ROMFSINST) scripts/udhcpc_rgcb.sh /etc/scripts/udhcpc_rgcb.sh
	$(ROMFSINST) scripts/udhcpc_rgcb.deconfig /etc/scripts/udhcpc_rgcb.deconfig
endif
	$(ROMFSINST) services /etc/services
	case "$(LINUXDIR)" in \
	*2.4.*) ;; \
	*2.6.*) ;; \
	*3.18.*) ;; \
	*) echo "ttyS0:linux:/bin/sh" >> $(ROMFSDIR)/etc/inittab ;; \
	esac
	$(ROMFSINST) /etc/motd
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
ifdef CONFIG_DEFAULTS_KERNEL_2_6
	$(ROMFSINST) ethertypes /etc/ethertypes
endif
	$(ROMFSINST) /etc/profile
ifneq ("$(CONFIG_KERNEL_2_6_30)$(CONFIG_DEFAULTS_KERNEL_3_18)","")
	cat squashfs-pf.in > squashfs-pf.list
	mkdir -p $(ROMFSDIR)/dev/misc
	mkdir -p $(ROMFSDIR)/dev/voip
	$(ROMFSINST) -s /tmp/log /dev/log
endif	

.PHONY: rcX
rcX:
	mkdir -p $(ROMFSDIR)/etc/init.d
	$(ROMFSINST) /etc/init.d/rcS

dev:
ifdef CONFIG_SQUASHFS
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	[ -d $(ROMFSDIR)/dev ] || mkdir -p $(ROMFSDIR)/dev
	rm $(ROMFSDIR)/dev/*
	for i in $(DEVICES); do \
		name=`echo $$i |cut -d"," -f1`;\
		type=`echo $$i |cut -d"," -f2`;\
		major=`echo $$i |cut -d"," -f3`;\
		minor=`echo $$i |cut -d"," -f4`;\
		mknod -m644 $(ROMFSDIR)/dev/$$name $$type $$major $$minor;\
	done
endif

image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.romvec \
			--remove-section=.text --remove-section=.ramvec \
			--remove-secti/on=.init \
			--remove-section=.bss --remove-section=.eram \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.data
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.ramvec \
			--remove-section=.bss --remove-section=.data \
			--remove-section=.eram \
			--set-section-flags=.romvec=CONTENTS,ALLOC,LOAD,READONLY,CODE \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.text
	cat $(IMAGEDIR)/linux.text $(IMAGEDIR)/linux.data $(ROMFSIMG) > $(IMAGE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot

