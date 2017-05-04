
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* for opendir, readdir, kill */
#include <dirent.h>
#include <signal.h>

#include "mibtbl.h"

int mib_tbl_get_by_name(char* name, char* arg, void *value);
int mib_get_by_name(char* name, void *value);
int mib_set_by_name(char* name, char *value);
void mib_get_all(void);
#ifdef CAN_RW_FILE
int flash_read_to_file(CONFIG_DATA_T data_type, char * filename);
int flash_write_from_file(CONFIG_DATA_T data_type, char * filename);
#endif // #ifdef CAN_RW_FILE
void unzip_web(char *file, char *dest_dir);

static int __inline__ _is_hex(char c)
{
	return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

static int __inline__ string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

/*
 * Show Flash File System Help
 */
static void showHelp(void)
{
	printf("Usage: flash cmd\n");
	printf("cmd:\n");
	printf("  get MIB-NAME \t\t\tget a specific mib from flash memory.\n");
	printf("  set MIB-NAME MIB-VALUE \tset a specific mib into flash memory.\n");
	printf("  all \t\t\t\tdump all flash parameters.\n");
#ifdef CAN_RW_FILE
	printf("  wds FILENAME \t\t\twrite flash parameters to default settings.\n");
	printf("  wcs FILENAME \t\t\twrite flash parameters to current settings.\n");
	printf("  whs FILENAME \t\t\twrite flash parameters to hardware settings.\n");
	printf("  rds FILENAME \t\t\tread flash parameters from default settings.\n");
	printf("  rcs FILENAME \t\t\tread flash parameters from current settings.\n");
	printf("  rhs FILENAME \t\t\tread flash parameters from hardware settings.\n");
#endif // #ifdef CAN_RW_FILE
	printf("  reset \t\t\treset current setting to default.\n");
	printf("  default \t\t\twrite program default value to default.\n");
	printf("  unzip web\t\t\tunzip web pages.\n\n");
}

int flash_main(int argc, char** argv)
{
	int action=0;
	int argNum=1;
	char mib[100]={0};
	char mibvalue[100]={0};
#ifdef CAN_RW_FILE
	CONFIG_DATA_T data_type;
#endif // #ifdef CAN_RW_FILE

	if ( argc > 1 )
	{
		if ( !strcmp(argv[argNum], "info") ) {
			//printf("CS offset = %x\n",CURRENT_SETTING_OFFSET);
			printf("CS offset = %x\n",g_cs_offset);
			printf("CS Backup offset = %x\n",g_cs_bak_offset);
			printf("DS offset = %x\n",DEFAULT_SETTING_OFFSET);
			printf("HS offset = %x\n",HW_SETTING_OFFSET);
			printf("WEB offset = %x\n",WEB_PAGE_OFFSET);
			// 20130315 W.H. Hung
			//printf("CODE offset = %x\n", CODE_IMAGE_OFFSET);
			printf("CODE offset = %x\n", g_rootfs_offset);
#ifdef CONFIG_DOUBLE_IMAGE
			printf("CODE Backup offset = %x\n", g_fs_bak_offset);
#endif

			return 0;
		}
		else if ( !strcmp(argv[argNum], "get") ) {
			action = 1;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "set") ) {
			action = 2;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "all") ) {
			action = 3;
		}
#ifdef CAN_RW_FILE
		else if ( !strcmp(argv[argNum], "rds") ) {
			action = 4;
			argNum++;
			data_type = DEFAULT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "rcs") ) {
			action = 5;
			argNum++;
			data_type = CURRENT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "rhs") ) {
			action = 6;
			argNum++;
			data_type = HW_SETTING;
		}
		else if ( !strcmp(argv[argNum], "wds") ) {
			action = 7;
			argNum++;
			data_type = DEFAULT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "wcs") ) {
			action = 8;
			argNum++;
			data_type = CURRENT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "whs") ) {
			action = 9;
			argNum++;
			data_type = HW_SETTING;
		}
#endif // #ifdef CAN_RW_FILE
		else if ( !strcmp(argv[argNum], "reset") ) {
			action = 10;
		}
		else if ( !strcmp(argv[argNum], "default") ) {
			action = 11;
		}
		else if ( !strcmp(argv[argNum], "unzip") ) {
			action = 12;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "check") ) {
			action = 13;
		}
		else if ( !strcmp(argv[argNum], "clear") ) {
			action = 14;
		}
		else if ( !strcmp(argv[argNum], "kill") ) {
			action = 15;
			argNum++;
		}
	}

	if(action==0)
	{
		showHelp();
		return 0;
	}

	if(action==11)
	{
		extern unsigned char cs_valid, ds_valid, hs_valid;

		__mib_init_mib_header();

		mib_init_mib_with_program_default(CURRENT_SETTING);
		cs_valid = 1;
		if ( !strcmp(argv[argNum+1], "ds") )
		{
			ds_valid = 1;
			mib_update(DEFAULT_SETTING);
		}

		if ( !strcmp(argv[argNum+1], "cs") )
		{
			cs_valid = 1;
			mib_update(CURRENT_SETTING);
		}

		mib_init_mib_with_program_default(HW_SETTING);
		if ( !strcmp(argv[argNum+1], "hs") )
		{
			hs_valid = 1;
			mib_update(HW_SETTING);
		}

		return 0;
	}

	if(action==12)
	{
		if(argNum < argc)
		{
			if ( !strcmp(argv[argNum], "web") )
			{
#ifdef __mips__
				unzip_web(NULL,"/var/web/");
#else
				if((argc - argNum) == 2)
					unzip_web(argv[argNum],argv[argNum+1]);
				else if((argc - argNum) == 1)
					unzip_web(argv[argNum],"/home/unzip_web/");
				else
					unzip_web("webpages.bin","/home/unzip_web/");
#endif
			}
		}
		return 0;
	}

	if(action==13)
	{
		int i;
		PARAM_HEADER_T header;
		CONFIG_DATA_T data_type;
		unsigned char* pFlashBank;
		unsigned int fileSize = 0;
		unsigned char* pFile = NULL;
		unsigned char* pContent = NULL;

		for(i=0;i<3;i++)
		{
			if(i==0)
			{
				data_type = CURRENT_SETTING;
				pFlashBank = "CS_BANK";
			}
			else if(i==1)
			{
				data_type = DEFAULT_SETTING;
				pFlashBank = "DS_BANK";
			}
			else if(i==2)
			{
				data_type = HW_SETTING;
				pFlashBank = "HS_BANK";
			}

			if(__mib_header_read(data_type, &header) != 1)
				goto flash_check_error;

			if(__mib_header_check(data_type, &header) != 1)
				goto flash_check_error;

			fileSize = header.len + sizeof(PARAM_HEADER_T);
			pFile = malloc(fileSize);
			if ( pFile == NULL )
				goto flash_check_error;

			if(__mib_file_read(data_type, pFile, fileSize) != 1)
				goto flash_check_error;

			pContent = &pFile[sizeof(PARAM_HEADER_T)];	// point to start of MIB data

			if(__mib_content_decod_check(data_type, &header, pContent) != 1)
				goto flash_check_error;

			free(pFile);
			printf("%s=1\n", pFlashBank);
			continue;
flash_check_error:
			if(pFile) free(pFile);
			printf("%s=0\n", pFlashBank);
		}

		return 0;
	}

	if(action==14)
	{
		unsigned char buf[] = "CLEAR";

		flash_write(buf, CURRENT_SETTING_OFFSET, sizeof(buf));
		flash_write(buf, DEFAULT_SETTING_OFFSET, sizeof(buf));
		flash_write(buf, HW_SETTING_OFFSET, sizeof(buf));

		return 0;
	}

	if(action==15)
	{
		DIR * d;
		struct dirent * de;
		char *ext;
		char psbuf[256];
		int i, h, l;
		char* appName;

		if (argc < argNum)
			return 0;

		appName = argv[argNum];

		d = opendir("/proc");
		if (!d) {
			return 0;
		}

		while (de = readdir(d)) {

			for(i=0;i<strlen(de->d_name);i++)
				if (!isdigit(de->d_name[i]))
					goto next;

			sprintf(psbuf, "/proc/%s/stat", de->d_name);

			h = open(psbuf, O_RDONLY);

			if (h==-1)
				continue;

			l = read(h, psbuf, 255);
			if (l<=0) {
				close(h);
				continue;
			}

			psbuf[l] = '\0';
			psbuf[255] = '\0';

			ext = strrchr(psbuf, ')');
			ext[0] = '\0';

			ext = strrchr(psbuf, '(')+1;

			if ( !strcmp(appName, ext) ) {
				int pid;

				sscanf(de->d_name, "%d", &pid);

				if (kill(pid, SIGTERM) != 0) {
					printf( "Could not kill pid '%d' \n", pid);
				} else {
					printf("%s is Killed\n",appName);
				}

				return 0;
			}

			next:
				;
		}

		return 0;
	}

	mib_init();

	if(action==1)
	{
		unsigned char buffer[512];
		while(argNum < argc)
		{
			memset(buffer, 0x00 , 512);
			sscanf(argv[argNum++], "%s", mib);

			if(mib_tbl_get_by_name(mib, argv[argNum], buffer) != 0)
			{
				argNum++;
				printf("%s\n",buffer);
			}
			else if(mib_get_by_name(mib, buffer) != 0)
			{
				printf("%s=%s\n",mib,buffer);
			}
		}
	}

	if(action==2)
	{
		while((argNum + 1) < argc)
		{
			sscanf(argv[argNum++], "%s", mib);
			sscanf(argv[argNum++], "%s", mibvalue);

			if(mib_set_by_name(mib, mibvalue) != 0)
				printf("set %s=%s\n",mib,mibvalue);
			else
				printf("set %s=%s fail!\n",mib,mibvalue);
		}

		if(mib_update(CURRENT_SETTING) == 0)
			printf("CS Flash error! \n");

		if(mib_update(HW_SETTING) == 0)
			printf("HS Flash error! \n");

	}

	if(action==3)
	{
		mib_get_all();
	}

#ifdef CAN_RW_FILE
	if((action>=4) && (action<=6))
	{
		if(argNum >= argc)
			return 0;

		sscanf(argv[argNum++], "%s", mibvalue);

		if(flash_read_to_file(data_type, mibvalue)==0)
		{
			printf("flash_read_to_file fail!\n");
		}
	}

	if((action>=7) && (action<=9))
	{
		if(argNum >= argc)
			return 0;

		sscanf(argv[argNum++], "%s", mibvalue);

		if(flash_write_from_file(data_type, mibvalue)==0)
		{
			printf("flash_write_from_file fail!\n");
		}
	}
#endif // #ifdef CAN_RW_FILE

	if(action==10)
	{
		if(mib_reset(CURRENT_SETTING) != 0)
		{
			printf("reset current settings\n");
		}
		else
		{
			printf("reset current settings fail!\n");
		}
	}

	return 0;
}


/*
 * Get MIB Table Value
 */
int mib_tbl_get_by_name(char* name, char* arg, void *value)
{
	int i;
	int id = -1;

	for (i=0; mib_chain_record_table[i].id; i++) {
		if ( !strcmp(mib_chain_record_table[i].name, name) )
		{
			id = mib_chain_record_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned int total = mib_chain_total(id);
		unsigned int recordNum;

		if ( !strcmp("NUM", arg) )
		{
			sprintf(value,"%s_NUM=%u",name , total);
			return 1;
		}

		sscanf(arg, "%u", &recordNum);

		if(recordNum < total)
		{
			if(id == MIB_ATM_VC_TBL)
			{
				MIB_CE_ATM_VC_Tp pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_VPI=%u\n%s_VCI=%u\n%s_QoS=%s\n%s_PCR=%u\n%s_SCR=%u\n%s_MBR=%u\n%s_CDVT=%u"
						,name , pEntry->vpi
						,name , pEntry->vci
						,name , (pEntry->qos == 0)?"ubr":(pEntry->qos == 1)?"cbr":(pEntry->qos == 2)?"rt-vbr":"nrt-vbr"
						,name , pEntry->pcr
						,name , pEntry->scr
						,name , pEntry->mbs
						,name , pEntry->cdvt);

					return 1;
				}
			}
		}
	}

	return 0;
}


/*
 * Get MIB Value
 */
int mib_get_by_name(char* name, void *value)	// get default value
{
	int i;
	int id = -1;

	for (i=0; mib_table[i].id; i++) {
		if ( !strcmp(mib_table[i].name, name) )
		{
			id = mib_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[128];
		unsigned int vUInt;
		int	vInt;
		memset(buffer, 0x00 , 128);
		if(mib_get(id, (void *)buffer)!=0)
		{
			switch (mib_table[i].type) {
				case IA_T:
					strcpy(value, inet_ntoa(*((struct in_addr *)buffer)));
					return 1;

				case BYTE_T:
					vUInt = (unsigned int) (*(unsigned char *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case WORD_T:
					vUInt = (unsigned int) (*(unsigned short *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case DWORD_T:
					vUInt = (unsigned int) (*(unsigned int *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case INTEGER_T:
					vInt = *((int *)buffer);
					sprintf(value,"%d",vInt);
					return 1;

				case BYTE6_T:
					sprintf(value, "%02x%02x%02x%02x%02x%02x",
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4], buffer[5]);
					return 1;

				case STRING_T:
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
 * Get all MIB Value
 */
void mib_get_all(void)
{
	int i;
	unsigned char buffer[128];

	for (i=0; mib_table[i].id; i++) {
		memset(buffer, 0x00 , 128);
		if(mib_get_by_name(mib_table[i].name, buffer) != 0)
			printf("%s=%s\n",mib_table[i].name,buffer);
	}
}

/*
 * Set MIB Value
 */
int mib_set_by_name(char* name, char *value)	// get default value
{
	int i;
	int id = -1;

	for (i=0; mib_table[i].id; i++) {
		if ( !strcmp(mib_table[i].name, name) )
		{
			id = mib_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[100];
		struct in_addr ipAddr;
		unsigned char vChar;
		unsigned short vShort;
		unsigned int vUInt;
		int vInt;

		switch (mib_table[i].type) {
			case IA_T:
				ipAddr.s_addr = inet_addr(value);
				return mib_set(id, (void *)&ipAddr);

			case BYTE_T:
				sscanf(value,"%u",&vUInt);
				vChar = (unsigned char) vUInt;
				return mib_set(id, (void *)&vChar);

			case WORD_T:
				sscanf(value,"%u",&vUInt);
				vShort = (unsigned short) vUInt;
				return mib_set(id, (void *)&vShort);

			case DWORD_T:
				sscanf(value,"%u",&vUInt);
				return mib_set(id, (void *)&vUInt);

			case INTEGER_T:
				sscanf(value,"%d",&vInt);
				return mib_set(id, (void *)&vInt);

			case BYTE6_T:
				string_to_hex(value, buffer, 12);
				return mib_set(id, (void *)&buffer);

			case STRING_T:
				return mib_set(id, (void *)value);

			default:
				return 0;
		}
	}

	return 0;
}

#ifdef CAN_RW_FILE
int flash_read_to_file(CONFIG_DATA_T data_type, char * filename)
{
	int len;
	unsigned char * buf;
	int fh;
	PARAM_HEADER_Tp pHeader = mib_get_mib_header(data_type);

	fh = open(filename, O_CREAT|O_RDWR);
	if ( fh == -1 )
	{
		printf("open file fail!\n");
		return 0;
	}

	len = sizeof(PARAM_HEADER_T) + pHeader->len;	// total len = Header + MIB data
	buf = malloc(len);

	if(buf == 0)
	{
		printf("allocate memory fail!\n");
		goto error;
	}

	switch(data_type)
	{
		case CURRENT_SETTING:
			if(flash_read(buf, CURRENT_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		case DEFAULT_SETTING:
			if(flash_read(buf, DEFAULT_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		case HW_SETTING:
			if(flash_read(buf, HW_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		default:
			goto error;
	}

	lseek(fh, 0, SEEK_SET);

	if ( write(fh, buf, len) != len)
	{
		printf("write file fail!\n");
		goto error;
	}

	close(fh);
	sync();

	free(buf);
	return 1;
error:
	if(buf) free(buf);
	close(fh);
	return 0;
}

int flash_write_from_file(CONFIG_DATA_T data_type, char * filename)
{
	int fh;
	int size;
	unsigned char * buf;
	unsigned char * ptr;
	PARAM_HEADER_Tp pFileHeader;
	PARAM_HEADER_Tp pHeader = mib_get_mib_header(data_type);

	fh = open(filename, O_RDONLY);
	if ( fh == -1 )
	{
		printf("open file fail!\n");
		return 0;
	}

	size = lseek(fh, 0, SEEK_END);
	lseek(fh, 0, SEEK_SET);

	buf = malloc(size);

	if(buf == 0)
	{
		printf("allocate memory fail!\n");
		goto error;
	}

	if ( read(fh, buf, size) != size)
	{
		printf("read file fail!\n");
		goto error;
	}

	pFileHeader = (PARAM_HEADER_Tp) buf;

	if ( memcmp(pFileHeader->signature, pHeader->signature, SIGNATURE_LEN) )
	{
		printf("file signature error!\n");
		goto error;
	}

	if(pFileHeader->len != pHeader->len)
	{
		printf("file size error!\n");
		goto error;
	}

	ptr = &buf[sizeof(PARAM_HEADER_T)];			// point to start of MIB data
	DECODE_DATA(ptr,  pFileHeader->len);

	if(pFileHeader->checksum != CHECKSUM(ptr, pFileHeader->len))
	{
		printf("checksum error!\n");
		goto error;
	}

	if(mib_update_from_tbl(data_type, ptr) == 0)
	{
		printf("flash write error!\n");
		goto error;
	}

	free(buf);
	close(fh);
	return 1;

error:
	if(buf) free(buf);
	close(fh);
	return 0;
}
#endif // #ifdef CAN_RW_FILE

static int getdir(char *fullname, char *path, int loop)
{
	char tmpBuf[100], *p, *p1;

	strcpy(tmpBuf, fullname);
	path[0] = '\0';

	p1 = tmpBuf;
	while (1) {
		if ((p=strchr(p1, '/'))) {
			if (--loop == 0) {
				*p = '\0';
				strcpy(path, tmpBuf);
				return 0;
			}
			p1 = ++p;
		}
		else
			break;
	}
	return -1;
}

void unzip_web(char *file, char *dest_dir)
{
	PARAM_HEADER_T header;
	char *buf=NULL;
	char tmpFile[100], tmpFile1[100], tmpBuf[100];
	int fh=0, i, loop, size;
	FILE_ENTRY_Tp pEntry;
	struct stat sbuf;

#ifdef __mips__
	if(file) {
		printf("No file name required !\n");
		return;
	}

	if ( flash_read((char *)&header, WEB_PAGE_OFFSET, sizeof(header)) == 0) {
		printf("Read web header failed!\n");
		return;
	}
#else
	if(!file) {
		printf("File name required !\n");
		return;
	}

	if ((fh = open(file, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", file);
		return;
	}
	lseek(fh, 0L, SEEK_SET);
	if (read(fh, &header, sizeof(header)) != sizeof(header)) {
		printf("Read web header failed %s!\n", file);
		close(fh);
		return;
	}

	header.len = DWORD_SWAP(header.len);
#endif

	if (memcmp(header.signature, WEB_SIGNATURE_TAG, SIGNATURE_LEN)) {
		printf("Invalid web image!\n");
		return;
	}

	buf = malloc(header.len);
	printf("%x %d\n",buf, header.len);
	if (buf == NULL) {
		printf("Allocate buffer failed!\n");
		return;
	}

#ifdef __mips__
	if ( flash_read(buf, WEB_PAGE_OFFSET+sizeof(header), header.len) == 0) {
		printf("Read web image failed!\n");
		goto unzip_web_error;
	}
#else
	if (read(fh, buf, header.len) != header.len) {
		printf("Read web image failed!\n");
		goto unzip_web_error;
	}
	close(fh);
#endif

	if ( CHECKSUM(buf, header.len) != header.checksum) {
		printf("Web image invalid!\n");
		goto unzip_web_error;
	}

	// save to a file
	strcpy(tmpFile, "/var/flashweb.gz");
	fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", tmpFile );
		goto unzip_web_error;
	}
	if ( write(fh, buf, header.len) != header.len) {
		printf("write file error %s!\n", tmpFile);
		goto unzip_web_error;
	}
	close(fh);
	sync();

//	free with cause memory bug (mmap.c)
//	free(buf);

	// decompress file
	sprintf(tmpFile1, "%sXXXXXX", tmpFile);
	mkstemp(tmpFile1);

	sprintf(tmpBuf, "gunzip -c %s > %s", tmpFile, tmpFile1);
	system(tmpBuf);

	unlink(tmpFile);

	if (stat(tmpFile1, &sbuf) != 0) {
		printf("Stat file error %s!\n", tmpFile1);
		goto unzip_web_error;
	}
	if (sbuf.st_size < sizeof(FILE_ENTRY_T) ) {
		sprintf(tmpBuf, "Invalid decompress file size %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		unlink(tmpFile1);
		goto unzip_web_error;
	}

	buf = malloc(sbuf.st_size);
	printf("%x %d\n",buf, sbuf.st_size);
	if (buf == NULL) {
		sprintf(tmpBuf,"Allocate buffer failed %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		goto unzip_web_error;
	}
	if ((fh = open(tmpFile1, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile1);
		goto unzip_web_error;
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, sbuf.st_size) != sbuf.st_size) {
		printf("Read file error %ld!\n", sbuf.st_size);
		goto unzip_web_error;
	}
	close(fh);
	sync();
	unlink(tmpFile1);
	size = sbuf.st_size;
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];

#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
#endif

		strcpy(tmpFile, dest_dir);
		strcat(tmpFile, pEntry->name);

		loop = 0;
		while (1) {
			if (getdir(tmpFile, tmpBuf, ++loop) < 0)
				break;
			if (tmpBuf[0] && stat(tmpBuf, &sbuf) < 0) { // not exist
 				if ( mkdir(tmpBuf, S_IREAD|S_IWRITE) < 0) {
					printf("Create directory %s failed!\n", tmpBuf);
					goto unzip_web_error;
				}
			}
		}

		fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
		if (fh == -1) {
			printf("Create output file error %s!\n", tmpFile );
			goto unzip_web_error;
		}

		if ( write(fh, &buf[i+sizeof(FILE_ENTRY_T)], pEntry->size) != pEntry->size ) {
			printf("Write file error %s, len=%ld!\n", tmpFile, pEntry->size);
			goto unzip_web_error;
		}
		close(fh);
		i += (pEntry->size + sizeof(FILE_ENTRY_T));
	}

unzip_web_error:
	if(buf) free(buf);
	return;
}

