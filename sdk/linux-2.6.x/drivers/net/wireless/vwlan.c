#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

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
#include <linux/mutex.h>


#include <linux/delay.h>

#include <linux/wireless.h>
#include <linux/vmalloc.h>
#include <net/sock.h>



#define DEFAULT_PORT 	2325
#define SIG_PORT 	2313
#define MODULE_NAME 	"vwlan"
#define DRV_VERSION 	"0.1.0"
#define SIOCGIWPRIV 	0x8B0D
#define SIOCSAPPPID     0x8b3e
#define SIOCGIWNAME     0x8B01
#define SIOCGMIIPHY		0x8947
//#ifdef SUPPORT_TX_MCAST2UNI
#define SIOCGIMCAST_ADD			0x8B80
#define SIOCGIMCAST_DEL			0x8B81
//#endif
#define CMD_TIMEOUT 	5*HZ
#define RCV_TIMEOUT		2*HZ
#define CMD_IOCTL		0
#define CMD_OPEN		1
#define CMD_CLOSE		2
#define CMD_SETHWADD	3
#define CMD_GETSTATE	4
#define CMD_FORCESTOP	5
#define	ETHER_ADDRLEN	6
#define ETHER_HDRLEN 	14

//#define DEBUG_VWLAN
#ifdef DEBUG_VWLAN
#define dbg(fmt, args...) printk(fmt, ##args)
#else
#define dbg(fmt, args...) {}
#endif	

#define INADDR_SEND ((unsigned long int)0x0AFDFD02) //10.253.253.2
#define INADDR_RECV ((unsigned long int)0x0AFDFD01) //10.253.253.1

#define MAXLENGTH 	(64000)	
#define DATALENGTH	(63900)

#define LENGTH		56
#define IFNAMSIZ	16

static struct socket *sock;
static struct socket *sock_send;
static struct sockaddr_in addr;
static struct sockaddr_in addr_send;

static struct proc_dir_entry *proc_wlan1;
static struct proc_dir_entry *proc_vwlan;

#ifdef CONFIG_RTL_VAP_SUPPORT
#define VAP_NUM			4
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
#endif
#ifdef CONFIG_RTL_WDS_SUPPORT
#define MAX_WDS_NUM 	8
#endif

static DEFINE_MUTEX(pm_mutex);

int vwlan_open(struct net_device *dev);
int vwlan_close(struct net_device *dev);
int vwlan_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
static int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
static int ksocket_receive(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
static void signal_socket_start();
static void signal_socket_end();

typedef struct vwlan_packet
{
	char cmd_type;
	struct iwreq wrq;
	int cmd;
	char ret;
	char data[DATALENGTH];
} vwlan_packet_t;

typedef struct vwlan_sig_packet
{
	char ret;
	char data[sizeof(pid_t)];
} vwlan_sig_packet_t;

typedef struct vwlan_if_packet
{
	char cmd_type;
	char ifname[IFNAMSIZ];
	char hwaddr[ETHER_ADDRLEN*3];
} vwlan_if_packet_t;

#define VWLAN_PLEN	(sizeof(vwlan_packet_t) - DATALENGTH)

char buffer[MAXLENGTH];

typedef struct vwlan_ioctl_data
{
	pid_t user_pid;
	int timeout;
} vwlan_ioctl_data_t;

struct kthread_t
{
        struct task_struct *thread;
        struct socket *sock;
        struct sockaddr_in addr;
        int running;
};

static struct kthread_t *kthread = NULL;

int vwlan_open_send(char *devname)
{
	char buf[sizeof(vwlan_if_packet_t)];
	vwlan_if_packet_t *vp;
	vp = (vwlan_if_packet_t *)buf;

	memset(buf, '\0', sizeof(buf)); 

	vp->cmd_type = CMD_OPEN;
	memcpy(vp->ifname, devname, IFNAMSIZ);
	memset(vp->ifname + 4, '0', 1);

	ksocket_send(sock_send, &addr_send, buf, sizeof(buf));
	
}

int vwlan_open(struct net_device *dev)
{
	vwlan_open_send(dev->name);
	return 0;
}

int vwlan_close_send(char *devname)
{
	char buf[sizeof(vwlan_if_packet_t)];
	vwlan_if_packet_t *vp;
	vp = (vwlan_if_packet_t *)buf;

	memset(buf, '\0', sizeof(buf)); 

	vp->cmd_type = CMD_CLOSE;
	memcpy(vp->ifname, devname, IFNAMSIZ);
	memset(vp->ifname + 4, '0', 1);

	ksocket_send(sock_send, &addr_send, buf, sizeof(buf));
}


int vwlan_close(struct net_device *dev)
{
	vwlan_close_send(dev->name);
	return 0;
}

void force_stop_vwlan_hw()
{
	char buf[LENGTH];
	char type = CMD_FORCESTOP;

	memset(buf, '\0', sizeof(buf)); 

	memcpy(buf, &type, sizeof(char));

	ksocket_send(sock_send, &addr_send, buf, LENGTH);

	printk("stop vwlan\n");
}

int  vwlan_set_hwaddr(struct net_device *dev, void *addrs)
{
	char buf[sizeof(vwlan_if_packet_t)];
	unsigned char ethaddr[ETHER_ADDRLEN];
	vwlan_if_packet_t *vp;
	struct sockaddr *saddr = addrs;
	int hwaddr_ind = 1;	

	if (!is_valid_ether_addr(saddr->sa_data))
			return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, saddr->sa_data, ETHER_ADDRLEN);
	memcpy(ethaddr, saddr->sa_data, ETHER_ADDRLEN);
	if(!strcmp(dev->name,"wlan1")){
#ifdef	CONFIG_RTL_VAP_SUPPORT
		hwaddr_ind += VAP_NUM;
#endif

#ifdef CONFIG_RTL_WDS_SUPPORT
		hwaddr_ind += (MAX_WDS_NUM);
#endif
		ethaddr[ETHER_ADDRLEN-1] += hwaddr_ind;
	}
	//clear buf
	memset(buf, '\0', sizeof(buf)); 
	vp = (vwlan_if_packet_t *) buf;

	vp->cmd_type = CMD_SETHWADD;
	memcpy(vp->ifname, dev->name, IFNAMSIZ);
	memset(vp->ifname + 4, '0', 1);
    dbg("%s %s: Setting MAC address to %pM\n", __FILE__, dev->name, ethaddr);
	sprintf(vp->hwaddr, "%pM", ethaddr);
	
	ksocket_send(sock_send, &addr_send, buf, sizeof(buf));
	
	return 0;
}

static void poll_func(unsigned long data) {
	vwlan_ioctl_data_t *vwlan_ioctl_d = data;
	printk("%s timeout!\n", __FILE__);
	vwlan_ioctl_d->timeout = 1;
	//kill_pid(find_get_pid(user_pid), SIGTERM, 1);
}

int vwlan_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	
	
	int size=-1, err, i;
	char type = CMD_IOCTL;
	int ret;
	struct iw_priv_args priv[48];
	struct iwreq *wrq = (struct iwreq *) ifr;
	struct iwreq twrq;
	struct timer_list tmr_poll;
	int send_len = VWLAN_PLEN;
	vwlan_packet_t *vwlan_pkt;
	vwlan_ioctl_data_t vwlan_ioctl_cmd;

	if(cmd == SIOCGIMCAST_ADD || cmd == SIOCGIMCAST_DEL){
		return 0;
	}

	if (!mutex_trylock(&pm_mutex)){
		return -1;
	}
	
	//change device name to wlan0
	//sprintf(wrq->ifr_name, "wlan0");
	memset(wrq->ifr_name + 4, '0', 1);
	//dbg("ifname %s\n", wrq->ifr_name);
	
	vwlan_pkt = (vwlan_packet_t *)(buffer);
	
	//clear buf
	memset(vwlan_pkt, 0, MAXLENGTH); 

	//cp type to buf
	vwlan_pkt->cmd_type = type;

	// cp iwreq struct to buf
	memcpy(&vwlan_pkt->wrq, wrq, sizeof(struct iwreq)); 

	// cp cmd to buf
	vwlan_pkt->cmd = cmd;
	dbg("cmd: %x\n", cmd);
	
	switch (cmd){
		case SIOCGIWNAME:
		case SIOCGMIIPHY:
		case SIOCGIWPRIV:
			break;
		default:			
			copy_from_user(vwlan_pkt->data, (void *)wrq->u.data.pointer, wrq->u.data.length); 
			send_len += wrq->u.data.length;
			break;
	}
		
	// send packet with format: iwreq / cmd / iwreq data content

	//ksocket_send(sock_send, &addr_send, buf, MAXLENGTH);
	size = ksocket_send(sock_send, &addr_send, buffer, send_len);
	if(size < 0){
		dbg("%s send failed\n", __FILE__);
		ret = -1;
		goto UN_LOCK;
	}

	//start timer
	//get user pid
    //vwlan_ioctl_cmd.user_pid = current->pid;
    //printk("user pid %d \n", vwlan_ioctl_cmd.user_pid);
	vwlan_ioctl_cmd.timeout = 0;
	init_timer(&tmr_poll);
	//tmr_poll.data = 0;
	tmr_poll.data = &vwlan_ioctl_cmd;
	tmr_poll.function = poll_func;	
	mod_timer(&tmr_poll, jiffies + CMD_TIMEOUT);
recieve_pkt:	

	//clear buf
	memset(buffer, 0, MAXLENGTH);
	
	// receive packet with format: ret / iwreq / iwreq data content
	size = ksocket_receive(sock, &addr, buffer, MAXLENGTH);

	if(size >= 0){
		dbg("recieved\n");
		//del timer if receive
		del_timer(&tmr_poll);
		
		if(cmd == SIOCSAPPPID){
			signal_socket_end();
			signal_socket_start();
		}

		//check if ioctl failed
		if(vwlan_pkt->ret==0) {
			dbg("%s ioctl failed\n", __FILE__);
			ret = -1;
			goto UN_LOCK;
		}

		//temporarily copy ioctl result
		memcpy(&twrq, &vwlan_pkt->wrq, sizeof(struct iwreq));
	
		// cp iwreq data content from recieved packet
		switch (cmd){
			case SIOCGIWPRIV:
				copy_to_user((void *)wrq->u.data.pointer, vwlan_pkt->data, sizeof(priv));
				break;
			case SIOCGIWNAME:
				copy_to_user((void *)wrq->u.name, twrq.u.name, sizeof(twrq.u.name));
				ret = 0;
				goto UN_LOCK;
			default:
				copy_to_user((void *)wrq->u.data.pointer, vwlan_pkt->data, twrq.u.data.length);
				break;
		}
		// cp iwreq data length & flags from recieved packet
		__put_user(twrq.u.data.length, &wrq->u.data.length);
		__put_user(twrq.u.data.flags, &wrq->u.data.flags);
		ret = 0;
		goto UN_LOCK;
	}
	else{
		dbg("error getting datagram, sock_recvmsg error = %d\n", size);
		
		if(vwlan_ioctl_cmd.timeout==1) {
			ret = -1;
			goto UN_LOCK;
		}
		
		if(size==(-EAGAIN)){ //rcv timeout
			del_timer(&tmr_poll);
			ret = -1;
			goto UN_LOCK;
		}
		else// if(size==(-ERESTARTSYS))
			goto recieve_pkt;
	}
UN_LOCK:
	mutex_unlock(&pm_mutex);
	return ret;
    
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
	struct sock *sk=sock->sk;
	sk->sk_rcvtimeo = RCV_TIMEOUT;

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

static void process_signal( vwlan_sig_packet_t *vp)
{
	pid_t pid;
	
	if(vp->ret == 2){ 
		memcpy(&pid, vp->data, sizeof(pid_t));
		dbg("send signal to pid (%d)\n", pid);
		kill_pid(find_get_pid(pid), SIGIO, 1);
	}
	else{
		dbg("do nothing!\n");
	}
}

static void ksocket_start(void)
{
	int size, err;
	char buf[sizeof(vwlan_sig_packet_t)];
	vwlan_sig_packet_t *vp;
	vp = (vwlan_sig_packet_t *) buf;

	lock_kernel();
	kthread->running = 1;
	current->flags |= PF_NOFREEZE;

	daemonize(MODULE_NAME);
	allow_signal(SIGKILL);
	unlock_kernel();
	
	if ( (err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &kthread->sock)) < 0)

	{
		printk(KERN_INFO MODULE_NAME": Could not create a datagram socket, error = %d\n", -ENXIO);
		goto out;
	}

	memset(&kthread->addr, 0, sizeof(struct sockaddr_in));
	kthread->addr.sin_family      = AF_INET;

	kthread->addr.sin_addr.s_addr     = htonl(INADDR_RECV);
	kthread->addr.sin_port      = htons(SIG_PORT);

	if ( (err = kthread->sock->ops->bind(kthread->sock, (struct sockaddr *)&kthread->addr, sizeof(struct sockaddr) ) ) < 0) 
	{
		printk(KERN_INFO MODULE_NAME": Could not bind or connect to socket, error = %d\n", -err);
		goto close_and_out;
	}

	for (;;)
	{
		memset(&buf, 0, sizeof(vwlan_sig_packet_t));
		size = ksocket_receive(kthread->sock, &kthread->addr, buf, sizeof(vwlan_sig_packet_t));

		if (signal_pending(current))
			break;

		if (size < 0)
		{
			dbg(KERN_INFO MODULE_NAME": error getting datagram, sock_recvmsg error = %d\n", size);
		}
		else 
		{
			dbg(KERN_INFO MODULE_NAME": received %d bytes\n", size);
			process_signal(vp); 
			
		}
	}

close_and_out:
	sock_release(kthread->sock);
	kthread->sock = NULL;

out:
	kthread->thread = NULL;
	kthread->running = 0;
}


static void signal_socket_start()
{

	/* start kernel thread */
	kthread->thread = kthread_run((void *)ksocket_start, NULL, MODULE_NAME);
	if (IS_ERR(kthread->thread)) 
	{
		printk(KERN_INFO MODULE_NAME": unable to start kernel thread\n");
		kfree(kthread);
		kthread = NULL;
		//return -ENOMEM;
	}

	//return 0;
}

static void signal_socket_end()
{
	int err;

	if(kthread->thread == NULL)
	{
		dbg(KERN_INFO MODULE_NAME": no kernel thread to kill\n");
	}
	else 
	{
		lock_kernel();
		err = kill_pid(find_get_pid(kthread->thread->pid), SIGKILL, 1);
		unlock_kernel();

		if (err < 0)
			printk(KERN_INFO MODULE_NAME": unknown error %d while trying to terminate kernel thread\n",-err);
		else 
		{
			while (kthread->running == 1)
				msleep(10);
			dbg(KERN_INFO MODULE_NAME": succesfully killed kernel thread!\n");
		}
	}

	if (kthread->sock != NULL) 
	{
		sock_release(kthread->sock);
		kthread->sock = NULL;
	}

}

/*Handle all user/system command from outside */
static int proc_vwlan_write(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	static char write_value='0'; 
    if (count < 2)
		return -EFAULT;
	if (buffer && !copy_from_user(&write_value, buffer, 1)) 
	{
		/*value = 1 is force slave wlan down*/
		if(write_value=='1')
		{  
			force_stop_vwlan_hw();
            return count;
        }
		else if(write_value=='2')
		{
			vwlan_open_send("wlan1");
		}
		else if(write_value=='3')
		{
			vwlan_close_send("wlan1");
		}
		/**/
	}
    return -EFAULT;
}



static int __init vwlan_init()
{
	int err;

	kthread = kmalloc(sizeof(struct kthread_t), GFP_KERNEL);
	memset(kthread, 0, sizeof(struct kthread_t));

  	if ( ( (err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock)) < 0) 
			||
		 ( (err = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock_send)) < 0 ))
	{
		printk(KERN_INFO MODULE_NAME": Could not create a datagram socket, error = %d\n", -ENXIO);
		goto out;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	memset(&addr_send, 0, sizeof(struct sockaddr_in));
	addr.sin_family      = AF_INET;
	addr_send.sin_family = AF_INET;

	addr.sin_addr.s_addr      = htonl(INADDR_RECV);
	addr_send.sin_addr.s_addr = htonl(INADDR_SEND);

	addr.sin_port      = htons(DEFAULT_PORT);
	addr_send.sin_port = htons(DEFAULT_PORT);

	if ((err = sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr) ) ) < 0 )
	{
		printk(KERN_INFO MODULE_NAME": Could not bind or connect to socket, error = %d\n", -err);
		sock_release(sock);
		sock_release(sock_send);
		goto out;
	}


	proc_wlan1 = proc_mkdir("wlan1",NULL);
#ifdef CONFIG_RTL_VAP_SUPPORT
	proc_wlan1 = proc_mkdir("wlan1-vap0",NULL);
	proc_wlan1 = proc_mkdir("wlan1-vap1",NULL);
	proc_wlan1 = proc_mkdir("wlan1-vap2",NULL);
	proc_wlan1 = proc_mkdir("wlan1-vap3",NULL);
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
	proc_wlan1 = proc_mkdir("wlan1-vxd",NULL);
#endif
   	proc_vwlan = create_proc_entry(MODULE_NAME, 0, NULL);
    	if(proc_vwlan) 
	{
        	proc_vwlan->write_proc = proc_vwlan_write;
    }


out:

	printk(KERN_INFO MODULE_NAME": version %s\n", DRV_VERSION);
	mutex_init(&pm_mutex);
	return 0;
}

static void __exit vwlan_exit()
{
	kfree(kthread);
    kthread = NULL;
	
	remove_proc_entry("wlan1",NULL);
#ifdef CONFIG_RTL_VAP_SUPPORT
	remove_proc_entry("wlan1-vap0",NULL);
	remove_proc_entry("wlan1-vap1",NULL);
	remove_proc_entry("wlan1-vap2",NULL);
	remove_proc_entry("wlan1-vap3",NULL);
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
	remove_proc_entry("wlan1-vxd",NULL);
#endif
	remove_proc_entry(MODULE_NAME, NULL);
	
	printk(KERN_INFO MODULE_NAME": module unloaded\n");

}

/* init and cleanup functions */
late_initcall(vwlan_init);
module_exit(vwlan_exit);

/* module information */
MODULE_DESCRIPTION("virtual wlan device driver for ioctl");
MODULE_AUTHOR("Paula Tseng <pntseng@realtek.com>");
MODULE_LICENSE("GPL");



