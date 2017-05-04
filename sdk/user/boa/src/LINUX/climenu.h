#include "cli_utility.h"

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

#ifdef DOS_SUPPORT
#define DOSENABLE						0x1
#define DOSSYSFLOODSYN					0x2
#define DOSSYSFLOODFIN					0x4
#define DOSSYSFLOODUDP					0x8
#define DOSSYSFLOODICMP				0x10
#define DOSIPFLOODSYN					0x20
#define DOSIPFLOODFIN					0x40
#define DOSIPFLOODUDP					0x80
#define DOSIPFLOODICMP					0x100
#define DOSTCPUDPPORTSCAN				0x200
#define DOSPORTSCANSENSI				0x800000
#define DOSICMPSMURFENABLED			0x400
#define DOSIPLANDENABLED				0x800
#define DOSIPSPOOFENABLED				0x1000
#define DOSIPTEARDROPENABLED			0x2000
#define DOSPINTOFDEATHENABLED			0x4000
#define DOSTCPSCANENABLED				0x8000
#define DOSTCPSYNWITHDATAENABLED		0x10000
#define DOSUDPBOMBENABLED				0x20000
#define DOSUDPECHOCHARGENENABLED	0x40000
#define DOSSOURCEIPBLOCK				0x400000
#endif

unsigned char first_time=1;
unsigned char dbg_enable=0;
int has_changed;

#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
static const char *g_pszEthMode[] = { "Bridged", "IPoE", "PPPoE", "n/a", "n/a", "n/a" };
#endif
static const char NOT_AUTHORIZED[] = "\nNot Authorized!\n";

static SECURITY_LEVEL loginLevel;	// security level: each login user has its own security level

static int exstat=1;
static void leave(void) __attribute__ ((noreturn));	/* abort cli shell */

enum _SET_3G_SETTINGS_IDX_
{
	SET_3G_SETTINGS_SHOW,
	SET_3G_SETTINGS_MODIFY,
	SET_3G_SETTINGS_QUIT,
	SET_3G_SETTINGS_END
};

static char *strSet3GSettings[]=
{
	"Show",
	"Modify",
	"Quit",
	NULL
};

enum _SET_BFD_SETTING_IDX_
{
	SET_BFD_SETTING_SHOW,
	SET_BFD_SETTING_MODIFY,
	SET_BFD_SETTING_QUIT,
	SET_BFD_SETTING_END
};

static char *strSetBFDSetting[]=
{
	"Show",
	"Modify",
	"Quit",
	NULL
};

const unsigned char *NTPServer[] = {"192.5.41.41", "192.5.41.209", "130.149.17.8", "203.117.180.36"};

typedef enum {
	CLI_ENC_DISABLED,
	CLI_ENC_WEP,
#ifndef NEW_WIFI_SEC
	CLI_ENC_WPA,
#endif
	CLI_ENC_WPA2,
	CLI_ENC_WPA2_MIXED,
#ifdef CONFIG_RTL_WAPI_SUPPORT
	CLI_ENC_WAPI,
#endif
	CLI_ENC_COUNT
} CLI_ENC_T;

enum {
	MENU_WLAN_BASIC = 1,
	MENU_WLAN_ADVANCED,
	MENU_WLAN_SECURITY,
#ifdef WLAN_ACL
	MENU_WLAN_ACC_CTRL,
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	MENU_WLAN_WPS,
#endif
#ifdef WLAN_WDS
	MENU_WLAN_WDS,
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
	MENU_WLAN_SITE_SURVEY,
#endif
#ifdef WLAN_MBSSID
	MENU_WLAN_MBSSID,
#endif
	MENU_WLAN_STATUS,
	MENU_WLAN_QUIT
};

const char *Menu_WLAN[] = {
	"Basic Settings",
	"Advanced Settings",
	"Security",
#ifdef WLAN_ACL
	"Access Control",
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	"WPS",
#endif
#ifdef WLAN_WDS
	"WDS",
#endif
#ifdef WLAN_CLIENT
	"Site Survey",
#endif
#ifdef WLAN_MBSSID
	"Multiple BSSID",
#endif
	"Status",
	"Quit"
};

enum _SET_DSL_CONF_IDX_
{
	SET_DSL_CONF_ADD,
	//SET_DSL_CONF_MODIFY,
	SET_DSL_CONF_DELETE,
#ifdef NEW_PORTMAPPING
	SET_DSL_CONF_PORT_MAPPING,
#endif
	SET_DSL_CONF_SHOW,
	SET_DSL_CONF_CONNECT,
	SET_DSL_CONF_DISCONNECT,
	SET_DSL_CONF_PVCSEARCH,
#ifdef CONFIG_USER_WT_146
	SET_DSL_CONF_BFDSETTING,
#endif //CONFIG_USER_WT_146
	SET_DSL_CONF_QUIT,
	SET_DSL_CONF_END
};

static char *strSetChanConf[]=
{
	"Add",
	//"Modify",
	"Delete",
#ifdef NEW_PORTMAPPING
	"Port Mapping",
#endif
	"Show",
	"Connect",
	"Disconnect",
	"Auto-PVC Search",
#ifdef CONFIG_USER_WT_146
	"BFD Setting",
#endif //CONFIG_USER_WT_146
	"Quit",
	NULL
};

enum _SET_ETHWAN_CONF_IDX_
{
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	SET_ETHWAN_CONF_ADD,
	SET_ETHWAN_CONF_DEL,
#else
	SET_ETHWAN_CONF_SET,
#endif
#ifdef NEW_PORTMAPPING
	SET_ETHWAN_CONF_PORT_MAPPING,
#endif
	SET_ETHWAN_CONF_SHOW,
	SET_ETHWAN_CONF_QUIT,
	SET_ETHWAN_CONF_END
};

static char *strSetEthWanConf[]=
{
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	"Add",
	"Delete",
#else
	"Set",
#endif
#ifdef NEW_PORTMAPPING
	"Port Mapping",
#endif
	"Show",
	"Quit",
	NULL
};

int showItf(unsigned char sel); //0:show all interface without bridge   1:show rip interface 2:show all itf

