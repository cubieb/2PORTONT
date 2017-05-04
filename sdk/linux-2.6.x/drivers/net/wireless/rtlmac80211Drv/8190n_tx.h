#ifndef _8190N_TX_H_
#define _8190N_TX_H_

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_util.h"
/*
enum _TX_QUEUE_ {
	MGNT_QUEUE		= 0,
	BK_QUEUE		= 1,
	BE_QUEUE		= 2,
	VI_QUEUE		= 3,
	VO_QUEUE		= 4,
	HIGH_QUEUE		= 5,
	BEACON_QUEUE	= 6
};
*/
// the purpose if actually just to link up all the desc in the same q
static __inline__ void init_txdesc(struct mac80211_shared_priv *priv, struct tx_desc *pdesc,
				unsigned long ringaddr, unsigned int i)
{
	
	if (i == (NUM_TX_DESC - 1))
		(pdesc + i)->Dword9 = set_desc(ringaddr); // NextDescAddress
	else
		(pdesc + i)->Dword9 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress

	
}

static __inline__ void desc_copy(struct tx_desc *dst, struct tx_desc *src)
{
#if	defined(RTL8190)
	dst->cmd   = src->cmd;
	dst->opt   = src->opt;
	dst->flen  = src->flen;
#elif defined(RTL8192SE)
	memcpy(dst, src, 32);
#endif

}

static __inline__ void descinfo_copy(struct tx_desc_info *dst, struct tx_desc_info *src)
{
	dst->type  = src->type;
	dst->len   = src->len;
	dst->rate  = src->rate;
}

#endif
