#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/atmdev.h>
#include <linux/atm.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "ra8670.h"
#include "ra_debug.h"
#include "../net/re830x.h"
//UINT16	mantissa[0x10]=	{
//	0x005, 0x016, 0x028, 0x03C, 
//	0x051, 0x068, 0x080, 0x09B, 
//	0x0B8, 0x0D8, 0x0FA, 0x120, 
//	0x14B, 0x179, 0x1AE, 0x1E8};

//UINT32	CRC32TBL[0x100];
//UINT16	CRC10TBL[0x100];
#ifdef CONFIG_RTL8672_ATM_QoS
#define SAR_INTERPOLATE 0
#else
#define SAR_INTERPOLATE 1
#define IC8671G
#endif
#define SAR_CHECK_RC 1
#define SAR_CHECK_RC_NEW 0 // new methold of check_cr


//tylo, for linux2.6 porting
extern int sar_irq;
#define __SWAP_DATA
#define __SWAP
int cr2reg(int pcr);
int 	sar_send (struct atm_vcc *vcc,struct sk_buff *skb);
extern BOOL GetLinkSpeed(char *Rate);

extern void skb_oamaal5_debug(const struct sk_buff *skb, int enable, int flag);

atomic_t lock_txbuff = ATOMIC_INIT(0);

#if 0
static int xxx = 0;
static char yyy[20000];
void scout(char ch)
{
	return;
    yyy[xxx++] = ch;
    if ( xxx == (sizeof(yyy)-1) )
    {
        yyy[xxx] = '\0'; // null terminated
        printk("\n\n");
        for( xxx = 0; yyy[xxx]; xxx++ ) printk("%c",yyy[xxx]);
        for(;;);
    }
}
#endif

/*--------------------------------
			Routines
---------------------------------*/

#if 0
struct sk_buff *get_skb_from_pool(INT8 ch_no){

	struct sk_buff	*skb=NULL;
	INT8				i=cp->vcc[ch_no].skb_pool_get;

	if(cp->vcc[ch_no].skb_pool[i]!=(UINT32)NULL){

		skb=(struct sk_buff *)cp->vcc[ch_no].skb_pool[i];
		cp->vcc[ch_no].skb_pool[i] = (UINT32)NULL;
		i++;
		i%=SAR_RX_DESC_NUM;
		cp->vcc[ch_no].skb_pool_get = i;
	}

	return skb;

}
void refill_skb_pool(INT8 ch_no){

	struct sk_buff 	*skb;
	INT8				put=cp->vcc[ch_no].skb_pool_put,
					get=cp->vcc[ch_no].skb_pool_get;

	while((put != get)||(cp->vcc[ch_no].skb_pool[get]==(UINT32)NULL)){
		skb=dev_alloc_skb(SAR_RX_Buffer_Size);
		if(skb!=(struct sk_buff *)NULL)
			cp->vcc[ch_no].skb_pool[put]=(UINT32)skb;
		else 
			break;
		put++;
		put%=SAR_RX_DESC_NUM;
		cp->vcc[ch_no].skb_pool_put = put;
	}
	return;
}

void init_skb_pool(INT8 ch_no){

	int 				i;
	struct sk_buff 	*skb;
	
	cp->vcc[ch_no].skb_pool_put = 0;
	cp->vcc[ch_no].skb_pool_get = 0;
	
	for(i=0;i<SAR_RX_DESC_NUM;i++){
		skb=dev_alloc_skb(SAR_RX_Buffer_Size);
		if(skb!=(struct sk_buff *)NULL)
			cp->vcc[ch_no].skb_pool[i]=(UINT32)skb;
		else 
			break;
	}

	i%=SAR_RX_DESC_NUM;
	cp->vcc[ch_no].skb_pool_put=i;
	
	return;
}

#endif

/* move to ra8670.c
struct VPIVCI{
	int vpi;
	int vci;
};

struct VPIVCI Possible_PVC[]={{0,35},{8,35},{0,43},{0,51},{0,59},{8,43},{8,51},{8,59},{0,8},{0,9},
					{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},{8,0},{9,0},{10,0},
					{0,0},{0,0},{0,0},{0,0},{0,0},{5,35},{0,0},{0,0},{0,0},{0,0},
					{0,0},{0,0},{0,0},{0,0},{0,0},{8,35},{0,0},{0,0},{0,0},{0,0},
					{0,0},{0,0},{0,0},{0,0},{0,0},{0,35},{0,0},{0,0},{0,0},{0,0},
					{0,0},{0,0},{0,0},{0,0},{0,0},{4,35},{0,0},{0,0},{0,0},{0,0},};
*/					


void  Alloc_desc(sar_private *cp)
{
	UINT32	*tmp;
	INT32	size;

#if 0
	/* TV Channels : 16 byte x 64 descriptors x Enable_VC_CNT channel, +256 for 256 byte alignment */
	size=sizeof(TV_CMD_DESC)*SAR_TX_RING_SIZE*Enable_VC_CNT+Desc_Align; 
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pTVDescBuf=(char *)tmp;
	cp->TVDesc=(TV_CMD_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0x0FFFFF00)|Uncache_Mask);
	memset(cp->TVDesc, 0 , size);

	/* RV Channels : 16 byte x 64 descriptors x Enable_VC_CNT channel, +256 for 256 byte alignment */
	size=sizeof(RV_CMD_DESC)*SAR_RX_RING_SIZE*Enable_VC_CNT+Desc_Align; 
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pRVDescBuf=(char *)tmp;
	cp->RVDesc=(RV_STS_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0xFFFFFF00)|Uncache_Mask );	
	memset(cp->RVDesc, 0 , size);

	/* TO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(TO_CMD_DESC)*SAR_TX_RING_SIZE+Desc_Align; 
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pTODescBuf=(char *)tmp;
	cp->TODesc=(TO_CMD_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0xFFFFFF00)|Uncache_Mask );	
	memset(cp->TODesc, 0 ,size);

	/* RO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(RO_CMD_DESC)*SAR_RX_RING_SIZE+Desc_Align; 
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pRODescBuf=(char *)tmp;
	cp->RODesc=(RO_STS_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0xFFFFFF00)|Uncache_Mask );	
	memset(cp->RODesc, 0 , size);
#else
	/* TV Channels : 16 byte x 64 descriptors x (Enable_VC_CNT+1) channel, +256 for 256 byte alignment */
	size=sizeof(TV_CMD_DESC)*SAR_TX_RING_SIZE*(Enable_VC_CNT+1)+Desc_Align; 
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pTVDescBuf=(char *)tmp;
	cp->TVDesc=(TV_CMD_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0x0FFFFF00)|Uncache_Mask);
	memset(cp->TVDesc, 0 , size);

	/* TO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	cp->pTODescBuf=(char *)NULL;
	cp->TODesc=(TO_CMD_DESC *) (&cp->TVDesc[OAM_CH_NO*SAR_TX_RING_SIZE]);	

	/* RV Channels : 16 byte x 64 descriptors x (Enable_VC_CNT+1) channel, +256 for 256 byte alignment */
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt 
	size=sizeof(RV_CMD_DESC)*SAR_RX_RING_SIZE*(Enable_VC_CNT+NUMBER_OF_OAM_CH)+Desc_Align;
	tmp=kmalloc(size, GFP_KERNEL);	
	cp->pRVDescBuf=(char *)tmp;
	cp->RVDesc=(RV_STS_DESC *) (((UINT32)((UINT32)tmp+Desc_Align)&0xFFFFFF00)|Uncache_Mask );	
	memset(cp->RVDesc, 0 , size);

	/* RO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	cp->pRODescBuf=(char *)NULL;	
	cp->RODesc=(RO_STS_DESC *) (&cp->RVDesc[OAM_CH_NO*SAR_RX_RING_SIZE]);	
#endif	
}

void  Free_desc(sar_private *cp)
{
	if (cp->pTVDescBuf)		kfree(cp->pTVDescBuf);
	if (cp->pTODescBuf)		kfree(cp->pTODescBuf);
	if (cp->pRVDescBuf)		kfree(cp->pRVDescBuf);
	if (cp->pRODescBuf)		kfree(cp->pRODescBuf);
}


void Init_reg(sar_private *cp)
{
	int i;
	sar_atm_vcc	*vcc;
	static int first_init=1;
	//vcc = kmalloc(sizeof(sar_atm_vcc)*(Enable_VC_CNT+1), GFP_KERNEL);  //5/22/04' hrchen, bug, extra malloc

	/* Set Register Address */
	for (i=0;i<Enable_VC_CNT;i++){

		vcc=&cp->vcc[i];

		memset(&vcc->TV, 0, sizeof(TV_Channel));
		memset(&vcc->RV, 0, sizeof(RV_Channel));
		if (first_init) {  //only reset stat information once
		    memset(&vcc->stat, 0, sizeof(ch_stat));
		    //printk("reset\n");
		};
		memset(&vcc->QoS, 0, sizeof(Traffic_Manage));

		vcc->TV.CtlSts_Addr	= TV_Ctrl_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.FDP_Addr	= TV_FDP_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.SCR_Addr	= TV_SCR_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.HDR_Addr	= TV_HDR_Addr + sizeof(TV_CMD_DESC)*i;
#ifdef CONFIG_RTL8672_ATM_QoS
		vcc->TV.QoS_PCR_Addr	= TV_QoS_PCR_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.QoS_SCR_Addr	= TV_QoS_SCR_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.QoS_SCR1_Addr	= TV_QoS_SCR1_Addr+ sizeof(TV_CMD_DESC)*i;
#endif
#if 0 // 8672 migration
		vcc->RV.CtlSts_Addr	= RV_Ctrl_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.FDP_Addr	= RV_FDP_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.CKS_Addr	= RV_CKS_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.HDR_Addr	= RV_HDR_Addr + sizeof(RV_CMD_DESC)*i;
#else
		/* pkta disabled mode..*/
		vcc->RV.CtlSts_Addr	= RV_Ctrl_Addr + 16*i;	// 8672 from 4 to 5	sizeof(RV_CMD_DESC)*i;
		vcc->RV.FDP_Addr	= RV_FDP_Addr + 16*i;	// 8672 from 4 to 5	sizeof(RV_CMD_DESC)*i;
		vcc->RV.CKS_Addr	= RV_CKS_Addr + 16*i;	// 8672 from 4 to 5	sizeof(RV_CMD_DESC)*i;
		vcc->RV.HDR_Addr	= RV_HDR_Addr + 16*i;	// 8672 from 4 to 5	sizeof(RV_CMD_DESC)*i;
#endif

		vcc->TV.SegmentCRC 	= 0xFFFFFFFF;
		vcc->RV.SegmentCRC 	= 0xFFFFFFFF;		
		
		vcc->TBE_Flag	= TRUE;
		vcc->created 		= VC_NOT_CREATED;
	}
    first_init=0;
	
	cp->vcc[OAM_CH_NO].TV.CtlSts_Addr	= TO_Ctrl_Addr;
	cp->vcc[OAM_CH_NO].TV.FDP_Addr	= TO_FDP_Addr;	
	cp->vcc[OAM_CH_NO].RV.CtlSts_Addr	= RO_Ctrl_Addr;
	cp->vcc[OAM_CH_NO].RV.FDP_Addr	= RO_FDP_Addr;
	
	cp->vcc[OAM_CH_NO].TV.CtlSts = 0;  //for reset rx CDOI
	cp->vcc[OAM_CH_NO].RV.CtlSts = 0;  //for reset tx CDOI
	
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672
	cp->vcc[OAM_CH_NO+1].RV.CtlSts_Addr	= RO1_Ctrl_Addr;
	cp->vcc[OAM_CH_NO+1].RV.FDP_Addr	= RO1_FDP_Addr;
	cp->vcc[OAM_CH_NO+1].RV.CtlSts = 0;  //for reset tx CDOI
#endif

	cp->CNFG_Reg	= 0x00000000;
	cp->STS_Reg		= 0x00000000;
	cp->TDFI_Reg	= 0x00000000;
	cp->TBEI_Reg	= 0x00000000;
	cp->RTOI_Reg	= 0x00000000;
	cp->RDAI_Reg	= 0x00000000;
	cp->RBFI_Reg	= 0x00000000;

	cp->tx_channel_on	=0x00000000;
	cp->rx_channel_on	=0x00000000;

	/* Assing FDP to SAR Register */
	for(i=0;i<Enable_VC_CNT;i++){
		cp->vcc[i].TV.FDP		= (UINT32)(cp->TVDesc + i*SAR_TX_RING_SIZE);
		cp->vcc[i].RV.FDP		= (UINT32)(cp->RVDesc + i*SAR_RX_RING_SIZE);
		cp->vcc[i].TV_Desc	= (TV_CMD_DESC *)(cp->TVDesc + i*SAR_TX_RING_SIZE);
		cp->vcc[i].RV_Desc	= (RV_STS_DESC *)(cp->RVDesc + i*SAR_RX_RING_SIZE);
		
	}
		
	cp->vcc[OAM_CH_NO].TV.FDP		= (UINT32)cp->TODesc;
	cp->vcc[OAM_CH_NO].RV.FDP		= (UINT32)cp->RODesc;	
	cp->vcc[OAM_CH_NO].TO_Desc		= (TO_CMD_DESC *)cp->TODesc;
	cp->vcc[OAM_CH_NO].RO_Desc		= (RO_STS_DESC *)cp->RODesc;	
		
	reg(cp->vcc[0].TV.FDP_Addr)	= (UINT32)cp->TVDesc;
	reg(cp->vcc[0].RV.FDP_Addr)	= (UINT32)cp->RVDesc;
	reg(cp->vcc[OAM_CH_NO].TV.FDP_Addr)	= (UINT32)cp->TODesc;
	reg(cp->vcc[OAM_CH_NO].RV.FDP_Addr)	= (UINT32)cp->RODesc;
	
	// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672
	cp->vcc[OAM_CH_NO+1].RV.FDP		= (UINT32)(cp->RODesc+ SAR_RX_RING_SIZE);	// 8672 RO1 support 
	cp->vcc[OAM_CH_NO+1].RO_Desc		= (RO_STS_DESC *)(cp->RODesc+ SAR_RX_RING_SIZE);// 8672 RO1 support 
	reg(cp->vcc[OAM_CH_NO+1].RV.FDP_Addr) = (UINT32)(cp->RODesc+ SAR_RX_RING_SIZE);	 	// 8672 RO1 support 
#endif
}

#if 0
void SetCRCTbl(void )
{
	UINT32 	i,tmp;
	UINT8 	j;
	
	for (i=0;i<0x0100;i++)
	{
		tmp =  i<<0x18;
		for (j=0;j<8;j++){
			if ((tmp & 0x80000000)!=0)
				tmp = (tmp<<1)^CRC32_POLY;
			else
				tmp = tmp<<1;
		};
		CRC32TBL[i]=tmp ;
	};
}

void GenCRC10Table(void)
{
	UINT16	Crc10Accum;
	int 		i, j;

	for (i = 0; i < 256; i++) {
		Crc10Accum = ((UINT16) i<<2);
		for (j = 0; j < 8; j++) {
			if ((Crc10Accum <<= 1) & 0x400) 
				Crc10Accum ^= CRC10_POLY	;
		}
		CRC10TBL[i] = Crc10Accum;
	}	
}
#endif


void Enable_Sachem_Loopback(void){
	/*REG16(0xb8000c44)=0x4100;*/	/* in FPGA */
#if 0	
	Idle(200000);
	REG16(0xb8600c44)=0xBE03;		/* in ASIC */
	Idle(200000);
	REG16(0xb8600032)=0x1800;
	Idle(200000);
	REG16(0xb8600162)=0x0000;
	Idle(200000);
#else
	// 8672 Sachem loopback on ATM port 0
	Idle(200000);
	REG16(0xb8600c44)=0x1D00;		
	Idle(200000);
	REG16(0xb8600032)=0x0800;
	Idle(200000);
	REG16(0xb8600c00)=0x2200;
	Idle(200000);
	REG16(0xb8600c20)=0xC600;
	Idle(200000);

#endif	
}

void Enable_Sachem_Utopia(void){
	Idle(200000);
	REG16(0xb8000c44)=0xBE03;		/* in ASIC */
	Idle(200000);
}

__SWAP
void Clear_TBE(int8 i, int8 CDOI)
{
#if 0//  8672 enable TBE.. mark 8671 old fashion
	uint32 tmp;

	tmp=*(unsigned int *)(0xb8300000+0x10*i);

	//sar_dev->vcc[i].TV.CtlSts |=TBEC;
	sar_dev->vcc[i].TV.CtlSts &= 0xFF00FFFF;
	sar_dev->vcc[i].TV.CtlSts |= ((uint32)CDOI)<<CDOI_Offset ;
	//tmp=sar_dev->vcc[i].TV.CtlSts;
	sar_dev->vcc[i].TV.CtlSts &=0x9fffffff;
	sar_dev->vcc[i].TV.CtlSts |= (tmp&0x60000000);
	/* Next Time we may not want to clear TBE */
	//sar_dev->vcc[i].TV.CtlSts &= (~TBEC);
#endif
	if(sar_dev){
//		printk("clear CH %d : %x = %x\n", i, sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts|TBEC);
	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts|TBEC);
	}
	return;
}

__SWAP
void Clear_RBF(sar_private *cp, int8 i, int8 CDOI)
{

	uint32 tmp=0;

	cp->vcc[i].RV.CtlSts |=RBFC;
/*	
	cp->vcc[i].RV.CtlSts &= 0xFF03FFFF;
	cp->vcc[i].RV.CtlSts |= ((uint32)CDOI)<<CDOI_Offset ;
*/	
	tmp=cp->vcc[i].RV.CtlSts;

	/* Next Time we may not want to clear RBF */
	cp->vcc[i].RV.CtlSts &= (~RBFC);

	WriteD_(cp->vcc[i].RV.CtlSts_Addr, tmp);
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, tmp);	
	return;
}


void Cell_Forced_Dropped(sar_private *cp, int8 i){
	uint32 tmp=0;

	cp->vcc[i].RV.CtlSts |=CFD;
	cp->vcc[i].RV.CtlSts |=RBFC;
		tmp=cp->vcc[i].RV.CtlSts;

	/* Next Time we may not want to clear RBF and drop cell */
	cp->vcc[i].RV.CtlSts &= (~RBFC);
	cp->vcc[i].RV.CtlSts &= (~CFD);

	WriteD_(cp->vcc[i].RV.CtlSts_Addr, tmp);

	return;
}

#if 1
//HW CDOI has bug, check desc to fix it
__SWAP
int GetTxCDOI(sar_private *pDrvCtrl, int ch_no)
{
  int currCDOI;
  int i=0;

	//return ((int)((REG32(pDrvCtrl->vcc[ch_no].TV.CtlSts_Addr)&CDOI_Mask)>>CDOI_Offset)&0x3F);
  
	currCDOI = pDrvCtrl->vcc[ch_no].TV.desc_pf;
	while (!((pDrvCtrl->TVDesc[ch_no*SAR_TX_RING_SIZE+currCDOI].CMD)&OWN)) {
	    currCDOI=(currCDOI+1)&(SAR_TX_DESC_NUM-1);
	    if ((i++)>=SAR_TX_DESC_NUM) {  //every tx desc wait to send pkt
	    	break;
	    };
	};
	return currCDOI;
}
#else
//HW CDOI has bug, check desc to fix it
__SWAP
int GetTxCDOI(sar_private *pDrvCtrl, int ch_no)
{
  int currCDOI, tmp;
  int i=0;
  
	currCDOI = pDrvCtrl->vcc[ch_no].TV.desc_pf;
	tmp= (currCDOI-1)&(SAR_TX_DESC_NUM-1);
	if (!((pDrvCtrl->TVDesc[ch_no*SAR_TX_RING_SIZE+tmp].CMD)&OWN)) 
	    return currCDOI;
	    
	while (!((pDrvCtrl->TVDesc[ch_no*SAR_TX_RING_SIZE+currCDOI].CMD)&OWN)) {
	    currCDOI=(currCDOI+1)&(SAR_TX_DESC_NUM-1);
	    if ((i++)>=SAR_TX_DESC_NUM) {  //every tx desc wait to send pkt
	    	break;
	    };
	};
	return currCDOI;
}
#endif

//HW CDOI has bug, check desc to fix it
int8 GetRxCDOI(sar_private *pDrvCtrl, int8 ch_no)
{
  int8 currCDOI;
  int i=0;
  
	//return ((int8)((REG32(pDrvCtrl->vcc[ch_no].RV.CtlSts_Addr)&CDOI_Mask)>>CDOI_Offset)&0x3F);
	
	currCDOI = pDrvCtrl->vcc[ch_no].RV.desc_pr;
	while (!((pDrvCtrl->RVDesc[ch_no*SAR_RX_RING_SIZE+currCDOI].STS)&OWN)) {
	    currCDOI=((currCDOI+1)>=SAR_RX_DESC_NUM)?0:(currCDOI+1);
	    if ((i++)>=SAR_RX_DESC_NUM) {  //every desc rx a pkt
	    	break;
	    };
	};
	return currCDOI;
}

void Reset_Sar(void){
	reg(Test_Reg_0)=0x00000001;
	Idle(200000);
	reg(Test_Reg_0)=0x00000000;
	Idle(200000);
	/*reg(Test_Reg_1)=0x00000000;*/
}

void Idle(int32 period){

	int32	i;

	for(i=0;i<(period);i++) asm("#sll\t$0, $0, 1\t\t\t# nop\n\t");
	return;
	
}

void Dump(uint32 Buffer, int32 size){

	int	k;	
	if(size%4)	size=size+4-(size%4);

	Buffer=Buffer&0xFFFFFFFC;
	
	if ((Buffer&0xF0000000)==0x80000000) Buffer |= Uncache_Mask;
	printk("Address  : Data");
	for(k=0;k<(size/4);k++){
		if ((k%4)==0) {
			printk ("\n");
			printk("%08X : ",Buffer+k*4);
		}		
		printk("%08X  ", reg(Buffer+k*4));
	}
	printk("\n");


}

void Search(uint32 pattern){

	uint32	i;

	printk("search pattern = 0x%08X\n", pattern);

	for(i=0;i<0x00FFFFFF;i=i+4){
		if(reg(0xA0000000+i)==pattern)	
			printk("--------->Match Address=0x%08X\n", 0xA0000000+i);
		if(!(i&0x000FFFFF))
			printk("Now Scan Address=0x%08X\n", 0xA0000000+i);
		
	}
	return;

}

void	SetVpiVci(sar_private *cp, uint8 VPI, uint16 VCI,int8  ch_no){

	uint32 HDR=0x00000000;
	HDR |=  ((((uint32) VPI)<< 20) | (((uint32) VCI) << 4));

	/* set VPI VCI value to corresponding channel, GFC is always set to 0 */
	cp->vcc[ch_no].TV.HDR = HDR;
	WriteD_(cp->vcc[ch_no].TV.HDR_Addr, cp->vcc[ch_no].TV.HDR);
	cp->vcc[ch_no].RV.HDR = HDR;
	WriteD_(cp->vcc[ch_no].RV.HDR_Addr, cp->vcc[ch_no].RV.HDR);
	cp->vcc[ch_no].vpi = VPI;
	cp->vcc[ch_no].vci = VCI;
	return;
}

//for ATM QoS mantisa value
#ifndef IC8671G	 
static int mantissa_CR[16]={ 
  	0x005, 0x016, 0x028, 0x03C, 0x051, 0x068, 0x080, 0x09B,
  	0x0B8, 0x0D8, 0x0FA, 0x120, 0x14B, 0x179, 0x1AE, 0x1E8 };
#endif
#define PRECISE_NUM 36
struct qos_rate_struct{
	unsigned short set_rate;
	unsigned short actual_rate;
};
static struct qos_rate_struct qos_rates[PRECISE_NUM]=
{{12,10},{23,20},{46,40},{78,70},{112,100},{160,143},{224,200},{256,250},
{342,301},{396,345},{447,400},{482,449},{511,500},{596,547},{676,598},{748,653},
{800,699},{844,744},{892,800},{936,859},{964,901},{996,955},{1020,1000},{1200,1103},
{1360,1207},{1488,1306},{1600,1406},{1696,1505},{1776,1600},{1856,1706},{1920,1802},
{1983,1910},{2032,2000},{2176,2098},{2240,2133},{2688,2415}};
int reg2cr(int rate){
	int mantisa, exp;
	unsigned int result;
	exp = (rate&0x00001E00)>>9;
	mantisa = rate&0x000001FF;
	result=((512+mantisa)<<exp)>>9;
	//printk("result:%d  %x\n",result,cr2reg(result));
	return result;
}
int check_precise(int rate){
	int i;
	for(i=0;i<PRECISE_NUM;i++){
		if(rate==qos_rates[i].actual_rate)
			return qos_rates[i].set_rate;
	}
	return 0;
}

int findProperRate(int which,int rate){ //which => 1:find upper_rate 0:find lower_rate
	int i;

	// out-of-bound conditions.
	if (rate <= qos_rates[0].actual_rate) {
		if (0==which) // lower
			return 1;
		else
			return qos_rates[0].set_rate;
	} else if (rate >= qos_rates[PRECISE_NUM-1].actual_rate) {
		return qos_rates[PRECISE_NUM-1].set_rate;
	}
	
	for(i=0;i<PRECISE_NUM-1;i++){
		//printk("rate:%d  %d  %d\n",rate,qos_rates[i].actual_rate,qos_rates[i+1].actual_rate);
		if((rate>=qos_rates[i].actual_rate) && (rate<qos_rates[i+1].actual_rate)){
			if (rate == qos_rates[i].actual_rate) {
				return qos_rates[i].set_rate;
			}
			
			if(which==1)//return upperCR
				return qos_rates[i+1].set_rate;	
			else//return lowerCR
				return qos_rates[i].set_rate;
		}
	}
	printk("FIND proper RATE ERROR  %d!!!!\n",rate);
	return 0;
}
int findActualRate(int which,int rate){
	int i;

	// out-of-bound conditions.
	if (rate <= qos_rates[0].actual_rate) {
		if (0==which) // lower
			return 1;
		else
			return qos_rates[0].actual_rate;
	} else if (rate >= qos_rates[PRECISE_NUM-1].actual_rate) {
		return qos_rates[PRECISE_NUM-1].actual_rate;
	}
	
	for(i=0;i<PRECISE_NUM-1;i++){
		//printk("rate:%d  %d  %d\n",rate,qos_rates[i].actual_rate,qos_rates[i+1].actual_rate);
		if((rate>qos_rates[i].actual_rate) && (rate<qos_rates[i+1].actual_rate)){
			if(which==1)//return upperCR
				return qos_rates[i+1].actual_rate;	
			else//return lowerCR
				return qos_rates[i].actual_rate;
		}
	}
	printk("FIND actual RATE ERROR  %d!!!!\n",rate);
	return 0;
}
#ifndef IC8671G	 
static int remap_mantisa(int rate)
{
  int mantisa, exp, i;
  
    exp     = (rate&0x00001E00)>>9;
    mantisa = rate&0x000001FF;
    for (i=0;i<16;i++) {
    	if (mantisa>mantissa_CR[i]) continue;
    	break;
    };
    if (i==16) {  //next exp level
    	exp++;
    	if (exp>=0x10) {  //overflow case
    		exp=0x0F;
            return ( (exp<<9)|mantissa_CR[15] );
    	} else 
            return ( (exp<<9)|mantissa_CR[0] );    	
    } else
        return ( (exp<<9)|mantissa_CR[i] );    
}
#endif
int exp2int(int rate) 
{
  int mantisa, exp;

	exp = (rate&0x00001E00)>>9;
    mantisa = rate&0x000001FF;
    return ( (1<<exp)+(1<<exp)*mantisa/512 );
}

int get_QoSlowerValue(int upperCR, int lowerCR, int rate, int factor)
{
  int value;
  int upperVal, lowerVal, rateVal;
  
    upperVal = exp2int(upperCR);
    lowerVal = exp2int(lowerCR);
    rateVal  = exp2int(rate);
    if ((upperVal == lowerVal)||(upperVal == rateVal)||(lowerVal == rateVal)) {
    	value = -1;  //the range is too small
    } else {
        value = (upperVal - rateVal)*factor/(upperVal - lowerVal);
    };
    return value;
}

void set_QoSParameter(sar_private *cp, int8 ch_no, int qos_type, int type, int rate)
{
#ifndef IC8671G//add for 2.6 and old QoS method
  int mantisa, exp, j;
#else
	int org_rate,set_rate;
#endif

  int upperCR, lowerCR;
  int actual_upperCR, actual_lowerCR;

	cp->vcc[ch_no].QoSinterpolateEnable = 0;
    cp->vcc[ch_no].numberCR = 0;
    
#ifdef IC8671G
	org_rate=reg2cr(rate);
	set_rate=check_precise(org_rate);
	if(set_rate){		
		if (type == 0) {
			cp->vcc[ch_no].QoSinterpolateEnable &= ~1;
			SetPCR(cp, ch_no, cr2reg(set_rate));
		}
		else
			SetSCR(cp, ch_no, cr2reg(set_rate));
		return;	
	}
	
	upperCR=cr2reg(findProperRate(1,org_rate));
	lowerCR=cr2reg(findProperRate(0,org_rate));
	actual_upperCR=cr2reg(findActualRate(1,org_rate));
	actual_lowerCR=cr2reg(findActualRate(0,org_rate));
	//printk("upper:%d  lower:%d\n",findProperRate(1,org_rate),findProperRate(0,org_rate));
	//printk("upper:%d  lower:%d\n",findActualRate(1,org_rate),findActualRate(0,org_rate));
	
#else
    if ((rate==remap_mantisa(rate))&&(qos_type==QoS_CBR)) {  //perfect case for CBR, HW CR match
    	//no need PCR interpolate
	    cp->vcc[ch_no].QoSinterpolateEnable &= ~1;	//tell timer to stop interpolate PCR of the channel
	    SetPCR(cp, ch_no, rate);			/*  Set Peak Cell Rate */
    	return;
    }
    //get cell rate upper & lower bound
    exp     = (rate&0x00001E00)>>9;
    mantisa = rate&0x000001FF;
    for (j=0;j<16;j++) {
    	if (mantisa>mantissa_CR[j]) continue;
    	break;
    };
    if (j==16) {
        upperCR = ( ((exp+1)<<9)|mantissa_CR[0] );
        lowerCR = ( (exp<<9)|mantissa_CR[15] );
    } else if (j==0) {
        upperCR = ( (exp<<9)|mantissa_CR[0]);
        lowerCR = ( ((exp-1)<<9)|mantissa_CR[15]);
    } else {
        upperCR = ( (exp<<9)|mantissa_CR[j]);
        lowerCR = ( (exp<<9)|mantissa_CR[j-1]);
    };
    actual_upperCR=upperCR;
    actual_lowerCR=lowerCR;
#endif

    //if(cp->vcc[ch_no].hwPRIO==3){
    //	lowerCR=upperCR;
    //	actual_lowerCR=actual_upperCR;
    //}
    //get cell rate swap ratio, the swap resolution is depend on system ticks(100Hz)
    if (type==0) {  //set PCR
        cp->vcc[ch_no].hwCR[UPPER_PCR] = upperCR;
	    cp->vcc[ch_no].hwCR[LOWER_PCR] = lowerCR;
	    cp->vcc[ch_no].hwCRtickData[LOWER_PCR] = get_QoSlowerValue(actual_upperCR, actual_lowerCR, rate, HZ);
	    if (cp->vcc[ch_no].hwCRtickData[LOWER_PCR]==-1) {
	        if (qos_type==QoS_CBR) {  //range too small, do not swap, set to upper bound
	    	    SetPCR(cp, ch_no, upperCR);
	    	    return;
	    	} else {
	    		cp->vcc[ch_no].hwCRtickData[LOWER_PCR] = 0;
	    	}
	    };
	    cp->vcc[ch_no].hwCRtickData[UPPER_PCR] = HZ - cp->vcc[ch_no].hwCRtickData[LOWER_PCR];
	    cp->vcc[ch_no].hwCRtick[UPPER_PCR] = 0;
	    cp->vcc[ch_no].hwCRtick[LOWER_PCR] = 0;
        cp->vcc[ch_no].QoSinterpolateEnable |= 1;	//tell timer to start interpolate PCR of the channel
        cp->vcc[ch_no].numberCR = 2;
    } else {  //set SCR
        cp->vcc[ch_no].hwCR[UPPER_SCR] = upperCR;
	    cp->vcc[ch_no].hwCR[LOWER_SCR] = lowerCR;
	    cp->vcc[ch_no].hwCRtickData[LOWER_SCR] = get_QoSlowerValue(actual_upperCR, actual_lowerCR, rate, HZ);
	    if (cp->vcc[ch_no].hwCRtickData[LOWER_SCR]==-1) {  //range too small, do not swap, set to upper bound
	    	cp->vcc[ch_no].hwCRtickData[LOWER_SCR] = 0;
	    }
	    cp->vcc[ch_no].hwCRtickData[UPPER_SCR] = HZ - cp->vcc[ch_no].hwCRtickData[LOWER_SCR];
	    cp->vcc[ch_no].hwCRtick[UPPER_SCR] = 0;
	    cp->vcc[ch_no].hwCRtick[LOWER_SCR] = 0;
        cp->vcc[ch_no].QoSinterpolateEnable |= 2;	//tell timer to start interpolate SCR of the channel
        cp->vcc[ch_no].numberCR = 4;

        // andrew, disable PCR swapping
        cp->vcc[ch_no].hwCRtickData[UPPER_PCR] = 0;
        cp->vcc[ch_no].hwCRtickData[LOWER_PCR] = 0;
	};
}

void SetQoS(sar_private *cp, int8 i, int8 QoS)
{
	/* Set Scheduler Option */
	uint32	SSL = 0 ;
	
	SSL |= (((uint32) QoS)<< 29);
	cp->vcc[i].QoS.Type=QoS;
	cp->vcc[i].TV.CtlSts &= 0x9FFFFFFF;
	cp->vcc[i].TV.CtlSts |= SSL;
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
}

//#define CR_OFFSET 1 
#define CR_OFFSET 2 // andrew, change to 2 so PCR960,SCR959 case will not lost cell.
char PrioChannelNum[5]={0,0,0,0,0};
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void SetQoSInterpolate(sar_private *cp, int8 i, struct SAR_IOCTL_CFG *cfg)
{
#if SAR_INTERPOLATE
  int pkt_exp=1,j;
#ifndef IC8671G
	int val, timeRatioPCR;
#endif
#endif
	//cli();
	local_irq_disable();	//2.6 migration

	SetQoS(cp, i, cfg->QoS.Type);			/* Set Scheduler Option */
    SetPCR(cp, i, cfg->QoS.PCR);
	SetSCR(cp, i, cfg->QoS.SCR);
	SetMBS(cp, i, cfg->QoS.MBS);

#if SAR_INTERPOLATE 
	SetQoS(cp, i, cfg->QoS.Type);			/* Set Scheduler Option */
    SetPCR(cp, i, cfg->QoS.PCR+CR_OFFSET);
	SetSCR(cp, i, 0);
	SetMBS(cp, i, 0);

	//calculate (cells per second*512)
	if(cfg->QoS.Type==QoS_CBR){
		for(j=0;j<((cfg->QoS.PCR&0x00001E00)>>9);j++){
			pkt_exp*=2;
		}
		cp->vcc[i].hwCellPerSecond=pkt_exp*(512+(cfg->QoS.PCR&0x000001FF));
		//printk("channel:%d   cells per second:%d\n",i,cp->vcc[i].hwCellPerSecond);
		cp->vcc[i].hwPRIO=1;
		PrioChannelNum[cp->vcc[i].hwPRIO]++;
		cp->vcc[i].hwTxCellCount=0;
	}
	else if(cfg->QoS.Type==QoS_rtVBR ||cfg->QoS.Type==QoS_nrtVBR){
		for(j=0;j<((cfg->QoS.PCR&0x00001E00)>>9);j++){
			pkt_exp*=2;
		}
		cp->vcc[i].hwCellPerSecond=pkt_exp*(512+(cfg->QoS.PCR&0x000001FF));
		//printk("channel:%d   cells per second:%d\n",i,cp->vcc[i].hwCellPerSecond);
		if(cfg->QoS.Type==QoS_rtVBR)
			cp->vcc[i].hwPRIO=2;
		else
			cp->vcc[i].hwPRIO=3;
		PrioChannelNum[cp->vcc[i].hwPRIO]++;
		cp->vcc[i].hwTxCellCount=0;
	}
	else{
		for(j=0;j<((cfg->QoS.PCR&0x00001E00)>>9);j++){
			pkt_exp*=2;
		}
		cp->vcc[i].hwCellPerSecond=pkt_exp*(512+(cfg->QoS.PCR&0x000001FF));
		//cp->vcc[i].hwCellPerSecond=0;
		cp->vcc[i].hwPRIO=4;
		PrioChannelNum[cp->vcc[i].hwPRIO]++;
		cp->vcc[i].hwTxCellCount=0;
	}

	switch (cfg->QoS.Type) {
		case QoS_rtVBR:
		case QoS_nrtVBR:
//0921 shlee			cfg->QoS.Type &= 1;  //remap priority
			SetQoS(cp, i, cfg->QoS.Type);			/* Set Scheduler Option */
			SetMBS(cp, i, cfg->QoS.MBS);	//0921 shlee

		    set_QoSParameter(cp, i, QoS_rtVBR, 0, cfg->QoS.PCR+CR_OFFSET);
	        set_QoSParameter(cp, i, QoS_rtVBR, 1, cfg->QoS.SCR+CR_OFFSET);

		    SetSCR(cp, i, cp->vcc[i].hwCR[UPPER_SCR]);		//0922 shlee	        
#ifndef IC8671G		    
		    //recalculate time ratio
		    val = exp2int(cfg->QoS.SCR);
		    if ( cfg->QoS.MBS>= val) //use PCR to send all the time
		    	timeRatioPCR = 100;
		    else {
		        timeRatioPCR = cfg->QoS.MBS*100/val+1;
		        if (timeRatioPCR>100) timeRatioPCR=100;
		    }
		    cp->vcc[i].hwCRtickData[LOWER_SCR] = cp->vcc[i].hwCRtickData[LOWER_SCR]*(100-timeRatioPCR)/100;
		    cp->vcc[i].hwCRtickData[UPPER_SCR] = cp->vcc[i].hwCRtickData[UPPER_SCR]*(100-timeRatioPCR)/100;
		    
		    cp->vcc[i].hwCRtickData[LOWER_PCR] = cp->vcc[i].hwCRtickData[LOWER_PCR]*timeRatioPCR/100;
		    cp->vcc[i].hwCRtickData[UPPER_PCR] = 100 - cp->vcc[i].hwCRtickData[LOWER_PCR]
		    							- cp->vcc[i].hwCRtickData[LOWER_SCR] - cp->vcc[i].hwCRtickData[UPPER_SCR];
#endif
		    break;
		case QoS_CBR:
		    set_QoSParameter(cp, i, QoS_CBR, 0, cfg->QoS.PCR+CR_OFFSET);
		    break;
		    
		case QoS_UBR:
			set_QoSParameter(cp, i, QoS_CBR, 0, cfg->QoS.PCR+CR_OFFSET);
		default:
		    break;
	};
#endif
	//sti();
	local_irq_enable();	// 2.6 migration
	return;	
}
char use_low_rate=0;
char use_low_rate_UBR=0;
char ubr_low_rate=0;
void swapCR(sar_private *cp, int ch_no)
{
	sar_atm_vcc	*vcc_dev;
	int idx;

		vcc_dev = &cp->vcc[ch_no];
		if (vcc_dev->numberCR==0) return;
		
		idx=vcc_dev->creditCR;
		if (!(vcc_dev->hwCRtick[idx])) {  ////timeout, change to next CR
			do {
			    idx=(idx+1)%(vcc_dev->numberCR);
			} while (!(vcc_dev->hwCRtickData[idx]));
			
			if (vcc_dev->hwCRtickData[idx])
		    	   vcc_dev->hwCRtick[idx] = vcc_dev->hwCRtickData[idx]-1;
			
		    if(use_low_rate && vcc_dev->hwPRIO==2)
		    	SetPCR(cp, ch_no, vcc_dev->hwCR_low[idx]);  //traffic more than line rate, decrease
		    else{
		    	if(vcc_dev->hwPRIO!=4 || ubr_low_rate==0) //not UBR or UBR normal mode
	    		{	//0921 shlee
		    		if(vcc_dev->hwPRIO == 2 || vcc_dev->hwPRIO==3)	{ //0921 shlee for rtVBR
	    				//SetSCR(cp, ch_no, vcc_dev->hwCR[UPPER_SCR]);	
		    			//SetPCR(cp, ch_no, vcc_dev->hwCR[idx]);
		    			// andrew, interpolate using SCR so we can have burst
		    			SetPCR(cp, ch_no, vcc_dev->hwCR[UPPER_PCR]);	
		    			SetSCR(cp, ch_no, vcc_dev->hwCR[idx]);
		    		
		    		} else	{	//0921 shlee 
		    		SetPCR(cp, ch_no, vcc_dev->hwCR[idx]);
		    		}
	    		}	//0921 shlee
		    }
		    vcc_dev->creditCR = idx;
		    //printk("CCR%d:%d %08x %d\n", ch_no, jiffies, vcc_dev->hwCR[idx], vcc_dev->hwCRtickData[idx]);
	    } else 
	        vcc_dev->hwCRtick[idx]--;
}

extern int total_pvc_number;
int CurrentUBR=0;
extern char checkRateEnable;
extern T_LinkSpeed LINE_rate;
char rtvbr_improve=0;
#define rtvbr_margin 30
#define ubr_margin 10
#if 0// SAR_CHECK_RC_NEW
static unsigned int rate_UBR = 1;

#define NUM_HISTORY 16
static unsigned int rate_history[NUM_HISTORY];
static int rate_index = 0, debugMsg = 0;

void check_CR(sar_private *cp){
	sar_atm_vcc *vcc_dev;
	int totalAvaiable = 0, totalActualUse = 0, nrtVBRActualUse = 0;
	int totalUsable = 0, targetRate;
	int totalUBR = 0, totalHwCR = 0;
	unsigned int pool = 0;
	int i;

	if (!debugMsg) {
		debugMsg = 1;
		
	}
	printk("**New Check CR\n");
	//get line rate
	if((sar_dev==NULL || checkRateEnable==0) || (PrioChannelNum[4] == 0))
		goto THE_END;

	totalAvaiable=LINE_rate.upstreamRate*1000/(53*8);

	if ((totalAvaiable==0) )
		goto THE_END;

	// compute the actual usage of (CBR+rtVBR+nrtVBR)
	//printk("==>");
	for (i=0;i<total_pvc_number;i++) {
		//printk("(%d,%d,%d)",i, cp->vcc[i].hwPRIO, cp->vcc[i].hwTxCellCount);
		if(cp->vcc[i].hwPRIO<=3) 
			totalActualUse+=cp->vcc[i].hwTxCellCount;

		if(cp->vcc[i].hwPRIO==3)
			nrtVBRActualUse+=cp->vcc[i].hwTxCellCount;
		else if (cp->vcc[i].hwPRIO==4)
			totalUBR+=cp->vcc[i].hwCellPerSecond;

		//totalHwCR +=cp->vcc[i].hwCellPerSecond;
	}
	//printk("\n\n");

	// covert from /500ms to /1sec.
	totalActualUse /= 256;
	nrtVBRActualUse /=256;
	totalUBR /=512;

	// total configured rate.
	totalHwCR /=512;
	

	// Other priority already exceed to limit, set UBR to lowest possible rate.
	if (totalActualUse >= totalAvaiable) {
		targetRate = 1;
	} else {
		totalUsable = 	totalAvaiable - totalActualUse;
		targetRate = totalUsable / PrioChannelNum[4];
		//targetRate = (targetRate < cp->vcc[i].hwCellPerSecond/512) ? targetRate : (cp->vcc[i].hwCellPerSecond/512);
	}

	

	if (nrtVBRActualUse && (totalUsable <= totalUBR)) {
		ubr_low_rate = 1; // disable swapping.
		printk("LowR %d, %d, %d\n", nrtVBRActualUse, totalUsable, totalUBR);
	} else {
		ubr_low_rate = 0; // enable rate swapping.
	}

	#if 1
	// adaptive increase UBR rate;
	if (targetRate >= rate_UBR) {
		rate_UBR = (rate_UBR + targetRate * 5) / 6;
	} else {
		rate_UBR = targetRate;
	}
	#endif

	//printk("[%d %d %d %d %d]\n", totalAvaiable, totalActualUse, targetRate, rate_UBR, nrtVBRActualUse);

	// debug only
	#if 0
	rate_history[rate_index % NUM_HISTORY] = rate_UBR;
	rate_index ++;
	if ((rate_index % NUM_HISTORY) == 0) {
		printk("index %d\n", rate_index);
		printk("%d %d %d %d %d %d %d %d\n", 
			rate_history[0], rate_history[1], rate_history[2], rate_history[3],
			rate_history[4], rate_history[5], rate_history[6], rate_history[7]);
		printk("%d %d %d %d %d %d %d %d\n", 
			rate_history[8], rate_history[9], rate_history[10], rate_history[11],
			rate_history[12], rate_history[13], rate_history[14], rate_history[15]);
	}
	#endif

	//pool = totalUsable;
	for(i=0;i<total_pvc_number;i++){		
		unsigned int setRate, vcRate;
		if(cp->vcc[i].hwPRIO!=4)
			continue;

		vcRate = (cp->vcc[i].hwCellPerSecond/512);
		if (rate_UBR > vcRate) {
			pool += (rate_UBR - vcRate); // save # of unused bandwidth
			setRate = vcRate;
		} else {			
			setRate = rate_UBR;
		}
		//printk("rate = (%d, %d) %d\n", i, cp->vcc[i].hwCellPerSecond, setRate);
		SetPCR(cp, i, cr2reg ( findProperRate(0, setRate) ));
	};

	// distribute unused bandwidth to UBR
	for(i=0;i<total_pvc_number;i++){		
		unsigned int setRate, vcRate;
		if (pool == 0)
			break;
			
		if(cp->vcc[i].hwPRIO!=4) 
			continue;		

		vcRate = (cp->vcc[i].hwCellPerSecond/512);

		if (vcRate < rate_UBR)
			continue;

		if ( (vcRate - rate_UBR) > pool) {
			setRate = rate_UBR + pool;
			pool = 0;
		} else {
			setRate = vcRate;
			pool -= (vcRate - rate_UBR);
		}
					
		SetPCR(cp, i, cr2reg ( findProperRate(0, setRate) ));
	};
	
THE_END:
	// reset the counter
	for (i=0;i<total_pvc_number;i++) {
			cp->vcc[i].hwTxCellCount=0;
	}

}

#endif

#ifdef IC8671G
void check_CR(sar_private *cp){
//	sar_atm_vcc *vcc_dev;
	int Hw_UpRate,Hw_UpRate2,tx_cell;
//	int HwThroughput1=0,HwThroughput2=0;
	int HwThroughput2=0;
	int i;//,j,k;
	//int mantisa, exp, j;
//	int  lowerCR,adjust_ubr, adjust_total_load=0;
	int adjust_ubr, adjust_total_load=0;
//	int rtvbr_send=0,nrtVBR_send=0,cbr_send=0;
	int nrtVBR_send=0;
//	unsigned int adjust_rtVBR;
	int lowest_ubr=0;
	int reserveHwRate; //rate reserved after cbr & rt-vbr
	int Hw_UpRate_rtvbr;
	int proper_rate;

	//get line rate
	if(sar_dev==NULL || checkRateEnable==0)
		return;


	tx_cell=LINE_rate.upstreamRate*1000/(53*8);

	if (tx_cell==0)
		return;
	Hw_UpRate=tx_cell*512/2;  //line rate, check every 500ms
	reserveHwRate=Hw_UpRate*2; //total cells that upstream can support
	Hw_UpRate2=Hw_UpRate;
	Hw_UpRate_rtvbr=Hw_UpRate;
	//printk("US:%d\n",rate.upstreamRate);

	//for nrt-VBR and UBR
	for (i=0;i<total_pvc_number;i++) {
		HwThroughput2+=cp->vcc[i].hwTxCellCount;
		if(cp->vcc[i].hwPRIO==3)
			nrtVBR_send+=cp->vcc[i].hwTxCellCount;
	}
	//printk("HwThroughput2:%d      Hw_UpRate2:%d       nrtVBR:%d\n",HwThroughput2,Hw_UpRate2,nrtVBR_send);

	if(((HwThroughput2 > Hw_UpRate2) || ubr_low_rate==1) && nrtVBR_send >0 && PrioChannelNum[4]>0){ //decrease UBR rate
		if(use_low_rate_UBR==0){
			//decrease ubr rate from now on
			//printk("Decrease UBR rate!\n");
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO==4){
					//set to low rate
					adjust_ubr=cr2reg(1);
					CurrentUBR=1;
					//printk("set ubr %d  %x\n",i,adjust_ubr);
					SetPCR(cp,i,adjust_ubr);
				}
			}
			use_low_rate_UBR=1;
			ubr_low_rate=1;
		}
		else{
			//adjust ubr rate
			//Hw_UpRate2*=2;
			//printk("Adjust UBR rate!\n");
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO<4){
					adjust_total_load+=cp->vcc[i].hwTxCellCount;

				}else{
					if(lowest_ubr==0)
						lowest_ubr=cp->vcc[i].hwCellPerSecond;
					else if(cp->vcc[i].hwCellPerSecond<lowest_ubr)
						lowest_ubr=cp->vcc[i].hwCellPerSecond;
				}
			}
			//printk("adjust_total_load:%d\n",adjust_total_load);
			Hw_UpRate2=(Hw_UpRate2-adjust_total_load)*2;
			if(Hw_UpRate2<=0)
				Hw_UpRate2=0;

			CurrentUBR=((Hw_UpRate2/PrioChannelNum[4])/512+CurrentUBR)/2;
			if(CurrentUBR<1)
				CurrentUBR=1;
			if(CurrentUBR>40){
				proper_rate=findProperRate(0, CurrentUBR - ubr_margin);
				//adjust_ubr=findActualRate(0, proper_rate);
				adjust_ubr= cr2reg(proper_rate);
			}
			else{
				adjust_ubr= cr2reg(1);
			}
			
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO==4){
					//printk("adjust ubr channel %d  %x\n",i,adjust_ubr);
					SetPCR(cp,i,adjust_ubr);
				}
			}
			//SetPCR(cp,i,adjust_ubr);
		}
	}
	else if(ubr_low_rate==1 && nrtVBR_send==0)
	{
		//printk("Normal UBR Rate!\n");
		//UBR normal rate
		for(i=0;i<total_pvc_number;i++){
			if(cp->vcc[i].hwPRIO==4){
				CurrentUBR=cp->vcc[i].hwCellPerSecond/512;
				adjust_ubr=cr2reg(cp->vcc[i].hwCellPerSecond/512);
				//printk("channel %d ubr normal rate! %x\n",i,adjust_ubr);
				SetPCR(cp,i,adjust_ubr);
			}
		}
		use_low_rate_UBR=0;
		ubr_low_rate=0;
	}

	for (i=0;i<total_pvc_number;i++) {
			cp->vcc[i].hwTxCellCount=0;
	}

}

#else

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void check_CR(sar_private *cp){
	int Hw_UpRate,Hw_UpRate2,tx_cell;
	int HwThroughput1=0,HwThroughput2=0;
	int i;
	int mantisa, exp, j;
	int  lowerCR,adjust_ubr, adjust_total_load=0;
	int rtvbr_send=0,nrtVBR_send=0,cbr_send=0;
	int adjust_rtVBR;
	int lowest_ubr=0;
	int reserveHwRate; //rate reserved after cbr & rt-vbr
	int Hw_UpRate_rtvbr;

	//get line rate
	if(sar_dev==NULL || checkRateEnable==0)
		return;


	tx_cell=LINE_rate.upstreamRate*1000/(53*8);

	if (tx_cell==0)
		return;
	Hw_UpRate=tx_cell*512/2;  //line rate, check every 500ms
	reserveHwRate=Hw_UpRate*2; //total cells that upstream can support
	Hw_UpRate2=Hw_UpRate;
	Hw_UpRate_rtvbr=Hw_UpRate;
	//printk("US:%d\n",rate.upstreamRate);

	//check pkts received in CBR,rt-VBR channels
	for (i=0;i<total_pvc_number;i++) {
		if(cp->vcc[i].hwPRIO==1 || cp->vcc[i].hwPRIO==2){
			HwThroughput1+=cp->vcc[i].hwTxCellCount;
			reserveHwRate-=cp->vcc[i].hwCellPerSecond;//for rt-vbr issue
		}
		if(cp->vcc[i].hwPRIO==1)
			cbr_send+=cp->vcc[i].hwTxCellCount;
		if(cp->vcc[i].hwPRIO==2)
			rtvbr_send+=cp->vcc[i].hwTxCellCount;
	}

	if(rtvbr_improve==0 && use_low_rate==1){
		//improve rtvbr rate
		Hw_UpRate_rtvbr*=2;
		Hw_UpRate_rtvbr=Hw_UpRate_rtvbr-(cbr_send*2)-rtvbr_margin;
		if(Hw_UpRate_rtvbr<=0)
			Hw_UpRate_rtvbr=1;
		adjust_rtVBR=cr2reg(Hw_UpRate_rtvbr/(PrioChannelNum[2]*512)-1);
		exp     = (adjust_rtVBR&0x00001E00)>>9;
		mantisa = adjust_rtVBR&0x000001FF;
		for (j=0;j<16;j++) {
			if (mantisa>mantissa_CR[j]) continue;
			break;
		};
		if (j==16) {
			lowerCR = ( (exp<<9)|mantissa_CR[15] );
  		} else if (j==0) {
			lowerCR = ( ((exp-1)<<9)|mantissa_CR[15]);
		} else {
			lowerCR = ( (exp<<9)|mantissa_CR[j-1]);
		};
		for(i=0;i<total_pvc_number;i++){
			if(cp->vcc[i].hwPRIO==2){//rt-VBR
				cp->vcc[i].hwCR_low[0]=lowerCR;
				cp->vcc[i].hwCR_low[1]=lowerCR;
				cp->vcc[i].hwCR_low[2]=lowerCR;
				cp->vcc[i].hwCR_low[3]=lowerCR;
				//printk("improve rtvbr set channel %d to %x\n",i,lowerCR);
			}
		}
		rtvbr_improve=1;
	}
	
	if(HwThroughput1 > Hw_UpRate && use_low_rate==0 && cbr_send>0 && PrioChannelNum[2]!=0 && reserveHwRate<0){ //if reserveHwRate>0, no need to decrease PCR for rt-vbr
  		//printk("decrease VBR rate!!HwThroughput:%d   HW_UpRate:%d\n",HwThroughput1,Hw_UpRate);
		//decrease rt-VBR rate
		Hw_UpRate*=2; //for 1 sec.
		for (i=0;i<total_pvc_number;i++) {
			if(cp->vcc[i].hwPRIO==1)//CBR
				Hw_UpRate-=cp->vcc[i].hwCellPerSecond;
		}
		//share throughput to rt-VBR
		//printk("adjust cell rate:%d\n",Hw_UpRate);
		if(Hw_UpRate<=0)
			Hw_UpRate=10;
		adjust_rtVBR=cr2reg(Hw_UpRate/(PrioChannelNum[2]*512)-1);
		//get cell rate upper & lower bound
		exp     = (adjust_rtVBR&0x00001E00)>>9;
		mantisa = adjust_rtVBR&0x000001FF;
		for (j=0;j<16;j++) {
			if (mantisa>mantissa_CR[j]) continue;
			break;
		};
		if (j==16) {
			lowerCR = ( (exp<<9)|mantissa_CR[15] );
  		} else if (j==0) {
			lowerCR = ( ((exp-1)<<9)|mantissa_CR[15]);
		} else {
			lowerCR = ( (exp<<9)|mantissa_CR[j-1]);
		};
		for(i=0;i<total_pvc_number;i++){
			if(cp->vcc[i].hwPRIO==2){//rt-VBR
				cp->vcc[i].hwCR_low[0]=lowerCR;
				cp->vcc[i].hwCR_low[1]=lowerCR;
				cp->vcc[i].hwCR_low[2]=lowerCR;
				cp->vcc[i].hwCR_low[3]=lowerCR;
				//printk("CR1 set channel %d to %x\n",i,lowerCR);
			}
		}
		use_low_rate=1;
	}else if(cbr_send==0){
		rtvbr_improve=0;
		use_low_rate=0;
	}

	//for nrt-VBR and UBR
	for (i=0;i<total_pvc_number;i++) {
		HwThroughput2+=cp->vcc[i].hwTxCellCount;
		if(cp->vcc[i].hwPRIO==3)
			nrtVBR_send+=cp->vcc[i].hwTxCellCount;
	}
	//printk("HwThroughput2:%d      Hw_UpRate2:%d       nrtVBR:%d\n",HwThroughput2,Hw_UpRate2,nrtVBR_send);

	if(((HwThroughput2 > Hw_UpRate2) || ubr_low_rate==1) && nrtVBR_send >0 && PrioChannelNum[4]>0){ //decrease UBR rate
		if(use_low_rate_UBR==0){
			//decrease ubr rate from now on
			//printk("Decrease UBR rate!\n");
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO==4){
					if(use_low_rate==1)
						adjust_ubr=cr2reg(5);
					else
						adjust_ubr=cr2reg(cp->vcc[i].hwCellPerSecond/(512*16));
					CurrentUBR=cp->vcc[i].hwCellPerSecond/(512*16);
					//printk("set ubr %d  %x\n",i,adjust_ubr);
					SetPCR(cp,i,adjust_ubr);
				}
			}
			use_low_rate_UBR=1;
			ubr_low_rate=1;
		}
		else{
			//adjust ubr rate
			//Hw_UpRate2*=2;
			//printk("Adjust UBR rate!\n");
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO<4){
					adjust_total_load+=cp->vcc[i].hwTxCellCount;

				}else{
					if(lowest_ubr==0)
						lowest_ubr=cp->vcc[i].hwCellPerSecond;
					else if(cp->vcc[i].hwCellPerSecond<lowest_ubr)
						lowest_ubr=cp->vcc[i].hwCellPerSecond;
				}
			}
			//printk("adjust_total_load:%d\n",adjust_total_load);
			Hw_UpRate2=(Hw_UpRate2-adjust_total_load)*2;
			if(Hw_UpRate2<=0)
				Hw_UpRate2=5;

			CurrentUBR=((Hw_UpRate2/PrioChannelNum[4])/512+CurrentUBR)/2;
			if(CurrentUBR<lowest_ubr/(512*16))
				CurrentUBR=lowest_ubr/(512*16);

			if(use_low_rate==1)
				adjust_ubr= cr2reg(5);
			else
				adjust_ubr= cr2reg(CurrentUBR - ubr_margin);
			for(i=0;i<total_pvc_number;i++){
				if(cp->vcc[i].hwPRIO==4){
					//printk("adjust ubr channel %d  %x\n",i,adjust_ubr);
					SetPCR(cp,i,adjust_ubr);
				}
			}
			//SetPCR(cp,i,adjust_ubr);
		}
	}
	else if(ubr_low_rate==1 && nrtVBR_send==0)
	{
		//printk("Normal UBR Rate!\n");
		//UBR normal rate
		for(i=0;i<total_pvc_number;i++){
			if(cp->vcc[i].hwPRIO==4){
				CurrentUBR=cp->vcc[i].hwCellPerSecond/512;
				adjust_ubr=cr2reg(cp->vcc[i].hwCellPerSecond/512);
				//printk("channel %d ubr normal rate! %x\n",i,adjust_ubr);
				SetPCR(cp,i,adjust_ubr);
			}
		}
		use_low_rate_UBR=0;
		ubr_low_rate=0;
	}

	for (i=0;i<total_pvc_number;i++) {
			cp->vcc[i].hwTxCellCount=0;
	}

}


#endif

#if 0
/* To solve tr069 will slow down the throughput. */
extern unsigned int tr069_pid;
long tr069_counter[3]={0,0,0};
void tr069_reduce_nice(void)
{
	if(tr069_pid)
	{
		struct task_struct *tsk, *pp, *ppp;
		
		tr069_counter[0]=0;
		tr069_counter[1]=0;
		tr069_counter[2]=0;
		tsk = find_task_by_pid( tr069_pid );
		if(tsk)
		{
			//printk("0");
			tsk->nice = 2000;
			tr069_counter[0]=tsk->counter;

			pp = tsk->p_pptr;
			if(pp)
			{
				pp->nice=2000;	
				tr069_counter[1]=pp->counter;
				
				ppp = pp->p_pptr;
				if(ppp)
				{
				   ppp->nice=2000;
				   tr069_counter[2]=ppp->counter;
				}
			}
		}
		
		//printk("kill SIGUSR1 signal to process tr069_pid (%d)\n", tr069_pid);
		//sys_kill(tr069_pid, SIGUSR1);	//raise sigusr2 to process startup
	}
}
void tr069_increase_nice()
{
	if(tr069_pid)
	{
		struct task_struct *tsk, *pp, *ppp;
		tsk = find_task_by_pid( tr069_pid );
		if(tsk)
		{
			//printk("1");
			tsk->nice=0;
			tsk->counter=tr069_counter[0];
			
			pp = tsk->p_pptr;
			if(pp)
			{
				pp->nice=0;
				pp->counter=tr069_counter[1];
	
				ppp = pp->p_pptr;
				if(ppp)
				{
					ppp->nice=0;
					ppp->counter=tr069_counter[2];
				}
			}
		}

		//printk("kill SIGUSR2 signal to process tr069_pid (%d)\n", tr069_pid);
		//sys_kill(tr069_pid, SIGUSR2);	//raise sigusr2 to process startup
	}		
}
#endif

//call by timer interrupt to interpolate Cell Rate
int Int_Count=0;
//for small pkt
int check_small_pkt=0;
int sec_check=0;
int sec_pkt=0;
extern int rx_small_pkt;
int change_BitSwap=0;
extern void EnterHeavyTraffic(void);
extern void ExitHeavyTraffic(void);
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
extern unsigned int SAR_TDFI_get_and_clear(void);
extern void clearSARvtxBuffer(int port, struct net_device *dev);
#endif
#if defined( CONFIG_EXT_SWITCH) && !defined(CONFIG_ETHWAN)
static int nicLinkStatCheckCnt=0;
#endif

void ATM_QoS_Interpolate(void)
{
	int i=0;
	unsigned int TDFI_Value;

	if (sar_dev==NULL) return;
#ifdef FAST_ROUTE_Test
	// Kaohj -- pass frame up each tick for MAC learning
	// called every tick
	fast_frame_up = 1;
#endif
	
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
	if ((TDFI_Value = SAR_TDFI_get_and_clear())) {
		for(i=0;i<Enable_VC_CNT ;i++){
			if(!(TDFI_Value&((unsigned int)0x00000001<<i)))
				continue;
			clearSARvtxBuffer(i, sar_dev->vcc[i].dev);
		}
	}
	if ((TDFI_Value = reg(SAR_TDFI_Addr))) {
		reg(SAR_TDFI_Addr) = (TDFI_Value & ((UINT32)0x00000001<<HW_OAM_CH_NO));	// write to clear TDFI reg
		if (sar_dev->TDFI_Reg&((UINT32)0x00000001<<HW_OAM_CH_NO))
			ClearTxBuffer(sar_dev, OAM_CH_NO);
	}
#else
	if ((TDFI_Value = reg(SAR_TDFI_Addr))) {
		reg(SAR_TDFI_Addr) = TDFI_Value;	// write to clear TDFI reg
	for(i=0;i<Enable_VC_CNT ;i++){
		if(!(TDFI_Value&((unsigned int)0x00000001<<i)))
			continue;
		ClearTxBuffer(sar_dev, i);
	}
		if (sar_dev->TDFI_Reg&((UINT32)0x00000001<<HW_OAM_CH_NO))
			ClearTxBuffer(sar_dev, OAM_CH_NO);
	}
#endif
#if defined( CONFIG_EXT_SWITCH) && !defined(CONFIG_ETHWAN)
	if (nicLinkStatCheckCnt++ >= 50 )//check link status per port every 500ms
	{
		static unsigned char portstatus[SW_PORT_NUM]={0};
		unsigned short regValue;
		static unsigned int swport=0;
		extern int virt2phy[5];
		extern unsigned char eth_close;
		extern int sw_link_stable_times[SW_PORT_NUM];
		extern int sw_link_checked[SW_PORT_NUM];
		extern unsigned int swtype;
		extern struct timer_list sw_check_timer;
		extern void miiar_read(unsigned char phyaddr, unsigned char regaddr, unsigned short *value);

		nicLinkStatCheckCnt = 0;

		if (eth_close)
			goto OMITNICLINKCHECK;
		
		if (0x5988 == swtype) {
			miiar_read(virt2phy[swport], 1, &regValue);
			//If the link had ever failed, this bit will be 0 until after reading this bit again.
			miiar_read(virt2phy[swport], 1, &regValue);
			if (((regValue>>2)&0x01) != portstatus[swport]) {
				if ((regValue>>2)&0x01)
					printk("port%d is link up\n", swport);
				else
					printk("port%d is link down\n", swport);
				portstatus[swport] = ((regValue>>2)&0x01);

				sw_link_stable_times[i]=0;
				sw_link_checked[i]=0;
				if(!timer_pending(&sw_check_timer)){
					mod_timer(&sw_check_timer, jiffies + 1*HZ);
				}
			}
			
		}
		if (++swport >= SW_PORT_NUM)
				swport = 0;
	}
OMITNICLINKCHECK:
#endif

#if SAR_INTERPOLATE
#if SAR_CHECK_RC
	if(Int_Count==50){ //check every 0.5 sec.
		check_CR(sar_dev);
		Int_Count=0;
	}
	else{
		Int_Count++;
	}
#endif
    for (i=0;i<total_pvc_number;i++) {
    	if (sar_dev->vcc[i].QoSinterpolateEnable)
    		swapCR(sar_dev, i);
	};
#endif
#if 1
	if(check_small_pkt==10){ //if rx 4000 small pkts in 0.5 min, disable bitswap
		if(rx_small_pkt>800){
			if( !change_BitSwap ){
				//if(tr069_pid) tr069_reduce_nice();
				EnterHeavyTraffic();
				change_BitSwap=1;
			}
		}
		
		if(sec_check==10){
			if(sec_pkt<1000) {
				if( change_BitSwap ){
					//if(tr069_pid) tr069_increase_nice();
					ExitHeavyTraffic();
					change_BitSwap=0;
				}
			}
			sec_check = 0;
			sec_pkt = 0;
		}
		else {
			sec_check++;
			sec_pkt+=rx_small_pkt;
		}		
		rx_small_pkt=0;
		check_small_pkt=0;
	}
	else{
		check_small_pkt++;
	}
#endif
}

void	SetPCR(sar_private *cp, int8 i, uint16 PCR)
{
	/* Set Peak Cell Rate */
	cp->vcc[i].TV.CtlSts &= 0xFFFF0000;
#ifndef CONFIG_RTL8672_ATM_QoS 	
	cp->vcc[i].QoS.PCR=PCR;
	cp->vcc[i].TV.CtlSts |= (uint32)PCR; 
#else
	cp->vcc[i].QoS.PCR=PCR_Period;
	cp->vcc[i].TV.CtlSts |= PCR_Period; 	
	WriteD_(cp->vcc[i].TV.QoS_PCR_Addr, (uint32)QoS_CLK*PCR_Period/PCR);
#endif

	//printk("cp->vcc[i].TV.CtlSts %x\n", cp->vcc[i].TV.CtlSts);
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;	
}

void	SetSCR(sar_private *cp, int8 i, uint16 SCR)
{
	/* Set Sustainable Cell Rate */
	cp->vcc[i].TV.SCR &= 0x0000FFFF;
#ifndef CONFIG_RTL8672_ATM_QoS
	cp->vcc[i].QoS.SCR=SCR;
	cp->vcc[i].TV.SCR |= ((uint32)SCR)<<16;
#else
	cp->vcc[i].QoS.SCR=SCR_Period;
	cp->vcc[i].TV.SCR |= SCR_Period<<16;	
	if(SCR) //don't have to set SCR value in CBR,UBR mode
		WriteD_(cp->vcc[i].TV.QoS_SCR_Addr, (uint32)QoS_CLK*SCR_Period/SCR);
#endif

	//printk("cp->vcc[i].TV.SCR %x\n", cp->vcc[i].TV.SCR);
	WriteD_(cp->vcc[i].TV.SCR_Addr, cp->vcc[i].TV.SCR);
	return;	
}

#ifdef CONFIG_RTL8672_ATM_QoS
void	SetMBS(sar_private *cp, int8 i, uint16 MBS)
#else
void	SetMBS(sar_private *cp, int8 i, uint8 MBS)
#endif
{
	/* Set Max Burst Size */
	cp->vcc[i].QoS.MBS=MBS;
#ifndef CONFIG_RTL8672_ATM_QoS
	cp->vcc[i].TV.SCR &= 0xFFFF00FF;
	cp->vcc[i].TV.SCR |= ((uint32)MBS)<<8;
	//printk("cp->vcc[i].TV.SCR %x\n", cp->vcc[i].TV.SCR);
	WriteD_(cp->vcc[i].TV.SCR_Addr, cp->vcc[i].TV.SCR);
#else
	/* RTL8672 corner case: the function of ATM QoS will fail if MBS <=1 */
	if( MBS <= 1)
	    MBS = 2;
	WriteD_(cp->vcc[i].TV.QoS_SCR1_Addr, (uint32)MBS);	
#endif
	return;	
}

uint8 GetCRD(sar_private *cp, int8 i)
{
	uint8	CRD;
	/* Set Cell Credit */
	cp->vcc[i].TV.SCR=ReadD_(cp->vcc[i].TV.SCR_Addr);
	CRD = (uint8)(cp->vcc[i].TV.SCR&0x000000FF);
	cp->vcc[i].QoS.CRD=CRD;
	return CRD;	
}

uint32 ReadD_ (uint32 address){
	if(!address){
		return 0;
	}
	address = address & 0xFFFFFFFC;
	if ((address&0xF0000000)==0x80000000) address |= Uncache_Mask;
	return reg(address);
}

void WriteD_ (uint32 address,  uint32 data){
	if(address){
		address = address & 0xFFFFFFFC;	
		if ((address&0xF0000000)==0x80000000) address |= Uncache_Mask;
		reg(address)=data;
	}
	return;
	
}

void Set1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0xFFFFFFFC;
	reg(address) = (reg(address)|(((uint32)0x00000001) << index));
	return;
}

void Reset1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0x0FFFFFFC;
	reg(address) &= (0xFFFFFFFF^(((uint32)0x00000001) << index));
	return;
}

uint8 Test1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0xFFFFFFFC;
	if(reg(address)&(((uint32)0x00000001) << index)) 	return TRUE;
	else 									return FALSE;
}

int32 S2i(uint8 * str_P)
{
    uint32  val;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;

        }
    }
    
    return val;
}


/*--------------------------------------------
	Enable/Disable Control routines used by commands
---------------------------------------------*/


void Enable_IERBF(sar_private *cp){
	cp->CNFG_Reg |=IERBF;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_IERBF(sar_private *cp){
	cp->CNFG_Reg &=(~IERBF);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Set_RDA(sar_private *cp, int8 RdaThr, int8 RdaTimer){

	cp->CNFG_Reg &= 0xF000FFFF;
	cp->CNFG_Reg |= (((uint32)RdaThr)<<RDATHR_Offset)&0x07000000;
	cp->CNFG_Reg |= (((uint32)RdaTimer)<<RDATIMER_Offset)&0x003F0000;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}


void Enable_IS8671G_1(sar_private *cp) {
printk("Enable 8671G 1 function\n");
	cp->CNFG_Reg |= IS8671G_1;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_IS8671G_0(sar_private *cp) {
	printk("Enable 8671 0 function\n");
	cp->CNFG_Reg |= IS8671G_0;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_IS8672_10(sar_private *cp) {
	printk("Enable 8672 function \n");
//	cp->CNFG_Reg |= (0x11<<Is8672_10_Offset);
	cp->CNFG_Reg |= 0x30000000;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}


void Enable_IERDA(sar_private *cp){

	cp->CNFG_Reg |=IERDA;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_IERDA(sar_private *cp){
	cp->CNFG_Reg &=(~IERDA);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_IERTO(sar_private *cp){
	cp->CNFG_Reg |=IERTO;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_IERTO(sar_private *cp){
	cp->CNFG_Reg &=(~IERTO);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_IETBE(sar_private *cp){
	cp->CNFG_Reg |=IETBE;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_IETBE(sar_private *cp){
	cp->CNFG_Reg &=(~IETBE);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_IETDF(sar_private *cp){
	cp->CNFG_Reg |=IETDF;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_IETDF(sar_private *cp){
	cp->CNFG_Reg &=(~IETDF);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_LoopBack(sar_private *cp)
{
	cp->CNFG_Reg |=UFLB;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_LoopBack(sar_private *cp)
{
	cp->CNFG_Reg &=(~UFLB);
//for performance tune test	
	//cp->CNFG_Reg |=UFLB;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_SAR(sar_private *cp)
{
	cp->CNFG_Reg |=SAREN;
	// 120210 -- Transmit utopia clk to sachem when SAR Disabled
	cp->CNFG_Reg |=UTO_CLK_MASK;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_SAR(sar_private *cp)
{
	cp->CNFG_Reg &=(~SAREN);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_QoS_Starvation(sar_private *cp)
{
	cp->CNFG_Reg |=QoS_Starvation;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Disable_QoS_Starvation(sar_private *cp)
{
	cp->CNFG_Reg &=(~QoS_Starvation);
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void Enable_ATRLREN(sar_private *cp, int8 i){
	cp->vcc[i].RV.CtlSts |=ATRLREN;	
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

void Disable_ATRLREN(sar_private *cp, int8 i){
	cp->vcc[i].RV.CtlSts &= (~ATRLREN);
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

void Enable_AAL5(sar_private *cp, int8 i)
{
	cp->vcc[i].RV.CtlSts |=AAL5;
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}


void Enable_Raw(sar_private *cp, int8 i)
{
	cp->vcc[i].RV.CtlSts &=(~AAL5);
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

void Enable_tx_ch(sar_private *cp, int8 i)
{
	cp->vcc[i].TV.CtlSts |=CHEN;
	cp->tx_channel_on|=(1<<i);
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);

	return;
}

void Enable_rx_ch(sar_private *cp, int8 i)
{
	cp->vcc[i].RV.CtlSts |=CHEN;
	cp->rx_channel_on|=(1<<i);	
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

void Disable_tx_ch(sar_private *cp, int8 i)
{	
	cp->vcc[i].TV.CtlSts &=(~CHEN);
	cp->tx_channel_on&=(~(1<<i));	
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;
}

void Disable_rx_ch(sar_private *cp, int8 i)
{	
	cp->vcc[i].RV.CtlSts &=(~CHEN);
	cp->rx_channel_on&=(~(1<<i));	
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}
//0922 shlee for rtl8671 (G)
void Enable_QoS_Priority(sar_private *cp, int8 i)
{
	cp->vcc[i].TV.CtlSts |=QosPriority;
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;
}

void Disable_QoS_Priority(sar_private *cp, int8 i)
{
	cp->vcc[i].TV.CtlSts &= (~QosPriority);
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;
}

void Enable_HP_CHTx(sar_private *cp, int8 i)
{
	cp->vcc[i].TV.CtlSts |=HpCHTx;
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;
}

void Disable_HP_CHTx(sar_private *cp, int8 i)
{
	cp->vcc[i].TV.CtlSts &=(~HpCHTx);
	WriteD_(cp->vcc[i].TV.CtlSts_Addr, cp->vcc[i].TV.CtlSts);
	return;
}
//0922 rtl8671(G) 
void Enable_Word_Insert(sar_private *cp, int8 i)
{	
	cp->vcc[i].RV.CKS |=WIEN;
	WriteD_(cp->vcc[i].RV.CKS_Addr, cp->vcc[i].RV.CKS);
	return;
}

void Set_L2Encap(sar_private *cp, int8 i, int8 encap)
{
	cp->vcc[i].RV.CKS &= ~(1<<L2Encap_Offset);
	cp->vcc[i].RV.CKS |= encap<<L2Encap_Offset;
	WriteD_(cp->vcc[i].RV.CKS_Addr, cp->vcc[i].RV.CKS);
//	printk("%s: ch: %d encap %d write %x to reg %x\n", __func__,i, encap, cp->vcc[i].RV.CKS, cp->vcc[i].RV.CKS_Addr);	
}

void Set_SARhdr(sar_private *cp, int8 i, int8 mode)
{
	UINT32 val;
	val = cp->vcc[i].RV.CKS; 
	val &= 0xffffe3ff;
	val |= mode<<SARhdr_Offset;	
	WriteD_(cp->vcc[i].RV.CKS_Addr, val);
	cp->vcc[i].RV.CKS = val;
//	printk("%s: ch:%d mode %d write %x to reg %x\n", __func__, i,mode,val, cp->vcc[i].RV.CKS_Addr);
}

void Write_IP_Parser(sar_private *cp, int8 i, int8 en, int8 offset)
{
	UINT32 val;

	val = cp->vcc[i].RV.CKS;
	val &= ~(0x3F);

	val |= ((en << IEPEN_Offset) | (offset));

	WriteD_(cp->vcc[i].RV.CKS_Addr, val);
	cp->vcc[i].RV.CKS = val;	
//	printk("%s: write %x to reg %x\n", __func__, val, cp->vcc[i].RV.CKS_Addr);	
}

void Disable_Word_Insert(sar_private *cp, int8 i)
{	
	cp->vcc[i].RV.CKS &= (~WIEN);
	WriteD_(cp->vcc[i].RV.CKS_Addr, cp->vcc[i].RV.CKS);
	return;
}

void Enable_Drop_NonOAM(sar_private *cp, int8 i)
{	
	cp->vcc[i].RV.CtlSts |=DNOAMEN;
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

void Disable_Drop_NonOAM(sar_private *cp, int8 i)
{	
	cp->vcc[i].RV.CtlSts &= (DNOAMEN ^ 0xFFFFFFFF);
	WriteD_(cp->vcc[i].RV.CtlSts_Addr, cp->vcc[i].RV.CtlSts);
	return;
}

#if 0 //not use anymore
void	Set_QoS_Int(sar_private *cp){
	cp->CNFG_Reg &= (~QCLKSEL) ;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}

void	Set_QoS_Ext(sar_private *cp, int clk_offset){
	cp->CNFG_Reg |= QCLKSEL;
	cp->CNFG_Reg |= (clk_offset&0x000F)<<QCLKOFFSET_Offset;
	WriteD_(SAR_CNFG_Addr, cp->CNFG_Reg);
	return;
}
#endif

#if 0
BOOL ClearRxBuffer(sar_private *cp, int8 ch_no){

	int8				j, i;

	if(cp->vcc[ch_no].RV.desc_pr==cp->vcc[ch_no].RV.desc_pc){
		printk("channel-%d-Rx Buffer Bull with desc_pr == desc_pc\n",ch_no);
		return FALSE;
	}

	cp->vcc[ch_no].RV.desc_pc=cp->vcc[ch_no].RV.desc_pa;
	
	/*  Read Descriptor and Allocate the memory */
	j=cp->vcc[ch_no].RV.desc_pa;
	i=(cp->vcc[ch_no].RV.desc_pr+SAR_RX_DESC_NUM-1)%SAR_RX_DESC_NUM;
	
	while(j!=i){
	    if (ch_no==OAM_CH_NO) {
		if(!(cp->RODesc[j].STS&OWN_32)) {
			/* Restore LEN and CMD field */
			cp->RODesc[j].LEN = cp->vcc[ch_no].RV.buffer_size;
			cp->RODesc[j].STS|=0x8000;
			cp->RODesc[j].STS&=0xC000; /* EOR and OWN bit */
			j++;
			j%=SAR_RX_DESC_NUM;
		} else break;
	    } else {
		if(!(cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].STS&OWN_32)){

			/* Free Memory for this descriptor */
			/* free(tmpRV_DESC->START_ADDR); */
				
			/* Allocate new buffer for this descriptor, and modify the address to non-cacheable area */
			/* cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].START_ADDR=(uint32)(malloc(Rx_Side))|Uncache_Mask;*/
	
			/* Restore LEN and CMD field */
			cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].LEN = SAR_RX_Buffer_Size;
			cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].STS|=0x8000;
			cp->RVDesc[ch_no*SAR_RX_RING_SIZE+j].STS&=0xC000; /* EOR and OWN bit */
			j++;
			j%=SAR_RX_DESC_NUM;
		} else break;
	    };
	}
	cp->vcc[ch_no].RV.desc_pa=j;	

	return TRUE;

}
#endif
//extern int sar_loopback_cnt[8];
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
/*__IRAM */BOOL ClearTxBuffer(sar_private *cp, int8 ch_no){
	int 	 		j=0;
	TV_CMD_DESC	*TVDesc;
	struct sk_buff 	*skb;
	//UINT32			encap_mode;
	
	if( atomic_read(&lock_txbuff) )
		return 0;
	atomic_set(&lock_txbuff, 1);
	
#ifdef	CONFIG_FLOW_CTRL
	int pw;

	pw = sar_dev->vcc[ch_no].TV.desc_pw;
#endif//	CONFIG_FLOW_CTRL
	j=cp->vcc[ch_no].TV.desc_pf;	
	/*cp->vcc[ch_no].TV.desc_pc=(uint8)((REG32(cp->vcc[ch_no].TV.CtlSts_Addr)&CDOI_Mask)>>18);		*/
	
	/* Now For 1483 only */
	//encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	TVDesc=&cp->TVDesc[ch_no*SAR_TX_RING_SIZE+j];
	skb = (struct sk_buff *)cp->vcc[ch_no].tx_buf[j];

	//while((j!=cp->vcc[ch_no].TV.desc_pw)&&(!(TVDesc->CMD&OWN))){
	while((!(TVDesc->CMD&OWN))&&skb){
		
			cp->vcc[ch_no].stat.tx_desc_ok_cnt++;
			//sar_loopback_cnt[ch_no]++;
			cp->vcc[ch_no].stat.tx_byte_cnt += TVDesc->LEN;
			//cp->vcc[ch_no].stat.tx_cell_cnt += ((TVDesc->LEN%48==0) ? (TVDesc->LEN/48):(TVDesc->LEN/48+1));
			//cp->net_stats.tx_bytes += TVDesc->LEN;
			/* if skb exists and this descriptor is not a 1483 header, free this mblk */
		#ifdef INS_DESC_HEADER
			//if ((skb!=NULL)&&(TVDesc->START_ADDR!=(UINT32)(&FrameHeader_1483[encap_mode]))){
		#else
			//if (skb!=NULL){
		#endif
				//if (netif_msg_tx_done(cp))
				//	printk(KERN_DEBUG "%s: tx ch %d done, desc_pf=%d\n", cp->dev->type, ch_no, j);	
				
			//if (skb!=NULL){
				if (ch_no!=OAM_CH_NO) {
				    dev_kfree_skb_irq(skb);
				    //rtl8671_putPreAlloc(skb);
				    if (cp->QoS_Tx_Credit)
				        cp->vcc[ch_no].creditQoSTx++;		
				} else {
				    kfree(skb);
				};
				cp->vcc[ch_no].tx_buf[j]=(UINT32)NULL;
				cp->vcc[ch_no].stat.tx_buf_free++;
			//};
			if(TVDesc->CMD&LS){/* one packet has been successfully transmmited */
				cp->vcc[ch_no].stat.tx_pkt_ok_cnt++;
				//cp->net_stats.tx_packets++;
			}
			//TVDesc->RSVD1=0x00000000;
			//TVDesc->START_ADDR=0x00000000;
			//j++; j%=SAR_TX_DESC_NUM;
			j=(j+1)&(SAR_TX_DESC_NUM-1);

			//TVDesc=&cp->TVDesc[ch_no*SAR_TX_RING_SIZE+j];
			TVDesc= (j!=0)?(TVDesc+1):(&cp->TVDesc[ch_no*SAR_TX_RING_SIZE]);
			skb = (struct sk_buff *)cp->vcc[ch_no].tx_buf[j];
	}
	cp->vcc[ch_no].TV.desc_pf=j;

	//wake queue
	if(cp->vcc[ch_no].dev!=NULL){
		int avail_desc;
		
		avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
		if (netif_queue_stopped(cp->vcc[ch_no].dev)){
			if (avail_desc >= SAR_TX_THRESHOLD)
				netif_wake_queue(cp->vcc[ch_no].dev);
		}
	}
	#if 0
	if (netif_queue_stopped(cp->dev) && 
		(((cp->vcc[ch_no].TV.desc_pf-cp->vcc[ch_no].TV.desc_pw)%SAR_TX_DESC_NUM) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);
	#endif
#ifdef	CONFIG_FLOW_CTRL
	if(FCCtrl && FCEnable){
		if (1 == FCFlag)
		{
			if (((j -1 -pw + SAR_TX_DESC_NUM)&(SAR_TX_DESC_NUM-1)) >= FC_high)
			{//used descriptor num.
				FCFlag = 0;
				REG32(0xB9800044)=0x0000000f;
			}
		}
	}
#endif
	
	atomic_set(&lock_txbuff, 0);
	return TRUE;

}



#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void FreeVcBuff(sar_private *cp, int8 ch_no){

	int				i;
	struct sk_buff 	*skb;
	
	if (ch_no>(Enable_VC_CNT+1))
		return;

	/*  initialize TX Descriptors: clear CMD and set EOR*/	
	//for(i=0;i<SAR_TX_DESC_NUM;i++){
	//	if(i==(SAR_TX_DESC_NUM-1))		
	//		cp->TVDesc[ch_no*SAR_TX_RING_SIZE+i].CMD=0x4000;
	//	else	/* EOR */
	//		cp->TVDesc[ch_no*SAR_TX_RING_SIZE+i].CMD=0x0000;
	//			
	//}

	/* Free RX Descriptors */ 
	for(i=0;i<SAR_TX_DESC_NUM;i++){

		if(ch_no==OAM_CH_NO){

			if(cp->vcc[ch_no].tx_buf[i] !=(u32)NULL)
				kfree((void *)(cp->vcc[ch_no].tx_buf[i]));

			cp->vcc[ch_no].tx_buf[i]=(UINT32)NULL;
						
			/* Clear OWN and EOR bit */
			cp->TODesc[i].CMD=0x0000;
			cp->TODesc[i].START_ADDR = 0;

		}else{

			skb=(struct sk_buff *)cp->vcc[ch_no].tx_buf[i];
			if(skb!=(struct sk_buff *)NULL)
				dev_kfree_skb(skb);


			/* Clear this skb */
			cp->vcc[ch_no].tx_buf[i]=(UINT32)NULL;

			cp->TVDesc[ch_no*SAR_TX_RING_SIZE+i].CMD=0x0000;



		}
	}
	
	for(i=0;i<SAR_RX_DESC_NUM;i++){

		if(ch_no==OAM_CH_NO){

			if(cp->vcc[ch_no].rx_buf[i] !=(u32)NULL)
				kfree((void *)(cp->vcc[ch_no].rx_buf[i]));  //5/22/04' hrchen, bug TVDesc is replaced by RVDesc
			
			cp->vcc[ch_no].rx_buf[i]=(UINT32)NULL;

			#ifdef CONFIG_RTL8672  // andrew, free memory on OAM ch1 as well
			if(cp->vcc[ch_no+1].rx_buf[i] !=(u32)NULL)
				kfree((void *)(cp->vcc[ch_no+1].rx_buf[i]));  
			
			cp->vcc[ch_no+1].rx_buf[i]=(UINT32)NULL;
			#endif
						
			/* Clear OWN and EOR bit */
			cp->RODesc[i].STS=0x0000;
			cp->RODesc[i].START_ADDR = 0;

		}else{

			skb=(struct sk_buff *)cp->vcc[ch_no].rx_buf[i];
			if(skb!=(struct sk_buff *)NULL)
				dev_kfree_skb(skb);

			/* Clear this skb */
			cp->vcc[ch_no].rx_buf[i]=(UINT32)NULL;

			cp->RVDesc[ch_no*SAR_RX_RING_SIZE+i].STS=0x0000;



		}
	}

	cp->vcc[ch_no].RV.desc_pr =0;
	cp->vcc[ch_no].RV.desc_pc =0;
	cp->vcc[ch_no].RV.desc_pa =0;
	cp->vcc[ch_no].TV.desc_pf =0;
	cp->vcc[ch_no].TV.desc_pc =0;
	cp->vcc[ch_no].TV.desc_pw =0;
	#ifdef CONFIG_RTL8672
	if(ch_no==OAM_CH_NO){
		// init outband ch1
		cp->vcc[ch_no+1].RV.desc_pr =0;
		cp->vcc[ch_no+1].RV.desc_pc =0;
		cp->vcc[ch_no+1].RV.desc_pa =0;
		cp->vcc[ch_no+1].TV.desc_pf =0;
		cp->vcc[ch_no+1].TV.desc_pc =0;
		cp->vcc[ch_no+1].TV.desc_pw =0;
	}
	#endif

	return;
}


void AllocVcBuff(sar_private *cp, int8 ch_no){

	int				i;
	struct sk_buff 	*skb=NULL;
	UINT32 *ptr;
	

	/*  save buffer size to this channel (using same size) */
	if(ch_no==OAM_CH_NO)	cp->vcc[ch_no].TV.buffer_size=SAR_OAM_Buffer_Size;
	else						cp->vcc[ch_no].TV.buffer_size=SAR_TX_Buffer_Size;
	
	if(ch_no==OAM_CH_NO)	cp->vcc[ch_no].RV.buffer_size=SAR_OAM_Buffer_Size;
	else						cp->vcc[ch_no].RV.buffer_size=SAR_RX_Buffer_Size;
	
	if (ch_no>(Enable_VC_CNT+1))
		return;
	
	if ((ch_no!=OAM_CH_NO)&&((per_vc_desc_number<1)||(per_vc_desc_number>SAR_RX_DESC_HI_LIMIT))) {
		printk(KERN_ERR ":Execute /bin/sarctl pvcnumber <num> before create PVC\n");
	    return;
	};

	/*  initialize TX Descriptors: clear CMD and set EOR*/	
	for(i=0;i<SAR_TX_DESC_NUM;i++){
		if(ch_no==OAM_CH_NO){
		    if(i==(SAR_TX_DESC_NUM-1))		
			cp->TODesc[i].CMD=0x4000;
		    else	/* EOR */
			cp->TODesc[i].CMD=0x0000;
		    cp->TODesc[i].START_ADDR = 0;
		} else {
			if(i==(SAR_TX_DESC_NUM-1))		
				cp->TVDesc[ch_no*SAR_TX_RING_SIZE+i].CMD=0x4000;
			else	/* EOR */
				cp->TVDesc[ch_no*SAR_TX_RING_SIZE+i].CMD=0x0000;
		}
		/* clear skb */
		cp->vcc[ch_no].tx_buf[i]=(UINT32)NULL;
	}

	/* initialize RX Descriptors */ 
	for(i=0;i<SAR_RX_DESC_NUM;i++){

		if(ch_no==OAM_CH_NO){
			ptr = (UINT32*)kmalloc(SAR_OAM_Buffer_Size, GFP_KERNEL);
			if(ptr==(u32)NULL)		goto free_desc;
			cp->RODesc[i].LEN = SAR_OAM_Buffer_Size;
			cp->RODesc[i].START_ADDR = (u32)ptr|Uncache_Mask;
			cp->vcc[ch_no].rx_buf[i] = (UINT32)ptr;

			/* set OWN and EOR bit */
			if(i==(SAR_RX_DESC_NUM-1))
				cp->RODesc[i].STS=0xC000;
			else
				cp->RODesc[i].STS=0x8000;		
			
			cp->vcc[ch_no].RV.desc_pa ++;
			cp->vcc[ch_no].RV.desc_pa %= SAR_RX_DESC_NUM;				

// Mason Yu. Recieve packet from OAM ch1(not cho) for AutoHunt
#ifdef CONFIG_RTL8672   //8672  for ro1 ch, allocate SAR_RX_Buffer_Size length buffer for normal packet			
			ptr = (UINT32*)kmalloc(SAR_OAM_Buffer_Size, GFP_KERNEL);
			if(ptr==(u32)NULL)		goto free_desc;			
			cp->RODesc[i+SAR_RX_RING_SIZE].LEN = SAR_OAM_Buffer_Size;
			cp->RODesc[i+SAR_RX_RING_SIZE].START_ADDR = (u32)ptr|Uncache_Mask;
			cp->vcc[ch_no+1].rx_buf[i] = (UINT32)ptr;

			/* set OWN and EOR bit */
			if(i==(SAR_RX_DESC_NUM-1))
				cp->RODesc[i+SAR_RX_RING_SIZE].STS=0xC000;
			else
				cp->RODesc[i+SAR_RX_RING_SIZE].STS=0x8000;		
			
			cp->vcc[ch_no+1].RV.desc_pa ++;
			cp->vcc[ch_no+1].RV.desc_pa %= SAR_RX_DESC_NUM;				
#endif
			
		}else{
		
			/* Get a Buffer */
			if(i < per_vc_desc_number){
#ifdef CONFIG_SAR_SHARE_PRIV_SKB_WITH_ETH
				skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else
				skb = dev_alloc_skb(SAR_RX_Buffer_Size);
#endif //CONFIG_SAR_SHARE_PRIV_SKB_WITH_ETH
				if (skb==NULL)
				{
					printk(KERN_ERR ":Error Could Not Get A Buffer..\n");
					goto free_desc;
				} else
					cp->vcc[ch_no].stat.rx_buf_alloc++;			
			}
			
			/*  allocate memory for RX descriptors */
			cp->RVDesc[ch_no*SAR_RX_RING_SIZE+i].LEN=SAR_RX_Buffer_Size;
			if(i < per_vc_desc_number)
				cp->RVDesc[ch_no*SAR_RX_RING_SIZE+i].START_ADDR=(u32)skb->data|Uncache_Mask;

			/* record this skb */
			if(i < per_vc_desc_number)
				cp->vcc[ch_no].rx_buf[i]=(UINT32)skb;
			else
				cp->vcc[ch_no].rx_buf[i]=(UINT32)NULL;

			/* set OWN and EOR bit */
			if(i==(SAR_RX_DESC_NUM-1))
				cp->RVDesc[ch_no*SAR_RX_RING_SIZE+i].STS=0xC000;
			else{
				if(i < per_vc_desc_number)
					cp->RVDesc[ch_no*SAR_RX_RING_SIZE+i].STS=0x8000;		
			}
			
			cp->vcc[ch_no].RV.desc_pa ++;
			cp->vcc[ch_no].RV.desc_pa %= SAR_RX_DESC_NUM;			
		}
	}

	if(cp->CNFG_Reg&IERBF)
		cp->vcc[ch_no].RV.desc_pa =(cp->vcc[ch_no].RV.desc_pa + SAR_RX_DESC_NUM -1) % SAR_RX_DESC_NUM;

	return;

free_desc:
	/* !!! free ring */
	FreeVcBuff(cp, ch_no);
	return;
	
}


int cr2reg(int pcr)
{
	int k, e, m, pow2, reg;
	
	k = pcr;
	e=0;

	while (k>1) {
		k = k/2;
		e++;
	}

	//printf("pcr=%d, e=%d\n", pcr,e);
	pow2 = 1;
	for (k = 1; k <= e; k++)
		pow2*=2;

	//printf("pow2=%d\n", pow2);
	//m = ((pcr/pow2)-1)*512;
	k = 0;
	while (pcr >= pow2) {
		pcr -= pow2;
		k++;
	}
	m = (k-1)*512 + pcr*512/pow2;
	//printf("m=%d\n", m);
	reg = (e<<9 | m );
	//printf("reg=%d\n", reg);
	return reg;
}

#if 0
void adjust_ATM_QoS(sar_atm_vcc *vcc_dev)
{
	if (vcc_dev->QoS.Type&0x02) {  //VBR type
		int credit_num;
	    credit_num=ReadD_(vcc_dev->TV.SCR_Addr)&0x00FF;
	    if (credit_num >= vcc_dev->SCR_Credit_hi) {  //normal credit number
	    	if (vcc_dev->write_flag) {
	    	    WriteD_(vcc_dev->TV.SCR_Addr, vcc_dev->TV.SCR);
	    	    vcc_dev->write_flag = 0;
	    	    printk("NormSCR %08x\n", vcc_dev->TV.SCR);
	    	};
	    } else if (credit_num <= vcc_dev->SCR_Credit_low) {  //credit is not enough, use Safe SCR to increase credit
	    	if (!vcc_dev->write_flag) {
	    	    WriteD_(vcc_dev->TV.SCR_Addr, vcc_dev->Safe_SCR);
	    	    vcc_dev->write_flag = 1;
	    	    printk("SafeSCR %08x\n", vcc_dev->Safe_SCR);
	    	};
	    }
	};
}
#endif

void CreateVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg, int reset_stat_info){
	
	int8	ch_no, encap_mode;
	
	ch_no = cfg->ch_no;

	cp->vcc[ch_no].ch_no		= cfg->ch_no;
	cp->vcc[ch_no].vpi		= cfg->vpi;
	cp->vcc[ch_no].vci		= cfg->vci;
	cp->vcc[ch_no].rfc		= cfg->rfc;
	cp->vcc[ch_no].framing	= cfg->framing;
	//cp->vcc[ch_no].loopback	= cfg->loopback;
	memcpy(&cp->vcc[ch_no].QoS, &cfg->QoS, sizeof(Traffic_Manage));	

	if(cp->vcc[ch_no].created == VC_NOT_CREATED){
		AllocVcBuff(cp, ch_no);
		if (reset_stat_info)
		    memset(&cp->vcc[ch_no].stat, 0, sizeof(ch_stat));
	}

	SetVpiVci(cp, cfg->vpi, cfg->vci, ch_no);	/* Set Vpi, Vci to Rx */
	// 8672  set each CH Rx/Tx FDP
	if(ch_no!=0&&ch_no!=OAM_CH_NO) {
		reg(cp->vcc[ch_no].TV.FDP_Addr)	= (unsigned int)&cp->TVDesc[ch_no*SAR_TX_RING_SIZE];
		reg(cp->vcc[ch_no].RV.FDP_Addr)	= (unsigned int)&cp->RVDesc[ch_no*SAR_RX_RING_SIZE];
	}
	Enable_AAL5(cp, ch_no);
	Enable_ATRLREN(cp, ch_no);
#if 1//SAR_INTERPOLATE
	SetQoSInterpolate(cp, ch_no, cfg);			/* Set cell rate interpolation */
	//printk("ch_no%d, (%x/%x) (%x/%x)\n", ch_no, cp->vcc[ch_no].hwCRtickData[LOWER_SCR], cp->vcc[ch_no].hwCRtickData[UPPER_SCR],
	//	cp->vcc[ch_no].hwCRtickData[LOWER_PCR], cp->vcc[ch_no].hwCRtickData[UPPER_PCR]);
#endif
	printk("create: ch%d (%d/%d) %d,%d\n", ch_no, cp->vcc[ch_no].vpi, cp->vcc[ch_no].vci,
	cfg->QoS.PCR, cfg->QoS.SCR);
    //for ATM Tx Credit
    if (total_pvc_number==1) {  //only 1 PVC
        cp->QoS_Tx_Credit = 0;
    } else {
		switch (cp->vcc[ch_no].QoS.Type) {
	    	case QoS_CBR:
	        	cp->vcc[ch_no].creditQoSTx = QoS_CREDIT_HI;
		        cp->QoS_Tx_Credit = 1;
		        break;
		    case QoS_nrtVBR:
	    	case QoS_rtVBR:
	        	cp->vcc[ch_no].creditQoSTx = QoS_CREDIT_MID;
		        cp->QoS_Tx_Credit = 1;
		        break;
		    case QoS_UBR:
	    	default:
	        	cp->vcc[ch_no].creditQoSTx = QoS_CREDIT_LOW;
			    break;
	    };
	};

	/* Now For 1483 only */
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	/* if 1483 Bridge Mode, header(10)+Eth(14) = 24 makes the TCP/IP just on the 4-byte boundary,
		Word Insertion is not needed */
	if(cp->vcc[ch_no].rfc == RFC1483_BRIDGED)
	{
	
		Write_IP_Parser(cp, ch_no, 1, 0);  // enable eth parse from 0 offset
		Disable_Word_Insert(cp, ch_no);
	}
#ifdef CONFIG_ATM_CLIP
	else if(cp->vcc[ch_no].rfc == RFC1483_ROUTED || cp->vcc[ch_no].rfc == RFC1577)	// Jenny, IPoA
#else
	else if(cp->vcc[ch_no].rfc == RFC1483_ROUTED)
#endif
	{
		Write_IP_Parser(cp, ch_no, 1, 0);  // enable eth parse from 0 offset
		Disable_Word_Insert(cp, ch_no);
	}
	else if (cp->vcc[ch_no].rfc == RFC2364) {
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
		Write_IP_Parser(cp, ch_no, 1, 0);
#else
		Enable_Word_Insert(cp, ch_no);
		Write_IP_Parser(cp, ch_no, 1, 2);  // enable eth parse, 2 offset
#endif
	}
	else if (cp->vcc[ch_no].rfc == RFC2516) {
		Write_IP_Parser(cp, ch_no, 1, 0);  // enable eth parse from 0 offset
		Disable_Word_Insert(cp, ch_no);
	}	
#ifndef SAR_CRC_GEN
		cp->vcc[ch_no].TV.Ether_Offset_Value=No_Offset;
#else
		cp->vcc[ch_no].TV.Ether_Offset_Value=FrameHeaderSize_1483[encap_mode];
#endif
    cp->vcc[ch_no].TV.Ether_Offset_Value&=ETHNT_OFFSET_MSK;

	Enable_rx_ch(cp, ch_no);
	Enable_tx_ch(cp, ch_no);

	cp->vcc[ch_no].created = VC_CREATED;
	cp->vcc[ch_no].dev=NULL;
	return;
}



void DeleteVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg){
	
	int8	ch_no;
	
	ch_no = cfg->ch_no;
	if (ch_no == OAM_CH_NO) {
		//ql -- set push to null
		cp->vcc[OAM_CH_NO].dev_data = NULL;
		
		return;
	}


	cp->vcc[ch_no].ch_no		= cfg->ch_no;
	//cp->vcc[ch_no].vpi		= cfg->vpi;
	//cp->vcc[ch_no].vci		= cfg->vci;
	//cp->vcc[ch_no].rfc		= cfg->rfc;
	//cp->vcc[ch_no].framing	= cfg->framing;
	//memcpy(&cp->vcc[ch_no].QoS, &cfg->QoS, sizeof(Traffic_Manage));	
	//move here to tell sar_rx abadoning rx action.... jim
	cp->vcc[ch_no].created = VC_NOT_CREATED;
	FreeVcBuff(cp, ch_no);

	SetVpiVci(cp, 0, 0, ch_no);	/* Set Vpi, Vci to Rx */

	
	//Enable_AAL5(ch_no);
	//Disable_ATRLREN(ch_no);

	//SetQoS(ch_no, 0);			/* Set Scheduler Option */
	//SetPCR(ch_no, 0);			/*  Set Peak Cell Rate */


	/* Now For 1483 only */
	//encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	/* if 1483 Bridge Mode, header(10)+Eth(14) = 24 makes the TCP/IP just on the 4-byte boundary,
		Word Insertion is not needed */
	//if(cp->vcc[ch_no].rfc == RFC1483_BRIDGED)
	//	Disable_Word_Insert(ch_no);
	//else if(cp->vcc[ch_no].rfc == RFC1483_ROUTED)
	//	Enable_Word_Insert(ch_no);

	
//	#ifndef SAR_CRC_GEN
//		cp->vcc[ch_no].TV.Ether_Offset_Value=No_Offset;
//#else
//		cp->vcc[ch_no].TV.Ether_Offset_Value=FrameHeaderSize_1483[encap_mode];
//#endif
//    cp->vcc[ch_no].TV.Ether_Offset_Value&=ETHNT_OFFSET_MSK;


	Disable_rx_ch(cp, ch_no);
	Disable_tx_ch(cp, ch_no);

	//cp->vcc[ch_no].created = VC_NOT_CREATED;
	cp->vcc[ch_no].dev=NULL;

	return;
}

static const unsigned char ppp_term[] = {
//	0xfe,
//	0xfe,
//	0x03,
//	0xcf,
	0xc0,
	0x21,
	0x05,	/* PPP TERM-REQ */
	0x00,	/* Identifier */
	0x00,
	0x04
};

static const unsigned char pppllc[] = { 0xFE, 0xFE, 0x03, 0xCF, 0xC0, 0x21 };
#define LLC_LEN		(4)
#define TERM_LEN	(6)
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
void sendTERM(void) {
	sar_private *cp = sar_dev;
	struct atm_vcc *vcc;
	int i;

	for (i=0; i<Enable_VC_CNT; i++) {
		if (cp->vcc[i].created==VC_CREATED) { // fake PPPoA TERM-REQ
			struct sk_buff *skb;
			if ((skb = dev_alloc_skb(sizeof (struct sk_buff)))==NULL)
				return;
		    	vcc = cp->vcc[i].dev_data;
			memcpy(skb->data, ppp_term, TERM_LEN);
			memcpy(skb_push(skb, LLC_LEN), pppllc, LLC_LEN);
			ATM_SKB(skb)->vcc = vcc;
			atomic_set(&skb->users, 0);
//			printk("sendTERM: %d/%d send skb->data=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", cp->vcc[i].vpi, cp->vcc[i].vci
//				, skb->data[0], skb->data[1], skb->data[2], skb->data[3], skb->data[4], skb->data[5], skb->data[6], skb->data[7], skb->data[8], skb->data[9]);
			//atomic_add(skb->truesize, &vcc->tx_inuse);
			//ATM_SKB(skb)->iovcnt = 0;
			ATM_SKB(skb)->atm_options = vcc->atm_options;
//			printk("atm_skb(%p)->vcc(%p)->dev(%p)\n", skb, vcc, vcc->dev);
			skb->len = TERM_LEN + LLC_LEN;
			sar_send(vcc, skb);
			dev_kfree_skb_any(skb);
		}
	}
}

//tylo, for DSP link-up/down inform
unsigned char adslup=0;
unsigned char getADSLLinkStatus(void){
	return adslup;
}
void AdslLinkUp(void){
	int i;
	sar_private *cp = sar_dev;
	
	adslup=1;
#ifndef CONFIG_RTL8672_ATM_QoS
	GetLinkSpeed((char *)&LINE_rate);
#endif
#ifdef CONFIG_FLOW_CTRL
	//enable flow control
	FCEnable=1;
	REG32(0xB9800044)=0x0f;
#endif
	// Kaohj -- update link state
	//printk("ADSL Link up\n");
	for (i=0; i<Enable_VC_CNT; i++) {
		if (cp->vcc[i].created == VC_CREATED && cp->vcc[i].dev)
			netif_carrier_on(cp->vcc[i].dev);
	}
}

void AdslLinkDown(void){
	int i;
	sar_private *cp = sar_dev;
	
	adslup=0;
#ifdef CONFIG_FLOW_CTRL
	//disable flow control
	FCEnable=0;
	REG32(0xB9800044)=0x0f;
#endif
	// Kaohj -- update link state
	//printk("ADSL Link down\n");
	for (i=0; i<Enable_VC_CNT; i++) {
		if (cp->vcc[i].created == VC_CREATED && cp->vcc[i].dev)
			netif_carrier_off(cp->vcc[i].dev);
	}
}

#if 0
void	flush_tx_desc(sar_private *cp){

	int	i,j;
	TV_CMD_DESC	*TVDesc;
	struct sk_buff 	*skb;
	
	for(i=0;i<Enable_VC_CNT;i++){
		if(cp->vcc[i].created==VC_CREATED){
			for(j=0;j<SAR_TX_DESC_NUM;j++){
				TVDesc=&cp->TVDesc[i*SAR_TX_RING_SIZE+j];
				skb = (struct sk_buff	*)cp->vcc[i].tx_buf[j];
				if(skb!=(struct sk_buff	*)NULL)
					dev_kfree_skb_irq(skb);
				cp->vcc[i].tx_buf[j]=(UINT32)NULL;
				TVDesc->RSVD1=0x00000000;
				TVDesc->START_ADDR=0x00000000;
				TVDesc->CMD=0x00000000;
			}
			cp->vcc[i].TV.desc_pf =0;
			cp->vcc[i].TV.desc_pc =0;
			cp->vcc[i].TV.desc_pw =0;
	        	Disable_tx_ch(i);
        		Enable_tx_ch(i);
		}
	}
	
	//for OB channel
	if(cp->vcc[OAM_CH_NO].created==VC_CREATED){
		for(j=0;j<SAR_TX_DESC_NUM;j++){
			TVDesc=&cp->TVDesc[OAM_CH_NO*SAR_TX_RING_SIZE+j];
			skb = (struct sk_buff	*)cp->vcc[OAM_CH_NO].tx_buf[j];
			if(skb!=(struct sk_buff	*)NULL)
				dev_kfree_skb_irq(skb);
			cp->vcc[OAM_CH_NO].tx_buf[j]=(UINT32)NULL;
			TVDesc->RSVD1=0x00000000;
			TVDesc->START_ADDR=0x00000000;
			TVDesc->CMD=0x00000000;
		}
		cp->vcc[OAM_CH_NO].TV.desc_pf =0;
		cp->vcc[OAM_CH_NO].TV.desc_pc =0;
		cp->vcc[OAM_CH_NO].TV.desc_pw =0;
	        Disable_tx_ch(OAM_CH_NO);
        	Enable_tx_ch(OAM_CH_NO);
	}
}
#endif

#ifdef LoopBack_Test

uint16 ipcsum(uint16 *ptr, uint32 len, uint16 resid) {
	uint32 csum = resid;
	uint32 odd = 0;

	if(len & 1) odd = 1;
	len = len >> 1;
	
	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);

	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;

	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	if(csum == 0xffff)	csum = 0;

	return((uint16)csum);
}


void exchange_mac(uint8 *buf){

	char 	tmpMac[6];

	memcpy(tmpMac, buf, 6);
	memcpy(buf, buf+6, 6);
	memcpy(buf+6, tmpMac, 6);	
	return;

}

INT16 Lying_Engine(uint8 *buf, INT16 len){

	char 	broadcast[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	char		remoteMac[6]={0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
	char 	tmpMac[6], tempIPMac[10], tempIP[4];
	UINT16	*Type, *Opcode, *csum, *Frag, fragment;
	UINT8	*Protocol, *ICMP_Type;


		Type =(UINT16  *)(buf + 12);
		Opcode =(UINT16 *)(buf + 20);
		Protocol = (UINT8 *)(buf + 23);
		ICMP_Type = (UINT8 *)(buf + 34);		
		Frag= (UINT16 *)(buf+ 20);
		fragment = (UINT16)*Frag & 0x1FFF;
		
	if(!memcmp(buf, broadcast, 6)){	/* Broadcast Packet */
		
		if(!memcmp(buf+28, buf+38, 4))	/* Initial ARP, don't lie */
			return len;
		 
		if((*Type==0x0806)&&(*Opcode==0x0001)){ /* this is a ARP Request */
			/* handle Ehternet Header - Mac Address */	
			memcpy(tmpMac, buf+6, 6);
			memcpy(buf+6, remoteMac, 6);
			memcpy(buf, tmpMac, 6);

			/* change IP Opcode to reply */
			*Opcode = 0x0002;
			
			/* ARP IP and Mac Address */
			memcpy(tempIPMac, buf+22, 10);
			memcpy(tempIP, buf+38, 4);	
			memcpy(buf+32, tempIPMac, 10);
			memcpy(buf+22, remoteMac, 6);
			memcpy(buf+28, tempIP, 4);
			/* memset(buf+42, 0x20, 18); */
			return (len+18);
		}
		
	}else {

		if((*Type==0x0800)&&(*Protocol==0x01)&&(*ICMP_Type==0x08)){ /* this is a ICMP Ping Request */

			/* handle Ehternet Header - exchange Mac Address */	
			memcpy(tmpMac, buf, 6);
			memcpy(buf, buf+6, 6);
			memcpy(buf+6, tmpMac, 6);

			/* ip check sum */
			csum = (UINT16 *)(buf + 24);
			*csum = 0;
			*csum = (~ipcsum((UINT16 *)(buf+14), 20, 0));

			/* exchange IP Address */
			memcpy(tempIP, buf+26, 4);
			memcpy(buf+26, buf+30, 4);
			memcpy(buf+30, tempIP, 4);

			/* ICMP header */
			*ICMP_Type = 0x00;

			/* ICMP checksum */
			csum = (UINT16 *)(buf + 36);
			/* *csum = 0;*/
			*csum += 0x0800;/*(~ipcsum((UINT16 *)(buf+34), len-34, 0));*/
			
		}else if ((*Type==0x0800)&&(*Protocol==0x01)&&(fragment!=0)){ /* this is a ICMP Ping Request, fragment packet */

			/* handle Ehternet Header - exchange Mac Address */	
			memcpy(tmpMac, buf, 6);
			memcpy(buf, buf+6, 6);
			memcpy(buf+6, tmpMac, 6);

			/* ip check sum */
			csum = (UINT16 *)(buf + 24);
			*csum = 0;
			*csum = (~ipcsum((UINT16 *)(buf+14), 20, 0));

			/* exchange IP Address */
			memcpy(tempIP, buf+26, 4);
			memcpy(buf+26, buf+30, 4);
			memcpy(buf+30, tempIP, 4);

		}

		if(len<64)	len=64;
		return len;
	}
	/* exchange_mac(buf); */
	return len;
}

#endif

/////////////////////////////////////////////for OAM
#ifdef ATM_OAM

extern int OAMFSendACell(unsigned char *pCell, int flagOAM);	/* OAM to SAR driver interface */

#define EMPTY	0
#define OCCUPY	1

#ifndef NULL
#define NULL		0
#endif

#define ATM_OAM_TRACE	printk
#define ATM_OAM_DEBUG	printk

OAMF5 OAMF5_info;

static const unsigned char f5_llid_all_0[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char f5_llid_all_1[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

/* OAM F5 */

unsigned long OAMF5GetTimeStamp(void)
{
//#if RTOS == VXWORKS
	return (unsigned long)(jiffies*(1000/HZ));
//#endif
}

__SWAP
int OAMF5StartTimer(OAMF5Timer* timer, VOIDFUNCPTR pFunc)
{
	if(timer->occupied == OCCUPY)
	{
		ATM_OAM_TRACE("OAMF5StartTimer (timer->occupied == OCCUPY)\n");
		return 0;
	}

	init_timer(&timer->timer);
        timer->timer.expires = jiffies + (timer->ulTimeout * HZ)/1000;
        timer->timer.function = pFunc;
        timer->timer.data = (unsigned long)(timer);
	add_timer(&timer->timer);

	timer->occupied = OCCUPY;	
	ATM_OAM_DEBUG("OAMF5StartTimer ok\n");

	return 1;
}

__SWAP
int OAMF5StopTimer(OAMF5Timer* timer)
{
	if(timer->occupied == EMPTY)
	{
		ATM_OAM_TRACE("OAMF5StopTimer (timer->occupied == EMPTY)\n");
		return 0;
	}

	del_timer(&timer->timer);

	timer->occupied = EMPTY;	
	ATM_OAM_DEBUG("OAMF5StopTimer ok\n");

	return 1;
}

void OAMF5Init(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5)
{
	int i, j;
	//unsigned char gfc = 0, clp = 0, ucPTI;

	pOAMF5->VPI = vpi;
	pOAMF5->VCI = vci;
	
	/* OAM FM --LB */
	pOAMF5->OAMF5lFaultLBState = FAULT_LB_IDLE;			// loopback state: idle
	pOAMF5->OAMF5ulFaultLBTag = 0;						// loopback correlation tag
	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pOAMF5->OAMF5ucCPID[i] = 0xFF;					// connection point ID
	}

	// initialize LB table for TMN transmission only
	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		pOAMF5->OAMF5LBList[i].tag = 0;
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF5->OAMF5LBList[i].locationID[j] = 0;
		}
		pOAMF5->OAMF5LBList[i].scope = 0;
		pOAMF5->OAMF5LBList[i].count = 0;
		pOAMF5->OAMF5LBList[i].status = FAULT_LB_IDLE;

		// initialize LB timer
		pOAMF5->OAMF5LBList[i].timer.pOAM = (void *)pOAMF5;
		pOAMF5->OAMF5LBList[i].timer.OAMFunctionType = FAULT_LB;
		pOAMF5->OAMF5LBList[i].timer.OAMUserData = i;
		pOAMF5->OAMF5LBList[i].timer.oneShot = 1;
		pOAMF5->OAMF5LBList[i].timer.occupied = EMPTY;
		pOAMF5->OAMF5LBList[i].timer.ulTimeout = OAM_LB_WAIT_TIME;		
	}

	for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF5->LBRXList[i].locationID[j] = 0;
		}
		pOAMF5->LBRXList[i].status = EMPTY;
		pOAMF5->LBRXList[i].sur_des_count = 0;
	}
	
	pOAMF5->OAMF5ulFaultLBTxGoodCell = 0;
	pOAMF5->OAMF5ulFaultLBRxGoodCell = 0;

	ATM_OAM_DEBUG("ATM OAM F5 initialized.\n");
}

void OAMF5ChangeState(OAMF5 *pOAMF5)
{
	int i;
	OAMF5_LB_INFO *pLBInfo = NULL;

	// FM LB state
	for (i = 0, pLBInfo = &pOAMF5->OAMF5LBList[0]; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if(pLBInfo[i].status == FAULT_LB_WAITING)
			return;
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	pOAMF5->OAMF5lFaultLBState = FAULT_LB_IDLE;
	
	ATM_OAM_DEBUG("OAMF5ChangeState >> IDLE\n");	
}

__SWAP
int OAMF5SetVpiVci(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5)
{
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_IDLE) return 0;
	pOAMF5->VPI = vpi;
	pOAMF5->VCI = vci;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::SetLBID()
//
//		Input		: ucIDType -- 0: all connection point with location ID enable
//								  1: specified by TNM (telecommunication network management)
//										country code  & network ID (4 bytes) + operator specific information (11 bytes)
//								  2: specified by TNM (telecommunication network management)
//										country code + network ID (4 bytes)
//								  3: specified by TNM (telecommunication network management)
//										partial NSAP based coding structure
//								  0x6A: no loopback
//								  0xFF: endpoint
//								  others: reserved for future use
//					  pDesID: pointer of the destination ID array
//					  pID: pointer of the specified location ID array
//
//		Output		: None
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Prepare loopback ID information for transmission path
//		I.610, Table 5, 10.2.3
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
void OAMF5SetLBID(unsigned char ucIDType, unsigned char *pDesID, unsigned char *pID)
{
	int i;

	switch (ucIDType) {
	case 0:
		// all 0's: all CP with location ID option enable
	case 0x6A:
		// all 0x6A: not specify CP, no loopback is perform
	case 0xFF:
		// all 1's: enpoint
		for (i = 0; i < OAM_LB_LLID_SIZE; i++)
			pDesID[i] = ucIDType;
		break;
	case 2:
		pDesID[0] = ucIDType;
		// specified by telecommunication network management (TNM)
		for (i = 0; i < 4; i++)
			pDesID[i+1] = pID[i];
		for (; i < 15; i++)
			pDesID[i+1] = 0x6A;
		break;
	case 1:
	case 3:
		pDesID[0] = ucIDType;
		// specified by telecommunication network management (TNM)
		for (i = 0; i < OAM_LB_LLID_SIZE-1; i++)
			pDesID[i+1] = pID[i];
		break;
	}
}

__SWAP
int OAMF5TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF5 *pOAMF5)
{
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_IDLE) return 0;
	OAMF5SetLBID(ucLocationType, pOAMF5->OAMF5ucCPID, pCPID);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::SendACell()
//
//		Input		: pointer of a cell, channel (ATM0/ATM1)
//
//		Output		: 0/1
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Send a OAM cell to queue
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int  OAMF5SendACell(unsigned char *pOamCell)
{
	unsigned char *dupCell;
	ATM_OAM_DEBUG("Into the OAMF5SendACell\n");

	dupCell = (unsigned char *) kmalloc(OAM_CELL_SIZE, GFP_KERNEL);

	if(dupCell == NULL)
	{
		ATM_OAM_TRACE("OAM malloc fail!\n");
		return 0;
	}
	
	memcpy(dupCell, pOamCell, OAM_CELL_SIZE);

#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x %.02x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

	return OAMFSendACell(dupCell, 1);
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::GenLBCell()
//
//		Input		: pointer of OAMF5_LB_INFO
//
//		Output		: NONE
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Generate OAM fault management loopback cell for transmission
//						(excluding HEC and CRC10)
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
void OAMF5GenLBCell(OAMF5_LB_INFO *pLBInfo, OAMF5 *pOAMF5)
{
	unsigned char ucPTI = (pLBInfo->scope)? 5 : 4;
	unsigned char gfc = 0, clp = 0;
	int i;

	ATM_OAM_DEBUG("Into the OAMF5GenLBCell\n");

	// generate cell header & payload
	pLBInfo->cell[OAM_FORMAT_H1] = (unsigned char) ((pOAMF5->VPI >> 4) & 0x0F) | ((gfc << 4) & 0xF0);
	pLBInfo->cell[OAM_FORMAT_H2] = (unsigned char) (((pOAMF5->VPI << 4) & 0x00F0) | (pOAMF5->VCI >> 12));
	pLBInfo->cell[OAM_FORMAT_H3] = (unsigned char) (pOAMF5->VCI >> 4);
	pLBInfo->cell[OAM_FORMAT_H4] = (unsigned char) ((pOAMF5->VCI << 4) & 0x00F0) | ((ucPTI << 1) & 0xE) | (clp & 1);

	pLBInfo->cell[OAM_FORMAT_TYPE] = FAULT_LB;	// OAM type & function
	pLBInfo->cell[OAM_FORMAT_LB_INDICATION] = 1;			// loopback indication bit
	pLBInfo->cell[OAM_FORMAT_LB_TAG] = (unsigned char)(pLBInfo->tag >> 24 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+1] = (unsigned char)(pLBInfo->tag >> 16 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+2] = (unsigned char)(pLBInfo->tag >> 8 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+3] = (unsigned char)(pLBInfo->tag & 0xFF);		// loopback correlation tag

	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pLBInfo->cell[OAM_FORMAT_LB_LLID+i] = pLBInfo->locationID[i];					// loopback location ID
		pLBInfo->cell[OAM_FORMAT_LB_SID+i] = pOAMF5->OAMF5ucCPID[i];					// loopback source ID
	}

	ATM_OAM_DEBUG("The Loopback ID =");
	for (i = 0; i < OAM_LB_SRCID_SIZE; i++)
		ATM_OAM_DEBUG("%.02x", pLBInfo->locationID[i]);
	ATM_OAM_DEBUG("\n");
		
	ATM_OAM_DEBUG("The Source ID =");
	for (i = 0; i < OAM_LB_SRCID_SIZE; i++)
		ATM_OAM_DEBUG("%.02x", pOAMF5->OAMF5ucCPID[i]);
	ATM_OAM_DEBUG("\n");
		
	for (i = 0; i < OAM_LB_UNUSE; i++)
		pLBInfo->cell[OAM_FORMAT_LB_UNUSE+i] = 0x6A;

	ATM_OAM_DEBUG("Leave the OAMF5GenLBCell\n");	
}

__SWAP
OAMF5_LB_INFO *OAMF5FindLBInfo(unsigned long tag, OAMF5 *pOAMF5)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (tag == pOAMF5->OAMF5LBList[i].tag && pOAMF5->OAMF5LBList[i].status != FAULT_LB_IDLE)
			return &(pOAMF5->OAMF5LBList[i]);
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return NULL;
}

__SWAP
OAMF5_LBRX_INFO *OAMF5FindLBRXInfo(OAMF5 *pOAMF5)
{
	return &(pOAMF5->LBRXList[0]);
}

//void OAMF5TMNLBTimeoutFunc(struct timer_list *id, int arg)
void OAMF5TMNLBTimeoutFunc(unsigned long arg)
{
	OAMF5Timer *pTimer;
	OAMF5 *pOAMF5;
	
	ATM_OAM_DEBUG("Into the OAMF5TMNLBTimeoutFunc %x\n", (unsigned int)arg);

	pTimer = (OAMF5Timer*) arg;
	if(pTimer)
	{
		OAMF5_LB_INFO *pLBInfo = NULL;
		//int i;
		pOAMF5 = pTimer->pOAM;
		pTimer->occupied = EMPTY;

		if(pTimer->OAMFunctionType != FAULT_LB)
		{
			ATM_OAM_TRACE("OAMF5TMNLBTimeoutFunc (pTimer->OAMFunctionType != FAULT_LB)\n");
			return;
		}

		pLBInfo = &pOAMF5->OAMF5LBList[pTimer->OAMUserData];

		// double check
		if(&(pLBInfo->timer) != pTimer)
		{
			ATM_OAM_TRACE("OAMF5TMNLBTimeoutFunc (&(pLBInfo->timer) != pTimer)\n");
			return;
		}

		// timestamp
		pLBInfo->rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

		// return to idle state
		pLBInfo->status = FAULT_LB_STOP;
		OAMF5ChangeState(pOAMF5);
	}

	ATM_OAM_DEBUG("Leave the OAMF5TMNLBTimeoutFunc\n");	
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::TMNTxLBStart()
//
//		Input		: Scope (1/0:end/segment), ATM channel, location ID
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Generate an OAM fault management loopback cell
//					  Send out to PHY layer
//					  Start waiting timer and adjust internal state
//					  return true -- cell is ready to be sent
//					  return false -- no cell available
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int OAMF5TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF5 *pOAMF5)
{	
	OAMF5_LB_INFO *pLBInfo = NULL;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF5TMNTxLBStart\n");

	// search for an empty OAMF5_LB_INFO from table
	if (!pLocID[0]) {
		if (pOAMF5->OAMF5LBList[0].status == FAULT_LB_WAITING) {
			ATM_OAM_TRACE("OAMF5TMNTxLBStart (pOAMF5->OAMF5LBList[0].status == FAULT_LB_WAITING)\n");
			return 0;
		}
		pLBInfo = &pOAMF5->OAMF5LBList[0];

		// clean up all-0 table
		for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
			pLBInfo[i].all0_status = FAULT_LB_IDLE;
			pLBInfo[i].count = 0;
			pLBInfo[i].tag = 0;
			for (j = 0; j < OAM_LB_LLID_SIZE; j++)
				pLBInfo[i].locationID[j] = 0;
			pLBInfo[i].timestamp = 0;
			pLBInfo[i].rtt = 0;
		}
	} else {
		for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
			if (pOAMF5->OAMF5LBList[i].status == FAULT_LB_IDLE) {
				pLBInfo = &pOAMF5->OAMF5LBList[i];
				break;
			}
		}

		// if no more IDLE list, pick a STOP list
		if(pLBInfo == NULL) {
			unsigned int timestamp = 0xFFFFFFFF;
			for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
				if ((pOAMF5->OAMF5LBList[i].status == FAULT_LB_STOP) && (pOAMF5->OAMF5LBList[i].timestamp < timestamp)){
					pLBInfo = &pOAMF5->OAMF5LBList[i];
					timestamp = pOAMF5->OAMF5LBList[i].timestamp;
				}
			}
		}
	}

	if (pLBInfo == NULL){
		ATM_OAM_TRACE("OAMF5TMNTxLBStart (pLBInfo == NULL)\n");
		return 0;
	}

	// prepare LB information
//	AtomicSet(&(pLBInfo->status), FAULT_LB_WAITING);	// loopback state: waiting
	pLBInfo->status = FAULT_LB_WAITING;
//	AtomicIncrement((long *)&OAMF5ulFaultLBTag);
	pOAMF5->OAMF5ulFaultLBTag++;
	pLBInfo->tag = pOAMF5->OAMF5ulFaultLBTag-1;
	pLBInfo->scope = Scope;
	pLBInfo->count = 0;

	for (i = 0; i < OAM_LB_LLID_SIZE; i++)
		pLBInfo->locationID[i] = pLocID[i];
	
	OAMF5GenLBCell(pLBInfo, pOAMF5);					// generate OAM LB cell
	OAMF5SendACell(pLBInfo->cell);	// Send out a LB OAM cell
	pOAMF5->OAMF5ulFaultLBTxGoodCell++;

	// adjust internal state
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_WAITING)
		pOAMF5->OAMF5lFaultLBState = FAULT_LB_WAITING;

	// timestamp
	pLBInfo->timestamp = OAMF5GetTimeStamp();
	pLBInfo->rtt = 0;
	
	// start timer
	if(OAMF5StartTimer(&(pLBInfo->timer), (VOIDFUNCPTR )OAMF5TMNLBTimeoutFunc) == 0)
	{
		pLBInfo->status = FAULT_LB_IDLE;
		OAMF5ChangeState(pOAMF5);
	}

	*Tag = pLBInfo->tag;

	ATM_OAM_DEBUG("Leave the OAMF5TMNTxLBStart\n");

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::TMNTxLBStop()
//
//		Input		: Tag
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Stop generating an OAM fault management loopback cell
//					  Cancel timer and adjust internal state
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int OAMF5TMNTxLBStop(unsigned long Tag, OAMF5 *pOAMF5)
{
	OAMF5_LB_INFO *pLBInfo;
	//OAMF5Timer *pLBTimer = NULL;
	OAMF5_LBRX_INFO *pLBRXInfo;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF5TMNTxLBStop\n");

	if ((pLBInfo = OAMF5FindLBInfo(Tag, pOAMF5)) == NULL) return 0;

	// clear OAM FM LB Rx List
	pLBRXInfo = &(pOAMF5->LBRXList[0]);
	for (i = 0; i < LB_ALL_NODE_SIZE; i++)
	{
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pLBRXInfo[i].locationID[j] = 0;
		}

		pLBRXInfo[i].status = EMPTY;
		pLBRXInfo[i].sur_des_count = 0;
	}

	// stop timer
	OAMF5StopTimer(&(pLBInfo->timer));

	// return to idle state
	pLBInfo->status = FAULT_LB_IDLE;
	OAMF5ChangeState(pOAMF5);
	
	ATM_OAM_DEBUG("Out the OAMF5TMNTxLBStop\n");
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::ProcessRxLBCell()
//
//		Input		: pointer of a cell received
//
//		Output		: false/true (discard/loopback)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Verify current loopback state
//					  Verify OAM data
//					  Please refer to figure C.1/I.610
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int OAMF5ProcessRxLBCell(unsigned char *pOamCell, OAMF5 *pOAMF5)
{
	unsigned long PTI = (pOamCell[OAM_FORMAT_H4] & 0xE);
	unsigned char ucLBIndicator = (pOamCell[OAM_FORMAT_LB_INDICATION] & 1);
	int i=0, j;
	// assumed CRC-10 is correct, ASIC will handle CRC-10

	ATM_OAM_DEBUG("Into the OAMF5ProcessRxLBCell\n");		

	// handle FM LB request	
	if ((PTI == 8 || PTI == 0xA) && ucLBIndicator) {
		// segment/end-to-end OAM F5 FM LB
		// it's segment endpoint or connection endpoint

		char Match1 = FALSE, Match2 = FALSE, Match3 = FALSE;		

		// verify LLID
		if (pOamCell[OAM_FORMAT_LB_LLID] == 0x6A)
		{
			ATM_OAM_DEBUG("OAMF5ProcessRxLBCell LLID 6A\n");
			return 0; // no loopback
		}

		ATM_OAM_DEBUG("The rcvd Loopback ID =");
		for (j = 0; j < OAM_LB_SRCID_SIZE; j++)
			ATM_OAM_DEBUG("%x", pOamCell[OAM_FORMAT_LB_LLID+j]);
		ATM_OAM_DEBUG("\n");
		
		if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF5->OAMF5ucCPID, OAM_LB_LLID_SIZE) == 0)
			Match1 = TRUE;	// match connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_0, OAM_LB_LLID_SIZE) == 0)
			Match2 = TRUE;	// all connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_1, OAM_LB_LLID_SIZE) == 0)
			Match3 = TRUE;	// end-to-end loopback

		if (PTI == 0xA)		// is this end-to-end LB, ????
			Match2 = FALSE;

		if (Match1 || Match2 || Match3) { // Loop cell back
			char SidFound = FALSE;
			OAMF5_LBRX_INFO *pLBRXInfo;

			pLBRXInfo = &(pOAMF5->LBRXList[0]);
		
			// reset loopback indication		
			pOamCell[OAM_FORMAT_LB_INDICATION] &= 0xFE;
			// set LLID		
			memcpy(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF5->OAMF5ucCPID, OAM_LB_LLID_SIZE);

			// save source ID
			// ITEX's implementation is save LLID, I think they are wrong. by Dick Tam
			for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
				if((pLBRXInfo[i].status == OCCUPY) &&
					(memcmp(&pOamCell[OAM_FORMAT_LB_SID], pLBRXInfo[i].locationID, OAM_LB_SRCID_SIZE) == 0)) {

					SidFound = TRUE;
					pLBRXInfo[i].sur_des_count++;
					break;
				}					
			}

			if(!SidFound) {
				for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
					if(pLBRXInfo[i].status == EMPTY) {
						pLBRXInfo[i].status = OCCUPY;
						memcpy(pLBRXInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_SID],  OAM_LB_SRCID_SIZE);
						pLBRXInfo[i].sur_des_count++;
						break;
					}
				}
			}

			// debug message
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
				if (pLBRXInfo[i].sur_des_count != 0)
				{
					ATM_OAM_DEBUG("The Reomte ID =");
					for (j = 0; j < OAM_LB_SRCID_SIZE; j++)
						ATM_OAM_DEBUG("%x", pLBRXInfo[i].locationID[j]);
					ATM_OAM_DEBUG(", The Rx Remote Count = %d\n", (unsigned int)pLBRXInfo[i].sur_des_count);
				}
			}

			// Do not have to change cell header
			// cell is ready to be sent out at the moment
			return 1;
		}
	}

	// handle FM LB response	
	if (pOAMF5->OAMF5lFaultLBState == FAULT_LB_WAITING) {
		if ((PTI == 8 || PTI == 0xA) && (!ucLBIndicator)){
			OAMF5_LB_INFO *pLBInfo = NULL;
			unsigned long LBTag;
			
			// verify loopback source ID
			if (memcmp(&pOamCell[OAM_FORMAT_LB_SID], pOAMF5->OAMF5ucCPID, OAM_LB_SRCID_SIZE) != 0)
				return 0;

			// verify correlation tag value
			LBTag = (unsigned long)(pOamCell[OAM_FORMAT_LB_TAG]); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+1])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+2])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+3]));
				
			if ((pLBInfo = OAMF5FindLBInfo(LBTag, pOAMF5)) != NULL) {

				// loopback successful
				if (pLBInfo->status == FAULT_LB_WAITING) {

					if (pLBInfo == &pOAMF5->OAMF5LBList[0])
					{
						// all zero LB, don't stop timer
						// wait 5 second to collect all response

						// save LLID to pLBInfo->locationID, handle counter
						for (i = 0; i < LB_ALL_NODE_SIZE; i++)
						{
							if (pLBInfo[i].all0_status == FAULT_LB_WAITING) {

								// match LLID
								if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pLBInfo[i].locationID, OAM_LB_LLID_SIZE) == 0)
								{
									pLBInfo[i].count++;
									return 0;
								}

							} else if (pLBInfo[i].all0_status == FAULT_LB_IDLE) {

								// save LLID
								pLBInfo[i].all0_status = FAULT_LB_WAITING;
								pLBInfo[i].count++;
								memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

								// timestamp
								pLBInfo[i].rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

								return 0;
							}
						}

						// more than LB_ALL_NODE_SIZE reponse, how to handle it ?

					} else {
						pLBInfo->count++;

						// timestamp
						pLBInfo->rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

						// stop timer
						OAMF5StopTimer(&(pLBInfo->timer));					

						// save LLID
						memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

						// return to idle state
						pLBInfo->status = FAULT_LB_STOP;
						OAMF5ChangeState(pOAMF5);
					}

					// Should tell TMN that LB successes.
					// TODO ... 
					// (or TMN may monitor Variable ulFaultLBRxGoodCell instead.)
				}
			}
		}
	}

	ATM_OAM_DEBUG("Leave the OAMF5ProcessRxLBCell\n");		

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//		OAMRxF5Cell
//
//		Input		: pointer of an OAM cell
//
//		Output		: None
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: OAM F5 handler routine
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
void OAMRxF5Cell(unsigned char *pOamCell, OAMF5 *pOAMF5)
{
	ATM_OAM_DEBUG("Into the RxOAMF5Cell %d\n", pOamCell[OAM_FORMAT_TYPE]);

	cell_debug(pOamCell, debug_obcell, 0);
#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %x %x %x %x %x %x %x %x %x %x %x %x %x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

 	switch (pOamCell[OAM_FORMAT_TYPE]) {

#if 0
	case FAULT_AIS:
	case FAULT_RDI:
	case FAULT_CC:
	case PERFORMANCE_FPM:
	case PERFORMANCE_BACKWARD_REPORT:
	case ACT_DEACT_FPM:
	case ACT_DEACT_FPM_BR:
	case ACT_DEACT_CC:
	case APS_GROUP_PROTECT:
	case APS_INDIVIDUAL_PROTECT:
	case SYSTEM_MANAGEMENT:
#endif


	case FAULT_LB:
		if  (pOAMF5 != NULL)
		{
			if (OAMF5ProcessRxLBCell(pOamCell, pOAMF5))
			{
				OAMF5SendACell(pOamCell);	// send LB cell out
				pOAMF5->OAMF5ulFaultLBTxGoodCell++;
			}
		}
		pOAMF5->OAMF5ulFaultLBRxGoodCell++;
		break;

	default:
		break;

	}

	ATM_OAM_DEBUG("Leave the OAMRxF5Cell\n");		
}



/* OAM F4 */
OAMF4 OAMF4_info;

#define OAMF4GetTimeStamp OAMF5GetTimeStamp
#define OAMF4StartTimer OAMF5StartTimer
#define OAMF4StopTimer OAMF5StopTimer
#define OAMF4SetLBID OAMF5SetLBID
#define OAMF4SendACell OAMF5SendACell

void OAMF4Init(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4)
{
	int i, j;
	//unsigned char gfc = 0, clp = 0, ucPTI;

	pOAMF4->VPI = vpi;
	pOAMF4->VCI = vci;
	
	/* OAM FM --LB */
	pOAMF4->OAMF4lFaultLBState = FAULT_LB_IDLE;			// loopback state: idle
	pOAMF4->OAMF4ulFaultLBTag = 0;						// loopback correlation tag
	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pOAMF4->OAMF4ucCPID[i] = 0xFF;					// connection point ID
	}

	// initialize LB table for TMN transmission only
	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		pOAMF4->OAMF4LBList[i].tag = 0;
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF4->OAMF4LBList[i].locationID[j] = 0;
		}
		pOAMF4->OAMF4LBList[i].scope = 0;
		pOAMF4->OAMF4LBList[i].count = 0;
		pOAMF4->OAMF4LBList[i].status = FAULT_LB_IDLE;

		// initialize LB timer
		pOAMF4->OAMF4LBList[i].timer.pOAM = (void *)pOAMF4;
		pOAMF4->OAMF4LBList[i].timer.OAMFunctionType = FAULT_LB;
		pOAMF4->OAMF4LBList[i].timer.OAMUserData = i;
		pOAMF4->OAMF4LBList[i].timer.oneShot = 1;
		pOAMF4->OAMF4LBList[i].timer.occupied = EMPTY;
		pOAMF4->OAMF4LBList[i].timer.ulTimeout = OAM_LB_WAIT_TIME;		
	}

	for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF4->LBRXList[i].locationID[j] = 0;
		}
		pOAMF4->LBRXList[i].status = EMPTY;
		pOAMF4->LBRXList[i].sur_des_count = 0;
	}
	
	pOAMF4->OAMF4ulFaultLBTxGoodCell = 0;
	pOAMF4->OAMF4ulFaultLBRxGoodCell = 0;

	ATM_OAM_DEBUG("ATM OAM F4 initialized.\n");
}

void OAMF4ChangeState(OAMF4 *pOAMF4)
{
	int i;
	OAMF4_LB_INFO *pLBInfo = NULL;

	// FM LB state
	for (i = 0, pLBInfo = &pOAMF4->OAMF4LBList[0]; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if(pLBInfo[i].status == FAULT_LB_WAITING)
			return;
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	pOAMF4->OAMF4lFaultLBState = FAULT_LB_IDLE;
	
	ATM_OAM_DEBUG("OAMF4ChangeState >> IDLE\n");	
}

__SWAP
int OAMF4SetVpiVci(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4)
{
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_IDLE) return 0;
	pOAMF4->VPI = vpi;
	pOAMF4->VCI = vci;
	return 1;
}


__SWAP
int OAMF4TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF4 *pOAMF4)
{
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_IDLE) return 0;
	OAMF5SetLBID(ucLocationType, pOAMF4->OAMF4ucCPID, pCPID);
	return 1;
}

__SWAP
void OAMF4GenLBCell(OAMF5_LB_INFO *pLBInfo, OAMF4 *pOAMF4)
{
	unsigned long VCI = (pLBInfo->scope)? 4 : 3;
	unsigned char gfc = 0, clp = 0, ucPTI = 0;
	int i;

	ATM_OAM_DEBUG("Into the OAMF4GenLBCell\n");

	// generate cell header & payload
	pLBInfo->cell[OAM_FORMAT_H1] = (unsigned char) ((pOAMF4->VPI >> 4) & 0x0F) | ((gfc << 4) & 0xF0);
	pLBInfo->cell[OAM_FORMAT_H2] = (unsigned char) (((pOAMF4->VPI << 4) & 0x00F0) | (VCI >> 12));
	pLBInfo->cell[OAM_FORMAT_H3] = (unsigned char) (VCI >> 4);
	pLBInfo->cell[OAM_FORMAT_H4] = (unsigned char) ((VCI << 4) & 0x00F0) | ((ucPTI << 1) & 0xE) | (clp & 1);

	pLBInfo->cell[OAM_FORMAT_TYPE] = FAULT_LB;	// OAM type & function
	pLBInfo->cell[OAM_FORMAT_LB_INDICATION] = 1;			// loopback indication bit
	pLBInfo->cell[OAM_FORMAT_LB_TAG] = (unsigned char)(pLBInfo->tag >> 24 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+1] = (unsigned char)(pLBInfo->tag >> 16 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+2] = (unsigned char)(pLBInfo->tag >> 8 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+3] = (unsigned char)(pLBInfo->tag & 0xFF);		// loopback correlation tag

	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pLBInfo->cell[OAM_FORMAT_LB_LLID+i] = pLBInfo->locationID[i];					// loopback location ID
		pLBInfo->cell[OAM_FORMAT_LB_SID+i] = pOAMF4->OAMF4ucCPID[i];					// loopback source ID
	}

	for (i = 0; i < OAM_LB_UNUSE; i++)
		pLBInfo->cell[OAM_FORMAT_LB_UNUSE+i] = 0x6A;

	ATM_OAM_DEBUG("Leave the OAMF4GenLBCell\n");	
}

__SWAP
OAMF4_LB_INFO *OAMF4FindLBInfo(unsigned long tag, OAMF4 *pOAMF4)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (tag == pOAMF4->OAMF4LBList[i].tag && pOAMF4->OAMF4LBList[i].status != FAULT_LB_IDLE)
			return &(pOAMF4->OAMF4LBList[i]);
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return NULL;
}

__SWAP
OAMF4_LBRX_INFO *OAMF4FindLBRXInfo(OAMF4 *pOAMF4)
{
	return &(pOAMF4->LBRXList[0]);
}

//void OAMF4TMNLBTimeoutFunc(struct timer_list *id, int arg)
void OAMF4TMNLBTimeoutFunc(unsigned long arg)
{
	OAMF5Timer *pTimer;
	OAMF4 *pOAMF4;
	
	ATM_OAM_DEBUG("Into the OAMF4TMNLBTimeoutFunc %x\n",(unsigned int) arg);

	pTimer = (OAMF4Timer*) arg;
	if(pTimer)
	{
		OAMF4_LB_INFO *pLBInfo = NULL;
		//int i;
		pOAMF4 = pTimer->pOAM;
		pTimer->occupied = EMPTY;

		if(pTimer->OAMFunctionType != FAULT_LB)
		{
			ATM_OAM_TRACE("OAMF4TMNLBTimeoutFunc (pTimer->OAMFunctionType != FAULT_LB)\n");
			return;
		}

		pLBInfo = &pOAMF4->OAMF4LBList[pTimer->OAMUserData];

		// double check
		if(&(pLBInfo->timer) != pTimer)
		{
			ATM_OAM_TRACE("OAMF4TMNLBTimeoutFunc (&(pLBInfo->timer) != pTimer)\n");
			return;
		}

		// timestamp
		pLBInfo->rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

		// return to idle state
		pLBInfo->status = FAULT_LB_STOP;
		OAMF4ChangeState(pOAMF4);
	}

	ATM_OAM_DEBUG("Leave the OAMF4TMNLBTimeoutFunc\n");	
}

__SWAP
int OAMF4TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF4 *pOAMF4)
{
	OAMF5_LB_INFO *pLBInfo = NULL;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF4TMNTxLBStart\n");

	// search for an empty OAMF4_LB_INFO from table
	if (!pLocID[0]) {
		if (pOAMF4->OAMF4LBList[0].status == FAULT_LB_WAITING) {
			ATM_OAM_TRACE("OAMF4TMNTxLBStart (pOAMF4->OAMF4LBList[0].status == FAULT_LB_WAITING)\n");
			return 0;
		}
		pLBInfo = &pOAMF4->OAMF4LBList[0];

		// clean up all-0 table
		for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
			pLBInfo[i].all0_status = FAULT_LB_IDLE;
			pLBInfo[i].count = 0;
			pLBInfo[i].tag = 0;
			for (j = 0; j < OAM_LB_LLID_SIZE; j++)
				pLBInfo[i].locationID[j] = 0;
			pLBInfo[i].timestamp = 0;
			pLBInfo[i].rtt = 0;
		}
	} else {
		for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
			if (pOAMF4->OAMF4LBList[i].status == FAULT_LB_IDLE) {
				pLBInfo = &pOAMF4->OAMF4LBList[i];
				break;
			}
		}

		// if no more IDLE list, pick a STOP list
		if(pLBInfo == NULL) {
			unsigned int timestamp = 0xFFFFFFFF;
			for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
				if ((pOAMF4->OAMF4LBList[i].status == FAULT_LB_STOP) && (pOAMF4->OAMF4LBList[i].timestamp < timestamp)){
					pLBInfo = &pOAMF4->OAMF4LBList[i];
					timestamp = pOAMF4->OAMF4LBList[i].timestamp;
				}
			}
		}
	}

	if (pLBInfo == NULL){
		ATM_OAM_TRACE("OAMF4TMNTxLBStart (pLBInfo == NULL)\n");
		return 0;
	}

	// prepare LB information
//	AtomicSet(&(pLBInfo->status), FAULT_LB_WAITING);	// loopback state: waiting
	pLBInfo->status = FAULT_LB_WAITING;
//	AtomicIncrement((long *)&OAMF5ulFaultLBTag);
	pOAMF4->OAMF4ulFaultLBTag++;
	pLBInfo->tag = pOAMF4->OAMF4ulFaultLBTag-1;
	pLBInfo->scope = Scope;
	pLBInfo->count = 0;

	for (i = 0; i < OAM_LB_LLID_SIZE; i++)
		pLBInfo->locationID[i] = pLocID[i];
	
	OAMF4GenLBCell(pLBInfo, pOAMF4);					// generate OAM LB cell
	OAMF4SendACell(pLBInfo->cell);	// Send out a LB OAM cell
	pOAMF4->OAMF4ulFaultLBTxGoodCell++;

	// adjust internal state
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_WAITING)
		pOAMF4->OAMF4lFaultLBState = FAULT_LB_WAITING;

	// timestamp
	pLBInfo->timestamp = OAMF4GetTimeStamp();
	pLBInfo->rtt = 0;
	
	// start timer
	if(OAMF4StartTimer(&(pLBInfo->timer), (VOIDFUNCPTR )OAMF4TMNLBTimeoutFunc) == 0)
	{
		pLBInfo->status = FAULT_LB_IDLE;
		OAMF4ChangeState(pOAMF4);
	}

	*Tag = pLBInfo->tag;

	ATM_OAM_DEBUG("Leave the OAMF4TMNTxLBStart\n");

	return 1;
}

__SWAP
int OAMF4TMNTxLBStop(unsigned long Tag, OAMF4 *pOAMF4)
{
	OAMF4_LB_INFO *pLBInfo;
	//OAMF4Timer *pLBTimer = NULL;
	OAMF4_LBRX_INFO *pLBRXInfo;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF4TMNTxLBStop\n");

	if ((pLBInfo = OAMF4FindLBInfo(Tag, pOAMF4)) == NULL) return 0;

	// clear OAM FM LB Rx List
	pLBRXInfo = &(pOAMF4->LBRXList[0]);
	for (i = 0; i < LB_ALL_NODE_SIZE; i++)
	{
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pLBRXInfo[i].locationID[j] = 0;
		}

		pLBRXInfo[i].status = EMPTY;
		pLBRXInfo[i].sur_des_count = 0;
	}

	// stop timer
	OAMF4StopTimer(&(pLBInfo->timer));

	// return to idle state
	pLBInfo->status = FAULT_LB_IDLE;
	OAMF4ChangeState(pOAMF4);
	
	ATM_OAM_DEBUG("Out the OAMF4TMNTxLBStop\n");
	return 1;
}

__SWAP
unsigned short get_vci(unsigned char *pCell)
{
	return (((((unsigned short) pCell[1]) << 12) & (0xf000)) | ((((unsigned short) pCell[2]) << 4) & (0x0ff0))
				| ((((unsigned short) pCell[3]) >> 4) & (0x000f)));
}

__SWAP
int OAMF4ProcessRxLBCell(unsigned char *pOamCell, OAMF4 *pOAMF4)
{
	unsigned long VCI = get_vci(pOamCell);
	unsigned char ucLBIndicator = (pOamCell[OAM_FORMAT_LB_INDICATION] & 1);
	int i=0, j;
	// assumed CRC-10 is correct, ASIC will handle CRC-10

	ATM_OAM_DEBUG("Into the OAMF4ProcessRxLBCell\n");		

	// handle FM LB request	
	if ((VCI == 3 || VCI == 4) && ucLBIndicator){
		// segment/end-to-end OAM F5 FM LB
		// it's segment endpoint or connection endpoint

		char Match1 = FALSE, Match2 = FALSE, Match3 = FALSE;		

		// verify LLID
		if (pOamCell[OAM_FORMAT_LB_LLID] == 0x6A)
		{
			ATM_OAM_DEBUG("OAMF4ProcessRxLBCell LLID 6A\n");
			return 0; // no loopback
		}

		if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF4->OAMF4ucCPID, OAM_LB_LLID_SIZE) == 0)
			Match1 = TRUE;	// match connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_0, OAM_LB_LLID_SIZE) == 0)
			Match2 = TRUE;	// all connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_1, OAM_LB_LLID_SIZE) == 0)
			Match3 = TRUE;	// end-to-end loopback

		if (VCI == 4)		// is this end-to-end LB
			Match2 = FALSE;

		if (Match1 || Match2 || Match3) { // Loop cell back
			char SidFound = FALSE;
			OAMF5_LBRX_INFO *pLBRXInfo;

			pLBRXInfo = &(pOAMF4->LBRXList[0]);
		
			// reset loopback indication		
			pOamCell[OAM_FORMAT_LB_INDICATION] &= 0xFE;
			// set LLID		
			memcpy(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF4->OAMF4ucCPID, OAM_LB_LLID_SIZE);

			// save source ID
			// ITEX's implementation is save LLID, I think they are wrong. by Dick Tam
			for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
				if((pLBRXInfo[i].status == OCCUPY) &&
					(memcmp(&pOamCell[OAM_FORMAT_LB_SID], pLBRXInfo[i].locationID, OAM_LB_SRCID_SIZE) == 0)) {

					SidFound = TRUE;
					pLBRXInfo[i].sur_des_count++;
					break;
				}					
			}

			if(!SidFound) {
				for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
					if(pLBRXInfo[i].status == EMPTY) {
						pLBRXInfo[i].status = OCCUPY;
						memcpy(pLBRXInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_SID],  OAM_LB_SRCID_SIZE);
						pLBRXInfo[i].sur_des_count++;
						break;
					}
				}
			}

			// debug message
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
				if (pLBRXInfo[i].sur_des_count != 0)
				{
					ATM_OAM_DEBUG("The Reomte ID =");
					for (j = 0; j < OAM_LB_SRCID_SIZE; j++)
						ATM_OAM_DEBUG("%x", pLBRXInfo[i].locationID[j]);
					ATM_OAM_DEBUG(", The Rx Remote Count = %d\n", (unsigned int)pLBRXInfo[i].sur_des_count);
				}
			}

			// Do not have to change cell header
			// cell is ready to be sent out at the moment
			return 1;
		}
	}

	// handle FM LB response	
	if (pOAMF4->OAMF4lFaultLBState == FAULT_LB_WAITING) {
		if ((VCI == 3 || VCI == 4) && (!ucLBIndicator)){
			OAMF4_LB_INFO *pLBInfo = NULL;
			unsigned long LBTag;
			
			// verify loopback source ID
			if (memcmp(&pOamCell[OAM_FORMAT_LB_SID], pOAMF4->OAMF4ucCPID, OAM_LB_SRCID_SIZE) != 0)
				return 0;

			// verify correlation tag value
			LBTag = (unsigned long)(pOamCell[OAM_FORMAT_LB_TAG]); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+1])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+2])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+3]));
				
			if ((pLBInfo = OAMF4FindLBInfo(LBTag, pOAMF4)) != NULL) {

				// loopback successful
				if (pLBInfo->status == FAULT_LB_WAITING) {

					if (pLBInfo == &pOAMF4->OAMF4LBList[0])
					{
						// all zero LB, don't stop timer
						// wait 5 second to collect all response

						// save LLID to pLBInfo->locationID, handle counter
						for (i = 0; i < LB_ALL_NODE_SIZE; i++)
						{
							if (pLBInfo[i].all0_status == FAULT_LB_WAITING) {

								// match LLID
								if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pLBInfo[i].locationID, OAM_LB_LLID_SIZE) == 0)
								{
									pLBInfo[i].count++;
									return 0;
								}

							} else if (pLBInfo[i].all0_status == FAULT_LB_IDLE) {

								// save LLID
								pLBInfo[i].all0_status = FAULT_LB_WAITING;
								pLBInfo[i].count++;
								memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

								// timestamp
								pLBInfo[i].rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

								return 0;
							}
						}

						// more than LB_ALL_NODE_SIZE reponse, how to handle it ?

					} else {
						pLBInfo->count++;

						// timestamp
						pLBInfo->rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

						// stop timer
						OAMF4StopTimer(&(pLBInfo->timer));					

						// save LLID
						memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

						// return to idle state
						pLBInfo->status = FAULT_LB_STOP;
						OAMF4ChangeState(pOAMF4);
					}

					// Should tell TMN that LB successes.
					// TODO ... 
					// (or TMN may monitor Variable ulFaultLBRxGoodCell instead.)
				}
			}
		}
	}

	ATM_OAM_DEBUG("Leave the OAMF4ProcessRxLBCell\n");		

	return 0;
}

__SWAP
void OAMRxF4Cell(unsigned char *pOamCell, OAMF4 *pOAMF4)
{
	ATM_OAM_DEBUG("Into the RxOAMF4Cell %d\n", pOamCell[OAM_FORMAT_TYPE]);

	cell_debug(pOamCell, debug_obcell, 0);
#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %x %x %x %x %x %x %x %x %x %x %x %x %x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

 	switch (pOamCell[OAM_FORMAT_TYPE]) {

#if 0
	case FAULT_AIS:
	case FAULT_RDI:
	case FAULT_CC:
	case PERFORMANCE_FPM:
	case PERFORMANCE_BACKWARD_REPORT:
	case ACT_DEACT_FPM:
	case ACT_DEACT_FPM_BR:
	case ACT_DEACT_CC:
	case APS_GROUP_PROTECT:
	case APS_INDIVIDUAL_PROTECT:
	case SYSTEM_MANAGEMENT:
#endif


	case FAULT_LB:
		if  (pOAMF4 != NULL)
		{
			if (OAMF4ProcessRxLBCell(pOamCell, pOAMF4))
			{
				OAMF4SendACell(pOamCell);	// send LB cell out
				pOAMF4->OAMF4ulFaultLBTxGoodCell++;
			}
		}
		pOAMF4->OAMF4ulFaultLBRxGoodCell++;
		break;

	default:
		break;

	}

	ATM_OAM_DEBUG("Leave the OAMRxF4Cell\n");		
}








/////////////////////////////////////////////////////////////////////////////
//
//		SetAtmOAMCpid
//
//		Input		: pointer of ATMOAMLBID structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int SetAtmOAMCpid(ATMOAMLBID *pATMOAMLBID)
{
	/* F5 */
	if(OAMF5SetVpiVci(pATMOAMLBID->vpi, pATMOAMLBID->vci, &OAMF5_info) == 0)
		return 0;

	if(OAMF5TMNSetCPID(pATMOAMLBID->LocID[0], &(pATMOAMLBID->LocID[1]), &OAMF5_info) == 0)
		return 0;

	if(OAMF4SetVpiVci(pATMOAMLBID->vpi, pATMOAMLBID->vci, &OAMF4_info) == 0)
		return 0;

	if(OAMF4TMNSetCPID(pATMOAMLBID->LocID[0], &(pATMOAMLBID->LocID[1]), &OAMF4_info) == 0)
		return 0;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		StartAtmOAMLoopBack
//
//		Input		: pointer of ATMOAMLBReq structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int StartAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq)
{
	int i;
	
	if((pATMOAMLBReq->vci == 3) || (pATMOAMLBReq->vci == 4))
	{	// F4 LB
		if (pATMOAMLBReq->vpi != 0 || pATMOAMLBReq->vci != 0)
			if(OAMF4SetVpiVci(pATMOAMLBReq->vpi, pATMOAMLBReq->vci, &OAMF4_info) == 0)
				return 0;
		for (i=0; i<16; i++)
		{
			if (pATMOAMLBReq->SrcID[i] != 0)
			{
				if (OAMF4TMNSetCPID(pATMOAMLBReq->SrcID[0], &pATMOAMLBReq->SrcID[1], &OAMF4_info) == 0)
					return 0;
				break;
			}
		}
		return OAMF4TMNTxLBStart(pATMOAMLBReq->Scope, pATMOAMLBReq->LocID, &(pATMOAMLBReq->Tag), &OAMF4_info);
	}
	else
	{	// F5 LB
		if (pATMOAMLBReq->vpi != 0 || pATMOAMLBReq->vci != 0)
			if(OAMF5SetVpiVci(pATMOAMLBReq->vpi, pATMOAMLBReq->vci, &OAMF5_info) == 0)
				return 0;
		for (i=0; i<16; i++)
		{
			if (pATMOAMLBReq->SrcID[i] != 0)
			{
				if (OAMF5TMNSetCPID(pATMOAMLBReq->SrcID[0], &pATMOAMLBReq->SrcID[1], &OAMF5_info) == 0)
					return 0;
				break;
			}
		}
		return OAMF5TMNTxLBStart(pATMOAMLBReq->Scope, pATMOAMLBReq->LocID, &(pATMOAMLBReq->Tag), &OAMF5_info);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		StopAtmOAMLoopBack
//
//		Input		: pointer of ATMOAMLBReq structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int StopAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq)
{
	if((pATMOAMLBReq->vci == 3) || (pATMOAMLBReq->vci == 4))
	{	// F4 LB
		return OAMF4TMNTxLBStop(pATMOAMLBReq->Tag, &OAMF4_info);
	}
	else
	{	// F5 LB
		return OAMF5TMNTxLBStop(pATMOAMLBReq->Tag, &OAMF5_info);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		GetAtmOAMLoopBackStatus
//
//		Input		: pointer of ATMOAMLBState structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int GetAtmOAMLoopBackStatus(ATMOAMLBState *pATMOAMLBState)
{
	if((pATMOAMLBState->vci == 3) || (pATMOAMLBState->vci == 4))
	{	// F4 LB
		OAMF4_LB_INFO* LBInfo = OAMF4FindLBInfo(pATMOAMLBState->Tag, &OAMF4_info);
		if(LBInfo == NULL)
		return 0;

		if(LBInfo == OAMF4_info.OAMF4LBList)
		{
			unsigned int i;
			for(i=0; i<6; i++)
			{
				memcpy(pATMOAMLBState->LocID[i], LBInfo[i].locationID, OAM_LB_LLID_SIZE);
				pATMOAMLBState->count[i] = LBInfo[i].count;
				pATMOAMLBState->rtt[i] = LBInfo[i].rtt;
				pATMOAMLBState->status[i] = LBInfo[i].all0_status;
			}
		}
		else
		{
			memset(pATMOAMLBState->LocID, 0x00, 6*16*sizeof(unsigned char));
			memset(pATMOAMLBState->count, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->rtt, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->status, 0x00, 6*sizeof(long));

			memcpy(pATMOAMLBState->LocID[0], LBInfo->locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBState->count[0] = LBInfo->count;
			pATMOAMLBState->rtt[0] = LBInfo->rtt;
			pATMOAMLBState->status[0] = LBInfo->status;			
		}

		return 1;
	}
	else
	{	// F5 LB
		OAMF5_LB_INFO* LBInfo = OAMF5FindLBInfo(pATMOAMLBState->Tag, &OAMF5_info);
		if(LBInfo == NULL)
			return 0;

		if(LBInfo == OAMF5_info.OAMF5LBList)
		{
			unsigned int i;
			for(i=0; i<6; i++)
			{
				memcpy(pATMOAMLBState->LocID[i], LBInfo[i].locationID, OAM_LB_LLID_SIZE);
				pATMOAMLBState->count[i] = LBInfo[i].count;
				pATMOAMLBState->rtt[i] = LBInfo[i].rtt;
				pATMOAMLBState->status[i] = LBInfo[i].all0_status;
			}
		}
		else
		{
			memset(pATMOAMLBState->LocID, 0x00, 6*16*sizeof(unsigned char));
			memset(pATMOAMLBState->count, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->rtt, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->status, 0x00, 6*sizeof(long));

			memcpy(pATMOAMLBState->LocID[0], LBInfo->locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBState->count[0] = LBInfo->count;
			pATMOAMLBState->rtt[0] = LBInfo->rtt;
			pATMOAMLBState->status[0] = LBInfo->status;			
		}

		return 1;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		GetAtmOAMLoopBackStatusFE
//
//		Input		: pointer of ATMOAMLBRXState structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
__SWAP
int GetAtmOAMLoopBackStatusFE(ATMOAMLBRXState *pATMOAMLBRXState)
{
	if((pATMOAMLBRXState->vci == 3) || (pATMOAMLBRXState->vci == 4))
	{	// F4 LB
		unsigned int i;
		for(i=0; i<LB_ALL_NODE_SIZE; i++)
		{
			memcpy(pATMOAMLBRXState->LocID[i], OAMF4_info.LBRXList[i].locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBRXState->count[i] = OAMF4_info.LBRXList[i].sur_des_count;
			pATMOAMLBRXState->status[i] = OAMF4_info.LBRXList[i].status;
		}

		return 1;
	}
	else
	{	// F5 LB
		unsigned int i;
		for(i=0; i<LB_ALL_NODE_SIZE; i++)
		{
			memcpy(pATMOAMLBRXState->LocID[i], OAMF5_info.LBRXList[i].locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBRXState->count[i] = OAMF5_info.LBRXList[i].sur_des_count;
			pATMOAMLBRXState->status[i] = OAMF5_info.LBRXList[i].status;
		}

		return 1;
	}
}





#ifdef ATM_OAM_TEST
/* For Test only */

static unsigned long current_tag;

void OAMTestInit(void)
{
	ATMOAMLBID lbid;
	unsigned int i;

	lbid.vpi = 5;
	lbid.vci = 35;

	for(i=0; i<16; i++)
	{
		lbid.LocID[i] = i+1;
	}

	SetAtmOAMCpid(&lbid);
}

void OAMF4LbTest(void)
{
	ATMOAMLBReq req;

	req.vpi = 0;
	req.vci = 3;
	req.Scope = 1;	
	req.LocID[0] = 0x86;
	req.LocID[1] = 0x70;
	req.LocID[2] = 0x86;
	req.LocID[3] = 0x70;
	req.LocID[4] = 0x86;
	req.LocID[5] = 0x70;
	req.LocID[6] = 0x86;
	req.LocID[7] = 0x70;
	req.LocID[8] = 0x86;
	req.LocID[9] = 0x70;
	req.LocID[10] = 0x86;
	req.LocID[11] = 0x70;
	req.LocID[12] = 0x86;
	req.LocID[13] = 0x70;
	req.LocID[14] = 0x86;
	req.LocID[15] = 0x70;

	StartAtmOAMLoopBack(&req);

	current_tag = req.Tag;

	ATM_OAM_DEBUG("Tag %u\n", (unsigned int)req.Tag);
}

//0=>fail, 1=>success
__SWAP
int OAMF5LbTest(ATMOAMLBReq *pucPtr)
{
	if (0==StartAtmOAMLoopBack(pucPtr)) {
	    printk("OAMF5LbTest no responce! Test stop.\n");
	    return 0;
	};

	current_tag = pucPtr->Tag;

	ATM_OAM_DEBUG("Tag %u\n", (unsigned int)pucPtr->Tag);
	return 1;
}

void OAMF5StopLbTest(void);

static OAMF5Timer oamf5lbtest_timer;
__SWAP
void OAMF5LbTestTimer(ATMOAMLBReq *pucPtr)
{
	if (0==OAMF5LbTest(pucPtr)) {
    	    OAMF5StopLbTest();
	    return;
	};

	// start timer for next OAM Tx
        init_timer (&oamf5lbtest_timer.timer);
        oamf5lbtest_timer.timer.expires = jiffies + oamf5lbtest_timer.ulTimeout;
        oamf5lbtest_timer.timer.function = (void (*)(unsigned long))OAMF5LbTestTimer;
        oamf5lbtest_timer.timer.data = (unsigned long)(pucPtr);
	add_timer(&oamf5lbtest_timer.timer);

	oamf5lbtest_timer.occupied = OCCUPY;
}

__SWAP
void OAMF5TMNAllStop(OAMF5 *pOAMF5)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (pOAMF5->OAMF5LBList[i].status != FAULT_LB_IDLE) {
			// stop timer
			OAMF5StopTimer(&(pOAMF5->OAMF5LBList[i].timer));
		};
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return;
}

static ATMOAMLBReq gblpucPtr;
__SWAP
void OAMF5ContiLbTest(ATMOAMLBReq *pucPtr)
{
	if(oamf5lbtest_timer.occupied == OCCUPY)
	{
		ATM_OAM_TRACE("OAMF5ContiLbTest (oamf5lbtest_timer.occupied == OCCUPY)\n");
		return;
	}
	oamf5lbtest_timer.ulTimeout = (300 * HZ) / 1000;
	memcpy(&gblpucPtr, pucPtr, sizeof(ATMOAMLBReq));
        OAMF5TMNAllStop(&OAMF5_info);
	OAMF5Init(pucPtr->vpi, pucPtr->vci, &OAMF5_info);	
	OAMF5LbTestTimer(&gblpucPtr);
}

__SWAP
void OAMF5StopLbTest(void)
{
    if(oamf5lbtest_timer.occupied == EMPTY)
	return;
    OAMF5TMNAllStop(&OAMF5_info);
    del_timer(&oamf5lbtest_timer.timer);
    oamf5lbtest_timer.occupied = EMPTY;
}

#if 0

void OAMF4LbTestTimer(struct timer_list *id, int arg)
{
	struct itimerspec time_value;

	if (timer_create(CLOCK_REALTIME, NULL, &oamf5lbtest_timer) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer timer_create Fail\n");
		return;
	}

	if (timer_connect(oamf5lbtest_timer, (VOIDFUNCPTR)OAMF4LbTestTimer, (int) NULL) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer timer_connect Fail\n");
		return;
	}

	bzero((char*)&time_value, sizeof(struct itimerspec));
	time_value.it_value.tv_sec = 1;
	time_value.it_value.tv_nsec = 0;

	if (timer_settime(oamf5lbtest_timer, 0, &time_value, NULL) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer  timer_settime Fail\n");
		return;
	}

	OAMF4LbTest();
}

void OAMF4ContiLbTest(void)
{
	OAMF4LbTestTimer(0, 0);
}

void OAMF5Dump(void)
{	// ITEX_ATM_OAM_STATUS

	int i, j, k;
	OAMF5_LB_INFO *pLBInfo = NULL;
	OAMF5_LBRX_INFO *pLBRXInfo = NULL;

	for (k = 0; k < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; k++) 
	{
		pLBInfo = &(OAMF5_info.OAMF5LBList[k]);

		ATM_OAM_TRACE("[FM LB Info] Tag:%d, status:%d, count:%d, timestamp:%u ,rtt:%u, LLID:"
			,pLBInfo->tag
			,pLBInfo->status
			,pLBInfo->count
			,pLBInfo->timestamp
			,pLBInfo->rtt);


		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
			ATM_OAM_TRACE("%x", pLBInfo->locationID[j]);
		ATM_OAM_TRACE("\n");
	}

	if ((pLBRXInfo = OAMF5FindLBRXInfo(&OAMF5_info)) != NULL) 
		{
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
			if (pLBRXInfo[i].sur_des_count != 0)
				{
				ATM_OAM_TRACE("The Reomte ID =");
					for (j = 0; j < OAM_LB_LLID_SIZE; j++)
					ATM_OAM_TRACE("%x", pLBRXInfo[i].locationID[j]);
				ATM_OAM_TRACE(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
				}
			}
			}
		}

void OAMF4Dump(void)
{	// ITEX_ATM_OAM_STATUS

	int i, j, k;
	OAMF4_LB_INFO *pLBInfo = NULL;
	OAMF4_LBRX_INFO *pLBRXInfo = NULL;

	for (k = 0; k < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; k++) 
	{
		pLBInfo = &(OAMF4_info.OAMF4LBList[k]);

		ATM_OAM_TRACE("[FM LB Info] Tag:%d, status:%d, count:%d, timestamp:%u ,rtt:%u, LLID:"
			,pLBInfo->tag
			,pLBInfo->status
			,pLBInfo->count
			,pLBInfo->timestamp
			,pLBInfo->rtt);

		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
			ATM_OAM_TRACE("%x", pLBInfo->locationID[j]);
		ATM_OAM_TRACE("\n");
	}

	if ((pLBRXInfo = OAMF4FindLBRXInfo(&OAMF4_info)) != NULL) 
	{
		for (i = 0; i < LB_ALL_NODE_SIZE; i++)
		{
			if (pLBRXInfo[i].sur_des_count != 0)
			{
				ATM_OAM_TRACE("The Reomte ID =");
				for (j = 0; j < OAM_LB_LLID_SIZE; j++)
					ATM_OAM_TRACE("%x", pLBRXInfo[i].locationID[j]);
				ATM_OAM_TRACE(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
			}
		}
	}
}
#endif

#endif // ATM_OAM_TEST

#ifdef REMOTE_MANAGEMENT_ENABLE

#define CRC32(c,crc) (crc32tab[((size_t)(crc>>24) ^ (c)) & 0xff] ^ (((crc) << 8)))

static unsigned long  crc32tab[256]  =
{
 0x00000000L, 0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L,
 0x130476DCL, 0x17C56B6BL, 0x1A864DB2L, 0x1E475005L,
 0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L, 0x2B4BCB61L,
 0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
 0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L,
 0x5F15ADACL, 0x5BD4B01BL, 0x569796C2L, 0x52568B75L,
 0x6A1936C8L, 0x6ED82B7FL, 0x639B0DA6L, 0x675A1011L,
 0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL, 0x745E66CDL,
 0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
 0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L,
 0xBE2B5B58L, 0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L,
 0xAD2F2D84L, 0xA9EE3033L, 0xA4AD16EAL, 0xA06C0B5DL,
 0xD4326D90L, 0xD0F37027L, 0xDDB056FEL, 0xD9714B49L,
 0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
 0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L,
 0xE13EF6F4L, 0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL,
 0x34867077L, 0x30476DC0L, 0x3D044B19L, 0x39C556AEL,
 0x278206ABL, 0x23431B1CL, 0x2E003DC5L, 0x2AC12072L,
 0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
 0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL,
 0x7897AB07L, 0x7C56B6B0L, 0x71159069L, 0x75D48DDEL,
 0x6B93DDDBL, 0x6F52C06CL, 0x6211E6B5L, 0x66D0FB02L,
 0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L, 0x53DC6066L,
 0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
 0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL,
 0xBFA1B04BL, 0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L,
 0x8AAD2B2FL, 0x8E6C3698L, 0x832F1041L, 0x87EE0DF6L,
 0x99A95DF3L, 0x9D684044L, 0x902B669DL, 0x94EA7B2AL,
 0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
 0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L,
 0xC6BCF05FL, 0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L,
 0xD5B88683L, 0xD1799B34L, 0xDC3ABDEDL, 0xD8FBA05AL,
 0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L, 0x644FC637L,
 0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
 0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL,
 0x5C007B8AL, 0x58C1663DL, 0x558240E4L, 0x51435D53L,
 0x251D3B9EL, 0x21DC2629L, 0x2C9F00F0L, 0x285E1D47L,
 0x36194D42L, 0x32D850F5L, 0x3F9B762CL, 0x3B5A6B9BL,
 0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
 0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L,
 0xF12F560EL, 0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L,
 0xE22B20D2L, 0xE6EA3D65L, 0xEBA91BBCL, 0xEF68060BL,
 0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L, 0xDA649D6FL,
 0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
 0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L,
 0xAE3AFBA2L, 0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL,
 0x9B3660C6L, 0x9FF77D71L, 0x92B45BA8L, 0x9675461FL,
 0x8832161AL, 0x8CF30BADL, 0x81B02D74L, 0x857130C3L,
 0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
 0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL,
 0x7B827D21L, 0x7F436096L, 0x7200464FL, 0x76C15BF8L,
 0x68860BFDL, 0x6C47164AL, 0x61043093L, 0x65C52D24L,
 0x119B4BE9L, 0x155A565EL, 0x18197087L, 0x1CD86D30L,
 0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
 0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L,
 0x2497D08DL, 0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L,
 0xC5A92679L, 0xC1683BCEL, 0xCC2B1D17L, 0xC8EA00A0L,
 0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL, 0xDBEE767CL,
 0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
 0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L,
 0x89B8FD09L, 0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L,
 0x9ABC8BD5L, 0x9E7D9662L, 0x933EB0BBL, 0x97FFAD0CL,
 0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL, 0xA2F33668L,
 0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L 
};


__SWAP
static unsigned long calc_crc(char *mem, int len, unsigned initial)
{  
  unsigned crc;
  crc = initial;
      
  for(;len;mem++,len--)
  {   
    crc = CRC32(*mem, crc);
  }
  return(crc);
}     

#define crc32sar( crc, mem, len) calc_crc(mem, len, crc)

#define ATM_HDR_GFC_SHIFT       28
#define ATM_HDR_VPI_SHIFT       20
#define ATM_HDR_VCI_SHIFT       4
#define ATM_HDR_PTI_SHIFT       1

//Soft SAR for Remote Management Channel 0/16
//Encode AAL5 
__SWAP
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
	__NOMIPS16
#endif
struct sk_buff *sarlib_encode_aal5 (struct sk_buff *skb) 
{
  int length, pdu_length;
  unsigned char *trailer;
  unsigned char *pad;
  uint crc = 0xffffffff;

  //ATM_OAM_TRACE("sarlib_encode_aal5 (0x%08x) called\n", skb);

  /* determine aal5 length */
  pdu_length = skb->len;
  length = ((pdu_length + 8 + 47) / 48) * 48;
  
  if ( skb_tailroom(skb) < (length - pdu_length) ) {
    struct sk_buff *out;
    /* get new skb */
    out = dev_alloc_skb( SAR_TX_Buffer_Size );    
    if (!out) return NULL;

    //ATM_OAM_TRACE("out->data = 0x%08x\n", out->data);    
    //ATM_OAM_TRACE("sarlib_encode_aal5 pdu length %d, allocated length %d\n", skb->len, length);
    memcpy (out->data, skb->data, skb->len);
    skb_put(out, skb->len);
    dev_kfree_skb_irq(skb);
    skb = out;
    }

  //ATM_OAM_TRACE("skb->data = 0x%08x\n", skb->data);
  /* note end of pdu and add length */
  pad = skb_put(skb, length - pdu_length);
  trailer = skb->tail - 8;

  //ATM_OAM_TRACE("trailer = 0x%08x\n", trailer);

  /* zero padding space */
  memset(pad, 0, length - pdu_length-8);
  
  /* add trailer */
  *trailer++ = (unsigned char) 0; /* UU  = 0 */
  *trailer++ = (unsigned char) 0; /* CPI = 0 */
  *trailer++ = (unsigned char) (pdu_length >> 8);
  *trailer++ = (unsigned char) (pdu_length & 0xff);
  //xprintfk("CRC on %08xh, len %d\n",skb->data, length-4);
  crc = ~crc32sar(crc, skb->data, length-4);
  *trailer++ = (unsigned char) (crc >>  24);
  *trailer++ = (unsigned char) (crc >>  16);
  *trailer++ = (unsigned char) (crc >>   8);
  *trailer++ = (unsigned char) (crc & 0xff);
  
  //ATM_OAM_TRACE("sarlib_encode_aal5 return 0x%08x (length %d)\n", skb, skb->len);
  return skb;
}

#ifdef RM_TEST
#include <asm/checksum.h>
unsigned short gInUDPPort=0;
#endif
//Out Band Channel AAL5 Tx
//return 0: fail; 1:success
__SWAP
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
	__NOMIPS16
#endif
int OutBandAAL5Tx(unsigned short vpi, unsigned short vci, struct sk_buff *skb)
{
  struct sk_buff *aal5_skb;
  int i;
  char *pCell;
  unsigned char pti, gfc;
  unsigned int atmHeader;

    if (skb==NULL) {
        ATM_OAM_TRACE("Remote Management: NULL tx packet\n");
    	return 0;
    };
#ifdef RM_TEST
    {
        //add llc/mac/IP/UDP header
    	struct sk_buff *testskb;
        unsigned char *tail;
        unsigned short *ptr;
        char mac[14]={0x00, 0x50, 0xba, 0x0f, 0x8a, 0xf2, 
                       0x00, 0xe0, 0x4c, 0x86, 0x70, 0x01, 
                       0x08, 0x00};
        char iphdr[20]={0x45, 0x00, 
                         0x00, 0x00,	//IP data gram size
                         0xf7, 0x70, 
                         0x00, 0x00, 0x80, 0x11,
                         0x00, 0x00,	//IP checksum
                         0xc0, 0xa8, 0x01, 0x01,	//SIP
                         0xc0, 0xa8, 0x01, 0x96,	//DIP
                         };
        testskb = dev_alloc_skb( SAR_TX_Buffer_Size );
        tail=testskb->data;
        skb_put(testskb, 10);
    	memcpy(tail, FrameHeader_1483[0], 10);
    	tail+=10;
    	//MAC
        skb_put(testskb, 14);
    	memcpy(tail, mac, 14);
    	tail+=14;
    	//IP
        skb_put(testskb, sizeof(iphdr));
        *((unsigned short*)&iphdr[2])=20+8+skb->len;  //length
        *((unsigned short*)&iphdr[10])=ip_compute_csum(iphdr, 20);  //IP checksum
    	memcpy(tail, iphdr, sizeof(iphdr));
    	tail+=sizeof(iphdr);
    	//UDP
        skb_put(testskb, 8);
        ptr=(unsigned short *)tail;
        *(ptr++)=161;  //sport=161
        *(ptr++)=gInUDPPort;  //dport
        *(ptr++)=8+skb->len;  //length
        *ptr=0;
        tail+=8;
        //snmp
        skb_put(testskb, skb->len);
    	memcpy(tail, skb->data, skb->len);
        //*(ptr)=csum_tcpudp_magic();  //UDP checksum
        dev_kfree_skb_irq(skb);
        skb = testskb;
        vpi = 1;
        vci = 39;
        /*for (i=0;i<testskb->len;i++) {
            if ((i&0x0f)==0) printk("\n");
            printk("%02X ", testskb->data[i]);
        };
        printk("\n");
        */
    }
#endif

	if ((aal5_skb=sarlib_encode_aal5(skb))==NULL)
	    return 0;
	
	skb_oamaal5_debug(aal5_skb, debug_obaal5, 1);
	//segment & tx
	gfc=0;
	pti=0;
	//cell header
    atmHeader  = ((unsigned long)gfc << ATM_HDR_GFC_SHIFT) 
                  | ((unsigned long)vpi << ATM_HDR_VPI_SHIFT) 
                  | ((unsigned long)vci << ATM_HDR_VCI_SHIFT) 
                  | ((unsigned long)pti << ATM_HDR_PTI_SHIFT) ;
//Enable_LoopBack(sar_dev);                  
	for (i=0;i<(aal5_skb->len/48);i++) {
		pCell=kmalloc(OAM_CELL_SIZE, GFP_KERNEL);
		if (pCell==NULL) return 0;
		
		if (i==(aal5_skb->len/48)-1)  //set pti bit in last cell
		    atmHeader|=0x2;
		memcpy(&pCell[0], &atmHeader, 4);
		//data
		memcpy(&pCell[4], (aal5_skb->data+i*48), 48);
		
    //printk("AAL5 TX %d/%d:\n", vpi, vci);
    //Dump(pCell, 52);

	    if (OAMFSendACell(pCell, 0)==0) {
		    //send fail
	        return 0;
	    };
	};
    dev_kfree_skb_irq(aal5_skb);
//Disable_LoopBack(sar_dev);                  
	
	return 1;
}

//Out Band Channel AAL5 Rx
//remember to add 10 bytes for llc header when skb is created
//return 0: fail, crc or size mismatch, drop the skb; 
//       1: current assemble success; 
//       2: full packet assemble success
struct sk_buff *outband_rx_skb=NULL;
__SWAP
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_ATM
__NOMIPS16
#endif
int OutBandAAL5Rx(struct atm_vcc *atm_vcc, char *rx_bfr)
{
  unsigned int atmHeader;
  int length;
  uint crc = 0xffffffff;
  unsigned char *bfr=rx_bfr, *tail;
  unsigned int upushAddr;

    if (bfr==NULL) {
        ATM_OAM_TRACE("Remote Management: NULL rx packet buffer\n");
        goto drop_rx_bfr;
    };
    
    if (outband_rx_skb==NULL) {
        outband_rx_skb = dev_alloc_skb( SAR_RX_Buffer_Size );	//create a new skb for next packet
        if (outband_rx_skb==NULL) {
            ATM_OAM_TRACE("Remote Management: Allocate rx skb fail\n");
            goto drop_rx_bfr;
    	};
    };
    
    
    //printk("AAL5 RX:\n");
    //Dump(rx_bfr, 52);
    
	//cell header
	memcpy(&atmHeader, bfr, 4);
	bfr+=4;  //skip cell header
    
	//Assemble cell
	if ((outband_rx_skb->len+48)>SAR_RX_Buffer_Size) {  //size too large
        ATM_OAM_TRACE("Remote Management: packet size error %d\n", (outband_rx_skb->len+48));
        goto drop_rx_bfr;
	};
	//Assemble cell
    tail=skb_put(outband_rx_skb, 48);
	memcpy(tail, bfr, 48);
	if (!(atmHeader&2)) { //normal cell
        //kfree(rx_bfr);
	    return 1;
	};
	//last cell
    crc = crc32sar(crc, outband_rx_skb->data, outband_rx_skb->len)^0xc704dd7b;
    if (crc) {  //crc error
        ATM_OAM_TRACE("Remote Management: packet CRC error 0x%08x\n", crc);
        goto drop_rx_bfr;
	};

	//remove AAL5 trailer
	length = ((bfr[42]&0x00ff)<<8)|(bfr[43]&0x00ff);
	skb_trim(outband_rx_skb, length);
#ifdef RM_TEST
    if (*((unsigned short*)&outband_rx_skb->data[0x2e])==161) {
        gInUDPPort = *((unsigned short*)&outband_rx_skb->data[0x2c]);
    } else {  //drop non SNMP packet
        goto drop_rx_bfr;
    };    	
    //remove llc/MAC/IP/UDP header
    skb_pull(outband_rx_skb, 52);
#endif
	skb_oamaal5_debug(outband_rx_skb, debug_obaal5, 0);
	// end of our responsability
	upushAddr = (unsigned int)atm_vcc->push;
	if (((0xff000000 & upushAddr)== 0x80000000) && !(upushAddr & 0x03))
		atm_vcc->push(atm_vcc, outband_rx_skb);
	else {
		printk("fatal error occur:%s %d atmvcc->push:0x%x\n", __FILE__, __LINE__, upushAddr);
		goto drop_rx_bfr;
	}
	
	outband_rx_skb = NULL;	//create a new skb for next packet
	//ATM_OAM_TRACE("Remote Management: get rx packet\n");
	//kfree(rx_bfr);
    
	return 2;

drop_rx_bfr:	
    if (outband_rx_skb) {
        dev_kfree_skb_irq(outband_rx_skb);
        outband_rx_skb = NULL;
    };
    //if (rx_bfr) 
    //    kfree(rx_bfr);
    return 0;
}
#endif  //REMOTE_MANAGEMENT_ENABLE
//Soft SAR for Remote Management Channel 0/16

#endif // ATM_OAM

int get_wlan_member(void);
void set_wlan_vid(int vid);

void pp_ifgroup_setup()
{
	extern int rtl8672_l2flush(void);
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
#ifdef CONFIG_EXT_SWITCH
	sar_private *cp = sar_dev;
	
	if(enable_port_mapping){
		int ch_no,k;
		extern struct r8305_struc	rtl8305_info;
		extern void set_intf_vid(int ch,int vlanid);
		for(ch_no=0;ch_no<total_pvc_number;ch_no++){
			struct atm_vcc * atm_vcc = cp->vcc[ch_no].dev_data;
			for (k=0; k<SW_PORT_NUM; k++) {
				if (atm_vcc->ifgrp.member == rtl8305_info.vlan[k].member) {
					set_intf_vid(ch_no,rtl8305_info.vlan[k].vid);
					break;
				}
			}
			for (k=0; k<SW_PORT_NUM; k++) {
				if (get_wlan_member() == rtl8305_info.vlan[k].member){
					set_intf_vid(15,rtl8305_info.vlan[k].vid);
					set_wlan_vid(rtl8305_info.vlan[k].vid);
					break;
				}
			}
		}
	}
#endif
	rtl8672_l2flush();
#endif
}
