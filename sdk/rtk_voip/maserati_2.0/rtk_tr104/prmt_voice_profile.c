#include "prmt_limit.h"
#include "prmt_voice_profile.h"
#include "prmt_voice_profile_line.h"
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_utility.h"
#include "str_mib.h"

//******* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo. EntityLEAF*******
struct CWMP_OP tVPServiceProviderEntityLeafOP = { getVPServiceProviderEntity, setVPServiceProviderEntity};

struct CWMP_PRMT tVPServiceProviderEntityLeafInfo[] =
{
    /*(name,   type,           flag,                  op)*/
	{ "Name",  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPServiceProviderEntityLeafOP},
};

enum eVPServiceProviderEntityLeaf
{
    eVPServiceProviderName
};

struct CWMP_LEAF tVPServiceProviderEntityLeaf[] =
{
    { &tVPServiceProviderEntityLeafInfo[eVPServiceProviderName]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.SIP. EntityLEAF*******
struct CWMP_OP tVPSIPEntityLeafOP = { getVPSipEntity, setVPSipEntity};

struct CWMP_PRMT tVPSIPEntityLeafInfo[] =
{
    /*(name,                                       type,           flag,                  op)*/
	{ "ProxyServer",                               eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "ProxyServerPort",                           eCWMP_tUINT,   		CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "ProxyServerTransport",                      eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegistrarServer",                           eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegistrarServerPort",                       eCWMP_tUINT,    	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegistrarServerTransport",                  eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "UserAgentDomain",                           eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "UserAgentPort",                             eCWMP_tUINT,    		CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "UserAgentTransport",                        eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "OutboundProxy",                             eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "OutboundProxyPort",                         eCWMP_tUINT,    	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "Organization",                              eCWMP_tSTRING,  	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegistrationPeriod",                        eCWMP_tUINT,    	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "VLANIDMark",                                eCWMP_tINT,     		CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "DSCPMark",                                  eCWMP_tUINT,    		CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegisterRetryInterval",                     eCWMP_tUINT,    	CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "RegisterExpires",                           eCWMP_tUINT,    		CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-ProxyServer",              eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-ProxyServerPort",          eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-ProxyServerTransport",     eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-RegistrarServer",          eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-RegistrarServerPort",      eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-RegistrarServerTransport", eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-OutboundProxy",            eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_Standby-OutboundProxyPort",        eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_Default_Proxy_Index",				eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_802-1pMark",                       eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_HeartbeatSwitch",                  eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_HeartbeatCycle",                   eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_CT-COM_HeartbeatCount",                   eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "InviteExpires",               eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_Standby-InviteExpires",               eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_Standby-RegistrationPeriod", 		   eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_Standby-RegisterRetryInterval",                     eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	{ "X_Standby-RegisterExpires",                           eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},		
	{ "X_CT-COM_SessionUpdateTimer",               eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPSIPEntityLeafOP},
	
};

enum eVPSIPEntityLeaf
{
    eSIPProxyServer,
    eSIPProxyServerPort,
    eSIPProxyServerTransport,
    eSIPRegistrarServer,
    eSIPRegistrarServerPort,
    eSIPRegistrarServerTransport,
    eSIPUserAgentDomain,
    eSIPUserAgentPort,
    eSIPUserAgentTransport,
    eSIPOutboundProxy,
    eSIPOutboundProxyPort,
    eSIPOrganization,
    eSIPRegistrationPeriod,
	eSIPVLANIDMark,
	eSIPDSCPMark,
	eSIPRegisterRetryInterval,
	eSIPRegisterExpires,
	eSIPXCT_STB_ProxyServer,
	eSIPXCT_STB_ProxyServerPort,
	eSIPXCT_STB_ProxyServerTransport,
	eSIPXCT_STB_RegistrarServer,
	eSIPXCT_STB_RegistrarServerPort,
	eSIPXCT_STB_RegistrarServerTransport,
	eSIPXCT_STB_OutboundProxy,
	eSIPXCT_STB_OutboundProxyPort,
	eSIPXCT_STB_Default_Proxy_Index,
	eSIPXCT_STB_802_1pMark,
	eSIPXCT_HeartbeatSwitch,
	eSIPXCT_HeartbeatCycle,
	eSIPXCT_HeartbeatCount,
	eSIPInviteExpires,
	eSIPXCT_STB_InviteExpires,
	eSIPXCT_STB_RegistrationPeriod,
	eSIPXCT_STB_RegisterRetryInterval,
	eSIPXCT_STB_RegisterExpires,
	eSIPXCT_SessionUpdateTimer,
};

struct CWMP_LEAF tVPSIPEntityLeaf[] =
{
    { &tVPSIPEntityLeafInfo[eSIPProxyServer]  },
    { &tVPSIPEntityLeafInfo[eSIPProxyServerPort]  },
    { &tVPSIPEntityLeafInfo[eSIPProxyServerTransport]  },
    { &tVPSIPEntityLeafInfo[eSIPRegistrarServer]  },
    { &tVPSIPEntityLeafInfo[eSIPRegistrarServerPort]  },
    { &tVPSIPEntityLeafInfo[eSIPRegistrarServerTransport]  },
    { &tVPSIPEntityLeafInfo[eSIPUserAgentDomain]  },
    { &tVPSIPEntityLeafInfo[eSIPUserAgentPort]  },
    { &tVPSIPEntityLeafInfo[eSIPUserAgentTransport]  },
    { &tVPSIPEntityLeafInfo[eSIPOutboundProxy]  },
    { &tVPSIPEntityLeafInfo[eSIPOutboundProxyPort]  },
    { &tVPSIPEntityLeafInfo[eSIPOrganization]  },
    { &tVPSIPEntityLeafInfo[eSIPRegistrationPeriod]  },
    { &tVPSIPEntityLeafInfo[eSIPVLANIDMark]  },
    { &tVPSIPEntityLeafInfo[eSIPDSCPMark]  },
    { &tVPSIPEntityLeafInfo[eSIPRegisterRetryInterval]  },
    { &tVPSIPEntityLeafInfo[eSIPRegisterExpires]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_ProxyServer]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_ProxyServerPort]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_ProxyServerTransport]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegistrarServer]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegistrarServerPort]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegistrarServerTransport]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_OutboundProxy]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_OutboundProxyPort]  },   
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_Default_Proxy_Index]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_802_1pMark]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_HeartbeatSwitch]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_HeartbeatCycle]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_HeartbeatCount]  },
    { &tVPSIPEntityLeafInfo[eSIPInviteExpires]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_InviteExpires]  },    
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegistrationPeriod]  },     
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegisterRetryInterval]  },
    { &tVPSIPEntityLeafInfo[eSIPXCT_STB_RegisterExpires]  },    
    { &tVPSIPEntityLeafInfo[eSIPXCT_SessionUpdateTimer]  },    
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.RTP. EntityLEAF*******
struct CWMP_OP tVPRTPEntityLeafOP = { getVPRTPEntity, setVPRTPEntity};

struct CWMP_PRMT tVPRTPEntityLeafInfo[] =
{
    /*(name,                 type,           flag,                  op)*/
	{ "VLANIDMark",          eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPRTPEntityLeafOP},
	{ "DSCPMark",            eCWMP_tUINT,    CWMP_READ|CWMP_WRITE,  &tVPRTPEntityLeafOP},
	{ "X_CT-COM_802-1pMark", eCWMP_tINT,     CWMP_READ|CWMP_WRITE,  &tVPRTPEntityLeafOP}
};

enum eVPRTPEntityLeaf
{
	eRTPVLANIDMark,
	eRTPDSCPMark,
	eRTPXCT_STB_802_1pMark
};

struct CWMP_LEAF tVPRTPEntityLeaf[] =
{
    { &tVPRTPEntityLeafInfo[eRTPVLANIDMark]  },
    { &tVPRTPEntityLeafInfo[eRTPDSCPMark]  },
    { &tVPRTPEntityLeafInfo[eRTPXCT_STB_802_1pMark]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.NumberingPlan. EntityLEAF*******
struct CWMP_OP tVPNumberingPlanEntityLeafOP = { getVPNumberingPlanEntity, setVPNumberingPlanEntity};
struct CWMP_PRMT tVPNumberingPlanEntityLeafInfo[] =
{
    /*(name,                 type,           flag,                  op)*/
	{ "X_CT-COM_NumberPlan", eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPNumberingPlanEntityLeafOP},
};

enum eVPNumberingPlanEntityLeaf
{
	eXCT_NumberPlan
};

struct CWMP_LEAF tVPNumberingPlanEntityLeaf[] =
{
    { &tVPNumberingPlanEntityLeafInfo[eXCT_NumberPlan]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.FaxT38. EntityLEAF*******
struct CWMP_OP tVPFaxT38EntityLeafOP = { getVPFaxT38Entity, setVPFaxT38Entity};
struct CWMP_PRMT tVPFaxT38EntityLeafInfo[] =
{
    /*(name,    type,           flag,                  op)*/
	{ "Enable", eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE,  &tVPFaxT38EntityLeafOP}
};

enum eVPFaxT38EntityLeaf
{
	eVPT38ENABLE
};

struct CWMP_LEAF tVPFaxT38EntityLeaf[] =
{
    { &tVPFaxT38EntityLeafInfo[eVPT38ENABLE]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}.X_CT-COM_G711FAX. EntityLEAF*******
struct CWMP_OP tVPXCT_G711FAXEntityLeafOP = { getVPXCT_G711FAXEntity, setVPXCT_G711FAXEntity};
struct CWMP_PRMT tVPXCT_G711FAXEntityLeafInfo[] =
{
    /*(name,         type,           flag,                  op)*/
	{ "Enable",      eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE,  &tVPXCT_G711FAXEntityLeafOP},
	{ "ControlType", eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE,  &tVPXCT_G711FAXEntityLeafOP}
};

enum eVPXCT_G711FAXEntityLeaf
{
	eXCT711FAXEnable,
    eXCT711FAXControlType
};

struct CWMP_LEAF tVPXCT_G711FAXEntityLeaf[] =
{
    { &tVPXCT_G711FAXEntityLeafInfo[eXCT711FAXEnable]  },
    { &tVPXCT_G711FAXEntityLeafInfo[eXCT711FAXControlType]  },
    { NULL }
};


//******* VoiceService.{i}.VoiceProfile.X_CT-COM_IADDiagnostics EntityLEAF*******
struct CWMP_OP tVPXCT_IADDiagEntityLeafOP = { getVPXCT_IADDiagEntity, setVPXCT_IADDiagEntity };
struct CWMP_PRMT tVPXCT_IADDiagEntityLeafInfo[] =
{
    /*(name,                 type,           flag,                 op)*/
	{ "IADDiagnosticsState", eCWMP_tSTRING, CWMP_READ|CWMP_WRITE,  &tVPXCT_IADDiagEntityLeafOP},
	{ "TestServer",          eCWMP_tUINT,   CWMP_READ|CWMP_WRITE,  &tVPXCT_IADDiagEntityLeafOP},
	{ "RegistResult",        eCWMP_tUINT,   CWMP_READ,             &tVPXCT_IADDiagEntityLeafOP},
	{ "Reason",              eCWMP_tUINT,   CWMP_READ,             &tVPXCT_IADDiagEntityLeafOP}
};

enum eVPXCT_IADDiagEntityLeaf
{
	eVPXCTIADDiagnosticsState,
    eVPXCTIADTestServer,
    eVPXCTIADRegistResult,
    eVPXCTIADReason
};

struct CWMP_LEAF tVPXCT_IADDiagEntityLeaf[] =
{
    { &tVPXCT_IADDiagEntityLeafInfo[eVPXCTIADDiagnosticsState]  },
    { &tVPXCT_IADDiagEntityLeafInfo[eVPXCTIADTestServer]  },
    { &tVPXCT_IADDiagEntityLeafInfo[eVPXCTIADRegistResult]  },
    { &tVPXCT_IADDiagEntityLeafInfo[eVPXCTIADReason]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i}. EntityLEAF*******
struct CWMP_OP tVoiceProfileEntityLeafOP = { getVoiceProfileEntity, setVoiceProfileEntity};	//FIXME

struct CWMP_PRMT tVoiceProfileEntityLeafInfo[] =
{
    /*(name,                               type,           flag,                 op)*/
	{ "Enable",                            eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "Reset",                             eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "NumberOfLines",                     eCWMP_tUINT,    CWMP_READ,            &tVoiceProfileEntityLeafOP},
	{ "Name",                              eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "SignalingProtocol",                eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "MaxSessions",                       eCWMP_tUINT,    CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "DTMFMethod",                        eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "DigitMapEnable",                    eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "DigitMap",                          eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_ServerType",               eCWMP_tSTRING,  CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_StartDigitTimer",          eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_InterDigitTimerShort",     eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_InterDigitTimerLong",      eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_HangingReminderToneTimer", eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_BusyToneTimer",            eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP},
	{ "X_CT-COM_NoAnswerTimer",            eCWMP_tINT,     CWMP_READ|CWMP_WRITE, &tVoiceProfileEntityLeafOP}
};

enum eVoiceProfileEntityLeaf
{
    eVPEnable,
    eVPReset,
    eVPNumberOfLines,
    eVPVoiceProfileName,  //will duplicated with service provider
    eVPSignallingProtocol,
    eVPMaxSessions,
    eVPDTMFMethod,
	eVPDigitMapEnable,
	eVPDigitMap,
	eVPXCT_ServerType,
	eVPXCT_StartDigitTimer,
	eVPXCT_InterDigitTimerShort,
	eVPXCT_InterDigitTimerLong,
	eVPXCT_HangingReminderToneTimer,
	eVPXCT_BusyToneTimer,
	eVPXCT_NoAnswerTimer
};

struct CWMP_LEAF tVoiceProfileEntityLeaf[] =
{
    { &tVoiceProfileEntityLeafInfo[eVPEnable]  },
    { &tVoiceProfileEntityLeafInfo[eVPReset]  },
    { &tVoiceProfileEntityLeafInfo[eVPNumberOfLines]  },
    { &tVoiceProfileEntityLeafInfo[eVPVoiceProfileName]  },
    { &tVoiceProfileEntityLeafInfo[eVPSignallingProtocol]  },
    { &tVoiceProfileEntityLeafInfo[eVPMaxSessions]  },
    { &tVoiceProfileEntityLeafInfo[eVPDTMFMethod]  },
    { &tVoiceProfileEntityLeafInfo[eVPDigitMapEnable]  },
    { &tVoiceProfileEntityLeafInfo[eVPDigitMap]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_ServerType]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_StartDigitTimer]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_InterDigitTimerShort]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_InterDigitTimerLong]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_HangingReminderToneTimer]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_BusyToneTimer]  },
    { &tVoiceProfileEntityLeafInfo[eVPXCT_NoAnswerTimer]  },
    { NULL }
};

//******* VoiceService.{i}.VoiceProfile.{i} EntityObject*******
struct CWMP_OP tVoiceProfileLineOP = { NULL, objVoiceProfileLine };
struct CWMP_PRMT tVoiceProfileEntityObjectInfo[] =
{
    /*(name,                    type,           flag,       op)*/
    {"ServiceProviderInfo",     eCWMP_tOBJECT,  CWMP_READ,  NULL},
    {"SIP",                     eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{"RTP",                     eCWMP_tOBJECT,  CWMP_READ,  NULL},
#if 0 //FXO_SUPPORT
	{"NumberingPlan",           eCWMP_tOBJECT,  CWMP_READ,  NULL},
#endif	
	{"FaxT38",                  eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{"X_CT-COM_G711FAX",        eCWMP_tOBJECT,  CWMP_READ,  NULL},
	{"X_CT-COM_IADDiagnostics", eCWMP_tOBJECT,  CWMP_READ,  NULL},
    {"Line",                    eCWMP_tOBJECT,  CWMP_READ,  &tVoiceProfileLineOP}
};
enum eVoiceProfileEntityObject
{
	eVPServiceProviderInfo,
	eVPSIP,
	eVPRTP,
#if 0 //FXO_SUPPORT	
	eVPNumberingPlan,
#endif
	eVPFaxT38,
	eVPXCT_G711FAX,
	eVPXCT_IADDiagnostics,
	eVPLine
};
struct CWMP_NODE tVoiceProfileEntityObject[] =
{
    /*info,                                                   leaf,                          next)*/
    {&tVoiceProfileEntityObjectInfo[eVPServiceProviderInfo],  tVPServiceProviderEntityLeaf,  NULL},
    {&tVoiceProfileEntityObjectInfo[eVPSIP],                  tVPSIPEntityLeaf,              NULL},
	{&tVoiceProfileEntityObjectInfo[eVPRTP],                  tVPRTPEntityLeaf,              NULL},
#if 0 //FXO_SUPPORT	
	{&tVoiceProfileEntityObjectInfo[eVPNumberingPlan],        tVPNumberingPlanEntityLeaf,    NULL},
#endif	
	{&tVoiceProfileEntityObjectInfo[eVPFaxT38],               tVPFaxT38EntityLeaf,           NULL},
	{&tVoiceProfileEntityObjectInfo[eVPXCT_G711FAX],          tVPXCT_G711FAXEntityLeaf,      NULL},
	{&tVoiceProfileEntityObjectInfo[eVPXCT_IADDiagnostics],   tVPXCT_IADDiagEntityLeaf,      NULL},
    {&tVoiceProfileEntityObjectInfo[eVPLine],                 NULL,                          NULL},
    {NULL,                                                    NULL,                          NULL}
};


//******* VoiceService.{i}.VoiceProfile  *******
struct CWMP_PRMT tVoiceProfileObjectInfo[] =
{
    /*(name,  type,           flag,		               op)*/
    {"0",     eCWMP_tOBJECT,  CWMP_READ|CWMP_LNKLIST,  NULL}
};
enum eVoiceProfileObject
{
    eVoiceProfile0
};
struct CWMP_LINKNODE tVoiceProfileObject[] =
{
    /*info,  			                        leaf,                     next,                       sibling,  instnum)*/
    {&tVoiceProfileObjectInfo[eVoiceProfile0],  tVoiceProfileEntityLeaf,  tVoiceProfileEntityObject,  NULL,     0}
};


int objVoiceProfile(char *name, struct CWMP_LEAF *e, int type, void *data)
{
    struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) 
            return -1;

		num = MAX_PROFILE_COUNT; /* Now, we has one voice profile only. */
        
		for( i=1; i<=num;i++ )
		{
			if( create_Object( c, tVoiceProfileObject, sizeof(tVoiceProfileObject), num, 1) < 0 )
				return -1;
		}
        add_objectNum( name, num );
		return 0;
	}
    	break;
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
         {
            CWMPDBG( 0, ( stderr, "<%s:%d> type %d unsupported now\n", __FUNCTION__, __LINE__, type) );
            return 0;
         }
    	break;
    case eCWMP_tUPDATEOBJ:
         {
            int num=0,i;
            struct CWMP_LINKNODE *old_table;
            num = MAX_PROFILE_COUNT;
            old_table = (struct CWMP_LINKNODE*)entity->next;
            entity->next= NULL;
            for( i=1; i<=num;i++ ) {
                struct CWMP_LINKNODE *remove_entity=NULL;

                remove_entity = remove_SiblingEntity( &old_table, i );
                if( remove_entity!=NULL )
                {
                	add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
                }else{ 
    
                    add_Object( name, (struct CWMP_LINKNODE **)&entity->next,
                        tVoiceProfileObject, sizeof(tVoiceProfileObject), (unsigned int *)&num );
                }
           }
           if( old_table )  destroy_ParameterTable( (struct CWMP_NODE *)old_table );           
           return 0;
    default:
            CWMPDBG( 0, ( stderr, "<%s:%d>Error: unknown type %d \n", __FUNCTION__, __LINE__, type) );
            break;
        }
	   	break;
	}
	return -1;
}

int getVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
    /* VoiceService.{i}.VoiceProfile.{i}. */
    unsigned int nVoiceProfileInstNum;
    union {
        enable_t Enable;
        int Reset;
        unsigned int NumberOfLines;
        char Name[ 40 ];
        signaling_protocol_t SignallingProtocol;
        unsigned int MaxSessions;
        DTMF_method_t DTMFMethod;
		int DigitMapEnable;
		char DigitMap[MAX_DIALPLAN_LENGTH];
        char XCT_ServerType[16];
        int XCT_Timer;	//For Timers
        int default_index;
    } s;
    
    const char *pszLastname = entity->info->name;

    if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
        return -1;

    if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
        return -1;

    if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )  /* convert to zero based */
        return -1;
    
    *type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
    if( strcmp( pszLastname, "Enable" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
        *data = strdup_Enable( s.Enable );
    } else if( strcmp( pszLastname, "Reset" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
        *data = booldup( s.Reset );
    } else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &s.NumberOfLines );
        *data = uintdup( s.NumberOfLines );
    } else if( strcmp( pszLastname, "Name" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, &s.Name );
        *data = strdup( s.Name );
    } else if( strcmp( pszLastname, "SignalingProtocol" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
        *data = strdup_SignalingProtocol( s.SignallingProtocol );
    } else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, &s.MaxSessions );
        *data = uintdup( s.MaxSessions );
    } else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
        *data = strdup_DTMFMethod( s.DTMFMethod );  
    } else if( strcmp( pszLastname, "DigitMapEnable" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__DIGIT_MAP_ENABLE, nVoiceProfileInstNum, &s.DigitMapEnable );
        *data = booldup( s.DigitMapEnable );
    } else if( strcmp( pszLastname, "DigitMap" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__DIGIT_MAP, nVoiceProfileInstNum, &s.DigitMap );
        *data = strdup( s.DigitMap );  
    } else if( strcmp( pszLastname, "X_CT-COM_ServerType" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_SERVER_TYPE, nVoiceProfileInstNum, &s.XCT_ServerType );
        *data = strdup( s.XCT_ServerType );
    } else if( strcmp( pszLastname, "X_CT-COM_StartDigitTimer" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_START_DIGIT_TIMER, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );  
    } else if( strcmp( pszLastname, "X_CT-COM_InterDigitTimerShort" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_SHORT, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );
    } else if( strcmp( pszLastname, "X_CT-COM_InterDigitTimerLong" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_LONG, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );  
    } else if( strcmp( pszLastname, "X_CT-COM_HangingReminderToneTimer" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_HANGING_REMINDER_TONE_TIMER, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );
    } else if( strcmp( pszLastname, "X_CT-COM_BusyToneTimer" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_BUSY_TONE_TIMER, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );  
    } else if( strcmp( pszLastname, "X_CT-COM_NoAnswerTimer" ) == 0 ) {
        mib_get_type1( MIB_VOICE_PROFILE__XCT_NO_ANSWER_TIMER, nVoiceProfileInstNum, &s.XCT_Timer );
        *data = intdup( s.XCT_Timer );
    } else {
        *data = NULL;
        return ERR_9005;
    }
    
    return 0;
}

int setVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}. */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
		int DigitMapEnable;
		char DigitMap[MAX_DIALPLAN_LENGTH];
        char XCT_ServerType[16];
        int XCT_Timer;	//For Timers      
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "Enable" ) == 0 ){
		if( str2id_Enable( data, &s.Enable ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		s.Reset = *( ( int * )data );
		res = mib_set_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		return ERR_9008;	/* Read only */
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "SignalingProtocol" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.SignallingProtocol ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		if( str2id_DTMFMethod( data, &s.DTMFMethod ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "DigitMapEnable" ) == 0 ) {
   			res = mib_set_type1( MIB_VOICE_PROFILE__DIGIT_MAP_ENABLE, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "DigitMap" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__DIGIT_MAP, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_ServerType" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_SERVER_TYPE, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_StartDigitTimer" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_START_DIGIT_TIMER, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_InterDigitTimerShort" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_SHORT, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_InterDigitTimerLong" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_LONG, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_HangingReminderToneTimer" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_HANGING_REMINDER_TONE_TIMER, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_BusyToneTimer" ) == 0 ) {
        res = mib_set_type1(  MIB_VOICE_PROFILE__XCT_BUSY_TONE_TIMER, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_NoAnswerTimer" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_NO_ANSWER_TIMER, nVoiceProfileInstNum, data );
	} else {
		return ERR_9005;
	}
	
	return res;	//1->self-reboot, 0->apply without reboot
}

int getVPSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrarServer[ 256 ];
		unsigned int RegistrarServerPort;
		transport_t RegistrarServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
        int VLANIDMark;
        unsigned int DSCPMark ;
        unsigned int RegisterRetryInterval;
        unsigned int RegisterExpires;
        int XCT_802_1_pMark;
        boolean XCT_HeartbeatSwitch;
        int XCT_HeartbeatCycle;
        int XCT_HeartbeatCount;
        int XCT_SessionUpdateTimer;
		unsigned int proxyindex;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

	CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, &s.ProxyServer );
		*data = strdup( s.ProxyServer );
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, &s.ProxyServerPort );
		*data = uintdup( s.ProxyServerPort );
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		*data = strdup_Transport( s.ProxyServerTransport );
	} else if( strcmp( pszLastname, "RegistrarServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER, nVoiceProfileInstNum, &s.RegistrarServer );
		*data = strdup( s.RegistrarServer );
	} else if( strcmp( pszLastname, "RegistrarServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, &s.RegistrarServerPort );
		*data = uintdup( s.RegistrarServerPort );
	} else if( strcmp( pszLastname, "RegistrarServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
		*data = strdup_Transport( s.RegistrarServerTransport );
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, &s.UserAgentDomain );
		*data = strdup( s.UserAgentDomain );
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, &s.UserAgentPort );
		*data = uintdup( s.UserAgentPort );
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
		*data = strdup_Transport( s.UserAgentTransport );
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, &s.OutboundProxy );
		*data = strdup( s.OutboundProxy );
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, &s.OutboundProxyPort );
		*data = uintdup( s.OutboundProxyPort );
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, &s.Organization );
		*data = strdup( s.Organization );
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, &s.RegistrationPeriod );
		*data = uintdup( s.RegistrationPeriod );
	} else if( strcmp( pszLastname, "X_Standby-RegistrationPeriod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRATION_PERIOD, nVoiceProfileInstNum, &s.RegistrationPeriod );
		*data = uintdup( s.RegistrationPeriod );		
	} else if( strcmp( pszLastname, "VLANIDMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__VLANIDMARK, nVoiceProfileInstNum, &s.VLANIDMark );
		*data = intdup( s.VLANIDMark );
	} else if( strcmp( pszLastname, "DSCPMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__DSCPMARK, nVoiceProfileInstNum, &s.DSCPMark );
		*data = uintdup( s.DSCPMark );
	} else if( strcmp( pszLastname, "RegisterRetryInterval" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTERRETRYINTERVAL, nVoiceProfileInstNum, &s.RegisterRetryInterval );
		*data = uintdup( s.RegisterRetryInterval );
	} else if( strcmp( pszLastname, "X_Standby-RegisterRetryInterval" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTERRETRYINTERVAL, nVoiceProfileInstNum, &s.RegisterRetryInterval );
		*data = uintdup( s.RegisterRetryInterval );		
	} else if( strcmp( pszLastname, "RegisterExpires" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTEREXPIRES, nVoiceProfileInstNum, &s.RegisterExpires );
		*data = uintdup( s.RegisterExpires );
	} else if( strcmp( pszLastname, "X_Standby-RegisterExpires" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTEREXPIRES, nVoiceProfileInstNum, &s.RegisterExpires );
		*data = uintdup( s.RegisterExpires );		
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER, nVoiceProfileInstNum, &s.ProxyServer );
		*data = strdup( s.ProxyServer );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_PORT, nVoiceProfileInstNum, &s.ProxyServerPort );
		*data = uintdup( s.ProxyServerPort );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		*data = strdup_Transport( s.ProxyServerTransport );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER, nVoiceProfileInstNum, &s.RegistrarServer );
		*data = strdup( s.RegistrarServer );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, &s.RegistrarServerPort );
		*data = uintdup( s.RegistrarServerPort );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
		*data = strdup_Transport( s.RegistrarServerTransport );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-OutboundProxy" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY, nVoiceProfileInstNum, &s.OutboundProxy );
		*data = strdup( s.OutboundProxy );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-OutboundProxyPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, &s.OutboundProxyPort );
		*data = uintdup( s.OutboundProxyPort );		
	} else if( strcmp( pszLastname, "X_Default_Proxy_Index" ) == 0 ) {
			mib_get_type1( MIB_VOICE_PROFILE__XCT_DEFAULT_PROXY_INDEX, nVoiceProfileInstNum, &s.proxyindex );
			*data = uintdup( s.proxyindex );		
	} else if( strcmp( pszLastname, "X_CT-COM_802-1pMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_802_1PMARK, nVoiceProfileInstNum, &s.XCT_802_1_pMark );
		*data = intdup( s.XCT_802_1_pMark );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatSwitch" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATSWITCH, nVoiceProfileInstNum, &s.XCT_HeartbeatSwitch );
		*data = booldup( s.XCT_HeartbeatSwitch );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatCycle" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCYCLE, nVoiceProfileInstNum, &s.XCT_HeartbeatCycle );
		*data = intdup( s.XCT_HeartbeatCycle );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatCount" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCOUNT, nVoiceProfileInstNum, &s.XCT_HeartbeatCount );
		*data = intdup( s.XCT_HeartbeatCount );
	} else if( strcmp( pszLastname, "InviteExpires" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__INVITEEXPIRES, nVoiceProfileInstNum, &s.XCT_SessionUpdateTimer );
		*data = intdup( s.XCT_SessionUpdateTimer );
		
	} else if( strcmp( pszLastname, "X_Standby-InviteExpires" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__XCT_INVITEEXPIRES, nVoiceProfileInstNum, &s.XCT_SessionUpdateTimer );
		*data = intdup( s.XCT_SessionUpdateTimer );

	} else if( strcmp( pszLastname, "X_CT-COM_SessionUpdateTimer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__INVITEEXPIRES, nVoiceProfileInstNum, &s.XCT_SessionUpdateTimer );
		*data = intdup( s.XCT_SessionUpdateTimer );		
	} else {
		*data = NULL;
		return ERR_9005;
	}
	return 0;
}

int setVPSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		signaling_protocol_t ProxyServerTransport;
		char RegistrarServer[ 256 ];
		unsigned int RegistrarServerPort;
		signaling_protocol_t RegistrarServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		signaling_protocol_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
        int VLANIDMark;
        unsigned int DSCPMark ;
        int RegisterRetryInterval;
        int RegisterExpires;
        int XCT_802_1_pMark;
        int XCT_HeartbeatSwitch;
        int XCT_HeartbeatCycle;
        int XCT_HeartbeatCount;
        int XCT_SessionUpdateTimer;
	} s;
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

	CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.ProxyServerTransport ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "RegistrarServer" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "RegistrarServerPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "RegistrarServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.RegistrarServerTransport ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.UserAgentTransport ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_Standby-RegistrationPeriod" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRATION_PERIOD, nVoiceProfileInstNum, data );

		
	} else if( strcmp( pszLastname, "VLANIDMark" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__SIP__VLANIDMARK, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "DSCPMark" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__SIP__DSCPMARK, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "RegisterRetryInterval" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTERRETRYINTERVAL, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_Standby-RegisterRetryInterval" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTERRETRYINTERVAL, nVoiceProfileInstNum, data );


    } else if( strcmp( pszLastname, "RegisterExpires" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTEREXPIRES, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_Standby-RegisterExpires" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTEREXPIRES, nVoiceProfileInstNum, data );		
    } else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServer" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServerPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-ProxyServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.ProxyServerTransport ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServer" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServerPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-RegistrarServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.RegistrarServerTransport ) ){
			res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-OutboundProxy" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_Standby-OutboundProxyPort" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_Default_Proxy_Index" ) == 0 ) {
			res = mib_set_type1( MIB_VOICE_PROFILE__XCT_DEFAULT_PROXY_INDEX, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_802-1pMark" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_STB_802_1PMARK, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatSwitch" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATSWITCH, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatCycle" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCYCLE, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_HeartbeatCount" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCOUNT, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "InviteExpires" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__INVITEEXPIRES, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_Standby-InviteExpires" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_INVITEEXPIRES, nVoiceProfileInstNum, data );
	} else if( strcmp( pszLastname, "X_CT-COM_SessionUpdateTimer" ) == 0 ) {
		/* for e8c set session timer, update to proxy0 and proxy1 */
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__INVITEEXPIRES, nVoiceProfileInstNum, data );
		res = mib_set_type1( MIB_VOICE_PROFILE__SIP__XCT_INVITEEXPIRES, nVoiceProfileInstNum, data );
	
	} else {
		return ERR_9005;
	}
	return res;	//1->self-reboot, 0->apply without reboot
}


int getVPRTPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.RTP */
	unsigned int nVoiceProfileInstNum;
	union {
        int VLANIDMark;
        unsigned int DSCPMark ;
        int XCT_802_1_pMark;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

	CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "VLANIDMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RTP__VLANIDMARK, nVoiceProfileInstNum, &s.VLANIDMark );
		*data = intdup( s.VLANIDMark );
	} else if( strcmp( pszLastname, "DSCPMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RTP__DSCPMARK, nVoiceProfileInstNum, &s.DSCPMark );
		*data = uintdup( s.DSCPMark );
    } else if( strcmp( pszLastname, "X_CT-COM_802-1pMark" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RTP__XCT_STB_802_1PMARK, nVoiceProfileInstNum, &s.XCT_802_1_pMark );
		*data = intdup( s.XCT_802_1_pMark );
	} else {
		return ERR_9005;
	}

    return 0;
}

int setVPRTPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.RTP */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "VLANIDMark" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__RTP__VLANIDMARK, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "DSCPMark" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__RTP__DSCPMARK, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "X_CT-COM_802-1pMark" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__RTP__XCT_STB_802_1PMARK, nVoiceProfileInstNum, data );
    } else {
        return ERR_9005;
    }

    return res;	//1->self-reboot, 0->apply without reboot
}

int getVPNumberingPlanEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.NumberingPlan */
	unsigned int nVoiceProfileInstNum;
	union {
        char XCT_NumberPlan[256];
		// add more parameters here
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "X_CT-COM_NumberPlan" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NUMBERINGPLAN__XCT_NUMBERPLAN, nVoiceProfileInstNum, &s.XCT_NumberPlan );
		*data = strdup( s.XCT_NumberPlan );
	} else {
		return ERR_9005;
	}

    return 0;
}

int setVPNumberingPlanEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.NumberingPlan */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "X_CT-COM_NumberPlan" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__NUMBERINGPLAN__XCT_NUMBERPLAN, nVoiceProfileInstNum, data );
    } else {
        return ERR_9005;
    }

    return res;	//1->self-reboot, 0->apply without reboot
}

int getVPFaxT38Entity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.FaxT38 */
	unsigned int nVoiceProfileInstNum;
    union{
	    boolean Enable;
        // add more parameters here
    } s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__FAXT38__ENABLE, nVoiceProfileInstNum, &s.Enable );
		*data = booldup(s.Enable);
	} else {
		return ERR_9005;
	}

    return 0;
}

int setVPFaxT38Entity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.FaxT38 */
    int res = 0;
	unsigned int nVoiceProfileInstNum;

	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "Enable" ) == 0 ) {
       	res = mib_set_type1( MIB_VOICE_PROFILE__FAXT38__ENABLE, nVoiceProfileInstNum, data );
    
    } else {
        return ERR_9005;
    }

    return res;	//1->self-reboot, 0->apply without reboot
}


int getVPXCT_G711FAXEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.X_G711FAX */
	unsigned int nVoiceProfileInstNum;
    union{
	    int Enable;
        char ControlType[16];
    } s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__XCT_G711FAX__ENABLE, nVoiceProfileInstNum, &s.Enable );
		*data = intdup(s.Enable);
	} else if( strcmp( pszLastname, "ControlType" ) == 0  ){
		mib_get_type1( MIB_VOICE_PROFILE__XCT_G711FAX__CONTROLTYPE, nVoiceProfileInstNum, &s.ControlType );
		*data = strdup(s.ControlType);
    } else {
		return ERR_9005;
	}

    return 0;
}

int setVPXCT_G711FAXEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.FaxT38 */
    int res = 0;
    unsigned int nVoiceProfileInstNum;
    /*union{
        enable_t Enable;
        char ControlType[16];
    } s;*/

    const char *pszLastname = entity->info->name;

    if( (name==NULL) || (data==NULL) || (entity==NULL)) 
        return -1;

    if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
        return -1;

    if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )  /* convert to zero based */
        return -1;
            
    if( entity->info->type != type )
        return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );

    if( strcmp( pszLastname, "Enable" ) == 0 ) {
       res = mib_set_type1( MIB_VOICE_PROFILE__XCT_G711FAX__ENABLE, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "ControlType" ) == 0  ){
		res = mib_set_type1( MIB_VOICE_PROFILE__XCT_G711FAX__CONTROLTYPE, nVoiceProfileInstNum, data );
    } else {
        return ERR_9005;
    }    

    return res; //1->self-reboot, 0->apply without reboot
}


int getVPXCT_IADDiagEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.X_CT-COM_IADDiagnostics */
	unsigned int nVoiceProfileInstNum;
	union {
        char IADDiagnosticsState[32];
        unsigned int TestServer;
        unsigned int RegistResult;
        unsigned int Reason;
	} s;

	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;	
	*data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "IADDiagnosticsState" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__IADDIAGNOSTICSSTATE, nVoiceProfileInstNum, &s.IADDiagnosticsState );
		*data = strdup( s.IADDiagnosticsState );
	} else if( strcmp( pszLastname, "TestServer" ) == 0 ) { 
	    mib_get_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__TESTSERVER, nVoiceProfileInstNum, &s.TestServer );
		*data = uintdup( s.TestServer );
    } else if( strcmp( pszLastname, "RegistResult" ) == 0 ) { 
        mib_get_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__REGISTRESULT, nVoiceProfileInstNum, &s.RegistResult );
		*data = uintdup( s.RegistResult );
    } else if( strcmp( pszLastname, "Reason" ) == 0 ){ 
        mib_get_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__REASON, nVoiceProfileInstNum, &s.Reason );
		*data = uintdup( s.Reason );
    } else {
		return ERR_9005;
	}

    return 0;
}

int setVPXCT_IADDiagEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.X_IADDiagnostics */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> set %s value \n", __FUNCTION__, __LINE__, name) );
    
	if( strcmp( pszLastname, "IADDiagnosticsState" ) == 0 ) {
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__IADDIAGNOSTICSSTATE, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "TestServer" ) == 0 ) { 
    	/* E8C: TBD  NOT Support yet */
        res = mib_set_type1( MIB_VOICE_PROFILE__XCT_IADDIAG__TESTSERVER, nVoiceProfileInstNum, data );
    } else if( strcmp( pszLastname, "RegistResult" ) == 0 ) { 
		return ERR_9008;	/* Read only */
    } else if( strcmp( pszLastname, "Reason" ) == 0 ){ 
        return ERR_9008;	/* Read only */
    } else {
        return ERR_9005;
    }

    return res;	//1->self-reboot, 0->apply without reboot
}

int getVPServiceProviderEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	char szName[ 256 ];
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity->info->type;
    *data = NULL;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );
		
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, &szName );
		*data = strdup( szName );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	return 0;
}

int setVPServiceProviderEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo */
    int res = 0;
	unsigned int nVoiceProfileInstNum;
	const char *pszLastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity->info->type != type )
		return ERR_9006;

    CWMPDBG( 3, ( stderr, "<%s:%d> get %s value \n", __FUNCTION__, __LINE__, name) );

	if( strcmp( pszLastname, "Name" ) == 0 ) {
		res = mib_set_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, data );
	} else {
		return ERR_9005;
	}
	return res;	//1->self-reboot, 0->apply without reboot
}


#if 0
static int getVoiceProfileSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setVoiceProfileSipEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

static int getServiceProviderInfoEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setServiceProviderInfoEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

static int getVoiceProfileEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
static int setVoiceProfileEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data);

struct sCWMP_ENTITY tVoiceProfileSipEntity[] = {
/*	{ name,								type,			flag,					accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "ProxyServer",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "ProxyServerPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "ProxyServerTransport",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServer",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServerPort",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServerTransport",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentDomain",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentPort",					eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentTransport",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "OutboundProxy",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "OutboundProxyPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "Organization",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationPeriod",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "",								eCWMP_tNONE,	0,						NULL,		NULL,						NULL,						NULL,		NULL },
};

struct sCWMP_ENTITY tServiceProviderInfoEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "Name",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getServiceProviderInfoEntity, 	setServiceProviderInfoEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,							NULL,							NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfileEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,				setvalue,				next_table,					sibling } */
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "Reset",				eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "NumberOfLines",		eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "Name",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "SignalingProtocol",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "MaxSessions",		eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "DTMFMethod",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "ServiceProviderInfo",eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					NULL,					tServiceProviderInfoEntity,	NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					NULL,					tVoiceProfileSipEntity,		NULL },
	{ "Line",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					objVoiceProfileLine,	NULL,						NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,					NULL,					NULL,		NULL },
};

struct sCWMP_ENTITY tVoiceProfile[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileEntity, 	NULL },
};

static int getVoiceProfileSipEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrationServer[ 256 ];
		unsigned int RegistrationServerPort;
		transport_t RegistrationServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;	
		
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, &s.ProxyServer );
		*data = strdup( s.ProxyServer );
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, &s.ProxyServerPort );
		*data = uintdup( s.ProxyServerPort );
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		*data = strdup_Transport( s.ProxyServerTransport );
	} else if( strcmp( pszLastname, "RegistrationServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER, nVoiceProfileInstNum, &s.RegistrationServer );
		*data = strdup( s.RegistrationServer );
	} else if( strcmp( pszLastname, "RegistrationServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, &s.RegistrationServerPort );
		*data = uintdup( s.RegistrationServerPort );
	} else if( strcmp( pszLastname, "RegistrationServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrationServerTransport );
		*data = strdup_Transport( s.RegistrationServerTransport );
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, &s.UserAgentDomain );
		*data = strdup( s.UserAgentDomain );
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, &s.UserAgentPort );
		*data = uintdup( s.UserAgentPort );
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
		*data = strdup_Transport( s.UserAgentTransport );
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, &s.OutboundProxy );
		*data = strdup( s.OutboundProxy );
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, &s.OutboundProxyPort );
		*data = uintdup( s.OutboundProxyPort );
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, &s.Organization );
		*data = strdup( s.Organization );
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, &s.RegistrationPeriod );
		*data = uintdup( s.RegistrationPeriod );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setVoiceProfileSipEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrationServer[ 256 ];
		unsigned int RegistrationServerPort;
		transport_t RegistrationServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.ProxyServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "RegistrationServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.RegistrationServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrationServerTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.UserAgentTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, data );
		return 1;
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getServiceProviderInfoEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	char szName[ 256 ];
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;	
		
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, &szName );
		*data = strdup( szName );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setServiceProviderInfoEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, data );
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getVoiceProfileEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	/* VoiceService.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		*data = booldup( s.Reset );
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &s.NumberOfLines );
		*data = uintdup( s.NumberOfLines );
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, &s.Name );
		*data = strdup( s.Name );
	} else if( strcmp( pszLastname, "SignalingProtocol" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
		*data = strdup_SignalingProtocol( s.SignallingProtocol );
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, &s.MaxSessions );
		*data = uintdup( s.MaxSessions );
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
		*data = strdup_DTMFMethod( s.DTMFMethod );	
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		if( str2id_Enable( data, &s.Enable ) ){
			mib_set_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		s.Reset = *( ( int * )data );
		mib_set_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		return 1;
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		return ERR_9008;	/* Read only */
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "SignalingProtocol" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.SignallingProtocol ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		if( str2id_DTMFMethod( data, &s.DTMFMethod ) ){
			mib_set_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
			return 1;
		}else{
			return ERR_9007;
		}
	} else {
		return ERR_9005;
	}
	
	return 1;
}

int objVoiceProfile(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;
		//MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			//p = &route_entity;
			//if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			//	continue;
			
			//if( p->InstanceNum==0 ) //maybe createn by web or cli
			//{
			//	MaxInstNum++;
			//	p->InstanceNum = MaxInstNum;
			//	mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			//}else
			//	MaxInstNum = p->InstanceNum;
			
			if( create_Object( c, tVoiceProfile, sizeof(tVoiceProfile), MAX_PROFILE_COUNT, 1 ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, 1 );
		return 0;
	}
#if 0
	// TODO: Not implement add/del/update yet.  
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, &entity->next_table,  tForwarding, sizeof(tForwarding), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_ROUTE_T fentry;
			memset( &fentry, 0, sizeof( MIB_CE_IP_ROUTE_T ) );
			fentry.InstanceNum = *(unsigned int*)data;
			fentry.FWMetric = -1;
			fentry.ifIndex = 0xff;
			mib_chain_add( MIB_IP_ROUTE_TBL, (unsigned char*)&fentry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	id = getChainID( entity->next_table, *(int*)data  );
	     	if(id==-1) return ERR_9005;
	     	mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
		ret = del_Object( name, &entity->next_table, *(int*)data );
		if( ret == 0 ) return 1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {     	int num=0,i;
	     	struct sCWMP_ENTITY *old_table;
	     	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	     	old_table = entity->next_table;
	     	entity->next_table = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct sCWMP_ENTITY *remove_entity=NULL;
			MIB_CE_IP_ROUTE_T *p,route_entity;

			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( &entity->next_table, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;			
				add_Object( name, &entity->next_table,  tForwarding,     							sizeof(tForwarding), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{     p->InstanceNum = MaxInstNum;
				      mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	if( old_table ) 	destroy_ParameterTable( old_table );	     	
	     	return 0;
	     }
#endif
	}
	return -1;
}
#endif
