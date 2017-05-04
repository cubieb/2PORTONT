#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/ethtool.h>
#include <sys/sysinfo.h>
#include "prmt_wandevice.h"
#include "prmt_wandsldiagnostics.h"
#include "prmt_wancondevice.h"
#include <sys/ioctl.h>
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom_wanext.h"
#endif

#ifdef CONFIG_DEV_xDSL
#include "cwmpc_utility.h"


struct CWMP_OP tStatsTotalLeafOP = { getStatsTotal, NULL };
struct CWMP_PRMT tStatsTotalLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ReceiveBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"TransmitBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"CellDelin",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"LinkRetrain",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"InitErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"InitTimeouts",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"LossOfFraming",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ErroredSecs",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"SeverelyErroredSecs",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"FECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCFECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"HECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCHECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"CRCErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP}
};
enum eStatsTotalLeaf
{
	eTT_ReceiveBlocks,
	eTT_TransmitBlocks,
	eTT_CellDelin,
	eTT_LinkRetrain,
	eTT_InitErrors,
	eTT_InitTimeouts,
	eTT_LossOfFraming,
	eTT_ErroredSecs,
	eTT_SeverelyErroredSecs,
	eTT_FECErrors,
	eTT_ATUCFECErrors,
	eTT_HECErrors,
	eTT_ATUCHECErrors,
	eTT_CRCErrors,
	eTT_ATUCCRCErrors
};
struct CWMP_LEAF tStatsTotalLeaf[] =
{
{ &tStatsTotalLeafInfo[eTT_ReceiveBlocks] },
{ &tStatsTotalLeafInfo[eTT_TransmitBlocks] },
{ &tStatsTotalLeafInfo[eTT_CellDelin] },
{ &tStatsTotalLeafInfo[eTT_LinkRetrain] },
{ &tStatsTotalLeafInfo[eTT_InitErrors] },
{ &tStatsTotalLeafInfo[eTT_InitTimeouts] },
{ &tStatsTotalLeafInfo[eTT_LossOfFraming] },
{ &tStatsTotalLeafInfo[eTT_ErroredSecs] },
{ &tStatsTotalLeafInfo[eTT_SeverelyErroredSecs] },
{ &tStatsTotalLeafInfo[eTT_FECErrors] },
{ &tStatsTotalLeafInfo[eTT_ATUCFECErrors] },
{ &tStatsTotalLeafInfo[eTT_HECErrors] },
{ &tStatsTotalLeafInfo[eTT_ATUCHECErrors] },
{ &tStatsTotalLeafInfo[eTT_CRCErrors] },
{ &tStatsTotalLeafInfo[eTT_ATUCCRCErrors] },
{ NULL }
};



struct CWMP_OP tStatsShowtimeLeafOP = { getStatsShowtime,NULL };
struct CWMP_PRMT tStatsShowtimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ReceiveBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"TransmitBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"CellDelin",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"LinkRetrain",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"InitErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"InitTimeouts",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"LossOfFraming",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ErroredSecs",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"SeverelyErroredSecs",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"FECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCFECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"HECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCHECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"CRCErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP}
};
enum eStatsShowtimeLeaf
{
	eST_ReceiveBlocks,
	eST_TransmitBlocks,
	eST_CellDelin,
	eST_LinkRetrain,
	eST_InitErrors,
	eST_InitTimeouts,
	eST_LossOfFraming,
	eST_ErroredSecs,
	eST_SeverelyErroredSecs,
	eST_FECErrors,
	eST_ATUCFECErrors,
	eST_HECErrors,
	eST_ATUCHECErrors,
	eST_CRCErrors,
	eST_ATUCCRCErrors
};
struct CWMP_LEAF tStatsShowtimeLeaf[] =
{
{ &tStatsShowtimeLeafInfo[eST_ReceiveBlocks] },
{ &tStatsShowtimeLeafInfo[eST_TransmitBlocks] },
{ &tStatsShowtimeLeafInfo[eST_CellDelin] },
{ &tStatsShowtimeLeafInfo[eST_LinkRetrain] },
{ &tStatsShowtimeLeafInfo[eST_InitErrors] },
{ &tStatsShowtimeLeafInfo[eST_InitTimeouts] },
{ &tStatsShowtimeLeafInfo[eST_LossOfFraming] },
{ &tStatsShowtimeLeafInfo[eST_ErroredSecs] },
{ &tStatsShowtimeLeafInfo[eST_SeverelyErroredSecs] },
{ &tStatsShowtimeLeafInfo[eST_FECErrors] },
{ &tStatsShowtimeLeafInfo[eST_ATUCFECErrors] },
{ &tStatsShowtimeLeafInfo[eST_HECErrors] },
{ &tStatsShowtimeLeafInfo[eST_ATUCHECErrors] },
{ &tStatsShowtimeLeafInfo[eST_CRCErrors] },
{ &tStatsShowtimeLeafInfo[eST_ATUCCRCErrors] },
{ NULL }
};


struct CWMP_PRMT tWANDSLIFSTATSObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Total",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"Showtime",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eWANDSLIFSTATSObject
{
	eDSLIFTotal,
	eDSLIFShowtime
};
struct CWMP_NODE tWANDSLIFSTATSObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLIFSTATSObjectInfo[eDSLIFTotal],	tStatsTotalLeaf,	NULL},
{&tWANDSLIFSTATSObjectInfo[eDSLIFShowtime],	tStatsShowtimeLeaf,	NULL},
{NULL,						NULL,			NULL}
};


struct CWMP_OP tWANDSLIFCFGLeafOP = { getWANDSLIfCfg,	setWANDSLIfCfg };
struct CWMP_PRMT tWANDSLIFCFGLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANDSLIFCFGLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"LinkEncapsulationSupported",	eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"LinkEncapsulationRequested",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANDSLIFCFGLeafOP},
{"LinkEncapsulationUsed",	eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{"ModulationType",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"StandardsSupported",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef CONFIG_E8B
{"StandardUsed",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tWANDSLIFCFGLeafOP},
#else
{"StandardUsed",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#endif
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{"LineEncoding",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef CONFIG_VDSL
{"AllowedProfiles",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"CurrentProfile",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"UPBOKLE",				eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"TRELLISds",			eCWMP_tINT,		CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"TRELLISus",			eCWMP_tINT,		CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ACTSNRMODEds",		eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ACTSNRMODEus",		eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ACTUALCE",			eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"SNRMpbus",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"SNRMpbds",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#endif /*CONFIG_VDSL*/
{"DataPath",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"InterleaveDepth",		eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*LineNumber*/
{"UpstreamCurrRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamCurrRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamMaxRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamMaxRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamNoiseMargin",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamNoiseMargin",	eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamAttenuation",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamAttenuation",	eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamPower",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamPower",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"ATURVendor",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ATURCountry",			eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*ATURANSIStd*/
/*ATURANSIRev*/
{"ATUCVendor",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ATUCCountry",			eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*ATUCANSIStd*/
/*ATUCANSIRev*/
{"TotalStart",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"ShowtimeStart",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP}
/*LastShowtimeStart*/
/*CurrentDayStart*/
/*QuarterHourStart*/
};
enum eWANDSLIFCFGLeaf
{
	eDSL_Enable,
	eDSL_Status,
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSL_LinkEncapsulationSupported,
	eDSL_LinkEncapsulationRequested,
	eDSL_LinkEncapsulationUsed,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	eDSL_ModulationType,
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSL_StandardsSupported,
	eDSL_StandardUsed,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	eDSL_LineEncoding,
#ifdef CONFIG_VDSL
	eDSL_AllowedProfiles,
	eDSL_CurrentProfile,
	eDSL_UPBOKLE,
	eDSL_TRELLISds,
	eDSL_TRELLISus,
	eDSL_ACTSNRMODEds,
	eDSL_ACTSNRMODEus,
	eDSL_ACTUALCE,
	eDSL_SNRMpbus,
	eDSL_SNRMpbds,
#endif /*CONFIG_VDSL*/
	eDSL_DataPath,
	eDSL_InterleaveDepth,
	/*LineNumber*/
	eDSL_UpstreamCurrRate,
	eDSL_DownstreamCurrRate,
	eDSL_UpstreamMaxRate,
	eDSL_DownstreamMaxRate,
	eDSL_UpstreamNoiseMargin,
	eDSL_DownstreamNoiseMargin,
	eDSL_UpstreamAttenuation,
	eDSL_DownstreamAttenuation,
	eDSL_UpstreamPower,
	eDSL_DownstreamPower,
	eDSL_ATURVendor,
	eDSL_ATURCountry,
	/*ATURANSIStd*/
	/*ATURANSIRev*/
	eDSL_ATUCVendor,
	eDSL_ATUCCountry,
	/*ATUCANSIStd*/
	/*ATUCANSIRev*/
	eDSL_TotalStart,
	eDSL_ShowtimeStart
	/*LastShowtimeStart*/
	/*CurrentDayStart*/
	/*QuarterHourStart*/
};
struct  CWMP_LEAF tWANDSLIFCFGLeaf[] =
{
{ &tWANDSLIFCFGLeafInfo[eDSL_Enable] },
{ &tWANDSLIFCFGLeafInfo[eDSL_Status] },
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationSupported] },
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationRequested] },
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationUsed] },
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_ModulationType] },
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_StandardsSupported] },
{ &tWANDSLIFCFGLeafInfo[eDSL_StandardUsed] },
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_LineEncoding] },
#ifdef CONFIG_VDSL
{ &tWANDSLIFCFGLeafInfo[eDSL_AllowedProfiles] },
{ &tWANDSLIFCFGLeafInfo[eDSL_CurrentProfile] },
{ &tWANDSLIFCFGLeafInfo[eDSL_UPBOKLE] },
{ &tWANDSLIFCFGLeafInfo[eDSL_TRELLISds] },
{ &tWANDSLIFCFGLeafInfo[eDSL_TRELLISus] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ACTSNRMODEds] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ACTSNRMODEus] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ACTUALCE] },
{ &tWANDSLIFCFGLeafInfo[eDSL_SNRMpbus] },
{ &tWANDSLIFCFGLeafInfo[eDSL_SNRMpbds] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFCFGLeafInfo[eDSL_DataPath] },
{ &tWANDSLIFCFGLeafInfo[eDSL_InterleaveDepth] },
/*LineNumber*/
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamCurrRate] },
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamCurrRate] },
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamMaxRate] },
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamMaxRate] },
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamNoiseMargin] },
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamNoiseMargin] },
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamAttenuation] },
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamAttenuation] },
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamPower] },
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamPower] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ATURVendor] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ATURCountry] },
/*ATURANSIStd*/
/*ATURANSIRev*/
{ &tWANDSLIFCFGLeafInfo[eDSL_ATUCVendor] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ATUCCountry] },
/*ATUCANSIStd*/
/*ATUCANSIRev*/
{ &tWANDSLIFCFGLeafInfo[eDSL_TotalStart] },
{ &tWANDSLIFCFGLeafInfo[eDSL_ShowtimeStart] },
/*LastShowtimeStart*/
/*CurrentDayStart*/
/*QuarterHourStart*/
{ NULL }
};


#ifdef _SUPPORT_ADSL2WAN_PROFILE_
struct CWMP_OP tWANDSLIFTestParamsLeafOP = { getWANDSLIFTestParams,	NULL };
struct CWMP_PRMT tWANDSLIFTestParamsLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
#ifdef CONFIG_VDSL
{"HLOGGds",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"HLOGGus",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
#endif /*CONFIG_VDSL*/
{"HLOGpsds",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"HLOGpsus",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"HLOGMTds",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"HLOGMTus",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
#ifdef CONFIG_VDSL
{"QLNGds",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"QLNGus",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
#endif /*CONFIG_VDSL*/
{"QLNpsds",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"QLNpsus",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"QLNMTds",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"QLNMTus",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
#ifdef CONFIG_VDSL
{"SNRGds",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SNRGus",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
#endif /*CONFIG_VDSL*/
{"SNRpsds",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SNRpsus",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SNRMTds",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SNRMTus",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"LATNds",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"LATNus",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SATNds",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
{"SATNus",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLIFTestParamsLeafOP},
};

struct  CWMP_LEAF tWANDSLIFTestParamsLeaf[] =
{
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTus] },
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTus] },
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNus] },
{ NULL }
};

int getWANDSLIFTestParams(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	char		*pChar=NULL;
	//int		vINT=0;
	unsigned int	vUINT=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "HLOGpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "HLOGpsus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "HLOGMTds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "HLOGMTus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "QLNpsus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "QLNMTds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNMTus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRpsus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRMTds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRMTus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "LATNds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_LATNds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "LATNus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_LATNus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SATNds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SATNds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SATNus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SATNus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );

#ifdef CONFIG_VDSL
	}else if( strcmp( lastname, "HLOGGds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGGds, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "HLOGGus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGGus, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNGds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNGds, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNGus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNGus, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRGds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRGds, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRGus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRGus, &vUINT )<0 )
			*data = uintdup( 1 );
		else
			*data = uintdup( vUINT );
#endif /*CONFIG_VDSL*/

	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_SUPPORT_ADSL2WAN_PROFILE_

struct CWMP_PRMT tWANDSLIFCFGObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"TestParams",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
};
enum eWANDSLIFCFGObject
{
	eDSLIFCFG_Stats,
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSLIFCFG_TestParams,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
};
struct CWMP_NODE tWANDSLIFCFGObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLIFCFGObjectInfo[eDSLIFCFG_Stats], NULL, tWANDSLIFSTATSObject},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{&tWANDSLIFCFGObjectInfo[eDSLIFCFG_TestParams],	tWANDSLIFTestParamsLeaf,NULL},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{NULL,						NULL,			NULL}
};
#endif  //#ifdef CONFIG_DEV_xDSL

#ifdef CONFIG_ETHWAN
struct CWMP_OP tWANEthInfCfgLeafOP = {getWANEthInfCfg, setWANEthInfCfg};
struct CWMP_PRMT tWANEthInfCfgLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Enable", eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP},
	{"Status", eCWMP_tSTRING,	CWMP_READ,		&tWANEthInfCfgLeafOP},
	{"MACAddress", eCWMP_tSTRING,	CWMP_READ,		&tWANEthInfCfgLeafOP},
	{"MaxBitRate", eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP},
	{"DuplexMode", eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP}
};

enum eWANEthInfCfgLeaf
{
	eETHWAN_Enable,
	eETHWAN_Status,
	eETHWAN_MACAddress,
	eETHWAN_MaxBitRate,
	eETHWAN_DuplexMode
};

struct  CWMP_LEAF tWANEthInfCfgLeaf[] =
{
	{ &tWANEthInfCfgLeafInfo[eETHWAN_Enable] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_Status] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_MACAddress] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_MaxBitRate] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_DuplexMode] },
	{ NULL }
};

struct CWMP_PRMT tWANEthInfCfgObjectInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Stats", eCWMP_tOBJECT, CWMP_READ, NULL},
};

enum eWANEthInfCfgObject
{
	eWANEthInfCfg_Stats,
};

struct CWMP_OP tWANEthInfCfgStatsLeafOP = {getWANEthStatsLeaf, NULL};
struct CWMP_PRMT tWANEthInfCfgStatsLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"BytesSent", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"BytesReceived", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"PacketsSent", eCWMP_tUINT,	CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"PacketsReceived", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP}
};

enum eWANEthInfCfgStatsLeaf
{
	eETHWANStats_BytesSent,
	eETHWANStats_BytesReceived,
	eETHWANStats_PacketsSent,
	eETHWANStats_PacketsReceived
};

struct  CWMP_LEAF tWANEthInfCfgStatsLeaf[] =
{
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_BytesSent] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_BytesReceived] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_PacketsSent] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_PacketsReceived] },
	{ NULL }
};

struct CWMP_NODE tWANEthInfCfgObject[] =
{
 /*info,  					leaf,			node)*/
 {&tWANEthInfCfgObjectInfo[eWANEthInfCfg_Stats], tWANEthInfCfgStatsLeaf, NULL},
 {NULL, NULL, NULL}
};
#endif // #ifdef CONFIG_ETHWAN

struct CWMP_OP tWANCmnIfCfgLeafOP = { getWANCmnIfCfg, setWANCmnIfCfg };
struct CWMP_PRMT tWANCmnIfCfgLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"EnabledForInternet",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANCmnIfCfgLeafOP},
{"WANAccessType",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"Layer1UpstreamMaxBitRate",	eCWMP_tUINT,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"Layer1DownstreamMaxBitRate",	eCWMP_tUINT,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"PhysicalLinkStatus",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"WANAccessProvider",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"TotalBytesSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalBytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP}
/*MaximumActiveConnections*/
/*NumberOfActiveConnections*/
};
enum eWANCmnIfCfgLeaf
{
	eEnabledForInternet,
	eWANAccessType,
	eLayer1UpstreamMaxBitRate,
	eLayer1DownstreamMaxBitRate,
	ePhysicalLinkStatus,
	eWANAccessProvider,
	eTotalBytesSent,
	eTotalBytesReceived,
	eTotalPacketsSent,
	eTotalPacketsReceived
};
struct CWMP_LEAF tWANCmnIfCfgLeaf[] =
{
{ &tWANCmnIfCfgLeafInfo[eEnabledForInternet] },
{ &tWANCmnIfCfgLeafInfo[eWANAccessType] },
{ &tWANCmnIfCfgLeafInfo[eLayer1UpstreamMaxBitRate] },
{ &tWANCmnIfCfgLeafInfo[eLayer1DownstreamMaxBitRate] },
{ &tWANCmnIfCfgLeafInfo[ePhysicalLinkStatus] },
{ &tWANCmnIfCfgLeafInfo[eWANAccessProvider] },
{ &tWANCmnIfCfgLeafInfo[eTotalBytesSent] },
{ &tWANCmnIfCfgLeafInfo[eTotalBytesReceived] },
{ &tWANCmnIfCfgLeafInfo[eTotalPacketsSent] },
{ &tWANCmnIfCfgLeafInfo[eTotalPacketsReceived] },
{ NULL }
};

struct CWMP_OP tWANDevEntityLeafOP = { getWANDevEntity, NULL };
struct CWMP_PRMT tWANDevEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANDevEntityLeafOP},
};
enum eWANDevEntityLeaf
{
	eWANConnectionNumberOfEntries,
};
struct CWMP_LEAF tWANDevEntityLeaf[] =
{
{ &tWANDevEntityLeafInfo[eWANConnectionNumberOfEntries] },
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tCTWANDevEntityLeafInfo[eX_CTCOM_WANIndex] },
#endif
{ NULL }
};

struct CWMP_OP tWAN_WANConnectionDevice_OP = { NULL, objConDev };
struct CWMP_PRMT tWANDevEntityObjectInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"WANCommonInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef CONFIG_ETHWAN
	{"WANEthernetInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef CONFIG_DEV_xDSL
	{"WANDSLInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
	{"WANDSLConnectionManagement",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
	{"WANDSLDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
	{"WANConnectionDevice",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANConnectionDevice_OP},
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
	{"WANGponInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
	{"WANEponInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
};

enum eWANDevEntityObject
{
	eWANCommonInterfaceConfig,
#ifdef CONFIG_ETHWAN
	eWANEthernetInterfaceConfig,
#endif
#ifdef CONFIG_DEV_xDSL
	eWANDSLInterfaceConfig,
	eWANDSLConnectionManagement,
	eWANDSLDiagnostics,
#endif
	eWANConnectionDevice,
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
	eWANGpongInterfaceConfig,
#endif
#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
	eWANEpongInterfaceConfig,
#endif
};

#ifdef CONFIG_DEV_xDSL
struct CWMP_NODE tWANDevEntityObjectDSL[] =  // WANDevce for ADSL mode
{
	/*info,  						leaf,			node)*/
	{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
	{ &tWANDevEntityObjectInfo[eWANDSLInterfaceConfig],	tWANDSLIFCFGLeaf,	tWANDSLIFCFGObject},
	{ &tWANDevEntityObjectInfo[eWANDSLConnectionManagement],tWANDSLCNTMNGLeaf,	tWANDSLCNTMNGObject},
	{ &tWANDevEntityObjectInfo[eWANDSLDiagnostics],		tWANDSLDIAGLeaf, NULL},
	{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL, NULL},
	{ NULL, NULL, NULL }
};
#ifdef CONFIG_PTMWAN
struct CWMP_NODE tWANDevEntityObjectPTM[] =  // WANDevce for VDSL mode
{
	/*info,  						leaf,			node)*/
	{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
	{ &tWANDevEntityObjectInfo[eWANDSLInterfaceConfig],	tWANDSLIFCFGLeaf,	tWANDSLIFCFGObject},
	{ &tWANDevEntityObjectInfo[eWANDSLDiagnostics],		tWANDSLDIAGLeaf, NULL},
	{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL, NULL},
	{ NULL, NULL, NULL }
};
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/

#ifdef CONFIG_ETHWAN
struct CWMP_NODE tWANDevEntityObjectEth[] = // WANDevce for Ethernet WAN mode
{
	/*info,  						leaf,			node)*/
	{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
	{ &tWANDevEntityObjectInfo[eWANEthernetInterfaceConfig],	tWANEthInfCfgLeaf,	tWANEthInfCfgObject},
	{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL, NULL},
	{ NULL, NULL, NULL }
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
struct CWMP_NODE tWANDevEntityObjectGPON[] = // WANDevce for Ethernet WAN mode
{
	/*info,  						leaf,			node)*/
	{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
	{ &tWANDevEntityObjectInfo[eWANGpongInterfaceConfig],	tCT_XPONInterfaceConfLeaf,	tCT_XPONInterfaceConfObject},
	{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL, NULL},
	{ NULL, NULL, NULL }
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
struct CWMP_NODE tWANDevEntityObjectEPON[] = // WANDevce for Ethernet WAN mode
{
	/*info,  						leaf,			node)*/
	{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
	{ &tWANDevEntityObjectInfo[eWANEpongInterfaceConfig],	tCT_XPONInterfaceConfLeaf,	tCT_XPONInterfaceConfObject},
	{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL, NULL},
	{ NULL, NULL, NULL }
};
#endif

struct CWMP_PRMT tWANDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0", eCWMP_tOBJECT, CWMP_READ/*|CWMP_WRITE*/|CWMP_LNKLIST, NULL},
};
enum eWANDeviceObject
{
	eWANDevice0
};

#ifdef CONFIG_DEV_xDSL
struct CWMP_LINKNODE tWANDeviceObjectDSL[] =  // WANDevce for ADSL mode
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice0], tWANDevEntityLeaf,	tWANDevEntityObjectDSL, NULL, 0},
};
#ifdef CONFIG_PTMWAN
struct CWMP_LINKNODE tWANDeviceObjectPTM[] =  // WANDevce for VDSL mode
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice0], tWANDevEntityLeaf,	tWANDevEntityObjectPTM, NULL, 0},
};
#endif /*CONFIG_PTMWAN*/
#endif /*CONFIG_DEV_xDSL*/

#ifdef CONFIG_ETHWAN
struct CWMP_LINKNODE tWANDeviceObjectEth[] =  // WANDevce for ADSL mode
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice0], tWANDevEntityLeaf,	tWANDevEntityObjectEth, NULL, 0},
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_GPON_FEATURE)
struct CWMP_LINKNODE tWANDeviceObjectGPON[] =  // WANDevce for GPON
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice0], tWANDevEntityLeaf,	tWANDevEntityObjectGPON, NULL, 0},
};
#endif

#if defined(_PRMT_X_CT_COM_WANEXT_) && defined(CONFIG_EPON_FEATURE)
struct CWMP_LINKNODE tWANDeviceObjectEPON[] =  // WANDevce for EPON
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice0], tWANDevEntityLeaf,	tWANDevEntityObjectEPON, NULL, 0},
};
#endif


#ifdef CONFIG_DEV_xDSL
int getStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//unsigned int vUInt=0;
	Modem_DSLConfigStatus MDS;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ReceiveBlocks" )==0 )
	{
		*data = uintdup( MDS.ReceiveBlocks );
	}else if( strcmp( lastname, "TransmitBlocks" )==0 )
	{
		*data = uintdup( MDS.TransmitBlocks );
	}else if( strcmp( lastname, "CellDelin" )==0 )
	{
		*data = uintdup( MDS.CellDelin );
	}else if( strcmp( lastname, "LinkRetrain" )==0 )
	{
		*data = uintdup( MDS.LinkRetain );
	}else if( strcmp( lastname, "InitErrors" )==0 )
	{
		*data = uintdup( MDS.InitErrors );
	}else if( strcmp( lastname, "InitTimeouts" )==0 )
	{
		*data = uintdup( MDS.InitTimeouts );
	}else if( strcmp( lastname, "LossOfFraming" )==0 )
	{
		*data = uintdup( MDS.LOF );
	}else if( strcmp( lastname, "ErroredSecs" )==0 )
	{
		*data = uintdup( MDS.ES );
	}else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{
		*data = uintdup( MDS.SES );
	}else if( strcmp( lastname, "FECErrors" )==0 )
	{
		*data = uintdup( MDS.FEC );
	}else if( strcmp( lastname, "ATUCFECErrors" )==0 )
	{
		*data = uintdup( MDS.AtucFEC );
	}else if( strcmp( lastname, "HECErrors" )==0 )
	{
		*data = uintdup( MDS.HEC );
	}else if( strcmp( lastname, "ATUCHECErrors" )==0 )
	{
		*data = uintdup( MDS.AtucHEC );
	}else if( strcmp( lastname, "CRCErrors" )==0 )
	{
		*data = uintdup( MDS.CRC );
	}else if( strcmp( lastname, "ATUCCRCErrors" )==0 )
	{
		*data = uintdup( MDS.AtucCRC );
	}else{
		return ERR_9005;
	}

	return 0;
}


int getStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//char	buf[256]={0};
	//Modem_def_counter_set vCs;
	//unsigned int vUInt=0;
	Modem_DSLConfigStatus MDS;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_SHOWTIME, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ReceiveBlocks" )==0 )
	{
		*data = uintdup( MDS.ReceiveBlocks );
	}else if( strcmp( lastname, "TransmitBlocks" )==0 )
	{
		*data = uintdup( MDS.TransmitBlocks );
	}else if( strcmp( lastname, "CellDelin" )==0 )
	{
		*data = uintdup( MDS.CellDelin );
	}else if( strcmp( lastname, "LinkRetrain" )==0 )
	{
		*data = uintdup( MDS.LinkRetain );
	}else if( strcmp( lastname, "InitErrors" )==0 )
	{
		*data = uintdup( MDS.InitErrors );
	}else if( strcmp( lastname, "InitTimeouts" )==0 )
	{
		*data = uintdup( MDS.InitTimeouts );
	}else if( strcmp( lastname, "LossOfFraming" )==0 )
	{
		*data = uintdup( MDS.LOF );
	}else if( strcmp( lastname, "ErroredSecs" )==0 )
	{
		*data = uintdup( MDS.ES );
	}else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{
		*data = uintdup( MDS.SES );
	}else if( strcmp( lastname, "FECErrors" )==0 )
	{
		*data = uintdup( MDS.FEC );
	}else if( strcmp( lastname, "ATUCFECErrors" )==0 )
	{
		*data = uintdup( MDS.AtucFEC );
	}else if( strcmp( lastname, "HECErrors" )==0 )
	{
		*data = uintdup( MDS.HEC );
	}else if( strcmp( lastname, "ATUCHECErrors" )==0 )
	{
		*data = uintdup( MDS.AtucHEC );
	}else if( strcmp( lastname, "CRCErrors" )==0 )
	{
		*data = uintdup( MDS.CRC );
	}else if( strcmp( lastname, "ATUCCRCErrors" )==0 )
	{
		*data = uintdup( MDS.AtucCRC );
	}else{
		return ERR_9005;
	}

	return 0;
}

int getWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256]="";
	//unsigned int vUInt=0;
	double vd=0;
	Modem_Identification vMId;
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	Modem_ADSL2WANConfig vMA2WC;
#endif //_SUPPORT_ADSL2WAN_PROFILE_
#ifdef CONFIG_VDSL
		int msgval[4];
#endif /*CONFIG_VDSL*/

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = intdup( 1 );
	}else if( strcmp( lastname, "Status" )==0 )
	{
		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
			*data = strdup( "Initializing" );
		else if( strncmp( buf, "SHOWTIME", 8 )==0 )
			*data = strdup( "Up" );
		else if( (strncmp( buf, "ACTIVATING", 10 )==0) ||
		         (strncmp( buf, "IDLE", 4 )==0) )
			*data = strdup( "EstablishingLink" );
		else
			*data = strdup( "NoSignal" );//or Error, Disabled
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "LinkEncapsulationSupported" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationSupported=0x%x\n", vMA2WC.LinkEncapSupported );
			buf[0]=0;;
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcat( buf, "G.992.3_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcat( buf, "G.992.3_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcat( buf, "G.993.2_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcat( buf, "G.993.2_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_994_1))
				strcat( buf, "G.994.1," );
			if(buf[0]) buf[ strlen(buf)-1 ]=0;
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "LinkEncapsulationRequested" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationRequested=0x%x\n", vMA2WC.LinkEncapRequested );
			if(vMA2WC.LinkEncapRequested & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcpy( buf, "G.992.3_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcpy( buf, "G.992.3_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcpy( buf, "G.993.2_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcpy( buf, "G.993.2_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_994_1))
				strcpy( buf, "G.994.1" );
			else
				strcpy( buf, "" );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "LinkEncapsulationUsed" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationUsed=0x%x\n", vMA2WC.LinkEncapUsed );
			if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcpy( buf, "G.992.3_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcpy( buf, "G.992.3_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcpy( buf, "G.993.2_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcpy( buf, "G.993.2_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_994_1))
				strcpy( buf, "G.994.1" );
			else
				strcpy( buf, "" );
			*data = strdup( buf );
		}
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "ModulationType" )==0 )
	{
		getAdslInfo(ADSL_GET_MODE, buf, 256);
		if( strncmp( buf, "T1.413", 6 )==0 )
			*data = strdup( "ADSL_ANSI_T1.413" );
		else if( strncmp( buf, "G.dmt", 5 )==0 )
			*data = strdup( "ADSL_G.dmt" );
		else if( strncmp( buf, "G.Lite", 6 )==0 )
			*data = strdup( "ADSL_G.lite" );
		else if( strncmp( buf, "ADSL2", 5 )==0 )
			*data = strdup( "ADSL_G.dmt.bis" );
		else if( strncmp( buf, "ADSL2+", 6 )==0 )
			*data = strdup( "ADSL_2plus" );
#ifdef CONFIG_VDSL
		else if( strncmp( buf, "VDSL2", 5 )==0 )
			*data = strdup( "VDSL2" );//spec doesn't define VDSL2
#endif /*CONFIG_VDSL*/
		else
			*data = strdup( "" );
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "StandardsSupported" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardsSupported=0x%x\n", vMA2WC.StandardsSuported );
			strcpy( buf, "" );
			std_idx=0;
			while( strAdsl2WanStd[std_idx] != NULL )
			{
				if( vMA2WC.StandardsSuported & (1<<std_idx) )
				{
					if( (strlen(buf)+strlen(strAdsl2WanStd[std_idx])+1)>=sizeof(buf) )
					{
						fprintf( stderr, "(%s:%d)buf is too small!!\n", __FUNCTION__, __LINE__ );
						break;
					}
					if(buf[0]) strcat( buf, "," );
					strcat( buf, strAdsl2WanStd[std_idx] );
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "StandardUsed" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardUsed=0x%x\n", vMA2WC.StandardUsed );
			strcpy( buf, "" );
			std_idx=0;
			while( strAdsl2WanStd[std_idx] != NULL )
			{
				if( vMA2WC.StandardUsed & (1<<std_idx) )
				{
					strcpy( buf, strAdsl2WanStd[std_idx] );
					break;
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "LineEncoding" )==0 )
	{
		*data = strdup( "DMT" );
#ifdef CONFIG_VDSL
	}else if( strcmp( lastname, "AllowedProfiles" )==0 )
	{
		buf[0]=0;
		if(dsl_msg_get_array(GetHskXdslMode, msgval))
		{
			if( msgval[0]&MODE_VDSL2 )
			{
				int pval=msgval[1];
				if(pval&VDSL2_PROFILE_8A) strcat(buf, "8a,");
				if(pval&VDSL2_PROFILE_8B) strcat(buf, "8b,");
				if(pval&VDSL2_PROFILE_8C) strcat(buf, "8c,");
				if(pval&VDSL2_PROFILE_8D) strcat(buf, "8d,");
				if(pval&VDSL2_PROFILE_12A) strcat(buf, "12a,");
				if(pval&VDSL2_PROFILE_12B) strcat(buf, "12b,");
				if(pval&VDSL2_PROFILE_17A) strcat(buf, "17a,");
				if(pval&VDSL2_PROFILE_30A) strcat(buf, "30a,");
				if(buf[0]) buf[strlen(buf)-1]=0;
			}
		}
		*data = strdup( buf );
	}else if( strcmp( lastname, "CurrentProfile" )==0 )
	{
		int mval=0, pval=0;

		buf[0]=0;
		if( dsl_msg_get(GetPmdMode,&mval) &&
			dsl_msg_get(GetVdslProfile,&pval) )
		{
			if(mval&MODE_VDSL2)
			{
				if(pval&VDSL2_PROFILE_8A) strcpy(buf, "8a");
				else if(pval&VDSL2_PROFILE_8B) strcpy(buf, "8b");
				else if(pval&VDSL2_PROFILE_8C) strcpy(buf, "8c");
				else if(pval&VDSL2_PROFILE_8D) strcpy(buf, "8d");
				else if(pval&VDSL2_PROFILE_12A) strcpy(buf, "12a");
				else if(pval&VDSL2_PROFILE_12B) strcpy(buf, "12b");
				else if(pval&VDSL2_PROFILE_17A) strcpy(buf, "17a");
				else if(pval&VDSL2_PROFILE_30A) strcpy(buf, "30a");
			}
		}
		*data = strdup( buf );
	}else if( strcmp( lastname, "UPBOKLE" )==0 )
	{
		if(dsl_msg_get_array(GetUPBOKLE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "TRELLISds" )==0 )
	{
		if(dsl_msg_get_array(GetTrellis, msgval))
		{
			if(msgval[0]) *data = intdup( 1 );
			else *data = intdup( 0 );
		}else
			*data = intdup( -1 );
	}else if( strcmp( lastname, "TRELLISus" )==0 )
	{
		if(dsl_msg_get_array(GetTrellis, msgval))
		{
			if(msgval[1]) *data = intdup( 1 );
			else *data = intdup( 0 );
		}else
			*data = intdup( -1 );
	}else if( strcmp( lastname, "ACTSNRMODEds" )==0 )
	{
		if(dsl_msg_get_array(GetACTSNRMODE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "ACTSNRMODEus" )==0 )
	{
		if(dsl_msg_get_array(GetACTSNRMODE, msgval))
		{
			*data = uintdup( msgval[1] );
		}else
			*data = uintdup( 0 );
	}else if( strcmp( lastname, "ACTUALCE" )==0 )
	{
		if(dsl_msg_get_array(GetACTUALCE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 99 );
	}else if( strcmp( lastname, "SNRMpbus" )==0 )
	{
		char *pChar;
		if( getDSLParameterValue( GET_DSL_SNRMpbus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRMpbds" )==0 )
	{
		char *pChar;
		if( getDSLParameterValue( GET_DSL_SNRMpbds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
#endif /*CONFIG_VDSL*/
	}else if( strcmp( lastname, "DataPath" )==0 )
	{
		getAdslInfo(ADSL_GET_LATENCY, buf, 256);
		if( strncmp( buf, "Fast", 4 )==0 )
			*data = strdup( "Fast" );
		else if( strncmp( buf, "Interleave", 10 )==0 )
			*data = strdup( "Interleaved" );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "InterleaveDepth" )==0 )
	{
		getAdslInfo(ADSL_GET_D_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamCurrRate" )==0 )
	{
		getAdslInfo(ADSL_GET_RATE_US, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "DownstreamCurrRate" )==0 )
	{
		getAdslInfo(ADSL_GET_RATE_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamMaxRate" )==0 )
	{
		getAdslInfo(ADSL_GET_ATTRATE_US, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "DownstreamMaxRate" )==0 )
	{
		getAdslInfo(ADSL_GET_ATTRATE_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamNoiseMargin" )==0 )
	{
		getAdslInfo(ADSL_GET_SNR_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamNoiseMargin" )==0 )
	{
		getAdslInfo(ADSL_GET_SNR_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "UpstreamAttenuation" )==0 )
	{
		getAdslInfo(ADSL_GET_LPATT_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamAttenuation" )==0 )
	{
		getAdslInfo(ADSL_GET_LPATT_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "UpstreamPower" )==0 )
	{
		getAdslInfo(ADSL_GET_POWER_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamPower" )==0 )
	{
		getAdslInfo(ADSL_GET_POWER_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "ATURVendor" )==0 )
	{
		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = strdup("");
		else
		{
			snprintf( buf, 256, "%c%c%c%c",
					(int)(vMId.ITU_VendorId.vendorCode>>24) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode>>16) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode>>8) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode) &0xff
					 );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "ATURCountry" )==0 )
	{
		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vMId.ITU_VendorId.countryCode );
	}else if( strcmp( lastname, "ATUCVendor" )==0 )
	{
		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = strdup("");
		else
		{
			snprintf( buf, 256, "%c%c%c%c",
					(int)(vMId.ITU_VendorId.vendorCode>>24) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode>>16) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode>>8) &0xff,
					(int)(vMId.ITU_VendorId.vendorCode) &0xff
					 );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "ATUCCountry" )==0 )
	{
		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vMId.ITU_VendorId.countryCode );
	}else if( strcmp( lastname, "TotalStart" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}else if( strcmp( lastname, "ShowtimeStart" )==0 )
	{
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[0] );
	}else{
		return ERR_9005;
	}

	return 0;
}


int setWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if( *i!= 1 ) return ERR_9001;
	}
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	else if( strcmp( lastname, "LinkEncapsulationRequested" )==0 )
	{
		char *buf=data;
		if( buf==NULL || strlen(buf)==0 ) return ERR_9007;
		//fprintf( stderr, "set %s=%s\n", name, buf );
		if( strcmp( buf, "G.992.3_Annex_K_ATM" )!=0 ) return ERR_9001;
		return 0;
	}
#ifdef CONFIG_E8B
	else if( strcmp( lastname, "StandardUsed" )==0 )
	{
		/*e8 use this parameter to enable/disable ADSL2+ Annex M*/

		char *buf=data;
		unsigned short mode;

		if( buf==NULL || strlen(buf)==0 ) return ERR_9007;
		//fprintf( stderr, "set %s=%s\n", name, buf );

		mib_get(MIB_ADSL_MODE, &mode);
		if( strcmp( buf, "G.992.5_Annex_M" ) == 0 )
			mode |= ADSL_MODE_ANXM;
		else if( strcmp(buf, "G.992.5_Annex_A") == 0)
			mode &= ~ADSL_MODE_ANXM;
		else
			return ERR_9001;

		mib_set(MIB_ADSL_MODE, &mode);
		gWanBitMap = UINT_MAX;

		return CWMP_NEED_RESTART_WAN;
	}
#endif
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	else{
		return ERR_9005;
	}

	return 0;
}
#endif // #ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_ETHWAN
int getWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//unsigned int bs=0,br=0,ps=0,pr=0;
	char	buf[256]="";
	struct net_link_info netlink_info;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "Enable")==0 )
	{
		int flags;
		getInFlags(ALIASNAME_NAS0, &flags);
		*data = booldup(flags & IFF_UP);
	}
	else if( strcmp(lastname, "Status")==0 )
	{
		int link_status = get_net_link_status(ALIASNAME_NAS0);

		switch(link_status)
		{
			case -1:
				strcpy(buf, "Error");
				break;
			case 0:
				strcpy(buf, "NoLink");
				break;
			case 1:
				strcpy(buf, "Up");
				break;
			default:
				return ERR_9002;
		}

		*data = strdup(buf);
	}
	else if( strcmp(lastname, "MACAddress")==0 )
	{
		//struct in_addr inAddr;
		char macadd[MAC_ADDR_LEN] = {0};
		struct sockaddr sa;

		if(!getInAddr(ALIASNAME_NAS0, HW_ADDR, (void *)&sa))
		{
			*data = strdup("");
		}else{
			memcpy( macadd, sa.sa_data, MAC_ADDR_LEN );
			sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0]&0xff, macadd[1]&0xff, macadd[2]&0xff, macadd[3]&0xff, macadd[4]&0xff, macadd[5]&0xff);
			*data = strdup(buf);
		}
	}
	else if( strcmp(lastname, "MaxBitRate")==0 )
	{
		if(get_net_link_info(ALIASNAME_NAS0, &netlink_info))
			return ERR_9002;

		sprintf(buf, "%d", netlink_info.speed);

		*data = strdup(buf);
	}
	else if( strcmp(lastname, "DuplexMode")==0 )
	{
		if(get_net_link_info(ALIASNAME_NAS0, &netlink_info))
			return ERR_9002;

		if(netlink_info.duplex == 0)
			*data = strdup("Half");
		else if(netlink_info.duplex == 1)
			*data = strdup("Full");
		else
			return ERR_9002;
	}
	else
		return ERR_9005;

	return 0;
}

int setWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		char buf[256];
		//int entrynum;
		//MIB_CE_ATM_VC_T *pEntry, tmpentry;
		//pEntry = &tmpentry;

		int *i = data;

		if( i == NULL ) return ERR_9007;

		if( *i == 1 )
		{
			sprintf(buf, "ifconfig %s up", ALIASNAME_NAS0 );
			system(buf);
		}else if( *i == 0 )
		{
			sprintf(buf, "ifconfig %s down", ALIASNAME_NAS0 );
			system(buf);
		}else
			return ERR_9003;
	}
	else if( strcmp(lastname, "MaxBitRate")==0 )
	{
		return ERR_9001;
	}
	else if( strcmp(lastname, "DuplexMode")==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	//return ERR_9001;

	return 0;
}

int getWANEthStatsLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	//unsigned int bs=0,br=0,ps=0,pr=0;
	struct net_device_stats nds;
	struct ethtool_stats *stats = NULL;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(get_net_device_stats(ALIASNAME_NAS0, &nds) < 0)
		return ERR_9002;

	stats = ethtool_gstats(ALIASNAME_NAS0);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "BytesSent")==0 )
	{
		*data = uintdup(stats ? stats->data[TX_OCTETS] : nds.tx_bytes);
	}
	else if( strcmp(lastname, "BytesReceived")==0 )
	{
		*data = uintdup(stats ? stats->data[RX_OCTETS] : nds.rx_bytes);
	}
	else if( strcmp(lastname, "PacketsSent")==0 )
	{
		*data = uintdup(stats ? stats->data[TX_UCAST_PACKETS]
				+ stats->data[TX_MCAST_PACKETS]
				+ stats->data[TX_BCAST_PACKETS]
				: nds.tx_packets);
	}
	else if( strcmp(lastname, "PacketsReceived")==0 )
	{
		*data = uintdup(stats ? stats->data[RX_UCAST_PACKETS]
				+ stats->data[RX_MCAST_PACKETS]
				+ stats->data[RX_BCAST_PACKETS]
				: nds.rx_packets);
	}
	else
	{
		free(stats);
		return ERR_9005;
	}
	free(stats);

	return 0;
}
#endif // #ifdef CONFIG_ETHWAN

int getWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256]="";
	unsigned long bs=0,br=0,ps=0,pr=0;
	struct net_device_stats nds;
	struct ethtool_stats *stats = NULL;
	struct net_link_info netlink_info;
	unsigned int wandevnum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wandevnum = getWANDevInstNum(name);

#ifdef CONFIG_DEV_xDSL
	if(wandevnum==WANDEVNUM_ATM)
	{
		memset( &nds, 0, sizeof(nds) );
		if(get_DSLWANStat( &nds ) != 0)
		{
			fprintf(stderr, "<%s:%d> Get DSL statistics error!\n");
		}
	}else
#endif
	if(wandevnum==WANDEVNUM_ETH
#ifdef CONFIG_PTMWAN
		|| wandevnum==WANDEVNUM_PTM
#endif /*CONFIG_PTMWAN*/
		)
	{
		const char *ifname = ALIASNAME_NAS0;

#ifdef CONFIG_PTMWAN
		if (wandevnum==WANDEVNUM_PTM)
			ifname = ALIASNAME_PTM0;
#endif /*CONFIG_PTMWAN*/
		stats = ethtool_gstats(ifname);

		if (stats) {
			nds.tx_bytes = stats->data[TX_OCTETS];
			nds.rx_bytes = stats->data[RX_OCTETS];
			nds.tx_packets = stats->data[TX_UCAST_PACKETS]
				+ stats->data[TX_MCAST_PACKETS]
				+ stats->data[TX_BCAST_PACKETS];
			nds.rx_packets = stats->data[RX_UCAST_PACKETS]
				+ stats->data[RX_MCAST_PACKETS]
				+ stats->data[RX_BCAST_PACKETS];

			free(stats);
		} else {
			if(get_net_device_stats(ifname, &nds) < 0)
				memset( &nds, 0, sizeof(nds) );
		}
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "EnabledForInternet" )==0 )
	{
		*data = booldup(1);
	}else if( strcmp( lastname, "WANAccessType" )==0 )
	{
		if( (wandevnum==WANDEVNUM_ATM)
			#ifdef CONFIG_PTMWAN
			||(wandevnum==WANDEVNUM_PTM)
			#endif /*CONFIG_PTMWAN*/
		  )
			*data = strdup("DSL");
		else
			*data = strdup("Ethernet");
	}else if( strcmp( lastname, "Layer1UpstreamMaxBitRate" )==0 )
	{
		if( (wandevnum==WANDEVNUM_ATM)
			#ifdef CONFIG_PTMWAN
			||(wandevnum==WANDEVNUM_PTM)
			#endif /*CONFIG_PTMWAN*/
		  )
		{
#ifdef CONFIG_DEV_xDSL
			unsigned int vUint[3];
			if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
				*data = uintdup( 0 );
			else
				*data = uintdup( vUint[2] );
#else
			*data = uintdup(0);
#endif
		}else{
			if(get_net_link_info(ALIASNAME_NAS0, &netlink_info)) {
				return ERR_9002;
			}

			*data = uintdup(netlink_info.speed*1000000);
		}
	}else if( strcmp( lastname, "Layer1DownstreamMaxBitRate" )==0 )
	{
		if( (wandevnum==WANDEVNUM_ATM)
			#ifdef CONFIG_PTMWAN
			||(wandevnum==WANDEVNUM_PTM)
			#endif /*CONFIG_PTMWAN*/
		  )
		{
#ifdef CONFIG_DEV_xDSL
			unsigned int vUint[3];
			if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
				*data = uintdup( 0 );
			else
				*data = uintdup( vUint[1] );
#else
			*data = uintdup(0);
#endif
		}else{
			if(get_net_link_info(ALIASNAME_NAS0, &netlink_info)) {
				return ERR_9002;
			}

			*data = uintdup(netlink_info.speed*1000000);
		}
	}else if( strcmp( lastname, "PhysicalLinkStatus" )==0 )
	{
		if( (wandevnum==WANDEVNUM_ATM)
			#ifdef CONFIG_PTMWAN
			||(wandevnum==WANDEVNUM_PTM)
			#endif /*CONFIG_PTMWAN*/
		  )
		{
#ifdef CONFIG_DEV_xDSL
			getAdslInfo(ADSL_GET_STATE, buf, 256);
			if( strncmp( buf, "HANDSHAKING", 11 )==0 )
				*data = strdup( "Initializing" );
			else if( strncmp( buf, "SHOWTIME", 8 )==0 )
				*data = strdup( "Up" );
			else if( (strncmp( buf, "ACTIVATING", 10 )==0) || (strncmp( buf, "IDLE", 4 )==0) )
				*data = strdup( "Down" );
			else
				*data = strdup( "Unavailable" );
#else
			*data = strdup("Unavailable");
#endif
		}else{
			*data = strdup("Up");
		}
	}else if( strcmp( lastname, "WANAccessProvider" )==0 )
	{
			*data = strdup("");
	}else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{
			*data = uintdup(nds.tx_bytes);
	}else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{
			*data = uintdup(nds.rx_bytes);
	}else if( strcmp( lastname, "TotalPacketsSent" )==0 )
	{
			*data = uintdup(nds.tx_packets);
	}else if( strcmp( lastname, "TotalPacketsReceived" )==0 )
	{
			*data = uintdup(nds.rx_packets);
	}else
		return ERR_9005;

	return 0;
}

int setWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "EnabledForInternet" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if( *i!= 1 ) return ERR_9001;
	}else{
		return ERR_9005;
	}

	return 0;
}

int getWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_ATM_VC_T vc_entity;
	int numofentries, i, totalwancon = 0;
	unsigned int wandevnum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	wandevnum = getWANDevInstNum(name);

	if( strcmp( lastname, "WANConnectionNumberOfEntries" )==0 )
	{
		numofentries = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < numofentries; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
			{
				CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
				return ERR_9002;
			}

			if(isWANDevModeMatch(wandevnum, vc_entity.ifIndex))
			{
				MIB_CE_ATM_VC_T pre_entity;
				int j,isadded;
				isadded=0;
				for(j=0;j<i;j++)
				{
					if(!mib_chain_get(MIB_ATM_VC_TBL, j, (void*)&pre_entity))
					{
						CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index j=%d)", j);
						return ERR_9002;
					}

					if(isWANDevModeMatch(wandevnum, pre_entity.ifIndex))
					{
						if(vc_entity.ConDevInstNum==pre_entity.ConDevInstNum)
						{
							isadded=1;
							break;
						}
					}
				}
				if(isadded==0) totalwancon++;
			}
		}

		*data = uintdup(totalwancon);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int objWANDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			update_WANDEVNUM();
#ifdef CONFIG_DEV_xDSL
			if(WAN_MODE & MODE_ATM) // Create WANDevice for ADSL
			{
				if( create_Object(c, tWANDeviceObjectDSL, sizeof(tWANDeviceObjectDSL), 1, WANDEVNUM_ATM) < 0 )
				{
					CWMPDBP(1, "Create WANDevice object for DSL failed!\n");
					return -1;
				}
			}

#ifdef CONFIG_PTMWAN
			if(WAN_MODE & MODE_PTM) // Create WANDevice for PTM
			{
				if( create_Object(c, tWANDeviceObjectPTM, sizeof(tWANDeviceObjectPTM), 1, WANDEVNUM_PTM) < 0 )
				{
					CWMPDBP(1, "Create WANDevice object for DSL-PTM failed!\n");
					return -1;
				}
			}
#endif /*CONFIG_PTMWAN*/
#endif  /*CONFIG_DEV_xDSL*/

#ifdef CONFIG_ETHWAN
#if defined(_PRMT_X_CT_COM_WANEXT_) && (defined(CONFIG_EPON_FEATURE) || defined(CONFIG_GPON_FEATURE))
			if(WAN_MODE & MODE_Ethernet) // Create WANDevice for PON
			{
				int mode;

				mib_get(MIB_PON_MODE, &mode);

				if(mode == ETH_MODE)
				{
					if( create_Object(c, tWANDeviceObjectEth, sizeof(tWANDeviceObjectEth), 1, WANDEVNUM_ETH) < 0 )
					{
						CWMPDBP(1, "Create WANDevice object for Ethernet WAN failed!\n");
						return -1;
					}
				}
			#ifdef CONFIG_GPON_FEATURE
				else if(mode == GPON_MODE)
				{
					if( create_Object(c, tWANDeviceObjectGPON, sizeof(tWANDeviceObjectGPON), 1, WANDEVNUM_ETH) < 0 )
					{
						CWMPDBP(1, "Create WANDevice object for GPON WAN failed!\n");
						return -1;
					}
				}
			#endif
			#ifdef CONFIG_EPON_FEATURE
				else if(mode == EPON_MODE)
				{
					if( create_Object(c, tWANDeviceObjectEPON, sizeof(tWANDeviceObjectEPON), 1, WANDEVNUM_ETH) < 0 )
					{
						CWMPDBP(1, "Create WANDevice object for EPON WAN failed!\n");
						return -1;
					}
				}
			#endif
				else
				{
					CWMPDBP(1, "Invalid PON_MODE value %d!\n", mode);
					return -1;
				}
			}
#else
			if(WAN_MODE & MODE_Ethernet) // Create WANDevice for Ethernet WAN
			{
				if( create_Object(c, tWANDeviceObjectEth, sizeof(tWANDeviceObjectEth), 1, WANDEVNUM_ETH) < 0 )
				{
					CWMPDBP(1, "Create WANDevice object for Ethernet WAN failed!\n");
					return -1;
				}
			}
#endif
#endif /*CONFIG_ETHWAN*/

			return 0;
		}
		case eCWMP_tADDOBJ:
			return ERR_9001;
		case eCWMP_tDELOBJ:
			return ERR_9001;
		case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *table = (struct CWMP_LINKNODE *)entity->next;
			int change;

			change = update_WANDEVNUM();
#ifdef CONFIG_DEV_xDSL
			if (WAN_MODE & MODE_ATM) // Update WANDevice for ADSL
			{
				table->instnum = WANDEVNUM_ATM;
				table = table->sibling;
			}
#ifdef CONFIG_PTMWAN
			if (WAN_MODE & MODE_PTM) // Update WANDevice for PTM
 			{
				table->instnum = WANDEVNUM_PTM;
				table = table->sibling;
 			}
#endif /*CONFIG_PTMWAN*/
#endif  /*CONFIG_DEV_xDSL*/

#ifdef CONFIG_ETHWAN
			if(WAN_MODE & MODE_Ethernet) // Create WANDevice for Ethernet WAN
			{
				table->instnum = WANDEVNUM_ETH;
			}
#endif /*CONFIG_ETHWAN*/

			if (change)
				notify_set_wan_changed();

 			return 0;
 		}
	}

	return -1;
}

