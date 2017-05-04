#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <net/route.h>
#include <net/xfrm.h>
#include <net/ip.h>
#include <linux/icmp.h>
#include <linux/signal.h>
#include <linux/syscalls.h>
#include "dos.h"

#define PROCFS_NAME         "DoS"
#define MODULE_NAME         "Realtek SD2-FastPath"
#define MODULE_VERSION      "v1.00beta_2.4.26-uc0"

#define __SWAP
#define __SWAP_DATA

#define SUCCESS 0
#define FAILED 1
#define TCP_FIN 1
#define TCP_SYN 2
#define HashSize 256
#define TableSize 1024
#define HighSensitivity 10
#define LowSensitivity 200
#define SmartHighThres 4000
#define SmartLowThres  500
#define DOS_FLAG_SIZE 80

struct s_dos_pkt {
	char use;
	int syn_cnt;
	int fin_cnt;
	int udp_cnt;
	int icmp_cnt;
	int  scan_cnt;
	unsigned int ip;
	unsigned short id;
	unsigned short offset;
	unsigned short dest;
};

static unsigned int LogFlag,ConnectedIp[HashSize] __SWAP_DATA;
static struct s_dos_pkt dos_pkt[TableSize] __SWAP_DATA, *cur_p_pkt __SWAP_DATA;
static struct timer_list dos_timer;
static char _tcpDosScanBitmap[64],op_mode;
static char dos_flag[DOS_FLAG_SIZE];
struct proc_dir_entry *res5=NULL;
struct proc_dir_entry *doslog=NULL;
unsigned int dos_log=0;
struct proc_dir_entry *dos_syslog=NULL;
static unsigned int boa_pid;
static char dos_info[1024];

static int whole_syn_threshold;
static int whole_fin_threshold;
static int whole_udp_threshold;
static int whole_icmp_threshold;
static int per_syn_threshold;
static int per_fin_threshold;
static int per_udp_threshold;
static int per_icmp_threshold;
static int block=0,block_time=0,block_count=0;

static unsigned int item=0;
unsigned int dos_lan_addr,dos_lan_mask;
unsigned int dos_attack_saddr[14],dos_attack_daddr[14];

enum {
	EnableDosSet=0x1,
	WholeSynFloodSet=0x2,
	WholeFinFloodSet=0x4,
	WholeUdpFloodSet=0x8,
	WholeIcmpFloodSet=0x10,
	PerSynFloodSet=0x20,
	PerFinFloodSet=0x40,
	PerUdpFloodSet=0x80,
	PerIcmpFloodSet=0x100,
	TcpUdpPortScanSet=0x200,
	IcmpSmurfSet=0x400,
	IpLandSet=0x800,
	IpSpoofSet=0x1000,
	TearDropSet=0x2000,
	PingOfDeathSet=0x4000,
	TcpScanSet=0x8000,
	TcpSynWithDataSet=0x10000,
	UdpBombSet=0x20000,
	UdpEchoChargenSet=0x40000,
	IpBlockSet=0x400000,
	SensitivitySet=0x800000

};
enum {
	PerSynFlood=1,
	PerFinFlood,
	PerUdpFlood,
	PerIcmpFlood,
	TcpUdpPortScan,
	IcmpSmurf,
	IpLand,
	IpSpoof,
	TearDrop,
	PingOfDeath,
	TcpScan,
	TcpSynWithData,
	UdpBomb,
	UdpEchoChargen
};
//int dos(struct sk_buff *skb);
static void dos_timer_fn(unsigned long arg);
static void dos_pkt_init(void);
static void dos_whole_flood(void);
static int dos_pkt_syn_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph);
static int dos_pkt_fin_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph);
static int dos_pkt_udp_flood(u_int32_t item,struct iphdr *iph);
static int dos_pkt_icmp_flood(u_int32_t item,struct iphdr *iph);
static int dos_pkt_locate(struct iphdr *iph);
static int _IpSpoof(struct iphdr *iph);
static int _IcmpSmurf(struct iphdr *iph);
static int _IpLand(struct iphdr *iph);
static int _UdpBomb(struct iphdr *iph, struct udphdr *udph);
static int _TcpSynWithData(struct iphdr *iph,struct tcphdr *tcph);
static int _PingOfDeath(struct iphdr *iph);
static int _UdpEchoChargen(struct iphdr *iph, struct udphdr *udph);
static int _TcpScan(struct iphdr *iph,struct tcphdr *tcph);
static int _TearDrop(struct iphdr *iph);
static int _TcpUdpPortScan(struct iphdr *iph, struct tcphdr *tcph,struct udphdr *udph);
static void ShowLog(u_int32_t flag);

static void dos_timer_fn(unsigned long arg)
{
	if(item)
	{
		if(block_count >= block_time)
		{
			block=0;
			block_count=0;
		}
		if(block == 1)
			block_count++;

 		dos_whole_flood();
		ShowLog(LogFlag);
		dos_pkt_init();
	}
	mod_timer(&dos_timer, jiffies + 100);
}

static int __SWAP dos_pkt_locate(struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	short idx=0;

	idx=iph->saddr % TableSize;
	p_pkt=&dos_pkt[idx];
	if(iph->saddr == ConnectedIp[iph->saddr % HashSize] && p_pkt->ip==iph->saddr)
	{
		p_pkt->use=0;
		return 0;
	}
	if(p_pkt->ip!=iph->saddr && p_pkt->use ==0)
	{
		p_pkt->ip=iph->saddr;
		p_pkt->use=1;
 		cur_p_pkt=p_pkt;
		return 1;
	}
	else if(p_pkt->ip==iph->saddr && p_pkt->use ==1)
	{
 		cur_p_pkt=p_pkt;
		return 1;
	}
	else
		return 0;
}

static void __SWAP dos_pkt_init()
{
	struct s_dos_pkt *p_pkt;
	int idx;
	p_pkt=&dos_pkt[0];
	for(idx=0; idx< TableSize;idx++)
	{
		p_pkt->use=0;
		p_pkt->ip=0;
		p_pkt->syn_cnt=0;
		p_pkt->fin_cnt=0;
		p_pkt->udp_cnt=0;
		p_pkt->icmp_cnt=0;
		p_pkt->scan_cnt=0;
		p_pkt++;
	}
}

static void __SWAP dos_whole_flood()
{
	struct s_dos_pkt *p_pkt;
	int whole_syn_pkt=0;
	int whole_fin_pkt=0;
	int whole_udp_pkt=0;
	int whole_icmp_pkt=0;
	int idx;
	if(item & ( WholeSynFloodSet | WholeFinFloodSet | WholeUdpFloodSet | WholeIcmpFloodSet))
	{
		for(idx=0,p_pkt=&dos_pkt[0]; idx< TableSize;idx++,p_pkt++)
		{
			if(p_pkt->use ==1 && (item &  WholeSynFloodSet)==WholeSynFloodSet && p_pkt->syn_cnt >0)
				whole_syn_pkt+=p_pkt->syn_cnt;
			if(p_pkt->use ==1 && (item &  WholeFinFloodSet)==WholeFinFloodSet && p_pkt->fin_cnt >0)
				whole_fin_pkt+=p_pkt->fin_cnt;
			if(p_pkt->use ==1 && (item &  WholeUdpFloodSet)==WholeUdpFloodSet && p_pkt->udp_cnt >0)
				whole_udp_pkt+=p_pkt->udp_cnt;
			if(p_pkt->use ==1 && (item &  WholeIcmpFloodSet)==WholeIcmpFloodSet && p_pkt->icmp_cnt >0)
				whole_icmp_pkt+=p_pkt->icmp_cnt;
		}

		if(whole_syn_pkt > whole_syn_threshold && (item & WholeSynFloodSet)==WholeSynFloodSet && whole_syn_threshold > 0)
			LogFlag |=WholeSynFloodSet;
		if(whole_fin_pkt > whole_fin_threshold && (item & WholeFinFloodSet)==WholeFinFloodSet && whole_fin_threshold > 0)
			LogFlag |=WholeFinFloodSet;
		if(whole_udp_pkt > whole_udp_threshold && (item & WholeUdpFloodSet)==WholeUdpFloodSet && whole_udp_threshold > 0)
			LogFlag |=WholeUdpFloodSet;
		if(whole_icmp_pkt > whole_icmp_threshold && (item & WholeIcmpFloodSet)==WholeIcmpFloodSet && whole_icmp_threshold >0)
			LogFlag |=WholeIcmpFloodSet;
	}
}

static int __SWAP dos_pkt_syn_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph)
{
	unsigned char tflag;
	struct s_dos_pkt *p_pkt;
	
	p_pkt=cur_p_pkt;
	tflag= *(unsigned char *)((unsigned int)tcph + 13);

	if(iph->protocol==IPPROTO_TCP && (tflag & 0x3f)==TCP_SYN)
	{
		if(block==1 && dos_attack_saddr[PerSynFlood]==iph->saddr)
			return FAILED;
		(p_pkt->syn_cnt)++;
		if(p_pkt->syn_cnt > per_syn_threshold && (item & PerSynFloodSet)==PerSynFloodSet && per_syn_threshold > 0)
		{
		 	dos_attack_saddr[PerSynFlood]=iph->saddr;
			dos_attack_daddr[PerSynFlood]=iph->daddr;
			block=1;
			LogFlag |=PerSynFloodSet;
		}
	}
	return SUCCESS;

}

static int __SWAP dos_pkt_fin_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph)
{
	unsigned char tflag;
	struct s_dos_pkt *p_pkt;
	
	p_pkt=cur_p_pkt;
	tflag = *(unsigned char *)((unsigned int)tcph + 13);

	if(iph->protocol==IPPROTO_TCP && (tflag & 0x3f)==TCP_FIN )
	{
		if(block==1 && dos_attack_saddr[PerFinFlood]==iph->saddr)
			return FAILED;
		(p_pkt->fin_cnt)++;
		if(p_pkt->fin_cnt > per_fin_threshold && (item & PerFinFloodSet)==PerFinFloodSet && per_fin_threshold > 0)
		{
		 	dos_attack_saddr[PerFinFlood]=iph->saddr;
			dos_attack_daddr[PerFinFlood]=iph->daddr;
			block=1;
			LogFlag |=PerFinFloodSet;
		}
	}
	return SUCCESS;
}

static int __SWAP dos_pkt_udp_flood(u_int32_t item,struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	p_pkt=cur_p_pkt;
	
	if(iph->protocol==IPPROTO_UDP)
	{
		if(block==1 && dos_attack_saddr[PerUdpFlood]==iph->saddr)
			return FAILED;
		(p_pkt->udp_cnt)++;
		if(p_pkt->udp_cnt > per_udp_threshold && (item & PerUdpFloodSet)==PerUdpFloodSet && per_udp_threshold > 0)
		{
		 	dos_attack_saddr[PerUdpFlood]=iph->saddr;
			dos_attack_daddr[PerUdpFlood]=iph->daddr;
			block=1;
			LogFlag |=PerUdpFloodSet;
		}
	}
	return SUCCESS;
}

static int __SWAP dos_pkt_icmp_flood(u_int32_t item,struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	p_pkt=cur_p_pkt;
	
	if(iph->protocol==IPPROTO_ICMP)
	{
		if(block==1 && dos_attack_saddr[PerIcmpFlood]==iph->saddr)
			return FAILED;
		(p_pkt->icmp_cnt)++;
		if(p_pkt->icmp_cnt > per_icmp_threshold && (item & PerIcmpFloodSet)==PerIcmpFloodSet && per_icmp_threshold >0)
		{
		 	dos_attack_saddr[PerIcmpFlood]=iph->saddr;
			dos_attack_daddr[PerIcmpFlood]=iph->daddr;
			block=1;
			LogFlag |=PerIcmpFloodSet;
		}
	}
	return SUCCESS;
}

static int __SWAP _IpSpoof(struct iphdr *iph)
{
	if((iph->saddr & dos_lan_mask)==(dos_lan_addr & dos_lan_mask) && iph->protocol!=IPPROTO_ICMP)
	{
		LogFlag |=IpSpoofSet;
		dos_attack_saddr[IpSpoof]=iph->saddr;
		dos_attack_daddr[IpSpoof]=iph->daddr;
		return FAILED;
	}
	return SUCCESS;
}

static int __SWAP _IcmpSmurf(struct iphdr *iph)
{
	struct icmphdr *icmph;
	
	icmph=(void *) iph + iph->ihl*4;
	if(iph->protocol==IPPROTO_ICMP && icmph->type == ICMP_ECHO)
	{
		if((iph->saddr & dos_lan_mask)==(dos_lan_addr & dos_lan_mask))
		{
	   	LogFlag |=IcmpSmurfSet;
		dos_attack_saddr[IcmpSmurf]=iph->saddr;
		dos_attack_daddr[IcmpSmurf]=iph->daddr;
		return FAILED;
		}
	}
	return SUCCESS;

}
static int __SWAP _IpLand(struct iphdr *iph)
{
	if(iph->saddr==iph->daddr)
	{
		LogFlag |=IpLandSet;
		dos_attack_saddr[IpLand]=iph->saddr;
		dos_attack_daddr[IpLand]=iph->daddr;
		return FAILED;
	}

	return SUCCESS;
}
static int __SWAP _UdpBomb(struct iphdr *iph, struct udphdr *udph)
{
	int ipPayLoadLength;

	if(iph->protocol==IPPROTO_UDP)
	{
        if (!(iph->frag_off & (IP_OFFSET|IP_MF)))
		{
	       	ipPayLoadLength = ntohs(iph->tot_len) - ((iph->ihl) << 2);
	    	if (ipPayLoadLength > ntohs(udph->len))
			{
				LogFlag |=UdpBombSet;
				dos_attack_saddr[UdpBomb]=iph->saddr;
				dos_attack_daddr[UdpBomb]=iph->daddr;
				return FAILED;
			}
		}
	}
	return SUCCESS;
}

static int __SWAP _TcpSynWithData(struct iphdr *iph,struct tcphdr *tcph)
{
	unsigned char tflag;
	
	tflag = *(unsigned char *)((unsigned int)tcph + 13);
	if(iph->protocol==IPPROTO_TCP && (tflag & 0x3f)== TCP_SYN)
	{
		unsigned long datalen= ntohs(iph->tot_len)-((iph->ihl)<<2)- (tcph->doff<<2);
		if(datalen>0)
		{
			LogFlag |= TcpSynWithDataSet;
			dos_attack_saddr[TcpSynWithData]=iph->saddr;
			dos_attack_daddr[TcpSynWithData]=iph->daddr;
			return FAILED;
		}
		if(htons(iph->frag_off) & IP_MF)
		{
			LogFlag |=TcpSynWithDataSet;
			dos_attack_saddr[TcpSynWithData]=iph->saddr;
			dos_attack_daddr[TcpSynWithData]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}

static int __SWAP _PingOfDeath(struct iphdr *iph)
{
	unsigned short iph_off = ntohs(iph->frag_off);
	unsigned long  val;

	if((iph_off & IP_MF) == 0 && (iph_off & IP_OFFSET))
	{
        iph_off &= IP_OFFSET;
        val = (iph_off << 3) + ntohs(iph->tot_len) -((iph->ihl) << 2);
        if(val > 65535)
		{
			LogFlag |=PingOfDeathSet;
			dos_attack_saddr[PingOfDeath]=iph->saddr;
			dos_attack_daddr[PingOfDeath]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}
static int __SWAP _UdpEchoChargen(struct iphdr *iph, struct udphdr *udph)
{
	if(iph->protocol==IPPROTO_UDP)
	{
        if((udph->dest==htons(7)||udph->dest==htons(17)||udph->dest==htons(19)) || (udph->source==htons(7)||udph->source==htons(17)||udph->source==htons(19)))
		{
			LogFlag |=UdpEchoChargenSet;
			dos_attack_saddr[UdpEchoChargen]=iph->saddr;
			dos_attack_daddr[UdpEchoChargen]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}
static int __SWAP _TcpScan(struct iphdr *iph,struct tcphdr *tcph)
{
	unsigned char tflag;

	tflag = *(unsigned char *)((unsigned int)tcph + 13);
	if(iph->protocol==IPPROTO_TCP)
	{
		if(_tcpDosScanBitmap[tflag & 0x3f])
		{
			LogFlag |=TcpScanSet;
			dos_attack_saddr[TcpScan]=iph->saddr;
			dos_attack_daddr[TcpScan]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}
static int __SWAP _TearDrop(struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	if(dos_pkt_locate(iph))
		p_pkt=cur_p_pkt;
	else
		return SUCCESS;
	if(ntohs(iph->id)!=p_pkt->id && ntohs(p_pkt->id) !=0)
		return SUCCESS;

	if((ntohs(iph->frag_off)) & (IP_MF | IP_OFFSET))
	{
        if(((ntohs(iph->frag_off) & IP_OFFSET) << 3) >= p_pkt->offset)
		{
			if(!(ntohs(iph->frag_off) & IP_MF))
			{
				p_pkt->id=0;
				p_pkt->offset=0;
			}
			else
			{
				p_pkt->id=ntohs(iph->id);
				p_pkt->offset=p_pkt->offset + ntohs(iph->tot_len)-((iph->ihl) << 2);
			}
		}
		else
		{
			LogFlag |=TearDropSet;
			dos_attack_saddr[TearDrop]=iph->saddr;
			dos_attack_daddr[TearDrop]=iph->daddr;
			return FAILED;
		}

	}
	return SUCCESS;
}
static int __SWAP _TcpUdpPortScan(struct iphdr *iph, struct tcphdr *tcph,struct udphdr *udph)
{
	struct s_dos_pkt *p_pkt;
	p_pkt=cur_p_pkt;

	if(iph->protocol==IPPROTO_TCP)
	{
		if(p_pkt->dest != 0 && p_pkt->dest!=tcph->dest)
			(p_pkt->scan_cnt)++;
		if(p_pkt->dest == 0)
			p_pkt->dest=tcph->dest;
	}
	if(iph->protocol==IPPROTO_UDP)
	{
		if(p_pkt->dest != 0 && p_pkt->dest!=udph->dest)
			(p_pkt->scan_cnt)++;
		if(p_pkt->dest == 0)
			p_pkt->dest=udph->dest;
	}

	if((item & SensitivitySet)==SensitivitySet && p_pkt->scan_cnt > HighSensitivity)
	{
		LogFlag |=TcpUdpPortScanSet;
		dos_attack_saddr[TcpUdpPortScan]=iph->saddr;
		dos_attack_daddr[TcpUdpPortScan]=iph->daddr;
	}
	if((item & SensitivitySet)!=SensitivitySet && p_pkt->scan_cnt > LowSensitivity)
	{
		LogFlag |=TcpUdpPortScanSet;
		dos_attack_saddr[TcpUdpPortScan]=iph->saddr;
		dos_attack_daddr[TcpUdpPortScan]=iph->daddr;
	}

	return SUCCESS;
}

static void __SWAP ShowLog(unsigned int flag)
{
	int floodSet = 1;
	
	if(flag & WholeSynFloodSet) {
	  	sprintf(dos_info, "DoS: Whole System SYN Flood Attack\n");
		flag &= (~WholeSynFloodSet);
	}
	else if(flag & WholeFinFloodSet) {
		sprintf(dos_info, "DoS: Whole System FIN Flood Attack\n");
		flag &= (~WholeFinFloodSet);
	}
	else if(flag & WholeUdpFloodSet) {
		sprintf(dos_info, "DoS: Whole System UDP Flood Attack\n");
		flag &= (~WholeUdpFloodSet);
	}
	else if(flag & WholeIcmpFloodSet) {
		sprintf(dos_info, "DoS: Whole System ICMP Flood Attack\n");
		flag &= (~WholeIcmpFloodSet);
	}
	else if(flag & PerSynFloodSet) {
		sprintf(dos_info, "DoS: Per-source SYN Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[PerSynFlood]),NIPQUAD(dos_attack_daddr[PerSynFlood]));
		flag &= (~PerSynFloodSet);
	}
	else if(flag & PerFinFloodSet) {
		sprintf(dos_info, "DoS: Per-source FIN Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[PerFinFlood]),NIPQUAD(dos_attack_daddr[PerFinFlood]));
		flag &= (~PerFinFloodSet);
	}
	else if(flag & PerUdpFloodSet) {
		sprintf(dos_info, "DoS: Per-source UDP Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[PerUdpFlood]),NIPQUAD(dos_attack_daddr[PerUdpFlood]));
		flag &= (~PerUdpFloodSet);
	}
	else if(flag & PerIcmpFloodSet) {
		sprintf(dos_info, "DoS: Per-source ICMP Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[PerIcmpFlood]),NIPQUAD(dos_attack_daddr[PerIcmpFlood]));
		flag &= (~PerIcmpFloodSet);
	}
	else if(flag & TcpUdpPortScanSet) {
		sprintf(dos_info, "DoS: Port Scan Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[TcpUdpPortScan]),NIPQUAD(dos_attack_daddr[TcpUdpPortScan]));
		flag &= (~TcpUdpPortScanSet);
	}
	else if(flag & TcpScanSet) {
		sprintf(dos_info, "DoS: Tcp Scan Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[TcpScan]),NIPQUAD(dos_attack_daddr[TcpScan]));
		flag &= (~TcpScanSet);
	}
	else if(flag & TcpSynWithDataSet) {
		sprintf(dos_info, "DoS: Tcp SYN With Data Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[TcpSynWithData]),NIPQUAD(dos_attack_daddr[TcpSynWithData]));
		flag &= (~TcpSynWithDataSet);
	}
	else if(flag & IpLandSet) {
		sprintf(dos_info, "DoS: IP Land Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[IpLand]),NIPQUAD(dos_attack_daddr[IpLand]));
		flag &= (~IpLandSet);
	}
	else if(flag & UdpEchoChargenSet) {
		printk("DoS: UdpEchoChargen Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[UdpEchoChargen]),NIPQUAD(dos_attack_daddr[UdpEchoChargen]));
		flag &= (~UdpEchoChargenSet);
	}
	else if(flag & UdpBombSet) {
		sprintf(dos_info, "DoS: UdpBomb Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[UdpBomb]),NIPQUAD(dos_attack_daddr[UdpBomb]));
		flag &= (~UdpBombSet);
	}
	else if(flag & PingOfDeathSet) {
		sprintf(dos_info, "DoS: PingOfDeath Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[PingOfDeath]),NIPQUAD(dos_attack_daddr[PingOfDeath]));
		flag &= (~PingOfDeathSet);
	}
	else if(flag & IcmpSmurfSet) {
		sprintf(dos_info, "DoS: IcmpSmurf Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[IcmpSmurf]),NIPQUAD(dos_attack_daddr[IcmpSmurf]));
		flag &= (~IcmpSmurfSet);
	}
	else if(flag & IpSpoofSet) {
		sprintf(dos_info, "DoS: IpSpoof Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[IpSpoof]),NIPQUAD(dos_attack_daddr[IpSpoof]));
		flag &= (~IpSpoofSet);
	}
	else if(flag & TearDropSet) {
		sprintf(dos_info, "DoS: TearDrop Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(dos_attack_saddr[TearDrop]),NIPQUAD(dos_attack_daddr[TearDrop]));
		flag &= (~TearDropSet);
	}
	else
		floodSet = 0;
	
	if (floodSet)
	{
		printk("%s", dos_info);
		if (boa_pid)
			sys_kill(boa_pid, SIGUSR1);
	}
	
	//for log shown on webpage
	dos_log |= LogFlag;
	LogFlag = flag;
}

int __SWAP  dos(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	struct udphdr *udph;

	//printk("dos enter, item=0x%x\n", item);
	if (!(item & EnableDosSet))
		return 0;

	iph = ip_hdr(skb);
	tcph = (struct tcphdr *)((unsigned int)iph + (iph->ihl<<2));
	udph = (struct udphdr *)((unsigned int)iph + (iph->ihl<<2));

	if(item & (WholeSynFloodSet | WholeFinFloodSet | WholeUdpFloodSet  | WholeIcmpFloodSet | PerSynFloodSet | PerFinFloodSet | PerUdpFloodSet | PerIcmpFloodSet | TcpUdpPortScanSet))
	{
		if(dos_pkt_locate(iph))
		{
			if(item & IpBlockSet) {
				if ((iph->protocol == 6) && (item & (WholeSynFloodSet | PerSynFloodSet)))
					if (dos_pkt_syn_flood(item,iph,tcph) == FAILED)
						return FAILED;
				if ((iph->protocol == 6) && (item & (WholeFinFloodSet | PerFinFloodSet)))
					if (dos_pkt_fin_flood(item,iph,tcph) == FAILED)
						return FAILED;
				if ((iph->protocol == 17) && (item & (WholeUdpFloodSet | PerUdpFloodSet)))
					if (dos_pkt_udp_flood(item,iph) == FAILED)
						return FAILED;
				if ((iph->protocol == 1) && (item & (WholeIcmpFloodSet | PerIcmpFloodSet)))
					if (dos_pkt_icmp_flood(item,iph) == FAILED)
						return FAILED;
				if (((iph->protocol == 6) || (iph->protocol == 17)) && (item & TcpUdpPortScanSet))
					if (_TcpUdpPortScan(iph,tcph,udph) == FAILED)
						return FAILED;
			}
		}
	}
	if ((iph->protocol == 6) && (item & TcpScanSet))
		if (_TcpScan(iph,tcph) == FAILED)
			return FAILED;
	if ((iph->protocol == 6) && (item & TcpSynWithDataSet))
		if (_TcpSynWithData(iph,tcph) == FAILED)
			return FAILED;
	if (item & IpLandSet)
		if (_IpLand(iph) == FAILED)
			return FAILED;
	if ((iph->protocol == 17) && (item & UdpEchoChargenSet))
		if (_UdpEchoChargen(iph, udph) == FAILED)
			return FAILED;
	if ((iph->protocol == 17) && (item & UdpBombSet))
		if (_UdpBomb(iph, udph) == FAILED)
			return FAILED;
	if (item & PingOfDeathSet)
		if (_PingOfDeath(iph) == FAILED)
			return FAILED;
	if ((iph->protocol == 1) && (item & IcmpSmurfSet))
		if (_IcmpSmurf(iph) == FAILED)
			return FAILED;
	if (item & IpSpoofSet)
		if (_IpSpoof(iph) == FAILED)
			return FAILED;
	if (item & TearDropSet)
		if (_TearDrop(iph) == FAILED)
			return FAILED;

	return SUCCESS;
}

static int dos_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%s\n", dos_flag);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int dos_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpbuf[80];
	char *tokptr, *strptr=tmpbuf;
	unsigned char idx=1;
	unsigned int val;
	unsigned long buffer_size;

	if (count < 2)
		return -EFAULT;
	
	if (count > DOS_FLAG_SIZE)
		buffer_size = DOS_FLAG_SIZE;
	else
		buffer_size = count;

	memset(dos_flag, 0, DOS_FLAG_SIZE);

	if (buffer && !copy_from_user(dos_flag, buffer, buffer_size)) {
		strncpy(tmpbuf,dos_flag,buffer_size);

		while ((tokptr = strsep(&strptr," ")) !=NULL)
		{
			val=simple_strtol(tokptr,NULL,0);
			switch(idx)
			{
			case 1:
				op_mode=val;
				//printk("op mode:%d\n", op_mode);
				break;
			case 2:
				val=simple_strtol(tokptr,NULL,16);
				dos_lan_addr=val;
				//test
				//printk("ip:%x\n", dos_lan_addr);
				break;
			case 3:
				val=simple_strtol(tokptr,NULL,16);
				dos_lan_mask=val;
				//test
				//printk("mask:%x\n", dos_lan_mask);
				break;
			case 4:
				item=val;
				//test
				printk("set dos:0x%x\n", item);
				break;
			case 5:
				whole_syn_threshold=val;
				//printk("whole_syn_threshold=%d\n", whole_syn_threshold);
				break;
			case 6:
				whole_fin_threshold=val;
				//printk("whole_fin_threshold=%d\n", whole_fin_threshold);
				break;
			case 7:
				whole_udp_threshold=val;
				//printk("whole_udp_threshold=%d\n", whole_udp_threshold);
				break;
			case 8:
				whole_icmp_threshold=val;
				//printk("whole_icmp_threshold=%d\n", whole_icmp_threshold);
				break;
			case 9:
				per_syn_threshold=val;
				//printk("per_syn_threshold=%d\n", per_syn_threshold);
				break;
			case 10:
				per_fin_threshold=val;
				//printk("per_fin_threshold=%d\n", per_fin_threshold);
				break;
			case 11:
				per_udp_threshold=val;
				//printk("per_udp_threshold=%d\n", per_udp_threshold);
				break;
			case 12:
				per_icmp_threshold=val;
				//printk("per_icmp_threshold=%d\n", per_icmp_threshold);
				break;
			case 13:
				block_time=val;
				//test
				//printk("block time:%d\n", block_time);
				break;
			default:
				//printk("unknown\n");
				break;
			}
			idx++;

		}
		return buffer_size;
	}
	return -EFAULT;
}

//for dos_log
static int doslog_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", dos_log);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int doslog_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	dos_log=0;
	return 0;
}

static int dos_syslog_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%s", dos_info);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int dos_syslog_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char proc_buffer[10];
	
	/* write data to the buffer */
	memset(proc_buffer, 0, sizeof(proc_buffer));
	if (buffer && !copy_from_user(proc_buffer, buffer, count) )
	{
		boa_pid = simple_strtol(proc_buffer, NULL, 0);
		printk("=====>>>>set boa_pid=%d\n", boa_pid);
	}

	return count;
}


EXPORT_SYMBOL(dos);

static int dos_init(void)
{
//ql- init dos
#ifdef CONFIG_DOS
	//create proc
	res5 = create_proc_entry("enable_dos", 0644, NULL);
	if (res5 == NULL) {
		remove_proc_entry("enable_dos", &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			"enable_dos");
		return -ENOMEM;
	}

	res5->read_proc  = (read_proc_t *)dos_read_proc;
	res5->write_proc  = (write_proc_t *)dos_write_proc;
	//res5->owner 	  = THIS_MODULE;
	res5->mode 	  = S_IFREG | S_IRUGO;
	res5->uid 	  = 0;
	res5->gid 	  = 0;
	res5->size 	  = 37;

	//create proc for log
	doslog = create_proc_entry("log_dos", 0644, NULL);
	if (doslog == NULL) {
		remove_proc_entry("log_dos", &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			"log_dos");
		return -ENOMEM;
	}

	doslog->read_proc  = (read_proc_t *)doslog_read_proc;
	doslog->write_proc  = (write_proc_t *)doslog_write_proc;
	//doslog->owner 	  = THIS_MODULE;
	doslog->mode 	  = S_IFREG | S_IRUGO;
	doslog->uid 	  = 0;
	doslog->gid 	  = 0;
	doslog->size 	  = 37;

	dos_syslog = create_proc_entry("dos_syslog", 0644, NULL);
	if (dos_syslog == NULL) {
		remove_proc_entry("dos_syslog", &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			"dos_syslog");
		return -ENOMEM;
	}
	dos_syslog->read_proc  = (read_proc_t *)dos_syslog_read_proc;
	dos_syslog->write_proc  = (write_proc_t *)dos_syslog_write_proc;

	init_timer(&dos_timer);
	dos_timer.expires  = jiffies + 100;
	dos_timer.data     = 0L;
	dos_timer.function = dos_timer_fn;
	mod_timer(&dos_timer, jiffies + 100);

	memset(&_tcpDosScanBitmap[0], 0, sizeof(_tcpDosScanBitmap));
	_tcpDosScanBitmap[0]=_tcpDosScanBitmap[3]=_tcpDosScanBitmap[8]=_tcpDosScanBitmap[9]= _tcpDosScanBitmap[32]=_tcpDosScanBitmap[33]=_tcpDosScanBitmap[40]=_tcpDosScanBitmap[41]= _tcpDosScanBitmap[58]=_tcpDosScanBitmap[63]=1;

    printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
#endif
    return 1;
}

static void dos_fini(void)
{
    printk("%s %s removed!\n", MODULE_NAME, MODULE_VERSION);
}

module_init(dos_init);
module_exit(dos_fini);
MODULE_LICENSE("GPL");

