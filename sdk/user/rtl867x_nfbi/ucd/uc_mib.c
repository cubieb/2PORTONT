#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "uc_mib.h"

static UC_MIB_T uc_cs_mib;
static uc_mib_table_entry_T uc_mib_table[]=
{
UC_MIB_TBL_ENTRY(UC_MIB_ELAN_MAC_ADDR,			UC_CURRENT_SETTING, 	"ELAN_MAC_ADDR",			UC_BYTE6_T,		_UC_OFFSET(elanMacAddr),		_UC_SIZE(elanMacAddr),	"00E04C867701"),
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_LAN_IP,			UC_CURRENT_SETTING,		"LAN_IP_ADDR",				UC_IA_T,		_UC_OFFSET(ipAddr),				_UC_SIZE(ipAddr),		"192.168.1.17"),
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_LAN_SUBNET,		UC_CURRENT_SETTING,		"LAN_SUBNET",				UC_IA_T,		_UC_OFFSET(subnetMask),			_UC_SIZE(subnetMask),	"255.255.255.0"),
UC_MIB_TBL_ENTRY(UC_MIB_DEVICE_TYPE,			UC_CURRENT_SETTING,		"DEVICE_TYPE",				UC_BYTE_T,		_UC_OFFSET(deviceType), 		_UC_SIZE(deviceType),	"9"),
UC_MIB_TBL_ENTRY(UC_MIB_INIT_LINE,				UC_CURRENT_SETTING,		"INIT_LINE",				UC_BYTE_T,		_UC_OFFSET(initLine), 			_UC_SIZE(initLine),		"1"),
#ifdef CONFIG_VDSL
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_MODE,				UC_CURRENT_SETTING,		"ADSL_MODE",				UC_WORD_T,		_UC_OFFSET(adslMode), 			_UC_SIZE(adslMode),		"1024"),
#else
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_MODE,				UC_CURRENT_SETTING,		"ADSL_MODE",				UC_WORD_T,		_UC_OFFSET(adslMode), 			_UC_SIZE(adslMode),		"83"),
#endif
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_OLR,				UC_CURRENT_SETTING,		"ADSL_OLR",					UC_BYTE_T,		_UC_OFFSET(adslOlr), 			_UC_SIZE(adslOlr),		"3"),
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_TONE,	 	 		UC_CURRENT_SETTING,		"ADSL_TONE",				UC_BYTE_ARRAY_T,_UC_OFFSET(adslTone), 			_UC_SIZE(adslTone),		"00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"),    // Added by Mason Yu for correct Tone Mib Type
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_HIGH_INP,			UC_CURRENT_SETTING,		"ADSL_HIGH_INP",			UC_BYTE_T,		_UC_OFFSET(adslHighInp), 		_UC_SIZE(adslHighInp),	"0"),
#ifdef FIELD_TRY_SAFE_MODE
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_FIELDTRYSAFEMODE, 		UC_CURRENT_SETTING,	"ADSL_FIELDTRYSAFEMODE", 	UC_BYTE_T, 		_UC_OFFSET(adslFtMode),			_UC_SIZE(adslFtMode), 			"0"),
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_FIELDTRYTESTPSDTIMES, 	UC_CURRENT_SETTING,	"ADSL_FIELDTRYTESTPSDTIMES",UC_INTEGER_T,	_UC_OFFSET(adslFtTestPSDTimes),	_UC_SIZE(adslFtTestPSDTimes), 	"50"),
UC_MIB_TBL_ENTRY(UC_MIB_ADSL_FIELDTRYCTRLIN,		UC_CURRENT_SETTING,	"ADSL_FIELDTRYCTRLIN",		UC_INTEGER_T,	_UC_OFFSET(adslFtCtrlIn),		_UC_SIZE(adslFtCtrlIn),			"0"),
#endif
#ifdef CONFIG_VDSL
UC_MIB_TBL_ENTRY(UC_MIB_VDSL2_PROFILE,			UC_CURRENT_SETTING,		"VDSL2_PROFILE",			UC_WORD_T,		_UC_OFFSET(vdsl2Profile), 		_UC_SIZE(vdsl2Profile),		"255"),
#endif /*CONFIG_VDSL*/
{0}
};	


static int uc_is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int uc_string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) 
	{
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !uc_is_hex(tmpBuf[0]) || !uc_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static unsigned char * uc_mib_get_mib_tbl(UC_CONFIG_DATA_T data_type)
{
	switch(data_type)
	{
		case UC_CURRENT_SETTING:
			return (unsigned char *) &uc_cs_mib;;
		default:
			return NULL;
	}
}

static int uc_mib_init_default(void)
{
	unsigned char * pMibTbl;
	int idx;
	int i;
	unsigned char ch;
	unsigned short wd;
	unsigned int dwd;
	unsigned char buffer[512];
	struct in_addr addr;
#ifdef CONFIG_IPV6
	struct in6_addr addr6;
#endif
	UC_CONFIG_DATA_T mib_table_data_type;
	UC_CONFIG_DATA_T data_type;

	printf("Load from program default!\n");
	data_type=UC_CURRENT_SETTING;
	pMibTbl = uc_mib_get_mib_tbl(data_type);
	if(pMibTbl == 0)
	{
		printf("Error: uc_mib_get_mib_tbl() fail!\n");
		return 0;
	}

	mib_table_data_type = data_type;
	memset(pMibTbl,0x00,sizeof(UC_MIB_T) );
	for (idx=0; uc_mib_table[idx].id; idx++) {
		if(mib_table_data_type != uc_mib_table[idx].mib_type)
			continue;

		if(uc_mib_table[idx].defaultValue == 0)
			continue;

		printf("uc_mib_init_default %s=%s, table type=%d\n",uc_mib_table[idx].name, uc_mib_table[idx].defaultValue, data_type);

		switch (uc_mib_table[idx].type) {
		case UC_BYTE_T:
			sscanf(uc_mib_table[idx].defaultValue,"%u",&dwd);
			ch = (unsigned char) dwd;
			memcpy( ((char *)pMibTbl) + uc_mib_table[idx].offset, &ch, 1);
			break;

		case UC_WORD_T:
			sscanf(uc_mib_table[idx].defaultValue,"%u",&dwd);
			wd = (unsigned short) dwd;
			memcpy( ((char *)pMibTbl) + uc_mib_table[idx].offset, &wd, 2);
			break;

		case UC_DWORD_T:
			sscanf(uc_mib_table[idx].defaultValue,"%u",&dwd);
			memcpy( ((char *)pMibTbl) + uc_mib_table[idx].offset, &dwd, 4);
			break;

		case UC_INTEGER_T:
			sscanf(uc_mib_table[idx].defaultValue,"%d",&i);
			memcpy( ((char *)pMibTbl) + uc_mib_table[idx].offset, &i, 4);
			break;

		case UC_STRING_T:
			if ( strlen(uc_mib_table[idx].defaultValue) < uc_mib_table[idx].size )
			{
				strcpy((char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (char *)uc_mib_table[idx].defaultValue);
			}			
			break;

		case UC_BYTE5_T:
			uc_string_to_hex((char *)uc_mib_table[idx].defaultValue, buffer, 10);
			memcpy((unsigned char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (unsigned char *)buffer, 5);
			break;

		case UC_BYTE6_T:
			uc_string_to_hex((char *)uc_mib_table[idx].defaultValue, buffer, 12);
			memcpy((unsigned char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (unsigned char *)buffer, 6);
			break;

		case UC_BYTE13_T:
			uc_string_to_hex((char *)uc_mib_table[idx].defaultValue, buffer, 26);
			memcpy((unsigned char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (unsigned char *)buffer, 13);
			break;

		case UC_BYTE_ARRAY_T:
			for (i=0; i<uc_mib_table[idx].size; i++)
			{
				int val;
				sscanf(&uc_mib_table[idx].defaultValue[i*3],"%d %*d",&val);
				*(((char *)pMibTbl) + uc_mib_table[idx].offset + i) = (char)val;
			}
			break;
			
		case UC_IA_T:
			addr.s_addr = inet_addr(uc_mib_table[idx].defaultValue);
			memcpy((unsigned char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (unsigned char *)(&addr),  4);
			break;

		#ifdef CONFIG_IPV6
		case UC_IA6_T:
			inet_pton(PF_INET6, uc_mib_table[idx].defaultValue, &addr6);
			memcpy((unsigned char *)(((long)pMibTbl) + uc_mib_table[idx].offset), (unsigned char *)(&addr6),  16);
			break;
		#endif
		
		default:
			break;
		}

	}

	return 1;
}


static int uc_mib_info_id(int id, uc_mib_table_entry_T *t)
{
	int k;

	if(!t) return 0;

	for (k=0; uc_mib_table[k].id; k++) {
		if (uc_mib_table[k].id == id)
			break;
	}

	if (uc_mib_table[k].id == 0)
		return 0;

	memcpy((void *)t, (void *)&uc_mib_table[k], sizeof(uc_mib_table_entry_T));
	return 1;
}

int uc_mib_get_size_by_id(int id)
{
	uc_mib_table_entry_T t;
	
	if(uc_mib_info_id(id, &t)) return t.size;

	return 0;
}

int uc_mib_get(int id, void *value) /* get mib value */
{
	int i;

	unsigned char * pMibTbl;
	uc_mib_table_entry_T *pTbl = uc_mib_table;

	// search current setting mib table
	for (i=0; uc_mib_table[i].id; i++) {
		if ( uc_mib_table[i].id == id )
		{
			//printf("uc_mib_get %s\n",uc_mib_table[i].name);
			break;
		}
	}

	//if((uc_mib_table[i].mib_type != UC_CURRENT_SETTING) &&
	//	(uc_mib_table[i].mib_type != UC_HW_SETTING))
	if(uc_mib_table[i].mib_type != UC_CURRENT_SETTING)
	{
		printf("uc_mib_get id=%d unknown\n",id);
		return 0;
	}

	pMibTbl = uc_mib_get_mib_tbl(uc_mib_table[i].mib_type);

	switch (pTbl[i].type) {
	case UC_BYTE_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 1);
		break;

	case UC_WORD_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 2);
		break;

	case UC_DWORD_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 4);
		break;

	case UC_INTEGER_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 4);
		break;

	case UC_STRING_T:
		strcpy( (char *)value, (const char *)(((long)pMibTbl) + pTbl[i].offset) );
		break;

	case UC_BYTE5_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 5);
		break;

	case UC_BYTE6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 6);
		break;

	case UC_BYTE13_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 13);
		break;
	
	case UC_IA_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 4);
		break;
	
#ifdef CONFIG_IPV6
	case UC_IA6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 16);
		break;
#endif
	
	case UC_BYTE_ARRAY_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;

	default:
		printf("uc_mib_get fail!\n");
		return 0;
		
	}
	
	return 1;
}

int uc_mib_set(int id, void *value) /* set mib value */
{
	int i;
	unsigned char * pMibTbl;
	uc_mib_table_entry_T *pTbl = uc_mib_table;
	
	// search current setting mib table
	for (i=0; uc_mib_table[i].id; i++) {
		if ( uc_mib_table[i].id == id )
		{
			//printf("uc_mib_set %s\n",uc_mib_table[i].name);
			break;
		}
	}

	//if((uc_mib_table[i].mib_type != UC_CURRENT_SETTING) &&
	//	(uc_mib_table[i].mib_type != UC_HW_SETTING))
	if(uc_mib_table[i].mib_type != UC_CURRENT_SETTING)
	{
		return 0;
	}

	pMibTbl = uc_mib_get_mib_tbl(uc_mib_table[i].mib_type);
#if 0
	// log message
	{
		int k;
		char buf[64];
		
		sprintf(buf, "0x");
		for (k=0; k<pTbl[i].size; k++)
			snprintf(buf, 64, "%s%.02x ", buf, *(((unsigned char *)pMibTbl)+pTbl[i].offset+k));
		printf("uc_mib_set: %s=%s%s\n", pTbl[i].name, buf, pTbl[i].size>=32?"...":"");
	}
#endif

	switch (pTbl[i].type) {
	case UC_BYTE_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 1);
		break;

	case UC_WORD_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 2);
		break;

	case UC_DWORD_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 4);
		break;

	case UC_INTEGER_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 4);
		break;

	case UC_STRING_T:
		if ( strlen(value)+1 > pTbl[i].size )
			return 0;
		strcpy((char *)(((long)pMibTbl) + pTbl[i].offset), (char *)value);
		break;

	case UC_BYTE5_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 5);
		break;

	case UC_BYTE6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 6);
		break;

	case UC_BYTE13_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 13);
		break;

	case UC_BYTE_ARRAY_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, pTbl[i].size);
		break;

	case UC_IA_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  4);
		break;

#ifdef CONFIG_IPV6
	case UC_IA6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  16);
		break;
#endif
	
	default:
		return 0;
	}

	return 1;
}


/*
 * Get MIB Value
 */
int uc_mib_get_by_name(char* name, void *value)
{
	int i, idx;
	int id = -1;
	unsigned char tmp[100];

	for (i=0; uc_mib_table[i].id; i++) {
		if ( !strcmp(uc_mib_table[i].name, name) )
		{
			id = uc_mib_table[i].id;
			idx = i;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[512];
		unsigned int vUInt;
		int	vInt;
		memset(buffer, 0x00 , 512);
		if(uc_mib_get(id, (void *)buffer)!=0)
		{
			switch (uc_mib_table[i].type) {
				case UC_IA_T:
					strcpy(value, inet_ntoa(*((struct in_addr *)buffer)));
					return 1;

				#ifdef CONFIG_IPV6
				case UC_IA6_T:
					inet_ntop(PF_INET6, buffer, value, 48);
					return 1;
				#endif

				case UC_BYTE_T:
					vUInt = (unsigned int) (*(unsigned char *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;
					
				case UC_WORD_T:
					vUInt = (unsigned int) (*(unsigned short *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case UC_DWORD_T:
					vUInt = (unsigned int) (*(unsigned int *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case UC_INTEGER_T:
					vInt = *((int *)buffer);
					sprintf(value,"%d",vInt);
					return 1;

				case UC_BYTE5_T:
					sprintf(value, "%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4]);
					return 1;

				case UC_BYTE6_T:
					sprintf(value, "%02x%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4], buffer[5]);
					return 1;

				case UC_BYTE13_T:
					sprintf(value, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4], buffer[5],
						buffer[6], buffer[7],buffer[8], buffer[9], buffer[10], buffer[11], buffer[12]);
					return 1;

				case UC_BYTE_ARRAY_T:
					for (i=0; i<uc_mib_table[idx].size; i++)
					{
#if 0
//#ifdef USE_11N_UDP_SERVER
//						if( !udp_server_cmd ) {
//#endif
//						sprintf(tmp, "%d", buffer[i]);
//						if (i != (uc_mib_table[idx].size-1))
//							strcat(tmp, ",");
//#ifdef USE_11N_UDP_SERVER
//						}
//						else
//							sprintf(tmp, "%02x", buffer[i]);
//#endif
#else
						sprintf(tmp, "%02x", buffer[i]);
#endif
						strcat(value, tmp);
					}
					return 1;
					
				case UC_STRING_T:
					strcpy(value, buffer);
					return 1;
					
				default:
					return 0;
			}		
		}
	}

	return 0;
}

/*
 * Set MIB Value
 */
int uc_mib_set_by_name(char* name, char *value)
{
	int i;
	int id = -1;

	for (i=0; uc_mib_table[i].id; i++) {
		if ( !strcmp(uc_mib_table[i].name, name) )
		{
			id = uc_mib_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[512];
		struct in_addr ipAddr;
#ifdef CONFIG_IPV6
        struct in6_addr ip6Addr;
#endif
		unsigned char vChar;
		unsigned short vShort;
		unsigned int vUInt;
		int vInt;
	
		switch (uc_mib_table[i].type) {
			case UC_IA_T:
				ipAddr.s_addr = inet_addr(value);
				return uc_mib_set(id, (void *)&ipAddr);

#ifdef CONFIG_IPV6
			case UC_IA6_T:
				inet_pton(PF_INET6, value, &ip6Addr);
				return uc_mib_set(id, (void *)&ip6Addr);
#endif

			case UC_BYTE_T:
				sscanf(value,"%u",&vUInt);
				vChar = (unsigned char) vUInt;
				return uc_mib_set(id, (void *)&vChar);

			case UC_WORD_T:
				sscanf(value,"%u",&vUInt);
				vShort = (unsigned short) vUInt;
				return uc_mib_set(id, (void *)&vShort);
				
			case UC_DWORD_T:
				sscanf(value,"%u",&vUInt);
				return uc_mib_set(id, (void *)&vUInt);

			case UC_INTEGER_T:
				sscanf(value,"%d",&vInt);
				return uc_mib_set(id, (void *)&vInt);

			case UC_BYTE5_T:
				uc_string_to_hex(value, buffer, 10);
				return uc_mib_set(id, (void *)&buffer);

			case UC_BYTE6_T:
				uc_string_to_hex(value, buffer, 12);
				return uc_mib_set(id, (void *)&buffer);

			case UC_BYTE13_T:
				uc_string_to_hex(value, buffer, 26);
				return uc_mib_set(id, (void *)&buffer);

			case UC_STRING_T:
				return uc_mib_set(id, (void *)value);

			case UC_BYTE_ARRAY_T:
				uc_string_to_hex(value, buffer, uc_mib_table[i].size*2);
				return uc_mib_set(id, (void *)&buffer);

			default:
				return 0;
		}
	}

	return 0;
}

void uc_mib_dump(void)
{
	int i;
	unsigned char buffer[512];

	/* -------------------------------------------
	 *	mib table
	 * -------------------------------------------*/
	printf("\n---------------------------------------------------------\n");
	printf("MIB Table Information\n");
	printf("---------------------------------------------------------\n");
	for (i=0; uc_mib_table[i].id; i++) {
		memset(buffer, 0x00 , 512);
		if(uc_mib_get_by_name(uc_mib_table[i].name, buffer) != 0) {
			printf("%s=%s\n",uc_mib_table[i].name,buffer);
		}
	}
	printf("---------------------------------------------------------\n");
	printf("Total size: %d bytes\n", sizeof(UC_MIB_T));
}

void uc_mib_dump_info(void)
{
	int i;

	/* -------------------------------------------
	 *	mib table
	 * -------------------------------------------*/
	printf("\n---------------------------------------------------------\n");
	printf("MIB Table Information\n");
	printf("---------------------------------------------------------\n");
	printf("%10s%10s  name\n", "id", "size");
	for (i=0; uc_mib_table[i].id; i++) 
	{
		printf("%10d%10d  %s\n",uc_mib_table[i].id, uc_mib_table[i].size, uc_mib_table[i].name);
	}
	printf("---------------------------------------------------------\n");
	printf("Total size: %d bytes\n", sizeof(UC_MIB_T));
}

void uc_mib_init(void) 
{
	uc_mib_init_default(); 
}


