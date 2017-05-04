#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <linux/if.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <rtk/utility.h>

#ifdef CONFIG_LUNA
#include <rtk_rg_struct.h>
#endif

enum lbd_port_status
{
	NO_LOOPBACK = 0,
	LOOPBACK_DETECTED_DIABLED,
	LOOPBACK_DETECTED_ONLY,
};

struct lbd_data
{
	unsigned int int_magic;
	unsigned char ifnum;
	unsigned char str_magic[41];	//make data length be 46 bytes
} __attribute__((packed));

struct vlan_lbd_data
{
	unsigned short vid;	// only first 12 bits are valid
	unsigned short type;	// real ehternet type
	struct lbd_data data;
} __attribute__((packed));

struct lbd_lan_port
{
	int send_sock;
	int ifindex;
	int timerfd;
};

struct lbd_lan_port lan_if[ELANVIF_NUM] = {0};
struct vlan_lbd_data send_data = {0};
struct sockaddr_ll addr={0};

// arguments, name from e8 TR-069 spec.
unsigned int LoopExistPeriod = 5;
unsigned int LoopCancelPeriod = 300;
unsigned short EthernetType = 0xfffa;
unsigned short *vid_list = NULL;
//unsigned short vid_list[] = {0};	/*For debugging*/
int vid_size = 0;
unsigned char port_status[ELANVIF_NUM] = {0};

static char loopback_pidfile[] = "/var/run/loopback.pid";
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = loopback_pidfile;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

int load_lbd_config()
{
	int i;
	MIB_CE_LBD_VLAN_T entry = {0};
	int total = 0;

	mib_get(MIB_LBD_EXIST_PERIOD, &LoopExistPeriod);
	fprintf(stderr, "loopback: LoopExistPeriod = %u\n", LoopExistPeriod);
	
	mib_get(MIB_LBD_CANCEL_PERIOD, &LoopCancelPeriod);
	fprintf(stderr, "loopback: LoopCancelPeriod = %u\n", LoopCancelPeriod);

	mib_get(MIB_LBD_ETHER_TYPE, &EthernetType);
	fprintf(stderr, "loopback: EthernetType = 0x%X\n", EthernetType);

	total = mib_chain_total(MIB_LBD_VLAN_TBL);
	if(total == 0)
		return -1;

	vid_list = malloc(sizeof(unsigned short) * total);
	if(!vid_list)
		return -1;

	fprintf(stderr, "loopback: vid = ");
	for(i = 0 ; i < total ; i++)
	{
		if(mib_chain_get(MIB_LBD_VLAN_TBL, i, &entry) < 0)
			continue;

		vid_list[vid_size++] = entry.vid;

		if(i > 0)
			fprintf(stderr, ",");
		
		if(entry.vid == 0)
			fprintf(stderr, "untagged");
		else
			fprintf(stderr, "%hu", entry.vid);
	}
	fprintf(stderr, "\ntotal %d vlan values\n", vid_size);

	if(vid_size == 0)
		return -1;

	return 0;
}

#ifdef CONFIG_LUNA
#ifndef CONFIG_RTL9602C_SERIES
rtk_rg_port_idx_t rg_port[] = {RTK_RG_PORT0, RTK_RG_PORT1, RTK_RG_PORT2, RTK_RG_PORT3};
#else
rtk_rg_port_idx_t rg_port[] = {RTK_RG_PORT0, RTK_RG_PORT1};
#endif
int acl_idx, acl_vlan_idx;

void lbd_rg_exit(void)
{
	int i;
	rtk_rg_cvlan_info_t cvlan_info = {0};

	for(i = 0 ; i < vid_size ; i++)
	{
		if(vid_list[i] != 0)
		{
			cvlan_info.vlanId = vid_list[i];

			if(rtk_rg_cvlan_get(&cvlan_info) == RT_ERR_RG_OK
				&& (cvlan_info.memberPortMask.portmask & (1<<RTK_RG_PORT_PON)))
			{
				// This vid is used by WAN interface
				cvlan_info.memberPortMask.portmask &= ~(RG_get_lan_phyPortMask(0xf));
				cvlan_info.untagPortMask.portmask |= (RG_get_lan_phyPortMask(0xf));
				rtk_rg_cvlan_add(&cvlan_info);
			}
			else
				rtk_rg_cvlan_del(vid_list[i]);
		}
	}

	// Remove the ACL rule we added
	rtk_rg_aclFilterAndQos_del(acl_idx);
	rtk_rg_aclFilterAndQos_del(acl_vlan_idx);
	system("echo stop > /proc/loopback_detect");
}

void lbd_rg_init(void)
{
	int i;
	rtk_rg_cvlan_info_t cvlan_info = {0};
	rtk_rg_aclFilterAndQos_t acl = {0};
	char cmd[256] = {0};

	// Let LAN port can send/receive configured vlan
	for(i = 0 ; i < vid_size ; i++)
	{
		if(vid_list[i] != 0)
		{
			cvlan_info.vlanId = vid_list[i];
			rtk_rg_cvlan_get(&cvlan_info);

			cvlan_info.memberPortMask.portmask |= (RG_get_lan_phyPortMask(0xf)|(1<<RTK_RG_PORT_CPU));
			cvlan_info.untagPortMask.portmask &= ~(RG_get_lan_phyPortMask(0xf));
			rtk_rg_cvlan_add(&cvlan_info);
		}
	}

	acl.fwding_type_and_direction = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
	acl.filter_fields = INGRESS_ETHERTYPE_BIT | INGRESS_PORT_BIT | INGRESS_DMAC_BIT | INGRESS_SMAC_BIT;
	acl.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
	acl.ingress_ethertype = EthernetType;
	acl.ingress_dmac.octet[0] = 0xff;
	acl.ingress_dmac.octet[1] = 0xff;
	acl.ingress_dmac.octet[2] = 0xff;
	acl.ingress_dmac.octet[3] = 0xff;
	acl.ingress_dmac.octet[4] = 0xff;
	acl.ingress_dmac.octet[5] = 0xff;
	mib_get(MIB_ELAN_MAC_ADDR, acl.ingress_smac.octet);
	acl.action_type = ACL_ACTION_TYPE_TRAP_TO_PS;

	if(rtk_rg_aclFilterAndQos_add(&acl, &acl_idx))
	{
		perror("rtk_rg_aclFilterAndQos_add");
		exit(-1);
	}


	acl.ingress_ethertype = 0x8100;
	if(rtk_rg_aclFilterAndQos_add(&acl, &acl_vlan_idx))
	{
		perror("rtk_rg_aclFilterAndQos_add vlan");
		exit(-1);
	}
	
	// init shareMeter
	rtk_rg_shareMeter_set (30,300,ENABLED);  // for broadcast
	rtk_rg_shareMeter_set (29,3000,ENABLED); // for multicast
	system("echo 30 > /proc/rg/BC_rate_limit");
	system("echo 29 > /proc/rg/IPv4_MC_rate_limit");
	system("echo 29 > /proc/rg/IPv6_MC_rate_limit");

	snprintf(cmd, sizeof(cmd), "echo 0x%x > /proc/rg/BC_rate_limit_portMask", RG_get_lan_phyPortMask(0xf));
	system(cmd);
	snprintf(cmd, sizeof(cmd), "echo 0x%x > /proc/rg/IPv4_MC_rate_limit_portMask", RG_get_lan_phyPortMask(0xf));
	system(cmd);
	snprintf(cmd, sizeof(cmd), "echo 0x%x > /proc/rg/IPv6_MC_rate_limit_portMask", RG_get_lan_phyPortMask(0xf));
	system(cmd);
}

void rg_ifconfig(int ifnum, int enable)
{
	uint32 value;
	rtk_rg_port_idx_t port = RG_get_lan_phyPortId(ifnum);

	if(enable)
	{
		rtk_port_phyReg_get(port, 0, 0, &value);
		value &= ~(0x0800); /* Power down bit in standard PHY standard register */
		rtk_port_phyReg_set(port, 0, 0, value);

	}
	else
	{
		rtk_port_phyReg_get(port, 0, 0, &value);
		value |= 0x0800; /* Power down bit in standard PHY standard register */
		rtk_port_phyReg_set(port, 0, 0, value);
	}
}
#endif

void disable_interface(int ifnum)
{
	struct itimerspec timer = {0}, dummy = {0};

	// If a loopbacke occur, we may detect loopback many times
	// Only diable an interface per round
	if(port_status[ifnum] == NO_LOOPBACK)
	{
#ifdef CONFIG_LUNA
		// move disable interface(mac) in kernel space
		//rg_ifconfig(ifnum, 0);
		fprintf(stderr, "Loop detected on %s\n", ELANVIF[ifnum]);
#else
		va_cmd(IFCONFIG, 2, 1, ELANVIF[ifnum], "down");
#endif
		timer.it_value.tv_sec = LoopCancelPeriod;
		port_status[ifnum] = LOOPBACK_DETECTED_DIABLED;
		mib_set(MIB_RS_LBD_PORT_STATUS, port_status);
		timerfd_settime(lan_if[ifnum].timerfd, 0, &timer, &dummy);
	}
}

void enable_interface(int ifnum)
{
	port_status[ifnum] = NO_LOOPBACK;
	mib_set(MIB_RS_LBD_PORT_STATUS, port_status);

#ifdef CONFIG_LUNA
	system("echo disableg > /proc/loopback_detect");
	system("echo start > /proc/loopback_detect");
	rg_ifconfig(ifnum, 1);
	#include <unistd.h>
	sleep(2);
	system("echo enableg > /proc/loopback_detect");
#else
	va_cmd(IFCONFIG, 2, 1, ELANVIF[ifnum], "up");
#endif
}

void quit_signal(int signo)
{
	int i;

#ifdef CONFIG_LUNA
	lbd_rg_exit();
#endif

	// enable disabled interfaces before exit
	for(i = 0 ; i < ELANVIF_NUM; i++)
	{
		if(port_status[i] == LOOPBACK_DETECTED_DIABLED)
		{
			enable_interface(i);
		}
	}

	unlink(loopback_pidfile);
	exit(0);
}

const unsigned int int_magic = 0xdeadbeef;
const unsigned char str_magic[] = "realtek_loopback_detect_packet";
void lbd_init(int epollfd)
{
	int i = 0;
	struct ifreq ifr;
	const static unsigned char broadcast_addr[]= {0xff,0xff,0xff,0xff,0xff,0xff};
	struct epoll_event ev;

	// prepare common data to send
	send_data.data.int_magic = 0xdeadbeef;
	strcpy(send_data.data.str_magic, str_magic);

	// prepare common destination
	addr.sll_family = AF_PACKET;
	addr.sll_halen = ETHER_ADDR_LEN;
	memcpy(addr.sll_addr, broadcast_addr, ETHER_ADDR_LEN);

	ev.events = EPOLLIN;

	for(i = 0 ; i < ELANVIF_NUM ; i++)
	{
		//prepare socket for sending packets
		lan_if[i].send_sock = socket(AF_PACKET, SOCK_DGRAM, EthernetType);
		if( lan_if[i].send_sock < 0)
		{
			fprintf(stderr, "Open socket error [%d].\n", i);
			exit(-1);
		}

		strncpy(ifr.ifr_name, ELANVIF[i], sizeof(ifr.ifr_name));

		if (ioctl(lan_if[i].send_sock, SIOCGIFINDEX, &ifr)==-1)
		{
			fprintf(stderr, "Get %s ifindex failed: %s\n", ifr.ifr_name, strerror(errno));
			exit(-1);
		}
		lan_if[i].ifindex = ifr.ifr_ifindex;

		//Initialize check timer
		lan_if[i].timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
		if(lan_if[i].timerfd < 0)
		{
			perror("Invalid check_timerfd\n");
			exit(-1);
		}

		// Add timer to epoll
		ev.data.fd = lan_if[i].timerfd;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lan_if[i].timerfd, &ev) == -1) 
		{
			fprintf(stderr, "epoll_ctl: lan_if[%d].timerfd\n", i);
			exit(-1);
		}
	}
}

void send_lbd_packets(int lan_idx)
{
	int i;

	if(lan_idx < 0 || lan_idx >= ELANVIF_NUM)
	{
		fprintf(stderr, "<%s:%d> Invalid LAN interface: %d\n", __func__, __LINE__, lan_idx);
		return ;
	}

	if(port_status[lan_idx] != NO_LOOPBACK)
		return;

	send_data.data.ifnum = lan_idx;
	addr.sll_ifindex = lan_if[lan_idx].ifindex;

	for(i = 0 ; i < vid_size ; i++)
	{
		if(vid_list[i] == 0)
		{
			addr.sll_protocol = htons(EthernetType);

			// Send data without vlan tag
			if(sendto(lan_if[lan_idx].send_sock, &send_data.data, sizeof(struct lbd_data), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
				fprintf(stderr, "Send failed: %s\n", strerror(errno));
				continue;
			}
		}
		else
		{
			addr.sll_protocol = htons(ETH_P_8021Q);

			send_data.vid = htons(vid_list[i]);
			send_data.type = htons(EthernetType);

			// Send data with vlan tag
			if(sendto(lan_if[lan_idx].send_sock, &send_data, sizeof(struct vlan_lbd_data), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
				fprintf(stderr, "Send failed: %s\n", strerror(errno));
				continue;
			}
		}
	}
}

void check_loopback_detect(void)
{
	FILE *fp = NULL;
	int i = 0xff;
	fp=fopen("/proc/loopback_detect","r");
        if (fp)
        {
		fscanf(fp,"%d",&i);
		if(i != 0xff && i >= 0)
			disable_interface(i);
		fclose(fp);
	}
}

#ifndef CONFIG_LUNA
int prepare_recv_socket(int is_vlan)
{
	int fd;
	struct ifreq ifr = {0};
	int ifindex;
	struct sockaddr_ll sll;

	fd = socket(AF_PACKET, SOCK_DGRAM, EthernetType);

	if(fd < 0)
	{
		fprintf(stderr, "<%s:%d> Create socket failed.\n", __func__, __LINE__);
		return fd;
	}

	strncpy(ifr.ifr_name, ALIASNAME_BR0, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFINDEX, &ifr)==-1)
	{
		fprintf(stderr, "<%s:%d> Get br0 ifindex failed: %s\n", __func__, __LINE__, strerror(errno));
		close(fd);
		return -1;
	}

	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_pkttype = PACKET_BROADCAST;

	if(is_vlan)
		sll.sll_protocol = htons(ETH_P_8021Q);
	else
		sll.sll_protocol = htons(EthernetType);

	bind(fd, (struct sockaddr *)&sll, sizeof (sll));

	return fd;
}

int check_lbd_data(struct lbd_data *data)
{
	//printf("Start to check lbd_data\n");

	if(data->int_magic != int_magic)
	{
		//printf("Invalid magic number\n");
		return -1;
	}

	if(strcmp(data->str_magic, str_magic) != 0)
	{
		//printf("Invalid magic string\n");
		return -1;
	}

	if(data->ifnum >= ELANVIF_NUM)
	{
		//printf("Invalid port number\n");
		return -1;
	}

	return data->ifnum;
}
int check_vlan_lbd_data(struct vlan_lbd_data *data)
{
	int i;

	for(i = 0 ; i <= vid_size ; i++)
	{
		if(data->vid == vid_list[i])
			break;
	}

	if(i >= vid_size)
	{
		//printf("Invalid vid\n");
		return -1;
	}

	if(data->type != EthernetType)
	{
		//printf("Invalid ether type\n");
		return -1;
	}

	return check_lbd_data(&data->data);
}
#endif //#ifndef CONFIG_LUNA
int main(int argc, char *argv[])
{
	int i;
	// socket for capture packets
	int recv_socket;
	int recv_vlan_socket;

	// epoll
#define MAX_EPOLL_EVENT 10
	struct epoll_event ev, events[MAX_EPOLL_EVENT];
	int epollfd;

	//timers
	int send_timerfd;
	struct itimerspec timer = {0}, dummy = {0};

	log_pid();

	if(load_lbd_config() < 0)
	{
		perror("load_lbd_config\n");
		unlink(loopback_pidfile);
		exit(0);
	}

	// initialize epoll
	epollfd = epoll_create(MAX_EPOLL_EVENT);
	if(epollfd < 0)
	{
		perror("epoll_create\n");
		unlink(loopback_pidfile);
		exit(0);
	}

	lbd_init(epollfd);
#ifdef CONFIG_LUNA
	lbd_rg_init();
	system("echo start > /proc/loopback_detect");
#endif

	signal (SIGINT,  quit_signal);
	signal (SIGTERM, quit_signal);
	signal (SIGQUIT, quit_signal);

	//initialize timers
	send_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if(send_timerfd < 0)
	{
		perror("Invalid send_timerfd\n");
		raise(SIGTERM);
	}
#ifndef CONFIG_LUNA
	// initialize packet listener
	recv_socket = prepare_recv_socket(0);
	if(recv_socket < 0)
	{
		perror("prepare_recv_socket 0\n");
		raise(SIGTERM);
	}
	recv_vlan_socket = prepare_recv_socket(1);
	if(recv_vlan_socket < 0)
	{
		perror("prepare_recv_socket 1\n");
		raise(SIGTERM);
	}
#endif
	// add fds to epoll
	ev.events = EPOLLIN;
	ev.data.fd = send_timerfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, send_timerfd, &ev) == -1) 
	{
		perror("epoll_ctl: send_timerfd\n");
		raise(SIGTERM);
	}
#ifndef CONFIG_LUNA
	ev.data.fd = recv_socket;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, recv_socket, &ev) == -1) 
	{
		perror("epoll_ctl: recv_socket\n");
		raise(SIGTERM);
	}

	ev.data.fd = recv_vlan_socket;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, recv_vlan_socket, &ev) == -1) 
	{
		perror("epoll_ctl: recv_vlan_socket\n");
		raise(SIGTERM);
	}
#endif // #ifndef CONFIG_LUNA
	// Start timer for sending LBD packet
	timer.it_value.tv_sec = 1;				//send first round ASAP.
	timer.it_interval.tv_sec = LoopExistPeriod;
	if( timerfd_settime(send_timerfd, 0, &timer, &dummy) <0)
    {
		perror("timerfd_settime: set timer failed\n");
		raise(SIGTERM);
    } 
	
	while(1)
	{
		int num_ev;

		num_ev = epoll_wait(epollfd, events, MAX_EPOLL_EVENT, -1);
		if(num_ev > 0)
		{
			for(i = 0 ; i < num_ev ; i++)
			{
				static struct timeval last_disabled = {0};
#ifndef CONFIG_LUNA
				if(events[i].data.fd == recv_socket)
				{
					struct lbd_data buf = {0};
					int len;
					struct timeval now = {0};

					len = recv(recv_socket, &buf, sizeof(struct lbd_data), MSG_TRUNC);
					//printf("recv len = %d\n", len);

					gettimeofday(&now, NULL);

					// When there is a loopback, we may find 2 interfaces in the loopback.
					// To avoid turn both interfaces down, we only disable a interface in a round.
					if(len == sizeof(struct lbd_data)
						&& now.tv_sec - last_disabled.tv_sec >= LoopExistPeriod)
					{
						int loop_if = check_lbd_data(&buf);
						if( loop_if >= 0)
						{
							disable_interface(buf.ifnum);
							gettimeofday(&last_disabled, NULL);
							fprintf(stderr, "Loop detected on %s\n", ELANVIF[buf.ifnum]);
						}
					}
				}
				else if(events[i].data.fd == recv_vlan_socket)
				{
					struct vlan_lbd_data buf = {0};
					int len;
					struct timeval now = {0};

					len = recv(recv_vlan_socket, &buf, sizeof(struct vlan_lbd_data), MSG_TRUNC);
					//printf("recv vlan len = %d\n", len);

					gettimeofday(&now, NULL);

					// When there is a loopback, we may find 2 interfaces in the loopback.
					// To avoid turn both interfaces down, we only disable a interface per round.
					if(len == sizeof(struct vlan_lbd_data)
						&& now.tv_sec - last_disabled.tv_sec >= LoopExistPeriod)
					{
						int loop_if = check_vlan_lbd_data(&buf);
						if( loop_if >= 0)
						{
							disable_interface(buf.data.ifnum);
							gettimeofday(&last_disabled, NULL);
							fprintf(stderr, "Loop detected on %s, vid=%d\n", ELANVIF[buf.data.ifnum], buf.vid);
						}
					}
				}
				else 
#endif //#ifndef CONFIG_LUNA
				if(events[i].data.fd == send_timerfd)
				{
					int j;
					uint64_t value;

					//read to clear this event
					read(send_timerfd, &value, sizeof(uint64_t));

					//send LBD packets from each LAN port
					for(j = 0 ; j < ELANVIF_NUM ; j++)
						send_lbd_packets(j);
					#ifdef CONFIG_LUNA
					// check loopback is detect or not each round
					check_loopback_detect();
					#endif
				}
				else
				{
					int j;
					uint64_t value;

					for(j = 0 ; j < ELANVIF_NUM; j++)
					{
						//find the interface which is time out.
						if(events[i].data.fd == lan_if[j].timerfd)
						{
							// Timeout, enable the disabled LAN interface.
							// If the interface is still in loopback status,
							// it will be disabled again when next LBD packets are sent.
					
							//read to clear this event
							read(lan_if[j].timerfd, &value, sizeof(uint64_t));

							enable_interface(j);

							printf("%s is re-enabled to check loopback again\n", ELANVIF[j]);
							break;
						}
					}
					if(j >= ELANVIF_NUM)
						fprintf(stderr, "<%d:%s> FIX ME! Unknown fd %d\n", events[i].data.fd);
				}
			}
		}
	}

	return 0;
}

