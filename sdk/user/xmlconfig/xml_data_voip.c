#include "xml_config_define.h"

#ifdef VOIP_SUPPORT
//struct voipCfgParam_s _voipCfgParam;
//struct voipCfgParam_s *voipCfgParam = &_voipCfgParam;
static int voipCfgParam_Idx=0;
extern voipCfgParam_t *voipCfgParam;

/* CHAIN VOIP */
//XML_DIR_SINGLE("VoiceService", voipCfgParam, root, MIB_VOIP_CFG_TBL);
XML_DIR_ARRAY("VoiceService", voipCfgParam, root, voipCfgParam_Idx, 1, MIB_VOIP_CFG_TBL);
XML_ENTRY_PRIMITIVE2("SIGNATURE", signature, voipCfgParam, "%ld", voipCfgParam->signature, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("VERSION", version, voipCfgParam, "%hd",voipCfgParam->version, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("FEATURE", feature, voipCfgParam, "%ld", voipCfgParam->feature, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("EXTEND_FEATURE", extend_feature, voipCfgParam, "%ld", voipCfgParam->extend_feature, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("MIB_VERSION", mib_version, voipCfgParam, "%hd", voipCfgParam->mib_version, 0);	//V_WORD
// RFC flags
XML_ENTRY_PRIMITIVE2("RFC_FLAGS", rfc_flags, voipCfgParam, "%d", voipCfgParam->rfc_flags, 0);	//V_UINT
// tone
XML_ENTRY_PRIMITIVE2("TONE_OF_COUNTRY", tone_of_country, voipCfgParam, "%hhd", voipCfgParam->tone_of_country, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TONE_OF_CUSTDIAL", tone_of_custdial, voipCfgParam, "%hhd", voipCfgParam->tone_of_custdial, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TONE_OF_CUSTRING", tone_of_custring, voipCfgParam, "%hhd", voipCfgParam->tone_of_custring, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TONE_OF_CUSTBUSY", tone_of_custbusy, voipCfgParam, "%hhd", voipCfgParam->tone_of_custbusy, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TONE_OF_CUSTWAITING", tone_of_custwaiting, voipCfgParam, "%hhd", voipCfgParam->tone_of_custwaiting, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TONE_OF_CUSTOMIZE", tone_of_customize, voipCfgParam, "%hhd", voipCfgParam->tone_of_customize, 0);	//V_BYTE
// customize tone DIR
static int ToneCfgParam_Idx = 0;
XML_DIR_ARRAY("CUST_TONE", ToneCfgParam, voipCfgParam, ToneCfgParam_Idx, TONE_CUSTOMER_MAX, 0); 	//V_MIB_LIST
XML_ENTRY_PRIMITIVE2("CUST_TONE_TYPE", toneType, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].toneType, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CYCLE", cycle, ToneCfgParam, "%hd", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].cycle, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_NUM", cadNUM, ToneCfgParam, "%hd", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].cadNUM, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_ON0", CadOn0, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOn0, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_ON1", CadOn1, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOn1, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_ON2", CadOn2, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOn2, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_ON3", CadOn3, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOn3, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_OFF0", CadOff0, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOff0, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_OFF1", CadOff1, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOff1, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_OFF2", CadOff2, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOff2, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_CAD_OFF3", CadOff3, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].CadOff3, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_PATTERN_OFF", PatternOff, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].PatternOff, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_NUM", ToneNUM, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].ToneNUM, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_FREQ0", Freq0, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Freq0, 0);   //V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_FREQ1", Freq1, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Freq1, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_FREQ2", Freq2, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Freq2, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_FREQ3", Freq3, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Freq3, 0);	//V_DWORD
XML_ENTRY_PRIMITIVE2("CUST_TONE_GAIN1", Gain1, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Gain1, 0);	//V_LONG
XML_ENTRY_PRIMITIVE2("CUST_TONE_GAIN2", Gain2, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Gain2, 0);	//V_LONG
XML_ENTRY_PRIMITIVE2("CUST_TONE_GAIN3", Gain3, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].Gain3, 0);	//V_LONG
//XML_ENTRY_PRIMITIVE2("CUST_TONE_DUMMY", dummy, ToneCfgParam, "%ld", voipCfgParam->cust_tone_para[ToneCfgParam_Idx].dummy, 0);	//V_LONG

// disconnect tone det
XML_ENTRY_PRIMITIVE2("DISTONE_NUM", distone_num, voipCfgParam, "%hhd", voipCfgParam->distone_num, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("D1FREQNUM", d1freqnum, voipCfgParam, "%hhd", voipCfgParam->d1freqnum, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("D1FREQ1", d1Freq1, voipCfgParam, "%hd", voipCfgParam->d1Freq1, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1FREQ2", d1Freq2, voipCfgParam, "%hd", voipCfgParam->d1Freq2, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1ACCUR", d1Accur, voipCfgParam, "%hhd", voipCfgParam->d1Accur, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("D1LEVEL", d1Level, voipCfgParam, "%hd", voipCfgParam->d1Level, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1ONUP", d1ONup, voipCfgParam, "%hd", voipCfgParam->d1ONup, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1ONLOW", d1ONlow, voipCfgParam, "%hd", voipCfgParam->d1ONlow, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1OFFUP", d1OFFup, voipCfgParam, "%hd", voipCfgParam->d1OFFup, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D1OFFLOW", d1OFFlow, voipCfgParam, "%hd", voipCfgParam->d1OFFlow, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2FREQNUM", d2freqnum, voipCfgParam, "%hhd", voipCfgParam->d2freqnum, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("D2FREQ1", d2Freq1, voipCfgParam, "%hd", voipCfgParam->d2Freq1, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2FREQ2", d2Freq2, voipCfgParam, "%hd", voipCfgParam->d2Freq2, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2ACCUR", d2Accur, voipCfgParam, "%hhd", voipCfgParam->d2Accur, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("D2LEVEL", d2Level, voipCfgParam, "%hd", voipCfgParam->d2Level, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2ONUP", d2ONup, voipCfgParam, "%hd", voipCfgParam->d2ONup, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2ONLOW", d2ONlow, voipCfgParam, "%hd", voipCfgParam->d2ONlow, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2OFFUP", d2OFFup, voipCfgParam, "%hd", voipCfgParam->d2OFFup, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("D2OFFLOW", d2OFFlow, voipCfgParam, "%hd", voipCfgParam->d2OFFlow, 0);	//V_WORD
// ring
XML_ENTRY_PRIMITIVE2("RING_CAD", ring_cad, voipCfgParam, "%hhd", voipCfgParam->ring_cad, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("RING_GROUP", ring_group, voipCfgParam, "%hhd", voipCfgParam->ring_group, 0);	//V_BYTE
static int ring_phone_num_Idx;
XML_ENTRY_PRIMITIVE_ARRAY2("RING_PHONE_NUM", ring_phone_num, voipCfgParam, "%u", voipCfgParam->ring_phone_num, ring_phone_num_Idx, RING_GROUP_MAX, 0);	//V_UINT_LIST
static int ring_cadence_use_Idx;
XML_ENTRY_PRIMITIVE_ARRAY2("RING_CADENCE_USE", ring_cadence_use, voipCfgParam, "%hhd", voipCfgParam->ring_cadence_use, ring_cadence_use_Idx, RING_GROUP_MAX, 0);	//V_BYTE_LIST
XML_ENTRY_PRIMITIVE2("RING_CADENCE_SEL", ring_cadence_sel, voipCfgParam, "%hhd", voipCfgParam->ring_cadence_sel, 0);	//V_BYTE
static int ring_cadon_Idx;
XML_ENTRY_PRIMITIVE_ARRAY2("RING_CAD_ON", ring_cadon, voipCfgParam, "%hd", voipCfgParam->ring_cadon, ring_cadon_Idx, RING_CADENCE_MAX, 0);	//V_WORD_LIST
static int ring_cadoff_Idx;
XML_ENTRY_PRIMITIVE_ARRAY2("RING_CAD_OFF", ring_cadoff, voipCfgParam, "%hd", voipCfgParam->ring_cadoff, ring_cadoff_Idx, RING_CADENCE_MAX, 0);	//V_WORD_LIST
// function key
XML_ENTRY_STRING2("FUNCKEY_PSTN", funckey_pstn, voipCfgParam, voipCfgParam->funckey_pstn, 0);	//V_STRING
XML_ENTRY_STRING2("FUNCKEY_TRANSFER", funckey_transfer, voipCfgParam, voipCfgParam->funckey_transfer, 0);	//V_STRING
// other
XML_ENTRY_PRIMITIVE2("AUTO_DIAL", auto_dial, voipCfgParam, "%hd", voipCfgParam->auto_dial, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("OFF_HOOK_ALARM", off_hook_alarm, voipCfgParam, "%hd", voipCfgParam->off_hook_alarm, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("CALLER_ID_AUTO_DET_SELECT", cid_auto_det_select, voipCfgParam, "%hd", voipCfgParam->cid_auto_det_select, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("CALLER_ID_DET_MODE", caller_id_det_mode, voipCfgParam, "%hd", voipCfgParam->caller_id_det_mode, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("ONE_STAGE_DIAL", one_stage_dial, voipCfgParam, "%hhd", voipCfgParam->one_stage_dial, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("TWO_STAGE_DIAL", two_stage_dial, voipCfgParam, "%hhd", voipCfgParam->two_stage_dial, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("AUTO_BYPASS_RELAY", auto_bypass_relay, voipCfgParam, "%hhd", voipCfgParam->auto_bypass_relay, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("PULSE_DIAL_GEN", pulse_dial_gen, voipCfgParam, "%hhd", voipCfgParam->pulse_dial_gen, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("PULSE_GEN_PPS", pulse_gen_pps, voipCfgParam, "%hhd", voipCfgParam->pulse_gen_pps, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("PULSE_GEN_MAKE_TIME", pulse_gen_make_time, voipCfgParam, "%hhd", voipCfgParam->pulse_gen_make_time, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("PULSE_GEN_INTERDIGIT_PAUSE", pulse_gen_interdigit_pause, voipCfgParam, "%hd", voipCfgParam->pulse_gen_interdigit_pause, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("PULSE_DIAL_DET", pulse_dial_det, voipCfgParam, "%hhd", voipCfgParam->pulse_dial_det, 0);	//V_BYTE
XML_ENTRY_PRIMITIVE2("PULSE_DET_PAUSE", pulse_det_pause, voipCfgParam, "%hd", voipCfgParam->pulse_det_pause, 0);	//V_WORD
// auto config
XML_ENTRY_PRIMITIVE2("AUTO_CFG_VER", auto_cfg_ver, voipCfgParam, "%hd", voipCfgParam->auto_cfg_ver, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("AUTO_CFG_MODE", auto_cfg_mode, voipCfgParam, "%hhd", voipCfgParam->auto_cfg_mode, 0);  //V_BYTE
XML_ENTRY_STRING2("AUTO_CFG_HTTP_ADDR", auto_cfg_http_addr, voipCfgParam, voipCfgParam->auto_cfg_http_addr, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("AUTO_CFG_HTTP_PORT", auto_cfg_http_port, voipCfgParam, "%hd", voipCfgParam->auto_cfg_http_port, 0);  //V_WORD
/*+++++added by Jack for auto provision for tftp and ftp+++++*/
XML_ENTRY_STRING2("AUTO_CFG_TFTP_ADDR", auto_cfg_tftp_addr, voipCfgParam, voipCfgParam->auto_cfg_tftp_addr, 0);  //V_STRING
XML_ENTRY_STRING2("AUTO_CFG_FTP_ADDR", auto_cfg_ftp_addr, voipCfgParam, voipCfgParam->auto_cfg_ftp_addr, 0);  //V_STRING
XML_ENTRY_STRING2("AUTO_CFG_FTP_USER", auto_cfg_ftp_user, voipCfgParam, voipCfgParam->auto_cfg_ftp_user, 0);  //V_STRING
XML_ENTRY_STRING2("AUTO_CFG_FTP_PASSWD", auto_cfg_ftp_passwd, voipCfgParam, voipCfgParam->auto_cfg_ftp_passwd, 0);  //V_STRING
/*-----end-----*/
XML_ENTRY_STRING2("AUTO_CFG_FILE_PATH", auto_cfg_file_path, voipCfgParam, voipCfgParam->auto_cfg_file_path, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("AUTO_CFG_EXPIRE", auto_cfg_expire, voipCfgParam, "%hd", voipCfgParam->auto_cfg_expire, 0);  //V_WORD
// Fw update setting
XML_ENTRY_PRIMITIVE2("FW_UPDATE_MODE", fw_update_mode, voipCfgParam, "%hhd", voipCfgParam->fw_update_mode, 0);  //V_BYTE
XML_ENTRY_STRING2("FW_UPDATE_TFTP_ADDR", fw_update_tftp_addr, voipCfgParam, voipCfgParam->fw_update_tftp_addr, 0);  //V_STRING
XML_ENTRY_STRING2("FW_UPDATE_HTTP_ADDR", fw_update_http_addr, voipCfgParam, voipCfgParam->fw_update_http_addr, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("FW_UPDATE_HTTP_PORT", fw_update_http_port, voipCfgParam, "%hd", voipCfgParam->fw_update_http_port, 0);  //V_WORD
XML_ENTRY_STRING2("FW_UPDATE_FTP_ADDR", fw_update_ftp_addr, voipCfgParam, voipCfgParam->fw_update_ftp_addr, 0);  //V_STRING
XML_ENTRY_STRING2("FW_UPDATE_FTP_USER", fw_update_ftp_user, voipCfgParam, voipCfgParam->fw_update_ftp_user, 0);  //V_STRING
XML_ENTRY_STRING2("FW_UPDATE_FTP_PASSWD", fw_update_ftp_passwd, voipCfgParam, voipCfgParam->fw_update_ftp_passwd, 0);  //V_STRING
XML_ENTRY_STRING2("FW_UPDATE_FILE_PATH", fw_update_file_path, voipCfgParam, voipCfgParam->fw_update_file_path, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("FW_UPDATE_POWER_ON", fw_update_power_on, voipCfgParam, "%hhd", voipCfgParam->fw_update_power_on, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("FW_UPDATE_SCHEDULING_DAY", fw_update_scheduling_day, voipCfgParam, "%hd", voipCfgParam->fw_update_scheduling_day, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("FW_UPDATE_SCHEDULING_TIME", fw_update_scheduling_time, voipCfgParam, "%hhd", voipCfgParam->fw_update_scheduling_time, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("FW_UPDATE_AUTO", fw_update_auto, voipCfgParam, "%hhd", voipCfgParam->fw_update_auto, 0);  //V_BYTE
XML_ENTRY_STRING2("FW_UPDATE_FILE_PREFIX", fw_update_file_prefix, voipCfgParam, voipCfgParam->fw_update_file_prefix, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("FW_UPDATE_NEXT_TIME", fw_update_next_time, voipCfgParam, "%ld", voipCfgParam->fw_update_next_time, 0);  //V_DWORD
XML_ENTRY_STRING2("FW_UPDATE_FW_VERSION", fw_update_fw_version, voipCfgParam, voipCfgParam->fw_update_fw_version, 0);  //V_STRING
// VLAN setting
XML_ENTRY_PRIMITIVE2("WAN_VLAN_ENABLE", wanVlanEnable, voipCfgParam, "%hhd", voipCfgParam->wanVlanEnable, 0);  //V_BYTE
// VLAN for Voice
XML_ENTRY_PRIMITIVE2("WAN_VLAN_ID_VOICE", wanVlanIdVoice, voipCfgParam, "%hd", voipCfgParam->wanVlanIdVoice, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("WAN_VLAN_PRIORITY_VOICE", wanVlanPriorityVoice, voipCfgParam, "%hhd", voipCfgParam->wanVlanPriorityVoice, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("WAN_VLAN_CFI_VOICE", wanVlanCfiVoice, voipCfgParam, "%hhd", voipCfgParam->wanVlanCfiVoice, 0);  //V_BYTE
// VLAN for Data
XML_ENTRY_PRIMITIVE2("WAN_VLAN_ID_DATA", wanVlanIdData, voipCfgParam, "%hd", voipCfgParam->wanVlanIdData, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("WAN_VLAN_PRIORITY_DATA", wanVlanPriorityData, voipCfgParam, "%hhd", voipCfgParam->wanVlanPriorityData, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("WAN_VLAN_CFI_DATA", wanVlanCfiData, voipCfgParam, "%hhd", voipCfgParam->wanVlanCfiData, 0);  //V_BYTE 
// VLAN for Video
XML_ENTRY_PRIMITIVE2("WAN_VLAN_ID_VIDEO", wanVlanIdVideo, voipCfgParam, "%hd", voipCfgParam->wanVlanIdVideo, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("WAN_VLAN_PRIORITY_VIDEO", wanVlanPriorityVideo, voipCfgParam, "%hhd", voipCfgParam->wanVlanPriorityVideo, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("WAN_VLAN_CFI_VIDEO", wanVlanCfiVideo, voipCfgParam, "%hhd", voipCfgParam->wanVlanCfiVideo, 0);  //V_BYTE
// VLAN others
XML_ENTRY_PRIMITIVE2("VLAN_ENABLE", vlan_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_TAG", vlan_tag, voipCfgParam, "%hd", voipCfgParam->vlan_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_BRIDGE_ENABLE", vlan_bridge_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_bridge_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_BRIDGE_TAG", vlan_bridge_tag, voipCfgParam, "%hd", voipCfgParam->vlan_bridge_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_BRIDGE_PORT", vlan_bridge_port, voipCfgParam, "%hd", voipCfgParam->vlan_bridge_port, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_BRIDGE_MULTICAST_ENABLE", vlan_bridge_multicast_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_bridge_multicast_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_BRIDGE_MULTICAST_TAG", vlan_bridge_multicast_tag, voipCfgParam, "%hd", voipCfgParam->vlan_bridge_multicast_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_HOST_ENABLE", vlan_host_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_host_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_HOST_TAG", vlan_host_tag, voipCfgParam, "%hd", voipCfgParam->vlan_host_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_HOST_PRI", vlan_host_pri, voipCfgParam, "%hd", voipCfgParam->vlan_host_pri, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_ENABLE", vlan_wifi_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_wifi_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_TAG", vlan_wifi_tag, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_PRI", vlan_wifi_pri, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_pri, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP0_ENABLE", vlan_wifi_vap0_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_wifi_vap0_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP0_TAG", vlan_wifi_vap0_tag, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap0_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP0_PRI", vlan_wifi_vap0_pri, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap0_pri, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP1_ENABLE", vlan_wifi_vap1_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_wifi_vap1_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP1_TAG", vlan_wifi_vap1_tag, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap1_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP1_PRI", vlan_wifi_vap1_pri, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap1_pri, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP2_ENABLE", vlan_wifi_vap2_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_wifi_vap2_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP2_TAG", vlan_wifi_vap2_tag, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap2_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP2_PRI", vlan_wifi_vap2_pri, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap2_pri, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP3_ENABLE", vlan_wifi_vap3_enable, voipCfgParam, "%hhd", voipCfgParam->vlan_wifi_vap3_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP3_TAG", vlan_wifi_vap3_tag, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap3_tag, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("VLAN_WIFI_VAP3_PRI", vlan_wifi_vap3_pri, voipCfgParam, "%hd", voipCfgParam->vlan_wifi_vap3_pri, 0);  //V_WORD
// HWNAT enable
XML_ENTRY_PRIMITIVE2("HWNAT_ENABLE", hwnat_enable, voipCfgParam, "%hhd", voipCfgParam->hwnat_enable, 0);  //V_BYTE
//Port Bandwidth Mgr
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT0_EGRES", bandwidth_LANPort0_Egress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort0_Egress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT1_EGRES", bandwidth_LANPort1_Egress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort1_Egress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT2_EGRES", bandwidth_LANPort2_Egress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort2_Egress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT3_EGRES", bandwidth_LANPort3_Egress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort3_Egress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_WANPORT_EGRES", bandwidth_WANPort_Egress, voipCfgParam, "%hd", voipCfgParam->bandwidth_WANPort_Egress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT0_INGRES", bandwidth_LANPort0_Ingress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort0_Ingress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT1_INGRES", bandwidth_LANPort1_Ingress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort1_Ingress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT2_INGRES", bandwidth_LANPort2_Ingress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort2_Ingress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_LANPORT3_INGRES", bandwidth_LANPort3_Ingress, voipCfgParam, "%hd", voipCfgParam->bandwidth_LANPort3_Ingress, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BANDWIDTH_WANPORT_INGRES", bandwidth_WANPort_Ingress, voipCfgParam, "%hd", voipCfgParam->bandwidth_WANPort_Ingress, 0);  //V_WORD
// FXO volume
XML_ENTRY_PRIMITIVE2("DAA_TX_VOLUME", daa_txVolumne, voipCfgParam, "%hhd", voipCfgParam->daa_txVolumne, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("DAA_RX_VOLUME", daa_rxVolumne, voipCfgParam, "%hhd", voipCfgParam->daa_rxVolumne, 0);  //V_BYTE
// DSCP
XML_ENTRY_PRIMITIVE2("RTP_DSCP", rtpDscp, voipCfgParam, "%hhd", voipCfgParam->rtpDscp, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SIP_DSCP", sipDscp, voipCfgParam, "%hhd", voipCfgParam->sipDscp, 0);  //V_BYTE
//eric add ,e8c
XML_ENTRY_STRING2("VOIP_INTERFACE", voip_interface, voipCfgParam, voipCfgParam->voip_interface, 0);	//V_STRING
XML_ENTRY_PRIMITIVE2("X_CT_SERVERTYPE", X_CT_servertype, voipCfgParam, "%hhd", voipCfgParam->X_CT_servertype, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("INTERDIGIT_LONG", InterDigitTimerLong, voipCfgParam, "%hd", voipCfgParam->InterDigitTimerLong, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("HANGINGTONE_TIMER", HangingReminderToneTimer, voipCfgParam, "%hd", voipCfgParam->HangingReminderToneTimer, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("BUSYTONE_TIMER", BusyToneTimer, voipCfgParam, "%hd", voipCfgParam->BusyToneTimer, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("NOANSWER_TIMER", NoAnswerTimer, voipCfgParam, "%hd", voipCfgParam->NoAnswerTimer, 0);  //V_WORD
// port setting
static int voipCfgPortParam_Idx;
XML_DIR_ARRAY("PORT", voipCfgPortParam, voipCfgParam, voipCfgPortParam_Idx, VOIP_PORTS, 0); 	//V_MIB_LIST
// port setting - proxies
static int voipCfgProxy_Idx;
XML_DIR_ARRAY("PROXIES", voipCfgProxy, voipCfgPortParam, voipCfgProxy_Idx, MAX_PROXY, 0); 	//V_MIB_LIST
XML_ENTRY_STRING2("DISPLAY_NAME", display_name, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].display_name, 0);	//V_STRING
XML_ENTRY_STRING2("NUMBER", number, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].number, 0);	//V_STRING
XML_ENTRY_STRING2("LOGIN_ID", login_id, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].login_id, 0);	//V_STRING
XML_ENTRY_STRING2("PASSWORD", password, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].password, 0);	//V_STRING
// proxy setting - register server
XML_ENTRY_PRIMITIVE2("ENABLE", enable, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].enable, 0);	//V_WORD
XML_ENTRY_STRING2("ADDR", addr, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].addr, 0);	//V_STRING
XML_ENTRY_PRIMITIVE2("PORT", port, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].port, 0);	//V_WORD
XML_ENTRY_STRING2("DOMAIN_NAME", domain_name, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].domain_name, 0);	//V_STRING
XML_ENTRY_PRIMITIVE2("REG_EXPIRE", reg_expire, voipCfgProxy, "%d", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].reg_expire, 0);	//V_UINT
// proxy setting - nat traversal server
XML_ENTRY_PRIMITIVE2("OUTBOUND_ENABLE", outbound_enable, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].outbound_enable, 0);	//V_WORD
XML_ENTRY_STRING2("OUTBOUND_ADDR", outbound_addr, voipCfgProxy, voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].outbound_addr, 0);	//V_STRING
XML_ENTRY_PRIMITIVE2("OUTBOUND_PORT", outbound_port, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].outbound_port, 0);	//V_WORD
// sip tls enable
XML_ENTRY_PRIMITIVE2("SIPTLS_ENABLE", siptls_enable, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].siptls_enable, 0);	//V_WORD
//rcm voip add
XML_ENTRY_PRIMITIVE2("HEARTBEAT_CYCLE", HeartbeatCycle, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].HeartbeatCycle, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("HEARTBEAT_COUNT", HeartbeatCount, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].HeartbeatCount, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("SESSION_UPDATE_TIMER", SessionUpdateTimer, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].SessionUpdateTimer, 0);	//V_WORD
XML_ENTRY_PRIMITIVE2("REG_RETRY", RegisterRetryInterval, voipCfgProxy, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].proxies[voipCfgProxy_Idx].RegisterRetryInterval, 0);	//V_WORD
// port setting - proxies end

XML_ENTRY_PRIMITIVE2("DEFAULT_PROXY", default_proxy, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].default_proxy, 0);	//V_BYTE
// port setting - NAT Traversal
XML_ENTRY_PRIMITIVE2("STUN_ENABLE", stun_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].stun_enable, 0);  //V_BYTE
XML_ENTRY_STRING2("STUN_ADDR", stun_addr, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].stun_addr, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("STUN_PORT", stun_port, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].stun_port, 0);  //V_WORD
// port setting - advanced
XML_ENTRY_PRIMITIVE2("SIP_PORT", sip_port, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].sip_port, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("MEDIA_PORT", media_port, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].media_port, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("DTMF_MODE", dtmf_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dtmf_mode, 0);  //V_BYTE

XML_ENTRY_PRIMITIVE2("DTMF_RFC2833_PAYLOAD_TYPE", dtmf_2833_pt, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].dtmf_2833_pt, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("DTMF_RFC2833_PI", dtmf_2833_pi, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].dtmf_2833_pi, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("FAXMODEM_RFC2833_PT_SAME_DTMF", fax_modem_2833_pt_same_dtmf, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].fax_modem_2833_pt_same_dtmf, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("FAXMODEM_RFC2833_PT", fax_modem_2833_pt, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].fax_modem_2833_pt, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("FAXMODEM_RFC2833_PI", fax_modem_2833_pi, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].fax_modem_2833_pi, 0);  //V_WORD





XML_ENTRY_PRIMITIVE2("SIP_INFO_DURATION", sip_info_duration, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].sip_info_duration, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("CALL_WAITING_ENABLE", call_waiting_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].call_waiting_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("FXSPORT_FLAG", fxsport_flag, voipCfgPortParam, "%ld", voipCfgParam->ports[voipCfgPortParam_Idx].fxsport_flag, 0);  //V_LONG
// port setting - forward
XML_ENTRY_PRIMITIVE2("UC_FORWARD_ENABLE", uc_forward_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].uc_forward_enable, 0);  //V_BYTE
XML_ENTRY_STRING2("UC_FORWARD", uc_forward, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].uc_forward, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("BUSY_FORWARD_ENABLE", busy_forward_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].busy_forward_enable, 0);  //V_BYTE
XML_ENTRY_STRING2("BUSY_FORWARD", busy_forward, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].busy_forward, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("NA_FORWARD_ENABLE", na_forward_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].na_forward_enable, 0);  //V_BYTE
XML_ENTRY_STRING2("NA_FORWARD", na_forward, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].na_forward, 0);  //V_STRING
XML_ENTRY_PRIMITIVE2("NA_FORWARD_TIME", na_forward_time, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].na_forward_time, 0);  //V_WORD
// port setting - speed dial
static int SpeedDialCfg_s_Idx;
XML_DIR_ARRAY("SPEED_DIAL", SpeedDialCfg_s, voipCfgPortParam, SpeedDialCfg_s_Idx, MAX_SPEED_DIAL, 0); 	//V_MIB_LIST
XML_ENTRY_STRING2("NAME", name, SpeedDialCfg_s, voipCfgParam->ports[voipCfgPortParam_Idx].speed_dial[SpeedDialCfg_s_Idx].name, 0);	//V_STRING
XML_ENTRY_STRING2("URL", url, SpeedDialCfg_s, voipCfgParam->ports[voipCfgPortParam_Idx].speed_dial[SpeedDialCfg_s_Idx].url, 0);	//V_STRING
// port setting - speed dial end

// port setting - dial plan
//#ifdef CONFIG_RTK_VOIP_DIALPLAN
XML_ENTRY_PRIMITIVE2("REPLACE_RULE_OPTION", replace_rule_option, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].replace_rule_option, 0);  //V_BYTE
XML_ENTRY_STRING2("REPLACE_RULE_SOURCE", replace_rule_source, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].replace_rule_source, 0);  //V_STRING
XML_ENTRY_STRING2("REPLACE_RULE_TARGET", replace_rule_target, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].replace_rule_target, 0);  //V_STRING
XML_ENTRY_STRING2("DIAL_PLAN", dialplan, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].dialplan, 0);  //V_STRING
//eric add
XML_ENTRY_PRIMITIVE2("DIGITMAP_ENABLE", digitmap_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].digitmap_enable, 0);  //V_BYTE

XML_ENTRY_STRING2("AUTO_PREFIX", auto_prefix, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].auto_prefix, 0);  //V_STRING
XML_ENTRY_STRING2("PREFIX_UNSET_PLAN", prefix_unset_plan, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].prefix_unset_plan, 0);  //V_STRING
//XML_ENTRY_PRIMITIVE2("DIAL_PLAN_TIME", dialplan_time, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].dialplan_time, 0);  //V_WORD
//#endif
// port setting - dial plan end

// port setting - codec
static int frame_size_Idx[VOIP_PORTS];
XML_ENTRY_PRIMITIVE_ARRAY2("FRAME_SIZE", frame_size, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].frame_size, frame_size_Idx[voipCfgPortParam_Idx], _CODEC_MAX, 0);	//V_BYTE_LIST
static int precedence_Idx[VOIP_PORTS];
XML_ENTRY_PRIMITIVE_ARRAY2("PRECEDENCE", precedence, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].precedence, precedence_Idx[voipCfgPortParam_Idx], _CODEC_MAX, 0);	//V_BYTE_LIST
XML_ENTRY_PRIMITIVE2("VAD", vad, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].vad, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("VAD_THRESHOLD", vad_thr, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].vad_thr, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("CNG", cng, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].cng, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("CNG_THRESHOLD", cng_thr, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].cng_thr, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SID_MODE", sid_gainmode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].sid_gainmode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SID_LEVEL", sid_noiselevel, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].sid_noiselevel, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SID_GAIN", sid_noisegain, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].sid_noisegain, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("PLC", PLC, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].PLC, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("RTCP_INTERVAL", RTCP_Interval, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].RTCP_Interval, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("RTCPXR", RTCP_XR, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].RTCP_XR, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("G7231_RATE", g7231_rate, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].g7231_rate, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ILBC_MODE", iLBC_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].iLBC_mode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPEEX_NB_RATE", speex_nb_rate, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].speex_nb_rate, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("G726_PACK", g726_packing, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].g726_packing, 0);  //V_BYTE
// port setting - RTP redundant 
XML_ENTRY_PRIMITIVE2("RTP_RED_PAYLOAD_TYPE", rtp_redundant_payload_type, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].rtp_redundant_payload_type, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("RTP_RED_CODEC", rtp_redundant_codec, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].rtp_redundant_codec, 0);  //V_BYTE
// port setting - DSP
XML_ENTRY_PRIMITIVE2("SLIC_TX_VOLUME", slic_txVolumne, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].slic_txVolumne, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SLIC_RX_VOLUME", slic_rxVolumne, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].slic_rxVolumne, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("JITTER_DELAY", jitter_delay, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].jitter_delay, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MAX_DELAY", maxDelay, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].maxDelay, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("JITTER_FACTOR", jitter_factor, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].jitter_factor, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("LEC", lec, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].lec, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("NLP", nlp, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].nlp, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ECHO_TAIL", echoTail, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].echoTail, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("CALLER_ID_MODE", caller_id_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].caller_id_mode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("CALL_WAITING_CID", call_waiting_cid, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].call_waiting_cid, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("CID_DTMF_MODE", cid_dtmf_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].cid_dtmf_mode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPEAKERAGC", speaker_agc, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].speaker_agc, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPK_AGC_LVL", spk_agc_lvl, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].spk_agc_lvl, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPK_AGC_GU", spk_agc_gu, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].spk_agc_gu, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPK_AGC_GD", spk_agc_gd, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].spk_agc_gd, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MICAGC", mic_agc, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].mic_agc, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MIC_AGC_LVL", mic_agc_lvl, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].mic_agc_lvl, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MIC_AGC_GU", mic_agc_gu, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].mic_agc_gu, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MIC_AGC_GD", mic_agc_gd, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].mic_agc_gd, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("FSK_GEN_MODE", cid_fsk_gen_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].cid_fsk_gen_mode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SPK_VOICE_GAIN", spk_voice_gain, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].spk_voice_gain, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("MIC_VOICE_GAIN", mic_voice_gain, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].mic_voice_gain, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ANSTONE", anstone, voipCfgPortParam, "%d", voipCfgParam->ports[voipCfgPortParam_Idx].anstone, 0);  //V_DWORD
XML_ENTRY_PRIMITIVE2("FAX_MODEM_RFC2833", faxmodem_rfc2833, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].faxmodem_rfc2833, 0);  //V_BYTE
// QoS
XML_ENTRY_PRIMITIVE2("VOICE_QOS", voice_qos, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].voice_qos, 0);  //V_BYTE
//T.38
XML_ENTRY_PRIMITIVE2("T38_ENABLE", useT38, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].useT38, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_PORT", T38_port, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].T38_port, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("FAX_MODEM_DET", fax_modem_det, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].fax_modem_det, 0);  //V_BYTE
//T.38 parameters 
XML_ENTRY_PRIMITIVE2("T38_PARAM_ENABLE", T38ParamEnable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38ParamEnable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_MAX_BUFFER", T38MaxBuffer, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].T38MaxBuffer, 0);  //V_SHORT
XML_ENTRY_PRIMITIVE2("FAX_RATE_MGT", T38RateMgt, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38RateMgt, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_MAX_RATE", T38MaxRate, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38MaxRate, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_ENABLE_ECM", T38EnableECM, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38EnableECM, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_ECC_SIGNAL", T38ECCSignal, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38ECCSignal, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_ECC_DATA", T38ECCData, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38ECCData, 0);  //V_BYTE

XML_ENTRY_PRIMITIVE2("T38_ENABLE_SPOOF", T38EnableSpoof, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38EnableSpoof, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("T38_DUPLICATE_NUM", T38DuplicateNum, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].T38DuplicateNum, 0);  //V_BYTE


// V.152
XML_ENTRY_PRIMITIVE2("V152_ENABLE", useV152, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].useV152, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("V152_PAYLOAD_TYPE", v152_payload_type, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].v152_payload_type, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("V152_CODEC_TYPE", v152_codec_type, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].v152_codec_type, 0);  //V_BYTE
// Hot Line
XML_ENTRY_PRIMITIVE2("HOTLINE_ENABLE", hotline_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].hotline_enable, 0);  //V_BYTE
XML_ENTRY_STRING2("HOTLINE_NUMBER", hotline_number, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].hotline_number, 0);  //V_STRING
// DND
XML_ENTRY_PRIMITIVE2("DND_MODE", dnd_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dnd_mode, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("DND_FROM_HOUR", dnd_from_hour, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dnd_from_hour, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("DND_FROM_MIN", dnd_from_min, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dnd_from_min, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("DND_TO_HOUR", dnd_to_hour, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dnd_to_hour, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("DND_TO_MIN", dnd_to_min, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].dnd_to_min, 0);  //V_BYTE

// flash hook time
XML_ENTRY_PRIMITIVE2("FLASH_HOOK_TIME", flash_hook_time, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].flash_hook_time, 0);  //V_WORD
XML_ENTRY_PRIMITIVE2("FLASH_HOOK_TIME_MIN", flash_hook_time_min, voipCfgPortParam, "%hd", voipCfgParam->ports[voipCfgPortParam_Idx].flash_hook_time_min, 0);  //V_WORD

// security
/* +++++Add by Jack for VoIP security 240108+++++ */
//#ifdef CONFIG_RTK_VOIP_SRTP
XML_ENTRY_PRIMITIVE2("SECURITY_ENABLE", security_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].security_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("SECURITY_KEY_EXCHANGE_MODE", key_exchange_mode, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].key_exchange_mode, 0);  //V_BYTE
//#endif /*CONFIG_RTK_VOIP_SRTP*/
/*-----end-----*/

// auth
XML_ENTRY_STRING2("OFFHOOK_PASSWD", offhook_passwd, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].offhook_passwd, 0);  //V_STRING

// port setting - abbreviated dial
//{MIB_VOIP_ABBREVIATED_DIAL, "ABBREVIATED_DIAL", V_MIB_LIST, VOIP_PORT_MIB_LIST_FIELD(abbr_dial, mibtbl_abbreviated_dial)},
// port setting - abbreviated dial end

// port setting - alarm
XML_ENTRY_PRIMITIVE2("ALARM_ENABLE", alarm_enable, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].alarm_enable, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ALARM_TIME_HH", alarm_time_hh, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].alarm_time_hh, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ALARM_TIME_MM", alarm_time_mm, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].alarm_time_mm, 0);  //V_BYTE
#if 0
XML_ENTRY_PRIMITIVE2("ALARM_RING_LAST_DAY", alarm_ring_last_day, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].alarm_ring_last_day, 0);  //V_BYTE
XML_ENTRY_PRIMITIVE2("ALARM_RING_DEFER", alarm_ring_defer, voipCfgPortParam, "%hhd", voipCfgParam->ports[voipCfgPortParam_Idx].alarm_ring_defer, 0);  //V_BYTE
#endif
// port setting - PSTN routing prefix
XML_ENTRY_STRING2("PSTN_ROUTING_PREFIX", PSTN_routing_prefix, voipCfgPortParam, voipCfgParam->ports[voipCfgPortParam_Idx].PSTN_routing_prefix, 0);  //V_STRING
// port setting end
#endif //VOIP_SUPPORT 
