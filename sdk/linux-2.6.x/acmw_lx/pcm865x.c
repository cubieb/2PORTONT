#include <linux/signal.h>
#include <linux/netdevice.h>
#include <asm/io.h>
#include "acmw_global_typedef.h"

#include "./ac_drv/AC49xDrv_Config.h"
#include "../rtk_voip/include/rtk_voip.h"
#ifdef T38_STAND_ALONE_HANDLER
//#include "../rtk_voip/include/voip_types.h"
#include "../rtk_voip/voip_drivers/t38_handler.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
extern uint32* piis_RxBufTmp; // For reducing memcpy
extern uint32* piis_TxBufTmp;
#else
extern uint32* pRxBufTmp;
extern uint32* pTxBufTmp;
#endif
#endif
#define PCM_10MSLEN_INSHORT 80

//int rtkVoipIsEthernetDsp = 0;

typedef volatile struct 
{
	unsigned short *RxBuff;
	unsigned short *TxBuff;
}Tacmw_transiverBuff;

static Tacmw_transiverBuff transiverBuff[ACMW_MAX_NUM_CH];

extern int ACMWPcmProcess(const Tacmw_transiverBuff *transiverBuff, const unsigned int maxCh, const unsigned int size);
extern unsigned long  Ac49xUserDef_DisableInterrupts(void);
extern void Ac49xUserDef_RestoreInterrupts(unsigned long flags);


#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

void rtl8972B_hw_init(int mode)
{
	int temp;

	temp = *((volatile unsigned int *)0xb800000c);

	if ( !(temp & 0x8) ) {

		//*((volatile unsigned int *)0xb8000010) = 0x7fffd6;
		*((volatile unsigned int *)0xb8000010) &= ~0x800000;
		//*((volatile unsigned int *)0xb8000030) = 0x05540303;
		*((volatile unsigned int *)0xb8000030) = 
			*((volatile unsigned int *)0xb8000030) & ~0x00FFFF83 | 0x00500003;
		*((volatile unsigned int *)0xb800351c) = 0;
		//*((volatile unsigned int *)0xb8003524) = 0x1000;
		*((volatile unsigned int *)0xb8003524) = 
			*((volatile unsigned int *)0xb8003524) & ~0x1800 | 0x1000;
		//*((volatile unsigned int *)0xb8003530) = 0;	// EG IMR
		*((volatile unsigned int *)0xb8003530) &= ~0x03C00000;
		//*((volatile unsigned int *)0xb8003534) = 0;	// GH IMR
		*((volatile unsigned int *)0xb8008000) = 0x3000;

		if (mode & 1) { /* need init iis  */
			*((volatile unsigned int *)0xb8000030) =
				*((volatile unsigned int *)0xb8000030) & ~0x00000380 | 0x00000100;
			*((volatile unsigned int *)0xb8009000) = 0x80000000;
			*((volatile unsigned int *)0xb8009000) = 0x0;
			*((volatile unsigned int *)0xb8009000) = 0x80000000;
		}
	}
}
#endif

/*****************************************************************************
* Function Name: ACMWPCM_handler
* Description  : PCM processing routine
******************************************************************************
* Input:  None.
* Output: None.
******************************************************************************/
  #define __pcmImem	__attribute__ ((section(".speedup.text")))

#if 1
__pcmImem void ACMWPCM_handler(char chanEnabled[])
{
	unsigned int pcmChid;
	unsigned long saved_flags;
	while(1)
	{

  		saved_flags = Ac49xUserDef_DisableInterrupts();
		for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
		{

			transiverBuff[pcmChid].TxBuff = NULL;
			transiverBuff[pcmChid].RxBuff = NULL;

#ifdef T38_STAND_ALONE_HANDLER
			if ((chanEnabled[pcmChid] == 0) || (t38RunningState[pcmChid] == T38_START))
				continue;
#else
			if (chanEnabled[pcmChid] == 0)
				continue;
#endif
			if(pcm_get_read_rx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].RxBuff) )
			{
				Ac49xUserDef_RestoreInterrupts(saved_flags);
				return 0;
			}

			pcm_get_write_tx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].TxBuff);
		}
		
		Ac49xUserDef_RestoreInterrupts(saved_flags);
		
#ifdef T38_STAND_ALONE_HANDLER
		
		int chid;
		for(chid=0; chid<ACMW_MAX_NUM_CH; chid++)
		{
			if( t38RunningState[chid] == T38_START )
			{
				saved_flags = Ac49xUserDef_DisableInterrupts();
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
				if( pcm_get_read_rx_fifo_addr(chid, (void*)&piis_RxBufTmp))
#else
				if( pcm_get_read_rx_fifo_addr(chid, (void*)&pRxBufTmp))
#endif
				{
					Ac49xUserDef_RestoreInterrupts(saved_flags);
					return 0;
				}

#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
				pcm_get_write_tx_fifo_addr(chid, &piis_TxBufTmp);
#else
				pcm_get_write_tx_fifo_addr(chid, &pTxBufTmp);
#endif

				Ac49xUserDef_RestoreInterrupts(saved_flags);
				
				PCM_handler_T38(chid);
				
				saved_flags = Ac49xUserDef_DisableInterrupts();
				pcm_read_rx_fifo_done(chid); 
				Ac49xUserDef_RestoreInterrupts(saved_flags);
			}
		}

#endif	


		ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );


		saved_flags = Ac49xUserDef_DisableInterrupts();
		for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
		{

			if(transiverBuff[pcmChid].TxBuff != NULL)
				pcm_write_tx_fifo_done(pcmChid);

			if(transiverBuff[pcmChid].RxBuff != NULL)
				pcm_read_rx_fifo_done(pcmChid);


		}
		Ac49xUserDef_RestoreInterrupts(saved_flags);

	}

	return 0;

}
#else
__pcmImem int ACMWPCM_handler(unsigned int pcmPeriodsIn10msec, char chanEnabled[], long chNum)
{
	unsigned int pcmChid, chid;

#if 0 // can work

	while(1)
	{

			
		for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
		{
			
			for(chid=0; chid < ACMW_MAX_NUM_CH; chid++)
			{
				transiverBuff[chid].TxBuff = NULL;
				transiverBuff[chid].RxBuff = NULL;	
			}

			if (chanEnabled[pcmChid] == 0)
				continue;

			if(pcm_get_read_rx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].RxBuff) )
				return 0;

			pcm_get_write_tx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].TxBuff);


#ifdef T38_STAND_ALONE_HANDLER
			if( t38RunningState[ pcmChid ] == T38_START )
			{
				pcm_get_read_rx_fifo_addr(pcmChid, (void*)&pRxBufTmp);
	
				PCM_handler_T38( pcmChid );
				pcm_read_rx_fifo_done(pcmChid);
			}
			else
#endif
			{			
				//activate AC MiddelWare		
				ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );
				
				if(transiverBuff[pcmChid].TxBuff != NULL)
					pcm_write_tx_fifo_done(pcmChid);

				if(transiverBuff[pcmChid].RxBuff != NULL)
					pcm_read_rx_fifo_done(pcmChid);
			}

		}

   	}

#else // also can work

	for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
	{
			
		if (chanEnabled[pcmChid] == 0)
			continue;


		while(1)
		{
			
			if(pcm_get_read_rx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].RxBuff) )
				break;

			pcm_get_write_tx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].TxBuff);


#ifdef T38_STAND_ALONE_HANDLER
			if( t38RunningState[ pcmChid ] == T38_START )
			{
				pcm_get_read_rx_fifo_addr(pcmChid, (void*)&pRxBufTmp);
	
				PCM_handler_T38( pcmChid );
				pcm_read_rx_fifo_done(pcmChid);
			}
			else
#endif
			{			
				//activate AC MiddelWare		
				ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );
				
				if(transiverBuff[pcmChid].TxBuff != NULL)
					pcm_write_tx_fifo_done(pcmChid);

				if(transiverBuff[pcmChid].RxBuff != NULL)
					pcm_read_rx_fifo_done(pcmChid);
			}

		}

		transiverBuff[pcmChid].TxBuff = NULL;
		transiverBuff[pcmChid].RxBuff = NULL;	
   	}

#endif
	return 0;
	

}
#endif

