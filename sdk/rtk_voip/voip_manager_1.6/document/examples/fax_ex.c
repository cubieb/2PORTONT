#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

// test option:
// 	- FXO_DIAL_FXS0 has defined
//		* fxo ring will dial fxs0 automatically
//	- FXO_DIAL_FXS0 has not defined
//		* fxo ring will prompt user dial voip number via IVR
//  - Allow receiving fax from FXO line

int fax_use_t38 = 1;
#define USE_TAPI_RTKGETFAXMDEMEVT
#define FXO_DIAL_FXS0_AUTO

#define FUNCKEY_FXO_0	".0" /* keypad presses *0 to dial FXO0 */
#define FUNCKEY_FXO_1	".1"
#define FUNCKEY_FXO_2	".2"
#define FUNCKEY_FXO_3	".3"

#define FUNCKEY_FXS_0 	".4" /* keypad presses *4 to dial FXS0 */
#define FUNCKEY_FXS_1 	".5" /* keypad presses *5 to dial FXS1 */

#define CHANNEL_IS_FXS(chid)	RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature )
#define CHANNEL_IS_FXO(chid)	RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature )

#define SELECT_WAIT_SEC		5
#define DEBUG_FXO			

#ifdef DEBUG_FXO 
char * get_evt_name(VoipEventID evt);
#endif
int do_fax_event( int chid, VoipEventID evt );
int check_fax_event(int chid, VoipEventID evt );

int g_VoIPPorts = 0;

int do_fxo_event( int chid, SIGNSTATE evt );

enum {
	FXO_NO_DAA = 0,
	FXO_VIRTUAL_DAA,
	FXO_DAA
};

enum {
	STATE_IDLE = 0,
	STATE_RING,
	STATE_CONNECT,
	STATE_DAA_RING,			// virtual daa state
	STATE_DAA_CONNECT		// virtual daa state
};

enum {
	DIAL_NONE = 0,
	DIAL_FXO_0,
	DIAL_FXO_1,
	DIAL_FXO_2,
	DIAL_FXO_3,
	DIAL_FXS_0,
	DIAL_FXS_1,
	DIAL_OTHER,
};

typedef struct channel_s channel_t;

struct channel_s {
	// phone info
	int chid;
	int state;
	char dial_code[101];
	int dial_index;
	// rtp info
	unsigned long ip;
	unsigned short port;
	// remote channel (just peer simulation)
	channel_t *remote;
	//FAX
	uint32 fax_type;
	int fax_clear;
	uint32 fax_end;
	int fax_end_clear;
};

void channel_init(channel_t *channel, int chid);
void channel_reset(channel_t *channel, int chid);
int channel_input(channel_t *channel, SIGNSTATE evt);
void channel_dial_local(channel_t *channel, channel_t *remote_channel);
void channel_dial_out(channel_t *channel);
void channel_switch_to_fxo(channel_t *channel);	// virtual daa
void channel_rtp_start(channel_t *channel);
void channel_t38_start(channel_t *channel);
void channel_rtp_stop(channel_t *channel);

// global variable
int fxo_type;
channel_t channels[CON_CH_NUM];

void channel_init(channel_t *channel, int chid)
{
	memset(channel, 0, sizeof(*channel));
	// phone
	channel->chid = chid;
	channel->state = STATE_IDLE;
	channel->dial_code[0] = 0;
	channel->dial_index = 0;
	// rtp
	channel->ip = ntohl(inet_addr("127.0.0.1"));
	channel->port = 9000 + chid;
	// reomte 
	channel->remote = NULL;
	channel->fax_clear = 1;
}

int channel_input(channel_t *channel, SIGNSTATE val)
{
	if (channel->dial_index >= sizeof(channel->dial_code) - 1) 
	{
		printf("%s: out of buffer\n", __FUNCTION__);
		return DIAL_NONE;
	}

	if (val >= SIGN_KEY1 && val <= SIGN_KEY9) 
		channel->dial_code[channel->dial_index++] = (char) '0' + val;
	else if (val == SIGN_KEY0)
		channel->dial_code[channel->dial_index++] = '0';
	else if (val == SIGN_STAR)
		channel->dial_code[channel->dial_index++] = '.';
	else if (val == SIGN_HASH)
		return DIAL_OTHER;

	channel->dial_code[channel->dial_index] = 0;

	// check local dial
	if (strcmp(channel->dial_code, FUNCKEY_FXO_0) == 0)
		return DIAL_FXO_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_1) == 0)
		return DIAL_FXO_1;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_2) == 0)
		return DIAL_FXO_2;
	else if (strcmp(channel->dial_code, FUNCKEY_FXO_3) == 0)
		return DIAL_FXO_3;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_0) == 0)
		return DIAL_FXS_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_1) == 0)
		return DIAL_FXS_1;

	return DIAL_NONE;
}

void channel_dial_local(channel_t *channel, channel_t *remote_channel)
{
	if (remote_channel->chid == channel->chid)
	{
		printf("%s: couldn't dial itself\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		return;
	}

	// 1. create local rtp
	channel->remote = remote_channel;
	channel_rtp_start(channel);
	channel->state = STATE_CONNECT;

	// 2. signal remote channel
	remote_channel->remote = channel;

	if (CHANNEL_IS_FXS(remote_channel->chid))
	{
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		// signal fxs
		rtk_SetRingFxs(remote_channel->chid, 1);
	}
	else
	{	
		// signal fxo
		rtk_FxoRingOn(remote_channel->chid);
	}

	remote_channel->state = STATE_RING;
}

void channel_resetup_local_t38(channel_t *channel)
{
	channel_rtp_stop(channel);
	channel_t38_start(channel);
}

void channel_dial_out(channel_t *channel)
{
	printf("%s: dial %s done.\n", __FUNCTION__, channel->dial_code);
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
}

void channel_switch_to_fxo(channel_t *channel)
{
	if (rtk_DaaOffHook(channel->chid) == 0)
	{
		printf("%s: ok.\n", __FUNCTION__);
	}
	else	// pstn out failed
	{
		printf("%s: failed.\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
	}
}

void channel_rtp_start(channel_t *channel)
{
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;

	// set rtp session
	rtp_config.chid = channel->chid;
	rtp_config.sid = 0;
	rtp_config.isTcp = 0;		// use udp
	rtp_config.extIp = htonl(channel->ip);
	rtp_config.remIp = htonl(channel->remote->ip);
	rtp_config.extPort = htons(channel->port);
	rtp_config.remPort = htons(channel->remote->port);
#ifdef SUPPORT_VOICE_QOS
	rtp_config.tos = 0;
#endif
	rtp_config.rfc2833_payload_type_local = 0;		// use in-band
	rtp_config.rfc2833_payload_type_remote = 0;		// use in-band
	rtp_config.rfc2833_fax_modem_pt_local = 0;		// use in-band
	rtp_config.rfc2833_fax_modem_pt_remote = 0;		// use in-band
#if defined(SUPPORT_RTP_REDUNDANT_APP)
	rtp_config.rtp_redundant_payload_type_local = 0;
	rtp_config.rtp_redundant_payload_type_remote = 0;
	rtp_config.rtp_redundant_max_Audio = 0;
	rtp_config.rtp_redundant_max_RFC2833 = 0;
#elif defined(SUPPORT_RTP_REDUNDANT)
	rtp_config.rtp_redundant_payload_type_local = 0;
	rtp_config.rtp_redundant_payload_type_remote = 0;
	rtp_config.rtp_redundant_max_Audio = 0;
	rtp_config.rtp_redundant_max_RFC2833 = 0;
#endif
	rtp_config.SID_payload_type_local = 0;
	rtp_config.SID_payload_type_remote = 0;
	rtp_config.init_randomly = 1;
	rtp_config.init_seqno = 0;
	rtp_config.init_SSRC = 0;
	rtp_config.init_timestamp = 0;
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
    rtp_config.ipv6 = 0;
#endif
	rtk_SetRtpConfig(&rtp_config);

	// set rtp payload, and other session parameters.
	codec_config.chid = channel->chid;
	codec_config.sid = 0;
	codec_config.local_pt = 0;
	codec_config.remote_pt = 0;
	codec_config.uLocalPktFormat = rtpPayloadPCMU;
	codec_config.uRemotePktFormat = rtpPayloadPCMU;
	codec_config.local_pt_vbd = rtpPayloadUndefined;
	codec_config.remote_pt_vbd = rtpPayloadUndefined;
	codec_config.uPktFormat_vbd = rtpPayloadUndefined;
	codec_config.nG723Type = 0;
	codec_config.nLocalFramePerPacket = 1;
	codec_config.nRemoteFramePerPacket = 1;
	codec_config.nFramePerPacket_vbd = 1;
	codec_config.bVAD = 0;
	codec_config.bPLC = 1;
	codec_config.nJitterDelay = 4;
	codec_config.nMaxDelay = 13;
	codec_config.nMaxStrictDelay = 10000;
	codec_config.nJitterFactor = 7;
	codec_config.nG726Packing = 2;//pack right
	if ( (codec_config.uLocalPktFormat == rtpPayloadG722) || (codec_config.uRemotePktFormat == rtpPayloadG722) )
		codec_config.nPcmMode = 3;	// wide-band
	else
		codec_config.nPcmMode = 2;	// narrow-band
	rtk_SetRtpPayloadType(&codec_config);

	// start rtp 
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_sendrecv);
}

void channel_t38_start(channel_t *channel)
{
	t38udp_config_t t38udp_config;
	t38_payloadtype_config_t t38_config;

	// set rtp session
	t38udp_config.chid = channel->chid;
	t38udp_config.sid = 0;
	t38udp_config.isTcp = 0;		// use udp
	t38udp_config.extIp = htonl(channel->ip);
	t38udp_config.remIp = htonl(channel->remote->ip);
	t38udp_config.extPort = htons(channel->port);
	t38udp_config.remPort = htons(channel->remote->port);
	t38udp_config.ipv6 = 0;
#ifdef SUPPORT_VOICE_QOS
	t38udp_config.tos = 0;
#endif
	
	rtk_SetT38UdpConfig(&t38udp_config);

	// set rtp payload, and other session parameters.
	t38_config.chid = channel->chid;
	t38_config.sid = 0;
#if 1 /* original */
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
#else
	t38_config.bT38ParamEnable = 1;
	//t38_config.nT38MaxBuffer = 72;
	t38_config.nT38MaxBuffer = 200;
	t38_config.nT38RateMgt = 2;
	t38_config.nT38MaxRate = 5;
	t38_config.bT38EnableECM = 1;
	t38_config.nT38ECCSignal = 5;
	t38_config.nT38ECCData = 2;
	t38_config.bT38EnableSpoof = 1;
	t38_config.nT38DuplicateNum = 2;
	t38_config.nPcmMode = 2;	// narrow-band
#endif	
	rtk_SetT38PayloadType(&t38_config);

	// start rtp 
	//rtk_SetRtpSessionState(channel->chid, 0, rtp_session_sendrecv);
}

void channel_rtp_stop(channel_t *channel)
{
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_inactive);
}

void channel_handle_input(channel_t *channel, VoipEventID evt)
{
	int ret;

	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
	ret = channel_input(channel, evt);
	switch (ret)
	{
	case DIAL_FXO_0:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+0]);	// dial fxo 0
		}
		break;
#ifdef RTK_VOIP_DRIVERS_FXO
	case DIAL_FXO_1:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+1]);	// dial fxo 1
		}
		break;
	case DIAL_FXO_2:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+2]);	// dial fxo 2
		}
		break;
	case DIAL_FXO_3:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[RTK_VOIP_DAA_CH_OFFSET( g_VoIP_Feature )+3]);	// dial fxo 3
		}
		break;
#endif
	case DIAL_FXS_0:
		channel_dial_local(channel, &channels[0]);	// dial fxs 0
		break;
	case DIAL_FXS_1:
		if (RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) < 2)
		{
			printf("%s: no fxs 1\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		channel_dial_local(channel, &channels[1]);	// dial fxs 1
		break;
	case DIAL_OTHER:
		channel_dial_out(channel);
		break;
	default:
		// nothing
		break;
	}
}

void channel_handle_onhook(channel_t *channel)
{
	switch (channel->state)
	{
	case STATE_CONNECT:
		channel_rtp_stop(channel);
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FxoOnHook(channel->chid);
		}

		// check remote channel is terminated 
		if (channel->remote->remote == NULL) 
			break;

		// signal remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			if(channel->remote->state == STATE_RING)
			{
				channel->remote->state = STATE_IDLE;
				rtk_SetRingFxs(channel->remote->chid, 0);
			}
			else if(channel->remote->state == STATE_CONNECT)
			{
				rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
				channel_rtp_stop(channel->remote);
			}
		}
		else
		{
			rtk_FxoBusy(channel->remote->chid);
		}
		break;
	case STATE_DAA_CONNECT:
		// virtual daa
		rtk_DaaOnHook(channel->chid);
		break;
	default:
		break;
	}

#ifdef CONFIG_RTK_VOIP_IVR
	rtk_IvrStopPlaying(channel->chid, 0); // clear ivr
#endif
	rtk_SetTranSessionID(channel->chid, 255);	// clear resource count
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);	// clear tone
	channel_init(channel, channel->chid);
}

void channel_handle_offhook(channel_t *channel)
{
	// active session 0, and record resource + 1
	rtk_SetTranSessionID(channel->chid, 0);

	switch (channel->state)
	{
	case STATE_IDLE:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		else
		{
			char caller_id[21];
			char caller_date[9];
			char caller_name[51];

		#ifdef FXO_DIAL_FXS0_AUTO
			rtk_GetDaaCallerID(channel->chid, caller_id, caller_date, caller_name);

			if (caller_id[0])
				printf("caller id is %s\n", caller_id);
			else
				printf("no caller id detect\n");

			if (caller_name[0])
				printf("caller name is %s\n", caller_name);
				
#if 1	// Gen FSK CID

			//rtk_SetCidFskGenMode(channel->chid - 4, 1); // soft gen

			/* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
			rtk_SetFskArea(channel->chid - g_VoIPPorts, 0x108/*area*/);   
			rtk_GenCidAndFxsRing(channel->chid - g_VoIPPorts, 1/*fsk*/, caller_id, caller_date, caller_name, 0/*type1*/, 0);
#endif

//			channel_dial_local(channel, &channels[0]);	// dial fxs 0
			channel_dial_local(channel, &channels[channel->chid - g_VoIPPorts]);	// dial fxs 0
		#else
			char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};

			usleep(500000); // after phone off-hook, wait for a second, and then play IVR
			rtk_FxoOffHook(channel->chid);
		#ifdef CONFIG_RTK_VOIP_IVR
			rtk_IvrStartPlaying(channel->chid, 0, text);
		#else
			// use dial tone to replace if no IVR
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		#endif
		#endif
		}
		break;
	case STATE_RING:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetRingFxs(channel->chid, 0);
			rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_RINGING, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
		else
		{
			// daa
			rtk_FxoOffHook(channel->chid);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
				
		// ack remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FxoOffHook(channel->remote->chid);
		}
		break;
	case STATE_DAA_RING:
		// virtual daa
		rtk_DaaOffHook(channel->chid);
		channel->state = STATE_DAA_CONNECT;
		break;
	default:
		break;
	}
}

int main(int argc , char *argv[])
{
	int i;
	int ret;
	int chid;
	fd_set fdset;
	struct timeval timeout;
	TstFlushVoipEvent stFlushVoipEvent;
	TstVoipEvent stVoipEvent;
	SIGNSTATE val;

	if( argc < 2 )
		return printf("check input arguments\n");

	fax_use_t38 = atoi( argv[1] );   // 0: g711 passthrough , 1: t38
	
	if( fax_use_t38 == 0 )
		printf("g711 passthrough\n");
	else if( fax_use_t38 == 1 )
		printf("t38\n");
	else
		return printf("unsupport mode\n");

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == NO_DAA)
		fxo_type = FXO_NO_DAA;
	else if ((g_VoIP_Feature & DAA_TYPE_MASK) == VIRTUAL_DAA)
		fxo_type = FXO_VIRTUAL_DAA;
	else
		fxo_type = FXO_DAA;

	g_VoIPPorts = RTK_VOIP_SLIC_NUM(g_VoIP_Feature);
	//RTK_VOIP_SLIC_NUM( g_VoIP_Feature )

	// init 
	for (chid=0; chid<CON_CH_NUM; chid++)
	{
		// init dsp
		rtk_InitDsp(chid);

		// init phone
		rtk_SetVoiceGain(chid, 0, 0);	// 0db
		rtk_SetEchoTail(chid, 32, 1, 5, 5, 0);			// 4ms, NLP on

		// init fxs & fxo
		if (CHANNEL_IS_FXS(chid))
		{
			// fxs
			rtk_SetFlashHookTime(chid, 100, 300);		// 100~300 ms
			rtk_SetDtmfCidParam(chid, DTMF_DIGIT_A, DTMF_DIGIT_C, 0xf, 80, 80, 300, 300);	
			// priori 1st ring, auto AC, auto ring, digit duration = 80ms, inter digit pause = 80ms
		}
		else
		{
			// fxo
			rtk_SetFxoRingDetection(300, 300, 4500);
			rtk_SetCidDetMode(chid,
				2,										// auto detect mode (NOT NTT)
				CID_DTMF
			);
		}

		// set dtmf mode
		rtk_SetDtmfMode(chid, DTMF_INBAND);

		// set fax modem det mode
		//rtk_SetFaxModemDet(chid, 0); //0:auto-hi-speed-fax
		rtk_SetFaxModemDet(chid, 3); //3:auto_2

		// set fax event detection
		//rtk_SetAnswerToneDet(chid , 0x31212|0x3c0000 , 0 , 10);
		rtk_SetAnswerToneDet(chid , 0x3f1212 , 0 , 10);

		// flush kernel fifo before app run
		rtk_SetFlushFifo(chid);

		// init local data
		channel_init(&channels[chid], chid);

		rtk_OnHookReinit(chid);

		// flush all voip events 
		stFlushVoipEvent.ch_id = chid;
		stFlushVoipEvent.type = VET_ALL;
		stFlushVoipEvent.mask = VEM_ALL;
		rtk_FlushVoIPEvent( &stFlushVoipEvent );
	}

	#ifdef DEBUG_FXO
	printf( "Wait %d seconds...", SELECT_WAIT_SEC );
	fflush( stdout );
	#endif

	// main loop
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

		// fill input data & get it 
		for ( chid = 0 ; chid <CON_CH_NUM ; chid ++ )
		{
			// read all events
			for ( i = 0 ; ; i ++ )
			{
			
			#if defined(USE_TAPI_RTKGETFAXMDEMEVT)	/* check fax modem event */
				channel_t *channel;
				channel = &channels[chid];
				VoipEventID fax_evt;


				rtk_GetFaxModemEvent(chid, &channel->fax_type, &fax_evt , 0);
				
				if( fax_evt != VEID_NONE)
					printf("FaxModemEvent ch=%d fax_type=%u fax_clear=%d fax_evt=%s\n",
						chid , channel->fax_type , channel->fax_clear ,get_evt_name(fax_evt));
				
				//1:low-speed fax, 2: high-speed fax, 3:local-modem, 4: remote-modem
				if ((channel->fax_type == 1) || (channel->fax_type == 2))
				{
					do_fax_event( chid, fax_evt );
					continue;
				}
				check_fax_event( chid , fax_evt );
			#endif				
			
				memset ((void*)&stVoipEvent, 0, sizeof(TstVoipEvent));
				stVoipEvent.ch_id = chid;
				stVoipEvent.type = VET_ALL;
				stVoipEvent.mask = VEM_ALL;
				
				rtk_GetVoIPEvent( &stVoipEvent );
				
				// no more data 
				if( stVoipEvent.id == VEID_NONE )
					break;

				// print out 
				#ifdef DEBUG_FXO
				printf( "[%d] on chid=%d\n", i, stVoipEvent.ch_id );
				printf( "\tID=%s(%08X), p0=%u p1=%u @%08X\n", get_evt_name(stVoipEvent.id), stVoipEvent.id, 
						stVoipEvent.p0, stVoipEvent.p1, stVoipEvent.time );
				#endif


			#if !defined(USE_TAPI_RTKGETFAXMDEMEVT)
				if ( stVoipEvent.id & VET_FAXMDM )
				{
					do_fax_event( chid, stVoipEvent.id );
					check_fax_event( chid , stVoipEvent.id );
					continue;
				}
			#endif				

				if (CHANNEL_IS_FXS(chid))
					rtk_ProcessFxsEvent( chid, stVoipEvent.id, &val );
				else
					rtk_ProcessFxoEvent( chid, stVoipEvent.id, &val );

				do_fxo_event( stVoipEvent.ch_id, val );
			}
		}
	} /* while(1) main loop */

	return 0;
}

int do_fxo_event( int chid, SIGNSTATE val )
{
	channel_t *channel;
	channel = &channels[chid];

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
		if ( channel->state == STATE_IDLE )
		{
			channel_handle_input(channel, val);
		}
		break;
	case SIGN_ONHOOK:
		/* when phone onhook, stop play tone, disable pcm and DTMF detection */
		//rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, RTK_DISABLE, DSPCODEC_TONEDIRECTION_LOCAL);
		//usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!

		channel_handle_onhook(channel);
		rtk_OnHookAction(chid);	// rtk_OnHookAction have to be the last step in SIGN_ONHOOK

		channel->fax_clear = 1;
		channel->fax_end_clear = 1;
		break;
	case SIGN_OFFHOOK:
		rtk_OffHookAction(chid);	// rtk_OffHookAction have to be the first step in SIGN_OFFHOOK
		channel_handle_offhook(channel);
		break;
	case SIGN_RING_ON:		// virtual daa ring on
		if (channel->state == STATE_IDLE)
		{
			channel->state = STATE_DAA_RING;
			rtk_SetRingFxs(channel->chid, 1);
		}
		break;
	case SIGN_RING_OFF: 	// virtual daa ring off
		if (channel->state == STATE_DAA_RING)
		{
			rtk_SetRingFxs(channel->chid, 0);
			channel->state = STATE_IDLE;
		}
		break;
	case SIGN_FLASHHOOK:
		break;
	case SIGN_OFFHOOK_2: // off-hook but no event
		break;
	case SIGN_NONE:
		break;
	default:
		printf("unknown event(chid:%d, sign:0x%08X)\n", chid, val);
		break;
	}

	return 0;
}

int do_fax_event( int chid, VoipEventID evt )
{
	channel_t *channel;
	channel = &channels[chid];

	// Thlin add: once detect CED, change codec to T.38
	/*************** FAX Det ****************/
	if (CHANNEL_IS_FXS(chid))
	{
		//rtk_GetFaxModemEvent(chid, &channel->fax_type, NULL, 0);
		//printf("%s(%d) ch=%d fax_type=%u fax_clear=%d evt=%s\n", __FUNCTION__ , __LINE__ , chid , channel->fax_type , channel->fax_clear ,get_evt_name(evt));
		
		//1:low-speed fax, 2: high-speed fax, 3:local-modem, 4: remote-modem
		//if (((channel->fax_type == 1) || (channel->fax_type == 2)) && (channel->fax_clear == 1) )
		
		if (channel->fax_clear == 1)
		{
			if( fax_use_t38 ){
				channel_resetup_local_t38(channel);
				channel_resetup_local_t38(channel->remote);
			}
			channel->fax_clear = 0;
		}
	}


	return 0;
}

int check_fax_event(int chid, VoipEventID evt )
{
	channel_t *channel;
	channel = &channels[chid];

	uint32 dis = 0;
	
	dis = 0;
	rtk_GetFaxDisRxDetect(chid, &dis); 
	if (dis == 1){
		printf("Get FAX DIS RX event, ch=%d\n", chid);
	}
	
	dis = 0;				
	rtk_GetFaxDisTxDetect(chid, &dis); 
	if (dis == 1){
		printf("Get FAX DIS TX event, ch=%d\n", chid);
	}
	
	rtk_GetFaxEndDetect(chid, &channel->fax_end); 
	if (channel->fax_end == 1){
		printf("Get FAX End event, ch=%d\n", chid);
		channel->fax_end_clear = 0;
	}

	return 0;
}


#ifdef DEBUG_FXO
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
		default:
			return("unknown");
	}

	return("unknown");

}
#endif

