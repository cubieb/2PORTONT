#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <net/tcp.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>


#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif


static struct timer_list ipsec_time;
static struct isakmp_data_s *_isakmpDb=NULL;
char ipsec_flag='1';


#ifdef IPSEC_ESP_HOOK
static void _prepend_to_hash(struct nf_conntrack_tuple_hash *tuplehash, u_int32_t spi)
{
	unsigned int hash;
	u_int16_t	*pu16 = (u_int16_t *)&spi;
	struct nf_conntrack_tuple *tuple;
	
	// It's the first reply, associate to this connection
	tuple = &tuplehash->tuple;
	hash = (ntohl(tuple->src.u3.ip + tuple->dst.u3.ip
	 + tuple->src.u.all + tuple->dst.u.all
	 + tuple->dst.protonum)	+ ntohs(tuple->src.u.all))
	 % nf_conntrack_htable_size;
	
//	LIST_DELETE(&ip_conntrack_hash[hash], tuplehash);
	//list_del(&tuplehash->list);	
	hlist_nulls_del_rcu(&tuplehash->hnnode);

	
	tuple->src.u.all = pu16[0];
	tuple->dst.u.all = pu16[1];
	// re-hash this tuple
	hash = (ntohl(tuple->src.u3.ip + tuple->dst.u3.ip
	 + tuple->src.u.all + tuple->dst.u.all
	 + tuple->dst.protonum)	+ ntohs(tuple->src.u.all))
	 % nf_conntrack_htable_size;
//	list_prepend(&ip_conntrack_hash[hash], tuplehash);
	//list_add(&tuplehash->list, &ip_conntrack_hash[hash]);	
	hlist_nulls_add_head_rcu(&tuplehash->hnnode, &init_net.ct.hash[hash]);	
}


static struct isakmp_data_s*
_findEspIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int32_t ispi){
	int i;
	struct isakmp_data_s *tp,*new_tp;
	tp = _isakmpDb;
	new_tp=NULL;
	
	for(i = 0 ; i < IPSEC_MaxSession ; i++)
	{
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&
		   tp->state == IPSEC_USED &&

		   //(tp->ispi & ispi) == tp->ispi )
		   tp->ispi == ispi )
		{	
			tp->idle_timer = 0;	   	

			//tp->ispi = ispi;
		   	DEBUGP("Found session #%d in\n", i);
			return tp;
		}
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&

		   tp->ispi == 0 &&
		   tp->state == IPSEC_USED)
		{

			DEBUGP("Refresh ESP session #%d on reply (new spi)\n", i);
			new_tp=tp;
	  	}
		tp++;
		
	}
	if(new_tp!=NULL)
	{
		// Here comes new spi
		new_tp->idle_timer = 0;	   	
		new_tp->ispi = ispi;

		if (new_tp->pctrack != 0)
		{
			_prepend_to_hash(&new_tp->pctrack->tuplehash[IP_CT_DIR_REPLY], ispi);
		}
		else
			DEBUGP("should not be here!\n");
		return new_tp;
	}
	
	return NULL;
}


static struct isakmp_data_s*
_findEspOut(u_int32_t local_ip,u_int32_t peer_ip, u_int32_t ospi){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < IPSEC_MaxSession ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&

		   //(tp->ospi  & ospi ) == tp->ospi &&
		   tp->ospi == ospi &&
		   tp->state == IPSEC_USED ){	
		   	tp->idle_timer = 0;
		   	tp->ospi = ospi;
		   	DEBUGP("Found session #%d out\n", i);
		   	return tp;
		}
		tp++;
	}
	
	return NULL;
}



static struct isakmp_data_s*
_addEsp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t ospi){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < IPSEC_MaxSession ; i++){
		if(tp->state == IPSEC_USED && tp->peer_ip == peer_ip
		   && tp->local_ip == local_ip){
			// Here comes new spi
			DEBUGP("New ESP session #%d out, spi=%x -> %x\n", i, tp->ospi, ospi);
			tp->idle_timer = 0;
			tp->ospi = ospi;
			tp->ispi = 0;
			return tp;
		}
		tp++;
	}
	return NULL;
}
#endif //IPSEC_ESP_HOOK



/*********************************************************************************
* Routine Name :  _findIsakmpIn
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_findIsakmpIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;

	u_int32_t *pu32;
	tp = _isakmpDb;
	
	for(i = 0 ; i < IPSEC_MaxSession ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&
		   tp->icookie == icookie &&
		   tp->state == IPSEC_USED){
			tp->idle_timer = 0;
			tp->icookie = icookie;

			DEBUGP("find IKE in i=%d, local_ip=%x, ", i, tp->local_ip);
			pu32 = (u_int32_t *)&icookie;
			DEBUGP("icookie=%x, %x\n", pu32[0], pu32[1]);
			return tp;
		}
		tp++;
	}
	
	return NULL;
}


/*********************************************************************************
* Routine Name :  _findIsakmpOut
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_findIsakmpOut(u_int32_t local_ip,u_int32_t peer_ip,u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < IPSEC_MaxSession  ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&
		   tp->icookie == icookie &&
		   tp->state == IPSEC_USED){

			DEBUGP("find IKE out i=%d, local %d.%d.%d.%d peer %d.%d.%d.%d icookie=%x\n", i,NIPQUAD(local_ip), NIPQUAD(peer_ip), icookie);
			return tp;
		}
		tp++;
	}
	
	return NULL;
}



/*********************************************************************************
* Routine Name :  _addIsakmp
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_addIsakmp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t alias_ip,
		u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	// find the existed one
	for(i = 0 ; i < IPSEC_MaxSession  ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&
		   tp->state == IPSEC_USED) {
		   	// session refresh
			tp->idle_timer = 0;
			tp->icookie = icookie;
			tp->rcookie = rcookie;
			DEBUGP("Existed session #%d been found\n", i);
			return tp;
		}
		tp++;
	}
	
	tp = _isakmpDb;
	// if not already exists, find a new one
	for(i = 0 ; i < IPSEC_MaxSession  ; i++){
		if(tp->state == IPSEC_FREE){
			memset(tp,0,sizeof(struct isakmp_data_s));
			tp->idle_timer = 0;
			tp->peer_ip = peer_ip;
			tp->local_ip = local_ip;
			tp->alias_ip = alias_ip;
			tp->icookie = icookie;
			tp->rcookie = rcookie;
			tp->state = IPSEC_USED;

			DEBUGP("Free session #%d been found\n", i);
			return tp;
		}
		tp++;
	}
	return NULL;
}


#ifdef IPSEC_ESP_HOOK
//static unsigned int esp_help(struct sk_buff **pskb,
static unsigned int esp_help(struct sk_buff *skb,
				struct nf_conn *ct,	
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp)
{
	struct isakmp_data_s *tb;
	struct iphdr *iph = ip_hdr(skb);

	u_int32_t  *spi = (void *) iph + iph->ihl * 4;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr,o_spi;
	o_spi= *spi;
	
	if(ipsec_flag=='0')
		return NF_DROP;

	#if 0
	if (hooknum != 0)
		return NF_ACCEPT;
	#endif
	
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	DEBUGP("%d.%d.%d.%d -------> %d.%d.%d.%d, spi=%x\n", NIPQUAD(s_addr), NIPQUAD(d_addr), o_spi);
	
	//if (ctinfo < IP_CT_IS_REPLY)
	{	// original
		//printk("original\n");
		//printk("ori, %x, %x\n",
		// (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
		// (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all));
		//printk("rep, %x, %x\n",
		// (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
		// (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all));
		DEBUGP("findEspOut src %d.%d.%d.%d, dst %d.%d.%d.%d\n", NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), NIPQUAD(d_addr));
		tb = _findEspOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, o_spi );
		if(tb==NULL)
		{
        		tb=_addEsp(ct->tuplehash[dir].tuple.src.u3.ip,d_addr,o_spi);
			if(tb != NULL)
			{
				// original SA been changed
				if (tb->pctrack != ct){
					DEBUGP("original SA been changed, update it !\n");	
					tb->pctrack = ct;
				}
				return NF_ACCEPT;
			}
			else
			{
				// maybe it's reply
				//printk("can not bind to session on original\n");
//				return NF_DROP;
			}
		}
		else
			return NF_ACCEPT;
	}
	
	//else // reply
	{	
		//printk("reply\n");
		DEBUGP("findEspIn src %d.%d.%d.%d, dst %d.%d.%d.%d\n", NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip));
		tb = _findEspIn(ct->tuplehash[dir].tuple.src.u3.ip, ct->tuplehash[dir].tuple.dst.u3.ip, o_spi);
		if(tb!=NULL)
		{
			if (tb->pctrack != ct)
			{
				DEBUGP("a new ct, reply should have been refreshed\n");
				DEBUGP("modify ip from %d.%d.%d.%d to %d.%d.%d.%d\n", NIPQUAD(iph->daddr), NIPQUAD(tb->local_ip));
				iph->daddr=tb->local_ip;
				//ramen 20080815 start--let the esp packet from wan to go through our modem
				ct->tuplehash[!dir].tuple.src.u3.ip=tb->local_ip;			
				//end--let the esp packet from wan to go through our modem
				skb->nfctinfo=IP_CT_RELATED;
				iph->check=0;
				iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
			}
			
			DEBUGP("reply to localip=%x\n", tb->local_ip);
		}
		else
		{
			DEBUGP("can not bind to session on reply, drop it!\n");
			return NF_DROP;
		}
	}
	
	return NF_ACCEPT;
}
#endif //IPSEC_ESP_HOOK

static unsigned int in_help(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp)
{
	struct iphdr *iph = ip_hdr(skb);
	struct udphdr *udph = (void *) iph + iph->ihl * 4;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr;
	u_int32_t *pu32;
	u_int64_t *dptr,icookie, rcookie;
	struct isakmp_data_s *tb;

	
	if(ipsec_flag=='0')
		return NF_DROP;

	DEBUGP("-------------  In-bound helper  -------------\n");

	
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	dptr = (u_int64_t *) ((void *) udph + sizeof(struct udphdr));
	icookie= dptr[0];
	rcookie= dptr[1];
	DEBUGP("s_addr=%x, d_addr=%x ", s_addr, d_addr);
	pu32 = (u_int32_t *)&icookie;
	DEBUGP("icookie=%x, %x, \n", pu32[0], pu32[1]);
	pu32 = (u_int32_t *)&rcookie;
	DEBUGP("rcookie=%x, %x\n", pu32[0], pu32[1]);
       	
#if 0
	// Mason Yu IKE Bug
	if( (hooknum==0 && rcookie!=0) ||
	    (hooknum==0 && icookie==0) )
#else
	if( rcookie!=0 || icookie==0)
#endif
	{		
		if (ctinfo >= IP_CT_IS_REPLY)			
		{				
			DEBUGP("This is REPLY dir packet, finding in IN way\n");
			// It's reply
			tb = _findIsakmpIn(ct->tuplehash[dir].tuple.src.u3.ip, ct->tuplehash[dir].tuple.dst.u3.ip, icookie, rcookie);
			if(tb!=NULL)
			{
				if(iph->daddr != tb->local_ip) 
				{
					DEBUGP("change ip-daddr %d.%d.%d.%d to %d.%d.%d.%d\n", NIPQUAD(iph->daddr),NIPQUAD( tb->local_ip));					
					iph->daddr=tb->local_ip;
					iph->check=0;
					iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
					udph->check=0;
					udph->check=csum_partial((char *)udph,ntohs(udph->len),0);
					udph->check=csum_tcpudp_magic(iph->saddr,iph->daddr ,ntohs(udph->len),IPPROTO_UDP,udph->check);
					DEBUGP("New local:%d.%d.%d.%d  IPcheck=%x UDPcheck=%x\n",NIPQUAD(d_addr),iph->check,udph->check);
				}
			}
		}
		// Kaohj -- remove for checkpoint inter-operability
		#if 0
		else	// It's original
		{
			DEBUGP("This is ORIGINAL dir packet, finding in OUT way\n");		
			tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, icookie, rcookie);			
			
			// Mason Yu IKE Bug
			//if (tb == NULL)	
			if (tb == NULL && icookie!=0)
			{
				DEBUGP("Not found IKE session, drop it\n");
				return NF_DROP;
			}	
			
		}
		#endif
	}
	
	return NF_ACCEPT;
}


static unsigned int out_help(struct sk_buff *skb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp)
{
	struct iphdr *iph = ip_hdr(skb);
	struct udphdr *udph = (void *) iph + iph->ihl * 4;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr;
	u_int32_t *pu32;
	u_int64_t *dptr,icookie, rcookie;
	struct isakmp_data_s *tb;
	
	if(ipsec_flag=='0')
		return NF_DROP;


	DEBUGP("-------------- out-bound helper -----------------\n");
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	dptr = (u_int64_t *) ((void *) udph + sizeof(struct udphdr));
	icookie= dptr[0];
	rcookie= dptr[1];
	DEBUGP("s_addr=%d.%d.%d.%d, d_addr=%d.%d.%d.%d ", NIPQUAD(s_addr), NIPQUAD(d_addr));
	pu32 = (u_int32_t *)&icookie;
	DEBUGP("icookie=%x, %x, \n", pu32[0], pu32[1]);
	pu32 = (u_int32_t *)&rcookie;
	DEBUGP("rcookie=%x, %x\n", pu32[0], pu32[1]);

#if 0
	// Mason Yu IKE Bug
	if( (hooknum==4 && rcookie==0 )|| 
	    (hooknum==4 && icookie==0))
#else
	if( rcookie==0 || icookie==0)
#endif
	{
		DEBUGP("This is ORIGINAL dir packet, finding in OUT way\n");
		tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, icookie, rcookie);
		
		if(tb==NULL)
		{			  
			_addIsakmp(ct->tuplehash[dir].tuple.src.u3.ip,d_addr,s_addr,icookie,rcookie);
			return NF_ACCEPT;
		}
		else
		{
			//printk("drop the very first IKE\n");
			// Mason Yu IKE Bug 		
			//return NF_DROP;	// first packet, should not be found
		}
	}
	return NF_ACCEPT;
}


static void check_timeout(unsigned long data)
{
	struct isakmp_data_s *tp=_isakmpDb;
	int i;
	
	for(i=0; i < IPSEC_MaxSession ;i++)
	{
		if(tp == NULL || _isakmpDb == NULL)
			break;
		if(tp->state == IPSEC_FREE)
		{
			tp++;
			continue;
		}
		tp->idle_timer++;
		if(tp->idle_timer > IPSEC_IDLE_TIME)
		{
			tp->state = IPSEC_FREE;
		}
		tp++;
	}
	
	ipsec_time.expires=jiffies + HZ;
	add_timer(&ipsec_time);
}

static int read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	
	len = sprintf(page, "%c\n", ipsec_flag);
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

#ifdef IPSEC_ESP_HOOK
#include <net/netfilter/nf_conntrack_l4proto.h>
extern struct nf_conntrack_l4proto ip_conntrack_protocol_esp;
#endif //IPSEC_ESP_HOOK
static int write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(&ipsec_flag, buffer, 1)) 
	{		
#ifdef IPSEC_ESP_HOOK
		if( ipsec_flag == '0' )
		{
			struct nf_conntrack_l4proto *p=&ip_conntrack_protocol_esp;
			if(__nf_ct_l4proto_find( p->l3proto, p->l4proto)==p)
			{
				DEBUGP( "unregister ip_conntrack_protocol_esp\n" );
				nf_conntrack_l4proto_unregister(&ip_conntrack_protocol_esp);
			}
		}
#endif //IPSEC_ESP_HOOK

		return count;
	}
	
	return -EFAULT;
}


/* This function is intentionally _NOT_ defined as  __exit, because
 * it is needed by init() */
static void fini(void)
{
	rcu_assign_pointer(ip_nat_ipsec_inbound_hook, NULL);
	rcu_assign_pointer(ip_nat_ipsec_outbound_hook, NULL);
#ifdef IPSEC_ESP_HOOK
	rcu_assign_pointer(ip_nat_esp_hook, NULL);
#endif //IPSEC_ESP_HOOK

	synchronize_rcu();
}

static int __init init(void)
{
	int ret = 0;
	struct proc_dir_entry *res=create_proc_entry("algonoff_ipsec",0,NULL);
	
	if (res) {
		res->read_proc=read_proc;
		res->write_proc=write_proc;
	}


	BUG_ON(ip_nat_ipsec_inbound_hook != NULL);
	BUG_ON(ip_nat_ipsec_outbound_hook != NULL);
#ifdef IPSEC_ESP_HOOK
	BUG_ON(ip_nat_esp_hook != NULL);
#endif //IPSEC_ESP_HOOK
	rcu_assign_pointer(ip_nat_ipsec_inbound_hook, in_help);
	rcu_assign_pointer(ip_nat_ipsec_outbound_hook, out_help);
#ifdef IPSEC_ESP_HOOK
	rcu_assign_pointer(ip_nat_esp_hook, esp_help);
#endif //IPSEC_ESP_HOOK

	_isakmpDb = kmalloc(IPSEC_MaxSession*(sizeof(struct isakmp_data_s)),GFP_KERNEL);
	memset(_isakmpDb,0,IPSEC_MaxSession*(sizeof(struct isakmp_data_s)));
	
	init_timer(&ipsec_time);	
	ipsec_time.expires=jiffies + HZ;
	ipsec_time.data=(unsigned long)_isakmpDb;
	ipsec_time.function=&check_timeout;
	add_timer(&ipsec_time);
	
	return ret;
}

module_init(init);
module_exit(fini);
