
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>

#include "../../linux-2.6.x/drivers/net/wireless/rtl8192cd_wlan/ieee802_mib.h"
#include "../auth/src/dlisten/iwcommon.h"

#define SERV_PORT 9999

#define BUFSIZ 1024
#define SIOCGIWNAME                             0x8B01
#define SIOCGIWRTLSCANREQ		0x8B33
#define SIOCGIWRTLGETBSSDB		0x8B34
#define IFNAMSIZ        16



#define DEBUG_ICC_RCV
#ifdef DEBUG_ICC_RCV
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) {}
#endif

typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    struct bss_desc bssdb[64];
} SS_STATUS_T, *SS_STATUS_Tp;

static int dumpWifiMib(void)
{
	struct iwreq wrq;
	int skfd;
	struct wifi_mib *pmib;
	FILE * pFile;
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("socket() fail\n");
		return -1;
	}

	strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface %s open failed!\n", "wlan0");
		close(skfd);
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		close(skfd);
		return -1;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B42, &wrq) < 0) {
		printf("Get WLAN MIB failed!\n");
		close( skfd );
		return -1;
	}

	dbg("length : %d \n" , wrq.u.data.length);
	dbg("ssid : %s \n" , pmib->dot11StationConfigEntry.dot11DesiredSSID);
	
	// write to tmp file
	pFile = fopen ("/tmp/8b42", "wb");
	fwrite (pmib , sizeof(char), sizeof(struct wifi_mib), pFile);
	fclose (pFile);
	close( skfd );
	return 0;
	
}

static int dumpBSSDB(int number)
{
	struct iwreq wrq;
	int skfd, i;
	SS_STATUS_Tp pStatus;
	struct bss_desc *pBss;
	
	FILE * pFile;
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("socket() fail\n");
		return -1;
	}

	strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface %s open failed!\n", "wlan0");
		close(skfd);
		return -1;
	}
	pStatus = calloc(1, sizeof(SS_STATUS_T));
	pStatus->number = number; // request BSS DB

	wrq.u.data.pointer = (caddr_t)pStatus;
	if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);
	
	if (ioctl(skfd, SIOCGIWRTLGETBSSDB, &wrq) < 0) {
		printf("Get SIOCGIWRTLGETBSSDB failed!\n");
		close( skfd );
		return -1;
	}

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		dbg("%d %02x:%02x:%02x:%02x:%02x:%02x\n", i,
			pBss->bssid[0], pBss->bssid[1], pBss->bssid[2],
			pBss->bssid[3], pBss->bssid[4], pBss->bssid[5]);
	}
	
	dbg("Get SIOCGIWRTLGETBSSDB success!\n");
	// write to tmp file
	pFile = fopen ("/tmp/8b34", "wb");
	fwrite (pStatus , sizeof(char), sizeof(SS_STATUS_T), pFile);
	fclose (pFile);
	close( skfd );
	
	return 0;
	
}

static int doSCANREQ()
{
    int skfd;
    struct iwreq wrq;
    unsigned char result;
	//int *pStatus;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

	strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
	
    /* Get wireless name */
    if ( ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (ioctl(skfd, SIOCGIWRTLSCANREQ, &wrq) < 0) {
      close( skfd );
      return -1;
    }
	printf("Get SIOCGIWRTLSCANREQ success!\n");
    close( skfd );

    return 0;
}


int main()
{
	int socket_fd;   
	int recfd; 
	int length; 
	int nbytes; 
	char buf[BUFSIZ];
	char *execute;
	struct sockaddr_in myaddr; 
	struct sockaddr_in client_addr; 
	
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
	}

	bzero ((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERV_PORT);
	
	if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0) {
		perror ("bind failed\n");
		exit(1);
	}


	length = sizeof(client_addr);

	while (1) 
	{
		if ((nbytes = recvfrom(socket_fd, buf, BUFSIZ, 0, &client_addr, &length)) <0) 
		{
			printf ("could not read datagram!!");
			continue;
		}

		dbg("Received data form %s : %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port)); 
		dbg("%s\n", &buf);
		buf[strlen(buf)-1] = '\0';
		buf[strlen(buf)-1] = '\0';	
		if(strncmp(buf, "CMD" , 3) == 0) // this is command
		{
			execute = &buf[4];
			system(execute);
		}
		else if(strncmp(buf, "IOCTL" , 5) == 0) // this is ioctl
		{
			execute = &buf[6];
			// extract ioctl number
			char *hexstring = &buf[6] ;
			hexstring[4] = '\0';
			int ioctl_num = (int)strtol(hexstring, NULL, 16);
			dbg("ioctl num : %x \n" , ioctl_num);
			// call ioctl
			
			switch(ioctl_num)
			{
				case 0x8B42 : // get ap mib
					dumpWifiMib();
					break;
				case SIOCGIWRTLGETBSSDB:
					dumpBSSDB(0);
					break;
				case SIOCGIWRTLSCANREQ:
					doSCANREQ();
					break;
				default:
					break;
			}
			
		}
		// send COMPLETE message to notify icc sender
		memset(buf, 0 , BUFSIZ);
		strcpy(buf, "COMPLETE");
		if (sendto(socket_fd, &buf, nbytes, 0, &client_addr, length)< 0) 
		{
			perror("Could not send datagram!!\n");
			continue;
		}
	}
}
