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
 * Purpose : Definition of ME attribute: Large string (157)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Large string (157)
 */

#ifndef __MIB_LARGE_STRING_TABLE_H__
#define __MIB_LARGE_STRING_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_LARGE_STRING_ATTR_NUM (17)
#define MIB_TABLE_LARGE_STRING_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_LARGE_STRING_NUM_OF_PARTS_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_LARGE_STRING_PART_1_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_LARGE_STRING_PART_2_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_LARGE_STRING_PART_3_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_LARGE_STRING_PART_4_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_LARGE_STRING_PART_5_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_LARGE_STRING_PART_6_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_LARGE_STRING_PART_7_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_LARGE_STRING_PART_8_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_LARGE_STRING_PART_9_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_LARGE_STRING_PART_10_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_LARGE_STRING_PART_11_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_LARGE_STRING_PART_12_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_LARGE_STRING_PART_13_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_LARGE_STRING_PART_14_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_LARGE_STRING_PART_15_INDEX ((MIB_ATTR_INDEX)17)

#define MIB_TABLE_LARGE_STRING_PART_LEN (25)


typedef struct {
    UINT16  EntityId;
    UINT8   NumOfParts;
    CHAR    Part1[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part2[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part3[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part4[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part5[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part6[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part7[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part8[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part9[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part10[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part11[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part12[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part13[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part14[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
    CHAR    Part15[MIB_TABLE_LARGE_STRING_PART_LEN + 1];
} __attribute__((aligned)) MIB_TABLE_LARGE_STRING_T;


#ifdef __cplusplus
}
#endif

#endif
