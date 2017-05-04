#APOLLO_SDK_PATH = ../../vendors/Realtek/apollo/sdk
#APOLLO_SDK_PATH = $(SDKDIR)
#ccwei add
# Mason Yu. patch_apollo
APOLLO_SDK_PATH = $(APOLLODIR)/sdk
CFLAGS += -include $(ROOTDIR)/include/linux/autoconf.h
include $(APOLLODIR)/Compiler_Flag
#CONFIG_FPGA_MODE=n
#CONFIG_VIRTUAL_DRV_MODE=n
CLI_PATH:= $(APOLLO_SDK_PATH)/src/app/diag_rtk/config
CPARSER_FILE:=$(APOLLO_SDK_PATH)/src/app/diag_rtk/src/parser/cparser_tree.c
diag_cli=$(wildcard $(CLI_PATH)/*.cli)
#export CONFIG_FPGA_MODE CONFIG_VIRTUAL_DRV_MODE



all: $(CPARSER_FILE)
	$(MAKE) -C $(APOLLO_SDK_PATH)/src all
#	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest all
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app

diag: check_tree
	$(MAKE) -C $(APOLLO_SDK_PATH)/src all
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app


$(CPARSER_FILE): $(diag_cli)
	$(MAKE) shell_tree;

clean:

	$(MAKE) -C $(APOLLO_SDK_PATH)/src clean
	$(MAKE) -C $(APOLLO_SDK_PATH)/unittest clean
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app clean

#Mason Yu. patch_apollo
shell_tree:
	$(MAKE) -C $(APOLLO_SDK_PATH) shell_tree
	
romfs:
	$(MAKE) -C $(APOLLO_SDK_PATH)/src/app install
	cp -af $(APOLLO_SDK_PATH)/src/app/bin/* $(ROMFSDIR)/bin/
