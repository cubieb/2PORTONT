# makefile for wscd

# CC			= mips-uclibc-gcc
COPT		= -g -O2
CDEF		= 
CWARN		= -Wall
#CDBG		= -ggdb
#CFLAGS		= ${CWARN} ${COPT} ${CDEF} ${CDBG}
#LDFLAGS		= --static -Wl,--gc-sections
#LDFLAGS		=

ifeq ($(RTL8186_KB),1)
	LIBUPNP=1
endif

ifeq ($(OFFICIAL_OPENSSL),1)
########## Build with OPENSSL Library ##################
OPENSSLDIR	= ../../openssl-0.9.7i

else
########## Build with porting-SSL Library ##################
OPENSSLDIR	= ../portingssl
CFLAGS		+= -DUSE_PORTING_OPENSSL
endif


ifeq ($(LIBUPNP),1)
########## Build with libupnp Library ##################
UPNPDIR		= ../../upnpd/libupnp-1.4.1
UPNPINC		= $(UPNPDIR)/upnp/inc
IXMLINC		= $(UPNPDIR)/ixml/inc
UPNPLIB 	= -lpthread $(UPNPDIR)/ixml/.libs/libixml.so.2 \
	$(UPNPDIR)/threadutil/.libs/libthreadutil.so.2 \
	$(UPNPDIR)/upnp/.libs/libupnp.so.2
SOURCES = wsc.c txpkt.c rxpkt.c utils.c sample_util.c simplecfg_upnp_main.c

else
########## Build with MINI-UPNP Library ##################
UPNPDIR		= ../../mini_upnp
UPNPINC		= $(UPNPDIR)
IXMLINC		= ./
CFLAGS		+= -DUSE_MINI_UPNP
SOURCES 	= wsc.c txpkt.c rxpkt.c utils.c simplecfg_mini_upnp_main.c

	ifeq ($(STATIC_LIB),1)
	################# Static Library ##################
		UPNPLIB		= $(UPNPDIR)/mini_upnp.a
		CFLAGS		+= -DSTAND_ALONE_MINIUPNP

	else
	################# Shared Library ##################
	ifdef CONFIG_USE_RSDK_WRAPPER
	UPNPLIB = -L../../../lib/librtk -lmini_upnp
	else
	UPNPLIB = $(UPNPDIR)/mini_upnp.so
	endif
	endif
endif

ifeq ($(OFFICIAL_OPENSSL),1)
INCS		= -I$(OPENSSLDIR)/include/ -I$(UPNPINC) -I$(IXMLINC)
else
INCS		= -I$(OPENSSLDIR)/crypto/include -I$(UPNPINC) -I$(IXMLINC)
endif

LIBS =  $(OPENSSLDIR)/libcrypto.a 
LIBS += $(UPNPLIB)

ifeq ($(CONFIG_USB_RTL8187SU_SOFTAP),y)
CFLAGS += -DCONFIG_USB_RTL8187SU_SOFTAP
endif

ifeq ($(CONFIG_USB_RTL8192SU_SOFTAP),y)
CFLAGS += -DCONFIG_USB_RTL8192SU_SOFTAP
endif

ifeq ($(CONFIG_MASTER_WLAN0_ENABLE),y)
ifeq ($(CONFIG_SLAVE_WLAN1_ENABLE),y)
CFLAGS += -DFOR_DUAL_BAND
endif
endif

ifeq ($(CONFIG_USE_PCIE_SLOT_0),y)
ifeq ($(CONFIG_USE_PCIE_SLOT_1),y)
CFLAGS += -DFOR_DUAL_BAND
endif
endif

ifeq ($(CONFIG_RTL_92D_DMDP),y)
CFLAGS += -DFOR_DUAL_BAND
endif

ifeq ($(RTL8186_KB),1)
CFLAGS		+= -DCONFIG_RTL8186_KB
endif

ifeq ($(RTL8186_TR),1)
CFLAGS		+= -DCONFIG_RTL8186_TR
endif

ifeq ($(OFFICIAL_OPENSSL),1)
CFLAGS		+= $(INCS)
else
CFLAGS		+= -D__linux__ -DRSDK_BUILT -DB_ENDIAN $(INCS)
endif

ifeq ($(NOIWCTL),1)
CFLAGS		+= -DNO_IWCONTROL
endif

SHELL		= /bin/sh
STRIP		= mips-uclibc-strip
PROGS		= wscd

ifeq ($(SLINK),1)
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif

ifeq ($(CONFIG_RTL_WPS2_SUPPORT),y)
CFLAGS += -DWPS2DOTX
endif

ifeq ($(CONFIG_RTL_WPS_VERSION_USER_CONFIG),y)
CFLAGS += -DWPS_VERSION_CONFIGURABLE
endif

all:	BUILT_TIME $(PROGS)

clean:
	rm -f $(PROGS) core tags ID *.o *.d *~ *.bak built_time

sources = $(SOURCES)

#include $(sources:.c=.d)

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time	

$(PROGS):	$(sources:.c=.o)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS) -L../../boa/src-project/LINUX $(LIBMIB) $(LIBVOIP)
	#$(STRIP) $@

%.d: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
		      | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		      [ -s $@ ] || rm -f $@'
