#
# Makefile for the web server form/asp functions
#  for the Linux OS
#
#
#

all:	compile

WEB_LIB	= ../libwebs.a
ASP_HDR	= ./asp.h
ML_HDR = ./multilang.h
ML_UNIX_HDR = ./multilang_unix.h
ML_FINAL_HDR = ./multilang_final.h

# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

# Digest Access switch
#DASW	= -DDIGEST_ACCESS_SUPPORT

# SSL switches (enable all to activate)
# SSLINC = /RSASSL/library/include
# SSLLIB = /RSASSL/library/lib/libsslc.a
# SSLSW	= -DWEBS_SSL_SUPPORT -I$(SSLINC)
# SSLPATCHFILE = ../websSSL.o

# If-Modified-Support switches (requires math library, libm.a)
# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# IFMODLIB = /usr/lib/libm.a

# Dependencies
DEPEND_FILES = fmget.o fmtcpip.o fmfwall.o fmmgmt.o \
		  fmwan.o fmbridge.o fmroute.o \
		  fmcapture.o \
		  fmping.o fmreboot.o fmmultilang.o \
		  fmigmproxy.o \
		  fmothers.o fmstatus.o fmupnp.o fmdiagtest.o\
		  fmeth2pvc_vlan.o fmqos.o fmacc.o fmacl.o fmdomainblk.o fmurl.o \
		  fmmenucreate.o fmmenucreate_user.o boamain.o fmlink.o fmalgonoff.o fmaddressmap.o multilang.o

obj_y =

obj_$(CONFIG_DEV_xDSL)			+= fmoamlb.o
obj_$(CONFIG_USER_PPTP_CLIENT_PPTP)		+= fmvpn.o
obj_$(CONFIG_USER_L2TPD_L2TPD)			+= fmvpn.o
obj_$(CONFIG_NET_IPIP)				+= fmvpn.o
obj_$(CONFIG_XFRM)				+= fmvpn.o
obj_$(CONFIG_USER_MINIDLNA)			+= fmdms.o
obj_$(CONFIG_USER_CUPS)				+= fmPrtServer.o
obj_$(CONFIG_USER_SAMBA)			+= fmsamba.o
obj_$(CONFIG_USER_PPPOMODEM)			+= fm3g.o
obj_$(CONFIG_GPON_FEATURE)			+= fmstatus_pon.o fmgpon.o
obj_$(CONFIG_EPON_FEATURE)			+= fmstatus_pon.o fmepon.o

obj_$(CONFIG_GPON_FEATURE)			+= fmvlan.o
obj_$(CONFIG_USER_CWMP_TR069)			+= fmtr069.o
obj_$(CONFIG_USER_RADVD)			+= fmradvd.o
obj_$(CONFIG_USER_WIRELESS_TOOLS)		+= fmwlstatus.o fmwlan.o
obj_$(CONFIG_IPV6)				+= fmstatus_ipv6.o fmmldproxy.o fmipv6.o fmdhcpv6.o fmfwall_v6.o
obj_$(CONFIG_USER_IP_QOS_3)			+= fmqos_3.o
obj_$(CONFIG_INIT_SCRIPTS) += fmInitScripts.o
obj_$(CONFIG_USER_DOT1AG_UTILS) += fmdot1ag.o
obj_$(CONFIG_USER_DHCP_SERVER) += fmdhcpd.o fmdhcrelay.o fmdhcpmode.o fmmacbase.o fmiprange.o
ifeq ($(MULTI_LANG_DL),0)
obj_$(CONFIG_USER_BOA_WITH_MULTILANG_EN)	+= multilang_en_EN.o
obj_$(CONFIG_USER_BOA_WITH_MULTILANG_RU)	+= multilang_ru_RU.o
obj_$(CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW)	+= multilang_zh_TW.o
obj_$(CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN)	+= multilang_zh_CN.o
endif
obj_$(CONFIG_APACHE_FELIX_FRAMEWORK)		+= fmosgi.o
obj_$(CONFIG_USER_Y1731)		+= fmethoam.o
obj_$(CONFIG_USER_DDNS)				+= fmddns.o
obj_$(CONFIG_USER_BRIDGE_GROUPING) += fmbridge_grouping.o
DEPEND_FILES += $(obj_y)

RTFLAGS += -DNO_ACTION
WEBFLAG	= -DWEBS -DUEMF -DOS="LINUX" -DLINUX $(WLANSUPP) $(UMSW) $(DASW) \
	$(SSLSW) $(IFMODSW) $(RTFLAGS)
OTHERS	= -DB_STATS -DB_FILL -DDEBUG
#DEBUG	= -g -Wall
#DEBUG	=
DEBUG	= -D_DEBUG_MESSAGE
IFLAGS 	= -I.. -I../..
#LDFLAGS	= $(SSLLIB) $(IFMODLIB)
DLFLAG = -DMULTI_LANG_DL=$(MULTI_LANG_DL)
ASPHDR_FLAG=-E -x c -P


compile: $(WEB_LIB) $(ASP_HDR)

#
#	Build archive of objects
#
$(WEB_LIB): $(DEPEND_FILES)
	$(AR) $(ARFLAGS) $(WEB_LIB) $?

ifeq ($(MULTI_LANG_DL),1)
ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_EN),y)
	$(CC) -c -fPIC multilang_en_EN.c
	$(CC) -shared -o libmultilang_en.so multilang_en_EN.o
	install -m 644 libmultilang_en.so $(ROOTDIR)/lib/librtk/libmultilang_en.so
endif
ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_RU),y)
	$(CC) -c -fPIC multilang_ru_RU.c
	$(CC) -shared -o libmultilang_ru.so multilang_ru_RU.o
	install -m 644 libmultilang_ru.so $(ROOTDIR)/lib/librtk/libmultilang_ru.so
endif
ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW),y)
	$(CC) -c -fPIC multilang_zh_TW.c
	$(CC) -shared -o libmultilang_tw.so multilang_zh_TW.o
	install -m 644 libmultilang_tw.so $(ROOTDIR)/lib/librtk/libmultilang_tw.so
endif
ifeq ($(CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN),y)
	$(CC) -c -fPIC multilang_zh_CN.c
	$(CC) -shared -o libmultilang_cn.so multilang_zh_CN.o
	install -m 644 libmultilang_cn.so $(ROOTDIR)/lib/librtk/libmultilang_cn.so
endif
endif

$(ASP_HDR): $(ML_HDR)
	rm -f $(ML_UNIX_HDR) $(ML_FINAL_HDR)
	sed 's/^M$$//' $(ML_HDR) > $(ML_UNIX_HDR)
	$(CC) $(ASPHDR_FLAG) $(ML_UNIX_HDR) > $(ML_FINAL_HDR)
	./gen_asp_hdr.sh

#
#	Primary link
#

romfs:

clean:
	rm -f $(WEB_LIB)
	rm -f $(ASP_HDR) $(ML_UNIX_HDR) $(ML_FINAL_HDR) 

cleanall:
	rm -f *.o

#
#	Dependencies
#

$(DEPEND_FILES): ../options.h

%.o: %.c $(ROOTDIR)/include/linux/autoconf.h $(ROOTDIR)/config/autoconf.h
	$(CC) -c -o $@ $(DEBUG) $(FLASH) $(CFLAGS) $(WEBFLAG) $(IFLAGS) $(DLFLAG) $<

