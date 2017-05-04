/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source File of Model Code for RTL8672
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icTest.c,v 1.1.1.1 2010/05/05 09:00:49 jiunming Exp $
*/

#include "rtl_types.h"
#include "rtl_glue.h"
#include "cle_struct.h"
#include "types.h"
//#include <rtl8672_sar.h>
#include "rtl8672_asicregs.h"
#include "rtl8672_Proto.h"
#include "hsModel.h"
#include "icModel.h"
#include "icTest.h"
#include "modelRandom.h"
#include "virtualMac.h"
#include "icModel_ALE.h"
#include "icModel_Packet.h"
#include "icModel_pktParser.h"
#include "icModel_pktTranslator.h"
#include "icModel_Packet.h"
#include "rtl_utils.h"
#include "icTest_ALE.h"
#include "icTest_PktParser.h"
#include "icTest_PktTranslator.h"
#include "icTest_ringController.h"
#include "rtl8672_tblDrv.h"

#define DRVTEST_CASENO_LOWERBOUND	100
#define DRVTEST_CASENO_UPPERBOUND	10000

struct MODEL_TEST_CASE_S
{	
	int32 no;
	char* name;
	int32 (*fp)(uint32);
	uint32 group;
};
typedef struct MODEL_TEST_CASE_S MODEL_TEST_CASE_T;


#define MODEL_TEST_CASE( case_no, func, group_mask ) \
	{ \
		no: case_no, \
		name: #func, \
		fp: func, \
		group: group_mask, \
	}


#define DRV_TEST_CASE( case_no, func, group_mask ) \
	{ \
		no: case_no, \
		name: #func, \
		fp: func, \
		group: group_mask, \
	}


/*
 * To make source code more clear, we define this macro.
 * This is for modelCompHsb() and modelCompHsa() functions.
 */
#define CHECK_EQUAL( p1, p2, field, retvar, retval ) \
	do { \
		if ( p1->field != p2->field ) \
		{ \
			rtlglue_printf( "%s(): '%s' NOT equal( %d , %d ).\n", __FUNCTION__, #field, p1->field, p2->field ); \
			retvar = retval; \
		} \
	} while(0);

int32 dumpPkt(struct rtl_pkt *pPkt)
{
	/* dump packet */
#if 1
	int i = 0;
	if(pPkt){
		for(i=0; i<pPkt->length; i++){
			if((i%10)==0) rtlglue_printf("     ");
			if((i%20)==0) rtlglue_printf("\n");
			rtlglue_printf("  0x%02x",(pPkt->content[i]));
		}
		rtlglue_printf("\n=============================================================");
		rtlglue_printf("      =============================================================\n");
	}
#endif
	return SUCCESS;
}

int32 compHsb( hsb_param_t* hsb1, hsb_param_t* hsb2 )
{
	int ret=SUCCESS;
	if(hsb1->fromcpu!=hsb2->fromcpu)		
		{rtlglue_printf("hsb1->fromcpu=0x%x; hsb2->fromcpu=0x%x;\n",hsb1->fromcpu,hsb2->fromcpu);  ret=FAILED;}
	if(hsb1->hasmac!=hsb2->hasmac)
		{rtlglue_printf("hsb1->hasmac=0x%x; hsb2->hasmac=0x%x;\n",hsb1->hasmac,hsb2->hasmac);  ret=FAILED;}
	if(hsb1->tif!=hsb2->tif)	
		{rtlglue_printf("hsb1->tif=%d; hsb2->tif=%d;\n",hsb1->tif,hsb2->tif);  ret=FAILED;}	
	if(hsb1->pif!=hsb2->pif)	
		{rtlglue_printf("hsb1->pif=0x%x; hsb2->pif=0x%x;\n",hsb1->pif,hsb2->pif);  ret=FAILED;}
	if(hsb1->srcportidx!=hsb2->srcportidx)	
		{rtlglue_printf("hsb1->srcportidx=0x%x; hsb2->srcportidx=0x%x;\n",hsb1->srcportidx,hsb2->srcportidx);  ret=FAILED;}
	if(hsb1->l3hwfwdip != hsb2->l3hwfwdip)
		{rtlglue_printf("hsab->l3hwfwdip=0x%x; hsb2->l3hwfwdip=0x%x;\n",hsb1->l3hwfwdip,hsb2->l3hwfwdip); ret = FAILED;}
	if(hsb1->linkid!=hsb2->linkid)	
		{rtlglue_printf("hsb1->linkid=0x%x; hsb2->linkid=0x%x;\n",hsb1->linkid,hsb2->linkid);  ret=FAILED;}
	if(hsb1->pid!=hsb2->pid)	
		{rtlglue_printf("hsb1->pid=0x%x; hsb2->pid=0x%x;\n",hsb1->pid,hsb2->pid);  ret=FAILED;}
	if(hsb1->cfi!=hsb2->cfi)	
		{rtlglue_printf("hsb1->cfi=0x%x; hsb2->cfi=0x%x;\n",hsb1->cfi,hsb2->cfi);  ret=FAILED;}
	if(hsb1->vid!=hsb2->vid)
		{rtlglue_printf("hsb1->vid=0x%x; hsb2->vid=0x%x;\n",hsb1->vid,hsb2->vid);  ret=FAILED;}
	if(hsb1->pppcompable!=hsb2->pppcompable)	
		{rtlglue_printf("hsb1->pppcompable=0x%x; hsb2->pppcompable=0x%x;\n",hsb1->pppcompable,hsb2->pppcompable);  ret=FAILED;}
	if(hsb1->framectrl!=hsb2->framectrl)	
		{rtlglue_printf("hsb1->framectrl=0x%x; hsb2->framectrl=0x%x;\n",hsb1->framectrl,hsb2->framectrl);  ret=FAILED;}
	if(hsb1->ethtp!=hsb2->ethtp)	
		{rtlglue_printf("hsb1->ethtp=0x%x; hsb2->ethtp=0x%x;\n",hsb1->ethtp,hsb2->ethtp);  ret=FAILED;}
	if(memcmp(&hsb1->dmac[0],&hsb2->dmac[0],6)!=0)
	{		
		rtlglue_printf("hsb1->dmac[]=%02x:%02x:%02x:%02x:%02x:%02x; hsb2->dmac[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsb1->dmac[0],hsb1->dmac[1],hsb1->dmac[2],hsb1->dmac[3],hsb1->dmac[4],hsb1->dmac[5]
			,hsb2->dmac[0],hsb2->dmac[1],hsb2->dmac[2],hsb2->dmac[3],hsb2->dmac[4],hsb2->dmac[5]);
		ret=FAILED;
	}

	if(memcmp(&hsb1->smac[0],&hsb2->smac[0],6)!=0)
	{		
		rtlglue_printf("hsb1->smac[]=%02x:%02x:%02x:%02x:%02x:%02x; hsb2->smac[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsb1->smac[0],hsb1->smac[1],hsb1->smac[2],hsb1->smac[3],hsb1->smac[4],hsb1->smac[5]
			,hsb2->smac[0],hsb2->smac[1],hsb2->smac[2],hsb2->smac[3],hsb2->smac[4],hsb2->smac[5]);
		ret=FAILED;
	}

	if((hsb1->mac3[0] != hsb2->mac3[0]) || (hsb1->mac3[1] != hsb2->mac3[1]) || (hsb1->mac3[2] != hsb2->mac3[2])
		|| (hsb1->mac3[3]!= hsb2->mac3[3]) || (hsb1->mac3[4] != hsb2->mac3[4]) || (hsb1->mac3[5] != hsb2->mac3[5]))
	{		
		rtlglue_printf("hsb1->mac3[]=%02x:%02x:%02x:%02x:%02x:%02x; hsb2->mac3[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsb1->mac3[0],hsb1->mac3[1],hsb1->mac3[2],hsb1->mac3[3],hsb1->mac3[4],hsb1->mac3[5]
			,hsb2->mac3[0],hsb2->mac3[1],hsb2->mac3[2],hsb2->mac3[3],hsb2->mac3[4],hsb2->mac3[5]);
		ret=FAILED;
	}
	if(hsb1->sid!=hsb2->sid)	
		{rtlglue_printf("hsb1->sid=0x%x; hsb2->sid=0x%x;\n",hsb1->sid,hsb2->sid);  ret=FAILED;}
	if((hsb1->mac4[0] != hsb2->mac4[0]) || (hsb1->mac4[1] != hsb2->mac4[1]) || (hsb1->mac4[2] != hsb2->mac4[2])
		|| (hsb1->mac4[3]!= hsb2->mac4[3]) || (hsb1->mac4[4] != hsb2->mac4[4]) || (hsb1->mac4[5] != hsb2->mac4[5]))
	{		
		rtlglue_printf("hsb1->mac4[]=%02x:%02x:%02x:%02x:%02x:%02x; hsb2->mac4[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsb1->mac4[0],hsb1->mac4[1],hsb1->mac4[2],hsb1->mac4[3],hsb1->mac4[4],hsb1->mac4[5]
			,hsb2->mac4[0],hsb2->mac4[1],hsb2->mac4[2],hsb2->mac4[3],hsb2->mac4[4],hsb2->mac4[5]);
		ret=FAILED;
	}	
	//if(hsb1->l3cs!=hsb2->l3cs)	
	//	{rtlglue_printf("hsb1->l3cs=0x%x; hsb2->l3cs=0x%x;\n",hsb1->l3cs,hsb2->l3cs);  ret=FAILED;}		
	if(hsb1->l3len!=hsb2->l3len)
		{rtlglue_printf("hsb1->l3len=0x%x; hsb2->l3len=0x%x;\n",hsb1->l3len,hsb2->l3len);  ret=FAILED;}
	
	if(hsb1->sip!=hsb2->sip)	
		{rtlglue_printf("hsb1->sip=0x%x; hsb2->sip=0x%x;\n",hsb1->sip,hsb2->sip);  ret=FAILED;}
	if(hsb1->dip!=hsb2->dip)	
		{rtlglue_printf("hsb1->dip=0x%x; hsb2->dip=0x%x;\n",hsb1->dip,hsb2->dip);  ret=FAILED;}
	if(hsb1->tos!=hsb2->tos)	
		{rtlglue_printf("hsb1->tos=0x%x; hsb2->tos=0x%x;\n",hsb1->tos,hsb2->tos);  ret=FAILED;}
	if(hsb1->l3offset!=hsb2->l3offset)	
		{rtlglue_printf("hsb1->l3offset=0x%x; hsb2->l3offset=0x%x;\n",hsb1->l3offset,hsb2->l3offset);  ret=FAILED;}
	if(hsb1->ttl!=hsb2->ttl)	
		{rtlglue_printf("hsb1->ttl=0x%x; hsb2->ttl=0x%x;\n",hsb1->ttl,hsb2->ttl);  ret=FAILED;}	
	if(hsb1->l3type!=hsb2->l3type)	
		{rtlglue_printf("hsb1->l3type=0x%x; hsb2->l3type=0x%x;\n",hsb1->l3type,hsb2->l3type);  ret=FAILED;}	
	if(hsb1->ipfrag!=hsb2->ipfrag)	
		{rtlglue_printf("hsb1->ipfrag=0x%x; hsb2->ipfrag=0x%x;\n",hsb1->ipfrag,hsb2->ipfrag);  ret=FAILED;}		
	//if(hsb1->l3csok!=hsb2->l3csok)	
	//	{rtlglue_printf("hsb1->l3csok=0x%x; hsb2->l3csok=0x%x;\n",hsb1->l3csok,hsb2->l3csok);  ret=FAILED;}	
	if(hsb1->udpnocs!=hsb2->udpnocs)	
		{rtlglue_printf("hsb1->udpnocs=0x%x;hsb2->udpnocs=0x%x;\n",hsb1->udpnocs,hsb2->udpnocs);  ret=FAILED;}
	//if(hsb1->l4csok!=hsb2->l4csok)	
	//	{rtlglue_printf("hsb1->l4csok=0x%x;hsb2->l4csok=0x%x;\n",hsb1->l4csok,hsb2->l4csok);  ret=FAILED;}
	if(hsb1->reason!=hsb2->reason)	
		{rtlglue_printf("hsb1->reason=0x%x;hsb2->reason=0x%x;\n",hsb1->reason,hsb2->reason);  ret=FAILED;}
	if(hsb1->l4proto!=hsb2->l4proto)	
		{rtlglue_printf("hsb1->l4proto=0x%x;hsb2->l4proto=0x%x;\n",hsb1->l4proto,hsb2->l4proto);  ret=FAILED;}
	if(hsb1->tcpflag!=hsb2->tcpflag)	
		{rtlglue_printf("hsb1->tcpflag=0x%x;hsb2->tcpflag=0x%x;\n",hsb1->tcpflag,hsb2->tcpflag);  ret=FAILED;}
	//if(hsb1->l4cs!=hsb2->l4cs)	
	//	{rtlglue_printf("hsb1->l4cs=0x%x; hsb2->l4cs=0x%x;\n",hsb1->l4cs,hsb2->l4cs);  ret=FAILED;}		
	if(hsb1->sport!=hsb2->sport)	
		{rtlglue_printf("hsb1->sport=0x%x; hsb2->sport=0x%x;\n",hsb1->sport,hsb2->sport);  ret=FAILED;}	
	if(hsb1->dport!=hsb2->dport)	
		{rtlglue_printf("hsb1->dport=0x%x;hsb2->dport=0x%x;\n",hsb1->dport,hsb2->dport);  ret=FAILED;}		
	return ret;
}

int32 compHsa( hsa_param_t* hsa1, hsa_param_t* hsa2 )
{
	int ret=SUCCESS;
	if(hsa1->outiftype!=hsa2->outiftype)		
		{rtlglue_printf("hsa1->outiftype=0x%x; hsa2->outiftype=0x%x;\n",hsa1->outiftype,hsa2->outiftype);  ret=FAILED;}
	if(hsa1->outl2encap!=hsa2->outl2encap)		
		{rtlglue_printf("hsa1->outl2encap=0x%x; hsa2->outl2encap=0x%x;\n",hsa1->outl2encap,hsa2->outl2encap);  ret=FAILED;}
	if(hsa1->outlanfcs!=hsa2->outlanfcs)		
		{rtlglue_printf("hsa1->outlanfcs=0x%x; hsa2->outlanfcs=0x%x;\n",hsa1->outlanfcs,hsa2->outlanfcs);  ret=FAILED;}
	if(hsa1->pppprotolen!=hsa2->pppprotolen)		
		{rtlglue_printf("hsa1->pppprotolen=0x%x; hsa2->pppprotolen=0x%x;\n",hsa1->pppprotolen,hsa2->pppprotolen);  ret=FAILED;}
	if(hsa1->outsarhdr!=hsa2->outsarhdr)		
		{rtlglue_printf("hsa1->outsarhdr=0x%x; hsa2->outsarhdr=0x%x;\n",hsa1->outsarhdr,hsa2->outsarhdr);  ret=FAILED;}
	if(hsa1->droppacket!=hsa2->droppacket)		
		{rtlglue_printf("hsa1->droppacket=0x%x; hsa2->droppacket=0x%x;\n",hsa1->droppacket,hsa2->droppacket);  ret=FAILED;}
	if(hsa1->pif!=hsa2->pif)		
		{rtlglue_printf("hsa1->pif=0x%x; hsa2->pif=0x%x;\n",hsa1->pif,hsa2->pif);  ret=FAILED;}
	if(hsa1->l3change!=hsa2->l3change)		
		{rtlglue_printf("hsa1->l3change=0x%x; hsa2->l3change=0x%x;\n",hsa1->l3change,hsa2->l3change);  ret=FAILED;}
	if(hsa1->l4change!=hsa2->l4change)		
		{rtlglue_printf("hsa1->l4change=0x%x; hsa2->l4change=0x%x;\n",hsa1->l4change,hsa2->l4change);  ret=FAILED;}
	if(hsa1->fromcpu!=hsa2->fromcpu)		
		{rtlglue_printf("hsa1->fromcpu=0x%x; hsa2->fromcpu=0x%x;\n",hsa1->fromcpu,hsa2->fromcpu);  ret=FAILED;}
	if(hsa1->tocpu!=hsa2->tocpu)		
		{rtlglue_printf("hsa1->tocpu=0x%x; hsa2->tocpu=0x%x;\n",hsa1->tocpu,hsa2->tocpu);  ret=FAILED;}
	if(hsa1->iniftype!=hsa2->iniftype)		
		{rtlglue_printf("hsa1->iniftype=0x%x; hsa2->iniftype=0x%x;\n",hsa1->iniftype,hsa2->iniftype);  ret=FAILED;}
	if(hsa1->tif!=hsa2->tif)		
		{rtlglue_printf("hsa1->tif=0x%x; hsa2->tif=0x%x;\n",hsa1->tif,hsa2->tif);  ret=FAILED;}
	if(hsa1->linkid!=hsa2->linkid)		
		{rtlglue_printf("hsa1->linkid=0x%x; hsa2->linkid=0x%x;\n",hsa1->linkid,hsa2->linkid);  ret=FAILED;}
	if(hsa1->dstportidx!=hsa2->dstportidx)		
		{rtlglue_printf("hsa1->dstportidx=0x%x; hsa2->dstportidx=0x%x;\n",hsa1->dstportidx,hsa2->dstportidx);  ret=FAILED;}
	if(hsa1->srcportidx!=hsa2->srcportidx)		
		{rtlglue_printf("hsa1->srcportidx=0x%x; hsa2->srcportidx=0x%x;\n",hsa1->srcportidx,hsa2->srcportidx);  ret=FAILED;}
	if(hsa1->ethtp!=hsa2->ethtp)		
		{rtlglue_printf("hsa1->ethtp=0x%x; hsa2->ethtp=0x%x;\n",hsa1->ethtp,hsa2->ethtp);  ret=FAILED;}
	if(hsa1->framectrl!=hsa2->framectrl)		
		{rtlglue_printf("hsa1->framectrl=0x%x; hsa2->framectrl=0x%x;\n",hsa1->framectrl,hsa2->framectrl);  ret=FAILED;}
	if(memcmp(&hsa1->dmac[0],&hsa2->dmac[0],6)!=0)
	{		
		rtlglue_printf("hsa1->dmac[]=%02x:%02x:%02x:%02x:%02x:%02x; hsa2->dmac[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsa1->dmac[0],hsa1->dmac[1],hsa1->dmac[2],hsa1->dmac[3],hsa1->dmac[4],hsa1->dmac[5]
			,hsa2->dmac[0],hsa2->dmac[1],hsa2->dmac[2],hsa2->dmac[3],hsa2->dmac[4],hsa2->dmac[5]);
		ret=FAILED;
	}
	if(memcmp(&hsa1->smac[0],&hsa2->smac[0],6)!=0)
	{		
		rtlglue_printf("hsa1->smac[]=%02x:%02x:%02x:%02x:%02x:%02x; hsa2->smac[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsa1->smac[0],hsa1->smac[1],hsa1->smac[2],hsa1->smac[3],hsa1->smac[4],hsa1->smac[5]
			,hsa2->smac[0],hsa2->smac[1],hsa2->smac[2],hsa2->smac[3],hsa2->smac[4],hsa2->smac[5]);
		ret=FAILED;
	}
	if(memcmp(&hsa1->mac3[0],&hsa2->mac3[0],6)!=0)
	{		
		rtlglue_printf("hsa1->mac3[]=%02x:%02x:%02x:%02x:%02x:%02x; hsa2->mac3[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsa1->mac3[0],hsa1->mac3[1],hsa1->mac3[2],hsa1->mac3[3],hsa1->mac3[4],hsa1->mac3[5]
			,hsa2->mac3[0],hsa2->mac3[1],hsa2->mac3[2],hsa2->mac3[3],hsa2->mac3[4],hsa2->mac3[5]);
		ret=FAILED;
	}
	if(memcmp(&hsa1->mac4[0],&hsa2->mac4[0],6)!=0)
	{		
		rtlglue_printf("hsa1->mac4[]=%02x:%02x:%02x:%02x:%02x:%02x; hsa2->mac4[]=%02x:%02x:%02x:%02x:%02x:%02x;\n"
			,hsa1->mac4[0],hsa1->mac4[1],hsa1->mac4[2],hsa1->mac4[3],hsa1->mac4[4],hsa1->mac4[5]
			,hsa2->mac4[0],hsa2->mac4[1],hsa2->mac4[2],hsa2->mac4[3],hsa2->mac4[4],hsa2->mac4[5]);
		ret=FAILED;
	}
	if(hsa1->sid!=hsa2->sid)		
		{rtlglue_printf("hsa1->sid=0x%x; hsa2->sid=0x%x;\n",hsa1->sid,hsa2->sid);  ret=FAILED;}
	if(hsa1->pid!=hsa2->pid)		
		{rtlglue_printf("hsa1->pid=0x%x; hsa2->pid=0x%x;\n",hsa1->pid,hsa2->pid);  ret=FAILED;}
	if(hsa1->cfi!=hsa2->cfi)		
		{rtlglue_printf("hsa1->cfi=0x%x; hsa2->cfi=0x%x;\n",hsa1->cfi,hsa2->cfi);  ret=FAILED;}
	if(hsa1->vid!=hsa2->vid)		
		{rtlglue_printf("hsa1->vid=0x%x; hsa2->vid=0x%x;\n",hsa1->vid,hsa2->vid);  ret=FAILED;}
	if(hsa1->vidremark!=hsa2->vidremark)
		{rtlglue_printf("hsa1->vidremark=0x%x; hsa2->vidremark=0x%x;\n",hsa1->vidremark,hsa2->vidremark);  ret=FAILED;}
	if(hsa1->l3type!=hsa2->l3type)		
		{rtlglue_printf("hsa1->l3type=0x%x; hsa2->l3type=0x%x;\n",hsa1->l3type,hsa2->l3type);  ret=FAILED;}
	if(hsa1->tosremr!=hsa2->tosremr)		
		{rtlglue_printf("hsa1->tosremr=0x%x; hsa2->tosremr=0x%x;\n",hsa1->tosremr,hsa2->tosremr);  ret=FAILED;}
	if(hsa1->tos!=hsa2->tos)		
		{rtlglue_printf("hsa1->tos=0x%x; hsa2->tos=0x%x;\n",hsa1->tos,hsa2->tos);  ret=FAILED;}
	if(hsa1->ttl!=hsa2->ttl)		
		{rtlglue_printf("hsa1->ttl=0x%x; hsa2->ttl=0x%x;\n",hsa1->ttl,hsa2->ttl);  ret=FAILED;}
	if(hsa1->inl3offset!=hsa2->inl3offset)		
		{rtlglue_printf("hsa1->inl3offset=0x%x; hsa2->inl3offset=0x%x;\n",hsa1->inl3offset,hsa2->inl3offset);  ret=FAILED;}
	if(hsa1->inl3len!=hsa2->inl3len)		
		{rtlglue_printf("hsa1->inl3len=0x%x; hsa2->inl3len=0x%x;\n",hsa1->inl3len,hsa2->inl3len);  ret=FAILED;}
	if(hsa1->l3cs!=hsa2->l3cs)		
		{rtlglue_printf("hsa1->l3cs=0x%x; hsa2->l3cs=0x%x;\n",hsa1->l3cs,hsa2->l3cs);  ret=FAILED;}
	if(hsa1->sip!=hsa2->sip)		
		{rtlglue_printf("hsa1->sip=0x%x; hsa2->sip=0x%x;\n",hsa1->sip,hsa2->sip);  ret=FAILED;}
	if(hsa1->dip!=hsa2->dip)		
		{rtlglue_printf("hsa1->dip=0x%x; hsa2->dip=0x%x;\n",hsa1->dip,hsa2->dip);  ret=FAILED;}
	if(hsa1->sport!=hsa2->sport)		
		{rtlglue_printf("hsa1->sport=0x%x; hsa2->sport=0x%x;\n",hsa1->sport,hsa2->sport);  ret=FAILED;}
	if(hsa1->dport!=hsa2->dport)		
		{rtlglue_printf("hsa1->dport=0x%x; hsa2->dport=0x%x;\n",hsa1->dport,hsa2->dport);  ret=FAILED;}
	if(hsa1->l4cs!=hsa2->l4cs)		
		{rtlglue_printf("hsa1->l4cs=0x%x; hsa2->l4cs=0x%x;\n",hsa1->l4cs,hsa2->l4cs);  ret=FAILED;}
	if(hsa1->l4proto!=hsa2->l4proto)		
		{rtlglue_printf("hsa1->l4proto=0x%x; hsa2->l4proto=0x%x;\n",hsa1->l4proto,hsa2->l4proto);  ret=FAILED;}
	if(hsa1->reason!=hsa2->reason)		
		{rtlglue_printf("hsa1->reason=0x%x; hsa2->reason=0x%x;\n",hsa1->reason,hsa2->reason);  ret=FAILED;}

	return ret;
}

int32 compHsbp( rtl8672_tblAsic_hsbpTable_t* hsbp1, rtl8672_tblAsic_hsbpTable_t* hsbp2 )
{
	int ret=SUCCESS;
	if(hsbp1->pif!=hsbp2->pif)		
		{rtlglue_printf("hsbp1->pif=0x%x; hsbp2->pif=0x%x;\n",hsbp1->pif,hsbp2->pif);  ret=FAILED;}
	if(hsbp1->tif!=hsbp2->tif)
		{rtlglue_printf("hsbp1->tif=0x%x; hsbp2->tif=0x%x;\n",hsbp1->tif,hsbp2->tif);  ret=FAILED;}
	if(hsbp1->compppp!=hsbp2->compppp)	
		{rtlglue_printf("hsbp1->compppp=%d; hsbp2->compppp=%d;\n",hsbp1->compppp,hsbp2->compppp);  ret=FAILED;}	
	if(hsbp1->pppproto!=hsbp2->pppproto)	
		{rtlglue_printf("hsbp1->pppproto=0x%x; hsbp2->pppproto=0x%x;\n",hsbp1->pppproto,hsbp2->pppproto);  ret=FAILED;}
	if(hsbp1->RxHdrInSram!=hsbp2->RxHdrInSram)	
		{rtlglue_printf("hsbp1->RxHdrInSram=0x%x; hsbp2->RxHdrInSram=0x%x;\n",hsbp1->RxHdrInSram,hsbp2->RxHdrInSram);  ret=FAILED;}
	if(hsbp1->rxshift!= hsbp2->rxshift)
		{rtlglue_printf("hsbp1->rxshift=0x%x; hsbp2->rxshift=0x%x;\n",hsbp1->rxshift,hsbp2->rxshift); ret = FAILED;}
#if 0
	if(hsbp1->sramaddr!=hsbp2->sramaddr)	
		{rtlglue_printf("hsbp1->sramaddr=0x%x; hsbp2->sramaddr=0x%x;\n",hsbp1->sramaddr,hsbp2->sramaddr);  ret=FAILED;}
	if(hsbp1->dramaddr!=hsbp2->dramaddr)	
		{rtlglue_printf("hsbp1->dramaddr=0x%x; hsbp2->dramaddr=0x%x;\n",hsbp1->dramaddr,hsbp2->dramaddr);  ret=FAILED;}
#endif
	return ret;
}


#if 0
/*
 *  Test the random function.
 */
int32 testModel_random(uint32 caseNo)
{
	int32 retval = FAILED;
	int i, j;
	
	for( i = IC_TYPE_MIN; i < IC_TYPE_MAX; i++ )
	{

		retval = model_setTestTarget( i );
			
		if ( retval == VMAC_ERROR_NON_AVAILABLE ) continue; /* Not support, we don't compare. */

		rtlglue_srandom( modelRandomSeed );
		rtlglue_printf( "\ni=%d\n", i );
		for( j = 0; j < 0x40; j++ )
		{
			rtlglue_printf( "%08x  ", rtlglue_random() );
		}
	}

	return SUCCESS;
}
#endif


/****************************************************************************
 ****__*****__*************                                                ** 
 ***|  |***|  |****__******                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|__|*****                                                ** 
 ***|  |***|  |************      Add Your Model Test Cases HERE !!!        ** 
 ***|   ___   |****__******                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|  |***|  |***|  |*****                                                ** 
 ***|__|***|__|***|__|*****                                                **
 ****************************************************************************/
MODEL_TEST_CASE_T modelTestCase[] =
{
#if 0
	MODEL_TEST_CASE(  101, testParser_PktFromMAC, GRP_ALL | GRP_PARSER),
	MODEL_TEST_CASE(  102, testParser_PktFromSAR, GRP_ALL | GRP_PARSER),
	MODEL_TEST_CASE(  103, testParser_PktFromWLAN, GRP_ALL | GRP_PARSER),
	MODEL_TEST_CASE(  104, testParser_PktTrapToCPU, GRP_ALL | GRP_PARSER),
	MODEL_TEST_CASE(  105, testParser_RandomPkt_MAP_SRAM, GRP_ALL | GRP_PARSER),
 	MODEL_TEST_CASE(  106, testParser_PktFrom8139ToMACvRx, GRP_ALL | GRP_PARSER),	
 	MODEL_TEST_CASE(  107, testParser_PktFrom8139ToEXTpTx, GRP_ALL | GRP_PARSER),	
  	MODEL_TEST_CASE(  108, testParser_PktFrom8139ToMACpTx, GRP_ALL | GRP_PARSER),	
//  	MODEL_TEST_CASE(  109, testParser_PktFrom8139ToSARpTx, GRP_ALL | GRP_PARSER),	

	MODEL_TEST_CASE(  204, testRingCtrl_Registers, GRP_ALL | GRP_RINGCONTROLLER),	
 	MODEL_TEST_CASE(  205, testRingCtrl_from_sar_prx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  206, testRingCtrl_from_sar_prx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  207, testRingCtrl_from_sar_prx_and_prx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  208, testRingCtrl_from_sar_prx_to_vrx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  209, testRingCtrl_from_sar_prx_to_vrx_but_vrx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  210, testRingCtrl_from_sar_vtx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  211, testRingCtrl_from_sar_vtx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),

	MODEL_TEST_CASE(  212, testRingCtrl_from_mac_prx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  213, testRingCtrl_from_mac_prx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  214, testRingCtrl_from_mac_prx_and_prx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  215, testRingCtrl_from_mac_prx_to_vrx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  216, testRingCtrl_from_mac_prx_to_vrx_but_vrx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  217, testRingCtrl_from_mac_vtx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  218, testRingCtrl_from_mac_vtx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),

	MODEL_TEST_CASE(  219, testRingCtrl_from_ext_prx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  220, testRingCtrl_from_ext_prx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  221, testRingCtrl_from_ext_prx_and_prx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  222, testRingCtrl_from_ext_prx_to_vrx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  223, testRingCtrl_from_ext_prx_to_vrx_but_vrx_full, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  224, testRingCtrl_from_ext_vtx_to_ptx, GRP_ALL | GRP_RINGCONTROLLER),
	MODEL_TEST_CASE(  225, testRingCtrl_from_ext_vtx_to_ptx_but_ptx_full, GRP_ALL | GRP_RINGCONTROLLER),	

	MODEL_TEST_CASE(  250, testRingCtrl_from_mac_prx_to_ptx_with_phy,  GRP_ALL | GRP_RINGCONTROLLER),	

	
	MODEL_TEST_CASE(  301, testALE_PNP, GRP_ALL | GRP_ALE),
	MODEL_TEST_CASE(  302, testALE_ReasonPriority, GRP_ALL | GRP_ALE),
	MODEL_TEST_CASE(  303, testALE_L2ModelFlow, GRP_ALL | GRP_ALE),	
	MODEL_TEST_CASE(  304, testALE_L34ModelFlow, GRP_ALL | GRP_ALE),		
	MODEL_TEST_CASE(  305, testALE_SMACCheck, GRP_ALL | GRP_ALE),		
	MODEL_TEST_CASE(  401, testTranslator_MACToMAC, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  402, testTranslator_SARToMAC, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  403, testTranslator_WLANToMAC, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  404, testTranslator_MACToSAR, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  405, testTranslator_SARToSAR, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  406, testTranslator_WLANToSAR, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  407, testTranslator_MACToWLAN, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  408, testTranslator_SARToWLAN, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  409, testTranslator_WLANToWLAN, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  410, testTranslator_RandomPkt_MAP_SRAM, GRP_ALL | GRP_TRANSLATOR),
	MODEL_TEST_CASE(  500, testRingCtrl_init_ring,  GRP_ALL),	

	//MODEL_TEST_CASE(  701, testPktFromSAR,GRP_NONE),
	//MODEL_TEST_CASE(  702, testSARdesc,GRP_NONE),
	//MODEL_TEST_CASE(  703, testSARrxshift,GRP_NONE),
	//MODEL_TEST_CASE(  704, testSARTxSram,GRP_NONE),
	//MODEL_TEST_CASE(  705, testSARTxSram2,GRP_NONE),
	//MODEL_TEST_CASE(  706, testSARTxSram3,GRP_NONE),
	//MODEL_TEST_CASE(  711, testSARRx,GRP_NONE),
	//MODEL_TEST_CASE(  712, testSARloopback,GRP_NONE),
	//MODEL_TEST_CASE(  721, toggleSPI,GRP_NONE),
	//MODEL_TEST_CASE(  801, GPIO_test,GRP_NONE),
#endif
	/* Final case, DO NOT remove it */
	MODEL_TEST_CASE( 10000, NULL, GRP_ALL ),
};



#if 0
int32 runModelTest(uint32 userId,  int32 argc,int8 **saved)
{
	int8 *nextToken;
	int32 size;
	int32 lower, upper;
	int i;
	int32 retval;
	int32 totalCase = 0;
	int32 okCase = 0;
	int32 failCase = 0;
	int32 failCaseIdx[sizeof(modelTestCase)/sizeof(modelTestCase[0])];
	int32 caseNoSeq = 1; /* to generate auto-increased case number. */
	uint32 groupmask = 0xffffffff;
//	enum IC_TYPE nowtype; 
	
	
	lower = 0;
	upper = modelTestCase[sizeof(modelTestCase)/sizeof(modelTestCase[0])-1].no;
	
	cle_getNextCmdToken(&nextToken,&size,saved);
	if ( !strcmp( nextToken, "all" ) )
		groupmask = GRP_ALL;
	else if ( !strcmp( nextToken, "parser" ) )
		groupmask = GRP_PARSER;
	else if ( !strcmp( nextToken, "translator" ) )
		groupmask = GRP_TRANSLATOR;
	else if ( !strcmp( nextToken, "ale" ) )
		groupmask = GRP_ALE;
	else if ( !strcmp( nextToken, "ringctrl" ) )
		groupmask = GRP_RINGCONTROLLER;
	else
		lower = upper = U32_value(nextToken);
	
	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS )
		upper = U32_value(nextToken);
	
	/* generate auto-increased pCase->no */
	for( i = 0; i < sizeof(modelTestCase)/sizeof(modelTestCase[0]); i++ )
	{
		MODEL_TEST_CASE_T *pCase = &modelTestCase[i];
		
		if  ( pCase->no == AUTO_CASE_NO )
			pCase->no = (caseNoSeq++);
		else
			caseNoSeq = pCase->no+1; /* auto learn the next no */
	}
	
	if ( modelRandomSeedStratagem == ENUM_SEED_SYSTIME )
	{
		uint32 t; /* time_t */
		modelRandomSeed = rtlglue_time(&t);
	}

//	model_getTestTarget(&nowtype);
//	model_setTestTarget(nowtype);
	
	rtlglue_printf( "Running from %d to %d with group mask 0x%08x, seed=0x%08x:\n\n", lower, upper, groupmask, modelRandomSeed );
	rtlglue_srandom( modelRandomSeed );

	for( i = 0; i < sizeof(modelTestCase)/sizeof(modelTestCase[0]); i++ )
	{

		MODEL_TEST_CASE_T *pCase = &modelTestCase[i];

		if ( pCase->no < lower || pCase->no > upper ) continue;
		if ( ( pCase->group & groupmask ) == 0 ) continue;
		if ( pCase->fp == NULL ) continue;

		totalCase++;
		
		rtlglue_printf( "Running Model Test Case %d: %s() ...\n", pCase->no, pCase->name );


#if 0
		/* Prepare virtualMac environment */
		for( t = IC_TYPE_MIN; t < IC_TYPE_MAX; t++ )
		{
			int ret;

			
			ret = model_setTestTarget( t );
		
			if ( ret == VMAC_ERROR_NON_AVAILABLE ) continue; /* Not support, we don't compare. */

			/* Always SEMI RESET to reset queue, because HSB/HSA testing causes QM choas. */
			/*WRITE_MEM32( SSIR, FULL_RST );	[FIXME] */
			for( ret = 0; ret < 38400/10/100; ret++ ) /* delay for 1/100 sec */
				rtlglue_printf("\r");

			ret = virtualMacInit();
			ASSERT( ret==SUCCESS );

		}
#endif		
		
		modelIcInit();
		modelIcSetDefaultValue();
		
		retval = pCase->fp( pCase->no );


#if 0
		{ /* pump all packet in queue */
			int ret;
			for( t = IC_TYPE_MIN; t < IC_TYPE_MAX; t++ )
			{

				ret = model_setTestTarget( t );

				if ( ret == VMAC_ERROR_NON_AVAILABLE ) continue; /* Not support, we don't compare. */


#if 0  /* tony: to avoid program stop */
				ret = virtualMacPumpPackets( PM_PORT_ALL );
#endif
					
				ASSERT( ret==SUCCESS );
			}
		}
#endif

		if ( retval == SUCCESS )
		{
			rtlglue_printf( " ok!\n\n" );
			okCase++;
		}
		else
		{
			rtlglue_printf( "\033[31;43m fail!! \033[m\n\n" );
			failCaseIdx[failCase] = i;
			failCase++;
		}
	
		modelIcExit();
	}

	rtlglue_printf( ">>Total Case: %d,  Pass: %d,  Fail: %d\n\n", totalCase, okCase, failCase );
	if ( failCase > 0 )
	{
		/* list fail cases */
		int i;
		
		rtlglue_printf("  +-- Failed Cases --------------------------------------- seed=0x%08x --+\n", modelRandomSeed );
		for( i = 0; i < failCase; i++ )
		{
			rtlglue_printf("  | case %-5d  %-60s |\n", modelTestCase[failCaseIdx[i]].no,
			                                           modelTestCase[failCaseIdx[i]].name );
		}
		rtlglue_printf("  +--------------------------------------------------------------------------+\007\n\n");
	}

	/* Finally, we had finished all test cases. We must recover the registers to original setting. */
	
//	model_setTestTarget( nowtype );


	return SUCCESS;
}
#endif
void dumpHSB(hsb_param_t *hsb)
{
	rtlglue_printf("----- [Dump HSB] -------------------------------------------------------------\n");
	rtlglue_printf("[HSB] fromcpu=%01x    hasmac=%01x    tif=%01x    pif=%01x     srcportidx=%01x   l3hwfwdip=%01x \n",
		hsb->fromcpu, hsb->hasmac, hsb->tif, hsb->pif, hsb->srcportidx, hsb->l3hwfwdip);
	rtlglue_printf("[HSB] linkid=%01x     pid=%01x       cfi=%01x    vid=%03x   pppCompable=%01x  framectrl=%03x  ethtp=%04x \n",
		hsb->linkid, hsb->pid, hsb->cfi, hsb->vid, hsb->pppcompable, hsb->framectrl, hsb->ethtp);
	rtlglue_printf("[HSB] dmac=%02x:%02x:%02x:%02x:%02x:%02x   smac=%02x:%02x:%02x:%02x:%02x:%02x   sid=%02x \n",
		hsb->dmac[0],hsb->dmac[1],hsb->dmac[2],hsb->dmac[3],hsb->dmac[4],hsb->dmac[5],
		hsb->smac[0],hsb->smac[1],hsb->smac[2],hsb->smac[3],hsb->smac[4],hsb->smac[5],
		hsb->sid);
	rtlglue_printf("[HSB] mac3=%02x:%02x:%02x:%02x:%02x:%02x   mac4=%02x:%02x:%02x:%02x:%02x:%02x   l3cs=%02x \n",
		hsb->mac3[0],hsb->mac3[1],hsb->mac3[2],hsb->mac3[3],hsb->mac3[4],hsb->mac3[5],
		hsb->mac4[0],hsb->mac4[1],hsb->mac4[2],hsb->mac4[3],hsb->mac4[4],hsb->mac4[5],
		hsb->l3cs);
	rtlglue_printf("[HSB] l3len=%04x   sip=%08x   dip=%08x   tos=%02x   l3offset=%04x \n",
		hsb->l3len, hsb->sip, hsb->dip, hsb->tos, hsb->l3offset);
	rtlglue_printf("[HSB] ttl=%02x     l3type=%01x      ipfrag=%01x    l3csok=%01x   udpnocs=%01x   l4csok=%01x \n",
		hsb->ttl, hsb->l3type, hsb->ipfrag, hsb->l3csok, hsb->udpnocs, hsb->l4csok);
	rtlglue_printf("[HSB] reason=%02x  l4proto=%04x  tcpflag=%02x  l4cs=%04x  sport=%04x  dport=%04x \n",
		hsb->reason, hsb->l4proto, hsb->tcpflag, hsb->l4cs, hsb->sport, hsb->dport);
	rtlglue_printf("------------------------------------------------------------------------------\n");

}

void dumpHSA(hsa_param_t *hsa)
{
	rtlglue_printf("----- [Dump HSA] -------------------------------------------------------------\n");
	rtlglue_printf("[HSA] outiftype=%01x   outl2encap=%01x   outlanfcs=%01x   pppprotolen=%01x   outsarhdr=%01x \n",
		hsa->outiftype, hsa->outl2encap, hsa->outlanfcs, hsa->pppprotolen, hsa->outsarhdr);
	rtlglue_printf("[HSA] droppacket=%01x  pif=%01x     l3change=%01x  l4change=%01x    fromcpu=%01x  tocpu=%01x \n",
		hsa->droppacket, hsa->pif, hsa->l3change, hsa->l4change, hsa->fromcpu, hsa->tocpu);
	rtlglue_printf("[HSA] iniftype=%01x    tif=%01x     linkid=%01x    dstportidx=%01x  srcportidx=%01x \n",
		hsa->iniftype, hsa->tif, hsa->linkid, hsa->dstportidx, hsa->srcportidx);
	rtlglue_printf("[HSA] framectrl=%03x   ethtp=%04x   pid=%01x    cfi=%01x    vid=%03x    vidremark=%01x	reason=%02x \n",
		hsa->framectrl, hsa->ethtp, hsa->pid, hsa->cfi, hsa->vid, hsa->vidremark, hsa->reason);
	rtlglue_printf("[HSA] dmac=%02x:%02x:%02x:%02x:%02x:%02x   smac=%02x:%02x:%02x:%02x:%02x:%02x   sid=%02x \n",
		hsa->dmac[0],hsa->dmac[1],hsa->dmac[2],hsa->dmac[3],hsa->dmac[4],hsa->dmac[5],
		hsa->smac[0],hsa->smac[1],hsa->smac[2],hsa->smac[3],hsa->smac[4],hsa->smac[5],
		hsa->sid);
	rtlglue_printf("[HSA] mac3=%02x:%02x:%02x:%02x:%02x:%02x   mac4=%02x:%02x:%02x:%02x:%02x:%02x   l3type=%01x \n",
		hsa->mac3[0],hsa->mac3[1],hsa->mac3[2],hsa->mac3[3],hsa->mac3[4],hsa->mac3[5],
		hsa->mac4[0],hsa->mac4[1],hsa->mac4[2],hsa->mac4[3],hsa->mac4[4],hsa->mac4[5],
		hsa->l3type);
	rtlglue_printf("[HSA] tosremr=%01x   tos=%02x  ttl=%02x   inl3offset=%04x   inl3len=%04x  l3cs=%04x \n",
		hsa->tosremr, hsa->tos, hsa->ttl, hsa->inl3offset, hsa->inl3len, hsa->l3cs);
	rtlglue_printf("[HSA] sip=%08x  dip=%08x   sport=%04x  dport=%04x  l4cs=%04x  l4proto=%02x \n",
		hsa->sip, hsa->dip, hsa->sport, hsa->dport, hsa->l4cs,hsa->l4proto);
	rtlglue_printf("------------------------------------------------------------------------------\n");

}

void dumpHSBP(rtl8672_tblAsic_hsbpTable_t *hsbp)
{
	rtlglue_printf("----- [Dump HSBP] -------------------------------------------------------------\n");
	rtlglue_printf("[HSBP] pif=%01x    tif=%01x    compppp=%01x    pppproto=%04x     RxHdrInSram=%01x   rxshift=%02x \n",
		hsbp->pif, hsbp->tif, hsbp->compppp, hsbp->pppproto, hsbp->RxHdrInSram, hsbp->rxshift);
	rtlglue_printf("[HSBP] sramaddr=%08x     dramaddr=%08x\n",
		hsbp->sramaddr, hsbp->dramaddr);
	rtlglue_printf("------------------------------------------------------------------------------\n");

}


#if 0
int32   runModelTarget(uint32 userId,  int32 argc,int8 **saved){
	int idx;
	int8 *nextToken;
	int32 size;		
	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
		idx = U32_value(nextToken);
	} else {
		return FAILED;
	}

#ifdef RTL867X_MODEL_USER
	if(idx==IC_TYPE_REAL)
	{
		rtlglue_printf("Sorry! USER model can't change to IC_TYPE_REAL.\n");
		return FAILED;
	}
#endif	
	model_setTestTarget(idx);
	return SUCCESS;
}
#endif

int rtl8672_dumpRing(int ring,int idx)
{
	int i;

	if((ring>4) || (idx>=SAR_INTFS+MAC_TX_INTFS+EXT_INTFS))
	{
		rtlglue_printf("parameters failed\n");
		return FAILED;
	}
	
	rtlglue_printf("----- [Dump %s Ring] Interface idx = %02d (%s) ------------------------------\n",(ring==0)?"pRx":(ring==1)?"vRx":(ring==2)?"vTx":(ring==3)?"pTx":"SPL", idx,(idx<SAR_INTFS)?"SAR":((idx<SAR_INTFS+MAC_TX_INTFS)?"MAC":"EXT"));

	if(idx<SAR_INTFS)
	{
		switch(ring)
		{
			case 0:
				rtlglue_printf("PRX RING addr=%x\n",(uint32)user_sar_prx_idx(idx,0));
				for(i=0;i<SAR_PRX_RING_SIZE;i++)
				{
					int pp,ip;
					struct sar_pRx *prx=user_sar_prx_idx(idx,i);
					pp=(READ_MEM32(SPRXDESC0+(idx<<2))>>8)&0xff;
					ip=READ_MEM32(SPRXDESC0+(idx<<2))&0xff;						
					rtlglue_printf("%s%s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x S_EN=%d ID=%d OF=%d\n",(pp==i)?"P":" ",(ip==i)?"I":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
				}					
				break;
			case 1:
				rtlglue_printf("VRX RING addr=%x\n",(uint32)user_sar_vrx_idx(idx,0));
				for(i=0;i<SAR_VRX_RING_SIZE;i++)
				{
					int pp;
					struct sar_vRx *vrx=user_sar_vrx_idx(idx,i);
					pp=READ_MEM32(SVRXDESC0+(idx<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x\n",(uint32)vrx,(pp==i)?"P":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift,vrx->skb_header_addr);
				}					
				break;
			case 2:
				rtlglue_printf("VTX RING addr=%x\n",(uint32)user_sar_vtx_idx(idx,0));				
				for(i=0;i<SAR_VTX_RING_SIZE;i++)
				{
					int pp;
					struct sar_vTx *vtx=user_sar_vtx_idx(idx,i);
					pp=READ_MEM32(SVTXDESC0+(idx<<2))&0xff;
					rtlglue_printf("%s  OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x\n",(pp==i)?"P":" ",vtx->own,vtx->eor,vtx->data_length,vtx->tx_buffer_addr,(vtx->orgAddr<<2));
				}				
				break;
			case 3:
				rtlglue_printf("PTX RING addr=%x\n",(uint32)user_sar_ptx_idx(idx,0));
				for(i=0;i<SAR_PTX_RING_SIZE;i++)
				{
					int ip,pp;
					struct sar_pTx *ptx=user_sar_ptx_idx(idx,i);
					pp=(READ_MEM32(SPTXDESC0+(idx<<2))>>8)&0xff;						
					ip=READ_MEM32(SPTXDESC0+(idx<<2))&0xff;
					rtlglue_printf("%s%s OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x OEOR=%d S_ID=%03d S_SIZE=%03d\n",(pp==i)?"P":" ",(ip==i)?"I":" ",ptx->own,ptx->eor,ptx->data_length,ptx->tx_buffer_addr,(ptx->orgAddr<<2),ptx->oeor,ptx->sram_map_id,ptx->sram_size);
				}
				break;
			case 4:
				for(i=0;i<SAR_PTX_RING_SIZE;i++)
				{
					int pp;
					struct sp_pRx *prx=user_sp_to_sar_idx(idx,i);
					pp=READ_MEM32(SPSAR0DESC+(idx<<2))&0xff;						
					rtlglue_printf("%s  OWN=%d EOR=%d BUF=%08x SKB=%08x S_EN=%d ID=%d OF=%d\n",(pp==i)?"P":" ",prx->own,prx->eor,prx->rx_buffer_addr,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
				}					
				break;					
		}
	}
	else if(idx<SAR_INTFS+MAC_TX_INTFS)
	{

		if((idx-SAR_INTFS>=MAC_RX_INTFS)&&(ring<2))
		{
			rtlglue_printf("non-exist RX interface\n");
			return FAILED;
			
		}
		switch(ring)
		{
			case 0:					
				for(i=0;i<MAC_PRX_RING_SIZE;i++)
				{
					int pp,ip;
					struct mac_pRx *prx=user_mac_prx_idx(idx-SAR_INTFS,i);
					pp=(READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))>>8)&0xff;
					ip=READ_MEM32(MPRXDESC0+((idx-SAR_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x S_EN=%d ID=%d OF=%d\n",(uint32)prx,(pp==i)?"P":" ",(ip==i)?"I":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
				}
				break;
			case 1:
				for(i=0;i<MAC_VRX_RING_SIZE;i++)
				{
					int pp;
					struct mac_vRx *vrx=user_mac_vrx_idx(idx-SAR_INTFS,i);
					pp=READ_MEM32(MVRXDESC0+((idx-SAR_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x\n",(uint32)vrx,(pp==i)?"P":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift,vrx->skb_header_addr);
				}					
				break;
			case 2:
				for(i=0;i<MAC_VTX_RING_SIZE;i++)
				{
					int pp;
					struct mac_vTx *vtx=user_mac_vtx_idx(idx-SAR_INTFS,i);
					pp=READ_MEM32(MVTXDESC0+((idx-SAR_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x\n",(uint32)vtx,(pp==i)?"P":" ",vtx->own,vtx->eor,vtx->data_length,vtx->tx_buffer_addr,(vtx->orgAddr<<2));
				}				
				break;
#if 1				
			case 3:

				for(i=0;i<MAC_PTX_RING1_SIZE;i++)
				{
					int ip,pp;
					struct mac_pTx *ptx=user_mac_ptx_idx(idx-SAR_INTFS,i);
					pp=(READ_MEM32(MPTXDESC0+((idx-SAR_INTFS)<<2))>>8)&0xff;						
					ip=READ_MEM32(MPTXDESC0+((idx-SAR_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s%s OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x OEOR=%d S_ID=%03d S_SIZE=%03d\n",(uint32)ptx,(pp==i)?"P":" ",(ip==i)?"I":" ",ptx->own,ptx->eor,ptx->data_length,ptx->tx_buffer_addr,(ptx->orgAddr<<2),ptx->oeor,ptx->sram_map_id,ptx->sram_size);
				}
				break;
			case 4:

				for(i=0;i<MAC_PTX_RING1_SIZE;i++)
				{
					int pp;
					struct sp_pRx *prx=user_sp_to_mac_idx(idx-SAR_INTFS,i);
					pp=READ_MEM32(SPMAC0DESC+((idx-SAR_INTFS)<<2))&0xff;						
					rtlglue_printf("%08x %s  OWN=%d EOR=%d BUF=%08x SKB=%08x S_EN=%d ID=%d OF=%d\n",(uint32)prx,(pp==i)?"P":" ",prx->own,prx->eor,prx->rx_buffer_addr,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
				}					
#endif				
				break;						
		}
	}
	else
	{
		switch(ring)
		{
			case 0:					
				for(i=0;i<EXT_PRX_RING_SIZE;i++)
				{
					int pp;
					struct ext_Rx *prx=user_ext_prx_idx(idx-SAR_INTFS-MAC_TX_INTFS,i);
					pp=READ_MEM32(EPRXDESC0+((idx-SAR_INTFS-MAC_TX_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x LID=0x%x\n",(uint32)prx,(pp==i)?"P":" ",prx->own,prx->eor,prx->data_length,prx->rx_buffer_addr,prx->rx_shift,prx->skb_header_addr,prx->linkid);
				}
				break;
			case 1:
				for(i=0;i<EXT_VRX_RING_SIZE;i++)
				{
					int pp;
					struct ext_Rx *vrx=user_ext_vrx_idx(idx-SAR_INTFS-MAC_TX_INTFS,i);
					pp=READ_MEM32(EVRXDESC0+((idx-SAR_INTFS-MAC_TX_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x SFT=%d SKB=%08x\n",(uint32)vrx,(pp==i)?"P":" ",vrx->own,vrx->eor,vrx->data_length,vrx->rx_buffer_addr,vrx->rx_shift,vrx->skb_header_addr);
				}					
				break;
			case 2:
				for(i=0;i<EXT_VTX_RING_SIZE;i++)
				{
					int pp;
					struct ext_Tx *vtx=user_ext_vtx_idx(idx-SAR_INTFS-MAC_TX_INTFS,i);
					pp=READ_MEM32(EVTXDESC0+((idx-SAR_INTFS-MAC_TX_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x\n",(uint32)vtx,(pp==i)?"P":" ",vtx->own,vtx->eor,vtx->data_length,vtx->tx_buffer_addr,(vtx->orgAddr<<2));
				}				
				break;
#if 1				
			case 3:
				for(i=0;i<EXT_PTX_RING2_SIZE;i++)
				{
					int pp;
					struct ext_Tx *ptx=user_ext_ptx_idx(idx-SAR_INTFS-MAC_TX_INTFS,i);
					pp=READ_MEM32(EPTXDESC0+((idx-SAR_INTFS-MAC_TX_INTFS)<<2))&0xff;
					rtlglue_printf("%08x %s  OWN=%d EOR=%d LEN=%04d BUF=%08x ORG=%08x OEOR=%d RSV=%x SKB=%x\n",(uint32)ptx,(pp==i)?"P":" ",ptx->own,ptx->eor,ptx->data_length,ptx->tx_buffer_addr,(ptx->orgAddr<<2),ptx->oeor,ptx->rsv,ptx->skb_header_addr);
				}
				break;
			case 4:
				for(i=0;i<EXT_PTX_RING2_SIZE;i++)
				{
					int pp;
					struct sp_pRx *prx=user_sp_to_ext_idx(idx-SAR_INTFS-MAC_TX_INTFS,i);
					pp=READ_MEM32(SPEXT0DESC+((idx-SAR_INTFS-MAC_TX_INTFS)<<2))&0xff;
					rtlglue_printf("%s  OWN=%d EOR=%d BUF=%08x SKB=%08x S_EN=%d ID=%d OF=%d\n",(pp==i)?"P":" ",prx->own,prx->eor,prx->rx_buffer_addr,prx->skb_header_addr,prx->sram_en,prx->sram_map_id,prx->sram_map_of);
				}					
				break;						
#endif				
		}	
	}

	rtlglue_printf("-----------------------------------------------------------------------------\n");
	return SUCCESS;


}

#if 0
int rtl8672_parse_l34token(rtl8672_tblAsicDrv_l4Param_t *l34param,int8 **saved)
{
	int8 *nextToken;
	int32 size;
	int i;

	
	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "pid") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->PriorityID = U32_value(nextToken);	
			rtlglue_printf("PriorityID=%d\n",l34param->PriorityID);
		}
		else
		{
			rtlglue_printf("pid error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "valid") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->Valid = U32_value(nextToken);	
			rtlglue_printf("Valid=%d\n",l34param->Valid);
		}
		else
		{
			rtlglue_printf("Valid error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}	


	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "dstportidx") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->DstPortIdx = U32_value(nextToken);	
			rtlglue_printf("DstPortIdx=%d\n",l34param->DstPortIdx);
		}
		else
		{
			rtlglue_printf("DstPortIdx error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "tos") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->Tos = U32_value(nextToken);	
			rtlglue_printf("Tos=%d\n",l34param->Tos);
		}
		else
		{
			rtlglue_printf("Tos error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "age") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->Age = U32_value(nextToken);	
			rtlglue_printf("Age=%d\n",l34param->Age);
		}
		else
		{
			rtlglue_printf("Age error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "proto") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->Proto = U32_value(nextToken);	
			rtlglue_printf("Proto=%d\n",l34param->Proto);
		}
		else
		{
			rtlglue_printf("Proto error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "sip") != 0) return FAILED;

		l34param->SrcIP=0;

		for(i=0;i<4;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l34param->SrcIP |= U32_value(nextToken)<<((3-i)<<3);				
			}
			else
			{
				rtlglue_printf("SIP error\n");
				return FAILED;
			}
		}

		rtlglue_printf("SrcIP=%x\n",l34param->SrcIP);
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "dip") != 0) return FAILED;

		l34param->DstIP=0;

		for(i=0;i<4;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l34param->DstIP |= U32_value(nextToken)<<((3-i)<<3);				
			}
			else
			{
				rtlglue_printf("DIP error\n");
				return FAILED;
			}
		}

		rtlglue_printf("DstIP=%x\n",l34param->DstIP);
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "sport") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->SrcPort = U32_value(nextToken);	
			rtlglue_printf("SrcPort=%d\n",l34param->SrcPort);
		}
		else
		{
			rtlglue_printf("SrcPort error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "dport") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->DstPort = U32_value(nextToken);	
			rtlglue_printf("DstPort=%d\n",l34param->DstPort);
		}
		else
		{
			rtlglue_printf("DstPort error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "nsip") != 0) return FAILED;

		l34param->NewSrcIP=0;

		for(i=0;i<4;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l34param->NewSrcIP |= U32_value(nextToken)<<((3-i)<<3);				
			}
			else
			{
				rtlglue_printf("NewSrcIP error\n");
				return FAILED;
			}
		}

		rtlglue_printf("NewSrcIP=%x\n",l34param->NewSrcIP);
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "ndip") != 0) return FAILED;

		l34param->NewDstIP=0;

		for(i=0;i<4;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l34param->NewDstIP |= U32_value(nextToken)<<((3-i)<<3);				
			}
			else
			{
				rtlglue_printf("NewDstIP error\n");
				return FAILED;
			}
		}

		rtlglue_printf("NewDstIP=%x\n",l34param->NewDstIP);
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "nsport") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->NewSrcPort = U32_value(nextToken);	
			rtlglue_printf("NewSrcPort=%d\n",l34param->NewSrcPort);
		}
		else
		{
			rtlglue_printf("NewSrcPort error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "ndport") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->NewDstPort = U32_value(nextToken);	
			rtlglue_printf("NewDstPort=%d\n",l34param->NewDstPort);
		}
		else
		{
			rtlglue_printf("NewDstPort error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "nhmac") != 0) return FAILED;

		for(i=0;i<6;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l34param->NHMAC[i] = U32_value(nextToken);				
			}
			else
			{
				rtlglue_printf("NHMAC error\n");
				return FAILED;
			}
		}

		rtlglue_printf("NHMAC=%02x-%02x-%02x-%02x-%02x-%02x\n",l34param->NHMAC[0] ,l34param->NHMAC[1] ,l34param->NHMAC[2] ,l34param->NHMAC[3] ,l34param->NHMAC[4] ,l34param->NHMAC[5] );
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "sid") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->SessionID = U32_value(nextToken);	
			rtlglue_printf("SessionID=%d\n",l34param->SessionID);
		}
		else
		{
			rtlglue_printf("SessionID error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "lastdword") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l34param->TtlDe = (U32_value(nextToken)&0x80000000)?1:0;
			rtlglue_printf("TtlDe=%d\n",l34param->TtlDe);
			l34param->PrecedRemr = (U32_value(nextToken)&0x40000000)?1:0;
			rtlglue_printf("PrecedRemr=%d\n",l34param->PrecedRemr);
			l34param->TosRemr = (U32_value(nextToken)&0x20000000)?1:0;
			rtlglue_printf("TosRemr=%d\n",l34param->TosRemr);
			l34param->Dot1PRemr = (U32_value(nextToken)&0x10000000)?1:0;
			rtlglue_printf("Dot1PRemr=%d\n",l34param->Dot1PRemr);
			l34param->OTagIf = (U32_value(nextToken)&0x8000000)?1:0;
			rtlglue_printf("OTagIf=%d\n",l34param->OTagIf);
			l34param->IPppoeIf = (U32_value(nextToken)&0x4000000)?1:0;
			rtlglue_printf("IPppoeIf=%d\n",l34param->IPppoeIf);
			l34param->OPppoeIf = (U32_value(nextToken)&0x2000000)?1:0;
			rtlglue_printf("OPppoeIf=%d\n",l34param->OPppoeIf);
			l34param->IVlanIDChk = (U32_value(nextToken)&0x1000000)?1:0;
			rtlglue_printf("IVlanIDChk=%d\n",l34param->IVlanIDChk);
			l34param->IVlanID=(U32_value(nextToken)>>12)&0xfff;
			rtlglue_printf("IVlanID=%d\n",l34param->IVlanID);
			l34param->OVlanID=U32_value(nextToken)&0xfff;
			rtlglue_printf("OVlanID=%d\n",l34param->OVlanID);

			
		}
		else
		{
			rtlglue_printf("lastdword error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	return SUCCESS;

}

int rtl8672_parse_l2token(rtl8672_tblAsicDrv_l2Param_t *l2param,int8 **saved)
{
	int8 *nextToken;
	int32 size;
	int i;
	
	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "mac") != 0) return FAILED;

		for(i=0;i<6;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l2param->MAC[i] = U32_value(nextToken);				
			}
			else
			{
				rtlglue_printf("mac addr error\n");
				return FAILED;
			}
		}

		rtlglue_printf("MAC=%02x-%02x-%02x-%02x-%02x-%02x\n",l2param->MAC[0] ,l2param->MAC[1] ,l2param->MAC[2] ,l2param->MAC[3] ,l2param->MAC[4] ,l2param->MAC[5] );
	} 
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "vid") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->VlanID = U32_value(nextToken);	
			rtlglue_printf("VID=%d\n",l2param->VlanID);
		}
		else
		{
			rtlglue_printf("vid error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}
	

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "port") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->Port = U32_value(nextToken);	
			rtlglue_printf("Port=%d\n",l2param->Port);
		}
		else
		{
			rtlglue_printf("port error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "age") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->Age = U32_value(nextToken);	
			rtlglue_printf("Age=%d\n",l2param->Age);
		}
		else
		{
			rtlglue_printf("age error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}		
	

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "pidr") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->PriorityIDRemr = U32_value(nextToken);	
			rtlglue_printf("PriorityIDRemr=%d\n",l2param->PriorityIDRemr);
		}
		else
		{
			rtlglue_printf("PriorityIDRemr error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "otagif") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->OTagIf = U32_value(nextToken);	
			rtlglue_printf("OTagIf=%d\n",l2param->OTagIf);
		}
		else
		{
			rtlglue_printf("OTagIf error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}		

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "sadrop") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->SADrop = U32_value(nextToken);	
			rtlglue_printf("SADrop=%d\n",l2param->SADrop);
		}
		else
		{
			rtlglue_printf("SADrop error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "1premr") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->Dot1PRemr = U32_value(nextToken);	
			rtlglue_printf("Dot1PRemr=%d\n",l2param->Dot1PRemr);
		}
		else
		{
			rtlglue_printf("Dot1PRemr error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}

	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "valid") != 0) return FAILED;
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
		{
			l2param->Valid = U32_value(nextToken);	
			rtlglue_printf("Valid=%d\n",l2param->Valid);
		}
		else
		{
			rtlglue_printf("Valid error\n");
			return FAILED;
		}		
	} 		
	else 
	{
		return FAILED;
	}		


	if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
	{
		if (strcmp(nextToken, "apmac") != 0) return FAILED;

		for(i=0;i<6;i++)
		{
			if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) 
			{
				l2param->APMAC[i] = U32_value(nextToken);				
			}
			else
			{
				rtlglue_printf("APMAC addr error\n");
				return FAILED;
			}
		}

		rtlglue_printf("APMAC=%02x-%02x-%02x-%02x-%02x-%02x\n",l2param->APMAC[0] ,l2param->APMAC[1] ,l2param->APMAC[2] ,l2param->APMAC[3] ,l2param->APMAC[4] ,l2param->APMAC[5] );
	} 
	else 
	{
		return FAILED;
	}

	return SUCCESS;
}

int32   rtl8672_addCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	enum IC_TYPE ictype;

	
	
	model_getTestTarget( &ictype );
	rtlglue_printf(" Current State :" );
	switch (ictype)
	{
	    case IC_TYPE_REAL:
	        rtlglue_printf("   IC_REAL\n" );
	        break;
	    case IC_TYPE_MODEL:
	        rtlglue_printf("   MODEL\n" );
	        break;
	    default:
	        rtlglue_printf("   WARNING ....................................\n" );
	}

	cle_getNextCmdToken(&nextToken,&size,saved);

	if (strcmp(nextToken, "l2") == 0) {
		rtl8672_tblAsicDrv_l2Param_t l2param;
		
		if(rtl8672_parse_l2token(&l2param,saved)==FAILED) return FAILED;

		if(rtl8672_addAsicL2Entry(&l2param,OPT_SRAMONLY)==SUCCESS) 
		{
			rtlglue_printf("hashidx in SRAM=%d\n",rtl8672_L2Hash_Sram(&l2param.MAC[0], l2param.VlanID));	
		}
		else
		{
			if(rtl8672_addAsicL2Entry(&l2param,OPT_SDRAMONLY)==FAILED) return FAILED;
			rtlglue_printf("hashidx in SDRAM=%d\n",rtl8672_L2Hash(&l2param.MAC[0], l2param.VlanID));
		}
		
	}
	else	if (strcmp(nextToken, "l34") == 0) {

		rtl8672_tblAsicDrv_l4Param_t l34param;
		
		if(rtl8672_parse_l34token(&l34param,saved)==FAILED) return FAILED;


		if(rtl8672_addAsicL4Entry(&l34param,OPT_SRAMONLY)==SUCCESS) 
		{
			rtlglue_printf("hashidx in SRAM=%d\n",rtl8672_L4Hash_Sram(l34param.Proto, l34param.SrcIP, l34param.DstIP, l34param.SrcPort, l34param.DstPort));	
		}
		else
		{
			if(rtl8672_addAsicL4Entry(&l34param,OPT_SDRAMONLY)==FAILED) return FAILED;
			rtlglue_printf("hashidx in SDRAM=%d\n",rtl8672_L4Hash(l34param.Proto, l34param.SrcIP, l34param.DstIP, l34param.SrcPort, l34param.DstPort));	
		}	}

	return SUCCESS;	

}



int32   rtl8672_delCmd(uint32 userId,  int32 argc,int8 **saved){
	int8 *nextToken;
	int32 size;
	enum IC_TYPE ictype;

	
	
	model_getTestTarget( &ictype );
	rtlglue_printf(" Current State :" );
	switch (ictype)
	{
	    case IC_TYPE_REAL:
	        rtlglue_printf("   IC_REAL\n" );
	        break;
	    case IC_TYPE_MODEL:
	        rtlglue_printf("   MODEL\n" );
	        break;
	    default:
	        rtlglue_printf("   WARNING ....................................\n" );
	}

	cle_getNextCmdToken(&nextToken,&size,saved);

	if (strcmp(nextToken, "l2") == 0) {
		rtl8672_tblAsicDrv_l2Param_t l2param;
		if(rtl8672_parse_l2token(&l2param,saved)==FAILED) return FAILED;
		if(rtl8672_delAsicL2Entry(&l2param,OPT_AUTO)==FAILED) 
		{
			return FAILED;			
		}		
	}

	if (strcmp(nextToken, "l34") == 0) {
		rtl8672_tblAsicDrv_l4Param_t l34param;
		if(rtl8672_parse_l34token(&l34param,saved)==FAILED) return FAILED;
		if(rtl8672_delAsicL4Entry(&l34param,OPT_AUTO)==FAILED) 
		{
			return FAILED;			
		}		
	}

	return SUCCESS;	

}
#endif
//tylo, temp removed.
void hsdump(void){
	hsb_param_t *hsb;
	hsa_param_t *hsa;
	rtl8672_tblAsic_hsbpTable_t *hsbp;
	hsb=(hsb_param_t *)HSB_BASE;
	hsa=(hsa_param_t *)HSA_BASE;	
	hsbp = (rtl8672_tblAsic_hsbpTable_t *)HSBP_BASE;
	dumpHSB(hsb);
	dumpHSA(hsa);
	dumpHSBP(hsbp);
}
void l34dump(int idx){
		int32 retval;
		uint32 way = 0;
		//uint32 idx;
		uint32 ale_l4enable;
		uint32 ale_l4hib, ale_l4idx, ale_l4way;
		rtl8672_tblAsicDrv_l4Param_t l4p;
		
				
		rtlglue_printf("----- [Dump L4 Table] idx = %04d ----------------------------------------------------------------------------------------------------------------- \n", idx);
		rtlglue_printf("         v Pt SrcIP:Port    DstIP:Port    Age NewSrcIP:Port NewDstIP:Port DPI NextHop-MAC       Td Pr Tr TOS VC iVID oVID oTI 1Pr PID iPI oPI SID  \n");
		
		/* SRAM */
		if (idx < ALE_L4TABLE_SRAM_SIZE) {
			for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL4Table_Sram(idx, way, &l4p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L4 $%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
						way,
						l4p.Valid,
						l4p.Proto,
						l4p.SrcIP,
						l4p.SrcPort,
						l4p.DstIP,
						l4p.DstPort,
						l4p.Age,
						l4p.NewSrcIP,
						l4p.NewSrcPort,
						l4p.NewDstIP,
						l4p.NewDstPort,
						l4p.DstPortIdx,
						l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
						l4p.TtlDe,
						l4p.PrecedRemr,
						l4p.TosRemr,
						l4p.Tos,
						l4p.IVlanIDChk,
						l4p.IVlanID,
						l4p.OVlanID,
						l4p.OTagIf,
						l4p.Dot1PRemr,
						l4p.PriorityID,
						l4p.IPppoeIf,
						l4p.OPppoeIf,
						l4p.SessionID);
				}
			}
		}
		else
		{
			int hash;
			for(hash=0;hash<ALE_L4TABLE_SRAM_SIZE;hash++)
			{			
				for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++) 
				{
					retval = rtl8672_getAsicL4Table_Sram(hash, way, &l4p);
					if ((retval == SUCCESS) &&(l4p.Valid==1))
					{
						rtlglue_printf("[L4 $%02d:%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
							hash,
							way,
							l4p.Valid,
							l4p.Proto,
							l4p.SrcIP,
							l4p.SrcPort,
							l4p.DstIP,
							l4p.DstPort,
							l4p.Age,
							l4p.NewSrcIP,
							l4p.NewSrcPort,
							l4p.NewDstIP,
							l4p.NewDstPort,
							l4p.DstPortIdx,
							l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
							l4p.TtlDe,
							l4p.PrecedRemr,
							l4p.TosRemr,
							l4p.Tos,
							l4p.IVlanIDChk,
							l4p.IVlanID,
							l4p.OVlanID,
							l4p.OTagIf,
							l4p.Dot1PRemr,
							l4p.PriorityID,
							l4p.IPppoeIf,
							l4p.OPppoeIf,
							l4p.SessionID);
					}
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL4Ability(&ale_l4enable);
		rtl8672_getAsicL4HashIdxBits(&ale_l4hib);
		rtl8672_getAsicL4Way(&ale_l4way);
		ale_l4idx = (0x1 << ale_l4hib);
		if (ale_l4enable && (idx < ale_l4idx)) {
			for (way = 0; way < ale_l4way; way++) {
				retval = rtl8672_getAsicL4Table(idx, way, &l4p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L4 #%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
						way,
						l4p.Valid,
						l4p.Proto,
						l4p.SrcIP,
						l4p.SrcPort,
						l4p.DstIP,
						l4p.DstPort,
						l4p.Age,
						l4p.NewSrcIP,
						l4p.NewSrcPort,
						l4p.NewDstIP,
						l4p.NewDstPort,
						l4p.DstPortIdx,
						l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
						l4p.TtlDe,
						l4p.PrecedRemr,
						l4p.TosRemr,
						l4p.Tos,
						l4p.IVlanIDChk,
						l4p.IVlanID,
						l4p.OVlanID,
						l4p.OTagIf,
						l4p.Dot1PRemr,
						l4p.PriorityID,
						l4p.IPppoeIf,
						l4p.OPppoeIf,
						l4p.SessionID);
				}
			}
		}
		
		rtlglue_printf("-------------------------------------------------------------------------------------------------------------------------------------------------- \n");
		
		return ;
}
void intfdump(void){
		int idx;
		rtl8672_tblAsicDrv_intfParam_t intf;	/* Source Interface */
		idx=0;
		rtl8672_getAsicNetInterface(idx, &intf);

		rtlglue_printf("----- [Dump Interface Table] interface idx = %02d (%s) -----------------------\n", idx,(idx<SAR_INTFS)?"SAR":((idx<SAR_INTFS+MAC_TX_INTFS)?"MAC":"EXT"));
		rtlglue_printf("ATMPORT=%x, TRLREN=%x, CLP=%x PTI=%x TRLR=0x%x GMAC=%02x:%02x:%02x:%02x:%02x:%02x\n" , intf.ATMPORT, intf.TRLREN, intf.CLP, intf.PTI, intf.TRLR,intf.GMAC[0],intf.GMAC[1],intf.GMAC[2],intf.GMAC[3],intf.GMAC[4],intf.GMAC[5]);
		rtlglue_printf("L2Encap=%x, LanFCS=%x CompPPP=%x IfType=%x SARhdr=%x RXshift=0x%x\n" , intf.L2Encap, intf.LanFCS, intf.CompPPP, intf.IfType, intf.SARhdr, intf.RXshift);
		rtlglue_printf("GIP=%d.%d.%d.%d, AcceptTagged=%x AcceptUntagged=%x PortPriorityID=%x\n" , NIPQUAD(intf.GIP), intf.AcceptTagged, intf.AcceptUntagged, intf.PortPriorityID);
		rtlglue_printf("SrcPortFilter=%d L2Bridge=%d 1QREMR=%d LogicalID=%d PortVlanId=%d\n",intf.SrcPortFilter,intf.L2BridgeEnable,intf.Dot1QRemr,intf.LogicalID,intf.PortVlanID);
		rtlglue_printf("MTU=%d, VlanIDRemr=0x%x\n" , intf.MTU, intf.VlanIDRemr);
		//memDump((void *)IFTABLE_BASE+sizeof(struct rtl8672_tblAsic_intfTable_s)*idx,sizeof(rtl8672_tblAsicDrv_intfParam_t),"Content");
		rtlglue_printf("-----------------------------------------------------------------------------\n");

		idx=9;
		rtl8672_getAsicNetInterface(idx, &intf);

		rtlglue_printf("----- [Dump Interface Table] interface idx = %02d (%s) -----------------------\n", idx,(idx<SAR_INTFS)?"SAR":((idx<SAR_INTFS+MAC_TX_INTFS)?"MAC":"EXT"));
		rtlglue_printf("ATMPORT=%x, TRLREN=%x, CLP=%x PTI=%x TRLR=0x%x GMAC=%02x:%02x:%02x:%02x:%02x:%02x\n" , intf.ATMPORT, intf.TRLREN, intf.CLP, intf.PTI, intf.TRLR,intf.GMAC[0],intf.GMAC[1],intf.GMAC[2],intf.GMAC[3],intf.GMAC[4],intf.GMAC[5]);
		rtlglue_printf("L2Encap=%x, LanFCS=%x CompPPP=%x IfType=%x SARhdr=%x RXshift=0x%x\n" , intf.L2Encap, intf.LanFCS, intf.CompPPP, intf.IfType, intf.SARhdr, intf.RXshift);
		rtlglue_printf("GIP=%d.%d.%d.%d, AcceptTagged=%x AcceptUntagged=%x PortPriorityID=%x\n" , NIPQUAD(intf.GIP), intf.AcceptTagged, intf.AcceptUntagged, intf.PortPriorityID);
		rtlglue_printf("SrcPortFilter=%d L2Bridge=%d 1QREMR=%d LogicalID=%d PortVlanId=%d\n",intf.SrcPortFilter,intf.L2BridgeEnable,intf.Dot1QRemr,intf.LogicalID,intf.PortVlanID);
		rtlglue_printf("MTU=%d, VlanIDRemr=0x%x\n" , intf.MTU, intf.VlanIDRemr);
		//memDump((void *)IFTABLE_BASE+sizeof(struct rtl8672_tblAsic_intfTable_s)*idx,sizeof(rtl8672_tblAsicDrv_intfParam_t),"Content");
		rtlglue_printf("-----------------------------------------------------------------------------\n");


}
void macdump(void){
		int32 retval;
		uint32 way = 0;
		uint32 idx, port_idx;
		uint32 ale_l2enable;
		uint32 ale_l2hib, ale_l2idx, ale_l2way;
		rtl8672_tblAsicDrv_l2Param_t l2p;
		
			port_idx = 8;
		
		rtlglue_printf("----- [Dump MAC Table] interface idx = %01x --------------------------------------------- \n", port_idx);
		rtlglue_printf("             v MAC               VID Port Age SADrop oTagIf 1PRemr PIDR APMAC             \n");
		
		/* SRAM */
		for(idx = 0; idx < ALE_L2TABLE_SRAM_SIZE; idx++) {
			for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL2Table_Sram(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d$%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL2Ability(&ale_l2enable);
		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2idx = (0x1 << ale_l2hib);
		for (idx = 0; ale_l2enable && (idx < ale_l2idx); idx++) {
			for (way = 0; way < ale_l2way; way++) {
				retval = rtl8672_getAsicL2Table(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d#%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		rtlglue_printf("----------------------------------------------------------------------------------------- \n");


		port_idx = 9;
		
		rtlglue_printf("----- [Dump MAC Table] interface idx = %01x --------------------------------------------- \n", port_idx);
		rtlglue_printf("             v MAC               VID Port Age SADrop oTagIf 1PRemr PIDR APMAC             \n");
		
		/* SRAM */
		for(idx = 0; idx < ALE_L2TABLE_SRAM_SIZE; idx++) {
			for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL2Table_Sram(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d$%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL2Ability(&ale_l2enable);
		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2idx = (0x1 << ale_l2hib);
		for (idx = 0; ale_l2enable && (idx < ale_l2idx); idx++) {
			for (way = 0; way < ale_l2way; way++) {
				retval = rtl8672_getAsicL2Table(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d#%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		rtlglue_printf("----------------------------------------------------------------------------------------- \n");

		//return SUCCESS;

}
int32   rtl8672_dumpCmd(uint32 userId,  int32 argc,int8 **saved){
#if 0
	int8 *nextToken;
	int32 size;
	enum IC_TYPE ictype;

	model_getTestTarget( &ictype );
	rtlglue_printf(" Current State :" );
	switch (ictype)
	{
	    case IC_TYPE_REAL:
	        rtlglue_printf("   IC_REAL\n" );
	        break;
	    case IC_TYPE_MODEL:
	        rtlglue_printf("   MODEL\n" );
	        break;
	    default:
	        rtlglue_printf("   WARNING ....................................\n" );
	}

	cle_getNextCmdToken(&nextToken,&size,saved);
	if ( 0 )
	{
	    // never been here ....
	    // just for 'else if' statements ....
	}
	else if (strcmp(nextToken, "hs") == 0) {
	hsb_param_t *hsb;
	hsa_param_t *hsa;
	rtl8672_tblAsic_hsbpTable_t *hsbp;
	hsb=(hsb_param_t *)HSB_BASE;
	hsa=(hsa_param_t *)HSA_BASE;	
	hsbp = (rtl8672_tblAsic_hsbpTable_t *)HSBP_BASE;
	dumpHSB(hsb);
	dumpHSA(hsa);
	dumpHSBP(hsbp);

	}
	else if(strcmp(nextToken, "intf") == 0)
	{
		int idx;
		rtl8672_tblAsicDrv_intfParam_t intf;	/* Source Interface */
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			idx = U32_value(nextToken);
		} else {
			return FAILED;
		}

		rtl8672_getAsicNetInterface(idx, &intf);

		rtlglue_printf("----- [Dump Interface Table] interface idx = %02d (%s) -----------------------\n", idx,(idx<SAR_INTFS)?"SAR":((idx<SAR_INTFS+MAC_TX_INTFS)?"MAC":"EXT"));
		rtlglue_printf("ATMPORT=%x, TRLREN=%x, CLP=%x PTI=%x TRLR=0x%x GMAC=%02x:%02x:%02x:%02x:%02x:%02x\n" , intf.ATMPORT, intf.TRLREN, intf.CLP, intf.PTI, intf.TRLR,intf.GMAC[0],intf.GMAC[1],intf.GMAC[2],intf.GMAC[3],intf.GMAC[4],intf.GMAC[5]);
		rtlglue_printf("L2Encap=%x, LanFCS=%x CompPPP=%x IfType=%x SARhdr=%x RXshift=0x%x\n" , intf.L2Encap, intf.LanFCS, intf.CompPPP, intf.IfType, intf.SARhdr, intf.RXshift);
		rtlglue_printf("GIP=%d.%d.%d.%d, AcceptTagged=%x AcceptUntagged=%x PortPriorityID=%x\n" , NIPQUAD(intf.GIP), intf.AcceptTagged, intf.AcceptUntagged, intf.PortPriorityID);
		rtlglue_printf("SrcPortFilter=%d L2Bridge=%d 1QREMR=%d LogicalID=%d PortVlanId=%d\n",intf.SrcPortFilter,intf.L2BridgeEnable,intf.Dot1QRemr,intf.LogicalID,intf.PortVlanID);
		rtlglue_printf("MTU=%d, VlanIDRemr=0x%x\n" , intf.MTU, intf.VlanIDRemr);
		memDump((void *)IFTABLE_BASE+sizeof(struct rtl8672_tblAsic_intfTable_s)*idx,sizeof(rtl8672_tblAsicDrv_intfParam_t),"Content");
		rtlglue_printf("-----------------------------------------------------------------------------\n");
		
	}
	else if(strcmp(nextToken, "mac") == 0)	/* MAC Table */
	{
		int32 retval;
		uint32 way = 0;
		uint32 idx, port_idx;
		uint32 ale_l2enable;
		uint32 ale_l2hib, ale_l2idx, ale_l2way;
		rtl8672_tblAsicDrv_l2Param_t l2p;
		
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			port_idx = U32_value(nextToken);
		} else {
			return FAILED;
		}
		
		rtlglue_printf("----- [Dump MAC Table] interface idx = %01x --------------------------------------------- \n", port_idx);
		rtlglue_printf("             v MAC               VID Port Age SADrop oTagIf 1PRemr PIDR APMAC             \n");
		
		/* SRAM */
		for(idx = 0; idx < ALE_L2TABLE_SRAM_SIZE; idx++) {
			for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL2Table_Sram(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d$%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL2Ability(&ale_l2enable);
		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2idx = (0x1 << ale_l2hib);
		for (idx = 0; ale_l2enable && (idx < ale_l2idx); idx++) {
			for (way = 0; way < ale_l2way; way++) {
				retval = rtl8672_getAsicL2Table(idx, way, &l2p);
				if (retval == SUCCESS && l2p.Valid == 1 && l2p.Port == port_idx) {
					rtlglue_printf("[L2 %04d#%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						idx,
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		rtlglue_printf("----------------------------------------------------------------------------------------- \n");
		
		return SUCCESS;

	}
	else if(strcmp(nextToken, "l2") == 0)	/* SDRam L2 Table */
	{
		int32 retval;
		uint32 way = 0;
		uint32 idx;
		uint32 ale_l2enable;
		uint32 ale_l2hib, ale_l2idx, ale_l2way;
		rtl8672_tblAsicDrv_l2Param_t l2p;
		
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			idx = U32_value(nextToken);
		} else {
			return FAILED;
		}
		
		rtlglue_printf("----- [Dump L2 Table] idx = %04d ---------------------------------------------------- \n", idx);
		rtlglue_printf("         v MAC               VID Port Age SADrop oTagIf 1PRemr PIDR APMAC             \n");
		
		/* SRAM */
		if (idx < ALE_L2TABLE_SRAM_SIZE) {
			for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL2Table_Sram(idx, way, &l2p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L2 $%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		else
		{
			int hash;
			for(hash=0;hash<ALE_L2TABLE_SRAM_SIZE;hash++)
			{
				for (way = 0; way < ALE_L2TABLE_SRAM_WAY; way++) 
				{
					retval = rtl8672_getAsicL2Table_Sram(hash, way, &l2p);
					if ((retval == SUCCESS)&&(l2p.Valid==1)) 
					{
						rtlglue_printf("[L2 $%02d:%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
							hash,
							way,
							l2p.Valid,
							l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
							l2p.VlanID,
							l2p.Port,
							l2p.Age,
							l2p.SADrop,
							l2p.OTagIf,
							l2p.Dot1PRemr,
							l2p.PriorityIDRemr,
							l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
					}
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL2Ability(&ale_l2enable);
		rtl8672_getAsicL2HashIdxBits(&ale_l2hib);
		rtl8672_getAsicL2Way(&ale_l2way);
		ale_l2idx = (0x1 << ale_l2hib);
		if (ale_l2enable && (idx < ale_l2idx)) {
			for (way = 0; way < ale_l2way; way++) {
				retval = rtl8672_getAsicL2Table(idx, way, &l2p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L2 #%02d] %01x %02x:%02x:%02x:%02x:%02x:%02x %03x %01x    %01x   %01x      %01x      %01x      %01x    %02x:%02x:%02x:%02x:%02x:%02x \n",
						way,
						l2p.Valid,
						l2p.MAC[0], l2p.MAC[1], l2p.MAC[2], l2p.MAC[3], l2p.MAC[4], l2p.MAC[5],
						l2p.VlanID,
						l2p.Port,
						l2p.Age,
						l2p.SADrop,
						l2p.OTagIf,
						l2p.Dot1PRemr,
						l2p.PriorityIDRemr,
						l2p.APMAC[0], l2p.APMAC[1], l2p.APMAC[2], l2p.APMAC[3], l2p.APMAC[4], l2p.APMAC[5]);
				}
			}
		}
		
		rtlglue_printf("------------------------------------------------------------------------------------- \n");
		
		return SUCCESS;
	}
	else if(strcmp(nextToken, "l34") == 0)	/* SDRam L4 Table */
	{
		int32 retval;
		uint32 way = 0;
		uint32 idx;
		uint32 ale_l4enable;
		uint32 ale_l4hib, ale_l4idx, ale_l4way;
		rtl8672_tblAsicDrv_l4Param_t l4p;
		
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			idx = U32_value(nextToken);
		} else {
			return FAILED;
		}
		
		rtlglue_printf("----- [Dump L4 Table] idx = %04d ----------------------------------------------------------------------------------------------------------------- \n", idx);
		rtlglue_printf("         v Pt SrcIP:Port    DstIP:Port    Age NewSrcIP:Port NewDstIP:Port DPI NextHop-MAC       Td Pr Tr TOS VC iVID oVID oTI 1Pr PID iPI oPI SID  \n");
		
		/* SRAM */
		if (idx < ALE_L4TABLE_SRAM_SIZE) {
			for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++) {
				retval = rtl8672_getAsicL4Table_Sram(idx, way, &l4p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L4 $%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
						way,
						l4p.Valid,
						l4p.Proto,
						l4p.SrcIP,
						l4p.SrcPort,
						l4p.DstIP,
						l4p.DstPort,
						l4p.Age,
						l4p.NewSrcIP,
						l4p.NewSrcPort,
						l4p.NewDstIP,
						l4p.NewDstPort,
						l4p.DstPortIdx,
						l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
						l4p.TtlDe,
						l4p.PrecedRemr,
						l4p.TosRemr,
						l4p.Tos,
						l4p.IVlanIDChk,
						l4p.IVlanID,
						l4p.OVlanID,
						l4p.OTagIf,
						l4p.Dot1PRemr,
						l4p.PriorityID,
						l4p.IPppoeIf,
						l4p.OPppoeIf,
						l4p.SessionID);
				}
			}
		}
		else
		{
			int hash;
			for(hash=0;hash<ALE_L4TABLE_SRAM_SIZE;hash++)
			{			
				for (way = 0; way < ALE_L4TABLE_SRAM_WAY; way++) 
				{
					retval = rtl8672_getAsicL4Table_Sram(hash, way, &l4p);
					if ((retval == SUCCESS) &&(l4p.Valid==1))
					{
						rtlglue_printf("[L4 $%02d:%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
							hash,
							way,
							l4p.Valid,
							l4p.Proto,
							l4p.SrcIP,
							l4p.SrcPort,
							l4p.DstIP,
							l4p.DstPort,
							l4p.Age,
							l4p.NewSrcIP,
							l4p.NewSrcPort,
							l4p.NewDstIP,
							l4p.NewDstPort,
							l4p.DstPortIdx,
							l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
							l4p.TtlDe,
							l4p.PrecedRemr,
							l4p.TosRemr,
							l4p.Tos,
							l4p.IVlanIDChk,
							l4p.IVlanID,
							l4p.OVlanID,
							l4p.OTagIf,
							l4p.Dot1PRemr,
							l4p.PriorityID,
							l4p.IPppoeIf,
							l4p.OPppoeIf,
							l4p.SessionID);
					}
				}
			}
		}
		
		/* SDRAM */
		rtl8672_getAsicL4Ability(&ale_l4enable);
		rtl8672_getAsicL4HashIdxBits(&ale_l4hib);
		rtl8672_getAsicL4Way(&ale_l4way);
		ale_l4idx = (0x1 << ale_l4hib);
		if (ale_l4enable && (idx < ale_l4idx)) {
			for (way = 0; way < ale_l4way; way++) {
				retval = rtl8672_getAsicL4Table(idx, way, &l4p);
				if (retval == SUCCESS) {
					rtlglue_printf("[L4 #%02d] %01x %02x %08x:%04x %08x:%04x %01x   %08x:%04x %08x:%04x %01x   %02x:%02x:%02x:%02x:%02x:%02x %01x  %01x  %01x  %02x  %01x  %03x  %03x  %01x   %01x   %01x   %01x   %01x   %04x \n",
						way,
						l4p.Valid,
						l4p.Proto,
						l4p.SrcIP,
						l4p.SrcPort,
						l4p.DstIP,
						l4p.DstPort,
						l4p.Age,
						l4p.NewSrcIP,
						l4p.NewSrcPort,
						l4p.NewDstIP,
						l4p.NewDstPort,
						l4p.DstPortIdx,
						l4p.NHMAC[0], l4p.NHMAC[1], l4p.NHMAC[2], l4p.NHMAC[3], l4p.NHMAC[4], l4p.NHMAC[5],
						l4p.TtlDe,
						l4p.PrecedRemr,
						l4p.TosRemr,
						l4p.Tos,
						l4p.IVlanIDChk,
						l4p.IVlanID,
						l4p.OVlanID,
						l4p.OTagIf,
						l4p.Dot1PRemr,
						l4p.PriorityID,
						l4p.IPppoeIf,
						l4p.OPppoeIf,
						l4p.SessionID);
				}
			}
		}
		
		rtlglue_printf("-------------------------------------------------------------------------------------------------------------------------------------------------- \n");
		
		return SUCCESS;
	}
	else if(strcmp(nextToken, "ring") == 0)
	{
		int ring,idx;
	
		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			ring = U32_value(nextToken);
		} else {
			return FAILED;
		}		

		if ( cle_getNextCmdToken(&nextToken,&size,saved) == SUCCESS ) {
			idx = U32_value(nextToken);
		} else {
			return FAILED;
		}
		return rtl8672_dumpRing(ring,idx);
	}
#endif
	return SUCCESS;

}


