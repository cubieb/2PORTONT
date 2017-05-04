/* ra867x_pp.h - RealTek rtl8672 sar header file for Packet Processor */

#ifndef SAR_RA867xPP_H
#define SAR_RA867xPP_H

#include <linux/atmdev.h>

extern int pp_enabled;	// Packet Processor enable
extern char pvc_mode[16];
extern char pvc_encap[16];
struct rtl_sar_private;

int pp_sar_send(struct atm_vcc *vcc, struct sk_buff* skb, uint32 len, int port);
void pp_set_vc_type(int num);
void pp_init_sar_desc(struct rtl_sar_private *cp, int vcnum);
int sar_rx_sw(struct atm_vcc *vcc, struct sk_buff *skb);

#endif // of SAR_RA867xPP_H
