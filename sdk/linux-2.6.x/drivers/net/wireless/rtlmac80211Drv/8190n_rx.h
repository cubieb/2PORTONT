
#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_util.h"

#ifdef RTL8192SE
#define init_frinfo(pinfo) \
	do	{	\
			pinfo->pskb = pskb;		\
			pinfo->rssi = 0;		\
			INIT_LIST_HEAD(&(pinfo->mpdu_list)); \
			INIT_LIST_HEAD(&(pinfo->rx_list)); \
	} while(0)

static __inline__ void init_rxdesc(struct sk_buff *pskb, int i, struct mac80211_shared_priv *priv)
{
	struct rtl8190_hw	*phw;
	struct rx_frinfo	*pfrinfo;
	int offset;

	phw = GET_HW(priv);

	offset = 0x20 - ((((unsigned long)pskb->data) + sizeof(struct rx_frinfo)) & 0x1f);	// need 32 byte aligned
	skb_reserve(pskb, sizeof(struct rx_frinfo) + offset);

	pfrinfo = get_pfrinfo(pskb);

	init_frinfo(pfrinfo);

	phw->rx_infoL[i].pbuf  = (void *)pskb;
	phw->rx_infoL[i].paddr = get_physical_addr(priv, pskb->data, (MAX_RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
	phw->rx_descL[i].Dword6 = set_desc(phw->rx_infoL[i].paddr);
	phw->rx_descL[i].Dword0 = set_desc((i == (NUM_RX_DESC - 1)? RX_EOR : 0) | RX_OWN | ((MAX_RX_BUF_LEN - sizeof(struct rx_frinfo)) & RX_LengthMask));
}
#endif

