#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/atm.h>
#include <unistd.h>

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#include "mibtbl.h"
#include "utility.h"

void show_help(const char *prog)
{
	printf("%s: MIB command line utility\n", prog);
	printf("	%s get <name>\n", prog);
	printf("	%s set <name> <value>\n", prog);
	printf("	%s show [cs|rs|hs]\n", prog);
	printf("	%s commit\n", prog);
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

int get_by_index(int index, void *value)
{
	int j=0;
	unsigned char buffer[512], tmp[16];

	if (index != -1) {
		memset(buffer, 0x00, sizeof(buffer));

		/* fails */
		if (mib_get(mib_table[index].id, buffer) == 0)
			return 0;

		switch (mib_table[index].type) {
		case IA_T:
			strcpy(value, inet_ntoa(*(struct in_addr *)buffer));
			return 1;

#ifdef CONFIG_IPV6
		case IA6_T:
			inet_ntop(PF_INET6, buffer, value, 48);
			return 1;
#endif

		case BYTE_T:
			sprintf(value, "%hhu", *(unsigned char *)buffer);
			return 1;

		case WORD_T:
			sprintf(value, "%hu", *(unsigned short *)buffer);
			return 1;

		case DWORD_T:
			sprintf(value, "%u", *(unsigned int *)buffer);
			return 1;

		case INTEGER_T:
			sprintf(value, "%d", *(int *)buffer);
			return 1;

		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			((unsigned char *)value)[0] = '\0';
			for (j = 0; j < mib_table[index].size; j++) {
				sprintf(tmp, "%02x", buffer[j]);
				strcat(value, tmp);
			}
			return 1;

		case BYTE_ARRAY_T:
			((unsigned char *)value)[0] = '\0';
			for (j = 0; j < mib_table[index].size; j++) {
				sprintf(tmp, "%02x", buffer[j]);
				if (j != (mib_table[index].size - 1))
					strcat(tmp, ",");
				strcat(value, tmp);
			}
			return 1;

		case STRING_T:
			strcpy(value, buffer);
			return 1;

		default:
			return 0;
		}
	}

	return 0;
}

/*
 * Get MIB Value
 */
int get_by_name(char *name, void *value)	// get default value
{
	int i;

	for (i = 0; mib_table[i].id; i++) {
		if (!strcmp(mib_table[i].name, name)) {
			break;
		}
	}

	return get_by_index(i, value);
}

int do_get(char *name)
{
	char output[1024] = {0};
	int ret = 0;

	if (get_by_name(name, output))
	{
		printf("%s\n", output);
		return 0;
	}
	else
		fprintf(stderr, "Cannot get value of %s.\n", name);

	return -1;
}

int do_show(char *type)
{
	int i = 0;
	char value[1024] = {0};
	int flag = 0;

	if(type == NULL)
		flag = CURRENT_SETTING | RUNNING_SETTING;
	else if (strcmp(type, "cs") == 0)
		flag = CURRENT_SETTING;
	else if (strcmp(type, "rs") == 0)
		flag = RUNNING_SETTING;
	else if (strcmp(type, "hs") == 0)
		flag = HW_SETTING;
	else
	{
		fprintf(stderr, "Cannot show unknown type: %s.", type);
		return -1;
	}

	for (i=0; mib_table[i].id; i++)
	{
		if(flag & mib_table[i].mib_type)
		{
			get_by_index(i, value);
			printf("%s=%s\n", mib_table[i].name, value);
		}
	}

	return 0;
}

/*
 * Set MIB Value
 */
CONFIG_DATA_T set_by_name(char *name, char *value)	// get default value
{
	int i, id = -1;
	unsigned char buffer[512];
	struct in_addr ipAddr;
#ifdef CONFIG_IPV6
	struct in6_addr ip6Addr;
#endif
	unsigned char vUChar;
	unsigned short vUShort;
	unsigned int vUInt;
	int vInt;
	void *p;
	CONFIG_DATA_T cdtype = UNKNOWN_SETTING;

	for (i = 0; mib_table[i].id; i++) {
		if (!strcmp(mib_table[i].name, name)) {
			id = mib_table[i].id;
			cdtype = mib_table[i].mib_type;
			break;
		}
	}

	if (id != -1) {
		switch (mib_table[i].type) {
		case IA_T:
			ipAddr.s_addr = inet_addr(value);
			p = &ipAddr;
			break;

#ifdef CONFIG_IPV6
		case IA6_T:
			inet_pton(PF_INET6, value, &ip6Addr);
			p = &ip6Addr;
			break;
#endif

		case BYTE_T:
			sscanf(value, "%hhu", &vUChar);
			p = &vUChar;
			break;

		case WORD_T:
			sscanf(value, "%hu", &vUShort);
			p = &vUShort;
			break;

		case DWORD_T:
			sscanf(value, "%u", &vUInt);
			p = &vUInt;
			break;

		case INTEGER_T:
			sscanf(value, "%d", &vInt);
			p = &vInt;
			break;

		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
		case BYTE_ARRAY_T:
			string_to_hex(value, buffer, mib_table[i].size * 2);
			p = buffer;
			break;

		case STRING_T:
			p = value;
			break;

		default:
			return 0;
		}
	}

	//return _mib_set(id, p);
	if (mib_set(id, p))
		return cdtype;
	else
		return 0;
}

int do_set(char *mib, char *mibvalue)
{
	CONFIG_DATA_T tmpType=UNKNOWN_SETTING;
	CONFIG_DATA_T cdType=UNKNOWN_SETTING;

	if((tmpType=set_by_name(mib, mibvalue)) != 0)
	{
		cdType |= tmpType;
		printf("set %s=%s\n",mib,mibvalue);
		return 0;
	}
	else
		fprintf(stderr, "set %s=%s fail!\n",mib,mibvalue);

	return -1;
}

int do_commit()
{
	int ret = 0;

	if(!mib_update(CURRENT_SETTING, CONFIG_MIB_ALL))
	{
		fprintf(stderr, "Update Current Setting failed!\n");
		ret = -1;
	}
	if(!mib_update(HW_SETTING, CONFIG_MIB_ALL))
	{
		fprintf(stderr, "Update Hardware Setting failed!\n");
		ret = -1;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		show_help(argv[0]);
		return -1;
	}
	
	if( strcmp(argv[1], "get") == 0)
		return do_get(argv[2]);
	else if( strcmp(argv[1], "set") == 0)
	{
		if(argc < 4)
		{
			fprintf(stderr, "Too few argument for set command.\n");
			show_help(argv[0]);
		}
		return do_set(argv[2], argv[3]);
	}
	else if(strcmp(argv[1], "show") == 0)
	{
		if(argc > 2)
			return do_show(argv[2]);//show rs or cs
		else
			return do_show(NULL);	//show all
	}

	else if(strcmp(argv[1], "commit") == 0)
		return do_commit();
	else
	{
		fprintf(stderr, "Unknown command: %s\n", argv[1]);
		show_help(argv[0]);
	}

	return -1;
}
