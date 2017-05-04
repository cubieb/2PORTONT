/* ra867x_pp.c - RealTek rtl8672 sar API for Packet Processor */

#include "ra8670.h"
#include "ra867x_pp.h"
#include "../net/packet_processor/rtl8672PacketProcessor.h"

extern struct sar_pTx *user_sar_ptx_idx(int intf,int ring);
extern struct sar_pRx *user_sar_prx_idx(int intf,int ring);
extern struct sar_vTx *user_sar_vtx_idx(int intf,int ring);
extern struct sar_vRx *user_sar_vrx_idx(int intf,int ring);
extern int cpu_sar_vrx_idx[8];
extern int cpu_sar_vtx_idx[8];
extern int rtl8672_flushAsicL4Entry(void);
#include "../net/packet_processor/icTest_ringController.h"
#define SPTXDESC0		0xb8621300
#define SVTXDESC0		0xb8621100
#define SPRXDESC0		0xb8621200
#define SVRXDESC0		0xb8621000

int pp_enabled;	// Packet Processor enable

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
extern sar_private	*sar_dev;
unsigned char get_sar_ch(struct atm_vcc *atmvcc){
	sar_atm_vcc *vcc_dev = RTATM_VCC(atmvcc);
	if((sar_private *)RTATM_DEV(atmvcc->dev) == sar_dev)
	return vcc_dev->ch_no;
	else
		return -1;
}

//tylo, for 8672 packet processor sar test
void ppsar_rx(struct sk_buff* skb,int port){
	sar_private *cp=sar_dev;
	struct atm_vcc * atm_vcc = cp->vcc[port].dev_data;
	
	sar_rx_sw(atm_vcc, skb);
	atm_vcc->push (atm_vcc, skb);
}
#endif

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
extern unsigned char getADSLLinkStatus(void);
#endif
int pp_sar_send(struct atm_vcc *vcc, struct sk_buff* skb, uint32 len, int port)
{
#if defined(CONFIG_RTL867X_PACKET_PROCESSOR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	int ret;
	sar_atm_vcc *vcc_dev = RTATM_VCC(vcc);
	
	atomic_sub(skb->truesize, &sk_atm(vcc)->sk_wmem_alloc);

	if(getADSLLinkStatus()==0){
		dev_kfree_skb(skb);
		return 0;
	}

	ret = SARvtx(vcc, skb,skb->len,vcc_dev->ch_no);
	return ret;
#else
	return 0;
#endif
}

void pp_set_vc_type(int num)
{
#if defined(CONFIG_RTL867X_PACKET_PROCESSOR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	int ch_no=0;
	for(ch_no=0;ch_no<num;ch_no++)
		set_sar_intf(ch_no, pvc_mode[ch_no],pvc_encap[ch_no]);
#endif
}

void pp_init_sar_desc(struct rtl_sar_private *cp, int vcnum)
{
#if defined(CONFIG_RTL867X_PACKET_PROCESSOR) && !defined(CONFIG_RLE0437) && !defined(CONFIG_RTL8681)
	int i;
	//tylo, for packet processor test
	//disable normal channel interrupt 0~7
	//*(volatile unsigned int *)0xb8302000 &= 0xffffff83;
	//set SAR tx/rx FDP
	//init_ring();
	for(i=0;i<vcnum;i++) {
		int j;
		//tylo, for 8672 packet processor
		//cpu_sar_vtx_idx[i]=0;
		
		cpu_sar_vtx_idx[i]=0;
		printk("sarVTx:%x   sarPTx:%x\n",(uint32)user_sar_vtx_idx(i,0),(uint32)user_sar_ptx_idx(i,0));
		printk("sarVRx:%x   sarPRx:%x\n",(uint32)user_sar_vrx_idx(i,0),(uint32)user_sar_prx_idx(i,0));
		for(j=0;j<SAR_PTX_RING_SIZE;j++){
			struct sar_pTx *sarTx;			
			sarTx=user_sar_ptx_idx(i,j);
			
			sarTx->own=0;
			if(j==SAR_PTX_RING_SIZE-1) {
				sarTx->eor=1;
			}
			else{
				sarTx->eor=0;
			}
			
			sarTx->tx_buffer_addr=(uint32)NULL;
			sarTx->orgAddr=(uint32)NULL;	
			
		}

		for(j=0;j<SAR_VTX_RING_SIZE;j++){
			struct sar_vTx *sarvTx;			
			sarvTx=user_sar_vtx_idx(i,j);

			sarvTx->own=0;
			if(j==SAR_VTX_RING_SIZE-1) {
				sarvTx->eor=1;
			}
			else{
				sarvTx->eor=0;
			}					
		}
		
		for(j=0;j<SAR_PRX_RING_SIZE;j++){
			struct sar_pRx *sarRx;
			sarRx=user_sar_prx_idx(i,j);					
			sarRx->own=1;
			if(j==SAR_PRX_RING_SIZE-1) {
				sarRx->eor=1;
			}
			else{
				sarRx->eor=0;
			}			
		}


		for(j=0;j<SAR_VRX_RING_SIZE;j++){
			struct sar_vRx *sarvRx;
			sarvRx=user_sar_vrx_idx(i,j);
			sarvRx->own=1;
			if(j==SAR_VRX_RING_SIZE-1) {
				sarvRx->eor=1;			
			}
			else{
				sarvRx->eor=0;
			}
		}

		cpu_sar_vrx_idx[i]=0;
		reg(cp->vcc[i].TV.FDP_Addr)	=((uint32)user_sar_ptx_idx(i,0)|0xa0000000);
		reg(cp->vcc[i].RV.FDP_Addr)	=((uint32)user_sar_prx_idx(i,0)|0xa0000000);
		
		reg(SPTXDESC0+i*4)=(uint32)user_sar_ptx_idx(i,0);
		reg(SVTXDESC0+i*4)=(uint32)user_sar_vtx_idx(i,0);
		reg(SPRXDESC0+i*4)=(uint32)user_sar_prx_idx(i,0);
		reg(SVRXDESC0+i*4)=(uint32)user_sar_vrx_idx(i,0);
		
			//set SAR interface encap. here
			set_sar_intf(i, pvc_mode[i],pvc_encap[i]);
	}
	//set SRAM base address
	*(volatile unsigned int*) 0xb830202c = *(volatile unsigned int*) 0xb8620004;
	rtl8672_flushAsicL4Entry();
#endif
}

/*jiunming*/
#if defined(CONFIG_RTL867X_IPTABLES_FAST_PATH) || defined(CONFIG_RTL867X_PACKET_PROCESSOR)
extern void fastpath_notify(int event);
#endif

void DSPEnterShowtime( void )
{
	printk( "Enter %s\n", __FUNCTION__ );
#ifdef CONFIG_DSL_CODESWAP

#if defined(CONFIG_RTL867X_IPTABLES_FAST_PATH) || defined(CONFIG_RTL867X_PACKET_PROCESSOR)
	fastpath_notify( 1 ); //EVT_SAR_UP
#endif	//CONFIG_RTL867X_IPTABLES_FAST_PATH

	DSPInShowtime=1;	
#endif	//CONFIG_DSL_CODESWAP
	printk( "Exit %s\n", __FUNCTION__ );
}

void DSPExitShowtime( void )
{
	printk( "Enter %s\n", __FUNCTION__ );
#ifdef CONFIG_DSL_CODESWAP
	DSPInShowtime=0;

#if defined(CONFIG_RTL867X_IPTABLES_FAST_PATH) || defined(CONFIG_RTL867X_PACKET_PROCESSOR)
	fastpath_notify( 2 ); //EVT_SAR_DOWN
#endif	//CONFIG_RTL867X_IPTABLES_FAST_PATH

#endif
	printk( "Exit %s\n", __FUNCTION__ );
}

//return value: offset of the mac addr., 0: do not have mac
unsigned char PVC_mac_offset(int ch_no){
	if((pvc_mode[ch_no] == PPPoA_mode) || (pvc_mode[ch_no] == Routed_mode))
		return 0;
	if(pvc_encap[ch_no] == L2Encap_LLC)
		return 10;
	else
		return 2;
}

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
//for ipqos
struct net_device *get_sar_netdev(struct atm_vcc *vcc, int port){
	sar_private *cp = RTATM_DEV(vcc->dev);
	return cp->vcc[port].dev;
}
#endif
