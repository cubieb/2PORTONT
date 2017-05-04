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
 * Purpose : Definition of ME attribute: Traffic descriptor (280)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Traffic descriptor (280)
 */

#ifndef __MIB_TRAFFICDESCRIPTOR_TABLE_H__
#define __MIB_TRAFFICDESCRIPTOR_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table TrafficDescriptor attribute index */
#define MIB_TABLE_TRAFFICDESCRIPTOR_ATTR_NUM (9)
#define MIB_TABLE_TRAFFICDESCRIPTOR_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_TRAFFICDESCRIPTOR_CIR_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_TRAFFICDESCRIPTOR_PIR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_TRAFFICDESCRIPTOR_CBS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_TRAFFICDESCRIPTOR_PBS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_TRAFFICDESCRIPTOR_COLOUR_MODE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_TRAFFICDESCRIPTOR_INGRESS_COLOUR_MARKING_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_TRAFFICDESCRIPTOR_EGRESS_COLOUR_MARKING_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_TRAFFICDESCRIPTOR_METER_TYPE_INDEX ((MIB_ATTR_INDEX)9)

/* Table TrafficDescriptor attribute len, only string attrubutes have length definition */

typedef enum {
    TD_COLOUR_MODE_COLOUR_BLIND,
    TD_COLOUR_MODE_COLOUR_AWARE,
} td_attr_colour_mode_t;

typedef enum {
    TD_INGRESS_COLOUR_NO_MARKING,
    TD_INGRESS_COLOUR_DEI_MARKING,
    TD_INGRESS_COLOUR_PCP_8P0D_MARKING,
    TD_INGRESS_COLOUR_PCP_7P1D_MARKING,
    TD_INGRESS_COLOUR_PCP_6P2D_MARKING,
    TD_INGRESS_COLOUR_PCP_5P3D_MARKING,
    TD_INGRESS_COLOUR_DSCP_AF_CLASS_MARKING,
} td_attr_ingress_colour_marking_t;

typedef enum {
    TD_EGRESS_COLOUR_NO_MARKING,
    TD_EGRESS_COLOUR_INTERNAL_MARKING,
    TD_EGRESS_COLOUR_DEI_MARKING,
    TD_EGRESS_COLOUR_PCP_8P0D_MARKING,
    TD_EGRESS_COLOUR_PCP_7P1D_MARKING,
    TD_EGRESS_COLOUR_PCP_6P2D_MARKING,
    TD_EGRESS_COLOUR_PCP_5P3D_MARKING,
    TD_EGRESS_COLOUR_DSCP_AF_CLASS_MARKING,
} td_attr_egress_colour_marking_t;

typedef enum {
    TD_METER_TYPE_NOT_SPECIFIED,
    TD_METER_TYPE_RFC_4115,
    TD_METER_TYPE_RFC_2698,
} td_attr_meter_type_t;

typedef struct {
	UINT16   EntityId;
	UINT32   CIR;
	UINT32   PIR;
	UINT32   CBS;
	UINT32   PBS;
	UINT8    ColourMode;
	UINT8    IngressColourMarking;
	UINT8    EgressColourMarking;
	UINT8    MeterType;
} __attribute__((aligned)) MIB_TABLE_TRAFFICDESCRIPTOR_T;


#ifdef __cplusplus
}
#endif

#endif
