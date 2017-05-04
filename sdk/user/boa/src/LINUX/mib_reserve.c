#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <arpa/inet.h>
#include <netinet/in.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mib.h"
#include "mibtbl.h"
#include "mib_reserve.h"

#ifdef WEB_DEBUG_MSG
#define	TRACE	printf
#else
#define	TRACE
#endif

//ql add for key parameter reserving.
extern MIB_T table_backup;
extern unsigned char *chain_backup;
extern unsigned int backupChainSize;

#if 0
void mib_record_reserve(CONFIG_MIB_T type)
{
	unsigned char *pMibTbl;

//	if (type == CONFIG_MIB_ALL || type == CONFIG_MIB_TABLE) {
	if (type == CONFIG_MIB_TABLE) {
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(&table_bak, pMibTbl, sizeof(MIB_T));  //save setting
	}
	else if (type == CONFIG_MIB_CHAIN){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int chainRecordSize, mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if (chain_backup)	 free(chain_backup);
		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		backupChainSize = __mib_chain_all_table_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(_mib_read_to_raw(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		if (backupChainSize > 0) {
			pVarLenTable = &ptr[mibTblSize];
			if(__mib_chain_record_content_encod(CURRENT_SETTING, pVarLenTable, backupChainSize) != 1) {
				free(buf);
				return;
			}
		}
		chain_backup = (unsigned char *)malloc(backupChainSize);
		memcpy(chain_backup, pVarLenTable, backupChainSize);  //save MIB chain setting
		free(buf);
//		return;
	}
	else if (type == CONFIG_MIB_ALL){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int chainRecordSize, mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if (chain_backup)	 free(chain_backup);
		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		backupChainSize = __mib_chain_all_table_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(_mib_read_to_raw(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(&table_bak, pMibTbl, sizeof(MIB_T));  //save MIB table setting
		if (backupChainSize > 0) {
			pVarLenTable = &ptr[mibTblSize];
			if(__mib_chain_record_content_encod(CURRENT_SETTING, pVarLenTable, backupChainSize) != 1) {
				free(buf);
		return;
			}
		}
		chain_backup = (unsigned char *)malloc(backupChainSize);
		memcpy(chain_backup, pVarLenTable, backupChainSize);  //save MIB chain setting
		free(buf);
//		return;
	}
	else
		return;
}
#endif
int mib_table_record_retrive(int id) /* get mib value from backup mib Info*/
{
	int i;
	char value[512];
	unsigned char * pMibTbl;

	// search current setting mib table
	for (i=0; mib_table[i].id; i++) {
		if ( mib_table[i].id == id )
		{
//			TRACE("mib_get %s\n",mib_table[i].name);
			break;
		}
	}

	if((mib_table[i].mib_type != CURRENT_SETTING) &&
		(mib_table[i].mib_type != HW_SETTING))
	{
		TRACE("mib_get id=%d unknown\n",id);
		return 0;
	}

	pMibTbl = (unsigned char *)&table_backup;
	memset(value, 0, 512);

	switch (mib_table[i].type) {
	case STRING_T:
		strcpy((char *)value, (const char *)(pMibTbl + mib_table[i].offset));
		break;

	case BYTE_T:
	case WORD_T:
	case DWORD_T:
	case INTEGER_T:
	case BYTE5_T:
	case BYTE6_T:
	case BYTE13_T:
	case IA_T:
#ifdef CONFIG_IPV6
	case IA6_T:
#endif
	case BYTE_ARRAY_T:
		memcpy(value, pMibTbl + mib_table[i].offset, mib_table[i].size);
		break;

	default:
		TRACE("mib_get fail!\n");
		return 0;
	}

	_mib_set(id, value);

	return 1;
}

int mib_chain_record_retrive(int id)
{
	unsigned int idx = 0;
	unsigned char *ptr = chain_backup;
	unsigned int len = backupChainSize;

	while(idx < len)
	{
		int i;
		int numOfRecord;
		MIB_CHAIN_RECORD_HDR_Tp pCRHeader = (MIB_CHAIN_RECORD_HDR_Tp) (ptr + idx);

		idx += sizeof(MIB_CHAIN_RECORD_HDR_T);

		// search chain record mib table
		for (i=0; mib_chain_record_table[i].id; i++) {
			if ( mib_chain_record_table[i].id == pCRHeader->id )
			{
				break;
			}
		}

		if ( mib_chain_record_table[i].id == 0 )		// ID NOT found
		{
			TRACE("chain record id(%d) NOT found!\n",pCRHeader->id);
			return 0;
		}

		if((idx + pCRHeader->len) > len)	// check record size
		{
			TRACE("invalid chain record size! Header len(%u), len(%u)\n",pCRHeader->len, len - idx);
			return 0;
		}

		if((pCRHeader->len % mib_chain_record_table[i].per_record_size) != 0)		// check record size
		{
			TRACE("invalid chain record size! len(%d), record size(%d)\n",pCRHeader->len, mib_chain_record_table[i].per_record_size);
			return 0;
		}

		if ( mib_chain_record_table[i].id != id )
		{
			idx += pCRHeader->len;
			continue;
		}

		numOfRecord = pCRHeader->len / mib_chain_record_table[i].per_record_size;

		//remove existing record
		__mib_chain_clear(mib_chain_record_table[i].id);

//		TRACE("chain record decod %s, %d record\n",mib_chain_record_table[i].name, numOfRecord);
		while(numOfRecord > 0)
		{
			if(__mib_chain_add(pCRHeader->id, ptr+idx) != 1)
			{
				TRACE("add chain record fail!\n");
				return 0;
			}

			numOfRecord--;
			idx += mib_chain_record_table[i].per_record_size;
		}
		return 1;
	}

	return 1;
}

//end ql
