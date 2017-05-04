include ../../../config/.config

#add flag for WLAN MIB set support
ifdef CONFIG_USER_WIRELESS_TOOLS
#DWDS=-DWLAN_WDS
#DACL=-DWLAN_ACL
#WIFI=-DWIFI_TEST
WLANSUPP= -DWLAN_SUPPORT -DWLAN_WPA $(DWDS) -DWLAN_8185AG $(DACL) $(WIFI) -DWLAN_WDS -DWLAN_1x
ifdef CONFIG_USER_WIRELESS_TOOLS_RTL8185_IAPP
WLANSUPP += -DWLAN_IAPP
endif
endif

OBJS = ethwan_vars.o icmp_vars.o ip_vars.o tcp_vars.o udp_vars.o systm.o iface_vars.o mib_tool.o atm_vars.o

ifdef CONFIG_DEV_xDSL
OBJS += adsl_vars.o adsl2_vars.o
endif

ifdef CONFIG_USER_WIRELESS_TOOLS
OBJS += wlan_vars.o
endif

ifdef CONFIG_USER_REMOTE_MANAGEMENT
OBJS+= rm_sysinfo_vars.o rm_system_vars.o atmvcl_vars.o rm_pvc_vars.o rm_ppp_vars.o ppp_security_secret_vars.o rm_ip_vars.o rm_dhcp_vars.o rm_dns_vars.o rm_ping_vars.o rm_ftp_vars.o rm_adslline.o
endif

ifdef CONFIG_RTL8686_SWITCH
OBJS += nscrt_vars.o
endif

LIBRARY = libmodules.a
CFLAGS += -I../snmplib $(WLANSUPP)
RANLIB = ranlib

all: $(LIBRARY)

test:

romfs:

$(LIBRARY): $(OBJS)
	$(AR) rcv $@ $(OBJS)
	-$(RANLIB) $@

clean::
	$(RM) $(OBJS) $(LIBRARY)
