.EXPORT_ALL_VARIABLES:

CXXFLAGS += $(INCSG)
CFLAGS += $(INCSG) -DSERVER

ifdef CONFIG_USER_REMOTE_MANAGEMENT
CFLAGS += -DCONFIG_USER_SNMPD_MODULE_RM
endif

DIRS = snmplib modules snmpd

all romfs test:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

