/* String matching match for iptables
 *
 * (C) 2005 Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>

/*linux-2.6.19*/
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/tcp.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_string.h>
#include <linux/textsearch.h>

MODULE_AUTHOR("Pablo Neira Ayuso <pablo@eurodev.net>");
MODULE_DESCRIPTION("Xtables: string-based matching");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_string");
MODULE_ALIAS("ip6t_string");



/* QL 20110629: url filter fail in some special circumstance, we should match more key words. */
#define http_get_len	4
#define http_host_len	6
#define http_refer_len	9
char * method[] = {	
	"GET ",
	"HEAD ",
	"POST ",
	"PUT ",
	"OPTIONS ",
	"DELETE ",
	"TRACE ",
	"CONNECT ",
	0
};


/*linux-2.6.19*/
/* Linear string search based on memcmp() */
char *search_linear (char *needle, char *haystack, int needle_len, int haystack_len) 
{
	char *k = haystack + (haystack_len-needle_len);
	char *t = haystack;
	
	while ( t <= k ) {
		if (memcmp(t, needle, needle_len) == 0)
			return t;
		t++;
	}

	return NULL;
}

/* Backward Linear string search based on memcmp() */
char *backward_search_linear (char *needle, char *haystack, int needle_len, int haystack_len) 
{
	char *t = haystack + haystack_len -needle_len -5;
	
	if (memcmp(t, needle, needle_len) == 0)
		return t;

	return NULL;
}


#define http_start_len 4
#define http_end_len 9 
#define TCP_HEADER_LENTH 20

static int match_HTTP_GET(char **haystack,int *hlen, char *needle, int nlen)
{    
    char http_start[]="GET ";
    //char http_end[]="\r\nAccept:";
    /*QL 20110629 start*/
	char http_refer[]="Referer: ";
	char http_host[]="Host: ";
	char *pchkStart=NULL;//point to the check start place
	int chkLen;//check len	
	int idx;
	char *pUri;
	int uriLen;
	/*QL 20110629 end */
    char *ptend;
    
    //alex  
    int tcp_datalen;		
	char *ptstart;
	int datalen;
	
    tcp_datalen = (((struct tcphdr*)(*haystack))->doff)*4;
    //point to the HTTP Header
    ptstart=(*haystack)+tcp_datalen;
    datalen=*hlen-tcp_datalen;  

	/*QL 20110629 start*/
#if 0
    // Check if the packet is HTTT GET packet.
    if(memcmp(ptstart,http_start,http_start_len)!= 0) {
    	//printk("match_HTTP: Not Find the HTTP GET\n");
    	return 0;
    }
    
    
    //Check if the packet match the URL string of the rules   
    //if((ptend=search_linear(http_end,*haystack,http_end_len,datalen))== NULL){
    if((ptend=search_linear(needle,ptstart,nlen,datalen))== NULL){
    	//printk("match_HTTP: URL string not match rule\n");
    	return 0;
    }
#else
	for (idx=0; method[idx]; idx++)
	{
		if (memcmp(ptstart, method[idx], strlen(method[idx]))==0) {
			pchkStart = ptstart + strlen(method[idx]);
			break;
		}
	}
	if (!pchkStart)//didn't find method, pass it.
		return 0;

	for (chkLen=0; pchkStart[chkLen]; chkLen++)
	{
		if ((pchkStart[chkLen] == ' ') || 
			((pchkStart[chkLen]=='\r') && (pchkStart[chkLen+1]=='\n')))
			break;
	}
	if ((1==chkLen) && (pchkStart[0]==0x2f))
	{//first request packet, uri is null, then just check host string only.
		if ((pchkStart=strstr(ptstart, http_host)) != NULL) {
			pchkStart = pchkStart + http_host_len;
			for (chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
				return 0;
		}
	}
	else
	{//first check referer string, if start with "http://", then just only check it, else check host; if referer string not exists, then check uri.
		pUri = pchkStart;
		uriLen = chkLen;

		if ((pchkStart=strstr(ptstart, http_start)) != NULL) {
			pchkStart=pchkStart + http_get_len;
			for(chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == ' ') || 
					((pchkStart[chkLen]=='\r') && (pchkStart[chkLen+1]=='\n')))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) != NULL)
				return 1;
		}
		if ((pchkStart=strstr(ptstart, http_refer)) != NULL) {
			pchkStart = pchkStart + http_refer_len;
			if (memcmp(pchkStart, "http://", strlen("http://")) == 0) {
				for (chkLen=0; pchkStart[chkLen]; chkLen++)
				{
					if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
						break;
				}
				if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
					return 0;
				return 1;
			}
		}

		//go to check host string, if not find, go to check uri.
		if ((pchkStart=strstr(ptstart, http_host)) != NULL) {
			pchkStart = pchkStart + http_host_len;
			for (chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
				return 0;
		} else {//only in http 1.0
			if ((ptend=search_linear(needle, pUri, nlen, uriLen)) == NULL)
				return 0;
		}
	}
#endif
	/*QL 20110629 end */

    return 1;
    
}

static int match_HTTP_GET_URL_ALLOW(char **haystack,int *hlen, char *needle, int nlen)
{    
    char http_start[]="GET ";
    //char http_end[]="\r\nAccept:";
    char *ptend;
	/*QL 20110629 start*/
 	char http_refer[]="Referer: ";
	char http_host[]="Host: ";
	char *pchkStart=NULL;//point to the check start place
	int chkLen;//check len
	int idx;
	char *pUri;
	int uriLen;
	/*QL 20110629 end */
	
    //point to the HTTP Header
    //char *ptstart=(*haystack)+TCP_HEADER_LENTH;
    //int datalen=*hlen-TCP_HEADER_LENTH;
    char *ptstart;
    int datalen;
    //alex  
    int tcp_datalen;		
    tcp_datalen = (((struct tcphdr*)(*haystack))->doff)*4;
    //point to the HTTP Header
    ptstart=(*haystack)+tcp_datalen;
    datalen=*hlen-tcp_datalen;  

	/*QL 20110629 start*/
#if 0
    // Check if the packet is HTTT GET packet.
    if(memcmp(ptstart,http_start,http_start_len)!= 0) {
    	//printk("match_HTTP: Not Find the HTTP GET\n");
    	return 2;
    }
    
    
    //Check if the packet match the URL string of the rules   
    //if((ptend=search_linear(http_end,*haystack,http_end_len,datalen))== NULL){
    if((ptend=search_linear(needle,ptstart,nlen,datalen))== NULL){
    	//printk("match_HTTP: URL string not match rule\n");
    	return 0;
    }
    //printk("match_HTTP: URL string  match rule\n");
#else
	for (idx=0; method[idx]; idx++)
	{
		if (memcmp(ptstart, method[idx], strlen(method[idx]))==0) {
			pchkStart = ptstart + strlen(method[idx]);
			break;
		}
	}
	if (!pchkStart)//didn't find method
		return 2;
	
	for (chkLen=0; pchkStart[chkLen]; chkLen++)
	{
		if ((pchkStart[chkLen] == ' ') || 
			((pchkStart[chkLen]=='\r') && (pchkStart[chkLen+1]=='\n')))
			break;
	}

	if ((1==chkLen) && (pchkStart[0]==0x2f))
	{//first request packet, uri is null, then just check host string only.
		if ((pchkStart=strstr(ptstart, http_host)) != NULL) {
			pchkStart = pchkStart + http_host_len;
			for (chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
				return 0;
		}
	}
	else
	{//first check referer string, if start with "http://", then just only check it, else check host; if referer string not exists, then check uri.
		pUri = pchkStart;
		uriLen = chkLen;
		if ((pchkStart=strstr(ptstart, http_start)) != NULL) {
			pchkStart=pchkStart + http_get_len;
			for(chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == ' ') || 
					((pchkStart[chkLen]=='\r') && (pchkStart[chkLen+1]=='\n')))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) != NULL)
				return 1;
		}
		if ((pchkStart=strstr(ptstart, http_refer)) != NULL) {
			pchkStart = pchkStart + http_refer_len;
			if (memcmp(pchkStart, "http://", strlen("http://")) == 0) {
				for (chkLen=0; pchkStart[chkLen]; chkLen++)
				{
					if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
						break;
				}
				if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
					return 0;
				return 1;
			}
		}

		//go to check host string, if not find, go to check uri.
		if ((pchkStart=strstr(ptstart, http_host)) != NULL) {
			pchkStart = pchkStart + http_host_len;
			for (chkLen=0; pchkStart[chkLen]; chkLen++)
			{
				if ((pchkStart[chkLen] == '\r') && (pchkStart[chkLen+1]=='\n'))
					break;
			}
			if ((ptend=search_linear(needle, pchkStart, nlen, chkLen)) == NULL)
				return 0;
		} else {//only in http 1.0
			if ((ptend=search_linear(needle, pUri, nlen, uriLen)) == NULL)
				return 0;
		}
	}
#endif
	/*QL 20110629 end */

    return 1;
    
}


#define UDP_HEADER_LENTH 8
static int match_DNS_QUERY(char **haystack,int *hlen, char *needle, int nlen)
{      
    char *ptend;
    
    //point to the DNS Header
    char *ptstart=(*haystack)+UDP_HEADER_LENTH;
    int datalen=*hlen-UDP_HEADER_LENTH; 
    
    //Check if the packet match the URL string of the rules  
    //if((ptend=search_linear(needle,ptstart,nlen,datalen))== NULL){    
    //if((ptend=search_linear(cmpStr,ptstart,strlen(cmpStr),datalen))== NULL){   
    if((ptend=backward_search_linear(needle,ptstart,nlen,datalen))== NULL){
    	//printk("match_DNS_QUERY: Domain name string Not match rule\n");
    	return 0;
    }
 	
    //printk("match_DNS_QUERY: Domain name string match rule\n");
    return 1;
    
}



static bool
string_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_string_info *conf = par->matchinfo;
	struct ts_state state;
#if 1
	/*linux-2.6.19*/
	struct iphdr *ip = ip_hdr(skb);//skb->nh.iph;
	int hlen, nlen;
	char *needle, *haystack;
	int ret=0;

	if (!ip)//process ip packet only
		return 0;
	//get length
	nlen = conf->patlen;
	hlen = ntohs(ip->tot_len) - (ip->ihl*4);
	if (nlen > hlen)
		return 0;

	needle = (char *)&conf->pattern;
	haystack = (char *)ip + (ip->ihl*4);

	// Added by Mason Yu for Domain Blocking
	// Check if the packet is DNS query packet and match the Domain String of the rules	
	if(conf->flagStr==IPT_DOMAIN_STRING && (ip->protocol!=IPPROTO_UDP || !match_DNS_QUERY(&haystack,&hlen,needle,nlen))){
		// Mason Yu
		//printk("Do string match(3.2): The packet is not DNS Query packet or not match the Domain String of the rules.\n");
        	return 0;
	}	

	if (conf->flagStr == IPT_URL_STRING && 
		(ip->protocol!=IPPROTO_TCP || !match_HTTP_GET(&haystack, &hlen, needle, nlen))) {
		//URL Blocking
		return 0;
	}
	
	 else if (conf->flagStr == IPT_URL_ALW_STRING &&
		(ip->protocol!=IPPROTO_TCP || ((ret = match_HTTP_GET_URL_ALLOW(&haystack, &hlen, needle, nlen))==0))) {
		 //URL ALLOWING
		 //printk("needle:%s\n", needle);
		if (!strncmp(needle, "endofurl", 8)) {
			return 1;	//drop
		}
		return 0;
	} else if (conf->flagStr == IPT_GENERAL_STRING){
		memset(&state, 0, sizeof(struct ts_state));

		return (skb_find_text((struct sk_buff *)skb, conf->from_offset, 
			     conf->to_offset, conf->config, &state) 
			     != UINT_MAX) ^ conf->invert;
	}
	//printk("flag:%d needle:%s\n", conf->flagStr, needle);
	if ((conf->flagStr == IPT_URL_ALW_STRING) &&
		(ip->protocol == IPPROTO_TCP) && (ret == 2))
	{
		if (!strncmp(needle, "endofurl", 8))
			return 0;
	}
	
	return 1;

#else
	int invert;

	memset(&state, 0, sizeof(struct ts_state));

	invert = (par->match->revision == 0 ? conf->u.v0.invert :
				    conf->u.v1.flags & XT_STRING_FLAG_INVERT);

	return (skb_find_text((struct sk_buff *)skb, conf->from_offset,
			     conf->to_offset, conf->config, &state)
			     != UINT_MAX) ^ invert;
#endif
}

#define STRING_TEXT_PRIV(m) ((struct xt_string_info *)(m))

static bool string_mt_check(const struct xt_mtchk_param *par)
{
	struct xt_string_info *conf = par->matchinfo;
	struct ts_config *ts_conf;
#if 1

	/*linux-2.6.19*/
	/* Damn, can't handle this case properly with iptables... */
	if (conf->flagStr == IPT_GENERAL_STRING) {
		if (conf->from_offset > conf->to_offset)
			return 0;
		if (conf->algo[XT_STRING_MAX_ALGO_NAME_SIZE - 1] != '\0')
		    	return 0;
		if (conf->patlen > XT_STRING_MAX_PATTERN_SIZE)
			return 0;
		ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
					     GFP_KERNEL, TS_AUTOLOAD);
		if (IS_ERR(ts_conf))
			return 0;

		conf->config = ts_conf;
	}
	return 1;
#else
	int flags = TS_AUTOLOAD;

	/* Damn, can't handle this case properly with iptables... */
	if (conf->from_offset > conf->to_offset)
		return false;
	if (conf->algo[XT_STRING_MAX_ALGO_NAME_SIZE - 1] != '\0')
		return false;
	if (conf->patlen > XT_STRING_MAX_PATTERN_SIZE)
		return false;
	if (par->match->revision == 1) {
		if (conf->u.v1.flags &
		    ~(XT_STRING_FLAG_IGNORECASE | XT_STRING_FLAG_INVERT))
			return false;
		if (conf->u.v1.flags & XT_STRING_FLAG_IGNORECASE)
			flags |= TS_IGNORECASE;
	}
	ts_conf = textsearch_prepare(conf->algo, conf->pattern, conf->patlen,
				     GFP_KERNEL, flags);
	if (IS_ERR(ts_conf))
		return false;

	conf->config = ts_conf;

	return true;
#endif
}

static void string_mt_destroy(const struct xt_mtdtor_param *par)
{
#if 1
	/*linux-2.6.19*/
	return;
#else
	textsearch_destroy(STRING_TEXT_PRIV(par->matchinfo)->config);
#endif
}




static struct xt_match xt_string_mt_reg[] __read_mostly = {
	{
		.name 		= "string",
		.revision	= 0,
		.family		= NFPROTO_UNSPEC,
		.checkentry	= string_mt_check,
		.match 		= string_mt,
		.destroy 	= string_mt_destroy,
		.matchsize	= sizeof(struct xt_string_info),	
		.me 		= THIS_MODULE
	},
#if 0
	{
		.name 		= "string",
		.revision	= 1,
		.family		= NFPROTO_UNSPEC,
		.checkentry	= string_mt_check,
		.match 		= string_mt,
		.destroy 	= string_mt_destroy,
		.matchsize	= sizeof(struct xt_string_info),
		.me 		= THIS_MODULE
	},
#endif
};

static int __init string_mt_init(void)
{
	return xt_register_matches(xt_string_mt_reg,
				   ARRAY_SIZE(xt_string_mt_reg));
}

static void __exit string_mt_exit(void)
{
	xt_unregister_matches(xt_string_mt_reg, ARRAY_SIZE(xt_string_mt_reg));
}

module_init(string_mt_init);
module_exit(string_mt_exit);
