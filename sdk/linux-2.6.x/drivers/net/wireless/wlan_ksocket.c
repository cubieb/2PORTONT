#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/types.h>

#include <linux/version.h>


#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/pid.h>


#include <linux/delay.h>

#include <linux/wireless.h>

#define MODULE_NAME 	"wlan_ksocket"
#define DRV_VERSION 	"0.0.3"
#define SIOCGIWPRIV 	0x8B0D
#define SIOCSAPPPID     0x8b3e
#define CMD_TIMEOUT 	5*HZ

#define DEFAULT_PORT 	2325
#define SIG_PORT 	2313
#define INADDR_SEND ((unsigned long int)0xC0A80101)
#define INADDR_RECV ((unsigned long int)0xC0A80102)

//#define DEBUG_WLAN_KSOCKET
#ifdef DEBUG_WLAN_KSOCKET
#define dbg(fmt, args...) printk(fmt, ##args)
#else
#define dbg(fmt, args...) {}
#endif	

#define MAXLENGTH 1224

static struct socket *sock;
static struct socket *sock_send;
static struct socket *sock_sig;
static struct sockaddr_in addr;
static struct sockaddr_in addr_send;
static struct sockaddr_in addr_sig;
static pid_t pid_real;

static struct net_device *my_dev;
#ifdef CONFIG_RTL_VAP_SUPPORT
#define VAP_NUM			4
static struct net_device *my_vap[VAP_NUM];
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
static struct net_device *my_vxd;
#endif
#ifdef CONFIG_RTL_WDS_SUPPORT
#define MAX_WDS_NUM 	8
static struct net_device *my_wds[MAX_WDS_NUM];
#endif

static int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
static int ksocket_receive(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);

struct kthread_t
{
        struct task_struct *thread;
        int running;
};

static struct kthread_t *kthread = NULL;

static void run_ioctl(char *buf)
{
	struct iwreq wrq;
	struct iw_priv_args	priv[48];
	char data_p[1024];
	int cmd;
	int skfd, err;
	pid_t pid;
	struct net_device * dev;
	

	// receive buf with format: iwreq / cmd / iwreq data content
	
	// cp iwreq struct from buf
	memcpy(&wrq, (buf), sizeof(struct iwreq));
	// cp cmd from buf
	memcpy(&cmd, (buf + sizeof(struct iwreq)), sizeof(int));

	//get ifname
	dev = __dev_get_by_name(&init_net, wrq.ifr_name);
	dbg("%s \n", wrq.ifr_name);

	//adjust iwreq data pointer 
	if(cmd == SIOCGIWPRIV){
		wrq.u.data.pointer = (caddr_t) priv;
		wrq.u.data.length = 0;
		wrq.u.data.flags = 0;
		}
	else if(cmd == SIOCSAPPPID){
		memcpy(&pid_real, (buf + sizeof(struct iwreq)+ sizeof(int)), wrq.u.data.length);
		//memcpy(wrq_real.ifrn_name, wrq.ifrn_name, 16);
		dbg("pid real %d\n", pid_real);
		pid = current->pid;
		dbg("pid current %d\n", pid);
		wrq.u.data.pointer = (caddr_t) &pid;
		wrq.u.data.length = sizeof(pid_t);
	}
	else{
		wrq.u.data.pointer = data_p;
		// cp iwreq data content from buf
		memcpy(wrq.u.data.pointer, (buf + sizeof(struct iwreq)+ sizeof(int)), wrq.u.data.length);
	}
	
	//clear buf
	memset(buf, '\0', sizeof(buf));

	extern int rtl8192cd_ioctl(struct net_device * dev, struct ifreq * ifr, int cmd);
	if( rtl8192cd_ioctl(dev, &wrq, cmd) < 0){
		dbg("%s %d ioctl failed\n", __FUNCTION__, __LINE__);
		//cp ret value to buf
		memset(buf , '0', 1);
	}
	else{
		dbg("data length: %d\n", wrq.u.data.length);

		//fill buf with format: ret / iwreq / iwreq data content
		
		//cp ret value to buf
		memset(buf , '1', 1);
		
		//cp iwreq to buf
		memcpy((buf  + sizeof(char)), &wrq, sizeof(struct iwreq));
		
		//cp iwreq content to buf
		if(cmd == SIOCGIWPRIV)
			memcpy((buf + sizeof(char) + sizeof(struct iwreq)), wrq.u.data.pointer, sizeof(priv));
		else
			memcpy((buf + sizeof(char) + sizeof(struct iwreq)), wrq.u.data.pointer, wrq.u.data.length);
	}
	
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

static void RequestEvent()
{
	char buf[MAXLENGTH];

	dbg("send signal to wlanapp (pid: %d)\n", pid_real);

	//clear buf
	memset(buf, '\0', sizeof(buf));
	
	sprintf(buf , "2");
	memcpy((buf + sizeof(char) + sizeof(struct iwreq)), &pid_real, sizeof(pid_t));

	ksocket_send(sock_sig, &addr_sig, buf, sizeof(buf));
}

static void ksocket_start(void)
{
	int size, err;
	char buf[MAXLENGTH];
	struct net_device * netdev;

	siginfo_t info;
	int sig;


	lock_kernel();
	kthread->running = 1;
	current->flags |= PF_NOFREEZE;

	daemonize(MODULE_NAME);
	allow_signal(SIGKILL);
	allow_signal(SIGIO);
	unlock_kernel();

	if ( ((err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock)) < 0)
	  || ((err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock_send)) < 0)
	  || ((err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock_sig)) < 0) )
	{
		printk(KERN_INFO MODULE_NAME": Could not create a datagram socket, error = %d\n", -ENXIO);
		goto out;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	memset(&addr_send, 0, sizeof(struct sockaddr_in));
	memset(&addr_sig, 0, sizeof(struct sockaddr_in));
	
	addr.sin_family      = AF_INET;
	addr_send.sin_family      = AF_INET;
	addr_sig.sin_family      = AF_INET;

	addr.sin_addr.s_addr      = htonl(INADDR_ANY);
	addr_send.sin_addr.s_addr      = htonl(INADDR_SEND);
	addr_sig.sin_addr.s_addr      = htonl(INADDR_SEND);
	
	addr.sin_port      = htons(DEFAULT_PORT);
	addr_send.sin_port      = htons(DEFAULT_PORT);
	addr_sig.sin_port      = htons(SIG_PORT);

	if ((err = sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr) ) ) < 0 )
	{
		printk(KERN_INFO MODULE_NAME": Could not bind or connect to socket, error = %d\n", -err);
		goto close_and_out;
	}

	for (;;)
	{
		memset(&buf, 0, MAXLENGTH);
		size = ksocket_receive(sock, &addr, buf, sizeof(buf));

		if (signal_pending(current)){
			sig = dequeue_signal (current, &current->blocked, &info);
			if(sig == SIGIO)
				RequestEvent();
			else
				break;
		}

		if (size < 0)
		{
			dbg(KERN_INFO MODULE_NAME": error getting datagram, sock_recvmsg error = %d\n", size);
		}
		else 
		{
			dbg(KERN_INFO MODULE_NAME": received %d bytes\n", size);
			run_ioctl(buf); 
			ksocket_send(sock_send, &addr_send, buf, sizeof(buf));
			dbg("send packet\n");
		}
	}

close_and_out:
	sock_release(sock);
	sock = NULL;
	sock_release(sock_send);
	sock_send = NULL;
	sock_release(sock_sig);
	sock_sig = NULL;

out:
	kthread->thread = NULL;
	kthread->running = 0;
}


static int wlan_ksocket_start()
{

	// start kernel thread
	kthread->thread = kthread_run((void *)ksocket_start, NULL, MODULE_NAME);
	if (IS_ERR(kthread->thread)) 
	{
		printk(KERN_INFO MODULE_NAME": unable to start kernel thread\n");
		kfree(kthread);
		kthread = NULL;
		return -ENOMEM;
	}

	return 0;
}

static void wlan_ksocket_end()
{
	int err;

	if (kthread->thread==NULL)
	{
		dbg(KERN_INFO MODULE_NAME": no kernel thread to kill\n");
	}
	else 
	{
		lock_kernel();
		err = kill_pid(find_get_pid(kthread->thread->pid), SIGKILL, 1);
		unlock_kernel();

		if (err < 0)
		{
			dbg(KERN_INFO MODULE_NAME": unknown error %d while trying to terminate kernel thread\n",-err);
		}
		else 
		{
			while (kthread->running == 1)
				msleep(10);
			dbg(KERN_INFO MODULE_NAME": succesfully killed kernel thread!\n");
		}
	}

	if (sock != NULL) 
	{
		sock_release(sock);
		sock = NULL;
	}
	if (sock_send != NULL) 
	{
		sock_release(sock_send);
		sock_send = NULL;
	}
	if (sock_sig != NULL) 
	{
		sock_release(sock_sig);
		sock_sig = NULL;
	}

	//printk(KERN_INFO MODULE_NAME": module unloaded\n");
}
static int __init wlan_ksocket_init()
{
	int err;
	struct net_device * netdev;

	kthread = kmalloc(sizeof(struct kthread_t), GFP_KERNEL);
	memset(kthread, 0, sizeof(struct kthread_t));

	printk(KERN_INFO MODULE_NAME": version %s\n", DRV_VERSION);
	
	return wlan_ksocket_start();
}

static void __exit wlan_ksocket_exit()
{
	wlan_ksocket_end();

	kfree(kthread);
	kthread = NULL;

	printk(KERN_INFO MODULE_NAME": module unloaded\n");
}

/* init and cleanup functions */
module_init(wlan_ksocket_init);
module_exit(wlan_ksocket_exit);

/* module information */
MODULE_DESCRIPTION("wlan device kernel socket");
MODULE_AUTHOR("Paula Tseng <pntseng@realtek.com>");
MODULE_LICENSE("GPL");


