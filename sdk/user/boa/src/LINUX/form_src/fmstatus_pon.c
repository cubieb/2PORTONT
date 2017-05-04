/*
 *      Web server handler routines for System Pon status
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "multilang.h"
#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#include "rtk/stat.h"
#include "rtk/switch.h"
#endif

#ifdef CONFIG_RTK_OMCI_V1
#include <omci_api.h>
#include <gos_type.h>
#endif

unsigned short get_usflow_index(void)
{
	rtk_switch_devInfo_t devInfo;
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_switch_deviceInfo_get(&devInfo);
#else
	rtk_switch_deviceInfo_get(&devInfo);
#endif
	switch(devInfo.chipId)
	{
		case RTL9601B_CHIP_ID:
		return 32;
		case APOLLOMP_CHIP_ID:
		default:
		return 127;
	}
}

const char *loid_get_authstatus(int index)
{
	switch (index)
	{
		case 0:
		return multilang(LANG_LOID_STATUS_INIT);
		case 1:
		return multilang(LANG_LOID_STATUS_SUCCESS);
		case 2:
		return multilang(LANG_LOID_STATUS_ERROR);
		case 3:
		return multilang(LANG_LOID_STATUS_PWDERR);
		case 4:
		return multilang(LANG_LOID_STATUS_DUPLICATE);
		default:
		return "WRONG";
	}
}

int showgpon_status(int eid, request * wp, int argc, char **argv)
{
	unsigned int pon_mode;
	int nBytesSent=0;
	int loid_st = 0;
	unsigned int usflow = 0;
	rtk_gpon_fsm_status_t onu;
	rtk_gpon_usFlow_attr_t aAttr;
#ifdef CONFIG_RTK_OMCI_V1
	PON_OMCI_CMD_T msg;
#endif

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0) 
	{
		if (pon_mode != GPON_MODE) 
			return 0;
	}
	else
		return 0;

#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_gpon_ponStatus_get(&onu);
#else
	rtk_gpon_ponStatus_get(&onu);
#endif
	usflow = get_usflow_index();
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_gpon_usFlow_get(usflow, &aAttr);
#else
	rtk_gpon_usFlow_get(usflow, &aAttr);
#endif	
#ifdef CONFIG_RTK_OMCI_V1
	memset(&msg, 0, sizeof(msg));
	msg.cmd = PON_OMCI_CMD_LOIDAUTH_GET_RSP;

	if(omci_SendCmdAndGet(&msg) == GOS_OK)
		loid_st = msg.state;
	else
		loid_st = -1; // wrong status
#endif


	nBytesSent += boaWrite(wp, "<tr> <td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>%s</b></font></td> </tr>"
        "<tr bgcolor=\"#DDDDDD\">"
		"<td width=\"30%%\"><font size=2><b>%s</b></td>"
        "<td width=\"70%%\"><font size=2>O%d</td> </tr>\n"
		"<tr bgcolor=\"#DDDDDD\">"
		"<td width=\"30%%\"><font size=2><b>%s</b></td>"
		"<td width=\"70%%\"><font size=2>%d</td> </tr>\n"
		"<tr bgcolor=\"#DDDDDD\">"
		"<td width=\"30%%\"><font size=2><b>%s</b></td>"
		"<td width=\"70%%\"><font size=2>%s</td> </tr>\n", 
		multilang(LANG_GPON_STATUS), multilang(LANG_ONU_STATE), onu,
		multilang(LANG_ONU_ID),aAttr.gem_port_id,
		multilang(LANG_LOID_STATUS),loid_get_authstatus(loid_st));	return 0;
}

int showepon_LLID_status(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
    int entryNum=4;
	char tmpBuf[100];
	int i;
	unsigned int pon_mode;

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0) 
	{
		if (pon_mode != EPON_MODE) 
			return 0;
	}
	else
			return 0;
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_epon_llidEntryNum_get(&entryNum);
#else
	rtk_epon_llidEntryNum_get(&entryNum);
#endif

	if(entryNum<=0)
	{
 		strcpy(tmpBuf, multilang(LANG_GET_LLIDENTRYNUM_ERROR));
		ERR_MSG(tmpBuf);
		return -1;
	}

    nBytesSent += boaWrite(wp, "<td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b> EPON LLID Status </b></font></td>");

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"33%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"33%%\" bgcolor=\"#808080\">%s</td>\n"
	"</font></tr>\n", "index", multilang(LANG_STATUS));

	for (i=0; i<entryNum; i++) {
		rtk_epon_llid_entry_t llid_entry;

		memset(&llid_entry, 0, sizeof(llid_entry));
		llid_entry.llidIdx = i;
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_llid_entry_get(&llid_entry);
#else
		rtk_epon_llid_entry_get(&llid_entry);
#endif
		nBytesSent += boaWrite(wp, "<tr>"
				"<td align=center width=\"33%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>\n"
				"<td align=center width=\"33%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td><br>\n",
				llid_entry.llidIdx, llid_entry.valid);
	}
	return 0;

}

void formStatus_pon(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;
}

int ponGetStatus(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	char *name;
	uint64 value = 0;
	rtk_transceiver_data_t transceiver, readableCfg;
	rtk_stat_port_cntr_t counters;
	unsigned int pon_mode;
#if !defined(CONFIG_RTK_L34_ENABLE)
	unsigned int pon_port_idx = 1;
#endif

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}
#if !defined(CONFIG_RTK_L34_ENABLE)
	rtk_switch_phyPortId_get(RTK_PORT_PON, &pon_port_idx);
#endif

	memset(&transceiver, 0, sizeof(transceiver));
	memset(&readableCfg, 0, sizeof(readableCfg));
	memset(&counters, 0, sizeof(counters));

	if (!strcmp(name, "vendor-name")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "part-number")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	}
	else if (!strcmp(name, "bytes-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_OCTETS_INDEX,
				     &value);
#else
		rtk_stat_port_get(pon_port_idx, IF_OUT_OCTETS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "bytes-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_OCTETS_INDEX,
				     &value);
#else
		rtk_stat_port_get(pon_port_idx, IF_IN_OCTETS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutUcastPkts + counters.ifOutMulticastPkts
						+ counters.ifOutBrocastPkts);
	} else if (!strcmp(name, "packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInUcastPkts + counters.ifInMulticastPkts
						+ counters.ifInBroadcastPkts);
	} else if (!strcmp(name, "unicast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutUcastPkts);
	} else if (!strcmp(name, "unicast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInUcastPkts);
	} else if (!strcmp(name, "multicast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutMulticastPkts);
	} else if (!strcmp(name, "multicast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInMulticastPkts);
	} else if (!strcmp(name, "broadcast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutBrocastPkts);
	} else if (!strcmp(name, "broadcast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(pon_port_idx, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInBroadcastPkts);
	} else if (!strcmp(name, "fec-errors")) {
		mib_get(MIB_PON_MODE, &pon_mode);

		if (pon_mode == GPON_MODE) {
			rtk_gpon_global_counter_t counter;

			memset(&counter, 0, sizeof(counter));
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_PHY, &counter);
#else
			rtk_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_PHY, &counter);
#endif
			nBytesSent += boaWrite(wp, "%u", counter.dsphy.rx_fec_uncor_cw);
		} else {
			rtk_epon_counter_t counter;

			memset(&counter, 0, sizeof(counter));
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_epon_mibCounter_get(&counter);
#else
			rtk_epon_mibCounter_get(&counter);
#endif

			nBytesSent += boaWrite(wp, "%u", counter.fecUncorrectedBlocks);
		}

		return nBytesSent;
	} else if (!strcmp(name, "hec-errors")) {
		mib_get(MIB_PON_MODE, &pon_mode);

		if (pon_mode == GPON_MODE) {
			rtk_gpon_global_counter_t counter;

			memset(&counter, 0, sizeof(counter));
#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_GEM,
						      &counter);
#else
			rtk_gpon_globalCounter_get(RTK_GPON_PMTYPE_DS_GEM,
						   &counter);
#endif
			nBytesSent += boaWrite(wp, "%u", counter.dsgem.rx_hec_correct);
		} else {
			// EPON has no HEC error
			nBytesSent += boaWrite(wp, "0");
		}

		return nBytesSent;
	} else if (!strcmp(name, "packets-dropped")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_DISCARDS_INDEX,
				     &value);
#else
		rtk_stat_port_get(pon_port_idx, IF_OUT_DISCARDS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "pause-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_OUT_PAUSE_FRAMES_INDEX,
				     &value);
#else
		rtk_stat_port_get(pon_port_idx, DOT3_OUT_PAUSE_FRAMES_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "pause-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_IN_PAUSE_FRAMES_INDEX,
				     &value);
#else
		rtk_stat_port_get(pon_port_idx, DOT3_IN_PAUSE_FRAMES_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "temperature")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "voltage")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE,
					      &transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE,
					   &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "tx-power")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER,
					   &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "rx-power")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER,
					   &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "bias-current")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
#endif
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver, &readableCfg);
		return boaWrite(wp, "%s", readableCfg.buf);
	}

	return -1;
}
#ifdef CONFIG_GPON_FEATURE
void restartOMCIsettings()
{
	rtk_gpon_deActivate();
	system("omcicli mib reset");
	rtk_gpon_activate(RTK_GPONMAC_INIT_STATE_O1);
}

int showOMCI_OLT_mode(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	char omci_olt_mode;
	char *OMCI_OLT_MODE_STR_DISP[4];

	OMCI_OLT_MODE_STR_DISP[0] = multilang(LANG_OMCI_OLT_MODE_1);
	OMCI_OLT_MODE_STR_DISP[1] = multilang(LANG_OMCI_OLT_MODE_2);
	OMCI_OLT_MODE_STR_DISP[2] = multilang(LANG_OMCI_OLT_MODE_3);
	OMCI_OLT_MODE_STR_DISP[3] = multilang(LANG_OMCI_OLT_MODE_4);

	if(!mib_get(MIB_OMCI_OLT_MODE,  (void *)&omci_olt_mode))
		printf("get MIB_OMCI_OLT_MODE failed\n");

	nBytesSent += boaWrite(wp," <tr>"
	"<td width=\"30%%\"><font size=2><b>%s:</b></td>"
	"<td width=\"70%%\"><font size=2>"
	"<select name=\"omci_olt_mode\">", multilang(LANG_OMCI_OLT_MODE));

	for(i=0;i<4;i++){
		nBytesSent +=  boaWrite(wp, "<option value=\"%d\" %s>%s</option>", 
		i, omci_olt_mode == i ? "selected":"", OMCI_OLT_MODE_STR_DISP[i]);
	}

	nBytesSent +=  boaWrite(wp,"</select></td></tr>");	

	return 0;
}

int fmOmciInfo_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name, *strData;
	char tmpBuf[100];
	char vChar;
	int i;
	
	if (boaArgs(argc, argv, "%s", &name) < 1) 
	{
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}
	if(!strcmp(name, "omci_olt_info_readonly")) 	
	{			
		if(!mib_get(MIB_OMCI_OLT_MODE,  (void *)&vChar))		
		{	  		
			strcpy(tmpBuf, "get omci olt mode error!\n");			
			goto setErr;		
		}	
		if(vChar == 0)
			boaWrite(wp, "readonly");		
		return 0;	
	}
	if(!strcmp(name, "omci_olt_mode")) 	
	{			
		if(!mib_get(MIB_OMCI_OLT_MODE,  (void *)&vChar))		
		{	  		
			strcpy(tmpBuf, "get omci olt mode error!\n");			
			goto setErr;		
		}	
		boaWrite(wp, "\"%d\"", vChar);		
		return 0;	
	}
setErr:
	ERR_MSG(tmpBuf);
	return -1;
}

void formOmciInfo(request * wp, char *path, char *query)
{
	char *strData, tmpBuf[200];
	char vChar;
	int intVal;

	strData = boaGetVar(wp, "omci_vendor_id", "");	
	if ( strData[0] )	
	{
		if(!mib_set(MIB_OMCI_VENDOR_ID, (void *)strData))		
		{			
			strcpy(tmpBuf, "Save omci vendor id Error!\n");
			goto setErr;
		}	
	}
	
	strData = boaGetVar(wp, "omci_sw_ver1", "");	
	if ( strData[0] )	
	{
		if(!mib_set(MIB_OMCI_SW_VER1, (void *)strData))		
		{			
			strcpy(tmpBuf, "Save omci software version 1 Error!\n");			
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "omci_sw_ver2", "");	
	if ( strData[0] )	
	{	
		if(!mib_set(MIB_OMCI_SW_VER2, (void *)strData))		
		{			
			strcpy(tmpBuf, "Save omci software version 2 Error!\n");			
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "omcc_ver", "");	
	if ( strData[0] )	
	{	

		string_to_dec(strData, &intVal);
		vChar = intVal;

		if(!mib_set(MIB_OMCC_VER, (void *)&vChar))		
		{			
			strcpy(tmpBuf, "Save omcc version Error!\n");			
			goto setErr;		
		}
	}

	strData = boaGetVar(wp, "omci_tm_opt", "");	
	if ( strData[0] )	
	{
		vChar = strData[0] - '0';
		if(!mib_set(MIB_OMCI_TM_OPT, (void *)&vChar))		
		{			
			strcpy(tmpBuf, "Save omci traffic managament option Error!\n");			
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "omci_eqid", "");	
	if ( strData[0] )	
	{
		if(!mib_set(MIB_OMCI_EQID, (void *)strData))		
		{			
			strcpy(tmpBuf, "Save omci eqid Error!\n");			
			goto setErr;		
		}	
	}

	strData = boaGetVar(wp, "omci_ont_ver", "");	
	if ( strData[0] )	
	{
		if(!mib_set(MIB_OMCI_ONT_VER, (void *)strData))		
		{			
			strcpy(tmpBuf, "Save omci ont version Error!\n");			
			goto setErr;		
		}	
	}

	restartOMCIsettings();

	strData = boaGetVar(wp, "submit-url", "");

	OK_MSG(strData);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	return;
	
setErr:
	ERR_MSG(tmpBuf);

}
#endif

