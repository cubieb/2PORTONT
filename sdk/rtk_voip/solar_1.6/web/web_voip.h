#ifndef __WEB_VOIP_H
#define __WEB_VOIP_H

#include "voip_flash.h"
#include "voip_types.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"
#include "voip_manager.h"

#ifdef CONFIG_APP_BOA
	#include "boa.h"
	#include "asp_page.h"
#else
	#include "webs.h"
#endif

#ifndef CONFIG_RTK_VOIP_PACKAGE_867X
#include "apform.h"
#endif

#ifdef __mips__
#if CONFIG_RTK_VOIP_PACKAGE_865X
#define VOIP_CONFIG_PATH	"/www/config_voip.dat"
#elif CONFIG_RTK_VOIP_PACKAGE_867X
#define VOIP_CONFIG_PATH	"/var/config_voip.dat"
#else
#define VOIP_CONFIG_PATH	"/web/config_voip.dat"
#endif
#else
#define VOIP_CONFIG_PATH	"../web/config_voip.dat"
#endif

#ifdef CONFIG_APP_BOA
#define gstrcmp strcmp
#define boaRedirect	send_redirect_perm
#define boaWrite		req_format_write
#define boaGetVar		req_get_cstream_var
#endif

/* To show the register status on Web page. */
#define _PATH_TMP_STATUS	"/tmp/status"

// SIP Config
#define _PATH_SIP_CONFIG	"/etc/solar.conf"

#if defined(CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
//#define FXO_REDIAL
#endif

// init web functions
int web_voip_init();

#ifdef CONFIG_APP_BOA
int asp_voip_getInfo(request * wp, int argc, char **argv);
int asp_voip_GeneralGet(request * wp, int argc, char **argv);
int asp_voip_DialPlanGet( request * wp, int argc, char **argv);
int asp_voip_ToneGet(request * wp, int argc, char **argv);
int asp_voip_RingGet(request * wp, int argc, char **argv);
int asp_voip_OtherGet(request * wp, int argc, char **argv);
int asp_voip_ConfigGet(request * wp, int argc, char **argv);
int asp_voip_FwupdateGet(request * wp, int argc, char **argv);
int asp_voip_FxoGet(request * wp, int argc, char **argv);
int asp_voip_NetGet(request * wp, int argc, char **argv);
#else
int asp_voip_getInfo(int eid, request * wp, int argc, char **argv);
int asp_voip_GeneralGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_DialPlanGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_ToneGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_RingGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_OtherGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_ConfigGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_FwupdateGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_FxoGet(int ejid, request * wp, int argc, char **argv);
int asp_voip_NetGet(int ejid, request * wp, int argc, char **argv);
#endif

void asp_voip_GeneralSet(request * wp, char *path, char *query);
void asp_voip_DialPlanSet(request * wp, char *path, char *query);
void asp_voip_ToneSet(request * wp, char *path, char *query);
void asp_voip_RingSet(request * wp, char *path, char *query);
void asp_voip_OtherSet(request * wp, char *path, char *query);
void asp_voip_ConfigSet(request * wp, char *path, char *query);
void asp_voip_FwSet(request * wp, char *path, char *query);
void asp_voip_IvrReqSet(request * wp, char *path, char *query);
void asp_voip_FxoSet(request * wp, char *path, char *query);
void asp_voip_NetSet(request * wp, char *path, char *query);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
void asp_voip_TLSCertUpload(request * wp, char *path, char *query);
int asp_voip_TLSGetCertInfo(int eid, request * wp, int argc, char **argv);
#endif
#endif

// flash api in WEB
int web_flash_get(voipCfgParam_t **cfg);
int web_flash_set(voipCfgParam_t *cfg);

// misc function
int web_restart_solar();
int web_voip_saveConfig();

