
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/atm.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* for opendir, readdir, kill */
#include <dirent.h>
#include <signal.h>

/* for ioctl */
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#include "mibtbl.h"
#include "utility.h"

//#define FLASH_TEST

static int mib_table_index(char *mibname);
static int mib_name_resolve(char *from, char *to);
static int mib_value_resolve(int index, int count, char** av, char *value);
int mib_tbl_get_by_name(char* name, char* arg, void *value);
int mib_get_by_name(char* name, void *value);
CONFIG_DATA_T mib_set_by_index(int index, char *value);
CONFIG_DATA_T mib_set_by_name(char* name, char *value);
void mib_get_all(void);
void mib_get_all_by_type(CONFIG_DATA_T data_type);
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
	printf("  all [cs|hs] \t\t\tdump all flash parameters.\n");
#ifdef CAN_RW_FILE
	printf("  wds FILENAME \t\t\twrite flash parameters to default settings.\n");
	printf("  wcs FILENAME \t\t\twrite flash parameters to current settings.\n");
	printf("  whs FILENAME \t\t\twrite flash parameters to hardware settings.\n");
	printf("  rds FILENAME \t\t\tread flash parameters from default settings.\n");
	printf("  rcs FILENAME \t\t\tread flash parameters from current settings.\n");
	printf("  rhs FILENAME \t\t\tread flash parameters from hardware settings.\n");
#endif // #ifdef CAN_RW_FILE
#ifdef VOIP_SUPPORT
	printf(" voip \t\t\tvoip setings.\n");
#endif /*VOIP_SUPPORT*/
	printf("  reset \t\t\treset current setting to default.\n");
	printf("  default <cs/hs>\t\twrite program default value to flash.\n");
	printf("  dump <cs/hs>\t\t\tdump flash setting.\n");
	printf("  unzip web\t\t\tunzip web pages.\n\n");
}

#ifdef USE_11N_UDP_SERVER
static unsigned char udp_server_cmd=0;
#endif

int main(int argc, char** argv)
{
	int action=0;
	int argNum=1;
	char mib[100]={0};
	char mibvalue[512]={0};
#ifdef CAN_RW_FILE
	CONFIG_DATA_T data_type;
#endif // #ifdef CAN_RW_FILE

	// Kaohj
	mib_init();

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
		else if ( !strcmp(argv[argNum], "loop") ) {
			while(1)
			{
				usleep(1000);
			}
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
			argNum++;
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
		else if ( !strcmp(argv[argNum], "ip") ) {
			action = 16;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "dump") ) {
			action = 17;
 			argNum++;
		}
#ifdef FLASH_TEST
		// Kaohj, for test
		else if ( !strcmp(argv[argNum], "test") ) {
			action = 18;
		}
#endif
/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
		else if(!strcmp(argv[argNum], "voip")){
			if (argc >= 2) // have voip param
				return flash_voip_cmd(argc - 2, &argv[2]);
			else
				return flash_voip_cmd(0, NULL);
		}

#endif /*VOIP_SUPPORT*/
/*-----end-----*/
#if 0 //def CONFIG_WIFI_SIMPLE_CONFIG
		else if ( !strcmp(argv[argNum], "upd-wsc-conf") ) {
			return updateWscConf(argv[argNum+1], argv[argNum+2], 0);
		}

		else if ( !strcmp(argv[argNum], "gen-pin") ) {
			return updateWscConf(0, 0, 1);
		}
		else if (!strcmp(argv[argNum], "-param_file")) {
			readFileSetParam(argv[argNum+1]);
			return 0;

		}
#endif // WIFI_SIMPLE_CONFIG

#ifdef USE_11N_UDP_SERVER //cathy test
		else if ( !strcmp(argv[argNum], "gethw") ) {
			//printf("gethw => action 19\n");
			action = 19;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "sethw") ) {
			//printf("gethw => action 19\n");
			action = 20;
			argNum++;
		}
#endif
		// Write a word(4 bytes) value to flash(offset)
		// Usage: flash w [offset] [value]
		// ex. Write 0x12 to 0xbffc0010 --> flash w 0x3c0010 0x12
		else if (!strcmp(argv[argNum], "w")){
			unsigned int offset, fvalue;

			if (argc < 4) {
				printf("Invalid arguments!\n");
				return 0;
			}
			argNum++;
			offset = strtol(argv[argNum++], 0, 0);
			fvalue = strtoul(argv[argNum++], 0, 0);
			printf("flash write 0x%x to 0x%x\n", fvalue, 0xbfc00000+offset);
			return flashdrv_write2(offset, &fvalue, 4);
		}
		// Read a word(4 bytes) from flash(offset)
		// Usage: flash r [offset]
		// ex. Read a word on 0xbffc0000 --> flash r 0x3c0000
		else if (!strcmp(argv[argNum], "r")){
			unsigned int offset;
			unsigned mydata;

			if (argc < 3) {
				printf("Invalid arguments!\n");
				return 0;
			}
			argNum++;
			offset = strtol(argv[argNum++], 0, 0);
			flashdrv_read(&mydata, offset, 4);
			printf("data=0x%x\n", mydata);
			return 0;
		}
		// Erase flash block with address(offset)
		// Usage: flash erase [offset]
		// ex. Erase block 0xbffc0000 --> flash erase 0x3c0000
		else if (!strcmp(argv[argNum], "erase")){
			unsigned int offset;

			if (argc < 3) {
				printf("Invalid arguments!\n");
				return 0;
			}
			argNum++;
			offset = strtol(argv[argNum++], 0, 0);
			//printf("flash erase on 0x%x\n", 0xbfc00000+offset);
			return flashdrv_erase(offset);
		}
	}

	if(action==0)
	{
		showHelp();
		return 0;
	}

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
		return 0;
	}

	if(action==17)
	{
		PARAM_HEADER_T header;
		unsigned int fileSize, i;
		unsigned char *pFile=NULL;
		CONFIG_DATA_T data_type;
		int file_offset, table_size;
		char tdata[SIGNATURE_LEN+1];

		if (argc <=2) {
			printf("command error!\n");
			showHelp();
			return 0;
		}
		if ( !strcmp(argv[argNum], "ds") )
		{
			data_type = DEFAULT_SETTING;
			file_offset = DEFAULT_SETTING_OFFSET;
			table_size = sizeof(MIB_T);
		}
		else if ( !strcmp(argv[argNum], "cs") )
		{
			data_type = CURRENT_SETTING;
			//file_offset = CURRENT_SETTING_OFFSET;
			file_offset = g_cs_offset;
			table_size = sizeof(MIB_T);
		}
		else if ( !strcmp(argv[argNum], "hs") )
		{
			data_type = HW_SETTING;
			file_offset = HW_SETTING_OFFSET;
			table_size = sizeof(HW_MIB_T);
		}
		else
		{
			return 0;
		}

		if(__mib_header_read(data_type, &header) != 1)
			return 0;

		fileSize = header.len + sizeof(PARAM_HEADER_T);
		pFile = malloc(fileSize);
		if ( pFile == NULL )
			return 0;

		if(__mib_file_read(data_type, pFile, fileSize) != 1)
			return 0;

		printf("Header Length: %d\tData Length: %d (Table=%d, Chain=%d)\n",
			sizeof(PARAM_HEADER_T), header.len, table_size, header.len-table_size);
		for (i=0; i<SIGNATURE_LEN; i++)
			tdata[i] = pFile[i];
		tdata[SIGNATURE_LEN] = '\0';
		printf("Signature: %s\nVersion: %d\nChecksum: 0x%.2x\nLength: %d\n", tdata, header.version, header.checksum, header.len);
		printf("------------------ Header+Data ------------------------\n");
		for (i=0;i<fileSize;i+=16)
		{
			//printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
			printf("%.5x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n"
				,file_offset+i ,pFile[i], pFile[i+1], pFile[i+2], pFile[i+3], pFile[i+4], pFile[i+5], pFile[i+6], pFile[i+7], pFile[i+8], pFile[i+9], pFile[i+10], pFile[i+11], pFile[i+12], pFile[i+13], pFile[i+14], pFile[i+15]);
		}

		if(pFile) free(pFile);
		return 0;
	}

	if(action==11)
	{
		if (argc <=2) {
			printf("command error!\n");
			showHelp();
			return 0;
		}
		if ( !strcmp(argv[argNum+1], "ds") )
		{
			mib_init_mib_with_program_default(DEFAULT_SETTING, FLASH_DEFAULT_TO_ALL);
		}
		else if ( !strcmp(argv[argNum+1], "cs") )
		{
			mib_init_mib_with_program_default(CURRENT_SETTING, FLASH_DEFAULT_TO_ALL);
		}
		else if ( !strcmp(argv[argNum+1], "hs") )
		{
			mib_init_mib_with_program_default(HW_SETTING, FLASH_DEFAULT_TO_ALL);
		}

		return 0;
	}

	if(action==12)
	{
		if(argNum < argc)
		{
			if ( !strcmp(argv[argNum], "web") )
			{
#ifdef EMBED
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
		unsigned char* pFlashBank, reset_flash, cs_first_time;
		unsigned int fileSize = 0;
		unsigned char* pFile = NULL;
		unsigned char* pContent = NULL;
		// Kaohj, check for mib chain
		unsigned char* pVarLenTable = NULL;
		unsigned int contentMinSize = 0;
		unsigned int varLenTableSize = 0;
		
		for(i=0;i<3;i++)
		{
			if(i==0)
			{
				data_type = CURRENT_SETTING;
				pFlashBank = "CS_BANK";
				cs_first_time = 1;
			}
			else if(i==1)
			{
				data_type = DEFAULT_SETTING;
				pFlashBank = "DS_BANK";

				// Added by Mason Yu. Not use default seetting.
				continue;
			}
			else if(i==2)
			{
				data_type = HW_SETTING;
				pFlashBank = "HS_BANK";
			}
			reset_flash=0;

check_flash:
			if(__mib_header_read(data_type, &header) != 1)
				goto flash_check_error;

			if(__mib_header_check(data_type, &header) != 1)
				goto flash_check_error;

			fileSize = header.len + sizeof(PARAM_HEADER_T);
			pFile = malloc(fileSize);
			if ( pFile == NULL )
				goto flash_check_error;

			if(__mib_file_read(data_type, pFile, fileSize) != 1) {
				free(pFile);
				goto flash_check_error;
			}

			pContent = &pFile[sizeof(PARAM_HEADER_T)];	// point to start of MIB data

			if(__mib_content_decod_check(data_type, &header, pContent) != 1) {
				free(pFile);
				goto flash_check_error;
			}

			// Kaohj, check for mib chain
			contentMinSize = __mib_content_min_size(data_type);
			varLenTableSize = header.len - contentMinSize;
			if(varLenTableSize > 0)
			{
				pVarLenTable = &pContent[contentMinSize];	// point to start of variable length MIB data

				// parse variable length MIB data
				if( __mib_chain_record_content_decod(pVarLenTable, varLenTableSize) != 1)
				{
					free(pFile);
					//printf("mib_chain_record decode fail !\n");
					goto flash_check_error;
				}
			}

			free(pFile);
			printf("%s ok! (minsize=%d, varsize=%d)\n", pFlashBank, contentMinSize, varLenTableSize);
			continue;
flash_check_error:
			if (data_type == CURRENT_SETTING && cs_first_time) {
				/* the checking of CS fails first time */
				cs_first_time = 0;

				flash_read(&header, g_cs_bak_offset, sizeof(header));

				if (memcmp(header.signature, CS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN) ||
					(header.len < CURRENT_SETTING_MIN_LEN) || 
					(header.len > CURRENT_SETTING_MAX_LEN)) {
					fileSize = CURRENT_SETTING_MAX_LEN;
				} else {
					fileSize = header.len + sizeof(header);
				}

				pFile = malloc(fileSize);
				if (pFile == NULL)
					goto flash_check_error;
				flash_read(pFile, g_cs_bak_offset, fileSize);
				flash_write(pFile, g_cs_offset, fileSize);
				free(pFile);

                                goto check_flash;  //check again
			}

                        if (!reset_flash) {  //not reset yet
                            if (mib_init_mib_with_program_default(data_type, FLASH_DEFAULT_TO_AUGMENT)) {  //try to reset flash
                            	printf("%s check fail! Reset to default\n", pFlashBank);
                                reset_flash=1;
#ifdef CONFIG_USER_RTK_RECOVER_SETTING
					FILE *fp;
					fp = fopen(FLASH_CHECK_FAIL, "w");
					if (fp)
						fclose(fp);
#endif
                                goto check_flash;  //check again
                            };
                        };

			printf("%s fail!\n", pFlashBank);
			return -1;
		}

		return 0;
	}

	if(action==14)
	{
		unsigned char buf[] = "CLEAR";

		//flash_write(buf, CURRENT_SETTING_OFFSET, sizeof(buf));
		flash_write(buf, g_cs_offset, sizeof(buf));
		flash_write(buf, g_cs_bak_offset, sizeof(buf));
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

	if(action==16)
	{
		struct in_addr ipAddr;

		if (getInAddr( argv[argNum], IP_ADDR, (void *)&ipAddr) != 1)
			printf("IP=0.0.0.0\n");
		else
			printf("IP=%s\n", inet_ntoa(ipAddr));

		return 0;
	}
#ifdef FLASH_TEST
	// Kaohj, for test
	if(action==18)
	{
		int idx;
		volatile int i;

		printf("writing start at 0xbfc20000 ");
		for (idx=0; idx<=23; idx++) {
			// write start at offset 0x20000, each block 0x10000
			flash_write(0x80500000, 0x20000+idx*0x10000, 0x10000);
			printf(".");
			i=0;
			usleep(200000);
			//while(i<=0xfffff)
			//	i++;
		}
		printf("write Ok\n");
		return 0;
	}
#endif

#ifdef USE_11N_UDP_SERVER	//cathy, 11N_UDP_SERVER
	if( action == 19 ) {	//flash gethw
		unsigned char buffer[1024];
		udp_server_cmd = 1;
		while(argNum < argc)
		{
			mib_name_resolve(argv[argNum], mib);
			if(mib_get_by_name(mib, buffer) != 0)
			{
				printf("%s=%s\n",argv[argNum],buffer);
			}

			argNum++;
		}
		udp_server_cmd = 0;
		return 0;
	}
	if( action == 20 ) {	//flash sethw
		CONFIG_DATA_T tmpType=UNKNOWN_SETTING;
		CONFIG_DATA_T cdType=UNKNOWN_SETTING;
		int index;
		while((argNum + 1) < argc)
		{
			mib_name_resolve(argv[argNum], mib);
			if ((index = mib_table_index(mib)) == -1) {
				printf("mib name error: %s\n", argv[argNum]);
				return 0;
			}
			if (mib_table[index].mib_type != HW_SETTING) {
				printf("mib(%s) not in HW_SETTING.\n", mib);
				return 0;
			}
			argNum += mib_value_resolve(index, argc-argNum, &argv[argNum], mibvalue);

			if((tmpType=mib_set_by_index(index, mibvalue)) != 0) {
				cdType |= tmpType;
				printf("set %s=%s\n",mib,mibvalue);
			}
			else
				printf("set %s=%s fail!\n",mib,mibvalue);
		}
		if((cdType&CURRENT_SETTING) && _mib_update(CURRENT_SETTING) == 0)
			printf("CS Flash error! \n");

		if((cdType&HW_SETTING) && _mib_update(HW_SETTING) == 0)
			printf("HS Flash error! \n");

		return 0;
	}
#endif
	if(action==1) // get
	{
		unsigned char buffer[1024];
		while(argNum < argc)
		{
			memset(buffer, 0x00 , 1024);

			mib_name_resolve(argv[argNum], mib);
#ifdef USE_11N_UDP_SERVER
			if(!strncmp(mib , "HW_WLAN" , 7))
				udp_server_cmd = 1;
#endif

			if(mib_tbl_get_by_name(mib, argv[argNum+1], buffer) != 0)
			{
				argNum++; // chain index
				printf("%s\n",buffer);
			}
			else if(mib_get_by_name(mib, buffer) != 0)
			{
				printf("%s=%s\n",argv[argNum],buffer);
			}
			argNum++;
#ifdef USE_11N_UDP_SERVER
			udp_server_cmd = 0;
#endif
		}
	}

	if(action==2) // set
	{
		CONFIG_DATA_T tmpType=UNKNOWN_SETTING;
		CONFIG_DATA_T cdType=UNKNOWN_SETTING;
		int index;
		while((argNum + 1) < argc)
		{
			mib_name_resolve(argv[argNum], mib);
			if ((index = mib_table_index(mib)) == -1) {
				printf("mib name error: %s\n", argv[argNum]);
				return 0;
			}
			argNum += mib_value_resolve(index, argc-argNum, &argv[argNum], mibvalue);

			if((tmpType=mib_set_by_index(index, mibvalue)) != 0) {
				cdType |= tmpType;
				printf("set %s=%s\n",mib,mibvalue);
			}
			else
				printf("set %s=%s fail!\n",mib,mibvalue);
		}

		if((cdType&CURRENT_SETTING) && _mib_update(CURRENT_SETTING) == 0)
			printf("CS Flash error! \n");

		if((cdType&HW_SETTING) && _mib_update(HW_SETTING) == 0)
			printf("HS Flash error! \n");
	}

	if(action==3)
	{
		if (argc == 2)
			mib_get_all();
		else {
			if (!strcmp(argv[argNum], "hs"))
				mib_get_all_by_type(HW_SETTING);
			else if (!strcmp(argv[argNum], "cs"))
				mib_get_all_by_type(CURRENT_SETTING);
			else if (!strcmp(argv[argNum], "rs"))
				mib_get_all_by_type(RUNNING_SETTING);
			else
				mib_get_all_by_type(UNKNOWN_SETTING);
		}
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



	return 0;
}

// Kaohj: copied from saveconfig.c
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
		for (i = 0; i < array_size; i++) {
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
		printf("%s: Unknown data type  %d!\n", name, type);
	}

	printf("%-18s = %s\n", name, svalue);
}

/*
 *	Get mib_table index by mibname.
 *	Return mib_table index on success or -1 on error.
 */
static int mib_table_index(char *mibname)
{
	int index;

	for (index = 0; mib_table[index].id; index++) {
		if (!strcmp(mib_table[index].name, mibname))
			break;
	}
	if (mib_table[index].id == 0)
		index = -1;
	return index;
}

/*
 *	Copy or map mib name.
 *	Return mib_table index on success or -1 on error.
 */
static int mib_name_resolve(char *from, char *to)
{
	if((!strcmp(from, "HW_NIC0_ADDR"))  || (!strcmp(from, "HW_WLAN0_WLAN_ADDR")) )
	{
		strcpy(to, "ELAN_MAC_ADDR");
	}
	else if((!strcmp(from, "HW_NIC1_ADDR"))  || (!strcmp(from, "HW_WLAN1_WLAN_ADDR")) )
	{
		strcpy(to, "ELAN_MAC_ADDR");
	}
	else
		strcpy(to, from);

	return 0;
}

/*
 *	Get mib value according to av list.
 *	index: mib_table index
 *	count: argument count
 *	av: list of argument; first argument is mib_name
 *	value: return value
 */
static int mib_value_resolve(int index, int count, char** av, char *value)
{
	int avNum=0;
	unsigned char pwr;

	value[0] = 0;

	if (mib_table[index].type == BYTE_ARRAY_T && !memcmp((av[avNum]+9), "TX_POWER", 8) && count>=15) {
		// wifi hw rf parameter; translate to HEX string
		//printf("get %s , count : %d , avNum : %d \n " , av[avNum], count, avNum);
		avNum++;
		while( (avNum < count) &&  (avNum < 200) ) {
			pwr = atoi(av[avNum++]);
			sprintf(value, "%s%02x", value, pwr);
		}
	}
	else {
		avNum++;
		sscanf(av[avNum++], "%s", value);
	}

	return avNum;
}

static char chain_buf[8192];
static char chainEntry[2048];
int mib_tbl_get_by_name(char* name, char* arg, void *value)
{
	int i, k;
	int id = -1;
	mib_chain_member_entry_T *rec_desc;

	for (i=0; mib_chain_record_table[i].id; i++) {
		if ( !strcmp(mib_chain_record_table[i].name, name) )
		{
			id = mib_chain_record_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned int total = _mib_chain_total(id);
		unsigned int recordNum;

		if ( !strcmp("NUM", arg) )
		{
			sprintf(value,"%s_NUM=%u",name , total);
			return 1;
		}

		sscanf(arg, "%u", &recordNum);

		if(recordNum < total)
		{
			if (mib_chain_desc_id(id, (void *)&chain_buf)==0) {
				printf("Empty MIB chain %s descriptor !\n", name);
				return 0;
			}

			rec_desc = (mib_chain_member_entry_T *)&chain_buf[0];
			mib_chain_get(id, recordNum, (void *)&chainEntry[0]);
			k = 0;
			while (rec_desc[k].name[0] != 0) {
				print_name_value(rec_desc[k].name, chainEntry+rec_desc[k].offset, rec_desc[k].type, rec_desc[k].size);
				k++;
			}
		}
		else
			printf("out of range !\n");
		return 1;
	}

	return 0;
}

#if 0
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
		unsigned int total = _mib_chain_total(id);
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
				char *pStr;
				MIB_CE_ATM_VC_Tp pEntry = (MIB_CE_ATM_VC_Tp) _mib_chain_get(MIB_ATM_VC_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					char myip[16];
					strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
					myip[15] = '\0';
					sprintf(value,"%s_IFINDEX=0x%x\n%s_VPI=%u\n%s_VCI=%u\n%s_QoS=%s\n%s_PCR=%u\n%s_SCR=%u\n%s_MBS=%u\n%s_CDVT=%u\n"
						,name , pEntry->ifIndex
						,name , pEntry->vpi
						,name , pEntry->vci
						,name , (pEntry->qos == 0)?"ubr":(pEntry->qos == 1)?"cbr":(pEntry->qos == 2)?"rt-vbr":"nrt-vbr"
						,name , pEntry->pcr
						,name , pEntry->scr
						,name , pEntry->mbs
						,name , pEntry->cdvt);
					pStr = (char *)value+strlen((char *)value);
					sprintf(pStr, "%s_ENCAP=%u\n%s_NAPT=%u\n%s_CMODE=%u\n%s_BRMODE=%u\n"
						"%s_PPPNAME=%s\n%s_PPPPWD=%s\n%s_PPPTYPE=%u\n"
						"%s_PPPIDLE=%u\n%s_DHCP=%u\n%s_RIP=%u\n"
						"%s_MYIP=%s\n%s_REMOTEIP=%s\n%s_ENABLE=%u"
						, name, pEntry->encap
						, name, pEntry->napt
						, name, pEntry->cmode
						, name, pEntry->brmode
						, name, pEntry->pppUsername
						, name, pEntry->pppPassword
						, name, pEntry->pppCtype
						, name, pEntry->pppIdleTime
						, name, pEntry->ipDhcp
						, name, pEntry->rip
						, name, myip
						, name, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr))
						, name, pEntry->enable);

					return 1;
				}
			}
			else if(id == MIB_PORT_FW_TBL)
			{
				MIB_CE_PORT_FW_Tp pEntry = (MIB_CE_PORT_FW_Tp) _mib_chain_get(MIB_PORT_FW_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_IP=%s\n%s_FROM=%u\n%s_TO=%u\n%s_PROTO=%u"
						,name , inet_ntoa(*((struct in_addr *)(pEntry->ipAddr)))
						,name , pEntry->fromPort
						,name , pEntry->toPort
						,name , pEntry->protoType);

					return 1;
				}
			}
			else if(id == MIB_MAC_FILTER_TBL)
			{
				MIB_CE_MAC_FILTER_Tp pEntry = (MIB_CE_MAC_FILTER_Tp) _mib_chain_get(MIB_MAC_FILTER_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_ACTION=%u\n%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x"
						,name, pEntry->action
						,name
						,pEntry->srcMac[0]
						,pEntry->srcMac[1]
						,pEntry->srcMac[2]
						,pEntry->srcMac[3]
						,pEntry->srcMac[4]
						,pEntry->srcMac[5]);

					return 1;
				}
			}
			else if(id == MIB_IP_PORT_FILTER_TBL)
			{
				MIB_CE_IP_PORT_FILTER_Tp pEntry = (MIB_CE_IP_PORT_FILTER_Tp) _mib_chain_get(MIB_IP_PORT_FILTER_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_ACTION=%u\n%s_PROTO=%u\n%s_SIP=%s\n%s_SMASK=%u\n%s_SFROM=%u\n%s_STO=%u\n"
							"%s_DIP=%s\n%s_DMASK=%u\n%s_DFROM=%u\n%s_DTO=%u"
						,name , pEntry->action
						,name , pEntry->protoType
						,name , inet_ntoa(*((struct in_addr *)(pEntry->srcIp)))
						,name , pEntry->smaskbit
						,name , pEntry->srcPortFrom
						,name , pEntry->srcPortTo
						,name , inet_ntoa(*((struct in_addr *)(pEntry->dstIp)))
						,name , pEntry->dmaskbit
						,name , pEntry->dstPortFrom
						,name , pEntry->dstPortTo);

					return 1;
				}
			}
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
			else if(id == MIB_WLAN_AC_TBL)
			{
				MIB_CE_WLAN_AC_Tp pEntry = (MIB_CE_WLAN_AC_Tp) _mib_chain_get(MIB_WLAN_AC_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_wlanIdx=%u\n%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x"
						,name
						,pEntry->wlanIdx
						,name
						,pEntry->macAddr[0]
						,pEntry->macAddr[1]
						,pEntry->macAddr[2]
						,pEntry->macAddr[3]
						,pEntry->macAddr[4]
						,pEntry->macAddr[5]);

					return 1;
				}
			}
#endif
			else if(id == MIB_MBSSIB_TBL)
			{
				MIB_CE_MBSSIB_Tp pEntry = (MIB_CE_MBSSIB_Tp)_mib_chain_get(MIB_MBSSIB_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value, "%s_idx=%u\n%s_encrypt=%u\n%s_enable1X=%u\n%s_wep=%u\n%s_wpaAuth=%u\n"
						"%s_wpaPSKFormat=%u\n%s_wpaGroupRekeyTime=%u\n%s_wlanDisabled=%u\n%s_ssid=%s\n%s_wlanMode=%u\n"
						"%s_authType=%u\n%s_unicastCipher=%u\n%s_wpa2UnicastCipher=%u\n"
						, name, pEntry->idx
						, name, pEntry->encrypt
						, name, pEntry->enable1X
						, name, pEntry->wep
						, name, pEntry->wpaAuth
						, name, pEntry->wpaPSKFormat
						, name, pEntry->wpaGroupRekeyTime
						, name, pEntry->wlanDisabled
						, name, pEntry->ssid
						, name, pEntry->wlanMode
						, name, pEntry->authType
						, name, pEntry->unicastCipher
						, name, pEntry->wpa2UnicastCipher);
					return 1;
				}
			}
#endif

			else if(id == MIB_PPPOE_SESSION_TBL)	// Jenny
			{
				MIB_CE_PPPOE_SESSION_Tp pEntry = (MIB_CE_PPPOE_SESSION_Tp) _mib_chain_get(MIB_PPPOE_SESSION_TBL, recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_IFNO=%u\n%s_VPI=%u\n%s_VCI=%u\n%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x\n%s_SESSIONID=%u\n"
						,name, pEntry->ifNo
						,name, pEntry->vpi
						,name, pEntry->vci
						,name
						,pEntry->acMac[0]
						,pEntry->acMac[1]
						,pEntry->acMac[2]
						,pEntry->acMac[3]
						,pEntry->acMac[4]
						,pEntry->acMac[5]
						,name, pEntry->sessionId);

					return 1;
				}
			}
#ifdef ACCOUNT_CONFIG
			else if (id == MIB_ACCOUNT_CONFIG_TBL)	// Jenny
			{
				MIB_CE_ACCOUNT_CONFIG_Tp pEntry = (MIB_CE_ACCOUNT_CONFIG_Tp) _mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_USER=%s\n%s_PASSWD=%s\n%s_PRIV=%u\n"
						,name, pEntry->userName
						,name, pEntry->userPassword
						,name, pEntry->privilege);
					return 1;
				}
			}
#endif
		}
	}

	return 0;
}
#endif


/*
 * Get MIB Value
 */
int mib_get_by_name(char *name, void *value)	// get default value
{
	int i, j, id = -1;
	unsigned char buffer[1024], tmp[16];

	for (i = 0; mib_table[i].id; i++) {
		if (!strcmp(mib_table[i].name, name)) {
			id = mib_table[i].id;
			break;
		}
	}

	if (id != -1) {
		memset(buffer, 0x00, sizeof(buffer));

		/* fails */
		if (_mib_get(id, buffer) == 0)
			return 0;

		switch (mib_table[i].type) {
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
			for (j = 0; j < mib_table[i].size; j++) {
				sprintf(tmp, "%02x", buffer[j]);
				strcat(value, tmp);
			}
			return 1;

		case BYTE_ARRAY_T:
			((unsigned char *)value)[0] = '\0';
			for (j = 0; j < mib_table[i].size; j++) {
#ifdef USE_11N_UDP_SERVER
				if (udp_server_cmd) {
					sprintf(tmp, "%02x", buffer[j]);
				} else {
#endif
					sprintf(tmp, "%02x", buffer[j]);
					if (j != (mib_table[i].size - 1))
						strcat(tmp, ",");
#ifdef USE_11N_UDP_SERVER
				}
#endif
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
 * Get all MIB Value
 */
void mib_get_all(void)
{
	int i, total;
	unsigned char buffer[1024];

	/* -------------------------------------------
	 *	mib chain
	 * -------------------------------------------*/
	printf("---------------------------------------------------------\n");
	printf("MIB Chain Information\n");
	printf("---------------------------------------------------------\n");
	printf("%4s %-26s %12s %5s %5s\n", "ID", "Name", "size/record", "Num", "Max");
	for (i=0; mib_chain_record_table[i].id; i++) {
		total = _mib_chain_total(mib_chain_record_table[i].id);
		printf("%4d %-26s %12d %5d %5d\n", mib_chain_record_table[i].id,
			mib_chain_record_table[i].name,
			mib_chain_record_table[i].per_record_size, total,
			mib_chain_record_table[i].table_size);
	}
	printf("Header length per Chain record: %d bytes\n", sizeof(MIB_CHAIN_RECORD_HDR_T));
	/* -------------------------------------------
	 *	mib table
	 * -------------------------------------------*/
	printf("\n---------------------------------------------------------\n");
	printf("MIB Table Information\n");
	printf("---------------------------------------------------------\n");
	for (i=0; mib_table[i].id; i++) {
		if(mib_table[i].mib_type != RUNNING_SETTING && mib_get_by_name(mib_table[i].name, buffer) != 0) {
			printf("%s=%s\n",mib_table[i].name,buffer);
		}
	}
	printf("---------------------------------------------------------\n");
	printf("Total size: %d bytes\n", sizeof(MIB_T));
}

void mib_get_all_by_type(CONFIG_DATA_T data_type)
{
	int i;
	unsigned char buffer[1024];

	printf("\n---------------------------------------------------------\n");
	printf("MIB Table Information(%d)\n", data_type);
	printf("---------------------------------------------------------\n");
	for (i=0; mib_table[i].id; i++) {
		if((mib_table[i].mib_type & data_type) && mib_get_by_name(mib_table[i].name, buffer) != 0) {
			printf("%s=%s\n",mib_table[i].name,buffer);
		}
	}
}

/*
 * Set MIB Value
 */
CONFIG_DATA_T mib_set_by_index(int index, char *value)
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

	id = mib_table[index].id;
	cdtype = mib_table[index].mib_type;

	if (id != -1) {
		switch (mib_table[index].type) {
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
			string_to_hex(value, buffer, mib_table[index].size * 2);
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
	if (_mib_set(id, p))
		return cdtype;
	else
		return 0;
}

/*
 * Set MIB Value
 */
CONFIG_DATA_T mib_set_by_name(char *name, char *value)
{
	int index;

	if ((index = mib_table_index(name)) == -1)
		return UNKNOWN_SETTING;
	return mib_set_by_index(index, value);
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
			//if(flash_read(buf, CURRENT_SETTING_OFFSET, len) == 0)
			if(flash_read(buf, g_cs_offset, len) == 0)
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

	if(_mib_update_from_tbl(data_type, ptr) == 0)
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

#ifdef EMBED
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

#ifdef EMBED
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

#ifndef EMBED
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

