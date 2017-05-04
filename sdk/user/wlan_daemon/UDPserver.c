//UDP server : accept commands from the client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/wireless.h>

#define MYPORT 9034                    // the port users will be connecting to
#define BUFLEN 128                      // length of the buffer
#define MP_TX_PACKET 0x8B71
#define MP_BRX_PACKET 0x8B73

/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int iw_get_ext(int skfd,    /* Socket to the kernel */
           			char *ifname,        	/* Device name */
           			int request,        		/* WE ID */
           			struct iwreq *pwrq)    /* Fixed part of the request */
{
  	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);	/* Set device name */
  	return(ioctl(skfd, request, pwrq));			/* Do the request */
}

int MP_get_ext(char *ifname, char *buf, unsigned int ext_num)
{
    	int skfd;
    	struct iwreq wrq;

    	skfd = socket(AF_INET, SOCK_DGRAM, 0);
    	wrq.u.data.pointer = (caddr_t)buf;
    	wrq.u.data.length = strlen(buf);

    	if (iw_get_ext(skfd, ifname, ext_num, &wrq) < 0) {
    		printf("MP_get_ext failed\n");
		return -1;
    	}
	
    	close(skfd);
    	return 0;
}

int main(void) {
	int sockfd;                     				// socket descriptors
	struct sockaddr_in my_addr;     		// my address information
	struct sockaddr_in their_addr;  			// connector¡¦s address information
	int addr_len, numbytes;
	FILE *fp;
	char buf[BUFLEN], buf_tmp[BUFLEN], 
		pre_result[BUFLEN];				// buffer that stores message

	// create a socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         		// host byte order
	my_addr.sin_port = htons(MYPORT);     	// short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; 	// automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); 	// zero the rest of the struct

	// bind the socket with the address
	if (bind(sockfd, (struct sockaddr *)&my_addr,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		close(sockfd);
		exit(1);
	}

	addr_len = sizeof(struct sockaddr);

	printf("MP AUTOMATION daemon (ver 1.0)\n");
	
	// main loop : wait for the client
	while (1) {
		//receive the command from the client
		memset(buf, 0, BUFLEN);
		if ((numbytes = recvfrom(sockfd, buf, BUFLEN, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			fprintf(stderr,"Receive failed!!!\n");
			close(sockfd);
			exit(1);
		}
		
		//printf("received command (%s) from IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));

		if (!memcmp(buf, "query", 5)) {
			strcpy(buf, pre_result);
		}
		else {
			//ack to the client
			if (!memcmp(buf, "flash get", 9))
				sprintf(pre_result, "%s > /tmp/MP.txt ok", buf);
			else
				sprintf(pre_result, "%s ok", buf);
			
			if (!memcmp(buf, "iwpriv wlan0 mp_brx stop", 24)) {
				strcpy(buf, "stop");
				MP_get_ext("wlan0", buf, MP_BRX_PACKET);
			}
			else if (!memcmp(buf, "iwpriv wlan0 mp_tx", 18) && buf[18] == ' ') {
				memcpy(buf_tmp, buf+19, strlen(buf)-19);
				MP_get_ext("wlan0", buf_tmp, MP_TX_PACKET);
				strcpy(buf, buf_tmp);
			}
			else if (!memcmp(buf, "probe", 5))
				strcpy(buf, "ack");
			else if (!memcmp(buf, "verify_flw", 10)) {
				if ((fp = fopen("/tmp/MP.txt", "r")) == NULL)
					fprintf(stderr, "opening MP.txt failed !\n");
	
				if (fp) {
					fgets(buf, BUFLEN, fp);
					buf[BUFLEN-1] = '\0';
					fclose(fp);
				}
			}
			else {
				if (!memcmp(buf, "flash get", 9))
					strcat(buf, " > /tmp/MP.txt");
				
				if (!memcmp(buf, "flash set", 9)) {
					//printf("1 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
					if ((numbytes = sendto(sockfd, pre_result, strlen(pre_result), 0,
						(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
						fprintf(stderr, "send failed\n");
						close(sockfd);
						exit(1);
					}
					//printf("2 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
				}
			
				system(buf);
			}
			
			strcpy(buf_tmp, pre_result);
			strcpy(pre_result, buf);
			strcpy(buf, buf_tmp);
		}

		if (memcmp(buf, "flash set", 9) != 0) {
			//printf("1 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
			if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
				(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
				fprintf(stderr, "send failed\n");
				close(sockfd);
				exit(1);
			}
			//printf("2 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
		}
      }

	return 0;
}
