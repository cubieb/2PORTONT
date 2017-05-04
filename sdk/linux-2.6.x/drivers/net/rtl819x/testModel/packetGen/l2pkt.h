#ifndef L2PKT_H
#define L2PKT_H

#include <net/rtl/rtl_types.h>

#pragma pack(push,1)
typedef struct _etherHdr_s {
	uint8	da[6], sa[6];
	uint16	etherType;
} _etherHdr_t;

typedef struct _vlanHdr_s {
	uint16	tag;
	uint16	etherType;
} _vlanHdr_t;

typedef struct _llcHdr_s {
	uint8	llc_dsap, llc_ssap, ctrl;
	uint8	org_code[3];
	uint16	etherType;
} _llcHdr_t;

typedef struct _pppoeHdr_s {
  #ifdef _LITTLE_ENDIAN
	uint8	type:4, ver:4;
  #else
	uint8	ver:4, type:4;
  #endif
	uint8	code;
	uint16	sessionId, 
			length; //Length of the PPPoE payload, does not include Ethernet and PPPoE header
	uint16	proto;	//PPP protocol field
} _pppoeHdr_t;
#pragma pack(pop)


#endif

