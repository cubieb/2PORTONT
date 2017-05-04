#ifndef RTL865X_FDB_API_H
#define RTL865X_FDB_API_H
#include <net/rtl/rtl_types.h>

#define RTL_LAN_FID									0
#if defined (CONFIG_RTL_IVL_SUPPORT)	
#define RTL_WAN_FID								1
#else
#define RTL_WAN_FID								0
#endif
#define FDB_STATIC						0x01		/* flag for FDB: process static entry only */
#define FDB_DYNAMIC						0x02		/* flag for FDB: process dynamic entry only */

#define RTL865x_L2_TYPEI			0x0001		/* Referenced by ARP/PPPoE */
#define RTL865x_L2_TYPEII			0x0002		/* Referenced by Protocol */
#define RTL865x_L2_TYPEIII			0x0004		/* Referenced by PCI/Extension Port */

void update_hw_l2table(const char *srcName,const unsigned char *addr);
int32 rtl_get_hw_fdb_age(uint32 fid,ether_addr_t *mac, uint32 flags);
int32 rtl865x_delLanFDBEntry(uint16 l2Type,  const unsigned char *addr);
int32 rtl865x_addFDBEntry(const unsigned char *addr);
int32 rtl865x_delAuthLanFDBEntry(uint16 l2Type,  const unsigned char *addr);
#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
int32 rtl865x_delAuthLanFDBEntry(uint16 l2Type,  const unsigned char *addr);
void rtl865x_new_AuthFDB(const unsigned char *addr);
#endif
int32 rtl865x_Lookup_L2_by_MAC(const unsigned char *addr);

#endif

