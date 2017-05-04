#include <string.h>
#include <unistd.h>
#include "mib.h"
#include "utility.h"
 
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#include "rtk/stat.h"
#include "rtk/switch.h"
#endif

#ifdef CONFIG_GPON_FEATURE
const char *OMCI_SW_VER_DEF[] = {"V1.7.1", "V1R007C00S001", "V1.7.1", "V1.7.1"};
const char OMCC_VER_DEF[] = {0x80,0x86,0xA0,0x80};
const char OMCI_TM_OPT_DEF[] = {2,2,2,2};
const char *OMCI_EQID_DEF[] = {"RTL9601B", "RTL9601B", "RTL9601B", "RTL9601B"};
const char *OMCI_ONT_VER_DEF[] = {"R1", "R1", "R1", "R1"};
const char *OMCI_VENDOR_ID_DEF[] = {"RTKG", "HWTC", "ZTEG", "RTKG"};

void getOMCI_EQID(char *buf)
{
	unsigned int chipId, rev, subType;
	rtk_switch_version_get(&chipId, &rev, &subType);
	switch(chipId)
	{			
		case RTL9601B_CHIP_ID:
			snprintf(buf, 10, "RTL9601B");
			break;
		case RTL9602C_CHIP_ID:
			snprintf(buf, 10, "RTL9602C");
			break;
		case RTL9607B_CHIP_ID:
			snprintf(buf, 10, "RTL9607B");
			break;
		case APOLLOMP_CHIP_ID:
		default:
			snprintf(buf, 10, "RTL%04X", chipId>>16);
			break;
	}
}

int checkOMCImib(char mode)
{
	char buf[100], newline[256], buf2[20];
	FILE *fp;
	char *cmd;
	
	int index, i;
	char *delim = " ";
	char *pch;
	char vChar;
	int startup=0;

	if(!mib_get(MIB_OMCI_OLT_MODE, &vChar))
		return 0;

	if(mode==-1){ //startup checking
		startup = 1;
		mode = vChar;
	}
	else if(mode == vChar) //if mode is not changed, do not update omci mib
		return 0;

	if(mode==0){
		fp = popen("nv getenv sw_version0", "r");
		if(fgets(newline, 256, fp) != NULL){
			sscanf(newline, "%*[^=]=%s", buf);
			//printf("sw version 1 %s\n", buf);
			if(!mib_set(MIB_OMCI_SW_VER1, buf))
				printf("set omci sw ver 1 failed\n");
		}
		pclose(fp);

		newline[0]='\0';
		buf[0]='\0';
		
		fp = popen("nv getenv sw_version1", "r");
		if(fgets(newline, 256, fp) != NULL){
			sscanf(newline, "%*[^=]=%s", buf);
			//printf("sw version 2 %s\n", buf);
			if(!mib_set(MIB_OMCI_SW_VER2, buf))
				printf("set omci sw ver 2 failed\n");
		}
		pclose(fp);

		buf[0]='\0';
		getOMCI_EQID(buf);
		if(!mib_set(MIB_OMCI_EQID, buf))
			printf("set omci eqid failed\n");

		newline[0]='\0';
		buf[0]='\0';
		fp = popen("dbg_tool get info", "r");
		if(fgets(newline, 256, fp) != NULL){
			sscanf(newline, "%s %*s", buf);
			index = atoi(buf);
			//printf("index %d\n", index);
			for(i=0;i<3; i++)
				fgets(newline, 256, fp);

			//printf ("Splitting string \"%s\" into tokens:\n",str);
			pch = strtok(newline,delim);
			i = 0;
			while (pch != NULL)
			{
				if(index==i) break;
				//printf ("%s\n",pch);
				pch = strtok (NULL, delim);
				i++;
			} 
			//printf ("%s\n",pch);
			snprintf(buf, 100, "R%d", atoi(pch));
			if(!mib_set(MIB_OMCI_ONT_VER, buf))
				printf("set omci ont version failed\n");
		}
		pclose(fp);

		if(!startup){
			vChar = 0x80;
			if(!mib_set(MIB_OMCC_VER, &vChar))
				printf("set omcc version failed\n");
			vChar = 2;
			if(!mib_set(MIB_OMCI_TM_OPT, &vChar))
				printf("set omci tm opt failed\n");
		}
		

	}
	else{
		if(!mib_set(MIB_OMCI_SW_VER1, OMCI_SW_VER_DEF[mode]))
			printf("set omci sw ver 1 failed\n");
		if(!mib_set(MIB_OMCI_SW_VER2, OMCI_SW_VER_DEF[mode]))
			printf("set omci sw ver 2 failed\n");
		//if(!mib_set(MIB_OMCI_EQID, OMCI_EQID_DEF[mode]))
		//	printf("set omci eqid failed\n");

		getOMCI_EQID(buf);
		if(!mib_set(MIB_OMCI_EQID, buf))
			printf("set omci eqid failed\n");
		
		if(!mib_set(MIB_OMCI_ONT_VER, OMCI_ONT_VER_DEF[mode]))
			printf("set omci ont version failed\n");
		
		if(!startup){
			vChar = OMCC_VER_DEF[mode];
			if(!mib_set(MIB_OMCC_VER, &vChar))
				printf("set omcc version failed\n");
			vChar = OMCI_TM_OPT_DEF[mode];
			if(!mib_set(MIB_OMCI_TM_OPT, &vChar))
				printf("set omci tm opt failed\n");
		}
		
	}
	if(mode==0 || mode==3){
		buf[0]='\0';
		mib_get(MIB_GPON_SN, buf);
		strncpy(buf2, buf, 4);
		buf2[5]="\0";
		if(!mib_set(MIB_OMCI_VENDOR_ID, buf2))
			printf("set omci vendor id failed\n");
		
	}
	else{
		if(!mib_set(MIB_OMCI_VENDOR_ID, OMCI_VENDOR_ID_DEF[mode]))
			printf("set omci vendor id failed\n");
	}
	return 1;
}
void checkOMCI_startup(void)
{
	char mode;
	int ret;
	
	if(!mib_get(MIB_OMCI_OLT_MODE, &mode))
		return;

	if(mode==0){
		ret = checkOMCImib(-1);

#ifdef COMMIT_IMMEDIATELY
		if(ret)
			Commit();
#endif
	}

}

#endif
