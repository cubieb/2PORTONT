
//
// rtpterm.cpp
//

//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/interrupt.h>
//#include <string.h>
#include "rtpterm.h"
#include "rtpTypes.h"
#include "Rtp.h"
#include "../dsp_r0/dspcodec_0.h"
#include "../dsp_r0/dspparam.h"
#include "dsp_main.h"
//#include "Slic_api.h"
#include "snd_define.h"
#include "voip_control.h"
#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_params.h"

#if defined (PULSE_DIAL_GEN) && defined (OUTBAND_AUTO_PULSE_DIAL_GEN)
//#include "Daa_api.h"
#include "snd_mux_daa.h"
extern int pulse_dial_in_dch(uint32 ch_id, char input);
#endif

//#include <debug.h>

#include "codec_descriptor.h"

#include "../voip_manager/voip_mgr_events.h"

///////////////////////////////////////////////////////////////////
// global variable
int m_nSIDFrameLen[MAX_DSP_RTK_SS_NUM];                                           // the length of SID frame
RtpSessionState sessionState[MAX_DSP_RTK_SS_NUM];
RtpSessionError sessionError[MAX_DSP_RTK_SS_NUM];
#ifdef SUPPORT_RTCP
unsigned char RtcpOpen[MAX_DSP_RTK_SS_NUM] = {0}; /* if RtcpOpen[sid] =1, it means Rtcp session sid if open. else if 0, close.*/
unsigned char Rtcp_Bye[MAX_DSP_RTK_SS_NUM] = {0};
#endif
unsigned char RtpOpen[MAX_DSP_RTK_SS_NUM] = {0}; /* if RtpOpen[sid] =1, it means Rtp session sid if open. else if 0, close.*/
///////////////////////////////////////////////////////////////////
// static variable
//static CRtpConfig m_xConfig[MAX_SESS_NUM];		// the configuration
//CRtpConfig m_xConfig[MAX_DSP_RTK_SS_NUM];
#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
static int m_nFrameLen[MAX_DSP_RTK_SS_NUM];	// always 0. temporal modification		// the length of frames
static int m_nFrameNum[MAX_DSP_RTK_SS_NUM];	// always 0. temporal modification		// the num of frames
static int m_nFramePerPacket[MAX_DSP_RTK_SS_NUM];	// always 1. // the max num of frames packed into a packet
#endif
/*static int m_nSIDFrameLen[MAX_SESS_NUM];  */		// the length of SID frame
static BOOL m_bSilenceState[MAX_DSP_RTK_SS_NUM];		// true if in silence state
#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
static char m_aFrameBuffer[MAX_DSP_RTK_SS_NUM][512];		// the buffer to put frames
#endif

/**** For playing tone while receiving 2833 packet *****/
BOOL m_bPlayTone[MAX_DSP_RTK_SS_NUM] = {0};			// the flag indicates playing tone
BOOL m_bFlashEvent[MAX_DSP_RTK_SS_NUM] = {0};
BOOL check_2833_stop[MAX_DSP_RTK_SS_NUM] = {0};
BOOL get_2833_edge[MAX_DSP_RTK_SS_NUM]={0};
unsigned short play_2833_time_cnt[MAX_DSP_RTK_SS_NUM]={0};	// must be unsigned short.
int play_2833_timeout_cnt[MAX_DSP_RTK_SS_NUM]={0};
uint32 m_uTone[MAX_DSP_RTK_SS_NUM];				// the event
static uint32 m_uTimestamp[MAX_DSP_RTK_SS_NUM];		// the timestamp of the event
//static int m_nCount[MAX_DSP_RTK_SS_NUM];			// how many packets received
//static uint32 session;
extern unsigned char rfc2833_dtmf_pt_local[MAX_DSP_SS_NUM];	// DTMF RFC2833 payload type, move to dsp_define.c
extern unsigned char rfc2833_fax_modem_pt_local[MAX_DSP_SS_NUM];	// Fax/Modem RFC2833 payload type
#ifdef RFC2833TRUNKEVENT
extern unsigned char rfc2833_trunk_pt_local[MAX_DSP_SS_NUM]; // RFC2833 trunk event
extern int RFC2833TrunkEventIn(int chid, int event);
#endif
extern void DspcodecWriteSkipSeqNo( uint32 sid, uint32 nSeq );

uint32 nTxSilencePacket[MAX_DSP_RTK_SS_NUM];

#ifdef SUPPORT_RTP_REDUNDANT
static uint16 RtpRedundantPT_local[ DSP_RTK_SS_NUM ];		// local accept PT
static uint16 RtpRedundantPT_remote[ DSP_RTK_SS_NUM ];	// remote accept PT 
uint32 RtpRedundantTimestamp_local[ DSP_RTK_SS_NUM ];
#endif

uint16 SID_payload_type_local[ DSP_RTK_SS_NUM ];
uint16 SID_payload_type_remote[ DSP_RTK_SS_NUM ];
uint32 SID_count_tx[ DSP_RTK_SS_NUM ];
uint32 SID_count_rx[ DSP_RTK_SS_NUM ];

uint32 fResume_plc_wsola_later[MAX_DSP_RTK_SS_NUM] = {0};
uint32 resume_cnt[MAX_DSP_RTK_SS_NUM]={0};

// Support FaxModem RTP Removal Flag
static uint32 Support_FaxModem_RFC2833RxPlay[MAX_DSP_RTK_CH_NUM] = {0};

int SetFaxModem_RFC2833RxPlay(uint32 chid, uint32 flag)
{
	Support_FaxModem_RFC2833RxPlay[chid] = flag;
	return 0;
}

int GetFaxModem_RFC2833RxPlay(uint32 chid)
{
	return Support_FaxModem_RFC2833RxPlay[chid];
}

#ifdef SUPPORT_RTP_REDUNDANT	
uint32 GetRtpRedundantStatus( uint32 sid )
{
	return RtpRedundantPT_local[ sid ];
}
#endif

#ifdef SUPPORT_RTP_REDUNDANT	
RtpPayloadType GetRtpRedundantPayloadType( uint32 sid, int local )
{
	if( local )
		return ( RtpPayloadType )RtpRedundantPT_local[ sid ];
		
	return ( RtpPayloadType )RtpRedundantPT_remote[ sid ];
}
#endif

void ResetSessionTxStatistics( uint32 sid )
{
	nTxSilencePacket[ sid ] = 0;
}

void RFC2833_receiver_init(uint32 sid)
{
	extern void flush_rfc2833_event_fifo(uint32 s_id);
	extern int outband_dmtf_play_state[];
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	extern uint32 fResume_plc_wsola_later[];
	extern void rfc2833_play_reset(int sid);
	
	unsigned int flags;
	int chid;
	chid = chanInfo_GetChannelbySession(sid);

	save_flags(flags);cli();

	m_bPlayTone[sid] = false;
	check_2833_stop[sid] = false;
	get_2833_edge[sid] = false;
	play_2833_time_cnt[sid] = 0;
	play_2833_timeout_cnt[sid] = 0;	

	flush_rfc2833_event_fifo(sid);
	voip_event_flush_rfc2833_fifo(chid, sid);
	
	if (outband_dmtf_play_state[sid] == 1)
	{
		hc_SetPlayTone(chid, sid, DSPCODEC_TONE_0_CONT, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		outband_dmtf_play_state[sid] = 0;
	}
	
	fResume_plc_wsola_later[sid] = 0;
	rfc2833_play_reset(sid);
	
	restore_flags(flags);
}

//
// CRtpTerminal - constructor/destructor
//
//static char m_pBuf[1600];
//unsigned int m_uBufSize;

void CRtpTerminal_Init(uint32 sid, CRtpConfig *pConfig)
{
	const codec_payload_desc_t *pTranCodecPayloadDesc;

	//m_xConfig[sid] = *pConfig;
//	m_uBufSize = 1600;

#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
	m_nFrameLen[sid] = 0;
	m_nFrameNum[sid] = 0;
#endif

	pTranCodecPayloadDesc = GetCodecPayloadDesc( pConfig ->m_uTranPktFormat );
	
	if( pTranCodecPayloadDesc )
	{
#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
		m_nFramePerPacket[sid] = _idiv32(m_xConfig[sid].m_nTranFrameRate, 
										 pTranCodecPayloadDesc ->nTranFrameRate );
#endif
		m_nSIDFrameLen[sid] = pTranCodecPayloadDesc ->nSidTxFrameBytes;
 #ifdef SUPPORT_RTCP
		//m_xConfig[sid].m_nPeriod = m_nFramePerPacket[sid] * 
		//						   pTranCodecPayloadDesc ->nFramePeriod;
 #endif		
	}
	else
	{
		printk("[%s] Unknown Tran frame type %d\n", __FUNCTION__, pConfig ->m_uTranPktFormat);
		assert(0);
	}

	m_bSilenceState[sid] = FALSE;
	m_bPlayTone[sid] = FALSE;
	//m_nCount[sid] = 0;

	//m_xConfig[sid].m_uTRMode = rtp_session_sendrecv;

#ifdef SUPPORT_RTCP
	RtpOpen[sid] = 0;
	RtcpOpen[sid] = 0;
#endif
	//sessionState[sid] = rtp_session_sendrecv;

	RtpRx_setFormat(sid, pConfig ->m_uRecvPktFormat, pConfig ->m_nRecvFrameRate);
	RtpTx_setFormat(sid, pConfig ->m_uTranPktFormat, pConfig ->m_nTranFrameRate);
	
	RtpSession_setSessionState(sid, pConfig ->m_uTRMode);
	
	RFC2833_receiver_init(sid);

#ifdef SUPPORT_RTP_REDUNDANT
	RtpRedundantPT_local[ sid ] = 0;
	RtpRedundantPT_remote[ sid ] = 0;
	RtpRedundantTimestamp_local[ sid ] = 0;
#endif
}

static int RtpTerminal_Read_ProcessRFC2833( uint32 chid, uint32 sid, 
							RtpSeqNumber seqNo,
							RtpTime timestamp,
							RtpEventDTMFRFC2833 *eventPayload )
{
	extern int rfc2833_event_fifo_wrtie(uint32 s_id, unsigned int event);
	extern void Update_current_event_fifo_state(uint32 s_id, RtpEventDTMFRFC2833* pEvent);
	extern int rfc2833_event_fifo_read(uint32 s_id);
	extern int Is_DAA_Channel(int chid);

#if defined (PULSE_DIAL_GEN) && defined (OUTBAND_AUTO_PULSE_DIAL_GEN)
	static char into_pulse_fifo[DSP_RTK_CH_NUM] = {0};
	static char event_in_pulse_fifo[DSP_RTK_CH_NUM] = {0};
#endif

	if( (!m_bPlayTone[sid]) && (!m_bFlashEvent[sid]))	/* no playtone */
	{
		DspcodecWriteSkipSeqNo(sid, seqNo);
		
		if(!eventPayload->edge)	/* not edge */
		{
			if ( (eventPayload->event < 0) ||
				((eventPayload->event >16) && (eventPayload->event <32))
					|| (eventPayload->event > 49))
			{
#ifdef RFC2833TRUNKEVENT			
				if ((eventPayload->event >= RFC2833TrunkEventBeginID ) && (eventPayload->event <= RFC2833TrunkEventEndID))
					RFC2833TrunkEventIn(chid, eventPayload->event);
				else
#endif				
				{			
					printk(AC_FORE_RED "Receive unrecognized RFC2833 event %d\n" AC_RESET, eventPayload->event);
				}
			}
			else	/* get timestamp & DTMF digit then playtone */
			{						
#if defined (PULSE_DIAL_GEN) && defined (OUTBAND_AUTO_PULSE_DIAL_GEN)
				if ( (1 == DAA_Get_Dial_Mode(chid)) && ( 1 == Is_DAA_Channel(chid)) )
				{
					if ( (into_pulse_fifo[chid] == 0) || (event_in_pulse_fifo[chid] != eventPayload->event) )
					{
						pulse_dial_in_dch(chid, eventPayload->event);
						into_pulse_fifo[chid] = 1;
						event_in_pulse_fifo[chid] = eventPayload->event;
						//printk("in\n");
					}
				}
				else
#endif
				{
					if ( 16 == eventPayload->event )// flash event
					{
						m_bFlashEvent[sid] = true;
						voip_event_hook_in( chid, VEID_HOOK_OUTBAND_FLASH_EVENT );
						//PRINT_G("hook_in(%d, OUTBAND_FLASH_EVENT)\n", chid);
					}
					else
					{
						rfc2833_event_fifo_wrtie(sid, eventPayload->event);
						Update_current_event_fifo_state(sid, eventPayload);
						m_bPlayTone[sid] = true;
					}
					m_uTimestamp[sid] = timestamp;
					m_uTone[sid] = eventPayload->event;
					
					//printk("Start(sid=%d)\n", sid);
					PRINT_MSG("Get 2833 DTMF Event:%d (sid=%d)\n", eventPayload->event, sid);

					voip_event_rfc2833_in( chid, sid, eventPayload->event );
				}

			}
		}
#if defined (PULSE_DIAL_GEN) && defined (OUTBAND_AUTO_PULSE_DIAL_GEN)
		else
		{
			//printk("edge(sid=%d)\n", sid);
			into_pulse_fifo[chid] = 0;
		}
#endif
	}
	else	/* playing tone */
	{
		DspcodecWriteSkipSeqNo(sid, seqNo);
		
		if(timestamp == m_uTimestamp[sid])
		{
			if(eventPayload->edge)
			{
				if ( 16 == eventPayload->event )// flash event
					m_bFlashEvent[sid] = false;
				else
				{
					Update_current_event_fifo_state(sid, eventPayload);
					get_2833_edge[sid] = true;
					m_bPlayTone[sid] = false;
				}
				//printk("edge(sid=%d)\n", sid);
			}
			else
			{
				if ( 16 != eventPayload->event )
				Update_current_event_fifo_state(sid, eventPayload);
			}
		}
		else /* get another event, getRtpTime() != m_uTimestamp */
		{
			if ( (eventPayload->event < 0) ||((eventPayload->event >16) && 
				(eventPayload->event <32))|| (eventPayload->event > 49))
			{
				rfc2833_event_fifo_read(sid);
				m_bFlashEvent[sid] = false;
				m_bPlayTone[sid] = false;
				printk(AC_FORE_RED "Receive unrecognized RFC2833 event %d\n" AC_RESET, eventPayload->event);
			}
			else
			{	/* Thlin: Should not go to here*/
				rfc2833_event_fifo_read(sid);
				m_bPlayTone[sid] = false;
				printk(AC_FORE_RED "RFC2833 receive error, sid=%d, event=%d\n" AC_RESET, sid, eventPayload->event);
			
				// Get diff timestamp 2833 event, stop previous DTMF tone, and then accept this diff timestamp 2833 event as new one.
				if ( 16 == eventPayload->event )// flash event
					m_bFlashEvent[sid] = true;
				else
				{		
					rfc2833_event_fifo_wrtie(sid, eventPayload->event);
					Update_current_event_fifo_state(sid, eventPayload);
					m_bPlayTone[sid] = true;
				}
				m_uTimestamp[sid] = timestamp;
				m_uTone[sid] = eventPayload->event;
				//printk(AC_FORE_RED "timestamp(sid=%d)= %u\n" AC_RESET, sid, m_uTimestamp[sid]);
				//printk(AC_FORE_RED "get pkt timestamp= %u\n" AC_RESET, getRtpTime(p));
			}
		}
	}

	return 0;
}		

#ifdef SUPPORT_RTP_REDUNDANT
static int32 RtpTerminal_Read_ProcessRedundant( uint32 chid, uint32 sid, 
			uint8* pBuf, int32 nSize, 
			RtpSeqNumber seqNo, RtpTime timestamp )
{
	uint8 *pSrcHeader, *pSrcPayload;
	int32 len, count, total_len;
	RtpEventDTMFRFC2833 eventpayload;
	
	if( !GetRtpRedundantStatus( sid ) )
		return 0;
	
	// check whether payload format is valid 
	pSrcHeader = pBuf; total_len = 0; count = 0;
	
	while( ( ( RtpRedHeader * )pSrcHeader ) ->F == 1 ) {
		len = ( ( RtpRedHeader * )pSrcHeader ) ->block_length;
		total_len += ( len + sizeof( RtpRedHeader ) );
		count ++;
		pSrcHeader += sizeof( RtpRedHeader );
	}
	
	total_len += sizeof( RtpRedPrimaryHeader );
	
	if( total_len >= nSize ) {
		printk( "Red2833: total_len=%d, packet_len=%d\n", total_len, nSize );
		return 0;
	}
	
	// point to primary block 
	//pSrcHeader = pSrcHeader;
	memcpy(&eventpayload, pBuf + total_len, sizeof(RtpEventDTMFRFC2833));
	//pSrcPayload = pBuf + total_len;
	pSrcPayload = &eventpayload;
	
	// ignore PT == 0
	if( ( ( RtpRedPrimaryHeader * )pSrcHeader ) ->blockPT == 0 )
		return 1;
	
	// The primary encoding block header is placed last in the packet. 
	if( ( ( ( RtpRedPrimaryHeader * )pSrcHeader ) ->blockPT == rfc2833_dtmf_pt_local[sid]) 
		|| ( ( ( RtpRedPrimaryHeader * )pSrcHeader ) ->blockPT == rfc2833_fax_modem_pt_local[sid]))
	{
		if( nSize - total_len != sizeof( RtpEventDTMFRFC2833 ) ) {
			printk( "Red2833: %d - %d != %d\n", nSize, total_len, sizeof( RtpEventDTMFRFC2833 ) );
		}
		
		RtpTerminal_Read_ProcessRFC2833( chid, sid,
				seqNo, timestamp, 
				( RtpEventDTMFRFC2833 * )pSrcPayload );
	}
	
	//printk( ")" );
	
	return 1;
}
#endif		

//
// CRtpTerminal - public interface
//

/* 
We get the data by the following layout:
 +---------+
 |         |
 |  Data   |
 |         |
 +---------+
 | Seq No. |
 +---------+
 |timestamp|
 +---------+
 */

int32 RtpTerminal_Read(uint32* pchid, uint32* psid, uint8 *pBuf, int32 nSize)
{
	RtpPacket *p = NULL;
	int32 nGet = 0, nPos;
	uint32 chid, sid;
	RtpSeqNumber seqNo;
	RtpEventDTMFRFC2833 *p2833;
	extern void InitAP(uint32 sid);
	extern int jbc_disable_wsola[MAX_DSP_RTK_SS_NUM];
	extern Flag fUsePLC[MAX_DSP_RTK_SS_NUM]; 

	//if(++session >= DSP_RTK_SS_NUM)
	//	session = 0;

	assert(pBuf);
	p = Rtp_receive();

	if(p)
	{
		*pchid = p->chid;	/* set receive channel id */
		*psid = p->sid;
		chid = *pchid;
		sid = *psid;
		seqNo = getSequence(p);

#ifdef SUPPORT_RTP_REDUNDANT
		// it may need further process after redundancy process, so
		// we don't break follow checking 
		if((getPayloadType(p) == GetRtpRedundantPayloadType( sid, 1 /* local */ )) 
		  && (rfc2833_dtmf_pt_local[sid] || rfc2833_fax_modem_pt_local[sid]))
		{
			// because we process merely RFC2833 redundancy here!! 
			RtpTerminal_Read_ProcessRedundant( chid, sid, 
							getPayloadLoc(p),
							getPayloadUsage(p),
							getSequence(p),
							getRtpTime(p)
							 );
		}
#endif

		//printk("%d\n", rfc2833_payload_type_local[sid]);
		if(((getPayloadType(p) == rfc2833_dtmf_pt_local[sid]) && (rfc2833_dtmf_pt_local[sid])!= 0)
			|| ((getPayloadType(p) == rfc2833_fax_modem_pt_local[sid]) && (rfc2833_fax_modem_pt_local[sid])!= 0)
#ifdef RFC2833TRUNKEVENT			
			|| ((getPayloadType(p) == rfc2833_trunk_pt_local[sid]) && (rfc2833_trunk_pt_local[sid])!= 0)
#endif			
			)
		{
			RtpTerminal_Read_ProcessRFC2833( chid, sid, 
						seqNo,
						getRtpTime(p),
						(RtpEventDTMFRFC2833*)(getPayloadLoc(p)) );

			p2833 = (RtpEventDTMFRFC2833*)(getPayloadLoc(p));

#if 1
			if (p->header->marker == 1)
			{
				if (fResume_plc_wsola_later[sid] == 0)
				{
					InitAP(sid);
					jbc_disable_wsola[sid] = 1;
					fUsePLC[sid] = 0;
					//PRINT_R( "0");
				}
				
				if (fResume_plc_wsola_later[sid] == 1)
				{
					fResume_plc_wsola_later[sid] = 0;
					resume_cnt[sid] = 0;
				}
			}
			
			if (p2833->edge == 1)
			{
				fResume_plc_wsola_later[sid] = 1;
			}
#endif
			
#if 1
			/* invalid length to distinguish that it is an event packet */
			nGet = -1;
#else
			
			nGet = 160;
			memset(pBuf, 0, nGet);

			if(nGet & 0x00000003)		/* not align */
				nPos = 4 - (nGet & 0x00000003) + nGet;
			else
				nPos = nGet;

			*((uint32 *)(pBuf + nPos)) = getSequence(p);
						//PRINT_R( "%d ", getSequence(p));
			*((uint32 *)(pBuf + nPos + 4)) = getRtpTime(p) + p2833->duration;
						//PRINT_G( "%x ", getRtpTime(p));
#endif
		}
		else	/* non RFC2833 packet */
		{
				if (fResume_plc_wsola_later[sid] == 1)
				{
					resume_cnt[sid]++;
					
					if (resume_cnt[sid] > 60) // 60 RTP packets.
					{
						jbc_disable_wsola[sid] = nDspSessCfgBak[sid].jbc_disable_wsola_bak;
						InitAP(sid);
						fUsePLC[sid] = nDspSessCfgBak[sid].fUsePLC_bak;
						//PRINT_R( "1");
						
						fResume_plc_wsola_later[sid] = 0;
						resume_cnt[sid] = 0;
					}
				}

				if(nSize >= getPayloadUsage(p) + 8)
					nGet = getPayloadUsage(p);
				else
					assert(0);

				memcpy(pBuf, getPayloadLoc(p), nGet);

#if 1
				// remove padding bytes 
				if( getPaddingFlag( p ) ) {
					nGet -= pBuf[ nGet - 1 ];
					
					if( nGet <= 0 ) {
						nGet = 0;
						printk( "PZ " );
					} 
				}
#endif

				if(nGet & 0x00000003)		/* not align */
					nPos = 4 - (nGet & 0x00000003) + nGet;
				else
					nPos = nGet;
				*((uint32 *)(pBuf + nPos)) = getSequence(p);
				*((uint32 *)(pBuf + nPos + 4)) = getRtpTime(p);
		}
		p->own = OWNBYDSP;
	}

	// move to DSP 10ms timer 
//#ifdef SUPPORT_RTCP
//	//printk("--- RtcpOpen[%d] = %d --- \n", session, RtcpOpen[session]);
//	if(RtcpOpen[session] == 1)	//thlin: if RtcpOpen =1 , do RtpSession_processRTCP, eles NOT.
//	{
//		RtpSession_processRTCP(session);
//	}
//#endif
	return nGet;
}

int32 RtpTerminal_Write(uint32 chid, uint32 sid, uint8 *pBuf, int32 nSize)
{
	extern unsigned int check_answer_on(unsigned int chid);
	extern int get_2833_send_flag(uint32 sid);
	extern int get_2833_three_end(uint32 sid);
	extern unsigned char rfc2833_dtmf_pt_local[];
	extern unsigned char rfc2833_dtmf_pt_remote[];
	extern unsigned char rfc2833_fax_modem_pt_local[];
	extern unsigned char rfc2833_fax_modem_pt_remote[];
	
	// Check if in outband RFC2833 mode
	if ((rfc2833_dtmf_pt_local[sid]!=0 && rfc2833_dtmf_pt_remote[sid]!=0)
		|| (rfc2833_fax_modem_pt_local[sid]!=0 && rfc2833_fax_modem_pt_remote[sid]!=0))
	{
		// RFC2833 packet is sending, and not end
		if ((get_2833_send_flag(sid) == 1) || (get_2833_three_end(sid) == 0))
		{
			//printk("%d-%d=", get_2833_send_flag(chid), get_2833_three_end(chid));
			return 0;
		}
		
		// Check if answer tone on, if it is, remove inband RTP.
		if (check_answer_on(chid) == 1)
		{
			//printk("*");
			return 0;
		}
	}

	/* no more frame */
	if(nSize == 0)
		return 0;

	/* silence period */
	else if(nSize == -1)	// invalid length to distinguish that it is silence
	{
		RtpTx_addTimestampOfOneFrame(sid);
#if 0	// pkshih: why?
 #ifndef __ECOS
		if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
			RtpTx_addTimestamp(sid);
 #endif
#endif
		return 0;
	}

	/* normal frame or SID frame */
	else
	{
		assert(pBuf);
//		ASSERT(pBuf);
		//send_data_in_ppp_format( 2 , "write rtpTerminal" , 17 ) ;		// Howard. 2004.12.8 for debug
		if(nSize == m_nSIDFrameLen[sid])	// SID frame
		{
			nTxSilencePacket[ sid ] ++;
			
			if(m_bSilenceState[sid])
			{
				/* RFC 3389 Real-time Transport Protocol (RTP) 
				 * Payload for Comfort Noise (CN)said:
 				 * The RTP packet SHOULD NOT have the marker bit set.
 				 */
				//RtpTx_setMarkerOnce(sid);
				
				RtpTx_transmitRaw(chid, sid, (char*)pBuf, (int)nSize);
				RtpTx_subTimestampIfNecessary( sid, ( char * )pBuf, nSize );
#if 0	// pkshih: why??
 #ifndef __ECOS
				if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
					RtpTx_addTimestamp(sid);
 #endif
				if((m_xConfig[sid].m_uPktFormat == rtpPayloadPCMU) || (m_xConfig[sid].m_uPktFormat == rtpPayloadPCMA))
					RtpTx_addTimestamp(sid);
#endif
			}
			else
			{
#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
				if(m_nFrameLen[sid] != 0)
				{	// always false ( 0 != 0 )
					RtpTx_subTimestamp(sid);
					RtpTx_transmitRaw(chid, sid, m_aFrameBuffer[sid], m_nFrameLen[sid]);
					RtpTx_subTimestampIfNecessary( sid, m_aFrameBuffer[sid], m_nFrameLen[sid] );
					m_nFrameLen[sid] = 0;
					m_nFrameNum[sid] = 0;
				}
#endif
				RtpTx_transmitRaw(chid, sid, (char*)pBuf, (int)nSize);
				RtpTx_subTimestampIfNecessary( sid, ( char * )pBuf, nSize );
#if 0	// pkshih: why??
 #ifndef __ECOS
				if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
					RtpTx_addTimestamp(sid);
 #endif
				if((m_xConfig[sid].m_uPktFormat == rtpPayloadPCMU) || (m_xConfig[sid].m_uPktFormat == rtpPayloadPCMA))
					RtpTx_addTimestamp(sid);
#endif
				m_bSilenceState[sid] = true;
			}
		}
		else	// normal frame
		{
			if(m_bSilenceState[sid])
			{
				RtpTx_setMarkerOnce(sid);
				m_bSilenceState[sid] = false;
			}
			
#ifndef UNUSED_RTP_TX_FRAME_PER_PACKET
			RtpTx_transmitRaw(chid, sid, pBuf, nSize);
			RtpTx_subTimestampIfNecessary( sid, pBuf, nSize );
#else
			memcpy(m_aFrameBuffer[sid]+m_nFrameLen[sid], pBuf, nSize);
			m_nFrameLen[sid] += nSize;
			m_nFrameNum[sid]++;
			if(m_nFrameNum[sid] == m_nFramePerPacket[sid])
			{	// always true ( 1 == 1 )
				RtpTx_transmitRaw(chid, sid, m_aFrameBuffer[sid], m_nFrameLen[sid]);
				RtpTx_subTimestampIfNecessary( sid, m_aFrameBuffer[sid], m_nFrameLen[sid] );
				m_nFrameLen[sid] = 0;
				m_nFrameNum[sid] = 0;
			}
			else
				RtpTx_addTimestamp(sid);
#endif
		}

		return nSize;
	}
}

void RtpTerminal_setFormat(uint32 sid, RtpPayloadType tranType, RtpPayloadType recvType, int tranRate, int recvRate)
{
	RtpTx_setFormat(sid, tranType, tranRate);
	RtpRx_setFormat(sid, recvType, recvRate);
}

/* The very function that used to config endpoint */
void RtpTerminal_SetConfig(uint32 chid, uint32 sid, CRtpConfig *pRtpConfig)
{
	//bool rtpConfig_changed = false;
	//bool rtcpConfig_changed = false;
	//CRtpConfig* pRtpConfig = NULL;
	const codec_payload_desc_t *pTranCodecPayloadDesc;

	assert(pRtpConfig);
	//pRtpConfig = (CRtpConfig *)pConfig;

#if 0
//#ifdef SUPPORT_RTCP

	/* RTCP part */
	if(m_xConfig[sid].m_nRtcpRemotePort != pRtpConfig->m_nRtcpRemotePort)
	{
		rtcpConfig_changed = true;
		m_xConfig[sid].m_nRtcpRemotePort = pRtpConfig->m_nRtcpRemotePort;
	}
	if(m_xConfig[sid].m_nRtcpLocalPort != pRtpConfig->m_nRtcpLocalPort)
	{
		rtcpConfig_changed = true;
		m_xConfig[sid].m_nRtcpLocalPort = pRtpConfig->m_nRtcpLocalPort;
	}

if(rtcpConfig_changed==true)
	{
		if(RtpOpen[sid])	/* if Rtp is open, then can open and close Rtcp session*/
		{
			if((pRtpConfig->m_nRtcpRemotePort == 0) || (pRtpConfig->m_nRtcpLocalPort == 0))
			{
				/*
				   As our rule, we close the rtcp session,
				   when the Rtcp remote port or local port is zero.
				*/
				if(RtcpOpen[sid])
				{
					RtcpTx_transmitRTCPBYE(sid);
					rtcp_Session_UnConfig(chid, sid);
					RtcpOpen[sid] = 0;
				}
			}

		}
		else
		{
			/*
			   If rtp session is already closed,
			   we only need to close the flag and do not need to close rtcp session.
			   The rtcp session is already closed when rtp_session_inactive.
			*/
			if(RtcpOpen[sid])
				if((pRtpConfig->m_nRtcpRemotePort == 0) || (pRtpConfig->m_nRtcpLocalPort == 0))
					RtcpOpen[sid] = 0;
			return;
		}
	}
	/*-------------------------------------------------------------------------*/

	if (pRtpConfig->m_uTRMode == rtp_session_inactive)/*Wish to stop RTP transfer*/
	{
		if(RtpOpen[sid])
		{

			if(RtcpOpen[sid])
			{
				RtcpTx_transmitRTCPBYE(sid);
				rtcp_Session_UnConfig(chid, sid);
			}

			//rtp_Session_UnConfig(chid, sid);	/* clear voip port register by ROME driver function */


			RtpOpen[sid] = 0;
		}
		m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;
		RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);
		printk("\nRTP session stop on ch:%d system session:%d\n", chid, sid);
		DSP_CodecStop(chid, sid);
		return;
	}

	
#endif //by bruce
	//m_xConfig[sid].m_uRecvPktFormat = pRtpConfig->m_uRecvPktFormat;
	//m_xConfig[sid].m_uTranPktFormat = pRtpConfig->m_uTranPktFormat;
	//m_xConfig[sid].m_nRecvFrameRate = pRtpConfig->m_nRecvFrameRate;
	//m_xConfig[sid].m_nTranFrameRate = pRtpConfig->m_nTranFrameRate;
	//m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;

	RtpTerminal_setFormat( sid,	pRtpConfig ->m_uTranPktFormat, 
								pRtpConfig ->m_uRecvPktFormat, 
								pRtpConfig ->m_nTranFrameRate, 
								pRtpConfig ->m_nRecvFrameRate );

	RtpSession_setSessionState(sid, pRtpConfig ->m_uTRMode);

#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
	m_nFrameLen[sid] = 0;
	m_nFrameNum[sid] = 0;
#endif

	pTranCodecPayloadDesc = GetCodecPayloadDesc( pRtpConfig ->m_uTranPktFormat );
	
	if( pTranCodecPayloadDesc ) {
#ifdef UNUSED_RTP_TX_FRAME_PER_PACKET
		m_nFramePerPacket[sid] = _idiv32(m_xConfig[sid].m_nTranFrameRate, 
										 pTranCodecPayloadDesc ->nTranFrameRate );
#endif
		m_nSIDFrameLen[sid] = pTranCodecPayloadDesc ->nSidTxFrameBytes;
	} else {
		printk("[%s] Unknown frame type %d\n", __FUNCTION__, pRtpConfig ->m_uTranPktFormat);
		assert(0);
	}

	m_bSilenceState[sid] = false;
	m_bPlayTone[sid] = false;
	//m_nCount[sid] = 0;

	RtpRx_setFormat(sid, pRtpConfig ->m_uRecvPktFormat, pRtpConfig ->m_nRecvFrameRate);
	RtpTx_setFormat(sid, pRtpConfig ->m_uTranPktFormat, pRtpConfig ->m_nTranFrameRate);
#if 0
	if (pRtpConfig->m_uTRMode == rtp_session_inactive)
	{
		//rtp_Session_UnConfig(chid, sid);	// clear voip port register
//		chanInfo_CloseSessionID(chid, sid);

		m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;
		RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);

		m_xConfig[sid].m_nRtpRemoteIP = 0;
		m_xConfig[sid].m_nRtpRemotePort = 0;

		return;
	}
	
#endif
	if (pRtpConfig->m_uTRMode == rtp_session_inactive)
	{
		//m_xConfig[sid].m_nRtpRemoteIP = 0;
		//m_xConfig[sid].m_nRtpRemotePort = 0;
	}
}

/*+++++ add by Jack for set session state+++++*/
void RtpTerminal_SetSessionState(uint32 sid, uint32 state){
	//m_xConfig[sid].m_uTRMode = state;
	RtpSession_setSessionState(sid, state);

}
/*-----end-----*/
#if 0
void RtpTerminal_GetConfig(uint32 sid, void *pConfig)
{
	assert(pConfig);
	*((CRtpConfig *)pConfig) = m_xConfig[sid];
}
#endif

#ifdef SUPPORT_RFC_2833
unsigned char RtpTerminal_SendDTMFEvent(uint32 chid, uint32 sid, int32 nEvent, int duration)

{
	return RtpTx_transmitEvent(chid, sid, nEvent, duration);
}
#endif

#if 0
void RtpTerminal_TransmitData(void)
{
	// transmit data from source terminals to destination terminals
	// note:
	// the data multiplexing is done in target (sink) terminals rather than in stream
	//
	uint32 chid;
	int nRealSize;
	while (1)
	{
		nRealSize = RtpTerminal_Read(&chid, m_pBuf, m_uBufSize);
		DSP_Write(chid, m_pBuf, nRealSize);
		if(nRealSize == 0)
			break;
	}

	while(1)
	{
		nRealSize = DSP_Read(&chid, m_pBuf, m_uBufSize);
		RtpTerminal_Write(chid, m_pBuf, nRealSize);
		if(nRealSize == 0)
			break;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////
// for RTP session
void RtpSession_setSessionState (uint32 sid, RtpSessionState state)
{
	if( state == rtp_session_unchange )
		return;
	
	sessionState[sid] = state;

	switch(state)
	{
		case rtp_session_inactive:
			//PRINT_MSG("+++++Debug:go to rtp_session_inactive %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_droppacket);
			RtpTx_setMode(sid, rtptran_droppacket);
			RtcpTx_setMode(sid, rtptran_droppacket);
			break;
		case rtp_session_sendonly:
			//PRINT_MSG("+++++Debug:go to rtp_session_sendonly %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_droppacket);
			RtpTx_setMode(sid, rtptran_normal);
			RtcpTx_setMode(sid, rtptran_normal);
			break;
		case rtp_session_recvonly:
			//PRINT_MSG("+++++Debug:go to rtp_session_recvonly %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_normal);
			RtpTx_setMode(sid, rtptran_droppacket);
			RtcpTx_setMode(sid, rtprecv_normal);
			break;
		case rtp_session_sendrecv:
			//PRINT_MSG("+++++Debug:go to rtp_session_sendrecv %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_normal);
			RtpTx_setMode(sid, rtptran_normal);
			RtcpTx_setMode(sid, rtptran_normal);
			break;
		default:
			assert(0);
			break;
	}
}

void RtpSession_renew(uint32 sid, 
		const TstVoipMgrSession *rtpcfg, // RTP only 
		const TstVoipRtcpSession *rtcpcfg // RTCP only 
		)
{
	extern unsigned short rx_frames_per_packet[MAX_DSP_RTK_SS_NUM];
	
	if( sid >= DSP_RTK_SS_NUM ) 
		goto label_renew_rtp_session_done;

	if (rtpcfg == NULL)
	{
		PRINT_R("%s, line%d, error, rtpcfg = %p\n", __FUNCTION__, __LINE__, rtpcfg);
		return;
	}
	
	// RTP Redundancy 
#ifdef SUPPORT_RTP_REDUNDANT
	if( rtpcfg ->rtp_redundant_payload_type_local < 96 || 
		rtpcfg ->rtp_redundant_payload_type_remote < 96 ||
		rtpcfg ->rtp_redundant_payload_type_local > 127 || 
		rtpcfg ->rtp_redundant_payload_type_remote > 127 )
	{
		RtpRedundantPT_local[ sid ] = 0;
		RtpRedundantPT_remote[ sid ] = 0;
	} else {
		RtpRedundantPT_local[ sid ] = rtpcfg ->rtp_redundant_payload_type_local;
		RtpRedundantPT_remote[ sid ] = rtpcfg ->rtp_redundant_payload_type_remote;
	}
	
	RtpRedundantTimestamp_local[ sid ] = 0;
#endif

	// SID local and remote payload type are indenpendent 
	if( rtpcfg ->SID_payload_type_local < 96 || 
		rtpcfg ->SID_payload_type_local > 127 )
	{
		SID_payload_type_local[ sid ] = 0;
	} else
		SID_payload_type_local[ sid ] = rtpcfg ->SID_payload_type_local;
	
	if( rtpcfg ->SID_payload_type_remote < 96 || 
		rtpcfg ->SID_payload_type_remote > 127 )
	{
		SID_payload_type_remote[ sid ] = 0;
	} else
		SID_payload_type_remote[ sid ] = rtpcfg ->SID_payload_type_remote;
	
	SID_count_tx[ sid ] = SID_count_rx[ sid ] = 0;
	
	// RX frames per packet
	rx_frames_per_packet[ sid ] = 0;
	
#ifdef SUPPORT_RTP_REDUNDANT
	RtpTx_renewSession(sid, 
				rtpcfg ->init_randomly, rtpcfg ->init_SSRC,
				rtpcfg ->init_seqno, rtpcfg ->init_timestamp,
				rtpcfg ->rtp_redundant_max_Audio, rtpcfg ->rtp_redundant_max_RFC2833 );
#else
	RtpTx_renewSession(sid, 
				rtpcfg ->init_randomly, rtpcfg ->init_SSRC,
				rtpcfg ->init_seqno, rtpcfg ->init_timestamp);
#endif
	/*
	 * pkshih uncomment following. 
	 * Because sourceSet[] is set, updateSource() will try to change 
	 * source and drop packets in begining of session. 
	 * We guess that it affects voice quality only if VAD is on, 
	 * because voice frames are almost silence in begining.
	 * In other words, drop these silence frames will not affect 
	 * voice quality.
	 *
	 * WARNING: I don't know why someone comment it!!
	 */
	RtpRx_renewSession(sid);

	extern int Rfc2833EventTxBuf_Reset(sid);
	Rfc2833EventTxBuf_Reset(sid);

label_renew_rtp_session_done:

#ifdef SUPPORT_RTCP
	if( sid >= RTCP_SID_OFFSET ) {
		sid -= RTCP_SID_OFFSET;

		if (rtcpcfg == NULL)
		{
			PRINT_R("%s, line%d, error, rtcpcfg = %p\n", __FUNCTION__, __LINE__, rtcpcfg);
			return;
		}

		RtcpTx_InitByID( sid, rtcpcfg ->enableXR, rtcpcfg ->tx_interval );
		RtcpRx_InitByID( sid, rtcpcfg ->enableXR );
	}
#endif
}

#ifdef SUPPORT_RTCP
void RtpSession_processRTCP (uint32 sid)
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int32 rtk_trap_unregister(uint32 s_id);
#endif

	if (RtcpTx_checkIntervalRTCP(sid))
	{
		RtcpTx_transmitRTCP(sid);
		//printk("T%d\n", sid);
	}
	
	if (Rtcp_Bye[sid])
	{
		RtcpTx_transmitRTCPBYE(sid);
		Rtcp_Bye[sid] = 0;
		RtcpOpen[sid] = 0;
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		rtk_trap_unregister(sid+ RTCP_SID_OFFSET);
#endif
	}

	RtcpRx_receiveRTCP(sid);

    return ;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_rtpterm_read_proc(struct seq_file *f, void *v)
{
	//extern int g_dynamic_pt_remote_vbd[];
	//extern int g_dynamic_pt_local_vbd[];
	//extern TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[];
	extern int sprintf_redtx( uint32 sid, struct seq_file *f );
	int ss;
	
	if( IS_CH_PROC_DATA( v ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( v );
		
		seq_printf( f, "session=%d\n", ss );
#ifdef SUPPORT_RTP_REDUNDANT
		seq_printf( f, "RtpRedundantPT local/remote:\t%d/%d\n", RtpRedundantPT_local[ ss ], RtpRedundantPT_remote[ ss ] );
		seq_printf( f, "RtpRedundantTimestamp  local:\t%d\n", RtpRedundantTimestamp_local[ ss ] );
		sprintf_redtx( ss, f );
#endif
		seq_printf( f, "SID_payload_type local/remote:\t%d/%d\n", SID_payload_type_local[ ss ], SID_payload_type_remote[ ss ] );
		seq_printf( f, "SID_count rx/tx:\t%lu/%lu\n", SID_count_rx[ ss ], SID_count_tx[ ss ] );
	}

	return 0;
}

#else
static int voip_rtpterm_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	//extern int g_dynamic_pt_remote_vbd[];
	//extern int g_dynamic_pt_local_vbd[];
	//extern TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[];
	extern int sprintf_redtx( uint32 sid, char *buf );
	int ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		
		n = sprintf( buf, "session=%d\n", ss );
#ifdef SUPPORT_RTP_REDUNDANT
		n += sprintf( buf + n, "RtpRedundantPT local/remote:\t%d/%d\n", RtpRedundantPT_local[ ss ], RtpRedundantPT_remote[ ss ] );
		n += sprintf( buf + n, "RtpRedundantTimestamp  local:\t%d\n", RtpRedundantTimestamp_local[ ss ] );
		n += sprintf_redtx( ss, buf + n );
#endif
		n += sprintf( buf + n, "SID_payload_type local/remote:\t%d/%d\n", SID_payload_type_local[ ss ], SID_payload_type_remote[ ss ] );
		n += sprintf( buf + n, "SID_count rx/tx:\t%lu/%lu\n", SID_count_rx[ ss ], SID_count_tx[ ss ] );
	}
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int rtpterm_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_rtpterm_read_proc, NULL);
}

struct file_operations proc_rtpterm_fops = {
	.owner	= THIS_MODULE,
	.open	= rtpterm_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_rtpterm_read_proc
};
#endif


int __init voip_rtpterm_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	create_voip_session_proc_read_entry( "rtpterm", &proc_rtpterm_fops );
#else
	//create_voip_channel_proc_read_entry( "rtpterm", voip_rtpterm_read_proc );
	create_voip_session_proc_read_entry( "rtpterm", voip_rtpterm_read_proc );
#endif
	return 0;
}

void __exit voip_rtpterm_proc_exit( void )
{
	//remove_voip_channel_proc_entry( "rtpterm" );
	remove_voip_session_proc_entry( "rtpterm" );
}

voip_initcall_proc( voip_rtpterm_proc_init );
voip_exitcall( voip_rtpterm_proc_exit );

