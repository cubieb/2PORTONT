#ifndef _APOLLO_RAW_TRUNK_H_
#define _APOLLO_RAW_TRUNK_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RAW_TRUNK_HASH_SOURCE_PORT      (0x01)
#define RAW_TRUNK_HASH_SOURCE_MAC       (0x02)
#define RAW_TRUNK_HASH_DEST_MAC         (0x04)
#define RAW_TRUNK_HASH_SOURCE_IP        (0x08)
#define RAW_TRUNK_HASH_DEST_IP          (0x10)
#define RAW_TRUNK_HASH_SOURCE_L4PORT    (0x20)
#define RAW_TRUNK_HASH_DEST_L4PORT      (0x40)

#define RAW_TRUNK_HASH_ALL  \
( RAW_TRUNK_HASH_SOURCE_PORT |   \
  RAW_TRUNK_HASH_SOURCE_MAC |    \
  RAW_TRUNK_HASH_DEST_MAC |      \
  RAW_TRUNK_HASH_SOURCE_IP |     \
  RAW_TRUNK_HASH_DEST_IP |       \
  RAW_TRUNK_HASH_SOURCE_L4PORT | \
  RAW_TRUNK_HASH_DEST_L4PORT )

#define RAW_TRUNK_ALGORITHM_NUM         (7)
#define RAW_TRUNK_HASH_VALUE_MAX        (15)

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum raw_trunk_mode_e
{
    RAW_TRUNK_USER_MODE = 0,
    RAW_TRUNK_DUMB_MODE,
    RAW_TRUNK_MODE_END
} raw_trunk_mode_t;

typedef enum raw_trunk_floodMode_e
{
    RAW_TRUNK_NORMAL_FLOOD = 0,
    RAW_TRUNK_FWD_TO_FIRST_PORT,
    RAW_TRUNK_FLOOD_MODE_END
} raw_trunk_floodMode_t;

typedef enum raw_trunk_port_e
{
    RAW_TRUNK_PORT_FIRST = 0,
    RAW_TRUNK_PORT_SECOND,
    RAW_TRUNK_PORT_THIRD,
    RAW_TRUNK_PORT_FOURTH,
    RAW_TRUNK_PORT_END
} raw_trunk_port_t;

extern int32 apollo_raw_trunk_memberPort_set(rtk_portmask_t *pPortmask);
extern int32 apollo_raw_trunk_memberPort_get(rtk_portmask_t *pPortmask);
extern int32 apollo_raw_trunk_hashAlgorithm_set(uint32 hashsel);
extern int32 apollo_raw_trunk_hashAlgorithm_get(uint32 *pHashsel);
extern int32 apollo_raw_trunk_flood_set(raw_trunk_floodMode_t mode);
extern int32 apollo_raw_trunk_flood_get(raw_trunk_floodMode_t *pMode);
extern int32 apollo_raw_trunk_mode_set(raw_trunk_mode_t mode);
extern int32 apollo_raw_trunk_mode_get(raw_trunk_mode_t *pMode);
extern int32 apollo_raw_trunk_flowControl_set(rtk_enable_t state);
extern int32 apollo_raw_trunk_flowControl_get(rtk_enable_t *pState);
extern int32 apollo_raw_trunk_hashMapping_set(uint32 hash_value, raw_trunk_port_t port);
extern int32 apollo_raw_trunk_hashMapping_get(uint32 hash_value, raw_trunk_port_t *pPort);

#endif /*#ifndef _APOLLO_RAW_TRUNK_H_*/

