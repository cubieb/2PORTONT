
#include <stdarg.h>
#include <rtk_rg_internal.h>
#include <rtk_rg_debug.h>


#define COLOR_Y "\033[1;33m"
#define COLOR_NM "\033[0m"
#define COLOR_H "\033[1;37;41m"
#define COLOR_G "\033[1;32m"


#ifdef CONFIG_RG_DEBUG
#include <rtk_rg_acl.h>
#include <rtk_rg_liteRomeDriver.h>
//#include <rtk_rg_internal.h>
#include <rtk_rg_fwdEngine.h>
#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
//#include <rtk_rg_debug.h>
#include <rtk_rg_alg_tool.h>
#include <rtk_rg_callback.h>

#if defined(CONFIG_APOLLO)
#include <rtk/acl.h>
#include <rtk/classify.h>
#include <rtk/l2.h>
#include <rtk/l34.h>
#include <rtk/qos.h>
#include <rtk/svlan.h>
#include <ioal/mem32.h>


#if defined(CONFIG_RTL9600_SERIES)
#include <dal/apollomp/raw/apollomp_raw_hwmisc.h>
#elif defined(CONFIG_RTL9602C_SERIES)
#include <dal/rtl9602c/dal_rtl9602c_hwmisc.h>
#endif


#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) &&  defined(CONFIG_XDSL_ROMEDRIVER)
/*rtx865x Boyce 2014-07-18*/
//#include <rtk_rg_xdsl_extAPI.h>
#include <rtk_rg_xdsl_struct.h>
#endif

#include <linux/proc_fs.h>
#include <asm/stacktrace.h> //for dump_stack()


#ifdef CONFIG_RG_WMUX_SUPPORT
#include <rtk_rg_wmux.h>
#endif


#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
#include <rtk_rg_igmpsnooping.h>
extern struct rtl_mCastTimerParameters rtl_mCastTimerParas; 
extern struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];
#endif


#if defined(CONFIG_RTL9600_SERIES)
#define DIAG_UTIL_CHIP_TYPE APOLLOMP_CHIP_ID
#elif defined(CONFIG_RTL9602C_SERIES)
#define DIAG_UTIL_CHIP_TYPE RTL9602C_CHIP_ID
#endif


void _rtk_rg_dump_stack(void)
{
	struct pt_regs regs;
	unsigned long sp,ra,pc;
//	prepare_frametrace(&regs);

    memset(&regs, 0, sizeof(regs));

	__asm__ __volatile__(
	".set push\n\t"
	".set noat\n\t"
	"1: la $1, 1b\n\t"
	"sw $1, %0\n\t"
	"sw $29, %1\n\t"
	"sw $31, %2\n\t"
	".set pop\n\t"
	: "=m" (regs.cp0_epc),
	"=m" (regs.regs[29]), "=m" (regs.regs[31])
	: : "memory");

	
	sp = regs.regs[29];
	ra = regs.regs[31];
	pc = regs.cp0_epc;
	
	if (!__kernel_text_address(pc)) 
	{
		return;
	}	
	
	rtlglue_printf("\033[1;33;41m");
	pc = unwind_stack(current, &sp, pc, &ra);
	while(1)
	{
		if(!pc) break;
		pc = unwind_stack(current, &sp, pc, &ra);
		if(!pc) break;
		rtlglue_printf("[%pS]", (void *) pc);		
		//printk("[%p:%pS]\n", (void *) pc, (void *) pc);		
	}
	rtlglue_printf("\033[0m\n");
}



void assert_ok_sub_func(int assert_ret, char *assert_str,const char *func,int line)
{ 
	if(assert_ret!=0) 
	{ 
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) 
		{
        	rtlglue_printf("\033[1;33;41m[ASSERT_FAIL]'%s=0x%x'\033[1;30m @%s:%d\033[0m\n",assert_str,assert_ret,func,line);
#ifdef CONFIG_RG_DEBUG			
			_rtk_rg_dump_stack();	
#endif
		}
	}
}

void assert_sub_func(int assert_ret, char *assert_str,const char *func,int line)
{
	if(!(assert_ret)) 
	{
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) 
		{
			rtlglue_printf("\033[1;33;41m[ASSERT]'%s'\033[1;30m @%s:%d\033[0m\n", assert_str,func,line);
#ifdef CONFIG_RG_DEBUG			
			_rtk_rg_dump_stack();
#endif
		}
	}
}




//#define diag_util_inet_mactoa mactoa
int8 *diag_util_inet_mactoa (const uint8 *mac)
{
        static int8 str[6*sizeof "123"];

    if (NULL == mac)
    {
        sprintf(str,"NULL");
        return str;
    }

    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return str;
} /* end of diag_util_mac2str */

/*IPv4 address to string*/
int8 *diag_util_inet_ntoa(uint32 ina)
{	
	static int8 buf[4*sizeof "123"];    
	sprintf(buf, "%d.%d.%d.%d", ((ina>>24)&0xff), ((ina>>16)&0xff), ((ina>>8)&0xff), (ina&0xff));	
	return (buf);
}

/*IPv6 address to string*/
int8 *diag_util_inet_n6toa(const uint8 *ipv6)
{
	static int8 buf[8*sizeof "FFFF:"];
    uint32  i;
    uint16  ipv6_ptr[8] = {0};

    for (i = 0; i < 8 ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }

    sprintf(buf, "%X:%X:%X:%X:%X:%X:%X:%X", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);
	return (buf);
}

#if defined(CONFIG_APOLLO) 
#define diag_util_printf(fmt, args...)    printk( fmt, ## args)
#include <diag_display.h>
#endif

#if defined(CONFIG_APOLLO)

/*internal utils*/
#define inet_ntoa_r(x,y) _inet_ntoa_r(x,y)
#define inet_ntoa(x) _inet_ntoa(x)


static inline int8 *_ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}

static int8 *_inet_ntoa_r(ipaddr_t ipaddr, 	int8 *p)
{
	uint8 *ucp = (unsigned char *)&ipaddr;
	assert(p!=NULL);
	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';
	return (p);
}

static int8 *_inet_ntoa(rtk_ip_addr_t ina)
{
	static int8 buf[4*sizeof "123"];
	int8 *p = buf;
	uint8 *ucp = (unsigned char *)&ina;

	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';

	return (buf);
}
#endif //defined(CONFIG_APOLLO)

#if defined(CONFIG_APOLLO)

/*internal function*/
void hs_displayL4Hsb_S(rtk_l34_hsb_t * hsbWatch,struct seq_file *s)
{
    //rtk_ip_addr_t addr;

    // PROC_PRINTF("L4HSB(");
    PROC_PRINTF("\tspa:%d\n",hsbWatch->spa);
    PROC_PRINTF("\tda:%02x-%02x-%02x-%02x-%02x-%02x\n",hsbWatch->dmac.octet[0],hsbWatch->dmac.octet[1],hsbWatch->dmac.octet[2],hsbWatch->dmac.octet[3],hsbWatch->dmac.octet[4],hsbWatch->dmac.octet[5]);
    //addr =ntohl( hsbWatch->sip);
#ifdef CONFIG_APOLLO_TESTING
    PROC_PRINTF("\tsip:%s(hex:%08x)   ",inet_ntoa(addr),hsbWatch->sip);
#else
    //PROC_PRINTF("\tsip:%08x ",hsbWatch->sip);
	PROC_PRINTF("\tsip:");

    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[0]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[1]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[2]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[3]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[4]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[5]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[6]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[7]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[8]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[9]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[10]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[11]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[12]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[13]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[14]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[15]);
#endif
#if defined(CONFIG_RTL9602C_SERIES)
    PROC_PRINTF("\tis_policy:%d policy_nh_idx:%d\n ",(int)hsbWatch->is_policy_route,hsbWatch->policy_nh_idx);
#endif
    PROC_PRINTF("\tsprt:%d(hex:%08x)\n ",(int)hsbWatch->sport_icmpid_chksum,hsbWatch->sport_icmpid_chksum);
    PROC_PRINTF("\tdip:");

    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[0]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[1]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[2]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[3]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[4]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[5]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[6]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[7]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[8]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[9]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[10]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[11]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[12]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[13]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[14]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[15]);

    PROC_PRINTF("\tdprt:%d(hex:%08x)\n",hsbWatch->dport_l4chksum,hsbWatch->dport_l4chksum);

    PROC_PRINTF("\ttype:%d",hsbWatch->type);

    PROC_PRINTF("\tttlst:0x%x\n",hsbWatch->ttls);
    PROC_PRINTF("\tpppoetagif:%d\tpppoeId:%d\n",hsbWatch->pppoe_if,hsbWatch->pppoe_id);
    PROC_PRINTF("\tctagif:%d",hsbWatch->cvlan_if);
    PROC_PRINTF("\tvid :%d\n",hsbWatch->cvid);
    PROC_PRINTF("\tstagif:%d",hsbWatch->svlan_if);
    PROC_PRINTF("\tlen:%d\n",hsbWatch->len);

    PROC_PRINTF("\tudp_nocs:%d",hsbWatch->udp_no_chksum);
    PROC_PRINTF("\ttcpflg:%x\n",hsbWatch->tcp_flag);
    PROC_PRINTF("\tparsfail:%d",hsbWatch->parse_fail);
    PROC_PRINTF("\tdirtx:%d\n",hsbWatch->cpu_direct_tx);
    PROC_PRINTF("\tl3csok:%d\tl4csok:%d\n",hsbWatch->l3_chksum_ok,hsbWatch->l4_chksum_ok);
    PROC_PRINTF("\tipmf:%d\tipfragif:%d\n",hsbWatch->ipmf,hsbWatch->ipfrag_s);
    PROC_PRINTF("\tisFromWan:%d\n",hsbWatch->isFromWan);

//    PROC_PRINTF("\tl2bridge:%d\n)\n",hsbWatch->extl2);
    PROC_PRINTF("\tl2bridge:%d\n",hsbWatch->l2bridge);

#if defined(CONFIG_RTL9602C_SERIES)
	PROC_PRINTF("\tis_policy_route:%d policy_nh_idx:%d\n",hsbWatch->is_policy_route,hsbWatch->policy_nh_idx);
#endif

}

void hs_displayL4Hsa_S(rtk_l34_hsa_t *hsaWatch,struct seq_file *s)
{
    rtk_ip_addr_t addr;

	
//   PROC_PRINTF(("L4HSA("));
    addr =ntohl( hsaWatch->ip);
#ifdef CONFIG_APOLLO_TESTING
	PROC_PRINTF("\ttrip:%s(hex:%08x)",inet_ntoa(addr),hsaWatch->ip);
#else
    PROC_PRINTF("\ttrip:%08x",hsaWatch->ip);
#endif
    PROC_PRINTF("\tprt:%d\n",hsaWatch->port);
    PROC_PRINTF("\tl3cs:0x%x",hsaWatch->l3_chksum);
    PROC_PRINTF("\tl4cs:0x%x\n",hsaWatch->l4_chksum);
    PROC_PRINTF("\tmacidx:%d\n",hsaWatch->nexthop_mac_idx);
    PROC_PRINTF("\tpppif:%d",hsaWatch->pppoe_if);
    PROC_PRINTF("\tpppid:%d",hsaWatch->pppid_idx);
	PROC_PRINTF("\tpppoeKeep:%d\n",hsaWatch->pppoeKeep);
    PROC_PRINTF("\tdvid:%d(0x%x)",hsaWatch->dvid,hsaWatch->dvid);
    PROC_PRINTF("\tdestination interface :%d\n",hsaWatch->difid);

    PROC_PRINTF("\tfrag:%d\n",hsaWatch->frag);
    PROC_PRINTF("\tpriority:%d\tvalid:%d\n",hsaWatch->l4_pri_sel,hsaWatch->l4_pri_valid);
    PROC_PRINTF("\taction:%d\n",hsaWatch->action);
    PROC_PRINTF("\tInternal NETIF:%d",hsaWatch->interVlanIf);
    PROC_PRINTF("\tl2tr:%d",hsaWatch->l2trans);
    PROC_PRINTF("\tl34tr:%d\n",hsaWatch->l34trans);
    PROC_PRINTF("\tbindvidtrans:%d\n",hsaWatch->bindVidTrans);
    PROC_PRINTF("\treason:%d\n",hsaWatch->reason);

//    PROC_PRINTF(")\n");
	return ;


}







extern void _diag_debug_hsb_display(rtk_hsb_t *hsaDatb);
extern void _diag_debug_hsa_display(rtk_hsa_t *hsaData);
extern void _diag_debug_hsd_display(rtk_hsa_debug_t *hsaDatd);


#if defined(CONFIG_RTL9602C_SERIES)

int32 dump_ipmv6_l3mcr_table(struct seq_file *s, void *v)
{
	int i,j;
	int len=0;
	uint32 getVal=0;	
	
	PROC_PRINTF("ASIC IPMv6 L3MCR Table : \n");
	for(i=0;i<MAX_IPMV6_L3MCR_TABL_SIZE;i++){
		PROC_PRINTF("[%d]",i);
		for(j=0;j<RTK_RG_MAX_MAC_PORT;j++){
			getVal=0;
			rtk_l34_ip6mcRoutingTransIdx_get(i,j,&getVal);
			PROC_PRINTF("   port[%d]->idx=%d",j,getVal);	
		}
		PROC_PRINTF("\n");
	}
	return len;

}



int32 dump_dslite_status(struct seq_file *s, void *v)
{
	int len =0;
	uint32 status;
    PROC_PRINTF(">>DSLITE Status:\n\n");
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_DSLITE_STATE,&status);
    PROC_PRINTF("DSLITE Enable:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_MC_PREFIX_UNMATCH,&status);	
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_MC_PREFIX_UNMATCH:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_DS_UNMATCH_ACT,&status);	
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_DS_UNMATCH_ACT:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP6_NH_ACTION,&status);	
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP6_NH_ACTION:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP6_FRAGMENT_ACTION,&status);	
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP6_FRAGMENT_ACTION:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP4_FRAGMENT_ACTION,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP4_FRAGMENT_ACTION:%d \n",status);
	return len;

}


int32 dump_dslite_table(struct seq_file *s, void *v)
{
	int len =0;
	int i,j;
	rtk_l34_dsliteInf_entry_t dslite;
    PROC_PRINTF(">>DSLITE Table:\n\n");
	for(i=0;i<4;i++){
		dslite.index=i;
		rtk_l34_dsliteInfTable_get(&dslite);
		if(dslite.valid){
			PROC_PRINTF("[%d] flowLabel:%x hopLimit:%d tc:%x tcOpt:%s \n",i,dslite.flowLabel,dslite.hopLimit,dslite.tc,dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_ASSIGN?"Assign":"Copy from IPv4 TOS");

			PROC_PRINTF("\tAFTRIP:\t");
			for(j=0;j<8;j++)
				PROC_PRINTF("%02x%02x ",dslite.ipAftr.ipv6_addr[j*2],dslite.ipAftr.ipv6_addr[j*2+1]);
			PROC_PRINTF("\n");

			PROC_PRINTF("\tB4IP:\t");
			for(j=0;j<8;j++)
				PROC_PRINTF("%02x%02x ",dslite.ipB4.ipv6_addr[j*2],dslite.ipB4.ipv6_addr[j*2+1]);
			PROC_PRINTF("\n");
		}
	}

	return len;

}

int32 dump_dsliteMc_table(struct seq_file *s, void *v)
{
	int len=0;
	int i,j;
	rtk_l34_dsliteMc_entry_t dsliteMc;
    PROC_PRINTF(">>DSLITEMC Table:\n\n");
	for(i=0;i<4;i++){
		dsliteMc.index=i;
		rtk_l34_dsliteMcTable_get(&dsliteMc);

		PROC_PRINTF("[%d] \n",i);
		PROC_PRINTF("\tipMPrefix64:\t\t");
		for(j=0;j<8;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipMPrefix64.ipv6_addr[j*2],dsliteMc.ipMPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("\n");
		
		PROC_PRINTF("\tipMPrefix64Mask:\t");
		for(j=0;j<8;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2],dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("\n");
		
		PROC_PRINTF("\tipUPrefix64:\t\t");
		for(j=0;j<8;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipUPrefix64.ipv6_addr[j*2],dsliteMc.ipUPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("\n");
		
		PROC_PRINTF("\tipUPrefix64Mask:\t");
		for(j=0;j<8;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2],dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("\n");

	}
	return len;


}

int _rtk_rg_proc_host_policing_get(struct seq_file *s, void *v)
{
    
	int len=0;
    int i;
    rtk_mac_t mac;
    uint32 meteridx;
    rtk_enable_t policing_en;
    rtk_enable_t logging_en;
    uint64 counter;
    uint32 exceed;
    uint32 bucket_size;
    rtk_enable_t ifg;
    rtk_rate_metet_mode_t mode;
    uint32 rate;

    for(i = 0; i < HOST_POLICING_TABLE_SIZE; i++) {
        rtlglue_printf("host[%d]: ", i);
        ASSERT_EQ(rtk_rg_rate_hostMacAddr_get(i, &mac), RT_ERR_OK);
        rtlglue_printf("%02X%02X%02X%02X%02X%02X  ", mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]);
        ASSERT_EQ(rtk_rg_rate_hostEgrBwCtrlState_get(i, &policing_en), RT_ERR_OK);
        rtlglue_printf("egress_en: %d  ", policing_en);
        ASSERT_EQ(rtk_rg_rate_hostIgrBwCtrlState_get(i, &policing_en), RT_ERR_OK);
        rtlglue_printf("ingress_en: %d  ", policing_en);
        ASSERT_EQ(rtk_rg_rate_hostBwCtrlMeterIdx_get(i, &meteridx), RT_ERR_OK);
        rtlglue_printf("meter index: %d  ", meteridx);
        ASSERT_EQ(rtk_rg_rate_shareMeter_get(meteridx, &rate, &ifg), RT_ERR_OK);
        rtlglue_printf("meter rate: %d, ifg: %d  ", rate, ifg);
        ASSERT_EQ(rtk_rg_rate_shareMeterMode_get(meteridx, &mode), RT_ERR_OK);
        rtlglue_printf("meter mode: %d  ", mode);
        ASSERT_EQ(rtk_rg_rate_shareMeterExceed_get(meteridx, &exceed), RT_ERR_OK);
        rtlglue_printf("meter exceed: %d  ", exceed);
        ASSERT_EQ(rtk_rg_rate_shareMeterBucket_get(meteridx, &bucket_size), RT_ERR_OK);
        rtlglue_printf("bucket size: %d  ", bucket_size);
        
        ASSERT_EQ(rtk_rg_stat_hostState_get(i, &logging_en), RT_ERR_OK);
        rtlglue_printf("mib_en: %d  ", logging_en);
        ASSERT_EQ(rtk_rg_stat_hostCnt_get(i, STAT_HOST_RX_OCTETS, &counter), RT_ERR_OK);
        rtlglue_printf("mib_rx: %lld  ", counter);
        ASSERT_EQ(rtk_rg_stat_hostCnt_get(i, STAT_HOST_TX_OCTETS, &counter), RT_ERR_OK);
        rtlglue_printf("mib_tx: %lld  ", counter);
        
        
    }
    
    return len;
}

#endif


int32 dump_tcpudp(struct seq_file *s, void *v)
{    

	uint32 idx=0, entry=0, retval=0;
	int len=0;
    rtk_l34_naptOutbound_entry_t asic_outtcpudp;
    rtk_l34_naptInbound_entry_t asic_intcpudp;
	char *napt_state[11]={"INVALID","SYN_RECV","UDP_FIRST","SYN_ACK_RECV","UDP_SECOND","TCP_CONNECTED","UDP_CONNECTED","FIRST_FIN","RST_RECV","FIN_SEND_AND_RECV","LAST_ACK"};

    PROC_PRINTF(">>ASIC NAPT TCP/UDP Table:\n");
    PROC_PRINTF(" ---------------- Outbound ----------------\n");
    for(idx=0; idx<MAX_NAPT_OUT_HW_TABLE_SIZE; idx++)
    {
        retval = rtk_l34_naptOutboundTable_get(idx, &asic_outtcpudp);
        if (retval == FAIL)
            continue;

        if (asic_outtcpudp.valid == 1)
        {
            PROC_PRINTF("  [%4d] INIDX(%d) priValid(%d) priority(%d) - extPort(0x%x) state(%s) idle(%d) replace(%d) noAddHw(%d)\n",
                           idx,
                           asic_outtcpudp.hashIdx,
                           asic_outtcpudp.priValid,
                           asic_outtcpudp.priValue,
                           rg_db.naptOut[idx].extPort,
                           napt_state[rg_db.naptOut[idx].state], 
                           rg_db.naptOut[idx].idleSecs,
                           rg_db.naptOut[idx].canBeReplaced,
                           rg_db.naptOut[idx].cannotAddToHw
                           );
            entry++;
        }
    }

    entry=0;
    PROC_PRINTF(" ---------------- Inbound ----------------\n");
    for(idx=0; idx<MAX_NAPT_IN_HW_TABLE_SIZE; idx++)
    {
        retval = rtk_l34_naptInboundTable_get(idx, &asic_intcpudp);
        if (retval == FAIL)
            continue;

        if (asic_intcpudp.valid != 0)
        {
        	uint32 remoteIp=0;
			uint16 remotePort=0;
        	if(rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)
        	{
        		int outIdx=rg_db.naptIn[idx].symmetricNaptOutIdx;
				remoteIp=rg_db.naptOut[outIdx].remoteIp;
				remotePort=rg_db.naptOut[outIdx].remotePort;
        	}
#if defined(CONFIG_RTL9602C_SERIES)
			PROC_PRINTF("  [%4d] %d.%d.%d.%d:%d V(%d), IPIDX(%d} REMHASH(%d) EXTPORT(0x%x) TCP(%d) PRI_EN(%d) PRI(%d) - remote(%d.%d.%d.%d:%d) idle(%d) replace(%d) noAddHw(%d) sw_valid(%d) %s\n",
                           idx,
                           asic_intcpudp.intIp>>24, (asic_intcpudp.intIp&0x00ff0000) >> 16,
                           (asic_intcpudp.intIp&0x0000ff00)>>8, asic_intcpudp.intIp&0x000000ff,
                           asic_intcpudp.intPort,
                           asic_intcpudp.valid, asic_intcpudp.extIpIdx,
                           asic_intcpudp.remHash,
                           ((uint32)asic_intcpudp.extPortHSB<<8 | (uint32)asic_intcpudp.extPortLSB), asic_intcpudp.isTcp, asic_intcpudp.priValid, asic_intcpudp.priId,
                           remoteIp>>24, (remoteIp&0x00ff0000) >> 16,
                           (remoteIp&0x0000ff00)>>8, remoteIp&0x000000ff,
                           remotePort,
                           rg_db.naptIn[idx].idleSecs,
                           rg_db.naptIn[idx].canBeReplaced,
                           rg_db.naptIn[idx].cannotAddToHw,
                           rg_db.naptIn[idx].rtk_naptIn.valid,
                           (rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE")
                           );
#elif defined(CONFIG_RTL9600_SERIES)
            PROC_PRINTF("  [%4d] %d.%d.%d.%d:%d V(%d), IPIDX(%d} REMHASH(%d) EPLSB(0x%x) TCP(%d) PRI_EN(%d) PRI(%d) - remote(%d.%d.%d.%d:%d) idle(%d) replace(%d) noAddHw(%d) sw_valid(%d) %s\n",
                           idx,
                           asic_intcpudp.intIp>>24, (asic_intcpudp.intIp&0x00ff0000) >> 16,
                           (asic_intcpudp.intIp&0x0000ff00)>>8, asic_intcpudp.intIp&0x000000ff,
                           asic_intcpudp.intPort,
                           asic_intcpudp.valid, asic_intcpudp.extIpIdx,
                           asic_intcpudp.remHash,
                           asic_intcpudp.extPortLSB, asic_intcpudp.isTcp, asic_intcpudp.priValid, asic_intcpudp.priId,
                           remoteIp>>24, (remoteIp&0x00ff0000) >> 16,
                           (remoteIp&0x0000ff00)>>8, remoteIp&0x000000ff,
                           remotePort,
                           rg_db.naptIn[idx].idleSecs,
                           rg_db.naptIn[idx].canBeReplaced,
                           rg_db.naptIn[idx].cannotAddToHw,
                           rg_db.naptIn[idx].rtk_naptIn.valid,
                           (rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE")
                           );
#endif
            entry++;
        }
    }
    PROC_PRINTF("Total entry: %d\n", entry);	
    return len;
}

int32 dump_napt_entry(int outIdx,int outHashIdx,struct seq_file *s)
{	
	char *napt_state[11]={"INVALID","SYN_RECV","UDP_FIRST","SYN_ACK_RECV","UDP_SECOND","TCP_CONNECTED","UDP_CONNECTED","FIRST_FIN","RST_RECV","FIN_SEND_AND_RECV","LAST_ACK"};

	//if(rg_db.naptOut[outIdx].rtk_naptOut.valid != 0)
	if(rg_db.naptOut[outIdx].state!=INVALID)
	{
		int inIdx;
		inIdx=rg_db.naptOut[outIdx].rtk_naptOut.hashIdx;
		if(rg_db.naptIn[inIdx].rtk_naptIn.valid !=0)
		{
			uint32 extip=rg_db.extip[rg_db.naptIn[inIdx].rtk_naptIn.extIpIdx].rtk_extip.extIpAddr;
			int inHash=_rtk_rg_naptTcpUdpInHashIndex(rg_db.naptIn[inIdx].rtk_naptIn.isTcp,extip,rg_db.naptOut[outIdx].extPort);
			//PROC_PRINTF("[O:%04d(%03d),I:%04d(%03d)] [%s %03d.%03d.%03d.%03d:%05d<->%03d.%03d.%03d.%03d:%05d<->%03d.%03d.%03d.%03d:%05d] priEnOut(%d) priOut(%d) V(%d) IPIDX(%d) REMHASH(%d) EPLSB(0x%x) priEnIn(%d) priIn(%d)\n",
			PROC_PRINTF("[O:%04d(%03d),I:%04d(%03d)] %s %03d.%03d.%03d.%03d:%05d[PRI:%d(%s)] <-> %03d.%03d.%03d.%03d:%05d(%d) <-> %03d.%03d.%03d.%03d:%05d[PRI:%d(%s)] %s IDLE:%d TYPE:%s\n",				
						   outIdx,outHashIdx,
						   inIdx,inHash,						   
						   (rg_db.naptIn[inIdx].rtk_naptIn.isTcp==1)?"TCP":"UDP", 
						   rg_db.naptIn[inIdx].rtk_naptIn.intIp>>24, (rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x00ff0000) >> 16,
						   (rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x0000ff00)>>8, rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x000000ff,
						   rg_db.naptIn[inIdx].rtk_naptIn.intPort,
						   rg_db.naptIn[inIdx].rtk_naptIn.priId,
						   (rg_db.naptIn[inIdx].rtk_naptIn.priValid==1)?"on":"off",
							extip>>24,(extip&0x00ff0000) >> 16,
							(extip&0x0000ff00)>>8, extip&0x000000ff,
						   rg_db.naptOut[outIdx].extPort,
							(rg_db.naptIn[inIdx].rtk_naptIn.isTcp==1)?rg_db.naptTcpExternPortUsedRefCount[rg_db.naptOut[outIdx].extPort]:rg_db.naptUdpExternPortUsedRefCount[rg_db.naptOut[outIdx].extPort],
						   rg_db.naptOut[outIdx].remoteIp>>24,(rg_db.naptOut[outIdx].remoteIp&0x00ff0000) >> 16,
						   (rg_db.naptOut[outIdx].remoteIp&0x0000ff00)>>8, rg_db.naptOut[outIdx].remoteIp&0x000000ff,
						   rg_db.naptOut[outIdx].remotePort,
							rg_db.naptOut[outIdx].rtk_naptOut.priValue,
							(rg_db.naptOut[outIdx].rtk_naptOut.priValid==1)?"on":"off",
						   
						   napt_state[rg_db.naptOut[outIdx].state], 
						   rg_db.naptOut[outIdx].idleSecs,
                           (rg_db.naptIn[inIdx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[inIdx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE")
						   /*
						   rg_db.naptOut[outIdx].rtk_naptOut.priValid, rg_db.naptOut[idx].rtk_naptOut.priValue,
						   rg_db.naptIn[inIdx].rtk_naptIn.valid,
						   rg_db.naptIn[inIdx].rtk_naptIn.extIpIdx,
						   rg_db.naptIn[inIdx].rtk_naptIn.remHash,
						   rg_db.naptIn[inIdx].rtk_naptIn.extPortLSB,							   
						   rg_db.naptIn[inIdx].rtk_naptIn.priValid, rg_db.naptIn[inIdx].rtk_naptIn.priId
						   */
						   );

		   
			if(	outIdx>=MAX_NAPT_OUT_HW_TABLE_SIZE)
				return 1; //SW entry
			if(	inIdx>=MAX_NAPT_IN_HW_TABLE_SIZE)
				return 1; //SW entry				
			if((rg_db.naptOut[outIdx].state == TCP_CONNECTED)||(rg_db.naptOut[outIdx].state == UDP_CONNECTED)||
				(rg_db.naptOut[outIdx].state == FIRST_FIN)||(rg_db.naptOut[outIdx].state == RST_RECV))
				return 2; //HW entry
			return 1;	
		
		}
	
	}
	return 0;
}

int32 dump_netstat(struct seq_file *s, void *v)
{
	int i;
	int portIdx;
	int validBit;
	int cnt=0;
	int len=0;


	PROC_PRINTF(">>Software netstat:\n");
	
	//TCP
	PROC_PRINTF("\nTCP\n");
	for(i=0;i<65536;i++)
	{
		portIdx=i>>5;
		validBit=i&31;
		if(rg_db.naptTcpExternPortUsed[portIdx]&(0x1<<validBit))
		{
			PROC_PRINTF("%d(%d),\t",i,rg_db.naptTcpExternPortUsedRefCount[i]);
			cnt++;
		}
	}
	PROC_PRINTF("\nTCP ref_cnt=%d\nUDP\n",cnt);
	cnt=0;
	//UDP
	for(i=0;i<65536;i++)
	{
		portIdx=i>>5;
		validBit=i&31;
		if(rg_db.naptUdpExternPortUsed[portIdx]&(0x1<<validBit))
		{
			PROC_PRINTF("%d(%d),\t",i,rg_db.naptUdpExternPortUsedRefCount[i]);
			cnt++;
		}
	}
	PROC_PRINTF("\nUDP ref_cnt=%d\n",cnt);
#if 1
	for(i=0;i<65536/32;i++)
	{
		if((rg_db.naptTcpExternPortUsed[i]!=0) || (rg_db.naptUdpExternPortUsed[i]!=0))
			PROC_PRINTF("[%d=>%x,%x]\t",i,rg_db.naptTcpExternPortUsed[i],rg_db.naptUdpExternPortUsed[i]);
	}
#endif
	PROC_PRINTF("\n\nTotal Entry: {Fwd:%d} {PS:%d}\n",atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]),atomic_read(&rg_db.naptProtcolStackEntryNumber[0])+atomic_read(&rg_db.naptProtcolStackEntryNumber[1]));


	return len;
}

int32 dump_napt(struct seq_file *s, void *v)
{
    uint32 idx=0,ret,naptHWNum=0,naptSWNum=0;
	rtk_rg_table_naptOut_linkList_t	*pNaptOutLinkList;
	int len=0;
	
    PROC_PRINTF(">>ASIC NAPT OUT TCP/UDP Table:\n");
    for(idx=0; idx<MAX_NAPT_OUT_HW_TABLE_SIZE; idx++)
    {
		int outIdx=idx;
		ret = dump_napt_entry(outIdx,outIdx>>2,s);
		if(ret==2) naptHWNum++;
		if(ret==1) naptSWNum++;
    }
	
	PROC_PRINTF(">>SOFTWARE LINK LIST NAPT OUT TCP/UDP Table:\n");

	for(idx=0;idx<MAX_NAPT_OUT_HW_TABLE_SIZE>>2;idx++)
	{
		pNaptOutLinkList=rg_db.pNaptOutHashListHead[idx];
		while(pNaptOutLinkList!=NULL)
		{
			ret = dump_napt_entry(pNaptOutLinkList->idx,idx,s);
			pNaptOutLinkList=pNaptOutLinkList->pNext;
			if(ret==2) naptHWNum++;
			if(ret==1) naptSWNum++;

		}
	}
	PROC_PRINTF("Total Entry: {OUT_HW:%d} {OUT_SW:%d} {Fwd:%d} {PS:%d}\n",naptHWNum,naptSWNum,atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]),atomic_read(&rg_db.naptProtcolStackEntryNumber[0])+atomic_read(&rg_db.naptProtcolStackEntryNumber[1]));

    return len;
}

int32 dump_sw_shortcut(struct seq_file *s, void *v)
{
		int len=0;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
{
		int i;
		rtk_rg_napt_shortcut_t *pNaptSc=NULL;
		PROC_PRINTF("-------------------IPv4 SW shortcut-------------------\n");
		for(i=0;i<MAX_NAPT_SHORTCUT_SIZE;i++)
		{
			pNaptSc=&rg_db.naptShortCut[i];
			
			if(pNaptSc->sip!=0) //valid
			{
				PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n 	 sip=%d.%d.%d.%d dip=%d.%d.%d.%d sport=%d dport=%d isTcp=%d spa=%d notFinishUpdated=%d hairpin=%d\n"
					,i
					,(pNaptSc->isBridge==1)?"L2":((pNaptSc->isNapt==1?"L4":"L3"))
					,pNaptSc->idleSecs
					,(pNaptSc->sip>>24)&0xff,(pNaptSc->sip>>16)&0xff,(pNaptSc->sip>>8)&0xff,(pNaptSc->sip)&0xff
					,(pNaptSc->dip>>24)&0xff,(pNaptSc->dip>>16)&0xff,(pNaptSc->dip>>8)&0xff,(pNaptSc->dip)&0xff
					,pNaptSc->sport
					,pNaptSc->dport
					,pNaptSc->isTcp
					,pNaptSc->spa
					,pNaptSc->notFinishUpdated
					,pNaptSc->isHairpinNat);
				PROC_PRINTF("	   [ACT] dir=%s lut_idx=%d intf_idx=%d eip_idx=%d naptIdx=%d pri=%d spri=%d intVlanId=%d intCFPri=%d dscp=%d uniPortmask=0x%x\n"
					,(pNaptSc->direction==NAPT_DIRECTION_OUTBOUND)?"OUT":((pNaptSc->direction==NAPT_DIRECTION_INBOUND)?"IN":"X")
					,pNaptSc->new_lut_idx
					,pNaptSc->new_intf_idx
					,pNaptSc->new_eip_idx
					,pNaptSc->naptIdx
					,pNaptSc->priority
					,pNaptSc->servicePriority
					,pNaptSc->internalVlanID
					,pNaptSc->internalCFPri
					,pNaptSc->dscp
					,pNaptSc->uniPortmask);

				PROC_PRINTF("	   vlanID=%d vlanTagif=%d svlanID=%d svlanTagif=%d dmac2cvlanID=%d dmac2cvlanTagif=%d tagAccType=%d arpIdx=%d streamID=%d\n"
					,pNaptSc->vlanID
					,pNaptSc->vlanTagif
					,pNaptSc->serviceVlanID
					,pNaptSc->serviceVlanTagif
					,pNaptSc->dmac2cvlanID
					,pNaptSc->dmac2cvlanTagif
					,pNaptSc->tagAccType
					,pNaptSc->arpIdx
#ifdef CONFIG_GPON_FEATURE	
					,pNaptSc->streamID
#else
					,0
#endif					
					);	
#if defined(CONFIG_RTL9602C_SERIES)
					PROC_PRINTF("	   smacL2Idx=%d mibDirect=%d mibNetifIdx=%d mibTagDelta=%d\n"
					,pNaptSc->smacL2Idx
					,pNaptSc->mibDirect
					,pNaptSc->mibNetifIdx
					,pNaptSc->mibTagDelta);
#endif
			}			
		}
}		
#endif


#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
{
		int i;
		rtk_ipv6_addr_t zeroIP={{0}};
		rtk_rg_naptv6_shortcut_t *pV6NaptSc=NULL;
		
		PROC_PRINTF("-------------------IPv6 SW shortcut-------------------\n");
		for(i=0;i<MAX_NAPT_V6_SHORTCUT_SIZE;i++)
		{
			pV6NaptSc=&rg_db.naptv6ShortCut[i];
			
			if(memcmp(pV6NaptSc->sip.ipv6_addr, zeroIP.ipv6_addr, IPV6_ADDR_LEN)) //valid	
			{
				PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n 	 sip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x dip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x sport=%d dport=%d isTcp=%d spa=%d notFinishUpdated=%d\n"
					,i
					,(pV6NaptSc->isBridge==1)?"L2":"L3"
					,pV6NaptSc->idleSecs
					,pV6NaptSc->sip.ipv6_addr[0],pV6NaptSc->sip.ipv6_addr[1],pV6NaptSc->sip.ipv6_addr[2],pV6NaptSc->sip.ipv6_addr[3]
					,pV6NaptSc->sip.ipv6_addr[4],pV6NaptSc->sip.ipv6_addr[5],pV6NaptSc->sip.ipv6_addr[6],pV6NaptSc->sip.ipv6_addr[7]
					,pV6NaptSc->sip.ipv6_addr[8],pV6NaptSc->sip.ipv6_addr[9],pV6NaptSc->sip.ipv6_addr[10],pV6NaptSc->sip.ipv6_addr[11]
					,pV6NaptSc->sip.ipv6_addr[12],pV6NaptSc->sip.ipv6_addr[13],pV6NaptSc->sip.ipv6_addr[14],pV6NaptSc->sip.ipv6_addr[15]
					,pV6NaptSc->dip.ipv6_addr[0],pV6NaptSc->dip.ipv6_addr[1],pV6NaptSc->dip.ipv6_addr[2],pV6NaptSc->dip.ipv6_addr[3]
					,pV6NaptSc->dip.ipv6_addr[4],pV6NaptSc->dip.ipv6_addr[5],pV6NaptSc->dip.ipv6_addr[6],pV6NaptSc->dip.ipv6_addr[7]
					,pV6NaptSc->dip.ipv6_addr[8],pV6NaptSc->dip.ipv6_addr[9],pV6NaptSc->dip.ipv6_addr[10],pV6NaptSc->dip.ipv6_addr[11]
					,pV6NaptSc->dip.ipv6_addr[12],pV6NaptSc->dip.ipv6_addr[13],pV6NaptSc->dip.ipv6_addr[14],pV6NaptSc->dip.ipv6_addr[15]
					,pV6NaptSc->sport
					,pV6NaptSc->dport
					,pV6NaptSc->isTcp
					,pV6NaptSc->spa
					,pV6NaptSc->notFinishUpdated);
				PROC_PRINTF("	   [ACT] lut_idx=%d intf_idx=%d pri=%d spri=%d intVlanId=%d intCFPri=%d dscp=%d uniPortmask=0x%x\n"
					,pV6NaptSc->new_lut_idx
					,pV6NaptSc->new_intf_idx
					,pV6NaptSc->priority
					,pV6NaptSc->servicePriority
					,pV6NaptSc->internalVlanID
					,pV6NaptSc->internalCFPri
					,pV6NaptSc->dscp
					,pV6NaptSc->uniPortmask);
				PROC_PRINTF("	   vlanID=%d vlanTagif=%d svlanID=%d svlanTagif=%d dmac2cvlanID=%d dmac2cvlanTagif=%d tagAccType=%d neighborIdx=%d streamID=%d\n"
					,pV6NaptSc->vlanID
					,pV6NaptSc->vlanTagif
					,pV6NaptSc->serviceVlanID
					,pV6NaptSc->serviceVlanTagif
					,pV6NaptSc->dmac2cvlanID
					,pV6NaptSc->dmac2cvlanTagif
					,pV6NaptSc->tagAccType
					,pV6NaptSc->neighborIdx
#ifdef CONFIG_GPON_FEATURE	
					,pV6NaptSc->streamID
#else
					,0
#endif					
					);	
#if defined(CONFIG_RTL9602C_SERIES)
					PROC_PRINTF("	   smacL2Idx=%d mibDirect=%d mibNetifIdx=%d mibTagDelta=%d\n"
					,pV6NaptSc->smacL2Idx
					,pV6NaptSc->mibDirect
					,pV6NaptSc->mibNetifIdx
					,pV6NaptSc->mibTagDelta);
#endif
			}			
		}
}		
#endif


	return len;
}


int32 mibdump_frag(struct seq_file *s, void *v)
{
    uint32 idx=0;
	rtk_rg_ipv4_fragment_out_t *pFragOut;
	rtk_rg_ipv4_fragment_in_t *pFragIn;
	int len=0;
	
    PROC_PRINTF(">>SOFTWARE Fragment Link-List:\n");

	//count free out list number
	idx=0;
	pFragOut=rg_db.pFragOutFreeListHead;
	while(pFragOut!=NULL)
	{
		idx++;
		pFragOut=pFragOut->pNext;
	}
	PROC_PRINTF("free Frag Out list has %d elements\n",idx);

	//display used hash head
	for(idx=0;idx<MAX_NAPT_OUT_HW_TABLE_SIZE>>2;idx++)
	{
		if(rg_db.pFragOutHashListHead[idx]!=NULL)
		{
			PROC_PRINTF("pFragOutHashListHead[%d]->%p(%s,idle:%lu,act:%d)",idx,rg_db.pFragOutHashListHead[idx],
				rg_db.pFragOutHashListHead[idx]->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",
				rg_db.pFragOutHashListHead[idx]->beginIdleTime,rg_db.pFragOutHashListHead[idx]->fragAction);
			//travel all list
			pFragOut=rg_db.pFragOutHashListHead[idx]->pNext;
			while(pFragOut!=NULL)
			{
				PROC_PRINTF("->%p(%s,idle:%lu,act:%d)",pFragOut,pFragOut->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",
					pFragOut->beginIdleTime,rg_db.pFragOutHashListHead[idx]->fragAction);
				pFragOut=pFragOut->pNext;
			}
			PROC_PRINTF("->NULL\n");
		}
	}

	PROC_PRINTF("===================================\n");

	//count free in list number
	idx=0;
	pFragIn=rg_db.pFragInFreeListHead;
	while(pFragIn!=NULL)
	{
		idx++;
		pFragIn=pFragIn->pNext;
	}
	PROC_PRINTF("free Frag In list has %d elements\n",idx);

	//display used hash head
	for(idx=0;idx<MAX_NAPT_IN_HW_TABLE_SIZE>>2;idx++)
	{
		if(rg_db.pFragInHashListHead[idx]!=NULL)
		{
			PROC_PRINTF("pFragInHashListHead[%d]->%p(%s,idle:%lu,act:%d)",idx,rg_db.pFragInHashListHead[idx],
				rg_db.pFragInHashListHead[idx]->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",
				rg_db.pFragInHashListHead[idx]->beginIdleTime,rg_db.pFragInHashListHead[idx]->fragAction);
			//travel all list
			pFragIn=rg_db.pFragInHashListHead[idx]->pNext;
			while(pFragIn!=NULL)
			{
				PROC_PRINTF("->%p(%s,idle:%lu,act:%d)",pFragIn,pFragIn->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",
					pFragIn->beginIdleTime,rg_db.pFragInHashListHead[idx]->fragAction);
				pFragIn=pFragIn->pNext;
			}
			PROC_PRINTF("->NULL\n");
		}
	}

	PROC_PRINTF(">>IPv4 fragmentsQueue:\n");
	for(idx=0;idx<MAX_IPV4_FRAGMENT_QUEUE_SIZE;idx++)
	{
		//------------------ Critical Section start -----------------------//
		rg_lock(&rg_kernel.ipv4FragQueueLock);
		if(rg_db.ipv4FragmentQueue[idx].occupied)
		{
			PROC_PRINTF("  queue[%d]: queueTime=%lu, direction=%d, skb[%p]\n",
				idx,rg_db.ipv4FragmentQueue[idx].queue_time,rg_db.ipv4FragmentQueue[idx].direction,rg_db.ipv4FragmentQueue[idx].queue_skb);
		}
		//------------------ Critical Section End -----------------------//
		rg_unlock(&rg_kernel.ipv4FragQueueLock);
	}
	
    return len;
}
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
int32 mibdump_v6Frag(struct seq_file *s, void *v)
{
	int i,_count;
	rtk_rg_ipv6_layer4_linkList_t *pLayer4List;
	int len=0;
	
	PROC_PRINTF(">>IPv6 Stateful fragments:\n");
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
	{
		if(!list_empty(&rg_db.ipv6Layer4HashListHead[i]))
		{
			_count=0;			
			list_for_each_entry(pLayer4List,&rg_db.ipv6Layer4HashListHead[i],layer4_list)
			{
				if(!pLayer4List->isFrag)continue;
				if(_count==0)PROC_PRINTF("[%d]========================================================\n",i);

				if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND)
					PROC_PRINTF("   Outbound[%p]",pLayer4List);
				else
					PROC_PRINTF("   Inbound [%p]",pLayer4List);

				PROC_PRINTF("  idle:%lu action:%d, queueCount:%d\n",
					(jiffies-pLayer4List->beginIdleTime)/TICKTIME_PERIOD,pLayer4List->fragAction,pLayer4List->queueCount);
				PROC_PRINTF("	  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x --> \n	 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ID:%04x%04x\n",
					pLayer4List->srcIP.ipv6_addr[0],pLayer4List->srcIP.ipv6_addr[1],pLayer4List->srcIP.ipv6_addr[2],pLayer4List->srcIP.ipv6_addr[3],
					pLayer4List->srcIP.ipv6_addr[4],pLayer4List->srcIP.ipv6_addr[5],pLayer4List->srcIP.ipv6_addr[6],pLayer4List->srcIP.ipv6_addr[7],
					pLayer4List->srcIP.ipv6_addr[8],pLayer4List->srcIP.ipv6_addr[9],pLayer4List->srcIP.ipv6_addr[10],pLayer4List->srcIP.ipv6_addr[11],
					pLayer4List->srcIP.ipv6_addr[12],pLayer4List->srcIP.ipv6_addr[13],pLayer4List->srcIP.ipv6_addr[14],pLayer4List->srcIP.ipv6_addr[15],
					pLayer4List->destIP.ipv6_addr[0],pLayer4List->destIP.ipv6_addr[1],pLayer4List->destIP.ipv6_addr[2],pLayer4List->destIP.ipv6_addr[3],
					pLayer4List->destIP.ipv6_addr[4],pLayer4List->destIP.ipv6_addr[5],pLayer4List->destIP.ipv6_addr[6],pLayer4List->destIP.ipv6_addr[7],
					pLayer4List->destIP.ipv6_addr[8],pLayer4List->destIP.ipv6_addr[9],pLayer4List->destIP.ipv6_addr[10],pLayer4List->destIP.ipv6_addr[11],
					pLayer4List->destIP.ipv6_addr[12],pLayer4List->destIP.ipv6_addr[13],pLayer4List->destIP.ipv6_addr[14],pLayer4List->destIP.ipv6_addr[15],
					pLayer4List->srcPort,pLayer4List->destPort);				
				
				_count++;
			}
			if(_count>0)PROC_PRINTF("[%d] has %d Fragment.\n",i,_count);
		}
		
	}

	PROC_PRINTF(">>IPv6 Stateful fragmentsQueue:\n");
	for(i=0;i<MAX_IPV6_FRAGMENT_QUEUE_SIZE;i++)
	{
		//------------------ Critical Section start -----------------------//
		rg_lock(&rg_kernel.ipv6FragQueueLock);
		if(rg_db.ipv6FragmentQueue[i].occupied)
		{
			PROC_PRINTF("  queue[%d]: queueTime=%lu, direction=%d, wanType=%s, skb[%p]\n",
				i,(jiffies-rg_db.ipv6FragmentQueue[i].queue_time)/TICKTIME_PERIOD,rg_db.ipv6FragmentQueue[i].direction,
				rg_db.ipv6FragmentQueue[i].wanType==L34_NH_ETHER?"ETHER":"PPPOE",rg_db.ipv6FragmentQueue[i].queue_skb);
		}
		//------------------ Critical Section End -----------------------//
		rg_unlock(&rg_kernel.ipv6FragQueueLock);
	}
	
    return len;
}
#endif

#if defined(CONFIG_APOLLO_GPON_FPGATEST)

int32 mibclean_vmac_skb(struct seq_file *s, void *v)
{	
	int len=0;
	rtk_rg_vmac_skb_linlList_t *pVmacEntry,*pNextEntry;

	if(!list_empty(&rg_db.vmacSkbListHead))	
	{
		list_for_each_entry_safe(pVmacEntry,pNextEntry,&rg_db.vmacSkbListHead,vmac_list)
		{
			//Delete from head list
			list_del_init(&pVmacEntry->vmac_list);

			dev_kfree_skb_any(pVmacEntry->skb);
			pVmacEntry->skb=NULL;

			//Add back to free list
			list_add(&pVmacEntry->vmac_list,&rg_db.vmacSkbListFreeListHead);
		}
	}

	return len;
}


int32 mibdump_vmac_skb(struct seq_file *s, void *v)
{	
	int len=0,i=0;
	rtk_rg_vmac_skb_linlList_t *pVmacEntry;

	PROC_PRINTF(">>virtualmac with PON:\n\n");
		
	if(!list_empty(&rg_db.vmacSkbListHead))	
	{
		list_for_each_entry(pVmacEntry,&rg_db.vmacSkbListHead,vmac_list)
		{
			PROC_PRINTF("-->skb_%d[%x] len=%d %08x %08x %08x %08x\n",i++,(unsigned int)pVmacEntry->skb&0xffff,pVmacEntry->skb->len,
				*(unsigned int *)pVmacEntry->skb->data,
				*(unsigned int *)(pVmacEntry->skb->data+1),
				*(unsigned int *)(pVmacEntry->skb->data+2),
				*(unsigned int *)(pVmacEntry->skb->data+3));
			memDump(pVmacEntry->skb,pVmacEntry->skb->len,"PACKET");
		}
	}

	return len;
}
#endif

int32 dump_netif(struct seq_file *s, void *v)
{ 
	uint8	*mac;
    int32	i, j;
    rtk_portmask_t mbr,untag;
	int len=0;

    PROC_PRINTF(">>ASIC Netif Table:\n\n");

    for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
    {
    	rtk_l34_netif_entry_t intf;
#if defined(CONFIG_RTL9602C_SERIES)
		rtk_l34_mib_t wanIfMibt;
#endif
        int retval  = 0;
		memset(&intf,0,sizeof(rtk_l34_netif_entry_t));
		retval = rtk_l34_netifTable_get(i,&intf);
        if (retval == FAIL)
            continue;

        if (intf.valid)
        {
            mac = (uint8 *)&intf.gateway_mac.octet[0];
            PROC_PRINTF("  [%d]-vid[%d] %02x:%02x:%02x:%02x:%02x:%02x (l2Idx=%d)",
                           i, intf.vlan_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],rg_db.netif[i].l2_idx);
            PROC_PRINTF("  L3/4 HW acc %s ",
                           intf.enable_rounting==TRUE? "enabled": "disabled" );
            PROC_PRINTF("\n      %d MAC Addresses, MTU %d Bytes", intf.mac_mask, intf.mtu);
#if defined(CONFIG_RTL9602C_SERIES)
			PROC_PRINTF("\n 	DSlite state[%s], Index[%d]", intf.dslite_state==1?"enabled": "disabled", intf.dslite_idx);
			PROC_PRINTF("\n 	For mib counter lookup, isL34:%d, isCtagIf:%d, IPv4 Addr: %d.%d.%d.%d", intf.isL34, intf.isCtagIf, (intf.ipAddr>>24),
						   ((intf.ipAddr&0x00ff0000)>>16), ((intf.ipAddr&0x0000ff00)>>8), (intf.ipAddr&0xff));
			
			memset(&wanIfMibt,0,sizeof(rtk_l34_mib_t));
			wanIfMibt.ifIndex = i;
			if(rtk_l34_mib_get(&wanIfMibt)==RT_ERR_OK)
				PROC_PRINTF("\n 	HW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)", wanIfMibt.ifInUcstPkts, wanIfMibt.ifInOctets, wanIfMibt.ifOutUcstPkts, wanIfMibt.ifOutOctets);
			PROC_PRINTF("\n 	SW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)", rg_db.netif[i].rtk_mib.ifInUcstPkts, rg_db.netif[i].rtk_mib.ifInOctets, rg_db.netif[i].rtk_mib.ifOutUcstPkts, rg_db.netif[i].rtk_mib.ifOutOctets);
#endif

            mbr.bits[0]=0;
            untag.bits[0]=0;
            retval = rtk_vlan_port_get(intf.vlan_id,&mbr,&untag);
            //assert_ok(retval==RT_ERR_OK);
            PROC_PRINTF("\n		Untag member ports:");
            for(j=0; j<RTK_RG_MAX_MAC_PORT; j++)
                if(untag.bits[0] & (1<<j))
                    PROC_PRINTF("%d ", j);
            PROC_PRINTF("\n		Active member ports:");
            for(j=0; j<RTK_RG_MAX_MAC_PORT; j++)
                if(mbr.bits[0] & (1<<j))
                    PROC_PRINTF("%d ", j);
            mbr.bits[0]=0;
            retval = rtk_vlan_extPort_get(intf.vlan_id,&mbr);
			//assert_ok(retval==RT_ERR_OK);            
            for(j=1; j<RTK_RG_MAX_EXT_PORT; j++)
                if(mbr.bits[0] & (1<<j))
                    PROC_PRINTF("EXT%d ", j);
            PROC_PRINTF("\n\n");
        }
    }

    return len;
}
int32 dump_pppoe(struct seq_file *s, void *v)
{
	rtk_l34_pppoe_entry_t pppEntry;
	int32	i=0,retval=0;
	int len=0;

	PROC_PRINTF(">>PPPOE Table:\n");
	for(i=0; i<MAX_PPPOE_HW_TABLE_SIZE; i++)
	{
		memset(&pppEntry,0,sizeof(rtk_l34_pppoe_entry_t));
		retval = rtk_l34_pppoeTable_get(i,  &pppEntry);
		if (retval == FAIL)
			continue;
		PROC_PRINTF("  [%d]	sessionID(%d) hex:0x%x\n", i, pppEntry.sessionID, pppEntry.sessionID);
	}

#if 0
    rtl865x_tblAsicDrv_pppoeParam_t asic_pppoe;
    int32	i=0,retval=0;

    PROC_PRINTF(">>PPPOE Table:\n");
    for(i=0; i<MAX_PPPOE_HW_TABLE_SIZE; i++)
    {
        memset(&asic_pppoe,0,sizeof(asic_pppoe));
        retval = apollo_getAsicPppoe(i,  &asic_pppoe);
        if (retval == FAILED)
            continue;
        PROC_PRINTF("  [%d]  sessionID(%d)  ageSec(%d)\n", i, asic_pppoe.sessionId, asic_pppoe.age);
    }
#endif
    return len;
}
int32 dump_nexthop(struct seq_file *s, void *v)
{
	rtk_l34_nexthop_entry_t nextHopEntry;
	uint32 idx, refcnt, rt_flag, retval;

	uint8	isPppoe=0;
	uint8  	dvid=0;
	uint8  	pppoeIdx=0;
	uint16 	nextHopRow=0;
	uint16 	nextHopColumn=0;
	uint8  	pppoekeep=0;
	uint8 	extIntIpIdx=0;
	int len=0;


	PROC_PRINTF(">>ASIC Next Hop Table:\n");

	for(idx=0; idx<MAX_NEXTHOP_HW_TABLE_SIZE; idx++)
	{
		refcnt = rt_flag = 0;
		retval = rtk_l34_nexthopTable_get(idx, &nextHopEntry);
		ASSERT_EQ(retval,RT_ERR_OK);


		dvid = nextHopEntry.ifIdx;
		nextHopRow = nextHopEntry.nhIdx>>2;
		nextHopColumn = nextHopEntry.nhIdx&0x3;
		pppoeIdx = nextHopEntry.pppoeIdx;
		isPppoe = nextHopEntry.type;
		pppoekeep = nextHopEntry.keepPppoe;

		if((nextHopRow==0) && (nextHopColumn==0) && (isPppoe==0) && (extIntIpIdx==0) && (dvid==0) && (pppoeIdx==0)) 
			continue;

		PROC_PRINTF("  [%d]\ttype(%s) keep(%d) IFIdx(%d) pppoeIdx(%d) nextHop(%d,%d) refCount(%d)\n", idx,
					   (isPppoe==TRUE? "pppoe": "ethernet"),pppoekeep,
					   dvid, pppoeIdx, (nextHopRow),nextHopColumn,rg_db.systemGlobal.nxpRefCount[idx]);
	}

#if 0
    rtl865x_tblAsicDrv_nextHopParam_t asic_nxthop;

    uint32 idx, refcnt, rt_flag, retval;

    PROC_PRINTF(">>ASIC Next Hop Table:\n");

    for(idx=0; idx<MAX_NEXTHOP_HW_TABLE_SIZE; idx++)
    {
        refcnt = rt_flag = 0;
        retval = apollo_getAsicNextHopTable(idx, &asic_nxthop);
        ASSERT_EQ(retval,RT_ERR_OK);


        if((asic_nxthop.nextHopRow==0) && (asic_nxthop.nextHopColumn==0) && (asic_nxthop.isPppoe==0) && (asic_nxthop.extIntIpIdx==0) && (asic_nxthop.dvid==0) && (asic_nxthop.pppoeIdx==0)) continue;

        PROC_PRINTF("  [%d]\ttype(%s%s) IFIdx(%d) pppoeIdx(%d) nextHop(%d,%d)\n", idx,
                       (asic_nxthop.isPppoe==TRUE? "pppoe": "ethernet"),(asic_nxthop.pppoekeep?", Keep":", Non-Keep"),
                       asic_nxthop.dvid, asic_nxthop.pppoeIdx, (asic_nxthop.nextHopRow),asic_nxthop.nextHopColumn);

    }
#endif
    return len;
}
int32 dump_arp(struct seq_file *s, void *v)
{

#if defined(CONFIG_RTL9600_SERIES)
	rtk_l34_routing_entry_t asic_l3;
	uint32 j;
	ipaddr_t ipAddr;
#endif		
	uint32	i, retval;
	rtk_l34_arp_entry_t asic_arp;
	rtk_rg_arp_linkList_t *pSoftwareArp;
	int len=0;

	int8 ipBuf[sizeof"255.255.255.255"];

	PROC_PRINTF(">>Arp Table:\n");
	for(i=0; i<MAX_ARP_HW_TABLE_SIZE; i++)
	{
		retval = rtk_l34_arpTable_get(i,  &asic_arp);
		if ((retval == FAIL) || asic_arp.valid!=1)
			continue;
		{
#if defined(CONFIG_RTL9600_SERIES)			
			for(j=0; j<MAX_L3_HW_TABLE_SIZE; j++)
			{
				memset(&asic_l3,0,sizeof(rtk_l34_arp_entry_t));
				retval = rtk_l34_routingTable_get(j, &asic_l3);
				if ( retval == FAIL || asic_l3.process!= 2 || asic_l3.valid!=1)
					continue;
				
				if(asic_l3.arpStart <= (i>>2) &&  (i>>2) <= asic_l3.arpEnd)					
				{
					ipAddr= (i-(asic_l3.arpStart<<2))+(asic_l3.ipAddr & (~((1<<(31-asic_l3.ipMask))-1)));	
					inet_ntoa_r(ntohl(ipAddr), ipBuf);
					if(rg_db.arp[i].staticEntry)
						{PROC_PRINTF("  [%3d] : %-16s (STATIC) : ",i, ipBuf);}
					else
						{PROC_PRINTF("  [%3d] : %-16s (DYNAMIC) : ",i, ipBuf);}
					if(rg_db.systemGlobal.gatherLanNetInfo)
						PROC_PRINTF("Name:%s DevType:%s Brand:%s OS:%s ConnType:%s",
							rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].dev_name,
							rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
							rg_lanNet_brand[(int)rg_db.arp[i].lanNetInfo.brand][0],
							rg_lanNet_os[(int)rg_db.arp[i].lanNetInfo.os][0],
							rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
					PROC_PRINTF(" -> L2:%d (L3Idx:%d Idle %d Secs)\n", asic_arp.nhIdx,rg_db.arp[i].routingIdx,rg_db.arp[i].idleSecs);
					break;
				}
			}
#elif defined(CONFIG_RTL9602C_SERIES)
			inet_ntoa_r(ntohl(asic_arp.ipAddr), ipBuf);
			if(rg_db.arp[i].staticEntry){
				PROC_PRINTF("  [%3d] : %-16s (STATIC) : ",i, ipBuf);
			}else{
				PROC_PRINTF("  [%3d] : %-16s (DYNAMIC) : ",i, ipBuf);
			}
			if(rg_db.systemGlobal.gatherLanNetInfo)
				PROC_PRINTF("Name:%s DevType:%s Brand:%s OS:%s ConnType:%s\n",
					rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].dev_name,
					rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
					rg_lanNet_brand[(int)rg_db.arp[i].lanNetInfo.brand][0],
					rg_lanNet_os[(int)rg_db.arp[i].lanNetInfo.os][0],
					rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			PROC_PRINTF("\t -> L2:%d (Idle %d Secs)\n", asic_arp.nhIdx, rg_db.arp[i].idleSecs);			
#endif			
		}

	}

#if defined(CONFIG_RTL9602C_SERIES) && defined(CONFIG_APOLLO_FPGA_PHY_TEST) 	
	PROC_PRINTF(">>Arp Hardware list:\n");
	for(i=0;i<MAX_ARP_HW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.hardwareArpTableHead[i]))
		{
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpTableHead[i],arp_list)
			{
				inet_ntoa_r(ntohl(rg_db.arp[pSoftwareArp->idx].ipv4Addr), ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC");
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF(">>Arp Hardware Free list:\n");
	list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpFreeListHead,arp_list)
	{
		inet_ntoa_r(ntohl(rg_db.arp[pSoftwareArp->idx].ipv4Addr), ipBuf);
		PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s) \n",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC");
	}
	PROC_PRINTF("\n");

	PROC_PRINTF(">> ARP valid:\n");
	for(i=0;i<=(MAX_ARP_HW_TABLE_SIZE/32);i++) PROC_PRINTF("[%08x]",rg_db.arpValidSet[i]);
	PROC_PRINTF("\n");
#endif

	
	PROC_PRINTF(">>Arp Software Table:\n");
	for(i=0;i<MAX_ARP_SW_TABLE_HEAD;i++){
		if(!list_empty(&rg_db.softwareArpTableHead[i])){
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.softwareArpTableHead[i],arp_list){
				inet_ntoa_r(ntohl(rg_db.arp[pSoftwareArp->idx].ipv4Addr), ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC");
				if(rg_db.systemGlobal.gatherLanNetInfo)
					PROC_PRINTF("Name:%s DevType:%s Brand:%s OS:%s ConnType:%s\n",
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].dev_name,
						rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
						rg_lanNet_brand[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.brand][0],
						rg_lanNet_os[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.os][0],
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			}
			PROC_PRINTF("\n");
		}
	}

    return len;
}
int32 dump_ip(struct seq_file *s, void *v)
{
#if 1
	rtk_l34_ext_intip_entry_t asic_ip;
    int32	i;
    int8 intIpBuf[sizeof"255.255.255.255"];
    int8 extIpBuf[sizeof"255.255.255.255"];
	int len=0;

    PROC_PRINTF(">>IP Table:\n");
    for(i=0; i<MAX_EXTIP_HW_TABLE_SIZE; i++)
    {
        int retval = 0;
		memset(&asic_ip,0,sizeof(rtk_l34_ext_intip_entry_t));
		retval = rtk_l34_extIntIPTable_get(i,&asic_ip);
        if ( retval != RT_ERR_OK)
        {
            PROC_PRINTF("  [%d] (Invalid) --> %x.\n", i,retval);
            continue;
        }
        else if (asic_ip.valid==0)
        {
            PROC_PRINTF("  [%d] (Invalid)\n", i);
            continue;
        }
        else
        {
            inet_ntoa_r(ntohl(asic_ip.intIpAddr), intIpBuf);
            inet_ntoa_r(ntohl(asic_ip.extIpAddr),extIpBuf);
            PROC_PRINTF("  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d) PriValid(%d) Priority(%d)\n",
                           i, intIpBuf,extIpBuf,
                           ((asic_ip.type&0x2)!=0? "LP" : ((asic_ip.type&0x1)!=0 ? "NAT" : "NAPT")), asic_ip.nhIdx, asic_ip.prival, asic_ip.pri);
        }
    }
#endif
    return len;
}

int32 dump_ipv6_extIp(struct seq_file *s, void *v){
	int len=0;	
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	int i;
	rtk_rg_table_v6ExtIp_t v6ExtIp;


	for(i=0;i<MAX_IPV6_ROUTING_SW_TABLE_SIZE;i++){
		bzero(&v6ExtIp,sizeof(v6ExtIp));
		_rtk_rg_ipv6_externalIp_get(i,&v6ExtIp);
		
		if(v6ExtIp.valid==1){
			PROC_PRINTF("[%d] EXT_IP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)  NextHop[%d]\n",i,
			v6ExtIp.externalIp.ipv6_addr[0],v6ExtIp.externalIp.ipv6_addr[1],v6ExtIp.externalIp.ipv6_addr[2],v6ExtIp.externalIp.ipv6_addr[3],
			v6ExtIp.externalIp.ipv6_addr[4],v6ExtIp.externalIp.ipv6_addr[5],v6ExtIp.externalIp.ipv6_addr[6],v6ExtIp.externalIp.ipv6_addr[7],
			v6ExtIp.externalIp.ipv6_addr[8],v6ExtIp.externalIp.ipv6_addr[9],v6ExtIp.externalIp.ipv6_addr[10],v6ExtIp.externalIp.ipv6_addr[11],
			v6ExtIp.externalIp.ipv6_addr[12],v6ExtIp.externalIp.ipv6_addr[13],v6ExtIp.externalIp.ipv6_addr[14],v6ExtIp.externalIp.ipv6_addr[15],
			v6ExtIp.nextHopIdx);
		}else{
			PROC_PRINTF("[%d] (Invalid)\n",i);
		}
	}
#endif
	return len;
}

int32 dump_l3(struct seq_file *s, void *v)
{

#if 1
	rtk_l34_routing_entry_t asic_l3;
	uint32 idx, retval;
	int8 *str[4] = { "CPU", "DROP", "ARP", "NxtHop"};
#if defined(CONFIG_RTL9600_SERIES)	
	int8 *nhstr[3] = {"PER-PACKET","PER-SESSION","PER-SIP"};
#endif
	int len=0;
	PROC_PRINTF(">>L3 Routing Table:\n");
	for(idx=0; idx<MAX_L3_HW_TABLE_SIZE; idx++)
	{
	    retval = rtk_l34_routingTable_get(idx, &asic_l3);
	    if ( retval != RT_ERR_OK)
	    {
	        PROC_PRINTF("[%d]  (Invalid)\n", idx);
	        continue;
	    }
	    else
	    {
			if(asic_l3.ipAddr!=0) asic_l3.ipMask++;

			if(asic_l3.valid==1)
			{
		        PROC_PRINTF("[%d] Valid %d.%d.%d.%d/%d Internal(%d) RT2WAN(%d)\n", idx, (asic_l3.ipAddr>>24)&0xff,
		        (asic_l3.ipAddr>>16)&0xff, (asic_l3.ipAddr>>8)&0xff, (asic_l3.ipAddr&0xff),
		        asic_l3.ipMask,asic_l3.internal,asic_l3.rt2waninf);
			}
			else
			{
				PROC_PRINTF("[%d]  (Invalid)\n", idx);
	        	continue;
			}
		    
		    switch(asic_l3.process)
		    {
		    case 0x00:	/* CPU */
		        PROC_PRINTF("            [%s PROCESS] \n\n", str[asic_l3.process]);
		        break;
				
		    case 0x01:	/* DROP */
		        PROC_PRINTF("            [%s PROCESS] \n\n", str[asic_l3.process]);
		        break;

#if defined(CONFIG_RTL9602C_SERIES)
			case 0x02:	/* ARP */
				PROC_PRINTF("			[%s PROCESS]: NET_IF(%d)\n\n", str[asic_l3.process],asic_l3.netifIdx);
				break;
			case 0x03:	/* NextHop */
				PROC_PRINTF("			[%s PROCESS]: NEXT_HOP(%d)\n\n", str[asic_l3.process],asic_l3.nhNxt);


#elif defined(CONFIG_RTL9600_SERIES)
			case 0x02:	/* ARP */
				PROC_PRINTF("			[%s PROCESS]: NETIF(%d) ARPSTA(%d) ARPEND(%d) \n\n", str[asic_l3.process],asic_l3.netifIdx,asic_l3.arpStart, asic_l3.arpEnd);
				break;
			case 0x03:	/* NextHop */
				PROC_PRINTF("			[%s PROCESS]: NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%s) IPDOMAIN(%d)\n\n", str[asic_l3.process],asic_l3.nhStart,
							   asic_l3.nhNum, asic_l3.nhNxt, nhstr[asic_l3.nhAlgo], asic_l3.ipDomain);

#endif



		        break;
		    default:
		        assert(0);
		    }
		}
	}
#endif 
 
    return len;
}
int32 dump_l4hs(struct seq_file *s, void *v)
{
	int retval = 0;
	rtk_l34_hsb_t hsb_r;
	rtk_l34_hsa_t hsa_r;
	int len=0;
	
	memset(&hsb_r,0,sizeof(rtk_l34_hsb_t));
	memset(&hsa_r,0,sizeof(rtk_l34_hsa_t));
	retval = rtk_l34_hsbData_get(&hsb_r);
	ASSERT_EQ(retval,RT_ERR_OK);
	retval = rtk_l34_hsaData_get(&hsa_r);
	ASSERT_EQ(retval,RT_ERR_OK);

	PROC_PRINTF("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsb_S(&hsb_r,s);
	PROC_PRINTF("----------------------------------------------\n");
	PROC_PRINTF("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsa_S(&hsa_r,s);
	PROC_PRINTF("----------------------------------------------\n");
	
#if 0
	l4_hsb_param_t hsb_r;
	l4_hsa_param_t hsa_r;
	memset((void*)&hsb_r,0,sizeof(hsb_r));
	memset((void*)&hsa_r,0,sizeof(hsa_r));

	apollo_virtualMacGetL4Hsb( &hsb_r );
	apollo_virtualMacGetL4Hsa( &hsa_r );

	PROC_PRINTF("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsb_S(&hsb_r);
	PROC_PRINTF("----------------------------------------------\n");
	PROC_PRINTF("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsa_S(&hsa_r);
	PROC_PRINTF("----------------------------------------------\n");
#endif
    return len;
}

rtk_hsb_t rawHsb;
rtk_hsa_t rawHsa;
rtk_hsa_debug_t rawHsd;

int32 dump_hs(struct seq_file *s, void *v)
{
#if 1
    int32 ret;

	ret=0;
    memset((void*)&rawHsb,0,sizeof(rawHsb));
    memset((void*)&rawHsa,0,sizeof(rawHsa));
    memset((void*)&rawHsd,0,sizeof(rawHsd));



#if defined(CONFIG_RTL9600_SERIES)
		ret = apollomp_raw_hsbData_get(&rawHsb);
	    ASSERT_EQ(ret,RT_ERR_OK);
	    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
	    ret = apollomp_raw_hsaData_get(&rawHsa);
	    ASSERT_EQ(ret,RT_ERR_OK);
	    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
	    ret = apollomp_raw_hsdData_get(&rawHsd);
	    ASSERT_EQ(ret,RT_ERR_OK);
#endif

#if defined(CONFIG_RTL9602C_SERIES)
		ret = rtl9602c_hsbData_get(&rawHsb);
	    ASSERT_EQ(ret,RT_ERR_OK);
	    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
	    ret = rtl9602c_hsaData_get(&rawHsa);
	    ASSERT_EQ(ret,RT_ERR_OK);
	    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
	    ret = rtl9602c_hsdData_get(&rawHsd);
	    ASSERT_EQ(ret,RT_ERR_OK);
#endif


    rtlglue_printf("---- "COLOR_Y "[HSB:]" COLOR_NM "------------------------------------\n");
    _diag_debug_hsb_display(&rawHsb);
    rtlglue_printf("----------------------------------------------\n");
    rtlglue_printf("---- "COLOR_Y "[HSA:]" COLOR_NM "------------------------------------\n");
    _diag_debug_hsa_display(&rawHsa);
    rtlglue_printf("----------------------------------------------\n");
    rtlglue_printf("---- "COLOR_Y "[HSD:]" COLOR_NM "------------------------------------\n");
    _diag_debug_hsd_display(&rawHsd);
    rtlglue_printf("----------------------------------------------\n");
#endif
    return SUCCESS;

}

int32 dump_lut(int idx,struct seq_file *s)
{
    rtk_l2_addr_table_t	data;
    int in_idx=idx;
    int out_idx=in_idx;
    int ret;


	data.method = 0;
    ret=rtk_l2_nextValidEntry_get(&out_idx,&data);
    if(ret!=RT_ERR_OK) 
	{
		PROC_PRINTF("rtk_l2_nextValidEntry_get Error at idx=%d\n",idx);
		return FAIL;
    }

    if(in_idx==out_idx)
    {
#if defined(CONFIG_RTL9600_SERIES)	
		PROC_PRINTF("--------------- LUT TABLE (%d)----------------\n",idx);
#else	//support lut traffic bit
		if(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)
			PROC_PRINTF("--------------- LUT TABLE (%d)----------------[Idle %d Secs], [arpRefCount: %d]\n",idx, rg_db.lut[idx].idleSecs, rg_db.lut[idx].arp_refCount);
		else
			PROC_PRINTF("--------------- LUT TABLE (%d)----------------[Idle %d Secs]\n",idx, rg_db.lut[idx].idleSecs);
#endif

        PROC_PRINTF("LUT idx=%d\n",out_idx);
        if(data.entryType==RTK_LUT_L2UC)
        {
            PROC_PRINTF("[P1] mac=%02x:%02x:%02x:%02x:%02x:%02x cvid=%d l3lookup=%d ivl=%d\n"
                           ,data.entry.l2UcEntry.mac.octet[0]
                           ,data.entry.l2UcEntry.mac.octet[1]
                           ,data.entry.l2UcEntry.mac.octet[2]
                           ,data.entry.l2UcEntry.mac.octet[3]
                           ,data.entry.l2UcEntry.mac.octet[4]
                           ,data.entry.l2UcEntry.mac.octet[5]
                           ,data.entry.l2UcEntry.vid
                           ,0
                           ,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL)?1:0);

#if defined(CONFIG_RTL9600_SERIES)
			PROC_PRINTF("efid=%d sapri_en=%d lutpri_en=%d lutpri=%d fwdpri_en=%d\n",data.entry.l2UcEntry.efid,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0
			,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_LOOKUP_PRI)?1:0,data.entry.l2UcEntry.priority,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0);
#elif defined(CONFIG_RTL9602C_SERIES)
			PROC_PRINTF("ctagif=%d ",(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0);
#endif

            PROC_PRINTF("fid=%d spa=%d age=%d auth1x=%d sablock=%d\n"                           
                           ,data.entry.l2UcEntry.fid                           
                           ,data.entry.l2UcEntry.port
                           ,data.entry.l2UcEntry.age
                           ,data.entry.l2UcEntry.auth
                           ,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0);

            PROC_PRINTF("dablock=%d ext_spa=%d arp_used=%d static=%d\n"
                           ,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0
                           ,data.entry.l2UcEntry.ext_port
                           ,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)?1:0                  
                           ,(data.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0);

        }
        else if(data.entryType==RTK_LUT_L2MC)
        {
            PROC_PRINTF("[P2] mac=%02x:%02x:%02x:%02x:%02x:%02x ivl=%d vid=%d fid=%d l3lookup=%d\n"
                           ,data.entry.l2McEntry.mac.octet[0]
                           ,data.entry.l2McEntry.mac.octet[1]
                           ,data.entry.l2McEntry.mac.octet[2]
                           ,data.entry.l2McEntry.mac.octet[3]
                           ,data.entry.l2McEntry.mac.octet[4]
                           ,data.entry.l2McEntry.mac.octet[5]
                           ,(data.entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0
                           ,data.entry.l2McEntry.vid
                           ,data.entry.l2McEntry.fid
                           ,0);

            PROC_PRINTF("mbr=0x%x extmbr=0x%x static=%d\n"
                           ,data.entry.l2McEntry.portmask.bits[0]
                           ,data.entry.l2McEntry.ext_portmask.bits[0],(data.entry.l2McEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0);
			
#if defined(CONFIG_RTL9600_SERIES)
            PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                           ,data.entry.l2McEntry.priority
                           ,(data.entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_FWD_PRI)?1:0);
#endif

        }
        else if(data.entryType==RTK_LUT_L3MC)
        {

#if defined(CONFIG_RTL9602C_SERIES)

			{
				PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((data.entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data.entry.ipmcEntry.dip>>16)&0xff,(data.entry.ipmcEntry.dip>>8)&0xff,(data.entry.ipmcEntry.dip)&0xff);
				PROC_PRINTF("sip=%d.%d.%d.%d %s%d\n",(data.entry.ipmcEntry.sip>>24)&0xff,(data.entry.ipmcEntry.sip>>16)&0xff,(data.entry.ipmcEntry.sip>>8)&0xff,(data.entry.ipmcEntry.sip)&0xff,
					(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?"IVL VID=":"SVL FID=",
					(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?data.entry.ipmcEntry.vid:data.entry.ipmcEntry.fid);
				PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",data.entry.ipmcEntry.portmask.bits[0],data.entry.ipmcEntry.ext_portmask.bits[0]);
				PROC_PRINTF("l3_trans_index = %x\n",data.entry.ipmcEntry.l3_trans_index);
				PROC_PRINTF("sip_filter_en=%d static=%d ipv6=%d  wan_sa=%d\n",(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0,
					(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0,
					(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IPV6)?1:0,
					(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0 );

			}


			
#elif defined(CONFIG_RTL9600_SERIES)
            if(!(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY))				
            {
                PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((data.entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data.entry.ipmcEntry.dip>>16)&0xff,(data.entry.ipmcEntry.dip>>8)&0xff,(data.entry.ipmcEntry.dip)&0xff);
                PROC_PRINTF("sip=%d.%d.%d.%d cvid=%d\n",(data.entry.ipmcEntry.sip>>24)&0xff,(data.entry.ipmcEntry.sip>>16)&0xff,(data.entry.ipmcEntry.sip>>8)&0xff,(data.entry.ipmcEntry.sip)&0xff,data.entry.ipmcEntry.vid);
                PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",data.entry.ipmcEntry.portmask.bits[0],data.entry.ipmcEntry.ext_portmask.bits[0]);
                PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                               ,data.entry.ipmcEntry.priority
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0);
            }
            else
            {
                PROC_PRINTF("[P4] gip=%d.%d.%d.%d\n",((data.entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data.entry.ipmcEntry.dip>>16)&0xff,(data.entry.ipmcEntry.dip>>8)&0xff,(data.entry.ipmcEntry.dip)&0xff);
                PROC_PRINTF("mbr=0x%x extmbr=0x%x l3trans=0x%x\n",data.entry.ipmcEntry.portmask.bits[0],data.entry.ipmcEntry.ext_portmask.bits[0],data.entry.ipmcEntry.l3_trans_index);
                PROC_PRINTF("lutpri=%d fwdpri_en=%d dip_only=%d ext_fr=%d wan_sa=%d static=%d\n"
                               ,data.entry.ipmcEntry.priority
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY)?1:0
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FORCE_EXT_ROUTE)?1:0
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0
                               ,(data.entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0
                              );
            }
#endif			

        }
#if defined(CONFIG_RTL9602C_SERIES)
		else if (data.entryType==RTK_LUT_L3V6MC)
		{

			
				PROC_PRINTF("[P4] v6ip= ---%02x(dip117_112)%02x(dip111_104)-----%02x(dip43_40)%02x:%02x%02x:%02x%02x \n",
					data.entry.ipmcEntry.dip6.ipv6_addr[1]&0x3f,data.entry.ipmcEntry.dip6.ipv6_addr[2],data.entry.ipmcEntry.dip6.ipv6_addr[10]&0xf,
					data.entry.ipmcEntry.dip6.ipv6_addr[11],data.entry.ipmcEntry.dip6.ipv6_addr[12],data.entry.ipmcEntry.dip6.ipv6_addr[13],
					data.entry.ipmcEntry.dip6.ipv6_addr[14],data.entry.ipmcEntry.dip6.ipv6_addr[15]);
				PROC_PRINTF("mbr=0x%x  ext_mbr=0x%x	l3mcr_idx=%d \n", data.entry.ipmcEntry.portmask.bits[0],data.entry.ipmcEntry.ext_portmask.bits[0],data.entry.ipmcEntry.l3_mcr_index);
				PROC_PRINTF("notsalearn=%d  l3lookup=%d  ip6=%d  wan_sa=%d\n",
					data.entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_STATIC,1,data.entry.ipmcEntry.flags &RTK_L2_IPMCAST_FLAG_IPV6  ,
					data.entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN);

		}
#endif
		
    }


	return SUCCESS;
}

int32 dump_lut_table(struct seq_file *s, void *v)
{
    int i;
	int r;
	int len=0;
    for(i=0; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        r=dump_lut(i,s);
		if(r==FAIL) break;
    }
#if defined(CONFIG_RTL9602C_SERIES) && defined(CONFIG_APOLLO_FPGA_PHY_TEST)
	{	
		rtk_rg_lut_linkList_t *plutCamEntry;
		PROC_PRINTF("\n>>lutCam list:\n");
		for(i=0;i<(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>2;i++)
		{
			if(!list_empty(&rg_db.lutBCAMTableHead[i]))
			{
				PROC_PRINTF("  L2_Hash[%3d] : \n",i);
				list_for_each_entry(plutCamEntry,&rg_db.lutBCAMTableHead[i],lut_list)
				{
					PROC_PRINTF("	 LutCam idx[%d]\n", plutCamEntry->idx);
				}
				PROC_PRINTF("\n");
			}
		}
		PROC_PRINTF(">>lutCam Free list:\n");
		list_for_each_entry(plutCamEntry,&rg_db.lutBCAMFreeListHead,lut_list)
		{
			PROC_PRINTF("LutCam idx[%d]\n", plutCamEntry->idx);
		}
		PROC_PRINTF("\n");
	}
#endif

    return len;
}

int32 dump_lut_bcam_table(struct seq_file *s, void *v)
{
    int i;
	int len=0;
    for(i=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        dump_lut(i,s);
    }
    return len;
}

void _mibdump_lut(int idx,struct seq_file *s)
{
	rtk_l2_addr_table_t	*pdata;

	
   	pdata=&rg_db.lut[idx].rtk_lut;
	PROC_PRINTF("LUT idx=%d",idx);
    if(pdata->entryType==RTK_LUT_L2UC)
    {
    	//static entry won't show category
    	if((pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0)
    	{
			if(_rtK_rg_checkCategoryPortmask(&pdata->entry.l2UcEntry)==SUCCESS)
    			PROC_PRINTF(", Category<%d>",rg_db.lut[idx].category);

			//display wlan_dev_index, only used when this MAC is wifi host
			if(pdata->entry.l2UcEntry.port==RTK_RG_PORT_CPU && pdata->entry.l2UcEntry.ext_port!=(RTK_RG_PORT_CPU-RTK_RG_PORT_CPU))
				PROC_PRINTF(", wlan_dev_idx=%d",rg_db.lut[idx].wlan_device_idx);
		}
		if(rg_db.systemGlobal.fix_l34_to_untag_enable==RG_HWNAT_ENABLE && rg_db.lut[idx].fix_l34_vlan)
			PROC_PRINTF(", remove L34 tag");
		if(rg_db.lut[idx].redirect_http_req){
			if(rg_db.lut[idx].redirect_http_req==1)PROC_PRINTF(", TRAP FIRST HTTP[Def]");	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
			else PROC_PRINTF(", TRAP FIRST HTTP[%d]",rg_db.lut[idx].redirect_http_req-2);	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
		}
		PROC_PRINTF("\n");
        PROC_PRINTF("[P1] mac=%02x:%02x:%02x:%02x:%02x:%02x cvid=%d l3lookup=%d ivl=%d\n"
                       ,pdata->entry.l2UcEntry.mac.octet[0]
                       ,pdata->entry.l2UcEntry.mac.octet[1]
                       ,pdata->entry.l2UcEntry.mac.octet[2]
                       ,pdata->entry.l2UcEntry.mac.octet[3]
                       ,pdata->entry.l2UcEntry.mac.octet[4]
                       ,pdata->entry.l2UcEntry.mac.octet[5]
                       ,pdata->entry.l2UcEntry.vid
                       ,0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL)?1:0);

#if defined(CONFIG_RTL9600_SERIES)
#elif defined(CONFIG_RTL9602C_SERIES)
		PROC_PRINTF("ctagif=%d ",(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0);
#endif

        PROC_PRINTF("efid=%d fid=%d sapri_en=%d spa=%d age=%d auth1x=%d sablock=%d\n"
                       ,pdata->entry.l2UcEntry.efid
                       ,pdata->entry.l2UcEntry.fid
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0
                       ,pdata->entry.l2UcEntry.port
                       ,pdata->entry.l2UcEntry.age
                       ,pdata->entry.l2UcEntry.auth
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0);

        PROC_PRINTF("dablock=%d ext_spa=%d arp_used=%d lutpri_en=%d lutpri=%d fwdpri_en=%d notsalearn=%d \n"
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0
                       ,pdata->entry.l2UcEntry.ext_port
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)?1:0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_LOOKUP_PRI)?1:0
                       ,pdata->entry.l2UcEntry.priority
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0);

    }
    else if(pdata->entryType==RTK_LUT_L2MC)
    {
    	//MC entry won't show category
        PROC_PRINTF("\n[P2] mac=%02x:%02x:%02x:%02x:%02x:%02x ivl=%d vid=%d fid=%d l3lookup=%d ivl=%d\n"
                       ,pdata->entry.l2McEntry.mac.octet[0]
                       ,pdata->entry.l2McEntry.mac.octet[1]
                       ,pdata->entry.l2McEntry.mac.octet[2]
                       ,pdata->entry.l2McEntry.mac.octet[3]
                       ,pdata->entry.l2McEntry.mac.octet[4]
                       ,pdata->entry.l2McEntry.mac.octet[5]
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0
                       ,pdata->entry.l2McEntry.vid
                       ,pdata->entry.l2McEntry.fid
                       ,0
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0);

        PROC_PRINTF("mbr=0x%x extmbr=0x%x\n"
                       ,pdata->entry.l2McEntry.portmask.bits[0]
                       ,pdata->entry.l2McEntry.ext_portmask.bits[0]);

        PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                       ,pdata->entry.l2McEntry.priority
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_FWD_PRI)?1:0);

    }
    else if(pdata->entryType==RTK_LUT_L3MC)
    {

#if defined(CONFIG_RTL9602C_SERIES)
		{
			PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
			PROC_PRINTF("sip=%d.%d.%d.%d %s%d\n",(pdata->entry.ipmcEntry.sip>>24)&0xff,(pdata->entry.ipmcEntry.sip>>16)&0xff,(pdata->entry.ipmcEntry.sip>>8)&0xff,(pdata->entry.ipmcEntry.sip)&0xff,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?"IVL VID=":"SVL FID=",
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?pdata->entry.ipmcEntry.vid:pdata->entry.ipmcEntry.fid);
			PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0]);
			PROC_PRINTF("l3_trans_index = %x\n",pdata->entry.ipmcEntry.l3_trans_index);
			PROC_PRINTF("sip_filter_en=%d static=%d ipv6=%d  wan_sa=%d\n",(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IPV6)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0 );

		}

#else
		//MC entry won't show category
		PROC_PRINTF("\n");
        if(!(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY))
        {
            PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
            PROC_PRINTF("sip=%d.%d.%d.%d vid=%d\n",(pdata->entry.ipmcEntry.sip>>24)&0xff,(pdata->entry.ipmcEntry.sip>>16)&0xff,(pdata->entry.ipmcEntry.sip>>8)&0xff,(pdata->entry.ipmcEntry.sip)&0xff,pdata->entry.ipmcEntry.vid);
            PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0]);
            PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                           ,pdata->entry.ipmcEntry.priority
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0);
        }
        else
        {
            PROC_PRINTF("[P4] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
            PROC_PRINTF("mbr=0x%x extmbr=0x%x l3trans=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0],pdata->entry.ipmcEntry.l3_trans_index);
            PROC_PRINTF("lutpri=%d fwdpri_en=%d dip_only=%d ext_fr=%d wan_sa=%d notsalearn=%d\n"
                           ,pdata->entry.ipmcEntry.priority
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FORCE_EXT_ROUTE)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0
                          );
        }
#endif


    }
#if defined(CONFIG_RTL9602C_SERIES)
	else if (pdata->entryType==RTK_LUT_L3V6MC)
	{
		PROC_PRINTF("[P4] v6ip= ---%02x(dip117_112)%02x(dip111_104)-----%02x(dip43_40)%02x:%02x%02x:%02x%02x \n",
			pdata->entry.ipmcEntry.dip6.ipv6_addr[1]&0x3f,pdata->entry.ipmcEntry.dip6.ipv6_addr[2],pdata->entry.ipmcEntry.dip6.ipv6_addr[10]&0xf,
			pdata->entry.ipmcEntry.dip6.ipv6_addr[11],pdata->entry.ipmcEntry.dip6.ipv6_addr[12],pdata->entry.ipmcEntry.dip6.ipv6_addr[13],
			pdata->entry.ipmcEntry.dip6.ipv6_addr[14],pdata->entry.ipmcEntry.dip6.ipv6_addr[15]);
		PROC_PRINTF("mbr=0x%x  ext_mbr=0x%x	l3mcr_idx=%d \n", pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0],pdata->entry.ipmcEntry.l3_mcr_index);
		PROC_PRINTF("notsalearn=%d  l3lookup=%d  ip6=%d  wan_sa=%d\n",
			pdata->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_STATIC,1,pdata->entry.ipmcEntry.flags &RTK_L2_IPMCAST_FLAG_IPV6  ,
			pdata->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN);
	}
		
#endif
}

void mibdump_lut(int idx,struct seq_file *s)
{
    if(rg_db.lut[idx].valid==1)
    {
        PROC_PRINTF("--------------- LUT TABLE (%d)----------------\n",idx);
		_mibdump_lut(idx,s);
    }
}
int32 mibdump_lut_table(struct seq_file *s, void *v)
{
    int i;
	rtk_rg_lut_linkList_t *pSoftwareLut;
	int len=0;

    for(i=0; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        mibdump_lut(i,s);
    }

	PROC_PRINTF(">>>>>>Lut Link List:<<<<<<\n");
	for(i=0;i<MAX_LUT_SW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.softwareLutTableHead[i]))
		{
			PROC_PRINTF("  hashIdx=[%3d] : \n",i);
			list_for_each_entry(pSoftwareLut,&rg_db.softwareLutTableHead[i],lut_list)
			{
				PROC_PRINTF("----------------------------------------------\n");
				_mibdump_lut(pSoftwareLut->idx,s);
			}
		}
	}
    return len;
}

int32 mibdump_lut_bcam_table(struct seq_file *s, void *v)
{
    int i;
	int len=0;
    for(i=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        mibdump_lut(i,s);
    }
    return len;
}

int32 mibdump_softwareLearningCategories(struct seq_file *s, void *v)
{
	int i,j;
	struct print_config
	{
		unsigned int printed:1;
		unsigned int software:1;
	}pconfig={0};
	int len=0;

	PROC_PRINTF(">>access WAN Category statistics:\n");
	for(i=0;i<WanAccessCategoryNum;i++)
	{		
		pconfig.printed=0;
		pconfig.software=0;
		for(j=0;j<MAX_LUT_SW_TABLE_SIZE;j++)
		{
			if(rg_db.lut[j].valid && rg_db.lut[j].rtk_lut.entryType==RTK_LUT_L2UC && rg_db.lut[j].category==i &&
				(rg_db.lut[j].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0)
			{
				//Check if the MAC is under LAN portmask
				if(_rtK_rg_checkCategoryPortmask(&rg_db.lut[j].rtk_lut.entry.l2UcEntry)==SUCCESS)
				{
					//This category has at least one entry, print the title!!
					if(!pconfig.printed)
					{
						PROC_PRINTF("===============================================================Category <%d>\n",i);
						pconfig.printed=1;
					}
					if(!pconfig.software && j>=MAX_LUT_HW_TABLE_SIZE)
					{
						PROC_PRINTF(">>>>Software Lut:\n");
						pconfig.software=1;
					}
					mibdump_lut(j,s);
				}
			}
		}
	}

	return len;
}

int32 mibdump_softwareLearningStatistics(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	

	PROC_PRINTF(">>access WAN Port learning statistics:\n");
	for(i=0;i<RTK_RG_EXT_PORT0;i++)
	{
#ifdef __KERNEL__
		PROC_PRINTF("  port[%d] ",i);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
		{
			PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);
		}
		else
		{
			PROC_PRINTF("Unlimited");
		}
		PROC_PRINTF(", count is %04d, limitReachAction is %s\n",
			atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]),
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#else
		PROC_PRINTF("  port[%d] ",i);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
 			PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);
		else
			PROC_PRINTF("Unlimited");
		PROC_PRINTF", limitReachAction is %s\n",
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#endif
	}
	//Extension port
	for(i=RTK_RG_EXT_PORT0;i<RTK_RG_PORT_MAX;i++)
	{
#ifdef __KERNEL__
		PROC_PRINTF("  EXTport[%d] ",i-RTK_RG_EXT_PORT0);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)		
			{PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);}
		else
			{PROC_PRINTF("Unlimited");}
		PROC_PRINTF(", count is %04d, limitReachAction is %s\n",			
			atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]),
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#else
		PROC_PRINTF("  EXTport[%d] ",i-RTK_RG_EXT_PORT0);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
			PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);
		else
			PROC_PRINTF("Unlimited");
		PROC_PRINTF(", limitReachAction is %s\n",
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#endif
	}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Wlan0 device
	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("  Wlan0Dev[%d] ",i);
			if(rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[i]>=0)
				{PROC_PRINTF("limit is %04d",rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[i]);}
			else
				{PROC_PRINTF("Unlimited");}
			PROC_PRINTF(", count is %04d, limitReachAction is %s\n",			
				atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[i]),
				rg_db.systemGlobal.wlan0SourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.wlan0SourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
		}
	}
#endif

	return len;
}
//reverse port mask translator
int _rtk_rg_portmask_re_translator(rtk_rg_portmask_t *out_pmask, rtk_portmask_t in_mac_pmask, rtk_portmask_t in_ext_pmask){
	int i;
	int EXT_CPU_PORT_flag = DISABLED;
	
	if(out_pmask==NULL)
		RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	
	bzero(out_pmask,sizeof(rtk_portmask_t));
	for(i=0;i<RTK_RG_MAX_MAC_PORT;i++){
		if(in_mac_pmask.bits[0] & (1<<i))
		{
			out_pmask->portmask |= (1<<i);
			if(i==6) EXT_CPU_PORT_flag = ENABLED;	//CPU port is enable
		}
	}
	//set ext portmask
	for(i=0;i<RTK_RG_MAX_MAC_PORT;i++){
		if(in_ext_pmask.bits[0] & (1<<i)){
			out_pmask->portmask |= (1<<(i+(RTK_RG_MAX_MAC_PORT-1)));
			EXT_CPU_PORT_flag = ENABLED;//ENABLE MAC_CPU_PORT if any EXT_PORT is ENABLED
		}
	}
	if(EXT_CPU_PORT_flag==ENABLED)
		out_pmask->portmask |= (1<<(RTK_RG_MAX_MAC_PORT-1));	//check cpu port for all cases if CPU port is enable
	return (RT_ERR_RG_OK);
	
}

//rg command line should be close to the line-start.
static char tmpStr[255];
static char buf[32];
int32 diag_shell_dump(struct seq_file *s, void *v)
{
	int i,j;
	int len=0;
	rtk_enable_t state;

	PROC_PRINTF("#dump init\n");
	if(rg_db.systemGlobal.initParam.fwdVLAN_CPU || rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block)
	{
		if((rg_db.systemGlobal.initParam.initByHwCallBack==&_rtk_rg_initParameterSetByHwCallBack) 
		&& (rg_db.systemGlobal.initParam.arpAddByHwCallBack==&_rtk_rg_arpAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.arpDelByHwCallBack==&_rtk_rg_arpDelByHwCallBack) 
		&& (rg_db.systemGlobal.initParam.macAddByHwCallBack==&_rtk_rg_macAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.macDelByHwCallBack==&_rtk_rg_macDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.routingAddByHwCallBack==&_rtk_rg_routingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.routingDelByHwCallBack==&_rtk_rg_routingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.naptAddByHwCallBack==&_rtk_rg_naptAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.naptDelByHwCallBack==&_rtk_rg_naptDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.bindingAddByHwCallBack==&_rtk_rg_bindingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.bindingDelByHwCallBack==&_rtk_rg_bindingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceAddByHwCallBack==&_rtk_rg_interfaceAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceDelByHwCallBack==&_rtk_rg_interfaceDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.neighborAddByHwCallBack==&_rtk_rg_neighborAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.neighborDelByHwCallBack==&_rtk_rg_neighborDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.v6RoutingAddByHwCallBack==&_rtk_rg_v6RoutingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.v6RoutingDelByHwCallBack==&_rtk_rg_v6RoutingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack==&_rtk_rg_pppoeBeforeDiagByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pptpBeforeDialByHwCallBack==&_rtk_rg_pptpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.l2tpBeforeDialByHwCallBack==&_rtk_rg_l2tpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack==&_rtk_rg_dhcpRequestByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeDsliteBeforeDialByHwCallBack==&_rtk_rg_pppoeDsliteBeforeDialByHwCallBack))
		{
PROC_PRINTF("rg init callback default igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d ivlMulticastSupport %d fwdVlan_CPU %d fwdVlan_Proto_Block %d\n\n",
	rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
	rg_db.systemGlobal.initParam.ivlMulticastSupport,rg_db.systemGlobal.initParam.fwdVLAN_CPU,rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block);
		}
		else
PROC_PRINTF("rg init callback igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d ivlMulticastSupport %d\n\n",
	rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
	rg_db.systemGlobal.initParam.ivlMulticastSupport);
	}
	else{
		if((rg_db.systemGlobal.initParam.initByHwCallBack==&_rtk_rg_initParameterSetByHwCallBack) 
		&& (rg_db.systemGlobal.initParam.arpAddByHwCallBack==&_rtk_rg_arpAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.arpDelByHwCallBack==&_rtk_rg_arpDelByHwCallBack) 
		&& (rg_db.systemGlobal.initParam.macAddByHwCallBack==&_rtk_rg_macAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.macDelByHwCallBack==&_rtk_rg_macDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.routingAddByHwCallBack==&_rtk_rg_routingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.routingDelByHwCallBack==&_rtk_rg_routingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.naptAddByHwCallBack==&_rtk_rg_naptAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.naptDelByHwCallBack==&_rtk_rg_naptDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.bindingAddByHwCallBack==&_rtk_rg_bindingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.bindingDelByHwCallBack==&_rtk_rg_bindingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceAddByHwCallBack==&_rtk_rg_interfaceAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceDelByHwCallBack==&_rtk_rg_interfaceDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.neighborAddByHwCallBack==&_rtk_rg_neighborAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.neighborDelByHwCallBack==&_rtk_rg_neighborDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.v6RoutingAddByHwCallBack==&_rtk_rg_v6RoutingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.v6RoutingDelByHwCallBack==&_rtk_rg_v6RoutingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack==&_rtk_rg_pppoeBeforeDiagByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pptpBeforeDialByHwCallBack==&_rtk_rg_pptpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.l2tpBeforeDialByHwCallBack==&_rtk_rg_l2tpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack==&_rtk_rg_dhcpRequestByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeDsliteBeforeDialByHwCallBack==&_rtk_rg_pppoeDsliteBeforeDialByHwCallBack))
		{
PROC_PRINTF("rg init callback default igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d ivlMulticastSupport %d fwdVLAN_CPU %d fwdVlan_Proto_Block %d\n\n",
	rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
	rg_db.systemGlobal.initParam.ivlMulticastSupport,rg_db.systemGlobal.initParam.fwdVLAN_CPU,rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block);
		}
		else
PROC_PRINTF("rg init callback igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d ivlMulticastSupport %d\n\n",
	rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
	rg_db.systemGlobal.initParam.ivlMulticastSupport);
	}
	PROC_PRINTF("#wan/lan setting\n");
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
	{
		if(rg_db.systemGlobal.interfaceInfo[i].valid == 1)
		{
			if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan == 0)	//LAN
			{
				if(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr!=0L)	//if no ip, skip it
				{
					memset(tmpStr,0,255);
					if(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->vlan_based_pri_enable)
						sprintf(tmpStr,"enable vlan-based-pri %d",rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->vlan_based_pri);
					else
						sprintf(tmpStr,"disable");
PROC_PRINTF("rg set lan-intf ip-version %d gateway-mac %02x:%02x:%02x:%02x:%02x:%02x ip-addr %d.%d.%d.%d ip-mask %d.%d.%d.%d ipv6-addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_network_mask_length %d port-mask 0x%x untag-mask 0x%x intf-vlan_id %d vlan-based-pri-enable %s mtu %d isIVL %d\n\n",
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_version,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[5],
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[0]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[1]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[2]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[3]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[4]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[5]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[6]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[7]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[8]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[9]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[10]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[11]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[12]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[13]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[14]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[15]),
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_network_mask_length,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->port_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->untag_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->intf_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->mtu,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->isIVL);
PROC_PRINTF("rg add lan-intf entry\n\n");
				}
			}
			else	//WAN
			{
				memset(tmpStr,0,255);
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri_enable)
					sprintf(tmpStr,"enable vlan-based-pri %d",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri);
				else
					sprintf(tmpStr,"disable");
PROC_PRINTF("rg set wan-intf wan-type %d gateway-mac  %02x:%02x:%02x:%02x:%02x:%02x wan-port %d port-binding-mask 0x%x egress-vlan-tag-on %d egress-vlan-id %d vlan-based-pri-enable %s isIVL %d %s wlan0-binding-mask 0x%x\n\n",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD?RTK_RG_EXT_PORT2:
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD?RTK_RG_EXT_PORT3:rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.isIVL,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.none_internet ? "none_internet":"",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wlan0_dev_binding_mask);
PROC_PRINTF("rg add wan-intf entry\n\n");

				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr!=0L)
					{
PROC_PRINTF("rg set dhcpClientInfo stauts %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.stauts,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.mtu,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[5]);
						setFlag = 1;
					}
					
					if(!(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_version==IPVER_V4ONLY || 
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[15]==0))
					{
PROC_PRINTF("rg set dhcpClientInfo-ipv6 ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[7],					
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[5]);
						setFlag = 1;
					}
					
					if(setFlag==1)
PROC_PRINTF("rg add dhcpClientInfo wan_intf_idx %d\n\n",i);
					
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC)
				{
					uint8 setFlag = 0;
					//if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr!=0L)
					//{
PROC_PRINTF("rg set wan-intf-static-info ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[5]);
					setFlag = 1;
					//}
					
					if(!(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version==IPVER_V4ONLY || rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]==0))
					{
PROC_PRINTF("rg set wan-intf-static-info-ipv6 ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x\n\n",
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[5]);
					setFlag = 1;
					}
					if(setFlag==1)
PROC_PRINTF("rg add wan-intf-static-info intf-index %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)	//pppoe 
				{
					uint8 setFlag = 0;						
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr != 0L)
					{
PROC_PRINTF("rg set pppoeClientInfoAfterDial sessionId %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.sessionId,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.mtu,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[5]);
					setFlag = 1;
					}
					if(!(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_version==IPVER_V4ONLY  || rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[15] == 0))
					{
						if(!((rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[0]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[1]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[2]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[3]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[4]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[5]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[6]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[7]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[8]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[9]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[10]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[11]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[12]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[13]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[14]==0) &&
							(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[15]==0))){
PROC_PRINTF("rg set pppoeClientInfoAfterDial-ipv6 ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[7],					
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[5]);
					setFlag = 1;
					}
					}
					if(setFlag == 1)
PROC_PRINTF("rg add pppoeClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)	//pptp
				{
#if 1
					PROC_PRINTF("rg set pptpClientInfoBeforeDial username %s password %s pptp_server_addr_for_ipv4 %d.%d.%d.%d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.username,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.password,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff));						
					PROC_PRINTF("rg add pptpClientInfoBeforeDial wan_intf_idx %d\n\n",i);
#endif
					PROC_PRINTF("rg set pptpClientInfoAfterDial callId %d gateway_callId %d ipv4_addr %d.%d.%d.%d ipv4_network_mask %d.%d.%d.%d gateway_ipv4_addr %d.%d.%d.%d mtu %d ipv4_napt_enable %d ipv4_default_gateway_on %d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.callId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.gateway_callId,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask& 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.mtu,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.napt_enable,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ipv4_default_gateway_on);
					PROC_PRINTF("rg add pptpClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP)	//l2tp
				{
#if 1
					PROC_PRINTF("rg set l2tpClientInfoBeforeDial username %s password %s l2tp_server_addr_for_ipv4 %d.%d.%d.%d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.username,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.password,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff));
					PROC_PRINTF("rg add l2tpClientInfoBeforeDial wan_intf_idx %d\n\n",i);
#endif
					PROC_PRINTF("rg set l2tpClientInfoAfterDial outer_port %d gateway_outer_port %d tunnelId %d sessionId %d gateway_tunnelId %d gateway_sessionId %d ipv4_addr %d.%d.%d.%d ipv4_network_mask %d.%d.%d.%d gateway_ipv4_addr %d.%d.%d.%d mtu %d ipv4_napt_enable %d ipv4_default_gateway_on %d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.outer_port,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_outer_port,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.tunnelId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.sessionId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_tunnelId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_sessionId,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask& 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.mtu,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.napt_enable,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ipv4_default_gateway_on);
					PROC_PRINTF("rg add l2tpClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE)	//dslite 
				{
					uint8 setFlag = 0;
					//if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr!=0L)
					//{
PROC_PRINTF("rg set wan-intf-dslite-info ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d mtu %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_version,				
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.napt_enable,					
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv4_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.mtu);
					setFlag = 1;
					//}
	
					if(!(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version==IPVER_V4ONLY || rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]==0))
					{
#if defined(CONFIG_RTL9600_SERIES)
PROC_PRINTF("rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x\n\n",
#else

						if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS)
							snprintf(buf,32,"copy_from_IPv4_TOS");
						else
							snprintf(buf,32,"0x%x",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.tc);
PROC_PRINTF("rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x dslite_hoplimit %d dslite_flowlabel 0x%x dslite_tc %s\n\n",
#endif
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.mtu,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[5],
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[15]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_auto_learn,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[5]
#if defined(CONFIG_RTL9600_SERIES)					
					);
#else
					,rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.hopLimit,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.flowLabel,
					buf);
#endif

					setFlag = 1;
					}
				
					if(setFlag==1)
PROC_PRINTF("rg add wan-intf-dslite-info intf-index %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)	//pppoe dslite 
				{
					uint8 setFlag = 0;						
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr != 0L)
					{
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial sessionId %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d mtu %d \n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.sessionId,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv4_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.mtu);
					setFlag = 1;
					}
					if(!(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_version==IPVER_V4ONLY  || rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[15] == 0))
					{
#if defined(CONFIG_RTL9600_SERIES)
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x\n\n",
#else
						if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS)
							snprintf(buf,32,"copy_from_IPv4_TOS");
						else
							snprintf(buf,32,"0x%x",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tc);
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x dslite_hoplimit %d dslite_flowlabel 0x%x dslite_tc %s\n\n",
#endif
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_napt_enable,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[7],					
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[7],					
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[7],					
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_auto_learn,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[5]
#if defined(CONFIG_RTL9600_SERIES)					
					);
#else
					,rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.hopLimit,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.flowLabel, 
					buf);
#endif

					setFlag = 1;
					}
					
					if(setFlag == 1)
						PROC_PRINTF("rg add pppoeDsliteInfoAfterDial wan_intf_idx %d\n\n",i);
				}				
			}
		}
	}
	PROC_PRINTF("#add virtual server entry\n");
	for(i=0;i<rg_db.systemGlobal.virtualServerTotalNum;i++){
	//if(rg_db.systemGlobal.virtualServerGroup[i].local_ip!=0)
		{
			PROC_PRINTF("rg set virtualServer is_tcp %d wan_intf_idx %d gateway_port_start %d local_ip %d.%d.%d.%d local_port_start %d mappingPortRangeCnt %d mappingType %d valid %d hookAlgType 0x%x disable_wan_check %d\n\n",
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->is_tcp,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->wan_intf_idx,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->gateway_port_start,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff000000)>>24,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff0000)>>16,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff00)>>8,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff),
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_port_start,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->mappingPortRangeCnt,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->mappingType,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->valid,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->hookAlgType,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->disable_wan_check
					
					);		
			PROC_PRINTF("rg add virtualServer entry\n\n");
		}
	}
	PROC_PRINTF("\n#set perPortBase Limit to WAN\n");
	for(i=0;i<RTK_RG_PORT_MAX;i++)
	{
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>0){
			PROC_PRINTF("rg set softwareSourceAddrLearningLimit learningLimitNumber %d action %d\n",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i],rg_db.systemGlobal.sourceAddrLearningAction[i]);
			PROC_PRINTF("rg add softwareSourceAddrLearningLimit port_idx %d\n",i);
		}
	}
	PROC_PRINTF("\n#set portMask Limit to WAN\n");
	if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask>0){
		//wlan_idx 0 is reserved for slave/master; only 0 could be used now
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member>0)
		{
			PROC_PRINTF("rg add wanAccessLimitPortMask port_mask 0x%x wlan_idx 0 wlan_dev_mask 0x%x learningLimitNumber %d action %d\n",rg_db.systemGlobal.accessWanLimitPortMask_member.portmask,
				rg_db.systemGlobal.accessWanLimitPortMask_wlan0member,
				rg_db.systemGlobal.accessWanLimitPortMask,
				rg_db.systemGlobal.accessWanLimitPortMaskAction);
		}
		else			
#endif
		{
			PROC_PRINTF("rg add wanAccessLimitPortMask port_mask 0x%x learningLimitNumber %d action %d\n",
				rg_db.systemGlobal.accessWanLimitPortMask_member.portmask,
				rg_db.systemGlobal.accessWanLimitPortMask,
				rg_db.systemGlobal.accessWanLimitPortMaskAction);
		}
	}
			
	PROC_PRINTF("\n#set categoryBase Limit to WAN\n");
	for(i=0;i<WanAccessCategoryNum;i++){
		if(rg_db.systemGlobal.accessWanLimitCategory[i]>=0){
			PROC_PRINTF("rg add wanAccessLimitCategory category %d learningLimitNumber %d action %d\n",i,
			rg_db.systemGlobal.accessWanLimitCategory[i],
			rg_db.systemGlobal.accessWanLimitCategoryAction[i]);		
		}
	}

	PROC_PRINTF("\n#set dmzHost\n");
	for(i=0;i<MAX_DMZ_TABLE_SIZE;i++){		//MAX_DMZ_TABLE_SIZE is defined as MAX_NETIF_SW_TABLE_SIZE
		if(rg_db.dmzInfo[i].enabled==1)
		{
			PROC_PRINTF("rg set dmzHost enabled 1 ipversion %d private_ip %d.%d.%d.%d private_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.dmzInfo[i].ipversion,
					(rg_db.dmzInfo[i].private_ip & 0xff000000)>>24,
					(rg_db.dmzInfo[i].private_ip & 0xff0000)>>16,
					(rg_db.dmzInfo[i].private_ip & 0xff00)>>8,
					(rg_db.dmzInfo[i].private_ip & 0xff),
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[0],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[1],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[2],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[3],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[4],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[5],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[6],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[7],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[8],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[9],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[10],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[11],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[12],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[13],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[14],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[15]);
			PROC_PRINTF("rg add dmzHost wan_intf_idx %d\n\n", i);
		}
	}
	PROC_PRINTF("#add cvlan\n");
	for(i=0;i<MAX_VLAN_SW_TABLE_SIZE;i++)
	{
		if(rg_db.vlan[i].valid == 1 && rg_db.vlan[i].addedAsCustomerVLAN == 1)
		{
			rtk_rg_portmask_t tmpmask;
			_rtk_rg_portmask_re_translator(&tmpmask,rg_db.vlan[i].MemberPortmask,rg_db.vlan[i].Ext_portmask);
			if(rg_db.vlan[i].priorityEn == 1)				
			{
				PROC_PRINTF("rg set cvlan vlanId %d isIVL %d memberPortMask 0x%x untagPortMask 0x%x vlan-based-pri-enable enable vlan-based-pri %d\n\n",
						i,(rg_db.vlan[i].fidMode == VLAN_FID_IVL)? 1:0,						
						tmpmask.portmask, rg_db.vlan[i].UntagPortmask.bits[0], rg_db.vlan[i].priority);
			}
			else
			{
				PROC_PRINTF("rg set cvlan vlanId %d isIVL %d memberPortMask 0x%x untagPortMask 0x%x vlan-based-pri-enable disable\n\n",
						i,(rg_db.vlan[i].fidMode == VLAN_FID_IVL)? 1:0,
						tmpmask.portmask, rg_db.vlan[i].UntagPortmask.bits[0]);
			}
									
			PROC_PRINTF("rg add cvlan entry\n");
		}
	}
	PROC_PRINTF("#add pvid\n");
	for(i=0;i<RTK_RG_PORT_MAX;i++)
	{
		if(rg_db.systemGlobal.portBasedVID[i] !=0)
			PROC_PRINTF("rg set port-based-vlan port_idx %d vlanId %d\n\n",
			i,rg_db.systemGlobal.portBasedVID[i]
			);
	}
	PROC_PRINTF("#add port binding\n");
	for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
	{
		if(rg_db.bind[i].rtk_bind.vidLan==0L) continue;
		if(rg_db.bind[i].valid==1)
		{
			
			for(j=0;j<RTK_RG_PORT_MAX;j++)
			{
				if((rg_db.bind[i].rtk_bind.portMask.bits[0] & (1<<j))!=0) break;	//any chance for no portMask bit is set?
			}
			PROC_PRINTF("rg set binding port_idx %d ingress_vid %d wan_intf_idx %d\n\n",j,rg_db.bind[i].rtk_bind.vidLan,rg_db.nexthop[rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.nhIdx].rtk_nexthop.ifIdx);
			PROC_PRINTF("rg add binding entry\n\n");
		}
	}
	PROC_PRINTF("#add alg\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress !=0L)
		{
			PROC_PRINTF("rg set serverInLanAppsIpAddr algType 0x%x serverAddress %d.%d.%d.%d\n\n",
					rg_db.algServInLanIpMapping[i].algType,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff000000)>>24,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff0000)>>16,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff00)>>8,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff)
		);
			PROC_PRINTF("rg add serverInLanAppsIpAddr entry\n\n");
		}
	}
	if(rg_db.algFunctionMask!=0)
	{
		PROC_PRINTF("rg add algApps app_mask 0x%x\n\n",rg_db.algFunctionMask);
	}
	
	PROC_PRINTF("#add url filter\n");
	PROC_PRINTF("rg clear url-filter entry\n\n");
	for(i=0;i<MAX_URL_FILTER_ENTRY_SIZE;i++){
		if(strlen(rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.url_filter_string)!=0)
		{
			PROC_PRINTF("rg set url-filter url %s path %s path-exactly-match %d wan-interface %d\n\n",
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.url_filter_string,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.path_filter_string,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.path_exactly_match,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.wan_intf);
		PROC_PRINTF("rg add url-filter\n\n");
		}
	}
	PROC_PRINTF("#add httpRedirectAll\n");
	if(rg_db.redirectHttpAll.enable!=0){
		PROC_PRINTF("rg set redirectHttpAll enable %d pushweb %s\n",rg_db.redirectHttpAll.enable,rg_db.redirectHttpAll.pushweb);
	}
	PROC_PRINTF("\n#add httpRedirectURL\n");
	if(!list_empty(&rg_db.redirectHttpURLListHead)){
		rtk_rg_redirectHttpURL_linkList_t *pRedEntry;
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpURLListHead,url_list){
			PROC_PRINTF("rg add redirectHttpURL url %s dst_url %s count %d\n",pRedEntry->url_data.url_str,pRedEntry->url_data.dst_url_str,pRedEntry->url_data.count);
		}
	}
	PROC_PRINTF("\n#add httpRedirectWhiteList\n");
	if(!list_empty(&rg_db.redirectHttpWhiteListListHead)){
		rtk_rg_redirectHttpWhiteList_linkList_t *pRedEntry;
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpWhiteListListHead,white_list){
			PROC_PRINTF("rg add redirectHttpWhiteList url %s keyword %s\n",pRedEntry->white_data.url_str,pRedEntry->white_data.keyword_str);
		}
	}
	
	PROC_PRINTF("#add acl-filter\n");	//acl_weight, action(*), fwding_type_and_direction and pattern(*)	
	
	{
		_dump_rg_acl_and_cf_diagshell(s);
	}
	
	
	{
		uint32 svlanTpid;
		uint enable;
		rtk_rg_apollo_svlanTpid_get(&svlanTpid);
PROC_PRINTF("rg add svlanTpid 0x%x\n",svlanTpid);
		for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
		{
			rtk_rg_apollo_svlanServicePort_get(i,&enable);
PROC_PRINTF("rg add svlanServicePort port %d enable %d\n", i, enable);
		}
		PROC_PRINTF("\n");
	}

	PROC_PRINTF("#add dslite multicast\n");
	for(i=0;i<MAX_DSLITEMC_SW_TABLE_SIZE;i++){
		PROC_PRINTF("rg set dsliteMc index %d mprefix64_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mprefix64_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x uprefix64_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x uprefix64_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			rg_db.dsliteMc[i].rtk_dsliteMc.index,
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[0],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[1],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[2],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[3],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[4],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[5],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[6],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[7],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[8],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[9],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[10],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[11],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[12],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[13],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[14],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[15],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[0],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[1],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[2],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[3],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[4],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[5],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[6],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[7],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[8],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[9],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[10],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[11],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[12],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[13],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[14],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[15],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[0],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[1],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[2],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[3],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[4],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[5],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[6],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[7],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[8],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[9],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[10],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[11],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[12],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[13],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[14],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[15],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[0],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[1],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[2],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[3],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[4],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[5],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[6],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[7],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[8],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[9],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[10],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[11],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[12],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[13],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[14],
			rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[15]);
		}
	PROC_PRINTF("\n");

PROC_PRINTF("exit\n");
	{
		PROC_PRINTF("# Difference of rg init (default enable IGMP)\n");
		
		if (rg_db.systemGlobal.BCRateLimitShareMeterIdx != RG_INIT_DEFAULT_BC_rate_limit) { //-1
			PROC_PRINTF("echo %d > proc/rg/BC_rate_limit\n", rg_db.systemGlobal.BCRateLimitShareMeterIdx);
		}
		
		if (rg_db.systemGlobal.BCRateLimitPortMask != RG_INIT_DEFAULT_BC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %d > proc/rg/BC_rate_limit_portMask\n", rg_db.systemGlobal.BCRateLimitPortMask);
		}
		
		if (rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx != RG_INIT_DEFAULT_IPv4_MC_rate_limit) { //-1 disable
			PROC_PRINTF("echo %d > proc/rg/IPv4_MC_rate_limit\n", rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx);
		}
		
		if (rg_db.systemGlobal.IPv4MCRateLimitPortMask != RG_INIT_DEFAULT_IPv4_MC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %d > proc/rg/IPv4_MC_rate_limit_portMask\n", rg_db.systemGlobal.IPv4MCRateLimitPortMask);
		}
		
		if (rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx != RG_INIT_DEFAULT_IPv6_MC_rate_limit) { //-1
			PROC_PRINTF("echo %d > proc/rg/IPv6_MC_rate_limit\n", rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx);
		}
		
		if (rg_db.systemGlobal.IPv6MCRateLimitPortMask != RG_INIT_DEFAULT_IPv6_MC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %d > proc/rg/IPv6_MC_rate_limit_portMask\n", rg_db.systemGlobal.IPv6MCRateLimitPortMask);
		}
		
		if (rg_db.systemGlobal.aclDropIpRangeBySwEnable != RG_INIT_DEFAULT_acl_drop_ip_range_rule_handle_by_sw) { //0: disable
			PROC_PRINTF("echo %d > proc/rg/acl_drop_ip_range_rule_handle_by_sw\n", rg_db.systemGlobal.aclDropIpRangeBySwEnable);
		}
		
		if (rg_db.systemGlobal.aclPermitIpRangeBySwEnable != RG_INIT_DEFAULT_acl_permit_ip_range_rule_handle_by_sw) { //0: disable
			PROC_PRINTF("echo %d > proc/rg/acl_permit_ip_range_rule_handle_by_sw\n", rg_db.systemGlobal.aclPermitIpRangeBySwEnable);
		}
		
		//PROC_PRINTF("echo %d > proc/rg/acl_reserved_arrange\n", rg_db.systemGlobal.acl_reserved_arrange);
		
		if (rg_db.systemGlobal.arp_max_request_count != RG_INIT_DEFAULT_arp_max_request_count) { //0
			PROC_PRINTF("echo %d > proc/rg/arp_max_request_count\n", rg_db.systemGlobal.arp_max_request_count);
		}
		
		if (rg_db.systemGlobal.arp_requset_interval_sec != RG_INIT_DEFAULT_arp_request_interval_sec) { //1
			PROC_PRINTF("echo %d > proc/rg/arp_request_interval_sec\n", rg_db.systemGlobal.arp_requset_interval_sec);
		}
		
		if (rg_db.systemGlobal.arp_timeout != RG_INIT_DEFAULT_arp_timeout) { //300
			PROC_PRINTF("echo %d > proc/rg/arp_timeout\n", rg_db.systemGlobal.arp_timeout);
		}
		
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY] != RG_INIT_DEFAULT_assign_ack_priority_and_disable_svlan) //disable : 0
		{
			PROC_PRINTF("echo %d > proc/rg/assign_ack_priority_and_disable_svlan\n",rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY]);
		}
		
		for (i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++) {
			if (rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]) { //default : 0
				//PROC_PRINTF("Netif[%d]:%s\n",i,rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]==1?"Drop IPv6(IPv4 Pass)":"Drop IPv4(IPv46 Pass)");
				PROC_PRINTF("echo %d %d > proc/rg/bridgeWan_drop_by_protocal\n", i, rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]);
			}
		}
		
		/*
			PROC_PRINTF("echo %d > proc/rg/callback\n");
		*/
		
		if (rg_db.systemGlobal.congestionCtrlIntervalMicroSecs != RG_INIT_DEFAULT_congestion_ctrl_interval_usec) { //0
//			p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//			p->write_proc = (void *)_rtk_rg_proc_congestionCtrlHwTimerFunc_set;
			PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_interval_usec\n", rg_db.systemGlobal.congestionCtrlIntervalMicroSecs);
			
			//if (rg_db.systemGlobal.congestionCtrlInboundAckToHighQueue != 0) { //Always set again
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_inbound_ack_to_high_queue\n", rg_db.systemGlobal.congestionCtrlInboundAckToHighQueue);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlInboundAckToHighQueue_set;
			//}
			
			//if (rg_db.systemGlobal.congestionCtrlPortMask != 0) { //If it is 0, the effect as congestionCtrlIntervalMicroSecs to be 0
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_port_mask\n", rg_db.systemGlobal.congestionCtrlPortMask);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlPortMask_set;
			//}
			
			//if (rg_db.systemGlobal.congestionCtrlSendBytesPerSec[0] != 12500000) { //Use Port0
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_byte_per_sec\n", rg_db.systemGlobal.congestionCtrlSendBytesPerSec[0]);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendBytePerSec_set;
			//}
			
			//if (rg_db.systemGlobal.congestionCtrlSendBytesPerSec[RTK_RG_PORT_PON] != 12500000) { //Use PON port
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_byte_per_sec_for_wan\n", rg_db.systemGlobal.congestionCtrlSendBytesPerSec[RTK_RG_PORT_PON]);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendBytePerSecForWan_set;
			//}
			
			//if (rg_db.systemGlobal.congestionCtrlSendRemainderInNextGap != 0) {
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_remainder_in_next_gap\n", rg_db.systemGlobal.congestionCtrlSendRemainderInNextGap);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendRemainderInNextGap_set;
			//}
			
			//if (rg_db.systemGlobal.congestionCtrlSendTimesPerPort != 0) {
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_times_per_port\n", rg_db.systemGlobal.congestionCtrlSendTimesPerPort); 
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendTimesPerPort_set;
			//}

		}

		ASSERT_EQ(rtk_l34_globalState_get(L34_GLOBAL_TTLMINUS_STATE, &state),RT_ERR_OK);
		if (state != ENABLED) { // 1
			PROC_PRINTF("echo 0 > proc/rg/control_hw_TTL_minus\n");
		}
		
		/*	PROC_PRINTF("echo %d > proc/rg/debug_level\n", rg_db.systemGlobal.debug_level);
			PROC_PRINTF("echo %d > proc/rg/dump_ps_rx_pkt\n", rg_db.systemGlobal.dump_ps_rx_pkt);
			PROC_PRINTF("echo %d > proc/rg/filter_level\n", rg_db.systemGlobal.filter_level);
			PROC_PRINTF("echo %d > proc/rg/fwd_statistic\n");
		*/
		
		if (rg_db.systemGlobal.gponDsBCModuleEnable != RG_INIT_DEFAULT_gponDsBCModuleEnable) { //Default Disable(0)
			PROC_PRINTF("echo %d > proc/rg/gponDsBCModuleEnable\n", rg_db.systemGlobal.gponDsBCModuleEnable);
		}
		
		if (rg_db.systemGlobal.house_keep_sec != RG_INIT_DEFAULT_house_keep_sec) { //2
			PROC_PRINTF("echo %d > proc/rg/house_keep_sec\n", rg_db.systemGlobal.house_keep_sec);
		}
		
		if (rg_db.systemGlobal.hwnat_enable != RG_INIT_DEFAULT_hwnat) { // Default RG_HWNAT_ENABLE:1
			PROC_PRINTF("echo %d > proc/rg/hwnat\n", rg_db.systemGlobal.hwnat_enable);
		}
		
		if (rg_db.systemGlobal.initParam.igmpSnoopingEnable != RG_INIT_DEFAULT_igmpSnooping) { //default on:1
			PROC_PRINTF("echo %d > proc/rg/igmpSnooping\n", rg_db.systemGlobal.initParam.igmpSnoopingEnable);
//			p->read_proc = (void *)igmp_show; 
//			p->write_proc = (void *)_rtk_rg_igmpSnooping_set;
		}
		
		if (rtl_mCastTimerParas.dvmrpRouterAgingTime != RG_INIT_DEFAULT_igmp_dvmrpRouterAgingTime) { //120
			PROC_PRINTF("echo %d > proc/rg/igmp_dvmrpRouterAgingTime\n", rtl_mCastTimerParas.dvmrpRouterAgingTime);
		}
		
		if (rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave != RG_INIT_DEFAULT_igmp_fastLeave) { //default off
			PROC_PRINTF("echo %d > proc/rg/igmp_fastLeave\n", rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave);
		}
		
		/* just show status, can not be set
			PROC_PRINTF("echo %d > proc/rg/igmp_groupList_memDump\n");
		*/
		
		if (rtl_mCastTimerParas.groupMemberAgingTime != RG_INIT_DEFAULT_igmp_groupMemberAgingTime) { //260
			PROC_PRINTF("echo %d > proc/rg/igmp_groupMemberAgingTime\n", rtl_mCastTimerParas.groupMemberAgingTime);
		}
		
		if (rtl_mCastTimerParas.lastMemberAgingTime != RG_INIT_DEFAULT_igmp_lastMemberAgingTime) { //10
			PROC_PRINTF("echo %d > proc/rg/igmp_lastMemberAgingTime\n", rtl_mCastTimerParas.lastMemberAgingTime);
		}
		
		if (rg_db.systemGlobal.igmp_max_simultaneous_group_size != RG_INIT_DEFAULT_igmp_max_simultaneous_group_size) {
			PROC_PRINTF("echo %d > proc/rg/igmp_max_simultaneous_group_size\n", rg_db.systemGlobal.igmp_max_simultaneous_group_size);
		}
		
		
	#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
		//igmp query filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpMldQueryPortmask != RG_INIT_DEFAULT_igmp_mld_query_filter_portmask) { //0xffff
			PROC_PRINTF("echo %d > proc/rg/igmp_mld_query_filter_portmask\n", rg_db.systemGlobal.igmpMldQueryPortmask);
		}
	#endif
		
		if (rtl_mCastTimerParas.mospfRouterAgingTime != RG_INIT_DEFAULT_igmp_mospfRouterAgingTime) { //120
			PROC_PRINTF("echo %d > proc/rg/igmp_mospfRouterAgingTime\n", rtl_mCastTimerParas.mospfRouterAgingTime);
		}
		
		if (rtl_mCastTimerParas.pimRouterAgingTime != RG_INIT_DEFAULT_igmp_pimRouterAgingTime) { //120
			PROC_PRINTF("echo %d > proc/rg/igmp_pimRouterAgingTime\n", rtl_mCastTimerParas.pimRouterAgingTime);
		}
		
		if (rtl_mCastTimerParas.querierPresentInterval != RG_INIT_DEFAULT_igmp_querierPresentInterval) { //260
			PROC_PRINTF("echo %d > proc/rg/igmp_querierPresentInterval\n", rtl_mCastTimerParas.querierPresentInterval);
		}
		
	#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
		//igmp report ingress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpReportIngressPortmask != RG_INIT_DEFAULT_igmp_report_filter_portmask) { //0xffff
			PROC_PRINTF("echo %d > proc/rg/igmp_report_ingress_filter_portmask\n", rg_db.systemGlobal.igmpReportIngressPortmask); 
		}
		
		//igmp report egress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpReportPortmask != RG_INIT_DEFAULT_igmp_report_filter_portmask) { //0xffff
			PROC_PRINTF("echo %d > proc/rg/igmp_report_filter_portmask\n", rg_db.systemGlobal.igmpReportPortmask); 
		}
	#endif
		
		if (rg_db.systemGlobal.igmp_sys_timer_sec != RG_INIT_DEFAULT_igmp_sys_timer_sec) { //10
			PROC_PRINTF("echo %d > proc/rg/igmp_sys_timer_sec\n", rg_db.systemGlobal.igmp_sys_timer_sec);
		}
		
		if (rg_db.systemGlobal.igmp_Trap_to_PS_enable != RG_INIT_DEFAULT_igmp_trap_to_PS) { //Disable
			PROC_PRINTF("echo %d > proc/rg/igmp_trap_to_PS\n", rg_db.systemGlobal.igmp_Trap_to_PS_enable);
		}
		
		if (rg_db.systemGlobal.ipsec_passthru != RG_INIT_DEFAULT_ipsec_passthru) { //PASS_OLD
			PROC_PRINTF("echo %d > proc/rg/ipsec_passthru\n", rg_db.systemGlobal.ipsec_passthru);
		}
		
		if (rg_db.systemGlobal.ipv6MC_translate_ingressVID_enable != RG_INIT_DEFAULT_ipv6MC_tranlate_ingressVid) { //ENABLED
			PROC_PRINTF("echo %d > proc/rg/ipv6MC_tranlate_ingressVid\n", rg_db.systemGlobal.ipv6MC_translate_ingressVID_enable);
		}
		
		if (rg_db.systemGlobal.keepPsOrigCvlan != RG_INIT_DEFAULT_keep_protocol_stack_packets_orig_cvlan) { //default 0: Follow Normal CVLAN Decision
			PROC_PRINTF("echo %d > proc/rg/keep_protocol_stack_packets_orig_cvlan\n", rg_db.systemGlobal.keepPsOrigCvlan);
		}
		
		if (rg_db.systemGlobal.enableL4ChoiceHwIn != RG_INIT_DEFAULT_l4_choice_hw_in) { //current 0
			PROC_PRINTF("echo %d > proc/rg/l4_choice_hw_in\n", rg_db.systemGlobal.enableL4ChoiceHwIn);
		}
		
		if (rg_db.systemGlobal.enableL4MaxWays != RG_INIT_DEFAULT_l4ways) { //current : disable 
			PROC_PRINTF("echo %d > proc/rg/l4ways\n", rg_db.systemGlobal.enableL4MaxWays );
		}
		
		if (rg_db.systemGlobal.enableL4WaysList != RG_INIT_DEFAULT_l4ways_list) {//currrent : 0
			PROC_PRINTF("echo %d > proc/rg/l4ways_list\n", rg_db.systemGlobal.enableL4WaysList);
		}
		
		if (rg_db.systemGlobal.forceReportResponseTime != RG_INIT_DEFAULT_mcast_force_report_sec) { //current : disable
			PROC_PRINTF("echo %d > proc/rg/mcast_force_report_sec\n",rg_db.systemGlobal.forceReportResponseTime);
		}
		
		if (rg_db.systemGlobal.multicastProtocol != RG_INIT_DEFAULT_mcast_protocol) { //current: RG_MC_BOTH_IGMP_MLD
			PROC_PRINTF("echo %d > proc/rg/mcast_protocol\n", rg_db.systemGlobal.multicastProtocol);
		}
		
		if (rg_db.systemGlobal.mcast_query_sec != RG_INIT_DEFAULT_mcast_query_sec) { //default : 30 sec
			PROC_PRINTF("echo %d > proc/rg/mcast_query_sec\n", rg_db.systemGlobal.mcast_query_sec);
		}
		
		if (rg_db.systemGlobal.mld_Trap_to_PS_enable != RG_INIT_DEFAULT_mld_trap_to_PS) { //DISABLED
			PROC_PRINTF("echo %d > proc/rg/mld_trap_to_PS\n", rg_db.systemGlobal.mld_Trap_to_PS_enable);
		}
		
		if (rg_db.systemGlobal.neighbor_timeout != RG_INIT_DEFAULT_neighbor_timeout) { //default : 300 sec
			PROC_PRINTF("echo %d > proc/rg/neighbor_timeout\n", rg_db.systemGlobal.neighbor_timeout);
		}
		
		if (rg_db.systemGlobal.ponPortUnmatchCfDrop != RG_INIT_DEFAULT_pon_port_unmatch_cf_drop) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/pon_port_unmatch_cf_drop\n", rg_db.systemGlobal.ponPortUnmatchCfDrop);
		}
		
	#if defined(CONFIG_APOLLO)
		if (rg_db.systemGlobal.port_binding_by_protocal != RG_INIT_DEFAULT_portBindingByProtocal) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/portBindingByProtocal\n", rg_db.systemGlobal.port_binding_by_protocal);
		}
		
		if (rg_db.systemGlobal.port_binding_by_protocal_filter_vid != RG_INIT_DEFAULT_portBindingByProtocal_filter_downstream_vid) { //current: 0
			PROC_PRINTF("echo %d > proc/rg/portBindingByProtocal_filter_downstream_vid\n", rg_db.systemGlobal.port_binding_by_protocal_filter_vid);
		}
	#endif
		
		if (rg_db.systemGlobal.pppoe_bc_passthrought_to_bindingWan_enable != RG_INIT_DEFAULT_pppoe_bc_passthrought_to_bindingWan) { //current: DISABLED
			PROC_PRINTF("echo %d > proc/rg/pppoe_bc_passthrought_to_bindingWan\n", rg_db.systemGlobal.pppoe_bc_passthrought_to_bindingWan_enable);
		}
		
		if (rg_db.systemGlobal.pppoe_mc_routing_trap != RG_INIT_DEFAULT_pppoe_mc_routing_trap) { //current: DISABLED
			PROC_PRINTF("echo %d > proc/rg/pppoe_mc_routing_trap\n", rg_db.systemGlobal.pppoe_mc_routing_trap);
		}
		
		if (rg_db.systemGlobal.pppoeProxyAllowBindingOnly != RG_INIT_DEFAULT_pppoe_proxy_only_for_binding_packet) { //current : 0/NORMAL
			PROC_PRINTF("echo %d > proc/rg/pppoe_proxy_only_for_binding_packet\n", rg_db.systemGlobal.pppoeProxyAllowBindingOnly);
		}
		
		if (rg_db.systemGlobal.proc_to_pipe != RG_INIT_DEFAULT_proc_to_pipe ) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/proc_to_pipe\n", rg_db.systemGlobal.proc_to_pipe);
		}
		
		if (rg_db.systemGlobal.psRxMirrorToPort0 != RG_INIT_DEFAULT_ps_rx_mirror_to_port0) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/ps_rx_mirror_to_port0\n", rg_db.systemGlobal.psRxMirrorToPort0);
		}
		
		/* only read
			PROC_PRINTF("echo %d > proc/rg/qosInternalAndRemark\n");
		*/
		
	#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RTL9600_SERIES)
		/* only write, cat nothing
			PROC_PRINTF("echo %d > proc/rg/qos_type\n");
		*/
	#endif
	
		/*if (rg_db.systemGlobal.redirect_first_http_req_by_mac != ) {
			PROC_PRINTF("echo %d > proc/rg/redirect_first_http_req_by_mac\n");
		}*/
		
		for (i=0;i<MAX_FORCE_PORTAL_URL_NUM;i++)
			if (rg_db.systemGlobal.forcePortal_url_list[i].valid) {
				bzero(buf,64);
				strncpy(buf, rg_db.systemGlobal.forcePortal_url_list[i].url_string, 64);
				buf[strnlen(buf, 64)-1]='\0';
				PROC_PRINTF("echo a %d '%s' > proc/rg/redirect_first_http_req_set_url\n", i, buf);
			}
		
		if (rg_db.systemGlobal.fix_l34_to_untag_enable != RG_INIT_DEFAULT_remove_l34_tag_for_same_mac) { //default : RG_HWNAT_DISABLE
			PROC_PRINTF("echo %d > proc/rg/remove_l34_tag_for_same_mac\n", rg_db.systemGlobal.fix_l34_to_untag_enable);
		}
		
		/* only for writing and cat nothing, just for debugging purpose
			PROC_PRINTF("echo %d > proc/rg/send_from_cpu\n");
		*/
		//if (rg_kernel.stag_enable != RG_INIT_DEFAULT_stag_enable) {//current : RTK_RG_DISABLED/0
			PROC_PRINTF("# rg_kernel.stag_enable default 0\n");
			PROC_PRINTF("echo %d > proc/rg/stag_enable\n", rg_kernel.stag_enable);
		//}
		
		if (rg_db.systemGlobal.strangeSA_drop != RG_INIT_DEFAULT_strange_packet_drop) { //current : RG_HWNAT_DISABLE / strangeSA_drop=0, PERMIT!
			PROC_PRINTF("echo %d > proc/rg/strange_packet_drop\n", rg_db.systemGlobal.strangeSA_drop);
		}

		if (rg_db.systemGlobal.tcpDisableStatefulTracking != RG_INIT_DEFAULT_tcp_disable_stateful_tracking) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_disable_stateful_tracking\n", rg_db.systemGlobal.tcpDisableStatefulTracking);
		}
		
		if (rg_db.systemGlobal.tcpDoNotDelWhenRstFin != RG_INIT_DEFAULT_tcp_do_not_del_when_rst_fin) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_do_not_del_when_rst_fin\n", rg_db.systemGlobal.tcpDoNotDelWhenRstFin);
		}
		
		if (rg_db.systemGlobal.tcp_hw_learning_at_syn != RG_INIT_DEFAULT_tcp_hw_learning_at_syn) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_hw_learning_at_syn\n", rg_db.systemGlobal.tcp_hw_learning_at_syn);
		}
		
		if (rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn != RG_INIT_DEFAULT_tcp_in_shortcut_learning_at_syn) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_in_shortcut_learning_at_syn\n", rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn);
		}
		
		if (rg_db.systemGlobal.tcp_long_timeout != RG_INIT_DEFAULT_tcp_long_timeout) { //default : 3600
			PROC_PRINTF("echo %d > proc/rg/tcp_long_timeout\n", rg_db.systemGlobal.tcp_long_timeout);
		}
		
		if (rg_db.systemGlobal.tcp_short_timeout != RG_INIT_DEFAULT_tcp_short_timeout) { //default : 20
			PROC_PRINTF("echo %d > proc/rg/tcp_short_timeout\n", rg_db.systemGlobal.tcp_short_timeout);
		}
		
		if (rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies != RG_INIT_DEFAULT_tcp_short_timeout_housekeep_jiffies) {//current : 200
			PROC_PRINTF("echo %d > proc/rg/tcp_short_timeout_housekeep_jiffies\n", rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies);
		}

		/* not need trace filter, just for debugging purpose
			PROC_PRINTF("echo %d > proc/rg/trace_filter\n");
		*/
	
		/* just for special custom using, not general system usage for setting network interface
			PROC_PRINTF("echo %d > proc/rg/trap_lan_add_host\n");
			PROC_PRINTF("echo %d > proc/rg/trap_lan_del_host\n");
		*/
		
		PROC_PRINTF("# trap_lan_enable default 0:disable\n");
		if (rg_kernel.arp_number_for_LAN==0 && rg_kernel.arp_number_for_WAN==MAX_ARP_HW_TABLE_SIZE) {//current 0: disable
			PROC_PRINTF("echo 1 > proc/rg/trap_lan_enable\n");
		}else{
			PROC_PRINTF("echo 0 > proc/rg/trap_lan_enable\n");
		}
		
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP] != RG_INIT_DEFAULT_trap_syn_and_disable_svlan) { //current disable:0
			PROC_PRINTF("echo %d > proc/rg/trap_syn_and_disable_svlan\n", 1);
		}
		
		if (rg_db.systemGlobal.arp_traffic_off != RG_INIT_DEFAULT_turn_off_arp_hw_traffic_info) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_arp_hw_traffic_info\n", rg_db.systemGlobal.arp_traffic_off);
		}
		
	#ifdef CONFIG_ROME_NAPT_SHORTCUT
		if (rg_db.systemGlobal.ipv4_shortcut_off != RG_INIT_DEFAULT_turn_off_ipv4_shortcut) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_ipv4_shortcut\n", rg_db.systemGlobal.ipv4_shortcut_off);
		}
	#endif

	#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
		if (rg_db.systemGlobal.ipv6_shortcut_off != RG_INIT_DEFAULT_turn_off_ipv6_shortcut) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_ipv6_shortcut\n", rg_db.systemGlobal.ipv6_shortcut_off);
		}
	#endif
		
		if (rg_db.systemGlobal.udp_long_timeout != RG_INIT_DEFAULT_udp_long_timeout) { //default : RTK_RG_DEFAULT_UDP_LONG_TIMEOUT/600
			PROC_PRINTF("echo %d > proc/rg/udp_long_timeout\n", rg_db.systemGlobal.udp_long_timeout);
		}
		
		if (rg_db.systemGlobal.udp_short_timeout != RG_INIT_DEFAULT_udp_short_timeout) { //default : RTK_RG_DEFAULT_UDP_SHORT_TIMEOUT/20
			PROC_PRINTF("echo %d > proc/rg/udp_short_timeout\n", rg_db.systemGlobal.udp_short_timeout);
		}
		
		if (rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx != RG_INIT_DEFAULT_unknownDA_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/unknownDA_rate_limit\n", rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx);
		}
		
		if (rg_db.systemGlobal.unKnownDARateLimitPortMask != RG_INIT_DEFAULT_unknownDA_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo %d > proc/rg/unknownDA_rate_limit_portMask\n", rg_db.systemGlobal.unKnownDARateLimitPortMask);
		}
		
		if (rg_db.systemGlobal.unknownDA_Trap_to_PS_enable != RG_INIT_DEFAULT_unknownDA_trap_to_PS) { //current : Disable
			PROC_PRINTF("echo %d > proc/rg/unknownDA_trap_to_PS\n", rg_db.systemGlobal.unknownDA_Trap_to_PS_enable);
		}
		
		if (rg_db.systemGlobal.urlFilterMode != RG_INIT_DEFAULT_urlFilter_mode) { //current : RG_FILTER_BLACK Mode.
			PROC_PRINTF("echo %d > proc/rg/urlFilter_mode\n", 1);
		}

#if defined(CONFIG_APOLLO_GPON_FPGATEST)		
		if (rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask != RG_INIT_DEFAULT_virtualMAC_with_PON) { //current : virtaulMAC with PON: 0/Turn Off.
			PROC_PRINTF("echo %d > proc/rg/virtualMAC_with_PON\n", rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask);
		}
#endif
		
		if (rg_db.systemGlobal.wifiTxRedirect != RG_INIT_DEFAULT_wifi_tx_redirect_to_port0) { // current: 0
			PROC_PRINTF("echo %d > proc/rg/wifi_tx_redirect_to_port0\n", rg_db.systemGlobal.wifiTxRedirect);
		}
		
		/* only dump usage desc. 
		 * reference diag rg init callback default .., please
			PROC_PRINTF("echo %d > proc/rg/wmux_add\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_del\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_flag\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_info\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_init\n");
		*/
	}
	return len;
}

int32 mibdump_accessWanStatistics(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_UNLIMIT)
	{
		PROC_PRINTF(">>access WAN limit statistics:Umlimit\n");
	}
	else
	{
		if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
		{
			PROC_PRINTF(">>access WAN limit statistics:PortMask\n");
			//portmask
			PROC_PRINTF("  portmask 0x%x",rg_db.systemGlobal.accessWanLimitPortMask_member.portmask);			
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			PROC_PRINTF(" wlan0devmask 0x%x",rg_db.systemGlobal.accessWanLimitPortMask_wlan0member);
#endif
			if(rg_db.systemGlobal.accessWanLimitPortMask>=0)
				{PROC_PRINTF("\n  Limit is %04d",rg_db.systemGlobal.accessWanLimitPortMask);}
			else
				{PROC_PRINTF("\n  Unlimited");}
			PROC_PRINTF(", count is %04d, limitReachAction is %s\n",
				atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount),
				rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
		}
		else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_CATEGORY)
		{
			PROC_PRINTF(">>access WAN limit statistics:Category\n");
			//category
			for(i=0;i<WanAccessCategoryNum;i++)
			{
				PROC_PRINTF("  Category[%d] ",i);
				if(rg_db.systemGlobal.accessWanLimitCategory[i]>=0)
					{PROC_PRINTF("Limit is %04d",rg_db.systemGlobal.accessWanLimitCategory[i]);}
				else
					{PROC_PRINTF("Unlimited");}
				PROC_PRINTF(", count is %04d, limitReachAction is %s\n",
					atomic_read(&rg_db.systemGlobal.accessWanLimitCategoryCount[i]),
					rg_db.systemGlobal.accessWanLimitCategoryAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
					rg_db.systemGlobal.accessWanLimitCategoryAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
			}
		}
		else
		{
			PROC_PRINTF(">>access WAN limit statistics:Port\n");
			PROC_PRINTF("Please refer to /proc/dump/sw_layer2LearningStatistics for more information.\n");
		}
	}

	return len;
}

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
int32 mibdump_ipv6_stateful_connections(struct seq_file *s, void *v)
{
	int i,_count;
	rtk_rg_ipv6_layer4_linkList_t *pLayer4List;
	int len=0;
	
	PROC_PRINTF(">>IPv6 Stateful connections:\n");
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
	{
		if(!list_empty(&rg_db.ipv6Layer4HashListHead[i]))
		{
			_count=0;
			list_for_each_entry(pLayer4List,&rg_db.ipv6Layer4HashListHead[i],layer4_list)
			{
				if(pLayer4List->isFrag)continue;
				if(_count==0)PROC_PRINTF("[%d]========================================================\n",i);
				if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND || pLayer4List->direction==IPV6_ROUTE_OUTBOUND)
					PROC_PRINTF("   OutList");
				else
					PROC_PRINTF("   InList ");
				
				PROC_PRINTF("[%p]:(pair[%p]) %s ",pLayer4List,pLayer4List->pPair_list,pLayer4List->isTCP?"TCP":"UDP");
				
				switch(pLayer4List->state)
				{
					case INVALID:
						PROC_PRINTF("state:INVALID ");
						break;
					case SYN_RECV:
						PROC_PRINTF("state:SYN_RECV ");
						break;
					case UDP_FIRST:
						PROC_PRINTF("state:UDP_FIRST ");
						break;
					case SYN_ACK_RECV:
						PROC_PRINTF("state:SYN_ACK_RECV ");
						break;
					case UDP_SECOND:
						PROC_PRINTF("state:UDP_SECOND ");
						break;
					case TCP_CONNECTED:
						PROC_PRINTF("state:TCP_CONNECTED ");
						break;
					case UDP_CONNECTED:
						PROC_PRINTF("state:UDP_CONNECTED ");
						break;
					case FIRST_FIN:
						PROC_PRINTF("state:FIRST_FIN ");
						break;
					case RST_RECV:
						PROC_PRINTF("state:RST_RECV ");
						break;
					default:
						break;
				}
					
				PROC_PRINTF("idle:%d\n     %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d --> \n     %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d\n",
					pLayer4List->idleSecs,
					pLayer4List->srcIP.ipv6_addr[0],pLayer4List->srcIP.ipv6_addr[1],pLayer4List->srcIP.ipv6_addr[2],pLayer4List->srcIP.ipv6_addr[3],
					pLayer4List->srcIP.ipv6_addr[4],pLayer4List->srcIP.ipv6_addr[5],pLayer4List->srcIP.ipv6_addr[6],pLayer4List->srcIP.ipv6_addr[7],
					pLayer4List->srcIP.ipv6_addr[8],pLayer4List->srcIP.ipv6_addr[9],pLayer4List->srcIP.ipv6_addr[10],pLayer4List->srcIP.ipv6_addr[11],
					pLayer4List->srcIP.ipv6_addr[12],pLayer4List->srcIP.ipv6_addr[13],pLayer4List->srcIP.ipv6_addr[14],pLayer4List->srcIP.ipv6_addr[15],
					pLayer4List->srcPort,
					pLayer4List->destIP.ipv6_addr[0],pLayer4List->destIP.ipv6_addr[1],pLayer4List->destIP.ipv6_addr[2],pLayer4List->destIP.ipv6_addr[3],
					pLayer4List->destIP.ipv6_addr[4],pLayer4List->destIP.ipv6_addr[5],pLayer4List->destIP.ipv6_addr[6],pLayer4List->destIP.ipv6_addr[7],
					pLayer4List->destIP.ipv6_addr[8],pLayer4List->destIP.ipv6_addr[9],pLayer4List->destIP.ipv6_addr[10],pLayer4List->destIP.ipv6_addr[11],
					pLayer4List->destIP.ipv6_addr[12],pLayer4List->destIP.ipv6_addr[13],pLayer4List->destIP.ipv6_addr[14],pLayer4List->destIP.ipv6_addr[15],
					pLayer4List->destPort);

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
					if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND){
						PROC_PRINTF("     (ExtIp:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ExtPort:%d)\n",
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[0],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[1],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[2],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[3],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[4],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[5],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[6],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[7],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[8],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[9],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[10],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[11],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[12],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[13],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[14],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[15],
							pLayer4List->externalPort);

					}else if(pLayer4List->direction==NAPT_DIRECTION_INBOUND){
						PROC_PRINTF("     (IntIp:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x IntPort:%d)\n",
							pLayer4List->internalIP.ipv6_addr[0],pLayer4List->internalIP.ipv6_addr[1],pLayer4List->internalIP.ipv6_addr[2],pLayer4List->internalIP.ipv6_addr[3],
							pLayer4List->internalIP.ipv6_addr[4],pLayer4List->internalIP.ipv6_addr[5],pLayer4List->internalIP.ipv6_addr[6],pLayer4List->internalIP.ipv6_addr[7],
							pLayer4List->internalIP.ipv6_addr[8],pLayer4List->internalIP.ipv6_addr[9],pLayer4List->internalIP.ipv6_addr[10],pLayer4List->internalIP.ipv6_addr[11],
							pLayer4List->internalIP.ipv6_addr[12],pLayer4List->internalIP.ipv6_addr[13],pLayer4List->internalIP.ipv6_addr[14],pLayer4List->internalIP.ipv6_addr[15],
							pLayer4List->internalPort);
					}
#endif	
				_count++;
			}
			if(_count!=0)PROC_PRINTF("[%d] has %d Connections.\n",i,_count);
		}
		
	}

	PROC_PRINTF(">>Overall System Connections:%d\n",atomic_read(&rg_db.systemGlobal.v6StatefulConnectionNum));

	return len;
}
#endif

#ifdef CONFIG_APOLLO_ROMEDRIVER
int32 mibdump_alg_dynamic_port(struct seq_file *s, void *v)
{
	rtk_rg_alg_dynamicPort_t *pList;
	int len=0;

	PROC_PRINTF(">>ALG Dynamic Port:\n");	
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.algDynamicLock);
	if(!list_empty(&rg_db.algDynamicCheckListHead))
	{
		list_for_each_entry(pList,&rg_db.algDynamicCheckListHead,alg_list)
		{
			if(pList->serverInLan)
				{PROC_PRINTF("    Port[%d] %s ServerInLan(intIP:%x) timeout:%d\n",
					pList->portNum,pList->isTCP?"TCP":"UDP",pList->intIP,pList->timeout);}
			else
				{PROC_PRINTF("    Port[%d] %s ServerInWan timeout:%d\n",
					pList->portNum,pList->isTCP?"TCP":"UDP",pList->timeout);}
		}
	}
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.algDynamicLock);

	return len;
}

int32 mibdump_alg_srvInLan_ip(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	PROC_PRINTF(">>ALG Server In Lan Internal IP:\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress)
			PROC_PRINTF("  algType=0x%06x : IP=%s\n",rg_db.algServInLanIpMapping[i].algType,	inet_ntoa(rg_db.algServInLanIpMapping[i].serverAddress));
	}

	return len;
}
#endif

int32 dump_ipmc_group(struct seq_file *s, void *v)
{
	int i;
	int len=0;

#if defined(CONFIG_RTL9602C_SERIES)
	i=0;
	PROC_PRINTF("Not support in this chip!");
#elif defined(CONFIG_RTL9600_SERIES)


    PROC_PRINTF("GIP\tPMSK\n");

	for(i=0;i<MAX_IPMCGRP_HW_TABLE_SIZE;i++)
	{
		if(rg_db.ipmcgrp[i].valid==TRUE)
		{
			PROC_PRINTF("[%02d] %d.%d.%d.%d\t0x%x\n",i,(rg_db.ipmcgrp[i].groupIp>>24)&0xff
				,(rg_db.ipmcgrp[i].groupIp>>16)&0xff
				,(rg_db.ipmcgrp[i].groupIp>>8)&0xff
				,(rg_db.ipmcgrp[i].groupIp)&0xff
				,rg_db.ipmcgrp[i].portMsk.bits[0]);
		}		
	}
#endif	

    return len;
}
int32 dump_ipmc_routing(struct seq_file *s, void *v)
{
	int i;
	rtk_l34_ipmcTrans_entry_t ipmc;
	int len=0;

	PROC_PRINTF("Idx\tNetif\tSIP_T_EN\tEXTIP_IDX\tPPPOE_ACT\tPPPOE_IDX\n");

	for(i=0; i<16; i++)
	{
		rtk_l34_ipmcTransTable_get(i,&ipmc);
//		PROC_PRINTF("%02d\t%d\t%d\t\t%02d\t\t%d\t\t%d\n",i,ipmc.netifIdx,ipmc.sipTransEnable,ipmc.extipIdx,ipmc.isPppoeIf,ipmc.pppoeIdx);
		PROC_PRINTF("[%02d]\t%d\t%d\t\t%02d\t\t%d\t\t%d\n",i,ipmc.netifIdx,ipmc.sipTransEnable,ipmc.extipIdx,ipmc.pppoeAct,ipmc.pppoeIdx);
#if defined(CONFIG_RTL9602C_SERIES)
		PROC_PRINTF("\tUntagSetPortMask =%x \n",ipmc.untagMbr.bits[0]);
#endif
	}
    return len;
}
int32 dump_rg_vlan_table(struct seq_file *s, void *v)
{

	int i,j;
	int len=0;

	PROC_PRINTF(">>RG VLAN Table:\n");

	for(i=0; i<MAX_VLAN_HW_TABLE_SIZE; i++)
	{
		// clean up
		if(!rg_db.vlan[i].valid) continue;	// ignore unconfigured entry

		PROC_PRINTF(" -- VID[%d] --", i);
		PROC_PRINTF("\n\tMember Ports:0x%x",rg_db.vlan[i].MemberPortmask.bits[0]);
		PROC_PRINTF("\n\tExtension Member Ports:0x%x",rg_db.vlan[i].Ext_portmask.bits[0]);
		PROC_PRINTF("\n\tUntag Member Ports:0x%x",rg_db.vlan[i].UntagPortmask.bits[0]);
		PROC_PRINTF("\n\tFID: %d,\tIVL_SVL: %s\n", rg_db.vlan[i].fid, (rg_db.vlan[i].fidMode==VLAN_FID_IVL)?"IVL":"SVL");    //0:SVL, 1:IVL but VLAN_FID_IVL = 0, VLAN_FID_SVL = 1
		PROC_PRINTF("\tBased Priority: %s, %d\n", (rg_db.vlan[i].priorityEn==ENABLED)?"enable":"disable", rg_db.vlan[i].priority);
		PROC_PRINTF("\tExtension Ports: ");
		if(rg_db.vlan[i].Ext_portmask.bits[0]) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(RTK_RG_MAX_EXT_PORT); j++)
			{
				if(rg_db.vlan[i].Ext_portmask.bits[0] & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		PROC_PRINTF("\tWlan0 Devices: ");
		if(rg_db.vlan[i].wlan0DevMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0DevMask & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
#endif
	}

	//Display Port-based VLAN ID information
	PROC_PRINTF("\n>>RG Port-based VLAN settings:\n");
	PROC_PRINTF("PORT:\t   0    1    2    3 4PON 5MII 6CPU 7EX0 8EX1    9   10   11\nVID:\t");
		
	for(i=0;i<RTK_RG_MAX_MAC_PORT;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[i]);
	}
	
	for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_PORT_CPU+i]);
	}
	PROC_PRINTF("\n");	

	//Display Port-and-protocol-based VLAN ID information
	PROC_PRINTF("\n>>RG Port-and-Protocol-based VLAN settings:\n");

	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++) 	//port
	{
		int firstDump=TRUE;
		for(j=0; j<MAX_PORT_PROTO_GROUP_SIZE; j++)
		{
			if(rg_db.systemGlobal.protoBasedVID[i].protoVLANCfg[j].valid)
			{
				if(firstDump==TRUE) {PROC_PRINTF("  Port %d:\n",i); firstDump=FALSE; }
				switch(rg_db.systemGlobal.protoGroup[j].frametype)
				{
					case FRAME_TYPE_ETHERNET:
						PROC_PRINTF("    Group[%d] Ethertype:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_RFC1042:
						PROC_PRINTF("    Group[%d] frame_type:RFC1042 frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_SNAP8021H:
						PROC_PRINTF("    Group[%d] frame_type:SNAP8021H frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_SNAPOTHER:
						PROC_PRINTF("    Group[%d] frame_type:SNAPOTHER frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_LLCOTHER:
						PROC_PRINTF("    Group[%d] frame_type:LLCOTHER frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					default:
						break;
				}	
				//PROC_PRINTF("\tVID:%d PRI:%d CFI:%d\n",ppbCfg.vid,ppbCfg.pri,ppbCfg.dei);
				PROC_PRINTF(" VID[%d]\n",rg_db.systemGlobal.protoBasedVID[i].protoVLANCfg[j].vid);
			}
		}
	}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Display WLAN0-device-based VLAN ID information
	PROC_PRINTF("\n>>WLAN0-device-based VLAN settings:\n");

	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("DEV[%d] DVID=%d\n",i,rg_db.systemGlobal.wlan0DeviceBasedVID[i]);
		}
	}
#endif
	

    return len;
}

int32 dump_vlan_table(struct seq_file *s, void *v)
{

	int i,j,pvid;
	rtk_portmask_t mbr, untag, ext;
	rtk_fid_t fid;
	rtk_fidMode_t ivlsvl;
	rtk_pri_t pri;
	rtk_enable_t en;
	rtk_vlan_protoVlanCfg_t ppbCfg;
	rtk_vlan_protoGroup_t protoGroup;
	int len=0;

	PROC_PRINTF(">>ASIC VLAN Table:\n");

	for(i=0; i<MAX_VLAN_HW_TABLE_SIZE; i++)
	{
		// clean up
		mbr.bits[0] = 0;
		untag.bits[0] = 0;
		ext.bits[0] = 0;
		fid = 0;
		ivlsvl = 0;
		pri = 0;


		rtk_vlan_port_get(i, &mbr, &untag);
		if(mbr.bits[0]==0) continue;	// ignore unconfigured entry

		PROC_PRINTF(" -- VID[%d] --", i);
	#if 1
		PROC_PRINTF("\n\tMember Ports:0x%x",mbr.bits[0]);
		rtk_vlan_extPort_get(i, &mbr);
		PROC_PRINTF("\n\tExtension Member Ports:0x%x",mbr.bits[0]);
		PROC_PRINTF("\n\tUntag Member Ports:0x%x",untag.bits[0]);

	#else
		PROC_PRINTF("\n\tMember Ports: ");
		if(mbr.bits[0]==0) PROC_PRINTF("X");
		else
		{
			for(j=0; j<(MAX_APOLLO_PORT); j++)
			{
				if(mbr.bits[0] & (1<<j))  PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n\tUntag Member Ports: ");
		if(untag.bits[0]==0) PROC_PRINTF("X");
		else
		{
			for(j=0; j<(MAX_APOLLO_PORT); j++)
			{
				if(untag.bits[0] & (1<<j))	 PROC_PRINTF("%d ", j);
			}
		}
	#endif
	#ifdef CONFIG_RTL9602C_SERIES
		rtk_vlan_lutSvlanHashState_get(i, &en);
		PROC_PRINTF("\n\tSVLAN_CHK_IVL_SVL: %s", (en==ENABLED)?"Force SVLAN":"Not Force SVLAN");
	#endif
		rtk_vlan_fid_get(i, &fid);
		rtk_vlan_fidMode_get(i, &ivlsvl);
		PROC_PRINTF("\n\tFID: %d,\tIVL_SVL: %s\n", fid, (ivlsvl==VLAN_FID_IVL)?"IVL":"SVL");    //0:SVL, 1:IVL but VLAN_FID_IVL = 0, VLAN_FID_SVL = 1
		rtk_vlan_priority_get(i, &pri);
		rtk_vlan_priorityEnable_get(i, &en);
		PROC_PRINTF("\tBased Priority: %s, %d\n", (en==ENABLED)?"enable":"disable", pri);
		rtk_vlan_extPort_get(i, &ext);
		PROC_PRINTF("\tExtension Ports: ");
		if(ext.bits[0]==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(RTK_RG_MAX_EXT_PORT); j++)
			{
				if(ext.bits[0] & (1<<j))  { PROC_PRINTF("%d ", j);}
			}
		}
		PROC_PRINTF("\n");
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		PROC_PRINTF("\tWlan0 Devices: ");
		if(rg_db.vlan[i].wlan0DevMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0DevMask & (1<<j))  { PROC_PRINTF("%d ", j);}
			}
		}
		PROC_PRINTF("\n");
	#endif
	}

	//Display Port-based VLAN ID information
	PROC_PRINTF("\n>>ASIC Port-based VLAN settings:\n");
#if defined(CONFIG_RTL9602C_SERIES)
	PROC_PRINTF("PORT:\t   0    1 2PON 3CPU 4EX0 5EX1 6EX2 7EX3 8EX4 9EX5   10\nVID:\t");
#else
	PROC_PRINTF("PORT:\t   0    1    2    3 4PON 5MII 6CPU 7EX0 8EX1 9EX2   10   11\nVID:\t");
#endif
		
	for(i=0;i<RTK_RG_MAX_MAC_PORT;i++)
	{
		rtk_vlan_portPvid_get(i,&pvid);
		PROC_PRINTF("%4d ",pvid);
	}
	
	for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		rtk_vlan_extPortPvid_get(i,&pvid);
		PROC_PRINTF("%4d ",pvid);
	}
	PROC_PRINTF("\n");	

	//Display Port-based SVLAN ID information
#if defined(CONFIG_RTL9602C_SERIES)
	if(rg_kernel.stag_enable){
		PROC_PRINTF("\n>>ASIC Port-based SVALN settings:\n");
		PROC_PRINTF("PORT:\t   0    1 2PON 3CPU 4EX0 5EX1 6EX2 7EX3 8EX4 9EX5 10X3\nSVID:\t");
			
		for(i=0;i<RTK_RG_MAX_MAC_PORT;i++)
		{
			rtk_svlan_portSvid_get(i, &pvid);
			PROC_PRINTF("%4d ",pvid);
		}
		
		/*for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
		{
			rtk_vlan_extPortSvid_get(i, &pvid);
			PROC_PRINTF("%5d ",pvid);
		}*/
		PROC_PRINTF("\n");
	}
#endif
	
	//Display Port-and-protocol-based VLAN ID information
	PROC_PRINTF("\n>>ASIC Port-and-Protocol-based VLAN settings:\n");

	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++) 	//port
	{
		int firstDump=TRUE;
		for(j=0; j<MAX_PORT_PROTO_GROUP_SIZE; j++)
		{
			bzero(&protoGroup,sizeof(protoGroup));
			rtk_vlan_protoGroup_get(j, &protoGroup);
			bzero(&ppbCfg,sizeof(ppbCfg));
			rtk_vlan_portProtoVlan_get(i,j,&ppbCfg);
			if(ppbCfg.valid)
			{

				if(firstDump==TRUE) {PROC_PRINTF("  Port %d:\n",i); firstDump=FALSE; }
				switch(protoGroup.frametype)
				{
					case FRAME_TYPE_ETHERNET:
						PROC_PRINTF("    Group[%d] Ethertype:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_RFC1042:
						PROC_PRINTF("    Group[%d] frame_type:RFC1042 frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_SNAP8021H:
						PROC_PRINTF("    Group[%d] frame_type:SNAP8021H frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_SNAPOTHER:
						PROC_PRINTF("    Group[%d] frame_type:SNAPOTHER frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_LLCOTHER:
						PROC_PRINTF("    Group[%d] frame_type:LLCOTHER frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					default:
						break;
				}	
				//PROC_PRINTF("\tVID:%d PRI:%d CFI:%d\n",ppbCfg.vid,ppbCfg.pri,ppbCfg.dei);
				PROC_PRINTF(" VID[%d]\n",ppbCfg.vid);
			}
		}
	}
	
	ASSERT_EQ(rtk_vlan_vlanFunctionEnable_get(&en), RT_ERR_OK);
	PROC_PRINTF("\nCVLAN Filtering(%s)\n", en==ENABLED?"Enabled":"Disabled");
#if defined(CONFIG_RTL9602C_SERIES)
	if(rg_kernel.stag_enable){
		ASSERT_EQ(rtk_svlan_svlanFunctionEnable_get(&en), RT_ERR_OK);
		PROC_PRINTF("\nSVLAN Filtering(%s)\n", en==ENABLED?"Enabled":"Disabled");
	}
#endif
	
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Display WLAN0-device-based VLAN ID information
	PROC_PRINTF("\n>>WLAN0-device-based VLAN settings:\n");

	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("DEV[%d] DVID=%d\n",i,rg_db.systemGlobal.wlan0DeviceBasedVID[i]);
		}
	}
#endif

    return len;
}
int32 dump_vlan_mbr(struct seq_file *s, void *v)
{
	int i;
	uint32 val1, val2;
	int len=0;
	
	PROC_PRINTF(">>ASIC VLAN Member Configurations:\n\n");
	i=0;val1=0;val2=0;


#ifdef CONFIG_APOLLO_RLE0371
	for(i=0; i<32; i++)
	{
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, MBRf, &val2);
		if(val2==0) continue;

		PROC_PRINTF(" -- IDX[%d] --\n", i);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, EVIDf, &val1);
		PROC_PRINTF("\tCVID: %d\n", val1);
		PROC_PRINTF("\tMember Ports Mask: 0x%x\n", val2);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, EXT_MBRf, &val1);
		PROC_PRINTF("\tExtension Ports Mask: 0x%x\n", val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, VBPRIf, &val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, VBPENf, &val2);
		PROC_PRINTF("\tBased Priority: %s, %d\n",
					   val2?"enable":"disable", val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, FID_MSTIf, &val1);
		PROC_PRINTF("\tFID: %d\n", val1);
	}
#else
	switch(rg_kernel.apolloChipId)
	{


	
#if defined(CONFIG_RTL9600_SERIES)

		case APOLLOMP_CHIP_ID:
			for(i=0; i<32; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_MBRf, &val2);
				if(val2==0) continue;

				PROC_PRINTF(" -- IDX[%d] --\n", i);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_EVIDf, &val1);
				PROC_PRINTF("\tCVID: %d\n", val1);
				PROC_PRINTF("\tMember Ports Mask: 0x%x\n", val2);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_EXT_MBRf, &val1);
				PROC_PRINTF("\tExtension Ports Mask: 0x%x\n", val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VBPRIf, &val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VBPENf, &val2);
				PROC_PRINTF("\tBased Priority: %s, %d\n",
							   val2?"enable":"disable", val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_FID_MSTIf, &val1);
				PROC_PRINTF("\tFID: %d\n", val1);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif
    return len;
}
int32 dump_svlan_mbr(struct seq_file *s, void *v)
{
    int i,j;
    rtk_svlan_memberCfg_t ent= {0};
    int res = 0, count = 0;
	int len=0;
    PROC_PRINTF(">>ASIC SVLAN Member Configurations:\n\n");

    for(i=0; i<MAX_VLAN_HW_TABLE_SIZE; i++)
    {
        ent.svid = i;
        res = rtk_svlan_memberPortEntry_get(&ent);
        if((res!=RT_ERR_OK) || (ent.memberport.bits[0] == 0)) continue;
        else
        {
            PROC_PRINTF(" -- COUNT[%d] --\n", ++count);	// can't get real index here with rtk API
            PROC_PRINTF("\tSVID: %d", ent.svid);
            PROC_PRINTF("\tS-Priority: %d\n", ent.priority);

            PROC_PRINTF("\tMember Ports: ");
            if(ent.memberport.bits[0]==0) {PROC_PRINTF("X");}
            else
            {
                for(j=0; j<(RTK_RG_MAX_MAC_PORT+RTK_RG_MAX_EXT_PORT); j++)
                {
                    if(ent.memberport.bits[0] & (1<<j)) { PROC_PRINTF("%d ", j);}
                }
            }
            PROC_PRINTF("\n\tUntag Member Ports: ");
            if(ent.untagport.bits[0]==0) {PROC_PRINTF("X");}
            else
            {
                for(j=0; j<(RTK_RG_MAX_MAC_PORT+RTK_RG_MAX_EXT_PORT); j++)
                {
                    if(ent.untagport.bits[0] & (1<<j))  {PROC_PRINTF("%d ", j);}
                }
            }
            PROC_PRINTF("\n\tForce FID: %s, %d\n", ent.fiden?"enabled":"disabled", ent.fid);
            PROC_PRINTF("\tEnhanced FID: %s, %d\n",ent.efiden?"enabled":"disabled", ent.efid);
        }
    }

    return len;
}
int32 dump_svlan_c2s(struct seq_file *s, void *v)
{   
	int len=0;
	PROC_PRINTF("fix me");
#if 0
	// need too much time in FPGA
    /*    rtk_vlan_t svid=0, cvid;
        rtk_port_t port, pmsk=0;
        int res=0, count=0;
        for(cvid=0; cvid<(RTL865XC_VLAN_NUMBER*2-1); cvid++)
        {
            pmsk = 0;
            svid = 0;
            for(port=0; port<MAX_APOLLO_PORT; port++)
            {
                res=rtk_svlan_c2s_get(cvid, port, &svid);
                if(res==RT_ERR_OK) pmsk|=1<<port;
            }
            if(pmsk!=0)
            {
                PROC_PRINTF(" -- COUNT[%d] --\n", count++);
                PROC_PRINTF("\tEVID: %d\n", cvid);
                PROC_PRINTF("\tC2SENPMSK: 0x%x\n", pmsk);
                PROC_PRINTF("\tSVID: %d\n", svid);
            }
        }*/
    int i;
    uint32 val;
    PROC_PRINTF(">>ASIC SVLAN CVID/PVID to SVID Tables:\n\n");

    for(i=0; i<MAX_C2S_HW_TABLE_SIZE; i++)
    {
        ioal_mem32_read(0x014000+i*0x4,&val);
        if(((val>>0x6)&0x7F)==0) continue;
        PROC_PRINTF(" -- IDX[%d] --\n", i);
        PROC_PRINTF("\tSVIDX: 0x%x\n", (val)&0x1F);
        PROC_PRINTF("\tEVID: 0x%x\n", (val>>0xD)&0x1FFF);
        PROC_PRINTF("\tC2SENPMSK: 0x%x\n", (val>>0x6)&0x7F);
    }
#endif	
    return len;
}
int32 dump_svlan_mc2s(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	int i;
	uint32 val;
	int len=0;
	PROC_PRINTF(">>ASIC SVLAN Multicast to SVLAN Tables:\n\n");
	i=0;val=0;

	//SVLAN_MC2S
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:		
			for(i=0; i<SVLANMC2STBL_SIZE; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VALIDf, &val);
				if(val==0) continue;

				PROC_PRINTF(" -- IDX[%d] --\n", i);
				//PROC_PRINTF("\tValid: yes\n");
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_FORMATf, &val);
				PROC_PRINTF("\tFormat: %s\n", (val!=0)?"DIP":"DMAC");
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_SVIDXf, &val);
				PROC_PRINTF("\tSVIDX: 0x%x\n", val);
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_DATAf, &val);
				PROC_PRINTF("\tData: 0x%x\n", val);
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_MASKf, &val);
				PROC_PRINTF("\tMask: 0x%x\n", val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			PROC_PRINTF("9602C SERIES, RTL9601B_CHIP_ID=%d\n",RTL9601B_CHIP_ID);

			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif
    return len;
}
int32 dump_svlan_sp2c(struct seq_file *s, void *v)
{
    rtk_vlan_t svid, cvid;
    rtk_port_t port;
    int res=0, count=0;
	int len=0;
    for(svid=0; svid<MAX_VLAN_HW_TABLE_SIZE; svid++)
    {
        cvid = 0;
        for(port=0; port<RTK_RG_MAX_MAC_PORT; port++)
        {
            res=rtk_svlan_sp2c_get(svid, port, &cvid);
            if(res==RT_ERR_OK)
            {
                PROC_PRINTF(" -- COUNT[%d] --\n", count++);
                PROC_PRINTF("\tSVID: %d\n", svid);
                PROC_PRINTF("\tDestination Port: %d\n", port);
                PROC_PRINTF("\tExtened VID: %d\n", cvid);
            }
        }
    }

    return len;
}


int32 dump_acl_template(struct seq_file *s, void *v)
{
	int len =0;
	_dump_acl_template(s);
	return len;
}



int32 dump_acl_vidRangeTable(struct seq_file *s, void *v)
{
    int i;
    rtk_acl_rangeCheck_vid_t vidRangeEntry;
	int len=0;

    PROC_PRINTF("------------ ACL VID RANGE TABLES -------------\n");
    for(i=0; i<8; i++)
    {
        memset(&vidRangeEntry,0,sizeof(vidRangeEntry));
        vidRangeEntry.index=i;
        rtk_acl_vidRange_get(&vidRangeEntry);
        PROC_PRINTF("\tVIDRANGE[%d] upper:%d lower:%d type:0x%x\n",i,vidRangeEntry.upperVid,vidRangeEntry.lowerVid,vidRangeEntry.type);
    }
    return len;
}

int32 dump_acl_ipRangeTable(struct seq_file *s, void *v)
{
	int len =0;
	_dump_acl_ipRangeTable(s);

    return len;
}


int32 dump_acl_portRangeTable(struct seq_file *s, void *v)
{

	int len =0;
	_dump_acl_portRangeTable(s);		
    return len;
}

int32 dump_acl_pktlenRangeTable(struct seq_file *s, void *v)
{
    int i;
	int len=0;

    rtk_acl_rangeCheck_pktLength_t pktlenRangeEntry;
    PROC_PRINTF("------------ ACL PKTLEN RANGE TABLES -------------\n");
    for(i=0; i<8; i++)
    {
        memset(&pktlenRangeEntry,0,sizeof(pktlenRangeEntry));
        pktlenRangeEntry.index=i;
        rtk_acl_packetLengthRange_get(&pktlenRangeEntry);
        PROC_PRINTF("\tPKTLENRANGE[%d] upper:%d lower:%d type:0x%x\n",i,pktlenRangeEntry.upper_bound,pktlenRangeEntry.lower_bound,pktlenRangeEntry.type);
    }
    return len;
}


int32 dump_rg_naptPriority(struct seq_file *s, void *v){
	int i,j;
	int len=0;
	rtk_rg_naptFilterAndQos_t *naptFilterAndQos;
	rtk_rg_sw_naptFilterAndQos_t *pValidRule;


	for(i=0;i<MAX_NAPT_FILER_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.napt_SW_table_entry[i].valid==RTK_RG_ENABLED){
			naptFilterAndQos = &rg_db.systemGlobal.napt_SW_table_entry[i].naptFilter;
			PROC_PRINTF("==============[%d]=============\n",i);
			PROC_PRINTF("direction: %s \n",naptFilterAndQos->direction?"Downstream":"Upstream");
			PROC_PRINTF("patterns: 0x%x \n",naptFilterAndQos->filter_fields);
			if(naptFilterAndQos->filter_fields&INGRESS_SIP)
				PROC_PRINTF("ingress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_src_ipv4_addr);
			if(naptFilterAndQos->filter_fields&EGRESS_SIP)
				PROC_PRINTF("egress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_src_ipv4_addr);
			if(naptFilterAndQos->filter_fields&INGRESS_DIP)
				PROC_PRINTF("ingress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_dest_ipv4_addr);
			if(naptFilterAndQos->filter_fields&EGRESS_DIP)
				PROC_PRINTF("egress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_dest_ipv4_addr);
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT)
				PROC_PRINTF("ingress_src_l4_port: %d \n",naptFilterAndQos->ingress_src_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT)
				PROC_PRINTF("egress_src_l4_port: %d \n",naptFilterAndQos->egress_src_l4_port);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT)
				PROC_PRINTF("ingress_dest_l4_port: %d \n",naptFilterAndQos->ingress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT)
				PROC_PRINTF("egress_dest_l4_port: %d \n",naptFilterAndQos->egress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&INGRESS_SIP_RANGE)
				PROC_PRINTF("ingress_src_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->ingress_src_ipv4_addr_range_start,naptFilterAndQos->ingress_src_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_DIP_RANGE)
				PROC_PRINTF("ingress_dest_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->ingress_dest_ipv4_addr_range_start,naptFilterAndQos->ingress_dest_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT_RANGE)
				PROC_PRINTF("ingress_src_l4_port_range: %d ~ %d \n",naptFilterAndQos->ingress_src_l4_port_range_start,naptFilterAndQos->ingress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT_RANGE)
				PROC_PRINTF("ingress_dest_l4_port_range: %d ~ %d \n",naptFilterAndQos->ingress_dest_l4_port_range_start,naptFilterAndQos->ingress_dest_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_SIP_RANGE)
				PROC_PRINTF("egress_src_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->egress_src_ipv4_addr_range_start,naptFilterAndQos->egress_src_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_DIP_RANGE)
				PROC_PRINTF("egress_dest_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->egress_dest_ipv4_addr_range_start,naptFilterAndQos->egress_dest_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT_RANGE)
				PROC_PRINTF("egress_src_l4_port_range: %d ~ %d \n",naptFilterAndQos->egress_src_l4_port_range_start,naptFilterAndQos->egress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT_RANGE)
				PROC_PRINTF("egress_dest_l4_port_range: %d ~ %d \n",naptFilterAndQos->egress_dest_l4_port_range_start,naptFilterAndQos->egress_dest_l4_port_range_end);
			
			if(naptFilterAndQos->filter_fields&L4_PROTOCAL)
				PROC_PRINTF("ingress_l4_protocal: %d \n",naptFilterAndQos->ingress_l4_protocal);

			
			PROC_PRINTF("action_bit: 0x%x\n",naptFilterAndQos->action_fields);
			PROC_PRINTF("drop: %s\n",(naptFilterAndQos->action_fields&NAPT_DROP_BIT)?"Enable":"Disable");
			PROC_PRINTF("permit: %s\n",(naptFilterAndQos->action_fields&NAPT_PERMIT_BIT)?"Enable":"Disable");
			PROC_PRINTF("assign_priority: %d \n",naptFilterAndQos->assign_priority);	
			PROC_PRINTF("ruleType: %s \n",naptFilterAndQos->ruleType?"One Shot":"Persist");

		}
	}


	PROC_PRINTF("==============[US sorting by weight]=============\n");
	pValidRule = NULL;
	if(rg_db.systemGlobal.pValidUsNaptPriorityRuleStart!=NULL)
		pValidRule = rg_db.systemGlobal.pValidUsNaptPriorityRuleStart;
	else
		PROC_PRINTF("pValidUsNaptPriorityRuleStart is NULL \n");
	
	if(pValidRule!=NULL){
		j=0;//avoid cycle
		while(pValidRule!=NULL && j<MAX_NAPT_FILER_SW_ENTRY_SIZE){
			PROC_PRINTF("rule[%d]>",pValidRule->sw_index);
			pValidRule=pValidRule->pNextValid;
			j++;
		}
	}
	PROC_PRINTF("\n");

	PROC_PRINTF("==============[DS sorting by weight]=============\n");
	pValidRule = NULL;
	if(rg_db.systemGlobal.pValidDsNaptPriorityRuleStart!=NULL)
		pValidRule = rg_db.systemGlobal.pValidDsNaptPriorityRuleStart;
	else
		PROC_PRINTF("pValidDsNaptPriorityRuleStart is NULL \n");

	if(pValidRule!=NULL){
		j=0;//avoid cycle
		while(pValidRule!=NULL && j<MAX_NAPT_FILER_SW_ENTRY_SIZE){
			PROC_PRINTF("rule[%d]>",pValidRule->sw_index);
			pValidRule=pValidRule->pNextValid;
			j++;
		}
	}
	PROC_PRINTF("\n");

	
	return len;
}

char *name_of_ds_bc_tagDecision[]={
	"FORCE_UNATG",
	"FORCE_TAGGIN_WITH_CVID",
	"FORCE_TAGGIN_WITH_CVID_CPRI",
};

int dump_gpon_ds_bc_filterAndRemarkingRules(struct seq_file *s, void *v){
	int i;
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule;
	int len=0;

	for(i=0;i<MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].valid==RTK_RG_ENABLED){
			filterRule = &rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule;
			
			PROC_PRINTF("=======================[%d]=================================\n",i);
			PROC_PRINTF("filter_fields = 0x%x\n",filterRule->filter_fields);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT) PROC_PRINTF("ingress_stream_id= %d\n",filterRule->ingress_stream_id);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT) PROC_PRINTF("%s ",filterRule->ingress_stagIf?"WITH_STAG":"WITHOUT_STAG");
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_SVID_BIT) PROC_PRINTF("svid=%d\n",filterRule->ingress_stag_svid);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT) PROC_PRINTF("%s ",filterRule->ingress_ctagIf?"WITH_CTAG":"WITHOUT_CTAG");
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CVID_BIT) PROC_PRINTF("cvid=%d\n",filterRule->ingress_ctag_cvid);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_EGRESS_PORT_BIT) PROC_PRINTF("egres_port_mask=0x%x\n",filterRule->egress_portmask.portmask);
			PROC_PRINTF("action: %s cvid=%d cpri=%d\n",name_of_ds_bc_tagDecision[filterRule->ctag_action.ctag_decision],filterRule->ctag_action.assigned_ctag_cvid,filterRule->ctag_action.assigned_ctag_cpri);
		}
	}

	return len;
}


int32 dump_rg_acl(struct seq_file *s, void *v){
	int len=0;
	_dump_rg_acl(s);
	return len;
}


int32 dump_gatewayServicePort(struct seq_file *s, void *v){
	int i;
	int len=0;
	for(i=0;i<MAX_GATEWAYSERVICEPORT_TABLE_SIZE;i++){
		if(rg_db.gatewayServicePortEntry[i].valid==ENABLED){
			PROC_PRINTF("gatewayServicePortEntry[%d] port_num=%d type=%s\n",i,rg_db.gatewayServicePortEntry[i].port_num,rg_db.gatewayServicePortEntry[i].type?"CLIENT(sport)":"SERVER(dport)");
		}
	}
	return len;
}

int32 dump_acl(struct seq_file *s, void *v)
{
	int len = 0;
	_dump_acl(s);
    return len;
}

int32 dump_cf(struct seq_file *s, void *v)
{
	int len = 0;
	_dump_cf(s);
    return len;
}


int32 dump_rg_cf(struct seq_file *s, void *v){
	int len=0;

	_dump_rg_cf(s);
	
	return len;
}


int32 dump_rg_acl_reserved_info(struct seq_file *s, void *v){
	int len=0;

	//record current rg_kernel.debug_level
	rtk_rg_debug_level_t debug_level;
	debug_level = rg_kernel.debug_level;
	rg_kernel.debug_level = RTK_RG_DEBUG_LEVEL_ACL_RRESERVED;

	//show reserved ACL info
	_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_TAIL_END, NULL);


	//roll-back rg_kernel.debug_level
	rg_kernel.debug_level = debug_level;
	
	return len;
}

int32 dump_rg_urlFilter(struct seq_file *s, void *v){
	int i, index;
	char *keyword; //urlfilter url_filter_string
	char *path;//urlfilter path_filter_string
	int len=0;
	PROC_PRINTF("%s Mode, Total number:%d\n",rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK?"Black-list":"White_list",rg_db.systemGlobal.urlFilter_totalNum);
	for(i=0;i<MAX_URL_FILTER_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.urlFilter_valid_entry[i]==-1)//no rest valid urlFilter rule
			break;
		
		index = rg_db.systemGlobal.urlFilter_valid_entry[i]; //get the valid urlFilter entry
		keyword= rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.url_filter_string;
		path = rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_filter_string;
		PROC_PRINTF("========urlFilter[%d]=========\n",index);
		PROC_PRINTF("keyword: %s\n",keyword);
		PROC_PRINTF("path: %s\n",path);
		PROC_PRINTF("path_exactly_match: %d\n",rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_exactly_match);
	}
	return len;
}

int32 mibdump_port_isolation(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	for(i=RTK_RG_PORT0;i<RTK_RG_PORT_MAX;i++)
	{
		if(i<=RTK_RG_PORT_CPU)
			{PROC_PRINTF("Port[%d] isolation portmask %x\n",i,rg_db.systemGlobal.portIsolation[i].portmask);}
		else
			{PROC_PRINTF("extPort[%d] isolation portmask %x\n",i-RTK_RG_EXT_PORT0,rg_db.systemGlobal.portIsolation[i].portmask);}

	}
	return len;
}

int32 mibdump_redirect_httpAll(struct seq_file *s, void *v)
{
	int len=0;

	if(rg_db.redirectHttpAll.enable==0)
		PROC_PRINTF("Disable redirect.\n");
	else
		PROC_PRINTF("Redirect Http Info:enable=%d\n%s\n",rg_db.redirectHttpAll.enable,rg_db.redirectHttpAll.pushweb);

	return len;
}

int32 mibdump_redirect_httpURL(struct seq_file *s, void *v)
{
	int len=0,count;
	rtk_rg_redirectHttpURL_linkList_t *pRedEntry;

	if(list_empty(&rg_db.redirectHttpURLListHead)){
		PROC_PRINTF("No redirect URL setting.\n");
	}else{
		PROC_PRINTF("Redirect URL setting:\n");
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpURLListHead,url_list){
			count=atomic_read(&pRedEntry->count);
			PROC_PRINTF("    URL:%s DST_URL:%s ",pRedEntry->url_data.url_str,pRedEntry->url_data.dst_url_str);
			if(count==0)
				PROC_PRINTF("disable redirect.\n");
			else if(count<0)
				PROC_PRINTF("Non-stop redirect.\n");
			else
				PROC_PRINTF("Count:%d\n",count);
		}
	}
		
	return len;
}

int32 mibdump_redirect_httpWhiteList(struct seq_file *s, void *v)
{
	int len=0;
	rtk_rg_redirectHttpWhiteList_linkList_t *pRedEntry;
	
	if(list_empty(&rg_db.redirectHttpWhiteListListHead)){
		PROC_PRINTF("No redirect WhiteList setting.\n");
	}else{
		PROC_PRINTF("Redirect WhiteList setting:\n");
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpWhiteListListHead,white_list){
			PROC_PRINTF("    URL:%s Keyword:%s\n",pRedEntry->white_data.url_str,pRedEntry->white_data.keyword_str);
		}
	}
	return len;
}

#if 0 // for code size issue, this function is disabled.


int32 dump_reg(struct seq_file *s, void *v)
{
	int len=0;

#ifdef CONFIG_APOLLO_MODEL
	PROC_PRINTF("X86 Model do not support register dump!!\n");
#else
	//reg_array_field_read(APOLLOMP_XXXr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_XXXf, &val);
	int32 retv;
	uint32 val,val2;
	//rtk_portmask_t pmsk;
	//rtk_vlan_protoVlanCfg_t ppbCfg;
	int i,j;
	
	retv=0;val=0;val2=0;i=0;j=0;
#ifdef CONFIG_APOLLO_RLE0371
#else
	//rtk_vlan_protoGroup_t protoGroup;
#endif

#ifdef CONFIG_APOLLO_RLE0371
		//LUT regs
	PROC_PRINTF("\n-----LUT-----\n");
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_UNKN_UC_DA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_UC_DA_BEHAVE[%d]:0x%x\n",i,val);
	}
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_UNKN_UC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_UNKN_UC_FLOOD[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_BC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_BC_FLOOD[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_UNKN_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_SA_BEHAVE[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_UNMATCHED_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNMATCHED_SA_BEHAVE[%d]:0x%x\n",i,val);
	}
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(L2_SRC_PORT_PERMITr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_L2_SRC_PORT_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);
	}
	for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
	{
		reg_array_field_read(L2_SRC_EXT_PERMITr, REG_ARRAY_INDEX_NONE,i, ENf, &val);
		PROC_PRINTF("reg_L2_SRC_EXT_PERMIT[%d]:0x%x\n",i,(val>>(i-1))&0x1);
	}
		
	//Port Security regs
	PROC_PRINTF("\n-----Port Security-----\n");
		
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_LRN_LIMITNOr, i, REG_ARRAY_INDEX_NONE, NUMf, &val);
		PROC_PRINTF("reg_LUT_LRN_LIMITNO[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_LEARN_OVER_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_LUT_LEARN_OVER_ACT[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_DIS_AGEr, i, REG_ARRAY_INDEX_NONE, DIS_AGEf, &val);
		PROC_PRINTF("reg_LUT_DISABLE_AGE[%d]: %d\n",i,val);
	}
		
		
	reg_field_read(LUT_CFGr,  LUT_IPMC_HASHf, &val);
	PROC_PRINTF("reg_LUT_IPMC_HASH:0x%x\n",val);
		
	reg_field_read(LUT_CFGr,  LUT_IPMC_LOOKUP_OPf, &val);
		
	PROC_PRINTF("reg_LUT_IPMC_LOOKUP_OP:0x%x\n",val);

	//Multicast
	PROC_PRINTF("\n-----Multicast-----\n");

	reg_field_read(L34_GLB_CFGr,  L34_GLOBAL_CFGf, &val);
	PROC_PRINTF("reg_L34_Global_Enable: 0x%x\n",val);
		
	//Unknown Multicast Control
	PROC_PRINTF("\n-----Unknown Multicast Control-----\n");
		
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(UNKN_IP4_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_IP4_MC_ACT[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(UNKN_IP6_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_IP6_MC_ACT[%d]:0x%x\n",i,val);
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(UNKN_L2_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_L2_MC_ACT[%d]:0x%x\n",i,val);
	}
		
		
	PROC_PRINTF("reg_UNKNOWN_MCAST_TRAP_PRIORITY: not found\n");
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(LUT_UNKN_MC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_UNKN_MC_FLOOD[%d]:0x%x\n",i,val);
	}
		
		
	// QOS-Ingress Priority regs
	PROC_PRINTF("\n----- QOS-Ingress Priority-----\n");
		
	for(i=0; i<8; i++)
	{
		reg_array_field_read(QOS_1Q_PRI_REMAPr,REG_ARRAY_INDEX_NONE,i, INTPRI_1Qf, &val);
		PROC_PRINTF("reg_DOT1Q_PRI_PRIORITY[%d]:0x%x\n",i,val);//QOS_1Q_PRI_REMAP
	}
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(QOS_PB_PRIr, i, REG_ARRAY_INDEX_NONE, INTPRI_PBf, &val);
		PROC_PRINTF("reg_QOS_PORT_PRIORITY[%d]:0x%x\n",i,val);//QOS_PB_PRI
	}
		
	reg_field_read(PRI_SEL_TBL_CTRLr,	PORT_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_PORT_WEIGHT:0x%x\n",val); //PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	DOT1Q_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_1Q_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	DSCP_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_DSCP_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	ACL_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_ACL_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	CVLAN_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_CVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	LUTFWD_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_LUTFWD_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	SA_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_SA_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	SVLAN_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_SVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRL2r,  L4_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_L4_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL2
		
		
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(QOS_PORT_QMAP_CTRLr,i,REG_ARRAY_INDEX_NONE, IDXf, &val);
		PROC_PRINTF("reg_PORT_QTABLE_INDEX[%d]:0x%x\n",i,val);//QOS_PRI_REMAP_IN_CPU
	}

	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			reg_array_field_read(QOS_INTPRI_TO_QIDr,i,j, PRI_TO_QIDf, &val);
			PROC_PRINTF("reg_QOS_PRIORITY_TO_QID_TABLE[%d][%d]:0x%x\n",i,j,val);//QOS_INTPRI_TO_QID
		}
	}
		
	// QOS-Remarking
	PROC_PRINTF("\n----- QOS-Remarking-----\n");

	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(RMK_DOT1Q_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_REMARKING_1Q_PORT_ENABLE[%d]:0x%x\n",i,val);//RMK_DOT1Q_RMK_EN_CTRL
	}
		
//ioal_mem32_read(0x0231CC,&val);
	for(i=0; i<8; i++)
	{
		reg_array_field_read(RMK_1Q_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_1Qf, &val);
		PROC_PRINTF("reg_INTPRI_1QPRI[%d]:0x%x\n",i,val); //RMK_1Q_CTRL
	}
		
	//Qos
for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
    {
            reg_array_field_read(RMK_DSCP_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
            PROC_PRINTF("reg_RMK_DSCP_RMK_EN_CTRL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
    {
            reg_array_field_read(RMK_P_DSCP_SELr,REG_ARRAY_INDEX_NONE,i, SELf, &val);
            PROC_PRINTF("reg_RMK_DSCP_CFG_SEL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<64; i++)
    {
            reg_array_field_read(QOS_DSCP_REMAPr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_QOS_DSCP_REMAP[%d]:%d\n",i,val);
    }

    for(i=0; i<8; i++)
    {
            reg_array_field_read(RMK_DSCP_INT_PRI_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_RMK_DSCP_INT_PRI_CTRL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<64; i++)
    {
            reg_array_field_read(RMK_DSCP_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_RMK_DSCP_CTRL[%d]:0x%x\n",i,val);
    }
#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//LUT regs
			PROC_PRINTF("\n-----LUT-----\n");
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_UC_DA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_UC_DA_BEHAVE[%d]:0x%x\n",i,val);
			}
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_UC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_UNKN_UC_FLOOD[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_BC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_BC_FLOOD[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_SA_BEHAVE[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNMATCHED_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNMATCHED_SA_BEHAVE[%d]:0x%x\n",i,val);
			}
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_L2_SRC_PORT_PERMITr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_L2_SRC_PORT_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);
			}
			for(i=0; i<RTK_RG_MAX_EXT_PORT-1; i++)
			{
				reg_array_field_read(APOLLOMP_L2_SRC_EXT_PERMITr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_L2_SRC_EXT_PERMIT[%d]:0x%x\n",i,(val>>(i-1))&0x1);
			}
		
			//Port Security regs
			PROC_PRINTF("\n-----Port Security-----\n");
		
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_LRN_LIMITNOr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_NUMf, &val);
				PROC_PRINTF("reg_LUT_LRN_LIMITNO[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_LEARN_OVER_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_LUT_LEARN_OVER_ACT[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_AGEOUT_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_AGEOUT_OUTf, &val);
				PROC_PRINTF("reg_LUT_DISABLE_AGE[%d]: %d\n",i,val);
			}
		
		
			reg_field_read(APOLLOMP_LUT_CFGr,  APOLLOMP_LUT_IPMC_HASHf, &val);
			PROC_PRINTF("reg_LUT_IPMC_HASH:0x%x\n",val);
		
			reg_field_read(APOLLOMP_LUT_CFGr,  APOLLOMP_LUT_IPMC_LOOKUP_OPf, &val);
		
			PROC_PRINTF("reg_LUT_IPMC_LOOKUP_OP:0x%x\n",val);
		
			//Multicast
			PROC_PRINTF("\n-----Multicast-----\n");
		
			reg_field_read(APOLLOMP_L34_GLB_CFGr,  APOLLOMP_L34_GLOBAL_CFGf, &val);
			PROC_PRINTF("reg_L34_Global_Enable: 0x%x\n",val);
		
			reg_field_read(APOLLOMP_L34_IPMC_TTL_CFGr,	APOLLOMP_IP_MCST_TTL_1f, &val);
			PROC_PRINTF("reg_IP_MULTICAST_ROUTE_TTL_1: %d\n",val);
		
			//Unknown Multicast Control
			PROC_PRINTF("\n-----Unknown Multicast Control-----\n");
		
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_IP4_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_IP4_MC_ACT[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_IP6_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_IP6_MC_ACT[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_L2_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_L2_MC_ACT[%d]:0x%x\n",i,val);
			}
		
		
			PROC_PRINTF("reg_UNKNOWN_MCAST_TRAP_PRIORITY: not found\n");
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_MC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_UNKN_MC_FLOOD[%d]:0x%x\n",i,val);
			}
		
		
			// QOS-Ingress Priority regs
			PROC_PRINTF("\n----- QOS-Ingress Priority-----\n");
		
			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_1Q_PRI_REMAPr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_1Qf, &val);
				PROC_PRINTF("reg_DOT1Q_PRI_PRIORITY[%d]:0x%x\n",i,val);//QOS_1Q_PRI_REMAP
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_PB_PRIr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_INTPRI_PBf, &val);
				PROC_PRINTF("reg_QOS_PORT_PRIORITY[%d]:0x%x\n",i,val);//QOS_PB_PRI
			}
		
			//QOS_DSCP_REMAP
			for(i=0; i<64; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_DSCP_REMAPr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_QOS_DSCP_REMAP[%d]:%d\n",i,val);
			}
		
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_PORT_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_PORT_WEIGHT:0x%x\n",val); //PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_DOT1Q_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_1Q_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_DSCP_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_DSCP_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_ACL_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_ACL_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_CVLAN_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_CVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_LUTFWD_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_LUTFWD_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_SA_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_SA_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_SVLAN_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_SVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRL2r,  APOLLOMP_L4_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_L4_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL2
		
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_PORT_QMAP_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_IDXf, &val);
				PROC_PRINTF("reg_PORT_QTABLE_INDEX[%d]:0x%x\n",i,val);//QOS_PRI_REMAP_IN_CPU
			}
		
			for(i=0; i<4; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_QOS_INTPRI_TO_QIDr,i,j, APOLLOMP_PRI_TO_QIDf, &val);
					PROC_PRINTF("reg_QOS_PRIORITY_TO_QID_TABLE[%d][%d]:0x%x\n",i,j,val);//QOS_INTPRI_TO_QID
				}
			}
		
			// QOS-Remarking
			PROC_PRINTF("\n----- QOS-Remarking-----\n");
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DOT1Q_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_REMARKING_1Q_PORT_ENABLE[%d]:0x%x\n",i,val);//RMK_DOT1Q_RMK_EN_CTRL
			}
		
			ioal_mem32_read(0x0231CC,&val);
			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_1Q_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_1Qf, &val);
				PROC_PRINTF("reg_INTPRI_1QPRI[%d]:0x%x\n",i,val); //RMK_1Q_CTRL
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_RMK_DSCP_RMK_EN_CTRL[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_P_DSCP_SELr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_SELf, &val);
				PROC_PRINTF("reg_RMK_DSCP_CFG_SEL[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_INT_PRI_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_RMK_DSCP_INT_PRI_CTRL[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<64; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_RMK_DSCP_CTRL[%d]:0x%x\n",i,val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif
	
	//ACL regs
#ifdef CONFIG_APOLLO_RLE0371
	PROC_PRINTF("\n----- ACL-----\n");
	retv = reg_field_read(ACL_CFGr, MODEf, &val);
	assert(retv == RT_ERR_OK);
	PROC_PRINTF("reg_ACL_MODE:0x%x\n", val); /*0: 8 field template mode	 1: 4 field template + 3 field template */

	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(ACL_ENr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,val);	/*per port enable/disable ACL*/
	}
	for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
	{
		reg_array_field_read(ACL_PERMITr,i,REG_ARRAY_INDEX_NONE, PERMITf, &val);
		PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,val);	/*per port permit/drop frame while ACL rule unhit*/
	}
	
	retv = reg_field_read(CF_CFGr, CF_US_PERMITf, &val);
	assert(retv == RT_ERR_OK);
	PROC_PRINTF("reg_CF_US_PERMIT:0x%x\n", val); /*per port permit/drop frame while CF rule unhit*/

#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			PROC_PRINTF("\n----- ACL-----\n");
			//ioal_mem32_read(0x01530C,&val);
			//PROC_PRINTF("reg_ACL_MODE:0x%x\n",val&0x1); /*0: 8 field template mode   1: 4 field template + 3 field template */
			retv = reg_field_read(APOLLOMP_ACL_CFGr, APOLLOMP_MODEf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ACL_MODE:0x%x\n", val); /*0: 8 field template mode	 1: 4 field template + 3 field template */
		
		
			//ioal_mem32_read(0x015104,&val);
			//for(i=0; i<MAX_APOLLO_PORT; i++)
			//{
			//	  PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,(val>>i)&0x1);/*per port enable/disable ACL*/
			//}
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_ACL_ENr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,val);	/*per port enable/disable ACL*/
			}
		
			//ioal_mem32_read(0x015108,&val);
			//for(i=0; i<MAX_APOLLO_PORT; i++)
			//{
			//	  PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);/*per port permit/drop frame while ACL rule unhit*/
			//}
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_ACL_PERMITr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PERMITf, &val);
				PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,val);	/*per port permit/drop frame while ACL rule unhit*/
			}
		
		
			//check hit reason
			//ioal_mem32_read(0x01C0CC,&val);
			//PROC_PRINTF("STAT_ACL_REASON(0x01C0CC):0x%x\n",val);
			for(i=0; i<6; i++)
			{
				reg_array_field_read(APOLLOMP_STAT_ACL_REASONr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_ACL_HIT_INFOf, &val);
				PROC_PRINTF("STAT_ACL_REASON[%d]:0x%x\n",i,val);
			}
		
			//ioal_mem32_read(0x01C0D0,&val_1);
			//PROC_PRINTF("STAT_ACL_REASON(0x01C0D0):0x%x\n",val_1);
		
		//Classification regs
			PROC_PRINTF("\n----- Classification-----\n");
			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_SEL_PON_EN:0x%x\n",(val&0x4)>>2);	/*enable/disable PON port as CF port*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_SEL_PON_ENf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_SEL_PON_EN:0x%x\n", val);	/*enable/disable PON port as CF port*/
		
			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_SEL_RGMII_EN:0x%x\n",(val&0x8)>>3);/*enable/disable RGMII port as CF port*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_SEL_RGMII_ENf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_SEL_RGMII_EN:0x%x\n", val);	/*enable/disable RGMII port as CF port*/
		
		
			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_PERMIT:0x%x\n",(val&0x3)); /*per port permit/drop frame while CF rule unhit*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_US_PERMITf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_US_PERMIT:0x%x\n", val); /*per port permit/drop frame while CF rule unhit*/
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif
	
	
	//vlan
	PROC_PRINTF("\n-----vlan-----\n");
#ifdef CONFIG_APOLLO_RLE0371
	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PB_EFIDENr, i, REG_ARRAY_INDEX_NONE, PBFIDENf, &val);
		reg_array_field_read(VLAN_PB_EFIDr, i, REG_ARRAY_INDEX_NONE, PBFIDf, &val2);
		//assert(rtk_vlan_portFid_get(i, &val, &val2) == RT_ERR_OK);
		PROC_PRINTF("reg_PORTn_PBFIDEN[port%d]:0x%x,%s\n",i,val2,
			val==0?"DISABLED":"ENABLED");
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_EGRESS_TAGr, i, REG_ARRAY_INDEX_NONE, EGRESS_MODEf, &val);
		//assert(rtk_vlan_tagMode_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_EGRESS_MODE[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++)
	{
		//bzero(pmsk.bits,sizeof(pmsk.bits));
		reg_array_field_read(VLAN_EGRESS_KEEPr, i, REG_ARRAY_INDEX_NONE, MBRf, &val);
		//assert(rtk_vlan_portEgrTagKeepType_get(i, &pmsk, &val2)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_EGRESS_PORTn_VLAN_KEEP[port%d]:0x%x\n",i,val);
	}

	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VLAN_FILTERINGf, &val);
	//assert(rtk_vlan_vlanFunctionEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_FILTERING:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, TRANSPARENT_ENf, &val);
	//assert(rtk_vlan_transparentEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_TRANSPARENT_EN:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, CFI_KEEPf, &val);
	//assert(rtk_vlan_cfiKeepEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_CFI_KEEP:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VID_0_TYPEf, &val);
	//assert(rtk_vlan_reservedVidAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_VID0_TYPE:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VID_4095_TYPEf, &val2);
	PROC_PRINTF("reg_VLAN_VID4095_TYPE:0x%x\n",val2);

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PORT_ACCEPT_FRAME_TYPEr, i,REG_ARRAY_INDEX_NONE, FRAME_TYPEf, &val);
		//assert(rtk_vlan_portAcceptFrameType_get(i, &val)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORT_ACCEPT_FRAME_TYPE[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_INGRESSr, i,REG_ARRAY_INDEX_NONE, INGRESSf, &val);
		//assert(rtk_vlan_portIgrFilterEnable_get(i, &val)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_INGRESS[port%d]:0x%x\n",i,val);
	}

// port-based vlan
	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PB_VIDXr, i,REG_ARRAY_INDEX_NONE, VIDXf, &val);
		//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_VIDX[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<5; i++)
	{
		reg_array_field_read(VLAN_EXT_VIDXr, i,REG_ARRAY_INDEX_NONE, VIDXf, &val);
		//assert(rtk_vlan_extPortPvid_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort%d]:0x%x\n",i,val);
	}

// port-and-protocol-based vlan

	for(i=0; i<=3; i++)
	{
		
		reg_array_field_read(VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, FRAME_TYPEf, &val);
		//bzero(&protoGroup,sizeof(protoGroup));
		//assert(rtk_vlan_protoGroup_get(i, &protoGroup)== RT_ERR_OK);
		//PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,protoGroup.frametype);
		PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,val);
		reg_array_field_read(VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, ETHER_TYPEf, &val);
		//PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,protoGroup.framevalue);
		PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++) 	//port
	{
		PROC_PRINTF("Port %d:\n",i);
		for(j=0; j<=3; j++)
		{
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, VALIDf, &val);
			if(val==0)continue;	//invalid will pass
			//assert(rtk_vlan_portProtoVlan_get(i,j,&ppbCfg)== RT_ERR_OK);
			//PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,ppbCfg.valid);
			PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,val);
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, PPB_VIDXf, &val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,ppbCfg.vid);
			PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,val);
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, PPB_PRIf, &val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,ppbCfg.pri);
			PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,ppbCfg.dei);
		}
	}
		
#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PB_FIDENr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_PBFIDENf, &val);
				reg_array_field_read(APOLLOMP_VLAN_PB_FIDr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_PBFIDf, &val2);
				//assert(rtk_vlan_portFid_get(i, &val, &val2) == RT_ERR_OK);
				PROC_PRINTF("reg_PORTn_PBFIDEN[port%d]:0x%x,%s\n",i,val2,
					val==0?"DISABLED":"ENABLED");
			}
		
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_EGRESS_TAGr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_EGRESS_MODEf, &val);
				//assert(rtk_vlan_tagMode_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_EGRESS_MODE[port%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<=6; i++)
			{
				//bzero(pmsk.bits,sizeof(pmsk.bits));
				reg_array_field_read(APOLLOMP_VLAN_EGRESS_KEEPr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_MBRf, &val);
				//assert(rtk_vlan_portEgrTagKeepType_get(i, &pmsk, &val2)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_EGRESS_PORTn_VLAN_KEEP[port%d]:0x%x\n",i,val);
			}
		
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VLAN_FILTERINGf, &val);
			//assert(rtk_vlan_vlanFunctionEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_FILTERING:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_TRANSPARENT_ENf, &val);
			//assert(rtk_vlan_transparentEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_TRANSPARENT_EN:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_CFI_KEEPf, &val);
			//assert(rtk_vlan_cfiKeepEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_CFI_KEEP:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VID_0_TYPEf, &val);
			//assert(rtk_vlan_reservedVidAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_VID0_TYPE:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VID_4095_TYPEf, &val2);
			PROC_PRINTF("reg_VLAN_VID4095_TYPE:0x%x\n",val2);
		
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PORT_ACCEPT_FRAME_TYPEr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_FRAME_TYPEf, &val);
				//assert(rtk_vlan_portAcceptFrameType_get(i, &val)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORT_ACCEPT_FRAME_TYPE[port%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_INGRESSr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_INGRESSf, &val);
				//assert(rtk_vlan_portIgrFilterEnable_get(i, &val)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_INGRESS[port%d]:0x%x\n",i,val);
			}
		
		// port-based vlan
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PB_VIDXr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_VIDXf, &val);
				//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_VIDX[port%d]:0x%x\n",i,val);
			}

			//ext-port0 is cpu port, too
			reg_array_field_read(APOLLOMP_VLAN_PB_VIDXr, 6,REG_ARRAY_INDEX_NONE, APOLLOMP_VIDXf, &val);
			//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort0]:0x%x\n",val);
			for(i=0; i<5; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_EXT_VIDXr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_VIDXf, &val);
				//assert(rtk_vlan_extPortPvid_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort%d]:0x%x\n",i+1,val);
			}
		
		// port-and-protocol-based vlan
		
			for(i=0; i<=3; i++)
			{
				
				reg_array_field_read(APOLLOMP_VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_FRAME_TYPEf, &val);
				//bzero(&protoGroup,sizeof(protoGroup));
				//assert(rtk_vlan_protoGroup_get(i, &protoGroup)== RT_ERR_OK);
				//PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,protoGroup.frametype);
				PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,val);
				reg_array_field_read(APOLLOMP_VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_ETHER_TYPEf, &val);
				//PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,protoGroup.framevalue);
				PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,val);
			}
		
			for(i=0; i<=6; i++) 	//port
			{
				PROC_PRINTF("Port %d:\n",i);
				for(j=0; j<=3; j++)
				{
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_VALIDf, &val);
					//assert(rtk_vlan_portProtoVlan_get(i,j,&ppbCfg)== RT_ERR_OK);
					//PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,ppbCfg.valid);
					if(val==0)continue;
					PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,val);
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_PPB_VIDXf, &val);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,ppbCfg.vid);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,val);
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_PPB_PRIf, &val);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,ppbCfg.pri);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,ppbCfg.dei);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,val);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,0);
				}
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

	// svlan
	PROC_PRINTF("\n-----svlan-----\n");
#ifdef CONFIG_APOLLO_RLE0371
	reg_field_read(SVLAN_CFGr, VS_TPIDf, &val);
	//assert(rtk_svlan_tpidEntry_get(0,&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_TPID:0x%x\n",val);//SVLAN_CFG

	reg_field_read(SVLAN_CTRLr, VS_PRIf, &val);
	//assert(rtk_svlan_trapPri_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_TRAP_PRI:0x%x\n", val);//VS_TRAP_PRI

	PROC_PRINTF("reg_VS_PMSK: ");//SVLAN_UPLINK_PMSK
	for(i=0,j=0; i<=6; i++)
	{
		reg_array_field_read(SVLAN_UPLINK_PMSKr,REG_ARRAY_INDEX_NONE,i, ENf, &val);
		//assert(rtk_svlan_servicePort_get(i,&val)== RT_ERR_OK);
		if(val==1)
		{
			j=1;
			PROC_PRINTF("%d ",i);
		}
	}
	if(j==0) PROC_PRINTF("X\n");
	else PROC_PRINTF("\n");//SVLAN_UPLINK_PMSK

	reg_field_read(SVLAN_CTRLr, VS_SPRISELf, &val);
	//assert(rtk_svlan_priorityRef_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_SPRISEL:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNTAGf, &val);
	//assert(rtk_svlan_untagAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_UNTAG:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNTAG_SVIDXf, &val2);
	PROC_PRINTF("reg_VS_UNTAG_SVIDX:0x%x\n",val2);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNMATf, &val);
	//assert(rtk_svlan_unmatchAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_UNMAT:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNMAT_SVIDXf, &val2);
	PROC_PRINTF("reg_VS_UNMAT_SVIDX:0x%x\n",val2);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_CFI_KEEPf, &val);
	//assert(rtk_svlan_deiKeepState_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_CFI_KEEP:0x%x\n",val);//SVLAN_CTRL

	for(i=0; i<6; i++)	 //VS_PORTn_SVIDX[0]~[4]
	{
		reg_array_field_read(SVLAN_P_SVIDXr,REG_ARRAY_INDEX_NONE,i, SVIDXf, &val);
		//ASSERT(rtk_svlan_portSvid_get(i,&val)== RT_ERR_OK);
		//val2 = rtk_svlan_portSvid_get(i,&val);
		//if(val2==RT_ERR_SVLAN_INVALID)
			//PROC_PRINTF("Port-based SVALN is invalid\n");
		//else if(val2==RT_ERR_OK)
			PROC_PRINTF("reg_VS_PORTn_SVIDX[%d]:0x%x\n",i,val);//SVLAN_P_SVIDX
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(SVLAN_EP_DMAC_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		//assert(rtk_svlan_dmacVidSelState_get(i,&val)== RT_ERR_OK);
		PROC_PRINTF("reg_VS_PORTn_DMACVIDSE[%d]:0x%x\n", i, val);
	}
	
#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:			
			reg_field_read(APOLLOMP_SVLAN_CFGr, APOLLOMP_VS_TPIDf, &val);
			//assert(rtk_svlan_tpidEntry_get(0,&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_TPID:0x%x\n",val);//SVLAN_CFG
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_TYPEf, &val);
			//assert(rtk_svlan_lookupType_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_LOOK_UP_TYPE:0x%x\n", val);//SVLAN_LOOK_UP_TYPE
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_PRIf, &val);
			//assert(rtk_svlan_trapPri_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_TRAP_PRI:0x%x\n", val);//VS_TRAP_PRI
		
			PROC_PRINTF("reg_VS_PMSK: ");//SVLAN_UPLINK_PMSK
			for(i=0,j=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_UPLINK_PMSKr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				//assert(rtk_svlan_servicePort_get(i,&val)== RT_ERR_OK);
				if(val==1)
				{
					j=1;
					PROC_PRINTF("%d ",i);
				}
			}
			if(j==0) {PROC_PRINTF("X\n");}
			else {PROC_PRINTF("\n");}//SVLAN_UPLINK_PMSK
	
			reg_field_read(APOLLOMP_SVLAN_CTRLr,APOLLOMP_FORCED_DMACVIDSELf,&val);
			//assert(rtk_svlan_dmacVidSelForcedState_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_FORCED_DMACVIDSEL:0x%x\n", val);
		
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_SPRISELf, &val);
			//assert(rtk_svlan_priorityRef_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_SPRISEL:0x%x\n",val);//SVLAN_CTRL
		
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNTAGf, &val);
			//assert(rtk_svlan_untagAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_UNTAG:0x%x\n",val);//SVLAN_CTRL	
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNTAG_SVIDXf, &val2);
			PROC_PRINTF("reg_VS_UNTAG_SVIDX:0x%x\n",val2);//SVLAN_CTRL
		
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNMATf, &val);
			//assert(rtk_svlan_unmatchAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_UNMAT:0x%x\n",val);//SVLAN_CTRL	
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNMAT_SVIDXf, &val2);
			PROC_PRINTF("reg_VS_UNMAT_SVIDX:0x%x\n",val2);//SVLAN_CTRL
			
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_SP2C_UNMATf, &val);
			//assert(rtk_svlan_sp2cUnmatchCtagging_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_SP2C_UNMAT:0x%x\n",val);//SVLAN_CTRL
		
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_DEI_KEEPf, &val);
			//assert(rtk_svlan_deiKeepState_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_DEI_KEEP:0x%x\n",val);//SVLAN_CTRL
		
			for(i=0; i<6; i++)	 //VS_PORTn_SVIDX[0]~[4]
			{
				reg_array_field_read(APOLLOMP_SVLAN_P_SVIDXr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_SVIDXf, &val);
				//ASSERT(rtk_svlan_portSvid_get(i,&val)== RT_ERR_OK);
				//val2 = rtk_svlan_portSvid_get(i,&val);
				//if(val2==RT_ERR_SVLAN_INVALID)
					//PROC_PRINTF("Port-based SVALN is invalid\n");
				//else if(val2==RT_ERR_OK)
					PROC_PRINTF("reg_VS_PORTn_SVIDX[%d]:0x%x\n",i,val);//SVLAN_P_SVIDX
			}
		
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_EP_DMAC_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				//assert(rtk_svlan_dmacVidSelState_get(i,&val)== RT_ERR_OK);
				PROC_PRINTF("reg_VS_PORTn_DMACVIDSE[%d]:0x%x\n", i, val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}		
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else
	PROC_PRINTF("\n-----binding-----\n");
	
	//Binding
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			
			retv = reg_field_read(APOLLOMP_V6_BD_CTLr, APOLLOMP_PB_ENf, &val);
			//retv = rtk_l34_globalState_get(L34_GLOBAL_BIND_STATE,&val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_V6_BD_CTL:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L2L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L2L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L2L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L2L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L2L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L2L34:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L34:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L34L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L34L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L34L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L34L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L34L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L34L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L34:0x%x\n", val);
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else	
	// IPv6
	PROC_PRINTF("\n-----IPv6-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//retv = reg_field_read(APOLLOMP_V6_BD_CTLr, APOLLOMP_PB_ENf, &val);		//FIXME:no RTK api
			//assert(retv == RT_ERR_OK);
			//PROC_PRINTF("reg_IPV6_PB_EN:0x%x\n", val);		//binding control bit
		
			retv = reg_field_read(APOLLOMP_SWTCR0r, APOLLOMP_TTL_1ENABLEf, &val);	//FIXME:no RTK api
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_IPV6_TTL_1_EN:0x%x\n", val);		//TTL-1 control bit
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}	
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else	
	// Port Isolation
	PROC_PRINTF("\n-----Port Isolation-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			retv = reg_field_read(APOLLOMP_PISO_CTRLr, APOLLOMP_CTAG_SELf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ISOLATION_CTAG_SEL:0x%x\n", val);
		
			retv = reg_field_read(APOLLOMP_PISO_CTRLr, APOLLOMP_L34_SELf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ISOLATION_L34_SEL:0x%x\n", val);
		
			PROC_PRINTF("\n");
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_P_MODE0_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_PORT_MODE0[%d]:0x%x\n",i,val);
			}
		
			PROC_PRINTF("\n");
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_P_MODE1_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_PORT_MODE1[%d]:0x%x\n",i,val);
			}
		
			PROC_PRINTF("\n");
		
			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_EXT_MODE0_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_EXTPORT_MODE0[%d]:0x%x\n",i,val);
			}
		
			PROC_PRINTF("\n");
		
			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_EXT_MODE1_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_EXTPORT_MODE1[%d]:0x%x\n",i,val);
			}
		
			//PROC_PRINTF("reg_ISOLATION_EFID[MAX_APOLLO_PORT]:0x%x\n");
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
	
	// ALE34
	PROC_PRINTF("\n-----ALE34-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//PROC_PRINTF("reg_NAT_CTRL:0x%x\n");
	
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP0f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P0]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP1f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P1]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP2f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P2]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP3f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P3]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP4f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P4]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP5f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P5]:0x%x\n",val);
		
			PROC_PRINTF("\n");
		
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP0f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP0]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP1f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP1]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP2f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP2]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP3f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP3]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP4f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP4]:0x%x\n",val);
		
			PROC_PRINTF("\n");
		
			retv = reg_field_read(APOLLOMP_HSBA_CTRLr, APOLLOMP_TST_LOG_MDf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_HSBA_TST_LOG_MODE:0x%x\n",val);
		
			PROC_PRINTF("\n");
		
			//PROC_PRINTF("reg_VS_FIDEN:0x%x\n");
			//PROC_PRINTF("reg_VS_FID:0x%x\n");
		
			for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_L34_PORT_TO_WANr,i,j, APOLLOMP_PORT_TO_WAN_PERMITf, &val);
					PROC_PRINTF("reg_L34_PORT[%d]_TO_WAN[%d]:0x%x\n",i,j,val);
				}
				PROC_PRINTF("\n");
			}
		
			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_L34_EXTPORT_TO_WANr,i,j, APOLLOMP_EXTPORT_TO_WAN_PERMITf, &val);
					PROC_PRINTF("reg_L34_EXTPORT[%d]_TO_WAN[%d]:0x%x\n",i,j,val);
				}
				PROC_PRINTF("\n");
			}
		
			PROC_PRINTF("\n");
		
			for(j=0; j<8; j++)
			{
				for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
				{
					reg_array_field_read(APOLLOMP_L34_WAN_TO_PORTr,i,j, APOLLOMP_WAN_TO_PORT_PERMITf, &val);
					PROC_PRINTF("reg_L34_WAN[%d]_TO_PORT[%d]:0x%x\n",j,i,val);
				}
				PROC_PRINTF("\n");
			}
		
			PROC_PRINTF("\n");
		
			for(j=0; j<8; j++)
			{
				for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
				{
					reg_array_field_read(APOLLOMP_L34_WAN_TO_EXTPORTr,i,j, APOLLOMP_WAN_TO_EXTPORT_PERMITf, &val);
					PROC_PRINTF("reg_L34_WAN[%d]_TO_EXTPORT[%d]:0x%x\n",j,i,val);
				}
				PROC_PRINTF("\n");
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:	
			break;
#endif
#if defined(CONFIG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif
	
#endif
		return len;
	
}
#endif


int32 mibdump_l34_bind_table(struct seq_file *s, void *v)
{
#ifndef CONFIG_APOLLO_RLE0371
	uint32 idx;
	rtk_rg_vbind_linkList_t *pVbdEntry;
	int len=0;

	PROC_PRINTF(">>L34 Binding Table:\n");
	
	for(idx=0; idx<MAX_BIND_HW_TABLE_SIZE; idx++)
	{
		if (!rg_db.bind[idx].valid)
			continue;
		PROC_PRINTF("  [%d]	pmask(%02x) epmask(%02x) vid(%d) wt_idx(%d) bindPtl (%s)\n",
					   idx,
					   rg_db.bind[idx].rtk_bind.portMask.bits[0],
					   rg_db.bind[idx].rtk_bind.extPortMask.bits[0],
					   rg_db.bind[idx].rtk_bind.vidLan,
					   rg_db.bind[idx].rtk_bind.wanTypeIdx,
					   rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV4_IPV6?"other":rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV6?"!v6":rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV4?"!v4":"all"
					  );
	}
	
	PROC_PRINTF(">>Vlan-Binding Num=%d\n",rg_db.systemGlobal.vlanBindTotalNum);
	//diaplay vlan-bind
	for(idx=0;idx<RTK_RG_PORT_MAX;idx++)
	{
		if(!list_empty(&rg_db.vlanBindingListHead[idx]))
		{
			if(idx>RTK_RG_PORT_CPU)
				{PROC_PRINTF("  ExtPort[%d]:",idx-RTK_RG_PORT_CPU);}
			else
				{PROC_PRINTF("  Port[%d]:",idx);}
			list_for_each_entry(pVbdEntry,&rg_db.vlanBindingListHead[idx],vbd_list)
			{
				PROC_PRINTF(" V%d->W%d",pVbdEntry->vlanId,pVbdEntry->wanIdx);
				if(list_is_last(&pVbdEntry->vbd_list,&rg_db.vlanBindingListHead[idx]))
					{PROC_PRINTF("\n");}
				else
					{PROC_PRINTF(",");}
			}
			
		}
	}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	PROC_PRINTF(">>WLAN0 Device Binding:\n");
	for(idx=0;idx<MAX_WLAN_DEVICE_NUM;idx++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[idx].exist)
		{
			PROC_PRINTF("WLAN0 DEV[%d]:",idx);
			if(rg_db.systemGlobal.wlan0BindDecision[idx].set_bind)
				{PROC_PRINTF("Device-binding to WAN[%d].\n",rg_db.systemGlobal.wlan0BindDecision[idx].bind_wanIntf);}
			else
				{PROC_PRINTF("None-binding.\n");}
		}
	}
#endif
#endif
    return len;
}

int32 dump_l34_bind_table(struct seq_file *s, void *v)
{
#ifndef CONFIG_APOLLO_RLE0371
	rtk_binding_entry_t entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>ASIC L34 Binding Table:\n");
	for(idx=0; idx<MAX_BIND_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_bindingTable_get(idx, &entry)!= RT_ERR_OK)
			continue;
		PROC_PRINTF("  [%d]	pmask(%02x) epmask(%02x) vid(%d) wt_idx(%d) bindPtl (%s)\n",
					   idx,
					   entry.portMask.bits[0],
					   entry.extPortMask.bits[0],
					   entry.vidLan,
					   entry.wanTypeIdx,
					   entry.bindProto==L34_BIND_PROTO_NOT_IPV4_IPV6?"other":entry.bindProto==L34_BIND_PROTO_NOT_IPV6?"!v6":entry.bindProto==L34_BIND_PROTO_NOT_IPV4?"!v4":"all"
					  );
	}
#endif
    return len;
}
int32 dump_l34_wantype_table(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	rtk_wanType_entry_t entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>ASIC L34 WAN Type Table:\n");	
	for(idx=0; idx<MAX_WANTYPE_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_wanTypeTable_get(idx, &entry)!= RT_ERR_OK)
			continue;
		switch(entry.wanType)
		{         
			case L34_WAN_TYPE_L2_BRIDGE:
			case L34_WAN_TYPE_L3_ROUTE:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   entry.wanType==L34_WAN_TYPE_L2_BRIDGE?"(L2_BRIDGE)     ":"(L3_ROUTE)      ",
					   entry.nhIdx
					  );
				break;
			case L34_WAN_TYPE_L34NAT_ROUTE:
			case L34_WAN_TYPE_L34_CUSTOMIZED:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   entry.wanType==L34_WAN_TYPE_L34NAT_ROUTE?"(L34NAT_ROUTE)  ":"(L34_CUSTOMIZED)",
					   entry.nhIdx
					  );
				break;
			default:
				break;
		}
	}
	
	PROC_PRINTF(">>Software L34 WAN Type Table:\n");
	for(idx=0; idx<MAX_WANTYPE_SW_TABLE_SIZE; idx++)
	{
		if(!rg_db.wantype[idx].valid)
			continue;
		switch(rg_db.wantype[idx].rtk_wantype.wanType)
		{         
			case L34_WAN_TYPE_L2_BRIDGE:
			case L34_WAN_TYPE_L3_ROUTE:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   rg_db.wantype[idx].rtk_wantype.wanType==L34_WAN_TYPE_L2_BRIDGE?"(L2_BRIDGE)     ":"(L3_ROUTE)      ",
					   rg_db.wantype[idx].rtk_wantype.nhIdx
					  );
				break;
			case L34_WAN_TYPE_L34NAT_ROUTE:
			case L34_WAN_TYPE_L34_CUSTOMIZED:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   rg_db.wantype[idx].rtk_wantype.wanType==L34_WAN_TYPE_L34NAT_ROUTE?"(L34NAT_ROUTE)  ":"(L34_CUSTOMIZED)",
					   rg_db.wantype[idx].rtk_wantype.nhIdx
					  );
				break;
			default:
				break;
		}
	}
#endif
    return len;
}
int32 dump_ipv6_route_table(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	rtk_ipv6Routing_entry_t entry;
	uint32 idx;
	int len=0;

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	PROC_PRINTF(">>ASIC IPv6 Routing Table:(Stateful Routing support:on)\n");
#else
	PROC_PRINTF(">>ASIC IPv6 Routing Table:\n");
#endif
	
	for(idx=0; idx<MAX_IPV6_ROUTING_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_ipv6RoutingTable_get(idx, &entry) != RT_ERR_OK)
			continue;
		if(entry.valid!=TRUE)
			continue;

		switch(entry.type)
		{
			case L34_IPV6_ROUTE_TYPE_TRAP:
					PROC_PRINTF("  [%d]	valid type(TRAP)", idx);
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
					if(rg_db.v6route[idx].rtk_v6route.type!=L34_IPV6_ROUTE_TYPE_TRAP)
					{
						switch(rg_db.v6route[idx].rtk_v6route.type)
						{
							case L34_IPV6_ROUTE_TYPE_DROP:
								PROC_PRINTF("=>orig:DROP");
								break;
							case L34_IPV6_ROUTE_TYPE_LOCAL:
								PROC_PRINTF("=>orig:LOCAL DNET_IF(%d)",entry.nhOrIfidIdx);
								break;
							case L34_IPV6_ROUTE_TYPE_GLOBAL:
								PROC_PRINTF("=>orig:GLOBAL NEXTHOP(%d)",entry.nhOrIfidIdx);
								break;
							default:
								break;
						}
					}
#endif
				break;
    		case L34_IPV6_ROUTE_TYPE_DROP:
				PROC_PRINTF("  [%d]	valid type(DROP)", idx);
				break;
    		case L34_IPV6_ROUTE_TYPE_LOCAL:
				PROC_PRINTF("  [%d]	valid type(LOCAL) DNET_IF(%d)", idx,entry.nhOrIfidIdx);
				break;
    		case L34_IPV6_ROUTE_TYPE_GLOBAL:
				PROC_PRINTF("  [%d]	valid type(GLOBAL) NEXTHOP(%d)", idx,entry.nhOrIfidIdx);
				break;
			default:
				break;
		}

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		PROC_PRINTF("%s prefixLen(%d) \n	   DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
				(entry.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
				entry.ipv6PrefixLen,
				entry.ipv6Addr.ipv6_addr[0],entry.ipv6Addr.ipv6_addr[1],entry.ipv6Addr.ipv6_addr[2],entry.ipv6Addr.ipv6_addr[3],
				entry.ipv6Addr.ipv6_addr[4],entry.ipv6Addr.ipv6_addr[5],entry.ipv6Addr.ipv6_addr[6],entry.ipv6Addr.ipv6_addr[7],
				entry.ipv6Addr.ipv6_addr[8],entry.ipv6Addr.ipv6_addr[9],entry.ipv6Addr.ipv6_addr[10],entry.ipv6Addr.ipv6_addr[11],
				entry.ipv6Addr.ipv6_addr[12],entry.ipv6Addr.ipv6_addr[13],entry.ipv6Addr.ipv6_addr[14],entry.ipv6Addr.ipv6_addr[15]);

#else
		PROC_PRINTF("%s prefixLen(%d)\n     DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
						(entry.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
						entry.ipv6PrefixLen,
						entry.ipv6Addr.ipv6_addr[0],entry.ipv6Addr.ipv6_addr[1],entry.ipv6Addr.ipv6_addr[2],entry.ipv6Addr.ipv6_addr[3],
						entry.ipv6Addr.ipv6_addr[4],entry.ipv6Addr.ipv6_addr[5],entry.ipv6Addr.ipv6_addr[6],entry.ipv6Addr.ipv6_addr[7],
						entry.ipv6Addr.ipv6_addr[8],entry.ipv6Addr.ipv6_addr[9],entry.ipv6Addr.ipv6_addr[10],entry.ipv6Addr.ipv6_addr[11],
						entry.ipv6Addr.ipv6_addr[12],entry.ipv6Addr.ipv6_addr[13],entry.ipv6Addr.ipv6_addr[14],entry.ipv6Addr.ipv6_addr[15]);
#endif
	}
#endif


#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	PROC_PRINTF(">>Software IPv6 Routing Table:(IPV6 NAPT support:on)\n");
	for(idx=0; idx<MAX_IPV6_ROUTING_SW_TABLE_SIZE; idx++)
	{
		if(rg_db.v6route[idx].rtk_v6route.valid==1){

			PROC_PRINTF("[%d] ",idx);

			switch(rg_db.v6route[idx].rtk_v6route.type)
			{
				case L34_IPV6_ROUTE_TYPE_TRAP:
					PROC_PRINTF(" TRAP");
					break;				
				case L34_IPV6_ROUTE_TYPE_DROP:
					PROC_PRINTF(" DROP");
					break;
				case L34_IPV6_ROUTE_TYPE_LOCAL:
					PROC_PRINTF(" LOCAL DNET_IF(%d)",rg_db.v6route[idx].rtk_v6route.nhOrIfidIdx);
					break;
				case L34_IPV6_ROUTE_TYPE_GLOBAL:
					PROC_PRINTF(" GLOBAL NEXTHOP(%d)",rg_db.v6route[idx].rtk_v6route.nhOrIfidIdx);
					break;
				default:
					break;
			}

			
			PROC_PRINTF("%s %s prefixLen(%d) \n	   DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
					(entry.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
					(rg_db.v6route[idx].internal?"(EXTERNAL)":"(INTERNAL)"), entry.ipv6PrefixLen,
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[0],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[1],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[2],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[3],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[4],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[5],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[6],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[7],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[8],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[9],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[10],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[11],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[12],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[13],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[14],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[15]);

		}
	}

#endif

    return len;
}
int32 dump_ipv6_neighbor_table(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	rtk_ipv6Neighbor_entry_t entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF("\n\n>>ASIC IPv6 Neighbor Table:\n");
	for(idx=0; idx<MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_ipv6NeighborTable_get(idx, &entry) != RT_ERR_OK)
			continue;
		if (entry.valid==FALSE)
			continue;
		PROC_PRINTF("  [%d]	valid L2Idx(%d) routingIdx(%d) InterfaceID(%02x%02x:%02x%02x:%02x%02x:%02x%02x) idle=%d static=%d\n",
					   idx, entry.l2Idx,
					   entry.ipv6RouteIdx,
#if 0
					   (uint32)((entry.ipv6Ifid&0xff00000000000000)>>56),
					   (uint32)((entry.ipv6Ifid&0xff000000000000)>>48),
					   (uint32)((entry.ipv6Ifid&0xff0000000000)>>40),
					   (uint32)((entry.ipv6Ifid&0xff00000000)>>32),
#else
					   (uint32)((entry.ipv6Ifid>>56)&0xff),
					   (uint32)((entry.ipv6Ifid>>48)&0xff),
					   (uint32)((entry.ipv6Ifid>>40)&0xff),
					   (uint32)((entry.ipv6Ifid>>32)&0xff),
#endif
					   (uint32)((entry.ipv6Ifid&0xff000000)>>24),
					   (uint32)((entry.ipv6Ifid&0xff0000)>>16),
					   (uint32)((entry.ipv6Ifid&0xff00)>>8),
					   (uint32)((entry.ipv6Ifid&0xff)),
					   rg_db.v6neighbor[idx].idleSecs,
					   rg_db.v6neighbor[idx].staticEntry
					  );
	}
#endif

    return len;
}

int32 mibdump_ipv6_neighbor_table(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	rtk_ipv6Neighbor_entry_t *entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>ASIC IPv6 Neighbor Table:\n");
	for(idx=0; idx<MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE; idx++)
	{
		entry=&rg_db.v6neighbor[idx].rtk_v6neighbor;
		if (entry->valid==FALSE)
			continue;
		PROC_PRINTF("  [%d]	valid L2Idx(%d) routingIdx(%d) InterfaceID(%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
					   idx, entry->l2Idx,
					   entry->ipv6RouteIdx,
#if 0
					   (uint32)((entry.ipv6Ifid&0xff00000000000000)>>56),
					   (uint32)((entry.ipv6Ifid&0xff000000000000)>>48),
					   (uint32)((entry.ipv6Ifid&0xff0000000000)>>40),
					   (uint32)((entry.ipv6Ifid&0xff00000000)>>32),
#else
					   (uint32)((entry->ipv6Ifid>>56)&0xff),
					   (uint32)((entry->ipv6Ifid>>48)&0xff),
					   (uint32)((entry->ipv6Ifid>>40)&0xff),
					   (uint32)((entry->ipv6Ifid>>32)&0xff),
#endif
					   (uint32)((entry->ipv6Ifid&0xff000000)>>24),
					   (uint32)((entry->ipv6Ifid&0xff0000)>>16),
					   (uint32)((entry->ipv6Ifid&0xff00)>>8),
					   (uint32)((entry->ipv6Ifid&0xff))
					  );
	}
#endif

    return len;
}

int32 dump_piso(struct seq_file *s, void *v)
{
	int i,j,ret=0;
	rtk_portmask_t mbr= {{0}},extmbr= {{0}};
	int len=0;
	PROC_PRINTF(">>Port Isolation:\n");
	for(j=0; j<RTK_PORT_ISO_CFG_END; j++)
	{
		PROC_PRINTF("  - mode %d - \n", j);
		for(i=0; i<RTK_RG_MAX_MAC_PORT; i++)
		{

		 PROC_PRINTF("\t[Phy port %d:]", i);
		 ret = rtk_port_isolationEntry_get(j,i,&mbr,&extmbr);
		 PROC_PRINTF("\tPhy Port Mask:0x%x",mbr.bits[0]);
		 PROC_PRINTF("\tExtension Port Mask:0x%x\n",extmbr.bits[0]);

		}
		for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
		{

		 PROC_PRINTF("\t[Ext. port %d:]", i);
		 ret = rtk_port_isolationEntryExt_get(j,i,&mbr,&extmbr);
		 PROC_PRINTF("\tPhy Port Mask:0x%x",mbr.bits[0]);
		 PROC_PRINTF("\tExtension Port Mask:0x%x\n",extmbr.bits[0]);

		}
	}

    return len;
}

int32 dump_wlan_mbssid(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	PROC_PRINTF("IDX\tMAC\t\t\tINTF\tctag_if\tVID\tDiffJiffies\n");
	for(i=rg_db.wlanMbssidHeadIdx;i<rg_db.wlanMbssidHeadIdx+MAX_WLAN_MBSSID_SW_TABLE_SIZE;i++)
	{
		int idx=i%MAX_WLAN_MBSSID_SW_TABLE_SIZE;
		if(memcmp(rg_db.wlanMbssid[idx].mac.octet,"\x0\x0\x0\x0\x0\x0",6)==0) continue;
		PROC_PRINTF("%02d\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%d\t%d\t-%lu\n",idx,
			rg_db.wlanMbssid[idx].mac.octet[0],
			rg_db.wlanMbssid[idx].mac.octet[1],
			rg_db.wlanMbssid[idx].mac.octet[2],
			rg_db.wlanMbssid[idx].mac.octet[3],
			rg_db.wlanMbssid[idx].mac.octet[4],
			rg_db.wlanMbssid[idx].mac.octet[5],
			rg_db.wlanMbssid[idx].wlan_dev_idx,
			rg_db.wlanMbssid[idx].vlan_tag_if,
			rg_db.wlanMbssid[idx].vid,
			jiffies-rg_db.wlanMbssid[idx].learn_jiffies	);		

	}
	return len;
}


void dump_compare_packet(uint8 *output,uint8 *expect,int out_size,int expect_size)
{
    int off;
    u8 protocol=0;
    int i,j,pppoeif=0;
    uint8 *pkt=NULL;
    char tmpstr[32];
    int size=max(out_size,expect_size);
	


    for(j=1; j<3; j++)
    {
        //if(j==0) {pkt=input; sprintf(tmpstr,"INPUT");}
        if(j==1)
        {
            pkt=output;
            sprintf(tmpstr,"OUTPUT(size=%d)",out_size);
        }
        if(j==2)
        {
            pkt=expect;
            sprintf(tmpstr,"EXPECT(size=%d)",expect_size);
        }

        for(i=0; i<78; i++)
            rtlglue_printf("=");

        rtlglue_printf("\n");

        rtlglue_printf("%s\n",tmpstr);
        if(size==0) return;

        for(i=0; i<size; i++)
        {
            int diff=0;

            if(j==1)
            {
                if((i>=out_size)||(out_size<=4))
                    break;//goto skip_dump;
            }
            if(j==2)
            {
                if((i>=expect_size)||(expect_size<=4))
                    break;//goto skip_dump;
            }
            if((i%16)==0) rtlglue_printf("%08x ",(unsigned int)(unsigned long int)(&pkt[i]));
            //if((input[i]!=output[i])||(output[i]!=expect[i])) diff=1;
            if(i<min(out_size,expect_size-4))
                if(output[i]!=expect[i]) diff=1;
            rtlglue_printf("%s%02x%s ",(diff==1)?"*"COLOR_H:"",(u8)pkt[i],(diff==1)?COLOR_NM:"");
            if((i%16)==7) rtlglue_printf("  ");
            if((i%16)==15) rtlglue_printf("\n");
        }
        if(i%16!=15)
            rtlglue_printf("\n");

        rtlglue_printf(COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[0],pkt[1],pkt[2],pkt[3],pkt[4],pkt[5]
                       ,pkt[6],pkt[7],pkt[8],pkt[9],pkt[10],pkt[11]);
        off=12;
        if((pkt[off]==0x88)&&(pkt[off+1]==0x99))
        {
            rtlglue_printf("CPU:[" COLOR_Y "Protocol" COLOR_NM "=%d][" COLOR_Y "Res" COLOR_NM "=0x%x][" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "TTL_1" COLOR_NM "=0x%x][" COLOR_Y "L3R" COLOR_NM "=%d][" COLOR_Y "ORG" COLOR_NM "=%d][" COLOR_Y "SPA" COLOR_NM "=%d][" COLOR_Y "EPMSK" COLOR_NM "=0x%x]\n"
                           ,pkt[off+2],pkt[off+3],pkt[off+4]>>5,pkt[off+4]&0x1f
                           ,pkt[off+5]>>7,(pkt[off+5]>>6)&1,pkt[off+5]&7,pkt[off+7]&0x3f);
            off+=8;
        }

        if((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
        {
            rtlglue_printf("SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
            off+=4;
        }

        if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
        {
            rtlglue_printf("CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
            off+=4;
        }

        if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
        {
            rtlglue_printf("PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x]\n",
                           pkt[off+3],(pkt[off+4]<<8)|pkt[off+5]);
            off+=8;
            pppoeif=1;
        }

        if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)))		//IPv6 or IPv6 with PPPoE
        {
            rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%02x%02x%x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4, (pkt[off+2]&0xf)+(pkt[off+3]>>4), (pkt[off+3]&0xf)+(pkt[off+4]>>4), (pkt[off+4]&0xf)+(pkt[off+5]>>4), (pkt[off+5]&0xf), (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
            rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
                           ,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
            rtlglue_printf("     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
                           ,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

            protocol=pkt[off+8];
            if(protocol==0)	//hop-by-hop
            {
                rtlglue_printf("Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
                               ,pkt[off+42], pkt[off+43]);
                rtlglue_printf("          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
                               ,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
                for(i=0; i<pkt[off+43]; i++)
                {
                    rtlglue_printf("         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
                                   ,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
                                   ,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
                }

                protocol=pkt[off+42];
                off+=(50+pkt[off+43]*8);
            }
            else
                off+=42;
        }
        //rtlglue_printf("###############the pkt off before IPV4 is %02x off+1 is %02x, pppoeif is %d\n",pkt[off],pkt[off+1],pppoeif);
        if(((pkt[off]==0x08)&&(pkt[off+1]==0x00)&&(pppoeif==0))||((pkt[off]==0x00)&&(pkt[off+1]==0x21)&&(pppoeif==1)))
        {
            rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=%d(DSCP=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
                           ,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
            rtlglue_printf("	 [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                           ,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
            rtlglue_printf("	 [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
                           ,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

            protocol=pkt[off+11];
            off+=(pkt[off+2]&0xf)*4+2;
        }

        if(protocol==0x6) //TCP
        {
            rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                           ,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
                           ,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
            rtlglue_printf("	[" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                           ,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
                           ,(pkt[off+14]<<8)|pkt[off+15]);
            rtlglue_printf("	[" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
        }
        else if(protocol==0x11) //UDP
        {
            rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
                           ,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));
        }
//skip_dump:
        //      ;
    }

}



int32 mibdump_srvInLan_IP_table(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	PROC_PRINTF(">>MIB for ALG Server In LAN IP Table:\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress>0)
		{
			switch(rg_db.algServInLanIpMapping[i].algType)
			{
				case RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN SIP_TCP");
					break;
				case RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN SIP_UDP");
					break;
				case RTK_RG_ALG_H323_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN H323_TCP");
					break;
				case RTK_RG_ALG_H323_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN H323_UDP");
					break;
				case RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN RTSP_TCP");
					break;
				case RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN RTSP_UDP");
					break;
				case RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN FTP_TCP");
					break;
				case RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN FTP_UDP");
					break;
				default:
					break;
			}
			PROC_PRINTF("(%x) serverIP is %08x\n",
				rg_db.algServInLanIpMapping[i].algType,rg_db.algServInLanIpMapping[i].serverAddress);
		}
	}

	return len;
}

int32 mibdump_virtualServer(struct seq_file *s, void *v)
{
	int i,ret;
	rtk_rg_virtualServer_t virtual_server;
	int valid_idx;
	int len=0;

	PROC_PRINTF("Virtual server:\n");
	for(i=0;i<MAX_VIRTUAL_SERVER_SW_TABLE_SIZE;i=valid_idx+1)
	{
		valid_idx=i;
	
		ret=rtk_rg_apollo_virtualServer_find(&virtual_server,&valid_idx);
		if(ret==RT_ERR_RG_OK)
		{
			PROC_PRINTF("[%d] %s WAN-intf:%d, gwPortStart:%d, localIP:Port=>%08x:%d, mappingRangeCnt=%d, mappingType=[%s]\n",
				valid_idx,
				virtual_server.is_tcp?"TCP":"UDP",
				virtual_server.wan_intf_idx,
				virtual_server.gateway_port_start,
				virtual_server.local_ip,
				virtual_server.local_port_start,
				virtual_server.mappingPortRangeCnt,
				(virtual_server.mappingType==VS_MAPPING_N_TO_N)?"N-to-N":"N-to-1"
				);
		}
		else
			break;
	}

	return len;
}

int32 mibdump_upnpConnection(struct seq_file *s, void *v)
{
	int i,ret;
	rtk_rg_upnpConnection_t upnp;
	int valid_idx;
	int len=0;
		
	PROC_PRINTF(">>MIB for UPnP connections:\n");
	for(i=0;i<MAX_UPNP_SW_TABLE_SIZE;i=valid_idx+1)
	{
		valid_idx=i;
		
		ret=rtk_rg_apollo_upnpConnection_find(&upnp,&valid_idx);
		if(ret==RT_ERR_RG_OK)
		{
			PROC_PRINTF("[%d] %s WAN:%d TYPE:%s TIMEOUT:%d(Idle %d Secs) gwPort:%d localIPandPort=>%08x:%d limitIP=%s,IP=%08x limitPort=%s,Port=%d\n",
				valid_idx,
				upnp.is_tcp?"TCP":"UDP",
				upnp.wan_intf_idx,
				upnp.type==UPNP_TYPE_ONESHOT?"ONE-SHOT":"PERSIST",
				upnp.timeout,
				upnp.idle,
				upnp.gateway_port,
				upnp.local_ip,
				upnp.local_port,
				upnp.limit_remote_ip?"Enable":"DISABLE",
				upnp.remote_ip,
				upnp.limit_remote_port?"Enable":"DISABLE",
				upnp.remote_port);
		}
		else
			break;
	}
	
	return len;
}

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) &&  defined(CONFIG_XDSL_ROMEDRIVER)
//rtl865x

int32 sw_vlan(struct seq_file *s, void *v){

	int i=0;
	int len=0;
	
	PROC_PRINTF("SW VLAN Table: \n");	
	for(i=0;i<MAX_VLAN_SW_TABLE_SIZE;i++){
		if(rg_db.vlan[i].valid){

			PROC_PRINTF("VLAN[%d]\n\tCustomerVLAN:%d  MemberPortmask:%x	Ext_portmask:%x UntagPortmask=%x \n"
				,i,rg_db.vlan[i].addedAsCustomerVLAN,rg_db.vlan[i].MemberPortmask.bits[0],rg_db.vlan[i].Ext_portmask.bits[0],rg_db.vlan[i].UntagPortmask.bits[0]);
			if(rg_db.vlan[i].fidMode ==VLAN_FID_IVL){
				PROC_PRINTF("\tIVL Mode");
			}else if(rg_db.vlan[i].fidMode==VLAN_FID_SVL){
				PROC_PRINTF("\tSVL Mode");
			}
			PROC_PRINTF("fid=%d	priorityEn=%d  priority=%d\n",rg_db.vlan[i].fid,rg_db.vlan[i].priorityEn,rg_db.vlan[i].priority);
		}
	}
	return len;
}

int32 sw_napt(struct seq_file *s, void *v){

	int i=0;
	uint32 totalCount=0,inCount=0,outCount=0;
	int len=0;

	PROC_PRINTF("SW NAPT Table: \n");	
	for(i=0;i<MAX_NAPT_IN_HW_TABLE_SIZE;i++){
		if(rg_db.naptIn[i].rtk_naptIn.valid){

			if(rg_db.naptOut[i].rtk_naptOut.valid){
				totalCount++;
				//PROC_PRINTF("[%d] (inBound)symmetricNaptOutIdx=%d (outBound)symmetricNaptInIdx=%d \n",i,rg_db.naptIn[i].symmetricNaptOutIdx,rg_db.naptOut[i].rtk_naptOut.hashIdx);

				if(rg_db.naptIn[i].symmetricNaptOutIdx){
					PROC_PRINTF("[%d] (inBound)symmetricNaptOutIdx=%d \n",i,rg_db.naptIn[i].symmetricNaptOutIdx);
					inCount++;
				}
				if(rg_db.naptOut[i].rtk_naptOut.hashIdx){
					PROC_PRINTF("[%d] (outBound)symmetricNaptInIdx=%d\n",i,rg_db.naptOut[i].rtk_naptOut.hashIdx);
					outCount++;
				}
				
			}else{
				PROC_PRINTF("napt table[%d]< MAX_NAPT_IN_HW_TABLE_SIZE not syc!!!!!!!!!!\n",i);
			}

		}
	}

	for(i=0;i<MAX_NAPT_IN_HW_TABLE_SIZE;i++){
		if(rg_db.naptOut[i].rtk_naptOut.valid){
			if(rg_db.naptIn[i].rtk_naptIn.valid){
			}else{
				PROC_PRINTF("napt table[%d]< MAX_NAPT_IN_HW_TABLE_SIZE not syc!!!!!!!!!!\n",i);
			}

		}
	}

	PROC_PRINTF("hw_totalCount=%d  inCount=%d  outCount=%d \n",totalCount,inCount,outCount);

	PROC_PRINTF("========================================== \n");	
	PROC_PRINTF("SW NAPT_IN Table: \n");	

	for(i=MAX_NAPT_IN_HW_TABLE_SIZE;i<MAX_NAPT_IN_SW_TABLE_SIZE;i++){
		if(rg_db.naptIn[i].rtk_naptIn.valid){
			PROC_PRINTF("[%d] (inBound)symmetricNaptOutIdx=%d \n",i,rg_db.naptIn[i].symmetricNaptOutIdx);
			inCount++;
		}

	}
	PROC_PRINTF("Total inCount=%d  \n",inCount);


	PROC_PRINTF("========================================== \n");	
	PROC_PRINTF("SW NAPT_OUT Table: \n");		
	for(i=MAX_NAPT_OUT_HW_TABLE_SIZE;i<MAX_NAPT_OUT_SW_TABLE_SIZE;i++){
		if(rg_db.naptOut[i].rtk_naptOut.valid){
			outCount++;
			PROC_PRINTF("[%d] (outBound)symmetricNaptInIdx=%d\n",i,rg_db.naptOut[i].rtk_naptOut.hashIdx);
		}
	}
	
	PROC_PRINTF("Total outCount=%d \n",outCount);

	
	return len;
}


int32 sw_lutTable(struct seq_file *s, void *v){

	int i;
	int len=0;
	rtk_l2_ucastAddr_t	 *l2UcEntry;

	PROC_PRINTF("SW L2 Table: \n");	
	for(i=0;i<MAX_LUT_SW_TABLE_SIZE;i++){

		if(rg_db.lut[i].valid){
			if(rg_db.lut[i].rtk_lut.entryType ==RTK_LUT_L2UC){
				l2UcEntry= &(rg_db.lut[i].rtk_lut.entry.l2UcEntry);
				PROC_PRINTF("  %3d.[%3d,%d] Mac:%02x-%02x-%02x-%02x-%02x-%02x portID:%d fid:%d  vid=%4d age:%3d auth:%d\n",i,i>>2,i&0x3,l2UcEntry->mac.octet[0],l2UcEntry->mac.octet[1],l2UcEntry->mac.octet[2],
					l2UcEntry->mac.octet[3],l2UcEntry->mac.octet[4],l2UcEntry->mac.octet[5],l2UcEntry->port,l2UcEntry->fid,l2UcEntry->vid,l2UcEntry->age,l2UcEntry->auth);
			}else{
			//other  type

			}

		}
	}
	return len;
}


int32 sw_l3(struct seq_file *s, void *v){

	int i;
	int len=0;

	PROC_PRINTF("SW L3 Table: \n");	
	for(i=0;i<MAX_L3_SW_TABLE_SIZE;i++){

		if(rg_db.l3[i].rtk_l3.valid){
			PROC_PRINTF("[%d]ipAddr=%d.%d.%d.%d/%d\n",i,(rg_db.l3[i].rtk_l3.ipAddr >>24)&0xff,(rg_db.l3[i].rtk_l3.ipAddr >>16)&0xff,(rg_db.l3[i].rtk_l3.ipAddr >>8)&0xff,(rg_db.l3[i].rtk_l3.ipAddr)&0xff,
				rg_db.l3[i].rtk_l3.ipMask);
		}
	}
	return len;
}


#endif //defined(CONFIG_APOLLO)

#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
int32 mibdump_DMZ(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	
	PROC_PRINTF(">>MIB for DMZ settings:\n");
	for(i=0;i<MAX_DMZ_TABLE_SIZE;i++)
	{	
		if(rg_db.dmzInfo[i].enabled)
		{
			if(rg_db.dmzInfo[i].mac_mapping_enabled)
				{PROC_PRINTF("[%d] DMZ for %02x:%02x:%02x:%02x:%02x:%02x\n",
					i,
					rg_db.dmzInfo[i].mac.octet[0],
					rg_db.dmzInfo[i].mac.octet[1],
					rg_db.dmzInfo[i].mac.octet[2],
					rg_db.dmzInfo[i].mac.octet[3],
					rg_db.dmzInfo[i].mac.octet[4],
					rg_db.dmzInfo[i].mac.octet[5]);}
			else
				{PROC_PRINTF("[%d] DMZ for ipv4[%08x] ",i,rg_db.dmzInfo[i].private_ip);
				 PROC_PRINTF("ipv6[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n",
				 	rg_db.dmzInfo[i].private_ipv6.ipv6_addr[0],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[1],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[2],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[3],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[4],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[5],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[6],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[7],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[8],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[9],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[10],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[11],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[12],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[13],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[14],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[15]);}
		}
	}

	return len;
}
#endif

void common_dump(rtk_rg_debug_level_t level,const char *funcs,int line, char *comment,...)
{
	int show=1;
	char string[16]={0};
	int color, bgcolor;
	va_list a_list;
	va_start(a_list,comment);

	if(rg_kernel.filter_level&level)
	{
		show=_rtk_rg_trace_filter_compare(rg_db.pktHdr->skb,rg_db.pktHdr);
	}
	if(show==1)
	{
		int mt_trace_i;
		
		vsprintf( mt_watch_tmp, comment,a_list);
		for(mt_trace_i=1;mt_trace_i<512;mt_trace_i++)
		{
			if(mt_watch_tmp[mt_trace_i]==0)
			{
				if(mt_watch_tmp[mt_trace_i-1]=='\n') mt_watch_tmp[mt_trace_i-1]=' ';
				else break;
			}
		}

		switch(level)
		{
			case RTK_RG_DEBUG_LEVEL_DEBUG:
				sprintf(string,"DEBUG"); color=33; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_FIXME:
				sprintf(string,"FIXME"); color=34; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_CALLBACK:
				sprintf(string,"CALLBACK"); color=34; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_TRACE:
				sprintf(string,"TRACE"); color=35; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_ACL:
				sprintf(string,"ACL"); color=36; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_WARN:
				sprintf(string,"WARNING"); color=33; bgcolor=41;
				break;				
			case RTK_RG_DEBUG_LEVEL_TRACE_DUMP:
				sprintf(string,"TRACE_DUMP");color=35; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_WMUX:
				sprintf(string,"WMUX");color=32; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_MACLEARN:
				sprintf(string,"MACLEARN");color=31; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_TABLE:
				sprintf(string,"TABLE");color=31; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_ALG:
				sprintf(string,"ALG");color=31; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_IGMP:
				sprintf(string,"IGMP");color=31; bgcolor=40;
				break;				
			case RTK_RG_DEBUG_LEVEL_ACL_RRESERVED:
				sprintf(string,"ACL_RSV");color=36; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_RG_API:
				sprintf(string,"RG_API");color=36; bgcolor=40;
				break;
			default:
				sprintf(string,"XXX");color=37; bgcolor=40;
				break;

		}
		rtlglue_printf("\033[1;%d;%dm[%s] %s\033[1;30;40m @ %s(%d)\033[0m\n",color,bgcolor,string,mt_watch_tmp,funcs,line);
	}

	va_end (a_list);  
}


rtk_rg_proc_t debugProc[]=
{

#if defined(CONFIG_APOLLO) 
	{
		.name="l4" ,
		.get = dump_napt ,
		.set = NULL ,
	},
	{
		.name="sw_netstat" ,
		.get = dump_netstat ,
		.set = NULL ,
	},

#if defined(CONFIG_RTL9602C_SERIES)

	{
		.name="ipmv6_l3mcr" ,
		.get = dump_ipmv6_l3mcr_table ,
		.set = NULL ,
	},
	{
		.name="dslite" ,
		.get = dump_dslite_table ,
		.set = NULL ,
	},
	{
		.name="dslite_status" ,
		.get = dump_dslite_status ,
		.set = NULL ,
	},
	{
		.name="dsliteMc" ,
		.get = dump_dsliteMc_table ,
		.set = NULL ,
	},
    {
        .name="host_policing" ,
        .get =  _rtk_rg_proc_host_policing_get,
        .set =  NULL,
    },
#endif

	{
		.name="napt" ,
		.get = dump_tcpudp ,
		.set = NULL ,
	},
	{
		.name="netif" ,
		.get = dump_netif ,
		.set = NULL ,
	},
	{
		.name="pppoe" ,
		.get = dump_pppoe ,
		.set = NULL ,
	},
	{
		.name="nexthop" ,
		.get = dump_nexthop ,
		.set = NULL ,
	},
	{
		.name="arp" ,
		.get = dump_arp ,
		.set = NULL ,
	},
	{
		.name="ip" ,
		.get = dump_ip ,
		.set = NULL ,
	},
	{
		.name="v6Extip" ,
		.get = dump_ipv6_extIp ,
		.set = NULL ,
	},
	{
		.name="l3" ,
		.get = dump_l3 ,
		.set = NULL ,
	},
	{
		.name="l4hs" ,
		.get = dump_l4hs ,
		.set = NULL ,
	},
	{
		.name="hs" ,
		.get = dump_hs ,
		.set = NULL ,
	},
	{
		.name="lut" ,
		.get = dump_lut_table ,
		.set = NULL ,
	},
	{
		.name="bcamlut" ,
		.get = dump_lut_bcam_table ,
		.set = NULL ,
	},
	{
		.name="l2" ,
		.get = dump_lut_table ,
		.set = NULL ,
	},
	{
		.name="ipmcgrp" ,
		.get = dump_ipmc_group ,
		.set = NULL ,
	},
	{
		.name="ipmc" ,
		.get = dump_ipmc_routing ,
		.set = NULL ,
	},
	{
		.name="vlan" ,
		.get = dump_vlan_table ,
		.set = NULL ,
	},
	{
		.name="vlanmbr" ,
		.get = dump_vlan_mbr ,
		.set = NULL ,
	},
	{
		.name="svlan" ,
		.get = dump_svlan_mbr ,
		.set = NULL ,
	},
	{
		.name="gatewayServicePort" ,
		.get = dump_gatewayServicePort ,
		.set = NULL ,
	},
	{
		.name="acl" ,
		.get = dump_acl ,
		.set = NULL ,
	},
	{
		.name="acl_rg" ,
		.get = dump_rg_acl ,
		.set = NULL ,
	},
	{
		.name="sw_acl" ,
		.get = dump_rg_acl ,
		.set = NULL ,
	},
	{
		.name="napt_filterAndQos_rg" ,
		.get = dump_rg_naptPriority ,
		.set = NULL ,
	},
	{
		.name="sw_napt_filterAndQos" ,
		.get = dump_rg_naptPriority ,
		.set = NULL ,
	},
	{
		.name="acl_template" ,
		.get = dump_acl_template ,
		.set = NULL ,
	},
	{
		.name="acl_vidrange" ,
		.get = dump_acl_vidRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_iprange" ,
		.get = dump_acl_ipRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_portrange" ,
		.get = dump_acl_portRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_pktlenrange" ,
		.get = dump_acl_pktlenRangeTable ,
		.set = NULL ,
	},
	{
		.name="cf" ,
		.get = dump_cf ,
		.set = NULL ,
	},
	{
		.name="cf_rg" ,
		.get = dump_rg_cf ,
		.set = NULL ,
	},
	{
		.name="sw_cf" ,
		.get = dump_rg_cf ,
		.set = NULL ,
	},
	{
		.name="acl_reserved_info" ,
		.get = dump_rg_acl_reserved_info,
		.set = NULL ,
	},
#if 0	
	{
		.name="reg" ,
		.get = dump_reg ,
		.set = NULL ,
	},
#endif	
	{
		.name="bind" ,
		.get = dump_l34_bind_table ,
		.set = NULL ,
	},
	{
		.name="wantype" ,
		.get = dump_l34_wantype_table ,
		.set = NULL ,
	},

	{
		.name="v6route" ,

		.get = dump_ipv6_route_table ,
		.set = NULL ,
	},
	{
		.name="neighbor" ,
		.get = dump_ipv6_neighbor_table ,
		.set = NULL ,
	},
	{
		.name="portIso" ,
		.get = dump_piso ,
		.set = NULL ,
	},
	{
		.name="sw_portIso" ,
		.get = mibdump_port_isolation ,
		.set = NULL ,
	},
	{
		.name="sw_shortcut" ,
		.get = dump_sw_shortcut ,
		.set = NULL ,
	},
	{
		.name="sw_mbssid" ,
		.get = dump_wlan_mbssid ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpAll" ,
		.get = mibdump_redirect_httpAll ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpURL" ,
		.get = mibdump_redirect_httpURL ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpWhiteList" ,
		.get = mibdump_redirect_httpWhiteList ,
		.set = NULL ,
	},
	{
		.name="diag_shell" ,
		.get = diag_shell_dump ,
		.set = NULL ,
	},
	{
		.name="sw_layer2LearningStatistics" ,
		.get = mibdump_softwareLearningStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_urlFilter" ,
		.get = dump_rg_urlFilter ,
		.set = NULL ,
	},
	{
		.name="sw_accessWanStatistics" ,
		.get = mibdump_accessWanStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_categories" ,
		.get = mibdump_softwareLearningCategories ,
		.set = NULL,
	},
	{
		.name="sw_lut" ,
		.get = mibdump_lut_table ,
		.set = NULL ,
	},
	{
		.name="sw_bcamlut" ,
		.get = mibdump_lut_bcam_table ,
		.set = NULL ,
	},
	{
		.name="sw_neighbor" ,
		.get = mibdump_ipv6_neighbor_table ,
		.set = NULL ,
	},
	{
		.name="sw_frag" ,
		.get = mibdump_frag ,
		.set = NULL ,
	},
	{
		.name="sw_srvInLan" ,
		.get = mibdump_srvInLan_IP_table ,
		.set = NULL ,
	},
	{
		.name="sw_virtualServers" ,
		.get = mibdump_virtualServer ,
		.set = NULL ,
	},
	{
		.name="sw_upnpConnections" ,
		.get = mibdump_upnpConnection ,
		.set = NULL ,
	},
	{
		.name="sw_bind" ,
		.get = mibdump_l34_bind_table ,
		.set = NULL ,
	},

#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT

	{
		.name="sw_dmzSettings" ,
		.get = mibdump_DMZ ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_APOLLO_ROMEDRIVER

	{
		.name="sw_algDatabase" ,
		.get = _rtk_rg_alg_displayAllDataBase ,
		.set = NULL ,
	},
	{
		.name="sw_algDynamicPort" ,

		.get = mibdump_alg_dynamic_port ,
		.set = NULL ,
	},
	{
		.name="sw_algSrvInLanIPAddress" ,
		.get = mibdump_alg_srvInLan_ip ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_RG_WMUX_SUPPORT

	{
		.name="wmux" ,
		.get = _rtk_rg_wmux_info ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT

	{
		.name="sw_ipv6statefulInfo" ,
		.get = mibdump_ipv6_stateful_connections ,
		.set = NULL ,
	},
	{
		.name="sw_ipv6fragInfo" ,
		.get = mibdump_v6Frag ,
		.set = NULL ,
	},
#endif
	{
		.name="sw_vlan" ,
		.get = dump_rg_vlan_table ,
		.set = NULL ,
	},
	{
		.name="gponDsBcFilterAndRemarking" ,
		.get = dump_gpon_ds_bc_filterAndRemarkingRules ,
		.set = NULL ,
	},
	
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	{
		.name="sw_vmacSkb" ,
		.get = mibdump_vmac_skb ,
		.set = mibclean_vmac_skb ,
	},
#endif

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)//end APOLLO

	{
		.name="sw_lutTable" ,
		.get = sw_lutTable ,
		.set = NULL ,
	},
	{
		.name="sw_l3" ,
		.get = sw_l3 ,
		.set = NULL ,
	},
	{
		.name="sw_napt" ,
		.get = sw_napt ,
		.set = NULL ,
	},
	{
		.name="sw_vlan" ,
		.get = sw_vlan ,
		.set = NULL ,
	},
	//1 FIXME:compile error
/* 
	{
		.name="acl" ,
		.get = _dump_xdsl_acl ,
		.set = NULL ,
	},
	{
		.name="acl_rg" ,
		.get = dump_rg_acl ,
		.set = NULL,
	},
*/

#endif

};

static int NULL_debug_get(struct seq_file *s, void *v){ return 0;}
static int NULL_debug_single_open(struct inode *inode, struct file *file){return(single_open(file, NULL_debug_get, NULL));}

static int common_debug_single_open(struct inode *inode, struct file *file)
{
    int i;
    for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u\n",debugProc[i].inode_id,(unsigned int)inode->i_ino);
        if(debugProc[i].inode_id==(unsigned int)inode->i_ino)
        {
			return(single_open(file, debugProc[i].get, NULL));
        }
    }
    return -1;       
}


static ssize_t common_debug_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    int i;
    for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_write inode_id=%u i_ino=%u\n",debugProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);
        if(debugProc[i].inode_id==(unsigned int)file->f_dentry->d_inode->i_ino)
        {
			return debugProc[i].set(file,userbuf,count,off);
        }
    }
    return -1;      
}



void rg_proc_init(void)
{

	struct proc_dir_entry *p;
	struct proc_dir_entry *proc_root=NULL;
	int32 i;

	proc_root = proc_mkdir("dump", NULL);	


	for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
	{
		if(debugProc[i].get==NULL)
			debugProc[i].proc_fops.open=NULL_debug_single_open;
		else
			debugProc[i].proc_fops.open=common_debug_single_open;
		
		if(debugProc[i].set==NULL)
			debugProc[i].proc_fops.write=NULL;
		else
			debugProc[i].proc_fops.write=common_debug_single_write;
		
		debugProc[i].proc_fops.read=seq_read;
		debugProc[i].proc_fops.llseek=seq_lseek;
		debugProc[i].proc_fops.release=single_release;

		
		p = proc_create_data(debugProc[i].name, S_IRUGO, proc_root, &(debugProc[i].proc_fops),NULL);
		if(!p){
			printk("create proc dump/%s failed!\n",debugProc[i].name);
		}
		debugProc[i].inode_id = p->low_ino;
	}

		
	return ;

}

int rtk_rg_isakmp_get(struct seq_file *s, void *v)
{
	int len=0;

	PROC_PRINTF("ipsec_passthru = %d\n",rg_db.systemGlobal.ipsec_passthru);

	return len;
}
int rtk_rg_isakmp_set(struct file *file, const char *buff, unsigned long len, void *data)
{
	char	*tmpbuf;	
	char	*strptr=NULL;

	tmpbuf=rtk_rg_malloc(512);
	if(tmpbuf==NULL) return 0;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))	
	{		
		tmpbuf[len] = '\0';				
		strptr=tmpbuf;
	}
	
	if(strncmp(strptr, "1",1) == 0)
	{
		rg_db.systemGlobal.ipsec_passthru = PASS_ENABLE;
	}
	else if(strncmp(strptr, "0",1) == 0)
	{
		rg_db.systemGlobal.ipsec_passthru = PASS_OLD;
	}
	else if(strncmp(strptr, "2",1) == 0)
	{
		rg_db.systemGlobal.ipsec_passthru = PASS_DISABLE;
	}
	else	//outbound
		rtk_rg_isakmp_get(NULL,NULL);

	rtk_rg_free(tmpbuf);
	return 1;
}

static int ipsec_passthru_single_open(struct inode *inode, struct file *file)
{
	return(single_open(file, rtk_rg_isakmp_get, NULL));
}

static ssize_t ipsec_passthru_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
	return rtk_rg_isakmp_set(file, userbuf, count, off);
}

static struct file_operations ipsec_passthru_proc_fops = {
	.open			= ipsec_passthru_single_open,
	.write			= ipsec_passthru_single_write,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};


void rtk_rg_isakmp_system_proc_init(void){
	struct proc_dir_entry *p;//,*start;

	if(rg_kernel.proc_rg==NULL)
		rg_kernel.proc_rg = proc_mkdir("rg", NULL);		
	
	p = proc_create_data("ipsec_passthru", 0644, rg_kernel.proc_rg,&ipsec_passthru_proc_fops,NULL);
	rg_db.systemGlobal.ipsec_passthru = PASS_OLD;	
	if (!p){
		DEBUG("create proc rg/ipsec_passthru failed!\n");
	}
}

int _rtk_rg_trace_filter_compare(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	int32 i,j;
	int32 show=0;
	int32 da_hit=0,sa_hit=0;
#if 0	
	rtk_rg_pktHdr_t *pPktHdr;	
	pPktHdr=(rtk_rg_pktHdr_t *)(*(u32 *)(skb->data+skb->len));
	if((((u32)pPktHdr)&0xf0000000)!=0x80000000)
	{
		if((u32)pPktHdr!=0) //skip ARP,NB which send by fwdEngine
		{
			printk("FIXME: PKTHDR isn't put at end of skb.\n");
		}			
		return 1;
	}
#endif

	if(skb==NULL || pPktHdr==NULL)
		return 1;

	for(i=0;i<TRACFILTER_MAX;i++)
	{

		if(!(rg_kernel.traceFilterRuleMask & (1<<i)))
			continue;
		
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SPA)
		{
			if(rg_kernel.trace_filter[i].spa!=pPktHdr->ingressPort) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DA)
		{
			for(j=0;j<6;j++)
			{
				if((rg_kernel.trace_filter[i].dmac.octet[j]&rg_kernel.trace_filter[i].dmac_mask.octet[j])!=
					(skb->data[j]&rg_kernel.trace_filter[i].dmac_mask.octet[j])) 
						da_hit=1;
			}
			if(da_hit)
				continue;	
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SA)
		{
			for(j=0;j<6;j++)
			{
				if((rg_kernel.trace_filter[i].smac.octet[j]&rg_kernel.trace_filter[i].smac_mask.octet[j])!=
					(skb->data[j+6]&rg_kernel.trace_filter[i].smac_mask.octet[j])) 
						sa_hit=1;
			}
			if(sa_hit)
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_ETH)
		{
			if(rg_kernel.trace_filter[i].ethertype!=pPktHdr->etherType) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SIP)
		{
			if(rg_kernel.trace_filter[i].sip!=pPktHdr->ipv4Sip) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DIP)
		{
			if(rg_kernel.trace_filter[i].dip!=pPktHdr->ipv4Dip) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_IP)
		{
			if((rg_kernel.trace_filter[i].ip!=pPktHdr->ipv4Dip)&&(rg_kernel.trace_filter[i].ip!=pPktHdr->ipv4Sip)) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_L4PROTO)
		{
			if((pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))==0) 
				continue;
			else
				if(rg_kernel.trace_filter[i].l4proto!=pPktHdr->ipProtocol) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SPORT)
		{
			if(rg_kernel.trace_filter[i].sport!=pPktHdr->sport) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DPORT)
		{
			if(rg_kernel.trace_filter[i].dport!=pPktHdr->dport) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_REASON)
		{
			if(rg_kernel.trace_filter[i].reason!=pPktHdr->pRxDesc->opts3.bit.reason) continue;
		}

 		show=1;
		break;
	}


#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
/*show we care packet ( For Nic ) Boyce 2015-05-15*/
{
	extern int DumpSwNicTxRx_debug;
	if(show)
		DumpSwNicTxRx_debug=1;
	else
		DumpSwNicTxRx_debug=0;
}
#endif


	
	return show;
}




void dump_packet(u8 *pkt,u32 size,char *memo)
{
	int off;
	u8 protocol=0;
	int i;
	int pppoeif=0;
	for(i=0; i<78; i++)
		rtlglue_printf("=");
	rtlglue_printf("\n");

	if(size==0)
	{
		rtlglue_printf("%s\npacket_length=0\n",memo);
		return;
	}

	memDump(pkt,size,memo);
	rtlglue_printf("\n" COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[0],pkt[1],pkt[2],pkt[3],pkt[4],pkt[5]
		,pkt[6],pkt[7],pkt[8],pkt[9],pkt[10],pkt[11]);
	off=12;
	if((pkt[off]==0x88)&&(pkt[off+1]==0x99))
	{

		if(((pkt[off+8]==0x88)&&(pkt[off+9]==0xa8))||((pkt[off+8]==0x81)&&(pkt[off+9]==0x00))||((pkt[off+8]==0x88)&&((pkt[off+9]==0x63)||(pkt[off+9]==0x64)))||
		((pkt[off+8]==0x86)&&(pkt[off+9]==0xdd))||((pkt[off]==0x08)&&(pkt[off+1]==0x00)))
		{
			//TO CPU
			rtlglue_printf("CPU:[" COLOR_Y "Protocol" COLOR_NM "=%d][" COLOR_Y "Res" COLOR_NM "=0x%x][" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "TTL_1" COLOR_NM "=0x%x][" COLOR_Y "L3R" COLOR_NM "=%d][" COLOR_Y "ORG" COLOR_NM "=%d][" COLOR_Y "SPA" COLOR_NM "=%d][" COLOR_Y "EPMSK" COLOR_NM "=0x%x]\n"
				,pkt[off+2],pkt[off+3],pkt[off+4]>>5,pkt[off+4]&0x1f
				,pkt[off+5]>>7,(pkt[off+5]>>6)&1,pkt[off+5]&7,pkt[off+7]&0x3f);
			off+=8;
		}
		else
		{
			//FROM CPU
			rtlglue_printf("CPU:[" COLOR_Y "Proto" COLOR_NM "=%d][" COLOR_Y "L3CS" COLOR_NM "=%d][" COLOR_Y "L4CS" COLOR_NM "=%d][" COLOR_Y "TxPortMask" COLOR_NM "=0x%x][" COLOR_Y "EFID_EN" COLOR_NM "=%d][" COLOR_Y "EFID" COLOR_NM "=%d][" COLOR_Y "Priority" COLOR_NM "=%d]\n"
				,pkt[off+2],(pkt[off+3]>>7)&1,(pkt[off+3]>>6)&1,pkt[off+3]&0x3f,pkt[off+4]>>5,(pkt[off+4]>>3)&3,pkt[off+4]&7);
			rtlglue_printf("    [" COLOR_Y "Keep" COLOR_NM "=%d][" COLOR_Y "VSEL" COLOR_NM "=%d][" COLOR_Y "DisLrn" COLOR_NM "=%d][" COLOR_Y "PSEL" COLOR_NM "=%d][" COLOR_Y "Rsv1" COLOR_NM "=%d][" COLOR_Y "Rsv0" COLOR_NM "=%d][" COLOR_Y "L34Keep" COLOR_NM "=%d][" COLOR_Y "QSEL" COLOR_NM "=%d]\n"
				,pkt[off+5]>>7,(pkt[off+5]>>6)&1,(pkt[off+5]>>5)&1,(pkt[off+5]>>4)&1,(pkt[off+5]>>3)&1,(pkt[off+5]>>2)&1,(pkt[off+5]>>1)&1,pkt[off+5]&1);
			rtlglue_printf("    [" COLOR_Y "ExtSPA" COLOR_NM "=%d][" COLOR_Y "PPPoEAct" COLOR_NM "=%d][" COLOR_Y "PPPoEIdx" COLOR_NM "=%d][" COLOR_Y "L2BR" COLOR_NM "=%d][" COLOR_Y "QID" COLOR_NM "=%d]\n"
				,(pkt[off+6]>>5)&7,(pkt[off+6]>>3)&3,pkt[off+6]&7,(pkt[off+7]>>7)&1,pkt[off+6]&0x7f);
			off+=12;
		}
	}

{
#ifdef CONFIG_RTL9602C_SERIES
	uint32 vs_tpid=0;
	uint32 vs_tpid2=0;
	rtk_enable_t vs_tpid2_en=0;
	i = (pkt[off]<<8) | (pkt[off+1]);
	assert_ok(rtk_svlan_tpidEntry_get(0, &vs_tpid));
	assert_ok(rtk_svlan_tpidEntry_get(1, &vs_tpid2));
	assert_ok(rtk_svlan_tpidEnable_get(1, &vs_tpid2_en));
	if ((i==vs_tpid) || ((vs_tpid2_en)&&(i==vs_tpid2)))
#else
	if ((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
#endif
	{
		rtlglue_printf("SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
		off+=4;
	}
}
	if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
	{
		rtlglue_printf("CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
		off+=4;
	}

	if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
	{
		rtlglue_printf("PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x]\n",
			pkt[off+3],((u32)pkt[off+4]<<8)|pkt[off+5]);
		off+=8;
		pppoeif=1;
	}

	if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)))		//IPv6 or IPv6 with PPPoE
	{
		rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%02x%02x%x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
			,pkt[off+2]>>4, (pkt[off+2]&0xf)+(pkt[off+3]>>4), (pkt[off+3]&0xf)+(pkt[off+4]>>4), (pkt[off+4]&0xf)+(pkt[off+5]>>4), (pkt[off+5]&0xf), (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
		rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
			,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
			,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
		rtlglue_printf("     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
			,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
			,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

		protocol=pkt[off+8];
		if(protocol==0)	//hop-by-hop
		{
			rtlglue_printf("Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
				,pkt[off+42], pkt[off+43]);
			rtlglue_printf("          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
				,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
			for(i=0; i<pkt[off+43]; i++)
			{
				rtlglue_printf("         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
					,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
					,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
			}

			protocol=pkt[off+42];
			off+=(50+pkt[off+43]*8);
		}
		else
			off+=42;
	}

	if(((pkt[off]==0x08)&&(pkt[off+1]==0x00))||((pkt[off]==0x00)&&(pkt[off+1]==0x21)))
	{
		rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=%d(DSCP=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
			,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
			,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
		rtlglue_printf("     [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
			,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
		rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
			,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

		protocol=pkt[off+11];
		off+=(pkt[off+2]&0xf)*4+2;
	}

	if(protocol==0x6) //TCP
	{
		rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
			,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
			,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
		rtlglue_printf("    [" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
			,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
			,(pkt[off+14]<<8)|pkt[off+15]);
		rtlglue_printf("    [" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
	}
	else if(protocol==0x11) //UDP
	{
		rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
			,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));

	}
}

void _rtk_rg_error_id_mapping(int id, char *mappingName)
{
	switch(id)
	{
		case RT_ERR_RG_ADD_ARP_MAC_FAILED: strcpy(mappingName,"RT_ERR_RG_ADD_ARP_MAC_FAILED"); break;
		case RT_ERR_RG_ARP_NOT_FOUND: strcpy(mappingName,"RT_ERR_RG_ARP_NOT_FOUND"); break;
		case RT_ERR_RG_CHIP_NOT_SUPPORT: strcpy(mappingName,"RT_ERR_RG_CHIP_NOT_SUPPORT"); break;
		case RT_ERR_RG_CPU_TAG_DIFF_BRIDGE_WAN: strcpy(mappingName,"RT_ERR_RG_CPU_TAG_DIFF_BRIDGE_WAN"); break;
		case RT_ERR_RG_CREATE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_ERR_RG_CREATE_GATEWAY_LUT_FAIL"); break;
		case RT_ERR_RG_DELETE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_ERR_RG_DELETE_GATEWAY_LUT_FAIL"); break;
		case RT_ERR_RG_INTF_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_INTF_SET_FAIL"); break;
		case RT_ERR_RG_INVALID_PARAM: strcpy(mappingName,"RT_ERR_RG_INVALID_PARAM"); break;
		case RT_ERR_RG_NXP_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_NXP_SET_FAIL"); break;
		case RT_ERR_RG_PPPOE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_PPPOE_SET_FAIL"); break;
		case RT_ERR_RG_PORT_BIND_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_PORT_BIND_SET_FAIL"); break;
		case RT_ERR_RG_ROUTE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_ROUTE_SET_FAIL"); break;
		case RT_ERR_RG_WANTYPE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_WANTYPE_SET_FAIL"); break;
		case RT_ERR_RG_VLAN_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_VLAN_SET_FAIL"); break;
		case RT_ERR_RG_VLAN_PRI_CONFLICT_WIFI: strcpy(mappingName,"RT_ERR_RG_VLAN_PRI_CONFLICT_WIFI"); break;
		default: strcpy(mappingName,"Please refer to rtk_rg_struct.h!"); break;
	}
}




extern char StringErrName[];






rtk_rg_err_code_t return_err_mapping(int err_id, char *err_name,int line)
{

	if(err_id!=RT_ERR_RG_OK) 
	{			
		if(memcmp(err_name,"RT_ERR_RG_",10)==0)
		{
			strcpy(StringErrName,err_name);
		}
		else
		{
			_rtk_rg_error_id_mapping(err_id,StringErrName);
		}
		WARNING("Return Error (0x%x:%s) at line:%d",err_id,StringErrName,line);
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN)
		{			
			_rtk_rg_dump_stack();			
		}
	}
	return err_id;
}

#define MAX_PROC_PRINT_SIZE	1024
char proc_print_buf[MAX_PROC_PRINT_SIZE];
char *proc_printf(struct seq_file *s, char *fmt, ...)
{
    int n;
    int size = MAX_PROC_PRINT_SIZE;     /* Guess we need no more than 512 bytes */
    va_list ap;

    while (1) {
        va_start(ap, fmt);
        n = vsnprintf(proc_print_buf, size, fmt, ap);

		if((s==NULL)||(rg_db.systemGlobal.proc_to_pipe==0)) 
			{printk("%s",proc_print_buf);} 
		else 
			{seq_printf(s,"%s",proc_print_buf);}
			
        va_end(ap);

		if (n < 0)
		    return NULL;

		if (n < size)
		    return proc_print_buf;

		size = n + 1;

    }
	return NULL;
}



//#define PROC_PRINTF( comment ,arg...)	do { if((s==NULL)||(rg_db.systemGlobal.proc_to_pipe==0)) {printk(comment,##arg);} else {seq_printf(s,comment,##arg);} }while(0)
#endif 

