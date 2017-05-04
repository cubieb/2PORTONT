.EXPORT_ALL_VARIABLES:
SRCNAME = src-project
DIRS = $(SRCNAME)
RC_LEVEL_E8B=/etc/init.d/rc8
RC_LEVEL_FLASH_CHK=/etc/init.d/rc18
RC_LEVEL_CONFIGD=/etc/init.d/rc18
ifeq ($(CONFIG_RTK_OAM_V1),y)
RC_LEVEL_STARTUP=/etc/init.d/rc32
else
RC_LEVEL_STARTUP=/etc/init.d/rc22
endif
RC_LEVEL_BOA=/etc/init.d/rc34

#
# to add a new branch, please define new ID, SRCORIGNAME, and WEBNAME, all enclosed by ifeq
#
ID = default_web_id
SRCORIGNAME = src
WEBNAME = web_default
ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
ID = config_boa_web_e8b_ch_id
SRCORIGNAME = src-e8b-new
WEBNAME = web_e8b_ch 
endif

ifndef UCLINUX_BUILD_USER
include ../../config/.config
ifdef ROOTDIR
include $(ROOTDIR)/$(LINUXDIR)/.config
endif
endif

all romfs:
ifneq ($(ID),$(wildcard $(ID)))
	# remove symbolic links
	rm -rf $(SRCNAME)/web
	rm -rf $(SRCNAME)

	# add new symbolic links
	ln -s $(SRCORIGNAME) $(SRCNAME)
	cd $(SRCNAME); ln -s $(WEBNAME) web; cd ..

	make -C $(SRCNAME) clean

	# remove old IDs and touch the new ID
	rm -f *_id
	touch $(ID)
endif
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

.PHONY: rcX
rcX:
ifdef CONFIG_JFFS2_FS
ifdef CONFIG_BOA_WEB_E8B_CH
	$(ROMFSINST) -a "mount -t jffs2 /dev/mtdblock7 /usr/local/ct" $(RC_LEVEL_E8B)
endif
endif
ifdef CONFIG_BOA_WEB_E8B_CH
	$(ROMFSINST) -a "echo 200 > /proc/sys/vm/dirty_writeback_centisecs" $(RC_LEVEL_E8B)
	$(ROMFSINST) -a "echo 200 > /proc/sys/vm/dirty_expire_centisecs" $(RC_LEVEL_E8B)
endif
ifneq ($(CONFIG_USER_XMLCONFIG),y)
	$(ROMFSINST) -a "/bin/flash check" $(RC_LEVEL_FLASH_CHK)
endif

ifdef CONFIG_LUNA	
	$(ROMFSINST) -a "/etc/scripts/reset_default.sh" $(RC_LEVEL_CONFIGD)
endif
	$(ROMFSINST) -a "configd&" $(RC_LEVEL_CONFIGD)
ifdef CONFIG_E8B
	$(ROMFSINST) -a "cp /bin/ctadmin /usr/local/ct" $(RC_LEVEL_STARTUP)
endif
	$(ROMFSINST) -a "startup&" $(RC_LEVEL_STARTUP)



clean:
	# remove old IDs
	rm -f *_id;
	for i in $(DIRS) ; do make -C $$i clean ; done

