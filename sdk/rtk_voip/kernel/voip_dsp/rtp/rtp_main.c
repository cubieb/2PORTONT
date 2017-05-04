#include <linux/netfilter_ipv4.h>
#include "Rtp.h"
#include "rtpterm.h"
#include "RtpPacket.h"
#include "rtp_main.h"
#include "dsp_main.h"

#include "codec_descriptor.h"

#define OPTIMIZE_GET_CODEC_DESC		/* make it to be more efficient. */

#ifdef SUPPORT_MULTI_FRAME
extern unsigned char MFLocalFrameNo[MAX_DSP_RTK_SS_NUM];
extern unsigned char MFRemoteFrameNo[MAX_DSP_RTK_SS_NUM];
#endif

#ifdef OPTIMIZE_GET_CODEC_DESC
 #define CODEC_PAYLOAD_DESC_MAP_NUM	128	/* 128 includes all part of RtpPayloadType */
 
 static unsigned int idxMapCodecPayloadDesc[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
 };
 static unsigned int bLinearSearchCodecPayloadDesc = 1;
 CT_ASSERT( ( sizeof( idxMapCodecPayloadDesc ) / sizeof( idxMapCodecPayloadDesc[ 0 ] ) ) == CODEC_PAYLOAD_DESC_MAP_NUM );
#endif

#ifdef OPTIMIZE_GET_CODEC_DESC
static void ConstructCodecPayloadDescMapTable( void )
{
	unsigned int i;
	RtpPayloadType payloadType;
	unsigned int bLargeTypeId = 0;

	for( i = 0; i < NUM_OF_CODEC_PAYLOAD_DESC; i ++ ) {
	
		payloadType = g_codecPayloadDesc[ i ].payloadType;

		if( ( unsigned int )payloadType < CODEC_PAYLOAD_DESC_MAP_NUM )
			idxMapCodecPayloadDesc[ payloadType ] = i;
		else
			bLargeTypeId = 1;
	}
	
	/* payload type is always smaller than CODEC_PAYLOAD_DESC_MAP_NUM */
	if( bLargeTypeId == 0 )
		bLinearSearchCodecPayloadDesc = 0;
}
#endif /* OPTIMIZE_GET_CODEC_DESC */

const codec_payload_desc_t *GetCodecPayloadDesc( RtpPayloadType payloadType )
{
	unsigned int i;
	const codec_payload_desc_t *pCodecPayloadDesc;

#ifdef OPTIMIZE_GET_CODEC_DESC
	if( ( unsigned int )payloadType < CODEC_PAYLOAD_DESC_MAP_NUM ) {
		
		i = idxMapCodecPayloadDesc[ payloadType ];
		
		if( i < NUM_OF_CODEC_PAYLOAD_DESC )
			return &g_codecPayloadDesc[ i ];
	}

	//printk( "SE " );

	if( bLinearSearchCodecPayloadDesc == 0 )
		return NULL;

	//printk( "NS " );
#endif /* OPTIMIZE_GET_CODEC_DESC */
		
	for( i = 0; i < NUM_OF_CODEC_PAYLOAD_DESC; i ++ ) {
	
		pCodecPayloadDesc = &g_codecPayloadDesc[ i ];
	
		if( pCodecPayloadDesc ->payloadType == payloadType )
			return pCodecPayloadDesc;
	}
	
	return NULL;
}

void rtp_main(void)
{
	const codec_payload_desc_t *pRecvCodecPayloadDesc;
	const codec_payload_desc_t *pTranCodecPayloadDesc;

	int i;
	CRtpConfig Config;
	RtpRx_Init();
	RtpTx_Init ();

#ifdef SUPPORT_RTCP
	RtcpTx_Init();
	RtcpRx_Init();
	Ntp_Init();
#endif

#ifdef OPTIMIZE_GET_CODEC_DESC
	ConstructCodecPayloadDescMapTable();
#endif

	Config.m_uRecvPktFormat = rtpPayloadPCMU;
	Config.m_uTranPktFormat = rtpPayloadPCMU;

	pTranCodecPayloadDesc = GetCodecPayloadDesc( Config.m_uTranPktFormat );
	pRecvCodecPayloadDesc = GetCodecPayloadDesc( Config.m_uRecvPktFormat );

	if( pTranCodecPayloadDesc ) {
		Config.m_nTranFrameRate = pTranCodecPayloadDesc ->nTranFrameRate;
	} else {
		/* unknown frame type */
		printk("[%s] Unknown tran frame type %d\n", __FUNCTION__, Config.m_uTranPktFormat);
		assert(0);
	}
	
	Config.m_uTRMode = rtp_session_sendrecv;

	for(i=0; i<DSP_RTK_SS_NUM; i++)
		CRtpTerminal_Init(i, &Config);

}

void rtp_SetConfig(
	uint32 chid, 
	uint32 sid, 
	RtpPayloadType uLocalPktFormat, 
	RtpPayloadType uRemotePktFormat, 
	uint32 nLocalFramePerPacket,
	uint32 nRemoteFramePerPacket
	)
{
	extern void RtcpTx_reset( uint32 chid, uint32 sid );
	const codec_payload_desc_t *pLocalCodecPayloadDesc;
	const codec_payload_desc_t *pRemoteCodecPayloadDesc;
	CRtpConfig Config;
	//RtpTerminal_GetConfig(sid, &Config);
	Config.m_uTranPktFormat = uLocalPktFormat;
	Config.m_uRecvPktFormat = uRemotePktFormat;

	pLocalCodecPayloadDesc = GetCodecPayloadDesc( uLocalPktFormat );

	if( pLocalCodecPayloadDesc == NULL ) {
		printk("Rtpterm: Unknow Local frame type\n");
		assert(0);
		return;
	}
	pRemoteCodecPayloadDesc = GetCodecPayloadDesc( uRemotePktFormat );

	if( pRemoteCodecPayloadDesc == NULL ) {
		printk("Rtpterm: Unknow Remote frame type\n");
		assert(0);
		return;
	}

	PRINT_MSG("TX %s", pLocalCodecPayloadDesc ->pszSetCodecPrompt );
	PRINT_MSG("RX %s", pRemoteCodecPayloadDesc ->pszSetCodecPrompt );

#ifdef SUPPORT_MULTI_FRAME
	if(nLocalFramePerPacket == 0)
		nLocalFramePerPacket = MFLocalFrameNo[sid];

	if(nRemoteFramePerPacket == 0)
		nRemoteFramePerPacket = MFRemoteFrameNo[sid];
#endif

	/* pCodecPayloadDesc point to payload descriptor of the codec. */
	/* SUPPORT_BASEFRAME_10MS has been considered in descriptor. */
 #ifdef SUPPORT_BASEFRAME_10MS
	Config.m_nRecvFrameRate = pRemoteCodecPayloadDesc ->nRecvFrameRate;
	Config.m_nTranFrameRate = pLocalCodecPayloadDesc ->nTranFrameRate;
	
 	if( nLocalFramePerPacket <= 
 		( MULTI_FRAME_BUFFER_SIZE / pLocalCodecPayloadDesc ->nFrameBytes ) )
 	{
		MFLocalFrameNo[sid] = nLocalFramePerPacket;
	}
	else
		PRINT_G("%s, line%d, nLocalFramePerPacket(%d) is too large. Set to default 1\n", __FUNCTION__, __LINE__, nLocalFramePerPacket);

 	if( nRemoteFramePerPacket <= 
 		( MULTI_FRAME_BUFFER_SIZE / pRemoteCodecPayloadDesc ->nFrameBytes ) )
 	{
		MFRemoteFrameNo[sid] = nRemoteFramePerPacket;
	}
	else
		PRINT_G("%s, line%d, nRemoteFramePerPacket(%d) is too large. Set to default 1\n", __FUNCTION__, __LINE__, nRemoteFramePerPacket);
 #else
	Config.m_nRecvFrameRate = nLocalFramePerPacket * 
							  pRemoteCodecPayloadDesc ->nRecvFrameRate;

	Config.m_nTranFrameRate = nRemoteFramePerPacket *
							  pLocalCodecPayloadDesc ->nTranFrameRate;

 	if( nLocalFramePerPacket <= pCodecPayloadDesc ->nHThresTxFramePerPacket ) {
 		MFLocalFrameNo[sid] = nLocalFramePerPacket;
	}

 	if( nRemoteFramePerPacket <= pCodecPayloadDesc ->nHThresTxFramePerPacket ) {
 		MFRemoteFrameNo[sid] = nRemoteFramePerPacket;
	}
 #endif	
	
	Config.m_uTRMode = rtp_session_unchange;	// don't change previous setting 
	
	RtpTerminal_SetConfig(chid, sid, &Config);

#ifdef CONFIG_RTK_VOIP_RTCP_XR	
	RtcpTx_reset( chid, sid );
#endif
}
//remove by bruce

