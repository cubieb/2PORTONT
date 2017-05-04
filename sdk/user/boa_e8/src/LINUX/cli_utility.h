#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_bridge.h>
#include	<sys/param.h>
#include <net/route.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
#include "../md5.h"
#endif //USE_LIBMD5
#include <crypt.h>
#include <time.h>
#include <sys/stat.h>
/*ping_zhang:20090312 START:add STB checkbox for designate STB device type*/
#ifdef CONFIG_USER_UDHCP099PRE2
#include "../../../udhcp-0.9.9-pre2/dhcpd.h"
#else
#include "../../../udhcp-0.9.9-pre/dhcpd.h"
#endif
/*ping_zhang:20090312 END*/

#ifdef CONFIG_USER_BUSYBOX_BUSYBOX1124
#include "../../../busybox-1.12.4/include/autoconf.h"
#endif

#define CLIDEBUG printf
// Kaohj, defined if you want to clear the screen
#define CLEAR_SCR

#ifdef CLEAR_SCR
#define CLEAR		printf("\033[H\033[J")
#else
#define CLEAR		(void)0
#endif
#define MENU_LINE printf("%s\n", menuBar);
#define MSG_LINE printf("%s\n", messageBar);

#define FALSE 0
#define TRUE  1

//static char strbuf[256];
static const char *menuBar = "===============================================================================";
static const char *messageBar = "-------------------------------------------------------------------------------";
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLE[] = "Disable";
static const char IF_ENABLE[]= "Enable";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";
static const char *strQos[] = { "UBR", "CBR", "nrt-VBR", "rt-VBR" };
static const char *g_pszMode[] = { "br1483", "mer1483", "PPPoE", "PPPoA", "rt1483"
#ifdef CONFIG_ATM_CLIP
	, "rt1577"
#endif
#ifdef DUAL_STACK_LITE
#endif
};

// Mason Yu
char suName[MAX_NAME_LEN];
char usName[MAX_NAME_LEN];

typedef enum {
	SECURITY_ROOT,
	SECURITY_SUPERUSER,
	SECURITY_USER
} SECURITY_LEVEL;

// get typed input.
// return (0) if user enter nothing.
//        (1) if input is valid.
enum {
	INPUT_TYPE_UINT,
	INPUT_TYPE_STRING,
	INPUT_TYPE_IPADDR,
	INPUT_TYPE_IPMASK,
	INPUT_TYPE_IP6ADDR,
	INPUT_TYPE_ETHADDR,
	INPUT_TYPE_INT
};

enum _SET_WAN_ITF_IDX_
{
	SET_WAN_ITF_WAN_MODE,
	SET_WAN_ITF_ETH_CONFIG,
	SET_WAN_ITF_PTM_CONFIG,
	SET_WAN_ITF_DSL_CONFIG,
	SET_WAN_ITF_ATM_SETTINGS,
	SET_WAN_ITF_ADSL_SETTINGS,
#ifdef CONFIG_USER_PPPOMODEM
	SET_WAN_ITF_3G_SETTINGS,
#endif //CONFIG_USER_PPPOMODEM
	SET_WAN_ITF_QUIT,
	SET_WAN_ITF_IDX_END
};

static const char *strSetWANInterface[] =
{
	"WAN Mode",
	"Ethernet Config",
	"PTM Config",
#ifdef CONFIG_VDSL
	"ATM Config",
#else
	"DSL Config",
#endif /*CONFIG_VDSL*/
	"ATM Settings",
#ifdef CONFIG_VDSL
	"DSL Settings",
#else
	"ADSL Settings",
#endif /*CONFIG_VDSL*/
#ifdef CONFIG_USER_PPPOMODEM
	"3G Settings",
#endif //CONFIG_USER_PPPOMODEM
	"Quit",
	NULL
};

const char *WANMenu[SET_WAN_ITF_IDX_END+1];
int wan_menu_idx[SET_WAN_ITF_IDX_END];

enum MENU_DIAGNOSTICS
{
	MENU_DIAG_PING,
	MENU_DIAG_ATM_LOOPBACK,
	MENU_DIAG_ADSL_TONE,
#ifdef CONFIG_USER_XDSL_SLAVE
	MENU_DIAG_ADSL_SLV_TONE,
#endif /*CONFIG_USER_XDSL_SLAVE*/
	MENU_DIAG_ADSL_CONNECTION,
	MENU_DIAG_QUIT,
	MENU_DIAG_END
};

static const char *itemsDiagnosticsMenu[] =
{
	"Ping",
	"ATM Loopback",
#ifdef CONFIG_VDSL
	"DSL Tone Diagnostic",
#else
	"ADSL Tone Diagnostic",
#endif /*CONFIG_VDSL*/
#ifdef CONFIG_USER_XDSL_SLAVE
	"DSL Slave Tone Diagnostic",
#endif /*CONFIG_USER_XDSL_SLAVE*/
	"ADSL Connection Diagnostic",
	"Quit"
};

const char *DiagnosticsMenu[MENU_DIAG_END+1];
int diag_menu_idx[MENU_DIAG_END];

enum MENU_STATISTICS
{
	MENU_STA_INTERFACE,
	MENU_STA_ADSL,
#ifdef CONFIG_USER_XDSL_SLAVE
	MENU_STA_ADSL_SLAVE,
#endif /*CONFIG_USER_XDSL_SLAVE*/
	MENU_STA_QUIT,
	MENU_STA_END
};

static const char *itemsStatisticsMenu[] =
{
	"Interface",
#ifdef CONFIG_VDSL
	"DSL",
#else
	"ADSL",
#endif /*CONFIG_VDSL*/
#ifdef CONFIG_USER_XDSL_SLAVE
	"DSL Slave",
#endif /*CONFIG_USER_XDSL_SLAVE*/
	"Quit"
};

const char *StatisticsMenu[MENU_STA_END+1];
int sta_menu_idx[MENU_STA_END];

void printWaitStr();
void cli_show_menu(char *strlist[], int size);
int cli_show_dyn_menu(const char *menu[]);
int getInputNum();
int getInputStr( char *value, int maxlen, char *pcIsInputInvalid);
int checkString(char *str);
int getTypedInputDefault(int type, char *prompt, void *rv, void *arg1, void *arg2);
int getTypedInput(int type, char *prompt, void *rv, void *arg1, void *arg2);
int getInputIpAddr(char *prompt, struct in_addr *rv);
int getInputIpMask(char *prompt, struct in_addr *rv);
#ifdef CONFIG_IPV6
int getInputIp6Addr(char *prompt, struct in6_addr *rv);
#endif
int getInputString(char *prompt, char *rv, int len);
int getInputUint(char *prompt, unsigned int *rv, unsigned int *min, unsigned int *max);
int getInputInt(char *prompt, int *rv, int *min, int *max);
int getInputOption(unsigned int *rv, unsigned int min0, unsigned int max0);
void printWaitStr();

int auth_cli(const char *name, const char *passwd); // Jenny, check CLI user/passwd

int isStrIncludedSpace( char *str );
int isPhoneNumber( char *str );

void printspace(unsigned int x);

void base64encode(unsigned char *from, char *to, int len);
void calPasswdMD5(char *pass, char *passMD5);
#ifdef EMBED
void writePasswdFile();
#endif
void write_etcPassword();
char *getMibInfo(int id);
int _wanConfList(int bShowIndex, int *map);
int wanStatusList();
int atmConnList(int bShowIndex, int *map);
int cli_atmVcList(int *map);

int getDGW(char *buf);
int validateKey(char *str);
#ifdef APPLY_CHANGE
int restartSNTPC();
#endif

void bin_to_str(unsigned char *, int, char *);
void RegeneratePIN(char *);
int ValidatePINCode(char *);
int CheckPINCode(char *);

void InitDynamicMenu();

#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_PPP
int pppConnect(int, int);
#endif
#endif
