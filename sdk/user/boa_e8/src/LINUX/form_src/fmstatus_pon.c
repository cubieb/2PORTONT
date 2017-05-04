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
#endif

#ifdef CONFIG_GPON_FEATURE
int showgpon_status(int eid, request * wp, int argc, char **argv)
{
	unsigned int pon_mode;
	rtk_gpon_fsm_status_t onu;
	char *status_map[] = {
		"Unknown State",
		"Initial State",
		"Standby State",
		"Serial Number State",
		"Ranging State",
		"Operation State",
		"POPUP State",
		"Emergency Stop State",
	};
	int nBytesSent = 0;

	if (mib_get(MIB_PON_MODE, &pon_mode) != 0) {
		if (pon_mode != GPON_MODE) {
			fprintf(stderr, "not GPON_MODE Error!");
			return 0;
		}
	} else {
		fprintf(stderr, "mib_get Error!");
		return 0;
	}

#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_gpon_ponStatus_get(&onu);
#else
	rtk_gpon_ponStatus_get(&onu);
#endif
	nBytesSent += boaWrite(wp, "%s (O%d)", status_map[onu], onu);

	return nBytesSent;
}
#endif

#ifdef CONFIG_EPON_FEATURE
int showepon_status(int eid, request * wp, int argc, char **argv)
{
	unsigned int pon_mode;
	int nBytesSent = 0;
	rtk_epon_llid_entry_t llid_entry;

	if (mib_get(MIB_PON_MODE, &pon_mode) != 0) {
		if (pon_mode != EPON_MODE) {
			fprintf(stderr, "not EPON_MODE Error!");
			return 0;
		}
	} else {
		fprintf(stderr, "mib_get Error!");
		return 0;
	}

	llid_entry.llidIdx = 0;
#if defined(CONFIG_RTK_L34_ENABLE)
	rtk_rg_epon_llid_entry_get(&llid_entry);
#else
	rtk_epon_llid_entry_get(&llid_entry);
#endif
	nBytesSent += boaWrite(wp, "%s", llid_entry.valid ? "Up" : "No Signal");

	return nBytesSent;
}
#endif

int ponGetStatus(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	char *name;
	uint64 value = 0;
	rtk_enable_t state = DISABLED;
	rtk_transceiver_data_t transceiver, readableCfg;
	rtk_stat_port_cntr_t counters;
	unsigned int pon_mode;
	char buf[512];
#ifdef CONFIG_EPON_FEATURE
	rtk_epon_llid_entry_t llid_entry;
#endif
#ifdef CONFIG_GPON_FEATURE
	uint32 i;
	int ret = RT_ERR_OK;
	rtk_gpon_dsFlow_attr_t attr;
#endif

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	memset(&transceiver, 0, sizeof(transceiver));
	memset(&readableCfg, 0, sizeof(readableCfg));
	memset(&counters, 0, sizeof(counters));
	buf[0] = '\0';

	if (!strcmp(name, "vendor-name")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_NAME, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "part-number")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VENDOR_PART_NUM, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	}
#ifdef CONFIG_EPON_FEATURE
	else if (!strcmp(name, "epon-mac-address")) {
		llid_entry.llidIdx = 0;
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_llid_entry_get(&llid_entry);
#else
		rtk_epon_llid_entry_get(&llid_entry);
#endif

		return boaWrite(wp, "%02X:%02X:%02X:%02X:%02X:%02X",
				llid_entry.mac.octet[0], llid_entry.mac.octet[1],
				llid_entry.mac.octet[2], llid_entry.mac.octet[3],
				llid_entry.mac.octet[4], llid_entry.mac.octet[5]);
	} else if (!strcmp(name, "epon-fec-us-state")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_usFecState_get(&state);
#else
		rtk_epon_usFecState_get(&state);
#endif
		return boaWrite(wp, "%s",
				state == ENABLED ? "ÆôÓÃ" : "½ûÓÃ");
	} else if (!strcmp(name, "epon-fec-ds-state")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_dsFecState_get(&state);
#else
		rtk_epon_dsFecState_get(&state);
#endif
		return boaWrite(wp, "%s",
				state == ENABLED ? "ÆôÓÃ" : "½ûÓÃ");
	} else if (!strcmp(name, "epon-triple-churning")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_churningStatus_get(&state);
#else
		rtk_epon_churningStatus_get(&state);
#endif
		return boaWrite(wp, "%s", state == ENABLED ? "ÆôÓÃ" : "½ûÓÃ");
	} else if (!strcmp(name, "epon-los")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_losState_get(&state);
#else
		rtk_epon_losState_get(&state);
#endif
		if (state == ENABLED)
			return boaWrite(wp, "Loss of Signal");
	}

#endif
#ifdef CONFIG_GPON_FEATURE
	else if (!strcmp(name, "gpon-fec-us-state")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_usFecSts_get(&state);
#else
		rtk_gpon_usFecSts_get(&state);
#endif
		return boaWrite(wp, "%s",
				state == ENABLED ? "ÆôÓÃ" : "½ûÓÃ");
	} else if (!strcmp(name, "gpon-fec-ds-state")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_dsFecSts_get(&state);
#else
		rtk_gpon_dsFecSts_get(&state);
#endif
		return boaWrite(wp, "%s",
				state == ENABLED ? "ÆôÓÃ" : "½ûÓÃ");
	} else if (!strcmp(name, "gpon-encryption")) {
		for (i = 0; i < 128; i++) {
#if defined(CONFIG_RTK_L34_ENABLE)
			ret = rtk_rg_gpon_dsFlow_get(i, &attr);
#else
			ret = rtk_gpon_dsFlow_get(i, &attr);
#endif
			if (ret != RT_ERR_OK)
				continue;

			sprintf(buf, "%sFlow %u: %s<br>", buf, i, attr.aes_en ? "ÆôÓÃ" : "½ûÓÃ");
		}

		return boaWrite(wp, buf);
	} else if (!strcmp(name, "gpon-alarm")) {

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_alarmStatus_get(RTK_GPON_ALARM_LOS, (int32 *)&state);
#else
		rtk_gpon_alarmStatus_get(RTK_GPON_ALARM_LOS, (int32 *)&state);
#endif
		if (state == ENABLED)
			strcat(buf, "[ÐÅºÅ¶ªÊ§]");

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_alarmStatus_get(RTK_GPON_ALARM_LOF, (int32 *)&state);
#else
		rtk_gpon_alarmStatus_get(RTK_GPON_ALARM_LOF, (int32 *)&state);
#endif
		if (state == ENABLED) {
			if (strlen(buf) != 0)
				strcat(buf, ", ");
			strcat(buf, "[Ö¡¶ªÊ§]");
		}

#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_alarmStatus_get(RTK_GPON_ALARM_LOM, (int32 *)&state);
#else
		rtk_gpon_alarmStatus_get(RTK_GPON_ALARM_LOM, (int32 *)&state);
#endif
		if (state == ENABLED) {
			if (strlen(buf) != 0)
				strcat(buf, ", ");
			strcat(buf, "[¸´Ö¡¶ªÊ§]");
		}

		return boaWrite(wp, buf);
	}
#endif
	else if (!strcmp(name, "bytes-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_OUT_OCTETS_INDEX,
				     &value);
#else
		rtk_stat_port_get(RTK_PORT_PON, IF_OUT_OCTETS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "bytes-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, IF_IN_OCTETS_INDEX,
				     &value);
#else
		rtk_stat_port_get(RTK_PORT_PON, IF_IN_OCTETS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutUcastPkts + counters.ifOutMulticastPkts
						+ counters.ifOutBrocastPkts);
	} else if (!strcmp(name, "packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInUcastPkts + counters.ifInMulticastPkts
						+ counters.ifInBroadcastPkts);
	} else if (!strcmp(name, "unicast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutUcastPkts);
	} else if (!strcmp(name, "unicast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInUcastPkts);
	} else if (!strcmp(name, "multicast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutMulticastPkts);
	} else if (!strcmp(name, "multicast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifInMulticastPkts);
	} else if (!strcmp(name, "broadcast-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
#endif
		return boaWrite(wp, "%u", counters.ifOutBrocastPkts);
	} else if (!strcmp(name, "broadcast-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_getAll(RTK_RG_PORT_PON, &counters);
#else
		rtk_stat_port_getAll(RTK_PORT_PON, &counters);
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
		rtk_stat_port_get(RTK_PORT_PON, IF_OUT_DISCARDS_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "pause-packets-sent")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_OUT_PAUSE_FRAMES_INDEX,
				     &value);
#else
		rtk_stat_port_get(RTK_PORT_PON, DOT3_OUT_PAUSE_FRAMES_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "pause-packets-received")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_stat_port_get(RTK_RG_PORT_PON, DOT3_IN_PAUSE_FRAMES_INDEX,
				     &value);
#else
		rtk_stat_port_get(RTK_PORT_PON, DOT3_IN_PAUSE_FRAMES_INDEX, &value);
#endif
		return boaWrite(wp, "%llu", value);
	} else if (!strcmp(name, "temperature")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TEMPERATURE, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "voltage")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE,
					      &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE,
					   &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_VOLTAGE, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "tx-power")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER,
					   &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_TX_POWER, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "rx-power")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver);
		 _get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER,
					   &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_RX_POWER, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	} else if (!strcmp(name, "bias-current")) {
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver, &readableCfg);
#else
		rtk_ponmac_transceiver_get
		    (RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver);
		_get_data_by_type(RTK_TRANSCEIVER_PARA_TYPE_BIAS_CURRENT, &transceiver, &readableCfg);
#endif
		return boaWrite(wp, "%s", readableCfg.buf);
	}

	return -1;
}
