/*
 * Load configuration file and update to the system
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mibtbl.h"
#include "utility.h"

#if 0
#define DEBUGP	printf
#else
#define DEBUGP(format, args...)
#endif

#define error -1
#define chain_record_number	(MIB_CHAIN_TBL_END-CHAIN_ENTRY_TBL_ID+1)

FILE *fp;
char default_path[] = "/tmp/config.xml";
char chain_updated[chain_record_number];
char LINE[512];
//char buffer[4096];
char buffer[32*1024];

//char chainEntry[1024];
char chainEntry[18*1024];

mib_table_entry_T *info;
int info_total;

#define MAX_OBJ	8

struct chain_obj_s {
	char obj_name[16];
	int obj_per_size;
	int obj_entry;
	int cur_num;
};

// Use isxdigit() instead.
#if 0 
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}
#endif

static int get_line(char *s, int size)
{
	char *pstr;
	
	while (1) {
		if (!fgets(s,size,fp))
			return error;
		pstr = trim_white_space(s);
		if (strlen(pstr))
			break;
	}

	//printf("get line: %s\n", s);
	return 0;
}

/*
 *	Remove comma and blank in the string
 */
static int to_hex_string(char *string)
{
	char tostr[256];
	int i, k;

	for (i=0, k=0; i<256, string[i]!='\0'; i++) {
		if (string[i]!=',' && string[i]!=' ')
			tostr[k++] = string[i];
	}
	tostr[k] = '\0';
	strcpy(string, tostr);
	return 1;
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !isxdigit(tmpBuf[0]) || !isxdigit(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static int string_to_short(char *string, unsigned short *key, int len)
{
	char tmpBuf[9];
	int idx, ii=0, j;

	for( idx = 0; idx < len; idx += 4 )
	{
		for( j = 0; j < 4; j++ )
		{
			tmpBuf[j] = string[idx+j];
			if (!isxdigit(tmpBuf[j]))
				return 0;
		}

		tmpBuf[4] = 0;

		key[ii++] = (unsigned short) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static int string_to_integer(char *string, unsigned int *key, int len)
{
	char tmpBuf[9];
	int idx, ii=0, j;

	for( idx = 0; idx < len; idx += 8 )
	{
		for( j = 0; j < 8; j++ )
		{
			tmpBuf[j] = string[idx+j];
			if (!isxdigit(tmpBuf[j]))
				return 0;
		}

		tmpBuf[8] = 0;

		key[ii++] = (unsigned int) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static int put_value(char *entry, TYPE_T type, char *vstr, int size)
{
	struct in_addr *ipAddr;
#ifdef CONFIG_IPV6
	struct in6_addr *ip6Addr;
#endif
	unsigned char *vChar;
	unsigned short *vShort;
	unsigned int *vUInt;
	int *vInt;
	int ret;
	
	ret = 0;
	switch (type) {
		case IA_T:
			if (!strlen(vstr)) {
				vstr="0.0.0.0";
			}
			ipAddr = (struct in_addr *)entry;
			if (inet_pton(AF_INET, vstr, ipAddr)==0)
				ret = error;
			break;
#ifdef CONFIG_IPV6
		case IA6_T:
			if (!strlen(vstr)) {
				vstr="::";
			}
			ip6Addr = (struct in6_addr *)entry;
			if (inet_pton(AF_INET6, vstr, ip6Addr)==0)
				ret = error;
			break;
#endif
		case BYTE_T:
			vChar = (unsigned char *)entry;
			*vChar = (unsigned char)strtol(vstr, 0, 0);
			break;
		case WORD_T:
			vShort = (unsigned short *)entry;
			*vShort = (unsigned short)strtol(vstr, 0, 0);
			break;
		case DWORD_T:
			vUInt = (unsigned int *)entry;
			*vUInt = (unsigned int)strtoul(vstr, 0, 0);
			break;
		case INTEGER_T:
			vInt = (int *)entry;
			*vInt = (int)strtol(vstr, 0, 0);
			break;
		case BYTE_ARRAY_T:
			to_hex_string(vstr); // remove comma and blank
		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			string_to_hex(vstr, entry, size * 2);
			break;
		case STRING_T:
			strcpy(entry, vstr);
			break;
		case WORD_ARRAY_T:
			to_hex_string(vstr); // remove comma and blank
			string_to_short(vstr, (unsigned short *)entry, size*2);
			break;
		case INT_ARRAY_T:
			to_hex_string(vstr); // remove comma and blank
			string_to_integer(vstr, (unsigned int *)entry, size*2);
			break;
		default:
			printf("Unknown data type !\n");
	}

	return ret;
}

static int object_per_rec_size(mib_chain_member_entry_T *desc)
{
	int perSize;
	int k;
	mib_chain_member_entry_T *obj_desc;
	
	obj_desc=(mib_chain_member_entry_T *)malloc(32*1024);
	if (!obj_desc) {
		printf("object_per_rec_size: malloc failed\n");
		return 0;
	}
	mib_desc_get_addr(desc->record_desc, (mib_chain_member_entry_T *)obj_desc);	
	
	if (!obj_desc)
		return 0;
	k = 0; perSize=0;
	while (obj_desc[k].name[0] != 0)
		perSize+=obj_desc[k++].size;
	free(obj_desc);
	return perSize;
}

static int object_entry(mib_chain_member_entry_T *desc)
{
	int entryNum, perSize;
	
	perSize = object_per_rec_size(desc);
	if (!perSize)
		return 0;
	entryNum = desc->size/perSize;
	return entryNum;
}

/* Get object info.
 * obj_info: object info list
 * desc: descriptor of this object
 * index: return the index of obj_info for this object.
 * Return:
 *	0: successful
 * 	-1: error
 */
static int get_object_info(struct chain_obj_s *obj_info, mib_chain_member_entry_T *desc, int *index)
{
	int found, k;
	
	// find object info.
	found=0;
	for (k=0; k<MAX_OBJ; k++) {
		//printf("name=%s ptoken=%s\n", obj_info[k].obj_name, desc->name);
		if (obj_info[k].obj_name[0] == '\0')
			break;
		if (!strncmp(obj_info[k].obj_name, desc->name, 15)) {
			found=1;
			break;
		}
	}
	//printf("k=%d\n", k);
	if (k == MAX_OBJ) {
		printf("%s: data overflow !\n", __FUNCTION__);
		return error;
	}
	if (!found) { // create one
		//printf("not found, create one\n");
		strncpy(obj_info[k].obj_name, desc->name, 15);
		obj_info[k].obj_per_size = object_per_rec_size(desc);
		obj_info[k].obj_entry = object_entry(desc);
		obj_info[k].cur_num=0;
	}
	else {
		//printf("found\n");
	}
	*index = k;
	return 0;
}

/*
 * Return:
 *	-1 : error
 *	 0 : successful
 *	 1 : empty chain(object)
 */
static int put_value_object(char *entry, mib_chain_member_entry_T *root_desc, mib_chain_member_entry_T *this_desc)
{
	int i,k;
	int empty_chain;
	char *pstr, *ptoken;
	const char empty_str[]="";
	struct chain_obj_s object_info[MAX_OBJ];
	int ret, index;
	mib_chain_member_entry_T *obj_desc;	
	char *buf;
	char *pname, *pvalue;
	
	memset((void *)&object_info[0], 0, sizeof(object_info));
	
	empty_chain = 1;
	while(!feof(fp)) {
		get_line(LINE, sizeof(LINE));
		// remove leading space
		pstr = trim_white_space(LINE);
		if (!strncmp(pstr, "</chain", 7)) {
			break; // end of chain object
		}
		// check OBJECT_T
		if (!strncmp(pstr, "<chain", 6)) {
			// get Object name
			ptoken = strtok(pstr, "\"");
			ptoken = strtok(0, "\"");
			DEBUGP("obj_name=%s\n", ptoken);
			if (this_desc == 0)
				continue;
			i = 0;
			while (this_desc[i].name[0] != 0) {
				if (!strcmp(this_desc[i].name, ptoken))
					break;
				i++;
			}
			if (this_desc[i].name[0] == 0) {
				printf("%s: Chain Object %s member %s descriptor not found !\n", __FUNCTION__, root_desc->name, ptoken);
				return error;
			}
			// get object info.
			ret=get_object_info(&object_info[0], &this_desc[i], &index);
			//printf("index=%d\n", index);
			//printf("cur_num=%d obj_entry=%d per_size=%d\n", object_info[index].cur_num, object_info[index].obj_entry, object_info[index].obj_per_size);
			//printf("offset=%d\n", object_info[index].cur_num*object_info[index].obj_per_size);

			obj_desc=(mib_chain_member_entry_T *)malloc(32*1024);
			if (!obj_desc) {
				printf("%s: malloc failed\n", __FUNCTION__);
				return -1;
			}
			mib_desc_get_addr(this_desc[i].record_desc, (mib_chain_member_entry_T *)obj_desc);
			ret = put_value_object(entry+this_desc[i].offset+object_info[index].cur_num*object_info[index].obj_per_size, &this_desc[i], obj_desc);
			if (ret==0)
				object_info[index].cur_num++;
			free(obj_desc);
			if (ret==error) {
				return error;
			}
		}
		else {
			// get name
			ptoken = strtok(pstr, "\"");
			ptoken = strtok(0, "\"");
			pname = ptoken;
			DEBUGP("name=%s\n", ptoken);
			if (this_desc == 0)
				continue;
			i = 0;
			while (this_desc[i].name[0] != 0) {
				if (!strcmp(this_desc[i].name, ptoken))
					break;
				i++;
			}
			if (this_desc[i].name[0] == 0) {
				printf("Chain %s member %s not found !\n", root_desc->name, ptoken);
				return error;
			}
                	
			// get value
			ptoken = strtok(0, "\"");
			ptoken = strtok(0, "\"");
			if (strtok(0, "\"")==NULL)
				ptoken = (char *)empty_str;
			pvalue = ptoken;
			DEBUGP("value=%s\n", ptoken);
			// put value
			ret = put_value(entry+this_desc[i].offset, this_desc[i].type, ptoken, this_desc[i].size);
			if (ret==error) {
				printf("%s: Invalid chain member ! (name=\"%s\", value=\"%s\")\n", __FUNCTION__, pname, pvalue);
				return error;
			}
		}
		empty_chain = 0;
	}
	return empty_chain;
}

static int table_setting(char *line)
{
	int i;
	char *ptoken;
	char *pname, *pvalue;
	const char empty_str[]="";
	mib_table_entry_T info_entry;
	unsigned char buffer[128];
	struct in_addr ipAddr;
#ifdef CONFIG_IPV6
	struct in6_addr ip6Addr;
#endif
	unsigned char vChar;
	unsigned short vShort;
	unsigned int vUInt;
	int vInt;
	void *p;

	// get name
	ptoken = strtok(line, "\"");
	ptoken = strtok(0, "\"");
	pname = ptoken;
	DEBUGP("table name=%s\n", ptoken);

	for(i=0; i<info_total; i++){
		if(!strcmp(((mib_table_entry_T*)(info+i))->name, ptoken)){
			memcpy(&info_entry,(mib_table_entry_T*)(info+i),sizeof(mib_table_entry_T));
			break;
		}
	}

	if(i>=info_total) {
		printf("%s: Invalid table entry name: %s\n", __FUNCTION__, ptoken);
		return error;
	}

	// get value
	ptoken = strtok(0, "\"");
	ptoken = strtok(0, "\"");
	if (strtok(0, "\"")==NULL)
		ptoken = (char *)empty_str;
	pvalue = ptoken;
	DEBUGP("table value=%s\n", ptoken);

	switch (info_entry.type) {
	case IA_T:
		if (ptoken==empty_str) {
			ptoken="0.0.0.0";
		}
		if (inet_pton(AF_INET, ptoken, &ipAddr)==0) {
			printf("%s: Invalid table value ! (name=\"%s\", value=\"%s\")\n", __FUNCTION__, pname, pvalue);
			return error;
		}
		p = &ipAddr;
		break;
	#ifdef CONFIG_IPV6
	case IA6_T:
		if (ptoken==empty_str) {
			ptoken="::";
		}
		if (inet_pton(AF_INET6, ptoken, &ip6Addr)==0) {
			printf("%s: Invalid table value ! (name=\"%s\", value=\"%s\")\n", __FUNCTION__, pname, pvalue);
			return error;
		}
		p = &ip6Addr;
		break;
	#endif
	case BYTE_T:
		vChar = (unsigned char)strtol(ptoken, 0, 0);
		p = &vChar;
		break;
	case WORD_T:
		vShort = (unsigned short)strtol(ptoken, 0, 0);
		p = &vShort;
		break;
	case DWORD_T:
		vUInt = (unsigned int)strtol(ptoken, 0, 0);
		p = &vUInt;
		break;
	case INTEGER_T:
		vInt = (int)strtol(ptoken, 0, 0);
		p = &vInt;
		break;
	case BYTE_ARRAY_T:
		to_hex_string(ptoken); // remove comma and blank
	case BYTE5_T:
	case BYTE6_T:
	case BYTE13_T:
		string_to_hex(ptoken, buffer, info_entry.size * 2);
		p = buffer;
		break;
	case STRING_T:
		p = ptoken;
		break;
	case INT_ARRAY_T:
		to_hex_string(ptoken); // remove comma and blank
		string_to_integer(ptoken,(unsigned int *) buffer, info_entry.size*2);
		p = buffer;
		break;
	}
	if (!mib_set(info_entry.id, p)) {
		printf("Set MIB[%s] error!\n", info_entry.name);
		return error;
	}

	return 0;
}

static int chain_setting(char *line)
{
	int empty_chain;
	char *ptoken;
	mib_chain_record_table_entry_T chainInfo;
	mib_chain_member_entry_T *rec_desc;
	mib_chain_member_entry_T root_desc;

	memset((void *)&chainEntry[0], 0, sizeof(chainEntry));
	// get chain name
	ptoken = strtok(line, "\"");
	ptoken = strtok(0, "\"");
	DEBUGP("Chain name=%s\n", ptoken);
	// get chain info
	if (!mib_chain_info_name(ptoken, &chainInfo)) {
		printf("Invalid chain name: %s\n", ptoken);
		return error;
	}
	// get chain descriptor
	if (mib_chain_desc_id(chainInfo.id, (void *)&buffer)==0) {
		printf("Empty MIB chain %s descriptor !\n", chainInfo.name);
		rec_desc = 0;
	}
	else
		rec_desc = (mib_chain_member_entry_T *)&buffer[0];
	//clear orginal record
	if(chain_updated[chainInfo.id-CHAIN_ENTRY_TBL_ID]==0){
		mib_chain_clear(chainInfo.id);//clear chain record
		chain_updated[chainInfo.id-CHAIN_ENTRY_TBL_ID]=1;
	}

	strncpy(root_desc.name, chainInfo.name, 31);
	empty_chain=put_value_object(&chainEntry[0], &root_desc, rec_desc);
	if (empty_chain==1) {
				DEBUGP("Empty Chain.\n");
				mib_chain_clear(chainInfo.id);
	}
	else if (empty_chain==0) {
		mib_chain_add(chainInfo.id, (void *)chainEntry);
	}
	return empty_chain;
}

static int update_setting(char *line)
{
	int i, k;
	char str[32];
	int ret=0;

	// remove leading space
	i = 0; k = 0;
	while (line[i++]==' ')
		k++;
	sscanf(line, "%s", str);
	//printf("str=%s\n", str);
	if (!strcmp(str, "<Value"))
		table_setting(&line[k]);
	else if (!strcmp(str, "<chain"))
		ret=chain_setting(&line[k]);
	else
		printf("Unknown statement: %s\n", line);

	return ret;
}

#if 0
/*
 *	Check the consistency between chain record definition and its record descriptor.
 *	0 : Ok
 *	-1: fail
 */
int checkDesciptors()
{
	int idx, k, total;
	mib_chain_record_table_entry_T chain_info;
	mib_chain_member_entry_T *mbr_desc;

	// get chain record size
	idx = 0;
	while (mib_chain_info_index(idx, &chain_info)) {
		if (chain_info.id == 0)
			break;
		idx++;
		//printf("Checking Chain %s ...", chain_info.name);
		mbr_desc = chain_info.record_desc;
		if (mbr_desc == 0) {
			printf("Error: Null MIB-chain(%s) record descriptor !\n", chain_info.name);
			return error;
		}
		if (mib_chain_desc_id(chain_info.id, (void *)&buffer)==0) {
			printf("Error: Empty MIB-chain(%s) record descriptor !\n", chain_info.name);
			return error;
		}
		else
			mbr_desc = (mib_chain_member_entry_T *)&buffer[0];
		// compute the chain record size by member descriptor
		k = 0; total = 0;
		while (mbr_desc[k].size) {
			total += mbr_desc[k].size;
			k++;
		}
		if (chain_info.per_record_size != total) {
			printf("Error: Inconsistent MIB-chain(%s) record descriptor !\n", chain_info.name);
			printf("per_record_size=%d; total=%d\n", chain_info.per_record_size, total);
			return error;
		}
		//printf("Total %d ok.\n", total);
	}
	printf("Chain descriptors checking (total=%d) ok !\n", idx);
	return 0;
}
#endif

void show_usage()
{
	fprintf(stderr,	"Usage: loadconfig [ -f filename ]\n");
}

int main(int argc, char **argv)
{
	int i;
	int opt;
	int check;
	char userfile[64];
	char *loadfile;
	char *pstr;

	check = 0;
	loadfile = default_path;
	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "cf:")) > 0) {
		switch (opt) {
			case 'c':	// check chain member descriptor
				check = 1;
				break;
			case 'f':
				strncpy(userfile, optarg, 64);
				userfile[63] = '\0';
				loadfile = userfile;
				break;
			default:
				show_usage();
				return error;
		}
	}

	if (check) {
		i=mib_check_desc();
		if (i==1)
			printf("Check ok !\n");
		else
			printf("Check failed !\n");
		return 0;
		//return checkDesciptors();
	}
	/* initialize chain update flags */
	for(i=0;i<chain_record_number;i++){
		chain_updated[i]=0;
	}

	printf("Get user specific configuration file......\n\n");
	if (!(fp = fopen(loadfile, "r"))) {
		printf("User configuration file not exist: %s\n", loadfile);
		return error;
	}

	get_line(LINE, sizeof(LINE));
	pstr = trim_white_space(LINE);
	if(strcmp(pstr, CONFIG_HEADER)) {
		printf("Invalid config file!\n");
		fclose(fp);
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
		set_ctcom_alarm(CTCOM_ALARM_CONF_INVALID);
#endif
		return error;
	}

	info_total = mib_info_total();
	info=(mib_table_entry_T *)malloc(sizeof(mib_table_entry_T)*info_total);

	for(i=0;i<info_total;i++){
		if(!mib_info_index(i,info+i))
			break;
	}

	if(i<info_total){
		free(info);
		printf("get mib info total entry error!\n");
		return error;
	}

	while(!feof(fp)) {
		get_line(LINE, sizeof(LINE));//get one line from the file
		pstr = trim_white_space(LINE);
		if( !strcmp(pstr, CONFIG_TRAILER))
			break; // end of configuration

		if(update_setting(pstr) < 0) {
			fclose(fp);
			printf("update setting fail!\n");
			free(info);
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
			set_ctcom_alarm(CTCOM_ALARM_CONF_INVALID);
#endif
			return error;
		}
	}

	free(info);
	fclose(fp);
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
	clear_ctcom_alarm(CTCOM_ALARM_CONF_INVALID);
#endif
	printf("Restore settings from config file successful! \n");
	return 0;
}
