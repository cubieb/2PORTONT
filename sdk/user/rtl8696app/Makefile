## include $(CONFIG_CONFIG)

APPOLO_DRIVER_DIR=$(ROOTDIR)/$(LINUXDIR)/drivers/net/rtl8686_switch/sdk
APOLLODIR=$(shell pwd)
SDKDIR = $(APOLLODIR)/sdk/
export SDKDIR APOLLODIR

TARGETS = 
ifdef CONFIG_USER_APOLLO_DIAGSHELL
TARGETS += diag
endif

.PHONY: prepare
all: prepare lib $(TARGETS)		
	set
	
prepare:
	if [ ! -d sdk ]; then \
		find $(APPOLO_DRIVER_DIR) -type d -printf "mkdir -p sdk/%P\n" | sh ; \
		find $(APPOLO_DRIVER_DIR) -type f -a \( -name '*.c' -o -name '*.h' -o -name 'Makefile*' \) -printf "ln -fs $(APPOLO_DRIVER_DIR)/%P sdk/%P\n" | sh; \
		find $(APPOLO_DRIVER_DIR) -type f -a \( -name '*.cli' -o -name '*.py' \) -printf "ln -fs $(APPOLO_DRIVER_DIR)/%P sdk/%P\n" | sh; \
	fi		
	
lib: 
	$(MAKE) -C sdk -f build/Makefile.linux.user.rtk
	
diag:
	$(MAKE) -C sdk/unittest
	$(MAKE) -C sdk/src/app
			
romfs:
ifdef CONFIG_USER_APOLLO_DIAGSHELL
	$(ROMFSINST) sdk/src/app/bin/diag /bin/
endif	
	
clean:
	-rm -rf sdk object

