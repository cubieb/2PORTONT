#include <stdio.h>
#include <syslog.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/filter.h>

#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"

int GetIfIndex(char *ifname)
{
	int fd;
	struct ifreq ifr;

	if(ifname)
	{
		fd = socket(PF_PACKET, SOCK_DGRAM, 0);

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

		if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
		{
			perror("ioctl(SIOCGIFINDEX)");
			return -1;
		}
		return ifr.ifr_ifindex;
	}
	else
		return -1;

}

int main()
{
	int sockfd, n, i, ret = 0;
	unsigned char buff[1024];
	struct sockaddr_ll servaddr;
	struct timeval tv;
	int sll_len = sizeof(servaddr);
	struct arphdr *a = (struct arphdr*)buff;
	unsigned char *ptr;
	unsigned char sd_mac_ip[] = {0x00, 0x11, 0x33, 0x55, 0x77, 0xcc, 10, 253, 253, 2, 0x00, 0x11, 0x33, 0x55, 0x77, 0xcc, 0, 0, 0, 0};
	unsigned char set_ip[64] = {0};
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	if((sockfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_RARP))) < 0)
	{
		printf("Socket error!\n");
		return 1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));  // Set timeout.

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sll_family = AF_PACKET;
	servaddr.sll_protocol = htons(ETH_P_RARP);
	servaddr.sll_halen = 6;
	servaddr.sll_ifindex = GetIfIndex("eth0");
	memcpy(servaddr.sll_addr, MAC_BCAST_ADDR, 6);

	a->ar_hrd = htons(ARPHRD_ETHER);
	a->ar_pro = htons(ETH_P_IP);
	a->ar_hln = servaddr.sll_halen;
	a->ar_pln = 4;
	a->ar_op = htons(ARPOP_RREQUEST);

	ptr = (unsigned char *)(a+1);
	memcpy(ptr, sd_mac_ip, 20);

    //shlee, retry at most 20 times.. 
    for(i=0; i< 20; i++){

	sendto(sockfd, buff, ptr+20-buff, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

	if((n = recvfrom(sockfd, buff, 512, 0, NULL, NULL)) >= 0)
	{
		int sz_arphdr = sizeof(*a);
		char slave_ip[20];
		sprintf(slave_ip, "%u.%u.%u.%u", (unsigned char)*(buff+sz_arphdr+16), (unsigned char)*(buff+sz_arphdr+17),
			(unsigned char)*(buff+sz_arphdr+18), (unsigned char)*(buff+sz_arphdr+19));

		printf("Slave IP=%s\n", slave_ip);
		sprintf(set_ip, "/bin/ifconfig br0 %s" , slave_ip);
		system(set_ip);
		//va_cmd("/bin/ifconfig", 2, 1, "eth0", soc2_ip);
        ret = 0;
        break;
	}
	else if (n < 0)
	{
		if( errno == EWOULDBLOCK )
		{
			ret = -1;
		}
		else
		{
			ret = 1;
		}
	}
    } 

    if(ret != 0){
        // Set IP as default 192.168.1.2
        printf("[RARP] Cannot get Slave IP, use the default 10.253.253.2\n");
	system("/bin/ifconfig br0 10.253.253.2");
    }

	close(sockfd);

	return ret;

}
