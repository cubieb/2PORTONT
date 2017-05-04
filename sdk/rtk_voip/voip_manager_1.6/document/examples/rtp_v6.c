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
shell_old_cmd( "rtp_v6", "", "", user_rtp_main );
#endif


#if 1
	struct in6_addr src_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x7005}};
	struct in6_addr dst_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x8502}};
#else
	struct in6_addr src_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x8502}};
	struct in6_addr dst_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x7005}};
#endif


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
		printf("usage: %s chid src_ip6 dest_ip6 src_port dest_port tx_codec rx_codec tx_frame_no\n", argv[0]);
		printf("example: rtp_v6 0 fe80::2e0:4cff:fe86:8502 fe80::2e0:4cff:fe86:7005 9010 9000 0 0 2");
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
		case SIGN_KEY2:
		case SIGN_KEY3:
		case SIGN_KEY4:
		case SIGN_KEY5:
		case SIGN_KEY6:
		case SIGN_KEY7:
		case SIGN_KEY8:
		case SIGN_KEY9:
		case SIGN_KEY0:
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

			rtp_config.ipv6				= 1;			
#if 0
			rtp_config.extIp6 			= src_ip6_addr;
			rtp_config.remIp6 			= dst_ip6_addr;
#else
			int s1, s2;
			char str1[INET6_ADDRSTRLEN], str2[INET6_ADDRSTRLEN];

			s1 = inet_pton(AF_INET6, argv[2], &rtp_config.extIp6);
			if (s1 <= 0)
			{
               if (s1 == 0)
                   fprintf(stderr, "src addr not in presentation format\n");
               else
                   fprintf(stderr, "inet_pton error, line%d\n", __LINE__);

				return 0;
			}
			
			// show input IPv6 addr
			if (inet_ntop(AF_INET6, &rtp_config.extIp6, str1, INET6_ADDRSTRLEN) == NULL)
			{
               fprintf(stderr,"inet_ntop error, line%d\n", __LINE__);
               return 0;
			}
			
			//printf("input src ipv6 addr: %s\n", str1);

			s2 = inet_pton(AF_INET6, argv[3], &rtp_config.remIp6);
			if (s2 <= 0)
			{
               if (s2 == 0)
                   fprintf(stderr, "dst addr not in presentation format\n");
               else
                   fprintf(stderr, "inet_pton error, line%d\n", __LINE__);
			}
			
			// show input IPv6 addr
			if (inet_ntop(AF_INET6, &rtp_config.remIp6, str2, INET6_ADDRSTRLEN) == NULL)
			{
               fprintf(stderr,"inet_ntop error, line%d\n", __LINE__);
               return 0;
			}
			
			//printf("input dst ipv6 addr: %s\n", str2);
#endif

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
			rtk_SetRtpConfig(&rtp_config);

			// set RTCP session
			rtcp_config.chid	= rtp_config.chid;
			rtcp_config.sid		= rtp_config.sid;
			rtcp_config.remPort	= rtp_config.remPort + 1;
			rtcp_config.extPort	= rtp_config.extPort + 1;

			rtcp_config.extIp6 	= rtp_config.extIp6;
			rtcp_config.remIp6 	= rtp_config.remIp6;
			rtcp_config.ipv6	= 1;

#ifdef SUPPORT_VOICE_QOS
			rtcp_config.tos		= rtp_config.tos;
#endif
			
			rtcp_config.txInterval = 500;	// send rtcp every 500ms
			rtcp_config.enableXR = 1;
			
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
				str1, atoi(argv[4]), str2, atoi(argv[5]),tx_codec,rx_codec, tx_frame_no);
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

