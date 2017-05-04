#include <string.h>
#include <stdio.h>
//#include "apmib.h"
#include "mib_tr104.h"
#include "voip_flash.h"
#include "prmt_capabilities.h"
#include "prmt_limit.h"
#include "prmt_igd.h"

#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include "mibtbl.h"
#endif
#include "cwmpevt.h"


#define IS_VOICE_PROFILE(X,NUM) (X < MIB_VOICE_PROFILE__ARG_TYPE##NUM##_END)
#define IS_PHYINTERFACE(X,NUM) 	(X > MIB_VOICE_PROFILE__ARG_TYPE##NUM##_END &&\
                                 X < MIB_PHYINTERFACE__ARG_TYPE##NUM##_END)

/* voip share memory */
voipCfgParam_t *pVoipConfig = NULL;
extern cwmpEvtMsg pEvtMsg;

/* Init voip client */
int getVoipFlash(void){

    CWMPDBG( 3, ( stderr, "DEBUG: initialize configure\n") );
	if (voip_flash_get(&pVoipConfig) == -1){
		CWMPDBG( 0, ( stderr, "<%s:%d> ERROR: voip_flash_get failed.\n", __FUNCTION__, __LINE__) );
        return -1;        
    }

    if( pVoipConfig == NULL ){
        CWMPDBG( 0, ( stderr, "<%s:%d> ERROR: pVoipConfig is NULL\n", __FUNCTION__, __LINE__) );
        return -1;
    }
	return 0;
}

int update_VoIP_FLASH(){
	int res=0;
	if( pVoipConfig == NULL) {
		//printf("pVoipConfig is null %s\n",__FUNCTION__);
		 CWMPDBG( 0, ( stderr, "<%s:%d> no voip config need save.\n", __FUNCTION__, __LINE__) );
		  return 0;
	}
	//printf("update_VoIP_FLASH ok \n");
	voip_flash_set(pVoipConfig);
	voip_flash_server_init_variables();
	res=voip_flash_server_update();
	if(res==-1){
		printf("voip_flash_server_update fail\n");
	}
}

/* get the sip uri. If domain is NULL, device LAN ip is replaced*/
static void getSIPURI(char *sipURI, char *username, char *domain){
	boolean buserName=TRUE;
	char addr[DNS_LEN];
	
	if(NULL == username || 0 == strlen(username)){
		buserName=FALSE;
	}
	if(NULL == domain || 0 == strlen(domain)){
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_get(MIB_IP_ADDR, addr, sizeof(addr));
		domain=inet_ntoa(*((struct in_addr *)addr));
#else
		getMIB2Str(MIB_ADSL_LAN_IP, addr);		
		domain=addr;
#endif
	}
	if(TRUE == buserName)
		sprintf(sipURI,"%s@%s",username,domain);
	else
		sprintf(sipURI,"%s",domain);
}

/* get the flash codec index */
static int getCodecIndex(const char *codecName, int bitRate){
	
	if( NULL == codecName)
		return -1;
	
	if(0 == strcmp("G.711MuLaw",codecName)){
		return _CODEC_G711U;
	}else if(0 == strcmp("G.711ALaw",codecName)){
		return _CODEC_G711A;
	}else if(0 == strcmp("G.729",codecName)){
		return _CODEC_G729;
	}else if(0 == strcmp("G.723.1",codecName) ){
		return _CODEC_G723;
	}else if(0 == strcmp("G.726",codecName) && 16000 == bitRate){
		return _CODEC_G726_16;
	}else if(0 == strcmp("G.726",codecName) && 24000 == bitRate){
		return _CODEC_G726_24;
	}else if(0 == strcmp("G.726",codecName) && 32000 == bitRate){
		return _CODEC_G726_32;
	}else if(0 == strcmp("G.726",codecName) && 40000 == bitRate){
		return _CODEC_G726_40;
	}else if(0 == strcmp("GSM-FR",codecName)){
		return _CODEC_GSMFR;
	}else if(0 == strcmp("iLBC",codecName)){
		return _CODEC_ILBC;
	}else if(0 == strcmp("G.722",codecName)){
		return _CODEC_G722;
	}
	
	return -1;
}

void mib_config_clean(void){
	pVoipConfig = NULL;
}

/* verify the setting values */
static int verifySettingValues(idMib_t idMib, int nPort, voipCfgParam_t *voipConfig,void *pData){
//	voipCfgProxy_t *proxy = &voipConfig->ports[nPort].proxies[voipConfig->ports[nPort].default_proxy];

	switch(idMib){
#if 0 //eric: not need check string empty		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER: /*pass through*/
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY:
	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:		
//		if(proxy->enable && (0 == strlen((char *)pData) || NULL == pData)){
		/* only test string length */
		if((0 == strlen((char *)pData) || NULL == pData)){
			CWMPDBG( 0, ( stderr, "ERROR:Proxy server name should not be empty\n") );
			return VERIFY_ERROR;
		}
		break;
#endif
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT: /*pass through*/
	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT: /*pass through*/
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_PORT:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_PORT:
		if(*( ( unsigned int * )pData ) < 0 || *( ( unsigned int * )pData ) > 65535){
			CWMPDBG( 0, ( stderr, "ERROR:Port should be in the range 0~65535\n") );
			return VERIFY_ERROR;
		}
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRATION_PERIOD:
		if(*( ( unsigned int * )pData ) < 10 || *( ( unsigned int * )pData )  > 86400){
			CWMPDBG( 0, ( stderr, "ERROR:Registration period should be in range 10~86400\n") );
			return VERIFY_ERROR;
		}
		break;

    case MIB_VOICE_PROFILE__SIP__VLANIDMARK:
    case MIB_VOICE_PROFILE__RTP__VLANIDMARK:
        if(*( ( unsigned int * )pData ) < -1){
			CWMPDBG( 0, ( stderr, "ERROR:VLAN ID should be in equal or larger than -1.\n") );
			return VERIFY_ERROR;
		}
        break;

    case MIB_VOICE_PROFILE__SIP__DSCPMARK:
    case MIB_VOICE_PROFILE__RTP__DSCPMARK:
        if(*( ( unsigned int * )pData ) < 0 || *( ( unsigned int * )pData ) > 63){
			CWMPDBG( 0, ( stderr, "ERROR:DSCP Mark should be in range 0~63.\n") );
			return VERIFY_ERROR;
		}
        break;
        
    case MIB_VOICE_PROFILE__SIP__REGISTERRETRYINTERVAL:
    case MIB_VOICE_PROFILE__SIP__REGISTEREXPIRES:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTERRETRYINTERVAL:
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTEREXPIRES:
        if(*( ( unsigned int * )pData ) < 1){
			CWMPDBG( 0, ( stderr, "ERROR:REGISTER Expires or Retry Interval should be in equal or larger than 1.\n") );
			return VERIFY_ERROR;
		}
        break;
        
#ifdef CONFIG_RTK_VOIP_SLIC_NUM_2
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
	{
		int otherPort=0;
		otherPort = (nPort == 0) ? 1 : 0;
		if(*( ( unsigned int * )pData ) == voipConfig->ports[otherPort].sip_port){
			CWMPDBG( 0, ( stderr, "ERROR:Sip port of this line should not be as same as that of other line\n") );
			return VERIFY_ERROR;
		}
	}
		break;
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
	{
		char addr[DNS_LEN];
      
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
        char *ptrTempAddr=NULL; 
		mib_get(MIB_IP_ADDR, addr, sizeof(addr));
		ptrTempAddr = inet_ntoa(*((struct in_addr *)addr));
		if(0 == strcmp(ptrTempAddr,(char*)pData))
#else
		getMIB2Str(MIB_ADSL_LAN_IP, addr);
		if(0 == strcmp(addr,(char*)pData))
#endif
		{
			CWMPDBG( 0, ( stderr, "ERROR:Domain should not be the LAN IP\n") );
			return VERIFY_ERROR;
		}
	}	
		break;
	default:
		return VERIFY_OK;

	}
	return VERIFY_OK;
}

int mib_get_type1( idMib_t idMib, unsigned int nInstNum1, void *pData )
{	
	/* VoiceService.1.VoiceProfile.{ i }. */
    /* VoiceService.1.PhyInterface.{ i }. */

	voipCfgProxy_t *proxy1;/* main proxy */
	voipCfgProxy_t *proxy2;/* backup,second proxy */
	int nPort = 0;

	if( pVoipConfig == NULL && getVoipFlash()!=0 ) {	//Alex, Modified, 110913
		return 0;
	}
    
	if( IS_VOICE_PROFILE(idMib,1) && (nInstNum1 >= MAX_PROFILE_COUNT) ){
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong Voice Profile InstNum \n", __FUNCTION__, __LINE__));
		return 0;
    } else if( IS_PHYINTERFACE(idMib,1) && (nInstNum1 >= MAX_PHYINTERFACE_COUNT)) {
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong PhyInterface InstNum \n", __FUNCTION__, __LINE__));
		return 0;
	}
        
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif

	CWMPDBG( 3, ( stderr, "<%s:%d> DEBUG: getting value with InstNum1 %d idMib %d \n",
		__FUNCTION__, __LINE__, nInstNum1, idMib) );

	proxy1 = &pVoipConfig->ports[nPort].proxies[0];
	proxy2 = &pVoipConfig->ports[nPort].proxies[1];  
    
	switch( idMib ) {
	case MIB_VOICE_PROFILE__ENABLE:
		*( ( enable_t * )pData ) = TR104_ENABLE;
		break;
		
	case MIB_VOICE_PROFILE__RESET:
		*( ( boolean * )pData ) = FALSE;
		break;
		
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
		*( ( unsigned int * )pData ) = MAX_LINE_PER_PROFILE;
		break;
		
	case MIB_VOICE_PROFILE__NAME:
	{
		char profileName[PROFILE_NAME_LEN];
		sprintf(profileName,"%s%d","profile",nInstNum1 + 1);
		strcpy( ( char * )pData, profileName);
	}
		break;
		
	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
		*( ( signaling_protocol_t * )pData ) = SIP;
		break;
		
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
		*( ( unsigned int * )pData ) = PROFILE_MAX_SESSION;
		break;
	
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		*( ( DTMF_method_t * )pData ) = pVoipConfig ->ports[nPort].dtmf_mode;
		break;
		
	case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
		strcpy( ( char * )pData, SERVICE_PROVIDER );
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		strcpy( ( char * )pData, proxy1->addr);
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy1->port;
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER:
		strcpy( ( char * )pData, proxy1->addr);
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy1->port;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		if(NULL == proxy1->domain_name || 0 == strlen(proxy1->domain_name)){
			char addr[DNS_LEN];
       
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
			printf( "sizeof(addr):%d\n", sizeof(addr) );
			mib_get(MIB_IP_ADDR, addr, sizeof(addr));
			strcpy( ( char * )pData, inet_ntoa(*((struct in_addr *)addr)));
#else
			getMIB2Str(MIB_ADSL_LAN_IP, addr);
			strcpy( ( char * )pData, addr);
#endif
		}else{
			strcpy( ( char * )pData, proxy1->domain_name);
		}
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		*( ( unsigned int * )pData ) = pVoipConfig ->ports[nPort].sip_port;
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		if(proxy1->outbound_enable==1)
		strcpy( ( char * )pData, proxy1->outbound_addr);
		else
			 strcpy( ( char * )pData, "\0");
		break;

	case MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY:

		if(proxy2->outbound_enable==1)
		strcpy( ( char * )pData, proxy2->outbound_addr);
		else
			 strcpy( ( char * )pData, "\0");
		break;
		
		break;
		

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		*( ( unsigned int * )pData ) = proxy1->outbound_port;
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
		strcpy( ( char * )pData, ORGANIZATION_HEADER );
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		*( ( unsigned int * )pData ) = proxy1->reg_expire;
		break;
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRATION_PERIOD:
		*( ( unsigned int * )pData ) = proxy2->reg_expire;
		break;
		
	case MIB_VOICE_PROFILE__DIGIT_MAP_ENABLE:
		 *( ( unsigned int * )pData )= pVoipConfig ->ports[nPort].digitmap_enable ;

		break;

		break;
	case MIB_VOICE_PROFILE__DIGIT_MAP:
		strcpy( ( char * )pData, (const char *)pVoipConfig ->ports[nPort].dialplan);
		break;
	
	case MIB_VOICE_PROFILE__XCT_SERVER_TYPE:
		if(pVoipConfig ->X_CT_servertype==0)
			strcpy( ( char * )pData, "0");
		else 
			strcpy( ( char * )pData, "1");
		break;
	case MIB_VOICE_PROFILE__XCT_START_DIGIT_TIMER:
		*( ( unsigned int * )pData ) = pVoipConfig->off_hook_alarm;
		break;
	case MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_SHORT:
		*( ( unsigned int * )pData ) = pVoipConfig->auto_dial;
		break;		
	case MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_LONG:
		*( ( unsigned int * )pData ) = pVoipConfig->InterDigitTimerLong;
		break;

	case MIB_VOICE_PROFILE__XCT_HANGING_REMINDER_TONE_TIMER:
		*( ( unsigned int * )pData ) = pVoipConfig->HangingReminderToneTimer;
		break;		
	case MIB_VOICE_PROFILE__XCT_BUSY_TONE_TIMER:

		*( ( unsigned int * )pData ) = pVoipConfig->BusyToneTimer;
		break;	

		
	case MIB_VOICE_PROFILE__XCT_NO_ANSWER_TIMER:

		*( ( unsigned int * )pData ) = pVoipConfig->NoAnswerTimer;
		break;	
	case MIB_VOICE_PROFILE__XCT_DEFAULT_PROXY_INDEX:		
		*( ( unsigned int * )pData ) = pVoipConfig ->ports[nPort].default_proxy;
			break;	

		
		
	case MIB_VOICE_PROFILE__SIP__VLANIDMARK:
	case MIB_VOICE_PROFILE__RTP__VLANIDMARK:
		*( ( int * )pData ) = pVoipConfig->wanVlanIdVoice;
		break;

	case MIB_VOICE_PROFILE__SIP__DSCPMARK:
		*( ( unsigned int * )pData ) = pVoipConfig->sipDscp;
		break;

		
    case MIB_VOICE_PROFILE__SIP__REGISTERRETRYINTERVAL:
		*( ( unsigned int * )pData ) =  proxy1->RegisterRetryInterval;
		break;
		
   case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTERRETRYINTERVAL:
		*( ( unsigned int * )pData ) = proxy2->RegisterRetryInterval;
		break;		
		
    case MIB_VOICE_PROFILE__SIP__REGISTEREXPIRES:
        *( ( unsigned int * )pData ) =  proxy1->reg_expire;
		break;
	
    case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTEREXPIRES:
        *( ( unsigned int * )pData ) = proxy2->reg_expire;
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER:
		strcpy( ( char * )pData,proxy2->addr);
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy2->port;
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER:
		strcpy( ( char * )pData,proxy2->addr);
		break;
	
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy2->port;
		break;
	
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		

		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY_PORT:
		*( ( unsigned int * )pData ) = proxy2->outbound_port;		
		break;
	case MIB_VOICE_PROFILE__SIP__XCT_STB_802_1PMARK:
	case MIB_VOICE_PROFILE__RTP__XCT_STB_802_1PMARK:
		*( ( unsigned int * )pData ) = pVoipConfig->wanVlanPriorityVoice;		
		break;
		
    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATSWITCH:
		if(proxy1->enable & PROXY_OPTIONS_PING){
			*( ( boolean* )pData )=1;
		}else{
			*( ( boolean * )pData )=0;
		}
		break;
		
    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCYCLE:
		*( ( unsigned int * )pData ) = proxy1->HeartbeatCycle;		
		break;

    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCOUNT:
		*( ( unsigned int * )pData ) = proxy1->HeartbeatCount;		
		break;
		
    case MIB_VOICE_PROFILE__SIP__INVITEEXPIRES:
		*( ( unsigned int * )pData ) = proxy1->SessionUpdateTimer;		
		break;

    case MIB_VOICE_PROFILE__SIP__XCT_INVITEEXPIRES:
		*( ( unsigned int * )pData ) = proxy2->SessionUpdateTimer;		
		break;

		

	case MIB_VOICE_PROFILE__RTP__DSCPMARK:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			*( ( unsigned int * )pData ) = pVoipConfig ->rtpDscp;
        }
		break;

	case MIB_VOICE_PROFILE__NUMBERINGPLAN__XCT_NUMBERPLAN:
		/* e8c: NOT SUPPORT */
		strcpy( ( char * )pData, "");
		break;
    case MIB_VOICE_PROFILE__XCT_G711FAX__ENABLE:
		*( ( unsigned int * )pData ) =1;	 /* always support g711 */
		break;
	
    case MIB_VOICE_PROFILE__XCT_G711FAX__CONTROLTYPE:
		strcpy( ( char * )pData, "all");
		break;
		
	case MIB_VOICE_PROFILE__XCT_IADDIAG__TESTSERVER:
		/* E8C: TBD , read default proxy index */
	
		*( ( unsigned int * )pData ) =pEvtMsg.voiceProfileLineStatusMsg[nPort].TestServer+1;


		break;
	case MIB_VOICE_PROFILE__XCT_IADDIAG__IADDIAGNOSTICSSTATE:
		/* e8c : TBD: need check ACS server */
		strcpy( ( char * )pData, "None");
		break;
		
	case MIB_VOICE_PROFILE__XCT_IADDIAG__REGISTRESULT:

		*( ( unsigned int * )pData ) =pEvtMsg.voiceProfileLineStatusMsg[nPort].RegistResult; /* 0 is success , 1 fail*/
		break;
		
	case MIB_VOICE_PROFILE__XCT_IADDIAG__REASON:        
  
		  *( ( unsigned int * )pData ) =pEvtMsg.voiceProfileLineStatusMsg[nPort].Reason;
		break;
    case MIB_PHYINTERFACE__INTERFACEID:
		/*e8c : only have 1 fxs */
		*( ( unsigned int * )pData ) = 1;
		break;
   case MIB_PHYINTERFACE__PHYPORT:
		{
			char phyport[2];
			sprintf(phyport,"%d",nInstNum1 + 1);
			strcpy( ( char * )pData, phyport);
   		}
		break;
#if 1 //e8c project
    case MIB_PHYINTERFACE__TESTS__TESTSTATE:
		if(pEvtMsg.e8c_autotest.TestStatus==0)
			strcpy( ( char * )pData, "None");
		else if(pEvtMsg.e8c_autotest.TestStatus==1)
			strcpy( ( char * )pData, "Requested");
		else 
			strcpy( ( char * )pData, "Complete");
		break;		
    case MIB_PHYINTERFACE__TESTS__TESTSELECTOR:
		
		if( pEvtMsg.e8c_autotest.TestSelector == 0){
				strcpy( ( char * )pData, "PhoneConnectivityTest");
		}else{ /* testselector=1 */
			strcpy( ( char * )pData, "X_SimulateTest");

		}
		break;
    case MIB_PHYINTERFACE__TESTS__PHONECONNECTIVITY:
		if (pEvtMsg.e8c_autotest.PhoneConnectivity==0)
			*( ( boolean * )pData ) = FALSE;
		else
			*( ( boolean * )pData ) = TRUE;
		break;
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__TESTTYPE:
		if(pEvtMsg.e8c_autotest.enable==0)
			strcpy( ( char * )pData, "None");
		else if(pEvtMsg.e8c_autotest.TestType==0)
			strcpy( ( char * )pData, "Caller");
		else 
			strcpy( ( char * )pData, "Called");
		break;
		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLEDNUMBER:
		strcpy( ( char * )pData, pEvtMsg.e8c_autotest.callednumber);
		break;


    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMENABLE:
		*( ( unsigned int * )pData ) =  pEvtMsg.e8c_autotest.DailDTMFConfirmEnable;	
		break;

		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMNUMBER:

		strcpy( ( char * )pData, pEvtMsg.e8c_autotest.DailDTMFConfirmNumber);
		break;

		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMRESULT:
		
		*( ( unsigned int * )pData ) =  pEvtMsg.e8c_autotest.DailDTMFConfirmResult;	
		break;

    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__STATUS:

		strcpy( ( char * )pData, pEvtMsg.e8c_autotest.Simulate_Status);		
		break;
		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CONCLUSION:

		if(pEvtMsg.e8c_autotest.Conclusion==0)
			strcpy( ( char * )pData, "Success");
		else
			strcpy( ( char * )pData, "Fail");

		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLERFAILREASON:

		switch(pEvtMsg.e8c_autotest.CallerFailReason){
			case CALLER_NONE:
				strcpy( ( char * )pData, "\0");
				break;
			case CALLER_NO_DIALTONE:
				strcpy( ( char * )pData, "NoDialTone");
				break;
			case CALLER_OFFHOOK_RELEASE:
				strcpy( ( char * )pData, "OffhookRelease");
				break;
			case CALLER_AFTERDIAL_RELEASE:
				strcpy( ( char * )pData, "AfterDialRelease");
				break;
			case CALLER_NO_ANSWER:
				strcpy( ( char * )pData, "NoAnswer");
				break;				
		}
		break;
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLEDFAILREASON:

		switch(pEvtMsg.e8c_autotest.CalledFailReason){
			case CALLEE_NONE:
				strcpy( ( char * )pData, "\0");
				break;
			case CALLEE_NO_INCOMINGCALL:
				strcpy( ( char * )pData, "NoIncomingCall");
				break;
			case CALLEE_OFFHOOK_RELEASE:
				strcpy( ( char * )pData, "OffhookRelease");
				break;
			case CALLEE_NO_ANSWER:
				strcpy( ( char * )pData, "NoAnswer");
				break;				
		}
		break;
#endif
		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__FAILEDRESPONSECODE:
	      *( ( unsigned int * )pData ) = 1;
        break;
	case MIB_VOICE_PROFILE__FAXT38__ENABLE:
		  *( ( boolean* )pData )=pVoipConfig ->ports[nPort].useT38 ;
		break;
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type1( idMib_t idMib, unsigned int nInstNum1, 
				   void *pData )
{	
	/* VoiceService.1.VoiceProfile.{ i }. */
	//voipCfgParam_t voipConfig;
	//voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy1; // main proxy
	voipCfgProxy_t *proxy2; // second proxy

	
	int nPort=0;
	boolean bDirtyBit = FALSE;

	if( pVoipConfig == NULL && getVoipFlash()!=0 ) {	//Alex, Modified, 110913
		return 0;
	}    
    
	if( IS_VOICE_PROFILE(idMib,1) && (nInstNum1 >= MAX_PROFILE_COUNT) ){
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong Voice Profile InstNum \n", __FUNCTION__, __LINE__));
		return 0;
    } else if( IS_PHYINTERFACE(idMib,1) && (nInstNum1 >= MAX_PHYINTERFACE_COUNT)) {
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong PhyInterface InstNum \n", __FUNCTION__, __LINE__));
		return 0;
	}
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy1 = &pVoipConfig ->ports[nPort].proxies[0];
	proxy2 = &pVoipConfig ->ports[nPort].proxies[1];

	
	switch( idMib ) {	
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		pVoipConfig ->ports[nPort].dtmf_mode = *( ( DTMF_method_t * )pData );
		bDirtyBit = TRUE;
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy1->addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy1->port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
    case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_TRANSPORT:
    case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_TRANSPORT:
    case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_TRANSPORT:
   		// unable to set
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy1->addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRAR_SERVER_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy1->port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;
	
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy1->domain_name, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			pVoipConfig ->ports[nPort].sip_port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
			strcpy( proxy1->outbound_addr, ( char * )pData );
			if(0==strlen((char *)pData)){
				proxy1->outbound_enable=0;
			}else{
				proxy1->outbound_enable=1;
		}
            bDirtyBit = TRUE;
		break;

	case MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY:
		strcpy( proxy2->outbound_addr, ( char * )pData );
		if(0==strlen((char *)pData)){
			proxy2->outbound_enable=0;
		}else{
			proxy2->outbound_enable=1;
		}
		bDirtyBit = TRUE; 
		break;
	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy1->outbound_port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
   		//unable to set
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy1->reg_expire = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRATION_PERIOD:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy2->reg_expire = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;
		
	case MIB_VOICE_PROFILE__DIGIT_MAP_ENABLE:

		pVoipConfig ->ports[nPort].digitmap_enable =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
		
	case MIB_VOICE_PROFILE__DIGIT_MAP:		

		if(strlen((char *)pData)<MAX_DIALPLAN_LENGTH){
			strcpy( (char *)pVoipConfig ->ports[nPort].dialplan, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;
		
	case MIB_VOICE_PROFILE__XCT_SERVER_TYPE:
		if(strcmp(( char * )pData ,"0")==0)
			pVoipConfig ->X_CT_servertype=0;
		else
			pVoipConfig ->X_CT_servertype=1;
		break;
	case MIB_VOICE_PROFILE__XCT_START_DIGIT_TIMER:
		pVoipConfig->off_hook_alarm =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
	case MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_SHORT:
		pVoipConfig ->auto_dial =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
	case MIB_VOICE_PROFILE__XCT_INTER_DIGIT_TIMER_LONG:
		pVoipConfig ->InterDigitTimerLong =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
	case MIB_VOICE_PROFILE__XCT_HANGING_REMINDER_TONE_TIMER:
		pVoipConfig ->HangingReminderToneTimer =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
		
	case MIB_VOICE_PROFILE__XCT_BUSY_TONE_TIMER:
		pVoipConfig ->BusyToneTimer =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
	case MIB_VOICE_PROFILE__XCT_NO_ANSWER_TIMER:
		pVoipConfig ->NoAnswerTimer =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
	case MIB_VOICE_PROFILE__XCT_DEFAULT_PROXY_INDEX:
			pVoipConfig ->ports[nPort].default_proxy =  *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
			break;	

	case MIB_VOICE_PROFILE__RTP__VLANIDMARK:
	case MIB_VOICE_PROFILE__SIP__VLANIDMARK:
    	if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){	
			pVoipConfig ->wanVlanIdVoice =  *( ( int * )pData );
			bDirtyBit = TRUE;
        }
		break;
	
	case MIB_VOICE_PROFILE__SIP__DSCPMARK:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			pVoipConfig ->sipDscp =  *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
        }
		break;
	
    case MIB_VOICE_PROFILE__SIP__REGISTERRETRYINTERVAL:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy1->RegisterRetryInterval = *( ( unsigned int * )pData );
            //	proxy2->RegisterRetryInterval = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
        }
		break;
	
    case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTERRETRYINTERVAL:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy2->RegisterRetryInterval = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
        }
		break;
		
    case MIB_VOICE_PROFILE__SIP__REGISTEREXPIRES:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
            /* main and backup proxy use the same value */
			proxy1->reg_expire = *( ( unsigned int * )pData );
            //	proxy2->reg_expire = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
        }
		break;
		

    case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTEREXPIRES:
        if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
            /* main and backup proxy use the same value */
			proxy2->reg_expire = *( ( unsigned int * )pData );
            //	proxy2->reg_expire = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
        }
		break;
		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER:		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER:
		/* backup proxy server */
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy2->addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;
	case MIB_VOICE_PROFILE__SIP__XCT_STB_PROXY_SERVER_PORT:		
	case MIB_VOICE_PROFILE__SIP__XCT_STB_REGISTRAR_SERVER_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy2->port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;		


	case MIB_VOICE_PROFILE__SIP__XCT_STB_OUTBOUND_PROXY_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy2->outbound_port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__RTP__XCT_STB_802_1PMARK:	
	case MIB_VOICE_PROFILE__SIP__XCT_STB_802_1PMARK:
		/* Need check */
		pVoipConfig ->wanVlanPriorityVoice =  *( ( int * )pData );
		bDirtyBit = TRUE;
		break;
		
    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATSWITCH:
		/* main and backup proxy use the same value  , for OPTION message */
		if(*( ( int * )pData ) == 1){
			proxy1->enable|=PROXY_OPTIONS_PING;
//			proxy2->enable|=PROXY_OPTIONS_PING;
		}else{
			proxy1->enable &= ~PROXY_OPTIONS_PING;
            //		proxy2->enable &= ~PROXY_OPTIONS_PING;
		}

		bDirtyBit = TRUE;
		break;

    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCYCLE:
		/* main and backup proxy use the same value */
		proxy1->HeartbeatCycle = *( ( int * )pData );
        //	proxy2->HeartbeatCycle = *( ( int * )pData );
		bDirtyBit = TRUE;
		break;
		
    case MIB_VOICE_PROFILE__SIP__XCT_HEARTBEATCOUNT:
		/* main and backup proxy use the same value */
		proxy1->HeartbeatCount = *( ( int * )pData );
        //	proxy2->HeartbeatCount = *( ( int * )pData );
		bDirtyBit = TRUE;
		break;
		
    case MIB_VOICE_PROFILE__SIP__INVITEEXPIRES:
		/* main and backup proxy use the same value */
		proxy1->SessionUpdateTimer = *( ( int * )pData );
		bDirtyBit = TRUE;
		break;

    case MIB_VOICE_PROFILE__SIP__XCT_INVITEEXPIRES:
		/* main and backup proxy use the same value */
		proxy2->SessionUpdateTimer = *( ( int * )pData );
		bDirtyBit = TRUE;
		break;
		

	case MIB_VOICE_PROFILE__RTP__DSCPMARK:
		pVoipConfig ->rtpDscp =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;

	case MIB_VOICE_PROFILE__FAXT38__ENABLE:
		pVoipConfig ->ports[nPort].useT38 =  *( ( boolean * )pData );
		bDirtyBit = TRUE;
		break;

    case MIB_PHYINTERFACE__INTERFACEID:
        //FIXME, Alex
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;
        
    case MIB_PHYINTERFACE__PHYPORT:
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;
#if 1//e8c project
    case MIB_PHYINTERFACE__TESTS__TESTSTATE:

		if(strcmp(( char * )pData ,"Complete")==0){
			pEvtMsg.e8c_autotest.TestStatus=2;
		}else if (strcmp(( char * )pData ,"Requested")==0){
			pEvtMsg.e8c_autotest.TestStatus=1;
			pEvtMsg.e8c_autotest.enable=1;
		}else{
			pEvtMsg.e8c_autotest.TestStatus=0; /*None*/
		}
		break;
	
    case MIB_PHYINTERFACE__TESTS__TESTSELECTOR:

		if(strcmp(( char * )pData ,"PhoneConnectivityTest")==0){
			pEvtMsg.e8c_autotest.TestSelector=0;
		}else if (strcmp(( char * )pData ,"X_SimulateTest")==0){
			pEvtMsg.e8c_autotest.TestSelector=1;
		}
		break;
		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__TESTTYPE:
		if(strcmp(( char * )pData,"Caller")==0){

			pEvtMsg.e8c_autotest.TestType=0;

		}else if(strcmp(( char * )pData,"Called")==0){
		/* called must enable */
			pEvtMsg.e8c_autotest.enable=1;
			pEvtMsg.e8c_autotest.TestType=1;
		}else{/*None*/
			pEvtMsg.e8c_autotest.enable=0;
		}
		break;

		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLEDNUMBER:
		strcpy( pEvtMsg.e8c_autotest.callednumber,( char * )pData);
		break;
		
		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMENABLE:
		pEvtMsg.e8c_autotest.DailDTMFConfirmEnable=*( ( unsigned int * )pData );
		break;


		
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMNUMBER:
		strcpy( pEvtMsg.e8c_autotest.DailDTMFConfirmNumber,( char * )pData);
		break;
#endif
    case MIB_VOICE_PROFILE__NUMBERINGPLAN__XCT_NUMBERPLAN:
		/* ebc board not need support fxo */
    case MIB_VOICE_PROFILE__XCT_G711FAX__ENABLE:
    case MIB_VOICE_PROFILE__XCT_G711FAX__CONTROLTYPE:
   		/* not have this configure, always support g.711 fax */
	case MIB_VOICE_PROFILE__XCT_IADDIAG__IADDIAGNOSTICSSTATE:
	case MIB_VOICE_PROFILE__XCT_IADDIAG__TESTSERVER:
	case MIB_VOICE_PROFILE__XCT_IADDIAG__REGISTRESULT:
	case MIB_VOICE_PROFILE__XCT_IADDIAG__REASON:
	case MIB_VOICE_PROFILE__ENABLE:
	case MIB_VOICE_PROFILE__RESET:
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
	case MIB_VOICE_PROFILE__NAME:
	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
    case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
    case MIB_PHYINTERFACE__TESTS__PHONECONNECTIVITY:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__DAILDTMFCONFIRMRESULT:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__STATUS:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CONCLUSION:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLERFAILREASON:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__CALLEDFAILREASON:
    case MIB_PHYINTERFACE__TESTS__XCT_SIMULATETEST__FAILEDRESPONSECODE:
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		// unable to set, //Read Only, we might needn't to write
		break;
        
	default:
		return 0;	/* range error */
		break;
	}

	if(TRUE == bDirtyBit){
//        voip_flash_set(pVoipConfig); //move to cwmp-client


        apply_add( CWMP_PRI_N, apply_voip, CWMP_RESTART, 0, NULL, 0 ); 
    }
	return 0;
}

int mib_get_type2( idMib_t idMib, unsigned int nInstNum1, 
				   unsigned int nInstNum2,
				   void *pData )
{
	/* VoiceService.1.VoiceProfile.{ i }.Line.{i}. */
    /* VoiceService.1.PhyInterface.{ i }.X_Stats.PoorQualityList.{ i }. */

	//voipCfgParam_t voipConfig;
	//voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy1;
	voipCfgProxy_t *proxy2;
	int nPort = 0;
	char sipURI[100];

    if( pVoipConfig == NULL && getVoipFlash()!=0 ) {    //Alex, Modified, 110913
        return 0;
    }

	if( IS_VOICE_PROFILE(idMib,1) && (nInstNum1 >= MAX_PROFILE_COUNT || nInstNum2 >= MAX_LINE_PER_PROFILE ) ){
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong Voice Profile or Line InstNum \n", __FUNCTION__, __LINE__));
		return 0;
    } else if( IS_PHYINTERFACE(idMib,1) && (nInstNum1 >= MAX_PHYINTERFACE_COUNT || nInstNum2 >= MAX_POORQUALITYLIST_COUNT)) {
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong PhyInterface or PoorQualityList InstNum \n", __FUNCTION__, __LINE__));
		return 0;
	}
	
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy1 = &pVoipConfig ->ports[nPort].proxies[0];
	proxy2 = &pVoipConfig ->ports[nPort].proxies[1];

	getSIPURI(sipURI, proxy1->login_id, proxy1->domain_name);

   	CWMPDBG( 3, ( stderr, "<%s:%d> DEBUG: getting value with InstNum1 %d InstNum2 %d idMib %d \n",
		__FUNCTION__, __LINE__, nInstNum1, nInstNum2, idMib) );
    
	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__ENABLE:
		if(proxy1->enable & PROXY_ENABLED){
			*( ( enable_t * )pData ) = TR104_ENABLE;
		}else{
			*( ( enable_t * )pData ) = TR104_DISABLE;
		}
		break;

	case MIB_VOICE_PROFILE__LINE__XCT_ENABLE:
		if(proxy2->enable & PROXY_ENABLED){
			*( ( enable_t * )pData ) = TR104_ENABLE;
		}else{
			*( ( enable_t * )pData ) = TR104_DISABLE;
		}
		break;

	case MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER:
		strcpy( pData, proxy1->number);
		break;

	case MIB_VOICE_PROFILE__LINE__XCT_DIRECTORY_NUMBER:
		strcpy( pData, proxy2->number);
		break;		

	case MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLERIDNAME:
		strcpy( pData, proxy1->display_name);
		break;
		
	case MIB_VOICE_PROFILE__LINE__XCT_CALLING_FEATURES_CALLERIDNAME:
		strcpy( pData, proxy2->display_name);
		break;		

	case MIB_VOICE_PROFILE__LINE__STATUS:
	{
		//TBD:run time update 
//		*( ( line_status_t * )pData ) = UP;
		//*( ( line_status_t * )pData ) = pEvtMsg.voiceProfileLineStatusMsg[nPort].LineStatus;
		FILE *fh;
		char buf[MAX_VOIP_PORTS * MAX_PROXY];

		if ((proxy1->enable & PROXY_ENABLED) == 0) {
			*( ( line_status_t * )pData )=DISABLED;
			printf("proxy 0 disable\n");
			break;
		}

		fh = fopen(_PATH_TMP_STATUS, "r");
		if (!fh) {
			printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
			printf("\nerrno=%d\n", errno);
			*( ( line_status_t * )pData )=ERROR;
			break;
		}

		memset(buf, 0, sizeof(buf));
		if (fread(buf, sizeof(buf), 1, fh) == 0) {
			printf("tr104: _PATH_TMP_STATUS The content of /tmp/status is NULL!!\n");
			printf("\nerrno=%d\n", errno);
			*( ( line_status_t * )pData )=ERROR;
			break;
		}
		else {
			fprintf(stderr, "buf is %s.\n", buf);
			switch (buf[nPort * MAX_PROXY ]) {//proxy 1 status
				case '0'://Not Registered
					*( ( line_status_t * )pData )=INITIALIZING;
					break;
				case '1'://Registered
					*( ( line_status_t * )pData )=UP;
		break;
				case '2'://Registering
					*( ( line_status_t * )pData )=REGISTERING;
					break;
				case '5'://Register Fail
					*( ( line_status_t * )pData )=REGISTERING;
					break;
				case '3'://Register Fail
					*( ( line_status_t * )pData )=REGISTERING;
					break;
				case '4'://VoIP Restart...
					*( ( line_status_t * )pData )=INITIALIZING;
					break;
				default://error
					*( ( line_status_t * )pData )=ERROR;
					break;
			}
		}

		fclose(fh);
		break;
	}

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( (char *)pData, proxy1->login_id);
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_USER_NAME:
		strcpy( (char *)pData, proxy2->login_id);
		break;
		

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
//		strcpy( (char *)pData, proxy1->password);
        strcpy( (char *)pData, "");//password show empty
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_PASSWORD:
//		strcpy( (char *)pData, proxy2->password);
        strcpy( (char *)pData, "");//password show empty
		break;		

	case MIB_VOICE_PROFILE__LINE__SIP__URI:
		memset(sipURI, 0, sizeof(sipURI));
		getSIPURI(sipURI, proxy1->login_id, proxy1->domain_name);
		strcpy( (char *)pData, sipURI );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__XCT_URI:
		memset(sipURI, 0, sizeof(sipURI));
		getSIPURI(sipURI, proxy2->login_id, proxy2->domain_name);
		strcpy( (char *)pData, sipURI );
		break;

	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_FAILED:
    case MIB_VOICE_PROFILE__LINE__STATS__TOTAL_CALL_TIME:
    case MIB_VOICE_PROFILE__LINE__STATS__RESET_STATISTICS:
        //Those get from linphone
        *( ( unsigned int * )pData )=0;
		break;

	case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__TRANSMIT_GAIN:
		 *( ( unsigned int * )pData )=pVoipConfig ->ports[nPort].spk_voice_gain;
		break;

		
    case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__RECEIVE_GAIN:
		*( ( unsigned int * )pData )=pVoipConfig ->ports[nPort].mic_voice_gain;
		   break;


		
    case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_ENABLE:
	case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_INUSE:

		*( ( unsigned int * )pData )=pVoipConfig ->ports[nPort].lec;
		break;

		
	case MIB_VOICE_PROFILE__LINE__XCT_IMS__HOTLINEURI:/* run time update , get from call mgnt */
			strcpy((char *)pData,pVoipConfig ->ports[nPort].hotline_number);
			break;

    case MIB_VOICE_PROFILE__LINE__XCT_IMS__DIALTONEPATTERN: /* run time update , get from call mgnt */
			if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_SPECIAL_DIAL_TONE)
				*( ( unsigned int * )pData )=1; /* must play special dial tone */
			else
				*( ( unsigned int * )pData )=0;
			break;


		
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__MCIDSERVICE:/* run time update , get from call mgnt */
		if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_MCID_ACTIVATE)
				*( ( unsigned int * )pData )=0; /* provisioned , enable */
			else
				*( ( unsigned int * )pData )=1; /*  withdrawn , disable */
			break;


	
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__NODIALINGBEHAVIOUR:/* run time update , get from call mgnt */

		if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_IMMEDIATE_CALLSETUP){
			*( ( unsigned int * )pData )=1; /* hotline */
		}else if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_DEFERE_CALLSETUP){
			*( ( unsigned int * )pData )=2; /* delay - hotline */
		}else{
			*( ( unsigned int * )pData )=0; /*rejectcall*/
		}
	
		break;
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__HOLDSERVICE:/* run time update , get from call mgnt */

		if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_HOLD_SERVICE_ACTIVATE)
				*( ( unsigned int * )pData )=0; /* provisioned , enable */
			else
				*( ( unsigned int * )pData )=1; /*  withdrawn , disable */
			break;
		
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__THREEPARTYSERVICE:/* run time update , get from call mgnt */

	if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_CONFERENCE_ACTIVATE)
			*( ( unsigned int * )pData )=0; /* provisioned , enable */
		else
			*( ( unsigned int * )pData )=1; /*  withdrawn , disable */
		break;

		
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__CONFERENCEURI:/* run time update , get from call mgnt */
		/* TBD? */
		break;
		
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__CWSERVICE:/* run time update , get from call mgnt */
		if(pEvtMsg.voiceProfileLineStatusMsg[nPort].featureFlags & IMS_CALL_WAITING_ACTIVATE)
				*( ( unsigned int * )pData )=0; /* provisioned , enable */
			else
				*( ( unsigned int * )pData )=1; /*  withdrawn , disable */
			break;
	

    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__STATTIME:	
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__TXPACKETS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__RXPACKETS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MEANDELAY:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MEANJITTER:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FRACTIONLOSS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__LOCALIPADDRESS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__LOCALUDPPORT:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FARENDIPADDRESS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FARENDUDPPORT:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MOSLQ:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__CODEC:
        //FIXME, Alex, Get
        break;
	case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_TAIL:

		*( ( unsigned int * )pData )=pVoipConfig ->ports[nPort].echoTail;
		break;	

	case MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLWAITING_ENABLE:

		*( ( unsigned int * )pData )=pVoipConfig ->ports[nPort].call_waiting_enable;
		break;

		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type2( idMib_t idMib, unsigned int nInstNum1, 
				   unsigned int nInstNum2,
				   void *pData )
{
	/* VoiceService.1.VoiceProfile.{ i }.Line.{i}. */
	//voipCfgParam_t voipConfig;
	//voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy1;
	voipCfgProxy_t *proxy2;
	int nPort=0;
	boolean bDirtyBit=FALSE;

	if( pVoipConfig == NULL && getVoipFlash()!=0 ) {	//Alex, Modified, 110913
		return 0;
	}       
    
	if( IS_VOICE_PROFILE(idMib,1) && (nInstNum1 >= MAX_PROFILE_COUNT || nInstNum2 >= MAX_LINE_PER_PROFILE ) ){
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong Voice Profile or Line InstNum \n", __FUNCTION__, __LINE__));
		return 0;
    } else if( IS_PHYINTERFACE(idMib,1) && (nInstNum1 >= MAX_PHYINTERFACE_COUNT || nInstNum2 >= MAX_POORQUALITYLIST_COUNT)) {
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong PhyInterface or PoorQualityList InstNum \n", __FUNCTION__, __LINE__));
		return 0;
	}
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy1 = &pVoipConfig ->ports[nPort].proxies[0];
	proxy2 = &pVoipConfig ->ports[nPort].proxies[1];


    CWMPDBG( 3, ( stderr, "<%s:%d> DEBUG: setting value with InstNum1 %d InstNum2 %d idMib %d \n",
            __FUNCTION__, __LINE__, nInstNum1, nInstNum2, idMib) );

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__DIRECTORY_NUMBER:
		strcpy( proxy1->number, pData );
		bDirtyBit=TRUE;
		break;
	case MIB_VOICE_PROFILE__LINE__XCT_DIRECTORY_NUMBER:
		strcpy( proxy2->number, pData );
		bDirtyBit=TRUE;
		break;
	case MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLERIDNAME:
		strcpy( proxy1->display_name, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__XCT_CALLING_FEATURES_CALLERIDNAME:
		strcpy( proxy2->display_name, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( proxy1->login_id, pData );
		bDirtyBit=TRUE;
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_USER_NAME:
		strcpy( proxy2->login_id, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
		strcpy( proxy1->password, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__XCT_AUTH_PASSWORD:
		strcpy( proxy2->password, pData );
		bDirtyBit=TRUE;
		break;
			

	case MIB_VOICE_PROFILE__LINE__ENABLE:
		if(*( ( enable_t * )pData ) == TR104_DISABLE){
			proxy1->enable &= ~PROXY_ENABLED;
		}else{
			proxy1->enable |= PROXY_ENABLED;
		}
        bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__XCT_ENABLE:
		if(*( ( enable_t * )pData ) == TR104_DISABLE){
			proxy2->enable &= ~PROXY_ENABLED;
		}else{
			proxy2->enable |= PROXY_ENABLED;
		}
		bDirtyBit=TRUE;
		break;
			

    case MIB_VOICE_PROFILE__LINE__STATS__RESET_STATISTICS:
		pEvtMsg.voiceProfileLineStatusMsg[nPort].resetStatistics = *( ( unsigned int * )pData );
		break;
	case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__TRANSMIT_GAIN:
		pVoipConfig ->ports[nPort].spk_voice_gain =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
		
    case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__RECEIVE_GAIN:
		pVoipConfig ->ports[nPort].mic_voice_gain =  *( ( unsigned int * )pData );
		bDirtyBit = TRUE;
		break;
		
    case MIB_VOICE_PROFILE__LINE__VOIP_PROCESSING__ECHO_CANCELLATION_ENABLE:
		pVoipConfig ->ports[nPort].lec= *( ( unsigned int * )pData );
        bDirtyBit = TRUE;
		break;
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__HOTLINEURI:
		strcpy(pVoipConfig ->ports[nPort].hotline_number, ( char * )pData);
        bDirtyBit = TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__STATUS:		
   	case MIB_VOICE_PROFILE__LINE__SIP__URI:
	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__STATS__OUTGOING_CALLS_FAILED:
    case MIB_VOICE_PROFILE__LINE__STATS__TOTAL_CALL_TIME:    
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__STATTIME:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__TXPACKETS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__RXPACKETS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MEANDELAY:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MEANJITTER:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FRACTIONLOSS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__LOCALIPADDRESS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__LOCALUDPPORT:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FARENDIPADDRESS:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__FARENDUDPPORT:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__MOSLQ:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__DIALTONEPATTERN:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__MCIDSERVICE:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__NODIALINGBEHAVIOUR:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__HOLDSERVICE:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__THREEPARTYSERVICE:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__CONFERENCEURI:
    case MIB_VOICE_PROFILE__LINE__XCT_IMS__CWSERVICE:
    case MIB_PHYINTERFACE__XCT_STATUS__POORQUALITYLIST__CODEC:        
        // unable to set	//Read Only, we might needn't to write
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;
    case MIB_VOICE_PROFILE__LINE__CALLING_FEATURES_CALLWAITING_ENABLE:
		pVoipConfig ->ports[nPort].call_waiting_enable= *( ( unsigned int * )pData );
        bDirtyBit = TRUE;
		break;	
	default:
		return 0;	/* range error */
		break;
	}

	if(TRUE == bDirtyBit){
		//voip_flash_set(pVoipConfig);//move to cwmp-client

        apply_add( CWMP_PRI_N, apply_voip, CWMP_RESTART, 0, NULL, 0 );
    }
	return 0;

}

int mib_get_type3( idMib_t idMib, unsigned int nInstNum1, 
				   unsigned int nInstNum2, unsigned int nInstNum3,
				   void *pData )
{
	/* VoiceService.1.VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	//voipCfgParam_t voipConfig;
	//voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	int nPort=0;

    if( pVoipConfig == NULL && getVoipFlash()!=0 ) {    //Alex, Modified, 110913
        return 0;
    }

	if( ( nInstNum1 >= MAX_PROFILE_COUNT ) ||
		( nInstNum2 >= MAX_LINE_PER_PROFILE ) ||
		( nInstNum3 >= MAX_CODEC_LIST ) )
	{
   		CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Wrong InstNum \n", __FUNCTION__, __LINE__));
		return 0;
	}
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif

    CWMPDBG( 3, ( stderr, "<%s:%d> DEBUG: getting value with InstNum1 %d InstNum2 %d InstNum3 %d idMib %d \n",
                __FUNCTION__, __LINE__, nInstNum1, nInstNum2, nInstNum3, idMib) );

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:
//		strcpy( pData, lstCodecs[nInstNum3].pszPacketizationPeriod );
	{
		int codecIndex=0;
        char packetizationPeriod[64];
        codecIndex = getCodecIndex(lstCodecs[nInstNum3].pszCodec, lstCodecs[nInstNum3].nBitRate);
        if(-1 != codecIndex){
             /* transform to 10 base*/
             //printf(" Debug: Codec Index: %d\n", codecIndex);
             sprintf(packetizationPeriod, "%d", 10 * (pVoipConfig ->ports[nPort].frame_size[codecIndex] + 1));
             //printf(" Debug: Packetization Period: %s\n", packetizationPeriod);
             strcpy( (char *) pData, packetizationPeriod );
        }
	}
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION:
		(*( boolean*)pData) = pVoipConfig ->ports[nPort].vad;
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
	{
		int codecIndex=0;
		codecIndex = getCodecIndex(lstCodecs[nInstNum3].pszCodec, lstCodecs[nInstNum3].nBitRate);
		if(-1 != codecIndex)
            /* transform to 1 base*/
			*( ( unsigned int * )pData ) = 1 + pVoipConfig ->ports[nPort].precedence[codecIndex];
	}
		break;
		
	default:
		return 0;	/* range error */
		break;
	}
	return 1;
}

int mib_set_type3( idMib_t idMib, unsigned int nInstNum1, 
				   unsigned int nInstNum2, unsigned int nInstNum3,
				   void *pData )
{
	/* VoiceService.1.VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	//voipCfgParam_t voipConfig;
	//voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	int nPort=0;
	boolean bDirtyBit=FALSE;

	if( pVoipConfig == NULL && getVoipFlash()!=0 ) {	//Alex, Modified, 110913
		return 0;
	}    
    
	nPort = nInstNum1;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
    CWMPDBG( 3, ( stderr, "<%s:%d> DEBUG: getting value with InstNum1 %d InstNum2 %d InstNum3 %d idMib %d \n",
                __FUNCTION__, __LINE__, nInstNum1, nInstNum2, nInstNum3, idMib) );

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is read-only or not supported to configure now. \n", __FUNCTION__, __LINE__));
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:

    {
		int codecIndex=0;
        codecIndex = getCodecIndex(lstCodecs[nInstNum3].pszCodec, lstCodecs[nInstNum3].nBitRate);
   		if(-1 != codecIndex){
            int frame_size = 0;
            /* "10, 20, 30" ¡A"5-40" ¡A"5-10, 20, 30" not supported now */
			frame_size = atoi((char *)pData);

            /* The unit is 10ms now */
            if(frame_size > 0 && frame_size <= 60 && (frame_size % 10)==0 ){
                //printf("Debug: Setting %d to codec index %d\n", frame_size, codecIndex);
	            pVoipConfig ->ports[nPort].frame_size[codecIndex] = ((frame_size/10) - 1);
				bDirtyBit=TRUE;
			}            
        }        
    }
        break;

	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION: 
		//pVoipConfig ->ports[nPort].vad = (*( boolean*)pData);
        //bDirtyBit = TRUE;
        CWMPDBG( 0, ( stderr, "<%s:%d>Warning: Parameter is not supported to configure now. \n", __FUNCTION__, __LINE__));
	    break;
        
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
	{
		int codecIndex=0, swapIndex=0, i;
	
		codecIndex = getCodecIndex(lstCodecs[nInstNum3].pszCodec, lstCodecs[nInstNum3].nBitRate);
		if(-1 != codecIndex){
			for(i=_CODEC_G711U; i <  _CODEC_MAX; i++){
				if(pVoipConfig ->ports[nPort].precedence[i]==(*( unsigned int * )pData)-1){             /* transform to 0 base*/
					swapIndex = i;
					break;
				}
			}
			pVoipConfig ->ports[nPort].precedence[swapIndex] = pVoipConfig ->ports[nPort].precedence[codecIndex];
            /* transform to 0 base*/
			pVoipConfig ->ports[nPort].precedence[codecIndex]=(* ( unsigned int * )pData )-1;
			bDirtyBit=TRUE;
		}
	}
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	if(TRUE == bDirtyBit){
		//voip_flash_set(pVoipConfig);//move to cwmp-client
	
        apply_add( CWMP_PRI_N, apply_voip, CWMP_RESTART, 0, NULL, 0 );
    }
	return 0;
}


