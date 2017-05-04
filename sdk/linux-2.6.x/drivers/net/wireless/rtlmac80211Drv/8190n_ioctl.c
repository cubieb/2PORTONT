/*
 *      io-control handling routines
 *
 *      $Id: 8190n_ioctl.c,v 1.1.1.1 2010/05/05 09:00:44 jiunming Exp $
 */

#define _8190N_IOCTL_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#endif

#include "./8190n_cfg.h"

#ifdef __LINUX_2_6__
#include <linux/initrd.h>
#include <linux/syscalls.h>
#endif

#ifndef __KERNEL__
#include "./sys-support.h"
#endif

#include "./8190n_headers.h"
#include "./8190n_debug.h"
#include "./ieee802_mib.h"


// dump 8190 registers
#define DEBUG_8190

#define RTL8190_IOCTL_SET_MIB                           (SIOCDEVPRIVATE + 0x1)	// 0x89f1
#define RTL8190_IOCTL_GET_MIB                           (SIOCDEVPRIVATE + 0x2)	// 0x89f2

#define RTL8190_IOCTL_WRITE_REG				(SIOCDEVPRIVATE + 0x3)	// 0x89f3
#define RTL8190_IOCTL_READ_REG				(SIOCDEVPRIVATE + 0x4)	// 0x89f4
#define RTL8190_IOCTL_WRITE_MEM				(SIOCDEVPRIVATE + 0x5)	// 0x89f5
#define RTL8190_IOCTL_READ_MEM				(SIOCDEVPRIVATE + 0x6)	// 0x89f6
#define RTL8190_IOCTL_WRITE_RF_REG			(SIOCDEVPRIVATE + 0xc)	// 0x89fc
#define RTL8190_IOCTL_READ_RF_REG			(SIOCDEVPRIVATE + 0xd)	// 0x89fd

/*
#ifdef MP_TEST
#define MP_START_TEST	0x8B61
#define MP_STOP_TEST	0x8B62
#define MP_SET_RATE		0x8B63
#define MP_SET_CHANNEL	0x8B64
#define MP_SET_TXPOWER	0x8B65
#define MP_CONTIOUS_TX	0x8B66
#define MP_ARX			0x8B67
#define MP_SET_BSSID	0x8B68
#define MP_ANTENNA_TX	0x8B69
#define MP_ANTENNA_RX	0x8B6A
#define MP_SET_BANDWIDTH 0x8B6B
#define MP_SET_PHYPARA	0x8B6C
#define MP_QUERY_STATS 	0x8B6D
#define MP_TXPWR_TRACK	0x8B6E
#define MP_QUERY_TSSI	0x8B6F
#ifdef RTL8192SE
#define MP_QUERY_THER	0x8B77
#endif

#ifdef B2B_TEST
// set/get convention: set(even number) get (odd number)
#define MP_TX_PACKET	0x8B71
#define MP_RX_PACKET	0x8B70
#define MP_BRX_PACKET	0x8B73
#endif

#endif // MP_TEST
*/

#ifdef DEBUG_8190
#define SIOCGIWRTLREGDUMP		0x8B78
#endif

#define _OFFSET(field)	((int)(long *)&(((struct wifi_mib *)0)->field))
#define _SIZE(field)	sizeof(((struct wifi_mib *)0)->field)
#define _OFFSET_RFFT(field)	((int)(long *)&(((struct rf_finetune_var *)0)->field))
#define _SIZE_RFFT(field)	sizeof(((struct rf_finetune_var *)0)->field)

typedef enum {BYTE_T, INT_T, SSID_STRING_T, BYTE_ARRAY_T, ACL_T, IDX_BYTE_ARRAY_T, MULTI_BYTE_T,

#ifdef _DEBUG_RTL8190_
		DEBUG_T,
#endif
	DEF_SSID_STRING_T, STRING_T, RFFT_T, VARLEN_BYTE_T,
	ACL_INT_T,	// mac address + 1 int

} TYPE_T;

struct iwpriv_arg {
	char name[20];	/* mib name */
	TYPE_T type;	/* Type and number of args */
	int offset;		/* mib offset */
	int len;		/* mib byte len */
	int Default;	/* mib default value */
};

static struct iwpriv_arg mib_table[] = {
	// struct Dot11RFEntry
	{"RFChipID",	INT_T,		_OFFSET(dot11RFEntry.dot11RFType), _SIZE(dot11RFEntry.dot11RFType), 10},
	{"channel",	INT_T,		_OFFSET(dot11RFEntry.dot11channel), _SIZE(dot11RFEntry.dot11channel), 0},
	{"ch_low",	INT_T,		_OFFSET(dot11RFEntry.dot11ch_low), _SIZE(dot11RFEntry.dot11ch_low), 0},
	{"ch_hi",	INT_T,		_OFFSET(dot11RFEntry.dot11ch_hi), _SIZE(dot11RFEntry.dot11ch_hi), 0},
	{"TxPowerCCK",	BYTE_ARRAY_T, _OFFSET(dot11RFEntry.pwrlevelCCK), _SIZE(dot11RFEntry.pwrlevelCCK), 2727272727272727272828282828},
#if defined(CONFIG_RTL8192SE)
	{"TxPowerOFDM_1SS",	BYTE_ARRAY_T, _OFFSET(dot11RFEntry.pwrlevelOFDM_1SS), _SIZE(dot11RFEntry.pwrlevelOFDM_1SS), 0},
	{"TxPowerOFDM_2SS",	BYTE_ARRAY_T, _OFFSET(dot11RFEntry.pwrlevelOFDM_2SS), _SIZE(dot11RFEntry.pwrlevelOFDM_2SS), 0},
#else
	{"TxPowerOFDM",	BYTE_ARRAY_T, _OFFSET(dot11RFEntry.pwrlevelOFDM), _SIZE(dot11RFEntry.pwrlevelOFDM), 0},
#endif
	{"preamble",	INT_T,		_OFFSET(dot11RFEntry.shortpreamble), _SIZE(dot11RFEntry.shortpreamble), 0},
	{"disable_ch14_ofdm",	INT_T,	_OFFSET(dot11RFEntry.disable_ch14_ofdm), _SIZE(dot11RFEntry.disable_ch14_ofdm), 0},
#if defined(CONFIG_RTL8192SE)
	{"LOFDM_pwd_A",	INT_T,	_OFFSET(dot11RFEntry.LOFDM_pwd_A), _SIZE(dot11RFEntry.LOFDM_pwd_A), 35},
	{"LOFDM_pwd_B",	INT_T,	_OFFSET(dot11RFEntry.LOFDM_pwd_B), _SIZE(dot11RFEntry.LOFDM_pwd_B), 35},
	{"tssi1",		INT_T,	_OFFSET(dot11RFEntry.tssi1), _SIZE(dot11RFEntry.tssi1), 0},
	{"tssi2",		INT_T,	_OFFSET(dot11RFEntry.tssi2), _SIZE(dot11RFEntry.tssi2), 0},
	{"ther",		INT_T,	_OFFSET(dot11RFEntry.ther), _SIZE(dot11RFEntry.ther), 17},
#else
	{"LOFDM_pwrdiff",	INT_T,	_OFFSET(dot11RFEntry.legacyOFDM_pwrdiff), _SIZE(dot11RFEntry.legacyOFDM_pwrdiff), 0},
	{"antC_pwrdiff",INT_T,	_OFFSET(dot11RFEntry.antC_pwrdiff), _SIZE(dot11RFEntry.antC_pwrdiff), 0},
	{"ther_rfic",	INT_T,	_OFFSET(dot11RFEntry.ther_rfic), _SIZE(dot11RFEntry.ther_rfic), 0},
	{"crystalCap",	INT_T,	_OFFSET(dot11RFEntry.crystalCap), _SIZE(dot11RFEntry.crystalCap), 0},
	{"bw_pwrdiff",	INT_T,	_OFFSET(dot11RFEntry.bw_pwrdiff), _SIZE(dot11RFEntry.bw_pwrdiff), 0},
#endif
#if defined(RTL8192SE)
	{"MIMO_TR_mode",	INT_T,	_OFFSET(dot11RFEntry.MIMO_TR_mode), _SIZE(dot11RFEntry.MIMO_TR_mode), MIMO_2T2R},
#elif defined(RTL8190)
	{"MIMO_TR_mode",	INT_T,	_OFFSET(dot11RFEntry.MIMO_TR_mode), _SIZE(dot11RFEntry.MIMO_TR_mode), MIMO_2T4R},
#endif

	// struct Dot11StationConfigEntry
	{"ssid",		SSID_STRING_T,	_OFFSET(dot11StationConfigEntry.dot11DesiredSSID), _SIZE(dot11StationConfigEntry.dot11DesiredSSID), 0},
	{"defssid",		DEF_SSID_STRING_T,	_OFFSET(dot11StationConfigEntry.dot11DefaultSSID), _SIZE(dot11StationConfigEntry.dot11DefaultSSID), 0},
	{"bssid2join",	BYTE_ARRAY_T,	_OFFSET(dot11StationConfigEntry.dot11DesiredBssid), _SIZE(dot11StationConfigEntry.dot11DesiredBssid), 0},
	{"bcnint",		INT_T,		_OFFSET(dot11StationConfigEntry.dot11BeaconPeriod), _SIZE(dot11StationConfigEntry.dot11BeaconPeriod), 100},
	{"dtimperiod",	INT_T,		_OFFSET(dot11StationConfigEntry.dot11DTIMPeriod), _SIZE(dot11StationConfigEntry.dot11DTIMPeriod), 1},
	{"swcrypto",	INT_T,		_OFFSET(dot11StationConfigEntry.dot11swcrypto), _SIZE(dot11StationConfigEntry.dot11swcrypto), 0},
	{"aclmode",		INT_T,		_OFFSET(dot11StationConfigEntry.dot11AclMode), _SIZE(dot11StationConfigEntry.dot11AclMode), 0},
	{"aclnum",		INT_T,		_OFFSET(dot11StationConfigEntry.dot11AclNum), _SIZE(dot11StationConfigEntry.dot11AclNum), 0},
	{"acladdr",		ACL_T,		_OFFSET(dot11StationConfigEntry.dot11AclAddr), _SIZE(dot11StationConfigEntry.dot11AclAddr), 0},
	{"oprates",		INT_T,		_OFFSET(dot11StationConfigEntry.dot11SupportedRates), _SIZE(dot11StationConfigEntry.dot11SupportedRates), 0xfff},
	{"basicrates",	INT_T,		_OFFSET(dot11StationConfigEntry.dot11BasicRates), _SIZE(dot11StationConfigEntry.dot11BasicRates), 0xf},
	{"regdomain",	INT_T,		_OFFSET(dot11StationConfigEntry.dot11RegDomain), _SIZE(dot11StationConfigEntry.dot11RegDomain), 1},
	{"autorate",	INT_T,		_OFFSET(dot11StationConfigEntry.autoRate), _SIZE(dot11StationConfigEntry.autoRate), 1},
	{"fixrate",		INT_T,		_OFFSET(dot11StationConfigEntry.fixedTxRate), _SIZE(dot11StationConfigEntry.fixedTxRate), 0},
	{"swTkipMic",	INT_T,		_OFFSET(dot11StationConfigEntry.swTkipMic), _SIZE(dot11StationConfigEntry.swTkipMic), 1},
	{"disable_protection", INT_T,	_OFFSET(dot11StationConfigEntry.protectionDisabled), _SIZE(dot11StationConfigEntry.protectionDisabled), 0},
	{"disable_olbc", INT_T,		_OFFSET(dot11StationConfigEntry.olbcDetectDisabled), _SIZE(dot11StationConfigEntry.olbcDetectDisabled), 0},
	{"deny_legacy",	INT_T,		_OFFSET(dot11StationConfigEntry.legacySTADeny), _SIZE(dot11StationConfigEntry.legacySTADeny), 0},
#ifdef CLIENT_MODE
	{"fast_roaming", INT_T,		_OFFSET(dot11StationConfigEntry.fastRoaming), _SIZE(dot11StationConfigEntry.fastRoaming), 0},
#endif
	{"lowestMlcstRate", INT_T,	_OFFSET(dot11StationConfigEntry.lowestMlcstRate), _SIZE(dot11StationConfigEntry.lowestMlcstRate), 0},
	{"stanum",	INT_T,		_OFFSET(dot11StationConfigEntry.supportedStaNum), _SIZE(dot11StationConfigEntry.supportedStaNum), 0},

#if !defined(NEW_MAC80211_DRV) && defined(CONFIG_RTK_MESH)
	{"mesh_enable",			BYTE_T,			_OFFSET(dot1180211sInfo.mesh_enable),			_SIZE(dot1180211sInfo.mesh_enable),			0},
	{"mesh_root_enable",	BYTE_T,			_OFFSET(dot1180211sInfo.mesh_root_enable),		_SIZE(dot1180211sInfo.mesh_root_enable),	0},
	{"mesh_ap_enable",		BYTE_T,			_OFFSET(dot1180211sInfo.mesh_ap_enable),		_SIZE(dot1180211sInfo.mesh_ap_enable),		0},
	{"mesh_portal_enable",	BYTE_T,			_OFFSET(dot1180211sInfo.mesh_portal_enable),	_SIZE(dot1180211sInfo.mesh_portal_enable),	0},
	{"mesh_id",				STRING_T,		_OFFSET(dot1180211sInfo.mesh_id),				_SIZE(dot1180211sInfo.mesh_id),				0},
	{"mesh_max_neightbor",	WORD_T,			_OFFSET(dot1180211sInfo.mesh_max_neightbor),	_SIZE(dot1180211sInfo.mesh_max_neightbor),	0},

	{"log_enabled",			BYTE_T,			_OFFSET(dot1180211sInfo.log_enabled),		_SIZE(dot1180211sInfo.log_enabled),		0},
	{"mesh_privacy",		INT_T,		_OFFSET(dot11sKeysTable.dot11Privacy),	_SIZE(dot11sKeysTable.dot11Privacy),		0},

#if !defined(NEW_MAC80211_DRV) && defined(_MESH_ACL_ENABLE_)
	{"meshaclmode", 		INT_T,			_OFFSET(dot1180211sInfo.mesh_acl_mode), 		_SIZE(dot1180211sInfo.mesh_acl_mode),		0},
	{"meshaclnum",			INT_T,			_OFFSET(dot1180211sInfo.mesh_acl_num), 			_SIZE(dot1180211sInfo.mesh_acl_num),		0},
	{"meshacladdr", 		ACL_T,			_OFFSET(dot1180211sInfo.mesh_acl_addr), 		_SIZE(dot1180211sInfo.mesh_acl_addr),		0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(_11s_TEST_MODE_)
	{"mesh_reserved1",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved1),	_SIZE(dot1180211sInfo.mesh_reserved1),		0},
	{"mesh_reserved2",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved2),	_SIZE(dot1180211sInfo.mesh_reserved2),		0},
	{"mesh_reserved3",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved3),	_SIZE(dot1180211sInfo.mesh_reserved3),		0},
	{"mesh_reserved4",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved4),	_SIZE(dot1180211sInfo.mesh_reserved4),		0},
	{"mesh_reserved5",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved5),	_SIZE(dot1180211sInfo.mesh_reserved5),		0},
	{"mesh_reserved6",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved6),	_SIZE(dot1180211sInfo.mesh_reserved6),		0},
	{"mesh_reserved7",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved7),	_SIZE(dot1180211sInfo.mesh_reserved7),		0},
	{"mesh_reserved8",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved8),	_SIZE(dot1180211sInfo.mesh_reserved8),		0},
	{"mesh_reserved9",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserved9),	_SIZE(dot1180211sInfo.mesh_reserved9),		0},
	{"mesh_reserveda",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reserveda),	_SIZE(dot1180211sInfo.mesh_reserveda),		0},
	{"mesh_reservedb",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reservedb),	_SIZE(dot1180211sInfo.mesh_reservedb),		0},
	{"mesh_reservedc",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reservedc),	_SIZE(dot1180211sInfo.mesh_reservedc),		0},
	{"mesh_reservedd",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reservedd),	_SIZE(dot1180211sInfo.mesh_reservedd),		0},
	{"mesh_reservede",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reservede),	_SIZE(dot1180211sInfo.mesh_reservede),		0},
	{"mesh_reservedf",			WORD_T,			_OFFSET(dot1180211sInfo.mesh_reservedf),	_SIZE(dot1180211sInfo.mesh_reservedf),		0},
	{"mesh_reservedstr1",		STRING_T,		_OFFSET(dot1180211sInfo.mesh_reservedstr1),	_SIZE(dot1180211sInfo.mesh_reservedstr1),	0},
#endif

#endif

	// struct Dot1180211AuthEntry
#if defined(CONFIG_RTL8192SE) // temporary solution
	{"authtype",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11AuthAlgrthm), _SIZE(dot1180211AuthEntry.dot11AuthAlgrthm), 0},
#else
	{"authtype",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11AuthAlgrthm), _SIZE(dot1180211AuthEntry.dot11AuthAlgrthm), 2},
#endif
	{"encmode",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11PrivacyAlgrthm), _SIZE(dot1180211AuthEntry.dot11PrivacyAlgrthm), 0},
	{"wepdkeyid",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11PrivacyKeyIndex), _SIZE(dot1180211AuthEntry.dot11PrivacyKeyIndex), 0},
#if !defined(NEW_MAC80211_DRV) && defined(INCLUDE_WPA_PSK)
	{"psk_enable",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11EnablePSK), _SIZE(dot1180211AuthEntry.dot11EnablePSK), 0},
	{"wpa_cipher",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11WPACipher), _SIZE(dot1180211AuthEntry.dot11WPACipher), 0},
#if !defined(NEW_MAC80211_DRV) && defined(RTL_WPA2)
	{"wpa2_cipher",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11WPA2Cipher), _SIZE(dot1180211AuthEntry.dot11WPA2Cipher), 0},
#endif
#endif
	{"passphrase",	STRING_T,	_OFFSET(dot1180211AuthEntry.dot11PassPhrase), _SIZE(dot1180211AuthEntry.dot11PassPhrase), 0},
	{"gk_rekey",	INT_T,		_OFFSET(dot1180211AuthEntry.dot11GKRekeyTime), _SIZE(dot1180211AuthEntry.dot11GKRekeyTime), 0},

	// struct Dot118021xAuthEntry
	{"802_1x",	INT_T,		_OFFSET(dot118021xAuthEntry.dot118021xAlgrthm), _SIZE(dot118021xAuthEntry.dot118021xAlgrthm), 0},
	{"default_port",INT_T,		_OFFSET(dot118021xAuthEntry.dot118021xDefaultPort), _SIZE(dot118021xAuthEntry.dot118021xDefaultPort), 0},

	// struct Dot11DefaultKeysTable
	{"wepkey1",		BYTE_ARRAY_T,	_OFFSET(dot11DefaultKeysTable.keytype[0]), _SIZE(dot11DefaultKeysTable.keytype[0]), 0},
	{"wepkey2",		BYTE_ARRAY_T,	_OFFSET(dot11DefaultKeysTable.keytype[1]), _SIZE(dot11DefaultKeysTable.keytype[1]), 0},
	{"wepkey3",		BYTE_ARRAY_T,	_OFFSET(dot11DefaultKeysTable.keytype[2]), _SIZE(dot11DefaultKeysTable.keytype[2]), 0},
	{"wepkey4",		BYTE_ARRAY_T,	_OFFSET(dot11DefaultKeysTable.keytype[3]), _SIZE(dot11DefaultKeysTable.keytype[3]), 0},

	// struct Dot11OperationEntry
	{"opmode",	INT_T,		_OFFSET(dot11OperationEntry.opmode), _SIZE(dot11OperationEntry.opmode), 0x10},
	{"hiddenAP",	INT_T,		_OFFSET(dot11OperationEntry.hiddenAP), _SIZE(dot11OperationEntry.hiddenAP), 0},
	{"rtsthres",	INT_T,		_OFFSET(dot11OperationEntry.dot11RTSThreshold), _SIZE(dot11OperationEntry.dot11RTSThreshold), 2347},
	{"fragthres",	INT_T,		_OFFSET(dot11OperationEntry.dot11FragmentationThreshold), _SIZE(dot11OperationEntry.dot11FragmentationThreshold), 2347},
	{"shortretry",	INT_T,		_OFFSET(dot11OperationEntry.dot11ShortRetryLimit), _SIZE(dot11OperationEntry.dot11ShortRetryLimit), 0},
	{"longretry",	INT_T,		_OFFSET(dot11OperationEntry.dot11LongRetryLimit), _SIZE(dot11OperationEntry.dot11LongRetryLimit), 0},
	{"expired_time",INT_T,		_OFFSET(dot11OperationEntry.expiretime), _SIZE(dot11OperationEntry.expiretime), 30000},
	{"led_type",	INT_T,		_OFFSET(dot11OperationEntry.ledtype), _SIZE(dot11OperationEntry.ledtype), 0},
#if !defined(NEW_MAC80211_DRV) && defined(RTL8190_SWGPIO_LED)
	{"led_route",	INT_T,		_OFFSET(dot11OperationEntry.ledroute), _SIZE(dot11OperationEntry.ledroute), 0},
#endif
	{"iapp_enable",	INT_T,		_OFFSET(dot11OperationEntry.iapp_enable), _SIZE(dot11OperationEntry.iapp_enable), 0},
	{"block_relay",	INT_T,		_OFFSET(dot11OperationEntry.block_relay), _SIZE(dot11OperationEntry.block_relay), 0},
	{"deny_any",	INT_T,		_OFFSET(dot11OperationEntry.deny_any), _SIZE(dot11OperationEntry.deny_any), 0},
	{"crc_log",	INT_T,		_OFFSET(dot11OperationEntry.crc_log), _SIZE(dot11OperationEntry.crc_log), 0},
	{"wifi_specific",INT_T,		_OFFSET(dot11OperationEntry.wifi_specific), _SIZE(dot11OperationEntry.wifi_specific), 0},
#if !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)
	{"disable_txsc",INT_T,		_OFFSET(dot11OperationEntry.disable_txsc), _SIZE(dot11OperationEntry.disable_txsc), 0},
#endif
#if !defined(NEW_MAC80211_DRV) && defined(RX_SHORTCUT)
	{"disable_rxsc",INT_T,		_OFFSET(dot11OperationEntry.disable_rxsc), _SIZE(dot11OperationEntry.disable_rxsc), 0},
#endif
#if !defined(NEW_MAC80211_DRV) && defined(BR_SHORTCUT)
	{"disable_brsc",INT_T,		_OFFSET(dot11OperationEntry.disable_brsc), _SIZE(dot11OperationEntry.disable_brsc), 0},
#endif
	{"keep_rsnie",	INT_T,		_OFFSET(dot11OperationEntry.keep_rsnie), _SIZE(dot11OperationEntry.keep_rsnie), 0},
	{"guest_access",INT_T,		_OFFSET(dot11OperationEntry.guest_access), _SIZE(dot11OperationEntry.guest_access), 0},

	// struct bss_type
	{"band",	BYTE_T,		_OFFSET(dot11BssType.net_work_type), _SIZE(dot11BssType.net_work_type), 3},

	// struct erp_mib
	{"cts2self",	INT_T,		_OFFSET(dot11ErpInfo.ctsToSelf), _SIZE(dot11ErpInfo.ctsToSelf), 1},

#if !defined(NEW_MAC80211_DRV) && defined(WDS)
	// struct wds_info
	{"wds_enable",	INT_T,		_OFFSET(dot11WdsInfo.wdsEnabled), _SIZE(dot11WdsInfo.wdsEnabled), 0},
	{"wds_pure",	INT_T,		_OFFSET(dot11WdsInfo.wdsPure), _SIZE(dot11WdsInfo.wdsPure), 0},
	{"wds_priority",INT_T,		_OFFSET(dot11WdsInfo.wdsPriority), _SIZE(dot11WdsInfo.wdsPriority), 0},
	{"wds_num",		INT_T,		_OFFSET(dot11WdsInfo.wdsNum), _SIZE(dot11WdsInfo.wdsNum), 0},
	{"wds_add",		ACL_INT_T,		_OFFSET(dot11WdsInfo.entry), _SIZE(dot11WdsInfo.entry), 0},
	{"wds_encrypt",	INT_T,		_OFFSET(dot11WdsInfo.wdsPrivacy), _SIZE(dot11WdsInfo.wdsPrivacy), 0},
	{"wds_wepkey",	BYTE_ARRAY_T, _OFFSET(dot11WdsInfo.wdsWepKey), _SIZE(dot11WdsInfo.wdsWepKey), 0},
#if !defined(NEW_MAC80211_DRV) && defined(INCLUDE_WPA_PSK)
	{"wds_passphrase",	STRING_T, _OFFSET(dot11WdsInfo.wdsPskPassPhrase), _SIZE(dot11WdsInfo.wdsPskPassPhrase), 0},
#endif
#endif

#if !defined(NEW_MAC80211_DRV) && defined(RTK_BR_EXT)
	// struct br_ext_info
	{"nat25_disable",		INT_T,	_OFFSET(ethBrExtInfo.nat25_disable), _SIZE(ethBrExtInfo.nat25_disable), 0},
	{"macclone_enable",		INT_T,	_OFFSET(ethBrExtInfo.macclone_enable), _SIZE(ethBrExtInfo.macclone_enable), 0},
	{"dhcp_bcst_disable",	INT_T,	_OFFSET(ethBrExtInfo.dhcp_bcst_disable), _SIZE(ethBrExtInfo.dhcp_bcst_disable), 0},
	{"add_pppoe_tag",		INT_T,	_OFFSET(ethBrExtInfo.addPPPoETag), _SIZE(ethBrExtInfo.addPPPoETag), 1},
	{"clone_mac_addr",		BYTE_ARRAY_T,	_OFFSET(ethBrExtInfo.nat25_dmzMac), _SIZE(ethBrExtInfo.nat25_dmzMac), 0},
	{"nat25sc_disable",		INT_T,	_OFFSET(ethBrExtInfo.nat25sc_disable), _SIZE(ethBrExtInfo.nat25sc_disable), 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(DFS)
	//struct Dot11DFSEntry
	{"disable_DFS",	INT_T,		_OFFSET(dot11DFSEntry.disable_DFS), _SIZE(dot11DFSEntry.disable_DFS), 0},
	{"disable_tx",	INT_T,		_OFFSET(dot11DFSEntry.disable_tx), _SIZE(dot11DFSEntry.disable_tx), 0},
	{"DFS_timeout",	INT_T,		_OFFSET(dot11DFSEntry.DFS_timeout), _SIZE(dot11DFSEntry.DFS_timeout), 1},
	{"DFS_detected",INT_T,		_OFFSET(dot11DFSEntry.DFS_detected), _SIZE(dot11DFSEntry.DFS_detected), 0},
	{"NOP_timeout",	INT_T,		_OFFSET(dot11DFSEntry.NOP_timeout), _SIZE(dot11DFSEntry.NOP_timeout), 180500},
	{"rs1_threshold",INT_T,		_OFFSET(dot11DFSEntry.rs1_threshold), _SIZE(dot11DFSEntry.rs1_threshold), 8},
	{"Throughput_Threshold",INT_T,	_OFFSET(dot11DFSEntry.Throughput_Threshold), _SIZE(dot11DFSEntry.Throughput_Threshold), 15},
	{"RecordHistory_sec",	INT_T,	_OFFSET(dot11DFSEntry.RecordHistory_sec), _SIZE(dot11DFSEntry.RecordHistory_sec), 1},
	{"temply_disable_DFS",	INT_T,	_OFFSET(dot11DFSEntry.temply_disable_DFS), _SIZE(dot11DFSEntry.temply_disable_DFS), 0},
	{"Dump_Throughput",		INT_T,	_OFFSET(dot11DFSEntry.Dump_Throughput), _SIZE(dot11DFSEntry.Dump_Throughput), 0},
	{"disable_DtmDFS",		INT_T,	_OFFSET(dot11DFSEntry.disable_DetermineDFSDisable), _SIZE(dot11DFSEntry.disable_DetermineDFSDisable), 0},
#endif

	//struct miscEntry
	{"show_hidden_bss",    INT_T,          	_OFFSET(miscEntry.show_hidden_bss), _SIZE(miscEntry.show_hidden_bss), 0},
	{"ack_timeout",        INT_T,		_OFFSET(miscEntry.ack_timeout), _SIZE(miscEntry.ack_timeout), 0},
	{"tx_priority",        INT_T,		_OFFSET(miscEntry.tx_priority), _SIZE(miscEntry.tx_priority), 0},
	{"private_ie",         VARLEN_BYTE_T,	_OFFSET(miscEntry.private_ie), _SIZE(miscEntry.private_ie), 0},
	{"rxInt",		INT_T,		_OFFSET(miscEntry.rxInt_thrd), _SIZE(miscEntry.rxInt_thrd), 300},
#if !defined(NEW_MAC80211_DRV) && defined(DRVMAC_LB)
	{"dmlb",		INT_T,		_OFFSET(miscEntry.drvmac_lb), _SIZE(miscEntry.drvmac_lb), 0},
	{"lb_da",		BYTE_ARRAY_T,	_OFFSET(miscEntry.lb_da), _SIZE(miscEntry.lb_da), 0},
#endif
	{"groupID",		INT_T,		_OFFSET(miscEntry.groupID), _SIZE(miscEntry.groupID), 0},
#if !defined(NEW_MAC80211_DRV) && defined(CONFIG_RTL8192SE) && defined(MBSSID)
	{"vap_enable",	INT_T,		_OFFSET(miscEntry.vap_enable), _SIZE(miscEntry.vap_enable), 0},
#endif
	{"func_off",	INT_T,		_OFFSET(miscEntry.func_off), _SIZE(miscEntry.func_off), 0},

	//struct Dot11QosEntry
#if !defined(NEW_MAC80211_DRV) && defined(SEMI_QOS)
	{"qos_enable",	INT_T,		_OFFSET(dot11QosEntry.dot11QosEnable), _SIZE(dot11QosEntry.dot11QosEnable), 0},
#if !defined(NEW_MAC80211_DRV) && defined(WMM_APSD)
	{"apsd_enable",	INT_T,		_OFFSET(dot11QosEntry.dot11QosAPSD), _SIZE(dot11QosEntry.dot11QosAPSD), 0},
#endif
#endif

#if !defined(NEW_MAC80211_DRV) && defined(WIFI_SIMPLE_CONFIG)
	// struct WifiSimpleConfigEntry
	{"wsc_enable",	INT_T,	_OFFSET(wscEntry.wsc_enable), _SIZE(wscEntry.wsc_enable), 0},
	{"pin",			PIN_IND_T, 0, 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(GBWC)
	// struct GroupBandWidthControl
	{"gbwcmode",	INT_T,		_OFFSET(gbwcEntry.GBWCMode), _SIZE(gbwcEntry.GBWCMode), 0},
	{"gbwcnum",	INT_T,		_OFFSET(gbwcEntry.GBWCNum), _SIZE(gbwcEntry.GBWCNum), 0},
	{"gbwcaddr",	ACL_T,		_OFFSET(gbwcEntry.GBWCAddr), _SIZE(gbwcEntry.GBWCAddr), 0},
	{"gbwcthrd",	INT_T,		_OFFSET(gbwcEntry.GBWCThrd), _SIZE(gbwcEntry.GBWCThrd), 30000},
#endif

	// struct Dot11nConfigEntry
	{"supportedmcs",INT_T,		_OFFSET(dot11nConfigEntry.dot11nSupportedMCS), _SIZE(dot11nConfigEntry.dot11nSupportedMCS), 0xffff},
	{"basicmcs",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nBasicMCS), _SIZE(dot11nConfigEntry.dot11nBasicMCS), 0},
	{"use40M",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nUse40M), _SIZE(dot11nConfigEntry.dot11nUse40M), 0},
	{"2ndchoffset",	INT_T,		_OFFSET(dot11nConfigEntry.dot11n2ndChOffset), _SIZE(dot11nConfigEntry.dot11n2ndChOffset), 1},
	{"shortGI20M",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nShortGIfor20M), _SIZE(dot11nConfigEntry.dot11nShortGIfor20M), 0},
	{"shortGI40M",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nShortGIfor40M), _SIZE(dot11nConfigEntry.dot11nShortGIfor40M), 0},
	{"stbc",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nTxSTBC), _SIZE(dot11nConfigEntry.dot11nTxSTBC), 0},
	{"ampdu",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMPDU), _SIZE(dot11nConfigEntry.dot11nAMPDU), 2},
	{"amsdu",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMSDU), _SIZE(dot11nConfigEntry.dot11nAMSDU), 0},
	{"ampduSndSz",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMPDUSendSz), _SIZE(dot11nConfigEntry.dot11nAMPDUSendSz), 0},
#ifdef CONFIG_RTL8196B_GW_8M
	{"amsduMax",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMSDURecvMax), _SIZE(dot11nConfigEntry.dot11nAMSDURecvMax), 0},
#else
	{"amsduMax",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMSDURecvMax), _SIZE(dot11nConfigEntry.dot11nAMSDURecvMax), 1},
#endif
	{"amsduTimeout",INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMSDUSendTimeout), _SIZE(dot11nConfigEntry.dot11nAMSDUSendTimeout), 400},
	{"amsduNum",	INT_T,		_OFFSET(dot11nConfigEntry.dot11nAMSDUSendNum), _SIZE(dot11nConfigEntry.dot11nAMSDUSendNum), 15},
#if !defined(NEW_MAC80211_DRV) && (defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196B_TLD))
	{"lgyEncRstrct",INT_T,		_OFFSET(dot11nConfigEntry.dot11nLgyEncRstrct), _SIZE(dot11nConfigEntry.dot11nLgyEncRstrct), 0},
#else
	{"lgyEncRstrct",INT_T,		_OFFSET(dot11nConfigEntry.dot11nLgyEncRstrct), _SIZE(dot11nConfigEntry.dot11nLgyEncRstrct), 2},
#endif
#if !defined(NEW_MAC80211_DRV) && defined(WIFI_11N_2040_COEXIST)
	{"coexist",		INT_T,		_OFFSET(dot11nConfigEntry.dot11nCoexist), _SIZE(dot11nConfigEntry.dot11nCoexist), 0},
#endif

	// struct ReorderControlEntry
	{"rc_enable",	INT_T,		_OFFSET(reorderCtrlEntry.ReorderCtrlEnable), _SIZE(reorderCtrlEntry.ReorderCtrlEnable), 1},
	{"rc_winsz",	INT_T,		_OFFSET(reorderCtrlEntry.ReorderCtrlWinSz), _SIZE(reorderCtrlEntry.ReorderCtrlWinSz), RC_ENTRY_NUM},
	{"rc_timeout",	INT_T,		_OFFSET(reorderCtrlEntry.ReorderCtrlTimeout), _SIZE(reorderCtrlEntry.ReorderCtrlTimeout), 30000},

#if !defined(NEW_MAC80211_DRV) && defined(CONFIG_RTK_VLAN_SUPPORT)
	// struct VlanConfig
	{"global_vlan",	INT_T,		_OFFSET(vlan.global_vlan), _SIZE(vlan.global_vlan), 0},
	{"is_lan",	INT_T,		_OFFSET(vlan.is_lan), _SIZE(vlan.is_lan), 1},
	{"vlan_enable",	INT_T,		_OFFSET(vlan.vlan_enable), _SIZE(vlan.vlan_enable), 0},
	{"vlan_tag",	INT_T,		_OFFSET(vlan.vlan_tag), _SIZE(vlan.vlan_tag), 0},
	{"vlan_id",	INT_T,		_OFFSET(vlan.vlan_id), _SIZE(vlan.vlan_id), 0},
	{"vlan_pri",	INT_T,		_OFFSET(vlan.vlan_pri), _SIZE(vlan.vlan_pri), 0},
	{"vlan_cfi",	INT_T,		_OFFSET(vlan.vlan_cfi), _SIZE(vlan.vlan_cfi), 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(CONFIG_RTL_WAPI_SUPPORT)
	{"wapiType",		INT_T,	_OFFSET(wapiInfo.wapiType), _SIZE(wapiInfo.wapiType), 0},
#if !defined(NEW_MAC80211_DRV) && WAPI_SUPPORT_MULTI_ENCRYPT
	{"wapiUCastEncodeType",		INT_T,	_OFFSET(wapiInfo.wapiUCastEncodeType), _SIZE(wapiInfo.wapiUCastEncodeType), 0},
	{"wapiMCastEncodeType",		INT_T,	_OFFSET(wapiInfo.wapiMCastEncodeType), _SIZE(wapiInfo.wapiMCastEncodeType), 0},
#endif
	{"wapiPsk",	WAPI_KEY_T,	_OFFSET(wapiInfo.wapiPsk), _SIZE(wapiInfo.wapiPsk), 0},
	{"wapiPsklen",	INT_T,	_OFFSET(wapiInfo.wapiPsk.len), _SIZE(wapiInfo.wapiPsk.len), 0},
	{"wapiUCastKeyType",		INT_T,	_OFFSET(wapiInfo.wapiUpdateUCastKeyType), _SIZE(wapiInfo.wapiUpdateUCastKeyType), 0},
	{"wapiUCastKeyTimeout",		INT_T,	_OFFSET(wapiInfo.wapiUpdateUCastKeyTimeout), _SIZE(wapiInfo.wapiUpdateUCastKeyTimeout), 0},
	{"wapiUCastKeyPktNum",		INT_T,	_OFFSET(wapiInfo.wapiUpdateUCastKeyPktNum), _SIZE(wapiInfo.wapiUpdateUCastKeyPktNum), 0},
	{"wapiMCastKeyType",		INT_T,	_OFFSET(wapiInfo.wapiUpdateMCastKeyType), _SIZE(wapiInfo.wapiUpdateMCastKeyType), 0},
	{"wapiMCastKeyTimeout",		INT_T,	_OFFSET(wapiInfo.wapiUpdateMCastKeyTimeout), _SIZE(wapiInfo.wapiUpdateMCastKeyTimeout), 0},
	{"wapiMCastKeyPktNum",		INT_T,	_OFFSET(wapiInfo.wapiUpdateMCastKeyPktNum), _SIZE(wapiInfo.wapiUpdateMCastKeyPktNum), 0},
	{"wapiTimeout",		INT_ARRAY_T,	_OFFSET(wapiInfo.wapiTimeout), _SIZE(wapiInfo.wapiTimeout), 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(_DEBUG_RTL8190_)
	// debug flag
	{"debug_err",	DEBUG_T,	1, sizeof(rtl8190_debug_err), 0},
	{"debug_info",	DEBUG_T,	2, sizeof(rtl8190_debug_info), 0},
	{"debug_trace",	DEBUG_T,	3, sizeof(rtl8190_debug_trace), 0},
	{"debug_warn",	DEBUG_T,	4, sizeof(rtl8190_debug_warn), 0},
#endif

	// for RF debug
#if !defined(NEW_MAC80211_DRV) && defined(RTL8192SE)
	{"ofdm_1ss_oneAnt",	RFFT_T,		_OFFSET_RFFT(ofdm_1ss_oneAnt), _SIZE_RFFT(ofdm_1ss_oneAnt), 0},// 1ss and ofdm rate using one ant
	{"pathB_1T", RFFT_T, _OFFSET_RFFT(pathB_1T), _SIZE_RFFT(pathB_1T), 0},// using pathB as 1T2R/1T1R tx path
#ifdef EXT_ANT_DVRY
	{"ext_ad_th",		RFFT_T,		_OFFSET_RFFT(ext_ad_th), _SIZE_RFFT(ext_ad_th), 5},
	{"ext_ad_Ttry",		RFFT_T,		_OFFSET_RFFT(ext_ad_Ttry), _SIZE_RFFT(ext_ad_Ttry), 10},
	{"ext_ad_Ts",		RFFT_T,		_OFFSET_RFFT(ext_ad_Ts), _SIZE_RFFT(ext_ad_Ts), 30},
	{"ExtAntDvry",		RFFT_T,		_OFFSET_RFFT(ExtAntDvry), _SIZE_RFFT(ExtAntDvry), 1},
#endif
#endif
	{"rssi_dump",		RFFT_T,		_OFFSET_RFFT(rssi_dump), _SIZE_RFFT(rssi_dump), 0},
	{"rxfifoO",		RFFT_T,		_OFFSET_RFFT(rxfifoO), _SIZE_RFFT(rxfifoO), 0},
	{"raGoDownUpper",	RFFT_T,	_OFFSET_RFFT(raGoDownUpper), _SIZE_RFFT(raGoDownUpper), 50},
#if defined(CONFIG_RTL8192SE)
	{"raGoDown20MLower",RFFT_T,	_OFFSET_RFFT(raGoDown20MLower), _SIZE_RFFT(raGoDown20MLower), 18},
	{"raGoDown40MLower",RFFT_T,	_OFFSET_RFFT(raGoDown40MLower), _SIZE_RFFT(raGoDown40MLower), 15},
#else
	{"raGoDown20MLower",RFFT_T,	_OFFSET_RFFT(raGoDown20MLower), _SIZE_RFFT(raGoDown20MLower), 30},
	{"raGoDown40MLower",RFFT_T,	_OFFSET_RFFT(raGoDown40MLower), _SIZE_RFFT(raGoDown40MLower), 10},
#endif
	{"raGoUpUpper",		RFFT_T,	_OFFSET_RFFT(raGoUpUpper), _SIZE_RFFT(raGoUpUpper), 55},
#if defined(CONFIG_RTL8192SE)
	{"raGoUp20MLower",	RFFT_T,	_OFFSET_RFFT(raGoUp20MLower), _SIZE_RFFT(raGoUp20MLower), 23},
	{"raGoUp40MLower",	RFFT_T,	_OFFSET_RFFT(raGoUp40MLower), _SIZE_RFFT(raGoUp40MLower), 20},
#else
	{"raGoUp20MLower",	RFFT_T,	_OFFSET_RFFT(raGoUp20MLower), _SIZE_RFFT(raGoUp20MLower), 35},
	{"raGoUp40MLower",	RFFT_T,	_OFFSET_RFFT(raGoUp40MLower), _SIZE_RFFT(raGoUp40MLower), 15},
#endif
	{"digGoLowerLevel",	RFFT_T,	_OFFSET_RFFT(digGoLowerLevel), _SIZE_RFFT(digGoLowerLevel), 35},
	{"digGoUpperLevel",	RFFT_T,	_OFFSET_RFFT(digGoUpperLevel), _SIZE_RFFT(digGoUpperLevel), 40},
#if defined(CONFIG_RTL8192SE)
	{"dcThUpper",		RFFT_T,	_OFFSET_RFFT(dcThUpper), _SIZE_RFFT(dcThUpper), 30},
	{"dcThLower",		RFFT_T,	_OFFSET_RFFT(dcThLower), _SIZE_RFFT(dcThLower), 25},
#else
	{"dcThUpper",		RFFT_T,	_OFFSET_RFFT(dcThUpper), _SIZE_RFFT(dcThUpper), 25},
	{"dcThLower",		RFFT_T,	_OFFSET_RFFT(dcThLower), _SIZE_RFFT(dcThLower), 20},
#endif
	{"rssiTx20MUpper",	RFFT_T,	_OFFSET_RFFT(rssiTx20MUpper), _SIZE_RFFT(rssiTx20MUpper), 20},
	{"rssiTx20MLower",	RFFT_T,	_OFFSET_RFFT(rssiTx20MLower), _SIZE_RFFT(rssiTx20MLower), 15},
	{"rssi_expire_to",	RFFT_T,	_OFFSET_RFFT(rssi_expire_to), _SIZE_RFFT(rssi_expire_to), 60},

	// bcm old 11n chipset iot debug
	{"tx_pwr_ctrl",		RFFT_T,	_OFFSET_RFFT(tx_pwr_ctrl), _SIZE_RFFT(tx_pwr_ctrl), 1},

	// 11n ap AES debug
	{"aes_check_th",	RFFT_T,	_OFFSET_RFFT(aes_check_th), _SIZE_RFFT(aes_check_th), 2},

	// Tx power tracking
	{"tpt_period",		RFFT_T,	_OFFSET_RFFT(tpt_period), _SIZE_RFFT(tpt_period), 30},

	// TXOP enlarge
	{"txop_enlarge_upper",		RFFT_T,	_OFFSET_RFFT(txop_enlarge_upper), _SIZE_RFFT(txop_enlarge_upper), 20},
	{"txop_enlarge_lower",		RFFT_T,	_OFFSET_RFFT(txop_enlarge_lower), _SIZE_RFFT(txop_enlarge_lower), 15},

	// 2.3G support
	{"frq_2_3G",		RFFT_T,	_OFFSET_RFFT(use_frq_2_3G), _SIZE_RFFT(use_frq_2_3G), 0},

	// for mp test
#if !defined(NEW_MAC80211_DRV) && defined(NEW_MACMP_TEST)
	{"mp_specific",		RFFT_T,	_OFFSET_RFFT(mp_specific), _SIZE_RFFT(mp_specific), 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(IGMP_FILTER_CMO)
	{"igmp_deny",		RFFT_T,	_OFFSET_RFFT(igmp_deny), _SIZE_RFFT(igmp_deny), 0},
#endif
	//Support IP multicast->unicast
#if !defined(NEW_MAC80211_DRV) && defined(SUPPORT_TX_MCAST2UNI)
	{"mc2u_disable",	RFFT_T,	_OFFSET_RFFT(mc2u_disable), _SIZE_RFFT(mc2u_disable), 0},
#endif

#if defined(RTL_TX_AMPDU)
	{"ampduMax",		RFFT_T,	_OFFSET_RFFT(dot11nAMPDUBufferMax), _SIZE_RFFT(dot11nAMPDUBufferMax), 300},
	{"ampduTimeout",	RFFT_T,	_OFFSET_RFFT(dot11nAMPDUBufferTimeout), _SIZE_RFFT(dot11nAMPDUBufferTimeout), 400},
	{"ampduNum",		RFFT_T,	_OFFSET_RFFT(dot11nAMPDUBufferNum), _SIZE_RFFT(dot11nAMPDUBufferNum), 24},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(HIGH_POWER_EXT_PA)
	{"use_ext_pa",		RFFT_T,	_OFFSET_RFFT(use_ext_pa), _SIZE_RFFT(use_ext_pa), 0},
#endif

#if !defined(NEW_MAC80211_DRV) && defined(ADD_TX_POWER_BY_CMD)
	{"txPowerPlus_cck",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_cck), _SIZE_RFFT(txPowerPlus_cck), 0xff},
	{"txPowerPlus_ofdm_6",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_6), _SIZE_RFFT(txPowerPlus_ofdm_6), 0xff},
	{"txPowerPlus_ofdm_9",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_9), _SIZE_RFFT(txPowerPlus_ofdm_9), 0xff},
	{"txPowerPlus_ofdm_12",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_12), _SIZE_RFFT(txPowerPlus_ofdm_12), 0xff},
	{"txPowerPlus_ofdm_18",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_18), _SIZE_RFFT(txPowerPlus_ofdm_18), 0xff},
	{"txPowerPlus_ofdm_24",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_24), _SIZE_RFFT(txPowerPlus_ofdm_24), 0xff},
	{"txPowerPlus_ofdm_36",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_36), _SIZE_RFFT(txPowerPlus_ofdm_36), 0xff},
	{"txPowerPlus_ofdm_48",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_48), _SIZE_RFFT(txPowerPlus_ofdm_48), 0xff},
	{"txPowerPlus_ofdm_54",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_ofdm_54), _SIZE_RFFT(txPowerPlus_ofdm_54), 0xff},
	{"txPowerPlus_mcs_0",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_0), _SIZE_RFFT(txPowerPlus_mcs_0), 0xff},
	{"txPowerPlus_mcs_1",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_1), _SIZE_RFFT(txPowerPlus_mcs_1), 0xff},
	{"txPowerPlus_mcs_2",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_2), _SIZE_RFFT(txPowerPlus_mcs_2), 0xff},
	{"txPowerPlus_mcs_3",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_3), _SIZE_RFFT(txPowerPlus_mcs_3), 0xff},
	{"txPowerPlus_mcs_4",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_4), _SIZE_RFFT(txPowerPlus_mcs_4), 0xff},
	{"txPowerPlus_mcs_5",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_5), _SIZE_RFFT(txPowerPlus_mcs_5), 0xff},
	{"txPowerPlus_mcs_6",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_6), _SIZE_RFFT(txPowerPlus_mcs_6), 0xff},
	{"txPowerPlus_mcs_7",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_7), _SIZE_RFFT(txPowerPlus_mcs_7), 0xff},
	{"txPowerPlus_mcs_8",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_8), _SIZE_RFFT(txPowerPlus_mcs_8), 0xff},
	{"txPowerPlus_mcs_9",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_9), _SIZE_RFFT(txPowerPlus_mcs_9), 0xff},
	{"txPowerPlus_mcs_10",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_10), _SIZE_RFFT(txPowerPlus_mcs_10), 0xff},
	{"txPowerPlus_mcs_11",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_11), _SIZE_RFFT(txPowerPlus_mcs_11), 0xff},
	{"txPowerPlus_mcs_12",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_12), _SIZE_RFFT(txPowerPlus_mcs_12), 0xff},
	{"txPowerPlus_mcs_13",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_13), _SIZE_RFFT(txPowerPlus_mcs_13), 0xff},
	{"txPowerPlus_mcs_14",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_14), _SIZE_RFFT(txPowerPlus_mcs_14), 0xff},
	{"txPowerPlus_mcs_15",	RFFT_T,	_OFFSET_RFFT(txPowerPlus_mcs_15), _SIZE_RFFT(txPowerPlus_mcs_15), 0xff},
#endif
	{"rootFwBeacon",		RFFT_T,	_OFFSET_RFFT(rootFwBeacon), _SIZE_RFFT(rootFwBeacon), 0},
	{"ledBlinkingFreq",		RFFT_T,	_OFFSET_RFFT(ledBlinkingFreq), _SIZE_RFFT(ledBlinkingFreq), 1},
};

/* Bit mask value for flags, compatiable with old driver */
#define STA_INFO_FLAG_AUTH_OPEN     	0x01
#define STA_INFO_FLAG_AUTH_WEP      	0x02
#define STA_INFO_FLAG_ASOC          	0x04
#define STA_INFO_FLAG_ASLEEP        	0x08

#ifdef _DEBUG_RTL8190_
unsigned long rtl8190_debug_err=0xffffffff;
unsigned long rtl8190_debug_info=0;
unsigned long rtl8190_debug_trace=0;
unsigned long rtl8190_debug_warn=0;
#endif

/*
int _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				_DEBUG_ERR("error hex format!\n");
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}
*/

static struct iwpriv_arg *get_tbl_entry(char *pstr)
{
	int i=0;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);
	char name[128];

	while (*pstr && *pstr != '=')
		name[i++] = *pstr++;
	name[i] = '\0';

	for (i=0; i<arg_num; i++) {
		if (!strcmp(name, mib_table[i].name)) {
			return &mib_table[i];
		}
	}
	return NULL;
}

int get_array_val(unsigned char *dst, char *src, int len)
{
	char tmpbuf[4];
	int num=0;

	while (len > 0) {
		memcpy(tmpbuf, src, 2);
		tmpbuf[2]='\0';
		*dst++ = (unsigned char)_atoi(tmpbuf, 16);
		len-=2;
		src+=2;
		num++;
	}
	return num;
}


char *get_arg(char *src, char *val)
{
	int len=0;

	while (*src && *src!=',') {
		*val++ = *src++;
		len++;
	}
	if (len == 0)
		return NULL;

	*val = '\0';

	if (*src==',')
		src++;

	return src;
}

static int set_mib(struct mac80211_shared_priv *priv, unsigned char *data)
{
	struct iwpriv_arg *entry;
	int int_val, int_idx, len, *int_ptr;
	unsigned char byte_val;
	char *arg_val, tmpbuf[100];
#ifdef 	CONFIG_RTK_MESH
	unsigned short word;
#endif
	DEBUG_TRACE;

	//DEBUG_INFO("set_mib %s\n", data);
	printk("set_mib %s\n", data);

	entry = get_tbl_entry((char *)data);
	if (entry == NULL) {
		DEBUG_ERR("invalid mib name [%s] !\n", data);
		return -1;
	}

	// search value
	arg_val = (char *)data;
	while (*arg_val && *arg_val != '=')
		arg_val++;

	if (!*arg_val) {
		DEBUG_ERR("mib value empty [%s] !\n", data);
		return -1;
	}
	arg_val++;

	switch (entry->type) {
	case BYTE_T:
		byte_val = (unsigned char)_atoi(arg_val, 10);
		memcpy(((unsigned char *)priv->mib)+entry->offset, &byte_val,  1);
		break;
#ifdef 	CONFIG_RTK_MESH
	case WORD_T:
		word = (unsigned short)_atoi(arg_val, 10);
		memcpy(((unsigned char *)priv->mib)+entry->offset, &word,  2);
		break;
#endif
	case INT_T:
		int_val = _atoi(arg_val, 10);
#ifdef WIFI_SIMPLE_CONFIG
		if (strcmp(entry->name, "wsc_enable") == 0) {
			if (int_val == 4) { // disable hidden AP
				if (HIDDEN_AP && priv->pbeacon_ssid) {
					memcpy(priv->pbeacon_ssid, SSID, SSID_LEN);
					priv->hidden_ap_mib_backup = HIDDEN_AP;
					HIDDEN_AP = 0;
				}
				break;
			}
			if (int_val == 5) { // restore hidden AP
				if (priv->pbeacon_ssid && !HIDDEN_AP && priv->hidden_ap_mib_backup) {
					memset(priv->pbeacon_ssid, '\0', 32);
					HIDDEN_AP = priv->hidden_ap_mib_backup;
				}
				break;
			}
#ifdef CLIENT_MODE
			if ((priv->mib->wscEntry.wsc_enable == 1) && (int_val == 0)) { // for client mode
				if (priv->recover_join_req) {
					priv->recover_join_req = 0;
					priv->mib->wscEntry.wsc_enable = 0;
					memcpy(&priv->mib->dot11Bss, &priv->dot11Bss_original, sizeof(struct bss_desc));
					memcpy(SSID2SCAN, priv->mib->dot11Bss.ssid, priv->mib->dot11Bss.ssidlen);
					SSID2SCAN_LEN = priv->mib->dot11Bss.ssidlen;
					SSID_LEN = SSID2SCAN_LEN;
					memcpy(SSID, SSID2SCAN, SSID_LEN);
					memset(BSSID, 0, MACADDRLEN);
					priv->join_req_ongoing = 1;
					priv->authModeRetry = 0;
					start_clnt_join(priv);
					break;
				}
			}
			else if ((priv->mib->wscEntry.wsc_enable == 0) && (int_val == 1))
				memcpy(&priv->dot11Bss_original, &priv->mib->dot11Bss, sizeof(struct bss_desc));

#endif
		}
#endif
		memcpy(((unsigned char *)priv->mib)+entry->offset, (unsigned char *)&int_val, sizeof(int));
		break;

	/*case SSID_STRING_T:
		if (strlen(arg_val) > entry->len)
			arg_val[entry->len] = '\0';
		memset(priv->mib->dot11StationConfigEntry.dot11DesiredSSID, 0, sizeof(priv->mib->dot11StationConfigEntry.dot11DesiredSSID));
		memcpy(priv->mib->dot11StationConfigEntry.dot11DesiredSSID, arg_val, strlen(arg_val));
		priv->mib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(arg_val);
		if ((SSID_LEN == 3) &&
			((SSID[0] == 'A') || (SSID[0] == 'a')) &&
			((SSID[1] == 'N') || (SSID[1] == 'n')) &&
			((SSID[2] == 'Y') || (SSID[2] == 'y'))) {
			SSID2SCAN_LEN = 0;
			memset(SSID2SCAN, 0, 32);
		}
		else {
			SSID2SCAN_LEN = SSID_LEN;
			memcpy(SSID2SCAN, SSID, SSID_LEN);
		}
		break;*/

	case BYTE_ARRAY_T:
		len = strlen(arg_val);
		if (len/2 > entry->len) {
			DEBUG_ERR("invalid len of BYTE_ARRAY_T mib [%s] !\n", entry->name);
			return -1;
		}
		if (len%2) {
			DEBUG_ERR("invalid len of BYTE_ARRAY_T mib [%s] !\n", entry->name);
			return -1;
		}
		get_array_val(((unsigned char *)priv->mib)+entry->offset, arg_val, strlen(arg_val));
		break;

	case ACL_T:
	case ACL_INT_T:
		arg_val = get_arg(arg_val, tmpbuf);
		if (arg_val == NULL) {
			DEBUG_ERR("invalid ACL_T addr [%s] !\n", entry->name);
			return -1;
		}
		if (entry->type == ACL_T && strlen(tmpbuf)!=12) {
			DEBUG_ERR("invalid len of ACL_T mib [%s] !\n", entry->name);
			return -1;
		}
		int_ptr = (int *)(((unsigned char *)priv->mib)+entry->offset+entry->len);
		int_idx = *int_ptr;
		if (entry->type == ACL_T)
			get_array_val(((unsigned char *)priv->mib)+entry->offset+int_idx*6, tmpbuf, 12);
		else {
			get_array_val(((unsigned char *)priv->mib)+entry->offset+int_idx*(6+4), tmpbuf, 12);
			if (strlen(arg_val) > 0) {
				int_val = _atoi(arg_val, 10);
				memcpy(((unsigned char *)priv->mib)+entry->offset+int_idx*(6+4)+6, &int_val, 4);
			}
		}
		*int_ptr = *int_ptr + 1;
		break;

	case IDX_BYTE_ARRAY_T:
		arg_val = get_arg(arg_val, tmpbuf);
		if (arg_val == NULL) {
			DEBUG_ERR("invalid BYTE_ARRAY mib [%s] !\n", entry->name);
			return -1;
		}
		int_idx = _atoi(tmpbuf, 10);
		if (int_idx+1 > entry->len) {
			DEBUG_ERR("invalid BYTE_ARRAY mib index [%s, %d] !\n", entry->name, int_idx);
			return -1;
		}
		arg_val = get_arg(arg_val, tmpbuf);
		if (arg_val == NULL) {
			DEBUG_ERR("invalid BYTE_ARRAY mib [%s] !\n", entry->name);
			return -1;
		}
		byte_val = (unsigned char)_atoi(tmpbuf, 10);
		memcpy(((unsigned char *)priv->mib)+entry->offset+int_idx, (unsigned char *)&byte_val, sizeof(byte_val));
		break;

	case MULTI_BYTE_T:
		int_idx=0;
		while (1) {
			arg_val = get_arg(arg_val, tmpbuf);
			if (arg_val == NULL)
				break;
			if (int_idx+1 > entry->len) {
				DEBUG_ERR("invalid MULTI_BYTE_T mib index [%s, %d] !\n", entry->name, int_idx);
				return -1;
			}
			byte_val = (unsigned char)_atoi(tmpbuf, 16);
			memcpy(((unsigned char *)priv->mib)+entry->offset+int_idx++, (unsigned char *)&byte_val, sizeof(byte_val));
		}
		// copy length to next parameter
		memcpy( ((unsigned char *)priv->mib)+entry->offset+entry->len, (unsigned char *)&int_idx, sizeof(int));
		break;

#ifdef _DEBUG_RTL8190_
	case DEBUG_T:
		int_val = _atoi(arg_val, 16);
		if (entry->offset==1)
			rtl8190_debug_err = int_val;
		else if (entry->offset==2)
			rtl8190_debug_info = int_val;
		else if (entry->offset==3)
			rtl8190_debug_trace = int_val;
		else if (entry->offset==4)
			rtl8190_debug_warn = int_val;
		else {
			DEBUG_ERR("invalid debug index\n");
		}
		break;
#endif // _DEBUG_RTL8190_

	case DEF_SSID_STRING_T:
		if (strlen(arg_val) > entry->len)
			arg_val[entry->len] = '\0';
		memset(priv->mib->dot11StationConfigEntry.dot11DefaultSSID, 0, sizeof(priv->mib->dot11StationConfigEntry.dot11DefaultSSID));
		memcpy(priv->mib->dot11StationConfigEntry.dot11DefaultSSID, arg_val, strlen(arg_val));
		priv->mib->dot11StationConfigEntry.dot11DefaultSSIDLen = strlen(arg_val);
		break;

	case STRING_T:
		if (strlen(arg_val) > entry->len)
			arg_val[entry->len] = '\0';
		strcpy(((unsigned char *)priv->mib)+entry->offset, arg_val);
		break;

	case RFFT_T:
		if (entry->len == 1) {
			byte_val = _atoi(arg_val, 10);
			memcpy(((unsigned char *)&priv->pshare_hw->rf_ft_var)+entry->offset, (unsigned char *)&byte_val, entry->len);
		}
		else if (entry->len == 4) {
			int_val = _atoi(arg_val, 10);
			memcpy(((unsigned char *)&priv->pshare_hw->rf_ft_var)+entry->offset, (unsigned char *)&int_val, entry->len);
		}
		break;

	case VARLEN_BYTE_T:
		len = strlen(arg_val);
		if (len/2 > entry->len) {
			DEBUG_ERR("invalid len of VARLEN_BYTE_T mib [%s] !\n", entry->name);
			return -1;
		}
		if (len%2) {
			DEBUG_ERR("invalid len of VARLEN_BYTE_T mib [%s] !\n", entry->name);
			return -1;
		}
		memset(((unsigned char *)priv->mib)+entry->offset, 0, entry->len);
		len = get_array_val(((unsigned char *)priv->mib)+entry->offset, arg_val, strlen(arg_val));
		*(unsigned int *)(((unsigned char *)priv->mib)+entry->offset+entry->len) = len;
		break;

#ifdef WIFI_SIMPLE_CONFIG
	case PIN_IND_T:
		if (strlen(arg_val) > entry->len) {
			DOT11_WSC_PIN_IND wsc_ind;

			wsc_ind.EventId = DOT11_EVENT_WSC_PIN_IND;
			wsc_ind.IsMoreEvent = 0;
			strcpy(wsc_ind.code, arg_val);
			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&wsc_ind, sizeof(DOT11_WSC_PIN_IND));
			event_indicate(priv, NULL, -1);
		}
		break;

#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	case INT_ARRAY_T:
		int_idx=0;
		while (1) {
			arg_val = get_arg(arg_val, tmpbuf);
			if (arg_val == NULL)
				break;
			if (int_idx+1 > (entry->len)/sizeof(int)) {
				DEBUG_ERR("invalid MULTI_BYTE_T mib index [%s, %d] !\n", entry->name, int_idx);
				return -1;
			}
			int_val = _atoi(tmpbuf, 16);
			memcpy(((unsigned char *)priv->mib)+entry->offset+int_idx++, (void *)&int_val, sizeof(int_val));
		}
		break;
	case WAPI_KEY_T:
		{
			char tmppasswd[100]={0};
			wapiMibPSK *wapipsk=NULL;
			int_idx=0;

			/*Get Password*/
			arg_val = get_arg(arg_val, tmpbuf);
			if (arg_val == NULL)
				break;
			memcpy(tmppasswd, tmpbuf, strlen(tmpbuf));

			/*Get Password length*/
			arg_val=get_arg(arg_val, tmpbuf);
			int_val = _atoi(tmpbuf, 16);

			wapipsk=(wapiMibPSK *)((unsigned char *)(priv->mib)+entry->offset);

			/*Hex or passthru*/
			if((0==(strlen(tmppasswd) % 2))  && (int_val < strlen(tmppasswd)) &&
				(int_val == (strlen(tmppasswd)/2)))
			{
				/*Hex mode*/
				string_to_hex(tmppasswd,wapipsk->octet,strlen(tmppasswd));
			}
			else
			{
				strncpy(wapipsk->octet,tmppasswd,strlen(tmppasswd));
			}
			wapipsk->len = int_val;
			break;
	      }
#endif
	default:
		DEBUG_ERR("invalid mib type!\n");
		break;
	}

	return 0;
}

static int get_mib(struct mac80211_shared_priv *priv, unsigned char *data)
{
	struct iwpriv_arg *entry;
	struct wifi_mib *mib = priv->mib;
	int i, len, *int_ptr, copy_len;
	char tmpbuf[40];

	DEBUG_TRACE;

	//DEBUG_INFO("get_mib %s\n", data);
	printk("get_mib %s\n", data);

	entry = get_tbl_entry((char *)data);
	if (entry == NULL) {
		//DEBUG_ERR("invalid mib name [%s] !\n", data);
		printk("invalid mib name [%s] !\n", data);
		return -1;
	}
	copy_len = entry->len;

	switch (entry->type) {
	case BYTE_T:
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset,  1);
		PRINT_INFO("byte data: %d\n", *data);
		break;
#ifdef 	CONFIG_RTK_MESH
	case WORD_T:
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset,  2);
		PRINT_INFO("word data: %d\n", *data);
		break;
#endif
	case INT_T:
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset, sizeof(int));
		PRINT_INFO("int data: %d\n", *((int *)data));
		break;

	case SSID_STRING_T:
		memcpy(tmpbuf, priv->mib->dot11StationConfigEntry.dot11DesiredSSID, priv->mib->dot11StationConfigEntry.dot11DesiredSSIDLen);
		tmpbuf[priv->mib->dot11StationConfigEntry.dot11DesiredSSIDLen] = '\0';
		strcpy(data, tmpbuf);
		PRINT_INFO("ssid: %s\n", tmpbuf);
		break;

	case BYTE_ARRAY_T:
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset, entry->len);
		PRINT_INFO("data (hex): ");
		for (i=0; i<entry->len; i++)
			PRINT_INFO("%02x", *((unsigned char *)((unsigned char *)priv->mib)+entry->offset+i));
		PRINT_INFO("\n");
		break;

	case ACL_T:
		int_ptr = (int *)(((unsigned char *)priv->mib)+entry->offset+entry->len);
		PRINT_INFO("ACL table (%d):\n", *int_ptr);
		copy_len = 0;
		for (i=0; i<*int_ptr; i++) {
			memcpy(data, ((unsigned char *)priv->mib)+entry->offset+i*6, 6);
			PRINT_INFO("mac-addr: %02x-%02x-%02x-%02x-%02x-%02x\n",
				data[0],data[1],data[2],data[3],data[4],data[5]);
			data += 6;
			copy_len += 6;
		}
		DEBUG_INFO("\n");
		break;

	case IDX_BYTE_ARRAY_T:
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset, entry->len);
		PRINT_INFO("data (dec): ");
		for (i=0; i<entry->len; i++)
			PRINT_INFO("%d ", *((unsigned char *)((unsigned char *)priv->mib)+entry->offset+i));
		PRINT_INFO("\n");
		break;

	case MULTI_BYTE_T:
		memcpy(&len, ((unsigned char *)priv->mib)+entry->offset+entry->len, sizeof(int));
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset, len);
		PRINT_INFO("data (hex): ");
		for (i=0; i<len; i++)
			PRINT_INFO("%02x ", *((unsigned char *)((unsigned char *)priv->mib)+entry->offset+i));
		PRINT_INFO("\n");
		break;

#ifdef _DEBUG_RTL8190_
	case DEBUG_T:
		if (entry->offset==1)
			memcpy(data, (unsigned char *)&rtl8190_debug_err, sizeof(rtl8190_debug_err));
		else if (entry->offset==2)
			memcpy(data, (unsigned char *)&rtl8190_debug_info, sizeof(rtl8190_debug_info));
		else if (entry->offset==3)
			memcpy(data, (unsigned char *)&rtl8190_debug_trace, sizeof(rtl8190_debug_trace));
		else if (entry->offset==4)
			memcpy(data, (unsigned char *)&rtl8190_debug_warn, sizeof(rtl8190_debug_warn));
		else {
			DEBUG_ERR("invalid debug index\n");
		}
		PRINT_INFO("debug flag(hex): %08lx\n", *((unsigned long *)data));
		break;
#endif // _DEBUG_RTL8190_

	case DEF_SSID_STRING_T:
		memcpy(tmpbuf, priv->mib->dot11StationConfigEntry.dot11DefaultSSID, priv->mib->dot11StationConfigEntry.dot11DefaultSSIDLen);
		tmpbuf[priv->mib->dot11StationConfigEntry.dot11DefaultSSIDLen] = '\0';
		strcpy(data, tmpbuf);
		PRINT_INFO("defssid: %s\n", tmpbuf);
		break;

	case STRING_T:
		strcpy(data, ((unsigned char *)priv->mib)+entry->offset);
		PRINT_INFO("string data: %s\n", data);
		break;

	case RFFT_T:
		memcpy(data, ((unsigned char *)&priv->pshare_hw->rf_ft_var)+entry->offset, sizeof(int));
		PRINT_INFO("int data: %d\n", *((int *)data));
		break;

	case VARLEN_BYTE_T:
		copy_len = *(unsigned int *)(((unsigned char *)priv->mib)+entry->offset+entry->len);
		memcpy(data, ((unsigned char *)priv->mib)+entry->offset, copy_len);
		PRINT_INFO("data (hex): ");
		for (i=0; i<copy_len; i++)
			PRINT_INFO("%02x", *((unsigned char *)((unsigned char *)priv->mib)+entry->offset+i));
		PRINT_INFO("\n");
		break;

	default:
		DEBUG_ERR("invalid mib type!\n");
		return 0;
	}

	return copy_len;
}

#ifdef _IOCTL_DEBUG_CMD_
/*
 * Write register, command: "iwpriv wlanX write_reg,type,offset,value"
 * 	where: type may be: "b" - byte, "w" - word, "dw" - "dw" (based on wlan register offset)
 *			    "_b" - byte, "_w" - word, "_dw" - "dw" (based on register offset 0)
 *		offset and value should be input in hex
 */
static int write_reg(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char name[100];
	int i=0, op=0, offset;
	unsigned long ioaddr, val;

	DEBUG_TRACE;

	// get access type
	while (*data && *data != ',')
		name[i++] = *data++;
	name[i] = '\0';

	if (!strcmp(name, "b"))
		op = 1;
	else if (!strcmp(name, "w"))
		op = 2;
	else if (!strcmp(name, "dw"))
		op = 3;
	else if (!strcmp(name, "_b"))
		op = 0x81;
	else if (!strcmp(name, "_w"))
		op = 0x82;
	else if (!strcmp(name, "_dw"))
		op = 0x83;

	if (op == 0 || !*data++) {
		DEBUG_ERR("invalid type!\n");
		return -1;
	}

	if ( !(op&0x80))  // wlan register
		ioaddr = priv->pshare_hw->ioaddr;
	else
		ioaddr = 0;

	// get offset and value
	i=0;
	while (*data && *data != ',')
		name[i++] = *data++;
	name[i] = '\0';
	if (!*data++) {
		DEBUG_ERR("invalid offset!\n");
		return -1;
	}
	offset = _atoi(name, 16);
	val = (unsigned long)_atoi((char *)data, 16);

	DEBUG_INFO("write reg in %s: addr=%08x, val=0x%x\n",
			(op == 1 ? "byte" : (op == 2 ? "word" : "dword")),
			offset, (int)val);

	switch (op&0x7f) {
	case 1:
		RTL_W8(offset, ((unsigned char)val));
		break;
	case 2:
		RTL_W16(offset, ((unsigned short)val));
		break;
	case 3:
		RTL_W32(offset, ((unsigned long)val));
		break;
	}
	return 0;
}


/*
 * Read register, command: "iwpriv wlanX read_reg,type,offset"
 * 	where: type may be: "b" - byte, "w" - word, "dw" - "dw" (based on wlan register offset)
 *			    "_b" - byte, "_w" - word, "_dw" - "dw" (based on register offset 0)
 *		offset should be input in hex
 */
static int read_reg(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char name[100];
	int i=0, op=0, offset, len=0;
	unsigned long ioaddr, dw_val;
	unsigned char *org_ptr=data, b_val;
	unsigned short w_val;

	DEBUG_TRACE;

	// get access type
	while (*data && *data != ',')
		name[i++] = *data++;
	name[i] = '\0';

	if (!strcmp(name, "b"))
		op = 1;
	else if (!strcmp(name, "w"))
		op = 2;
	else if (!strcmp(name, "dw"))
		op = 3;
	else if (!strcmp(name, "_b"))
		op = 0x81;
	else if (!strcmp(name, "_w"))
		op = 0x82;
	else if (!strcmp(name, "_dw"))
		op = 0x83;

	if (op == 0 || !*data++) {
		DEBUG_ERR("invalid type!\n");
		return -1;
	}

	if ( !(op&0x80))  // wlan register
		ioaddr = priv->pshare_hw->ioaddr;
	else
		ioaddr = 0;

	// get offset
	offset = _atoi((char *)data, 16);

	switch (op&0x7f) {
	case 1:
		b_val = (unsigned char)RTL_R8(offset);
//		panic_printk("read byte reg %x=0x%02x\n", offset, b_val);
		len = 1;
		memcpy(org_ptr, &b_val, len);
		break;
	case 2:
		w_val = (unsigned short)RTL_R16(offset);
//		panic_printk("read word reg %x=0x%04x\n", offset, w_val);
		len = 2;
		memcpy(org_ptr, (char *)&w_val, len);
		break;
	case 3:
		dw_val = (unsigned long)RTL_R32(offset);
//		panic_printk("read dword reg %x=0x%08lx\n", offset, dw_val);
		len = 4;
		memcpy(org_ptr, (char *)&dw_val, len);
		break;
	}

	return len;
}


/*
 * Write memory, command: "iwpriv wlanX write_mem,type,start,len,value"
 * 	where: type may be: "b" - byte, "w" - word, "dw" - "dw"
 *		start, len and value should be input in hex
 */
static int write_mem(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char tmpbuf[100];
	int i=0, size=0, len;
	unsigned long val, start;

	DEBUG_TRACE;

	// get access type
	while (*data && *data != ',')
		tmpbuf[i++] = *data++;
	tmpbuf[i] = '\0';

	if (!strcmp(tmpbuf, "b"))
		size = 1;
	else if (!strcmp(tmpbuf, "w"))
		size = 2;
	else if (!strcmp(tmpbuf, "dw"))
		size = 4;

	if (size == 0 || !*data++) {
		DEBUG_ERR("invalid command!\n");
		return -1;
	}

	// get start, len, and value
	i=0;
	while (*data && *data != ',')
		tmpbuf[i++] = *data++;
	tmpbuf[i] = '\0';
	if (i==0 || !*data++) {
		DEBUG_ERR("invalid start!\n");
		return -1;
	}
	start = (unsigned long)_atoi(tmpbuf, 16);

	i=0;
	while (*data && *data != ',')
		tmpbuf[i++] = *data++;
	tmpbuf[i] = '\0';
	if (i==0 || !*data++) {
		DEBUG_ERR("invalid len!\n");
		return -1;
	}
	len = _atoi(tmpbuf, 16);
	val = (unsigned long)_atoi((char *)data, 16);

	DEBUG_INFO("write memory: start=%08lx, len=%x, data=0x%x (%s)\n",
		start,	len, (int)val,
		(size == 1 ? "byte" : (size == 2 ? "word" : "dword")));

	for (i=0; i<len; i++) {
		memcpy((char *)start, (char *)&val, size);
		start += size;
	}
	return 0;
}


/*
 * Read memory, command: "iwpriv wlanX read_mem,type,start,len"
 * 	where: type may be: "b" - byte, "w" - word, "dw" - "dw"
 *		start, and len should be input in hex
 */
static int read_mem(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char tmpbuf[100];

	int i=0, size=0, len, copy_len;
	unsigned long start, dw_val;
	unsigned short w_val;
	unsigned char b_val, *pVal=NULL, *org_ptr=data;

	DEBUG_TRACE;

	// get access type
	while (*data && *data != ',')
		tmpbuf[i++] = *data++;
	tmpbuf[i] = '\0';

	if (!strcmp(tmpbuf, "b")) {
		size = 1;
		pVal = &b_val;
	}
	else if (!strcmp(tmpbuf, "w")) {
		size = 2;
		pVal = (unsigned char *)&w_val;
	}
	else if (!strcmp(tmpbuf, "dw")) {
		size = 4;
		pVal = (unsigned char *)&dw_val;
	}

	if (size == 0 || !*data++) {
		DEBUG_ERR("invalid type!\n");
		return -1;
	}

	// get start and len
	i=0;
	while (*data && *data != ',')
		tmpbuf[i++] = *data++;
	tmpbuf[i] = '\0';
	if (i==0 || !*data++) {
		DEBUG_ERR("invalid start!\n");
		return -1;
	}
	start = (unsigned long)_atoi(tmpbuf, 16);
	len = _atoi((char *)data, 16);

	if (size*len > 128)
		copy_len = 128;
	else
		copy_len = size*len;
	memcpy(org_ptr,  (char *)start, copy_len);

	return copy_len;
}




static int write_rf_reg(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char tmpbuf[32];
	unsigned int path, offset, val, val_read;
	int i;

	DEBUG_TRACE;

	if (strlen(data) != 0) {
		i = 0;
		while (*data && *data != ',')
			tmpbuf[i++] = *data++;
		tmpbuf[i] = '\0';
		if (i==0 || !*data++) {
			DEBUG_ERR("invalid path!\n");
			return -1;
		}
		path = _atoi(tmpbuf, 16);

		i = 0;
		while (*data && *data != ',')
			tmpbuf[i++] = *data++;
		tmpbuf[i] = '\0';
		if (i==0 || !*data++) {
			DEBUG_ERR("invalid offset!\n");
			return -1;
		}
		offset = _atoi(tmpbuf, 16);

		val = (unsigned long)_atoi((char *)data, 16);

		PHY_SetRFReg(priv, path, offset, bMask12Bits, val);
		val_read = PHY_QueryRFReg(priv, path, offset, bMask12Bits, 1);
		printk("write RF %d offset 0x%02x val [0x%04x],  read back [0x%04x]\n",
			path, offset, (unsigned short)val, (unsigned short)val_read);
	}

	return 0;
}


static int read_rf_reg(struct mac80211_shared_priv *priv, unsigned char *data)
{
	char tmpbuf[32];
	unsigned char *arg = data;
	unsigned int path, offset, val;

	int i;

	DEBUG_TRACE;

	if (strlen(arg) != 0) {
		i = 0;
		while (*arg && *arg != ',')
			tmpbuf[i++] = *arg++;
		tmpbuf[i] = '\0';
		if (i==0 || !*arg++) {
			DEBUG_ERR("invalid path!\n");
			return -1;
		}
		path = _atoi(tmpbuf, 16);

		offset = (unsigned char)_atoi((char *)arg, 16);

		val = PHY_QueryRFReg(priv, path, offset, bMask20Bits, 1);
//		panic_printk("read RF %d reg %02x=0x%08x\n", path, offset, val);
		memcpy(data, (char *)&val, 4);
		return 4;

	}
	return 1;
}




#ifdef DEBUG_8190
static void reg_dump(struct mac80211_shared_priv *priv)
{
	int i, j, len;
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned char tmpbuf[100];

#ifdef CONFIG_RTL865X_WTDOG
	static unsigned long wtval;
	wtval = *((volatile unsigned long *)0xB800311C);
	*((volatile unsigned long *)0xB800311C) = 0xA5000000;	// disabe watchdog
#endif

//	panic_printk("\nMAC Registers:\n");
	for (i=0; i<0x1000; i+=0x10) {
		len = sprintf(tmpbuf, "%03X\t", i);
		for (j=i; j<i+0x10; j+=4)
			len += sprintf(tmpbuf+len, "%08X ", (unsigned int)RTL_R32(j));
		len += sprintf(tmpbuf+len, "\n");
//		panic_printk(tmpbuf);
	}
//	panic_printk("\n");

#ifdef RTL8192SE
//	panic_printk("\nRF Registers:\n");
	len = 0;
	for (i = RF90_PATH_A; i < priv->pshare_hw->phw->NumTotalRFPath; i++) {
		for (j = 0; j < 0x34; j++) {
			len += sprintf(tmpbuf+len, "%d%02x  %05x",
				i, j, PHY_QueryRFReg(priv, i, j, bMask20Bits, 1));

			if (j && !((j+1)%4))
				len += sprintf(tmpbuf+len, "\n");
			else
				len += sprintf(tmpbuf+len, "     ");
//			panic_printk(tmpbuf);
			len = 0;
		}
//		panic_printk("\n");
	}
#endif


#ifdef CONFIG_RTL865X_WTDOG
	*((volatile unsigned long *)0xB800311C) |=  1 << 23;
	*((volatile unsigned long *)0xB800311C) = wtval;
#endif
}
#endif
#endif // _IOCTL_DEBUG_CMD_





#ifdef DRVMAC_LB
void drvmac_loopback(struct mac80211_shared_priv *priv)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	struct stat_info *pstat;
	unsigned char *da = priv->mib->miscEntry.lb_da;

	// prepare station info
	if (memcmp(da, "\x0\x0\x0\x0\x0\x0", 6) && !IS_MCAST(da))
	{
		pstat = get_stainfo(priv, da);
		if (pstat == NULL)
		{
			pstat = alloc_stainfo(priv, da, -1);
			pstat->state = WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE;
			memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
			pstat->bssratelen = AP_BSSRATE_LEN;
			pstat->expire_to = 30000;
			list_add_tail(&pstat->asoc_list, &priv->asoc_list);
			cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
			if (QOS_ENABLE)
				pstat->QosEnabled = 1;
			if (priv->mib->dot11BssType.net_work_type & WIRELESS_11N) {
				pstat->ht_cap_len = priv->ht_cap_len;
				memcpy(&pstat->ht_cap_buf, &priv->ht_cap_buf, priv->ht_cap_len);
			}
			pstat->current_tx_rate = find_rate(priv, pstat, 1, 0);
			update_fwtbl_asoclst(priv, pstat);
			add_update_RATid(priv, pstat);
		}
	}

	// accept all packets
	RTL_W32(_RCR_, RTL_R32(_RCR_) | _AAP_);

	// enable MAC loopback
	RTL_W32(_CPURST_, RTL_R32(_CPURST_) | BIT(16) | BIT(17));
}
#endif // DRVMAC_LB


int dynamic_RF_pwr_adj(struct mac80211_shared_priv *priv, unsigned char *data)
{
	unsigned char *ptr = data;
	int index, minus_sign, adj_value;
	unsigned int writeVal, readVal;
	unsigned char byte0, byte1, byte2, byte3;
	unsigned char RF6052_MAX_TX_PWR = 0x3F;

	if (*ptr == '-') {
		minus_sign = 1;
		ptr++;
	}
	else if (*ptr == '+') {
		minus_sign = 0;
		ptr++;
	}
	else {
		sprintf(data, "[FAIL] No sign to know to add or subtract\n");
		return strlen(data)+1;
	}

	adj_value = _atoi(ptr, 10);
	if (adj_value >= 64) {
		sprintf(data, "[FAIL] Adjust value too large\n");
		return strlen(data)+1;
	}

	for (index=0; index<8; index++)
	{
		if ((index == 2) || (index == 3))
			continue;

		readVal = PHY_QueryBBReg(priv, rTxAGC_Rate18_06+index*4, 0x7f7f7f7f);
		byte0 = (readVal & 0xff000000) >> 24;
		byte1 = (readVal & 0x00ff0000) >> 16;
		byte2 = (readVal & 0x0000ff00) >> 8;
		byte3 = (readVal & 0x000000ff);

		if (minus_sign) {
			if (byte0 >= adj_value)
				byte0 -= adj_value;
			else
				byte0 = 0;
			if (byte1 >= adj_value)
				byte1 -= adj_value;
			else
				byte1 = 0;
			if (byte2 >= adj_value)
				byte2 -= adj_value;
			else
				byte2 = 0;
			if (byte3 >= adj_value)
				byte3 -= adj_value;
			else
				byte3 = 0;
		}
		else {
			byte0 += adj_value;
			byte1 += adj_value;
			byte2 += adj_value;
			byte3 += adj_value;
		}

		// Max power index = 0x3F Range = 0-0x3F
		if (byte0 > RF6052_MAX_TX_PWR)
			byte0 = RF6052_MAX_TX_PWR;
		if (byte1 > RF6052_MAX_TX_PWR)
			byte1 = RF6052_MAX_TX_PWR;
		if (byte2 > RF6052_MAX_TX_PWR)
			byte2 = RF6052_MAX_TX_PWR;
		if (byte3 > RF6052_MAX_TX_PWR)
			byte3 = RF6052_MAX_TX_PWR;

		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		PHY_SetBBReg(priv, rTxAGC_Rate18_06+index*4, 0x7f7f7f7f, writeVal);
	}

	byte0 = PHY_QueryBBReg(priv, rTxAGC_CCK_Mcs32, bTxAGCRateCCK);
	if (minus_sign)
		byte0 -= adj_value;
	else
		byte0 += adj_value;
	if (byte0 > RF6052_MAX_TX_PWR)
		byte0 = RF6052_MAX_TX_PWR;
	PHY_SetBBReg(priv, rTxAGC_CCK_Mcs32, bTxAGCRateCCK, byte0);

	sprintf(data, "[SUCCESS] %s %d level RF power\n", minus_sign?"Subtract":"Add", adj_value);
	return strlen(data)+1;
}



//int rtl819x_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
int rtl819x_ioctl(void *rtl_priv, struct iw_point *ptr, int cmd)
{
	struct mac80211_shared_priv	*priv = (struct mac80211_shared_priv *)rtl_priv;

//	struct mac80211_shared_priv	*priv = (struct mac80211_shared_priv *) 2000;

	unsigned long flags;
	//struct iwreq *wrq = (struct iwreq *) ifr;
	//struct iw_point *iw_ptr = ptr;
	unsigned char *tmpbuf;	//, *tmp1;
//	UINT16 sta_num;
	int	i = 0, ret = -1, sizeof_tmpbuf;

	static unsigned char tmpbuf1[1024];

	DEBUG_TRACE;

	sizeof_tmpbuf = sizeof(tmpbuf1);
	tmpbuf = tmpbuf1;
	memset(tmpbuf, '\0', sizeof_tmpbuf);

	SAVE_INT_AND_CLI(flags);

	switch ( cmd )
	{

	/*case SIOCGIWNAME:
		strcpy(wrq->u.name, "IEEE 802.11-DS");
		ret = 0;
		break;*/


	case SIOCGIWPRIV:	//-- get private ioctls for iwpriv --//
	        printk("try to get mib by iwprv\n");
		if (ptr->pointer) {
			struct iw_priv_args privtab[] = {
				{ RTL8190_IOCTL_SET_MIB, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_BYTE | 128, "set_mib" },
			    { RTL8190_IOCTL_GET_MIB, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_BYTE | 128, "get_mib" },
#ifdef _IOCTL_DEBUG_CMD_
				{ RTL8190_IOCTL_WRITE_REG, IW_PRIV_TYPE_CHAR | 128, 0, "write_reg" },
				{ RTL8190_IOCTL_READ_REG, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | 128, "read_reg" },
				{ RTL8190_IOCTL_WRITE_MEM, IW_PRIV_TYPE_CHAR | 128, 0, "write_mem" },
				{ RTL8190_IOCTL_READ_MEM, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | 128, "read_mem" },
				{ RTL8190_IOCTL_WRITE_RF_REG, IW_PRIV_TYPE_CHAR | 128, 0, "write_rf" },
				{ RTL8190_IOCTL_READ_RF_REG, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | 128, "read_rf" },
#endif

/*
#ifdef MP_TEST
				{ MP_START_TEST, IW_PRIV_TYPE_NONE, 0, "mp_start" },
				{ MP_STOP_TEST, IW_PRIV_TYPE_NONE, 0, "mp_stop" },
				{ MP_SET_RATE, IW_PRIV_TYPE_CHAR | 40, 0, "mp_rate" },
				{ MP_SET_CHANNEL, IW_PRIV_TYPE_CHAR | 40, 0, "mp_channel" },
				{ MP_SET_TXPOWER, IW_PRIV_TYPE_CHAR | 40, 0, "mp_txpower" },
				{ MP_CONTIOUS_TX, IW_PRIV_TYPE_CHAR | 128, 0, "mp_ctx" },
				{ MP_ARX, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 128, "mp_arx" },
				{ MP_SET_BSSID, IW_PRIV_TYPE_CHAR | 40, 0, "mp_bssid" },
				{ MP_ANTENNA_TX, IW_PRIV_TYPE_CHAR | 40, 0, "mp_ant_tx" },
				{ MP_ANTENNA_RX, IW_PRIV_TYPE_CHAR | 40, 0, "mp_ant_rx" },
				{ MP_SET_BANDWIDTH, IW_PRIV_TYPE_CHAR | 40, 0, "mp_bandwidth" },
				{ MP_SET_PHYPARA, IW_PRIV_TYPE_CHAR | 40, 0, "mp_phypara" },
#ifdef B2B_TEST
				{ MP_TX_PACKET, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR | 128, "mp_tx" },
				{ MP_BRX_PACKET, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 128, "mp_brx" },

#endif
				{ MP_QUERY_STATS, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 128, "mp_query" },
				{ MP_TXPWR_TRACK, IW_PRIV_TYPE_CHAR | 40, 0, "mp_pwrtrk" },
				{ MP_QUERY_TSSI, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 128, "mp_tssi" },
#ifdef RTL8192SE
				{ MP_QUERY_THER, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 128, "mp_ther" },
#endif
#endif // MP_TEST
*/
#ifdef DEBUG_8190
				{ SIOCGIWRTLREGDUMP, IW_PRIV_TYPE_CHAR | 40, 0, "reg_dump" },
#endif


			};
#ifdef __LINUX_2_6__
			ret = access_ok(VERIFY_WRITE, (const void *)ptr->pointer, sizeof(privtab));
			if (!ret) {
				ret = -EFAULT;
				DEBUG_ERR("user space valid check error!\n");
				break;
			}
#else
			ret = verify_area(VERIFY_WRITE, (const void *)ptr->pointer, sizeof(privtab));
			if (ret) {
				DEBUG_ERR("verify_area() error!\n");
				break;
			}
#endif

			ptr->length = sizeof(privtab) / sizeof(privtab[0]);
			if (copy_to_user((void *)ptr->pointer, privtab, sizeof(privtab)))
				ret = -EFAULT;
		}
		break;

	case RTL8190_IOCTL_SET_MIB:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		ret = set_mib(priv, tmpbuf);
		break;

	case RTL8190_IOCTL_GET_MIB:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		i = get_mib(priv, tmpbuf);
		if (i >= 0) {
			if ((i > 0) && copy_to_user((void *)ptr->pointer, tmpbuf, i))
				break;
			ptr->length = i;
			ret = 0;
		}
		break;
#ifdef _IOCTL_DEBUG_CMD_
	case RTL8190_IOCTL_WRITE_REG:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		ret = write_reg(priv, tmpbuf);
		break;

	case RTL8190_IOCTL_READ_REG:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		i = read_reg(priv, tmpbuf);
		if (i >= 0) {
			if ((i > 0) && copy_to_user((void *)ptr->pointer, tmpbuf, i))
				break;
			ptr->length = i;
			ret = 0;
		}
		break;

	case RTL8190_IOCTL_WRITE_MEM:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		ret = write_mem(priv, tmpbuf);
		break;

	case RTL8190_IOCTL_READ_MEM:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
				break;
		i = read_mem(priv, tmpbuf);
		if (i >= 0) {
			if ((i > 0) && copy_to_user((void *)ptr->pointer, tmpbuf, i))
				break;
			ptr->length = i;
			ret = 0;
		}
		break;


	case RTL8190_IOCTL_WRITE_RF_REG:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		ret = write_rf_reg(priv, tmpbuf);
		break;

	case RTL8190_IOCTL_READ_RF_REG:
		if ((ptr->length > sizeof_tmpbuf) ||
			copy_from_user(tmpbuf, (void *)ptr->pointer, ptr->length))
			break;
		i = read_rf_reg(priv, tmpbuf);
		if (i >= 0) {
			if ((i > 0) && copy_to_user((void *)ptr->pointer, tmpbuf, i))
				break;
			ptr->length = i;
			ret = 0;
		}
		break;
#endif // _IOCTL_DEBUG_CMD_




/*
#ifdef MP_TEST
	case MP_START_TEST:
		mp_start_test(priv);
		ret = 0;
		break;

	case MP_STOP_TEST:
		mp_stop_test(priv);
		ret = 0;
		break;

	case MP_SET_RATE:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_datarate(priv, tmpbuf);
		ret = 0;
		break;

	case MP_SET_CHANNEL:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_channel(priv, tmpbuf);
		ret = 0;
		break;

	case MP_SET_BANDWIDTH:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_bandwidth(priv, tmpbuf);
		ret = 0;
		break;

	case MP_SET_TXPOWER:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_tx_power(priv, tmpbuf);
		ret = 0;
		break;

	case MP_CONTIOUS_TX:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);

		mp_ctx(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		ret = 0;
		break;

	case MP_ARX:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		i = mp_arx(priv, tmpbuf);
		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
		}
		wrq->u.data.length = i;
		ret = 0;
		break;


	case MP_SET_BSSID:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_bssid(priv, tmpbuf);
		ret = 0;
		break;

	case MP_ANTENNA_TX:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_ant_tx(priv, tmpbuf);
		ret = 0;
		break;

	case MP_ANTENNA_RX:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_ant_rx(priv, tmpbuf);
		ret = 0;
		break;

	case MP_SET_PHYPARA:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		mp_set_phypara(priv, tmpbuf);
		ret = 0;
		break;

#ifdef B2B_TEST
	case MP_TX_PACKET:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);

		i = mp_tx(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
		}
		wrq->u.data.length = i;
		ret = 0;
		break;

	case MP_BRX_PACKET:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);

		i = mp_brx(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
		}
		wrq->u.data.length = i;
		ret = 0;
		break;
#endif // B2B_TEST

	case MP_QUERY_STATS:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		i = mp_query_stats(priv, tmpbuf);
		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
		}
		wrq->u.data.length = i;
		ret = 0;
		break;

	case MP_TXPWR_TRACK:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);

		mp_txpower_tracking(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		ret = 0;
		break;

	case MP_QUERY_TSSI:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);

		i = mp_query_tssi(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
			wrq->u.data.length = i;
			ret = 0;
		}
		break;

#ifdef RTL8192SE
	case MP_QUERY_THER:
		if(copy_from_user(tmpbuf, (void *)wrq->u.data.pointer, wrq->u.data.length))
			break;
		RESTORE_INT(flags);
		i = mp_query_ther(priv, tmpbuf);
		SAVE_INT_AND_CLI(flags);

		if (i > 0) {
			if (copy_to_user((void *)wrq->u.data.pointer, tmpbuf, i))
				break;
			wrq->u.data.length = i;
			ret = 0;
		}
		break;
#endif
#endif	// MP_TEST
*/

#ifdef DEBUG_8190
	case SIOCGIWRTLREGDUMP:
		reg_dump(priv);
		ret = 0;
		break;
#endif


	}

	RESTORE_INT(flags);

	return ret;
}

//void MDL_DEVINIT set_mib_default_tbl(struct mac80211_shared_priv *priv)
void set_mib_default_tbl(struct mac80211_shared_priv *priv)
{
	int i;
	int arg_num = sizeof(mib_table)/sizeof(struct iwpriv_arg);

	for (i=0; i<arg_num; i++) {
		if (mib_table[i].Default) {
			if (mib_table[i].type == BYTE_T)
				*(((unsigned char *)priv->mib)+mib_table[i].offset) = (unsigned char)mib_table[i].Default;
			else if (mib_table[i].type == INT_T)
				memcpy(((unsigned char *)priv->mib)+mib_table[i].offset, (unsigned char *)&mib_table[i].Default, sizeof(int));
			else if (mib_table[i].type == RFFT_T && mib_table[i].len == 1)
				*(((unsigned char *)&(priv->pshare_hw->rf_ft_var))+mib_table[i].offset) = (unsigned char)mib_table[i].Default;
			else if (mib_table[i].type == RFFT_T && mib_table[i].len == 4)
				memcpy(((unsigned char *)&(priv->pshare_hw->rf_ft_var))+mib_table[i].offset, (unsigned char *)&mib_table[i].Default, sizeof(int));
			else {
				// We only give default value of types of BYTE_T and INT_T here.
				// Some others are gave in set_mib_default().
			}
		}
        }
}

void delay_us(unsigned int t)
{
//	__udelay(t, __udelay_val);
}


void delay_ms(unsigned int t)
{
	mdelay(t);
}

