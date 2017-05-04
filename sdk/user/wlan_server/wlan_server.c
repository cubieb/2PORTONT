/* wlan udp server doing following things:
 * 1. recieve udp packet with wlan interface ioctl request 
 * 2. process ioctl in local
 * 3. send back the result of ioctl
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/autoconf.h>
#include "../auth/src/dlisten/iwcommon.h"

#define VERSION "0.1.0"

#define SERV_PORT 		2325
#define SIG_PORT 		2313
#define MAXLENGTH 	(64000)	
#define DATALENGTH	(63900)
#define SIOCGIWNAME				0x8B01
#define SIOCGIWPRIV 			0x8B0D
#define SIOCGIWRTLSTAINFO		0x8B30
#define SIOCSAPPPID     		0x8b3e
#define SIOCGMIIPHY 			0x8947
//#ifdef SUPPORT_TX_MCAST2UNI
#define SIOCGIMCAST_ADD			0x8B80
#define SIOCGIMCAST_DEL			0x8B81
//#endif
#define CMD_TIMEOUT 	5*HZ
#define CMD_IOCTL		0
#define CMD_OPEN		1
#define CMD_CLOSE		2
#define CMD_SETHWADD	3
#define CMD_GETSTATE	4
#define CMD_FORCESTOP	5
#define DEV_NAME		"wlan0"
#define	ETHER_ADDRLEN	6
#define ETHER_HDRLEN 	14
#define IFNAMSIZ	16

#define INADDR_SEND ((unsigned long int)0x0AFDFD01) //10.253.253.2
#define INADDR_RECV ((unsigned long int)0x0AFDFD02) //10.253.253.1

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

//#define DEBUG_WLAN_SERVER
#ifdef DEBUG_WLAN_SERVER
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) {}
#endif

int sig_fd;
struct sockaddr_in sig_addr; // address of sig 
struct sockaddr_in client_addr; // address of client 
static pid_t pid_real;

int do_cmd(const char *filename, char *argv[], int dowait)
{
	pid_t pid, wpid;
	int ret, status;
	sigset_t tmpset, origset;

	sigfillset(&tmpset);
	sigprocmask(SIG_BLOCK, &tmpset, &origset);
	pid = vfork();
	sigprocmask(SIG_SETMASK, &origset, NULL);

	if (pid == 0) {
		/* the child */
		char *env[3];

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		fprintf(stderr, "exec %s failed\n", filename);
		_exit(EXIT_FAILURE);
	} else if (pid > 0) {
		if (!dowait)
			ret = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&status)) != pid) {
				if (wpid == -1 && errno == ECHILD) {	/* see wait(2) manpage */
					break;
				}
			}

			ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork of %s failed\n", filename);
		ret = -1;
	}

	return ret;
}

//return 0:OK, other:fail
int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;

	//TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);

	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		//TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}

	//TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);

	return status;
}
static int run_ioctl(char *buf)
{
	struct iwreq wrq;
	struct iw_priv_args	priv[48];
	char data_p[DATALENGTH];
	int cmd;
	pid_t pid;
	int skfd;
	int send_len = VWLAN_PLEN;
	vwlan_packet_t *vwlan_pkt;
	vwlan_pkt = (vwlan_packet_t *)(buf);

	// cp iwreq struct from buf
	memcpy(&wrq, &vwlan_pkt->wrq, sizeof(struct iwreq));
	// cp cmd from buf
	cmd = vwlan_pkt->cmd;

	//printf("data length %d\n", wrq.u.data.length);
	//adjust iwreq data pointer 
	switch(cmd){
		case SIOCGIMCAST_ADD:
		case SIOCGIMCAST_DEL:
		case SIOCGIWNAME:
		case SIOCGMIIPHY:
			break;
			
		case SIOCGIWPRIV:
			wrq.u.data.pointer = (caddr_t) priv;
			wrq.u.data.length = 0;
			wrq.u.data.flags = 0;
			break;
			
		case SIOCSAPPPID:
			memcpy(&pid_real, vwlan_pkt->data, wrq.u.data.length);
			dbg("pid real %d\n", pid_real);
			pid = getpid();
			dbg("pid current %d\n", pid);
			wrq.u.data.pointer = (caddr_t) &pid;
			wrq.u.data.length = sizeof(pid_t);
			break;
		//case SIOCGIWRTLSTAINFO:
			//if(wrq.u.data.length > DATALENGTH)
			//	wrq.u.data.length = DATALENGTH;
		//	wrq.u.data.pointer = data_p;
		//	break;
		
		default:
			wrq.u.data.pointer = data_p;
			// cp iwreq data content from buf
			memcpy(wrq.u.data.pointer, vwlan_pkt->data, wrq.u.data.length);
			break;
	}
	//clear buf
	memset(buf, '\0', MAXLENGTH);

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
 	}
	
	dbg("cmd %04x\n", cmd);
	
	if(ioctl(skfd, cmd, &wrq) < 0){
		dbg("%s %d ioctl failed\n", __FUNCTION__, __LINE__);
		//cp ret value to buf
		memset(&vwlan_pkt->ret, 0, 1);
		close(skfd);
		return send_len;
	}
	else{
		close(skfd);
		
		//cp ret value to buf
		memset(&vwlan_pkt->ret, 1, 1);
		
		//cp iwreq to buf
		memcpy(&vwlan_pkt->wrq, &wrq, sizeof(struct iwreq));

		//cp iwreq content to buf
		switch(cmd){
			case SIOCGIMCAST_ADD:
			case SIOCGIMCAST_DEL:
				return 0;
			case SIOCGIWNAME:
				return send_len;
				//break;

			case SIOCGIWPRIV:
				memcpy(vwlan_pkt->data, wrq.u.data.pointer, sizeof(priv));
				send_len += sizeof(priv);
				return send_len;
				//break;
						
			default:
				memcpy(vwlan_pkt->data, wrq.u.data.pointer, wrq.u.data.length);
				send_len += wrq.u.data.length;
				return send_len;
				//break;
		}
	}
	
}

static void run_sethwadd(char *buf)
{
	char ether_addr[ETHER_ADDRLEN*3];
	char ifname[IFNAMSIZ];
	vwlan_if_packet_t *vp;
	vp = (vwlan_if_packet_t *) buf;
	
	memcpy(ifname, vp->ifname, IFNAMSIZ);
	memcpy(ether_addr, vp->hwaddr, ETHER_ADDRLEN*3);

	printf("dev name: %s\n", ifname);

	va_cmd("/bin/ifconfig", 4, 1, ifname, "hw", "ether", ether_addr);
	if(!strcmp(ifname, "wlan0"))
	{
		va_cmd("/bin/ifconfig", 4, 1, "eth0", "hw", "ether", ether_addr);
		va_cmd("/bin/ifconfig", 4, 1, "br0", "hw", "ether", ether_addr);
	}
}

static int run_cmd(char *tmp_buf, int size, int skfd)
{
	char type;
	int ret=0;
	char ifname[IFNAMSIZ];

	memcpy(&type, tmp_buf, 1);

	switch (type) {
		case CMD_IOCTL:
			ret = run_ioctl(tmp_buf);
			if(ret>0){
				if(sendto(skfd, tmp_buf, ret, 0, &client_addr, sizeof(client_addr)) < 0)
					dbg("%s %d send failed\n", __FUNCTION__, __LINE__);
			}
			break;
			
		case CMD_OPEN:
			memcpy(ifname, tmp_buf + 1, IFNAMSIZ);
			dbg("ifconfig %s up\n", ifname);
			if(!strcmp(ifname, "wlan0"))
			{
				va_cmd("/bin/brctl", 3, 1, "addif", "br0", "eth0");
				va_cmd("/bin/ifconfig", 2, 1, "eth0", "up");
				va_cmd("/bin/ifconfig", 2, 1, "br0", "up");
			}
			va_cmd("/bin/brctl", 3, 1, "addif", "br0", ifname);
			va_cmd("/bin/ifconfig", 2, 1, ifname, "up");
			break;
		case CMD_CLOSE:
			memcpy(ifname, tmp_buf + 1, IFNAMSIZ);
			va_cmd("/bin/brctl", 3, 1, "delif", "br0", ifname);
			dbg("ifconfig %s down\n", ifname);
			va_cmd("/bin/ifconfig", 2, 1, ifname, "down");
			break;
		case CMD_SETHWADD:
			run_sethwadd(tmp_buf);
			break;
		case CMD_GETSTATE:
			break;

		case CMD_FORCESTOP:
			printf("reboot\n");
			va_cmd("/bin/reboot", 1, 1, "-f");
			break;
		default:
			break;
	}

	return ret;
	
}

void RequestEvent(int sig)
{
	char buf[sizeof(vwlan_sig_packet_t)];
	vwlan_sig_packet_t *vwlan_pkt;
	vwlan_pkt = (vwlan_sig_packet_t *) buf;

	dbg("Send signal to wlanapp (pid: %d)\n", pid_real);

	memset(buf, '\0', sizeof(vwlan_sig_packet_t));

	memset(&vwlan_pkt->ret, 2, 1);
	memcpy(vwlan_pkt->data,  &pid_real, sizeof(pid_t));

	if(sendto(sig_fd, buf, sizeof(vwlan_sig_packet_t), 0, &sig_addr, sizeof(sig_addr)) < 0)
		dbg("%s %d send failed\n", __FUNCTION__, __LINE__);

}


void main()
{
	int socket_fd;   //file description into transport
	int recfd; // file descriptor to accept 
	int length; //length of address structure 
	int nbytes; // the number of read 
	char buf[MAXLENGTH];
	struct sockaddr_in myaddr; // address of this service
	struct sockaddr_in client_addr2; // address of this service
                           
	int skfd;
	struct ifreq       ifr;
	
	//Get a socket into UDP/IP
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
 	}

	if ((recfd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
 	}

	if ((sig_fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
 	}

	
	//Set up our address
	bzero ((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_RECV);
	myaddr.sin_port = htons(SERV_PORT);
	
	//Bind to the address to which the service will be offered
	if (bind(recfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0) {
		perror ("bind failed\n");
		exit(1);
	}

	//Set up client address
	bzero ((char *)&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_SEND);
	client_addr.sin_port = htons(SERV_PORT);

	//Set up signal address
	bzero ((char *)&sig_addr, sizeof(sig_addr));
	sig_addr.sin_family = AF_INET;
	sig_addr.sin_addr.s_addr = htonl(INADDR_SEND);
	sig_addr.sin_port = htons(SIG_PORT);

	printf("wlan_server (v%s) started\n", VERSION);

	pid_real = -1;
	signal(SIGIO, RequestEvent);

	//Loop continuously, waiting for datagrams and response a message
	length = sizeof(client_addr);
	dbg("Server is ready to receive !!\n");
	dbg("Can strike Ctrl-c to stop Server >>\n");

	while (1) {
		if ((nbytes = recvfrom(recfd, &buf, MAXLENGTH, 0, 
			&client_addr, &length)) <0) {
			perror ("could not read datagram!!");
			continue;
		}

		dbg("Received data from %s : %d\n", 
		inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port)); 
		client_addr.sin_port = htons(SERV_PORT);

		//parse buf and do cmd for wlan interface
		run_cmd(buf, nbytes, socket_fd);
	 
	}
	close(socket_fd);
	close(recfd);
	close(sig_fd);
}

