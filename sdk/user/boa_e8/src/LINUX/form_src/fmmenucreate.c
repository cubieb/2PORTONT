/*
*  fmmenucreate.c is used to create menu
*  added by xl_yue
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../webs.h"
#include "boa.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
//add by ramen to include the autoconf.h created by kernel
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif
#include <rtk/options.h>

#define BEGIN_CATALOG(name)  	boaWrite (wp, "mncata = new menu(\"%s\");", name)
#define FLUSH_CATALOG()  		boaWrite (wp, "mnroot.add(mncata);")

#define BEGIN_MENU(name)  		boaWrite (wp, "mnpage = new menu(\"%s\");", name)
#define ADD_MENU(link, page)  		boaWrite (wp, "mnpage.add(\"%s\",\"%s\");", link, page)
#define END_MENU()  				boaWrite (wp, "mncata.add(mnpage);")

#ifdef E8B_NEW_DIAGNOSE
//Added by robin, for diagnosis pages
int createMenuDiag(int eid, request * wp, int argc, char **argv)
{
	boaWrite(wp, "var mncata = null;\n");
	boaWrite(wp, "var mnpage = null;\n");

	//状态
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("状态");

	BEGIN_MENU("设备信息");
	ADD_MENU("diag_dev_basic_info.asp", "设备基本信息");
	END_MENU();

	BEGIN_MENU("网络侧信息");
	ADD_MENU("diag_net_connect_info.asp", "连接信息");
	ADD_MENU("diag_net_dsl_info.asp", "DSL信息");
	END_MENU();

	BEGIN_MENU("用户侧信息");
#ifdef WLAN_SUPPORT
	ADD_MENU("diag_wlan_info.asp", "WLan接口信息");
#endif
	ADD_MENU("diag_ethernet_info.asp", "以太网口信息");
	ADD_MENU("diag_usb_info.asp", "USB接口信息");
	END_MENU();

	BEGIN_MENU("远程管理状态");
	ADD_MENU("status_tr069_info.asp", "远程连接建立状态");
	ADD_MENU("status_tr069_config.asp", "业务配置下发状态");
	END_MENU();

	FLUSH_CATALOG();

	BEGIN_CATALOG("诊断");

	BEGIN_MENU("诊断测试");
	ADD_MENU("diag_ping.asp", "PING测试");
	ADD_MENU("diag_tracert.asp", "Tracert测试");
	ADD_MENU("diagnose_tr069.asp", "手动上报 Inform");
	END_MENU();

	FLUSH_CATALOG();

}
#endif
int createMenuEx(int eid, request * wp, int argc, char **argv)
{
#if defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE)
	unsigned int pon_mode;

	mib_get(MIB_PON_MODE, &pon_mode);
#endif
	struct user_info *pUser_info;

	pUser_info = search_login_list(wp);

	if (!pUser_info)
		return -1;

	boaWrite(wp, "var mncata = null;\n");
	boaWrite(wp, "var mnpage = null;\n");

	//状态
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("状态");	//user

	BEGIN_MENU("设备信息");	//user
	ADD_MENU("status_device_basic_info.asp", "设备基本信息");
	END_MENU();

	BEGIN_MENU("网络侧信息");	//user
	if (pUser_info->priv) {	//admin
		ADD_MENU("status_net_connet_info.asp", "IPv4连接信息");
		ADD_MENU("status_net_connet_info_ipv6.asp", "IPv6连接信息");
#ifdef SUPPORT_WAN_BANDWIDTH_INFO
		ADD_MENU("status_wan_bandwidth.asp", "WAN带宽信息");
#endif
	} else {
		ADD_MENU("status_user_net_connet_info.asp", "IPv4连接信息");
		ADD_MENU("status_user_net_connet_info_ipv6.asp",
			 "IPv6连接信息");
	}

#ifdef CONFIG_DEV_xDSL
	if (pUser_info->priv)	//admin
		ADD_MENU("status_net_dsl_info.asp", "DSL信息");	//user- new tech spec
	else
		ADD_MENU("status_user_net_dsl_info.asp", "DSL信息");	//user- new tech spec
#endif

#ifdef CONFIG_EPON_FEATURE
	if (pon_mode == EPON_MODE)
		ADD_MENU("status_epon.asp", "EPON 信息");
#endif

#ifdef CONFIG_GPON_FEATURE
	if (pon_mode == GPON_MODE)
		ADD_MENU("status_gpon.asp", "GPON 信息");
#endif

	END_MENU();

	BEGIN_MENU("用户侧信息");	//user
#ifdef WLAN_SUPPORT
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	ADD_MENU("status_wlan_info_11n.asp", "WLAN接口信息");
#else
	ADD_MENU("status_wlan_info.asp", "WLAN接口信息");
#endif
#endif
	ADD_MENU("status_ethernet_info.asp", "以太网接口信息");

#ifdef CONFIG_USER_LANNETINFO
		ADD_MENU("status_lan_net_info.asp", "下挂设备信息");
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
{
	unsigned char vChar=0;
	mib_get(MIB_LANHOST_BANDWIDTH_MONITOR_ENABLE, (void*)&vChar);
	if(vChar)
		ADD_MENU("status_lan_bandwidth_monitor.asp", "下挂设备带宽监测信息");
}
#endif

#ifdef USB_SUPPORT
	ADD_MENU("status_usb_info.asp", "USB接口信息");
#endif
	END_MENU();

#ifdef VOIP_SUPPORT
	//SD6-bohungwu, e8c voip
	BEGIN_MENU("宽带语音信息");
	ADD_MENU("status_voip_info.asp", "宽带语音信息");
	END_MENU();
#endif //#ifdef VOIP_SUPPORT

#ifdef E8B_NEW_DIAGNOSE
	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("远程管理状态");
		ADD_MENU("status_tr069_info_admin.asp", "远程连接建立状态");
		ADD_MENU("status_tr069_config_admin.asp", "业务配置下发状态");
		END_MENU();
	}
#endif

	FLUSH_CATALOG();
	//modify end by liuxiao 2008-01-23

	//网  络
	BEGIN_CATALOG("网  络");	//user

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("宽带设置");
#if defined(CONFIG_ETHWAN)
		ADD_MENU
		    ("boaform/formWanRedirect?redirect-url=/net_eth_links.asp&if=eth",
		     "Internet 连接");
#endif
#if defined(CONFIG_PTMWAN)
		ADD_MENU
		    ("boaform/formWanRedirect?redirect-url=/net_eth_links.asp&if=ptm",
		     "Internet PTM 连接");
#endif
#if defined(CONFIG_RTL8672_SAR)
		ADD_MENU("net_adsl_links.asp", "Internet PVC 连接");
#endif
		END_MENU();
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
		BEGIN_MENU("绑定设置");
		ADD_MENU("net_vlan_mapping.asp", "绑定模式");
		END_MENU();
#endif
	}

	BEGIN_MENU("LAN侧地址配置");
	ADD_MENU("net_dhcpd.asp", "IPv4配置");
	ADD_MENU("ipv6.asp", "IPv6 配置");
	ADD_MENU("dhcpdv6.asp", "IPv6 DHCP Server配置");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("radvdconf.asp", "RA 配置");
	}
	END_MENU();
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	if (pUser_info->priv){

		BEGIN_MENU("VPN WAN"); //user
		ADD_MENU("pptp.asp", "PPTP");	
		ADD_MENU("l2tp.asp", "L2TP");	
		END_MENU();
	}
#endif

#ifdef WLAN_SUPPORT
	BEGIN_MENU("WLAN配置");	//user
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	if (pUser_info->priv)	//admin
		ADD_MENU("net_wlan_basic_11n.asp", "WLAN配置");
	else
		ADD_MENU("net_wlan_basic_user_11n.asp", "WLAN配置");
#else
	if (pUser_info->priv)	//admin
		ADD_MENU("net_wlan_basic.asp", "WLAN配置");
	else
		ADD_MENU("net_wlan_basic_user.asp", "WLAN配置");
#endif
#ifdef WIFI_TIMER_SCHEDULE
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("net_wlan_sched.asp", "开关定时");
		ADD_MENU("net_wlan_timer.asp", "开关定时(周期)");
	}
#endif
	END_MENU();
#endif
	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("远程管理");
		ADD_MENU("net_tr069.asp", "ITMS服务器");
		ADD_MENU("net_certca.asp", "上传CA证书");
#ifdef CONFIG_MIDDLEWARE
		ADD_MENU("net_midware.asp", "中间件配置");
#endif
		ADD_MENU("usereg_inside_menu.asp", "逻辑ID注册");
		END_MENU();

		BEGIN_MENU("QoS");
		//ADD_MENU("net_qos_queue.asp", "队列配置");
	/*
#ifndef QOS_SETUP_IMQ
		ADD_MENU("net_qos_policy.asp", "策略配置");
#else
		ADD_MENU("net_qos_imq_policy.asp", "策略配置");
#endif
*/
		ADD_MENU("net_qos_imq_policy.asp", "策略配置");
		ADD_MENU("net_qos_cls.asp", "QoS分类");
//		ADD_MENU("net_qos_app.asp", "QoS业务");
		ADD_MENU("net_qos_traffictl.asp", "流量控制");
		END_MENU();

		BEGIN_MENU("时间管理");
		ADD_MENU("net_sntp.asp", "时间服务器");
		END_MENU();

		BEGIN_MENU("路由配置");
		// Mason Yu. 2630-e8b
		ADD_MENU("rip.asp", "动态路由");
		// Mason Yu. 2630-e8b
		ADD_MENU("routing.asp", "静态路由");
		END_MENU();
	}

	FLUSH_CATALOG();

	//安  全
	BEGIN_CATALOG("安  全");	//user

	BEGIN_MENU("广域网访问设置");	//user
	ADD_MENU("secu_urlfilter_cfg.asp", "广域网访问设置");
	END_MENU();

	BEGIN_MENU("防火墙");
	ADD_MENU("secu_firewall_level.asp", "安全级");	//user
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("secu_firewall_dosprev.asp", "攻击保护设置");
	}
	END_MENU();

	BEGIN_MENU("MAC过滤");	//user
#ifdef	MAC_FILTER_SRC_ONLY
	ADD_MENU("secu_macfilter_src.asp", "MAC过滤");
#else
	ADD_MENU("secu_macfilter_bridge.asp", "桥接MAC过滤");
	ADD_MENU("secu_macfilter_router.asp", "路由MAC过滤");
#endif
	END_MENU();

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("端口过滤");
		ADD_MENU("secu_portfilter_cfg.asp", "端口过滤");
		END_MENU();
	}

	FLUSH_CATALOG();

	//应  用
	BEGIN_CATALOG("应  用");	//user

	if (pUser_info->priv)	//admin
	{
#ifdef CONFIG_RG_SLEEPMODE_TIMER
		BEGIN_MENU("网关休眠配置");
		ADD_MENU("app_sleepmode_rule.asp", "网关休眠配置");
		END_MENU();
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER
		BEGIN_MENU("网关LED配置");
		ADD_MENU("app_led_sched.asp", "网关LED配置");
		END_MENU();
#endif
		BEGIN_MENU("DDNS配置");
		ADD_MENU("app_ddns_show.asp", "DDNS配置");
		END_MENU();

		BEGIN_MENU("高级NAT配置");
		// Mason Yu. 2630-e8b
		ADD_MENU("algonoff.asp", "ALG配置");
		ADD_MENU("fw-dmz.asp", "DMZ配置");
		ADD_MENU("app_nat_vrtsvr_cfg.asp", "虚拟主机配置");
#if 0
		ADD_MENU("app_nat_porttrig_show.asp", "端口触发");
#endif
		END_MENU();

		BEGIN_MENU("UPNP配置");
		ADD_MENU("app_upnp.asp", "UPNP配置");
#ifdef CONFIG_USER_MINIDLNA
		ADD_MENU("dms.asp", "DLNA配置");
#endif
		END_MENU();

		//SD6-bohungwu, e8c voip
#ifdef VOIP_SUPPORT
		BEGIN_MENU("宽带电话设置");
		ADD_MENU("app_voip.asp", "宽带电话设置");
		ADD_MENU("app_voip2.asp", "宽带电话高级设置");
		END_MENU();
#endif //#ifdef VOIP_SUPPORT

		BEGIN_MENU("IGMP设置");
		ADD_MENU("app_igmp_snooping.asp", "IGMP SNOOPING");
		ADD_MENU("app_igmp_proxy.asp", "IGMP Proxy ");
		END_MENU();

		// Mason Yu. MLD Proxy
		BEGIN_MENU("MLD配置");
		ADD_MENU("app_mld_snooping.asp", "MLD SNOOPING配置");	// Mason Yu. MLD snooping for e8b
		ADD_MENU("app_mldProxy.asp", "MLD Proxy配置");
		END_MENU();
		
#if defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR) || defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)
		BEGIN_MENU("下挂终端配置");
#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
		ADD_MENU("app_bandwidth_monitor.asp", "上下行带宽监测");
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
		ADD_MENU("app_bandwidth_control.asp", "上下行带宽限制");
#endif
		ADD_MENU("app_bandwidth_interval.asp", "带宽计算周期");
		END_MENU();
#endif	// end of (CONFIG_USER_LAN_BANDWIDTH_MONITOR) || defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)		
	}

	BEGIN_MENU("日常应用");	//user
#ifdef USB_SUPPORT
	ADD_MENU("app_storage.asp", "家庭存储");
#endif
#ifdef CONFIG_MCAST_VLAN
	if (pUser_info->priv)
		ADD_MENU("app_iptv.asp", "IPTV");
#endif
	END_MENU();

#ifndef USB_SUPPORT
if(pUser_info->priv)
#endif
	FLUSH_CATALOG();

	//管  理
	BEGIN_CATALOG("管  理");	//user

	BEGIN_MENU("用户管理");	//user
	ADD_MENU("mgm_usr_user.asp", "用户管理");
	END_MENU();

	BEGIN_MENU("设备管理");
	ADD_MENU("mgm_dev_reboot.asp", "设备重启");	//user
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("mgm_dev_reset.asp", "恢复出厂设置");
	}
#ifdef USB_SUPPORT
	ADD_MENU("mgm_dev_usbbak.asp", "USB备份配置");
	ADD_MENU("mgm_dev_usb_umount.asp", "USB卸载");
#endif
	END_MENU();

	if (pUser_info->priv)	//admin
	{
		BEGIN_MENU("日志文件管理");
		ADD_MENU("mgm_log_cfg.asp", "写入等级设置");
		ADD_MENU("mgm_log_view.asp", "设备日志");
		END_MENU();

		BEGIN_MENU("维护");
		ADD_MENU("mgm_mnt_mnt.asp", "维护");
		END_MENU();
	}

	FLUSH_CATALOG();

#ifdef E8B_NEW_DIAGNOSE
	if (pUser_info->priv)	//admin
	{
		//诊断
		BEGIN_CATALOG("诊断");

		BEGIN_MENU("网络诊断");
#ifdef CONFIG_RTL8672_SAR
		ADD_MENU("diag_f5loop_admin.asp", "线路测试");
#endif
		ADD_MENU("diag_ping_admin.asp", "PING测试");
		ADD_MENU("diag_tracert_admin.asp", "Tracert测试");
		ADD_MENU("diagnose_tr069_admin.asp", "手动上报 Inform");
		END_MENU();

#ifdef CONFIG_USER_RTK_LBD
		BEGIN_MENU("环路检测");
		ADD_MENU("diag_loopback_detect.asp", "环路检测");
		END_MENU();
#endif

		FLUSH_CATALOG();
	}
#endif

	//帮  助
	//modify by liuxiao 2008-01-23
	BEGIN_CATALOG("帮  助");	//user

	BEGIN_MENU("状态帮助");
	ADD_MENU("/help/help_status_device.html", "设备信息帮助");
	ADD_MENU("/help/help_status_net.asp", "网络侧信息帮助");
	ADD_MENU("/help/help_status_user.html", "用户侧信息帮助");
#ifdef VOIP_SUPPORT
	ADD_MENU("/help/help_status_voip.html", "宽带语音信息帮助");
#endif
	END_MENU();

	BEGIN_MENU("网络帮助");
	if (pUser_info->priv)	//admin
	{
#ifdef CONFIG_DEV_xDSL
		ADD_MENU("/help/help_net_broadband.html", "宽带设置帮助");
#elif defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		ADD_MENU("/help/help_net_pon.html", "宽带设置帮助");
#endif
		ADD_MENU("/help/help_net_dhcp.html", "DHCP设置帮助");
	}
#ifdef WLAN_SUPPORT
	ADD_MENU("/help/help_net_wlan.html", "WLAN配置帮助");
#endif
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_net_remote.html", "远程管理帮助");
		ADD_MENU("/help/help_net_qos.html", "QoS帮助");
		ADD_MENU("/help/help_net_time.html", "时间管理帮助");
		ADD_MENU("/help/help_net_route.html", "路由配置帮助");
	}
	END_MENU();

	BEGIN_MENU("安全帮助");
	ADD_MENU("/help/help_security_wanaccess.html", "广域网访问设置帮助");
	ADD_MENU("/help/help_security_firewall.html", "防火墙帮助");
	ADD_MENU("/help/help_security_macfilter.html", "MAC过滤帮助");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_security_portfilter.html", "端口过滤帮助");
	}
	END_MENU();

	BEGIN_MENU("应用帮助");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_apply_ddns.html", "DDNS配置帮助");
		ADD_MENU("/help/help_apply_nat.html", "高级NAT配置帮助");
		ADD_MENU("/help/help_apply_upnp.html", "UPNP配置帮助");
#ifdef VOIP_SUPPORT
		ADD_MENU("/help/help_apply_voip.html", "宽带电话设置帮助");
#endif
		ADD_MENU("/help/help_apply_igmp.html", "IGMP设置帮助");
	}
#ifdef USB_SUPPORT
	ADD_MENU("/help/help_apply_familymemory.html", "家庭存储帮助");
#endif
	END_MENU();

	BEGIN_MENU("管理帮助");
	ADD_MENU("/help/help_manage_user.html", "用户管理帮助");
	ADD_MENU("/help/help_manage_device.html", "设备管理帮助");
	if (pUser_info->priv)	//admin
	{
		ADD_MENU("/help/help_manage_logfile.html", "日志文件管理帮助");
		ADD_MENU("/help/help_manage_keep.html", "维护帮助");
	}
	END_MENU();

	FLUSH_CATALOG();
	//modify end by liuxiao 2008-01-23

	return 0;
}
