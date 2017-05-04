
/*
 *      Web server handler routines for MAC-Based Assignment for DHCP Server stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

void formmacBase(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned long v1, v2;

//star: for take effect immediately
	int change_flag = 0;
	unsigned short dhcpPortFilter;	
	
	// Port-base filter
	str = boaGetVar(wp, "dhcpPortFilter", "");
	if (str[0]) {
		set_dhcp_port_base_filter(0);
		dhcpPortFilter = atoi(str);

		//AUG_PRT("dhcpPortFilter : 0x%x\n", dhcpPortFilter);		
		if ( mib_set(MIB_DHCP_PORT_FILTER, (void *)&dhcpPortFilter) == 0) {
			strcpy(tmpBuf, Tset_mib_error);					
			goto setErr_route;
		}
		set_dhcp_port_base_filter(1);
		goto setOk_route;
	}

	// Delete
	str = boaGetVar(wp, "delIP", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */

		str = boaGetVar(wp, "select", "");
		//printf("str(select) = %s\n", str);

		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					change_flag = 1;
					// delete from chain record
					if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, idx) != 1) {
						strcpy(tmpBuf, strDelChainerror);
						goto setErr_route;
					}


				}
			} // end of for
		}
		goto setOk_route;
	}

	// Add
	str = boaGetVar(wp, "addIP", "");
	//printf("str=%s\n", str);
	if (str[0]) {
		MIB_CE_MAC_BASE_DHCP_T entry,Entry;
		int mibtotal,i, intVal;
		unsigned char macAddr[MAC_ADDR_LEN];

		str = boaGetVar(wp, "hostMac", "");
		// Modified by Mason Yu. 2008/12/26
		//strcpy(entry.macAddr, str);


		for (i=0; i<17; i++){
			if ((i+1)%3 != 0)
				str[i-(i+1)/3] = str[i];
		}
		str[12] = '\0';
		if (strlen(str)!=12  || !string_to_hex(str, entry.macAddr_Dhcp, 12)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_route;
		}
		if (!isValidMacAddr(entry.macAddr_Dhcp)) {
			strcpy(tmpBuf, strInvdMACAddr);
			goto setErr_route;
		}


		str = boaGetVar(wp, "hostIp", "");
		//strcpy(entry.ipAddr, str);

		if (str[0]){
			if ( !inet_aton(str, (struct in_addr *)&entry.ipAddr_Dhcp ) ) {
				strcpy(tmpBuf, strIPAddresserror);
				goto setErr_route;
			}
		}

		//printf("entry.ipAddr = 0x%x\n", ((struct in_addr *)&entry.ipAddr_Dhcp)->s_addr);

		mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
		for(i=0;i<mibtotal;i++)
		{
			mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
			v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
			v2 = *((unsigned long *)entry.ipAddr_Dhcp);

			if ( (	Entry.macAddr_Dhcp[0]==entry.macAddr_Dhcp[0] && Entry.macAddr_Dhcp[1]==entry.macAddr_Dhcp[1] && Entry.macAddr_Dhcp[2]==entry.macAddr_Dhcp[2] &&
	     			Entry.macAddr_Dhcp[3]==entry.macAddr_Dhcp[3] && Entry.macAddr_Dhcp[4]==entry.macAddr_Dhcp[4] && Entry.macAddr_Dhcp[5]==entry.macAddr_Dhcp[5] ) || (v1==v2)  ) {
				strcpy(tmpBuf, strStaticipexist);
				goto setErr_route;
			}

		}

		change_flag = 1;

		intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_route;
		}
	goto setOk_route;
	}

	//Mod
	str = boaGetVar(wp, "modIP", "");
	//printf("str=%s\n", str);
	if (str[0]) {
		unsigned int i, j;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */

		str = boaGetVar(wp, "select", "");
		//printf("str(select) = %s\n", str);

		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					change_flag = 1;
					MIB_CE_MAC_BASE_DHCP_T entry;

					str = boaGetVar(wp, "hostMac", "");
					//strcpy(entry.macAddr, str);
					for (j=0; j<17; j++){
						if ((j+1)%3 != 0)
							str[j-(j+1)/3] = str[j];
					}
					str[12] = '\0';
					if (strlen(str)!=12  || !string_to_hex(str, entry.macAddr_Dhcp, 12)) {
						strcpy(tmpBuf, strInvdMACAddr);
						goto setErr_route;
					}
					if (!isValidMacAddr(entry.macAddr_Dhcp)) {
						strcpy(tmpBuf, strInvdMACAddr);
						goto setErr_route;
					}

					//printf("entry.macAddr = 0x%x\n", entry.macAddr);

					str = boaGetVar(wp, "hostIp", "");
					//strcpy(entry.ipAddr, str);
					inet_aton(str, (struct in_addr *)&entry.ipAddr_Dhcp);
					//printf("entry.ipAddr = 0x%\n", entry.ipAddr);

					if(mib_chain_update(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry, idx) != 1){
						strcpy(tmpBuf, strModChainerror);
						goto setErr_route;
					}


				}
			} // end of for
		}
	}


setOk_route:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif
	if(change_flag == 1)
	{
		restart_dhcp();
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
		return;
	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_route:
	ERR_MSG(tmpBuf);

}

int showMACBaseTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;
	struct in_addr matchIp;
	char macaddr[20];

	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">%s</td></font></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_MAC_ADDRESS), multilang(LANG_ASSIGNED_IP_ADDRESS));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");
			return;
		}

		//matchIp.s_addr = Entry.ipAddr_Dhcp;
		//strcpy(ip,inet_ntoa(matchIp));

		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\"></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		i, macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp))    );
	}

	return 0;
}
