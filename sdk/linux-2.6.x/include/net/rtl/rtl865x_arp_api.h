#ifndef RTL865X_ARP_API_H
#define RTL865X_ARP_API_H

#define RTL8651_SW_ARPTBL_SIZE			1024
#define RTL8651_SW_ARPTBL_Block_SIZE		(RTL8651_SW_ARPTBL_SIZE>>3)

typedef struct rtl865x_arpMapping_entry_s
{
        ipaddr_t ip;
        ether_addr_t mac;
}rtl865x_arpMapping_entry_t;


struct rtl865x_arp_table {
        uint8                           allocBitmap[RTL8651_SW_ARPTBL_Block_SIZE];
        rtl865x_arpMapping_entry_t      mappings[RTL8651_SW_ARPTBL_SIZE];
};

/*for linux protocol stack sync*/
int32 rtl865x_addArp(ipaddr_t ip, ether_addr_t * mac);
int32 rtl865x_delArp(ipaddr_t ip);
uint32 rtl865x_arpSync( ipaddr_t ip, uint32 refresh );
int32 rtl865x_getArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping);
struct rtl865x_arp_table* rtl865x_getswARPTable(void);
#endif
