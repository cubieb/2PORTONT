/* rtl8672pp_extsw.h - RealTek rtl8672 header file for EXT_SWITCH */

#ifndef PP_EXTSW_H
#define PP_EXTSW_H

#include "icModel_ringController.h"

int pp_mac_vtx_sw (struct sk_buff *skb, struct mac_vTx *vtx);
int pp_SARvtx_sw(struct atm_vcc *vcc, struct sk_buff **skb, int port);
int pp_mac_rx_sw(int idx, struct mac_vRx *macRx, struct sk_buff *newskb);
int pp_mac_int_sw (struct mac_vRx *macRx, struct sk_buff *newskb);
void pp_check_igmp_snooping_rx(struct sk_buff *skb, int tag);
int sar_send_sw(struct atm_vcc *vcc, struct sk_buff **skb);
#endif // of PP_EXTSW_H
