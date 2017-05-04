#ifndef		_MIB_TOOL_H_
#define		_MIB_TOOL_H_

//#include "../../boa/src/LINUX/mib.h"
#include <rtk/sysconfig.h>
#include <rtk/utility.h>

// ASN.1 Type
#define ASN_BOOLEAN	    		(0x01)
#define ASN_INTEGER	    			(0x02)
#define ASN_BIT_STR	    			(0x03)
#define ASN_OCTET_STR	    		(0x04)
#define ASN_NULL	   			(0x05)
#define ASN_OBJECT_ID	    		(0x06)
#define ASN_SEQUENCE	    		(0x10)
#define ASN_SET		   		 	(0x11)

#define ASN_UNIVERSAL	    		(0x00)
#define ASN_APPLICATION     		(0x40)
#define ASN_CONTEXT	    		(0x80)
#define ASN_PRIVATE	    			(0xC0)

#define ASN_IPADDRESS   			(ASN_APPLICATION | 0)
#define ASN_UNSIGNED    			(ASN_APPLICATION | 2)   /* RFC 1902 - same as GAUGE */
#define ASN_TIMETICKS   			(ASN_APPLICATION | 3)

#define MAX_NAME_LEN 30

 struct mib_oid { 	
 	unsigned char		*name; 	
 	unsigned int		length;
 };

struct mib_oid_tbl {
	unsigned int		total;	
	unsigned char		*oid_name_pool;
	
	struct mib_oid	*oid;
};
	
struct channel_conf_para {
	int inputvpi;
	int inputvci;
	int cmode;
	int admin;
	int natmode;
	int dhcpmode;
	unsigned long pppIfIndex;
	unsigned long IpIndex;
	int encap;
	int brmode;
	
};


struct channel_conf_ppp_para {	
	unsigned long pppIfIndex;
	int admin;
	int IdleTime;
	unsigned char pppUsername[MAX_NAME_LEN];
	unsigned char pppPassword[MAX_NAME_LEN];	
	int mtu;
	
	
};


struct channel_conf_ip_para {	
	unsigned long IpIndex;	
	unsigned char ipAddr[IP_ADDR_LEN];
	unsigned char remoteIpAddr[IP_ADDR_LEN];	
	unsigned char netMask[IP_ADDR_LEN];	
	
};

struct eoc_create_aal5Encap {
	int vpi;
	int vci;
	int aal5EncapFlag;
	int aal5EncapValue;
};

struct eoc_create_pvcEncap {
	int vpi;
	int vci;
	int pvcEncapFlag;
	int pvcEncapValue;
};

	
struct eoc_create_PppMss {
	unsigned long pppIfIndex;
	int PppMssFlag;
	int PppMssValue;
};


struct eoc_create_PppDisconnectTime {
	unsigned long pppIfIndex;
	int PppDisconnectTimeFlag;
	int PppDisconnectTimeValue;
};

typedef enum {
	CONN_UP = 1,	
	CONN_DOWN,
	CONN_NOT_EXIST
} CONN_T;

extern unsigned int create_mib_tbl(struct mib_oid_tbl *tbl, unsigned int total, unsigned int max_oid_name);
extern void free_mib_tbl(struct mib_oid_tbl *tbl);
extern void print_mib_tbl(struct mib_oid_tbl *tbl);


extern int
snmp_oid_compare(const unsigned char * in_name1,
                 unsigned int len1, const unsigned char * in_name2, unsigned int len2);


extern int snmp_oid_getnext(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result);
extern int snmp_oid_get(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result);

extern int pvc_is_greater(int vpi1, int vci1, int vpi2, int vci2);
//extern MIB_CE_ATM_VC_T* vclTableSort(int vpi, int vci);
extern void vclTableSort(int vpi, int vci, MIB_CE_ATM_VC_Tp tmp_pEntry);
//extern void pppTableSort(unsigned long ifindex, MIB_CE_ATM_VC_Tp tmp_pEntry);
extern int modifyChannelConf(struct channel_conf_para *para);
extern int pppTesting(unsigned long ifIndex, int admin);

#endif
