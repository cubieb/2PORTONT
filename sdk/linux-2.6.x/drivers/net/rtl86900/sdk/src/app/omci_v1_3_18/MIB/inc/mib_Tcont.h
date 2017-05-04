/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of ME attribute: T-CONT (262)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: T-CONT (262)
 */

#ifndef __MIB_TCONT_TABLE_H__
#define __MIB_TCONT_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table Tcont attribute index */
#define MIB_TABLE_TCONT_ATTR_NUM (4)
#define MIB_TABLE_TCONT_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_TCONT_ALLOCID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_TCONT_MODEIND_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_TCONT_POLICY_INDEX ((MIB_ATTR_INDEX)4)


/* Table Tcont attribute len, only string attrubutes have length definition */

#define TCONT_ALLOC_ID_984_MIN				(0x0)
#define TCONT_ALLOC_ID_984_MAX				(0x0FFF)
#define TCONT_ALLOC_ID_984_RESERVED			(0x00FF)
#define TCONT_ALLOC_ID_987_MIN				(0x0)
#define TCONT_ALLOC_ID_987_MAX				(0x3FFF)
#define TCONT_ALLOC_ID_984_987_RESERVED		(0xFFFF)

#define m_TCONT_IS_ALLOC_ID_984_RESERVED(x) \
			((x) == TCONT_ALLOC_ID_984_RESERVED || \
			(x) == TCONT_ALLOC_ID_984_987_RESERVED)
#define m_TCONT_IS_ALLOC_ID_984_LEGAL(x) \
			((x) >= TCONT_ALLOC_ID_984_MIN && \
			(x) <= TCONT_ALLOC_ID_984_MAX && \
			!m_TCONT_IS_ALLOC_ID_984_RESERVED(x))
#define m_TCONT_IS_ALLOC_ID_987_RESERVED(x) \
			((x) == TCONT_ALLOC_ID_984_987_RESERVED)
#define m_TCONT_IS_ALLOC_ID_987_LEGAL(x) \
			((x) >= TCONT_ALLOC_ID_987_MIN && \
			(x) <= TCONT_ALLOC_ID_987_MAX && \
			!m_TCONT_IS_ALLOC_ID_987_RESERVED(x))

typedef enum {
	TCONT_POLICY_NULL,
	TCONT_POLICY_STRICT_PRIORITY,
	TCONT_POLICY_WEIGHTED_ROUND_ROBIN,
} tcont_attr_policy_t;

// Table Tcont entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 AllocID;
    UINT8  ModeInd;
    UINT8  Policy;
} __attribute__((aligned)) MIB_TABLE_TCONT_T;


#ifdef __cplusplus
}
#endif

#endif
