/*
 *      Web server handler routines for firewall
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "fmdefs.h"

#ifdef PORT_FORWARD_GENERAL
/////////////////////////////////////////////////////////////////////////////
void formPortFw(request * wp, char *path, char *query)
{
	char *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
	char *strIp, *strFrom, *strTo, *strComment;
	char tmpBuf[100];
	int intVal;
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v21, v31, v22, v32;
	unsigned char vChar, portfwcap;
	char ip2Enable;
#ifdef LAN_IP_INTERFACE
	int totalEntry = 0, isSameSubnet = 0;
	MIB_CE_LAN_ITF_T Entry;
	unsigned long v0;
#endif	// of LAN_IP_INTERFACE
#ifndef NO_ACTION
	int pid;
#endif

	// Set port forwarding Capability
	strVal = boaGetVar(wp, "apply", "");
	if (strVal[0]) {
		strVal = boaGetVar(wp, "portFwcap", "");
		if (strVal[0]) {
			if (strVal[0] == '0')
				portfwcap = 0;
			else if(strVal[0] == '1')
				portfwcap = 1;
			if ( !mib_set(MIB_PORT_FW_ENABLE, (void *)&portfwcap)) {
				strcpy(tmpBuf, "Set Port Forwarding Capability error!");
				goto setErr_portfw;
			}
		}
		goto  setOk_portfw;
 	}

	strAddPort = boaGetVar(wp, "addPortFw", "");
	strDelPort = boaGetVar(wp, "deleteSelPortFw", "");
	strDelAllPort = boaGetVar(wp, "deleteAllPortFw", "");

	memset(&entry, '\0', sizeof(entry));

	/* Add new port-forwarding table */
	if (strAddPort[0]) {

#if 0
		strVal = boaGetVar(wp, "enabled", "");
		if ( !gstrcmp(strVal, "ON"))
			vChar = 1;
		else
			vChar = 0;
		if ( mib_set( MIB_PORT_FW_ENABLE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, "Set enabled flag error!");
			goto setErr_portfw;
		}
#endif

		strIp = boaGetVar(wp, "ip", "");
		strFrom = boaGetVar(wp, "fromPort", "");
		strTo = boaGetVar(wp, "toPort", "");
		strComment = boaGetVar(wp, "comment", "");

		if (!strIp[0] && !strFrom[0] && !strTo[0] && !strComment[0])
			goto setOk_portfw;

		if (!strIp[0]) {
			strcpy(tmpBuf, "Error! No ip address to set.");
			goto setErr_portfw;
		}

		memset(  &entry, 0, sizeof(MIB_CE_PORT_FW_T) );
		{
//			char *fw_enable, *remoteIP, *remotePort, *interface;
			char *fw_enable, *remoteIP, *remoteFromPort, *remoteToPort, *interface;
			fw_enable = boaGetVar(wp, "fw_enable", "");
			if ( !gstrcmp(fw_enable, "1"))
			{
				entry.enable = 1;
				//printf( "entry.enable:%d", entry.enable );
			}

			remoteIP = boaGetVar(wp, "remoteIP", "");
			if( remoteIP[0] )
			{
				inet_aton(remoteIP, (struct in_addr *)&entry.remotehost);
				//printf( "remoteIP:%s\n", remoteIP );
			}

/*			remotePort= boaGetVar(wp, "remotePort", "");
			if(remotePort && remotePort[0] )
			{
				if ( !string_to_dec(remotePort, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, "Error! Invalid value of remote-port.");
					goto setErr_portfw;
				}
				entry.externalport = (unsigned short)intVal;
				//printf( "entry.externalport:%d\n",  entry.externalport );
			}
			*/
			remoteFromPort = boaGetVar(wp, "remoteFromPort", "");
			remoteToPort = boaGetVar(wp, "remoteToPort", "");
			if (remoteFromPort && remoteFromPort[0])
			{
				if ( !string_to_dec(remoteFromPort, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, "Error! Invalid value of remote-start-port.");
					goto setErr_portfw;
				}
				entry.externalfromport = (unsigned short)intVal;

				if (!remoteToPort[0])
					entry.externaltoport = entry.externalfromport;
				else {
					if (!string_to_dec(remoteToPort, &intVal) || intVal<1 || intVal>65535) {
						strcpy(tmpBuf, "Error! Invalid value of remote-end-port.");
						goto setErr_portfw;
					}
					entry.externaltoport = (unsigned short)intVal;
				}

				if ( entry.externalfromport  > entry.externaltoport ) {
					strcpy(tmpBuf, "Error! Invalid external port range.");
					goto setErr_portfw;
				}
			}
			//printf( "entry.externalfromport:%d entry.externaltoport=%d\n",  entry.externalfromport, entry.externaltoport);

			interface = boaGetVar(wp, "interface", "");
			if (interface) {
				if ( !string_to_dec(interface, &intVal)) {
					strcpy(tmpBuf, "Error! Invalid interface.");
					goto setErr_portfw;
				}
				entry.ifIndex = intVal;
			}

		}

		inet_aton(strIp, (struct in_addr *)&entry.ipAddr);
#ifdef LAN_IP_INTERFACE
		v0 = *((unsigned long *)entry.ipAddr);
		totalEntry = mib_chain_total(MIB_LAN_IP_INTERFACE_TBL);
		if (!totalEntry) {
			unsigned long v1, v2;
			inet_aton((char*)LAN_LOOPBACK_IP, (struct in_addr *)&curIpAddr);
			inet_aton((char*)LAN_LOOPBACK_MASK, (struct in_addr *)&curSubnet);
			v1 = *((unsigned long *)&curIpAddr);
			v2 = *((unsigned long *)&curSubnet);
			if ((v0 & v2) == (v1 & v2))
				isSameSubnet = 1;
		}
		for (i = 0; i < totalEntry; i ++) {
			unsigned long v1, v2;
			if (!mib_chain_get(MIB_LAN_IP_INTERFACE_TBL, i, (void *)&Entry)) {
				strcpy(tmpBuf, strGetChainerror);
				goto setErr_portfw;
			}
			v1 = *((unsigned long *)&Entry.ipAddr);
			v2 = *((unsigned long *)&Entry.subnetMask);
			if ((v0 & v2) == (v1 & v2)) {
				isSameSubnet = 1;
				break;
			}
		}
		if (!isSameSubnet) {
			strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
			goto setErr_portfw;
		}
#else	// of LAN_IP_INTERFACE
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ipAddr);
		v21 = *((unsigned long *)&curIpAddr);
		v31 = *((unsigned long *)&curSubnet);

#ifdef CONFIG_SECONDARY_IP
		// secondary IP
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2Enable);
		mib_get( MIB_ADSL_LAN_IP2,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET2,  (void *)&curSubnet);
		v22 = *((unsigned long *)&curIpAddr);
		v32 = *((unsigned long *)&curSubnet);
		if (ip2Enable) {
			if ( (v1 & v31) != (v21 & v31) && (v1 & v32) != (v22 & v32)) {
				strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
				goto setErr_portfw;
			}
		}
		else {
			if ( (v1 & v31) != (v21 & v31) ) {
				strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
				goto setErr_portfw;
			}
		}
#else
		if ( (v1 & v31) != (v21 & v31) ) {
			strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
			goto setErr_portfw;
		}
#endif
#endif	// of LAN_IP_INTERFACE

/*		if ( !strTo[0] ) {
			strcpy(tmpBuf, "Error! No port value to be set.");
			goto setErr_portfw;
		}

		if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, "Error! Invalid value of to-port.");
			goto setErr_portfw;
		}


		entry.toPort = (unsigned short)intVal;
		entry.fromPort = entry.toPort;

		if ( entry.fromPort  > entry.toPort ) {
			strcpy(tmpBuf, "Error! Invalid port range.");
			goto setErr_portfw;
		}
		*/
        if (strFrom && strFrom[0] && strTo && strTo[0])
        {
            if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535)
            {
                strcpy(tmpBuf, "Error! Invalid value of local start port.");
                goto setErr_portfw;
            }

            entry.fromPort = (unsigned short)intVal;

            if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535)
            {
                strcpy(tmpBuf, "Error! Invalid value of local end port.");
                goto setErr_portfw;
            }

            entry.toPort = (unsigned short)intVal;

            if (entry.fromPort  > entry.toPort)
            {
                strcpy(tmpBuf, "Error! Invalid local port range.");
                goto setErr_portfw;
            }
        }
        else if ((strFrom && strFrom[0]) && (!strTo || !strTo[0]))
        {
            strcpy(tmpBuf, "Error! Empty local end port.");
            goto setErr_portfw;
        }
        else if ((strTo && strTo[0]) && (!strFrom || !strFrom[0]))
        {
            strcpy(tmpBuf, "Error! Empty local start port.");
            goto setErr_portfw;
        }
        else
        {
            /* Local port is not set */
        }

		//printf( "entry.fromPort:%d entry.toPort=%d\n",  entry.fromPort, entry.toPort);

		strVal = boaGetVar(wp, "protocol", "");
		if (strVal[0]) {
			if ( strVal[0] == '4' )
				entry.protoType = PROTO_UDPTCP;
			else if ( strVal[0] == '1' )
				entry.protoType = PROTO_TCP;
			else if ( strVal[0] == '2' )
				entry.protoType = PROTO_UDP;
			else {
				strcpy(tmpBuf, "Error! Invalid protocol type.");
				goto setErr_portfw;
			}
		}
		else {
			strcpy(tmpBuf, "Error! Protocol type cannot be empty.");
			goto setErr_portfw;
		}

		if ( strComment[0] ) {
			if (strlen(strComment) > COMMENT_LEN-1) {
				strcpy(tmpBuf, "Error! Comment length too long.");
				goto setErr_portfw;
			}
			strcpy(entry.comment, strComment);
		}


		//if updating old portfw entry
		strVal = boaGetVar(wp, "select_id", "");
		if( strVal[0] )
		{
			unsigned int fw_id=0;
			MIB_CE_PORT_FW_T up_entry;

			sscanf( strVal, "select%u", &fw_id );
			//printf( "\n %s %d\n", strVal, fw_id );

			if (!mib_chain_get(MIB_PORT_FW_TBL, fw_id, (void *)&up_entry))
			{
				strcpy(tmpBuf, "Get mib chain error");
				goto setErr_portfw;
			}

			//old setting, not showing at web pages
			entry.leaseduration = up_entry.leaseduration;
			if( entry.ifIndex==up_entry.ifIndex )
				entry.InstanceNum = up_entry.InstanceNum;
			else
				entry.InstanceNum = 0;
			mib_chain_update( MIB_PORT_FW_TBL, (void*)&entry, fw_id );
		}
		else

		{
			#if 0
			entryNum = mib_chain_total(MIB_PORT_FW_TBL);
			// Check if there is any port overlapped
			for (i=0; i<entryNum; i++) {
				MIB_CE_PORT_FW_T CheckEntry;

				if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&CheckEntry)) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_portfw;
				}

				if(!(((entry.fromPort < CheckEntry.fromPort) && (entry.toPort < CheckEntry.fromPort)) ||
					((entry.fromPort > CheckEntry.toPort) && (entry.toPort > CheckEntry.toPort))) &&
				       (entry.protoType & CheckEntry.protoType) ) {
					strcpy(tmpBuf, "Setting port range has overlapped with used port numbers!");
					goto setErr_portfw;
				}
			}
			#endif
			intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)&entry);
			if (intVal == 0) {
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_portfw;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_portfw;
			}
		}

	}

	/* Delete entry */
	if (strDelPort[0])
	{
		int i, totalEntry = mib_chain_total(MIB_PORT_FW_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i = totalEntry - 1; i >= 0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");

			if (!gstrcmp(strVal, "ON")) {
				if (mib_chain_get(MIB_PORT_FW_TBL, i, &entry) != 1) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_portfw;
				}
				/* created by UPnP */
				if (entry.dynamic)
					continue;
				deleted++;
				if(mib_chain_delete(MIB_PORT_FW_TBL, i) != 1) {
					strcpy(tmpBuf, "Delete chain record error!");
					goto setErr_portfw;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, "There is no item selected to delete!");
			goto setErr_portfw;
		}

		goto setOk_portfw;
	}

	/* Delete all entry */
	if ( strDelAllPort[0])
	{
		int i, totalEntry = mib_chain_total(MIB_PORT_FW_TBL); /* get chain record size */

		for (i = totalEntry - 1; i >= 0; i--) {
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, &entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_portfw;
			}
			/* created by UPnP */
			if (entry.dynamic)
				continue;
			if (!mib_chain_delete(MIB_PORT_FW_TBL, i)) {
				strcpy(tmpBuf, "Delete chain record error!");
				goto setErr_portfw;
			}
		}

		goto setOk_portfw;
	}

setOk_portfw:

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#if defined(APPLY_CHANGE)
	// Take effect in real time
	setupPortFW();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
 	return;

setErr_portfw:
	ERR_MSG(tmpBuf);
}
#endif // of PORT_FORWARD_GENERAL

#ifdef NATIP_FORWARDING
void formIPFw(request * wp, char *path, char *query)
{
	char *submitUrl, *strAddEnt, *strDelEnt, *strVal, *strDelAllEnt;
	char *strLocal, *strRemote;
	char tmpBuf[100];
	int intVal;
	unsigned int entryNum, i;
	MIB_CE_IP_FW_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3, v4;
	unsigned char vChar;

	strAddEnt = boaGetVar(wp, "addEntry", "");
	strDelEnt = boaGetVar(wp, "delSelEntry", "");
	strDelAllEnt = boaGetVar(wp, "delAllEntry", "");

	memset(&entry, '\0', sizeof(entry));

	/* Add entry */
	if (strAddEnt[0]) {
		strVal = boaGetVar(wp, "enabled", "");
		if ( !gstrcmp(strVal, "ON"))
			vChar = 1;
		else
			vChar = 0;

		mib_set( MIB_IP_FW_ENABLE, (void *)&vChar);

		strLocal = boaGetVar(wp, "l_ip", "");
		strRemote = boaGetVar(wp, "r_ip", "");

		if (!strLocal[0] || !strRemote[0])
			goto setOk_ipfw;

		inet_aton(strLocal, (struct in_addr *)&entry.local_ip);
		inet_aton(strRemote, (struct in_addr *)&entry.remote_ip);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

		v1 = *((unsigned long *)entry.local_ip);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
			goto setErr_ipfw;
		}

		entryNum = mib_chain_total(MIB_IP_FW_TBL);

		// Check if there is any address conflict
		for (i=0; i<entryNum; i++) {
			MIB_CE_IP_FW_T CheckEntry;

			if (!mib_chain_get(MIB_IP_FW_TBL, i, (void *)&CheckEntry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_ipfw;
			}
			v1 = *((unsigned long *)entry.local_ip);
			v2 = *((unsigned long *)entry.remote_ip);
			v3 = *((unsigned long *)CheckEntry.local_ip);
			v4 = *((unsigned long *)CheckEntry.remote_ip);

			if (v1==v3 || v2==v4) {
				strcpy(tmpBuf, "Address conflict!");
				goto setErr_ipfw;
			}
		}

		intVal = mib_chain_add(MIB_IP_FW_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_ipfw;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_ipfw;
		}
	}

	/* Delete entry */
	if (strDelEnt[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_IP_FW_TBL);

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				if(mib_chain_delete(MIB_IP_FW_TBL, idx) != 1) {
					strcpy(tmpBuf, "Delete chain record error!");
					goto setErr_ipfw;
				}
			}
		}

		goto setOk_ipfw;
	}

	/* Delete all entry */
	if ( strDelAllEnt[0]) {
		mib_chain_clear(MIB_IP_FW_TBL);
		goto setOk_ipfw;
	}

setOk_ipfw:
//	mib_update(CURRENT_SETTING);
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_ipfw:
	ERR_MSG(tmpBuf);
}
#endif	// of NATIP_FORWARDING

#ifdef PORT_TRIGGERING
/////////////////////////////////////////////////////////////////////////////
#define PARSE_START	0
#define PARSE_DIGIT	1
#define PARSE_COMMA	2
#define PARSE_DASH	3
#define PARSE_BLANK	4
#define PARSE_NULL	5
#define PARSE_OTHER	6
#define PARSE_SECOND_DIGIT	7

/*
 *	Parse the trigger port range
 *	The trigger port format should follow these rules:
 *	. Each single port number should be seperated by comma or space
 *	. The port range should be something like: start-end (eg. 1234-1240)
 *		Note that there must not be any space around the dash
 *	. Example: "345,234,2345-2350,567"
 */
int parse_triggerPort(const char *inRange, char *outRange)
{
	int j, k;
	int cur_state, pre_state;

	k=0;
	pre_state = PARSE_START;

	for(j=0;j<GAMING_MAX_RANGE;j++)
	{
		if ((inRange[j]>='0')&&(inRange[j]<='9'))
			cur_state = PARSE_DIGIT;
		else if (inRange[j]==',')
			cur_state = PARSE_COMMA;
		else if (inRange[j]=='-')
			cur_state = PARSE_DASH;
		else if (inRange[j]==' ')
			cur_state = PARSE_BLANK;
		else if (inRange[j]==0)
			cur_state = PARSE_NULL;
		else
			cur_state = PARSE_OTHER;

		switch (cur_state) {
			case PARSE_BLANK:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = ',';
					pre_state = PARSE_COMMA;
				}
				break;
			case PARSE_DIGIT:
				outRange[k++] = inRange[j];
				pre_state = PARSE_DIGIT;
				break;
			case PARSE_COMMA:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = inRange[j];
					pre_state = PARSE_COMMA;
				}
				break;
			case PARSE_DASH:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = inRange[j];
					pre_state = PARSE_DASH;
				}
				break;
			case PARSE_NULL:
				if (pre_state != PARSE_DIGIT && pre_state != PARSE_START)
					outRange[k-1] = 0;
				else
					outRange[k] = 0;
				return 1;
			default: break;
		}
	}
	return 0;
}

extern int gm_postIndex;

void formGaming(request * wp, char *path, char *query)
{
	char *strVal;
	char *strIp, *strTCP, *strUDP;
	char *submitUrl;
	char tmpBuf[100];
	int intVal;
	unsigned int entryNum, i;
	MIB_CE_PORT_TRG_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3;

	strVal = boaGetVar(wp, "add", "");
	memset(&entry, 0, sizeof(entry));

	// Add
	if (strVal[0]) {
		//name
		strVal = boaGetVar(wp, "game", "");
		entryNum = mib_chain_total(MIB_PORT_TRG_TBL);

		// Check for name conflict
		for (i=0; i<entryNum; i++) {
			MIB_CE_PORT_TRG_T CheckEntry;

			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&CheckEntry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_portgm;
			}

			if (!strncmp(strVal, CheckEntry.name, 32)) {
				strcpy(tmpBuf, "Game name conflict!");
				goto setErr_portgm;
			}
		}

		strncpy(entry.name, strVal, 32);

		//ip
		strIp = boaGetVar(wp, "ip", "");
		inet_aton(strIp, (struct in_addr *)&entry.ip);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ip);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
			goto setErr_portgm;
		}

		//tcp open port
		strVal = boaGetVar(wp, "tcpopen", "");
		if (!parse_triggerPort(strVal, entry.tcpRange)) {
			strcpy(tmpBuf, "Invalid TCP range !");
			goto setErr_portgm;
		}

		//udp open port
		strVal = boaGetVar(wp, "udpopen", "");
		if (!parse_triggerPort(strVal, entry.udpRange)) {
			strcpy(tmpBuf, "Invalid UDP range !");
			goto setErr_portgm;
		}

		//enable
		entry.enable = atoi(boaGetVar(wp,"open",""));

		intVal = mib_chain_add(MIB_PORT_TRG_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_portgm;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_portgm;
		}
	}

	// Modify
	strVal = boaGetVar(wp, "modify", "");
	if (strVal[0]) {
		MIB_CE_PORT_TRG_T CheckEntry;
		int found=0;

		//name
		strVal = boaGetVar(wp, "game", "");
		entryNum = mib_chain_total(MIB_PORT_TRG_TBL);

		// Find the entry
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&CheckEntry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_portgm;
			}

			if (!strncmp(strVal, CheckEntry.name, 32)) {
				found = 1;
				break;
			}
		}

		if (!found)
			goto trg_submit;

		//name
		strncpy(entry.name, strVal, 32);

		//ip
		strIp = boaGetVar(wp, "ip", "");
		inet_aton(strIp, (struct in_addr *)&entry.ip);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ip);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
			goto setErr_portgm;
		}

		//tcp open port
		strVal = boaGetVar(wp, "tcpopen", "");
		if (!parse_triggerPort(strVal, entry.tcpRange)) {
			strcpy(tmpBuf, "Invalid TCP range !");
			goto setErr_portgm;
		}

		//udp open port
		strVal = boaGetVar(wp, "udpopen", "");
		if (!parse_triggerPort(strVal, entry.udpRange)) {
			strcpy(tmpBuf, "Invalid UDP range !");
			goto setErr_portgm;
		}

		//enable
		entry.enable = atoi(boaGetVar(wp,"open",""));
		memcpy((void *)&CheckEntry, (void *)&entry, sizeof(entry));
		mib_chain_update(MIB_PORT_TRG_TBL, (void *)&CheckEntry, i);
	}

	strVal = boaGetVar(wp, "action", "");
	if (strVal[0]) {
		int action, index;

		action = strVal[0]-'0';
		strVal = boaGetVar(wp, "idx", "");
		index = strVal[0]-'0';

		if (action == 0) { // delete
			mib_chain_delete(MIB_PORT_TRG_TBL, index);
		}
		else if (action == 1) { // post for editing
			gm_postIndex = index;
		}
	}

trg_submit:
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_portgm:
	ERR_MSG(tmpBuf);
}
#endif

//int checkRule_ipfilter(MIB_CE_IP_PORT_FILTER_T filterEntry)
int checkRule_ipfilter(MIB_CE_IP_PORT_FILTER_T filterEntry, unsigned char *sip, unsigned char *dip)
{
	int totalEntry, i;
	MIB_CE_IP_PORT_FILTER_T Entry;
	char *temp;
	long nSip, nDip;

	totalEntry = mib_chain_total(MIB_IP_PORT_FILTER_TBL);

	temp = inet_ntoa(*((struct in_addr *)sip));
	nSip = ntohl(inet_addr(temp));
	temp = inet_ntoa(*((struct in_addr *)dip));
	nDip = ntohl(inet_addr(temp));
	// If all parameters of Entry are all the same as new rule, drop this new rule.
	if (nSip == nDip && nSip != 0x0) {
		return 0;
	}
	/*if ( filterEntry.srcIp[0]==filterEntry.dstIp[0] && filterEntry.srcIp[1]==filterEntry.dstIp[1] &&
	     filterEntry.srcIp[2]==filterEntry.dstIp[2] && filterEntry.srcIp[3]==filterEntry.dstIp[3]   ) {
		return 0;
	}*/

	for (i=0; i<totalEntry; i++) {
		unsigned long v1, v2, pSip, pDip;
		int m;
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&Entry)) {
			//strcpy(tmpBuf, strGetChainerror);
			return 0;
		}
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.smaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pSip = v1&v2;
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.dmaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pDip = v1&v2;

		// If all parameters of Entry are all the same as mew rule, drop this new rule.
		if (nSip == pSip && nDip == pDip &&
			Entry.srcPortFrom == filterEntry.srcPortFrom && Entry.srcPortTo == filterEntry.srcPortTo &&
			Entry.dstPortFrom == filterEntry.dstPortFrom && Entry.dstPortTo == filterEntry.dstPortTo &&
			Entry.protoType == filterEntry.protoType && Entry.dir == filterEntry.dir) {
				//printf("Error! Conflict IP/port filter rule(0)!\n");
				return 0;
		}
		/*if ( 	Entry.srcIp[0]==filterEntry.srcIp[0] && Entry.srcIp[1]==filterEntry.srcIp[1] &&
		     	Entry.srcIp[2]==filterEntry.srcIp[2] && Entry.srcIp[3]==filterEntry.srcIp[3] &&
		     	Entry.dstIp[0]==filterEntry.dstIp[0] && Entry.dstIp[1]==filterEntry.dstIp[1] &&
		     	Entry.dstIp[2]==filterEntry.dstIp[2] && Entry.dstIp[3]==filterEntry.dstIp[3] &&
			Entry.smaskbit == filterEntry.smaskbit && Entry.dmaskbit == filterEntry.dmaskbit &&
			Entry.srcPortFrom == filterEntry.srcPortFrom && Entry.srcPortTo == filterEntry.srcPortTo &&
			Entry.dstPortFrom == filterEntry.dstPortFrom && Entry.dstPortTo == filterEntry.dstPortTo &&
			Entry.protoType == filterEntry.protoType && Entry.dir == filterEntry.dir) {
				//printf("Error! Conflict IP/port filter rule(0)!\n");
				return 0;
		}*/

		/*
		if ( 	Entry.srcIp[0]==filterEntry.srcIp[0] && Entry.srcIp[1]==filterEntry.srcIp[1] &&
		     	Entry.srcIp[2]==filterEntry.srcIp[2] && Entry.srcIp[3]==filterEntry.srcIp[3] &&
		     	Entry.dstIp[0]==filterEntry.dstIp[0] && Entry.dstIp[1]==filterEntry.dstIp[1] &&
		     	Entry.dstIp[2]==filterEntry.dstIp[2] && Entry.dstIp[3]==filterEntry.dstIp[3] &&
			Entry.smaskbit == filterEntry.smaskbit && Entry.dmaskbit == filterEntry.dmaskbit &&
			Entry.protoType == filterEntry.protoType && Entry.dir == filterEntry.dir ) {
			//Entry.action == filterEntry.action) {
				if ( (Entry.srcPortFrom < filterEntry.srcPortFrom && Entry.srcPortTo > filterEntry.srcPortFrom) ||
				     (Entry.srcPortFrom < filterEntry.srcPortTo && Entry.srcPortTo > filterEntry.srcPortTo) ||
				     (Entry.dstPortFrom < filterEntry.dstPortFrom && Entry.dstPortTo > filterEntry.dstPortFrom) ||
				     (Entry.dstPortFrom < filterEntry.dstPortTo && Entry.dstPortTo > filterEntry.dstPortTo)       ) {
					printf("Error! Conflict IP/port filter rule(1)!\n");
					return 0;
				}
		}
		*/

	}
	return 1;

}

int checkRule_macfilter(MIB_CE_MAC_FILTER_T macEntry)
{
	int total, i;
	MIB_CE_MAC_FILTER_T MacEntry;

	if ( macEntry.srcMac[0]==macEntry.dstMac[0] && macEntry.srcMac[1]==macEntry.dstMac[1] && macEntry.srcMac[2]==macEntry.dstMac[2] &&
	     macEntry.srcMac[3]==macEntry.dstMac[3] && macEntry.srcMac[4]==macEntry.dstMac[4] && macEntry.srcMac[5]==macEntry.dstMac[5]   ) {
		return 0;
	}

	total = mib_chain_total(MIB_MAC_FILTER_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&MacEntry))
			return 0;

		if ( MacEntry.srcMac[0]==macEntry.srcMac[0] && MacEntry.srcMac[1]==macEntry.srcMac[1] && MacEntry.srcMac[2]==macEntry.srcMac[2] &&
		     MacEntry.srcMac[3]==macEntry.srcMac[3] && MacEntry.srcMac[4]==macEntry.srcMac[4] && MacEntry.srcMac[5]==macEntry.srcMac[5] &&
		     MacEntry.dstMac[0]==macEntry.dstMac[0] && MacEntry.dstMac[1]==macEntry.dstMac[1] && MacEntry.dstMac[2]==macEntry.dstMac[2] &&
		     MacEntry.dstMac[3]==macEntry.dstMac[3] && MacEntry.dstMac[4]==macEntry.dstMac[4] && MacEntry.dstMac[5]==macEntry.dstMac[5] &&
		     MacEntry.dir== macEntry.dir	) {
		     	//printf("This ia a duplicate MacFilter Rule\n");
		     	return 0;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
void formFilter(request * wp, char *path, char *query)
{
	char *strSetDefaultAction;
	char *strAddIpPort, *strDelIpPort;
	char *strDelAllIpPort, *strVal, *submitUrl, *strComment;
#ifdef MAC_FILTER
	char *strSetMacDefault;
	char *strAddMac, *strDelMac, *strDelAllMac;
#endif
	int mibTblId;
	unsigned char vChar;
	char tmpBuf[100];
	unsigned int totalEntry;

	memset(tmpBuf,0x00,100);

	strSetDefaultAction = boaGetVar(wp, "setDefaultAction", "");
	strAddIpPort = boaGetVar(wp, "addFilterIpPort", "");
	strDelIpPort = boaGetVar(wp, "deleteSelFilterIpPort", "");
	strDelAllIpPort = boaGetVar(wp, "deleteAllFilterIpPort", "");

#ifdef MAC_FILTER
	strSetMacDefault = boaGetVar(wp, "setMacDft", "");
	strAddMac = boaGetVar(wp, "addFilterMac", "");
	strDelMac = boaGetVar(wp, "deleteSelFilterMac", "");
	strDelAllMac = boaGetVar(wp, "deleteAllFilterMac", "");
#endif

	if (strAddIpPort[0] || strDelIpPort[0] || strDelAllIpPort[0]) {	// IP/Port FILTER
		mibTblId = MIB_IP_PORT_FILTER_TBL;
	}
#ifdef MAC_FILTER
	else  {	// MAC FILTER
		mibTblId = MIB_MAC_FILTER_TBL;
	}
#endif


	totalEntry = mib_chain_total(mibTblId); /* get chain record size */

	if(strDelAllIpPort[0]
#ifdef MAC_FILTER
	 || strDelAllMac[0]
#endif
	)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
		goto setOk_filter;
	}

	if(strDelIpPort[0]
#ifdef MAC_FILTER
	 ||  strDelMac[0]
#endif
	)
	{
		unsigned int i;
		unsigned int idx;
		unsigned int deleted = 0;
		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				deleted ++;
				if(mib_chain_delete(mibTblId, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_filter;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, "There is no item selected to delete!");
			goto setErr_filter;
		}
		goto setOk_filter;
	}

	// IP filtering default action
	if (strSetDefaultAction[0])
	{
		strVal = boaGetVar(wp, "outAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_IPF_OUT_ACTION, (void *)&vChar);
		}

		strVal = boaGetVar(wp, "inAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_IPF_IN_ACTION, (void *)&vChar);
		}
		goto setOk_filter;
	}

#ifdef MAC_FILTER
	// Mac filtering default action
	if (strSetMacDefault[0])
	{
		strVal = boaGetVar(wp, "outAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_MACF_OUT_ACTION, (void *)&vChar);
			vChar = !vChar;
			mib_set( MIB_ETH_MAC_CTRL, (void *)&vChar);
			mib_set( MIB_WLAN_MAC_CTRL, (void *)&vChar);

		}

		strVal = boaGetVar(wp, "inAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_MACF_IN_ACTION, (void *)&vChar);
		}
		goto setOk_filter;
	}
#endif

	if (totalEntry >= MAX_FILTER_NUM)
	{
		strcpy(tmpBuf, Texceed_max_rules);
		goto setErr_filter;
	}

	if (strAddIpPort[0] ) {		// IP/Port FILTER
		MIB_CE_IP_PORT_FILTER_T filterEntry;
		unsigned long mask, mbit;
		//unsigned char noIP=1;
		char *strFrom, *strTo;
		int intVal;
		unsigned int totalEntry;
		MIB_CE_IP_PORT_FILTER_T Entry;
		int i;
		unsigned char smask[4], dmask[4], sip[4], dip[4];

		memset(&filterEntry, 0x00, sizeof(filterEntry));
		memset(sip, 0x0, 4);
		memset(dip, 0x0, 4);
		//sip[0] = dip[0] = '\0';

		// protocol
		strVal = boaGetVar(wp, "protocol", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, Tprotocol_empty);
			goto setErr_filter;
		}

		filterEntry.protoType = strVal[0] - '0';

		// source ip/port
		strFrom = boaGetVar(wp, "sfromPort", "");
		strTo = boaGetVar(wp, "stoPort", "");
		strVal = boaGetVar(wp, "sip", "");

		// Modified by Mason Yu
		if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP){
			strFrom = 0;
		}

		if (strVal[0]) {
			//unsigned long v1, v2, v3;
			//struct in_addr curIpAddr, curSubnet;

			//noIP=0;
			//char  srcip[20], dstip[20], *filterSIP;

			if (!inet_aton(strVal, (struct in_addr *)&filterEntry.srcIp)) {
				strcpy(tmpBuf, Tinvalid_source_ip);
				goto setErr_filter;
			}

			strVal = boaGetVar(wp, "smask", "");

			if (strVal[0]) {
				if (!isValidNetmask(strVal, 1)) {
					strcpy(tmpBuf, Tinvalid_source_netmask);
					goto setErr_filter;
				}
				inet_aton(strVal, (struct in_addr *)smask);
				inet_aton(strVal, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, Tinvalid_source_netmask);
					goto setErr_filter;
				}

				mbit=0;

				while (1) {
					if (mask&0x80000000) {
						mbit++;
						mask <<= 1;
					}
					else
						break;
				}

				filterEntry.smaskbit = mbit;
			}
			else {
				filterEntry.smaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)smask);
			}
			// Jenny, for checking duplicated source address
			sip[0] = filterEntry.srcIp[0] & smask[0];
			sip[1] = filterEntry.srcIp[1] & smask[1];
			sip[2] = filterEntry.srcIp[2] & smask[2];
			sip[3] = filterEntry.srcIp[3] & smask[3];

			/*
			mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
			mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

			v1 = *((unsigned long *)filterEntry.srcIp);
			v2 = *((unsigned long *)&curIpAddr);
			v3 = *((unsigned long *)&curSubnet);

			if ( (v1 & v3) != (v2 & v3) ) {
				strcpy(tmpBuf, "Invalid IP address! It should be set within the current subnet.");
				goto setErr_filter;
			}
			*/
		}

		/*
		if ((!strFrom[0]) && (noIP)) { // if port-forwarding, from port must exist
			strcpy(tmpBuf, "Error! No from-port value to be set.");
			goto setErr_filter;
		}
		*/

		// Modified by Mason Yu
		if(strFrom!= NULL && strFrom[0])
		{
			int intVal;
			if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, Tinvalid_source_port);
				goto setErr_filter;
			}
			filterEntry.srcPortFrom = (unsigned short)intVal;


			if ( !strTo[0] )
				filterEntry.srcPortTo = filterEntry.srcPortFrom;
			else {
				if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_source_port);
					goto setErr_filter;
				}
				filterEntry.srcPortTo = (unsigned short)intVal;
			}

			if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo ) {
				strcpy(tmpBuf, Tinvalid_port_range);
				goto setErr_filter;
			}

		}

		// destination ip/port
		strFrom = boaGetVar(wp, "dfromPort", "");
		strTo = boaGetVar(wp, "dtoPort", "");
		strVal = boaGetVar(wp, "dip", "");

		// Modified by Mason Yu
		if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP){
			strFrom = 0;
		}

		if (strVal[0]) {

			if (!inet_aton(strVal, (struct in_addr *)&filterEntry.dstIp)) {
				strcpy(tmpBuf, Tinvalid_destination_ip);
				goto setErr_filter;
			}

			strVal = boaGetVar(wp, "dmask", "");

			if (strVal[0]) {
				if (!isValidNetmask(strVal, 1)) {
					strcpy(tmpBuf, Tinvalid_destination_netmask);
					goto setErr_filter;
				}
				inet_aton(strVal, (struct in_addr *)dmask);
				inet_aton(strVal, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, Tinvalid_destination_netmask);
					goto setErr_filter;
				}

				mbit=0;

				while (1) {
					if (mask&0x80000000) {
						mbit++;
						mask <<= 1;
					}
					else
						break;
				}

				filterEntry.dmaskbit = mbit;
			}
			else {
				filterEntry.dmaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)dmask);
			}
			// Jenny, for checking duplicated destination address
			dip[0] = filterEntry.dstIp[0] & dmask[0];
			dip[1] = filterEntry.dstIp[1] & dmask[1];
			dip[2] = filterEntry.dstIp[2] & dmask[2];
			dip[3] = filterEntry.dstIp[3] & dmask[3];
		}

		// Modified by Mason Yu
		if(strFrom!= NULL && strFrom[0])
		{
			int intVal;
			if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, Tinvalid_destination_port);
				goto setErr_filter;
			}
			filterEntry.dstPortFrom = (unsigned short)intVal;


			if ( !strTo[0] )
				filterEntry.dstPortTo = filterEntry.dstPortFrom;
			else {
				if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_destination_port);
					goto setErr_filter;
				}
				filterEntry.dstPortTo = (unsigned short)intVal;
			}

			if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo ) {
				strcpy(tmpBuf, Tinvalid_port_range);
				goto setErr_filter;
			}

		}

		strVal = boaGetVar(wp, "filterMode", "");
		if ( strVal[0] ) {
			if (!strcmp(strVal, "Deny"))
				filterEntry.action = 0;
			else if (!strcmp(strVal, "Allow"))
				filterEntry.action = 1;
			else {
				strcpy(tmpBuf, Tinvalid_rule_action);
				goto setErr_filter;
			}
		}

		strVal = boaGetVar(wp, "dir", "");
		filterEntry.dir = strVal[0]-'0';

		// Mason Yu.
		//if ( !checkRule_ipfilter(filterEntry)) {
		if (!checkRule_ipfilter(filterEntry, sip, dip)) {	// Jenny
			strcpy(tmpBuf, Tinvalid_rule);
			goto setErr_filter;
		}

		intVal = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}

	} else {			// MAC FILTER
#ifdef MAC_FILTER
		MIB_CE_MAC_FILTER_T macEntry;
		char *strsmac, *strdmac;
		int i, intVal;
		unsigned int totalEntry;
		MIB_CE_MAC_FILTER_T Entry;

		memset(&macEntry, 0x00, sizeof(macEntry));

		strVal = boaGetVar(wp, "filterMode", "");
		if ( strVal[0] ) {
			if (!strcmp(strVal, "Deny"))
				macEntry.action = 0;
			else if (!strcmp(strVal, "Allow"))
				macEntry.action = 1;
			else {
				strcpy(tmpBuf, Tinvalid_rule_action);
				goto setErr_filter;
			}
		}

		strsmac = boaGetVar(wp, "srcmac", "");
		strdmac = boaGetVar(wp, "dstmac", "");
		if (!strsmac[0] && !strdmac[0])
		//if (!strsmac[0])
			goto setOk_filter;

		// source MAC
		//if (strsmac[0] && (strlen(strsmac)!=12 || !string_to_hex(strsmac, macEntry.srcMac, 12))) {
		if (strsmac[0]) {
			if ( !string_to_hex(strsmac, macEntry.srcMac, 12)) {
				strcpy(tmpBuf, Tinvalid_source_mac);
				goto setErr_filter;
			}
			if (!isValidMacAddr(macEntry.srcMac)) {
				strcpy(tmpBuf, Tinvalid_source_mac);
				goto setErr_filter;
			}
		}

		// destination MAC
		//if (strdmac[0] && (strlen(strdmac)!=12 || !string_to_hex(strdmac, macEntry.dstMac, 12))) {
		if (strdmac[0]) {
			if ( !string_to_hex(strdmac, macEntry.dstMac, 12)) {
				strcpy(tmpBuf, Tinvalid_dest_mac);
				goto setErr_filter;
			}
			if (!isValidMacAddr(macEntry.dstMac)) {
				strcpy(tmpBuf, Tinvalid_dest_mac);
				goto setErr_filter;
			}
		}

		// Added by Mason Yu for Incoming MAC filtering
		strVal = boaGetVar(wp, "dir", "");
		macEntry.dir = strVal[0]-'0';

		// Mason Yu. Check if the rule is duplicate or conficting ?
		if (!checkRule_macfilter(macEntry)) {
			strcpy(tmpBuf, Tinvalid_rule);
			goto setErr_filter;
		}

		intVal = mib_chain_add(MIB_MAC_FILTER_TBL, (unsigned char*)&macEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}
#endif // of MAC_FILTER
	}


setOk_filter:
//	Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	// Mason Yu. Take effect in real time
#if 0
	if ( strSetDefaultAction[0] || (mibTblId == MIB_IP_PORT_FILTER_TBL) )  // ipfilter
	{
		restart_IPFilter_DMZ();
	} else {  // Macfilter
		setupMacFilter();
	}
#else
	// Mason Yu. Take effect in real time; // Magician: Merge restart MAC filter into restart IPFilter and DMZ
	restart_IPFilter_DMZ_MACFilter();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  return;

setErr_filter:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
void formDMZ(request * wp, char *path, char *query)
{
	char *submitUrl, *strSave, *strVal;
	char tmpBuf[100];
	char vChar;
	struct in_addr ipAddr, curIpAddr, curSubnet, secondIpAddr, secondSubnet;
	struct in_addr dmzIp;
	unsigned long uInt;
	unsigned long v1, v2, v3, v4, v5;
	unsigned char ucPreStat;
	int entryNum;
	char s_entryNum[8];
	char enable;
#ifndef NO_ACTION
	int pid;
#endif

	strSave = boaGetVar(wp, "save", "");

	if (strSave[0]) {
#if 0
		strVal = boaGetVar(wp, "enabled", "");
		if ( !gstrcmp(strVal, "ON"))
			vChar = 1;
		else
			vChar = 0;

		mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
		if ( mib_set(MIB_DMZ_ENABLE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, "Set enabled flag error!");
			goto setErr_dmz;
		}
#endif
		strVal = boaGetVar(wp, "dmzcap", "");
		vChar = 0;
		if (strVal[0]) {
			if (strVal[0] == '0')
				vChar = 0;
			else if(strVal[0] == '1')
				vChar = 1;
		}

		if (vChar) {
			strVal = boaGetVar(wp, "ip", "");
			if (!strVal[0]) {
				goto setOk_dmz;
			}
			inet_aton(strVal, &ipAddr);
			mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
			mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
			mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&enable );
			mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
			mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);
#endif

			v1 = *((unsigned long *)&ipAddr);
			v2 = *((unsigned long *)&curIpAddr);
			v3 = *((unsigned long *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
			v4 = *((unsigned long *)&secondIpAddr);
			v5 = *((unsigned long *)&secondSubnet);
#endif

			if (v1) {
				if (v1 == v2) {
					strcpy(tmpBuf, "不合法的DMZ IP地址!"); //Invalid DMZ IP address!
					goto setErr_dmz;
				}
				if ( (((v1 & v3) != (v2 & v3))
#ifdef CONFIG_SECONDARY_IP
				&& !enable) ||
				( enable && ((v1 & v5) != (v4 & v5)) && ((v1 & v3) != (v2 & v3))
#endif
				) ) {
					strcpy(tmpBuf, Tdmz_error);
					goto setErr_dmz;
				}
			}

			mib_get(MIB_DMZ_IP, (void *)&dmzIp);
			uInt = *((unsigned long *)&dmzIp);

			if ( mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, "DMZ MIB 设定错误!"); //Set DMZ MIB error!
				goto setErr_dmz;
			}
		} // of if (vChar)
		mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
		if ( !mib_set(MIB_DMZ_ENABLE, (void *)&vChar)) {
			strcpy(tmpBuf, "DMZ Capability 设定错误!"); //Set DMZ Capability error!
			goto setErr_dmz;
		}
	}

setOk_dmz:
#if defined(APPLY_CHANGE)
	setupDMZ();
#endif

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
	waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	//OK_MSG(submitUrl);
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;

setErr_dmz:
	//ERR_MSG(tmpBuf);
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG1(tmpBuf, submitUrl);
}


/////////////////////////////////////////////////////////////////////////////
int portFwList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_PORT_FW_T Entry;
	char	*type, portRange[20], *ip, localIP[20];
	char	remoteIP[20], remotePort[20], *fw_enable;
//	char extPort[8];
	char extFromPort[8], extToPort[8];
	int interface_id=0;
	char interface_name[IFNAMSIZ];


	entryNum = mib_chain_total(MIB_PORT_FW_TBL);

	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local Port</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Enable</b></font></td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Remote Host</b></font></td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Public Port</b></font></td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Interface</b></font></td></tr>\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, errGetEntry);
			return -1;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
		strcpy( localIP, ip );
		if ( !strcmp(localIP, "0.0.0.0"))
			strcpy( localIP, "----" );

		if ( Entry.protoType == PROTO_UDPTCP )
			type = "TCP+UDP";
		else if ( Entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		if ( Entry.fromPort == 0)
			strcpy(portRange, "----");
		else if ( Entry.fromPort == Entry.toPort )
			snprintf(portRange, 20, "%d", Entry.fromPort);
		else
			snprintf(portRange, 20, "%d-%d", Entry.fromPort, Entry.toPort);


		ip = inet_ntoa(*((struct in_addr *)Entry.remotehost));
		strcpy( remoteIP, ip );
		if ( !strcmp(remoteIP, "0.0.0.0"))
			strcpy( remoteIP, "" );

/*		if ( Entry.externalport == 0)
			strcpy(remotePort, "----");
		else
			snprintf(remotePort, 20, "%d", Entry.externalport);
		*/
		if ( Entry.externalfromport == 0)
			strcpy(remotePort, "----");
		else if ( Entry.externalfromport == Entry.externaltoport )
			snprintf(remotePort, 20, "%d", Entry.externalfromport);
		else
			snprintf(remotePort, 20, "%d-%d", Entry.externalfromport, Entry.externaltoport);

		if ( Entry.enable == 0 )
			fw_enable = "Disable";
		else
			fw_enable = "Enable";

		if ( Entry.ifIndex == DUMMY_IFINDEX )
		{
			strcpy( interface_name, "---" );
		}else {			
			ifGetName(Entry.ifIndex, interface_name, sizeof(interface_name));
		}

//		snprintf(extPort, sizeof(extPort), "%u", Entry.externalport);
		snprintf(extFromPort, sizeof(extFromPort), "%u", Entry.externalfromport);
		snprintf(extToPort, sizeof(extToPort), "%u", Entry.externaltoport);

		nBytesSent += boaWrite(wp, "<tr>"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
				i, localIP, type, portRange, Entry.comment,
				fw_enable, remoteIP, remotePort, interface_name);
/*      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\" "

//			"onClick=\"postFW( '%s',%d,%d,%d,'%s',%d,'%s','%s',%d,'select%d' )\""
			"onClick=\"postFW( '%s',%d,%d,%d,'%s',%d,'%s','%s','%s',%d,'select%d' )\""
      			"></td></tr>\n"),
				localIP, type, portRange, Entry.comment,
				fw_enable, remoteIP, remotePort, interface_name,
				i,localIP, Entry.fromPort, Entry.toPort, Entry.protoType,
//				Entry.comment, Entry.enable, remoteIP,Entry.externalport ? extPort : "",Entry.ifIndex, i
				Entry.comment, Entry.enable, remoteIP,Entry.externalfromport ? extFromPort : "",Entry.externaltoport ? extToPort : "",Entry.ifIndex, i
				);
*/
	}

	return nBytesSent;
}

#ifdef NATIP_FORWARDING
/////////////////////////////////////////////////////////////////////////////
int ipFwList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_IP_FW_T Entry;
	char	local[16], external[16];

	entryNum = mib_chain_total(MIB_IP_FW_TBL);

	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>External IP Address</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_FW_TBL, i, (void *)&Entry)) {
  			boaError(wp, 400, errGetEntry);
			return -1;
		}

		strncpy(local, inet_ntoa(*((struct in_addr *)Entry.local_ip)), 16);
		strncpy(external, inet_ntoa(*((struct in_addr *)Entry.remote_ip)), 16);
		nBytesSent += boaWrite(wp, ("<tr>"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",
			local, external, i);
	}

	return nBytesSent;
}
#endif	// of NATIP_FORWARDING

/////////////////////////////////////////////////////////////////////////////
int ipPortFilterList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_IP_PORT_FILTER_T Entry;
	char *dir, *ract;
	char	*type, *ip;
	char	ipaddr[20], portRange[20];

	entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Direction</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src Address</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src Port</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst Address</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst Port</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Rule Action</b></font></td></tr>\n");
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.dir == DIR_OUT)
			dir = Toutgoing_ippfilter;
		else
			dir = Tincoming_ippfilter;

		// Modified by Mason Yu for Block ICMP packet
		if ( Entry.protoType == PROTO_ICMP )
		{
			type = (char *)ARG_ICMP;
		}
		else if ( Entry.protoType == PROTO_TCP )
			type = (char *)ARG_TCP;
		else
			type = (char *)ARG_UDP;

		ip = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(ipaddr, 20, "%s", ip);
			else
				snprintf(ipaddr, 20, "%s/%d", ip, Entry.smaskbit);
			ip = ipaddr;
		}

		if ( Entry.srcPortFrom == 0)
			strcpy(portRange, BLANK);
		else if ( Entry.srcPortFrom == Entry.srcPortTo )
			snprintf(portRange, 20, "%d", Entry.srcPortFrom);
		else
			snprintf(portRange, 20, "%d-%d", Entry.srcPortFrom, Entry.srcPortTo);
		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
      			i, dir, type, ip, portRange);
		ip = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(ipaddr, 20, "%s", ip);
			else
				snprintf(ipaddr, 20, "%s/%d", ip, Entry.dmaskbit);
			ip = ipaddr;
		}

		if ( Entry.dstPortFrom == 0)
			strcpy(portRange, BLANK);
		else if ( Entry.dstPortFrom == Entry.dstPortTo )
			snprintf(portRange, 20, "%d", Entry.dstPortFrom);
		else
			snprintf(portRange, 20, "%d-%d", Entry.dstPortFrom, Entry.dstPortTo);

		if ( Entry.action == 0 )
			ract = Tdeny_ippfilter;
		else
			ract = Tallow_ippfilter;
		nBytesSent += boaWrite(wp, 
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
//      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				ip, portRange, ract);
	}
	return nBytesSent;
}


#ifdef MAC_FILTER
/////////////////////////////////////////////////////////////////////////////
int macFilterList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_MAC_FILTER_T Entry;
	char *ract, *dir;
	char tmpBuf[100], tmpBuf2[100];

	entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Direction</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src MAC Address</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst MAC Address</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Rule Action</b></font></td></tr>\n");
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.dir == DIR_OUT)
			dir = Toutgoing_ippfilter;
		else
			dir = Tincoming_ippfilter;

		if ( Entry.action == 0 )
			ract = Tdeny_ippfilter;
		else
			ract = Tallow_ippfilter;


		if (   Entry.srcMac[0]==0 && Entry.srcMac[1]==0
		    && Entry.srcMac[2]==0 && Entry.srcMac[3]==0
		    && Entry.srcMac[4]==0 && Entry.srcMac[5]==0 ) {
			strcpy(tmpBuf, "------");
		}else {
			snprintf(tmpBuf, 100, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.srcMac[0], Entry.srcMac[1], Entry.srcMac[2],
				Entry.srcMac[3], Entry.srcMac[4], Entry.srcMac[5]);
		}


		if (   Entry.dstMac[0]==0 && Entry.dstMac[1]==0
		    && Entry.dstMac[2]==0 && Entry.dstMac[3]==0
		    && Entry.dstMac[4]==0 && Entry.dstMac[5]==0 ) {
			strcpy(tmpBuf2, "------");
		}else {
			snprintf(tmpBuf2, 100, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.dstMac[0], Entry.dstMac[1], Entry.dstMac[2],
				Entry.dstMac[3], Entry.dstMac[4], Entry.dstMac[5]);
		}

		nBytesSent += boaWrite(wp, "<tr>"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
			i, dir, tmpBuf, tmpBuf2, ract);
	}
	return nBytesSent;
}
#endif // of MAC_FILTER

#if 0 //def _CWMP_MIB_
int PortFwTR069_En(request * wp)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "\n"
			"\tenableTextField(document.formPortFwAdd.fw_enable);\n"
			"\tenableTextField(document.formPortFwAdd.remoteIP);\n"
			"\tenableTextField(document.formPortFwAdd.remotePort);\n"
			"\tenableTextField(document.formPortFwAdd.interface);\n"
			"\n" );

	return nBytesSent;
}
int PortFwTR069_Dis(request * wp)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "\n"
			"\tdisableTextField(document.formPortFwAdd.fw_enable);\n"
			"\tdisableTextField(document.formPortFwAdd.remoteIP);\n"
			"\tdisableTextField(document.formPortFwAdd.remotePort);\n"
			"\tdisableTextField(document.formPortFwAdd.interface);\n"
			"\n" );

	return nBytesSent;
}
int PortFwTR069_PostFW(request * wp)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "\n"
			"function postFW( localIP, localport_from, localport_to, protocol, comment, enable, remoteIP, remotePort, interface, select )\n"
			"{\n"
			"	if ( document.formPortFwAdd.enabled.checked==true )\n"
			"	{\n"
			"		document.formPortFwAdd.ip.value=localIP;\n"
			"		document.formPortFwAdd.toPort.value=localport_to;\n"
			"		document.formPortFwAdd.protocol.value=protocol;\n"
			"		document.formPortFwAdd.comment.value=comment;\n"
			"		if( enable==0 )\n"
			"			document.formPortFwAdd.fw_enable.checked=false;\n"
			"		else\n"
			"			document.formPortFwAdd.fw_enable.checked=true;\n"
			"\n"
			"		document.formPortFwAdd.remoteIP.value=remoteIP;\n"
			"		document.formPortFwAdd.remotePort.value=remotePort;\n"
			"		document.formPortFwAdd.interface.value=interface;\n"
			"		document.formPortFwAdd.select_id.value=select;\n"
			"\n"
			"	}\n"
			"}\n"
			"\n" );

	return nBytesSent;
}
#if 0
int PortFwTR069_AddFileds(request * wp)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "\n"
			"    <font size=2><b>&nbsp;&nbsp;<input type=\"checkbox\" name=\"fw_enable\" value=\"ON\">&nbsp;&nbsp;Enable</b>\n"
			"    <p><b>Remote IP Address:</b> <input type=\"text\" name=\"remoteIP\" size=\"10\" maxlength=\"15\">&nbsp;\n"
			"       <b>Port:</b> <input type=\"text\" name=\"remotePort\" size=\"3\">\n"
			"    <p><b>Interface:</b> <input type=\"text\" name=\"interface\" size=\"10\"></font>\n"
			"    <input type=\"hidden\" value=\"\" name=\"select_id\">\n"
			"\n" );

	return nBytesSent;
}
#endif
int portFwTR069(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	char *name;

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ( !strcmp(name, "EnTR069Fileds") )
		return PortFwTR069_En( wp );
	if ( !strcmp(name, "DisTR069Fileds") )
		return PortFwTR069_Dis( wp );
	if ( !strcmp(name, "FunctionPostFW") )
		return PortFwTR069_PostFW( wp );
#if 0
	if ( !strcmp(name, "AddTR069Fileds") )
		return PortFwTR069_AddFileds( wp );
#endif

	return nBytesSent;
}
#else
int portFwTR069(int eid, request * wp, int argc, char **argv)
{
	return 0;
}
#endif

#ifdef LAYER7_FILTER_SUPPORT
int AppFilterList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i,j=0;
	LAYER7_FILTER_T Entry;
	unsigned char appname[2*MAX_APP_NAME];

	entryNum = mib_chain_total(MIB_LAYER7_FILTER_TBL);

	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>应用软件</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>选择</b></font></td></tr>\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_LAYER7_FILTER_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "读取MIB_LAYER7_FILTER_TBL错误!\n");
			return -1;
		}

		snprintf(appname, 2*MAX_APP_NAME, "%s", Entry.appname);
		getappname(0,appname,1);

		if(!strcmp(appname,"unknown"))
			continue;

		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
      			appname);

		nBytesSent += boaWrite(wp, 
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\" value=\"s%d\"></td></tr>\n",
			i);

		j++;
	}
	nBytesSent += boaWrite(wp, "<input type=\"hidden\" value=%d name=\"listnumber\">", j);

	return nBytesSent;
}
void formLayer7(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl, *tmpVal;
	char tmpBuf[100];
	char *strAddApp,*strDelApp,*strDelAllApp;
	int totalEntry;

	memset(tmpBuf,0x00,100);

	strAddApp = boaGetVar(wp, "addLayer7", "");
	strDelApp = boaGetVar(wp, "deleteSelFilterApp", "");
	strDelAllApp = boaGetVar(wp, "deleteAllFilterApp", "");

	totalEntry = mib_chain_total(MIB_LAYER7_FILTER_TBL);

	if(strDelAllApp[0])
	{
		mib_chain_clear(MIB_LAYER7_FILTER_TBL); /* clear chain record */
		goto setOk_filter;
	}

	if(strDelApp[0])
	{
		unsigned int i;
		unsigned int idx;

		strVal=boaGetVar(wp, "select", "");
		if(strVal[0])
		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "s%d", idx);
			if ( !gstrcmp(strVal, tmpBuf) ) {
				if(mib_chain_delete(MIB_LAYER7_FILTER_TBL, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_filter;
				}
			}
		}
		goto setOk_filter;
	}

	if (strAddApp[0] ) {
		LAYER7_FILTER_T Entry;
		LAYER7_FILTER_T tmpEntry;
		int appnum;
		int i, intVal;

		memset(&Entry, 0x00, sizeof(Entry));

		strVal = boaGetVar(wp, "appname", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, "应用软件名称不能为空!");
			goto setErr_filter;
		}

		if(strlen(strVal)==1)
			appnum = (strVal[0] - '0')-1;
		else
			appnum = (strVal[0] - '0')*10+(strVal[1] - '0')-1;

		if(appnum < 0)
		{
			strcpy(tmpBuf, "应用软件名称不能为空!");
			goto setErr_filter;
		}

		getappname(appnum, Entry.appname, 0);

		for(i=0;i<totalEntry;i++)
		{
			if (!mib_chain_get(MIB_LAYER7_FILTER_TBL, i, (void *)&tmpEntry))
			{
  				boaError(wp, 400, "读取MIB_LAYER7_FILTER_TBL错误!\n");
				continue;
			}
			if(!strcmp(tmpEntry.appname,Entry.appname))
			{
				strcpy(tmpBuf, "该软件已经添加到列表中!");
				goto setErr_filter;
			}
		}

		intVal = mib_chain_add(MIB_LAYER7_FILTER_TBL, (unsigned char*)&Entry);
		if (intVal == 0) {
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}
	}

setOk_filter:

	setupAppFilter();

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_filter:
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG1(tmpBuf, submitUrl);

}
#endif

#ifdef PARENTAL_CTRL
extern MIB_PARENT_CTRL_T parentctrltable[MAX_PARENTCTRL_USER_NUM];
/*
	from submit function, refer to parental-ctrl.asp
	//1. update the change to flash
	//2. iptables create/del
*/
void formParentCtrl(request * wp, char *path, char *query)
{
	char *mac, *submitUrl, *tmpVal;
	char *strAddApp,*strDelApp,*strDelAllApp;
	MIB_PARENT_CTRL_T	pct_entry;
	int			i,totalEntry,idx;
	char		tmpBuf[200];



	/* add case, add to flash and update global table*/
	tmpVal= boaGetVar(wp, "usrname", "");
	if (strlen(tmpVal) > 0)
	{
		int intVal;
		memset(&pct_entry, 0x0, sizeof(MIB_PARENT_CTRL_T));
		strcpy(pct_entry.username, tmpVal);
		tmpVal= boaGetVar(wp, "mac", "");
		string_to_hex(tmpVal, pct_entry.mac, 12);
		tmpVal= boaGetVar(wp, "mond", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= MONDAY;
		tmpVal= boaGetVar(wp, "tued", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= TUESDAY;
		tmpVal= boaGetVar(wp, "wend", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= WEDNESSDAY;
		tmpVal= boaGetVar(wp, "thrd", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= THURSDAY;
		tmpVal= boaGetVar(wp, "frid", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= FRIDAY;
		tmpVal= boaGetVar(wp, "satd", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= SATURDAY;
		tmpVal= boaGetVar(wp, "sund", "");
		if (strlen(tmpVal) > 0)
			pct_entry.controlled_day |= SUNDAY;
		if (pct_entry.controlled_day == 0)
		{
			strcpy(tmpBuf, "At lease one day should be selected.");
				goto setErr_parental_ctrl;
		}
		if (mib_chain_total(MIB_PARENTAL_CTRL_TBL) >= MAX_PARENTCTRL_USER_NUM)
		{
			strcpy(tmpBuf, "Max Number of entries is 16, please delete some.");
				goto setErr_parental_ctrl;
		}

		tmpVal= boaGetVar(wp, "starthr", "");
		pct_entry.start_hr = atoi(tmpVal);
		tmpVal= boaGetVar(wp, "startmin", "");
		pct_entry.start_min = atoi(tmpVal);
		tmpVal= boaGetVar(wp, "endhr", "");
		pct_entry.end_hr= atoi(tmpVal);
		tmpVal= boaGetVar(wp, "endmin", "");
		pct_entry.end_min= atoi(tmpVal);

		parent_ctrl_table_add(&pct_entry);
		//save to flash
		intVal = mib_chain_add(MIB_PARENTAL_CTRL_TBL, (unsigned char*)&pct_entry);
		if (intVal == 0)
		{
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_parental_ctrl;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_parental_ctrl;
		}

	}

	// delete all case
	tmpVal= boaGetVar(wp, "deleteAllFilterMac", "");
	if (tmpVal[0])
	{
		parent_ctrl_table_delall();
		mib_chain_clear(MIB_PARENTAL_CTRL_TBL); /* clear all chain record */

	}

	// delete selected case
	tmpVal= boaGetVar(wp, "deleteSelFilterMac", "");
	if (tmpVal[0])
	{
		totalEntry = mib_chain_total(MIB_PARENTAL_CTRL_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++)
		{
			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			tmpVal = boaGetVar(wp, tmpBuf, "");
			if ( !gstrcmp(tmpVal, "ON") )
			{
				mib_chain_get(MIB_PARENTAL_CTRL_TBL, idx, &pct_entry);
				parent_ctrl_table_del(&pct_entry);
				if(mib_chain_delete(MIB_PARENTAL_CTRL_TBL, idx) != 1)
				{
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_parental_ctrl;
				}

			}
		}

	}

setOK_parental_ctrl:

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_parental_ctrl:
	ERR_MSG(tmpBuf);

}
/*
	For display of parental ctrl table
*/
int parentalCtrlList(int eid, request * wp, int argc, char **argv)
{
int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_PARENT_CTRL_T *Entry,the_entry;
	char *ract, *dir;
	const char td_front[] ="<td align=center  bgcolor=\"#C0C0C0\"> <font size=\"2\">";
	const char td_back[] ="</td>";
	char tmpbuf2[100];


	//entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
	nBytesSent += boaWrite(wp, "\r\n<tr>"
	"<td align=center width=\"12%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Name</b></font></td>\n"
      	"\r\n<td align=center width=\"18%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Mon</b></font></td>\n"
      	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Tue</b></font></td>\n"
	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Wed</b></font></td>\n"
	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Thu</b></font></td>\n"
	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Fri</b></font></td>\n");

	nBytesSent += boaWrite(wp,
	"<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Sat</b></font></td>\n"
	"\r\n<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Sun</b></font></td>\n"
	"\r\n<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Start</b></font></td>\n"
	"\r\n<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=\"2\"><b>End</b></font></td>\n"
      	"\r\n<td align=center width=\"7%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n");


	for  (i = 0; i<  mib_chain_total(MIB_PARENTAL_CTRL_TBL); i++)
	{

		mib_chain_get(MIB_PARENTAL_CTRL_TBL, i, &the_entry);
		Entry =&the_entry ;


		snprintf(tmpbuf2, 100, "%02x-%02x-%02x-%02x-%02x-%02x",
					Entry->mac[0], Entry->mac[1], Entry->mac[2],
					Entry->mac[3], Entry->mac[4], Entry->mac[5]);

		boaWrite(wp, "\r\n\r\n<tr>""%s%s%s",td_front,Entry->username,td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,tmpbuf2,td_back);

		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&MONDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&TUESDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&WEDNESSDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&THURSDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&FRIDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&SATURDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%s%s",td_front,(Entry->controlled_day&SUNDAY)?"v":"",td_back);
		boaWrite(wp, "\r\n%s%02d:%02d%s",td_front,Entry->start_hr,Entry->start_min,td_back);
		boaWrite(wp, "\r\n%s%02d:%02d%s",td_front,Entry->end_hr,Entry->end_min,td_back);

		boaWrite(wp, "\r\n%s<input type=\"checkbox\" name=\"select%d\" value=\"ON\">%s</tr>",td_front,i,td_back);
	}
	return nBytesSent;

}

#endif

// Mason Yu
#ifdef PORT_FORWARD_ADVANCE
void formPFWAdvance(request * wp, char *path, char *query)
{
	char	*str, *strVal, *strRule, *strgategory, *submitUrl, *interface, *strIp;
	char tmpBuf[100];
	int intVal, cur_id, rule_id;
	char *arg0, *token;
	MIB_CE_PORT_FW_ADVANCE_T entry, entry_get;
	unsigned int totalEntry;
	unsigned int i;

	// Clear ALL Rule.
	config_PFWAdvance(ACT_STOP);

	// Add
	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		strgategory = boaGetVar(wp, "gategory", "");
		if (!strgategory[0]) {
			strcpy(tmpBuf, "Error! No gategory to set.");
			goto setErr_portfwAdvance;
		} else {
			if ( strgategory[0] == '0' ) {
				//printf("Select VPN gategory\n");
				entry.gategory = PFW_VPN;
			}
		}

		interface = boaGetVar(wp, "interface", "");
		if (interface) {
			if ( !string_to_dec(interface, &intVal)) {
				strcpy(tmpBuf, "Error! Invalid interface.");
				goto setErr_portfwAdvance;
			}
			entry.ifIndex = (unsigned short)intVal;
		}

		strIp = boaGetVar(wp, "ip", "");
		if (!strIp[0]) {
			strcpy(tmpBuf, "Error! No ip address to set.");
			goto setErr_portfwAdvance;
		}
		inet_aton(strIp, (struct in_addr *)&entry.ipAddr);

		strRule = boaGetVar(wp, "ruleApply", "");
		if (strRule[0])
		{
			//printf("str=%s\n", strRule);
			arg0 = strRule;
			while ((token=strtok(arg0,","))!=NULL) {
				cur_id = atoi(token);
				//rule_id += cur_id;
				arg0 = 0;
			}
			entry.rule = cur_id;
		} else {
			strcpy(tmpBuf, "Error! No Rule to set.");
			goto setErr_portfwAdvance;
		}

		// Check if this is a duplicate rule ?
		totalEntry = mib_chain_total(MIB_PFW_ADVANCE_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&entry_get))
			{
				strcpy(tmpBuf, "Get chain record error for checking if this is a duplicate tule!");
  				goto setErr_portfwAdvance;
			} else {
				if ( entry_get.rule == entry.rule ) {
					strcpy(tmpBuf, "Error: This is a duplicate rule!");
  					goto setErr_portfwAdvance;
				}
			}
		}

		intVal = mib_chain_add(MIB_PFW_ADVANCE_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_portfwAdvance;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_portfwAdvance;
		}
		goto setOk_PFWAd;
	}

	// Delete all Rule
	str = boaGetVar(wp, "delAllRule", "");
	if (str[0]) {
		mib_chain_clear(MIB_PFW_ADVANCE_TBL); /* clear chain record */
		goto setOk_PFWAd;
	}

	/* Delete selected Rule */
	str = boaGetVar(wp, "delRule", "");
	if (str[0]) {
		unsigned int idx;
		unsigned int deleted = 0;

		totalEntry = mib_chain_total(MIB_PFW_ADVANCE_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				deleted ++;
				if(mib_chain_delete(MIB_PFW_ADVANCE_TBL, idx) != 1) {
					strcpy(tmpBuf, "Delete chain record error!");
					goto setErr_portfwAdvance;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, "There is no item selected to delete!");
			goto setErr_portfwAdvance;
		}

		goto setOk_PFWAd;
	}

setOk_PFWAd:
	config_PFWAdvance(ACT_START);
	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);

  	return;

setErr_portfwAdvance:
	ERR_MSG(tmpBuf);

}


int showPFWAdvTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_ADVANCE_T Entry;
	char strGategory[10], strRule[10], interface_name[8], lanIP[35];
	struct in_addr dest;

	entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Gategory</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">LAN IP Address</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Rule</td></font></tr>\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
  			printf("Get chain record error!\n");
			return 1;
		}

		// Gategory
		strcpy(strGategory, PFW_Gategory[(PFW_GATEGORY_T)Entry.gategory]);

		// LAN IP Address
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(lanIP, inet_ntoa(dest));

		// Rule
		strcpy(strRule, PFW_Rule[(PFW_RULE_T)Entry.rule]);

		// interface
		if ( Entry.ifIndex == DUMMY_IFINDEX )
		{
			strcpy( interface_name, "---" );
		}else {			
			ifGetName(Entry.ifIndex, interface_name, sizeof(interface_name));
		}

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n",
		i, strGategory, interface_name, lanIP, strRule);

	}
	return nBytesSent;
}
#endif

int showPFWAdvForm(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

#ifdef PORT_FORWARD_ADVANCE
	nBytesSent += boaWrite(wp, 
			"<table border=0 width=\"500\" cellspacing=0 cellpadding=0>\n"
				"<tr><hr size=1 noshade align=top></tr>\n"
				"<tr>\n"
					"<td>\n"
						"<input type=\"button\" value=\"Advance Setting\" name=\"advance\" onClick=\"portFWClick('/portfw-advance.asp')\">\n"
					"</td>\n"
				"</tr>\n"
			"</table>\n");

#else
	nBytesSent += boaWrite(wp, 
			"<input type=\"hidden\" name=advance>\n"
			"\n");
#endif
	return nBytesSent;
}


#ifdef VIRTUAL_SERVER_SUPPORT
int virtualSvrLeft(int eid, request * wp, int argc, char **argv)
{
	boaWrite(wp,"document.all(\"numleft\").innerHTML=\"%d\";\n",VIRTUAL_SERVER_RULES-mib_chain_total(MIB_VIRTUAL_SVR_TBL));
    	boaWrite(wp, "numToConfig=%d; \n", VIRTUAL_SERVER_RULES-mib_chain_total(MIB_VIRTUAL_SVR_TBL));
}
/*****************************
**虚拟服务器配置
*/
int virtualSvrList (int eid, request * wp, int argc, char **argv)
{
	struct vtlsvr_entryx	entry ;
	char			serverIp[16];
	int				cnt = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	int				index = 0;
	int				lineno = __LINE__;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	/************Place your code here, do what you want to do! ************/

	for(index = 0; index < cnt; index++)
	{
		/************Place your code here, do what you want to do! ************/
		mib_chain_get(MIB_VIRTUAL_SVR_TBL,index,&entry);
		/************Place your code here, do what you want to do! ************/		
		boaWrite(wp, "push(new it_nr(\"%d\"" _PTS _PTS _PTI _PTI _PTI _PTI "));\n", 
			index, _PMEIP(serverIp), _PME(svrName), _PME(wanStartPort), _PME(wanEndPort),
			_PME(lanPort), _PME(protoType));
	}
		
check_err:
	_TRACE_LEAVEL;
	return 0;
}

void formVrtsrv(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl, *strSubmit;
	unsigned int totalEntry;
	int i, intVal;
	char tmpBuf[100];
	struct vtlsvr_entryx entry;

	totalEntry = mib_chain_total(MIB_VIRTUAL_SVR_TBL);	/* get chain record size */
	strSubmit = boaGetVar(wp, "action", "");
	if (strcmp(strSubmit, "delete") == 0)
	{
		setupVtlsvr(VIRTUAL_SERVER_DELETE);

		for (i = totalEntry - 1; i >= 0; i--) {
			sprintf(tmpBuf, "rml%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");

			if (strVal[0]) {
				if (!gstrcmp(strVal, "ON")) {
					if (mib_chain_delete
					    (MIB_VIRTUAL_SVR_TBL, i) != 1) {
						strcpy(tmpBuf,
						       strDelChainerror);
						goto check_err;
					}
				}
			}
		}
		setupVtlsvr(VIRTUAL_SERVER_ADD);
	} else if (strcmp(strSubmit, "add") == 0) {
		if (totalEntry >= VIRTUAL_SERVER_RULES) {
			strcpy(tmpBuf, "对不起,规则数已达最大限制!");
			goto check_err;
		}

		strVal = boaGetVar_adv(wp, "radiosrv", "", 0);
		strVal =
		    boaGetVar(wp, strVal[0] ? "srvName" : "cusSrvName", "");
		if (strVal[0])
			strncpy(entry.svrName, strVal, sizeof(entry.svrName));
		else
			goto setSz_OK;
		for (i = 0; i < 12; i++) {
			strVal = boaGetVar_adv(wp, "wanStartPort", "", i);
			if (!strVal[0])
				continue;
			entry.wanStartPort = strtoul(strVal, NULL, 10);

			strVal = boaGetVar_adv(wp, "wanEndPort", "", i);
			if (!strVal[0])
				continue;
			entry.wanEndPort = strtoul(strVal, NULL, 10);

			strVal = boaGetVar_adv(wp, "protoType", "", i);
			if (!strVal[0])
				continue;
			entry.protoType = strtoul(strVal, NULL, 10);

			strVal = boaGetVar_adv(wp, "serverIp", "", i);
			if (!strVal[0])
				continue;
			inet_aton(strVal, entry.serverIp);

			strVal = boaGetVar_adv(wp, "lanPort", "", i);
			if (!strVal[0])
				continue;
			entry.lanPort = strtoul(strVal, NULL, 10);

			intVal = mib_chain_add(MIB_VIRTUAL_SVR_TBL, &entry);
			if (intVal == 0) {
				strcpy(tmpBuf, strAddChainerror);
				goto check_err;
			} else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto check_err;
			}
		}
		setupVtlsvr(VIRTUAL_SERVER_ADD);
	}
setSz_OK:
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0]) {
		boaRedirect(wp, submitUrl);
	} else
		boaDone(wp, 200);
	return;
check_err:
	ERR_MSG(tmpBuf);
}
#endif
