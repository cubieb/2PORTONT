#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"
 
//#define SUPPORT_RFC2833
 
int main(int argc, char *argv[])
{
	SIGNSTATE val;
	rtp_config_t rtp_config;
	rtcp_config_t rtcp_config;
	payloadtype_config_t codec_config;
	int ActiveSession;
	int chid = 0;
	int tx_codec=0, rx_codec=0;
	int tx_pktformat=0, rx_pktformat=0;          
	int tx_rtpredtype=0, rx_rtpredtype=0;
	int tx_frame_no=1;

	int mid;
	VoipEventID dsp_ent;
	uint32 dsp_data;
	int max_audio;

	if (argc != 14){
		printf("usage: %s chid src_ip dest_ip src_port dest_port tx_codec rx_codec tx_pktformat rx_pktformat tx_rtpredtype rx_rtpredtype tx_frame_no max_audio\n", argv[0]);
		printf("example: %s 0 192.168.1.1 192.168.1.7 9000 90004 8 8 8 8 96 96 1 1\n", argv[0]); // payload 8 (pcma) , dynamic payload 96 ,  redundant 1 previous rtp payload		
		printf("example: %s 0 192.168.1.1 192.168.1.7 9000 90004 8 8 8 8 96 96 1 2\n", argv[0]); // payload 8 (pcma) , dynamic payload 96 ,  redundant 2 previous rtp payload
		return 0;
	}

	chid           = atoi(argv[1]);

	tx_codec       = atoi(argv[6]);
	rx_codec       = atoi(argv[7]);
	tx_pktformat   = atoi(argv[8]);
	rx_pktformat   = atoi(argv[9]);
	tx_rtpredtype  = atoi(argv[10]);
	rx_rtpredtype  = atoi(argv[11]);
	tx_frame_no    = atoi(argv[12]);
	max_audio      = atoi(argv[13]);


	printf("tx_codec=%d rx_codec=%d tx_pktformat=%d rx_pktformat=%d tx_rtpredtype=%d rx_rtpredtype=%d max_audio=%d\n", 
		tx_codec , rx_codec , tx_pktformat , rx_pktformat , tx_rtpredtype , rx_rtpredtype , max_audio);

	// init dsp
	rtk_InitDsp(chid);
 
#ifdef SUPPORT_RFC2833
	rtk_SetDtmfMode(chid, DTMF_RFC2833);     // set dtmf mode
#else
	rtk_SetDtmfMode(chid, DTMF_INBAND);      // set dtmf mode
#endif
 
         // init flag
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
			case SIGN_OFFHOOK:
		        // call rtk_OffHookAction at first
		        rtk_OffHookAction(chid);
		        ActiveSession = 0;
		        rtk_SetTranSessionID(chid, ActiveSession);

		        // set RTP session
		        memset(&rtp_config, 0, sizeof(rtp_config));
		        rtp_config.chid     = chid;           // use channel 
		        rtp_config.sid      = ActiveSession;
		        rtp_config.isTcp    = 0;              // use udp
		        rtp_config.extIp    = inet_addr(argv[2]);
		        rtp_config.remIp    = inet_addr(argv[3]);
		        rtp_config.extPort  = htons(atoi(argv[4]));
		        rtp_config.remPort  = htons(atoi(argv[5]));
#ifdef SUPPORT_VOICE_QOS
                rtp_config.tos      = 0;
#endif

#ifdef SUPPORT_RFC2833
                rtp_config.rfc2833_payload_type_local  = 96;
                rtp_config.rfc2833_payload_type_remote = 96;
                rtp_config.rfc2833_fax_modem_pt_local  = 101;
                rtp_config.rfc2833_fax_modem_pt_remote = 101;
#else
                rtp_config.rfc2833_payload_type_local  = 0;
                rtp_config.rfc2833_payload_type_remote = 0;
                rtp_config.rfc2833_fax_modem_pt_local  = 0;
                rtp_config.rfc2833_fax_modem_pt_remote = 0;
#endif

#if defined(SUPPORT_RTP_REDUNDANT_APP)
                rtp_config.rtp_redundant_payload_type_local  = tx_rtpredtype;
                rtp_config.rtp_redundant_payload_type_remote = rx_rtpredtype;
                rtp_config.rtp_redundant_max_Audio = max_audio;
                rtp_config.rtp_redundant_max_RFC2833 = max_audio;
#elif defined(SUPPORT_RTP_REDUNDANT)
                rtp_config.rtp_redundant_payload_type_local  = tx_rtpredtype;
                rtp_config.rtp_redundant_payload_type_remote = rx_rtpredtype;
                rtp_config.rtp_redundant_max_Audio = max_audio;
                rtp_config.rtp_redundant_max_RFC2833 = max_audio;
#endif
                rtp_config.SID_payload_type_local  = 0;
                rtp_config.SID_payload_type_remote = 0;
                rtp_config.init_randomly = 1;
                rtp_config.init_seqno = 0;
                rtp_config.init_SSRC = 0;
                rtp_config.init_timestamp = 0;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
                rtp_config.ipv6 = 0;
#endif
                rtk_SetRtpConfig(&rtp_config);

                // set RTCP session
                rtcp_config.chid    = rtp_config.chid;
                rtcp_config.sid     = rtp_config.sid;
                rtcp_config.remIp   = rtp_config.remIp;
                rtcp_config.remPort = rtp_config.remPort + 1;
                rtcp_config.extIp   = rtp_config.extIp;
                rtcp_config.extPort = rtp_config.extPort + 1;
#ifdef SUPPORT_VOICE_QOS
                rtcp_config.tos     = rtp_config.tos;
#endif
                
                rtcp_config.txInterval = 500;  // send rtcp every 500ms
                rtcp_config.enableXR   = 1;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
                rtcp_config.ipv6       = 0;
#endif
                rtk_SetRtcpConfig(&rtcp_config);


                // set rtp payload, and other session parameters.
                codec_config.chid    = chid;
                codec_config.sid     = ActiveSession;

                codec_config.local_pt   = tx_codec;
                codec_config.remote_pt  = rx_codec;
                codec_config.uLocalPktFormat   = tx_pktformat;
                codec_config.uRemotePktFormat  = rx_pktformat;

                codec_config.local_pt_vbd      = rtpPayloadUndefined;
                codec_config.remote_pt_vbd     = rtpPayloadUndefined;
                codec_config.uPktFormat_vbd    = rtpPayloadUndefined;
                codec_config.nG723Type                           = 0;
                codec_config.nLocalFramePerPacket  = tx_frame_no;
                codec_config.nRemoteFramePerPacket = 1;
                codec_config.nFramePerPacket_vbd = 1;
                codec_config.bVAD                = 0;
                codec_config.bPLC                = 1;
                codec_config.nJitterDelay        = 4;
                codec_config.nMaxDelay           = 13;
                codec_config.nMaxStrictDelay     = 10000;
                codec_config.nJitterFactor       = 7;
                codec_config.nG726Packing        = 2;                                        //pack right

                if ((codec_config.uLocalPktFormat == rtpPayloadG722) ||
                    (codec_config.uRemotePktFormat == rtpPayloadG722) )
                         codec_config.nPcmMode = 3; // wide-band
                else
                         codec_config.nPcmMode = 2; // narrow-band

                rtk_SetRtpPayloadType(&codec_config);

                // start rtp (channel number = 0, session number = 0)
                rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);

                printf("%s(%d)\n" , __FUNCTION__ , __LINE__);
                printf("%s:%d -> %s:%d (codec tx:%d, rx:%d), (frame_no: tx:%d)\n", 
                         argv[2], atoi(argv[4]), argv[3], atoi(argv[5]),tx_codec,rx_codec, tx_frame_no);
                break;

       case SIGN_ONHOOK:
                // close rtp
                rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
                rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);

                // call rtk_OnHookAction at last
                rtk_OnHookAction(chid);
                break;


       default:
                break;
       }
 
		usleep(100000); // 100ms
	}

     return 0;
}

