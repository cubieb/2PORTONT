#ifndef _APOLLO_RAW_SWITCH_H_
#define _APOLLO_RAW_SWITCH_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/

#define APOLLO_MAXLENGTH_INDEX_NUM 	        2

#define APOLLO_FIELD_SELECTOR_NO 	        16
#define APOLLO_FIELD_SELECTOR_MAX 	        (APOLLO_FIELD_SELECTOR_NO - 1)
#define APOLLO_FIELD_SELECTOR_OFFSET_MAX 	255


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_cpuTagMode_e
{
    RAW_CPUTAGMODE_APOLLO  = 0,
    RAW_CPUTAGMODE_4BYTES,
    RAW_CPUTAGMODE_END,
} apollo_raw_cpuTagMode_t;

typedef enum apollo_raw_chipReset_e
{
    RAW_SW_GLOBAL_RST= 0,
    RAW_SW_CHIP_RST,
    RAW_CHIPRESET_END,
} apollo_raw_chipReset_t;

typedef enum apollo_raw_ipgCompMode_e
{
    RAW_IPGCOMPMODE_90PPM  = 0,
    RAW_IPGCOMPMODE_65PPM,
    RAW_IPGCOMPMODE_END,
} apollo_raw_ipgCompMode_t;


typedef enum apollo_raw_parserFieldSelctorMode_e
{
    RAW_FIELDSELMODE_DEFAULT = 0,
    RAW_FIELDSELMODE_RAW,
    RAW_FIELDSELMODE_LLC,
    RAW_FIELDSELMODE_IPV4,
    RAW_FIELDSELMODE_ARP,
    RAW_FIELDSELMODE_IPV6,
    RAW_FIELDSELMODE_IPPAYLOAD,
    RAW_FIELDSELMODE_L4PAYLOAD,
    RAW_FIELDSELMODE_END,    
} apollo_raw_parserFieldSelctorMode_t;

typedef enum apollo_raw_backPressure_e
{
    RAW_BACKPRESSURE_JAM  = 0,
    RAW_BACKPRESSURE_DEFER,
    RAW_BACKPRESSURE_END,
} apollo_raw_backPressure_t;

typedef enum apollo_raw_maxLengthCfg_e
{
    RAW_MAXLENGTH_CFG0  = 0,
    RAW_MAXLENGTH_CFG1,
    RAW_MAXLENGTH_END,
} apollo_raw_maxLengthCfg_t;


extern int32 apollo_raw_switch_macAddr_set(rtk_mac_t *pMacAddr);
extern int32 apollo_raw_switch_macAddr_get(rtk_mac_t *pMacAddr);
extern int32 apollo_raw_switch_maxPktLen_set(uint32 index, uint32 len);
extern int32 apollo_raw_switch_maxPktLen_get(uint32 index, uint32 *pLen);
extern int32 apollo_raw_switch_maxPktLenSpeed_set(rtk_port_t port, apollo_raw_linkSpeed_t speed, uint32 index);
extern int32 apollo_raw_switch_maxPktLenSpeed_get(rtk_port_t port, apollo_raw_linkSpeed_t speed, uint32 *pIndex);
extern int32 apollo_raw_switch_cpuTagMode_set(apollo_raw_cpuTagMode_t format);
extern int32 apollo_raw_switch_cpuTagMode_get(apollo_raw_cpuTagMode_t *pFormat);
extern int32 apollo_raw_switch_cpuTagAware_set(rtk_port_t port, rtk_enable_t enable);
extern int32 apollo_raw_switch_cpuTagAware_get(rtk_port_t port, rtk_enable_t *pEnable);

extern int32 apollo_raw_switch_rxCrcCheck_set(uint32 port, rtk_enable_t enable);
extern int32 apollo_raw_switch_rxCrcCheck_get(uint32 port, rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_smallTagIpg_set(uint32 port, rtk_enable_t enable);
extern int32 apollo_raw_switch_smallTagIpg_get(uint32 port, uint32 *pEnable);
extern int32 apollo_raw_switch_smallPkt_set(uint32 port, rtk_enable_t enable);
extern int32 apollo_raw_switch_smallPkt_get(uint32 port, uint32 *pEnable);
extern int32 apollo_raw_switch_chipReset_set(apollo_raw_chipReset_t reset);
extern int32 apollo_raw_switch_globalRxCrcCheck_set(rtk_enable_t enable);
extern int32 apollo_raw_switch_globalRxCrcCheck_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_48pass1Enable_set(rtk_enable_t enable);
extern int32 apollo_raw_switch_48pass1Enable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_shortIpgEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_switch_shortIpgEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_ipgCompensation_set(apollo_raw_ipgCompMode_t mode);
extern int32 apollo_raw_switch_ipgCompensation_get(apollo_raw_ipgCompMode_t *pMode);
extern int32 apollo_raw_switch_parserFieldSelector_set(uint32 index, apollo_raw_parserFieldSelctorMode_t mode, uint32 offset);
extern int32 apollo_raw_switch_parserFieldSelector_get(uint32 index, apollo_raw_parserFieldSelctorMode_t *pMode, uint32 *pOffset);
extern int32 apollo_raw_switch_limitPause_set(rtk_enable_t enable);
extern int32 apollo_raw_switch_limitPause_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_backPressureEnable_set(rtk_enable_t enable);
extern int32 apollo_raw_switch_backPressureEnable_get(rtk_enable_t *pEnable);
extern int32 apollo_raw_switch_backPressure_set(apollo_raw_backPressure_t state);
extern int32 apollo_raw_switch_backPressure_get(apollo_raw_backPressure_t *pState);

#endif /*#ifndef _APOLLO_RAW_SWITCH_H_*/

