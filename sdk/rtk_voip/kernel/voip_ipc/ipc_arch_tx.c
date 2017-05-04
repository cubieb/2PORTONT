//#include <linux/config.h>
#include <linux/string.h>
#include <linux/sched.h>	//schedule out
//#include <linux/interrupt.h>
#include "voip_types.h"
#include "voip_control.h"
#include "voip_ipc.h"
#include "voip_errno.h"
#include "voip_init.h"
#include "voip_limit.h"
#include "voip_dev.h"

#include "voip_mgr_transfer_id.h"

#include "ipc_arch_tx.h"

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_RESEND_CTRL
#define RESEND_THS	3
//#define RESEND_ERR	-3	// replaced by -EVOIP_RESEND_ERR
#endif


#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "con_mux.h"
//#include "../voip_rx/rtk_trap.h"
#include "ipc_arch_rx.h"
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#include "ipc_arch_help_dsp.h"
#include "../voip_dsp/rtp/RtpPacket.h"
#include "../voip_dsp/rtp/RtcpPacket.h"
#endif

#include "ipc_internal.h"

#ifdef IPC_ARCH_DEBUG_HOST
unsigned int host_ctrl_tx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_rtp_rtcp_tx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_t38_tx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_ack_tx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
#endif

#ifdef IPC_ARCH_DEBUG_DSP
unsigned int dsp_resp_tx_cnt = 0;
unsigned int dsp_rtp_rtcp_tx_cnt = 0;
unsigned int dsp_t38_tx_cnt = 0;
unsigned int dsp_event_tx_cnt = 0;
#endif

static const unsigned char voip_ver = 0x08;
static uint16 global_seq_no = 0;
static TstCheckResponse stCheckResponse;
static TstCheckAck stCheckAck;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
static unsigned int host_need_response( void )
{
	return ( ( TstCheckResponse volatile * )&stCheckResponse ) ->needResponse;
}

int host_check_response_and_fill_content( 
			uint16 category, uint16 seq_no,
			const unsigned char *content, uint16 cont_len,
			unsigned short dsp_id )
{
	if (( category == stCheckResponse.category) && (seq_no == stCheckResponse.seqNo))
	{
		stCheckResponse.needResponse = 0;
		
		// continue to fill content 
	} else {
		PRINT_R( "category %d %d, seq_no %d %d\n", 
					category, stCheckResponse.category,
					seq_no, stCheckResponse.seqNo );
		
		return -1;
	}
	
	if( dsp_id != stCheckResponse.cont_dsp_cpuid ) {
		PRINT_R( "dsp id %d %d\n", dsp_id, stCheckResponse.cont_dsp_cpuid );
	}
	
	if( cont_len == 0 ) {
		stCheckResponse.content = NULL;
		stCheckResponse.cont_len = 0;
		
		return 0;
	}

	if( cont_len != stCheckResponse.cont_len ) {
		if( stCheckResponse.category == VOIP_MGR_PLAY_IVR )
			;	// return header only 
		else
			PRINT_R( "cont_len %d %d\n", cont_len, stCheckResponse.cont_len );
	}
	
	if( !stCheckResponse.content )
		return 0;
	
	// copy to prepare buffer 
	memcpy( stCheckResponse.content, content, stCheckResponse.cont_len );
	
	return 0;
}
#endif

unsigned int dsp_need_ack( void )
{
	//return ( ( TstCheckAck volatile * )stCheckAck ) ->needAck;
	return 0;
}

static inline ipc_ctrl_pkt_t *__ipc_pkt_tx_allocate( unsigned int *pkt_len, 
											void **ipc_priv, uint8 protocol )
{
	ipc_ctrl_pkt_t *ipc_pkt;
	
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP	
	extern ipc_ctrl_pkt_t *ethernet_dsp_tx_allocate( unsigned int *pkt_len, 
											void **ipc_priv );
											
	ipc_pkt = ethernet_dsp_tx_allocate( pkt_len, ipc_priv );
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP )
	extern ipc_ctrl_pkt_t *coprocessor_dsp_tx_allocate( unsigned int *pkt_len, 
											void **ipc_priv, uint8 protocol );

	ipc_pkt = coprocessor_dsp_tx_allocate( pkt_len, ipc_priv, protocol );
#else
	???
#endif
	
	return ipc_pkt;
}

static inline void __ipc_pkt_tx_fill_frame_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl, uint8 protocol )
{
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP	
	extern void ethernet_dsp_fill_tx_frame_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl );
								
	ethernet_dsp_fill_tx_frame_header( ipc_pkt, txCtrl );
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP )
	extern void coprocessor_dsp_fill_tx_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl, uint8 protocol );

	// nothing!! 
	//coprocessor_dsp_fill_tx_header( ipc_pkt, txCtrl, protocol );
#else
	???
#endif	
}

static inline void __ipc_pkt_tx_start_xmit( void *ipc_priv, uint8 protocol )
{
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
	extern void ethernet_dsp_start_xmit( void *ipc_priv );

	ethernet_dsp_start_xmit( ipc_priv );
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP )
	extern void coprocessor_start_xmit( void *ipc_priv, uint8 protocol );

	coprocessor_start_xmit( ipc_priv, protocol );	
#else
	???
#endif
}

int ipc_pkt_tx_final( uint16 category, uint8 protocol, 
						uint8* pdata /* not const */, uint16 data_len, 
						const TstTxPktCtrl* txCtrl, uint16 *psent_seq,
						uint8* pconthdr )
{
	/* NOTE: pdata may be modified due to fetch data. */
	/*
	 * In case of IPC_PROT_CTRL/IPC_PROT_EVENT and resend==0, 
	 * this function will fill txCtrl ->seq_no = global_seq_no;
	 */
	
	unsigned int plhdr_len = 0;
	unsigned int pkt_len = 0;
	unsigned int conthdr_len = 0;
	unsigned short sent_seqno = ( unsigned short )-1;
	ipc_ctrl_pkt_t *ipc_pkt;
	
	void *ipc_priv;	// store 'struct sk_buff *' in ethernet dsp 
	
	//PRINT_R("cat=%d, type=%d\n", category, pkt_type);
	
	switch( protocol ) {
	case IPC_PROT_VOICE_TO_DSP:
		plhdr_len = SIZE_VOICE_FROM_HEAD( voice_cont_len );
		conthdr_len = SIZE_OF_VOICE_RTP2DSP_CONT_HEADER;
		break;
		
	case IPC_PROT_VOICE_TO_HOST:	// (ipc_voice_pkt_t)
	case IPC_PROT_T38_TO_HOST:
	case IPC_PROT_T38_TO_DSP:
		plhdr_len = SIZE_VOICE_FROM_HEAD( voice_cont_len );
		conthdr_len = SIZE_OF_VOICE_CONT_HEADER;
		break;
		
	case IPC_PROT_CTRL:	// control packet (ipc_ctrl_pkt_t)
	case IPC_PROT_RESP:	// response packet
	case IPC_PROT_EVENT:	// event packet
	case IPC_PROT_MIRROR_ACK:
	case IPC_PROT_RPC:
	case IPC_PROT_RPC_ACK:
	case IPC_PROT_DDD_H2D:
	case IPC_PROT_DDD_D2H:
		plhdr_len = SIZE_CTRL_FROM_HEAD( cont_len );
		break;

	case IPC_PROT_ACK:	// ack packet (ipc_ctrl_pkt_t)
		plhdr_len = SIZE_CTRL_FROM_HEAD( sequence );
		break;

	case IPC_PROT_MIRROR:	// mirror packet 
		plhdr_len = SIZE_CTRL_FROM_HEAD( cont_len ) +
					SIZE_OF_MIRROR_CONT_PLUS_HEADER;
		break;
		
	default:
		PRINT_R("Error protocol%d:%s:%s:%d\n", protocol, __FILE__, __FUNCTION__, __LINE__);
		return -1;

	}
	
	if( pconthdr == NULL )
		conthdr_len = 0;
	
	pkt_len = plhdr_len + conthdr_len + data_len;	// len_of_ether_frame
	
	// allocate space 
	ipc_pkt = __ipc_pkt_tx_allocate( &pkt_len, &ipc_priv, protocol );

	if( ipc_pkt == NULL ) {
		#ifdef CONFIG_RTK_VOIP_IPC_ARCH_DSP_IN_DUAL_LINUX
		if( printk_ratelimit() )
		#endif
		printk("ipc_pkt_tx_final :allocate return NULL. protocol %x\n", protocol);
		return -1;
	}
	
	// fill ethernet L2 or coprocessor share memory header 
	__ipc_pkt_tx_fill_frame_header( ipc_pkt, txCtrl, protocol );
	
	// fill IPC header 
	ipc_pkt ->dsp_cpuid = htons(txCtrl->dsp_cpuid);		/* DSP ID field */
	ipc_pkt ->protocol = htons(protocol);	/* payload type field */
	ipc_pkt ->voip_ver = htons(voip_ver);	/* VoIP Version field */
	
	// fill IPC content 
	if ((protocol == IPC_PROT_CTRL) || (protocol == IPC_PROT_EVENT) ||
		(protocol == IPC_PROT_RPC ) )
	{
		ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( ipc_ctrl_pkt_t * )ipc_pkt;
		
		if (protocol == IPC_PROT_CTRL)
		{
			//PRINT_R("Ctrl(%d)\n", category);
		}
		else
		{
			//PRINT_R("Event(%d)\n", category);
		}
			
		//*((unsigned short*)&skb->data[CATEGORY_SHIFT]) = htons(category);		/* category field */
		ipc_ctrl_pkt ->category = htons(category);		/* category field */
		
		if (txCtrl->resend_flag == 1)
		{			
			unsigned short resend_seq;
#if 1
			resend_seq = txCtrl ->seq_no;
#else
			if (global_seq_no>0)
				resend_seq = global_seq_no - 1;
			else
				resend_seq = 65535;
#endif
			//*((unsigned short*)&skb->data[SEQUENCE_SHIFT]) = htons(resend_seq);	/* sequence number field */
			ipc_ctrl_pkt ->sequence = htons(resend_seq);	/* sequence number field */
		}
		else {
			//txCtrl ->seq_no = global_seq_no;
			
			//*((unsigned short*)&skb->data[SEQUENCE_SHIFT]) = htons(seq_no);
			ipc_ctrl_pkt ->sequence =  htons(global_seq_no);
		}
					
		//*((unsigned short*)&skb->data[CONT_LEN_SHIFT]) = htons(data_len);	/* content length field */
		ipc_ctrl_pkt ->cont_len = htons(data_len);	/* content length field */
		//memcpy(skb->data+CONTENT_SHIFT, pdata, data_len); 			/* content field */
		memcpy(ipc_ctrl_pkt ->content, pdata, data_len); 			/* content field */
		
		// store sent seq no 
		sent_seqno = ipc_ctrl_pkt ->sequence;
	}
	else if( protocol == IPC_PROT_RESP || 
			 protocol == IPC_PROT_MIRROR_ACK ||
			 protocol == IPC_PROT_RPC_ACK )
	{
		ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( ipc_ctrl_pkt_t * )ipc_pkt;
		
		if (txCtrl->seq_no == -1)
			PRINT_R("Error seq no. for send response packet\n");
			
		//PRINT_R("Resp(%d)\n", category);
		//*((unsigned short*)&skb->data[CATEGORY_SHIFT]) = htons(category);	/* category field */
		ipc_ctrl_pkt ->category = htons(category);	/* category field */
		//*((unsigned short*)&skb->data[SEQUENCE_SHIFT]) = htons(txCtrl->seq_no);	/* sequence number field */
		ipc_ctrl_pkt ->sequence = htons(txCtrl->seq_no);	/* sequence number field */
		//*((unsigned short*)&skb->data[CONT_LEN_SHIFT]) = htons(data_len);/* content length field */
		ipc_ctrl_pkt ->cont_len = htons(data_len);/* content length field */
		
		if( data_len > 0 && pdata )
		{
			//memcpy(skb->data+CONTENT_SHIFT, pdata, data_len); 		 /* content field */
			memcpy(ipc_ctrl_pkt ->content, pdata, data_len); 		 /* content field */
		} 
		
		// store sent seq no 
		sent_seqno = ipc_ctrl_pkt ->sequence;
	}
	else if (protocol == IPC_PROT_VOICE_TO_HOST || protocol == IPC_PROT_VOICE_TO_DSP || protocol == IPC_PROT_T38_TO_HOST || protocol == IPC_PROT_T38_TO_DSP)
	{
		ipc_voice_pkt_t * const ipc_voice_pkt = ( ipc_voice_pkt_t * )ipc_pkt;
		
		//PRINT_Y("%d \n", data_len);
		//*((unsigned short*)&skb->data[VOICE_CONT_LEN_SHIFT]) = htons(data_len);	/* content length field*/
		ipc_voice_pkt ->voice_cont_len = htons(data_len + conthdr_len);	/* content length field*/
		
		if( conthdr_len ) {
			memcpy(ipc_voice_pkt ->voice_content, pconthdr, conthdr_len);	/* content header */
			memcpy( ( uint8 * )ipc_voice_pkt ->voice_content + conthdr_len, 
						pdata, data_len);			/* content field */
		} else {
			//memcpy(skb->data+VOICE_CONTENT_SHIFT, pdata, data_len);			/* content field */
			memcpy(ipc_voice_pkt ->voice_content, pdata, data_len);			/* content field */
		}
	}
	else if (protocol == IPC_PROT_ACK)
	{
		ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( ipc_ctrl_pkt_t * )ipc_pkt;
		
		if (txCtrl->seq_no == -1)
			PRINT_R("Error seq no. for send ack packet\n");
			
		//PRINT_R("Ack(%d)\n", category);
		//*((unsigned short*)&skb->data[CATEGORY_SHIFT]) = htons(category);	/* category field */
		ipc_ctrl_pkt ->category = htons(category);	/* category field */
		//*((unsigned short*)&skb->data[SEQUENCE_SHIFT]) = htons(txCtrl->seq_no);	/* sequence number field */
		ipc_ctrl_pkt ->sequence = htons(txCtrl->seq_no);	/* sequence number field */
		
		// store sent seq no 
		sent_seqno = ipc_ctrl_pkt ->sequence;
	}
	else if (protocol == IPC_PROT_MIRROR)
	{
		ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( ipc_ctrl_pkt_t * )ipc_pkt;
		mirror_all_content_t * const ipc_mirror_cont =
						( mirror_all_content_t * )ipc_ctrl_pkt ->content;
		
		ipc_ctrl_pkt ->category = htons( category );
		ipc_ctrl_pkt ->sequence = htons( global_seq_no );
		ipc_ctrl_pkt ->cont_len = htons( data_len + SIZE_OF_MIRROR_CONT_PLUS_HEADER );
		
		ipc_mirror_cont ->cch = txCtrl ->dsp_cch;
		memcpy( ipc_mirror_cont ->data, pdata, data_len );
		
		// store sent seq no 
		sent_seqno = ipc_ctrl_pkt ->sequence;
	}
	else if( protocol == IPC_PROT_DDD_H2D || protocol == IPC_PROT_DDD_D2H )
	{
		ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( ipc_ctrl_pkt_t * )ipc_pkt;
		ddd_content_t * const ipc_ddd_cont = 
						( ddd_content_t * )ipc_ctrl_pkt ->content;
		
		ipc_ctrl_pkt ->category = htons( category );
		ipc_ctrl_pkt ->sequence = htons( global_seq_no );
		ipc_ctrl_pkt ->cont_len = htons( data_len );
		
		memcpy( ipc_ddd_cont, pdata, data_len );
		
		// store sent seq no 
		sent_seqno = ipc_ctrl_pkt ->sequence;
	}
	
	
#if 0
	if (protocol == IPC_PROT_VOICE_TO_HOST || protocol == IPC_PROT_VOICE_TO_DSP || protocol == IPC_PROT_T38_TO_HOST || protocol == IPC_PROT_T38_TO_DSP)// data packet
		;
	else
	{
		int i;
		printk( "TX\n" );
		for( i = 0; i < 26; i ++ )
			printk( "%02X ", *( ( unsigned char * )ipc_pkt + i ) );
			
		printk( "\n" );
	}
#endif
	
	// fill check information 
	if ( txCtrl->resend_flag == 0)
	{
		if (protocol == IPC_PROT_CTRL)
		{
			// Need response for control packet
			stCheckResponse.seqNo = global_seq_no++;
			stCheckResponse.category = category;
			stCheckResponse.needResponse = 1;
			
			stCheckResponse.content = pdata;
			stCheckResponse.cont_len = data_len;
			stCheckResponse.cont_dsp_cpuid = txCtrl->dsp_cpuid;
		}
		//else if ((pkt_type == ETH_EVENT_PKT) || ((pkt_type == ETH_RESP_PKT)&& (data_len>0)))
		else if (protocol == IPC_PROT_EVENT)
		{
			// Need Ack for event packet (or response packet which carry content for host)
			stCheckAck.seqNo = global_seq_no++;
			stCheckAck.category = category;
			stCheckAck.needAck = 1;
		}
		else if (protocol == IPC_PROT_MIRROR ||
				 protocol == IPC_PROT_DDD_H2D || protocol == IPC_PROT_DDD_D2H)
		{
			global_seq_no++;
		}
	}
	
	// log TX data 
	if( ddinst_write( VOIP_DEV_IPC, LOG_IPC_TX_PREAMBLE, 4 ) >= 0 )
	{
		const uint32 now = timetick;
		
		ddinst_write( VOIP_DEV_IPC, ( const char * )&now, 4 );
		ddinst_write( VOIP_DEV_IPC, ( const char * )&pkt_len, 4 );
		ddinst_write( VOIP_DEV_IPC, ( const char * )ipc_pkt, pkt_len );
	}
	
	// store sent seq before TX (avoid race condition)
	if( psent_seq )
		*psent_seq = sent_seqno;
	
	// start send out 
	__ipc_pkt_tx_start_xmit( ipc_priv, protocol );
	
#if 0
	if( ( unsigned long )sent_seqno != 65535 )
		printk( "%d\n", sent_seqno );
#endif
	
	//PRINT_Y("send eth pkt...\n");

	switch(protocol)
	{
		case IPC_PROT_VOICE_TO_HOST://DSP -> Host
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_rtp_rtcp_tx_cnt++;
#endif
			break;
			
		case IPC_PROT_T38_TO_HOST://DSP -> Host
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_t38_tx_cnt++;
#endif
			break;
		
		case IPC_PROT_RESP://DSP -> Host
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_resp_tx_cnt++;
#endif
			break;

		case IPC_PROT_EVENT://DSP -> Host
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_event_tx_cnt++;
#endif
			break;

		case IPC_PROT_VOICE_TO_DSP://Host -> DSP
#ifdef IPC_ARCH_DEBUG_HOST
			host_rtp_rtcp_tx_cnt[txCtrl->dsp_cpuid]++;
#endif
			break;
		
		case IPC_PROT_T38_TO_DSP://Host -> DSP
#ifdef IPC_ARCH_DEBUG_HOST
			host_t38_tx_cnt[txCtrl->dsp_cpuid]++;
#endif
			break;
		
		case IPC_PROT_CTRL://Host -> DSP
#ifdef IPC_ARCH_DEBUG_HOST
			host_ctrl_tx_cnt[txCtrl->dsp_cpuid]++;
#endif
			break;
			
		case IPC_PROT_ACK://Host -> DSP
#ifdef IPC_ARCH_DEBUG_HOST
			host_ack_tx_cnt[txCtrl->dsp_cpuid]++;
#endif
			break;
		
		case IPC_PROT_MIRROR:
		case IPC_PROT_MIRROR_ACK:
			break;
			
		case IPC_PROT_RPC:
		case IPC_PROT_RPC_ACK:
			break;
		
		case IPC_PROT_DDD_H2D:
		case IPC_PROT_DDD_D2H:
			break;
		
		default:
			PRINT_R("?? @ %s, %s, line-%d\n", __FUNCTION__, __FILE__, __LINE__);
			break;
	}
	
	return 0;
}

// -----------------------------------------------------------------
// -----------------------------------------------------------------

int ipcSentControlPacket(unsigned short cmd, unsigned int chid, void* pMgr, unsigned short mgrLen,
							mgr_flags_t flags )
{
	// NOTE: Response data will be written to pMgr
	
	// flags - used to determine whether 'MF_FETCH' or not. 
	//         If NO_COPY_TO_USR, MF_NONE is given. Otherwise, MF_FETCH is suggested. 
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	timetick_t timestamp;
	unsigned long timeout=500;	
	TstTxPktCtrl stTxPktCtrl;
 #ifdef CONFIG_RTK_VOIP_IPC_ARCH_RESEND_CTRL
	unsigned int resend_cnt = 0;
 #endif
	uint16 resend_seqno;
	
	/* Host chid, DSP ID, DSP chid conversion */
	//stTxPktCtrl.host_cch = chid;
	stTxPktCtrl.dsp_cpuid = API_get_DSP_ID(cmd, chid);
	stTxPktCtrl.resend_flag = 0;
	
	/* Send Control Packet*/
	ipc_pkt_tx_final(cmd, IPC_PROT_CTRL, pMgr, mgrLen, &stTxPktCtrl, &resend_seqno, NULL);
	
	/* Check if need reponse (fetch) */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_ISSUE_TRIVIAL_ACK
	if( flags & MF_FETCH )
		;	// wait for response 
	else
		goto label_done;	// trivial ack (not issue) --> no response 
#endif
	
	/* Wait Response */
	timestamp = timetick + timeout;
	while(host_need_response())
	{
 #ifdef CONFIG_RTK_VOIP_IPC_ARCH_RESEND_CTRL
		if (timetick_after(timetick, timestamp) )
		{
			/* Re-send Control Packet*/
			stTxPktCtrl.seq_no = resend_seqno;//0;
			stTxPktCtrl.resend_flag = 1;
			ipc_pkt_tx_final(cmd, IPC_PROT_CTRL, pMgr, mgrLen, &stTxPktCtrl, NULL, NULL);
			PRINT_R("%d ", cmd);
			timestamp = timetick+timeout;
			
			resend_cnt++;
			if (resend_cnt > RESEND_THS)
			{
				PRINT_R("FAIL!\n");
				return -EVOIP_RESEND_ERR;
			}
		}
 #endif
		//schedule();
		schedule_timeout_interruptible(1); //schedule out
	}

label_done:

#endif
	return 0;
}

int ipcSentControlPacketNoChannel(unsigned short cmd, void* pMgr, unsigned short mgrLen,
									mgr_flags_t flags )
{
	// NOTE: Response data will be written to pMgr
	
	// flags - used to determine whether 'MF_FETCH' or not. 
	//         If NO_COPY_TO_USR, 0 is given. Otherwise, MF_FETCH is suggested. 
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	timetick_t timestamp;
	unsigned long timeout=500;	
	TstTxPktCtrl stTxPktCtrl;
	unsigned int resend_cnt;
	uint16 resend_seqno;
	int i=0;
	
	// global setting for every DSP
	for (i=0; i<API_get_DSP_NUM(); i++)
	{
		stTxPktCtrl.dsp_cpuid = i;
		stTxPktCtrl.resend_flag = 0;
		
		resend_cnt = 0;

		/* Send Control Packet*/
		ipc_pkt_tx_final(cmd, IPC_PROT_CTRL, pMgr, mgrLen, &stTxPktCtrl, &resend_seqno, NULL);
		
		/* Check if need reponse (fetch) */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_ISSUE_TRIVIAL_ACK
		if( flags & MF_FETCH )
			;	// wait for response 
		else
			goto label_done;	// trivial ack (not issue) --> no response 
#endif
		
      	/* Wait Response */
      	timestamp = timetick + timeout;
      	while(host_need_response())
      	{
 #ifdef CONFIG_RTK_VOIP_IPC_ARCH_RESEND_CTRL
      		if (timetick_after(timetick, timestamp) )
      		{
      			/* Re-send Control Packet*/
      			stTxPktCtrl.seq_no = resend_seqno;//0;
      			stTxPktCtrl.resend_flag = 1;
      			ipc_pkt_tx_final(cmd, IPC_PROT_CTRL, pMgr, mgrLen, &stTxPktCtrl, NULL, NULL);
      			PRINT_R("%d ", cmd);
      			timestamp = timetick+timeout;
      			
      			resend_cnt++;
  				if (resend_cnt > RESEND_THS)
  					return -EVOIP_RESEND_ERR;
      		}
 #endif
			//schedule();
			schedule_timeout_interruptible(1); //schedule out
      	}
label_done:
		;
	}
#endif
	return 0;
}

int ipcCheckRespPacket(int cmd, void* pCont, unsigned short* pDsp_id)
{
#if 0	// pkshih: don't need this anymore!! 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned char* pRespCont;
      	unsigned short resp_len, resp_cat, resp_dsp_id;
      	
      	/* Get Response pkt, and copy content to user*/
      	resp_len = get_response_content(&resp_dsp_id, &pRespCont, &resp_cat);
      	if (( pRespCont == NULL) || (resp_len == 0) || (resp_cat != cmd))
      	{
		PRINT_R("Error, No Content for Respnose pkt, or cmd(%d) not match(%d), %s-%s-%d\n", cmd, resp_cat,  __FILE__, __FUNCTION__, __LINE__);
      		return -1;
      	}
	memcpy(pCont, pRespCont, resp_len);
	*pDsp_id = resp_dsp_id;
 
 	return 0;     	
 #endif
#else
	*pDsp_id = stCheckResponse.cont_dsp_cpuid;
	
	return 0;
#endif
}



extern int last_process_seqno;

int ipcSentResponsePacket(unsigned short cmd, unsigned short seq_no, void* pdata, unsigned short data_len)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

	int ret_val = 0;
	TstTxPktCtrl stTxPktCtrl;
	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	
 #ifndef CONFIG_RTK_VOIP_IPC_ARCH_ISSUE_TRIVIAL_ACK
	if( pdata == NULL || data_len == 0 )
		return ret_val;		// trivial ack (not issue) --> no response 
 #endif
	
	stTxPktCtrl.seq_no = seq_no;
	if (last_process_seqno == seq_no)
	{
		stTxPktCtrl.resend_flag = 1;
		ret_val = -EVOIP_DUP_SEQ;
		PRINT_Y("Get the same seq ctrl pkt for cmd%d\n", cmd );
	}
	else
		stTxPktCtrl.resend_flag = 0;
		
	ret_val = ipc_pkt_tx_final(cmd, IPC_PROT_RESP, pdata, data_len, &stTxPktCtrl, NULL, NULL);
	if (ret_val < 0)
		ret_val = -EVOIP_IPC_TX_ERR;
	
	return ret_val;
#else
	return 0;
#endif
	
}

void ipc_RtpTx(/*RtpPacket*/struct stRtpPacket* pst)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	// Send RTP packet to Host by voice packet.
	TstTxPktCtrl stTxPktCtrl;
	//unsigned char rtp_wrtie_tmp[500];
	unsigned int len;
	//voice_content_t * const voice_content =
	//		( voice_content_t * )rtp_wrtie_tmp;
	voice_content_t voice_content_header;

	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	stTxPktCtrl.seq_no = -1;
	stTxPktCtrl.resend_flag = 0;

#if 1	
	voice_content_header.chid = pst->chid;
	voice_content_header.mid = API_GetMid( pst->chid, pst->sid );
#elif 1
	voice_content ->chid = pst->chid;
	voice_content ->mid = API_GetMid( pst->chid, pst->sid );
#else	
	*(unsigned int*)rtp_wrtie_tmp= pst->chid;
	*(unsigned int*)((unsigned char*)rtp_wrtie_tmp+4)= pst->sid;
#endif
	len = getTotalUsage(pst);
	
	//if (len > (500-8))
	//{
	//	PRINT_R("Error! no enought buf size, %s-%s-%d\n", __FILE__, __FUNCTION__, __LINE__);
	//}
	//memcpy(&rtp_wrtie_tmp[8], pst->header, len);
	//memcpy(voice_content ->voice, pst->header, len);
	
	ipc_pkt_tx_final( 0, IPC_PROT_VOICE_TO_HOST, 
						(unsigned char*)pst->header/*voice_content*/, 
						len /*+ SIZE_OF_VOICE_CONT_HEADER*/, 
						&stTxPktCtrl, NULL,
						( uint8 * )&voice_content_header);
	// Note: pst->header-pst_offset point to the chid address, so that receiver can get chid, sid, header info. Refer to the structure RtpPacket.
	// sizeof(RtpPacket) = 1524 bytes > 1518 bytes
	// PRINT_R("%d ", getTotalUsage(pst));
#endif
}

void ipc_RtcpTx(/*RtcpPacket*/struct stRtcpPacket* pst)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	//extern int rtcp_sid_offset;
	// Send RTCP packet to Host by voice packet.
	TstTxPktCtrl stTxPktCtrl;
	//unsigned char rtcp_wrtie_tmp[600];
	unsigned int len;
	//voice_content_t * const voice_content =
	//		( voice_content_t * )rtcp_wrtie_tmp;
	voice_content_t voice_content_header;

	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	stTxPktCtrl.seq_no = -1;
	stTxPktCtrl.resend_flag = 0;

#if 1
	voice_content_header.chid = pst->chid;
	voice_content_header.mid = API_GetMid( pst->chid, pst->sid ) + RTCP_MID_OFFSET;
#elif 1
	voice_content ->chid = pst->chid;
	voice_content ->mid = API_GetMid( pst->chid, pst->sid ) + RTCP_MID_OFFSET;
#else	
	*(unsigned int*)rtcp_wrtie_tmp = pst->chid;
	*(unsigned int*)((unsigned char*)rtcp_wrtie_tmp+4) = pst->sid + rtcp_sid_offset;
#endif
	len = RtcpPkt_getTotalUsage(pst);
	
	//PRINT_Y("rtcp len=%d\n", len); //len=100
	//if (len > (600-8))
	//{
	//	PRINT_R("Error! no enought buf size, %s-%s-%d\n", __FILE__, __FUNCTION__, __LINE__);
	//	PRINT_R("Need size: %d bytes.", len);
	//}
	//memcpy(&rtcp_wrtie_tmp[8], RtcpPkt_getPacketData(pst), len);
	//memcpy( voice_content ->voice, RtcpPkt_getPacketData(pst), len);
	
	ipc_pkt_tx_final( 0, IPC_PROT_VOICE_TO_HOST, 
						(unsigned char*)RtcpPkt_getPacketData(pst)/*voice_content*/, 
						len /*+ SIZE_OF_VOICE_CONT_HEADER*/, 
						&stTxPktCtrl, NULL,
						( uint8 * )&voice_content_header );
	// Note: pst->header-pst_offset point to the chid address, so that receiver can get chid, sid, header info. Refer to the structure RtpPacket.
	//PRINT_R("%d ", getTotalUsage(pst));
#endif
}

void ipc_T38Tx( unsigned int chid, unsigned int sid, void* packet, unsigned int pktLen)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

	// Send T38 packet to Host
	TstTxPktCtrl stTxPktCtrl;
	//unsigned char t38_tx_tmp[500];
	//unsigned int chid_new, sid_new, mid, len;
	//voice_content_t * const voice_content =
	//		( voice_content_t * )t38_tx_tmp;
	voice_content_t voice_content_header;

	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	stTxPktCtrl.seq_no = -1;
	stTxPktCtrl.resend_flag = 0;

#if 1
	voice_content_header.chid = chid;
	voice_content_header.mid = API_GetMid( chid, sid );
#elif 1
	voice_content ->chid = chid;
	voice_content ->mid = API_GetMid( chid, sid );
#else	
	*(unsigned int*)t38_tx_tmp = chid;
	*(unsigned int*)((unsigned char*)t38_tx_tmp+4) = sid;
#endif
	
	//if (pktLen > (500 - SIZE_OF_VOICE_CONT_HEADER))
	//{
	//	PRINT_R("Error! no enought buf size, %s-%s-%d\n", __FILE__, __FUNCTION__, __LINE__);
	//}
	//memcpy(&t38_tx_tmp[8], packet, pktLen);
	//memcpy(voice_content ->voice, packet, pktLen);
	
	ipc_pkt_tx_final( 0, IPC_PROT_T38_TO_HOST, 
						(unsigned char*)packet /*voice_content*/, 
						pktLen /*+ SIZE_OF_VOICE_CONT_HEADER*/, 
						&stTxPktCtrl, NULL,
						( uint8 * )&voice_content_header);
	//printk("t%d ", chid);
	
#endif
}

// -----------------------------------------------------------------
// -----------------------------------------------------------------

int ipcSentMirrorPacket( unsigned short category, unsigned int host_cch, void* mirror_data, unsigned short mirror_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstTxPktCtrl stTxPktCtrl;
	
	/* Host chid, DSP ID, DSP chid conversion */
	//stTxPktCtrl.host_cch = host_cch;
	stTxPktCtrl.dsp_cpuid = API_get_DSP_ID( category, host_cch );
	stTxPktCtrl.dsp_cch = API_get_DSP_CH( category, host_cch );
	
	stTxPktCtrl.resend_flag = 0;
	
	/* Send Mirror Packet*/	
	ipc_pkt_tx_final(category, IPC_PROT_MIRROR, mirror_data, mirror_len, &stTxPktCtrl, NULL, NULL);
#endif
	return 0;
}

int ipcSentMirrorAckPacket( unsigned short category, uint16 seq_no, void* mirror_ack_data, unsigned short mirror_ack_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	TstTxPktCtrl stTxPktCtrl;
	
	/* Host chid, DSP ID, DSP chid conversion */
	//stTxPktCtrl.host_cch = host_cch;
	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	stTxPktCtrl.seq_no = seq_no;
	
	stTxPktCtrl.resend_flag = 0;
	
	/* Send Mirror Ack Packet*/	
	ipc_pkt_tx_final( category, IPC_PROT_MIRROR_ACK, mirror_ack_data, mirror_ack_len, &stTxPktCtrl, NULL, NULL );
#endif
	return 0;
}

int ipcSentRpcPacket( unsigned short category, void* rpc_data, unsigned short rpc_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	TstTxPktCtrl stTxPktCtrl;
	
	/* Host chid, DSP ID, DSP chid conversion */
	//stTxPktCtrl.host_cch = host_cch;
	stTxPktCtrl.dsp_cpuid = Get_DSP_CPUID();
	
	stTxPktCtrl.resend_flag = 0;
	
	/* Send RPC Packet*/	
	ipc_pkt_tx_final( category, IPC_PROT_RPC, rpc_data, rpc_len, &stTxPktCtrl, NULL, NULL );
#endif
	return 0;
}

int ipcSentRpcAckPacket( unsigned short category, uint16 seq_no, unsigned int host_cch, void* rpc_ack_data, unsigned short rpc_ack_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstTxPktCtrl stTxPktCtrl;
	
	/* Host chid, DSP ID, DSP chid conversion */
	//stTxPktCtrl.host_cch = host_cch;
	stTxPktCtrl.dsp_cpuid = API_get_DSP_ID( category, host_cch );
	stTxPktCtrl.dsp_cch = API_get_DSP_CH( category, host_cch );
	
	stTxPktCtrl.seq_no = seq_no;
	
	stTxPktCtrl.resend_flag = 0;
	
	/* Send RPC ACK Packet*/	
	ipc_pkt_tx_final(category, IPC_PROT_RPC_ACK, rpc_ack_data, rpc_ack_len, &stTxPktCtrl, NULL, NULL);
#endif
	return 0;
}

static int _ipcSendDddH2dPacket( uint16 category, ddd_content_t *ddd_cont, uint16 ddd_len, 
									uint16 protocol )
{
#define DEFAULT_IPC_DDD_DSP_CPUID		0	// send to cpuid=0

	TstTxPktCtrl stTxPktCtrl;
	
	stTxPktCtrl.dsp_cpuid = DEFAULT_IPC_DDD_DSP_CPUID;
	
	stTxPktCtrl.resend_flag = 0;
	
	/* Send DDD H2D packet */
	ipc_pkt_tx_final( category, protocol, ( void * )ddd_cont, ddd_len, 
						&stTxPktCtrl, NULL, NULL );
	
	return 0;
}

int ipcSendDddH2dPacket( uint16 category, ddd_content_t *ddd_cont, uint16 ddd_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	_ipcSendDddH2dPacket( category, ddd_cont, ddd_len, IPC_PROT_DDD_H2D );
#endif
	
	return 0;
}

int ipcSendDddD2hPacket( uint16 category, ddd_content_t *ddd_cont, uint16 ddd_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	_ipcSendDddH2dPacket( category, ddd_cont, ddd_len, IPC_PROT_DDD_D2H );
#endif
	
	return 0;
}

