#ifndef _SERVERPACKET_H
#define _SERVERPACKET_H


int sendOffer(struct dhcpMessage *oldpacket);
int sendNAK(struct dhcpMessage *oldpacket);
int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr);
int send_inform(struct dhcpMessage *oldpacket);

unsigned char getDeviceType();
void addDhcpdOption(struct dhcpMessage *packet);
void find_match_serving_pool(struct dhcpMessage *dhcppacket);
#endif
