#
#	Makefile -- Build instructions for user2 level apps
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#


-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(LINUX_CONFIG)

CFLAGS += -include $(ROOTDIR)/autoconf.h
CFLAGS += -include $(ROOTDIR)/include/linux/autoconf.h

dir_y = proc_var

dir_$(CONFIG_USER_GDB_GDBSERVER)            += gdbserver
dir_$(CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT) += rtk_nv


all:
	for i in $(dir_y) ; do \
		if [ ! -f $$i/Makefile ]; then \
			cd $$i && ./configure --host=mips-linux --target=mips-linux && cd -; \
		fi; \
		make -C $$i || exit $?; \
	done	


romfs: 
	for i in $(dir_y) ; do make -C $$i romfs || exit $? ; done


.PHONY: rcX
rcX: 
	for i in $(dir_y) ; do \
		if [ "`grep "^rcX:" $$i/Makefile`" != "" ]; then \
			make -C $$i rcX || exit $? ; \
		fi \
	done	


clean:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || make -C $$i clean; \
	done 





