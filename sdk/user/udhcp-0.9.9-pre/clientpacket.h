#ifndef _CLIENTPACKET_H
#define _CLIENTPACKET_H

unsigned long random_xid(void);
int send_discover(unsigned long xid, unsigned long requested);
int send_selecting(unsigned long xid, unsigned long server, unsigned long requested);
int send_renew(unsigned long xid, unsigned long server, unsigned long ciaddr);
int send_renew(unsigned long xid, unsigned long server, unsigned long ciaddr);
int send_release(unsigned long server, unsigned long ciaddr);
int get_raw_packet(struct dhcpMessage *payload, int fd);

/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
void rt_prepare_microsoft_auto_ip_dhcpack(struct dhcpMessage *packet, u_int32_t addr);
#endif
