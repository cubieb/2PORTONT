#ifndef _APOLLO_RAW_PORT_H_
#define _APOLLO_RAW_PORT_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/port.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/


#define RAW_PORT_PHY_WRITE_ACT      (1)
#define RAW_PORT_PHY_READ_ACT       (0)

#define RAW_PORT_PHY_CMD            (1)
#define RAW_PORT_PHY_CMD_BUSY       (1)

typedef struct apollo_raw_port_ability_s
{
    rtk_port_speed_t        speed;
    rtk_port_duplex_t       duplex;
	rtk_enable_t 		    linkFib1g;
	rtk_port_linkStatus_t 	linkStatus;
	rtk_enable_t 		    txFc;
	rtk_enable_t 		    rxFc;
	rtk_enable_t 		    nwayAbility;
	rtk_enable_t 		    masterMod;
	rtk_enable_t 		    nwayFault;
	rtk_enable_t 		    lpi_100m;
	rtk_enable_t 		    lpi_giga;
    
} apollo_raw_port_ability_t;



extern int32 apollo_raw_port_isolation_set(rtk_port_t port, uint32 vidx);
extern int32 apollo_raw_port_isolation_get(rtk_port_t port, uint32 *pVidx);
extern int32 apollo_raw_port_isolationL34_set(rtk_port_t port, uint32 vidx);
extern int32 apollo_raw_port_isolationL34_get(rtk_port_t port, uint32 *pVidx);
extern int32 apollo_raw_extport_isolation_set(rtk_port_t port, uint32 vidx);
extern int32 apollo_raw_extport_isolation_get(rtk_port_t port, uint32 *pVidx);
extern int32 apollo_raw_extport_isolationL34_set(rtk_port_t port, uint32 vidx);
extern int32 apollo_raw_extport_isolationL34_get(rtk_port_t port, uint32 *pVidx);
extern int32 apollo_raw_dslvc_isolation_set(rtk_port_t vc, uint32 vidx);
extern int32 apollo_raw_dslvc_isolation_get(rtk_port_t vc, uint32* pVidx);
extern int32 apollo_raw_port_queueEmpty_get(rtk_portmask_t *pEmpty_mask);
extern int32 apollo_raw_port_ForceAbility_set(rtk_port_t port, apollo_raw_port_ability_t *pAbility);
extern int32 apollo_raw_port_ForceAbility_get(rtk_port_t port, apollo_raw_port_ability_t *pAbility);
extern int32 apollo_raw_port_ability_get(rtk_port_t port, apollo_raw_port_ability_t *pAbility);

extern int32 apollo_raw_port_phyReg_set(rtk_port_t port, uint32 reg, uint32 data);
extern int32 apollo_raw_port_phyReg_get(rtk_port_t port, uint32 reg, uint32 *pData);
extern int32 apollo_raw_port_forceDmp_set(rtk_enable_t state);
extern int32 apollo_raw_port_forceDmp_get(rtk_enable_t *pState);
extern int32 apollo_raw_port_forceDmpMask_set(rtk_port_t port, rtk_portmask_t mask);
extern int32 apollo_raw_port_forceDmpMask_get(rtk_port_t port, rtk_portmask_t  *pMask);

extern int32 apollo_raw_port_localPacket_set(rtk_port_t port, rtk_action_t action);
extern int32 apollo_raw_port_localPacket_get(rtk_port_t port, rtk_action_t *pAction);

extern int32 apollo_raw_port_isoIpmcastLeaky_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_port_isoIpmcastLeaky_get(rtk_port_t port, rtk_enable_t *pEnable);


#endif /*#ifndef _APOLLO_RAW_PORT_H_*/

