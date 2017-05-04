#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"
#include "voip_params.h"

/*
 *  Sample code of two RTP sessions.
 *  Send 2 RTP sreams to specific IP address and port number when 
 *  off-hook handset. The first RTP is send_receive and the 
 *  second one is send_only. 
 *  Remote peer user can press flash-hook to switch between two RTPs
 *  to decide listen sessions from me.
 *
 *  Near end can press digit key
 *
 *  #1 to play du du tone to sid 0
 *  #2 to play du du tone to sid 1
 *  #3 to play ivr to local  phone via sid 0
 *  #4 to play ivr to local  phone via sid 1
 *  #5 to play ivr to remote phone via sid 0
 *  #6 to play ivr to remote phone via sid 1
 *
 *  Far end user can swith btween two sessions to hear near end user action.	
 *  Please killall solar_monitor before running sample code.
 *
 */
#ifdef __ECOS
#include <commands.h>
shell_old_cmd( "rtp2", "", "", user_rtp_main );
#endif

#ifdef __ECOS
CMD_DECL( user_rtp_main )
#else
int main(int argc, char *argv[])
#endif
{
	SIGNSTATE val;
	rtp_config_t rtp_config;
	//rtcp_config_t rtcp_config;
	payloadtype_config_t codec_config;
	int ActiveSession;
	int bStartRTP;
	//int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid = 0;
#ifdef CONFIG_RTK_VOIP_IVR
	char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};
#endif
	if (argc != 6)
	{
		printf("usage: %s chid src_ip dest_ip src_port dest_port\n", argv[0]);
		return 0;
	}

	// init channel
	chid = atoi(argv[1]);

	// init dsp
	rtk_InitDsp(chid);

	rtk_SetDtmfMode(chid, DTMF_RFC2833);	// set dtmf mode

	// init flag
	bStartRTP = 0;
	ActiveSession = 0;

	while (1)
	{
		rtk_GetFxsEvent(chid, &val);
		switch (val)
		{
#ifdef CONFIG_RTK_VOIP_IVR
		case SIGN_KEY1:
				if (bStartRTP) {
					printf("play du du tone to chid:%d, sid:0 active session is %d\n",chid,ActiveSession);
					rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CONFIRMATION, 1, DSPCODEC_TONEDIRECTION_BOTH);
				}
				break;
		case SIGN_KEY2:
				if (bStartRTP) {
					printf("play du du tone to chid:%d, sid:1, active session is %d\n",chid,ActiveSession);
					rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_CONFIRMATION, 1, DSPCODEC_TONEDIRECTION_BOTH);
				}
				break;

		case SIGN_KEY3:
				if (bStartRTP) {
					printf("play local ivr to  chid:%d, sid:0, active session is %d\n",chid,ActiveSession);
					rtk_IvrStartPlaying(chid, 0, IVR_DIR_LOCAL, text);
				}
				break;

		case SIGN_KEY4:
				if (bStartRTP) {
					printf("play local ivr to  chid:%d, sid:1, active session is %d\n",chid,ActiveSession);
					rtk_IvrStartPlaying(chid, 1, IVR_DIR_LOCAL, text);
				}
				break;

		case SIGN_KEY5:
				if (bStartRTP) {
					printf("play remote ivr to chid:%d, sid:0, active session is %d\n",chid,ActiveSession);
					rtk_IvrStartPlaying(chid, 0, IVR_DIR_REMOTE, text);
				}
				break;

		case SIGN_KEY6:

				if (bStartRTP) {
					printf("play remote ivr to chid:%d, sid:1, active session is %d\n",chid,ActiveSession);
					rtk_IvrStartPlaying(chid, 1, IVR_DIR_REMOTE, text);
				}

				break;
#endif
		case SIGN_KEY7:
		case SIGN_KEY8:
		case SIGN_KEY9:
		case SIGN_KEY0:
		case SIGN_STAR:
		case SIGN_HASH:
			break;

		case SIGN_OFFHOOK:
			// call rtk_OffHookAction at first
			rtk_OffHookAction(chid);
			ActiveSession = 0;
			rtk_SetTranSessionID(chid, ActiveSession);

			// set RTP session
			memset(&rtp_config, 0, sizeof(rtp_config));
			rtp_config.chid 			= chid;					// use channel 
			rtp_config.sid 				= ActiveSession;
			rtp_config.isTcp 			= 0;					// use udp
			rtp_config.extIp 			= inet_addr(argv[2]);
			rtp_config.remIp 			= inet_addr(argv[3]);
			rtp_config.extPort 			= htons(atoi(argv[4]));
			rtp_config.remPort 			= htons(atoi(argv[5]));
#ifdef SUPPORT_VOICE_QOS
			rtp_config.tos 				= 0;
#endif
			rtp_config.rfc2833_payload_type_local  = 96;
			rtp_config.rfc2833_payload_type_remote = 96;
			rtp_config.rfc2833_fax_modem_pt_local = 101;
			rtp_config.rfc2833_fax_modem_pt_remote = 101;
#if defined(SUPPORT_RTP_REDUNDANT_APP)
			rtp_config.rtp_redundant_payload_type_local  = 0;
			rtp_config.rtp_redundant_payload_type_remote = 0;
			rtp_config.rtp_redundant_max_Audio = 0;
			rtp_config.rtp_redundant_max_RFC2833 = 0;
#elif defined(SUPPORT_RTP_REDUNDANT)
			rtp_config.rtp_redundant_payload_type_local  = 0;
			rtp_config.rtp_redundant_payload_type_remote = 0;
			rtp_config.rtp_redundant_max_Audio = 0;
			rtp_config.rtp_redundant_max_RFC2833 = 0;
#endif
			rtp_config.SID_payload_type_local  = 0;
			rtp_config.SID_payload_type_remote = 0;
			rtp_config.init_randomly = 1;
			rtp_config.init_seqno = 0;
			rtp_config.init_SSRC = 0;
			rtp_config.init_timestamp = 0;
			rtk_SetRtpConfig(&rtp_config);

			// set RTCP session
			//rtcp_config.chid	= rtp_config.chid;
			//rtcp_config.sid		= rtp_config.sid;
			//rtcp_config.remIp	= rtp_config.remIp;
			//rtcp_config.remPort	= rtp_config.remPort + 1;
			//rtcp_config.extIp	= rtp_config.extIp;
			//rtcp_config.extPort	= rtp_config.extPort + 1;
#ifdef SUPPORT_VOICE_QOS
			//rtcp_config.tos		= rtp_config.tos;
#endif
			
			//rtcp_config.txInterval = 500;	// send rtcp every 500ms
			//rtcp_config.enableXR = 1;
			
			//rtk_SetRtcpConfig(&rtcp_config);


			// set rtp payload, and other session parameters.
			codec_config.chid 				= chid;
			codec_config.sid 				= ActiveSession;
			codec_config.local_pt 			= 8;					// G729 use static pt 18
			codec_config.remote_pt 			= 8;					// G729 use static pt 18
			codec_config.uLocalPktFormat 		= rtpPayloadPCMA;
			codec_config.uRemotePktFormat 		= rtpPayloadPCMA;
			codec_config.local_pt_vbd 		= rtpPayloadUndefined;
			codec_config.remote_pt_vbd 		= rtpPayloadUndefined;
			codec_config.uPktFormat_vbd 	= rtpPayloadUndefined;
			codec_config.nG723Type 			= 0;
			codec_config.nLocalFramePerPacket 	= 1;
			codec_config.nRemoteFramePerPacket 	= 1;
			codec_config.nFramePerPacket_vbd = 1;
			codec_config.bVAD 				= 0;
			codec_config.bPLC 				= 1;
			codec_config.nJitterDelay 		= 4;
			codec_config.nMaxDelay 			= 13;
			codec_config.nMaxStrictDelay	= 10000;
			codec_config.nJitterFactor 		= 7;
			codec_config.nG726Packing 		= 2;					//pack right

			if ( (codec_config.uLocalPktFormat == rtpPayloadG722) || (codec_config.uRemotePktFormat == rtpPayloadG722) )
				codec_config.nPcmMode = 3;	// wide-band
			else
				codec_config.nPcmMode = 2;	// narrow-band

			rtk_SetRtpPayloadType(&codec_config);

			// start rtp (channel number = 0, session number = 0)
			rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
			bStartRTP = 1;
			printf("%s:%d -> %s:%d (send_recv sessin)\n", argv[2], atoi(argv[4]), argv[3], atoi(argv[5]));

			// ---------------------------------
			rtp_config.sid                 = ActiveSession+1; 
			rtp_config.extPort             = htons(atoi(argv[4]))+2;
			rtp_config.remPort             = htons(atoi(argv[5]))+2; 
			rtp_config.rfc2833_payload_type_local  = 96+4; 
			rtp_config.rfc2833_payload_type_remote = 96+4;
			rtk_SetRtpConfig(&rtp_config);

			codec_config.sid               = ActiveSession+1;
			rtk_SetRtpPayloadType(&codec_config);

			rtk_SetRtpSessionState(chid, ActiveSession+1, rtp_session_sendonly);
			printf("%s:%d -> %s:%d (send_only session)", argv[2], atoi(argv[4])+4, argv[3], atoi(argv[5])+4);

			printf("Active session is %d\n",ActiveSession);

			break;

		case SIGN_ONHOOK:
			// close rtp
			rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
			rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);
			bStartRTP = 0;

			// call rtk_OnHookAction at last
			rtk_OnHookAction(chid);
			break;

		case SIGN_FLASHHOOK:

			rtk_SetRtpSessionState(chid,ActiveSession,rtp_session_sendonly);

			ActiveSession = !ActiveSession;

			rtk_SetRtpSessionState(chid,ActiveSession,rtp_session_sendrecv);//sendonly 1, recvonly 2, sendrecv 3
			rtk_SetTranSessionID(chid, ActiveSession);

			rtk_SetPlayTone(chid, ActiveSession, DSPCODEC_TONE_CONFIRMATION, 1, DSPCODEC_TONEDIRECTION_BOTH);
			printf("\nSwitch Active session to %d\n",ActiveSession);
			break;

		default:
			break;
		}

		usleep(100000); // 100ms
	}

	return 0;
}

