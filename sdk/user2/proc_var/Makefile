RC_LEVEL_FIRST=/etc/init.d/rc2
RC_LEVEL_START=/etc/init.d/rc2
RC_LEVEL_PROC=/etc/init.d/rc10
RC_LEVEL_PON_DRV=/etc/init.d/rc21
ifeq ($(CONFIG_RTK_OAM_V1),y)
RC_LEVEL_PON=/etc/init.d/rc22
else
RC_LEVEL_PON=/etc/init.d/rc32
endif
#RC_LEVEL_WDT MUST be greater than RC_LEVEL_PON for OMCI firmware upgrade fail-safe requirement
RC_LEVEL_WDT=/etc/init.d/rc34
RC_LEVEL_IPV6=/etc/init.d/rc14
RC_LEVEL_SLAVE=/etc/init.d/rc3
RC_LEVEL_FINISH=/etc/init.d/rc35

-include $(LINUX_CONFIG)

ifdef CONFIG_MTD_LUNA_NOR_SPI
_CONFIG_LUNA_HAS_NOR=y
endif
ifdef CONFIG_MTD_NOR_SPIF_G3
_CONFIG_LUNA_HAS_NOR=y
endif

all:

romfs:

.PHONY: rcX
rcX:	rcX_kernel
	$(ROMFSINST) -a "/bin/mount -t proc proc /proc" $(RC_LEVEL_FIRST)
	$(ROMFSINST) -a "/bin/mount -t ramfs ramfs /var" $(RC_LEVEL_FIRST)
	$(ROMFSINST) -a "/bin/mount -t sysfs sysfs /sys" $(RC_LEVEL_FIRST)
	$(ROMFSINST) -a "/bin/echo 4096 > /proc/sys/vm/min_free_kbytes" $(RC_LEVEL_FINISH)
ifdef CONFIG_USER_JAMVM
	$(ROMFSINST) -a "jamvm -Xms30M -Xmx60M -Dgosh.args=--noi -jar /usr/local/class/felix/bin/felix.jar /tmp/felix-cache &" $(RC_LEVEL_FINISH)
endif
ifndef CONFIG_USER_NFBI_SLAVE
ifndef CONFIG_ARCH_LUNA_SLAVE
	$(ROMFSINST) -a "/bin/echo /sbin/mdev > /proc/sys/kernel/hotplug" $(RC_LEVEL_FIRST)
endif
	$(ROMFSINST) -a "/bin/mount -t tmpfs mdev /dev" $(RC_LEVEL_FIRST)
ifndef CONFIG_ARCH_LUNA_SLAVE
#	$(ROMFSINST) -a "/bin/mknod /dev/adsl0 c 100 0" $(RC_LEVEL_FIRST)
endif
#	$(ROMFSINST) mdev.conf /etc/mdev.conf
ifdef CONFIG_USER_NFBI_MASTER
	$(ROMFSINST) -a "/bin/mknod /dev/rtl_nfbi c 14 0" $(RC_LEVEL_FIRST)
endif #CONFIG_USER_NFBI_MASTER
ifdef CONFIG_USER_NFBI_SLAVE
	$(ROMFSINST) -a "/bin/mknod /dev/rtl_mdio c 14 0" $(RC_LEVEL_FIRST)
endif #CONFIG_USER_NFBI_SLAVE
	$(ROMFSINST) -a "mdev -s" $(RC_LEVEL_FIRST)
endif

	$(ROMFSINST) -a "mkdir /var/tmp" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/log" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/run" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/lock" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/ppp" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/config" $(RC_LEVEL_START)
ifdef CONFIG_MTD_NAND
ifdef CONFIG_YAFFS_FS
#	$(ROMFSINST) -a "mount -t yaffs2 -o tags-ecc-off /dev/mtdblock6 /var/config/" $(RC_LEVEL_START)
	$(ROMFSINST) mnt_cfgfs.sh /etc/scripts/mnt_cfgfs.sh
	$(ROMFSINST) -a "/etc/scripts/mnt_cfgfs.sh \"-t yaffs2 -o tags-ecc-off\" \"config\" \"/var/config\" || /etc/scripts/mnt_cfgfs.sh \"-t yaffs2 -o tags-ecc-off\" \"rootfs3\" \"/var/config\"" $(RC_LEVEL_START)
ifdef CONFIG_APACHE_FELIX_FRAMEWORK
	$(ROMFSINST) -a "mkdir /var/config_osgi" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mount -t yaffs2 -o tags-ecc-off /dev/mtdblock7 /var/config_osgi/" $(RC_LEVEL_START)
endif
else
ifdef CONFIG_JFFS2_FS
	$(ROMFSINST) -a "mount -t jffs2 /dev/mtdblock1 /var/config/" $(RC_LEVEL_START)
else
	echo "ERROR: No valid file system"
	exit 0
endif
endif
endif

ifdef _CONFIG_LUNA_HAS_NOR
ifdef CONFIG_JFFS2_FS
#	$(ROMFSINST) -e CONFIG_JFFS2_FS -a "mount -t jffs2 /dev/mtdblock1 /var/config/" $(RC_LEVEL_START)
	$(ROMFSINST) mnt_cfgfs.sh /etc/scripts/mnt_cfgfs.sh
	$(ROMFSINST) -a "/etc/scripts/mnt_cfgfs.sh \"-t jffs2\" \"config\" \"/var/config\"" $(RC_LEVEL_START)
else
	echo "ERROR: No valid file system-require JFFS2 enabled in linux kernel"
	exit 1
endif
endif
	$(ROMFSINST) -a "mkdir /var/udhcpd" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/udhcpc" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/mnt" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/ct" $(RC_LEVEL_START)
	$(ROMFSINST) -a "mkdir /var/ct/tmp" $(RC_LEVEL_START)
	$(ROMFSINST) -a "ifconfig lo 127.0.0.1" $(RC_LEVEL_START)
	$(ROMFSINST) -a "route add -net 127.0.0.0 netmask 255.255.255.0 lo" $(RC_LEVEL_START)
	$(ROMFSINST) -a "PATH=.:$\PATH" $(RC_LEVEL_START)
	$(ROMFSINST) -a "cd /etc/scripts" $(RC_LEVEL_START)

ifeq ($(CONFIG_GPON_FEATURE),y)
	$(ROMFSINST) -a "/etc/insdrv.sh" $(RC_LEVEL_PON_DRV)
	$(ROMFSINST) -a "/etc/runsdk.sh" $(RC_LEVEL_PON)
ifeq ($(CONFIG_EUROPA_FEATURE),y)
	$(ROMFSINST) insert_europa.sh /etc/insert_europa.sh
ifeq ($(CONFIG_EUROPA_I2C_PORT_0),y)
	$(ROMFSINST) -a \
	"I2C_PORT=0 INTR_PIN=$(CONFIG_EUROPA_INTERRUPT_GPIO_PIN) TXDIS_PIN=$(CONFIG_TX_DISABLE_GPIO_PIN) TXPWR_PIN=$(CONFIG_TX_POWER_GPO_PIN)" /etc/insert_europa.sh
else
	$(ROMFSINST) -a \
	"I2C_PORT=1 INTR_PIN=$(CONFIG_EUROPA_INTERRUPT_GPIO_PIN) TXDIS_PIN=$(CONFIG_TX_DISABLE_GPIO_PIN) TXPWR_PIN=$(CONFIG_TX_POWER_GPO_PIN)" /etc/insert_europa.sh
endif
	$(ROMFSINST) -a "/etc/insert_europa.sh" $(RC_LEVEL_PON)
endif
else
ifeq ($(CONFIG_EPON_FEATURE),y)
	$(ROMFSINST) -a "/etc/runsdk.sh" $(RC_LEVEL_PON)
else
ifeq ($(CONFIG_FIBER_FEATURE),y)
	$(ROMFSINST) -a "/etc/runsdk.sh" $(RC_LEVEL_PON)
endif
endif
endif
ifeq ($(CONFIG_LAN_SDS_FEATURE),y)
	 $(ROMFSINST) -a "/etc/runlansds.sh" $(RC_LEVEL_PON)
endif

ifeq ($(CONFIG_PON_DETECT_FEATURE),y)
	$(ROMFSINST) -a "/bin/pondetect &" $(RC_LEVEL_PON)
endif

ifeq ($(CONFIG_LUNA_WDT),y)
	$(ROMFSINST) -a "echo 0 > /proc/wdt/enable" $(RC_LEVEL_WDT)
endif

rcX_kernel:
ifdef CONFIG_LUNA_DUAL_LINUX
ifndef CONFIG_ARCH_LUNA_SLAVE
	$(ROMFSINST) -a "echo 'Bootup Slave CPU'" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "/etc/rc_boot_dual_linux" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "mkdir -p /tmp/slave" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -s /var/ethers /etc/ethers
endif
endif
	$(ROMFSINST) -a "echo 200 > /proc/sys/net/core/netdev_max_backlog" $(RC_LEVEL_PROC)
ifndef CONFIG_ARCH_LUNA_SLAVE
	$(ROMFSINST) -a "echo 600 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established" $(RC_LEVEL_PROC)
endif

ifneq ($(CONFIG_MODULE_BUILD_IN),y)
	$(ROMFSINST) -a "/etc/runsdk.sh" /etc/rc
endif

ifdef CONFIG_8M_SDRAM
#	$(ROMFSINST) -a "echo 256 > /proc/sys/net/ipv4/ip_conntrack_max" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 768 > /proc/sys/net/ipv4/route/max_size" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 256 > /proc/sys/net/ipv4/route/gc_thresh" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 1 > /proc/sys/net/ipv4/route/gc_elasticity" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 60 > /proc/sys/net/ipv4/route/gc_interval" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 20 > /proc/sys/net/ipv4/route/gc_timeout" $(RC_LEVEL_PROC)
else
  ifdef CONFIG_MEM_LIMITATION
	$(ROMFSINST) -a "echo 1500 > /proc/sys/net/ipv4/route/max_size" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 1500 > /proc/sys/net/ipv4/inet_peer_threshold" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 20 > /proc/sys/net/ipv4/inet_peer_minttl" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 60 > /proc/sys/net/ipv4/inet_peer_maxttl" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 50 > /proc/sys/net/ipv4/netfilter/ip_conntrack_generic_timeout" $(RC_LEVEL_PROC)
  endif
endif
	$(ROMFSINST) -a "echo 10 > /proc/sys/net/ipv4/tcp_keepalive_intvl" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 3 > /proc/sys/net/ipv4/tcp_keepalive_probes" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 10 > /proc/sys/net/ipv4/tcp_keepalive_time" $(RC_LEVEL_PROC)
	$(ROMFSINST) -a "echo 60 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_time_wait" $(RC_LEVEL_PROC)
ifdef CONFIG_ARCH_LUNA_SLAVE # specific networking interface configuration with Slave CPU
	$(ROMFSINST) -a "echo 'Config Network Interface...'" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "/bin/brctl addbr br0" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "/bin/ifconfig vwlan hw ether $(CONFIG_DEFAULT_SLAVE_IPC_MAC_ADDRESS)" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "/bin/ifconfig vwlan mtu 1496" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "/bin/ifconfig vwlan 10.253.253.2 netmask 255.255.255.252 up" $(RC_LEVEL_SLAVE)
	$(ROMFSINST) -a "arp -s 10.253.253.1 $(CONFIG_DEFAULT_MASTER_IPC_MAC_ADDRESS)" $(RC_LEVEL_SLAVE)
endif

# Added by Mason Yu.
ifdef CONFIG_IPV6
	$(ROMFSINST) -a "echo 2 > /proc/sys/net/ipv6/conf/default/accept_dad" $(RC_LEVEL_IPV6)
endif

ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	$(ROMFSINST) fwu_starter.sh /etc/scripts/fwu_starter.sh
endif

ifdef CONFIG_RGMII_RESET_PROCESS
	$(ROMFSINST) -a "echo 'Enable RGMII Port...'" $(RC_LEVEL_FINISH)
	$(ROMFSINST) -a "echo 1 > /proc/rgmii_rst" $(RC_LEVEL_FINISH)
	$(ROMFSINST) -a "cp /etc/topaz-linux.lzma.img /tmp/topaz-linux.lzma.img" $(RC_LEVEL_FINISH)
endif

ifdef CONFIG_SWITCH_INIT_LINKDOWN
	$(ROMFSINST) -a "echo 'Turn on phy power...'" $(RC_LEVEL_FINISH)
	$(ROMFSINST) -a "bin/diag port set phy-force-power-down port all state disable" $(RC_LEVEL_FINISH)
endif
	$(ROMFSINST) reset_default.sh /etc/scripts/reset_default.sh
	$(ROMFSINST) chk_swver.sh /etc/scripts/chk_swver.sh
	$(ROMFSINST) -a "/etc/scripts/chk_swver.sh" $(RC_LEVEL_PON_DRV)

clean:

