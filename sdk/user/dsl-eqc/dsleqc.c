/*
 *	DSL EQC program
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <linux/ip.h>

#include <rtk/utility.h>

struct cmd_struct {
	int id;
	int value;
};

#define BUFSIZE		1024
char	sendbuf[BUFSIZE];
char	recvbuf[BUFSIZE];
int	peer;
struct sockaddr_in toaddr, fromaddr;
int sendit = 0;
int test_case = 0;
const char usage_str[]="dsleqc [-s remote_ip] [-n test_number]";

// send command
void sig_alrm(int dummy)
{
	struct cmd_struct *cmd2send;
	#if 0
	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
	#endif
	
	cmd2send = (struct cmd_struct *)sendbuf;
	cmd2send->id = test_case;
	sendto(peer, sendbuf, sizeof(struct cmd_struct), 0, (struct sockaddr *)&toaddr, sizeof(toaddr));
	#if 0
	printf("send test_case %d\n", test_case);
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		printf("dls down\n");
	else
		printf("dsl up\n");
	#endif
	//alarm(1);
	return;
}

// read command, exit on test_case==0
void readloop(void)
{
	socklen_t len;
	int recvlen;
	struct iphdr *ip;
	int ipdatalen, iphdrlen;
	struct cmd_struct *cmd2recv;
	
	while (1) {
		recvlen = recvfrom(peer, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&fromaddr, &len);
		if (recvlen < 0) {
			if (errno == EINTR)
				continue;
			else
				printf("recvfrom error\n");
		}
		
		if (recvlen < sizeof(struct iphdr)) {
			printf("received packet too short (%u bytes) for IP header", recvlen);
			return;
		}
		
		// ip header + data
		ip  = (struct iphdr *)recvbuf;
		iphdrlen  = ip->ihl << 2;
		ipdatalen = ip->tot_len;
		// get data
		cmd2recv = (struct cmd_struct *)(recvbuf + iphdrlen);
		//printf("recv test_case %d\n", cmd2recv->id);
		if (!cmd2recv->id) // zero to exit
			break;
	}
}

int main(int argc, char **argv)
{
	int c;
	int ret;
	struct in_addr itf_addr;
	
	bzero(&toaddr, sizeof(toaddr));
	toaddr.sin_family = AF_INET;
	//toaddr.sin_addr.s_addr = 0xc0a80102;
	
	while ((c = getopt(argc, argv, "s:n:")) != -1) {
		switch (c) {
		case 's':
			sendit=1;
			inet_aton(optarg, &toaddr.sin_addr);
			break;
		case 'n':
			test_case = strtol(optarg, 0, 0);
			break;
		case '?':
			printf("Usage: %s\n", usage_str);
			return 0;
			break;
		default:
			break;
		}
	}
	
	if (sendit) {
		signal(SIGALRM, sig_alrm);
		alarm(1);
	}
	
	peer = socket(AF_INET, SOCK_RAW, 253);
	if (peer < 0) {
		printf("socket error\n");
		exit(1);
	}

	readloop();
	
	close(peer);
}

