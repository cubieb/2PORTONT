#ifndef	RTL865X_NAT_H
#define	RTL865X_NAT_H

/*  we let both napt and pure routing exist in asic napt table */
#define RTL8676_TCPUDPTBL_SIZE_HW	(RTL8651_TCPUDPTBL_SIZE)
#define RTL8676_TCPUDPTBL_SIZE_SW	(RTL8676_TCPUDPTBL_SIZE_HW)

/* NAT timeout value */
#define TCP_TIMEOUT					10	 	/* 10 secs */
#define UDP_TIMEOUT					10		/* 10 secs */
#define TCP_CLOSED_FLOW				8

#define RTL865X_PROTOCOL_UDP		0
#define RTL865X_PROTOCOL_TCP		1

#define NAT_INBOUND				(1<<0)
#define NAT_OUTBOUND			(1<<1)
#define NAT_PRIOTIY_VALID		(1<<2)
#define NAT_PRE_RESERVED		(1<<3)


#define NAT_INUSE(n)				( ((n)->flags&(NAT_INBOUND|NAT_OUTBOUND)) )
#define SET_NAT_FLAGS(n, v)		(n)->flags |= v
#define CLR_NAT_FLAGS(n, v)		((n)->flags &= (~v))

#define CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE
#define MAX_EXTPORT_TRY_CNT 8
#define RESERVE_EXPIRE_TIME	3	/*uinit:seconds*/



int32 rtl865x_nat_init(void);
int32 rtl865x_nat_reinit(void);


/*  Add/Delete inbound or outbound flow 
     (These APIs record not only NAPT connection ,  but also pure routing connection in sw napt table)  */
int32 rtl865x_addNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol
		,uint8 pri_valid,uint8 pri_value,uint8 isUpstream);
int32 rtl865x_delNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol,uint8 isUpstream);
int32 rtl865x_flushNapt(void);

/*   Lookup function  */
int32 rtl865x_lookupNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol,uint8 isUpstream);

/* check if dst ip is in use by napt connection*/
int32 rtl865x_checkNaptConnection(ipaddr_t ip);

#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
int rtl865x_optimizeExtPort(unsigned short origDelta, unsigned int rangeSize, unsigned short *newDelta);

int rtl865x_getAsicNaptHashScore( uint32 protocol, ipaddr_t intIp, uint32 intPort,
					                        ipaddr_t extIp, uint32 extPort,
					                        ipaddr_t remIp, uint32 remPort, 
					                        uint32 *naptHashScore);

int32 rtl865x_preReserveConn( uint32 protocol, ipaddr_t intIp, uint32 intPort,
					                        ipaddr_t extIp, uint32 extPort,
					                        ipaddr_t remIp, uint32 remPort);

#endif
#endif

