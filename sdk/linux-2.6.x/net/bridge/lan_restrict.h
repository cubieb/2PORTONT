//#include <common/rtl865x_common.h>
#include <common/rtl_types.h>
#include <common/rtl_queue.h>
#include <common/rtl865x_eventMgr.h>
#include "AsicDriver/rtl865x_asicCom.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include <AsicDriver/rtl865x_asicL2.h>
#else
#include <AsicDriver/rtl865xC_tblAsicDrv.h>
#endif
#include <l2Driver/rtl865x_fdb.h>


typedef struct _lan_restrict_info
{
	int16 		port_num;
	int16		enable;
	int32	max_num;
	int32	curr_num;
}lan_restrict_info;

int32 lan_restrict_getBlockAddr(int32 port , const unsigned char *swap_addr);
extern int __init lan_restrict_init(void);	
extern int	 lan_restrict_rcv(struct sk_buff *skb, struct net_device *dev);
extern int32 lan_restrict_CheckStatusByport(int32 port);
