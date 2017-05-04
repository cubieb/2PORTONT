
.EXPORT_ALL_VARIABLES:

ifdef CONFIG_IPV6
CFLAGS += -DWITH_IPV6
endif
CFLAGS += -DWITH_CASEINSENSITIVETAGS

CWMP_ROOT=$(shell pwd)
GSOAP_DIR=gsoaplib
LIBCWMP_DIR=libcwmp
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL_NEW),y)
MATRIXSSL_DIR=matrixssl-3-6-2-open
LIBMATRIXSSL_INC_DIR=$(CWMP_ROOT)/$(MATRIXSSL_DIR)/matrixssl
else
MATRIXSSL_DIR=matrixssl-1-8-3-open
LIBMATRIXSSL_INC_DIR=$(CWMP_ROOT)/$(MATRIXSSL_DIR)
endif
LIBOPENSSL_INC_DIR=$(CWMP_ROOT)/../../lib/libssl/include
# for released version; we should have lib_default and lib_e8 for released version
# The lib should be linked to proper lib$(LIB_SUFFIX).
ifeq ($(CONFIG_ARCH_CPU_RLX5281),y)
LIB_SUFFIX=5281
ifeq ($(CONFIG_E8B),y)
LIB_SUFFIX=5281_e8
endif
endif
ifeq ($(CONFIG_ARCH_CPU_RLX4281),y)
LIB_SUFFIX=4281
ifeq ($(CONFIG_E8B),y)
LIB_SUFFIX=4281_e8
endif
endif

CC += -Os

#For TR-181 Device.SupportedDataModel.{i}
ifeq ($(CONFIG_CWMP_TR181_SUPPORT),y)
DT_DIR = cwmpClient/dt_doc/
DT_FILES += rtk-xdsl-device-base.xml
ifeq ($(CONFIG_DEV_xDSL),y)
ifeq ($(CONFIG_VDSL),y)
DT_FILES += rtk-xdsl-device-xdsl.xml
else
DT_FILES += rtk-xdsl-device-adsl.xml
endif
endif
ifeq ($(CONFIG_PTMWAN),y)
DT_FILES += rtk-xdsl-device-ptm.xml
endif
DT_FILES += rtk-xdsl-device-traceroute.xml
ifeq ($(CONFIG_USER_WIRELESS_TOOLS),y)
DT_FILES += rtk-xdsl-device-wlan.xml
endif
DT_FPATH = $(addprefix $(DT_DIR),$(DT_FILES))
endif #CONFIG_CWMP_TR181_SUPPORT

all:
#	if [ -d $(MATRIXSSL_DIR) ]; then \
#		$(MAKE) -C $(MATRIXSSL_DIR)/src; \
#		if [ ! -d lib ]; then mkdir lib; fi; \
#		cp -f $(MATRIXSSL_DIR)/src/libmatrixsslstatic.a $(CWMP_ROOT)/lib/; \
#	fi
#	$(MAKE) -C matrixssl-1-8-3-open/examples
	if [ -d lib_$(LIB_SUFFIX) ]; then \
		rm -rf lib; \
		ln -s lib_$(LIB_SUFFIX) lib; \
	fi
	if [ -d $(GSOAP_DIR) ]; then \
		cd $(GSOAP_DIR); \
		$(MAKE); \
		$(MAKE) install; \
	fi
	if [ -d $(LIBCWMP_DIR) ]; then \
		cd $(LIBCWMP_DIR); \
		$(MAKE); \
		$(MAKE) install; \
	fi
# To support TR104
ifeq ($(CONFIG_USER_TR104),y)
#	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/tr104/
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/rtk_tr104/
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/voip_manager/
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/flash/
endif
	$(MAKE) -C cwmpClient

romfs:
	cp -f $(CWMP_ROOT)/cwmpClient/cwmpClient $(CWMP_ROOT)/../../romfs/bin
ifeq ($(CONFIG_USER_TR143),y)
	cp -f $(CWMP_ROOT)/cwmpClient/udpechoserver $(CWMP_ROOT)/../../romfs/bin
endif
ifeq ($(CONFIG_USER_CWMP_WITH_SSL),y)
	cp -f $(CWMP_ROOT)/cwmpClient/client.pem $(CWMP_ROOT)/../../romfs/etc
	cp -f $(CWMP_ROOT)/cwmpClient/cacert.pem $(CWMP_ROOT)/../../romfs/etc
endif
ifeq ($(CONFIG_CWMP_TR181_SUPPORT), y)
	mkdir -p $(CWMP_ROOT)/../../romfs/etc/dt_doc
	cp -f $(DT_FPATH) $(CWMP_ROOT)/../../romfs/etc/dt_doc
endif
	$(ROMFSINST) setprmt_reject /etc/setprmt_reject

clean:
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL_NEW),y)
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR) clean; \
	fi
else
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR)/src clean; \
	fi
endif
	if [ -d $(GSOAP_DIR) ]; then \
		$(MAKE) -C $(GSOAP_DIR) clean; \
	fi
	if [ -d $(LIBCWMP_DIR) ]; then \
		$(MAKE) -C $(LIBCWMP_DIR) clean; \
	fi
	$(MAKE) -C cwmpClient clean
ifeq ($(CONFIG_USER_TR104),y)
	$(MAKE) -C $(CWMP_ROOT)/../rtk_voip/rtk_tr104 clean
endif
	if [ -d $(LIBCWMP_DIR) -a -d include ]; then \
		rm -rf include; \
		rm -rf lib; \
	fi

