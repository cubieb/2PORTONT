/*
   netadsl.c - DSL EOC socket
   Created by Kaohj
*/

#include <linux/config.h>
#include <linux/net.h>		/* struct socket, struct net_proto,
				   struct proto_ops */
#include <linux/init.h>
#include <linux/timer.h>
#include <net/sock.h>		/* for sock_no_* */
/*linux-2.6.19*/
#include <linux/poll.h>

#define		EOC_POLL_INTERVAL	50	/* jiffies */
#define		EOC_MAX_LEN		512
#define		EOC_HDLC_PAYLOAD_SIZE	484
#define		HDLC_HDR_PID_SIZE	4  /* 2 bytes Herder + 2 bytes Protocol ID for SNMP */

#define DSL_DEBUG_EOC_ENABLE			(SIOCDEVPRIVATE+1)
#define DSL_DEBUG_NUM2PRINT			(SIOCDEVPRIVATE+2)
#define DSL_DEBUG_STATUS			(SIOCDEVPRIVATE+3)

const char hdlc_HDR_PID[]={0xff, 0x03, 0x81, 0x4c};
// loopback test
//#define LOOPBACK_TEST
#define IO_DEBUG

#ifdef IO_DEBUG
#define TRACE	printk
#else
#define TRACE	1?(void)0 : printk
#endif

#if 0
const char *msg[3]={"The information in this publication is believed to be accurate as of its publication date.",
	"Any express or implied licese or right to or under any ATM Forum member company's patent, copyright, trademark or trademark or trade secret rights which are or may be associated with the ideas, techniques, concepts or expressions contained herein;",
	"NOTE: The user's attention is called to the possibility that implementation of the ATM interoperability specification contained herein may require use of an invention covered by patent rights held by ATM Forum Member companies or others."};
#endif

wait_queue_head_t	adsl_sleep;		/* Sock wait queue	*/
struct sk_buff_head	adsl_receive_queue;	/* Incoming packets		*/
struct timer_list	adsl_timer;
char eoc_buf[EOC_MAX_LEN];
int debug_eoc=0;
int debug_eoc_num2print=100;

extern int mm_sendEOCmessage(unsigned char *msgPtr, unsigned short leng);
extern int mm_getEOCrxmessage(unsigned char *msgPtr, unsigned short *leng);

void read_eoc(unsigned long data)
{
	struct sk_buff *skb;
	unsigned short mylen;
	int length, ret;
	
	ret=mm_getEOCrxmessage(eoc_buf, &mylen);
	
	if (ret!=0 && mylen!=0) {
		// Input: strip HDLC overhead
		length = (int)mylen - HDLC_HDR_PID_SIZE;
		if (length > 0) {
			skb = dev_alloc_skb(EOC_MAX_LEN);
			if (skb) {
				skb_put(skb, length);
				memcpy(skb->data, eoc_buf+HDLC_HDR_PID_SIZE, length);
				// for debug
				/*
				{
					int k;
					TRACE("\nI:");
					for (k=0; k<length; k++) {
						if (k%16==0)
							TRACE("\n  ");
						TRACE(" %.02x", skb->data[k]);
					}
					TRACE("\n");
				}
				*/
				skb_queue_tail(&adsl_receive_queue,skb);
				wake_up(&adsl_sleep);
			}
		}
	}
	#if 0
	if (data >= 3)
		data = 0;
	skb = dev_alloc_skb(strlen(msg[data])+1);
	if (skb) {
		memcpy(skb_put(skb, strlen(msg[data])+1), msg[data], strlen(msg[data])+1);
		skb_queue_tail(&adsl_receive_queue,skb);
		wake_up(&adsl_sleep);
	}
	#endif
	// re-schedule timer
	adsl_timer.expires = jiffies+EOC_POLL_INTERVAL;
	adsl_timer.data=data+1;
	adsl_timer.function = read_eoc;
	add_timer(&adsl_timer);
}

/*linux-2.6.19*/
//int adsl_recvmsg(struct socket *sock,struct msghdr *m,int total_len,
//    int flags,struct scm_cookie *scm)
int adsl_recvmsg(struct kiocb *iocb, struct socket *sock,
			  struct msghdr *m, size_t total_len, int flags)

{
	DECLARE_WAITQUEUE(wait,current);
	struct sk_buff *skb;
	int eff_len,error;
	void *buff;
	int size, k;
	
	if (flags & ~MSG_DONTWAIT) return -EOPNOTSUPP;
	if (m->msg_iovlen != 1) return -ENOSYS; /* fix this later @@@ */
	buff = m->msg_iov->iov_base;
	size = m->msg_iov->iov_len;
	add_wait_queue(&adsl_sleep,&wait);
	set_current_state(TASK_INTERRUPTIBLE);
	error = 1; /* <= 0 is error */
	while (!(skb = skb_dequeue(&adsl_receive_queue))) {
		if (flags & MSG_DONTWAIT) {
			error = -EAGAIN;
			break;
		}
		schedule();
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			error = -ERESTARTSYS;
			break;
		}
	}
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&adsl_sleep,&wait);
	
	if (error <= 0)
	{
		return error;
	}
		
	eff_len = skb->len > size ? size : skb->len;
	if (skb->len > size) /* Not fit ?  Report it... */
		m->msg_flags |= MSG_TRUNC;
	
	// for debug
	if (debug_eoc) {
		TRACE("\nI: EoC");
		for (k=0; k<eff_len; k++) {
			if (k%16==0)
				TRACE("\n  ");
			TRACE(" %2.2x", skb->data[k]);
		}
		TRACE("\n");
	}
	error = copy_to_user(buff,skb->data,eff_len) ? -EFAULT : 0;
	kfree_skb(skb);
	return error ? error : eff_len;
}

/*linux-2.6.19*/
//int adsl_sendmsg(struct socket *sock,struct msghdr *m,int total_len,
//    struct scm_cookie *scm)
int adsl_sendmsg(struct kiocb *iocb, struct socket *sock,
	struct msghdr *m, size_t total_len)
{
	DECLARE_WAITQUEUE(wait,current);
	int error;
	const void *buff;
	int size, k;

	if (m->msg_name) return -EISCONN;
	if (m->msg_iovlen != 1) return -ENOSYS; /* fix this later @@@ */
	buff = m->msg_iov->iov_base;
	size = m->msg_iov->iov_len;
	if (!size) return 0;
	if (size < 0 || size > EOC_HDLC_PAYLOAD_SIZE) return -EMSGSIZE;
	// for debug
	if (debug_eoc) {
		TRACE("\nO: EoC");
		for (k=0; k<size; k++) {
			unsigned char *chbuf=(unsigned char *)buff;
			if (k%16==0)
				TRACE("\n  ");
			TRACE(" %2.2x", chbuf[k]);
		}
		TRACE("\n");
	}
	#ifndef LOOPBACK_TEST
	memcpy(eoc_buf, hdlc_HDR_PID, HDLC_HDR_PID_SIZE);
	copy_from_user(eoc_buf+HDLC_HDR_PID_SIZE, buff, size);
	add_wait_queue(&adsl_sleep,&wait);
	set_current_state(TASK_INTERRUPTIBLE);
	error = 0;
	while (!mm_sendEOCmessage(eoc_buf, size+HDLC_HDR_PID_SIZE)) {
		if (m->msg_flags & MSG_DONTWAIT) {
			error = -EAGAIN;
			break;
		}
		schedule();
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			error = -ERESTARTSYS;
			break;
		}
	}
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&adsl_sleep,&wait);
	if (error) return error;
	
	// loopback test
	#else
	{
		struct sk_buff *skb;
		skb = dev_alloc_skb(EOC_MAX_LEN);
		error = 0;
		if (skb) {
			if (copy_from_user(skb_put(skb,size),buff,size)) {
				kfree_skb(skb);
				return -EFAULT;
			}
			skb_queue_tail(&adsl_receive_queue,skb);
			wake_up(&adsl_sleep);
		}
	}
	#endif
	return error ? error : size;
}


static struct proto adsl_proto = {
	.name	= "EOC",
	.owner	= THIS_MODULE,
	.obj_size	= sizeof(struct sock),
};

/*linux-2.6.19*/
//struct sock *alloc_net_adsl_sk(int family)
struct sock *alloc_net_adsl_sk(struct net *net,int family)
{
	struct sock *sk;

	/*linux-2.6.19*/
	//sk = sk_alloc(family, GFP_KERNEL, &adsl_proto, 1);
	sk = sk_alloc(net,family, GFP_KERNEL, &adsl_proto);
	if (!sk)
		return NULL;
	sock_init_data(NULL, sk);
	return sk;
}

void free_net_adsl_sk(struct sock *sk)
{
	sk_free(sk);
}

/*linux-2.6.19*/
//int adsl_create(struct socket *sock,int protocol,int family)
int adsl_create(struct net *net,struct socket *sock,int protocol,int family)
{
	struct sock *sk;
	
	sock->sk = NULL;
	if (sock->type == SOCK_STREAM) return -EINVAL;
	/*linux-2.6.19*/
	//if (!(sk = alloc_net_adsl_sk(family))) return -ENOMEM;
	if (!(sk = alloc_net_adsl_sk(net,family))) return -ENOMEM;
	//init_waitqueue_head(&adsl_sleep);
	//skb_queue_head_init(&adsl_receive_queue);
	//skb_queue_head_init(&adsl_listenq);
	sock->sk = sk;
	return 0;
}

int adsl_release(struct socket *sock)
{
	if (sock->sk)
		free_net_adsl_sk(sock->sk);
	return 0;
}

unsigned int adsl_poll(struct file *file,struct socket *sock,poll_table *wait)
{
	struct sock *sk;
	unsigned int mask;

	sk = sock->sk;
	poll_wait(file,&adsl_sleep,wait);
	mask = 0;
	if (skb_peek(&adsl_receive_queue))
		mask |= POLLIN | POLLRDNORM;
	
	return mask;
}

int adsl_ioctl(struct socket *sock,unsigned int cmd,unsigned long arg)
{
	int val, ret_val;

	ret_val = 0;
	if (get_user(val,(int *)arg)) {
		ret_val = -EFAULT;
		goto done;
	}
	switch (cmd) {
		case DSL_DEBUG_EOC_ENABLE:
			debug_eoc = val;
			break;
		case DSL_DEBUG_NUM2PRINT:
			debug_eoc_num2print=val;
			break;
		case DSL_DEBUG_STATUS:
			printk("eoc = %d\n", debug_eoc);
			break;
		default:
			break;
	}
	
 done:
	return ret_val;
}



static struct proto_ops adsl_proto_ops = {
	family:		PF_DSLEOC,

	release:	adsl_release,
	bind:		0,
	connect:	0,
	socketpair:	0,
	accept:		0,
	getname:	0,
	poll:		adsl_poll,
	ioctl:		adsl_ioctl,
	listen:		0,
	shutdown:	0,
	setsockopt:	0,
	getsockopt:	0,
	sendmsg:	adsl_sendmsg,
	recvmsg:	adsl_recvmsg,
	mmap:		0,
	sendpage:	0,
};


#include <linux/smp_lock.h>
SOCKOPS_WRAP(adsl_proto, PF_DSLEOC);


/*linux-2.6.19*/
//static int netadsl_create(struct socket *sock,int protocol)
static int netadsl_create(struct net *net, struct socket *sock,int protocol)
{

	sock->ops = &adsl_proto_ops;
	/*linux-2.6.19*/
	//return adsl_create(sock,protocol,PF_DSLEOC);
	return adsl_create(net,sock,protocol,PF_DSLEOC);
}

static struct net_proto_family adsl_family_ops = {
	.family 	= PF_DSLEOC,
	.create 	= netadsl_create,
	.owner	= THIS_MODULE,
};


/*
 *	Initialize the ADSL EOC protocol family
 */


static int __init netadsl_init(void)
{
	int error;

	error = proto_register(&adsl_proto, 1);
        if (error != 0) {
                printk(KERN_CRIT "%s: Cannot create unix_sock SLAB cache!\n",
		       __FUNCTION__);
		goto out;
	}

	error = sock_register(&adsl_family_ops);
	if (error != 0) {
		printk(KERN_ERR "NETADSL: can't register (%d)",error);
		goto out;
	}
	// Kaohj
	init_waitqueue_head(&adsl_sleep);
	skb_queue_head_init(&adsl_receive_queue);
	// init and start timer
	init_timer(&adsl_timer);
	adsl_timer.expires = jiffies+EOC_POLL_INTERVAL;
	adsl_timer.data = 0;
	adsl_timer.function = read_eoc;
	add_timer(&adsl_timer);
//#ifdef CONFIG_PROC_FS
//	error = atm_proc_init();
//	if (error) printk("atm_proc_init fails with %d\n",error);
//#endif
out:
	return 0;
}

module_init(netadsl_init);
