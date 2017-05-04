#ifndef IPV6_INFO_H
#define IPV6_INFO_H

#define IPV6_BUF_SIZE_256 256
#define IPV6_BUF_SIZE_128 128

typedef struct dnsV6Info {
	unsigned char mode;
	unsigned int wanconn;
	unsigned char nameServer[IPV6_BUF_SIZE_256];
	unsigned char leaseFile[IPV6_BUF_SIZE_128];
} DNS_V6_INFO_T, *DNS_V6_INFO_Tp;


typedef struct prefixV6Info {
	int	RNTime;
	int	RBTime;
	int	PLTime;
	int MLTime;
	unsigned char mode;
	unsigned int wanconn;
	unsigned char prefixIP[IP6_ADDR_LEN];
	unsigned char prefixLen;
	unsigned char leaseFile[IPV6_BUF_SIZE_128];
} PREFIX_V6_INFO_T, *PREFIX_V6_INFO_Tp;

#endif