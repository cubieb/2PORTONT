#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include "rtk/epon.h"
#endif

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "multilang.h"

///////////////////////////////////////////////////////////////////

int fmepon_checkWrite(int eid, request * wp, int argc, char **argv)
{	
	char *name;	char tmpBuf[100];	
	char loid[100]={0};	
	char passwd[100]={0};
	
	if (boaArgs(argc, argv, "%s", &name) < 1) 
	{   		
		boaError(wp, 400, "Insufficient args\n");		
		printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );   		
		return -1;   	
	}	

	if(!strcmp(name, "fmepon_loid")) 	
	{		
		//printf("[%s:%d] name =%s\n",__func__,__LINE__,name);			
		if(!mib_get(MIB_LOID,  (void *)loid))		
		{	  		
			sprintf(tmpBuf, "%s (EPON LOID)",Tget_mib_error);			
			goto setErr;		
		}		
		boaWrite(wp, "%s", loid);		
		return 0;	
	}

        if(!strcmp(name, "fmepon_loid_password"))
        {               
        	//printf("[%s:%d] name =%s\n",__func__,__LINE__,name);
            if(!mib_get(MIB_LOID_PASSWD,  (void *)passwd))
            {
				sprintf(tmpBuf, "%s (EPON LOID Password)",Tget_mib_error);			
                goto setErr;
            }
            boaWrite(wp, "%s", passwd);
            return 0;
        }	
		
	return 0;
	
setErr:	
		ERR_MSG(tmpBuf);	
		return -1;
}


void formeponConf(request * wp, char *path, char *query)
{	
	char	*strData, *strData_loid, *strData_passwd;	
	char tmpBuf[100];	
	int entryNum=2;
	int index=0;	
	rtk_epon_llid_entry_t llid_entry;
	char oamcli_cmd[128]={0};

	
	strData_loid = boaGetVar(wp, "fmepon_loid", "");	
	if ( strData_loid[0] )	
	{		
		//printf("===>[%s:%d] fmepon_loid=%s\n",__func__,__LINE__,strData_loid);		
		if(!mib_set(MIB_LOID, strData_loid))		
		{			
			sprintf(tmpBuf, "%s (EPON LOID)",Tset_mib_error);			
			goto setErr;		
		}	
	}

	strData_passwd = boaGetVar(wp, "fmepon_loid_password", "");
       if ( strData_passwd[0] )
        {
               // printf("===>[%s:%d] fmepon_loid_password=%s\n",__func__,__LINE__,strData_passwd);
                if(!mib_set(MIB_LOID_PASSWD, strData_passwd))
                {
						sprintf(tmpBuf, "%s (EPON LOID Password)",Tset_mib_error);			
                        goto setErr;
                }
        }
#ifdef COMMIT_IMMEDIATELY	
	Commit();
#endif	

#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_epon_llidEntryNum_get(&entryNum);
#else
	rtk_epon_llidEntryNum_get(&entryNum);
#endif

	for(index=0;index<entryNum;index++)
	{
		sprintf(oamcli_cmd, "/bin/oamcli set ctc loid %d %s %s", index, strData_loid,strData_passwd);
		//printf("OAMCLI : %s \n" , oamcli_cmd);
		system(oamcli_cmd);
	}


	strData = boaGetVar(wp, "submit-url", "");	
	OK_MSG(strData);	
	
	return;
setErr:	
		ERR_MSG(tmpBuf);
}



void formepon_llid_mac_mapping(request * wp, char *path, char *query)
{
	char *strData;
	char tmpBuf[100];
    int entryNum=2;
	int index=0;	
	char oamcli_cmd[128]={0};
	rtk_epon_llid_entry_t llid_entry;
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_epon_llidEntryNum_get(&entryNum);
#else
	rtk_epon_llidEntryNum_get(&entryNum);
#endif
	if(entryNum<=0)
	{
		sprintf(tmpBuf, "%s (llidEntryNum)",Tget_mib_error);			
		ERR_MSG(tmpBuf);
		return;
	}
	
	for(index=0;index<entryNum;index++)
	{	
		MIB_CE_MIB_EPON_LLID_T mib_llidEntry;
		
		strData	 = boaGetVar_adv(wp, "mac_addr%5B%5D", "",index);

		if(strData[0])
		{	
			unsigned int mac[MAC_ADDR_LEN];
			int i;

			if(strlen(strData)!=17)   //xx:xx:xx:xx:xx:xx
			{
				printf("%s: Incorrect Mac Address %s for index! Please input for xx:xx:xx:xx:xx:xx\n",__func__,strData,index);
				sprintf(tmpBuf, "%s (mac address) %s %s %s xx:xx:xx:xx:xx:xx", strInvalidValue, strData,strFormatError,strShouldBe);
				goto setErr_filter;	
			}
			sscanf(strData,"%x:%x:%x:%x:%x:%x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
			memset(&llid_entry,0,sizeof(llid_entry));
			llid_entry.llidIdx = index;
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_epon_llid_entry_get(&llid_entry);
#else
			rtk_epon_llid_entry_get(&llid_entry);
#endif
			for(i=0;i<MAC_ADDR_LEN;i++)
				llid_entry.mac.octet[i] = (unsigned char) mac[i];
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_epon_llid_entry_set(&llid_entry);
#else
			rtk_epon_llid_entry_set(&llid_entry);
#endif

			sprintf(oamcli_cmd, "/bin/oamcli set config mac %d %s", index, strData);
			//printf("OAMCLI : %s \n" , oamcli_cmd);
			system(oamcli_cmd);
			
			memset(&mib_llidEntry,0,sizeof(mib_llidEntry));
			//save into mib
			if (mib_chain_get(MIB_EPON_LLID_TBL, index, (void *)&mib_llidEntry))
			{
				int modifyFlag=0;
				for(i=0;i<MAC_ADDR_LEN;i++)
				{
					mib_llidEntry.macAddr[i] = (unsigned char)  mac[i];
				}
				mib_chain_update(MIB_EPON_LLID_TBL, (void *) &mib_llidEntry, index);	
			}
			else
			{
				printf("%s: Get index %d entry from EPON LLID MIB Table fail!\n",__func__,index);
				sprintf(tmpBuf, "%s (EPON LLID)",Tget_mib_error);
				goto setErr_filter;	
			}
		}
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	strData = boaGetVar(wp, "submit-url", "");
	OK_MSG(strData);
	return;

setErr_filter:
	ERR_MSG(tmpBuf);
	return;
}
int showepon_LLID_MAC(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
    int entryNum=4;
	char tmpBuf[100];
	int i;
	char mac_str[20]={0};

#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_epon_llidEntryNum_get(&entryNum);
#else
	rtk_epon_llidEntryNum_get(&entryNum);
#endif

	if(entryNum<=0)
	{
 		sprintf(tmpBuf, "%s (llidEntryNum)",strInvalidValue);
		ERR_MSG(tmpBuf);
		return -1;
	}

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"55%%\" bgcolor=\"#808080\">%s</td>\n"
	"</font></tr>\n", multilang(LANG_INDEX), multilang(LANG_LLID),multilang(LANG_STATUS),
	multilang(LANG_MAC));

	for (i=0; i<entryNum; i++) {
		rtk_epon_llid_entry_t llid_entry;

		memset(&llid_entry, 0, sizeof(llid_entry));
		llid_entry.llidIdx = i;
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_llid_entry_get(&llid_entry);	
#else
		rtk_epon_llid_entry_get(&llid_entry);	
#endif
		sprintf(mac_str,"%02x:%02x:%02x:%02x:%02x:%02x",llid_entry.mac.octet[0],llid_entry.mac.octet[1],llid_entry.mac.octet[2],
				llid_entry.mac.octet[3],llid_entry.mac.octet[4],llid_entry.mac.octet[5]);
		
		nBytesSent += boaWrite(wp, "<tr>"
						"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>\n"
						"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>0x%x</b></font></td>\n"
						"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>\n"
						"<td align=center width=\"55%%\" bgcolor=\"#C0C0C0\"><input type=\"text\" name=\"mac_addr[]\" size=\"17\" maxlength=\"17\" value=\"%s\"></td></tr>\n",
						llid_entry.llidIdx, llid_entry.llid, llid_entry.valid, mac_str);
	}
	return 0;
}

