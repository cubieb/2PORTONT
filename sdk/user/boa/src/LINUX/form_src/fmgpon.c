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
#include "rtk/gpon.h"
#endif


/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

int fmgpon_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	char tmpBuf[100];
	
	if (boaArgs(argc, argv, "%s", &name) < 1) 
	{
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	if(!strcmp(name, "fmgpon_loid")) 	
	{			
		if(!mib_get(MIB_LOID,  (void *)tmpBuf))		
		{	  		
			sprintf(tmpBuf, "%s (GPON LOID)",Tget_mib_error);			
			goto setErr;		
		}		
		boaWrite(wp, "%s", tmpBuf);		
		return 0;	
	}
	
	if(!strcmp(name, "fmgpon_loid_password")) 
	{
		if(!mib_get(MIB_LOID_PASSWD,  (void *)tmpBuf))
		{
			sprintf(tmpBuf, "%s (GPON LOID Password)",Tget_mib_error);			
			goto setErr;
		}

		boaWrite(wp, "%s", tmpBuf);
		return 0;
	}

	if(!strcmp(name, "fmgpon_ploam_password")) 
	{
		if(!mib_get(MIB_GPON_PLOAM_PASSWD,  (void *)tmpBuf))
		{
			sprintf(tmpBuf, "%s (GPON PLOAM Password)",Tget_mib_error);			
			goto setErr;
		}

		boaWrite(wp, "%s", tmpBuf);
		return 0;
	}

	if(!strcmp(name, "fmgpon_sn")) 
	{
		if(!mib_get(MIB_GPON_SN,  (void *)tmpBuf))
		{
			sprintf(tmpBuf, "%s (GPON Serial Number)",Tget_mib_error);			
			goto setErr;
		}
		boaWrite(wp, "%s",tmpBuf);
		return 0;
	}

setErr:
	ERR_MSG(tmpBuf);
	return -1;
}

///////////////////////////////////////////////////////////////////
void formgponConf(request * wp, char *path, char *query)
{
	char	*strData,*strLoid,*strLoidPasswd;
	char tmpBuf[100];
	char omcicli[128];


	strLoid = boaGetVar(wp, "fmgpon_loid", "");	
	if ( strLoid[0] )	
	{		
		//printf("===>[%s:%d] fmgpon_loid=%s\n",__func__,__LINE__,strLoid);		
		if(!mib_set(MIB_LOID, strLoid))		
		{
			sprintf(tmpBuf, " %s (GPON LOID).",Tset_mib_error);			
			goto setErr;		
		}	
	}

	strLoidPasswd = boaGetVar(wp, "fmgpon_loid_password", "");	
	if ( strLoid[0] )	
	{		
		//printf("===>[%s:%d] fmgpon_loid_password=%s\n",__func__,__LINE__,strLoidPasswd);		
		if(!mib_set(MIB_LOID_PASSWD, strLoidPasswd))		
		{			
			sprintf(tmpBuf, " %s (GPON LOID Password).",Tset_mib_error);			
			goto setErr;		
		}	
	}
	
	if(strLoid[0] && strLoidPasswd[0])
	{
		sprintf(omcicli, "/bin/omcicli set loid %s %s", strLoid,strLoidPasswd);
		//printf("OMCICLI : %s \n" , omcicli);
		system(omcicli);
	}


	strData = boaGetVar(wp, "fmgpon_ploam_password", "");
	if ( strData[0] )
	{
		//Password: 10 characters.
		rtk_gpon_password_t gpon_ploam_password;

		//Since OMCI has already active this, so need to deactive
		printf("GPON deActivate.\n");
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_deActivate();
#else
		rtk_gpon_deActivate();
#endif
		memset(&gpon_ploam_password,0, sizeof(gpon_ploam_password));
		memcpy(gpon_ploam_password.password, strData, 10);
#if defined(CONFIG_RTK_L34_ENABLE)		
		rtk_rg_gpon_password_set(&gpon_ploam_password);
#else
		rtk_gpon_password_set(&gpon_ploam_password);
#endif
		if(!mib_set(MIB_GPON_PLOAM_PASSWD, strData))
		{
			sprintf(tmpBuf, " %s (GPON PLOAM Password).",Tset_mib_error);			
			goto setErr;
		}

#if 0
		strData = boaGetVar(wp, "fmgpon_sn", "");
		if ( strData[0] )
		{
			char specific_str[9] = {0};
			unsigned int  specific =0;

			//SN:format AAAABBBBBBBB
			//   AAAA: 4 characters string
			//   BBBBBBBB: 8 hex straing
			rtk_gpon_serialNumber_t SN;			
			memset(&SN, 0 ,sizeof(SN));
			memcpy(SN.vendor,strData,4);
			memcpy(specific_str,&strData[4],8);
	
			specific = (unsigned int) strtoll(specific_str,NULL,16); 
			SN.specific[0] = ( specific >>24 ) &0xFF; 
			SN.specific[1] = ( specific >>16 ) &0xFF; 
			SN.specific[2] = ( specific >>8 ) &0xFF; 
			SN.specific[3] = ( specific ) &0xFF; 
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_serialNumber_set(&SN);
#else
			rtk_gpon_serialNumber_set(&SN);
#endif
			if(!mib_set(MIB_GPON_SN, strData))
			{
				strcpy(tmpBuf, "Save GPON Serial Number Error");
				goto setErr;
			}
			
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif

	
		}
		else
		{
			strcpy(tmpBuf, "Get GPON Serial Number Error");
			goto setErr;
		}
#endif

		//Active GPON again
		printf("GPON Activate again.\n");
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_activate(RTK_GPONMAC_INIT_STATE_O1);
#else
		rtk_gpon_activate(RTK_GPONMAC_INIT_STATE_O1);
#endif
	}
	else
	{
 		sprintf(tmpBuf, "%s (GPON PLOAM Password)",strInvalidValue);
		goto setErr;
	}

#ifdef CONFIG_GPON_FEATURE
	char vChar;
	strData = boaGetVar(wp, "omci_olt_mode", "");
	if ( strData[0] )
	{
		vChar = strData[0] - '0';
		if(checkOMCImib(vChar)){
			if(!mib_set(MIB_OMCI_OLT_MODE, (void *)&vChar))
			{
				strcpy(tmpBuf, "Save OMCI OLT MODE Error");
				goto setErr;
			}
			restartOMCIsettings();
		}
	}
#endif
	
	strData = boaGetVar(wp, "submit-url", "");

	OK_MSG(strData);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	return;

setErr:
	ERR_MSG(tmpBuf);
}
