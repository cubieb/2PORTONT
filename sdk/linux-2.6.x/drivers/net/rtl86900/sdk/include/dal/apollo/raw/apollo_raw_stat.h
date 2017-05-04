#ifndef _APOLLO_RAW_SEC_H_
#define _APOLLO_RAW_SEC_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/stat.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef enum apollo_raw_mib_type_e
{
    APOLLO_MIB_IF_IN_OCTETS = 0,
    APOLLO_MIB_IF_IN_UCAST_PKTS,
    APOLLO_MIB_IF_IN_MULTICAST_PKTS,
    APOLLO_MIB_IF_IN_BROADCAST_PKTS,
    APOLLO_MIB_IF_OUT_OCTETS,
    APOLLO_MIB_IF_OUT_DISCARDS,
    APOLLO_MIB_IF_OUT_UCAST_PKTS_CNT,
    APOLLO_MIB_IF_OUT_MULTICAST_PKTS_CNT,
    APOLLO_MIB_IF_OUT_BROADCAST_PKTS_CNT,
    APOLLO_MIB_DOT1D_TP_PORT_IN_DISCARDS,
    APOLLO_MIB_DOT3_IN_PAUSE_FRAMES,
    APOLLO_MIB_DOT3_OUT_PAUSE_FRAMES,
    APOLLO_MIB_DOT3_STATS_SINGLE_COLLISION_FRAMES,
    APOLLO_MIB_DOT3_STATS_MULTIPLE_COLLISION_FRAMES,
    APOLLO_MIB_DOT3_STATS_DEFERRED_TRANSMISSIONS,
    APOLLO_MIB_DOT3_STATS_LATE_COLLISIONS,
    APOLLO_MIB_DOT3_STATS_EXCESSIVE_COLLISIONS,
    APOLLO_MIB_DOT3_STATS_SYMBOL_ERRORS,
    APOLLO_MIB_DOT3_CONTROL_IN_UNKNOWN_OPCODES,
    APOLLO_MIB_ETHER_STATS_DROP_EVENTS,
    APOLLO_MIB_ETHER_STATS_FRAGMENTS,
    APOLLO_MIB_ETHER_STATS_JABBERS,
    APOLLO_MIB_ETHER_STATS_COLLISIONS,
    APOLLO_MIB_ETHER_STATS_CRC_ALIGN_ERRORS,
    APOLLO_MIB_ETHER_STATS_TX_UNDER_SIZE_PKTS,
    APOLLO_MIB_ETHER_STATS_TX_OVERSIZE_PKTS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_64OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_65TO127OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_128TO255OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_256TO511OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_512TO1023OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_1024TO1518OCTETS,
    APOLLO_MIB_ETHER_STATS_TX_PKTS_1519TOMAXOCTETS,
    APOLLO_MIB_ETHER_STATS_TX_BROADCAST_PKTS,
    APOLLO_MIB_ETHER_STATS_TX_MULTICAST_PKTS,
    APOLLO_MIB_ETHER_STATS_RX_UNDER_SIZE_PKTS,
    APOLLO_MIB_ETHER_STATS_RX_UNDER_SIZE_DROP_PKTS,
    APOLLO_MIB_ETHER_STATS_RX_OVERSIZE_PKTS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_64OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_65TO127OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_128TO255OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_256TO511OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_512TO1023OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_1024TO1518OCTETS,
    APOLLO_MIB_ETHER_STATS_RX_PKTS_1519TOMAXOCTETS,
    APOLLO_MIB_IN_OAM_PDU_PKTS,
    APOLLO_MIB_OUT_OAM_PDU_PKTS,
    APOLLO_MIB_END
}apollo_raw_mib_type_t;

typedef enum apollo_raw_mib_rst_value_e
{
    APOLLO_MIB_RST_TO_0 = 0,
    APOLLO_MIB_RST_TO_1,
    APOLLO_MIB_RST_END
}apollo_raw_mib_rst_value_t;

typedef enum apollo_raw_mib_sync_mode_e
{
    APOLLO_MIB_SYNC_MODE_STOP_SYNC = 0,
    APOLLO_MIB_SYNC_MODE_FREE_SYNC,
    APOLLO_MIB_SYNC_MODE_END
}apollo_raw_mib_sync_mode_t;

typedef enum apollo_raw_mib_count_mode_e
{
    APOLLO_MIB_COUNT_MODE_FREE = 0,
    APOLLO_MIB_COUNT_MODE_TIMER,
    APOLLO_MIB_COUNT_MODE_END
}apollo_raw_mib_count_mode_t;

typedef enum apollo_raw_mib_tag_cnt_dir_e
{
    APOLLO_MIB_TAG_CNT_DIR_TX = 0,
    APOLLO_MIB_TAG_CNT_DIR_RX,
    APOLLO_MIB_TAG_CNT_DIR_END
}apollo_raw_mib_tag_cnt_dir_t;

typedef enum apollo_raw_mib_tag_cnt_state_e
{
    APOLLO_MIB_TAG_CNT_STATE_EXCLUDE = 0,
    APOLLO_MIB_TAG_CNT_STATE_INCLUDE,
    APOLLO_MIB_TAG_CNT_STATE_END
}apollo_raw_mib_tag_cnt_state_t;

extern int32 apollo_raw_stat_global_reset(void);
extern int32 apollo_raw_stat_port_reset(rtk_port_t port);
extern int32 apollo_raw_stat_acl_reset(uint32 index);
extern int32 apollo_raw_stat_acl_mode_set(uint32 index, rtk_stat_logCnt_mode_t mode);
extern int32 apollo_raw_stat_acl_mode_get(uint32 index, rtk_stat_logCnt_mode_t* pMode);
extern int32 apollo_raw_stat_acl_type_set(uint32 index, rtk_stat_logCnt_type_t type);
extern int32 apollo_raw_stat_acl_type_get(uint32 index, rtk_stat_logCnt_type_t* pType);
extern int32 apollo_raw_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr);
extern int32 apollo_raw_stat_global_get(rtk_stat_global_type_t cntr_idx, uint64 *pCntr);
extern int32 apollo_raw_stat_acl_get(uint32 index, uint64 *pCntr);
extern int32 apollo_raw_stat_pktInfo_get(rtk_port_t port, uint32 *pCode);
extern int32 apollo_raw_stat_rst_mib_value_set(rtk_mib_rst_value_t rst_value);
extern int32 apollo_raw_stat_rst_mib_value_get(rtk_mib_rst_value_t *rst_value);
extern int32 apollo_raw_stat_acl_counter_get(uint32 index, uint32 *cnt);
extern int32 apollo_raw_stat_mib_cnt_mode_get(rtk_mib_count_mode_t *pCnt_mode);
extern int32 apollo_raw_stat_mib_cnt_mode_set(rtk_mib_count_mode_t cnt_mode);
extern int32 apollo_raw_stat_mib_latch_timer_set(uint32 timer);
extern int32 apollo_raw_stat_mib_latch_timer_get(uint32 *pTimer);
extern int32 apollo_raw_stat_mib_sync_mode_get(rtk_mib_sync_mode_t *pSync_mode);
extern int32 apollo_raw_stat_mib_sync_mode_set(rtk_mib_sync_mode_t sync_mode);
extern int32 apollo_raw_stat_mib_count_tag_length_get(rtk_mib_tag_cnt_dir_t direction, rtk_mib_tag_cnt_state_t *pState);
extern int32 apollo_raw_stat_mib_count_tag_length_set(rtk_mib_tag_cnt_dir_t direction, rtk_mib_tag_cnt_state_t state);
#endif /*#ifndef _APOLLO_RAW_SEC_H_*/

