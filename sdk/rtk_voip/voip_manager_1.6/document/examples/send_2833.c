#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "voip_manager.h"

void start_Rtp(uint chid)
{
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;

	rtk_OffHookAction(chid);
	rtk_SetTranSessionID(chid, 0);
		/* 		
		 * set RTP session
		 */
	memset(&rtp_config, 0, sizeof(rtp_config));
	rtp_config.chid 	= chid;		// use channel 
	rtp_config.sid 		= 0;
	rtp_config.isTcp 	= 0;		// use udp
	rtp_config.extIp 	= inet_addr("192.168.1.1");
	rtp_config.remIp 	= inet_addr("192.168.1.1");
	rtp_config.extPort 	= htons(9000 + chid);
	rtp_config.remPort 	= htons(10000);
#ifdef SUPPORT_VOICE_QOS
	rtp_config.tos 		= 0;
#endif


	rtp_config.rfc2833_payload_type_local 	= 96;
	rtp_config.rfc2833_payload_type_remote 	= 96;
	rtp_config.rfc2833_fax_modem_pt_local = 101;
	rtp_config.rfc2833_fax_modem_pt_remote = 101;

#if defined(SUPPORT_RTP_REDUNDANT_APP)
	rtp_config.rtp_redundant_payload_type_local	 = 0;
	rtp_config.rtp_redundant_payload_type_remote = 0;
	rtp_config.rtp_redundant_max_Audio = 0;
	rtp_config.rtp_redundant_max_RFC2833 = 0;
#elif defined(SUPPORT_RTP_REDUNDANT)
	rtp_config.rtp_redundant_payload_type_local	 = 0;
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



	/* 
	 * set rtp payload, and other session parameters.
	 */
	codec_config.chid 			 = chid;
	codec_config.sid 			 = 0;
	codec_config.local_pt 		 = 0;					// G729 use static pt 18
	codec_config.remote_pt 		 = 0;					// G729 use static pt 18
	codec_config.local_pt_vbd 	 = rtpPayloadUndefined;
	codec_config.remote_pt_vbd 	 = rtpPayloadUndefined;
	codec_config.uPktFormat_vbd  = rtpPayloadUndefined;
	codec_config.nG723Type 		 = 0;
	codec_config.nLocalFramePerPacket = 1;
	codec_config.nRemoteFramePerPacket = 1;
	codec_config.nFramePerPacket_vbd = 1;
	codec_config.bVAD 			 = 0;
	codec_config.bPLC 			 = 1;
	codec_config.nG726Packing 	 = 2;
	codec_config.nJitterDelay 	 = 4;
	codec_config.nMaxDelay 		 = 20;
	codec_config.nMaxStrictDelay = 10000;
	codec_config.nJitterFactor 	 = 1;


	if ((codec_config.uLocalPktFormat == rtpPayloadG722) ||
	    (codec_config.uRemotePktFormat == rtpPayloadG722) )
		codec_config.nPcmMode = 3;	// wide-band
	else
		codec_config.nPcmMode = 2;	// narrow-band

	rtk_SetRtpPayloadType(&codec_config);

	rtk_SetRtpSessionState(chid, 0, rtp_session_sendrecv);	
}



int main(int argc, char *argv[])
{
	uint chid=0;
	uint mode=0;
	SIGNSTATE val =0;
	int volume_ctrl_mode=0;
	int opt = 0;
	int volume=0;
	int limitation=0;
	int limitation_enable=0;
	int interval=0;
		/*
	 * call rtk_OffHookAction at first
	 */

	if (argc < 3)
	{	
		printf("****** Usage ******\n");
		printf("%s chid sent_by.\n", argv[0]);
		printf("sent_by         		: 0 (DSP), 1 (application)\n\n");
		printf("Optional Config:\n");
		printf("-n value	:volume mode=NON_DSP_AUTO, volume=value\n");
		printf("-a 		:volume mode=DSP_AUTO \n");
		printf("-e value	:enable duration limitation, limit=value\n");
		printf("-d 		:disable duration limitation\n");
		printf("-i value	:RFC2833 packet interval\n\n");

		/*Example*/
		printf("  Example of chid 0 send RFC2833 by DSP auto\n");
		printf("    send_2833 0 0\n");
		printf("  Example of chid 0 send RFC2833 by DSP, volume ctrl NON_DSP_AUTO, vloume 20 -dBm\n");
		printf("    send_2833 0 0 -n 20\n");
		printf("  Example of chid 0 send RFC2833 by AP, volume ctrl NON_DSP_AUTO, vloume 10 -dBm\n");
		printf("    send_2833 0 1 -n 10\n");
		printf("  Example of chid 0 send RFC2833 by AP,  enable limitation 200ms\n");
		printf("    send_2833 0 1 -e 200\n");
		printf("  Example of chid 0 send RFC2833 by AP, volume ctrl NON_DSP_AUTO, vloume 10-dBm, enable limitation 80ms, packet interval 20\n");
		printf("    send_2833 0 1 -n 10 -e 80 -i 20\n");

		return 0;
	}




	chid = atoi(argv[1]);
	mode = atoi(argv[2]);
	
	rtk_InitDsp(chid);
	rtk_SetFlushFifo(chid);	
	rtk_SetDtmfMode(chid, 0);	// set dtmf mode

	if (argc == 3)
	{
		rtk_SetRFC2833SendByAP(chid, mode);
	}


	
	while (( opt = getopt(argc, argv, "n:ae:di:")) != -1 )
	{
		switch (opt)
		{
			case 'n':
				volume_ctrl_mode=0;
				sscanf(optarg, "%d", &volume);
				rtk_SetRFC2833TxConfig(chid, mode, volume_ctrl_mode, volume); //volume dsp auto mode, only support send by dsp mode.
				break;
			case 'a':
				volume_ctrl_mode=1;
				if (mode ==1)
				{
					printf("invalid setting. TX mode is AP, but volume is DSP auto mode.");
					return 0;
				}
				rtk_SetRFC2833TxConfig(chid, mode, volume_ctrl_mode, 0); //volume dsp auto mode, only support send by dsp mode.
				break;
			case 'e':
				limitation_enable=1;
				sscanf(optarg, "%d", &limitation);
				rtk_SetRFC2833SendByAP(chid, mode);
				rtk_LimitMaxRfc2833DtmfDuration(chid, limitation,limitation_enable);
				break;
			case 'd':
				limitation_enable=0;
				rtk_SetRFC2833SendByAP(chid, mode);
				rtk_LimitMaxRfc2833DtmfDuration(chid, 0,limitation_enable);

				break;
			case 'i':
				sscanf(optarg, "%d", &interval);
				rtk_SetRFC2833SendByAP(chid, mode);
				
				rtk_SetRfc2833PacketInterval(chid, 0, 0, interval);

				
				break;
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}
	
	






int ssid=0;	

	while(1)
	{	
		for(chid = 0 ; chid <CON_CH_NUM ; chid ++)
		{	
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
					if(mode==0){	//dsp mode
						/*DSP mode do nothing*/
					}else{	//AP mode 
					
						rtk_SetRtpRfc2833(chid, ssid, val, 100);
						
					}
					break;
					
				case SIGN_OFFHOOK:
					start_Rtp(chid);//start Rtp
					break;
		
				case SIGN_ONHOOK:
					rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);//close rtp
					break;
		
				default:
					break;
			}
		}
		usleep(100000); // 100ms
	}

	
	
	return 0;
}


