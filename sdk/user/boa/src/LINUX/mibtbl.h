/*
 *      Include file of mibtbl.c
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#ifndef INCLUDE_MIBTBL_H
#define INCLUDE_MIBTBL_H

#include "mib.h"

#ifdef WIN32
#ifdef FIELD_OFFSET
#undef FIELD_OFFSET
#endif
#endif

#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))
#define _OFFSET(field)			((int)FIELD_OFFSET(MIB_T,field))
#define _SIZE(field)			sizeof(((MIB_T *)0)->field)
#define _OFFSET_CE(type, field)		((int)(long *)&(((type *)0)->field))
#define _SIZE_CE(type, field)		sizeof(((type *)0)->field)

#define _OFFSET_HW(field)		((int)FIELD_OFFSET(HW_MIB_T,field))
#define _SIZE_HW(field)			sizeof(((HW_MIB_T *)0)->field)

#define _OFFSET_RS(field)		((int)FIELD_OFFSET(RS_MIB_T,field))
#define _SIZE_RS(field)			sizeof(((RS_MIB_T *)0)->field)

typedef enum 
{ 
	BYTE_T, 
	WORD_T,
	DWORD_T, 
	INTEGER_T, 
	STRING_T,
	BYTE5_T, 
	BYTE6_T,
	BYTE13_T, 
	IA_T,
	IA6_T,
	BYTE_ARRAY_T,
	WORD_ARRAY_T,
	INT_ARRAY_T,
#ifdef WLAN_WDS
	WDS_ARRAY_T,
#endif
	BYTE64_T=64,
	OBJECT_T,
} TYPE_T;


// MIB value, id mapping table
typedef struct _mib_table_entry {
	int id;
	CONFIG_DATA_T mib_type;
	char name[48];
	TYPE_T type;
	int offset;
	int size;
#ifdef INCLUDE_DEFAULT_VALUE
	const char *defaultValue;
#endif
} mib_table_entry_T;

// Kaohj
// MIB chain record-member descriptor
typedef struct _mib_chain_member_entry {
	char name[32];
	TYPE_T type;
	int offset;
	int size;
	struct _mib_chain_member_entry *record_desc;
} mib_chain_member_entry_T;

#ifdef INCLUDE_DEFAULT_VALUE
#define MIB_TBL_ENTRY(ID, CLASS, NAME, TYPE, OFFSET, SIZE, DEFAULT) {ID, CLASS, NAME, TYPE, OFFSET, SIZE, DEFAULT}
#else
#define MIB_TBL_ENTRY(ID, CLASS, NAME, TYPE, OFFSET, SIZE, DEFAULT) {ID, CLASS, NAME, TYPE, OFFSET, SIZE }
#endif

#ifdef INCLUDE_DEFAULT_VALUE
#define MIB_CHAIN_TBL_ENTRY(ID, CLASS, NAME, SIZE, MAXNUM, ENTRY, DEFAULT, DEFAULT_SIZE, CHAIN_DESC) {ID, CLASS, NAME, SIZE, MAXNUM, ENTRY, DEFAULT, DEFAULT_SIZE, CHAIN_DESC}
#else
#define MIB_CHAIN_TBL_ENTRY(ID, CLASS, NAME, SIZE, MAXNUM, ENTRY, DEFAULT, DEFAULT_SIZE) {ID, CLASS, NAME, SIZE, MAXNUM, ENTRY }
#endif


// Chain Record MIB value, id mapping table
typedef struct _mib_chain_record_table_entry {
	int id;
	CONFIG_DATA_T mib_type;
	char name[32];
	int per_record_size;
	int table_size;				// -1: infinite
	MIB_CHAIN_ENTRY_Tp pChainEntry;
#ifdef INCLUDE_DEFAULT_VALUE
	const void *defaultValue;
	int	defaultSize;
#endif
	// Kaohj --- record info.
	mib_chain_member_entry_T *record_desc;
} mib_chain_record_table_entry_T;

extern mib_table_entry_T mib_table[];
extern const unsigned int mib_table_size;
extern mib_chain_record_table_entry_T mib_chain_record_table[];

#endif // INCLUDE_MIBTBL_H
