#
# This file contains rules which are shared between multiple Audiocodes MW Makefiles.
#

ifeq "$(ACMW_LX)" "y"
ACLFLAGS += -DACMW_LX
endif

ifeq "$(AUDIOCODES_VOIP)" "y"
ACLFLAGS += -DAUDIOCODES_VOIP
endif

ifeq "$(ACL_BYPASS)" "y"
ACLFLAGS += -DACL_BYPASS
endif

ifeq "$(ACL_IBSGEN)" "y"
ACLFLAGS += -DACL_IBSGEN
endif

ifeq "$(ACL_IBSDET)" "y"
ACLFLAGS += -DACL_IBSDET
endif

ifeq "$(ACL_G729)" "y"
ACLFLAGS += -DACL_G729
endif

ifeq "$(ACL_G723)" "y"
ACLFLAGS += -DACL_G723
endif

ifeq "$(ACL_G711_PLC_ENABLE)" "y"
ACLFLAGS += -DACL_G711_PLC_ENABLE
endif

ifeq "$(ACL_G711_CNG_ENABLE)" "y"
ACLFLAGS += -DACL_G711_CNG_ENABLE
endif


ifeq "$(ACL_LEC3)" "y"
ACLFLAGS += -DACL_LEC3
endif

ifeq "$(ACL_3WCONF)" "y"
ACLFLAGS += -DACL_3WCONF
endif

ifeq "$(ACL_RELAY)" "y"
ACLFLAGS += -DACL_RELAY
endif

ifeq "$(ACL_ILBC)" "y"
ACLFLAGS += -DACL_ILBC
endif

ifeq "$(ACL_GSMFR)" "y"
ACLFLAGS += -DACL_GSMFR
endif

ifeq "$(ACL_AGC)" "y"
ACLFLAGS += -DACL_AGC
endif

ifeq "$(ACL_PLAYBACK)" "y"
ACLFLAGS += -DACL_PLAYBACK
endif

ifeq "$(ACL_CID)" "y"
ACLFLAGS += -DACL_CID
endif


ifeq "$(ACL_RAM_LOGGER)" "y"
ACLFLAGS += -DACL_RAM_LOGGER
endif

ifeq "$(AUDIOCODES_4W_IF)" "y"
ACLFLAGS += -DAUDIOCODES_4W_IF
endif

ifeq "$(ACL_DEBUG)" "y"
ACLFLAGS += -DACMW_DEBUG_SUPPORT
endif

ifeq "$(ACL_PROFILER_STATUS)" "y"
ACLFLAGS += -DACL_PROFILER_STATUS
endif

ifeq "$(ACL_PROFILER)" "y"
ACLFLAGS += -DACL_PROFILER
endif

ifeq "$(ACL_PROFILER_G729)" "y"
ACLFLAGS += -DACL_PROFILER_G729
endif

ifeq "$(ACL_PROFILER_G711)" "y"
ACLFLAGS += -DACL_PROFILER_G711
endif

ifeq "$(ACL_PROFILER_G723)" "y"
ACLFLAGS += -DACL_PROFILER_G723
endif

ifeq "$(ACL_PROFILER_LEC3)" "y"
ACLFLAGS += -DACL_PROFILER_LEC3
endif

ifeq "$(ACL_PROFILER_IBS_GEN)" "y"
ACLFLAGS += -DACL_PROFILER_IBS_GEN
endif

ifeq "$(ACL_PROFILER_IBS_DET)" "y"
ACLFLAGS += -DACL_PROFILER_IBS_DET
endif

ifeq "$(ACL_PROFILER_RELAY)" "y"
ACLFLAGS += -DACL_PROFILER_RELAY
endif

ifeq "$(ACL_PROFILER_ILBC)" "y"
ACLFLAGS += -DACL_PROFILER_ILBC
endif

ifeq "$(ACL_PROFILER_GSMFR)" "y"
ACLFLAGS += -DACL_PROFILER_GSMFR
endif


ifeq "$(ACL_RTL8186VA)" "y"
ACLFLAGS += -DACL_RTL8186VA
endif

ifeq "$(ACL_RTL8651BV)" "y"
ACLFLAGS += -DACL_RTL8651BV
endif

ifeq "$(ACL_RTL8651C)" "y"
ACLFLAGS += -DACL_RTL8651C
endif

ifeq "$(ACL_RTL8671VA)" "y"
ACLFLAGS += -DACL_RTL8671VA
endif


ifeq "$(ACL_LINUX_2_4_18)" "y"
ACLFLAGS += -DACL_LINUX_2_4_18
endif

ifeq "$(ACL_UCLINUX_2_4_X)" "y"
ACLFLAGS += -DACL_UCLINUX_2_4_X
endif


#ACLFLAGS += -DFEATURE_G723VERIFY
#ACLFLAGS += -DFEATURE_G729VERIFY
