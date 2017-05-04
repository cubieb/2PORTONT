#ifndef _MESH_11KV_H_
#define _MESH_11KV_H_

#include "../8190n.h"

extern void issue_11kv_LinkMeasureReq(struct rtl8190_priv * priv, struct stat_info * pstat, UINT16 lenTst, UINT16 cntTst, UINT8 prioTst);
extern void issue_11kv_LinkMeasureRepAck(struct rtl8190_priv * priv, struct stat_info * pstat);
extern void issue_11kv_LinkMeasureRepRep(struct rtl8190_priv * priv, struct stat_info * pstat);
extern unsigned int On11kvLinkMeasureReq(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo);
extern unsigned int On11kvLinkMeasureRep(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo);
extern UINT32 computeMetric(struct rtl8190_priv * priv, struct stat_info * pstat, UINT8 rate, UINT8 priority,
			UINT16 lenTotalSnd, UINT16 cntSnd, UINT16 lenTotalRcv, UINT16 cntRcv);
extern UINT32 getMetric(struct rtl8190_priv * priv, struct stat_info * pstat);
extern void metric_update(struct rtl8190_priv *priv);

#endif // __MESH_11KV_H_
