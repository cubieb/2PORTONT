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

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>

static int sendMsg2MidProcess(struct mwMsg * pMsg)
{
	int spid;
	FILE * spidfile;
	int msgid;
	
	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		fprintf(stdout,"LoadxmlCfg: get cwmp msgqueue error in %s!\n",__FUNCTION__);
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(CWMP_MIDPROC_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		fprintf(stdout,"LoadxmlCfg: midprocess pidfile not exists in %s\n",__FUNCTION__);
		return -1;
	}

	pMsg->msg_type = spid;
	pMsg->msg_datatype = MSG_MIDWARE;
	if(msgsnd(msgid, (void *)pMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"LoadxmlCfg: send message to midprocess error in %s!\n",__FUNCTION__);
		return -1;
	}
	
	return 0;
}

void sendSaveRebootMsg2MidProcess()
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;
	
	*(sendBuf) = OP_SaveReboot;
	sendMsg2MidProcess(&sendMsg);
}
#endif	//end fo CONFIG_MIDDLEWARE


#define error -1
#define chain_record_number	(MIB_CHAIN_TBL_END-CHAIN_ENTRY_TBL_ID+1)

FILE *fp;
char default_path[] = "/tmp/config.xml";
char chain_updated[chain_record_number];
char LINE[512];
char buffer[4096];
char chainEntry[1024];
char header_str1[64], header_str2[64];
char trailer_str1[64], trailer_str2[64];
mib_table_entry_T *info;
int info_total;

static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int get_line(char *s, int size)
{
	while (1) {
		if (!fgets(s,size,fp))
			return error;
		if (s[0] != '\n' && s[0] != '\r' && s[0] != '#')
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

static int string_to_integer(char *string, unsigned int *key, int len)
{
	char tmpBuf[9];
	int idx, ii=0, j;

	for( idx = 0; idx < len; idx += 8 )
	{
		for( j = 0; j < 8; j++ )
		{
			tmpBuf[j] = string[idx+j];
			if (!_is_hex(tmpBuf[j]))
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

	switch (type) {
		case IA_T:
			ipAddr = (struct in_addr *)entry;
			ipAddr->s_addr = inet_addr(vstr);
			break;
#ifdef CONFIG_IPV6
		case IA6_T:
			ip6Addr = (struct in6_addr *)entry;
			inet_pton(PF_INET6, vstr, ip6Addr);
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
			*vUInt = (unsigned int)strtol(vstr, 0, 0);
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
		case INT_ARRAY_T:
			to_hex_string(vstr); // remove comma and blank
			string_to_integer(vstr, (unsigned int *)entry, size*2);
			break;
		default:
			printf("Unknown data type !\n");
	}

	return 0;
}

static int table_setting(char *line)
{
	int i;
	char *ptoken;
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
	DEBUGP("table name=%s\n", ptoken);

	for(i=0; i<info_total; i++){
		if(!strcmp(((mib_table_entry_T*)(info+i))->name, ptoken)){
			memcpy(&info_entry,(mib_table_entry_T*)(info+i),sizeof(mib_table_entry_T));
			break;
		}
	}

	if(i>=info_total) {
		printf("Invalid table entry name: %s\n", ptoken);
		return error;
	}

	// get value
	ptoken = strtok(0, "\"");
	ptoken = strtok(0, "\"");
	if (strtok(0, "\"")==NULL)
		ptoken = (char *)empty_str;
	DEBUGP("table value=%s\n", ptoken);

	switch (info_entry.type) {
	case IA_T:
		ipAddr.s_addr = inet_addr(ptoken);
		p = &ipAddr;
		break;
	#ifdef CONFIG_IPV6
	case IA6_T:
		inet_pton(PF_INET6, ptoken, &ip6Addr);
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
	int i, k;
	int empty_chain;
	char *ptoken;
	const char empty_str[]="";
	mib_chain_record_table_entry_T chainInfo;
	mib_chain_member_entry_T *rec_desc;

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

	empty_chain = 1;
	while(!feof(fp)) {
		get_line(LINE, sizeof(LINE));
		// remove leading space
		i = 0; k = 0;
		while (LINE[i++]==' ')
			k++;
		if (!strncmp(&LINE[k], "</chain", 7)) {
			if (empty_chain) {
				DEBUGP("Empty Chain.\n");
				mib_chain_clear(chainInfo.id);
				return 0;
			}
			break; // end of chain
		}
		// get name
		ptoken = strtok(&LINE[k], "\"");
		ptoken = strtok(0, "\"");
		DEBUGP("name=%s\n", ptoken);
		if (rec_desc == 0)
			continue;
		i = 0;
		while (rec_desc[i].name[0] != 0) {
			if (!strcmp(rec_desc[i].name, ptoken))
				break;
			i++;
		}
		if (rec_desc[i].name[0] == 0) {
			printf("Chain %s member %s not found !\n", chainInfo.name, ptoken);
			return error;
		}

		// get value
		ptoken = strtok(0, "\"");
		ptoken = strtok(0, "\"");
		if (strtok(0, "\"")==NULL)
			ptoken = (char *)empty_str;
		DEBUGP("value=%s\n", ptoken);
		empty_chain = 0;
		// put value
		put_value(&chainEntry[rec_desc[i].offset], rec_desc[i].type, ptoken, rec_desc[i].size);
	}

	if (rec_desc != 0)
		mib_chain_add(chainInfo.id, (void *)chainEntry);
	return 0;
}

static int update_setting(char *line)
{
	int i, k;
	char str[32];

	// remove leading space
	i = 0; k = 0;
	while (line[i++]==' ')
		k++;
	sscanf(line, "%s", str);
	//printf("str=%s\n", str);
	if (!strcmp(str, "<Value"))
		table_setting(&line[k]);
	else if (!strcmp(str, "<chain"))
		chain_setting(&line[k]);
	else
		printf("Unknown statement: %s\n", line);

	return 0;
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

	// generate header string
	sprintf(header_str1, "%s\n", CONFIG_HEADER);
	sprintf(header_str2, "%s\r\n", CONFIG_HEADER);
	// generate trailer string
	sprintf(trailer_str1, "%s\n", CONFIG_TRAILER);
	sprintf(trailer_str2, "%s\r\n", CONFIG_TRAILER);

	get_line(LINE, sizeof(LINE));
	if(strcmp(LINE, header_str1) && strcmp(LINE, header_str2)) {
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
		if( (!strcmp(LINE, trailer_str1)) || !strcmp(LINE, trailer_str2))
			break; // end of configuration

		if(update_setting(LINE) < 0) {
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
#ifdef CONFIG_MIDDLEWARE
	printf("send saveReboot msg to midprocess\n");
	sendSaveRebootMsg2MidProcess();
#endif

	printf("Restore settings from config file successful! \n");
	return 0;
}
