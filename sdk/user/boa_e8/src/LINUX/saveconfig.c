/*
 * Save system configuration to file
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
#define FMT_RAW		1
#define FMT_XML		2

FILE *fp;
char default_path[] = "/tmp/config.xml";
char buffer[4096];
char chainEntry[1024];

static void print_name_value(char *name, void *addr, TYPE_T type, int size)
{
	unsigned char value[512], tmp[16], *svalue = value;
	int i, array_size;

	switch (type) {
	case IA_T:
		if (((struct in_addr *)addr)->s_addr == INADDR_NONE)
			svalue[0] = '\0';
		else
			svalue = inet_ntoa(*(struct in_addr *)addr);
		break;
#ifdef CONFIG_IPV6
	case IA6_T:
		if (!memcmp
		    (((struct in6_addr *)addr)->s6_addr, in6addr_any.s6_addr,
		     16))
			svalue[0] = '\0';
		else
			svalue = (unsigned char *)inet_ntop(PF_INET6, addr, value, sizeof(value));
		break;
#endif
	case BYTE_T:
		sprintf(svalue, "%hhu", *(unsigned char *)addr);
		break;

	case WORD_T:
		sprintf(svalue, "%hu", *(unsigned short *)addr);
		break;

	case DWORD_T:
		sprintf(svalue, "%u", *(unsigned int *)addr);
		break;

	case INTEGER_T:
		sprintf(svalue, "%d", *(int *)addr);
		break;

	case BYTE5_T:
	case BYTE6_T:
	case BYTE13_T:
		svalue[0] = '\0';
		for (i = 0; i < size; i++) {
			sprintf(tmp, "%02x", ((unsigned char *)addr)[i]);
			strcat(svalue, tmp);
		}
		break;

	case BYTE_ARRAY_T:
		svalue[0] = '\0';
		for (i = 0; i < size; i++) {
			sprintf(tmp, "%02x", ((unsigned char *)addr)[i]);
			if (i != (size - 1))
				strcat(tmp, ",");
			strcat(svalue, tmp);
		}
		break;
	case INT_ARRAY_T:
		array_size = size / sizeof(int);

		svalue[0] = '\0';
		for (i = 0; i < array_size; i++)
		{
			sprintf(tmp, "%08x", ((unsigned int *)addr)[i]);

			if (i != (array_size - 1))
				strcat(tmp, ",");

			strcat(svalue, tmp);
		}
		break;
	case STRING_T:
		svalue = addr;
		break;

	default:
		printf("%s: Unknown data type %d!\n", name, type);
	}

	fprintf(fp,"<Value Name=\"%s\" Value=\"%s\"/>\n", name, svalue);
	DEBUGP("<Value Name=\"%s\" Value=\"%s\"/>\n", name, svalue);
}

void show_usage()
{
	fprintf(stderr,	"Usage: saveconfig [ -f filename ] [ -t raw/xml ]\n");
}

static int save_xml_file(char *loadfile)
{
	int i, k;
	mib_table_entry_T info;
	mib_chain_record_table_entry_T chainInfo;
	mib_chain_member_entry_T *rec_desc;
	
	fp=fopen(loadfile,"w");
	fprintf(fp,"%s\n", CONFIG_HEADER);
	DEBUGP("%s\n", CONFIG_HEADER);
	i=0;

	// MIB Table
	while (1) {
		if (!mib_info_index(i++, &info))
			return error;

		if (info.id == 0)
			break;
		// only for CURRENT_SETTING
		if (info.mib_type != CURRENT_SETTING)
			continue;
		//printf("get table entry %s\n", info.name);
		mib_get(info.id, (void *)buffer);
		print_name_value(info.name, buffer, info.type, info.size);
	}

	//MIB chain record
	i = 0;
	while (1) {
		unsigned int entryNum, index;
		void *value;

		if (!mib_chain_info_index(i++, &chainInfo))
			return error;

		if (chainInfo.id == 0)
			break;

		entryNum = mib_chain_total(chainInfo.id);
		if (mib_chain_desc_id(chainInfo.id, (void *)&buffer)==0) {
			printf("Empty MIB chain %s descriptor !\n", chainInfo.name);
			continue;
		}

		rec_desc = (mib_chain_member_entry_T *)&buffer[0];
		DEBUGP("chain entry %d # %d\n", chainInfo.id, entryNum);

		if (entryNum == 0) {
			fprintf(fp,"<chain chainName=\"%s\">\n", chainInfo.name);
			DEBUGP("<chain chainName=\"%s\">\n", chainInfo.name);
			fprintf(fp,"</chain>\n");
			DEBUGP("</chain>\n");
		}
		else {
			for(index=0;index<entryNum;index++){
				mib_chain_get(chainInfo.id, index, (void *)&chainEntry[0]);
				fprintf(fp,"<chain chainName=\"%s\">\n", chainInfo.name);
				DEBUGP("<chain chainName=\"%s\">\n", chainInfo.name);
				k = 0;
				while (rec_desc[k].name[0] != 0) {
					print_name_value(rec_desc[k].name, chainEntry+rec_desc[k].offset, rec_desc[k].type, rec_desc[k].size);
					k++;
				}
				fprintf(fp,"</chain>\n");
				DEBUGP("</chain>\n");
			}
		}
	}

	fprintf(fp,"%s\n", CONFIG_TRAILER);
	DEBUGP("%s\n", CONFIG_TRAILER);
	fclose(fp);
	return 0;
}

static int save_raw_file(char *loadfile)
{
	PARAM_HEADER_T header;
	unsigned int fileSize;
	unsigned char *buf;
	
	fp=fopen(loadfile,"w");
	if(mib_read_header(CURRENT_SETTING, &header) != 1) {
		fclose(fp);
		return error;
	}

	fileSize = sizeof(PARAM_HEADER_T) + header.len;
	buf = malloc(fileSize);
	if ( buf == NULL ) {
		fclose(fp);
		return error;
	}

	printf("fileSize=%d\n",fileSize);
	if(mib_read_to_raw(CURRENT_SETTING, buf, fileSize) != 1)
	{
		free(buf);
		fclose(fp);
		printf("ERROR: Flash read fail.");
		return error;
	}
	
	fwrite(buf, fileSize, 1, fp);
	free(buf);
	fclose(fp);
	
	return 0;
}

int main(int argc, char **argv)
{
	int k;
	int opt;
	char userfile[64];
	char *loadfile;
	int filefmt;
	int desc_check;
	int save_to_flash;

	desc_check = 0;
	save_to_flash = 0;
	loadfile = default_path;
	filefmt = FMT_XML;
	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "f:t:cs")) > 0) {
		switch (opt) {
			case 'f':
				strncpy(userfile, optarg, 64);
				userfile[63] = '\0';
				loadfile = userfile;
				break;
			case 't':
				if (!strcmp("raw", optarg))
					filefmt = FMT_RAW;
				else if (!strcmp("xml", optarg))
					filefmt = FMT_XML;
				else {
					show_usage();
					return error;
				}
				break;
			case 'c':
				desc_check = 1;
				break;
			case 's': // Save Setting to flatfsd(flash).
				save_to_flash = 1;
				break;
			default:
				show_usage();
				return error;
		}
	}

	// Check the consistency between chain record and chain descriptor
	if (desc_check) {
		k=mib_check_desc();
		if (k==1)
			printf("Check ok !\n");
		else
			printf("Check failed !\n");
		return 0;
	}

	if (filefmt == FMT_XML)
		save_xml_file(loadfile);
	else // FMT_RAW
		save_raw_file(loadfile);

	if (save_to_flash) {
#ifdef CONFIG_USER_RTK_RECOVER_SETTING	// Save Setting to flatfsd(flash)
		va_cmd("/bin/cp", 2, 1, CONFIG_XMLFILE, OLD_SETTING_FILE);
		//ccwei_flatfsd
#ifdef CONFIG_USER_FLATFSD_XXX
		va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
#endif
	}

	return 0;
}

