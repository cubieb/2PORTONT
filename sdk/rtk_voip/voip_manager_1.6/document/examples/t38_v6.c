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
shell_old_cmd( "t38_v6", "", "", user_rtp_main );
#endif


#ifdef __ECOS
CMD_DECL( user_rtp_main )
#else
int main(int argc, char *argv[])
#endif
{
	SIGNSTATE val;
	t38udp_config_t t38udp_config;
	t38_payloadtype_config_t t38_config;

	int ActiveSession;
	int bStartRTP;
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid = 0;
	int mid;
	VoipEventID dsp_ent;
	uint32 dsp_data;


	if (argc != 6)
	{
		printf("usage: %s chid src_ip6 dest_ip6 src_port dest_port\n", argv[0]);
		printf("example: t38_v6 0 fe80::2e0:4cff:fe86:8502 fe80::2e0:4cff:fe86:7005 9010 9000");
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
			
			// set rtp session
			memset(&t38udp_config, 0, sizeof(t38udp_config));
			t38udp_config.chid = chid;
			t38udp_config.sid = ActiveSession;
			t38udp_config.isTcp = 0;		// use udp
			t38udp_config.ipv6	= 1;

#if 0			
			t38udp_config.extIp = htonl(channel->ip);
			t38udp_config.remIp = htonl(channel->remote->ip);
#else
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

			printf("%s:%d -> %s:%d\n", str1, atoi(argv[4]), str2, atoi(argv[5]));
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

