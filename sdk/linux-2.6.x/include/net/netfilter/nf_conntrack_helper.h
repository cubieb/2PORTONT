/*
 * connection tracking helpers.
 *
 * 16 Dec 2003: Yasuyuki Kozakai @USAGI <yasuyuki.kozakai@toshiba.co.jp>
 *	- generalize L3 protocol dependent part.
 *
 * Derived from include/linux/netfiter_ipv4/ip_conntrack_helper.h
 */

#ifndef _NF_CONNTRACK_HELPER_H
#define _NF_CONNTRACK_HELPER_H
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>

struct module;

#define NF_CT_HELPER_NAME_LEN	16

struct nf_conntrack_helper
{
	struct hlist_node hnode;	/* Internal use. */

	const char *name;		/* name of the module */
	struct module *me;		/* pointer to self */
	const struct nf_conntrack_expect_policy *expect_policy;

	/* Tuple of things we will help (compared against server response) */
	struct nf_conntrack_tuple tuple;

	/* Function to call when data passes; return verdict, or -1 to
           invalidate. */
	int (*help)(struct sk_buff *skb,
		    unsigned int protoff,
		    struct nf_conn *ct,
		    enum ip_conntrack_info conntrackinfo);

	void (*destroy)(struct nf_conn *ct);

	int (*to_nlattr)(struct sk_buff *skb, const struct nf_conn *ct);
	unsigned int expect_class_max;
};

extern struct nf_conntrack_helper *
__nf_conntrack_helper_find_byname(const char *name);

extern int nf_conntrack_helper_register(struct nf_conntrack_helper *);
extern void nf_conntrack_helper_unregister(struct nf_conntrack_helper *);

extern struct nf_conn_help *nf_ct_helper_ext_add(struct nf_conn *ct, gfp_t gfp);

extern int __nf_ct_try_assign_helper(struct nf_conn *ct, gfp_t flags);

static inline struct nf_conn_help *nfct_help(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_HELPER);
}

extern int nf_conntrack_helper_init(void);
extern void nf_conntrack_helper_fini(void);

//add by ramen
#ifdef CONFIG_IP_NF_ALG_ONOFF
#define ALGONOFF_INIT(PROTO)\
static unsigned char ginitflag=0;\
extern int alginit_##PROTO();\
static unsigned char algonoff_##PROTO= 0;\
static struct proc_dir_entry* FP_Proc_algonoff_##PROTO;\
int proc_debug_write_##PROTO(struct file *file, const char *buffer, unsigned long count,	   void *data)\
{\
         char proc_buffer[count];\
    memset(proc_buffer, 0, sizeof(proc_buffer));\
    if ( copy_from_user(proc_buffer, buffer, count) ) {\
        return -EFAULT;\
    }\
    if(proc_buffer[0]=='1'){\
	if(!algonoff_##PROTO)\
        	{     \
          printk("*****Enable %s ALG function!*****\n",#PROTO);\
	 algonoff_##PROTO=1;\
	}\
    }\
    else if(proc_buffer[0]=='0'){       \
	if(algonoff_##PROTO){\
		printk("*****Disable %s ALG function!*****\n",#PROTO);\
		algonoff_##PROTO=0;\
		}\
    }\
    else\
        printk("Error setting!\n");\
    return count;\
}\
int proc_debug_read_##PROTO(struct file *file, const char *buffer, unsigned long count, void *data)\
{\
	   if(algonoff_##PROTO==1)\
        		printk("%s ALG ON!\n",#PROTO);\
	    if(algonoff_##PROTO==0)\
        		printk("%s ALG OFF!\n",#PROTO);\
	    return 0;\
}\
int  Alg_OnOff_init_##PROTO()\
{\
	FP_Proc_algonoff_##PROTO= create_proc_entry(PROCFS_NAME,0644, &proc_root);\
    if (FP_Proc_algonoff_##PROTO == NULL) {\
        remove_proc_entry(PROCFS_NAME, &proc_root);\
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",\
            PROCFS_NAME);\
        return -ENOMEM;\
    }\
    FP_Proc_algonoff_##PROTO->read_proc  = (read_proc_t *)proc_debug_read_##PROTO;\
    FP_Proc_algonoff_##PROTO->write_proc  = (write_proc_t *)proc_debug_write_##PROTO;\
    FP_Proc_algonoff_##PROTO->mode       = S_IFREG | S_IRUGO;\
    FP_Proc_algonoff_##PROTO->uid       = 0;\
    FP_Proc_algonoff_##PROTO->gid       = 0;\
    FP_Proc_algonoff_##PROTO->size       = 4;    \
   ginitflag=1;\
    return 0;\
}
#endif


#endif /*_NF_CONNTRACK_HELPER_H*/
