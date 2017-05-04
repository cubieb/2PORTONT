
/*
 *      Web server handler routines for MAC-Based Assignment for DHCP Server stuffs
 *
 */


/*-- System inlcude files --*/
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
/*ping_zhang:20090312 START:add STB checkbox for designate STB device type*/
#ifdef CONFIG_USER_UDHCP099PRE2
#include "../../../udhcp-0.9.9-pre2/dhcpd.h"
#else
#include "../../../udhcp-0.9.9-pre/dhcpd.h"
#endif
/*ping_zhang:20090312 END*/


/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#ifdef IMAGENIO_IPTV_SUPPORT
void formIpRange(request * wp, char *path, char *query)
{
	DHCPS_SERVING_POOL_T dhcpSpEntry;
	MIB_CE_DHCP_OPTION_T optEntry;
	unsigned int i, totalEntry;
	char *strVal, *submitUrl;
	int dhcpd_changed_flag = 0;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	struct in_addr ip,opchAddr,stbDnsAdd1,stbDnsAdd2;
	unsigned short opchport, origport;

	totalEntry = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL); /* get chain record size */

	// Delete
	strVal = boaGetVar(wp, "del", "");
	if (strVal[0]) {
		unsigned int idx;

		strVal = boaGetVar(wp, "select", "");
		if (strVal[0]) {
			for (i=0; i<totalEntry; i++) {
				if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)&dhcpSpEntry ) )
					continue;
				idx=i;//idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);

				if ( !gstrcmp(strVal, tmpBuf) ) {
					dhcpd_changed_flag = 1;
					// clear dhcp option enty of this pool
					clearOptTbl(dhcpSpEntry.InstanceNum);
				#ifdef SUPPORT_DHCP_RESERVED_IPADDR
					clearDHCPReservedIPAddrByInstNum( dhcpSpEntry.InstanceNum );
				#endif //DHCP_SUPPORT_RESERVED_IPADDR

					// delete from chain record
					if(mib_chain_delete(MIB_DHCPS_SERVING_POOL_TBL, idx) != 1) {
						strcpy(tmpBuf, strDelChainerror);
						goto setErr_dhcpd;
					}

					break;
				}
			} // end of for
		}
		goto setOk_dhcpd;
	}

	strVal = boaGetVar(wp, "add", "");
	if (strVal[0]) {
		initSPDHCPOptEntry(&dhcpSpEntry);

		// poolname
		strVal = boaGetVar(wp, "name", "");
		if (strVal[0]) {
			DHCPS_SERVING_POOL_T entry;
			for (i=0; i<totalEntry; i++) {
				mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&entry);
				if (!gstrcmp(strVal, entry.poolname)) {
					strcpy(tmpBuf, strDeviceIsAlreadyExist);
					goto setErr_dhcpd;
				}
			}
			strcpy(dhcpSpEntry.poolname, strVal);
			dhcpSpEntry.poolname[MAX_NAME_LEN-1] = 0;
		} else {
			sprintf(tmpBuf, "%s (device name)",strNoSetError);
			goto setErr_dhcpd;
		}

		// start IP addr
		strVal = boaGetVar(wp, "sIp", "");
		if (strVal[0]) {
			if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.startaddr) ) {
				sprintf(tmpBuf, "%s (source IP)", strInvalidValue);
				goto setErr_dhcpd;
			}
		} else {
			sprintf(tmpBuf, "%s (source IP)", strInvalidValue);
			goto setErr_dhcpd;
		}

		// end IP addr
		strVal = boaGetVar(wp, "dIp", "");
		if (strVal[0]) {
			if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.endaddr) ) {
				sprintf(tmpBuf, "%s (destination IP)", strInvalidValue);
				goto setErr_dhcpd;
			}
		} else {
			sprintf(tmpBuf, "%s (destination IP)", strInvalidValue);
			goto setErr_dhcpd;
		}

		if (dhcpSpEntry.endaddr[3] <= dhcpSpEntry.startaddr[3]) {
			strcpy(tmpBuf, strDestIPShouldBeLargerThanSrcIP);
			goto setErr_dhcpd;
		}

/*ping_zhang:20090526 START:Add gateway for each ip range*/
		// gw IP addr
		strVal = boaGetVar(wp, "gwIp", "");
		if (strVal[0]) {
			if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.iprouter) ) {
				sprintf(tmpBuf, "%s (Gateway IP)", strInvalidValue);
				goto setErr_dhcpd;
			}
		} else {
			sprintf(tmpBuf, "%s (Gateway IP)", strInvalidValue);
			goto setErr_dhcpd;
		}
/*ping_zhang:20090526 END*/

		// option 60
		strVal = boaGetVar(wp, "Opt60", "");
		if (strVal[0]) {
			strncpy(dhcpSpEntry.vendorclass,strVal,OPTION_60_LEN);
			dhcpSpEntry.vendorclass[OPTION_60_LEN] = 0;
		} else {
			sprintf(tmpBuf, "%s (option60)", strInvalidValue);
			goto setErr_dhcpd;
		}

		// device type
		strVal = boaGetVar(wp, "dhcpDeviceType", "");
		if (strVal[0]) {
			dhcpSpEntry.deviceType = (unsigned char)atoi(strVal);
		}
		else
			dhcpSpEntry.deviceType = CTC_UNKNOWN;

		//reserved option code
		if(dhcpSpEntry.deviceType == CTC_STB)
		{
			dhcpSpEntry.rsvOptCode = DHCP_OPT_240;
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
			//from fmdhcpd.c
			strVal= boaGetVar(wp, "opchaddr", "");
			if (strVal[0]) {
				if (!inet_aton(strVal, &ip)) {
					strcpy(tmpBuf, strInvalidOpchaddr);
					goto setErr_dhcpd;
				}
			} else {
				strcpy(tmpBuf, strInvalidOpchaddr);
				goto setErr_dhcpd;
			}
			mib_get(MIB_OPCH_ADDRESS, (void *)&opchAddr);
			if (opchAddr.s_addr != ip.s_addr)
				dhcpd_changed_flag = 1;
			mib_set(MIB_OPCH_ADDRESS, (void *)&ip);

			strVal = boaGetVar(wp, "opchport", "");
			if (strVal[0]) {
				opchport = (unsigned short)atoi(strVal);
			} else {
				strcpy(tmpBuf, strInvalidOpchport);
				goto setErr_dhcpd;
			}
			mib_get(MIB_OPCH_PORT, (void *)&origport);
			if (origport != opchport)
				dhcpd_changed_flag = 1;
			mib_set(MIB_OPCH_PORT, (void *)&opchport);
#endif
/*ping_zhang:20090930 END*/	

			ip.s_addr = INADDR_NONE;
			strVal = boaGetVar(wp, "stbdns1", "");
			if (strVal[0]) {
				if (!inet_aton(strVal, &ip)) {
					strcpy(tmpBuf, Tinvalid_dns);
					goto setErr_dhcpd;
				}
			}
			mib_get(MIB_IMAGENIO_DNS1, (void *)&stbDnsAdd1);
			if (stbDnsAdd1.s_addr != ip.s_addr)
				dhcpd_changed_flag = 1;
			mib_set(MIB_IMAGENIO_DNS1, (void *)&ip);

			ip.s_addr = INADDR_NONE;
			strVal = boaGetVar(wp, "stbdns2", "");
			if (strVal[0]) {
				if (!inet_aton(strVal, &ip)) {
					strcpy(tmpBuf, Tinvalid_dns);
					goto setErr_dhcpd;
				}
			}
			mib_get(MIB_IMAGENIO_DNS2, (void *)&stbDnsAdd2);
			if (stbDnsAdd2.s_addr != ip.s_addr)
				dhcpd_changed_flag = 1;
			mib_set(MIB_IMAGENIO_DNS2, (void *)&ip);
			
/*ping_zhang:20090930 START:add for Telefonica new option 240*/			
			memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
			optEntry.enable = 1;
			optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
			optEntry.dhcpConSPInstNum = dhcpSpEntry.InstanceNum;
			optEntry.dhcpOptInstNum = 1;
			optEntry.tag = 240;
			//save option240 str for reserved option
			strVal = boaGetVar(wp, "OptStr240", "");
			if (strVal[0]) {
				strncpy(optEntry.value, strVal,OPTION_RSV_LEN);
				optEntry.value[OPTION_RSV_LEN] = 0;
				optEntry.len = strlen(optEntry.value);
			} else {
				sprintf(tmpBuf, "%s (optionStr240)", strInvalidValue);
				goto setErr_dhcpd;
			}
/*ping_zhang:20090930 END*/
		}
		else if(dhcpSpEntry.deviceType == CTC_UNKNOWN)
		{
			dhcpSpEntry.rsvOptCode = DHCP_END;
		}
		else
		{
			memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
			optEntry.enable = 1;
			optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
			optEntry.dhcpConSPInstNum = dhcpSpEntry.InstanceNum;
			optEntry.dhcpOptInstNum = 1;

			strVal = boaGetVar(wp, "reservedOption", "");
			if (strVal[0]) {
				dhcpSpEntry.rsvOptCode = (unsigned char)atoi(strVal);
				optEntry.tag = (unsigned char)atoi(strVal);
			}
			else{
				sprintf(tmpBuf, "%s (optionCode)", strInvalidValue);
				goto setErr_dhcpd;
			}
			//save option str for reserved option
			strVal = boaGetVar(wp, "OptStr", "");
			if (strVal[0]) {
				strncpy(optEntry.value, strVal,OPTION_RSV_LEN);
				optEntry.value[OPTION_RSV_LEN] = 0;
				optEntry.len = strlen(optEntry.value);
			} else {
				sprintf(tmpBuf, "%s (optionStr)", strInvalidValue);
				goto setErr_dhcpd;
			}
		}

		dhcpd_changed_flag = 1;
		if (!mib_chain_add(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcpSpEntry)) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_dhcpd;
		} else {
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
			//if( (dhcpSpEntry.deviceType != CTC_STB)&&(dhcpSpEntry.deviceType != CTC_UNKNOWN) )
			if(dhcpSpEntry.deviceType != CTC_UNKNOWN)
/*ping_zhang:20090930 END*/
			{
				if (!mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (void*)&optEntry)) {
					strcpy(tmpBuf, strAddChainerror);
					goto setErr_dhcpd;
				}
			}
			goto setOk_dhcpd;
		}
	}

	strVal = boaGetVar(wp, "mod", "");
	if (strVal[0]) {
		unsigned int idx;
		unsigned char oldDeviceType;

		strVal = boaGetVar(wp, "select", "");
		if (strVal[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = i; //idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(strVal, tmpBuf) ) {
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, idx, (void*)&dhcpSpEntry ) ) {
						sprintf(tmpBuf,"%s MIB_DHCPS_SERVING_POOL_TBL ",Tget_mib_error);
						goto setErr_dhcpd;
					}

					// poolname
					strVal = boaGetVar(wp, "name", "");
					if (strVal[0]) {
						DHCPS_SERVING_POOL_T entry;
						for (i=0; i<totalEntry; i++) {
							if (i== idx)
								continue;
							mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&entry);
							if (!gstrcmp(strVal, entry.poolname)) {
								strcpy(tmpBuf, strDeviceIsAlreadyExist);
								goto setErr_dhcpd;
							}
						}
						strcpy(dhcpSpEntry.poolname, strVal);
						dhcpSpEntry.poolname[MAX_NAME_LEN-1] = 0;
					} else {
						sprintf(tmpBuf, "%s (Device Name)", strInvalidValue);
						goto setErr_dhcpd;
					}

					// start IP addr
					strVal = boaGetVar(wp, "sIp", "");
					if (strVal[0]) {
						if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.startaddr) ) {
							sprintf(tmpBuf, "%s (source IP)", strInvalidValue);
							goto setErr_dhcpd;
						}
					} else {
						sprintf(tmpBuf, "%s (source IP)", strInvalidValue);
						goto setErr_dhcpd;
					}

					// end IP addr
					strVal = boaGetVar(wp, "dIp", "");
					if (strVal[0]) {
						if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.endaddr) ) {
							sprintf(tmpBuf, "%s (dest IP)", strInvalidValue);
							goto setErr_dhcpd;
						}
					} else {
						sprintf(tmpBuf, "%s (dest IP)", strInvalidValue);
						goto setErr_dhcpd;
					}

					if (dhcpSpEntry.endaddr[3] <= dhcpSpEntry.startaddr[3]) {
						strcpy(tmpBuf, "destination IP address should be larger than source IP address!");
						goto setErr_dhcpd;
					}

/*ping_zhang:20090526 START:Add gateway for each ip range*/
					// gw IP addr
					strVal = boaGetVar(wp, "gwIp", "");
					if (strVal[0]) {
						if ( !inet_aton(strVal, (struct in_addr *)dhcpSpEntry.iprouter) ) {
							sprintf(tmpBuf, "%s (gateway IP)", strInvalidValue);
							goto setErr_dhcpd;
						}
					} else {
						sprintf(tmpBuf, "%s (gateway IP)", strInvalidValue);
						goto setErr_dhcpd;
					}
/*ping_zhang:20090526 END*/

					// option 60
					strVal = boaGetVar(wp, "Opt60", "");
					if (strVal[0]) {
						strncpy(dhcpSpEntry.vendorclass,strVal,OPTION_60_LEN);
						dhcpSpEntry.vendorclass[OPTION_60_LEN] = 0;
					} else {
						sprintf(tmpBuf, "%s (option60)", strInvalidValue);
						goto setErr_dhcpd;
					}

					// device type
					oldDeviceType = dhcpSpEntry.deviceType;
					strVal = boaGetVar(wp, "dhcpDeviceType", "");
					if (strVal[0]) {
						dhcpSpEntry.deviceType = (unsigned char)atoi(strVal);
					}
					else
						dhcpSpEntry.deviceType = CTC_UNKNOWN;

					dhcpd_changed_flag = 1;
					if(mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcpSpEntry, idx) != 1) {
						strcpy(tmpBuf, strModChainerror);
						goto setErr_dhcpd;
					}

					//clear option table of the server pool
					clearOptTbl(dhcpSpEntry.InstanceNum);
					//reserved option code
					if(dhcpSpEntry.deviceType == CTC_STB)
					{
						dhcpSpEntry.rsvOptCode = DHCP_OPT_240;
						if(mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcpSpEntry, idx) != 1) {
							strcpy(tmpBuf, strModChainerror);
							goto setErr_dhcpd;
						}

/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
						//from fmdhcpd.c
						strVal= boaGetVar(wp, "opchaddr", "");
						if (strVal[0]) {
							if (!inet_aton(strVal, &ip)) {
								strcpy(tmpBuf, strInvalidOpchaddr);
								goto setErr_dhcpd;
							}
						} else {
							strcpy(tmpBuf, strInvalidOpchaddr);
							goto setErr_dhcpd;
						}
						mib_get(MIB_OPCH_ADDRESS, (void *)&opchAddr);
						if (opchAddr.s_addr != ip.s_addr)
							dhcpd_changed_flag = 1;
						mib_set(MIB_OPCH_ADDRESS, (void *)&ip);

						strVal = boaGetVar(wp, "opchport", "");
						if (strVal[0]) {
							opchport = (unsigned short)atoi(strVal);
						} else {
							strcpy(tmpBuf, strInvalidOpchport);
							goto setErr_dhcpd;
						}
						mib_get(MIB_OPCH_PORT, (void *)&origport);
						if (origport != opchport)
							dhcpd_changed_flag = 1;
						mib_set(MIB_OPCH_PORT, (void *)&opchport);
#endif
/*ping_zhang:20090930 END*/

						ip.s_addr = INADDR_NONE;
						strVal = boaGetVar(wp, "stbdns1", "");
						if (strVal[0]) {
							if (!inet_aton(strVal, &ip)) {
								strcpy(tmpBuf, Tinvalid_dns);
								goto setErr_dhcpd;
							}
						}
						mib_get(MIB_IMAGENIO_DNS1, (void *)&stbDnsAdd1);
						if (stbDnsAdd1.s_addr != ip.s_addr)
							dhcpd_changed_flag = 1;
						mib_set(MIB_IMAGENIO_DNS1, (void *)&ip);

						ip.s_addr = INADDR_NONE;
						strVal = boaGetVar(wp, "stbdns2", "");
						if (strVal[0]) {
							if (!inet_aton(strVal, &ip)) {
								strcpy(tmpBuf, Tinvalid_dns);
								goto setErr_dhcpd;
							}
						}
						mib_get(MIB_IMAGENIO_DNS2, (void *)&stbDnsAdd2);
						if (stbDnsAdd2.s_addr != ip.s_addr)
							dhcpd_changed_flag = 1;
						mib_set(MIB_IMAGENIO_DNS2, (void *)&ip);
						
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
						memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
						optEntry.enable = 1;
						optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
						optEntry.dhcpConSPInstNum = dhcpSpEntry.InstanceNum;
						optEntry.dhcpOptInstNum = 1;
						optEntry.tag = 240;
						//save option240 str for reserved option
						strVal = boaGetVar(wp, "OptStr240", "");
						if (strVal[0]) {
							strncpy(optEntry.value, strVal,OPTION_RSV_LEN);
							optEntry.value[OPTION_RSV_LEN] = 0;
							optEntry.len = strlen(optEntry.value);
						} else {
							sprintf(tmpBuf, "%s (optionStr)", strInvalidValue);
							goto setErr_dhcpd;
						}

						dhcpd_changed_flag = 1;
						if (!mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&optEntry)) {
							strcpy(tmpBuf, strAddChainerror);
							goto setErr_dhcpd;
						} else
							goto setOk_dhcpd;
/*ping_zhang:20090930 END*/
					}
					else if(dhcpSpEntry.deviceType == CTC_UNKNOWN)
					{
						dhcpSpEntry.rsvOptCode = DHCP_END;
						if(mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcpSpEntry, idx) != 1) {
							strcpy(tmpBuf, strModChainerror);
							goto setErr_dhcpd;
						}
					}
					else
					{
						memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
						optEntry.enable = 1;
						optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
						optEntry.dhcpConSPInstNum = dhcpSpEntry.InstanceNum;
						optEntry.dhcpOptInstNum = 1;

						strVal = boaGetVar(wp, "reservedOption", "");
						if (strVal[0]) {
							dhcpSpEntry.rsvOptCode = (unsigned char)atoi(strVal);
							optEntry.tag = (unsigned char)atoi(strVal);
							if(mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&dhcpSpEntry, idx) != 1) {
								strcpy(tmpBuf, strModChainerror);
								goto setErr_dhcpd;
							}
						}
						else{
							sprintf(tmpBuf, "%s (optionCode)", strInvalidValue);
							goto setErr_dhcpd;
						}
						//save option str for reserved option
						strVal = boaGetVar(wp, "OptStr", "");
						if (strVal[0]) {
							strncpy(optEntry.value, strVal,OPTION_RSV_LEN);
							optEntry.value[OPTION_RSV_LEN] = 0;
							optEntry.len = strlen(optEntry.value);
						} else {
							sprintf(tmpBuf, "%s (optionStr)", strInvalidValue);
							goto setErr_dhcpd;
						}

						dhcpd_changed_flag = 1;
						if (!mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&optEntry)) {
							strcpy(tmpBuf, strAddChainerror);
							goto setErr_dhcpd;
						} else
							goto setOk_dhcpd;

					}

					break;
				}
			} // end of for
		}
		goto setOk_dhcpd;
	}

setOk_dhcpd:
#ifdef COMMIT_IMMEDIATELY
	if( dhcpd_changed_flag == 1 )
	{
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
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
	if(dhcpd_changed_flag == 1)
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

setErr_dhcpd:
	ERR_MSG(tmpBuf);
}

int showDeviceIpTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0,id=-1;
	unsigned int entryNum,optEntryNum,i,j,optNum;
	DHCPS_SERVING_POOL_T Entry;
	MIB_CE_DHCP_OPTION_T rsvOptEntry,optEntry;
	char startIp[16], endIp[16];
/*ping_zhang:20090526 START:Add gateway for each ip range*/
	char gwIp[16];
/*ping_zhang:20090526 END*/
	char *devType;
	struct in_addr opchAddr;
	unsigned short opchport;
	char optionStr[DHCP_OPT_VAL_LEN];

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s%s</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\">Option60</td>"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\" colspan=\"2\">%s</td>"
	"</font></tr>\n", multilang(LANG_SELECT), multilang(LANG_DEVICE), multilang(LANG_NAME_1),
	multilang(LANG_DEVICE_TYPE), multilang(LANG_START_ADDRESS), multilang(LANG_END_ADDRESS), multilang(LANG_GATEWAY_ADDRESS),
	multilang(LANG_RESERVED_OPTION));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain(MIB_DHCPS_SERVING_POOL_TBL) record error!\n");
			return;
		}

		strcpy(startIp, inet_ntoa(*((struct in_addr *)Entry.startaddr)));
		strcpy(endIp, inet_ntoa(*((struct in_addr *)Entry.endaddr)));
		strcpy(gwIp, inet_ntoa(*((struct in_addr *)Entry.iprouter)));

		switch(Entry.deviceType)
		{
		case CTC_Computer:
			devType = multilang(LANG_PC);
			break;
		case CTC_Camera:
			devType = multilang(LANG_CAMERA);
			break;
		case CTC_HGW:
			devType = multilang(LANG_HGW);
			break;
		case CTC_STB:
			devType = multilang(LANG_STB);
			break;
		case CTC_PHONE:
			devType = multilang(LANG_PHONE);
			break;
		case CTC_UNKNOWN:
		default:
			devType = multilang(LANG_UNKNOWN);
			break;
		}

		optNum = getSPDHCPOptEntryNum(eUsedFor_DHCPServer_ServingPool, Entry.InstanceNum);
		id=-1;//re-init
		getSPDHCPRsvOptEntryByCode(Entry.InstanceNum, Entry.rsvOptCode, &rsvOptEntry, &id);
		/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
		if(Entry.deviceType == CTC_STB) {
			optNum += 1;
			mib_get(MIB_OPCH_ADDRESS, (void *)&opchAddr);
			mib_get(MIB_OPCH_PORT, (void *)&opchport);
			sprintf(optionStr, ":::::%s:%d", inet_ntoa(opchAddr), opchport);
		}else
#endif
		if(Entry.deviceType == CTC_UNKNOWN) {
			optNum += 1;
			strcpy(optionStr,"");
		}else {
			if(id!=-1)
				strcpy(optionStr,rsvOptEntry.value);
			else
				strcpy(optionStr,"");
		}

		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><input type=\"radio\" name=\"select\" value=\"s%d\" "
			"onClick=\"postsetting(devname[%d],devtype[%d],startip[%d],endip[%d],gwip[%d],option[%d],opCode[%d],opStr[%d])\"></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\" rowspan=\"%d\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</font></td>\n"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",
			optNum,i, i, i, i, i, i, i, i, i,
			optNum,Entry.poolname, optNum,devType, optNum,startIp, optNum,endIp,optNum,gwIp,optNum,Entry.vendorclass,Entry.rsvOptCode, optionStr);

		optEntryNum = mib_chain_total(MIB_DHCP_SERVER_OPTION_TBL);
		for (j=0; j<optEntryNum; j++)
		{
			if(j==id)
				continue;
			if (!mib_chain_get(MIB_DHCP_SERVER_OPTION_TBL, j, (void *)&optEntry) || !optEntry.enable)
				continue;
			if (optEntry.usedFor!=eUsedFor_DHCPServer_ServingPool ||optEntry.dhcpConSPInstNum!=Entry.InstanceNum)
				continue;
			nBytesSent += boaWrite(wp, "<tr>"
				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</font></td>\n"
				"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n"
				"</tr>\n",
				optEntry.tag,optEntry.value);
		}
		nBytesSent += boaWrite(wp, "</tr>\n");
	}

	return 0;
}
#endif
#endif
