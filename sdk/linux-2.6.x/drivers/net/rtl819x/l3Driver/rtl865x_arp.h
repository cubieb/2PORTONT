#ifndef RTL865X_ARP_H
#define RTL865X_ARP_H

#include <net/rtl/rtl865x_arp_api.h>
/*for driver initialization*/
int32 rtl865x_arp_init(void);
int32 rtl865x_arp_reinit(void);

/*for routing module usage*/
int32 rtl865x_arp_tbl_alloc(rtl865x_route_t *route);
int32 rtl865x_arp_tbl_free(rtl865x_route_t *route);

#endif

