#
#	Makefile -- Build instructions for user level apps
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

VEND=$(ROOTDIR)/vendors
ifdef CONFIG_USE_RSDK_WRAPPER
LDFLAGS += -L$(ROOTDIR)/lib/librtk
export LDFLAGS
endif #CONFIG_USE_RSDK_WRAPPER

ifeq ($(CONFIG_LUNA),y)
ifeq ($(CONFIG_RTL8686_SWITCH),y)
CFLAGS += -include $(ROOTDIR)/autoconf.h
CFLAGS += -I$(APOLLODIR)/sdk/include
CFLAGS += -I$(APOLLODIR)/sdk/system/include
CFLAGS += -I$(APOLLODIR)/romeDriver
LDFLAGS += -L$(APOLLODIR)/object/src/app/lib
ifeq ($(CONFIG_TR142_MODULE),y)
CFLAGS += -I$(APOLLODIR)/tr142/include -I$(APOLLODIR)/sdk/src/app/omci_v1/API
endif
LIBRTK = -lrtk -lpthread
endif # CONFIG_RTL8686_SWITCH
endif #CONFIG_LUNA

LIBMIB = -lmib $(LIBRTK)

ifeq ($(CONFIG_USER_WLAN_QCSAPI),y)
LIBMIB += -lqcsapi_client
LDFLAGS += -L$(ROOTDIR)/user/qcsapi  
endif

##### add for VoIP merging ##########
ifeq ($(CONFIG_USER_RTK_VOIP),y)
DIR_LINUX = $(ROOTDIR)/$(LINUXDIR)
DIR_USERS=$(ROOTDIR)/user
export DIR_LINUX
export DIR_USERS

export LIBVOIP
LIBVOIP = -L$(ROOTDIR)/user/rtk_voip/voip_manager -lvoip_manager

ifeq ($(CONFIG_E8B),y)
LIBMIB += $(LIBSSL)
endif
endif
######################################
#

# must run the vendor build first
#
dir_v = $(VEND)/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.
ifeq ($(CONFIG_USER_RTK_VOIP),y)
dir_v += rtk_voip
endif

dir_y =
dir_n =
dir_  =
dir_e =

dir_$(CONFIG_USER_RTK_VOIP)                 += rtk_voip	# before boa
dir_$(CONFIG_USER_WLAN_QCSAPI)              += qcsapi
dir_$(CONFIG_USER_WLAN_QCSAPI)              += ip_relay_1.1

dir_$(CONFIG_USER_AGETTY_AGETTY)            += agetty
dir_$(CONFIG_USER_AT_AT)                    += at
dir_$(CONFIG_USER_AT_ATD)                   += at
dir_$(CONFIG_USER_AT_ATRUN)                 += at
dir_$(CONFIG_USER_BASH_BASH)                += bash
ifndef CONFIG_E8B
dir_$(CONFIG_USER_BOA_SRC_BOA)              += boa
else
dir_$(CONFIG_USER_BOA_SRC_BOA)              += boa_e8
endif
ifeq ($(CONFIG_LUNA),y)
dir_$(CONFIG_USER_BOA_SRC_BOA)              += mpctl
endif

#because boa need to call GPON or EPON APIs in librtk.a and diagshell will build it
ifeq ($(CONFIG_LUNA),y)
ifeq ($(CONFIG_USER_APOLLO_DIAGSHELL),y)
dir_v +=diagshell
endif
endif

ifeq ($(CONFIG_PTM_BONDING),y)
CFLAGS += -DCONFIG_PTM_BONDING
endif

ifeq ($(CONFIG_PTM_BONDING_MASTER),y)
CFLAGS += -DCONFIG_PTM_BONDING_MASTER
endif

ifeq ($(CONFIG_PTM_BONDING_SLAVE),y)
CFLAGS += -DCONFIG_PTM_BONDING_SLAVE
endif

ifneq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
	dir_$(CONFIG_JFFS_FS)                       += mtd-utils-1.5.2  # 3.18
	dir_$(CONFIG_JFFS2_FS)                      += mtd-utils-1.5.2  # 3.18
	else
	dir_$(CONFIG_JFFS_FS)                       += mtd-utils-1.0.0
	dir_$(CONFIG_JFFS2_FS)                      += mtd-utils-1.0.0
endif

dir_$(CONFIG_USER_ZM8620_ECMMODE) += zecmapp
dir_$(CONFIG_USER_IPUTILS) += iputils
dir_$(CONFIG_USER_DBUS) += dbus
dir_$(CONFIG_USER_LXC) += lxc
dir_$(CONFIG_USER_GLIB) += glib
dir_$(CONFIG_USER_DBUS_CTC_IGD) 	    += ctc-igd
dir_$(CONFIG_USER_BOOTTOOLS_FLASHLOADER)    += boottools
dir_$(CONFIG_USER_BOOTTOOLS_HIMEMLOADER)    += boottools
dir_$(CONFIG_USER_BPALOGIN_BPALOGIN)        += bpalogin
dir_$(CONFIG_USER_BR2684CTL_BR2684CTL)      += br2684ctl
dir_$(CONFIG_USER_BRCFG_BRCFG)              += brcfg
dir_$(CONFIG_USER_BRCTL_BRCTL)              += bridge-utils
dir_$(CONFIG_USER_BUSYBOX_BUSYBOX)          += busybox
# Kao added 11/05/03
dir_$(CONFIG_USER_BUSYBOX_BUSYBOX100PRE3)   += busybox-1.00-pre3
# Mason Yu. for ipv6
dir_$(CONFIG_USER_BUSYBOX_BUSYBOX1124)      += busybox-1.12.4
dir_$(CONFIG_USER_BUSYBOX_BUSYBOX1221)      += busybox-1.22.1
dir_$(CONFIG_USER_CAL_CAL)                  += cal
dir_$(CONFIG_USER_CAL_DATE)                 += cal
dir_$(CONFIG_USER_CHAT_CHAT)                += pppd/chat
dir_$(CONFIG_USER_CKSUM_CKSUM)              += cksum
dir_$(CONFIG_USER_CLOCK_CLOCK)              += clock
dir_$(CONFIG_USER_CPU_CPU)                  += cpu
dir_$(CONFIG_USER_CRAMFS_MKCRAMFS)          += cramfs
dir_$(CONFIG_USER_CRAMFS_CRAMFSCK)          += cramfs
dir_$(CONFIG_USER_NTFS_NTFS3G)              += ntfs-3g
dir_$(CONFIG_USER_CRON_CRON)                += cron
dir_$(CONFIG_USER_CUPS)			    += cups
dir_$(CONFIG_USER_DEMO_BUTTON)              += demo
dir_$(CONFIG_USER_DEMO_MORSE)               += demo
dir_$(CONFIG_USER_DHRYSTONE_DHRYSTONE)      += dhrystone
dir_$(CONFIG_USER_DHCP_ISC_SERVER_DHCPD)    += dhcp-isc
dir_$(CONFIG_USER_DHCP_ISC_CLIENT_DHCLIENT) += dhcp-isc
dir_$(CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY)  += dhcp-isc
dir_$(CONFIG_USER_DHCPCD_DHCPCD)            += dhcpcd
dir_$(CONFIG_USER_DHCPCD_NEW_DHCPCD)        += dhcpcd-new
dir_$(CONFIG_USER_DHCPD_DHCPD)              += dhcpd
# Mason Yu. for DHCPv6
dir_$(CONFIG_USER_DHCPV6_ISC_DHCP411)	    += dhcp-4.1.1
dir_$(CONFIG_USER_DIALD_DIALD)              += diald
dir_$(CONFIG_USER_DISCARD_DISCARD)          += discard
dir_$(CONFIG_USER_DNSMASQ_DNSMASQ)          += dnsmasq
dir_$(CONFIG_USER_DNSMASQ_DNSMASQ245)       += dnsmasq-2.45
dir_$(CONFIG_USER_E2FSPROGS_E2FSCK_E2FSCK)  += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_BADBLOCKS) += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_CHATTR)    += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_DUMPE2FS)  += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_E2LABEL)   += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_FSCK)      += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_LSATTR)    += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_MKE2FS)    += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_MKLOST_FOUND) += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_TUNE2FS)   += e2fsprogs
dir_$(CONFIG_USER_E2FSPROGS_MISC_UUIDGEN)   += e2fsprogs
dir_$(CONFIG_USER_ELVISTINY_VI)             += elvis-tiny
dir_$(CONFIG_USER_ETHATTACH_ETHATTACH)      += ethattach
dir_$(CONFIG_USER_EZIPUPDATE_EZIPUPDATE)    += ez-ipupdate
dir_$(CONFIG_USER_FDISK_FDISK)              += fdisk
dir_$(CONFIG_USER_FILEUTILS_CAT)            += fileutils
dir_$(CONFIG_USER_FILEUTILS_CHGRP)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_CHMOD)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_CHOWN)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_CMP)            += fileutils
dir_$(CONFIG_USER_FILEUTILS_CP)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_DD)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_GREP)           += fileutils
dir_$(CONFIG_USER_FILEUTILS_L)              += fileutils
dir_$(CONFIG_USER_FILEUTILS_LN)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_LS)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_MKDIR)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_MKFIFO)         += fileutils
dir_$(CONFIG_USER_FILEUTILS_MKNOD)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_MORE)           += fileutils
dir_$(CONFIG_USER_FILEUTILS_MV)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_RM)             += fileutils
dir_$(CONFIG_USER_FILEUTILS_RMDIR)          += fileutils
dir_$(CONFIG_USER_FILEUTILS_SYNC)           += fileutils
dir_$(CONFIG_USER_FILEUTILS_TOUCH)          += fileutils
dir_$(CONFIG_USER_FLASH_FLASH)              += flash
dir_$(CONFIG_USER_FLASHW_FLASHW)            += flashw
dir_$(CONFIG_USER_FLATFSD_XXX)              += flatfsd
dir_$(CONFIG_USER_FON)			    += fongw
dir_$(CONFIG_USER_FREESWAN)                 += freeswan
dir_$(CONFIG_USER_FROB_LED_FROB_LED)        += frob-led
dir_$(CONFIG_USER_FSWCERT_FSWCERT)          += fswcert
dir_$(CONFIG_USER_FTP_FTP_FTP)              += ftp
dir_$(CONFIG_USER_FTPD_FTPD)                += ftpd
dir_$(CONFIG_USER_BFTPD_BFTPD)              += bftpd
dir_$(CONFIG_USER_BFTPD_BFTPD)              += ftp_manage
#Jia-chiam: gdbserver has been porting in user2@Luna trunk.
#           If someone want to recover it, plz let me know.
#dir_$(CONFIG_USER_GDB_GDB)                  += gdb-6.8
#dir_$(CONFIG_USER_GDB_GDBSERVER)            += gdb-6.8
#dir_$(CONFIG_USER_GDB_GDBREPLAY)            += gdb-6.8
dir_$(CONFIG_USER_GETTYD_GETTYD)            += gettyd
dir_$(CONFIG_USER_HD_HD)                    += hd
dir_$(CONFIG_USER_HTTPD_HTTPD)              += httpd
dir_$(CONFIG_USER_HWCLOCK_HWCLOCK)          += hwclock
dir_$(CONFIG_USER_IFATTACH_IFATTACH)        += ifattach
dir_$(CONFIG_USER_IFMOND2_IFMOND)           += ifmond2
dir_$(CONFIG_USER_INETD_INETD)              += inetd
dir_$(CONFIG_USER_INIT.ORG_INIT)            += init.org
dir_$(CONFIG_USER_INIT_EXPAND)              += init
dir_$(CONFIG_USER_INIT_INIT)                += init
dir_$(CONFIG_USER_IPCHAINS_IPCHAINS)        += ipchains
dir_$(CONFIG_USER_IPERF)                    += iperf-2.0.4-src
dir_$(CONFIG_USER_IPFWADM_IPFWADM)          += ipfwadm
dir_$(CONFIG_USER_IPMASQADM_IPMASQADM)      += ipmasqadm
dir_$(CONFIG_USER_IPPORTFW_IPPORTFW)        += ipportfw
dir_$(CONFIG_USER_IPREDIR_IPREDIR)          += ipredir
dir_$(CONFIG_USER_IPROUTE2)                 += iproute2-2.6.33
dir_$(CONFIG_USER_IPROUTE2_3_4_0)			+= iproute2-3.4.0
dir_$(CONFIG_USER_IPTABLES_IPTABLES)        += iptables
dir_$(CONFIG_USER_IPTABLES_IPTABLES_1_4_18)	+= iptables-1.4.18
dir_$(CONFIG_BRIDGE_NF_EBTABLES)            += ebtables
dir_$(CONFIG_USER_KLAXON_KLAXON)            += klaxon
dir_$(CONFIG_USER_LANG_A60)                 += a60
dir_$(CONFIG_USER_LCD_LCD)                  += lcd
#ql porting new l2tpd
#dir_$(CONFIG_USER_L2TPD_L2TPD)              += l2tpd
dir_$(CONFIG_USER_LEVEE_VI)                 += levee
dir_$(CONFIG_USER_LILO_LILO)                += lilo
dir_$(CONFIG_USER_LIRC)                     += lirc
dir_$(CONFIG_USER_LISSA_LISSA)              += lissa
dir_$(CONFIG_USER_LOATTACH_LOATTACH)        += loattach
dir_$(CONFIG_USER_LOGIN_LOGIN)              += login
dir_$(CONFIG_USER_LOGIN_PASSWD)             += login
dir_$(CONFIG_USER_RTK_LBD)					+= loopback
dir_$(CONFIG_USER_MAIL_MAIL_IP)             += mail
dir_$(CONFIG_USER_MATH_TEST)                += mathtest
dir_$(CONFIG_USER_MAWK_AWK)                 += mawk
ifneq ($(CONFIG_DEFAULTS_KERNEL_2_6),y)
ifneq ($(CONFIG_DEFAULTS_KERNEL_3_18),y)
dir_$(CONFIG_USER_MTDUTILS)                 += mtd-utils
else
dir_$(CONFIG_USER_MTDUTILS)                 += mtd-utils-1.5.2	# 3.18
endif
else
dir_$(CONFIG_USER_MTDUTILS)                 += mtd-utils-1.0.0	# 2.6.x
endif
dir_$(CONFIG_USER_MICROWIN)                 += microwin
dir_$(CONFIG_USER_MINI_HTTPD_MINI_HTTPD)    += mini_httpd
dir_$(CONFIG_USER_MOUNT_MOUNT)              += mount
dir_$(CONFIG_USER_MOUNT_UMOUNT)             += mount
dir_$(CONFIG_USER_MP3PLAY_MP3PLAY)          += mp3play
dir_$(CONFIG_USER_MSNTP_MSNTP)              += msntp
dir_$(CONFIG_USER_MUSICBOX_MUSICBOX)        += musicbox
dir_$(CONFIG_USER_NETFLASH_NETFLASH)        += netflash
dir_$(CONFIG_USER_NET_TOOLS_ARP)            += net-tools
dir_$(CONFIG_USER_NET_TOOLS_HOSTNAME)       += net-tools
dir_$(CONFIG_USER_NET_TOOLS_IFCONFIG)       += net-tools
dir_$(CONFIG_USER_NET_TOOLS_NAMEIF)         += net-tools
dir_$(CONFIG_USER_NET_TOOLS_NETSTAT)        += net-tools
dir_$(CONFIG_USER_NET_TOOLS_PLIPCONFIG)     += net-tools
dir_$(CONFIG_USER_NET_TOOLS_RARP)           += net-tools
dir_$(CONFIG_USER_NET_TOOLS_ROUTE)          += net-tools
dir_$(CONFIG_USER_NET_TOOLS_SLATTACH)       += net-tools
dir_$(CONFIG_USER_NET_TOOLS_IPMADDR)        += net-tools
dir_$(CONFIG_USER_NET_TOOLS_IPTUNNEL)       += net-tools
dir_$(CONFIG_USER_NET_TOOLS_MII_TOOL)       += net-tools
dir_$(CONFIG_USER_NETSTAT_NAT_NETSTAT_NAT)  += netstat-nat
dir_$(CONFIG_USER_NULL_NULL)                += null
dir_$(CONFIG_USER_NWSH_SH)                  += nwsh
dir_$(CONFIG_USER_OGGPLAY_EXAMPLE)          += oggplay
dir_$(CONFIG_USER_OGGPLAY_OGG123)           += oggplay
dir_$(CONFIG_USER_OPENSSL_APPS)             += openssl
dir_$(CONFIG_USER_OPENSSL_APPS_1_0_1C)      += openssl-1.0.1c
dir_$(CONFIG_XFRM)                          += ipsec-tools-0.7
dir_$(CONFIG_USER_PALMBOT_PALMBOT)          += palmbot
dir_$(CONFIG_USER_PCIUTILS_LSPCI)           += pciutils
dir_$(CONFIG_USER_PCIUTILS_SETPCI)          += pciutils
dir_$(CONFIG_USER_PCMCIA_CS)                += pcmcia-cs
dir_$(CONFIG_USER_PCSCLITE)                 += pcsc
dir_$(CONFIG_USER_PERL_PERL)                += perl
dir_$(CONFIG_USER_PING_PING)                += ping
dir_$(CONFIG_USER_PKTDELAY_PKTDELAY)        += pktdelay
dir_$(CONFIG_USER_PLAY_PLAY)                += play
dir_$(CONFIG_USER_PLAY_TONE)                += play
dir_$(CONFIG_USER_PLAYRT_PLAYRT)            += playrt
dir_$(CONFIG_USER_PLUG_PLUG)                += plug
dir_$(CONFIG_USER_PORTMAP_PORTMAP)          += portmap
dir_$(CONFIG_USER_PPPD_PPPD_PPPD)           += pppd
#dir_$(CONFIG_USER_PPPOE_PROXY)              += pppd
#dir_$(CONFIG_USER_PPTP_CLIENT_PPTP)         += pptp-client
#dir_$(CONFIG_USER_PPTP_CLIENT_PPTP_CALLMGR) += pptp-client
#dir_$(CONFIG_USER_PPTPD_PPTPCTRL)           += pptpd
#dir_$(CONFIG_USER_PPTPD_PPTPD)              += pptpd
dir_$(CONFIG_USER_PROCPS_FREE)              += procps
dir_$(CONFIG_USER_PROCPS_KILL)              += procps
dir_$(CONFIG_USER_PROCPS_PGREP)             += procps
dir_$(CONFIG_USER_PROCPS_PKILL)             += procps
dir_$(CONFIG_USER_PROCPS_PMAP)              += procps
dir_$(CONFIG_USER_PROCPS_PS)                += procps
dir_$(CONFIG_USER_PROCPS_PWDX)              += procps
dir_$(CONFIG_USER_PROCPS_SKILL)             += procps
dir_$(CONFIG_USER_PROCPS_SLABTOP)           += procps
dir_$(CONFIG_USER_PROCPS_SNICE)             += procps
dir_$(CONFIG_USER_PROCPS_SYSCTL)            += procps
dir_$(CONFIG_USER_PROCPS_TLOAD)             += procps
dir_$(CONFIG_USER_PROCPS_TOP)               += procps
dir_$(CONFIG_USER_PROCPS_UPTIME)            += procps
dir_$(CONFIG_USER_PROCPS_VMSTAT)            += procps
dir_$(CONFIG_USER_PROCPS_W)                 += procps
dir_$(CONFIG_USER_PROCPS_WATCH)             += procps
dir_$(CONFIG_USER_HTOP)                     += htop
dir_$(CONFIG_USER_PYTHON_PYTHON)            += python
# Mason Yu. for ipv6
#dir_$(CONFIG_USER_RADVD)	            += radvd-1.5
dir_$(CONFIG_USER_RADVD)	            += radvd-1.8
dir_$(CONFIG_USER_RARP)			    += rarp
dir_$(CONFIG_USER_RDATE_RDATE)              += rdate
dir_$(CONFIG_USER_RAMIMAGE_RAMFS64)         += ramimage
dir_$(CONFIG_USER_RAMIMAGE_RAMFS128)        += ramimage
dir_$(CONFIG_USER_RAMIMAGE_RAMFS256)        += ramimage
dir_$(CONFIG_USER_RAMIMAGE_RAMFS512)        += ramimage
dir_$(CONFIG_USER_READPROFILE_READPROFILE)  += readprofile
dir_$(CONFIG_USER_RECOVER_RECOVER)          += recover
dir_$(CONFIG_USER_REISERFSPROGS)            += reiserfsprogs
dir_$(CONFIG_USER_ROOTLOADER_ROOTLOADER)    += rootloader
dir_$(CONFIG_USER_ROUTE_ARP)                += route
dir_$(CONFIG_USER_ROUTE_IFCONFIG)           += route
dir_$(CONFIG_USER_ROUTE_MIITOOL)            += route
dir_$(CONFIG_USER_ROUTE_ROUTE)              += route
dir_$(CONFIG_USER_ROUTE_NETSTAT)            += route
dir_$(CONFIG_USER_ROUTED_ROUTED)            += routed
dir_$(CONFIG_USER_RP_PPPOE_PPPOE)           += rp-pppoe
#dir_$(CONFIG_USER_PPPOE_PROXY)              += rp-pppoe
dir_$(CONFIG_USER_RTC_M41T11)               += rtc-m41t11
dir_$(CONFIG_USER_RTC_DS1302)               += rtc-ds1302
dir_$(CONFIG_USER_SAMBA_3_0_24)             += samba-3.0.24
dir_$(CONFIG_USER_SAMBA_3_2_4)              += samba-3.2.4
dir_$(CONFIG_USER_SASH_REBOOT)              += sash
dir_$(CONFIG_USER_SASH_SH)                  += sash
dir_$(CONFIG_USER_SASH_SHUTDOWN)            += sash
dir_$(CONFIG_USER_SASH_SHUTDOWN)            += sash
dir_$(CONFIG_USER_SETHDLC_SETHDLC)          += sethdlc
dir_$(CONFIG_USER_SETSERIAL_SETSERIAL)      += setserial
dir_$(CONFIG_USER_SH_SH)                    += sh
dir_$(CONFIG_USER_SHUTILS_BASENAME)         += shutils
dir_$(CONFIG_USER_SHUTILS_DATE)             += shutils
dir_$(CONFIG_USER_SHUTILS_DIRNAME)          += shutils
dir_$(CONFIG_USER_SHUTILS_ECHO)             += shutils
dir_$(CONFIG_USER_SHUTILS_FALSE)            += shutils
dir_$(CONFIG_USER_SHUTILS_LOGNAME)          += shutils
dir_$(CONFIG_USER_SHUTILS_PRINTENV)         += shutils
dir_$(CONFIG_USER_SHUTILS_PWD)              += shutils
dir_$(CONFIG_USER_SHUTILS_TRUE)             += shutils
dir_$(CONFIG_USER_SHUTILS_UNAME)            += shutils
dir_$(CONFIG_USER_SHUTILS_WHICH)            += shutils
dir_$(CONFIG_USER_SHUTILS_WHOAMI)           += shutils
dir_$(CONFIG_USER_SHUTILS_YES)              += shutils
dir_$(CONFIG_USER_SLATTACH_SLATTACH)        += slattach
dir_$(CONFIG_USER_SMBMOUNT_SMBMOUNT)        += smbmount
dir_$(CONFIG_USER_SMBMOUNT_SMBUMOUNT)       += smbmount
dir_$(CONFIG_USER_SMTP_SMTPCLIENT)          += smtpclient
dir_$(CONFIG_USER_SNMPD_SNMPD)              += snmpd
dir_$(CONFIG_USER_SNMPD_SNMPD_V2C)          += snmpdv2c
dir_$(CONFIG_USER_SNMPD_SNMPD_V2CTRAP)      += snmpdv2cTrap
dir_$(CONFIG_USER_STUNNEL_STUNNEL)          += stunnel
dir_$(CONFIG_USER_SQUID_SQUID)              += squid
dir_$(CONFIG_USER_SSH_DROPBEAR)             += dropbear-0.48.1
dir_$(CONFIG_USER_SSH_SSHD)                 += ssh
dir_$(CONFIG_USER_SSH_SSH)                  += ssh
dir_$(CONFIG_USER_SSH_SSHKEYGEN)            += ssh
dir_$(CONFIG_USER_STP_STP)                  += stp
dir_$(CONFIG_USER_STUN_CLIENT) 		    += stun
dir_$(CONFIG_USER_STRACE_STRACE)            += strace
dir_$(CONFIG_USER_STTY_STTY)                += stty
dir_$(CONFIG_USER_SYSUTILS_DF)              += sysutils
dir_$(CONFIG_USER_SYSUTILS_FREE)            += sysutils
dir_$(CONFIG_USER_SYSUTILS_HOSTNAME)        += sysutils
dir_$(CONFIG_USER_SYSUTILS_KILL)            += sysutils
dir_$(CONFIG_USER_SYSUTILS_PS)              += sysutils
dir_$(CONFIG_USER_SYSUTILS_REBOOT)          += sysutils
dir_$(CONFIG_USER_SYSUTILS_SHUTDOWN)        += sysutils
dir_$(CONFIG_USER_TCPBLAST_TCPBLAST)        += tcpblast
dir_$(CONFIG_USER_TCPWRAP_TCPD)             += tcpwrappers
dir_$(CONFIG_USER_TCPDUMP_TCPDUMP)          += tcpdump
dir_$(CONFIG_USER_TCPDUMP_WEB)              += libpcap
dir_$(CONFIG_USER_TCPDUMP_WEB)              += tcpdump_web
dir_$(CONFIG_USER_TCSH_TCSH)                += tcsh
dir_$(CONFIG_USER_TELNET_TELNET)            += telnet
dir_$(CONFIG_USER_TELNETD_TELNETD)          += telnetd
dir_$(CONFIG_USER_TFTP_TFTP)                += tftp
dir_$(CONFIG_USER_TFTPD_TFTPD)              += tftpd
dir_$(CONFIG_USER_THREADDEMOS_THREADDEMOS)  += threaddemos
dir_$(CONFIG_USER_THTTPD_THTTPD)            += thttpd
dir_$(CONFIG_USER_TINYLOGIN_CRYPT_CRYPT)    += tinylogin
dir_$(CONFIG_USER_TINYLOGIN_TINYLOGIN)      += tinylogin
dir_$(CONFIG_USER_TIP_TIP)                  += tip
dir_$(CONFIG_USER_TIMEPEG_TPT)              += tpt
dir_$(CONFIG_USER_TRACEROUTE_TRACEROUTE)    += traceroute
dir_$(CONFIG_USER_TRIPWIRE_SIGGEN)          += tripwire
dir_$(CONFIG_USER_TRIPWIRE_TRIPWIRE)        += tripwire
dir_$(CONFIG_USER_3G_USBTOOLS)              += usbtools
dir_$(CONFIG_USER_USBMOUNT_USBMOUNT)        += usbmount
dir_$(CONFIG_USER_UCDSNMP_SNMPD)            += ucdsnmp
dir_$(CONFIG_USER_VERSION_VERSION)          += version
dir_$(CONFIG_USER_W3CAM_VIDCAT)             += w3cam
dir_$(CONFIG_USER_VIXIECRON_CRON)           += vixie-cron
dir_$(CONFIG_USER_VIXIECRON_CRONTAB)        += vixie-cron
dir_$(CONFIG_USER_VPLAY_VPLAY)              += vplay
dir_$(CONFIG_USER_VPLAY_VREC)               += vplay
dir_$(CONFIG_USER_VPLAY_MIXER)              += vplay
dir_$(CONFIG_USER_VPNLED_VPNLED)            += vpnled
dir_$(CONFIG_USER_LEDCON_LEDCON)            += ledcon
dir_$(CONFIG_USER_W3CAM_W3CAMD)             += w3cam
dir_$(CONFIG_USER_WAPI)                     += wapi-utils
dir_$(CONFIG_USER_WGET)                     += wget
dir_$(CONFIG_USER_WGET_MANAGE)              += wget_manage
dir_$(CONFIG_USER_WATCHDOG_WDG)             += watchdog
dir_$(CONFIG_USER_WIRELESS_TOOLS)           += wireless_tools
dir_$(CONFIG_USER_WLAN_SERVER)              += wlan_server
dir_$(CONFIG_USER_XMLCONFIG)                += xmlconfig
dir_$(CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH) += auth
#dir_$(CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH)           += ../linux-2.4.x/drivers/net/wireless/RTL8185_AP/auth/src
#dir_$(CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH)           += ../linux-2.4.x/drivers/net/wireless/RTL8185_AP/auth/src/dlisten
#dir_$(CONFIG_USER_WIRELESS_TOOLS_RTL8185_IAPP)           += ../linux-2.4.x/drivers/net/wireless/RTL8185_AP/IAPP
dir_$(CONFIG_USER_Y1731)					+= y1731-0.0.1
dir_$(CONFIG_USER_ZEBRA_BGPD_BGPD)          += zebra
dir_$(CONFIG_USER_ZEBRA_OSPFD_OSPFD)        += zebra
dir_$(CONFIG_USER_ZEBRA_RIPD_RIPD)          += zebra
dir_$(CONFIG_USER_ZEBRA_ZEBRA_ZEBRA)        += zebra
dir_$(CONFIG_USER_SFREE)                    += sfree
dir_$(CONFIG_USER_CWMP_TR069)               += cwmp-tr069
dir_$(CONFIG_USER_CWMP_UPNP_DM)             += upnpmd-cp
#add by xl_yue for midware interface
dir_$(CONFIG_MIDDLEWARE)                    += midware_interface
dir_$(CONFIG_MIDDLEWARE)                    += ct-midware
dir_$(CONFIG_USER_DOT1AG_UTILS)             += dot1ag-utils libpcap

dir_$(CONFIG_TR142_MODULE)                  += rtk_tr142

#add by alex_huang netlogger
#dir_$(CONFIG_USER_NETLOGGER_SUPPORT)+= netlogger
#dir_y += games
dir_$(CONFIG_RTL8672_SAR) += sarctl
dir_$(CONFIG_DEV_xDSL) += adslctrl
ifndef CONFIG_ARCH_LUNA_SLAVE
dir_$(CONFIG_NETDEVICES) += ethctl
endif
dir_$(CONFIG_RTL8685_PTM_MII) += ptmctl
dir_$(CONFIG_USER_WIRELESS_TOOLS) += wlan_daemon
dir_$(CONFIG_USB_RTL8672_USBCTL) += usbctl
#dir_y += goahead-2.1.4

#dir_y += wsc
#dir_y += mini_upnp

dir_$(CONFIG_USER_LANNETINFO) += lanNetInfo
dir_$(CONFIG_USER_OTHER_SH) += msh
dir_$(CONFIG_USER_UDHCP099PRE2) += udhcp-0.9.9-pre2
dir_$(CONFIG_PPP) += spppd
dir_$(CONFIG_USER_DDNS)			   +=updatedd-2.5
dir_$(CONFIG_USER_DDNS)			   +=updateddctrl
dir_$(CONFIG_USER_IGMPPROXY)               += igmpproxy
dir_$(CONFIG_USER_VSNTP)                   += vsntp
dir_$(CONFIG_USER_UPNPD)		   +=upnpd
dir_$(CONFIG_USER_UPNPD)		   +=upnpctrl
dir_$(CONFIG_USER_MINI_UPNPD)	+= mini_upnp
dir_$(CONFIG_USER_MINIUPNPD)		   +=miniupnpd
dir_$(CONFIG_USER_MINIUPNPD)		   +=upnpctrl
ifndef CONFIG_ARCH_LUNA_SLAVE
dir_$(CONFIG_WIFI_SIMPLE_CONFIG)	+= wsc
endif
dir_$(CONFIG_RTL_11R_SUPPORT) += ft_daemon
dir_$(CONFIG_DOT11K_TEST_TOOL) += dot11k
dir_$(CONFIG_USER_MINIDLNA)                +=minidlna
#dir_$(CONFIG_WIFI_SIMPLE_CONFIG)	+= auth
dir_$(CONFIG_USB_RTL8192SU_SOFTAP) += rtl8192su

#august for ethtool
dir_$(CONFIG_ETHTOOL) += ethtool-2.6.36

ifeq ($(CONFIG_USER_WT_146),y)
dir_e +=bfd
else
dir_$(CONFIG_USER_WT_146) +=bfd
endif

# Added by Mason Yu for p2r_test
dir_$(CONFIG_USER_MFCV6D) += mfcv6
# Added by Mason Yu for ecmh
dir_$(CONFIG_USER_ECMH) += ecmh

# Added by Tsung-Chan Tsai for jamvm
dir_$(CONFIG_USER_JAMVM) += jamvm-1.5.4
dir_$(CONFIG_APACHE_FELIX_FRAMEWORK) += osgi

# debug scripts
dir_$(CONFIG_USER_DBG_SCRIPTS) += dbg_scripts

#nfbi
dir_$(CONFIG_USER_NFBI) += rtl867x_nfbi

dir_$(CONFIG_USER_RTL8686_IPC_UTIL) += aipc_util

dir_$(CONFIG_USER_TCP2DEV)          += tcp2dev
dir_$(CONFIG_USER_RTK_SYSTEMD)		+= systemd
all:
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	ln -nfs $(DIR_LINUX)/.config rtk_voip/.linux_config;     \
	make -C rtk_voip prepare
endif
ifeq ($(CONFIG_USER_BUSYBOX_BUSYBOX1124),y)
	# generate autoconf.h to be referred by boa/src/LINUX/cli_utility.h
	make -C busybox-1.12.4 include/autoconf.h
endif
#like above busybox1124
ifeq ($(CONFIG_USER_BUSYBOX_BUSYBOX1221),y)
	make -C busybox-1.22.1 include/autoconf.h
endif

	for i in $(dir_v) $(dir_y) $(dir_e); do \
		if [ ! -f $$i/Makefile ]; then \
			if [ "$$i" = "iptables-1.4.18" ]; then \
				cd $$i && ./configure --disable-shared --host=mips-linux --target=mips-linux && cd -; \
			elif [ "$$i" = "ipsec-tools-0.7" ]; then \
				cd $$i && ./configure --host=mips-linux --target=mips-linux --with-kernel-headers="$(ROOTDIR)/$(LINUXDIR)/include" && cd -; \
			else \
				cd $$i && ./configure --host=mips-linux --target=mips-linux && cd -; \
			fi \
		fi; \
		make -C $$i || exit $?; \
	done

romfs:
	for i in $(dir_v) $(sort $(dir_y)) $(dir_e) ; do  make -C $$i romfs || exit $? ; done
	$(STRIP) $(ROMFSDIR)/bin/*

.PHONY: rcX
rcX:
	for i in $(dir_v) $(sort $(dir_y)) $(dir_e) ; do \
		if [ "`grep "^rcX:" $$i/Makefile`" = "rcX:" ]; then \
			 make -C $$i rcX;\
		fi \
	done

dev:
	make -C $(dir_v) dev

%_menuconfig:
	[ ! -d "$(@:_menuconfig=)" ] || make -C $(@:_menuconfig=) menuconfig

%_only:
	if [ -d "$(@:_only=)" ]; then \
		if [ ! -f $(@:_only=)/Makefile ]; then \
			if [ "$(@:_only=)" = "iptables-1.4.18" ]; then \
				cd $(@:_only=) && ./configure --disable-shared --host=mips-linux --target=mips-linux && cd -; \
			elif [ "$(@:_only=)" = "ipsec-tools-0.7" ]; then \
				cd $(@:_only=) && ./configure --host=mips-linux --target=mips-linux --with-kernel-headers="$(ROOTDIR)/$(LINUXDIR)/include" --with-openssl="$(ROOTDIR)/lib/libssl" --enable-security-context=no && cd -; \
			else \
				cd $(@:_only=) && ./configure --host=mips-linux --target=mips-linux && cd -; \
			fi \
		fi; \
		make -C $(@:_only=); \
	fi

%_romfs:
	[ ! -d "$(@:_romfs=)" ] || make -C $(@:_romfs=) romfs

%_clean:
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	ln -nfs $(DIR_LINUX)/.config rtk_voip/.linux_config;     \
	make -C rtk_voip prepare
endif
	[ ! -d "$(@:_clean=)" ] || make -C $(@:_clean=) clean

clean:
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	ln -nfs $(DIR_LINUX)/.config rtk_voip/.linux_config;     \
	make -C rtk_voip prepare
endif
	-for i in $(dir_v) $(sort $(dir_y) $(dir_n) $(dir_) $(dir_e)) ; do \
		[ ! -d $$i ] || make -C $$i clean; \
	done

prune:
	-for i in $(sort $(dir_n) $(dir_)) ; do \
		found=0; \
		for j in $(sort $(dir_y) $(dir_e)) ; do \
			if [ $$i == $$j ]; then \
				found=1; \
			fi; \
		done; \
		if [ $$found == 0 ]; then \
			rm -r $$i; \
		fi; \
	done
