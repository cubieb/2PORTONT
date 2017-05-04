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
#ifdef __ECOS
#include <commands.h>
shell_old_cmd( "rtp", "", "", user_rtp_main );
#endif

void codec_change(int chid, uint32 local_ip, uint32 remote_ip, uint16 local_port, uint16 remote_port, 
	int tx_codec, int rx_codec, int tx_frame_no)
{
	int ActiveSession;
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	payloadtype_config_t codec_config;

	// close rtp
	rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
	//rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);

	// call rtk_Offhook_Action at last
	//rtk_OnHookAction(chid);
	
	
	// call rtk_Offhook_Action at first
	//rtk_OffHookAction(chid);
	ActiveSession = 0;
	//rtk_SetTranSessionID(chid, ActiveSession);

	// set RTP session
	memset(&rtp_config, 0, sizeof(rtp_config));
	rtp_config.chid 			= chid;			// use channel 
	rtp_config.sid 				= ActiveSession;
	rtp_config.isTcp 			= 0;			// use udp
	rtp_config.extIp 			= local_ip;
	rtp_config.remIp 			= remote_ip;
	rtp_config.extPort 			= local_port;
	rtp_config.remPort 			= remote_port;
#ifdef SUPPORT_VOICE_QOS
	rtp_config.tos 				= 0;
#endif
#ifdef SUPPORT_RFC2833
	rtp_config.rfc2833_payload_type_local  	= 96;
	rtp_config.rfc2833_payload_type_remote 	= 96;
	rtp_config.rfc2833_fax_modem_pt_local 	= 101;
	rtp_config.rfc2833_fax_modem_pt_remote 	= 101;
#else
	rtp_config.rfc2833_payload_type_local  	= 0;
	rtp_config.rfc2833_payload_type_remote 	= 0;
	rtp_config.rfc2833_fax_modem_pt_local 	= 0;
	rtp_config.rfc2833_fax_modem_pt_remote 	= 0;
#endif
#if defined (SUPPORT_RTP_REDUNDANT_APP)
	rtp_config.rtp_redundant_payload_type_local  	= 0;
	rtp_config.rtp_redundant_payload_type_remote 	= 0;
	rtp_config.rtp_redundant_max_Audio 		= 0;
	rtp_config.rtp_redundant_max_RFC2833 		= 0;
#elif defined (SUPPORT_RTP_REDUNDANT)
	rtp_config.rtp_redundant_payload_type_local  	= 0;
	rtp_config.rtp_redundant_payload_type_remote 	= 0;
	rtp_config.rtp_redundant_max_Audio 		= 0;
	rtp_config.rtp_redundant_max_RFC2833 		= 0;
#endif
	rtp_config.SID_payload_type_local  	= 0;
	rtp_config.SID_payload_type_remote 	= 0;
	rtp_config.init_randomly 		= 1;
	rtp_config.init_seqno 			= 0;
	rtp_config.init_SSRC 			= 0;
	rtp_config.init_timestamp 		= 0;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	rtp_config.ipv6					= 0;
#endif
	rtk_SetRtpConfig(&rtp_config);

	// set RTCP session
	rtcp_config.chid			= rtp_config.chid;
	rtcp_config.sid				= rtp_config.sid;
	rtcp_config.remIp			= rtp_config.remIp;
	rtcp_config.remPort			= rtp_config.remPort + 1;
	rtcp_config.extIp			= rtp_config.extIp;
	rtcp_config.extPort			= rtp_config.extPort + 1;
#ifdef SUPPORT_VOICE_QOS
	rtcp_config.tos				= rtp_config.tos;
#endif
	rtcp_config.txInterval		= 500;		// send rtcp every 500ms
	rtcp_config.enableXR		= 1;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
	rtcp_config.ipv6			= 0;
#endif
	rtk_SetRtcpConfig(&rtcp_config);

	// set rtp payload, and other session parameters.
	codec_config.chid 			= chid;
	codec_config.sid 			= ActiveSession;
	codec_config.local_pt 			= tx_codec;	// G729 use static pt 18
	codec_config.remote_pt 			= rx_codec;	// G729 use static pt 18
	codec_config.uLocalPktFormat 		= tx_codec;
	codec_config.uRemotePktFormat		= rx_codec;
	codec_config.local_pt_vbd 		= rtpPayloadUndefined;
	codec_config.remote_pt_vbd 		= rtpPayloadUndefined;
	codec_config.uPktFormat_vbd 		= rtpPayloadUndefined;
	codec_config.nG723Type 			= 0;
	codec_config.nLocalFramePerPacket 	= tx_frame_no;
	codec_config.nRemoteFramePerPacket 	= 1;
	codec_config.nFramePerPacket_vbd = 1;
	codec_config.bVAD 			= 0;
	codec_config.bPLC 			= 1;
	codec_config.nJitterDelay 		= 4;
	codec_config.nMaxDelay 			= 13;
	codec_config.nMaxStrictDelay    = 10000;
	codec_config.nJitterFactor 		= 7;
	codec_config.nG726Packing 		= 2;		//pack right

	if ((codec_config.uLocalPktFormat == rtpPayloadG722) ||
	    (codec_config.uRemotePktFormat == rtpPayloadG722) )
		codec_config.nPcmMode 		= 3;		// wide-band
	else
		codec_config.nPcmMode 		= 2;		// narrow-band

	rtk_SetRtpPayloadType(&codec_config);

	// start rtp (channel number = 0, session number = 0)
	rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
	printf("codec_change (codec tx:%d, rx:%d), (frame_no: tx:%d)\n", 
		tx_codec,rx_codec, tx_frame_no);

}

#ifdef __ECOS
CMD_DECL( user_rtp_main )
#else
int main(int argc, char *argv[])
#endif
{
	SIGNSTATE val;
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	payloadtype_config_t codec_config;
	int ActiveSession;
	int bStartRTP;
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid = 0;
	int tx_codec=0;
	int rx_codec=0;
	int tx_frame_no=1;
	int mid;
	VoipEventID dsp_ent;
	uint32 dsp_data;


	if (argc != 9)
	{
		printf("usage: %s chid src_ip dest_ip src_port dest_port tx_codec rx_codec tx_frame_no\n", argv[0]);
		return 0;
	}

	tx_codec = atoi(argv[6]);
	rx_codec = atoi(argv[7]);
	tx_frame_no = atoi(argv[8]);

	// init channel
	chid = atoi(argv[1]);

	// init dsp
	rtk_InitDsp(chid);

#ifdef SUPPORT_RFC2833
	rtk_SetDtmfMode(chid, DTMF_RFC2833);	// set dtmf mode
#else
	rtk_SetDtmfMode(chid, DTMF_INBAND);	// set dtmf mode
#endif

	// init flag
	bStartRTP = 0;
	ActiveSession = 0;

	while (1)
	{
		for (mid=0; mid <2; mid++)
		{
			rtk_GetDspEvent(chid, mid, &dsp_ent, &dsp_data);
			if (dsp_ent == VEID_DSP_RTP_PAYLOAD_MISMATCH)
				printf("Get PT mis-matched event, ch%d, mid%d, PT=%d\n", chid, mid, dsp_data);
		}
		rtk_GetFxsEvent(chid, &val);
		switch (val)
		{
		case SIGN_KEY1:
			// Change TX to G.711a
			if (chid ==0 )
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadPCMA, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY2:
			if (chid ==0 )
			// Change TX to G.722
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadG722, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY3:
			if (chid ==0 )
			// Change TX to G.723
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadG723, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY4:
			if (chid ==0 )
			// Change TX to GSM-FR
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadGSM, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY5:
			if (chid ==0 )
			// Change TX to 
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadPCMU, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY6:
			if (chid ==0 )
			// Change TX to G.726-32k
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadG726_32, tx_codec, tx_frame_no);
			break;
		case SIGN_KEY7:
			if (chid ==0 )
			// Change TX to iLBC 30ms
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayload_iLBC, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY8:
			if (chid ==0 )
			// Change TX to iLBC 20ms
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayload_iLBC_20ms, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY9:
			if (chid ==0 )
			// Change TX to G.729
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadG729, rx_codec, tx_frame_no);
			break;
		case SIGN_KEY0:
			if (chid ==0 )
			// Change TX to G.711u
			codec_change(chid, inet_addr(argv[2]), inet_addr(argv[3]), 
				htons(atoi(argv[4])), htons(atoi(argv[5])), rtpPayloadPCMU, rx_codec, tx_frame_no);
			break;
		case SIGN_STAR:
		case SIGN_HASH:
			if (bStartRTP && ActiveSession == 0)
				rtk_SetRtpRfc2833(chid, 0, dtmf_val[val], 100);
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

#ifdef SUPPORT_RFC2833
			rtp_config.rfc2833_payload_type_local  = 96;
			rtp_config.rfc2833_payload_type_remote = 96;
			rtp_config.rfc2833_fax_modem_pt_local = 101;
			rtp_config.rfc2833_fax_modem_pt_remote = 101;
#else
			rtp_config.rfc2833_payload_type_local  = 0;
			rtp_config.rfc2833_payload_type_remote = 0;
			rtp_config.rfc2833_fax_modem_pt_local = 0;
			rtp_config.rfc2833_fax_modem_pt_remote = 0;
#endif
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
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
			rtp_config.ipv6			= 0;
#endif
			rtk_SetRtpConfig(&rtp_config);

			// set RTCP session
			rtcp_config.chid	= rtp_config.chid;
			rtcp_config.sid		= rtp_config.sid;
			rtcp_config.remIp	= rtp_config.remIp;
			rtcp_config.remPort	= rtp_config.remPort + 1;
			rtcp_config.extIp	= rtp_config.extIp;
			rtcp_config.extPort	= rtp_config.extPort + 1;
#ifdef SUPPORT_VOICE_QOS
			rtcp_config.tos		= rtp_config.tos;
#endif
			
			rtcp_config.txInterval = 500;	// send rtcp every 500ms
			rtcp_config.enableXR = 1;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
			rtcp_config.ipv6	= 0;
#endif
			rtk_SetRtcpConfig(&rtcp_config);


			// set rtp payload, and other session parameters.
			codec_config.chid 				= chid;
			codec_config.sid 				= ActiveSession;

			codec_config.local_pt 			= tx_codec;					// G729 use static pt 18
			codec_config.remote_pt 			= rx_codec;					// G729 use static pt 18

			codec_config.uLocalPktFormat 	= tx_codec;
			codec_config.uRemotePktFormat	= rx_codec;

			codec_config.local_pt_vbd 		= rtpPayloadUndefined;
			codec_config.remote_pt_vbd 		= rtpPayloadUndefined;
			codec_config.uPktFormat_vbd 	= rtpPayloadUndefined;
			codec_config.nG723Type 			= 0;
			codec_config.nLocalFramePerPacket 	= tx_frame_no;
			codec_config.nRemoteFramePerPacket 	= 1;
			codec_config.nFramePerPacket_vbd = 1;
			codec_config.bVAD 				= 0;
			codec_config.bPLC 				= 1;
			codec_config.nJitterDelay 		= 4;
			codec_config.nMaxDelay 			= 13;
			codec_config.nMaxStrictDelay	= 10000;
			codec_config.nJitterFactor 		= 7;
			codec_config.nG726Packing 		= 2;					//pack right

			if ((codec_config.uLocalPktFormat == rtpPayloadG722) ||
			    (codec_config.uRemotePktFormat == rtpPayloadG722) )
				codec_config.nPcmMode = 3;	// wide-band
			else
				codec_config.nPcmMode = 2;	// narrow-band

			rtk_SetRtpPayloadType(&codec_config);

			// start rtp (channel number = 0, session number = 0)
			rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
			bStartRTP = 1;

			printf("%s:%d -> %s:%d (codec tx:%d, rx:%d), (frame_no: tx:%d)\n", 
				argv[2], atoi(argv[4]), argv[3], atoi(argv[5]),tx_codec,rx_codec, tx_frame_no);
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

		usleep(100000); // 100ms
	}

	return 0;
}

