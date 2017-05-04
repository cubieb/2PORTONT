#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/types.h>

#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>

#include <linux/delay.h>
#include "icclog.h"

#define MODULE_NAME "icclog"
#define dbg(fmt, args...) //printk(fmt, ##args)

struct netlog_t
{
        struct task_struct *thread;
        struct socket *sock;
        struct sockaddr_in addr;
		struct sockaddr_in peer;
		
		char *log;
		unsigned int loglen;
		unsigned int pw;
		unsigned int pr;
		//atomic_t lock;

		struct timer_list tmr_send;	// timeout send
		struct timer_list tmr_poll;	// watchdog
		unsigned char sendbuf[1024];
		unsigned char recvbuf[128];
		rwlock_t send_lock;
		
        char running;
		char enabled;
		char kicked;
};

static struct netlog_t *netlog = NULL;

#define NETLOG_AVAIL_BUF(n)	(((n)->pr + NETLOG_MAX_BUFSIZE - 1 - (n)->pw) % NETLOG_MAX_BUFSIZE)
#define NETLOG_DATA_SIZE(n) (((n)->pw - (n)->pr + NETLOG_MAX_BUFSIZE) % NETLOG_MAX_BUFSIZE)
#define NEXT_IDX(k)	((k+1)%NETLOG_MAX_BUFSIZE)

/* function prototypes */
static int ksocket_receive(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
static int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
static int netlog_send_now(void);
static void netlog_ctl(char new_state);

extern void console_input(const char *buf, unsigned int len);

static int netlog_is_enabled(void) {
	return netlog && netlog->enabled;
}

static void netlog_poll_func(unsigned long data) {
//	printk("%s: timeout!\n",__FUNCTION__);
//	if (netlog_is_enabled())
//		netlog_ctl(0);
}

static void netlog_send_func(unsigned long data) {
	u32 datalen, next;
	netlog->kicked = 0;

	datalen = NETLOG_DATA_SIZE(netlog);
	//while (NETLOG_DATA_SIZE(netlog))
	if (datalen > 0)
		netlog_send_now();

	if (unlikely(datalen > NETLOG_SEND_THRESHOLD))
		next = msecs_to_jiffies(10);
	else
		next = NETLOG_PRINT_INTERVAL;
	mod_timer(&netlog->tmr_send, jiffies + next);
}


static void netlog_ctl(char new_state) {
	unsigned long flags;

	local_irq_save(flags);
	
	if (new_state == netlog->enabled)
		return;
	netlog->enabled = new_state;
	netlog->pr = netlog->pw = 0;
	netlog->loglen = NETLOG_MAX_BUFSIZE;	
	netlog->kicked = 0;
	//atomic_set(&netlog->lock, 0);
	if (new_state) {			
		if (!netlog->log) {
			printk("netlog: cannot alloc memory\n");
			netlog->enabled = 0;
		}
		else
		{	// reset memory
			memset(netlog->log , NULL ,NETLOG_MAX_BUFSIZE); 
		}
	} else {
		
		if (timer_pending(&netlog->tmr_poll))
			del_timer_sync(&netlog->tmr_poll);
		if (timer_pending(&netlog->tmr_send))
			del_timer_sync(&netlog->tmr_send);
		//kfree(netlog->log);					
	}

	if (netlog->enabled) {

		rwlock_init(&netlog->send_lock);
		/* start poll timer */
		init_timer(&netlog->tmr_poll);
		netlog->tmr_poll.data = 0;
		netlog->tmr_poll.function = netlog_poll_func;				

		init_timer(&netlog->tmr_send);
		netlog->tmr_send.data = 0;
		netlog->tmr_send.function = netlog_send_func;

		mod_timer(&netlog->tmr_poll, jiffies + NETLOG_WATCHDOGVAL);
		mod_timer(&netlog->tmr_send, jiffies + NETLOG_PRINT_INTERVAL);
	}

	local_irq_restore(flags);
	//printk("netlog enable=%d\n",netlog->enabled);
}

static inline char bit_reverse_char(char c) {
	return (c ^ 0xFF);
}


void netlog_emit_char(char c)
{
	unsigned long flags;
	if (!netlog_is_enabled())
		return;

	if ((!netlog->kicked) && (NETLOG_SEND_THRESHOLD < NETLOG_DATA_SIZE(netlog))) {
		mod_timer(&netlog->tmr_send, jiffies + msecs_to_jiffies(10));
		netlog->kicked = 1;
	}
	
	
	local_irq_save(flags);
	//netlog->log[netlog->pw] = bit_reverse_char(c);
	netlog->log[netlog->pw] = c;
	if (NEXT_IDX(netlog->pw) == netlog->pr)
		netlog->pr = NEXT_IDX(netlog->pr);
	netlog->pw = NEXT_IDX(netlog->pw);
	local_irq_restore(flags);
}

/*
void netlog_early_out(const char *s, unsigned n)
{
	while (n-- && *s) {
		if (*s == '\n')
			netlog_emit_char((unsigned char)'\r');
		netlog_emit_char((unsigned char)*s);
		s++;
	}
}
*/


/* return number of bytes fetched from netlog buffer */
static inline int netlog_peek_buf(char *buf, unsigned buflen) {
	unsigned int bytes_to_read, i;
	//unsigned long flags;
	bytes_to_read = (buflen > NETLOG_DATA_SIZE(netlog)) ? NETLOG_DATA_SIZE(netlog) : buflen;

	//local_irq_save(flags);
	for (i=0; i<bytes_to_read; i++) {
		//buf[i] = netlog->log[netlog->pr];
		//netlog->pr = NEXT_IDX(netlog->pr);
		buf[i] = netlog->log[ (netlog->pr + i) % NETLOG_MAX_BUFSIZE ];		
	}
	//local_irq_restore(flags);
	return bytes_to_read;
}

static int netlog_send_now(void) {
	unsigned int to_send;
	int len;
	int i = 0 ;
	read_lock(&netlog->send_lock);
	to_send = netlog_peek_buf(netlog->sendbuf, sizeof(netlog->sendbuf));
	len = ksocket_send(netlog->sock, &netlog->peer, netlog->sendbuf, to_send);	
	if(len > 0) {
		netlog->pr = (netlog->pr + len) % NETLOG_MAX_BUFSIZE;
	}
	read_unlock(&netlog->send_lock);
	return len;
}


static int parse_input_cmd(const char* buf, int len)
{     
	int ret;
	const char *bufp=buf;
	
	//dbg("netlog_parseInputCmd---\n");

	if(!memcmp(bufp, "CMD:START", 9))
		ret=NETLOG_START;
	
	else if(!memcmp(bufp, "CMD:STOP", 8))
		ret=NETLOG_STOP;
	
	else if(!memcmp(bufp, "INPUT:", 6))
		ret=NETLOG_INPUT;
	
	else if(!memcmp(bufp,"SIG:POL",7))
		ret=NETLOG_POLLING;		
	else
		ret=0;
	
	dbg("return value %d\n",ret);
	
	return ret;	
}   


static void bit_reverse(char *buf,unsigned long len)
{
	char* tempbuf=buf;
	while(len-- >0)
	{
		*tempbuf=bit_reverse_char(*tempbuf);
	    tempbuf++;
	};
}


static void netlog_process(const char *buf, unsigned long len) {
	// check pattern ?
	console_input(buf,len);
	netlog_ctl(1);
/*
	switch (parse_input_cmd(buf, len)) {
	case NETLOG_START:
		if (netlog_is_enabled()) 
			break;
		
		printk("netlog_start\n");	
		ksocket_send(netlog->sock, &netlog->peer, (u8 *)g_szAckStart, sizeof(g_szAckStart));
		netlog_ctl(1);		
		break;
	case NETLOG_STOP:
		if (!netlog_is_enabled())
			break;
		dbg("netlog_stop\n");
		ksocket_send(netlog->sock, &netlog->peer, (u8 *)g_szAckStop, sizeof(g_szAckStop));
		netlog_ctl(0);
		break;
	case NETLOG_INPUT:
		if (!netlog_is_enabled())
			break;
		console_input(buf+6,len-6);
		break;
	case NETLOG_POLLING:
		if (!netlog_is_enabled())
			break;
		if (timer_pending(&netlog->tmr_poll))
			mod_timer(&netlog->tmr_poll, jiffies + NETLOG_WATCHDOGVAL);
		else
			printk("%s(%d):\n", __FUNCTION__,__LINE__);
		//dbg("netlog_poll\n");
		break;
	default:
		dbg("netlog_bug:\n");
	}
	*/
}

static void kthread_start(void)
{
        int size, err;

        /* kernel thread initialization */
        lock_kernel();
        netlog->running = 1;
        current->flags |= PF_NOFREEZE;

        /* daemonize (take care with signals, after daemonize() they are disabled) */
        daemonize(MODULE_NAME);
        allow_signal(SIGKILL);
        unlock_kernel();
		
        /* create a socket */
        if ( (err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &netlog->sock)) < 0)
        {
                printk(KERN_INFO MODULE_NAME": Could not create a datagram socket, error = %d\n", -ENXIO);
                goto out;
        }

        memset(&netlog->addr, 0, sizeof(struct sockaddr));

        netlog->addr.sin_family      = AF_INET;
        netlog->addr.sin_addr.s_addr      = htonl(INADDR_ANY);
        netlog->addr.sin_port      = htons(NETLOG_PORT);

        if ( (err = netlog->sock->ops->bind(netlog->sock, (struct sockaddr *)&netlog->addr, sizeof(struct sockaddr) ) ) < 0) 
        {
                printk(KERN_INFO MODULE_NAME": Could not bind or connect to socket, error = %d\n", -err);
                goto close_and_out;
        }

        printk(KERN_INFO MODULE_NAME": listening on port %d\n", NETLOG_PORT);
        /* main loop */
        for (;;)
        {
                memset(netlog->recvbuf, 0, sizeof(netlog->recvbuf));
                size = ksocket_receive(netlog->sock, &netlog->peer, netlog->recvbuf, sizeof(netlog->recvbuf));

                if (signal_pending(current))
                        break;

                if (size < 0)
					printk(KERN_INFO MODULE_NAME": error getting datagram, sock_recvmsg error = %d\n", size);
                else 
                {                        						
					//bit_reverse(netlog->recvbuf, size);
						
					/* data processing */
					
					netlog_process(netlog->recvbuf, size);                        
                }
        }

close_and_out:
        sock_release(netlog->sock);       
        netlog->sock = NULL;       

out:
        netlog->thread = NULL;
        netlog->running = 0;
}

static int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len)
{
        struct msghdr msg;
        struct iovec iov;
        mm_segment_t oldfs;
        int size = 0;

        if (sock->sk==NULL)
           return 0;

        iov.iov_base = buf;
        iov.iov_len = len;

        msg.msg_flags = 0; //MSG_DONTWAIT;
        msg.msg_name = addr;
        msg.msg_namelen  = sizeof(struct sockaddr_in);
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = NULL;

        oldfs = get_fs();
        set_fs(KERNEL_DS);
        size = sock_sendmsg(sock,&msg,len);
        set_fs(oldfs);

        return size;
}

static int ksocket_receive(struct socket* sock, struct sockaddr_in* addr, unsigned char* buf, int len)
{
        struct msghdr msg;
        struct iovec iov;
        mm_segment_t oldfs;
        int size = 0;

        if (sock->sk==NULL) return 0;

        iov.iov_base = buf;
        iov.iov_len = len;

        msg.msg_flags = 0;
        msg.msg_name = addr;
        msg.msg_namelen  = sizeof(struct sockaddr_in);
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = NULL;

        oldfs = get_fs();
        set_fs(KERNEL_DS);
        size = sock_recvmsg(sock,&msg,len,msg.msg_flags);
        set_fs(oldfs);

        return size;
}

static int __init netlog_init(void)
{
        netlog = kmalloc(sizeof(struct netlog_t), GFP_KERNEL);
        memset(netlog, 0, sizeof(struct netlog_t));

        /* start kernel thread */
        netlog->thread = kthread_run((void *)kthread_start, NULL, MODULE_NAME);
        if (IS_ERR(netlog->thread)) 
        {
                printk(KERN_INFO MODULE_NAME": unable to start kernel thread\n");
                kfree(netlog);
                netlog = NULL;
                return -ENOMEM;
        }
		// pre allocate memory
		netlog->log = kmalloc(NETLOG_MAX_BUFSIZE, GFP_KERNEL);	
        return 0;
}

static void __exit netlog_exit(void)
{
        int err;

        if (netlog->thread==NULL)
                printk(KERN_INFO MODULE_NAME": no kernel thread to kill\n");
        else 
        {
                lock_kernel();
                err = kill_pid(find_get_pid(netlog->thread->pid), SIGKILL, 1);
                unlock_kernel();

                /* wait for kernel thread to die */
                if (err < 0)
                        printk(KERN_INFO MODULE_NAME": unknown error %d while trying to terminate kernel thread\n",-err);
                else 
                {
                        while (netlog->running == 1)
                                msleep(10);
                        printk(KERN_INFO MODULE_NAME": succesfully killed kernel thread!\n");
                }
        }

        /* free allocated resources before exit */
        if (netlog->sock != NULL) 
        {
                sock_release(netlog->sock);
                netlog->sock = NULL;
        }
		// release memory
		if(netlog->log) 
			kfree(netlog->log); 
        kfree(netlog);
        netlog = NULL;

        printk(KERN_INFO MODULE_NAME": module unloaded\n");
}

/* init and cleanup functions */
module_init(netlog_init);
module_exit(netlog_exit);

/* module information */
MODULE_DESCRIPTION("Netlog for remote debugging");
MODULE_AUTHOR("Andrew Chang <yachang@realtek.com>");

