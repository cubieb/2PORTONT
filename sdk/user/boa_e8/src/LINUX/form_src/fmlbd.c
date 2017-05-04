/*-- System inlcude files --*/
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include <linux/config.h>

static int parse_vlans(char *str, MIB_CE_LBD_VLAN_Tp vlans, char *err_buf)
{
	int got = 0;
	int vlan;
	char *start = str, *saveptr, *tok= NULL, *end;
	int i;

	tok = strtok_r(str, " ,", &saveptr);
	while(tok && got < MAX_LBD_VLANS)
	{
		errno = 0;
		vlan = strtol(tok, &end, 10);
		if(vlan < 0 || vlan > 4094 || end[0] != '\0')
		{
			if(err_buf)
				sprintf(err_buf, "%s 不是合法的VLAN值\n", tok);
			return -1;
		}

		for(i = 0 ; i < got ; i++)
		{
			if(vlans[i].vid == vlan)
				break;
		}
		if(i == got)
			vlans[got++].vid = vlan;

		tok = strtok_r(NULL, " ,", &saveptr);
	}
	if(tok)
	{
		if(err_buf)
			strcpy(err_buf, "VLAN值不能超过100个\n");
		return -1;
	}

	return got;
}

void formLBD(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];	
	unsigned char  enable=0;
	unsigned short ether_type;
	unsigned int vUint;
	int i;
	MIB_CE_LBD_VLAN_T vlans[MAX_LBD_VLANS] = {0};

	str = boaGetVar(wp, "enable", "0");
	if(str[0] && str[0] == '1')
		enable = 1;
	else
		enable = 0;
	mib_set(MIB_LBD_ENABLE, &enable);
	if(enable)
	{
		int size;

		str = boaGetVar(wp, "exist_period", "5");
		vUint = atol(str);
		mib_set(MIB_LBD_EXIST_PERIOD, &vUint);
		

		str = boaGetVar(wp, "cancel_period", "180");
		vUint = atol(str);
		mib_set(MIB_LBD_CANCEL_PERIOD, &vUint);

		str = boaGetVar(wp, "ether_type", "fffa");
		ether_type = (unsigned short)strtol(str, NULL, 16);
		mib_set(MIB_LBD_ETHER_TYPE, &ether_type);

		str = boaGetVar(wp, "vlans", "0");
		size = parse_vlans(str, vlans, tmpBuf);
		if( size <= 0)
			goto error;

		mib_chain_clear(MIB_LBD_VLAN_TBL);
		for(i = 0 ; i < size ; i++)
		{
			mib_chain_add(MIB_LBD_VLAN_TBL, &vlans[i]);
		}
	}
	
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	setupLBD();
		
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);

	return;	

error:
	ERR_MSG(tmpBuf);
}

void initLBDPage(int eid, request * wp, int argc, char **argv)
{
	unsigned char enable;
	unsigned int exist_period, cancel_period;
	unsigned short ether_type;
	MIB_CE_LBD_VLAN_T entry = {0};
	unsigned char port_status[ELANVIF_NUM];
	int i, total;
#ifdef CONFIG_EPON_FEATURE
	char cmd[128];
	FILE*fp;
	char line[128];
	unsigned char pon_mode=0;
#endif

	mib_get(MIB_LBD_ENABLE, &enable);
	boaWrite(wp, "var lbd_enable=%d;\n", enable);

	mib_get(MIB_LBD_EXIST_PERIOD, &exist_period);
	boaWrite(wp, "var lbd_exist_period=%d;\n", exist_period);

	mib_get(MIB_LBD_CANCEL_PERIOD, &cancel_period);
	boaWrite(wp, "var lbd_cancel_period=%d;\n", cancel_period);

	mib_get(MIB_LBD_ETHER_TYPE, &ether_type);
	boaWrite(wp, "var lbd_ether_type = 0x%04X;\n", ether_type);

	total = mib_chain_total(MIB_LBD_VLAN_TBL);
	boaWrite(wp, "var lbd_vlans = \"");
	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_LBD_VLAN_TBL, i, &entry);

		if(i != 0)
			boaWrite(wp, ",", entry.vid);

		boaWrite(wp, "%d", entry.vid);
	}
	boaWrite(wp, "\";\n");

#ifdef CONFIG_EPON_FEATURE
	mib_get(MIB_PON_MODE, &pon_mode);
	if(pon_mode == EPON_MODE)
	{
		snprintf(cmd,sizeof(cmd),"oamcli get ctc loopdetect");
		system(cmd);
		if(fp=fopen("/tmp/loopdetect","r"))
		{
			int portnum;
			char loopstatus[5];
			int disabletime;
			while(fgets(line,sizeof(line),fp))
			{
				portnum = line[0]-'0';
				if(portnum>=0 && portnum<ELANVIF_NUM)
				{
					if(sscanf(line,"%*d\t%*s\t\t%*s\t\t%s\t\t%d",loopstatus,&disabletime)==2)
					{
						if(disabletime>0)
							port_status[portnum] = 1;
						else
							port_status[portnum] = 0;
					}
				}
			}
			
			fclose(fp);
		}
	}
	else
#endif
	mib_get(MIB_RS_LBD_PORT_STATUS, port_status);
	boaWrite(wp, "var lbd_port_status = [");
	for(i = 0 ; i < ELANVIF_NUM ; i++)
	{
		if(i != 0)
			boaWrite(wp, ",");
		boaWrite(wp, "%hhu", port_status[i]);
	}
	boaWrite(wp, "];\n");

	return;   
}


