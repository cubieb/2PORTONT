#
# Copyright (C) 2014 Realtek Semiconductor Corp. 
# All Rights Reserved.
#
# This program is the proprietary software of Realtek Semiconductor
# Corporation and/or its licensors, and only be used, duplicated, 
# modified or distributed under the authorized license from Realtek. 
#
# ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
# THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
#
# Purpose : Compile procedure of OMCI
#
# Feature : The file includes the following modules and sub-modules
#           (1) Makefile
#

RELEASE_VERSION=1.8.1.0.1.8.1.0.1.8.1.0

MIB_DIR  		= MIB
FEATURE_DIR     = FAL/src/features
BIN_LIB_TABLES_DIR	:= LIB/omci
DRV_RG_DIR		= DRV/platform/rtl9607_rg
# library
LIB_GOS			= omci_gos
BIN_LIB_GOS		= lib$(LIB_GOS).so
LIB_MIB			= omci_mib
BIN_LIB_MIB		= lib$(LIB_MIB).so
LIB_OMCI_API            = omci_api
BIN_LIB_OMCI_API        = lib$(LIB_OMCI_API).so
LIB_FAL         = omci_fal
BIN_LIB_FAL     = lib$(LIB_FAL).so

# library list

# binary
BIN_OMCI_APP	= omci_app
BIN_OMCI_CLI	= omcicli
BIN_OMCI_DRV	= omcidrv.ko
BIN_PF_RTK		= pf_rtk.ko
BIN_PF_RG		= pf_rg.ko

# initial script
RUN_OMCI_SCRIPT = "runomci.sh"

all: clean TGT_DRV_RG
	if [ -f "$(BIN_LIB_GOS).$(RELEASE_VERSION)" ] ; then \
		find $(MIB_DIR)/src/tables/ -name "*.$(RELEASE_VERSION)" | xargs -i cp {} $(BIN_LIB_TABLES_DIR) ;\
		cp -af $(BIN_LIB_GOS).$(RELEASE_VERSION) LIB/$(BIN_LIB_GOS); \
		cp -af $(BIN_LIB_MIB).$(RELEASE_VERSION) LIB/$(BIN_LIB_MIB); \
		cp -af $(BIN_LIB_FAL).$(RELEASE_VERSION) LIB/$(BIN_LIB_FAL); \
		cp -af $(BIN_LIB_OMCI_API).$(RELEASE_VERSION) LIB/$(BIN_LIB_OMCI_API); \
		cp -af LIB/$(BIN_LIB_FAL) $(APOLLODIR)/object/src/app/lib/; \
		cp -af LIB/$(BIN_LIB_OMCI_API) $(APOLLODIR)/object/src/app/lib/; \
		find $(BIN_LIB_TABLES_DIR)/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv ;\
		find $(FEATURE_DIR)/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 cp ;\
	fi

TGT_DRV_RG:
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
	if [ -f "${DRV_RG_DIR}/omci_pf_rtl9607.c" ]; then \
		$(MAKE) -C $(DRV_RG_DIR) all; \
		cp -af ${DRV_RG_DIR}/$(BIN_PF_RG) ${DRV_RG_DIR}/$(BIN_PF_RG)_$(RELEASE_VERSION); \
		$(MAKE) -C $(DRV_RG_DIR) clean; \
	fi
endif

	
clean:
	rm -rf LIB/*.so.* $(BIN_LIB_TABLES_DIR)/*.so.*
	rm -rf LIB/*.so $(BIN_LIB_TABLES_DIR)/*.so
	find $(FEATURE_DIR)/ -name "*.so" | xargs --no-run-if-empty rm -rf
	rm -rf $(APOLLODIR)/object/src/app/lib/$(BIN_LIB_FAL) $(APOLLODIR)/object/src/app/lib/$(BIN_LIB_OMCI_API)

install:
	if [ -f "$(BIN_OMCI_APP).$(RELEASE_VERSION)" ]; then \
		cp -af $(BIN_OMCI_APP).$(RELEASE_VERSION) $(BINDIR)/$(BIN_OMCI_APP); \
		cp -af $(BIN_OMCI_CLI).$(RELEASE_VERSION) $(BINDIR)/$(BIN_OMCI_CLI); \
	fi
	$(STRIP) $(BIN_LIB_TABLES_DIR)/*.so
	cp -af LIB/* $(ROMFSDIR)/lib/
	@if [ ! -d $(ROMFSDIR)/lib/features/internal ]; then \
        mkdir -p $(ROMFSDIR)/lib/features/internal; \
    fi
	find $(FEATURE_DIR) -name "*.so" | xargs --no-run-if-empty -i cp {} $(ROMFSDIR)/lib/features/internal/
	@if [ ! -d $(ROMFSDIR)/lib/modules/features ] ; then \
		mkdir -p $(ROMFSDIR)/lib/modules/features; \
	fi
	if [ -f DRV/*.ko_$(RELEASE_VERSION) ]; then \
		cp -af DRV/*.ko_$(RELEASE_VERSION) $(ROMFSDIR)/lib/modules/$(BIN_OMCI_DRV); \
		cp -af DRV/platform/rtl9607_rg/*.ko_$(RELEASE_VERSION) $(ROMFSDIR)/lib/modules/$(BIN_PF_RG); \
		cp -af DRV/platform/rtl9607_rtk/*.ko_$(RELEASE_VERSION)  $(ROMFSDIR)/lib/modules/$(BIN_PF_RTK); \
	fi
	find DRV/features/ -name "*.ko_$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 cp
	find DRV/features/ -name "*.ko" | xargs --no-run-if-empty -i cp {} $(ROMFSDIR)/lib/modules/features/
	cp -af $(RUN_OMCI_SCRIPT)  $(ROMFSDIR)/etc/
