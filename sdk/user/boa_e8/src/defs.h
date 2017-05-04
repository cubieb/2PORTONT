#ifndef WEB_LANGUAGE_DEFS_H
#define WEB_LANGUAGE_DEFS_H
#include "LINUX/options.h"
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../include/linux/autoconf.h"
#endif

#define Tfast_config	"Fast Configure"
#define Tfc_intro     "the fast configuration will help you finish the configuration step by step."
#define Tattention    "attention:"
#define Tatt_intro    "original configuration will be replaced by the fast configuration."
#define Tint_conn_conf  "Internet Connection Configure -- ATM PVC config"
#define Tint_conn_intro "please set VPI and VCI. Do not modify it, unless ISP need modify it."
#define Tnext_step    "next step >"
#define Tint_conn_type  "Internet Connection Configure -- connection type"
#define Tconn_type_intro    "please select WAN connection type provided by ISP."
#define Tconn_type    "WAN connection type:"
#define Tencap_type   "encapsulation:"
#define Tenter_vpi	"please Enter VPI (0~255)"
#define Tinvalid_vpi	"invalid VPI!"
#define Tenter_vci	"please enter VCI (0~65535)"
#define Tinvalid_vci	"invalid VCI!"
#define Tget_chain_err	"Get chain record error!\n"
#define Tconn_exist	"Connection already exists, delete existed connection?"
#define Tvc_exceed	"Error: Maximum number of VC exceeds !"
#define Tadd_err		"Error! Add chain record."
#define Tsecond_ip_err		"Set second IP-address error!"
#define Tsecond_mask_err	"Set second subnet-mask error!"
#define Tdhcp_err			"Set dhcp mode MIB error!"
#define Tget_ip_err		"Get IP-address error!"
#define Tget_mask_err	"Get Subnet Mask error!"
#define Tdhcp_range_err	"Invalid DHCP client range!"
#define Tset_start_ip_err		"Set DHCP client start range error!"
#define Tset_end_ip_err		"Set DHCP client end range error!"
#define Tset_lease_err		"Set DHCP lease time error!"
#define Tset_ip_err		"Set IP-address error!"
#define Tset_mask_err	"Set net mask error!"
#define Tinvalid_ip		"Invalid IP-address value!"
#define Tinvalid_mask	"Invalid subnet-mask value!"
#define Tinvalid_ip2		"Invalid second IP-address value!"
#define Tinvalid_mask2	"Invalid second subnet-mask value!"
#define Tip2vsip		"first Lan and second Lan must belong to different subNet!"
#define Tinvalid_start_ip	"Invalid DHCP start IP-address value!"
#define Tinvalid_end_ip	"Invalid DHCP end IP-address value!"
#define Tinvalid_dhcp_ip	"DHCP address range error!"
#define Tuser_long		"Error! User Name length too long."
#define Tpassw_long	"Error! Password length too long."
#define Tinvalid_ppp_type	"Invalid PPP type value!"
#define Tconn_forever	", connect forever"
#define Tconn_on_dem	", connect on demand, idle for %d minute"
#define Tconn_manual	", connect manually"
#define Tdd_hh_mm		"%dday %dhour %dmin"
#define Tdhcp_ip_range	"DHCP IP Range"
#define Tdhcp_lease_t	"DHCP Lease Time"
#define Tnapt			"NAPT"
#define Twan_ip		"WAN IP"
#define Tres_gw		"Reserved Gateway"
#define Tauto_assign	"auto assigned"
#define Tno_wan_ip		"none"
#define Tdnss			"DNS Server"
#define Tbytes			"data bytes"
#define Tping_recv		"%d bytes from %s: icmp_seq=%u"
#define Tping_stat		"--- ping statistics ---"
#define Ttrans_pkt		"%ld packets transmitted, "
#define Trecv_pkt		"%ld packets received."
#define Tno_conn		"Connection not exists!"
#define Trecv_cell_suc	"Loopback cell接收成功!" //Loopback cell received successfully !
#define Trecv_cell_fail	"Loopback 失败!" //Loopback failed !
#define Tadsl_diag_suc	"ADSL Diagnostics successful !!"
#define Tadsl_diag_fail	"ADSL Diagnostics failed !!"
#define Tdownstream		"Downstream"
#define Tupstream		"Upstream"
#define Tloop_annu		"Loop Attenuation"
#define Tsig_annu		"Signal Attenuation"
#define Thlin_scale		"Hlin Scale"
#define Tsnr_marg		"SNR Margin"
#define Tattain_rate		"Attainable Rate"
#define Tout_power		"Output Power"
#define Ttone_num		"Tone Number"
#define Tlan_conn_chk	"LAN Connection Check"
#define Ttest_eth_conn	"Test Ethernet LAN Connection"
#define Ttest_adsl_syn	"Test ADSL Synchronization"
#define Ttest_oam_f5_seg	"Test ATM OAM F5 Segment Loopback"
#define Ttest_oam_f5_end 	"Test ATM OAM F5 End-to-end Loopback"
#define Ttest_oam_f4_seg	"Test ATM OAM F4 Segment Loopback"
#define Ttest_oam_f4_end	"Test ATM OAM F4 End-to-end Loopback"
#define Tadsl_conn_chk	"ADSL Connection Check"
#define Tint_conn_chk 	"Internet Connection Check"
#define Ttest_ppps_conn	"Test PPP Server Connection"
#define Ttest_auth		"Test Authentication with ISP"
#define Ttest_assigned_ip	"Test the assigned IP Address"
#define Tping_pri_dnss	"Ping Primary Domain Name Server"
#define Tback			"  Back  "
#define Tping_def_gw	"Ping Default Gateway"
#define Tadsl_diag_wait	"The test result will come out 3 minutes later and the page will refresh itself automatically. Please wait ...\n"
#define Tppp_conn_excd	"Maximum number of PPPoE connections exceeds in this vc!"
#define Tdef_gw_exist	"Default route already exists !"
#define Tinvalid_wan_dhcp	"Invalid WAN DHCP value!"
#define Tinvalid_gw		"Invalid reserved gateway IP-address value!"
#define Tinvalid_dns_mode	"Invalid DNS mode value!"
#define Tinvalid_dns	"Invalid DNS address!"
#define Treboot_wd0		"The System is Restarting ..."
#define Treboot_wd1		"The DSL bridge has been configured and is rebooting."
#define Treboot_wd2		"Close the DSL Router Configuration window and wait"\
			" for 2 minutes before reopening your web browser."\
			" If necessary, reconfigure your PC's IP address to match"\
			" your new configuration.";
#define Ttime_zone1		"System Date has been modified successfully."
#define Ttime_zone2		"Please reflesh your \"Status\" page."
#define Tset_tz			"set time zone error!"
#define Tset_date_err	"cannot set date"
#define Tset_ntp_ena	"Set enabled flag error!"
#define Tset_ntp_svr	"Set NTP server error!"
#define Tset_ntp_ip		"Set NTP server IP error!"
#define Tbrg_not_exist	"br0 interface not exists !!"
#define Tyes			"yes"
#define Tno				"no"
#define Tport_err		"port size error!"
#define Twan_port		"wan port set error!"
#define Tsvr_port		"server port set error!"
#define Tvrtsrv_rule	"execute virtual server rules failed!\n"
#define Tdmz_error		"Invalid IP address! It should be set within the current subnet."

#define strWrongIP "不合法的IP地址!" //"Invalid IP-address value!"
#define strSetIPerror "Set IP-address error!"
#define strWrongMask "不合法子网路遮罩!" //"Invalid subnet-mask value!"
#define strSetMaskerror "Set subnet-mask error!"

#define strGetDhcpModeerror "Get DHCP MIB error!"
#define strGetIperror "Get IP-address error!"
#define strGetMaskerror "Get Subnet Mask error!"
#define strInvalidRange "不合法的DHCP client范围" //"Invalid DHCP client range!"
#define strSetStarIperror "Set DHCP client start range error!"
#define strSetEndIperror "Set DHCP client end range error!"
#define strSetLeaseTimeerror "DHCP租约设定错误!" //"Set DHCP lease time error!"
#define strSetDomainNameerror "Set DHCP Domain Name error!"
#define strInvalidGatewayerror "不合法的DHCP Server网关地址!" //"Invalid DHCP Server Gateway Address value!"
#define strSetGatewayerror "Set DHCP Server Gateway Address error!"
#define strSetDhcpModeerror "DHCP设定错误!" //"Set DHCP error!"

#if defined(IP_BASED_CLIENT_TYPE) || defined(_PRMT_X_TELEFONICA_ES_DHCPOPTION_)
#define strInvalidRangepc "invalid pc IP pool range!"
#define strInvalidRangecmr "invalid camera IP pool range!"
#define strInvalidRangestb "invalid stb IP pool range!"
#define strInvalidRangephn "invalid phone IP pool range!"
#define strInvalidTypeRange "设备地址应在DHCP地址池的范围内"
#define strOverlapRange "设备地址不应重叠"
#define strInvalidNstbRange "address for every equipment that is not a STB: From .33 until .200"
#define strInvalidStbRage "For STB, it can use addresses from .200 on !"
#define strInvalidOpt60	"option 60 can not be NULL!"
#define strInvalidOpchaddr	"invalid opch address"
#define strInvalidOpchport	"invalid opch port"

#define strSetPcStartIperror "set pc IP pool start address failed!"
#define strSetPcEndIperror "set pc IP pool end address failed!"
#define strSetCmrStartIperror "set camera IP pool start address failed!"
#define strSetCmrEndIperror "set camera IP pool end address failed!"
#define strSetStbStartIperror "set stb IP pool start address failed!"
#define strSetStbEndIperror "set stb IP pool end address failed!"
#define strSetPhnStartIperror "set phone IP pool start address failed!"
#define strSetPhnEndIperror "set phone IP pool end address failed!"
#endif

#define strDelChainerror "chain record删除错误!" //Delete chain record error!
#define strModChainerror "chain record修改错误!" //Modify chain record error!

#define strInvalDhcpsAddress "Invalid DHCPS address value!"
#define strSetDhcpserror "Set DHCPS MIB error!"

#define strStaticipexist "This static ip configuration already exists!"

#define strConnectExist "Connection already exists!"
#define strMaxVc "Error: Maximum number of VC exceeds !"
#define strAddChainerror "Error! Add chain record."
#define strTableFull "Error! Table Full."

#define strUserNametoolong "Error! User Name length too long."
#define strUserNameempty "Error! User Name cannot be empty."
#define strPasstoolong "Error! Password length too long."
#define strPassempty "Error! Password cannot be empty."
#define strInvalPPPType "Invalid PPP type value!"
#define strDrouteExist  "Default route already exists !"
#define strACName "Error! AC-Name length too long."
#define strServerName "Error! Service-Name length too long."

#define strInvalDHCP "Invalid WAN DHCP value!"
#define strInvalIP "Invalid IP-address value!"
#define strIPAddresserror "Error! IP address cannot be empty."
#define strInvalGateway "Invalid remote IP-address value!"
#define strGatewayIpempty "Error! Remote IP address cannot be empty."
#define strInvalMask "Invalid subnet mask value!"
#define strMaskempty "Error! Subnet mask cannot be empty."
#define strMruErr	"Invalid MTU!"

#define strMaxNumPPPoE "Maximum number of PPPoE connections exceeds in this vc!"



#define strSelectvc "Please select one vc!"
#define strGetChainerror "Get chain record error!\n"
#define strInvalidQos "Invalid QoS!"
#define strInvalidPCR "Invalid PCR!"
#define strInvalidCDVT "Invalid CDVT!"
#define strInvalidSCR "Invalid SCR!"
#define strInvalidMBS "Invalid MBS!"

#define strSetcommunityROerror "Set snmpCommunityRO mib error!"
#define strSetcommunityRWerror "Set snmpCommunityRW mib error!"
#define strInvalTrapIp "Invalid Trap IP value!"
#define strSetTrapIperror "Set snmpTrapIpAddr mib error!"


#define strGetToneerror "ERROR: Get ADSL Tone MIB error!"
#define  WARNING_EMPTY_OLD_PASSWORD  "ERROR: Old Password cannot be empty."
#define  WARNING_EMPTY_NEW_PASSWORD  "ERROR: New Password cannot be empty."
#define WARNING_EMPTY_CONFIRMED_PASSWORD  "ERROR: Confirmed Password cannot be empty."
#define WARNING_UNMATCHED_PASSWORD  "ERROR: New Password is not the same as Confirmed Password."
#define WARNING_GET_PASSWORD  "ERROR,Get password failed!"
#define WARNING_SET_PASSWORD  "ERROR: Modify password failed!"
#define WARNING_WRONG_PASSWORD  "ERROR: wrong password!"
#define WARNING_WRONG_USER  "ERROR: wrong username!"


#define COMMIT_REBOOT_TITLE    "Commit and Reboot"
#define COMMIT_REBOOT_INFO " This page is used to commit changes to system memory and reboot your system."
#define COMMIT_REBOOT_BUTTON  "<input type=\"submit\" value=\"Commit and Reboot\" name=\"save\" onclick=\"return saveClick()\">"

#define SYSTEM_LOG_TITLE "System Log"

//for wireless setting
#define   strDisbWlanErr				"关闭错误!" //Set disabled flag error!
#define   strInvdTxPower			"不合法的TxPower值!" //Invalid TxPower value!
#define   strSetMIBTXPOWErr		"MIB_TX_POWER设定错误!" //Set MIB_TX_POWER error!
#define   strInvdMode				"设定模式错误!" //Invalid mode value!
#define   strSetWPAWarn			"无法设置client模式在WPA-RADIUS之下!<br><br>请在安全页面更改加密模式" //You cannot set client mode with WPA-RADIUS!<br><br>Please change the encryption method in security page first.
#define   strSetWEPWarn			"无法设置client模式在WEP-802.1x之下!<br><br>请在安全页面更改加密模式" //You cannot set client mode with WEP-802.1x!<br><br>Please change the encryption method in security page first.
#define   strSetMIBWLANMODEErr		"MIB_WLAN_MODE设定错误!" //Set MIB_WLAN_MODE error!
#define   strSetSSIDErr				"SSID设定错误!" //Set SSID error!
#define   strInvdChanNum			"不合法的频道!"  //Invalid channel number!
#define   strSetChanErr				"频道设定错误!" //Set channel number error!
#define   strInvdNetType			"不合法的网路种类!" //Invalid network type value!
#define   strSetMIBWLANTYPEErr		"MIB_WLAN_NETWORK_TYPE设定错误!" //Set MIB_WLAN_NETWORK_TYPE failed!
#define   strInvdAuthType			"错误! 不合法的认证模式" //Error! Invalid authentication value.
#define   strSetAuthTypeErr			"认证设定错误!" //Set authentication failed!
#define   strSetAdapRateErr			"rate adaptive设定错误!" //Set rate adaptive failed!
#define   strSetFixdRateErr			"fix rate设定错误!" //Set fix rate failed!
#define   strSetBaseRateErr			"Tx basic rate设定错误!" //Set Tx basic rate failed!
#define   strSetOperRateErr			"Tx operation rate设定错误!" //Set Tx operation rate failed!
#define   strInvdTxRate				"不合法的tx rate值!" //invalid value of tx rate!
#define   strInvdBrodSSID			"错误! 不合法的hiddenSSID值" //Error! Invalid hiddenSSID value.
#define   strSetBrodSSIDErr			"hidden ssid设定错误!" //Set hidden ssid failed!
#define   strInvdProtection             	"不合法的protection设定"      //add by yq_zhou 2.10 //Invalid protection setting
#define   strInvdAggregation	                "不合法的aggregation设定" //Invalid aggregation setting
#define   strInvdShortGI0 	                "不合法的short GI设定" //Invalid short GI setting
#define   strSetProtectionErr          	"protection设定错误" //Set protection error
#define   strSetAggregationErr	  	"aggregation设定错误" //Set aggregation  error
#define   strSetShortGI0Err			"guard interval设定错误" //Set guard interval error
#define   strSetChanWidthErr  		"channel width设定错误" //Set channel width error
#define   strSet11NCoexistErr  		"11N co-existence设定错误" //Set 11N co-existence error
#define   strSetCtlBandErr 			"control sideband设定错误" //Set control sideband error
#define   strSetBandErr				"band设定错误!" //Set band error!
#define   strEnabAccCtlErr			"启用access control设定错误!" //Set enabled access control error!
#define   strInvdMACAddr			"不合法的MAC位址" //Error! Invalid MAC address.
#define   strAddAcErrForFull			"条目数已达上限!" //Cannot add new entry because table is full!
#define   strMACInList				"条目已存在!" //Entry already exists!
#define   strAddListErr				"错误! 加入chain record" //Error! Add chain record.
#define   strDelListErr				"删除chain record错误!" //Delete chain record error!
#define   strFragThreshold			"错误! 不合法的fragment threshold" //Error! Invalid value of fragment threshold.
#define   strSetFragThreErr			"设定fragment threshold错误!" //Set fragment threshold failed!
#define   strRTSThreshold			"错误! 不合法的RTS threshold" //Error! Invalid value of RTS threshold.
#define   strSetRTSThreErr			"RTS threshold设定错误!" //Set RTS threshold failed!
#define   strInvdBeaconIntv			"错误! 不合法的Beacon Interval" //Error! Invalid value of Beacon Interval.
#define   strSetBeaconIntvErr		"Beacon interval设定错误!" //Set Beacon interval failed!
#define   strInvdPreamble			"错误! 不合法的Preamble值" //Error! Invalid Preamble value.
#define   strSetPreambleErr			"Preamble设定错误!" //Set Preamble failed!
#define   strInvdDTIMPerd			"错误! 不合法的DTIM周期" //Error! Invalid value of DTIM period.
#define   strSetDTIMErr				"DTIM周期设定失败!" //Set DTIM period failed!
#define   strInvdIAPP				"错误! 不合法的IAPP值" //Error! Invalid IAPP value.
#define   strMIBIAPPDISBErr			"MIB_WLAN_IAPP_DISABLED设定失败!" //Set MIB_WLAN_IAPP_DISABLED failed!
#define   strSetRelayBlockErr			"block relay设定失败!" //Set block relay failed!
#define   strSetLanWlanBlokErr		"block ethernet to wireless设定失败!" //Set block ethernet to wireless failed!
#define   strSetWlanWMMErr			"WMM(QoS) to wireless设定失败!" //Set WMM(QoS) to wireless failed!
#define   strMACAddr				"MAC地址" //MAC Address
#define   strSelect					"选择" //Select
#define   strGetMBSSIBTBLErr		"错误! MIB_MBSSIB_TBL读取错误" //Error! Get MIB_MBSSIB_TBL error.
#define   strGetVAPMBSSIBTBLErr		"错误! formWlanMBSSID: MIB_MBSSIB_TBL for VAP SSID 读取错误" //Error! formWlanMBSSID: Get MIB_MBSSIB_TBL for VAP SSID error.
#define   strGetMULTIAPTBLErr		"错误! formWlanMultipleAP: MIB_MBSSIB_TBL for VAP 读取错误" //Error! formWlanMultipleAP: Get MIB_MBSSIB_TBL for VAP error.
#define   strNotSuptSSIDType		"错误! 无法支援此SSID种类" //Error! Not support this SSID TYPE.
#define   strNoSSIDTypeErr			"错误! 无此SSID种类" //Error! no SSID TYPE.
#define   strNoEncryptionErr			"错误! 无此加密模式" //Error! no encryption method.
#define   strInvdEncryptErr			"不合法的加密模式!" //Invalid encryption method!
#define   strSetWLANENCRYPTErr		"MIB_WLAN_ENCRYPT设定错误!" //Set MIB_WLAN_ENCRYPT mib error!
#define   strSet8021xWarning		"错误! 802.1x认证无法用于client模式" //Error! 802.1x authentication cannot be used when device is set to client mode.
#define   strSet1xEnabErr			"1x enable flag设定错误!" //Set 1x enable flag error!
#define   strGetWLANWEPErr			"MIB_WLAN_WEP MIB 读取错误!" //Get MIB_WLAN_WEP MIB error!
#define   strSetWLANWEPErr			"WEP MIB设定错误!" //Set WEP MIB error!
#define   strSetWPARADIUSWarn		"错误! WPA-RADIUS认证无法用于client模式" //Error! WPA-RADIUS cannot be used when device is set to client mode.
#define   strInvdWPAAuthValue		"错误! 不合法的wpa认证值" //Error! Invalid wpa authentication value.
#define   strSetAUTHTYPEErr			"MIB_WLAN_AUTH_TYPE设定错误!" //Set MIB_WLAN_AUTH_TYPE failed!
#define   strNoPSKFormat			"错误! 没有psk格式" //Error! no psk format.
#define   strInvdPSKFormat			"错误! 不合法的psk格式" //Error! invalid psk format.
#define   strSetWPAPSKFMATErr		"MIB_WLAN_WPA_PSK_FORMAT设定错误!" //Set MIB_WLAN_WPA_PSK_FORMAT failed!
#define   strInvdPSKValue			"不合法的psk值" //Error! invalid psk value.
#define   strSetWPAPSKErr			"MIB_WLAN_WPA_PSK设定错误!" //Set MIB_WLAN_WPA_PSK error!
#define   strInvdRekeyDay			"错误! 不合法的rekey day值" //Error! Invalid value of rekey day.
#define   strInvdRekeyHr			"错误! 不合法的rekey hr值" //Error! Invalid value of rekey hr.
#define   strInvdRekeyMin			"错误! 不合法的rekey min值" //Error! Invalid value of rekey min.
#define   strInvdRekeySec			"错误! 不合法的rekey sec值" //Error! Invalid value of rekey sec.
#define   strSetREKEYTIMEErr		"MIB_WLAN_WPA_GROUP_REKEY_TIME设定错误!" //Set MIB_WLAN_WPA_GROUP_REKEY_TIME error!
#define   strInvdRSPortNum			"错误! 不合法的RS端口数值" //Error! Invalid value of RS port number.
#define   strSetRSPortErr			"RS端口设定错误!" //Set RS port error!
#define   strNoIPAddr				"没有RS IP位址!" //No RS IP address!
#define   strInvdRSIPValue			"不合法的RS IP位址!" //Invalid RS IP-address value!
#define   strSetIPAddrErr			"RS IP位址设定错误!" //Set RS IP-address error!
#define   strRSPwdTooLong			"RS密码过长!" //RS password length too long!
#define   strSetRSPwdErr			"RS密码设定错误!" //Set RS password error!
#define   strInvdRSRetry			"不合法的RS retry值!" //Invalid RS retry value!
#define   strSetRSRETRYErr			"MIB_WLAN_RS_RETRY设定错误!" //Set MIB_WLAN_RS_RETRY error!
#define   strInvdRSTime				"不合法的RS时间值!" //Invalid RS time value!
#define   strSetRSINTVLTIMEErr		"MIB_WLAN_RS_INTERVAL_TIME设定错误!" //Set MIB_WLAN_RS_INTERVAL_TIME error!
#define   strInvdWepKeyLen			"错误! 不合法的wepkeylen值" //Error! Invalid wepkeylen value.
#define   strSetMIBWLANWEPErr		"MIB_WLAN_WEP设定错误!" //Set MIB_WLAN_WEP failed!
#define   strGetMBSSIBTBLUpdtErr	"错误! MIB_MBSSIB_TBL 读取错误(Updated)." //Error! Get MIB_MBSSIB_TBL error(Updated).
#define   strGetMBSSIBWEPTBLErr	"错误! MIB_MBSSIB_WEP_TBL 读取错误" //Error! Get MIB_MBSSIB_WEP_TBL error.
#define   strKeyLenMustExist			"Key length必须存在!" //Key length must exist!
#define   strInvdKeyLen				"不合法的key length值!" //Invalid key length value
#define   strKeyTypeMustExist		"Key type必须存在!" //Key type must exist!
#define   strInvdKeyType			"不合法的key type值!" //Invalid key type value!
#define   strSetWepKeyTypeErr		"WEP key type设定错误!" //Set WEP key type error!
#define   strInvdDeftKey				"Invalid default tx key id!"
#define   strSetDeftKeyErr			"Set default tx key id error!"
#define   strInvdKey1Len			"不合法的key 1长度!" //Invalid key 1 length!
#define   strInvdWEPKey1			"不合法的wep-key1值!" //Invalid wep-key1 value!
#define   strSetWEPKey1Err			"wep-key1设定错误!" //Set wep-key1 error!
#define   strInvdKey2Len			"不合法的key 2长度!" //Invalid key 2 length!
#define   strInvdWEPKey2			"不合法的wep-key2值!" //Invalid wep-key2 value!
#define   strSetWEPKey2Err			"wep-key2设定错误!" //Set wep-key2 error!
#define   strInvdKey3Len			"不合法的key 3长度!" //Invalid key 3 length!
#define   strInvdWEPKey3			"不合法的wep-key3值!" //Invalid wep-key3 value!
#define   strSetWEPKey3Err			"wep-key3设定错误!" //Set wep-key3 error!
#define   strInvdKey4Len			"不合法的key 4长度!" //Invalid key 4 length!
#define   strInvdWEPKey4			"不合法的wep-key4值!" //Invalid wep-key4 value!
#define   strSetWEPKey4Err			"wep-key4设定错误!" //Set wep-key4 error!
#define   strGetMBSSIBWEPTLBUpdtErr   "错误! MIB_MBSSIB_WEP_TBL 读取错误(Updated)." //Error! Get MIB_MBSSIB_WEP_TBL error(Updated).
#define   strWdsComment			"Comment"
#define   strSetEnableErr   			"enabled flag设定错误!" //Set enabled flag error!
#define   strCommentTooLong 		"错误! Comment过长" //Error! Comment length too long.
#define   strGetEntryNumErr 			"读取条目数错误!" //Get entry number error!
#define   strErrForTablFull 			"条目数已达上限!" //Cannot add new entry because table is full!
#define   strAddEntryErr    			"加入条目失败!" //Add table entry error!
#define   strDelRecordErr   			"chain record删除失败!" //Delete chain record error!


#define   IDS_RESULT_EFFECT           "Setting successful!"
#define   IDS_RESULT_OK               "OK"



//for web upgrade
#define   FILEOPENFAILED "File open error!"
#define   FILEWITHWRONGSIG	"Invalid signature!"

#define  INFO_ENABLED 	"Enabled"
#define  INFO_DISABLED	"Disabled"
//for tr069
#if defined(_CWMP_MIB_)

#define strSetCerPasserror  "Set CPE Certificat's Password error!"
#define strACSURLWrong  "ACS's URL不能为空!" //ACS's URL can't be empty!
#define strSSLWrong "CPE不支援SSL! URL不应以'https://'开头!" //CPE does not support SSL! URL should not start with 'https://'!
#define strSetACSURLerror "ACS's URL 设定错误!" //Set ACS's URL error!
#define strSetUserNameerror "用户名称设定错误!" //Set User Name error!
#define strSetPasserror "密码设定错误!" //Set Password error!
#define strSetInformEnableerror "设定周期上报启用失败!" //Set Inform Enable error!
#define strSetInformIntererror "设定周期上报间隔时间失败!" //Set Inform Interval error!
#define strSetConReqUsererror "设定连接请求用户名称失败!" //Set Connection Request UserName error!
#define strSetConReqPasserror "设定连接请求密码失败!" //Set Connection Request Password error!
#define strSetCWMPFlagerror "CWMP_FLAG设定错误!" //Set CWMP_FLAG error!
#define strGetCWMPFlagerror "CWMP_FLAG读取错误!" //Get CWMP_FLAG error!
#define strUploaderror "上传失败!" //Upload error!
#define strMallocFail "malloc failure!"
#define strArgerror "Insufficient args\n"
#endif

#ifdef CONFIG_MIDDLEWARE
#define strSetMidwarePortError "设定middleware端口错误! " //set middleware port error!
#define strSetMidwareAddrError "设定middleware地址错误! " //set middleware address error!
#define strSetTR069EnableError "设定tr069启用错误" //set tr069 enabling error! 
#define strSwitchTR069EnableError "启用和混合之间不能直接切换! "
#endif

//for dns web fmdns.c
#define Tinvalid_DNS_mode "Invalid DNS mode value!"
#define TDNS_mib_get_error      "Get DNS MIB error!"
#define Tinvalid_DNS_address  	"Invalid DNS address value!"
#define TDNS_mib_set_error		"Set DNS MIB error!"

//for ipqos  fmqos.c  ip/portfilter, fmfwall.
#define Texceed_max_rules				"Error: Maximum number of Qos rule exceeds !"	
#define Tinvalid_source_ip   				"Error! Source IP."
#define Tinvalid_source_netmask 			"Error! Source Netmask"
#define Tinvalid_source_port 				"Error! Invalid source port."
#ifdef QOS_SPEED_LIMIT_SUPPORT
#define Tinvalid_pvc_bandwidth				"Error! Invalid pvc  bandwidth."
#endif
#define Tinvalid_destination_ip			"Error! Destination IP."
#define Tinvalid_destination_netmask		"Error! Destination Netmask."
#define Tinvalid_destination_port			"Error! Invalid destination port."
#define Tinvalid_speed					"Error!Invalid speed"
#define Tadd_chain_error					"Error! Add chain record."
#define Tdelete_chain_error				"Error! Delete chain record."
#define Tinvalid_if_ip					"Error! invalid IP addr!"
#define Tinvalid_if_mask				"Error! invalid mask addr."


//for firewall ip/port filter, fmfwall.c

#define Tprotocol_empty							"Error! Protocol type cannot be empty."
#define Tinvalid_port_range							"Error! port range error"
#define Tinvalid_rule_action						"Invalid Rule Action value!"
#define Tinvalid_source_mac						"Error! Invalid src MAC address."
#define Tinvalid_dest_mac						"Error! Invalid dst MAC address."
#define Toutgoing_ippfilter   						"Outgoing"
#define Tincoming_ippfilter						"Incoming"
#define Tdeny_ippfilter							"Deny"
#define Tallow_ippfilter						"Allow"
#define Tinvalid_rule							"Invaild! This is a duplicate or conflicting rule !"

//for dos
#define strSetDosSYSSYNFLOODErr   "Set DoS SYSSYN_FLOOD error!"
#define strSetDosSYSFINFLOODErr   "Set DoS SYSFIN_FLOOD error!" 
#define strSetDosSYSUDPFLOODErr   "Set DoS SYSUDP_FLOOD error!" 
#define strSetDosSYSICMPFLOODErr  "Set DoS SYSICMP_FLOOD error!"
#define strSetDosPIPSYNFLOODErr   "Set DoS PIPSYN_FLOOD error!"
#define strSetDosPIPFINFLOODErr   "Set DoS PIPFIN_FLOOD error!"
#define strSetDosPIPUDPFLOODErr   "Set DoS PIPUDP_FLOOD error!"
#define strSetDosPIPICMPFLOODErr  "Set DoS PIPICMP_FLOOD error!"
#define strSetDosIPBlockTimeErr   "Set DoS IP Block Timer error!"
#define strSetDosEnableErr        "Set DoS enable error!"

//for wan config(static pppoe IP)
#define Tip_addr		"IP Address"
#define Tdynamic_ip		"Dynamic IP"
#define Tstatic_ip		"Static IP"

#define Tinvalid_ip_net		"Invalid IP address! It should be set within the current subnet.\n"

#define TstrUrlExist        " FQDN already exists!"
#define TstrKeyExist        " KEYword already exists!"
#define TMaxUrl             "Maximum number of FQDN records exceeds!"
#define TMaxKey             "Maximum number of KEYWORD records exceeds!" 


#define strIpExist   "The IP addr has been exist!"
#define strMacExist	 "The MAC addr has been exist!"
#define strSetInterfaceerror   "Set ACL's Interface error!"
#define strSetACLCAPerror  "Set ACL Capability error!"

#define strSetNatSessionError "Set Nat session limitation error!"

#endif
