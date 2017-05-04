#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

//#define SUPPORT_RFC2833

/*
 *  RTP sample code.
 *  Send RTP and RTCP to specific IP address and port number when 
 *  off-hook handset. Press flash-hook to hold/resume RTP. 
 *  User hears a tone ,sounds like du du du, when the call is held.
 *  ATA sends RFC 2833 packets when user press phone digit key.
 *  Please killall solar_monitor before running this sample code.
 *
 */
#define SELECT_WAIT_SEC		5

int process_event(int chid , VoipEventID id , int argc, char *argv[]);
char * get_evt_name(VoipEventID evt);

int ActiveSession;
int bStartRTP;

int main(int argc, char *argv[])
{
	int chid = 0;
	int i;
	int ret;	
	fd_set fdset;	
	//VoipEventID dsp_ent;

	struct timeval timeout;
	//TstFlushVoipEvent stFlushVoipEvent;
	TstVoipEvent stVoipEvent;
	//SIGNSTATE val;

	if (argc != 6)
	{
		printf("usage: %s chid src_ip6 dest_ip6 src_port dest_port\n", argv[0]);
		//printf("example: t38_get 0 fe80::2e0:4cff:fe86:8502 fe80::2e0:4cff:fe86:7005 9010 9000");
		//printf("example: t38_get 0 192.168.1.1 192.168.1.7 46224 50002\n");
		//printf("example: t38_get 0 192.168.1.1 192.168.1.98 46224 50002\n");
		printf("example: t38_get 0 192.168.1.1 192.168.1.98 50002 46224 & \n");
		return 0;
	}

	// init channel
	chid = atoi(argv[1]);

	// init dsp
	rtk_InitDsp(chid);

#ifdef SUPPORT_RFC2833
	rtk_SetDtmfMode(chid, DTMF_RFC2833);	// set dtmf mode
#else
	rtk_SetDtmfMode(chid, DTMF_INBAND);	// set dtmf mode
#endif

	// init phone
	rtk_SetVoiceGain(chid, 0, 0);	// 0db
	rtk_SetEchoTail(chid, 32, 1, 5, 5, 0);			// 4ms, NLP on

	rtk_SetFlashHookTime(chid, 100, 300);		// 100~300 ms
	rtk_SetDtmfCidParam(chid, DTMF_DIGIT_A, DTMF_DIGIT_C, 0xf, 80, 80, 300, 300);	

	// set dtmf mode
	rtk_SetDtmfMode(chid, DTMF_INBAND);

	// set fax modem det mode
	rtk_SetFaxModemDet(chid, 3); //3:auto_2

	// set fax event detection
	rtk_SetAnswerToneDet(chid , 0x3f1212 , 0 , 10);

	// flush kernel fifo before app run
	rtk_SetFlushFifo(chid);

	// init flag
	bStartRTP = 0;
	ActiveSession = 0;

	while (1)
	{

		// set fd for select 
		FD_ZERO( &fdset );
		FD_SET( g_VoIP_Mgr_FD, &fdset );
		
		// set timeout for select
		timeout.tv_sec = SELECT_WAIT_SEC;
		timeout.tv_usec = 0;
		
		// select 
		ret = select( g_VoIP_Mgr_FD + 1, &fdset, NULL, NULL, &timeout );
		
		if( ret == 0 ) {
			//printf( "timeout\n" );
			continue;
		} 		
		
		// read all events
		for ( i = 0 ; ; i ++ )
		{
			memset ((void*)&stVoipEvent, 0, sizeof(TstVoipEvent));
			stVoipEvent.ch_id = chid;
			stVoipEvent.type = VET_ALL;
			stVoipEvent.mask = VEM_ALL;
			
			rtk_GetVoIPEvent( &stVoipEvent );
			
			// no more data 
			if( stVoipEvent.id == VEID_NONE )
				break;
				
			printf( "[%d] on chid=%d\n", i, stVoipEvent.ch_id );
			printf( "\ttype=%08X mask=%08X\n", stVoipEvent.type, stVoipEvent.mask );
			printf( "\tID=%s(%08X), p0=%u p1=%u @%08X\n", get_evt_name(stVoipEvent.id), stVoipEvent.id, 
					stVoipEvent.p0, stVoipEvent.p1, stVoipEvent.time );
				
			
			process_event( stVoipEvent.ch_id , stVoipEvent.id , argc , argv );
		}
	}

	return 0;
}

int process_event(int chid , VoipEventID id , int argc, char *argv[])
{
	t38udp_config_t t38udp_config;
	t38_payloadtype_config_t t38_config;
	

	printf("%s(%d) id=0x%08x\n", __FUNCTION__ , __LINE__ , id);	
	switch (id)
	{
	case VEID_DTMF_0:
	case VEID_DTMF_1:
	case VEID_DTMF_2:
	case VEID_DTMF_3:
	case VEID_DTMF_4:
	case VEID_DTMF_5:
	case VEID_DTMF_6:
	case VEID_DTMF_7:
	case VEID_DTMF_8:
	case VEID_DTMF_9:		
		break;

	case VEID_HOOK_PHONE_OFF_HOOK:
		// call rtk_OffHookAction at first
		rtk_OffHookAction(chid);
		ActiveSession = 0;
		rtk_SetTranSessionID(chid, ActiveSession);
		
		// set rtp session
		memset(&t38udp_config, 0, sizeof(t38udp_config));
		t38udp_config.chid = chid;
		t38udp_config.sid = ActiveSession;
		t38udp_config.isTcp = 0;		// use udp

#if 1		
		t38udp_config.ipv6	= 0;
		
		t38udp_config.extIp = inet_addr(argv[2]);
		t38udp_config.remIp = inet_addr(argv[3]);
#else
		t38udp_config.ipv6	= 1;
		int s1, s2;
		char str1[INET6_ADDRSTRLEN], str2[INET6_ADDRSTRLEN];

		s1 = inet_pton(AF_INET6, argv[2], &t38udp_config.extIp6);
		if (s1 <= 0)
		{
           if (s1 == 0)
               fprintf(stderr, "src addr not in presentation format\n");
           else
               fprintf(stderr, "inet_pton error, line%d\n", __LINE__);

			return 0;
		}
		
		// show input IPv6 addr
		if (inet_ntop(AF_INET6, &t38udp_config.extIp6, str1, INET6_ADDRSTRLEN) == NULL)
		{
           fprintf(stderr,"inet_ntop error, line%d\n", __LINE__);
           return 0;
		}
		
		//printf("input src ipv6 addr: %s\n", str1);

		s2 = inet_pton(AF_INET6, argv[3], &t38udp_config.remIp6);
		if (s2 <= 0)
		{
           if (s2 == 0)
               fprintf(stderr, "dst addr not in presentation format\n");
           else
               fprintf(stderr, "inet_pton error, line%d\n", __LINE__);
		}
		
		// show input IPv6 addr
		if (inet_ntop(AF_INET6, &t38udp_config.remIp6, str2, INET6_ADDRSTRLEN) == NULL)
		{
           fprintf(stderr,"inet_ntop error, line%d\n", __LINE__);
           return 0;
		}
		
		//printf("input dst ipv6 addr: %s\n", str2);
#endif
		t38udp_config.extPort = htons(atoi(argv[4]));
		t38udp_config.remPort = htons(atoi(argv[5]));
	#ifdef SUPPORT_VOICE_QOS
		t38udp_config.tos = 0;
	#endif
		
		rtk_SetT38UdpConfig(&t38udp_config);
	
		// set rtp payload, and other session parameters.
		t38_config.chid = chid;
		t38_config.sid = 0;
		t38_config.bT38ParamEnable = 0;
		t38_config.nT38MaxBuffer = 0;
		t38_config.nT38RateMgt = 0;
		t38_config.nT38MaxRate = 0;
		t38_config.bT38EnableECM = 0;
		t38_config.nT38ECCSignal = 0;
		t38_config.nT38ECCData = 0;
		t38_config.bT38EnableSpoof = 0;
		t38_config.nT38DuplicateNum = 0;
		t38_config.nPcmMode = 2;	// narrow-band
		rtk_SetT38PayloadType(&t38_config);


		// start rtp (channel number = 0, session number = 0)
		rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
		bStartRTP = 1;

		//printf("%s:%d -> %s:%d\n", str1, atoi(argv[4]), str2, atoi(argv[5]));
		break;

	case VEID_HOOK_PHONE_ON_HOOK:
		// close rtp
		rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
		rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);
		bStartRTP = 0;

		// call rtk_OnHookAction at last
		rtk_OnHookAction(chid);
		break;

	case VEID_HOOK_PHONE_FLASH_HOOK:
		ActiveSession = !ActiveSession;
		rtk_SetTranSessionID(chid, ActiveSession);
		if (ActiveSession == 1)
		{
		    // Get RTP statistics
		    TstRtpRtcpStatistics pRtpRtcpSt;
			rtk_GetRtpRtcpStatistics(chid, ActiveSession, 0, &pRtpRtcpSt);

			printf("\n== RTP 0 Statics ========================\n");
			printf("Rx RTP bytes	= %ld\n",pRtpRtcpSt.nRxBytes);
			printf("Rx RTP packets	= %ld\n",pRtpRtcpSt.nRxPkts);
			printf("Rx packet loss	= %ld\n",pRtpRtcpSt.nLost);
			printf("Tx RTP bytes	= %ld\n",pRtpRtcpSt.nTxBytes);
			printf("Tx RTP packets 	= %ld\n",pRtpRtcpSt.nTxPkts);
			printf("=========================================\n");


			// hold session
			rtk_HoldRtp(chid, 0, 1);

			// play a tone. Sounds like "du du du".
			rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_CONFIRMATION, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		else
		{
			// resume session
			rtk_HoldRtp(chid, 0, 0);

			// stop tone
			rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		break;

	default:
		break;
	}

	return 0;	
}


char * get_evt_name(VoipEventID evt)
{

	switch (evt)
	{
		case VEID_MASK_ID: 
			return("VEID_MASK_ID"); 
		case VEID_MASK_TYPE: 
			return("VEID_MASK_TYPE"); 
		case VEID_NONE: 
			return("VEID_NONE");
		case VEID_DTMF_WILDCARD: 
			return("VEID_DTMF_WILDCARD"); 
		case VEID_DTMF_DIGIT_MASK: 
			return("VEID_DTMF_DIGIT_MASK");
		case VEID_DTMF_0: 
			return("VEID_DTMF_0");
		case VEID_DTMF_1: 
			return("VEID_DTMF_1");
		case VEID_DTMF_2: 
			return("VEID_DTMF_2");
		case VEID_DTMF_3: 
			return("VEID_DTMF_3");
		case VEID_DTMF_4: 
			return("VEID_DTMF_4");
		case VEID_DTMF_5: 
			return("VEID_DTMF_5");
		case VEID_DTMF_6: 
			return("VEID_DTMF_6");
		case VEID_DTMF_7: 
			return("VEID_DTMF_7");
		case VEID_DTMF_8: 
			return("VEID_DTMF_8");
		case VEID_DTMF_9: 
			return("VEID_DTMF_9");
		case VEID_DTMF_STAR: 
			return("VEID_DTMF_STAR");
		case VEID_DTMF_SHARP: 
			return("VEID_DTMF_SHARP");
		case VEID_DTMF_A: 
			return("VEID_DTMF_A");
		case VEID_DTMF_B: 
			return("VEID_DTMF_B");
		case VEID_DTMF_C: 
			return("VEID_DTMF_C");
		case VEID_DTMF_D: 
			return("VEID_DTMF_D");
		case VEID_DTMF_ENERGY: 
			return("VEID_DTMF_ENERGY");
		case VEID_HOOK_PHONE_ON_HOOK: 
			return("VEID_HOOK_PHONE_ON_HOOK");
		case VEID_HOOK_PHONE_OFF_HOOK: 
			return("VEID_HOOK_PHONE_OFF_HOOK");
		case VEID_HOOK_PHONE_FLASH_HOOK: 
			return("VEID_HOOK_PHONE_FLASH_HOOK");
		case VEID_HOOK_PHONE_STILL_ON_HOOK: 
			return("VEID_HOOK_PHONE_STILL_ON_HOOK");
		case VEID_HOOK_PHONE_STILL_OFF_HOOK: 
			return("VEID_HOOK_PHONE_STILL_OFF_HOOK");
		case VEID_HOOK_FXO_ON_HOOK: 
			return("VEID_HOOK_FXO_ON_HOOK");
		case VEID_HOOK_FXO_OFF_HOOK: 
			return("VEID_HOOK_FXO_OFF_HOOK");
		case VEID_HOOK_FXO_FLASH_HOOK: 
			return("VEID_HOOK_FXO_FLASH_HOOK");
		case VEID_HOOK_FXO_STILL_ON_HOOK: 
			return("VEID_HOOK_FXO_STILL_ON_HOOK");
		case VEID_HOOK_FXO_STILL_OFF_HOOK: 
			return("VEID_HOOK_FXO_STILL_OFF_HOOK");
		case VEID_HOOK_FXO_RING_ON: 
			return("VEID_HOOK_FXO_RING_ON");
		case VEID_HOOK_FXO_RING_OFF: 
			return("VEID_HOOK_FXO_RING_OFF");
		case VEID_HOOK_FXO_BUSY_TONE: 
			return("VEID_HOOK_FXO_BUSY_TONE");
		case VEID_HOOK_FXO_CALLER_ID: 
			return("VEID_HOOK_FXO_CALLER_ID");
		case VEID_HOOK_FXO_RING_TONE_ON: 
			return("VEID_HOOK_FXO_RING_TONE_ON");
		case VEID_HOOK_FXO_RING_TONE_OFF: 
			return("VEID_HOOK_FXO_RING_TONE_OFF");
		case VEID_HOOK_FXO_POLARITY_REVERSAL: 
			return("VEID_HOOK_FXO_POLARITY_REVERSAL");
		case VEID_HOOK_FXO_CURRENT_DROP: 
			return("VEID_HOOK_FXO_CURRENT_DROP");
		case VEID_HOOK_FXO_DIS_TONE: 
			return("VEID_HOOK_FXO_DIS_TONE");
		case VEID_HOOK_OUTBAND_FLASH_EVENT: 
			return("VEID_HOOK_OUTBAND_FLASH_EVENT");
		case VEID_ENERGY: 
			return("VEID_ENERGY");
		case VEID_DECT_BUTTON_PAGE: 
			return("VEID_DECT_BUTTON_PAGE");
		case VEID_DECT_BUTTON_REGISTRATION_MODE: 
			return("VEID_DECT_BUTTON_REGISTRATION_MODE");
		case VEID_DECT_BUTTON_DEL_HS: 
			return("VEID_DECT_BUTTON_DEL_HS");
		case VEID_DECT_BUTTON_NOT_DEFINED: 
			return("VEID_DECT_BUTTON_NOT_DEFINED");
		case VEID_FAXMDM_AUDIOCODES_FAX: 
			return("VEID_FAXMDM_AUDIOCODES_FAX");
		case VEID_FAXMDM_AUDIOCODES_MODEM: 
			return("VEID_FAXMDM_AUDIOCODES_MODEM");
		case VEID_FAXMDM_LEC_AUTO_RESTORE: 
			return("VEID_FAXMDM_LEC_AUTO_RESTORE");
		case VEID_FAXMDM_LEC_AUTO_ON: 
			return("VEID_FAXMDM_LEC_AUTO_ON");
		case VEID_FAXMDM_LEC_AUTO_OFF: 
			return("VEID_FAXMDM_LEC_AUTO_OFF");
		case VEID_FAXMDM_FAX_CED: 
			return("VEID_FAXMDM_FAX_CED");
		case VEID_FAXMDM_FAX_DIS_TX: 
			return("VEID_FAXMDM_FAX_DIS_TX");
		case VEID_FAXMDM_FAX_DIS_RX: 
			return("VEID_FAXMDM_FAX_DIS_RX");
		case VEID_FAXMDM_FAX_DCN_TX: 
			return("VEID_FAXMDM_FAX_DCN_TX");
		case VEID_FAXMDM_FAX_DCN_RX: 
			return("VEID_FAXMDM_FAX_DCN_RX");
		case VEID_FAXMDM_MODEM_LOCAL: 
			return("VEID_FAXMDM_MODEM_LOCAL");
		case VEID_FAXMDM_MODEM_LOCAL_DELAY: 
			return("VEID_FAXMDM_MODEM_LOCAL_DELAY");
		case VEID_FAXMDM_MODEM_REMOTE: 
			return("VEID_FAXMDM_MODEM_REMOTE");
		case VEID_FAXMDM_ANSTONE_CNG_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_CNG_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSAM_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSAM_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL");
		case VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELLANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELLANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_V22_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V22_LOCAL");
		case VEID_FAXMDM_ANSTONE_V8BIS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V8BIS_LOCAL");
		case VEID_FAXMDM_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_V21FLAG_LOCAL");
		case VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL");
		case VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL: 
			return("VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL");
		case VEID_FAXMDM_ANSTONE_OFF_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_OFF_LOCAL");
		case VEID_FAXMDM_ANSTONE_V21C2_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V21C2_LOCAL");
		case VEID_FAXMDM_ANSTONE_V21C1_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V21C1_LOCAL");
		case VEID_FAXMDM_ANSTONE_V23_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_V23_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELL202ANS_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELL202ANS_LOCAL");
		case VEID_FAXMDM_ANSTONE_BELL202CP_LOCAL: 
			return("VEID_FAXMDM_ANSTONE_BELL202CP_LOCAL");
		case VEID_FAXMDM_ANSTONE_CNG_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_CNG_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSAM_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSAM_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE");
		case VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELLANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELLANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_V22_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V22_REMOTE");
		case VEID_FAXMDM_ANSTONE_V8BIS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V8BIS_REMOTE");
		case VEID_FAXMDM_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_V21FLAG_REMOTE");
		case VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE");
		case VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE: 
			return("VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE");
		case VEID_FAXMDM_ANSTONE_OFF_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_OFF_REMOTE");
		case VEID_FAXMDM_ANSTONE_V21C2_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V21C2_REMOTE");
		case VEID_FAXMDM_ANSTONE_V21C1_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V21C1_REMOTE");
		case VEID_FAXMDM_ANSTONE_V23_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_V23_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELL202ANS_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELL202ANS_REMOTE");
		case VEID_FAXMDM_ANSTONE_BELL202CP_REMOTE: 
			return("VEID_FAXMDM_ANSTONE_BELL202CP_REMOTE");
		case VEID_FAXMDM_FAST_FAXTONE_LOCAL: 
			return("VEID_FAXMDM_FAST_FAXTONE_LOCAL");
		case VEID_FAXMDM_FAST_MODEMTONE_LOCAL: 
			return("VEID_FAXMDM_FAST_MODEMTONE_LOCAL");
		case VEID_FAXMDM_FAST_FAXTONE_REMOTE: 
			return("VEID_FAXMDM_FAST_FAXTONE_REMOTE");
		case VEID_FAXMDM_FAST_MODEMTONE_REMOTE: 
			return("VEID_FAXMDM_FAST_MODEMTONE_REMOTE");
		case VEID_FAXMDM_V152_RTP_VBD: 
			return("VEID_FAXMDM_V152_RTP_VBD");
		case VEID_FAXMDM_V152_RTP_AUDIO: 
			return("VEID_FAXMDM_V152_RTP_AUDIO");
		case VEID_FAXMDM_V152_SIG_CED: 
			return("VEID_FAXMDM_V152_SIG_CED");
		case VEID_FAXMDM_V152_TDM_SIG_END: 
			return("VEID_FAXMDM_V152_TDM_SIG_END");
		case VEID_FAXMDM_V152_BI_SILENCE: 
			return("VEID_FAXMDM_V152_BI_SILENCE");
		case VEID_FAXMDM_V150_SIG_MODEM: 
			return("VEID_FAXMDM_V150_SIG_MODEM");
		case VEID_FAXMDM_SILENCE_TDM: 
			return("VEID_FAXMDM_SILENCE_TDM");
		case VEID_FAXMDM_SILENCE_IP: 
			return("VEID_FAXMDM_SILENCE_IP");
		case VEID_FAXMDM_SILENCE_TDM_IP: 
			return("VEID_FAXMDM_SILENCE_TDM_IP");
		case VEID_RFC2833_RX_WILDCARD: 
			return("VEID_RFC2833_RX_WILDCARD");

		#if 0 // duplicate case value as VEID_DTMF_DIGIT_MASK
		case VEID_RFC2833_RX_MASK: 
			return("VEID_RFC2833_RX_MASK");
		#endif

		#if 0 // duplicate case value as VEID_RFC2833_RX_WILDCARD
		case VEID_RFC2833_RX_DTMF_0: 
			returreturn("VEID_RFC2833_RX_DTMF_0");
		#endif

		case VEID_RFC2833_RX_DTMF_1: 
			return("VEID_RFC2833_RX_DTMF_1");
		case VEID_RFC2833_RX_DTMF_2: 
			return("VEID_RFC2833_RX_DTMF_2");
		case VEID_RFC2833_RX_DTMF_3: 
			return("VEID_RFC2833_RX_DTMF_3");
		case VEID_RFC2833_RX_DTMF_4: 
			return("VEID_RFC2833_RX_DTMF_4");
		case VEID_RFC2833_RX_DTMF_5: 
			return("VEID_RFC2833_RX_DTMF_5");
		case VEID_RFC2833_RX_DTMF_6: 
			return("VEID_RFC2833_RX_DTMF_6");
		case VEID_RFC2833_RX_DTMF_7: 
			return("VEID_RFC2833_RX_DTMF_7");
		case VEID_RFC2833_RX_DTMF_8: 
			return("VEID_RFC2833_RX_DTMF_8");
		case VEID_RFC2833_RX_DTMF_9: 
			return("VEID_RFC2833_RX_DTMF_9");
		case VEID_RFC2833_RX_DTMF_STAR: 
			return("VEID_RFC2833_RX_DTMF_STAR");
		case VEID_RFC2833_RX_DTMF_POUND: 
			return("VEID_RFC2833_RX_DTMF_POUND");
		case VEID_RFC2833_RX_DTMF_A: 
			return("VEID_RFC2833_RX_DTMF_A");
		case VEID_RFC2833_RX_DTMF_B: 
			return("VEID_RFC2833_RX_DTMF_B");
		case VEID_RFC2833_RX_DTMF_C: 
			return("VEID_RFC2833_RX_DTMF_C");
		case VEID_RFC2833_RX_DTMF_D: 
			return("VEID_RFC2833_RX_DTMF_D");
		case VEID_RFC2833_RX_DTMF_FLASH: 
			return("VEID_RFC2833_RX_DTMF_FLASH");
		case VEID_RFC2833_RX_DATA_FAX_ANS: 
			return("VEID_RFC2833_RX_DATA_FAX_ANS");
		case VEID_RFC2833_RX_DATA_FAX_ANSBAR: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSBAR");
		case VEID_RFC2833_RX_DATA_FAX_ANSAM: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSAM");
		case VEID_RFC2833_RX_DATA_FAX_ANSAMBAR: 
			return("VEID_RFC2833_RX_DATA_FAX_ANSAMBAR");
		case VEID_RFC2833_RX_DATA_FAX_CNG: 
			return("VEID_RFC2833_RX_DATA_FAX_CNG");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0");
		case VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1: 
			return("VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1");
		case VEID_RFC2833_RX_DATA_FAX_CRDI: 
			return("VEID_RFC2833_RX_DATA_FAX_CRDI");
		case VEID_RFC2833_RX_DATA_FAX_CRDR: 
			return("VEID_RFC2833_RX_DATA_FAX_CRDR");
		case VEID_RFC2833_RX_DATA_FAX_CRE: 
			return("VEID_RFC2833_RX_DATA_FAX_CRE");
		case VEID_RFC2833_RX_DATA_FAX_ESI: 
			return("VEID_RFC2833_RX_DATA_FAX_ESI");
		case VEID_RFC2833_RX_DATA_FAX_ESR: 
			return("VEID_RFC2833_RX_DATA_FAX_ESR");
		case VEID_RFC2833_RX_DATA_FAX_MRDI: 
			return("VEID_RFC2833_RX_DATA_FAX_MRDI");
		case VEID_RFC2833_RX_DATA_FAX_MRDR: 
			return("VEID_RFC2833_RX_DATA_FAX_MRDR");
		case VEID_RFC2833_RX_DATA_FAX_MRE: 
			return("VEID_RFC2833_RX_DATA_FAX_MRE");
		case VEID_RFC2833_RX_DATA_FAX_CT: 
			return("VEID_RFC2833_RX_DATA_FAX_CT");
		case VEID_DSP_DTMF_CLID_END: 
			return("VEID_DSP_DTMF_CLID_END");
		case VEID_DSP_FSK_CLID_END: 
			return("VEID_DSP_FSK_CLID_END");
		case VEID_DSP_FSK_CLID_TYPE2_NO_ACK: 
			return("VEID_DSP_FSK_CLID_TYPE2_NO_ACK");
		case VEID_DSP_LOCAL_TONE_END: 
			return("VEID_DSP_LOCAL_TONE_END");
		case VEID_DSP_REMOTE_TONE_END: 
			return("VEID_DSP_REMOTE_TONE_END");
		case VEID_DSP_LOCAL_TONE_USER_STOP: 
			return("VEID_DSP_LOCAL_TONE_USER_STOP");
		case VEID_DSP_REMOTE_TONE_USER_STOP: 
			return("VEID_DSP_REMOTE_TONE_USER_STOP");
		case VEID_FAXMDM_T38_FAX_END:
			return("VEID_FAXMDM_T38_FAX_END");
		default:
			return("unknown");
	}

	return("unknown");

}