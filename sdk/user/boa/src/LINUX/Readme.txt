Simple Flash File System
  Origial from RTL8181 Project. (David Hsu)
  Modified by Dick Tam (dicktam@realtek.com.tw, EXT3523) for RTL8670 ADSL Router Project.
  Version 2003/07/18

---------------------------------------------------------------------------------------------------
[File Description]
mib.c mib.h mibtbl.c mibtbl.h: Flash File System API (included MIB handler)
flash.c: Command line utility based on Flash File System API
compweb.c: Use to compress(gzip) web pages
others: Goahead 2.1.4 web page handler

---------------------------------------------------------------------------------------------------
[Flash File System Data Banks]

[Three Data Banks]          User User    Engineer
(1) CURRENT SETTING (CS)    R/W          R/W
(2) DEFAULT SETTING (DS)    R            R/W
(3) HW SETTING (HS)         R            R/W
Note that DS is the backup data bank of CS,if CS is distroyed by user, we can restore CS from DS.

[Data Bank Structure]

    | ----------------------------|
    | Data Bank Header            |
    | ----------------------------| 
    | MIB Content                 | 
    | ----------------------------|
    | MIB Chain Record Header # 1 |
    | ----------------------------|
    | MIB Chain Record # 1        |
    | ----------------------------|
    | MIB Chain Record Header # 2 |
    | ----------------------------|
    | MIB Chain Record # 2        |
    | ----------------------------|
    |    .                        |
    |    .                        |
    |    .                        |
    | ----------------------------|
    | MIB Chain Record Header # N |
    | ----------------------------|
    | MIB Chain Record # N        |
    | ----------------------------|

Data Bank Header:
  (1) 8-Byte Signature
  (2) 1-Byte Version (for future use)
  (3) 1-Byte Checksum (checksum of the whole data bank)
  (4) 4-Byte Length (length of the whole data bank)

MIB Content:
  The MIB data structure
  (1) CURRENT SETTING (CS)    struct MIB_T (mib.h)
  (2) DEFAULT SETTING (DS)    struct MIB_T (mib.h)
  (3) HW SETTING (HS)         struct HW_MIB_T (mib.h)

MIB Chain Record Header:
  (1) 2-Byte ID (ID of the Chain Record)
  (2) 4-Byte Length (Length of the Chain Record)

MIB Chain Record:
  Chain Record data

---------------------------------------------------------------------------------------------------
[Flash File System API]

[mib.h]

/* Write the specified setting to flash, this function will also check the length and checksum */
int mib_update_from_raw(unsigned char* ptr, int len); 

/* Load flash setting to the specified pointer */
int mib_read_to_raw(CONFIG_DATA_T data_type, unsigned char* ptr, int len);

/* Update RAM setting to flash */
int mib_update(CONFIG_DATA_T data_type);

/* Load flash header */
int mib_read_header(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader);

/* Load flash setting to RAM */
int mib_load(CONFIG_DATA_T data_type);

/* Reset to default */
int mib_reset(CONFIG_DATA_T data_type);

/* Update Firmware */
int mib_update_firmware(unsigned char* ptr, int len); 

/* Initialize */
int mib_init(void);

/* get mib value */
int mib_get(int id, void *value);

/* set mib value */
int mib_set(int id, void *value); 

/* get chain record size */
unsigned int mib_chain_total(int id);

/* clear chain record */
void mib_chain_clear(int id);

/* add chain record */
int mib_chain_add(int id, unsigned char* ptr);

/* delete the specified chain record */
int mib_chain_delete(int id, unsigned int recordNum);

/* get the specified chain record */
unsigned char* mib_chain_get(int id, unsigned int recordNum);

---------------------------------------------------------------------------------------------------
[Example]

*************************************************
Example 1 : How to add a MIB record

(1) define a MIB ID in mib.h

#define MIB_HOME_GW_IP						100

(2) add new record to MIB data structure (MIB_T, or HW_MIB_T) in mib.h

typedef struct config_setting {
...
	unsigned char home_gw_ip[4];
} __PACK__ HW_MIB_T, *HW_MIB_Tp;

(3) add MIB value, id mapping table (mib_table) in mibtbl.c

mib_table_entry_T mib_table[]={
...

MIB_TBL_ENTRY(MIB_HOME_GW_IP,	CURRENT_SETTING,	"HOME_GW_IP", 		IA_T,		_OFFSET(home_gw_ip),		_SIZE(home_gw_ip),			"172.19.31.250"),
};

    Entry
    [1]  MIB ID
    [2]  Data Bank (CURRENT_SETTING or HW_SETTING)
    [3]  record name
    [4]  record type (BYTE_T, WORD_T, DWORD_T, INTEGER_T, STRING_T, BYTE6_T, or IA_T)
    [5]  offset from the start of the MIB data structure (MIB_T, or HW_MIB_T)
    [6]  size of the MIB data structure
    [7]  default value (INCLUDE_DEFAULT_VALUE compile option is required)

*************************************************
Example 2 : How to add a MIB chain record (table)

(1) define a MIB TABLE ID in mib.h

#define MIB_IP_FILTER_TBL						200

(2) define the table structure in mib.h

typedef struct ipfilter_entry {
	unsigned char ipAddr[4];
	unsigned char protoType;
} __PACK__ MIB_CE_IP_FILTER_T;

(3) add MIB value, id mapping table (mib_chain_record_table) in mibtbl.c

mib_chain_record_table_entry_T mib_chain_record_table[]={
...
{MIB_IP_FILTER_TBL,		CURRENT_SETTING,	"IP_FILTER_TBL",		sizeof(MIB_CE_IP_FILTER_T),		0},
};

    Entry
    [1]  MIB ID
    [2]  Data Bank (CURRENT_SETTING or HW_SETTING)
    [3]  record name
    [4]  size of the MIB chain record data
    [5]  chain record pointer, must be NULL

*************************************************
Example 3 : MIB record read/write

test() {
	struct in_addr oldIpAddr, newSubnet;

	if ( mib_init() != 1 ) /* Initialize */ {
		printf("Initialize MIB failed!\n");
		return -1;
	}
	
    if(mib_get( MIB_HOME_GW_IP,  (void *)&oldIpAddr) != 1 ) /* get mib value */ {
		printf("get MIB failed!\n");
		return -1;
	}

    if(mib_set( MIB_HOME_GW_IP,  (void *)&newSubnet) != 1 ) /* set mib value */ {
		printf("set MIB failed!\n");
		return -1;
	}

	if ( mib_update(CURRENT_SETTING) != 1 ) /* Update RAM setting to flash */ {
		printf("update MIB failed!\n");
		return -1;
	}
}

*************************************************
Example 4 : MIB chain record read/write

test() {
    MIB_CE_IP_FILTER_T *pIpFilter;
    unsigned int totalChainRecord;
    unsigned int idx;

	if ( mib_init() != 1 ) /* Initialize */ {
		printf("Initialize MIB failed!\n");
		return -1;
	}

    totalChainRecord = mib_chain_total(MIB_IP_FILTER_TBL); /* get chain record size */

    for(idx=0;idx<totalChainRecord;idx++) {
        pIpFilter = (MIB_CE_IP_FILTER_T *) mib_chain_get(MIB_IP_FILTER_TBL, idx); /* get the specified chain record */
        
        if(pIpFilter == NULL) {
		    printf("get MIB chain record failed!\n");
		   return -1;
	    }

    }

    if(mib_chain_add( MIB_IP_FILTER_TBL,  (unsigned char*)pIpFilter) != 1 ) /* add chain record */ {
		printf("add MIB chain record failed!\n");
		return -1;
	}

    // you can use mib_chain_delete, or mib_chain_clear to erase chain record

	if ( mib_update(CURRENT_SETTING) != 1 ) /* Update RAM setting to flash */ {
		printf("update MIB failed!\n");
		return -1;
	}
}

*************************************************
Example 5 : Flash read/write

(1) int mib_init(void); /* Initialize */
  [1] read CURRENT SETTING data bank from flash, if fail read DEFAULT SETTING data bank 
  [2] read HW SETTING from flash

(2) int mib_update(CONFIG_DATA_T data_type); /* Update RAM setting to flash */
  This function is use to write MIB value back to flash after MIB value change.
 
  mib_init()
  mib_set(...)
  mib_chain_add(...)
  mib_update(CURRENT_SETTING);

(3) int mib_reset(CONFIG_DATA_T data_type); /* Reset to default */
  This function will read DEFAULT SETTING data bank , and then write to CURRENT SETTING data bank




