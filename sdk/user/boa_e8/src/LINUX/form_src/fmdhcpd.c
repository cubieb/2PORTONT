/*
 *      Web server handler routines for DHCP Server stuffs
 *      Authors: Kaohj	<kaohj@realtek.com.tw>
 *
 */


/*-- System inlcude files --*/
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
#include <linux/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "fmdefs.h"
#include "mib.h"
#include <rtk/utility.h>

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

///////////////////////////////////////////////////////////////////
#define goto_dhcp_check_err(errorStr) \
    do {lineno = __LINE__; strcpy(tmpBuf, errorStr); goto check_err;} while(0)

int setLanIp(unsigned int uiIp, unsigned int uiMask)
{
	struct in_addr inOrigIp, inOrigMask;
	unsigned char ucLanIpChanged = 0, ucWLanState = 0;

	mib_get(MIB_ADSL_LAN_IP, (void *)&inOrigIp);
	if (inOrigIp.s_addr != uiIp)
	{
		ucLanIpChanged = 1;
		delete_dsldevice_on_hosts();
		mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&uiIp);
		mib_set(MIB_ADSL_LAN_IP, (void *)&uiIp);
		add_dsldevice_on_hosts();
	}

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&inOrigMask);
	if (inOrigMask.s_addr != uiMask)
	{
		ucLanIpChanged = 1;
		mib_set(MIB_ADSL_LAN_SUBNET, (void *)&uiMask);
	}

	if (ucLanIpChanged)
	{
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		restart_lanip();
		//setupIngressSetting();
#ifdef CONFIG_RTK_RG_INIT
		//QL if LAN IP changed, ACL rule should be modified synchronously.
		//I just want to call RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW().
		restart_IPFilter_DMZ_MACFilter();
#endif
	}

	return ucLanIpChanged;
}

int setDhcpMode(unsigned char ucMode, void *pvPara)
{
	unsigned char ucOrigMode, ucDhcpdChanged = 0, ucWLanState = 0;
	struct in_addr inOrigDhcpPoolStart, inOrigDhcpPoolEnd, inOrigRelayServer;
	unsigned int uiOrigLease;
	dhcpd_entry *pstDhcpdEntry;

	mib_get(MIB_DHCP_MODE, (void *)&ucOrigMode);
	if (ucOrigMode != ucMode)
	{
		ucDhcpdChanged = 1;
		mib_set(MIB_DHCP_MODE, (void *)&ucMode);
	}

DBPRINT(2, "ucMode=%d, ucOrigMode=%d\n", ucMode, ucOrigMode);

	switch (ucMode)
	{
		case DHCP_LAN_SERVER:
//			if (DHCP_LAN_SERVER == ucOrigMode)
//			{
				pstDhcpdEntry = (dhcpd_entry *)pvPara;

				mib_get(MIB_DHCP_POOL_START, (void *)&inOrigDhcpPoolStart);

				if (inOrigDhcpPoolStart.s_addr != pstDhcpdEntry->dhcpRangeStart)
				{
					ucDhcpdChanged = 1;
					mib_set(MIB_DHCP_POOL_START, (void *)&pstDhcpdEntry->dhcpRangeStart);
				}

				mib_get(MIB_DHCP_POOL_END, (void *)&inOrigDhcpPoolEnd);
				if (inOrigDhcpPoolEnd.s_addr != pstDhcpdEntry->dhcpRangeEnd)
				{
					ucDhcpdChanged = 1;
					mib_set(MIB_DHCP_POOL_END, (void *)&pstDhcpdEntry->dhcpRangeEnd);
				}

				mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uiOrigLease);
				if (uiOrigLease != pstDhcpdEntry->ulTime)
				{
					ucDhcpdChanged = 1;
					mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&pstDhcpdEntry->ulTime);
				}
//			}

			break;

		case DHCP_LAN_NONE:
			break;

		default: /* DHCP_LAN_RELAY */
			mib_get(MIB_ADSL_WAN_DHCPS, (void*)&inOrigRelayServer);
			if (inOrigRelayServer.s_addr != *(unsigned long *)pvPara)
			{
				ucDhcpdChanged = 1;
				mib_set(MIB_ADSL_WAN_DHCPS, pvPara);
			}

			break;
	}

	if (ucDhcpdChanged)
	{
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		restart_dhcp();
	}

	return ucDhcpdChanged;
}


//add by chenzhuoxin
void formDhcpd(request *wp, char *path, char *query)
{
	struct dhcpd_entry entry;
	unsigned int	uIp = 0;            //	modem ip
	unsigned int	uMask = 0;          //	modem netmask
	unsigned char	uDhcpType = 0;		//	0- 禁用dhcpserver;  1- 启用dhcpserver ; 2-enable dhcprelay
	unsigned int	uServerIp = 0;		//	dhcp relay server ip
	char*			stemp = "";
	int				lineno = __LINE__;
	char            tmpBuf[128];
	void            *pvDhcpdPara = NULL;
	unsigned char   ucDhcpdMode = DHCP_LAN_NONE;

	tmpBuf[0] = 0;

	_TRACE_CALL;

	//modem ip
	_GET_IP(uIp, _NEED);
	if(uIp == 0)
	{
		goto_dhcp_check_err(strWrongIP);
	}

	//netmask
	_GET_IP(uMask, _NEED);
	if(uMask == 0)
	{
		goto_dhcp_check_err(strWrongMask);
	}

	//service type
	_GET_INT(uDhcpType, _NEED);
	if(uDhcpType > 2)
	{
		/* impossible */
		goto_dhcp_check_err(strSetDhcpModeerror);
	}

	if(uDhcpType == 1)	//1- 启用dhcpserver
	{
		_ENTRY_IP(dhcpRangeStart, _NEED);
		if(entry.dhcpRangeStart == 0)
		{
			goto_dhcp_check_err(strInvalidRange);
		}

		_ENTRY_IP(dhcpRangeEnd, _NEED);
		if(entry.dhcpRangeEnd == 0)
		{
			goto_dhcp_check_err(strInvalidRange);
		}

		if (entry.dhcpRangeEnd < entry.dhcpRangeStart)
		{
			goto_dhcp_check_err(strInvalidRange);
		}
#if 0
		_ENTRY_IP(ipMask, _NEED);
		if(entry.ipMask == 0)
  	{
			goto_dhcp_check_err(strWrongMask);
		}
#endif

		_ENTRY_INT(ulTime, _NEED);
		if(entry.ulTime == 0)
		{
			goto_dhcp_check_err(strSetLeaseTimeerror);
		}

		ucDhcpdMode = DHCP_LAN_SERVER;
		pvDhcpdPara = (void *)&entry;
	}
	else if(uDhcpType == 2)	//1- enable dhcprelay
	{
		//	dhcp relay server ip
		_GET_IP(uServerIp, _NEED);
		if(uServerIp == 0)
		{
			goto_dhcp_check_err(strInvalidGatewayerror);
		}

		ucDhcpdMode = DHCP_LAN_RELAY;
		pvDhcpdPara = (void *)&uServerIp;
	}

	/*
	*Add code by Realtek
	*/
	if(setLanIp(uIp, uMask) == 1)
	{
		//log out users to prevent user cannot login in 5 minutes.
		free_from_login_list(wp);
	}
	setDhcpMode(ucDhcpdMode, pvDhcpdPara);
	_COND_REDIRECT;
	_TRACE_LEAVEL;
	return;

check_err:
	_TRACE_LEAVEL;
	ERR_MSG(tmpBuf);
	return;
}

//add by chenzhuoxin
int init_dhcpmain_page(int eid, request * wp, int argc, char **argv)
{
	struct dhcpd_entry	entry;
	unsigned int	uIp;           //modem ip
	unsigned int	uMask;         // modem netmask
	unsigned char	uDhcpType = 1;				//0- 禁用;  1- 启用dhcpserver ; 2-enable dhcprelay
	unsigned int	uServerIp;     //dhcp rerty server ip
	int				lineno = __LINE__;

	_TRACE_CALL;

	/*
	*Add code by Realtek
	*/
	mib_get(MIB_ADSL_LAN_IP, (void *)&uIp);
	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&uMask);

	mib_get(MIB_DHCP_MODE, (void *)&uDhcpType);
	switch (uDhcpType)
	{
		case DHCP_LAN_SERVER:
			uDhcpType = 1;
			break;

		case DHCP_LAN_RELAY:
			uDhcpType = 2;
			break;

		default:
			uDhcpType = 0;
			break;
	}

	memset(&entry, 0, sizeof(dhcpd_entry));
	mib_get(MIB_DHCP_POOL_START, (void *)&entry.dhcpRangeStart);
	mib_get(MIB_DHCP_POOL_END, (void *)&entry.dhcpRangeEnd);
	mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&entry.ulTime);
	entry.ipMask = uMask;

	mib_get(MIB_ADSL_WAN_DHCPS, (void*)&uServerIp);

	_PUT_IP(uIp);
	_PUT_IP(uMask);

	_PUT_INT(uDhcpType);

	_PUT_ENTRY_IP(dhcpRangeStart);
	_PUT_ENTRY_IP(dhcpRangeEnd);
	_PUT_ENTRY_IP(ipMask);
	_PUT_ENTRY_INT(ulTime);

	_PUT_IP(uServerIp);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

//add by chenzhuoxin
void formMacAddrBase(request * wp, char *path, char *query)
{
	MIB_CE_MAC_BASE_DHCP_T entry, entry2;
	char tmpBuf[128];
	char macAddr_Dhcp[20], ipAddr_Dhcp[20], tbl_mac[20], tbl_ip[20];
	int mibtotal, i;
	char *stemp = "";
	int lineno = __LINE__;

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _NEED);

	if (strcmp(stemp, "sv") == 0)	//add
	{
		FETCH_INVALID_OPT(stemp, "macAddr_Dhcp", _NEED);
		strcpy(tbl_mac, stemp);
		convert_mac(stemp);
		memcpy(entry.macAddr_Dhcp, stemp, MAC_ADDR_LEN);

		FETCH_INVALID_OPT(stemp, "ipAddr_Dhcp", _NEED);
		strcpy(tbl_ip, stemp);
		inet_aton(stemp, (struct in_addr *)&entry.ipAddr_Dhcp);

		/************Place your code here, do what you want to do! ************/
		mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
#define MAX_MAC_BASE_DHCP_ENTRY 10
		if (mibtotal >= MAX_MAC_BASE_DHCP_ENTRY) {
			goto_dhcp_check_err(strTableFull);
		}

		for (i = 0; i < mibtotal; i++) {
			mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i,
				      (void *)&entry2);

			snprintf(macAddr_Dhcp, 18,
				 "%02x-%02x-%02x-%02x-%02x-%02x",
				 entry2.macAddr_Dhcp[0], entry2.macAddr_Dhcp[1],
				 entry2.macAddr_Dhcp[2], entry2.macAddr_Dhcp[3],
				 entry2.macAddr_Dhcp[4],
				 entry2.macAddr_Dhcp[5]);

			strcpy(ipAddr_Dhcp,
			       inet_ntoa(*(struct in_addr *)
					 &(entry2.ipAddr_Dhcp)));

			if (!strcmp(tbl_mac, macAddr_Dhcp)
			    || !strcmp(tbl_ip, ipAddr_Dhcp)) {
				goto_dhcp_check_err(strStaticipexist);
			}
		}

		mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char *)&entry);
		/************Place your code here, do what you want to do! ************/
	} else if (strcmp(stemp, "rm") == 0)	//remove
	{
		FETCH_INVALID_OPT(stemp, "macAddr_Dhcp", _NEED);
		strcpy(tbl_mac, stemp);
		FETCH_INVALID_OPT(stemp, "ipAddr_Dhcp", _NEED);
		strcpy(tbl_ip, stemp);

		/************Place your code here, do what you want to do! ************/
		mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
		for (i = 0; i < mibtotal; i++) {
			mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&entry);

			snprintf(macAddr_Dhcp, 18,
				 "%02x-%02x-%02x-%02x-%02x-%02x",
				 entry.macAddr_Dhcp[0], entry.macAddr_Dhcp[1],
				 entry.macAddr_Dhcp[2], entry.macAddr_Dhcp[3],
				 entry.macAddr_Dhcp[4], entry.macAddr_Dhcp[5]);

			strcpy(ipAddr_Dhcp,
			       inet_ntoa(*(struct in_addr *)
					 &(entry.ipAddr_Dhcp)));

			if (!strcmp(tbl_mac, macAddr_Dhcp)
			    || !strcmp(tbl_ip, ipAddr_Dhcp)) {
				break;
			}
		}

		mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, i);
		/************Place your code here, do what you want to do! ************/
	} else {
		goto_dhcp_check_err("");
	}

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	restart_dhcp();

	_COND_REDIRECT;
	_TRACE_LEAVEL;
	return;

check_err:
	_TRACE_LEAVEL;
	ERR_MSG(tmpBuf);
	return;
}

int showMACBaseTable(int eid, request *wp, int argc, char **argv)
{
	MIB_CE_MAC_BASE_DHCP_T	entry;
	int cnt = 0;
	int lineno = __LINE__;
	int i;
	char macAddr_Dhcp[20], ipAddr_Dhcp[20];

	_TRACE_CALL;

	/*test code*/
	/*
	cnt = 1;
	memset(&entry, 0, sizeof(entry));
	strcpy(entry.macAddr, "00-11-22-33-44-55");
	strcpy(entry.ipAddr, "192.168.1.2");
	*/
	/*test code*/

	/************Place your code here, do what you want to do! ************/
	cnt = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	/************Place your code here, do what you want to do! ************/

	for(i = 0; i < cnt; i++)
	{
		/************Place your code here, do what you want to do! ************/
		mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&entry);

		snprintf(macAddr_Dhcp, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
			entry.macAddr_Dhcp[0], entry.macAddr_Dhcp[1],
			entry.macAddr_Dhcp[2], entry.macAddr_Dhcp[3],
			entry.macAddr_Dhcp[4], entry.macAddr_Dhcp[5]);

		boaWrite(wp, "dmacips.push(new it_nr(\"%d\"" _PTS _PTS "));\n", i, _PMEX(macAddr_Dhcp), _PMEIP(ipAddr_Dhcp));
		/************Place your code here, do what you want to do! ************/
		/*printf("mac=%s; ip=%s\n", macAddr_Dhcp, ipAddr_Dhcp);*/
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int dhcpClientList(int eid, request * wp, int argc, char **argv)
{
	struct dhcp_device_info	entry;
	char			uIpAddr[16];
	int				cnt = 0;
	int				lineno = __LINE__;

	_TRACE_CALL;

	/*test code*/
	cnt = 0;
	memset(&entry, 0, sizeof(entry));
	memset(uIpAddr, 0, sizeof(uIpAddr));
	/*test code*/

	/************Place your code here, do what you want to do! ************/
#ifdef EMBED
	int pid;
	struct stat status;
	/* lijian: 20080904 START: Bug id 0003809: incorrect pointer cast, off_t *(64 bits) to unsigned long *(32 bits) */
	unsigned long leaseFileSize;
	char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
	unsigned int ipVal=0;
	FILE *fp;
	int i, entryNum, ret;
	DHCPS_SERVING_POOL_T dhcppoolentry;

	// siganl DHCP server to update lease file
	pid = read_pid(DHCPSERVERPID);
	if ( pid > 0)
		kill(pid, SIGUSR1);

	usleep(1000);

	if ( stat(DHCPD_LEASE, &status) < 0 )
		goto err;

	// read DHCP server lease file
	leaseFileSize = (unsigned long)(status.st_size);
	buf = malloc(leaseFileSize);

	if ( buf == NULL ) goto err;

	fp = fopen(DHCPD_LEASE, "r");

	if ( fp == NULL ) goto err;

	fread(buf, 1, leaseFileSize, fp);
	fclose(fp);
	ptr = buf;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	while (1)
	{
		ret = getOneDhcpClient(&ptr, &leaseFileSize, ipAddr, macAddr, liveTime);
		/* lijian: 20080904 END */

		if (ret < 0)
			break;

		if (ret == 0)
			continue;

		inet_aton(ipAddr, (struct in_addr *)&ipVal);

		for( i = 0; i < entryNum; i++)
		{
			if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
				continue;

			if(ipVal >= *(unsigned int *)dhcppoolentry.startaddr && ipVal <= *(unsigned int *)dhcppoolentry.endaddr)
			{
				strcpy(entry.devname, dhcppoolentry.poolname);
				break;
			}
		}

		strncpy(uIpAddr, ipAddr, 20);
		boaWrite(wp, "clts.push(new it_nr(\"%d\"" _PTS _PTS "));\n", cnt, _PME(devname), _PMEX(uIpAddr));
		cnt++;
	}

err:
	if (cnt == 0)
		boaWrite(wp, "clts.push(new it_nr(\"%d\"" _PTS _PTS "));\n", cnt, _PME(devname), _PMEX(uIpAddr));

	if (buf)
		free(buf);

	return 0;
#else
	return 0;
#endif

#if 0
	/************Place your code here, do what you want to do! ************/

	for(index = 0; index < cnt; index++)
	{
		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/

		boaWrite(wp, "clts.push(new it_nr(\"%d\"" _PTS _PTS "));\n", index, _PME(devname), _PMEIP(uIpAddr));
	}
#endif
check_err:
	_TRACE_LEAVEL;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void formReflashClientTbl(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}

//////////////////////////////////////////////////////////////////////////////
int isDhcpClientExist(char *name)
{
/*
	char tmpBuf[100];
	struct in_addr intaddr;

	if ( getInAddr(name, IP_ADDR, (void *)&intaddr ) ) {
		snprintf(tmpBuf, 100, "%s/%s-%s.pid", _DHCPC_PID_PATH, _DHCPC_PROG_NAME, name);
		if ( getPid(tmpBuf) > 0)
			return 1;
	}
*/
	return 0;
}

//dhcp_device init function
int init_dhcp_device_page(int eid, request * wp, int argc, char **argv)
{
	struct dhcp_client_entry entry;
	int i, entryNum;
	DHCPS_SERVING_POOL_T dhcppoolentry;
	int	lineno = __LINE__;

	_TRACE_CALL;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	memset(&entry, 0, sizeof(dhcp_client_entry));

	for( i = 0; i < entryNum; i++)
	{
		if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
			continue;

		if(dhcppoolentry.poolname)
		{
			if(!strcmp(dhcppoolentry.poolname, "Computer"))
			{
				memcpy(&entry.pcRangeStart, dhcppoolentry.startaddr, IP_ADDR_LEN);
				memcpy(&entry.pcRangeEnd, dhcppoolentry.endaddr, IP_ADDR_LEN);
			}
			else if(!strcmp(dhcppoolentry.poolname, "Camera"))
			{
				memcpy(&entry.cmrRangeStart, dhcppoolentry.startaddr, IP_ADDR_LEN);
				memcpy(&entry.cmrRangeEnd, dhcppoolentry.endaddr, IP_ADDR_LEN);
			}
			else if(!strcmp(dhcppoolentry.poolname, "STB"))
			{
				memcpy(&entry.stbRangeStart, dhcppoolentry.startaddr, IP_ADDR_LEN);
				memcpy(&entry.stbRangeEnd, dhcppoolentry.endaddr, IP_ADDR_LEN);
			}
			else if(!strcmp(dhcppoolentry.poolname, "Phone"))
			{
				memcpy(&entry.phoneRangeStart, dhcppoolentry.startaddr, IP_ADDR_LEN);
				memcpy(&entry.phoneRangeEnd, dhcppoolentry.endaddr, IP_ADDR_LEN);
			}
		}
	}

	_PUT_ENTRY_IP(pcRangeStart);
	_PUT_ENTRY_IP(pcRangeEnd);
	_PUT_ENTRY_IP(cmrRangeStart);
	_PUT_ENTRY_IP(cmrRangeEnd);
	_PUT_ENTRY_IP(stbRangeStart);
	_PUT_ENTRY_IP(stbRangeEnd);
	_PUT_ENTRY_IP(phoneRangeStart);
	_PUT_ENTRY_IP(phoneRangeEnd);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

#ifdef IP_BASED_CLIENT_TYPE
int setClientIpRange(dhcp_client_entry *pstClientIpRange)
{
	int i, entryNum, changed = 0;
	DHCPS_SERVING_POOL_T dhcppoolentry;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	for( i = 0; i < entryNum; i++)
	{
		if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&dhcppoolentry))
			continue;

		if(dhcppoolentry.poolname)
		{
			int ori_val = changed;

			if(!strcmp(dhcppoolentry.poolname, "Computer"))
			{
				if(memcmp(dhcppoolentry.startaddr, &pstClientIpRange->pcRangeStart, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.startaddr, &pstClientIpRange->pcRangeStart, IP_ADDR_LEN);
					changed++;
				}

				if(memcmp(dhcppoolentry.endaddr, &pstClientIpRange->pcRangeEnd, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.endaddr, &pstClientIpRange->pcRangeEnd, IP_ADDR_LEN);
					changed++;
				}
			}
			else if(!strcmp(dhcppoolentry.poolname, "Camera"))
			{
				if(memcmp(dhcppoolentry.startaddr, &pstClientIpRange->cmrRangeStart, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.startaddr, &pstClientIpRange->cmrRangeStart, IP_ADDR_LEN);
					changed++;
				}

				if(memcmp(dhcppoolentry.endaddr, &pstClientIpRange->cmrRangeEnd, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.endaddr, &pstClientIpRange->cmrRangeEnd, IP_ADDR_LEN);
					changed++;
				}
			}
			else if(!strcmp(dhcppoolentry.poolname, "STB"))
			{
				if(memcmp(dhcppoolentry.startaddr, &pstClientIpRange->stbRangeStart, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.startaddr, &pstClientIpRange->stbRangeStart, IP_ADDR_LEN);
					changed++;
				}

				if(memcmp(dhcppoolentry.endaddr, &pstClientIpRange->stbRangeEnd, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.endaddr, &pstClientIpRange->stbRangeEnd, IP_ADDR_LEN);
					changed++;
				}
			}
			else if(!strcmp(dhcppoolentry.poolname, "Phone"))
			{
				if(memcmp(dhcppoolentry.startaddr, &pstClientIpRange->phoneRangeStart, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.startaddr, &pstClientIpRange->phoneRangeStart, IP_ADDR_LEN);
					changed++;
				}

				if(memcmp(dhcppoolentry.endaddr, &pstClientIpRange->phoneRangeEnd, IP_ADDR_LEN) != 0)
				{
					memcpy(dhcppoolentry.endaddr, &pstClientIpRange->phoneRangeEnd, IP_ADDR_LEN);
					changed++;
				}
			}

			if( changed > ori_val )
				mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcppoolentry, i);
		}
	}

	if(changed)
	{
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		restart_dhcp();
		return 1;
	}
	else
		return 0;
}
#endif


//add by chenhzuoxin
void formIpRange(request * wp, char *path, char *query)
{
#ifdef IP_BASED_CLIENT_TYPE
	struct dhcp_client_entry	entry;
	unsigned char	uDhcpType = 0;		//	0- 禁用dhcpserver;  1- 启用dhcpserver ; 2-enable dhcprelay
	unsigned int	uServerIp = 0;		//	dhcp relay server ip
	char*			stemp = "";
	int				lineno = __LINE__;
	char            tmpBuf[128];
	unsigned int    uStart;
	unsigned int    uEnd;

	_TRACE_CALL;

	//pc
	_ENTRY_IP(pcRangeStart, _NEED);
	if(entry.pcRangeStart == 0)
	{
		goto_dhcp_check_err(strInvalidRangepc);
	}

	_ENTRY_IP(pcRangeEnd, _NEED);
	if(entry.pcRangeEnd == 0 || entry.pcRangeEnd < entry.pcRangeStart)
	{
		goto_dhcp_check_err(strInvalidRangepc);
	}

	_ENTRY_IP(cmrRangeStart, _NEED);
	if(entry.cmrRangeStart == 0)
	{
		goto_dhcp_check_err(strInvalidRangecmr);
	}

	_ENTRY_IP(cmrRangeEnd, _NEED);
	if(entry.cmrRangeEnd == 0 || entry.cmrRangeEnd < entry.cmrRangeStart)
	{
		goto_dhcp_check_err(strInvalidRangecmr);
	}

	_ENTRY_IP(stbRangeStart, _NEED);
	if(entry.stbRangeStart == 0)
	{
		goto_dhcp_check_err(strInvalidRangestb);
	}

	_ENTRY_IP(stbRangeEnd, _NEED);
	if(entry.stbRangeEnd == 0 || entry.stbRangeEnd < entry.stbRangeStart)
	{
		goto_dhcp_check_err(strInvalidRangestb);
	}

	_ENTRY_IP(phoneRangeStart, _NEED);
	if(entry.phoneRangeStart == 0)
	{
		goto_dhcp_check_err(strInvalidRangestb);
	}

	_ENTRY_IP(phoneRangeEnd, _NEED);
	if(entry.phoneRangeEnd == 0 || entry.phoneRangeEnd < entry.phoneRangeStart)
	{
		goto_dhcp_check_err(strInvalidRangestb);
	}

	/*
	*Add code by Realtek
	*/
	{
		unsigned int clientIpPool[4][2] = {
			{entry.pcRangeStart, entry.pcRangeEnd},
			{entry.cmrRangeStart, entry.cmrRangeEnd},
			{entry.stbRangeStart, entry.stbRangeEnd},
			{entry.phoneRangeStart, entry.phoneRangeEnd}
		};
		int i, j;

		mib_get(MIB_DHCP_POOL_START, (void *)&uStart);
		mib_get(MIB_DHCP_POOL_END, (void *)&uEnd);

		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 2; j++)
			{
				if ((clientIpPool[i][j] < uStart) || (clientIpPool[i][j] > uEnd))
				{
					goto_dhcp_check_err(strInvalidTypeRange);
				}
			}
		}

		for (i = 0; i < 2; i++)
		{
			for (j = i + 1; j < 3; j++)
			{
				if ((clientIpPool[i][1] >= clientIpPool[j][0]) && (clientIpPool[j][1] >= clientIpPool[i][0]))
				{
					goto_dhcp_check_err(strOverlapRange);
				}
			}
		}

		setClientIpRange(&entry);
	}

	_COND_REDIRECT;
	_TRACE_LEAVEL;
	return;

check_err:
	_TRACE_LEAVEL;
	ERR_MSG(tmpBuf);
	return;
#endif
}

