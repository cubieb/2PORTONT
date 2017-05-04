#ifndef		_MIB_TOOL_H_
#define		_MIB_TOOL_H_


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


 struct mib_oid {
 	unsigned char		*name;
 	unsigned int		length;
 };

struct mib_oid_tbl {
	unsigned int		total;
	unsigned char		*oid_name_pool;
	struct mib_oid	*oid;
};

extern unsigned int create_mib_tbl(struct mib_oid_tbl *tbl, unsigned int total, unsigned int max_oid_name);
extern void free_mib_tbl(struct mib_oid_tbl *tbl);
extern void print_mib_tbl(struct mib_oid_tbl *tbl);

extern int
snmp_oid_compare(const unsigned char * in_name1,
                 unsigned int len1, const unsigned char * in_name2, unsigned int len2);


extern int snmp_oid_getnext(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result);
extern int snmp_oid_get(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result);


#endif
