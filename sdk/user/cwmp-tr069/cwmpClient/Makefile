CWMPCLIENT_ROOT=$(shell pwd)

ifeq ($(CONFIG_USER_TR143),y)
TR143_EXEC = udpechoserver
TR143_LIBOBJS = udpechoserverlib.o
TR143_LIBDEPH = udpechoserverlib.h
else
TR143_EXEC =
TR143_LIBOBJS =
TR143_LIBDEPH =
endif

EXEC = cwmpClient
OBJS =	prmt_igd.o prmt_deviceinfo.o prmt_mngmtserver.o \
	prmt_layer3fw.o prmt_landevice.o prmt_landevice_eth.o prmt_landevice_wlan.o \
	prmt_wancondevice.o prmt_ippingdiag.o \
	prmt_time.o prmt_ddns.o \
	prmt_services.o prmt_userif.o \
	prmt_captiveportal.o prmt_traceroute.o prmt_landevice_usb.o \
	prmt_tr143.o prmt_apply.o porting.o main.o \
	prmt_wandevice.o prmt_lanconfsec.o prmt_nslookup.o\
	cwmpc_utility.o
TR181_OBJS = tr181/prmt_device2.o tr181/prmt_deviceinfo.o tr181/prmt_dns.o \
	tr181/prmt_nat.o \
	tr181/prmt_routing.o tr181/prmt_ifstack.o \
	tr181/prmt_ether.o tr181/prmt_ether_if.o tr181/prmt_ether_link.o \
	tr181/prmt_wifi.o tr181/prmt_wifi_radio.o tr181/prmt_wifi_ssid.o \
	tr181/prmt_wifi_ap.o \
	tr181/prmt_bridge.o tr181/prmt_bridge_port.o tr181/prmt_ppp.o \
	tr181/prmt_ip.o tr181/prmt_ip_if.o tr181/prmt_ip_diag.o

ifeq ($(CONFIG_USER_DHCP_SERVER), y)
TR181_OBJS += tr181/prmt_dhcpv4.o tr181/prmt_dhcpv4_client.o tr181/prmt_dhcpv4_server.o \
	tr181/prmt_dhcpv4_relay.o
endif

#QoS v1
ifeq ($(IP_QOS), y)
OBJS += prmt_queuemnt.o
endif

#QoS v3
ifneq ($(CONFIG_E8B), y)
ifeq ($(CONFIG_USER_IP_QOS_3), y)
OBJS += prmt_queuemnt_3.o
endif
endif

ifeq ($(CONFIG_USER_BRIDGE_GROUPING), y)
OBJS += prmt_layer2br.o
endif

#China Telecom e8 specified parameters, use QoS v2
ifeq ($(CONFIG_E8B), y)
OBJS += prmt_ctcom.o prmt_ctcom_wanext.o prmt_ctcom_lanext.o prmt_ctcom_ping.o
OBJS += prmt_ctcom_queuemnt.o
ifeq ($(CONFIG_USER_CWMP_UPNP_DM), y)
OBJS += prmt_ctcom_proxy_dev.o prmt_ctcom_proxy_dev_list.o ctcom_upnpdm_proxy.o
endif
endif

#xDSL
ifeq ($(CONFIG_DEV_xDSL), y)
OBJS += prmt_wandsldiagnostics.o prmt_wanatmf5loopback.o
TR181_OBJS +=  tr181/prmt_dsl.o tr181/prmt_dsl_line.o tr181/prmt_dsl_channel.o \
		tr181/prmt_atm.o tr181/prmt_dsl_diag.o
endif

#PTM WAN
ifeq ($(CONFIG_PTMWAN), y)
TR181_OBJS +=  tr181/prmt_ptm.o
endif

#Telefonica specified parameters
ifeq ($(CONFIG_BHS), y)
ifneq (, $(filter y, $(CONFIG_NET_IPIP) $(CONFIG_NET_IPGRE) $(CONFIG_IPV6_SIT_6RD) $(CONFIG_IPV6_TUNNEL)))
OBJS += prmt_telefonica_ipv6.o iptunnel.o
endif
endif

#TR-181 Support
ifeq ($(CONFIG_CWMP_TR181_SUPPORT), y)
OBJS += $(TR181_OBJS)
endif

DEPH =	$(ROOTDIR)/config/autoconf.h \
	$(ROOTDIR)/include/rtk/options.h \
	prmt_igd.h prmt_deviceinfo.h prmt_landevice_eth.h prmt_mngmtserver.h \
	prmt_layer3fw.h prmt_landevice.h prmt_landevice_wlan.h \
	prmt_wancondevice.h prmt_ippingdiag.h prmt_time.h prmt_ddns.h \
	prmt_wanatmf5loopback.h prmt_services.h prmt_userif.h prmt_ctcom.h \
	prmt_captiveportal.h prmt_traceroute.h prmt_landevice_usb.h \
	prmt_apply.h prmt_wandevice.h prmt_nslookup.h

#QoS v1
ifeq ($(IP_QOS), y)
DEPH += prmt_queuemnt.h
endif

#QoS v3
ifeq ($(CONFIG_USER_IP_QOS_3), y)
DEPH += prmt_queuemnt_3.h
endif

ifeq ($(CONFIG_USER_BRIDGE_GROUPING), y)
DEPH += prmt_layer2br.h
endif

#China Telecom e8 specified parameters
ifeq ($(CONFIG_E8B), y)
DEPH += prmt_ctcom.h prmt_ctcom_wanext.h prmt_ctcom_lanext.h prmt_ctcom_ping.h
ifeq ($(CONFIG_USER_CWMP_UPNP_DM), y)
DEPH += prmt_ctcom_proxy_dev.h prmt_ctcom_proxy_dev_list.h ctcom_upnpdm_proxy.h
CFLAGS += -I$(ROOTDIR)/user/upnpmd-cp/includes
endif
ifeq ($(NEW_IP_QOS_SUPPORT), y)
ifeq ($(CONFIG_USER_IPROUTE2_TC_TC), y)
DEPH += prmt_ctcom_queuemnt.h
endif
endif
endif

#xDSL
ifeq ($(CONFIG_DEV_xDSL), y)
DEPH += prmt_wandsldiagnostics.h
endif

#Telefonica specified parameters
ifeq ($(CONFIG_BHS), y)
ifneq (, $(filter y, $(CONFIG_NET_IPIP) $(CONFIG_NET_IPGRE) $(CONFIG_IPV6_SIT_6RD) $(CONFIG_IPV6_TUNNEL)))
DEPH += prmt_telefonica_ipv6.h iptunnel.h
endif
endif

CFLAGS += -I$(CWMPCLIENT_ROOT)/../include

# To support TR104
ifeq ($(CONFIG_USER_TR104),y)
DEPH += $(CWMPCLIENT_ROOT)/../../rtk_voip/rtk_tr104/prmt_voiceservice.h
#CFLAGS += -I$(CWMPCLIENT_ROOT)/../../rtk_voip/tr104 -I$(CWMPCLIENT_ROOT)
CFLAGS += -I$(CWMPCLIENT_ROOT)/../../rtk_voip/rtk_tr104 -I$(CWMPCLIENT_ROOT)
endif

#set the ssl libraries
ifeq ($(CONFIG_USER_CWMP_WITH_OPENSSL),y)
LIBCWMP = ../lib/libcwmpssl.a ../lib/libgsoapssl.a
LIBCWMPFLAG = -lssl -lcrypto
endif
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL),y)
LIBCWMP = ../lib/libcwmpmssl.a ../lib/libgsoapmssl.a
LIBCWMPFLAG = -lc -L$(CWMPCLIENT_ROOT)/../../../lib/libmatrixssl -lmatrixssl
endif
ifeq ($(CONFIG_USER_CWMP_WITH_MATRIXSSL_NEW),y)
LIBCWMP = ../lib/libcwmpmssl.a ../lib/libgsoapmssl.a
LIBCWMPFLAG = -lc -L$(CWMPCLIENT_ROOT)/../../../lib/libmatrixssl_new -lcore -lpscrypto -lmatrixssl_new
endif
ifneq ($(CONFIG_USER_CWMP_WITH_SSL),y)
LIBCWMP = ../lib/libcwmp.a ../lib/libgsoapck.a
LIBCWMPFLAG =
endif

# To Support TR104
ifeq ($(CONFIG_USER_TR104),y)
#LIBCWMP += ../../rtk_voip/tr104/voip_tr104.a ../../rtk_voip/voip_manager/libvoip_manager.a
LIBCWMP += ../../rtk_voip/rtk_tr104/voip_tr104.a ../../rtk_voip/voip_manager/libvoip_manager.a ../../rtk_voip/flash/libvoip_flash_client.a
endif

#additional linking library
#-lcrypt for crypt() when creating  the /etc/passwd file
LIBCWMPFLAG += -lcrypt -lm

# For e8-C GPON, EPON parameters
ifeq ($(CONFIG_E8B), y)
ifdef CONFIG_GPON_FEATURE
CFLAGS += -I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/include \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/include \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/romeDriver
LIBCWMPFLAG += -L$(APOLLODIR)/object/src/app/lib -lrtk
else
ifdef  CONFIG_EPON_FEATURE
CFLAGS += -I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/include \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/sdk/system/include \
-I$(ROOTDIR)/linux-2.6.x/drivers/net/rtl86900/romeDriver
LIBCWMPFLAG += -L$(APOLLODIR)/object/src/app/lib -lrtk
endif
endif
endif

all: $(EXEC) $(TR143_EXEC)

udpechoserver.o: udpechoserver.c $(TR143_LIBDEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TR143_LIBOBJS): udpechoserverlib.c $(TR143_LIBDEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TR143_EXEC): udpechoserver.o $(TR143_LIBOBJS)
	$(CC) $(LDFLAGS) -o $@ udpechoserver.o $(TR143_LIBOBJS)

%.o: %.c $(DEPH) $(TR143_LIBDEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXEC): $(OBJS) $(LIBCWMP) $(TR143_LIBOBJS) $(TR143_LIBDEPH)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBCWMP) $(TR143_LIBOBJS) $(LIBMIB) $(LIBVOIP) -lpthread -lrt $(LIBMD5) $(LIBCWMPFLAG)


clean:
	-rm -f $(EXEC) *.o tr181/*.o udpechoserver

