/*
 *      Web server handler routines for VLAN configuration stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

enum _vlan_cfg_type {
    VLAN_AUTO=0, VLAN_MANUAL
};

enum _vlan_manu_mode {
    VLAN_MANU_TRANS=0, VLAN_MANU_TAG, VLAN_MANU_SRV, VLAN_MANU_SP 
};


int fmvlan_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	char tmpBuf[100];
	unsigned char vChar;
	unsigned short sInt;
	
	if (boaArgs(argc, argv, "%s", &name) < 1) 
	{
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	if(!strcmp(name, "vlan_cfg_type_auto")) 	
	{			

		if (!mib_get(MIB_VLAN_CFG_TYPE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_CFG_TYPE);
			goto setErr;
		}
		if (vChar == VLAN_AUTO)
			boaWrite(wp, "checked");
		return 0;
	}

	if(!strcmp(name, "vlan_cfg_type_manual")) 	
	{			

		if (!mib_get(MIB_VLAN_CFG_TYPE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_CFG_TYPE);
			goto setErr;
		}
		if (vChar == VLAN_MANUAL)
			boaWrite(wp, "checked");
		return 0;
	}

	if(!strcmp(name, "vlan_manu_mode_trans")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_MODE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_MODE);
			goto setErr;
		}
		if (vChar == VLAN_MANU_TRANS)
			boaWrite(wp, "checked");
		return 0;
	}

	if(!strcmp(name, "vlan_manu_mode_tag")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_MODE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_MODE);
			goto setErr;
		}
		if (vChar == VLAN_MANU_TAG)
			boaWrite(wp, "checked");
		return 0;
	}
	
	if(!strcmp(name, "vlan_manu_mode_srv")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_MODE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_MODE);
			goto setErr;
		}
		if (vChar == VLAN_MANU_SRV)
			boaWrite(wp, "checked");
		return 0;
	}
	
	if(!strcmp(name, "vlan_manu_mode_sp")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_MODE, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_MODE);
			goto setErr;
		}
		if (vChar == VLAN_MANU_SP)
			boaWrite(wp, "checked");
		return 0;
	}

	if(!strcmp(name, "vlan_manu_tag_vid")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_TAG_VID, (void *)&sInt))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_TAG_VID);
			goto setErr;
		}
		if(sInt!=0xFFFF)
			boaWrite(wp, "%u", sInt);
		return 0;
	}

	if(!strcmp(name, "vlan_manu_tag_pri")) 	
	{			

		if (!mib_get(MIB_VLAN_MANU_TAG_PRI, (void *)&vChar))
		{
			sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_VLAN_MANU_TAG_PRI);
			goto setErr;
		}
		if(vChar==0xFF)
			boaWrite(wp, "-1");
		else
			boaWrite(wp, "%d", vChar);
		return 0;
	}
	
setErr:
	ERR_MSG(tmpBuf);
	return -1;
}

int config_omci_vlancfg()
{
	char cfg_type, manu_mode, manu_pri;
	unsigned short manu_vid;
	char parm[10], parm2[10];
	int status = -1;
	

	if(!mib_get(MIB_VLAN_CFG_TYPE, (void *)&cfg_type))
	{
		printf("MIB_VLAN_CFG_TYPE get failed!\n");
		return status;
	}

	if(cfg_type == VLAN_AUTO)
	{
		//system("/bin/omcicli set iotvlancfg 0 255 65535 255"); 
		// 0, 0xFF, 0xFFFF, 0xFF: don't care manual type, vid, prio
		status = va_cmd("/bin/omcicli", 6, 1, "set", "iotvlancfg", "0", "255", "65535", "255");
		return status;
	}
		
	if(!mib_get(MIB_VLAN_MANU_MODE, (void *)&manu_mode))
	{
		printf("MIB_VLAN_MANU_MODE get failed!\n");
		return status;
	}

	if(manu_mode != VLAN_MANU_TAG)
	{
		snprintf(parm, sizeof(parm), "%d", manu_mode);
		status = va_cmd("/bin/omcicli", 6, 1, "set", "iotvlancfg", "1", parm, "65535", "255");
	}
	else
	{
		if(!mib_get(MIB_VLAN_MANU_TAG_VID, (void *)&manu_vid))
		{
			printf("MIB_VLAN_MANU_TAG_VID get failed!\n");
			return status;
		}
		if(!mib_get(MIB_VLAN_MANU_TAG_PRI, (void *)&manu_pri))
		{
			printf("MIB_VLAN_MANU_TAG_PRI get failed!\n");
			return status;
		}

		snprintf(parm, sizeof(parm), "%d", manu_vid);
		snprintf(parm2, sizeof(parm2), "%d", manu_pri);
		status = va_cmd("/bin/omcicli", 6, 1, "set", "iotvlancfg", "1", "1", parm, parm2);

	}

	return status;

}

void formVlan(request * wp, char *path, char *query)
{
	char	*strData;
	char	vChar;
	unsigned short sInt;
	char tmpBuf[100];
	char omcicli[128];

	strData = boaGetVar(wp, "refresh", "");
	if( strData[0] ){
		system("/bin/omcicli debug detectiotvlan");
		goto setRefresh;
	}

	strData = boaGetVar(wp, "vlan_cfg_type", "");	
	if ( strData[0] )	
	{		
		vChar = atoi(strData);	
		if(!mib_set(MIB_VLAN_CFG_TYPE, (void *)&vChar))		
		{			
			sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_VLAN_CFG_TYPE);
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "vlan_manu_mode", "");	
	if ( strData[0] )	
	{		
		vChar = atoi(strData);	
		if(!mib_set(MIB_VLAN_MANU_MODE, (void *)&vChar))		
		{				
			sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_VLAN_MANU_MODE);
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "vlan_manu_tag_vid", "");	
	if ( strData[0] )	
	{		
		sInt = atoi(strData);	
		if(!mib_set(MIB_VLAN_MANU_TAG_VID, (void *)&sInt))		
		{			
			sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_VLAN_MANU_TAG_VID);
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "vlan_manu_tag_pri", "");	
	if ( strData[0] )	
	{		
		vChar = atoi(strData);
		if(vChar){
			vChar = vChar - 1;
			if(!mib_set(MIB_VLAN_MANU_TAG_PRI, (void *)&vChar))		
			{			
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_VLAN_MANU_TAG_PRI);
				goto setErr;		
			}
		}
	}

	strData = boaGetVar(wp, "submit-url", "");

	OK_MSG(strData);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	config_omci_vlancfg();
	return;

setRefresh:
	strData = boaGetVar(wp, "submit-url", "");
	boaRedirect(wp, strData);
	return;

setErr:
	ERR_MSG(tmpBuf);
}

int omciVlanInfo(int eid, request * wp, int argc, char **argv)
{
	FILE *fp;
	char buff[256];
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "<table border=\"0\">");
		
	fp = fopen("/tmp/omci_vlan_info", "r");
	if (fp) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			nBytesSent += boaWrite(wp, "<tr><td><font size=2>%s</td></tr>\n", buff);
		}
		fclose(fp);
	}

	nBytesSent += boaWrite(wp, "</table>");
	
	return nBytesSent;
}

