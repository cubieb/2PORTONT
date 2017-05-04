/* * short hack to grab interface information * gcc -o gi gi.c; strip gi * * Blu3, Jan 1999 */ 

#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <net/if.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#define SIOCGIFCONF 0x8912 /* get iface list */ 

int getIP(char *devName, int *pIp) { 
  int numreqs = 30, sd, n, found=0;
  struct ifconf ifc;
  struct ifreq *ifr;
  struct in_addr *ia;
  
    // // if there is an arg on the command line, print out the ip of that device 
    // only. note the numreqs in the above, modify that as is desired. 
    
    if(strlen(devName) > 64) { 
        fprintf(stderr, "specified device name too large, ignoring\n"); 
    } 
    
    sd=socket(AF_INET, SOCK_STREAM, 0); 
    ifc.ifc_buf = NULL; 
    ifc.ifc_len = sizeof(struct ifreq) * numreqs; 
    ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len); 
    if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) { 
        perror("SIOCGIFCONF"); 
    } 
    ifr = ifc.ifc_req; 
    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) { 
        ia= (struct in_addr *) ((ifr->ifr_ifru.ifru_addr.sa_data)+2); 
        if (strcmp(ifr->ifr_ifrn.ifrn_name, devName)==0) { 
            //fprintf(stdout, "%s\n", inet_ntoa(*ia));
            memcpy(pIp, ia, 4);
            found=1;
            break;
        } 
        ifr++; 
    } 
    free(ifc.ifc_buf); 
    return (found); 
} 
