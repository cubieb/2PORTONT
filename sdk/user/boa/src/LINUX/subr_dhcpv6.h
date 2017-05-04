#ifndef SUBR_DHCPV6_H
#define SUBR_DHCPV6_H

typedef struct delegationInfo {
	int	RNTime;
	int	RBTime;
	int	PLTime;
	int 	MLTime;
	unsigned char prefixLen;
	unsigned char prefixIP[IP6_ADDR_LEN];
	unsigned char nameServer[256];
} DLG_INFO_T, *DLG_INFO_Tp;
#endif
