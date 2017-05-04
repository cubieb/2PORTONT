/*

*  fmmenucreate_user.c is used to create menu

*  added by xl_yue

*/





#include <string.h>

#include <stdlib.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/wait.h>



#include "../webs.h"

#include "mib.h"

#include "webform.h"

#include "utility.h"

#include "multilang.h"



#ifdef CONFIG_DEFAULT_WEB	// default pages

/*

 *	Second Layer Menu

 */

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
struct RootMenu childmenu_wlan0_user[] = {

	{"Basic Settings", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlbasic.asp&wlan_idx=0",	"Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},

	{"Advanced Settings", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wladvanced.asp&wlan_idx=0",   "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},

	{"Security", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwpa.asp&wlan_idx=0", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},

#ifdef WLAN_11R
	{"Fast Roaming", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlft.asp&wlan_idx=0", "Fast BSS Transition", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif

#ifdef WLAN_ACL

	{"Access Control", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlactrl.asp&wlan_idx=0",	"Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},

#endif

#ifdef WLAN_WDS

	{"WDS", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwds.asp&wlan_idx=0", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},

#endif

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)

	{"Site Survey", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlsurvey.asp&wlan_idx=0", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},

#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS

	{"WPS", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwps.asp&wlan_idx=0", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},

#endif

	{"Status", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlstatus.asp&wlan_idx=0", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},

	{0, 0, 0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_wlan1_user[] = {

	{"Basic Settings", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlbasic.asp&wlan_idx=1",	"Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},

	{"Advanced Settings", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wladvanced.asp&wlan_idx=1",   "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},

	{"Security", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwpa.asp&wlan_idx=1", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},

#ifdef WLAN_11R
	{"Fast Roaming", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlft.asp&wlan_idx=1", "Fast BSS Transition (user)", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif

#ifdef WLAN_ACL

	{"Access Control", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlactrl.asp&wlan_idx=1",	"Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},

#endif

#ifdef WLAN_WDS

	{"WDS", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwds.asp&wlan_idx=1", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},

#endif

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)

	{"Site Survey", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlsurvey.asp&wlan_idx=1", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},

#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS

	{"WPS", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlwps.asp&wlan_idx=1", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},

#endif

	{"Status", MENU_URL, "../boaform/admin/formWlanRedirect?redirect-url=/admin/wlstatus.asp&wlan_idx=1", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},

	{0, 0, 0, 0, 0, 0, 0, 0}

};
#endif //CONFIG_RTL_92D_SUPPORT

/*

 *	First Layer Menu

 */

struct RootMenu childmenu_status_user[] = {
	{"Device", MENU_URL, "status.asp", "Device status", 0, 0, MENU_DISPLAY, LANG_DEVICE},
#ifdef CONFIG_IPV6
	{"IPv6", MENU_URL, "status_ipv6.asp", "IPv6 status", 0, 0, MENU_DISPLAY, LANG_IPV6},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};

struct RootMenu childmenu_wlan_user[] = {

#if defined(CONFIG_RTL_92D_SUPPORT)
  {"Wireless Band Mode", MENU_URL, "wlbandmode.asp",   "Setup wireless band mode", 0, 0, MENU_DISPLAY, LANG_WIRELESS_BAND_MODE},
  {"wlan0 (5GHz)", MENU_FOLDER, &childmenu_wlan0_user, "", sizeof (childmenu_wlan0_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_5GHZ},
  {"wlan1 (2.4GHz)", MENU_FOLDER, &childmenu_wlan1_user, "", sizeof (childmenu_wlan1_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_2_4GHZ},
#elif defined (WLAN_DUALBAND_CONCURRENT)
#if defined (CONFIG_WLAN0_2G_WLAN1_5G) || defined(WLAN1_QTN)
  {"wlan0 (2.4GHz)", MENU_FOLDER, &childmenu_wlan0_user, "", sizeof (childmenu_wlan0_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_2_4GHZ},
  {"wlan1 (5GHz)", MENU_FOLDER, &childmenu_wlan1_user, "", sizeof (childmenu_wlan1_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_5GHZ},
#else
  {"wlan0 (5GHz)", MENU_FOLDER, &childmenu_wlan0_user, "", sizeof (childmenu_wlan0_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_5GHZ},
  {"wlan1 (2.4GHz)", MENU_FOLDER, &childmenu_wlan1_user, "", sizeof (childmenu_wlan1_user) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_2_4GHZ},
#endif
#else //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT || CONFIG_MASTER_WLAN0_ENABLE || CONFIG_SLAVE_WLAN1_ENABLE

	{"Basic Settings", MENU_URL, "wlbasic.asp", "Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},

	{"Advanced Settings", MENU_URL, "wladvanced.asp", "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},

	{"Security", MENU_URL, "wlwpa.asp", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},

#ifdef WLAN_11R
	{"Fast Roaming", MENU_URL, "wlft.asp", "Fast BSS Transition", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif

#ifdef WLAN_ACL

	{"Access Control", MENU_URL, "wlactrl.asp", "Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},

#endif

#ifdef WLAN_WDS

	{"WDS", MENU_URL, "wlwds.asp", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},

#endif

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)

	{"Site Survey", MENU_URL, "wlsurvey.asp", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},

#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS

	{"WPS", MENU_URL, "wlwps.asp", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},

#endif

	{"Status", MENU_URL, "wlstatus.asp", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},

#endif //CONFIG_RTL_92D_SUPPORT

	{0, 0, 0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_wan_user[] = {

#ifdef CONFIG_ETHWAN
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	{PON_CONF_STR, MENU_URL, "../boaform/admin/formWanRedirect?redirect-url=/admin/multi_wan_generic.asp&if=pon", "PON WAN Configuration", 0, 0, MENU_DISPLAY, LANG_PON_WAN},
#else
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	{ETHWAN_CONF_STR, MENU_URL, "../boaform/admin/formWanRedirect?redirect-url=/admin/multi_wan_generic.asp&if=eth", "ETH WAN Configuration", 0, 0, MENU_DISPLAY, LANG_ETHERNET_WAN},
#else
	{ETHWAN_CONF_STR, MENU_URL, "waneth.asp", "Ethernet WAN Configuration", 0, 0, MENU_DISPLAY, LANG_ETHERNET_WAN},
#endif
#endif
#endif
#ifdef CONFIG_PTMWAN
	{PTMWAN_CONF_STR, MENU_URL, "../boaform/admin/formWanRedirect?redirect-url=/admin/multi_wan_generic.asp&if=ptm", "PTM WAN Configuration", 0, 0, MENU_DISPLAY, LANG_PTM_WAN},
#endif /*CONFIG_PTMWAN*/
#ifdef CONFIG_RTL8672_SAR
  {DSLWAN_CONF_STR, MENU_URL, "wanadsl.asp", "ADSL Channel Configuration", 0, 0, MENU_DISPLAY, LANG_ATM_WAN},
  {ATM_SETTINGS_STR, MENU_URL, "wanatm.asp", "Setup ATM", 0, 0, MENU_DISPLAY, LANG_ATM_SETTINGS},
  {ADSL_SETTINGS_STR, MENU_URL, "/admin/adsl-set.asp", "Setup ADSL", 0, 0, MENU_DISPLAY, LANG_DSL_SETTINGS},
  #ifdef CONFIG_DSL_VTUO
  {VTUO_SETTINGS_STR, MENU_URL, "/admin/vtuo-set.asp", "Setup VTU-O DSL", 0, 0, MENU_DISPLAY, LANG_VTUO_SETTINGS},
  #endif /*CONFIG_DSL_VTUO*/
#endif
#ifdef CONFIG_USER_PPPOMODEM
	{"3G Settings", MENU_URL, "wan3gconf.asp", "Setup 3G WAN", 0, 0, MENU_DISPLAY, LANG_3G_SETTINGS},
#endif //CONFIG_USER_PPPOMODEM

	{0, 0, 0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_fw_user[] = {

#ifdef MAC_FILTER
#ifdef CONFIG_RTK_RG_INIT
	{"MAC Filtering", MENU_URL, "fw-macfilter_rg.asp", "Setup MAC filering", 0, 0, MENU_DISPLAY, LANG_MAC_FILTERING},
#else
	{"MAC Filtering", MENU_URL, "fw-macfilter.asp", "Setup MAC filering", 0, 0, MENU_DISPLAY, LANG_MAC_FILTERING},
#endif
#endif

	{0, 0, 0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_admin_user[] = {

	{"Commit/Reboot", MENU_URL, "reboot.asp", "Commit/reboot the system", 0, 0, MENU_DISPLAY, LANG_COMMIT_REBOOT},
#ifdef CONFIG_USER_BOA_WITH_MULTILANG
	{"Multi-lingual Settings", MENU_URL, "multi_lang.asp", "Multi-language setting", 0, 0, MENU_DISPLAY, LANG_MULTI_LINGUAL_SETTINGS},
#endif
#ifdef ACCOUNT_LOGIN_CONTROL

	{"Logout", MENU_URL, "/admin/adminlogout.asp", "Logout", 0, 0, MENU_DISPLAY, LANG_LOGOUT},

#endif

	{"Password", MENU_URL, "/admin/user-password.asp", "Setup access password", 0, 0, MENU_DISPLAY, LANG_PASSWORD},

#ifdef IP_ACL

	{"ACL Config", MENU_URL, "acl.asp", "ACL Setting", 0, 0, MENU_DISPLAY, LANG_ACL_CONFIG},

#endif

//added by xl_yue

#ifdef USE_LOGINWEB_OF_SERVER

	{"Logout", MENU_URL, "/admin/logout.asp", "Logout", 0, 0, MENU_DISPLAY, LANG_LOGOUT},

#endif

	{0, 0, 0, 0, 0, 0, 0, 0}

};



/*

 *	Root Menu

 */

struct RootMenu rootmenu_user[] = {

	{"Status", MENU_FOLDER, &childmenu_status_user, "", sizeof(childmenu_status_user) / sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_STATUS},

#ifdef WLAN_SUPPORT

	{"Wireless", MENU_FOLDER, &childmenu_wlan_user, "", sizeof(childmenu_wlan_user) / sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WIRELESS},

#endif

	{"WAN", MENU_FOLDER, &childmenu_wan_user, "", sizeof(childmenu_wan_user) / sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WAN},

	{"Firewall", MENU_FOLDER, &childmenu_fw_user, "", sizeof(childmenu_fw_user) / sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_FIREWALL},

	{"Admin", MENU_FOLDER, &childmenu_admin_user, "", sizeof(childmenu_admin_user) / sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_ADMIN},

	{0, 0, 0, 0, 0, 0, 0, 0}

};

#endif				// of CONFIG_DEFAULT_WEB


int createMenu_user(int eid, request * wp, int argc, char ** argv)
{
	int i = 0, totalIdNums = 0, maxchildrensize = 0;

	int IdIndex = 0;

	unsigned char isRootMenuEnd = 0;

#ifdef CONFIG_RTL_92D_SUPPORT

	wlanMenuUpdate(rootmenu);

	wlanMenuUpdate(rootmenu_user);

#endif //CONFIG_RTL_92D_SUPPORT


	//calc the id nums and the max children size

	totalIdNums = calcFolderNum(rootmenu_user, &maxchildrensize);

	//product the js code

	addMenuJavaScript(wp, totalIdNums, maxchildrensize);

	//create the header
/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#ifdef CONFIG_11N_SAGEM_WEB
  boaWrite (wp, "<body  onload=\"initIt()\" bgcolor=\"#FFFFFF\" >\n");
#else
  boaWrite (wp, "<body  onload=\"initIt()\" bgcolor=\"#000000\" >\n");
#endif
	boaWrite(wp, "<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td  width=100%% align=left>\n");

	boaWrite(wp, "<table border=0 cellpadding=0 cellspacing=0>\n" "<tr><td width=18 height=18><img src=menu-images/menu_root.gif width=18 height=18></td>\n" "<td  height=18 colspan=4 class=link><font size=3>%s:</font></td></tr>\n</table>\n", multilang(LANG_SITE_CONTENTS));



	if (rootmenu_user[1].u.addr)

		addMenu(wp, &rootmenu_user[0], 0, &IdIndex, 0);

	else

		addMenu(wp, &rootmenu_user[0], 0, &IdIndex, 1);



	boaWrite(wp, "</td></tr>\n</table>\n");


	return 0;
}
