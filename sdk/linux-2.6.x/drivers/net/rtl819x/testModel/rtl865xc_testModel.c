/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source File for IC-specific Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: rtl865xc_testModel.c,v 1.3 2012/10/24 04:31:12 ikevin362 Exp $
*/

#include <linux/delay.h>
#include "rtl865xc_testModel.h"
#include <net/rtl/rtl867x_hwnat_api.h>



/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *  Shared Packet Memory Space for test cases.
 */
uint8 RTL865xC_Test_SharedPkt[RTL865xC_TEST_SHARE_PKT_NUM][RTL865xC_TEST_SHARE_PKT_LEN];

static void rtl865xC_swDriver_init(void)
{
	/* IPQoS */	
	#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	rtl8676_IPQos_Disable();
	#endif

	/* acl */
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER_ACL) 
	rtl865x_acl_control_L2_permit_clean();	
	rtl865x_acl_control_L34_permit_clean();
	rtl865x_acl_control_L34_redirect_clean();
	rtl865x_acl_control_set_mode(RTL865X_ACL_Mode_Normal);
	#else
	rtl865x_reinit_acl();
	#endif	

	/*event management */
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_reInitEventMgr();
	#endif

	/*l4*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L4
	rtl865x_nat_reinit();
	#endif
	
	/*l3*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	rtl865x_reinitMulticast();
	rtl865x_reinitRouteTable();
	rtl865x_reinitNxtHopTable();	
	rtl865x_reinitPppTable();	
	rtl865x_arp_reinit();	
	rtl865x_reinitIpTable();	
	#endif

	/*l2*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
	#endif	

	/*common*/	
	rtl865x_reinitNetifTable();	
	rtl865x_reinitVlantable();	
}


/*
	reinit sw driver ->    reinit hw  ->  reinit sw driver
	
     Note. when deleting sw entry, it would check whether the hw entry existes.
	 		Hence, we cannot reset hw until clear sw table .
			
			However, we have to reinit sw driver agina after hw reinit.
			sw. will init some reg. (ex. Mulitacst MTU)
*/
int32 rtl865xC_virtualMacInit(void)
{
	/* In pure model code mode (either USER or MODEL), we configure to MII-Like mode. */	
	
	enum PORTID port;
	int32 needDelay = 0;
	int i;

	/*re-init sequence eventmgr->l4->l3->l2->common is to make sure delete asic entry,
	if not following this sequence, 
	some asic entry can't be deleted due to reference count is not zero*/

	/* step1. reset sw : Use driver-layer API   */
	rtl865xC_swDriver_init();

	/* step2. reset switch and PCS  :  REG'h1800_030C_Bit[3]*/
	WRITE_MEM32(0xb800030c, READ_MEM32(0xb800030c)&(~0x8));
	mdelay(10);
	WRITE_MEM32(0xb800030c, READ_MEM32(0xb800030c)|0x8);
	mdelay(10);

	/* step3. Enable test mode (TX/RX port0~port5)  */
	WRITE_MEM32( TMCR, READ_MEM32(TMCR)|(0x3f<<RX_TEST_PORT_OFFSET));
	WRITE_MEM32( TMCR, READ_MEM32(TMCR)|(0x3f<<TX_TEST_PORT_OFFSET));

	/* step4. We set every port in 1000Mbps force mode. */
	for( port = PHY0; port<=RTL8651_MAC_NUMBER; port++ )
	{
		uint32 config;
		config = READ_MEM32( PCRP0+port*4 );
		if ( (config&EnForceMode)==1 )
		{
			/* already configured, do nothing for speed up */
		}
		else
		{	/* Not configured, this is the first time system initialization. */
	
			config &= (BYPASS_TCRC|MIIcfg_CRS|MIIcfg_COL|MIIcfg_RXER|GMIIcfg_CRS|BCSC_Types_MASK|
			           EnLoopBack|PauseFlowControl_MASK|DisBKP|STP_PortST_MASK|AcptMaxLen_MASK); /* keep value */
			
			config |= ((port<<ExtPHYID_OFFSET)|EnForceMode|ForceLink|ForceSpeed100M|ForceDuplex|
						PauseFlowControlEtxErx|EnablePHYIf|MacSwReset);			

			WRITE_MEM32( PCRP0+port*4, config );
			needDelay++;
		}
	}
	
	if ( needDelay > 0 )
	{ /* delay 1/100 sec for link REALLY up (only IC need delay) */
		mdelay(10);		
	}


	/* step5. prepare share packet buffer for test mode */
	for(i=0;i<RTL865xC_TEST_SHARE_PKT_NUM;i++)
		memset( RTL865xC_Test_SharedPkt[i], 0, RTL865xC_TEST_SHARE_PKT_LEN*sizeof(*RTL865xC_Test_SharedPkt[i]) );
	

	/* step6. start hw. */
	REG32(SIRR) = 1;	

	/* step7. reset sw : Use driver-layer API   */
	rtl865xC_swDriver_init();

	return SUCCESS;
}


/*
 *  Send the packet to the Mii-like port to simulate a received packet from physical port to SWCORE.
 *    len - included L2 CRC
 */
int32 rtl865xC_virtualMacInput( enum PORTID fromPort, uint8* packet, int32 len )
{
	int32 i;
	uint32 regRx;
	uint32 rxEnMask;
	uint32 byteOffset;

	/* GMII, support JUMBO frame */
	if ( len > (16*1024-1) )
	{
		rtlglue_printf("Leave %s @ %d  : Packet is larger than JUMBO frame (len=%d).\n", __FUNCTION__, __LINE__, len );
		return FAILED;
	}	

	if( packet ==NULL )
	{
		rtlglue_printf("Leave %s @ %d  : Ppacket ==NULL\n", __FUNCTION__, __LINE__ );
		return FAILED;
	}	
	

	/*************************************************************
	 *  MiiRx (CPU sends packet to ASIC)
	 *************************************************************/	



	/*------------------------------------------------------------
	 * 0. Prepare variables
	 */
	regRx = MIITM_RXR0+(fromPort/3)*4;
	rxEnMask = 1<<(P0RxEN_OFFSET+(fromPort%3));
	byteOffset = (fromPort%3)*8+P0RXD_OFFSET;
	

	/*------------------------------------------------------------
	 * 1. Send preamble (8 bytes, 5555 5555 5555 55d5)
	 */
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0x55<<byteOffset));
	WRITE_MEM32( regRx, rxEnMask|(0xd5<<byteOffset));
	
	/*------------------------------------------------------------
	 * 2. Send packet content
	 */
	for( i = 0; i<len; i++ )
	{
		WRITE_MEM32( regRx, rxEnMask|(packet[i]<<byteOffset) );
	}

	/*------------------------------------------------------------
	 * 3. Send tailing octets (12 Bytes)
	 */
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));
	WRITE_MEM32( regRx, (0x00<<byteOffset));

	return SUCCESS;
}


/*
 *  Receive the packet from the Mii-like port to simulate a sending packet from swcore to physical port.
 */
int32 rtl865xC_virtualMacOutput( uint32 *toPort, uint8* packet, int32 *len )
{
	/*************************************************************
	 *  MiiTx (ASIC sends packet to CPU)
	 *************************************************************/
	uint32 data;
	uint32 Tx_ready_port = 0xffffffff;
	uint32 retlen = 0;
	uint32 regTx;
	uint32 txEnMask;
	uint32 byteOffset;
	uint32 max_try = 512; /* Try times, I hope long enough for FPGA. (unit:byte) */

	
	uint32 port;
	int port_check=-1;



	/*  we only check one port once  */
	for( port = PN_PORT0; port <= PN_PORT5; port++ )
	{
		if ( (*toPort)&(1<<port) )
		{
			if(port_check==-1)
			{
				port_check = port;
			}
			else
			{
				printk("(%s %d) sorry, we only check one port once \n",__func__,__LINE__);
				goto failed;
			}
		}
	}

	if(port_check==-1)
	{
		printk("(%s %d) no ckeck port ?  \n",__func__,__LINE__);
		goto failed;
	}


	/*------------------------------------------------------------
	 * 0. Prepare variables
	 */
	retlen = 0;

	/*------------------------------------------------------------
	 * 1. Polling until Tx packet ready
	 *------------------------------------------------------------*/	
	#if 0 /* Since designer does not implement these status bits, we don't check them. */
	
	while( 1 )
	{
		data = READ_MEM32( TMCR );
		if ( (data>>MiiTxPktRDY_OFFSET)&*toPort )
			break;
			
		if ( (try--)== 0 ) goto failed;
	}
	#endif	
	

	/* Check if any port has valid data. */
	Tx_ready_port = (READ_MEM32(TMCR) & MiiTxPktRDY_MASK)>>MiiTxPktRDY_OFFSET;
	if( (1<<port_check) & Tx_ready_port )
	{
		*toPort = Tx_ready_port;
		goto preamble_found;
	}
	else
	{
		printk("(%s %d)  The port (your input:%d)  has no data  (the ports has data: 0x%X)\n",__func__,__LINE__,port_check,Tx_ready_port);
		goto failed;
	}
	
preamble_found:

	regTx = MIITM_TXR0+(port_check/3)*4;
	txEnMask = 1<<(P0TxEN_OFFSET+(port_check%3));
	byteOffset = (port_check%3)*8+P0TXD_OFFSET;
	//printk("(%s %d)  max_try=%d  *toPort=%u\n",__func__,__LINE__,max_try,*toPort);


	
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );


	/*------------------------------------------------------------
	 * 2. Polling until preamble found (~8Bytes)
	 *------------------------------------------------------------*/	

	/* find preample '55'. */
	do
	{
		if (((data>>byteOffset)&0xff)==0x55)
			break;
		WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
		data = READ_MEM32( regTx ); /* read next byte */
		if ( (max_try--)== 0 ) 
		{		
			goto failed;			
		}
	} while (1);


	
	/* find preample 'd5'. */
	do
	{
		if (((data>>byteOffset)&0xff)==0xd5)
			break;
		WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
		data = READ_MEM32( regTx ); /* read next byte */
		if ( (max_try--)== 0 ) 
		{		
			goto failed;			
		}
	} while (1);


	/*------------------------------------------------------------
	 * 3. Read packet (until packet ends)
	 *------------------------------------------------------------*/
	while( 1 )
	{
		WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
		mdelay(1);
		data = READ_MEM32( regTx ); /* read next byte */
		if ( (data&txEnMask) == 0 ) /* no data valid for P0_TXDV~P5_TXDV */
			break;
		
		packet[retlen] = data>>byteOffset;
		
		retlen++;
	}

	/*------------------------------------------------------------
	 * 4. Read tailing data (12 Bytes) 
	 *------------------------------------------------------------*/
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );
	WRITE_MEM32( regTx, txEnMask ); /* give Mii-Tx a clock */
	data = READ_MEM32( regTx );

	*len = retlen;
	return SUCCESS;

failed:
	*len = 0;
	*toPort = 0;
	return FAILED;
}



int32 rtl865xC_convertHsbToAsic( hsb_param_t* hsb, hsb_t* rawHsb )
{
	/* bit-to-bit mapping */
	rawHsb->spa = hsb->spa;
	rawHsb->trigpkt = hsb->trigpkt;
	rawHsb->resv = 0;
	rawHsb->len = hsb->len;
	rawHsb->vid = hsb->vid;
	rawHsb->tagif = hsb->tagif;
	rawHsb->pppoeif = hsb->pppoeif;
	rawHsb->sip29_0 = hsb->sip&0x3fffffff;
	rawHsb->sip31_30 = hsb->sip>>30;
	rawHsb->sprt = hsb->sprt;
	rawHsb->dip13_0 = hsb->dip&0x3fff;
	rawHsb->dip31_14 = hsb->dip>>14;
	rawHsb->dprt13_0 = hsb->dprt&0x3fff;
	rawHsb->dprt15_14 = hsb->dprt>>14;
	rawHsb->ipptl = hsb->ipptl;
	rawHsb->ipfg = hsb->ipfg;
	rawHsb->iptos = hsb->iptos;
	rawHsb->tcpfg = hsb->tcpfg;
	rawHsb->type = hsb->type;
	rawHsb->patmatch = hsb->patmatch;
	rawHsb->ethtype = hsb->ethtype;
#if 1 /* Since the endian is reversed, we must translate it. */
	rawHsb->da14_0 = hsb->da[5]|(hsb->da[4]<<8);
	rawHsb->da46_15 = (hsb->da[4]>>7)|(hsb->da[3]<<1)|(hsb->da[2]<<9)|(hsb->da[1]<<17)|(hsb->da[0]<<25);
	rawHsb->da47_47 = hsb->da[0]>>7;
	rawHsb->sa30_0 = hsb->sa[5]|(hsb->sa[4]<<8)|(hsb->sa[3]<<16)|(hsb->sa[2]<<24);
	rawHsb->sa47_31 = (hsb->sa[2]>>7)|(hsb->sa[1]<<1)|(hsb->sa[0]<<9);
#else
	rawHsb->da14_0 = hsb->da[0]|(hsb->da[1]<<8);
	rawHsb->da46_15 = (hsb->da[1]>>7)|(hsb->da[2]<<1)|(hsb->da[3]<<9)|(hsb->da[4]<<17)|(hsb->da[5]<<25);
	rawHsb->da47_47 = hsb->da[5]>>7;
	rawHsb->sa30_0 = hsb->sa[0]|(hsb->sa[1]<<8)|(hsb->sa[2]<<16)|(hsb->sa[3]<<24);
	rawHsb->sa47_31 = (hsb->sa[3]>>7)|(hsb->sa[4]<<1)|(hsb->sa[5]<<9);
#endif
	rawHsb->hiprior = hsb->hiprior;
	rawHsb->snap = hsb->snap;
	rawHsb->udpnocs = hsb->udpnocs;
	rawHsb->ttlst = hsb->ttlst;
	rawHsb->dirtx = hsb->dirtx;
	rawHsb->l3csok = hsb->l3csok;
	rawHsb->l4csok = hsb->l4csok;
	rawHsb->ipfo0_n = hsb->ipfo0_n;
	rawHsb->llcothr = hsb->llcothr;
	rawHsb->urlmch = hsb->urlmch;
	rawHsb->extspa = hsb->extspa;
	rawHsb->extl2 = hsb->extl2;
	rawHsb->linkid = hsb->linkid;
	rawHsb->pppoeid = hsb->pppoeid;
	return SUCCESS;
}


int32 rtl865xC_virtualMacSetHsb( hsb_t rawHsb )
{
	int32 retval = SUCCESS;	
	uint32 *pSrc, *pDst;
	uint32 i;

	WRITE_MEM32( TMCR, READ_MEM32( TMCR) | ENHSBTESTMODE ); /* Before reading HSABUSY, we must enable test mode. */


	/* We must assert structure size is the times of 4-bytes. */
	if ( (sizeof(rawHsb)%4) != 0 ) 
		printk( "sizeof(rawHsb) is not the times of 4-bytes." );

	pSrc = (uint32*)&rawHsb;
	pDst = (uint32*)HSB_BASE;
	for( i = 0; i < sizeof(rawHsb); i+=4 )
	{
		WRITE_MEM32((uint32)pDst, (*pSrc));
		pSrc++;
		pDst++;
	}		

	WRITE_MEM32( TMCR, READ_MEM32( TMCR ) & ~HSB_RDY );
	WRITE_MEM32( TMCR, READ_MEM32( TMCR ) | HSB_RDY ); /* Once HSB_RDY is set, FPGA and model code start ALE. */

	mdelay( 100 ); /* delay for ALE to process. */		
	while( (READ_MEM32(TMCR)&HSABUSY) == HSABUSY ); /* Wait until HSB has became HAS. */
	
	
	return retval;
}



int32 rtl865xC_convertHsaToAsic( hsa_param_t* hsa, hsa_t* rawHsa )
{
	/* bit-to-bit mapping */
#if 1 /* Since the endian is reversed, we must translate it. */
	rawHsa->nhmac0 = hsa->nhmac[5];
	rawHsa->nhmac1 = hsa->nhmac[4];
	rawHsa->nhmac2 = hsa->nhmac[3];
	rawHsa->nhmac3 = hsa->nhmac[2];
	rawHsa->nhmac4 = hsa->nhmac[1];
	rawHsa->nhmac5 = hsa->nhmac[0];
#else
	rawHsa->nhmac0 = hsa->nhmac[0];
	rawHsa->nhmac1 = hsa->nhmac[1];
	rawHsa->nhmac2 = hsa->nhmac[2];
	rawHsa->nhmac3 = hsa->nhmac[3];
	rawHsa->nhmac4 = hsa->nhmac[4];
	rawHsa->nhmac5 = hsa->nhmac[5];
#endif
	rawHsa->trip15_0 = hsa->trip&0xffff;
	rawHsa->trip31_16 = hsa->trip>>16;
	rawHsa->port = hsa->port;
	rawHsa->l3csdt = hsa->l3csdt;
	rawHsa->l4csdt = hsa->l4csdt;
	rawHsa->egif = hsa->egif;
	rawHsa->l2tr = hsa->l2tr;
	rawHsa->l34tr = hsa->l34tr;
	rawHsa->dirtxo = hsa->dirtxo;
	rawHsa->typeo = hsa->typeo;
	rawHsa->snapo = hsa->snapo;
	rawHsa->rxtag = hsa->rxtag;
	rawHsa->dvid = hsa->dvid;
	rawHsa->pppoeifo = hsa->pppoeifo;
	rawHsa->pppidx = hsa->pppidx;
	rawHsa->leno5_0 = hsa->leno&0x3f;
	rawHsa->leno14_6 = hsa->leno>>6;
	rawHsa->l3csoko = hsa->l3csoko;
	rawHsa->l4csoko = hsa->l4csoko;
	rawHsa->frag = hsa->frag;
	rawHsa->lastfrag = hsa->lastfrag;
	rawHsa->ipmcastr = hsa->ipmcastr;
	rawHsa->svid = hsa->svid;
	rawHsa->fragpkt = hsa->fragpkt;
	rawHsa->ttl_1if4_0 = hsa->ttl_1if&0x1f;
	rawHsa->ttl_1if5_5 = hsa->ttl_1if>>5;
	rawHsa->ttl_1if8_6 = hsa->ttl_1if>>6;
	rawHsa->dpc = hsa->dpc;
	rawHsa->spao = hsa->spao;
	rawHsa->hwfwrd = hsa->hwfwrd;
	rawHsa->dpext = hsa->dpext;
	rawHsa->spaext = hsa->spaext;
	rawHsa->why2cpu13_0 = hsa->why2cpu&0x3fff;
	rawHsa->why2cpu15_14 = hsa->why2cpu>>14;
	rawHsa->spcp = hsa->spcp;
	rawHsa->dvtag = hsa->dvtag;
	rawHsa->difid = hsa->difid;
	rawHsa->linkid = hsa->linkid;
	rawHsa->siptos = hsa->siptos;
	rawHsa->dp6_0 = hsa->dp;
	rawHsa->priority = hsa->priority;
	return SUCCESS;
}





