#ifndef INCLUDE_UC_MIB_H
#define INCLUDE_UC_MIB_H

/* ADSL Slave MIB ID */
#define UC_CS_ENTRY_ID								0
#define UC_MIB_ELAN_MAC_ADDR						UC_CS_ENTRY_ID + 1
#define UC_MIB_ADSL_LAN_IP							UC_CS_ENTRY_ID + 2
#define UC_MIB_ADSL_LAN_SUBNET						UC_CS_ENTRY_ID + 3
#define UC_MIB_ADSL_MODE							UC_CS_ENTRY_ID + 4
#define UC_MIB_ADSL_OLR								UC_CS_ENTRY_ID + 5
#define UC_MIB_DEVICE_TYPE							UC_CS_ENTRY_ID + 6
#define UC_MIB_INIT_LINE							UC_CS_ENTRY_ID + 7
#define UC_MIB_ADSL_TONE							UC_CS_ENTRY_ID + 8
#define UC_MIB_ADSL_HIGH_INP						UC_CS_ENTRY_ID + 9
#ifdef FIELD_TRY_SAFE_MODE
#define UC_MIB_ADSL_FIELDTRYSAFEMODE				UC_CS_ENTRY_ID + 10
#define UC_MIB_ADSL_FIELDTRYTESTPSDTIMES			UC_CS_ENTRY_ID + 11
#define UC_MIB_ADSL_FIELDTRYCTRLIN					UC_CS_ENTRY_ID + 12
#endif
#ifdef CONFIG_VDSL
#define UC_MIB_VDSL2_PROFILE						UC_CS_ENTRY_ID + 13
#endif /*CONFIG_VDSL*/


#define UC_MAC_ADDR_LEN			6
#define UC_IP_ADDR_LEN			4
#define UC_MAX_ADSL_TONE		64


					 
typedef enum { 
	UC_UNKNOWN_SETTING=0, 
	UC_RUNNING_SETTING=1, 
	UC_HW_SETTING=2, 
	UC_DEFAULT_SETTING=4, 
	UC_CURRENT_SETTING=8 
} UC_CONFIG_DATA_T;

typedef enum 
{ 
	UC_BYTE_T, 
	UC_WORD_T,
	UC_DWORD_T, 
	UC_INTEGER_T, 
	UC_STRING_T,
	UC_BYTE5_T, 
	UC_BYTE6_T,
	UC_BYTE13_T, 
	UC_IA_T,
	UC_IA6_T,
	UC_BYTE_ARRAY_T,
//	WDS_ARRAY_T,
//	BYTE64_T=64,
} UC_TYPE_T;

// MIB value, id mapping table
typedef struct _uc_mib_table_entry {
	int			id;
	UC_CONFIG_DATA_T mib_type;
	char		name[32];
	UC_TYPE_T	type;
	int			offset;
	int			size;
	const char*	defaultValue;
} uc_mib_table_entry_T;

#define UC_MIB_TBL_ENTRY(ID, CLASS, NAME, TYPE, OFFSET, SIZE, DEFAULT) {ID, CLASS, NAME, TYPE, OFFSET, SIZE, DEFAULT}
#define UC_FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))
#define _UC_OFFSET(field)				((int)UC_FIELD_OFFSET(UC_MIB_T,field))
#define _UC_SIZE(field)					sizeof(((UC_MIB_T *)0)->field)

typedef struct uc_config_setting 
{
	unsigned char  ipAddr[UC_IP_ADDR_LEN];
	unsigned char  subnetMask[UC_IP_ADDR_LEN];
	unsigned char  elanMacAddr[UC_MAC_ADDR_LEN];
	unsigned char  deviceType; // bridge: 0 or router: 1
	unsigned char  initLine; // init adsl line on startup
	unsigned short adslMode;	// 1: ANSI T1.413, 2: G.dmt, 3: multi-mode, 4: ADSL2, 8: AnnexL, 16: ADSL2+
	unsigned char  adslOlr;	// adsl capability, 0: disable 1: bitswap 3: SRA & bitswap
	unsigned char  adslHighInp; // adsl high INP flag
	unsigned char  adslTone[UC_MAX_ADSL_TONE];      // ADSL Tone
#ifdef FIELD_TRY_SAFE_MODE
	unsigned char  adslFtMode;
	unsigned int   adslFtTestPSDTimes;
	unsigned int   adslFtCtrlIn;
#endif
#ifdef CONFIG_VDSL
	unsigned short vdsl2Profile; //refer to VDSL2_PROFILE_xxx
#endif /*CONFIG_VDSL*/
} UC_MIB_T, *UC_MIB_Tp;


void uc_mib_init(void);
void uc_mib_dump(void);
int uc_mib_set_by_name(char* name, char *value);
int uc_mib_get_by_name(char* name, void *value);
int uc_mib_set(int id, void *value);
int uc_mib_get(int id, void *value);
int uc_mib_get_size_by_id(int id);
void uc_mib_dump_info(void);
#endif /*INCLUDE_UC_MIB_H*/

