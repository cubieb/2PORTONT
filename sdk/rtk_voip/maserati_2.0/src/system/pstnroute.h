#ifndef __PSTN_ROUTE_H__
#define __PSTN_ROUTE_H__

/* Get PSTN routing number. If it is not NULL, routing to the number */
/*
 * pURI: User's dialing number, or processed by dial plan 
 * bPrefixCallSz: after process of dial plan, its prefix may be 'call'. 
 */
extern const unsigned char *GetPSTNRoutingNumber( const LinphoneCore *lc, 
										   const unsigned char *pURI, 
										   int bPrefixCallSz );

#endif /* __PSTN_ROUTE_H__ */

