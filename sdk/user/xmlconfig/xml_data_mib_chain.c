#include <stdio.h>
#include "xml_config_define.h"

#define MAX_INFINITE	500

/* CHAIN ATM_VC_TBL */
static int atmvc_Idx=0;
MIB_CE_ATM_VC_T _atmvc[MAX_VC_NUM];
MIB_CE_ATM_VC_Tp atmvc = _atmvc;
XML_DIR_ARRAY("ATM_VC_TBL", atmvc, root, atmvc_Idx, MAX_VC_NUM, MIB_ATM_VC_TBL);
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, atmvc, "%u", (atmvc+atmvc_Idx)->ifIndex, 0);	//INTEGER_T
XML_ENTRY_PRIMITIVE2("VPI", vpi, atmvc, "%hhu", (atmvc+atmvc_Idx)->vpi, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("QoS", qos, atmvc, "%hhu", (atmvc+atmvc_Idx)->qos, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("VCI", vci, atmvc, "%hu", (atmvc+atmvc_Idx)->vci, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("PCR", pcr, atmvc, "%hu", (atmvc+atmvc_Idx)->pcr, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("SCR", scr, atmvc, "%hu", (atmvc+atmvc_Idx)->scr, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("MBS", mbs, atmvc, "%hu", (atmvc+atmvc_Idx)->mbs, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("CDVT", cdvt, atmvc, "%u", (atmvc+atmvc_Idx)->cdvt, 0);	//DWORD_T
XML_ENTRY_PRIMITIVE2("Encap", encap, atmvc, "%hhu", (atmvc+atmvc_Idx)->encap, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("NAPT", napt, atmvc, "%hhu", (atmvc+atmvc_Idx)->napt, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("ChannelMode", cmode, atmvc, "%hhu", (atmvc+atmvc_Idx)->cmode, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("BridgeType", brmode, atmvc, "%hhu", (atmvc+atmvc_Idx)->brmode, 0);	//BYTE_T
XML_ENTRY_STRING2("pppUser", pppUsername, atmvc, (atmvc+atmvc_Idx)->pppUsername, 0);	//STRING_T
XML_ENTRY_STRING2("pppPasswd", pppPassword, atmvc, (atmvc+atmvc_Idx)->pppPassword, 0);	//STRING_T
XML_ENTRY_PRIMITIVE2("pppAuth", pppAuth, atmvc, "%hhu", (atmvc+atmvc_Idx)->pppAuth, 0);	//BYTE_T
XML_ENTRY_STRING2("pppACName", pppACName, atmvc, (atmvc+atmvc_Idx)->pppACName, 0);	//STRING_T
XML_ENTRY_STRING2("pppServiceName", pppServiceName, atmvc, (atmvc+atmvc_Idx)->pppServiceName, 0);	//STRING_T
XML_ENTRY_PRIMITIVE2("pppConnectType", pppCtype, atmvc, "%hhu", (atmvc+atmvc_Idx)->pppCtype, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("pppIdleTime", pppIdleTime, atmvc, "%hu", (atmvc+atmvc_Idx)->pppIdleTime, 0);	//WORD_T
#ifdef CONFIG_USER_IP_QOS
XML_ENTRY_PRIMITIVE2("enableIpQos", enableIpQos, atmvc, "%hhu", (atmvc+atmvc_Idx)->enableIpQos, 0);   //BYTE_T
#endif	//defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#ifdef CONFIG_IGMPPROXY_MULTIWAN
XML_ENTRY_PRIMITIVE2("enableIGMP", enableIGMP, atmvc, "%hhu", (atmvc+atmvc_Idx)->enableIGMP, 0);   //BYTE_T
#endif	//CONFIG_IGMPPROXY_MULTIWAN
XML_ENTRY_PRIMITIVE2("ChannelAddrType", ipDhcp, atmvc, "%hhu", (atmvc+atmvc_Idx)->ipDhcp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("RIP", rip, atmvc, "%hhu", (atmvc+atmvc_Idx)->rip, 0);   //BYTE_T
XML_ENTRY_ADDR("LocalIPAddr", ipAddr, atmvc, (atmvc+atmvc_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_ADDR("RemoteIPAddr", remoteIpAddr, atmvc, (atmvc+atmvc_Idx)->remoteIpAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("DefaultGW", dgw, atmvc, "%hhu", (atmvc+atmvc_Idx)->dgw, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("MTU", mtu, atmvc, "%u", (atmvc+atmvc_Idx)->mtu, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("ChannelStatus", enable, atmvc, "%hhu", (atmvc+atmvc_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_ADDR("SubnetMask", netMask, atmvc, (atmvc+atmvc_Idx)->netMask, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("Ipunnum", ipunnumbered, atmvc, "%hhu", (atmvc+atmvc_Idx)->ipunnumbered, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DNSMode", dnsMode, atmvc, "%hhu", (atmvc+atmvc_Idx)->dnsMode, 0);   //BYTE_T
XML_ENTRY_ADDR("DNSV4IPAddr1", v4dns1, atmvc, (atmvc+atmvc_Idx)->v4dns1, 0);   //IA_T
XML_ENTRY_ADDR("DNSV4IPAddr2", v4dns2, atmvc, (atmvc+atmvc_Idx)->v4dns2, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("vlan", vlan, atmvc, "%hhu", (atmvc+atmvc_Idx)->vlan, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("vid", vid, atmvc, "%hu", (atmvc+atmvc_Idx)->vid, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("vprio", vprio, atmvc, "%hu", (atmvc+atmvc_Idx)->vprio, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("vpass", vpass, atmvc, "%hhu", (atmvc+atmvc_Idx)->vpass, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("itfGroup", itfGroup, atmvc, "%hu", (atmvc+atmvc_Idx)->itfGroup, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("itfGroupNum", itfGroupNum, atmvc, "%hu", (atmvc+atmvc_Idx)->itfGroupNum, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("cpePppIfIndex", cpePppIfIndex, atmvc, "%lu", (atmvc+atmvc_Idx)->cpePppIfIndex, 0);	//DWORD_T
XML_ENTRY_PRIMITIVE2("cpeIpIndex", cpeIpIndex, atmvc, "%lu", (atmvc+atmvc_Idx)->cpeIpIndex, 0);	//DWORD_T
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
XML_ENTRY_PRIMITIVE2("connDisable", connDisable, atmvc, "%hhu", (atmvc+atmvc_Idx)->connDisable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ConDevInstNum", ConDevInstNum, atmvc, "%u", (atmvc+atmvc_Idx)->ConDevInstNum, 0);	//DWORD_T
XML_ENTRY_PRIMITIVE2("ConIPInstNum", ConIPInstNum, atmvc, "%u", (atmvc+atmvc_Idx)->ConIPInstNum, 0);	//DWORD_T
XML_ENTRY_PRIMITIVE2("ConPPPInstNum", ConPPPInstNum, atmvc, "%u", (atmvc+atmvc_Idx)->ConPPPInstNum, 0);	//DWORD_T
XML_ENTRY_PRIMITIVE2("autoDisTime", autoDisTime, atmvc, "%hu", (atmvc+atmvc_Idx)->autoDisTime, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("warnDisDelay", warnDisDelay, atmvc, "%hu", (atmvc+atmvc_Idx)->warnDisDelay, 0);   //WORD_T
#ifdef _PRMT_TR143_
XML_ENTRY_PRIMITIVE2("TR143UDPEchoItf", TR143UDPEchoItf, atmvc, "%hhu", (atmvc+atmvc_Idx)->TR143UDPEchoItf, 0);   //BYTE_T
#endif	//_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_WANEXT_
XML_ENTRY_PRIMITIVE2("ServiceList", ServiceList, atmvc, "%hu", (atmvc+atmvc_Idx)->ServiceList, 0);   //WORD_T
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_
XML_ENTRY_STRING2("WanName", WanName, atmvc, (atmvc+atmvc_Idx)->WanName, 0);	//STRING_T
#ifdef CONFIG_USER_PPPOE_PROXY
XML_ENTRY_PRIMITIVE2("PPPoEProxyEnable", PPPoEProxyEnable, atmvc, "%hhu", (atmvc+atmvc_Idx)->PPPoEProxyEnable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("PPPoEProxyMaxUser", PPPoEProxyMaxUser, atmvc, "%u", (atmvc+atmvc_Idx)->PPPoEProxyMaxUser, 0);   //DWORD_T
#endif //CONFIG_USER_PPPOE_PROXY
XML_ENTRY_PRIMITIVE2("applicationtype", applicationtype, atmvc, "%u", (atmvc+atmvc_Idx)->applicationtype, 0);   //DWORD_T
#ifdef CONFIG_SPPPD_STATICIP
XML_ENTRY_PRIMITIVE2("StaticPPPoE", pppIp, atmvc, "%hhu", (atmvc+atmvc_Idx)->pppIp, 0);   //BYTE_T
#endif	//CONFIG_SPPPD_STATICIP
#ifdef CONFIG_USER_WT_146
XML_ENTRY_PRIMITIVE2("bfd_enable", bfd_enable, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_opmode", bfd_opmode, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_opmode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_role", bfd_role, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_role, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_mintxint", bfd_mintxint, atmvc, "%u", (atmvc+atmvc_Idx)->bfd_mintxint, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("bfd_minrxint", bfd_minrxint, atmvc, "%u", (atmvc+atmvc_Idx)->bfd_minrxint, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("bfd_minechorxint", bfd_minechorxint, atmvc, "%u", (atmvc+atmvc_Idx)->bfd_minechorxint, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("bfd_detectmult", bfd_detectmult, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_detectmult, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_authtype", bfd_authtype, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_authtype, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_authkeyid", bfd_authkeyid, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_authkeyid, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_authkeylen", bfd_authkeylen, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_authkeylen, 0);   //BYTE_T
XML_ENTRY_STRING2("bfd_authkey", bfd_authkey, atmvc, (atmvc+atmvc_Idx)->bfd_authkey, 0);	//STRING_T
XML_ENTRY_PRIMITIVE2("bfd_dscp", bfd_dscp, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_dscp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bfd_ethprio", bfd_ethprio, atmvc, "%hhu", (atmvc+atmvc_Idx)->bfd_ethprio, 0);   //BYTE_T
#endif //CONFIG_USER_WT_146
#ifdef CONFIG_IPV6
XML_ENTRY_PRIMITIVE2("IpProtocol", IpProtocol, atmvc, "%hhu", (atmvc+atmvc_Idx)->IpProtocol, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("AddrMode", AddrMode, atmvc, "%hhu", (atmvc+atmvc_Idx)->AddrMode, 0);   //BYTE_T
XML_ENTRY_ADDR6("Ipv6Addr", Ipv6Addr, atmvc, (atmvc+atmvc_Idx)->Ipv6Addr, 0);   //IA6_T
XML_ENTRY_ADDR6("RemoteIpv6Addr", RemoteIpv6Addr, atmvc, (atmvc+atmvc_Idx)->RemoteIpv6Addr, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("Ipv6AddrPrefixLen", Ipv6AddrPrefixLen, atmvc, "%hhu", (atmvc+atmvc_Idx)->Ipv6AddrPrefixLen, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Ipv6Dhcp", Ipv6Dhcp, atmvc, "%hhu", (atmvc+atmvc_Idx)->Ipv6Dhcp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Ipv6DhcpRequest", Ipv6DhcpRequest, atmvc, "%hhu", (atmvc+atmvc_Idx)->Ipv6DhcpRequest, 0);   //BYTE_T
XML_ENTRY_ADDR6("RemoteIpv6EndPointAddr", RemoteIpv6EndPointAddr, atmvc, (atmvc+atmvc_Idx)->RemoteIpv6EndPointAddr, 0);   //IA6_T
XML_ENTRY_ADDR6("Ipv6Dns1", Ipv6Dns1, atmvc, (atmvc+atmvc_Idx)->Ipv6Dns1, 0);   //IA6_T
XML_ENTRY_ADDR6("Ipv6Dns2", Ipv6Dns2, atmvc, (atmvc+atmvc_Idx)->Ipv6Dns2, 0);   //IA6_T
#endif //CONFIG_IPV6
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
XML_ENTRY_ADDR("SixrdBRv4IP", SixrdBRv4IP, atmvc, (atmvc+atmvc_Idx)->SixrdBRv4IP, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("SixrdIPv4MaskLen", SixrdIPv4MaskLen, atmvc, "%hhu", (atmvc+atmvc_Idx)->SixrdIPv4MaskLen, 0);	 //BYTE_T
XML_ENTRY_ADDR6("SixrdPrefix", SixrdPrefix, atmvc, (atmvc+atmvc_Idx)->SixrdPrefix, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("SixrdPrefixLen", SixrdPrefixLen, atmvc, "%hhu", (atmvc+atmvc_Idx)->SixrdPrefixLen, 0);	 //BYTE_T
#endif
XML_ENTRY_BYTE6("MacAddr", MacAddr, atmvc, (atmvc+atmvc_Idx)->MacAddr, 0);   //BYTE6_T
#ifdef CONFIG_RTK_RG_INIT
XML_ENTRY_PRIMITIVE2("RGWANIndex", rg_wan_idx, atmvc, "%d", (atmvc+atmvc_Idx)->rg_wan_idx, 0);   //INTEGER_T
#endif
#ifdef CONFIG_USER_DHCP_OPT_GUI_60
XML_ENTRY_PRIMITIVE2("EnableOpt60", enable_opt_60, atmvc, "%hhu", (atmvc+atmvc_Idx)->enable_opt_60, 0);	//BYTE_T
XML_ENTRY_STRING2("Opt60Value", opt60_val, atmvc, (atmvc+atmvc_Idx)->opt60_val, 0);    //STRING_T
XML_ENTRY_PRIMITIVE2("EnableOpt61", enable_opt_61, atmvc, "%hhu", (atmvc+atmvc_Idx)->enable_opt_61, 0);    //BYTE_T
XML_ENTRY_PRIMITIVE2("IAID", iaid, atmvc, "%u", (atmvc+atmvc_Idx)->iaid, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("DuidType", duid_type, atmvc, "%hhu", (atmvc+atmvc_Idx)->duid_type, 0);    //BYTE_T
XML_ENTRY_PRIMITIVE2("DUIDEntNumber", duid_ent_num, atmvc, "%u", (atmvc+atmvc_Idx)->duid_ent_num, 0);   //DWORD_T
XML_ENTRY_STRING2("DUIDIdentifier", duid_id, atmvc, (atmvc+atmvc_Idx)->duid_id, 0);    //STRING_T
XML_ENTRY_PRIMITIVE2("EnableOpt125", enable_opt_125, atmvc, "%hhu", (atmvc+atmvc_Idx)->enable_opt_125, 0);    //BYTE_T
XML_ENTRY_STRING2("ManufacturerOUI", manufacturer, atmvc, (atmvc+atmvc_Idx)->manufacturer, 0);    //STRING_T
XML_ENTRY_STRING2("ProductClass", product_class, atmvc, (atmvc+atmvc_Idx)->product_class, 0);    //STRING_T
XML_ENTRY_STRING2("ModelName", model_name, atmvc, (atmvc+atmvc_Idx)->model_name, 0);    //STRING_T
XML_ENTRY_STRING2("SerialNumber", serial_num, atmvc, (atmvc+atmvc_Idx)->serial_num, 0);    //STRING_T
#endif
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
XML_ENTRY_PRIMITIVE2("dslite_enable", dslite_enable, atmvc, "%hhu", (atmvc+atmvc_Idx)->dslite_enable, 0);	 //BYTE_T
XML_ENTRY_ADDR6("dslite_aftr_addr", dslite_aftr_addr, atmvc, (atmvc+atmvc_Idx)->dslite_aftr_addr, 0);   //IA6_T
XML_ENTRY_STRING2("dslite_aftr_hostname", dslite_aftr_hostname, atmvc, (atmvc+atmvc_Idx)->dslite_aftr_hostname, 0);   //STRING_T
#endif
#ifdef CONFIG_TR142_MODULE
XML_ENTRY_PRIMITIVE2("omci_configured", omci_configured, atmvc, "%hhu", (atmvc+atmvc_Idx)->omci_configured, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("omci_if_id", omci_if_id, atmvc, "%u", (atmvc+atmvc_Idx)->omci_if_id, 0);	//DWORD_T
#endif
/* CHAIN ATM_VC_TBL END*/

/* CHAIN IP_PORT_FILTER_TBL */
#ifdef IP_PORT_FILTER
static int ipportfilter_Idx=0;
MIB_CE_IP_PORT_FILTER_T _ipportfilter[MAX_FILTER_NUM];
MIB_CE_IP_PORT_FILTER_Tp ipportfilter = _ipportfilter;
XML_DIR_ARRAY("IP_PORT_FILTER_TBL", ipportfilter, root, ipportfilter_Idx, MAX_FILTER_NUM, MIB_IP_PORT_FILTER_TBL);
XML_ENTRY_PRIMITIVE2("FilterMode", action, ipportfilter, "%hhu", (ipportfilter+ipportfilter_Idx)->action, 0);   //BYTE_T
XML_ENTRY_ADDR("SrcIP", srcIp, ipportfilter, (ipportfilter+ipportfilter_Idx)->srcIp, 0);   //IA_T
XML_ENTRY_ADDR("DstIP", dstIp, ipportfilter, (ipportfilter+ipportfilter_Idx)->dstIp, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("SrcMask", smaskbit, ipportfilter, "%hhu", (ipportfilter+ipportfilter_Idx)->smaskbit, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DstMask", dmaskbit, ipportfilter, "%hhu", (ipportfilter+ipportfilter_Idx)->dmaskbit, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("SrcPortStart", srcPortFrom, ipportfilter, "%hu", (ipportfilter+ipportfilter_Idx)->srcPortFrom, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("DstPortStart", dstPortFrom, ipportfilter, "%hu", (ipportfilter+ipportfilter_Idx)->dstPortFrom, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("SrcPortEnd", srcPortTo, ipportfilter, "%hu", (ipportfilter+ipportfilter_Idx)->srcPortTo, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("DstPortEnd", dstPortTo, ipportfilter, "%hu", (ipportfilter+ipportfilter_Idx)->dstPortTo, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Direction", dir, ipportfilter, "%hhu", (ipportfilter+ipportfilter_Idx)->dir, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("protocol", protoType, ipportfilter, "%hhu", (ipportfilter+ipportfilter_Idx)->protoType, 0);   //BYTE_T
#endif
/* CHAIN IP_PORT_FILTER_TBL END*/

/* CHAIN MAC_FILTER_TBL */
#ifdef MAC_FILTER
static int macfilter_Idx=0;
MIB_CE_MAC_FILTER_T _macfilter[MAX_FILTER_NUM];
MIB_CE_MAC_FILTER_Tp macfilter = _macfilter;
XML_DIR_ARRAY("MAC_FILTER_TBL", macfilter, root, macfilter_Idx, MAX_FILTER_NUM, MIB_MAC_FILTER_TBL);
XML_ENTRY_PRIMITIVE2("FilterMode", action, macfilter, "%hhu", (macfilter+macfilter_Idx)->action, 0); //BYTE_T
XML_ENTRY_BYTE6("SrcMac", srcMac, macfilter, (macfilter+macfilter_Idx)->srcMac, 0);   //BYTE6_T
XML_ENTRY_BYTE6("DstMac", dstMac, macfilter, (macfilter+macfilter_Idx)->dstMac, 0);   //BYTE6_T
XML_ENTRY_STRING2("Comment", comment, macfilter, (macfilter+macfilter_Idx)->comment, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("Direction", dir, macfilter, "%hhu", (macfilter+macfilter_Idx)->dir, 0);   //BYTE_T
#endif
/* CHAIN MAC_FILTER_TBL END*/

/* CHAIN PORT_FW_TBL */
#ifdef PORT_FORWARD_GENERAL
static int portfw_Idx=0;
MIB_CE_PORT_FW_T _portfw[8];
MIB_CE_PORT_FW_Tp portfw = _portfw;
XML_DIR_ARRAY("PORT_FW_TBL", portfw, root, portfw_Idx, 8, MIB_PORT_FW_TBL);
XML_ENTRY_ADDR("IP", ipAddr, portfw, (portfw+portfw_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("PortStart", fromPort, portfw, "%hu", (portfw+portfw_Idx)->fromPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("PortEnd", toPort, portfw, "%hu", (portfw+portfw_Idx)->toPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Protocol", protoType, portfw, "%hhu", (portfw+portfw_Idx)->protoType, 0);   //BYTE_T
XML_ENTRY_STRING2("Comment", comment, portfw, (portfw+portfw_Idx)->comment, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("OutInf", ifIndex, portfw, "%u", (portfw+portfw_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("enable", enable, portfw, "%hhu", (portfw+portfw_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("leaseduration", leaseduration, portfw, "%u", (portfw+portfw_Idx)->leaseduration, 0);   //INTEGER_T
XML_ENTRY_ADDR("remotehost", remotehost, portfw, (portfw+portfw_Idx)->remotehost, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("externalportStart", externalfromport, portfw, "%u", (portfw+portfw_Idx)->externalfromport, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("externalportEnd", externaltoport, portfw, "%u", (portfw+portfw_Idx)->externaltoport, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("Dynamic", dynamic, portfw, "%hhu", (portfw+portfw_Idx)->dynamic, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, portfw, "%u", (portfw+portfw_Idx)->InstanceNum, 0);   //INTEGER_T
#endif
/* CHAIN PORT_FW_TBL END*/

/* CHAIN PORT_FW_ADVANCE_TBL*/
#ifdef PORT_FORWARD_ADVANCE
static int ipfw_advance_Idx=0;
MIB_CE_PORT_FW_ADVANCE_T _ipfw_advance[8];
MIB_CE_PORT_FW_ADVANCE_Tp ipfw_advance = _ipfw_advance;
XML_DIR_ARRAY("PORT_FW_ADVANCE_TBL", ipfw_advance, root, ipfw_advance_Idx, 8, MIB_PFW_ADVANCE_TBL);
XML_ENTRY_ADDR("IP", ipAddr, ipfw_advance, (ipfw_advance+ipfw_advance_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, ipfw_advance, "%u", (ipfw_advance+ipfw_advance_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("gategory", gategory, ipfw_advance, "%hhu", (ipfw_advance+ipfw_advance_Idx)->gategory, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("rule", rule, ipfw_advance, "%u", (ipfw_advance+ipfw_advance_Idx)->rule, 0);   //INTEGER_T
#endif
/* CHAIN PORT_FW_ADVANCE_TBL END*/

#ifdef VIRTUAL_SERVER_SUPPORT
/* CHAIN MIB_VIRTUAL_SVR_TBL*/
static int mvtlsvr_Idx=0;
MIB_CE_VTL_SVR_T _mmvtlsvr[VIRTUAL_SERVER_RULES];
MIB_CE_VTL_SVR_Tp mvtlsvr = _mmvtlsvr;
XML_DIR_ARRAY("VIRTUAL_SVR_TBL", mvtlsvr, root, mvtlsvr_Idx, VIRTUAL_SERVER_RULES, MIB_VIRTUAL_SVR_TBL);
XML_ENTRY_ADDR("svrIpAddr", svrIpAddr, mvtlsvr, (mvtlsvr+mvtlsvr_Idx)->svrIpAddr, 0);   //IA_T
XML_ENTRY_STRING2("svrName", svrName, mvtlsvr, (mvtlsvr+mvtlsvr_Idx)->svrName, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wanStartPort", wanStartPort, mvtlsvr, "%hu", (mvtlsvr+mvtlsvr_Idx)->wanStartPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("wanEndPort", wanEndPort, mvtlsvr, "%hu", (mvtlsvr+mvtlsvr_Idx)->wanEndPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("svrStartPort", svrStartPort, mvtlsvr, "%hu", (mvtlsvr+mvtlsvr_Idx)->svrStartPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("svrEndPort", svrEndPort, mvtlsvr, "%hhu",(mvtlsvr+mvtlsvr_Idx)->svrEndPort, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("protoType", protoType, mvtlsvr, "%u", (mvtlsvr+mvtlsvr_Idx)->protoType, 0);   //DWORD_T
/* CHAIN MIB_VIRTUAL_SVR_TBL END*/
#endif


/* CHAIN IP_ROUTE_TBL*/
#ifdef ROUTING
static int iproute_Idx=0;
MIB_CE_IP_ROUTE_T _iproute[8];
MIB_CE_IP_ROUTE_Tp iproute = _iproute;
XML_DIR_ARRAY("IP_ROUTE_TBL", iproute, root, iproute_Idx, 8, MIB_IP_ROUTE_TBL);
XML_ENTRY_ADDR("DstIP", destID, iproute, (iproute+iproute_Idx)->destID, 0);   //IA_T
XML_ENTRY_ADDR("Mask", netMask, iproute, (iproute+iproute_Idx)->netMask, 0);   //IA_T
XML_ENTRY_ADDR("NextHop", nextHop, iproute, (iproute+iproute_Idx)->nextHop, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("Enable", Enable, iproute, "%hhu", (iproute+iproute_Idx)->Enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Type", Type, iproute, "%hhu", (iproute+iproute_Idx)->Type, 0);   //BYTE_T
XML_ENTRY_ADDR("SourceIP", SourceIP, iproute, (iproute+iproute_Idx)->SourceIP, 0);   //IA_T
XML_ENTRY_ADDR("SourceMask", SourceMask, iproute, (iproute+iproute_Idx)->SourceMask, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("OutInf", ifIndex, iproute, "%u", (iproute+iproute_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("FWMetric", FWMetric, iproute, "%d", (iproute+iproute_Idx)->FWMetric, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, iproute, "%u", (iproute+iproute_Idx)->InstanceNum, 0);   //DWORD_T
#ifdef _PRMT_WT107_
XML_ENTRY_PRIMITIVE2("Flags", Flags, iproute, "%hhu", (iproute+iproute_Idx)->Flags, 0);   //BYTE_T
#endif
#ifdef CONFIG_RTK_RG_INIT
XML_ENTRY_PRIMITIVE2("RGWANIndex", rg_wan_idx, iproute, "%d", (iproute+iproute_Idx)->rg_wan_idx, 0);   //INTEGER_T
#endif
#endif
/* CHAIN IP_ROUTE_TBL END*/

/* CHAIN ACL_IP_TBL*/
#ifdef IP_ACL
static int aclip_Idx=0;
MIB_CE_ACL_IP_T _aclip[8];
MIB_CE_ACL_IP_Tp aclip = _aclip;
XML_DIR_ARRAY("ACL_IP_TBL", aclip, root, aclip_Idx, 8, MIB_ACL_IP_TBL);
#ifdef ACL_IP_RANGE
XML_ENTRY_ADDR("StartIPAddr", startipAddr, aclip, (aclip+aclip_Idx)->startipAddr, 0);   //IA_T
XML_ENTRY_ADDR("EndIPAddr", endipAddr, aclip, (aclip+aclip_Idx)->endipAddr, 0);   //IA_T
#endif
XML_ENTRY_ADDR("IPAddr", ipAddr, aclip, (aclip+aclip_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("NetMask", maskbit, aclip, "%hhu", (aclip+aclip_Idx)->maskbit, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("State", Enabled, aclip, "%hhu", (aclip+aclip_Idx)->Enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Interface", Interface, aclip, "%hhu", (aclip+aclip_Idx)->Interface, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("any", any, aclip, "%hhu", (aclip+aclip_Idx)->any, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("telnet", telnet, aclip, "%hhu", (aclip+aclip_Idx)->telnet, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ftp", ftp, aclip, "%hhu", (aclip+aclip_Idx)->ftp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("tftp", tftp, aclip, "%hhu", (aclip+aclip_Idx)->tftp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("web", web, aclip, "%hhu", (aclip+aclip_Idx)->web, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("snmp", snmp, aclip, "%hhu", (aclip+aclip_Idx)->snmp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ssh", ssh, aclip, "%hhu", (aclip+aclip_Idx)->ssh, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("icmp", icmp, aclip, "%hhu", (aclip+aclip_Idx)->icmp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("telnet_port", telnet_port, aclip, "%hu", (aclip+aclip_Idx)->telnet_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("web_port", web_port, aclip, "%hu", (aclip+aclip_Idx)->web_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("ftp_port", ftp_port, aclip, "%hu", (aclip+aclip_Idx)->ftp_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("https", https, aclip, "%hhu", (aclip+aclip_Idx)->https, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("https_port", https_port, aclip, "%hu", (aclip+aclip_Idx)->https_port, 0);   //WORD_T
/* CHAIN ACC_TBL END*/
#endif
/* CHAIN ACL_IP_TBL END*/

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/* CHAIN DHCP_SERVER_OPTION_TBL*/
static int dhcpopt_server_Idx=0;
MIB_CE_DHCP_OPTION_T _dhcpopt_server[MAX_INFINITE];
MIB_CE_DHCP_OPTION_Tp dhcpopt_server = _dhcpopt_server;
XML_DIR_ARRAY("DHCP_SERVER_OPTION_TBL", dhcpopt_server, root, dhcpopt_server_Idx, MAX_INFINITE, MIB_DHCP_SERVER_OPTION_TBL);
XML_ENTRY_PRIMITIVE2("Enable", enable, dhcpopt_server, "%hhu", (dhcpopt_server+dhcpopt_server_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("UsedFor", usedFor, dhcpopt_server, "%hhu", (dhcpopt_server+dhcpopt_server_Idx)->usedFor, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Order", order, dhcpopt_server, "%u", (dhcpopt_server+dhcpopt_server_Idx)->order, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("Tag", tag, dhcpopt_server, "%hhu", (dhcpopt_server+dhcpopt_server_Idx)->tag, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Len", len, dhcpopt_server, "%hhu", (dhcpopt_server+dhcpopt_server_Idx)->len, 0);   //BYTE_T
XML_ENTRY_BYTE_ARRAY("Value", value, dhcpopt_server, (dhcpopt_server+dhcpopt_server_Idx)->value, 0);   //BYTE_ARRAY_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, dhcpopt_server, "%u", (dhcpopt_server+dhcpopt_server_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("DhcpOptInstNum", dhcpOptInstNum, dhcpopt_server, "%u", (dhcpopt_server+dhcpopt_server_Idx)->dhcpOptInstNum, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("DhcpConSPInstNum", dhcpConSPInstNum, dhcpopt_server, "%u", (dhcpopt_server+dhcpopt_server_Idx)->dhcpConSPInstNum, 0);   //DWORD_T
/* CHAIN DHCP_SERVER_OPTION_TBL END*/

/* CHAIN DHCP_CLIENT_OPTION_TBL*/
static int dhcpopt_client_Idx=0;
MIB_CE_DHCP_OPTION_T _dhcpopt_client[MAX_INFINITE];
MIB_CE_DHCP_OPTION_Tp dhcpopt_client = _dhcpopt_client;
XML_DIR_ARRAY("DHCP_CLIENT_OPTION_TBL", dhcpopt_client, root, dhcpopt_client_Idx, MAX_INFINITE, MIB_DHCP_CLIENT_OPTION_TBL);
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, dhcpopt_client, "%u", (dhcpopt_client+dhcpopt_client_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("Enable", enable, dhcpopt_client, "%hhu", (dhcpopt_client+dhcpopt_client_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Order", order, dhcpopt_client, "%u", (dhcpopt_client+dhcpopt_client_Idx)->order, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("Tag", tag, dhcpopt_client, "%hhu", (dhcpopt_client+dhcpopt_client_Idx)->tag, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Len", len, dhcpopt_client, "%hhu", (dhcpopt_client+dhcpopt_client_Idx)->len, 0);   //BYTE_T
XML_ENTRY_BYTE_ARRAY("Value", value, dhcpopt_client, (dhcpopt_client+dhcpopt_client_Idx)->value, 0);   //BYTE_ARRAY_T
XML_ENTRY_PRIMITIVE2("UsedFor", usedFor, dhcpopt_client, "%hhu", (dhcpopt_client+dhcpopt_client_Idx)->usedFor, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DhcpOptInstNum", dhcpOptInstNum, dhcpopt_client, "%u", (dhcpopt_client+dhcpopt_client_Idx)->dhcpOptInstNum, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("DhcpConSPInstNum", dhcpConSPInstNum, dhcpopt_client, "%u", (dhcpopt_client+dhcpopt_client_Idx)->dhcpConSPInstNum, 0);   //DWORD_T
/* CHAIN DHCP_CLIENT_OPTION_TBL END*/
#endif

/*ping_zhang:20080919 END*/
//ql_xu add:
#ifdef MAC_ACL
/* CHAIN ACL_MAC_TBL*/
static int aclmac_Idx=0;
MIB_CE_ACL_MAC_T _aclmac[8];
MIB_CE_ACL_MAC_Tp aclmac = _aclmac;
XML_DIR_ARRAY("ACL_MAC_TBL", aclmac, root, aclmac_Idx, 8, MIB_ACL_MAC_TBL);
XML_ENTRY_BYTE6("MacAddr", macAddr, aclmac, (aclmac+aclmac_Idx)->macAddr, 0);   //BYTE6_T
XML_ENTRY_PRIMITIVE2("State", Enabled, aclmac, "%hhu", (aclmac+aclmac_Idx)->Enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Interface", Interface, aclmac, "%hhu", (aclmac+aclmac_Idx)->Interface, 0);   //BYTE_T
/* CHAIN ACL_MAC_TBL END*/
#endif

#ifdef NAT_CONN_LIMIT
/* CHAIN NAT_CONN_TBL*/
static int connlimit_Idx=0;
MIB_CE_CONN_LIMIT_T _connlimit[16];
MIB_CE_CONN_LIMIT_Tp connlimit = _connlimit;
XML_DIR_ARRAY("NAT_CONN_TBL", connlimit, root, connlimit_Idx, 16, MIB_CONN_LIMIT_TBL);
XML_ENTRY_ADDR("IPAddr", ipAddr, connlimit, (connlimit+connlimit_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("State", Enabled, connlimit, "%hhu", (connlimit+connlimit_Idx)->Enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("connNum", connNum, connlimit, "%u", (connlimit+connlimit_Idx)->connNum, 0);   //INTEGER_T
/* CHAIN NAT_CONN_TBL END*/
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
/* CHAIN WLAN_AC_TBL*/
static int wlac_Idx=0;
MIB_CE_WLAN_AC_T _wlac[MAX_WLAN_AC_NUM];
MIB_CE_WLAN_AC_Tp wlac = _wlac;
XML_DIR_ARRAY("WLAN_AC_TBL", wlac, root, wlac_Idx, MAX_WLAN_AC_NUM, MIB_WLAN_AC_TBL);
XML_ENTRY_PRIMITIVE2("wlanIdx", wlanIdx, wlac, "%hhu", (wlac+wlac_Idx)->wlanIdx, 0);   //BYTE_T
XML_ENTRY_BYTE6("MacAddr", macAddr, wlac, (wlac+wlac_Idx)->macAddr, 0);   //BYTE6_T
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
static int w1lac_Idx=0;
MIB_CE_WLAN_AC_T _w1lac[MAX_WLAN_AC_NUM];
MIB_CE_WLAN_AC_Tp w1lac = _w1lac;
XML_DIR_ARRAY("WLAN1_AC_TBL", w1lac, root, w1lac_Idx, MAX_WLAN_AC_NUM, MIB_WLAN1_AC_TBL);
XML_ENTRY_PRIMITIVE2("wlanIdx", wlanIdx, w1lac, "%hhu", (w1lac+w1lac_Idx)->wlanIdx, 0);   //BYTE_T
XML_ENTRY_BYTE6("MacAddr", macAddr, w1lac, (w1lac+w1lac_Idx)->macAddr, 0);   //BYTE6_T
#endif
/* CHAIN WLAN_AC_TBL END*/
#endif
#ifdef WLAN_11R
static int wlan_ftkh_Idx=0;
MIB_CE_WLAN_FTKH_T _wlan_ftkh[MAX_WLAN_FTKH_NUM];
MIB_CE_WLAN_FTKH_Tp wlan_ftkh = _wlan_ftkh;
XML_DIR_ARRAY("WLAN_FTKH_TBL", wlan_ftkh, root, wlan_ftkh_Idx, MAX_WLAN_FTKH_NUM, MIB_WLAN_FTKH_TBL);
XML_ENTRY_PRIMITIVE2("wlanIdx", wlanIdx, wlan_ftkh, "%hhu", (wlan_ftkh+wlan_ftkh_Idx)->wlanIdx, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("intfIdx", intfIdx, wlan_ftkh, "%hhu", (wlan_ftkh+wlan_ftkh_Idx)->intfIdx, 0);   //BYTE_T
XML_ENTRY_BYTE6("addr", addr, wlan_ftkh, (wlan_ftkh+wlan_ftkh_Idx)->addr, 0);   //BYTE6_T
XML_ENTRY_STRING2("r0kh_id", r0kh_id, wlan_ftkh, (wlan_ftkh+wlan_ftkh_Idx)->r0kh_id, 0);   //STRING_T
XML_ENTRY_STRING2("key", key, wlan_ftkh, (wlan_ftkh+wlan_ftkh_Idx)->key, 0);   //STRING_T
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
static int wlan1_ftkh_Idx=0;
MIB_CE_WLAN_FTKH_T _wlan1_ftkh[MAX_WLAN_FTKH_NUM];
MIB_CE_WLAN_FTKH_Tp wlan1_ftkh = _wlan1_ftkh;
XML_DIR_ARRAY("WLAN1_FTKH_TBL", wlan1_ftkh, root, wlan1_ftkh_Idx, MAX_WLAN_FTKH_NUM, MIB_WLAN1_FTKH_TBL);
XML_ENTRY_PRIMITIVE2("wlanIdx", wlanIdx, wlan1_ftkh, "%hhu", (wlan1_ftkh+wlan1_ftkh_Idx)->wlanIdx, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("intfIdx", intfIdx, wlan1_ftkh, "%hhu", (wlan1_ftkh+wlan1_ftkh_Idx)->intfIdx, 0);   //BYTE_T
XML_ENTRY_BYTE6("addr", addr, wlan1_ftkh, (wlan1_ftkh+wlan1_ftkh_Idx)->addr, 0);   //BYTE6_T
XML_ENTRY_STRING2("r0kh_id", r0kh_id, wlan1_ftkh, (wlan1_ftkh+wlan1_ftkh_Idx)->r0kh_id, 0);   //STRING_T
XML_ENTRY_STRING2("key", key, wlan1_ftkh, (wlan1_ftkh+wlan1_ftkh_Idx)->key, 0);   //STRING_T
#endif
#endif
#endif

/* CHAIN SW_PORT_TBL*/
static int swport_Idx=0;
MIB_CE_SW_PORT_T _swport[SW_PORT_NUM];
MIB_CE_SW_PORT_Tp swport = _swport;
XML_DIR_ARRAY("SW_PORT_TBL", swport, root, swport_Idx, SW_PORT_NUM, MIB_SW_PORT_TBL);
XML_ENTRY_PRIMITIVE2("PVCInterface", pvcItf, swport, "%u", (swport+swport_Idx)->pvcItf, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("InterfaceGroup", itfGroup, swport, "%hhu", (swport+swport_Idx)->itfGroup, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("PVID", pvid, swport, "%hhu", (swport+swport_Idx)->pvid, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("LinkMode", linkMode, swport, "%hhu", (swport+swport_Idx)->linkMode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("VLAN on LAN Enabled", vlan_on_lan_enabled, swport, "%hhu", (swport+swport_Idx)->vlan_on_lan_enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("VLAN on LAN InterfaceGroup", vlan_on_lan_itfGroup, swport, "%hhu", (swport+swport_Idx)->vlan_on_lan_itfGroup, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("VID", vid, swport, "%hu", (swport+swport_Idx)->vid, 0);   //WORD_T
/* CHAIN SW_PORT_TBL END*/

#ifdef VLAN_GROUP
/* CHAIN VLAN_TBL*/
static int vlan_Idx=0;
MIB_CE_VLAN_T _vlan[VLAN_NUM];
MIB_CE_VLAN_Tp vlan = _vlan;
XML_DIR_ARRAY("VLAN_TBL", vlan, root, vlan_Idx, VLAN_NUM, MIB_VLAN_TBL);
XML_ENTRY_PRIMITIVE2("Member", member, vlan, "%u", (vlan+vlan_Idx)->member, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("Tag", tag, vlan, "%hu", (vlan+vlan_Idx)->tag, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Dhcps", dhcps, vlan, "%hhu", (vlan+vlan_Idx)->dhcps, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("IGMP", igmp, vlan, "%hhu", (vlan+vlan_Idx)->igmp, 0);   //BYTE_T
/* CHAIN VLAN_TBL END*/
#endif

#if defined(CONFIG_USER_IP_QOS)
/* CHAIN IP_QOS_TBL*/
static int ipqos_Idx=0;
MIB_CE_IP_QOS_T _ipqos[MAX_QOS_RULE];
MIB_CE_IP_QOS_Tp ipqos = _ipqos;
XML_DIR_ARRAY("IP_QOS_TBL", ipqos, root, ipqos_Idx, MAX_QOS_RULE, MIB_IP_QOS_TBL);
XML_ENTRY_PRIMITIVE2("TYPE", ipqos_rule_type, ipqos, "%hhu", (ipqos+ipqos_Idx)->ipqos_rule_type, 0);   //BYTE_T
XML_ENTRY_STRING2("Name", RuleName, ipqos, (ipqos+ipqos_Idx)->RuleName, 0);   //STRING_T
XML_ENTRY_ADDR("SrcIP", sip, ipqos, (ipqos+ipqos_Idx)->sip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("SrcMask", smaskbit, ipqos, "%hhu", (ipqos+ipqos_Idx)->smaskbit, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("SrcPort", sPort, ipqos, "%hu", (ipqos+ipqos_Idx)->sPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("SrcPortRangeMax", sPortRangeMax, ipqos, "%hu", (ipqos+ipqos_Idx)->sPortRangeMax, 0);   //WORD_T
XML_ENTRY_ADDR("DstIP", dip, ipqos, (ipqos+ipqos_Idx)->dip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("DstMask", dmaskbit, ipqos, "%hhu", (ipqos+ipqos_Idx)->dmaskbit, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DstPort", dPort, ipqos, "%hu", (ipqos+ipqos_Idx)->dPort, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("DstPortRangeMax", dPortRangeMax, ipqos, "%hu", (ipqos+ipqos_Idx)->dPortRangeMax, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("EthernetType", ethType, ipqos, "%hu", (ipqos+ipqos_Idx)->ethType, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Protocol", protoType, ipqos, "%hhu", (ipqos+ipqos_Idx)->protoType, 0);   //BYTE_T
XML_ENTRY_BYTE6("SMac", smac, ipqos, (ipqos+ipqos_Idx)->smac, 0);   //BYTE6_T
XML_ENTRY_BYTE6("DMac", dmac, ipqos, (ipqos+ipqos_Idx)->dmac, 0);   //BYTE6_T
XML_ENTRY_PRIMITIVE2("PhyPort", phyPort, ipqos, "%hhu", (ipqos+ipqos_Idx)->phyPort, 0);   //BYTE_T
#if defined(CONFIG_USER_IP_QOS) || defined(QOS_DIFFSERV)
XML_ENTRY_PRIMITIVE2("qosDscp", qosDscp, ipqos, "%hhu", (ipqos+ipqos_Idx)->qosDscp, 0);   //BYTE_T
#endif
#ifdef CONFIG_USER_IP_QOS
XML_ENTRY_PRIMITIVE2("vlan1p", vlan1p, ipqos, "%hhu", (ipqos+ipqos_Idx)->vlan1p, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("tos", tos, ipqos, "%hhu", (ipqos+ipqos_Idx)->tos, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("OutInf", outif, ipqos, "%u", (ipqos+ipqos_Idx)->outif, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("Prior", prior, ipqos, "%hhu", (ipqos+ipqos_Idx)->prior, 0);   //BYTE_T
#ifdef QOS_DSCP
XML_ENTRY_PRIMITIVE2("Dscp", dscp, ipqos, "%hhu", (ipqos+ipqos_Idx)->dscp, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("m_ipprio", m_ipprio, ipqos, "%hhu", (ipqos+ipqos_Idx)->m_ipprio, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("m_iptos", m_iptos, ipqos, "%hhu", (ipqos+ipqos_Idx)->m_iptos, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("m_vid", m_vid, ipqos, "%hu", (ipqos+ipqos_Idx)->m_vid, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("m_1p", m_1p, ipqos, "%hhu", (ipqos+ipqos_Idx)->m_1p, 0);   //BYTE_T
#ifdef CONFIG_USER_IP_QOS
XML_ENTRY_PRIMITIVE2("m_dscp", m_dscp, ipqos, "%hhu", (ipqos+ipqos_Idx)->m_dscp, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("Enable", enable, ipqos, "%hhu", (ipqos+ipqos_Idx)->enable, 0);   //BYTE_T
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, ipqos, "%u", (ipqos+ipqos_Idx)->InstanceNum, 0);   //DWORD_T
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
XML_ENTRY_PRIMITIVE2("limitSpeedEnabled", limitSpeedEnabled, ipqos, "%hhu", (ipqos+ipqos_Idx)->limitSpeedEnabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("limitSpeedRank", limitSpeedRank, ipqos, "%hhu", (ipqos+ipqos_Idx)->limitSpeedRank, 0);   //BYTE_T
#endif
#ifdef QOS_DIFFSERV
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, ipqos, "%u", (ipqos+ipqos_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("enDiffserv", enDiffserv, ipqos, "%hhu", (ipqos+ipqos_Idx)->enDiffserv, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("totalBandwidth", totalBandwidth, ipqos, "%hu", (ipqos+ipqos_Idx)->totalBandwidth, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("htbRate", htbRate, ipqos, "%hu", (ipqos+ipqos_Idx)->htbRate, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("latency", latency, ipqos, "%u", (ipqos+ipqos_Idx)->latency, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("limitSpeed", limitSpeed, ipqos, "%u", (ipqos+ipqos_Idx)->limitSpeed, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("policing", policing, ipqos, "%hhu", (ipqos+ipqos_Idx)->policing, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("flags", flags, ipqos, "%hu", (ipqos+ipqos_Idx)->flags, 0);   //WORD_T
#ifdef BR_ROUTE_ONEPVC
XML_ENTRY_PRIMITIVE2("cmode", cmode, ipqos, "%hhu", (ipqos+ipqos_Idx)->cmode, 0);   //BYTE_T
#endif
#ifdef CONFIG_IPV6
XML_ENTRY_PRIMITIVE2("IPVersion", IpProtocol, ipqos, "%hhu", (ipqos+ipqos_Idx)->IpProtocol, 0);   //BYTE_T
XML_ENTRY_ADDR6("SrcIP6", sip6, ipqos, (ipqos+ipqos_Idx)->sip6, 0);   //IA6_T
XML_ENTRY_ADDR6("DstIP6", dip6, ipqos, (ipqos+ipqos_Idx)->dip6, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("SrcIP6PrefixLen", sip6PrefixLen, ipqos, "%hhu", (ipqos+ipqos_Idx)->sip6PrefixLen, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DstIP6PrefixLen", dip6PrefixLen, ipqos, "%hhu", (ipqos+ipqos_Idx)->dip6PrefixLen, 0);   //BYTE_T
#endif
#ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
XML_ENTRY_PRIMITIVE2("dhcpopt_type", dhcpopt_type, ipqos, "%hhu", (ipqos+ipqos_Idx)->dhcpopt_type, 0);   //BYTE_T
XML_ENTRY_STRING2("opt60_vendorclass", opt60_vendorclass, ipqos, (ipqos+ipqos_Idx)->opt60_vendorclass, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("opt61_iaid", opt61_iaid, ipqos, "%u", (ipqos+ipqos_Idx)->opt61_iaid, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("opt61_duid_type", opt61_duid_type, ipqos, "%hhu", (ipqos+ipqos_Idx)->opt61_duid_type, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("duid_hw_type", duid_hw_type, ipqos, "%hu", (ipqos+ipqos_Idx)->duid_hw_type, 0);   //WORD_T
XML_ENTRY_BYTE6("duid_mac", duid_mac, ipqos, (ipqos+ipqos_Idx)->duid_mac, 0);   //BYTE6_T
XML_ENTRY_PRIMITIVE2("duid_time", duid_time, ipqos, "%hu", (ipqos+ipqos_Idx)->duid_time, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("duid_ent_num", duid_ent_num, ipqos, "%hu", (ipqos+ipqos_Idx)->duid_ent_num, 0);   //WORD_T
XML_ENTRY_STRING2("duid_ent_id", duid_ent_id, ipqos, (ipqos+ipqos_Idx)->duid_ent_id, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("opt125_ent_num", opt125_ent_num, ipqos, "%hu", (ipqos+ipqos_Idx)->opt125_ent_num, 0);   //WORD_T
XML_ENTRY_STRING2("opt125_manufacturer", opt125_manufacturer, ipqos, (ipqos+ipqos_Idx)->opt125_manufacturer, 0);   //STRING_T
XML_ENTRY_STRING2("opt125_product_class", opt125_product_class, ipqos, (ipqos+ipqos_Idx)->opt125_product_class, 0);   //STRING_T
XML_ENTRY_STRING2("opt125_model", opt125_model, ipqos, (ipqos+ipqos_Idx)->opt125_model, 0);   //STRING_T
XML_ENTRY_STRING2("opt125_serial", opt125_serial, ipqos, (ipqos+ipqos_Idx)->opt125_serial, 0);   //STRING_T
#endif
/* CHAIN IP_QOS_TBL END*/
#endif

//ql 20081119 for IP QoS traffic shaping
#ifdef CONFIG_USER_IP_QOS
/* CHAIN IP_QOS_TC_TBL*/
static int ipqos_tc_Idx=0;
MIB_CE_IP_TC_T _ipqos_tc[MAX_QOS_RULE];
MIB_CE_IP_TC_Tp ipqos_tc = _ipqos_tc;
XML_DIR_ARRAY("IP_QOS_TC_TBL", ipqos_tc, root, ipqos_tc_Idx, MAX_QOS_RULE, MIB_IP_QOS_TC_TBL);
XML_ENTRY_PRIMITIVE2("entryid", entryid, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->entryid, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, ipqos_tc, "%u", (ipqos_tc+ipqos_tc_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_ADDR("Srcip", srcip, ipqos_tc, (ipqos_tc+ipqos_tc_Idx)->srcip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("srcmask", smaskbits, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->smaskbits, 0);   //BYTE_T
XML_ENTRY_ADDR("dstip", dstip, ipqos_tc, (ipqos_tc+ipqos_tc_Idx)->dstip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("dstmask", dmaskbits, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->dmaskbits, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("srcport", sport, ipqos_tc, "%hu", (ipqos_tc+ipqos_tc_Idx)->sport, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("dstport", dport, ipqos_tc, "%hu", (ipqos_tc+ipqos_tc_Idx)->dport, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("protocol", protoType, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->protoType, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("rate", limitSpeed, ipqos_tc, "%u", (ipqos_tc+ipqos_tc_Idx)->limitSpeed, 0);   //DWORD_T
#ifdef BR_ROUTE_ONEPVC
XML_ENTRY_PRIMITIVE2("cmode", cmode, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->cmode, 0);   //BYTE_T
#endif
#ifdef CONFIG_IPV6
XML_ENTRY_PRIMITIVE2("IPVersion", IpProtocol, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->IpProtocol, 0);   //BYTE_T
XML_ENTRY_ADDR6("SrcIP6", sip6, ipqos_tc, (ipqos_tc+ipqos_tc_Idx)->sip6, 0);   //IA6_T
XML_ENTRY_ADDR6("DstIP6", dip6, ipqos_tc, (ipqos_tc+ipqos_tc_Idx)->dip6, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("SrcIP6PrefixLen", sip6PrefixLen, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->sip6PrefixLen, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("DstIP6PrefixLen", dip6PrefixLen, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->dip6PrefixLen, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("direction", direction, ipqos_tc, "%hhu", (ipqos_tc+ipqos_tc_Idx)->direction, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("vlanID", vlanID, ipqos_tc, "%u", (ipqos_tc+ipqos_tc_Idx)->vlanID, 0);   //INTEGER_T
XML_ENTRY_STRING2("ssid", ssid, ipqos_tc, (ipqos_tc+ipqos_tc_Idx)->ssid, 0);   //STRING_T
/* CHAIN IP_QOS_TC_TBL END*/
#endif

#ifdef CONFIG_USER_IP_QOS
/* CHAIN IP_QOS_QUEUE_TBL*/
static int ipqos_queue_Idx=0;
MIB_CE_IP_QOS_QUEUE_T _ipqos_queue[MAX_QOS_QUEUE_NUM];
MIB_CE_IP_QOS_QUEUE_Tp ipqos_queue = _ipqos_queue;
XML_DIR_ARRAY("IP_QOS_QUEUE_TBL", ipqos_queue, root, ipqos_queue_Idx, MAX_QOS_QUEUE_NUM, MIB_IP_QOS_QUEUE_TBL);
XML_ENTRY_PRIMITIVE2("outif", outif, ipqos_queue, "%u", (ipqos_queue+ipqos_queue_Idx)->outif, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("prior", prior, ipqos_queue, "%hhu", (ipqos_queue+ipqos_queue_Idx)->prior, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("weight", weight, ipqos_queue, "%hhu", (ipqos_queue+ipqos_queue_Idx)->weight, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("enable", enable, ipqos_queue, "%hhu", (ipqos_queue+ipqos_queue_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_STRING2("desc", desc, ipqos_queue, (ipqos_queue+ipqos_queue_Idx)->desc, 0);   //STRING_T
//#ifdef _CWMP_MIB_ /*mib for cwmp-tr069*/
XML_ENTRY_PRIMITIVE2("QueueInstNum", QueueInstNum, ipqos_queue, "%u", (ipqos_queue+ipqos_queue_Idx)->QueueInstNum, 0);   //DWORD_T
//#endif
/* CHAIN IP_QOS_QUEUE_TBL END*/
#endif

#ifdef REMOTE_ACCESS_CTL
/* CHAIN ACC_TBL*/
static int acc_Idx=0;
MIB_CE_ACC_T _acc[MAX_INFINITE];
MIB_CE_ACC_Tp acc = _acc;
XML_DIR_ARRAY("ACC_TBL", acc, root, acc_Idx, MAX_INFINITE, MIB_ACC_TBL);
XML_ENTRY_PRIMITIVE2("telnet", telnet, acc, "%hhu", (acc+acc_Idx)->telnet, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ftp", ftp, acc, "%hhu", (acc+acc_Idx)->ftp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("tftp", tftp, acc, "%hhu", (acc+acc_Idx)->tftp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("web", web, acc, "%hhu", (acc+acc_Idx)->web, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("snmp", snmp, acc, "%hhu", (acc+acc_Idx)->snmp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ssh", ssh, acc, "%hhu", (acc+acc_Idx)->ssh, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("icmp", icmp, acc, "%hhu", (acc+acc_Idx)->icmp, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("netlog", netlog, acc, "%hhu", (acc+acc_Idx)->netlog, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("telnet_port", telnet_port, acc, "%hu", (acc+acc_Idx)->telnet_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("web_port", web_port, acc, "%hu", (acc+acc_Idx)->web_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("ftp_port", ftp_port, acc, "%hu", (acc+acc_Idx)->ftp_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("https", https, acc, "%hhu", (acc+acc_Idx)->https, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("https_port", https_port, acc, "%hu", (acc+acc_Idx)->https_port, 0);   //WORD_T
/* CHAIN ACC_TBL END*/
#endif

#ifdef PORT_TRIGGERING
/* CHAIN PORT_TRG_TBL*/
static int porttrg_Idx=0;
MIB_CE_PORT_TRG_T _porttrg[8];
MIB_CE_PORT_TRG_Tp porttrg= _porttrg;
XML_DIR_ARRAY("PORT_TRG_TBL", porttrg, root, porttrg_Idx, 8, MIB_PORT_TRG_TBL);
XML_ENTRY_STRING2("Name", name, porttrg, (porttrg+porttrg_Idx)->name, 0);   //STRING_T
XML_ENTRY_ADDR("IP", ip, porttrg, (porttrg+porttrg_Idx)->ip, 0);   //IA_T
XML_ENTRY_STRING2("TCP_Range", tcpRange, porttrg, (porttrg+porttrg_Idx)->tcpRange, 0);   //STRING_T
XML_ENTRY_STRING2("UDP_Range", udpRange, porttrg, (porttrg+porttrg_Idx)->udpRange, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("Enable", enable, porttrg, "%hhu", (porttrg+porttrg_Idx)->enable, 0);   //BYTE_T
/* CHAIN PORT_TRG_TBL END*/
#endif

#ifdef NATIP_FORWARDING
/* CHAIN IP_FW_TBL */
static int ipfw_Idx=0;
MIB_CE_IP_FW_T _ipfw[8];
MIB_CE_IP_FW_Tp ipfw=_ipfw;
XML_DIR_ARRAY("IP_FW_TBL", ipfw, root, ipfw_Idx, 8, MIB_IP_FW_TBL);
XML_ENTRY_PRIMITIVE2("Action", action, ipfw, "%hhu", (ipfw+ipfw_Idx)->action, 0);   //BYTE_T
XML_ENTRY_ADDR("Local_IP", local_ip, ipfw, (ipfw+ipfw_Idx)->local_ip, 0);   //IA_T
XML_ENTRY_ADDR("Remote_IP", remote_ip, ipfw, (ipfw+ipfw_Idx)->remote_ip, 0);   //IA_T
/* CHAIN IP_FW_TBL END*/
#endif

#ifdef URL_BLOCKING_SUPPORT
/* CHAIN URL_FQDN_TBL*/
static int urlfqdn_Idx=0;
MIB_CE_URL_FQDN_T _urlfqdn[8];
MIB_CE_URL_FQDN_Tp urlfqdn=_urlfqdn;
XML_DIR_ARRAY("URL_FQDN_TBL", urlfqdn, root, urlfqdn_Idx, 8, MIB_URL_FQDN_TBL);
XML_ENTRY_STRING2("URL", fqdn, urlfqdn, (urlfqdn+urlfqdn_Idx)->fqdn, 0);   //STRING_T
/* CHAIN URL_FQDN_TBL END*/

/* CHAIN KEYWD_FILTER_TBL*/
static int keywdfilter_Idx=0;
MIB_CE_KEYWD_FILTER_T _keywdfilter[8];
MIB_CE_KEYWD_FILTER_Tp keywdfilter=_keywdfilter;
XML_DIR_ARRAY("KEYWD_FILTER_TBL", keywdfilter, root, keywdfilter_Idx, 8, MIB_KEYWD_FILTER_TBL);
XML_ENTRY_STRING2("KEYWD", keyword, keywdfilter, (keywdfilter+keywdfilter_Idx)->keyword, 0);   //STRING_T
/* CHAIN KEYWD_FILTER_TBL END*/
#endif

#ifdef URL_ALLOWING_SUPPORT
/* CHAIN URL_ALLOW_FQDN_TBL*/
static int urlalwfqdn_Idx=0;
MIB_CE_URL_ALLOW_FQDN_T _urlalwfqdn[8];
MIB_CE_URL_ALLOW_FQDN_Tp urlalwfqdn=_urlalwfqdn;
XML_DIR_ARRAY("URL_ALLOW_FQDN_TBL", urlalwfqdn, root, urlalwfqdn_Idx, 8, MIB_URL_ALLOW_FQDN_TBL);
XML_ENTRY_STRING2("URLALW", fqdn, urlalwfqdn, (urlalwfqdn+urlalwfqdn_Idx)->fqdn, 0);   //STRING_T
/* CHAIN URL_ALLOW_FQDN_TBL END*/
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT  // Added by Mason Yu
/* CHAIN DOMAIN_BLOCKING_TBL*/
static int domainblk_Idx=0;
MIB_CE_DOMAIN_BLOCKING_T _domainblk[8];
MIB_CE_DOMAIN_BLOCKING_Tp domainblk=_domainblk;
XML_DIR_ARRAY("DOMAIN_BLOCKING_TBL", domainblk, root, domainblk_Idx, 8, MIB_DOMAIN_BLOCKING_TBL);
XML_ENTRY_STRING2("DOMAIN", domain, domainblk, (domainblk+domainblk_Idx)->domain, 0);   //STRING_T
/* CHAIN DOMAIN_BLOCKING_TBL END*/
#endif

#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
#ifdef WLAN_SUPPORT
/* CHAIN CWMP_PSK_TBL*/
static int cwmp_psk_Idx=0;
CWMP_PSK_T _cwmp_psk[8];
CWMP_PSK_Tp cwmp_psk=_cwmp_psk;
XML_DIR_ARRAY("CWMP_PSK_TBL", cwmp_psk, root, cwmp_psk_Idx, 8, CWMP_PSK_TBL);
XML_ENTRY_PRIMITIVE2("index", index, cwmp_psk, "%hhu", (cwmp_psk+cwmp_psk_Idx)->index, 0);   //BYTE_T
XML_ENTRY_STRING2("presharedkey", presharedkey, cwmp_psk, (cwmp_psk+cwmp_psk_Idx)->presharedkey, 0);   //STRING_T
XML_ENTRY_STRING2("keypassphrase", keypassphrase, cwmp_psk, (cwmp_psk+cwmp_psk_Idx)->keypassphrase, 0);   //STRING_T
/* CHAIN CWMP_PSK_TBL END*/
#endif
#endif /*_CWMP_MIB_*/

/* CHAIN MAC_BASE_DHCP_TBL*/
static int macBaseDhcp_Idx=0;
MIB_CE_MAC_BASE_DHCP_T _macBaseDhcp[8];
MIB_CE_MAC_BASE_DHCP_Tp macBaseDhcp=_macBaseDhcp;
XML_DIR_ARRAY("MAC_BASE_DHCP_TBL", macBaseDhcp, root, macBaseDhcp_Idx, 8, MIB_MAC_BASE_DHCP_TBL);
XML_ENTRY_BYTE6("macAddr", macAddr_Dhcp, macBaseDhcp, (macBaseDhcp+macBaseDhcp_Idx)->macAddr_Dhcp, 0);   //BYTE6_T
XML_ENTRY_ADDR("ipAddr", ipAddr_Dhcp, macBaseDhcp, (macBaseDhcp+macBaseDhcp_Idx)->ipAddr_Dhcp, 0);   //IA_T
/* CHAIN MAC_BASE_DHCP_TBL END*/

// add for auto-pvc-search
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
/* CHAIN AUTO_PVC_SEARCH_TBL*/
static int AutoPvcSearch_Idx=0;
MIB_AUTO_PVC_SEARCH_T _AutoPvcSearch[16];
MIB_AUTO_PVC_SEARCH_Tp AutoPvcSearch=_AutoPvcSearch;
XML_DIR_ARRAY("AUTO_PVC_SEARCH_TBL", AutoPvcSearch, root, AutoPvcSearch_Idx, 16, MIB_AUTO_PVC_SEARCH_TBL);
XML_ENTRY_PRIMITIVE2("VPI", vpi, AutoPvcSearch, "%hu", (AutoPvcSearch+AutoPvcSearch_Idx)->vpi, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("VCI", vci, AutoPvcSearch, "%u", (AutoPvcSearch+AutoPvcSearch_Idx)->vci, 0);   //DWORD_T
/* CHAIN AUTO_PVC_SEARCH_TBL END*/
#endif
#ifdef CONFIG_USER_DDNS
/* CHAIN MIB_DDNS_TBL*/
static int ddns_Idx=0;
MIB_CE_DDNS_T _ddns[8];
MIB_CE_DDNS_Tp ddns=_ddns;
XML_DIR_ARRAY("MIB_DDNS_TBL", ddns, root, ddns_Idx, 8, MIB_DDNS_TBL);
XML_ENTRY_STRING2("Provider", provider, ddns, (ddns+ddns_Idx)->provider, 0);   //STRING_T
XML_ENTRY_STRING2("Hostname", hostname, ddns, (ddns+ddns_Idx)->hostname, 0);   //STRING_T
XML_ENTRY_STRING2("Interface", interface, ddns, (ddns+ddns_Idx)->interface, 0);   //STRING_T
XML_ENTRY_STRING2("Username", username, ddns, (ddns+ddns_Idx)->username, 0);   //STRING_T
XML_ENTRY_STRING2("Password", password, ddns, (ddns+ddns_Idx)->password, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("Enabled", Enabled, ddns, "%hhu", (ddns+ddns_Idx)->Enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, ddns, "%d", (ddns+ddns_Idx)->InstanceNum, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("ServicePort", ServicePort, ddns, "%hu", (ddns+ddns_Idx)->ServicePort, 0);   //WORD_T
/* CHAIN MIB_DDNS_TBL END*/
#endif

/* CHAIN PPPOE_SESSION_TBL*/
static int pppoeSession_Idx=0;
MIB_CE_PPPOE_SESSION_T _pppoeSession[MAX_INFINITE];
MIB_CE_PPPOE_SESSION_Tp pppoeSession=_pppoeSession;
XML_DIR_ARRAY("PPPOE_SESSION_TBL", pppoeSession, root, pppoeSession_Idx, MAX_INFINITE, MIB_PPPOE_SESSION_TBL);
XML_ENTRY_PRIMITIVE2("InterfaceNo", uifno, pppoeSession, "%u", (pppoeSession+pppoeSession_Idx)->uifno, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("SessionID", sessionId, pppoeSession, "%hu", (pppoeSession+pppoeSession_Idx)->sessionId, 0);   //WORD_T
XML_ENTRY_BYTE6("ACMacAddr", acMac, pppoeSession, (pppoeSession+pppoeSession_Idx)->acMac, 0);   //BYTE6_T
/* CHAIN PPPOE_SESSION_TBL END*/

#ifdef ACCOUNT_CONFIG
/* CHAIN ACCOUNT_CONFIG_TBL*/
static int accountConfig_Idx=0;
MIB_CE_ACCOUNT_CONFIG_T _accountConfig[8];
MIB_CE_ACCOUNT_CONFIG_Tp accountConfig=_accountConfig;
XML_DIR_ARRAY("ACCOUNT_CONFIG_TBL", accountConfig, root, accountConfig_Idx, 8, MIB_ACCOUNT_CONFIG_TBL);
XML_ENTRY_STRING2("userName", userName, _accountConfig, (_accountConfig+accountConfig_Idx)->userName, 0);   //STRING_T
XML_ENTRY_STRING2("userPassword", userPassword, _accountConfig, (_accountConfig+accountConfig_Idx)->userPassword, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("privilege", privilege, _accountConfig, "%hhu", (_accountConfig+accountConfig_Idx)->privilege, 0);   //BYTE_T
/* CHAIN ACCOUNT_CONFIG_TBL END*/
#endif

/* CHAIN RIP_TBL*/
static int rip_Idx=0;
MIB_CE_RIP_T _rip[MAX_INFINITE];
MIB_CE_RIP_Tp rip=_rip;
XML_DIR_ARRAY("RIP_TBL", rip, root, rip_Idx, MAX_INFINITE, MIB_RIP_TBL);
XML_ENTRY_PRIMITIVE2("IfIndex", ifIndex, rip, "%d", (rip+rip_Idx)->ifIndex, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ReceiveMode", receiveMode, rip, "%hhu", (rip+rip_Idx)->receiveMode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("SendMode", sendMode, rip, "%hhu", (rip+rip_Idx)->sendMode, 0);   //BYTE_T
/* CHAIN RIP_TBL END*/

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
/* CHAIN OSPF_TBL*/
static int ospf_Idx=0;
MIB_CE_OSPF_T _ospf[MAX_INFINITE];
MIB_CE_OSPF_Tp ospf=_ospf;
XML_DIR_ARRAY("OSPF_TBL", ospf, root, ospf_Idx, MAX_INFINITE, MIB_OSPF_TBL);
XML_ENTRY_ADDR("ipAddr", ipAddr, ospf, (ospf+ospf_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_ADDR("netMask", netMask, ospf, (ospf+ospf_Idx)->netMask, 0);   //IA_T
/* CHAIN OSPF_TBL END*/
#endif

#ifdef WLAN_SUPPORT
/* CHAIN MBSSIB_TBL*/
static int mbssid_Idx=0;
MIB_CE_MBSSIB_T _mbssid[6 * NUM_WLAN_INTERFACE];
MIB_CE_MBSSIB_Tp mbssid=_mbssid;
XML_DIR_ARRAY("WLAN_MBSSIB_TBL", mbssid, root, mbssid_Idx, (6 * NUM_WLAN_INTERFACE), MIB_MBSSIB_TBL);
XML_ENTRY_PRIMITIVE2("idx", idx, mbssid, "%hhu", (mbssid+mbssid_Idx)->idx, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("encrypt", encrypt, mbssid, "%hhu", (mbssid+mbssid_Idx)->encrypt, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("enable1X", enable1X, mbssid, "%hhu", (mbssid+mbssid_Idx)->enable1X, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wep", wep, mbssid, "%hhu", (mbssid+mbssid_Idx)->wep, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpaAuth", wpaAuth, mbssid, "%hhu", (mbssid+mbssid_Idx)->wpaAuth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpaPSKFormat", wpaPSKFormat, mbssid, "%hhu", (mbssid+mbssid_Idx)->wpaPSKFormat, 0);   //BYTE_T
XML_ENTRY_STRING2("wpaPSK", wpaPSK, mbssid, (mbssid+mbssid_Idx)->wpaPSK, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wpaGroupRekeyTime", wpaGroupRekeyTime, mbssid, "%ld", (mbssid+mbssid_Idx)->wpaGroupRekeyTime, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("rsPort", rsPort, mbssid, "%hu", (mbssid+mbssid_Idx)->rsPort, 0);   //WORD_T
XML_ENTRY_ADDR("rsIpAddr", rsIpAddr, mbssid, (mbssid+mbssid_Idx)->rsIpAddr, 0);   //IA_T
XML_ENTRY_STRING2("rsPassword", rsPassword, mbssid, (mbssid+mbssid_Idx)->rsPassword, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wlanDisabled", wlanDisabled, mbssid, "%hhu", (mbssid+mbssid_Idx)->wlanDisabled, 0);   //BYTE_T
XML_ENTRY_STRING2("ssid", ssid, mbssid, (mbssid+mbssid_Idx)->ssid, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wlanMode", wlanMode, mbssid, "%hhu", (mbssid+mbssid_Idx)->wlanMode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("authType", authType, mbssid, "%hhu", (mbssid+mbssid_Idx)->authType, 0);   //BYTE_T
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
XML_ENTRY_PRIMITIVE2("cwmp_WLAN_BasicEncry", cwmp_WLAN_BasicEncry, mbssid, "%hhu", (mbssid+mbssid_Idx)->cwmp_WLAN_BasicEncry, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("unicastCipher", unicastCipher, mbssid, "%hhu", (mbssid+mbssid_Idx)->unicastCipher, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpa2UnicastCipher", wpa2UnicastCipher, mbssid, "%hhu", (mbssid+mbssid_Idx)->wpa2UnicastCipher, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bcnAdvtisement", bcnAdvtisement, mbssid, "%hhu", (mbssid+mbssid_Idx)->bcnAdvtisement, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("hidessid", hidessid, mbssid, "%hhu", (mbssid+mbssid_Idx)->hidessid, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("userIsolation", userisolation, mbssid, "%hhu", (mbssid+mbssid_Idx)->userisolation, 0);   //BYTE_T
#ifdef CONFIG_RTL_WAPI_SUPPORT
XML_ENTRY_STRING2("wapiPsk", wapiPsk, mbssid, (mbssid+mbssid_Idx)->wapiPsk, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wapiPskLen", wapiPskLen, mbssid, "%hhu", (mbssid+mbssid_Idx)->wapiPskLen, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wapiAuth", wapiAuth, mbssid, "%hhu", (mbssid+mbssid_Idx)->wapiAuth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wapiPskFormat", wapiPskFormat, mbssid, "%hhu", (mbssid+mbssid_Idx)->wapiPskFormat, 0);   //BYTE_T
XML_ENTRY_BYTE_ARRAY("wapiAsIpAddr", wapiAsIpAddr, mbssid, (mbssid+mbssid_Idx)->wapiAsIpAddr, 0); //BYTE_ARRAY_T
#endif
XML_ENTRY_PRIMITIVE2("wepKeyType", wepKeyType, mbssid, "%hhu", (mbssid+mbssid_Idx)->wepKeyType, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wepDefaultKey", wepDefaultKey, mbssid, "%hhu", (mbssid+mbssid_Idx)->wepDefaultKey, 0);   //BYTE_T
XML_ENTRY_STRING2("wep64Key1", wep64Key1, mbssid, (mbssid+mbssid_Idx)->wep64Key1, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key2", wep64Key2, mbssid, (mbssid+mbssid_Idx)->wep64Key2, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key3", wep64Key3, mbssid, (mbssid+mbssid_Idx)->wep64Key3, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key4", wep64Key4, mbssid, (mbssid+mbssid_Idx)->wep64Key4, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key1", wep128Key1, mbssid, (mbssid+mbssid_Idx)->wep128Key1, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key2", wep128Key2, mbssid, (mbssid+mbssid_Idx)->wep128Key2, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key3", wep128Key3, mbssid, (mbssid+mbssid_Idx)->wep128Key3, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key4", wep128Key4, mbssid, (mbssid+mbssid_Idx)->wep128Key4, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wmmEnabled", wmmEnabled, mbssid, "%hhu", (mbssid+mbssid_Idx)->wmmEnabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("rateAdaptiveEnabled", rateAdaptiveEnabled, mbssid, "%hhu", (mbssid+mbssid_Idx)->rateAdaptiveEnabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wlanBand", wlanBand, mbssid, "%hhu", (mbssid+mbssid_Idx)->wlanBand, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("fixedTxRate", fixedTxRate, mbssid, "%d", (mbssid+mbssid_Idx)->fixedTxRate, 0);   //INTEGER_T
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
XML_ENTRY_PRIMITIVE2("wsc_disabled", wsc_disabled, mbssid, "%hhu", (mbssid+mbssid_Idx)->wsc_disabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_configured", wsc_configured, mbssid, "%hhu", (mbssid+mbssid_Idx)->wsc_configured, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_upnp_enabled", wsc_upnp_enabled, mbssid, "%hhu", (mbssid+mbssid_Idx)->wsc_upnp_enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_auth", wsc_auth, mbssid, "%hhu", (mbssid+mbssid_Idx)->wsc_auth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_enc", wsc_enc, mbssid, "%hhu", (mbssid+mbssid_Idx)->wsc_enc, 0);   //BYTE_T
XML_ENTRY_STRING2("wscPsk", wscPsk, mbssid, (mbssid+mbssid_Idx)->wscPsk, 0);   //STRING_T
#endif
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
XML_ENTRY_PRIMITIVE2("mlcstRate", mlcstRate, mbssid, "%hu", (mbssid+mbssid_Idx)->mlcstRate, 0);   //WORD_T
#endif
#ifdef WLAN_11W
XML_ENTRY_PRIMITIVE2("dotIEEE80211W", dotIEEE80211W, mbssid, "%hhu", (mbssid+mbssid_Idx)->dotIEEE80211W, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("sha256", sha256, mbssid, "%hhu", (mbssid+mbssid_Idx)->sha256, 0);   //BYTE_T
#endif
#ifdef WLAN_11R
XML_ENTRY_PRIMITIVE2("WLAN_FT_ENABLE", ft_enable, mbssid, "%hhu", (mbssid+mbssid_Idx)->ft_enable, 0);   //BYTE_T
XML_ENTRY_STRING2("WLAN_FT_MDID", ft_mdid, mbssid, (mbssid+mbssid_Idx)->ft_mdid, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_OVER_DS", ft_over_ds, mbssid, "%hhu", (mbssid+mbssid_Idx)->ft_over_ds, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_RES_REQUEST", ft_res_request, mbssid, "%hhu", (mbssid+mbssid_Idx)->ft_res_request, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_R0KEY_TO", ft_r0key_timeout, mbssid, "%d", (mbssid+mbssid_Idx)->ft_r0key_timeout, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_REASOC_TO", ft_reasoc_timeout, mbssid, "%d", (mbssid+mbssid_Idx)->ft_reasoc_timeout, 0);   //INTEGER_T
XML_ENTRY_STRING2("WLAN_FT_R0KH_ID", ft_r0kh_id, mbssid, (mbssid+mbssid_Idx)->ft_r0kh_id, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_PUSH", ft_push, mbssid, "%hhu", (mbssid+mbssid_Idx)->ft_push, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_KH_NUM", ft_kh_num, mbssid, "%hhu", (mbssid+mbssid_Idx)->ft_kh_num, 0);   //BYTE_T
#endif
#ifdef WLAN_LIMITED_STA_NUM
XML_ENTRY_PRIMITIVE2("stanum", stanum, mbssid, "%u", (mbssid+mbssid_Idx)->stanum, 0);   //INTEGER_T
#endif
/* CHAIN MBSSIB_TBL END*/

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
/* CHAIN WLAN1_MBSSIB_TBL*/
static int wlan1_mbssid_Idx=0;
MIB_CE_MBSSIB_T _wlan1_mbssid[6 * NUM_WLAN_INTERFACE];
MIB_CE_MBSSIB_Tp wlan1_mbssid=_wlan1_mbssid;
XML_DIR_ARRAY("WLAN1_MBSSIB_TBL", wlan1_mbssid, root, wlan1_mbssid_Idx, (6 * NUM_WLAN_INTERFACE), MIB_WLAN1_MBSSIB_TBL);
XML_ENTRY_PRIMITIVE2("idx", idx, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->idx, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("encrypt", encrypt, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->encrypt, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("enable1X", enable1X, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->enable1X, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wep", wep, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wep, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpaAuth", wpaAuth, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wpaAuth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpaPSKFormat", wpaPSKFormat, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wpaPSKFormat, 0);   //BYTE_T
XML_ENTRY_STRING2("wpaPSK", wpaPSK, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wpaPSK, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wpaGroupRekeyTime", wpaGroupRekeyTime, wlan1_mbssid, "%ld", (wlan1_mbssid+wlan1_mbssid_Idx)->wpaGroupRekeyTime, 0);   //DWORD_T
XML_ENTRY_PRIMITIVE2("rsPort", rsPort, wlan1_mbssid, "%hu", (wlan1_mbssid+wlan1_mbssid_Idx)->rsPort, 0);   //WORD_T
XML_ENTRY_ADDR("rsIpAddr", rsIpAddr, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->rsIpAddr, 0);   //IA_T
XML_ENTRY_STRING2("rsPassword", rsPassword, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->rsPassword, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wlanDisabled", wlanDisabled, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wlanDisabled, 0);   //BYTE_T
XML_ENTRY_STRING2("ssid", ssid, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->ssid, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wlanMode", wlanMode, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wlanMode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("authType", authType, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->authType, 0);   //BYTE_T
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
XML_ENTRY_PRIMITIVE2("cwmp_WLAN_BasicEncry", cwmp_WLAN_BasicEncry, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->cwmp_WLAN_BasicEncry, 0);   //BYTE_T
#endif
XML_ENTRY_PRIMITIVE2("unicastCipher", unicastCipher, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->unicastCipher, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wpa2UnicastCipher", wpa2UnicastCipher, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wpa2UnicastCipher, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("bcnAdvtisement", bcnAdvtisement, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->bcnAdvtisement, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("hidessid", hidessid, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->hidessid, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("userIsolation", userisolation, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->userisolation, 0);   //BYTE_T
#ifdef CONFIG_RTL_WAPI_SUPPORT
XML_ENTRY_STRING2("wapiPsk", wapiPsk, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wapiPsk, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wapiPskLen", wapiPskLen, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wapiPskLen, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wapiAuth", wapiAuth, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wapiAuth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wapiPskFormat", wapiPskFormat, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wapiPskFormat, 0);   //BYTE_T
XML_ENTRY_BYTE_ARRAY("wapiAsIpAddr", wapiAsIpAddr, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wapiAsIpAddr, 0); //BYTE_ARRAY_T
#endif
XML_ENTRY_PRIMITIVE2("wepKeyType", wepKeyType, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wepKeyType, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wepDefaultKey", wepDefaultKey, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wepDefaultKey, 0);   //BYTE_T
XML_ENTRY_STRING2("wep64Key1", wep64Key1, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep64Key1, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key2", wep64Key2, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep64Key2, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key3", wep64Key3, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep64Key3, 0);   //STRING_T
XML_ENTRY_STRING2("wep64Key4", wep64Key4, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep64Key4, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key1", wep128Key1, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep128Key1, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key2", wep128Key2, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep128Key2, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key3", wep128Key3, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep128Key3, 0);   //STRING_T
XML_ENTRY_STRING2("wep128Key4", wep128Key4, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wep128Key4, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("wmmEnabled", wmmEnabled, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wmmEnabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("rateAdaptiveEnabled", rateAdaptiveEnabled, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->rateAdaptiveEnabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wlanBand", wlanBand, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wlanBand, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("fixedTxRate", fixedTxRate, wlan1_mbssid, "%d", (wlan1_mbssid+wlan1_mbssid_Idx)->fixedTxRate, 0);   //INTEGER_T
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
XML_ENTRY_PRIMITIVE2("wsc_disabled", wsc_disabled, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wsc_disabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_configured", wsc_configured, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wsc_configured, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_upnp_enabled", wsc_upnp_enabled, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wsc_upnp_enabled, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_auth", wsc_auth, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wsc_auth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("wsc_enc", wsc_enc, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->wsc_enc, 0);   //BYTE_T
XML_ENTRY_STRING2("wscPsk", wscPsk, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->wscPsk, 0);   //STRING_T
#endif
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
XML_ENTRY_PRIMITIVE2("mlcstRate", mlcstRate, wlan1_mbssid, "%hu", (wlan1_mbssid+wlan1_mbssid_Idx)->mlcstRate, 0);   //WORD_T
#endif
#ifdef WLAN_11W
XML_ENTRY_PRIMITIVE2("dotIEEE80211W", dotIEEE80211W, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->dotIEEE80211W, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("sha256", sha256, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->sha256, 0);   //BYTE_T
#endif
#ifdef WLAN_11R
XML_ENTRY_PRIMITIVE2("WLAN_FT_ENABLE", ft_enable, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_enable, 0);   //BYTE_T
XML_ENTRY_STRING2("WLAN_FT_MDID", ft_mdid, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->ft_mdid, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_OVER_DS", ft_over_ds, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_over_ds, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_RES_REQUEST", ft_res_request, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_res_request, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_R0KEY_TO", ft_r0key_timeout, wlan1_mbssid, "%d", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_r0key_timeout, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_REASOC_TO", ft_reasoc_timeout, wlan1_mbssid, "%d", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_reasoc_timeout, 0);   //INTEGER_T
XML_ENTRY_STRING2("WLAN_FT_R0KH_ID", ft_r0kh_id, wlan1_mbssid, (wlan1_mbssid+wlan1_mbssid_Idx)->ft_r0kh_id, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_PUSH", ft_push, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_push, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("WLAN_FT_KH_NUM", ft_kh_num, wlan1_mbssid, "%hhu", (wlan1_mbssid+wlan1_mbssid_Idx)->ft_kh_num, 0);   //BYTE_T
#endif
#ifdef WLAN_LIMITED_STA_NUM
XML_ENTRY_PRIMITIVE2("stanum", stanum, wlan1_mbssid, "%u", (wlan1_mbssid+wlan1_mbssid_Idx)->stanum, 0);   //INTEGER_T
#endif
/* CHAIN WLAN1_MBSSIB_TBL END*/
#endif /*END of defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)*/

#endif

#ifdef LAYER7_FILTER_SUPPORT
/* CHAIN LAYER7_FILTER_TBL*/
static int layer7_Idx=0;
LAYER7_FILTER_T _layer7[8];
LAYER7_FILTER_Tp layer7=_layer7;
XML_DIR_ARRAY("LAYER7_FILTER_TBL", layer7, root, layer7_Idx, 8, MIB_LAYER7_FILTER_TBL);
XML_ENTRY_STRING2("appname", appname, _layer7, (_layer7+layer7_Idx)->appname, 0);
/* CHAIN LAYER7_FILTER_TBL END*/
#endif

#ifdef QOS_SPEED_LIMIT_SUPPORT
/* CHAIN MBQOS_SPEED_LIMIT*/
static int ipqos_speedrank_Idx=0;
MIB_CE_IP_QOS_SPEEDRANK_T _ipqos_speedrank[MAX_INFINITE];
MIB_CE_IP_QOS_SPEEDRANK_Tp *ipqos_speedrank= _ipqos_speedrank;	//FIXME, if structure fixed
XML_DIR_ARRAY("MBQOS_SPEED_LIMIT", ipqos_speedrank, root, ipqos_speedrank_Idx, MAX_INFINITE, MIB_QOS_SPEED_LIMIT);
XML_ENTRY_PRIMITIVE2("index", index, ipqos_speedrank, "%hhu", (ipqos_speedrank+ipqos_speedrank_Idx)->index, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("prior", prior, ipqos_speedrank, "%hhu", (ipqos_speedrank+ipqos_speedrank_Idx)->prior, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("speed", speed, ipqos_speedrank, "%hhu", (ipqos_speedrank+ipqos_speedrank_Idx)->speed, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("count", count, ipqos_speedrank, "%hhu", (ipqos_speedrank+ipqos_speedrank_Idx)->count, 0);   //BYTE_T
/* CHAIN MBQOS_SPEED_LIMIT END*/
#endif

#ifdef VOIP_SUPPORT
voipCfgParam_t _voipCfgParam[1];
voipCfgParam_t *voipCfgParam = _voipCfgParam;
#endif /*VOIP_SUPPORT*/

#ifdef PARENTAL_CTRL
/* CHAIN PARENTAL_CTRL_TBL*/
static int parentctrl_Idx=0;
MIB_PARENT_CTRL_T _parentctrl[MAX_PARENTCTRL_USER_NUM];
MIB_PARENT_CTRL_Tp parentctrl= _parentctrl;	//FIXME, if structure fixed
XML_DIR_ARRAY("PARENTAL_CTRL_TBL", parentctrl, root, parentctrl_Idx, MAX_PARENTCTRL_USER_NUM, MIB_PARENTAL_CTRL_TBL);
XML_ENTRY_PRIMITIVE2("SpecfiedPC", specfiedPC, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->specfiedPC, 0);   //BYTE_T
XML_ENTRY_STRING2("Username", username, parentctrl, (parentctrl+parentctrl_Idx)->username, 0);   //STRING_T
XML_ENTRY_BYTE6("MAC", mac, parentctrl, (parentctrl+parentctrl_Idx)->mac, 0);   //BYTE6_T
XML_ENTRY_ADDR("StartIP", sip, parentctrl, (parentctrl+parentctrl_Idx)->sip, 0);   //IA_T
XML_ENTRY_ADDR("EndIP", eip, parentctrl, (parentctrl+parentctrl_Idx)->eip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("Day", controlled_day, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->controlled_day, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Start_hr", start_hr, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->start_hr, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("Start_min", start_min, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->start_min, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("End_hr", end_hr, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->end_hr, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("End_min", end_min, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->end_min, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("State", cur_state, parentctrl, "%hhu", (parentctrl+parentctrl_Idx)->cur_state, 0);   //BYTE_T
/* CHAIN PARENTAL_CTRL_TBL END*/
#endif

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_

/* CHAIN DHCPS_SERVING_POOL_TBL*/
static int dhcp_sp_Idx=0;
DHCPS_SERVING_POOL_T _dhcp_sp[MAX_INFINITE];
DHCPS_SERVING_POOL_Tp dhcp_sp= _dhcp_sp;	//FIXME, if structure fixed
XML_DIR_ARRAY("DHCPS_SERVING_POOL_TBL", dhcp_sp, root, dhcp_sp_Idx, MAX_INFINITE, MIB_DHCPS_SERVING_POOL_TBL);
XML_ENTRY_PRIMITIVE2("enable", enable, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("poolorder", poolorder, dhcp_sp, "%u", (dhcp_sp+dhcp_sp_Idx)->poolorder, 0);   //DWORD_T
XML_ENTRY_STRING2("poolname", poolname, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->poolname, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("deviceType", deviceType, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->deviceType, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("rsvOptCode", rsvOptCode, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->rsvOptCode, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("sourceinterface", sourceinterface, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->sourceinterface, 0);   //BYTE_T
XML_ENTRY_STRING2("vendorclass", vendorclass, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->vendorclass, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("vendorclassflag", vendorclassflag, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->vendorclassflag, 0);   //BYTE_T
XML_ENTRY_STRING2("vendorclassmode", vendorclassmode, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->vendorclassmode, 0);   //STRING_T
XML_ENTRY_STRING2("clientid", clientid, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->clientid, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("clientidflag", clientidflag, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->clientidflag, 0);   //BYTE_T
XML_ENTRY_STRING2("userclass", userclass, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->userclass, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("userclassflag", userclassflag, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->userclassflag, 0);   //BYTE_T
XML_ENTRY_BYTE6("chaddr", chaddr, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->chaddr, 0);   //BYTE6_T
XML_ENTRY_BYTE6("chaddrmask", chaddrmask, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->chaddrmask, 0);   //BYTE6_T
XML_ENTRY_PRIMITIVE2("chaddrflag", chaddrflag, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->chaddrflag, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("localserved", localserved, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->localserved, 0);   //BYTE_T
XML_ENTRY_ADDR("startaddr", startaddr, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->startaddr, 0);   //IA_T
XML_ENTRY_ADDR("endaddr", endaddr, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->endaddr, 0);   //IA_T
XML_ENTRY_ADDR("subnetmask", subnetmask, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->subnetmask, 0);   //IA_T
XML_ENTRY_ADDR("iprouter", iprouter, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->iprouter, 0);   //IA_T
XML_ENTRY_ADDR("dnsserver1", dnsserver1, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->dnsserver1, 0);   //IA_T
XML_ENTRY_ADDR("dnsserver2", dnsserver2, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->dnsserver2, 0);   //IA_T
XML_ENTRY_ADDR("dnsserver3", dnsserver3, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->dnsserver3, 0);   //IA_T
XML_ENTRY_STRING2("domainname", domainname, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->domainname, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("leasetime", leasetime, dhcp_sp, "%d", (dhcp_sp+dhcp_sp_Idx)->leasetime, 0);   //INTEGER_T
XML_ENTRY_ADDR("dhcprelayip", dhcprelayip, dhcp_sp, (dhcp_sp+dhcp_sp_Idx)->dhcprelayip, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("dnsservermode", dnsservermode, dhcp_sp, "%hhu", (dhcp_sp+dhcp_sp_Idx)->dnsservermode, 0);   //BYTE_T
#ifdef _CWMP_MIB_
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, dhcp_sp, "%u", (dhcp_sp+dhcp_sp_Idx)->InstanceNum, 0);   //DWORD_T
#endif
/* CHAIN DHCPS_SERVING_POOL_TBL END*/
#endif

#ifdef TCP_UDP_CONN_LIMIT
/* CHAIN TCP_UDP_CONN_TBL*/
static int tucl_Idx=0;
MIB_CE_TCP_UDP_CONN_LIMIT_T _tucl[8];
MIB_CE_TCP_UDP_CONN_LIMIT_Tp tucl=_tucl;
XML_DIR_ARRAY("TCP_UDP_CONN_TBL", tucl, root, tucl_Idx, 8, MIB_TCP_UDP_CONN_LIMIT_TBL);
XML_ENTRY_ADDR("IPAddr", ipAddr, tucl, (tucl+tucl_Idx)->ipAddr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("State", Enabled, tucl, "%hhu", (tucl+tucl_Idx)->Enabled, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("Protocol", protocol, tucl, "%hhu", (tucl+tucl_Idx)->protocol, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("connNum", connNum, tucl, "%u", (tucl+tucl_Idx)->connNum, 0);   //IA_T
/* CHAIN TCP_UDP_CONN_TBL END*/
#endif

#ifdef WEB_REDIRECT_BY_MAC
/* CHAIN WEB_REDIR_BY_MAC_TBL*/
static int wrbm_Idx=0;
MIB_WEB_REDIR_BY_MAC_T _wrbm[MAX_WEB_REDIR_BY_MAC];
MIB_WEB_REDIR_BY_MAC_Tp wrbm=_wrbm;
XML_DIR_ARRAY("WEB_REDIR_BY_MAC_TBL", wrbm, root, wrbm_Idx, MAX_WEB_REDIR_BY_MAC, MIB_WEB_REDIR_BY_MAC_TBL);
XML_ENTRY_BYTE6("MAC", mac, wrbm, (wrbm+wrbm_Idx)->mac, 0);   //BYTE6_T
/* CHAIN WEB_REDIR_BY_MAC_TBL END*/
#endif

#ifdef MULTI_ADDRESS_MAPPING
/* CHAIN MULTI_ADDR_MAPPING_TBL*/
static int muladdrmap_Idx=0;
MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T _muladdrmap[16];
MIB_CE_MULTI_ADDR_MAPPING_LIMIT_Tp muladdrmap=_muladdrmap;
XML_DIR_ARRAY("MULTI_ADDR_MAPPING_TBL", muladdrmap, root, muladdrmap_Idx, 16, MULTI_ADDRESS_MAPPING_LIMIT_TBL);
XML_ENTRY_PRIMITIVE2("AddressMappingType", addressMapType, muladdrmap, "%hhu", (muladdrmap+muladdrmap_Idx)->addressMapType, 0);   //BYTE_T
XML_ENTRY_ADDR("LocalStartIP", lsip, muladdrmap, (muladdrmap+muladdrmap_Idx)->lsip, 0);   //IA_T
XML_ENTRY_ADDR("LocalEndIP", leip, muladdrmap, (muladdrmap+muladdrmap_Idx)->leip, 0);   //IA_T
XML_ENTRY_ADDR("GlobalStartIP", gsip, muladdrmap, (muladdrmap+muladdrmap_Idx)->gsip, 0);   //IA_T
XML_ENTRY_ADDR("GlobalEndIP", geip, muladdrmap, (muladdrmap+muladdrmap_Idx)->geip, 0);   //IA_T
/* CHAIN MULTI_ADDR_MAPPING_TBL END*/
#endif //MULTI_ADDRESS_MAPPING

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
/* CHAIN CAPTIVEPORTAL_ALLOWED_LIST*/
static int cpal_Idx=0;
CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T _cpal[MAX_ALLOWED_LIST];
CWMP_CAPTIVEPORTAL_ALLOWED_LIST_Tp cpal=_cpal;
XML_DIR_ARRAY("CAPTIVEPORTAL_ALLOWED_LIST", cpal, root, cpal_Idx, MAX_ALLOWED_LIST, CWMP_CAPTIVEPORTAL_ALLOWED_LIST);
XML_ENTRY_ADDR("ipAddr", ip_addr, cpal, (cpal+cpal_Idx)->ip_addr, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("mask", mask, cpal, "%hhu", (cpal+cpal_Idx)->mask, 0);   //BYTE_T
/* CHAIN CAPTIVEPORTAL_ALLOWED_LIST END*/
#endif

#ifdef WLAN_QoS
/* CHAIN WLAN_QOS_AP_TBL*/
static int wlan_qos_ap_Idx=0;
MIB_WLAN_QOS_T _wlan_qos_ap[4];
MIB_WLAN_QOS_Tp wlan_qos_ap=_wlan_qos_ap;
XML_DIR_ARRAY("WLAN_QOS_AP_TBL", wlan_qos_ap, root, wlan_qos_ap_Idx, 4, MIB_WLAN_QOS_AP_TBL);
XML_ENTRY_PRIMITIVE2("txop", txop, wlan_qos_ap, "%u", (wlan_qos_ap+wlan_qos_ap_Idx)->txop, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ecwmin", ecwmin, wlan_qos_ap, "%u", (wlan_qos_ap+wlan_qos_ap_Idx)->ecwmin, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ecwmax", ecwmax, wlan_qos_ap, "%u", (wlan_qos_ap+wlan_qos_ap_Idx)->ecwmax, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("aifsn", aifsn, wlan_qos_ap, "%u", (wlan_qos_ap+wlan_qos_ap_Idx)->aifsn, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ack", ack, wlan_qos_ap, "%u", (wlan_qos_ap+wlan_qos_ap_Idx)->ack, 0);   //INTEGER_T
/* CHAIN WLAN_QOS_AP_TBL END*/

/* CHAIN WLAN_QOS_STA_TBL*/
static int wlan_qos_sta_Idx=0;
MIB_WLAN_QOS_T _wlan_qos_sta[4];
MIB_WLAN_QOS_Tp wlan_qos_sta=_wlan_qos_sta;
XML_DIR_ARRAY("WLAN_QOS_STA_TBL", wlan_qos_sta, root, wlan_qos_sta_Idx, 4, MIB_WLAN_QOS_STA_TBL);
XML_ENTRY_PRIMITIVE2("txop", txop, wlan_qos_sta, "%u", (wlan_qos_sta+wlan_qos_sta_Idx)->txop, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ecwmin", ecwmin, wlan_qos_sta, "%u", (wlan_qos_sta+wlan_qos_sta_Idx)->ecwmin, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ecwmax", ecwmax, wlan_qos_sta, "%u", (wlan_qos_sta+wlan_qos_sta_Idx)->ecwmax, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("aifsn", aifsn, wlan_qos_sta, "%u", (wlan_qos_sta+wlan_qos_sta_Idx)->aifsn, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ack", ack, wlan_qos_sta, "%u", (wlan_qos_sta+wlan_qos_sta_Idx)->ack, 0);   //INTEGER_T
/* CHAIN WLAN_QOS_STA_TBL END*/
#endif //WLAN_QoS

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
/* CHAIN DHCP_RESERVED_IPADDR_TBL*/
static int dria_Idx=0;
MIB_DHCP_RESERVED_IPADDR_T _dria[MAX_INFINITE];
MIB_DHCP_RESERVED_IPADDR_Tp dria=_dria;
XML_DIR_ARRAY("DHCP_RESERVED_IPADDR_TBL", dria, root, dria_Idx, MAX_INFINITE, MIB_DHCP_RESERVED_IPADDR_TBL);
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, dria, "%u", (dria+dria_Idx)->InstanceNum, 0);   //DWORD_T
XML_ENTRY_ADDR("IPAddr", IPAddr, dria, (dria+dria_Idx)->IPAddr, 0);   //IA_T
/* CHAIN DHCP_RESERVED_IPADDR_TBL END*/
#endif //SUPPORT_DHCP_RESERVED_IPADDR

#ifdef CONFIG_USER_PPPOMODEM
/* CHAIN WAN_3G_TBL*/
static int wan_3g_Idx=0;
MIB_WAN_3G_T _wan_3g[1];
MIB_WAN_3G_Tp wan_3g=_wan_3g;
XML_DIR_ARRAY("WAN_3G_TBL", wan_3g, root, wan_3g_Idx, 1, MIB_WAN_3G_TBL);
XML_ENTRY_PRIMITIVE2("enable", enable, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->enable, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("auth", auth, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->auth, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("ctype", ctype, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->ctype, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("napt", napt, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->napt, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("pin", pin, wan_3g, "%hu", (wan_3g+wan_3g_Idx)->pin, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("idletime", idletime, wan_3g, "%hu", (wan_3g+wan_3g_Idx)->idletime, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("mtu", mtu, wan_3g, "%hu", (wan_3g+wan_3g_Idx)->mtu, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("dgw", dgw, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->dgw, 0);   //BYTE_T
XML_ENTRY_STRING2("apn", apn, wan_3g, (wan_3g+wan_3g_Idx)->apn, 0);   //STRING_T
XML_ENTRY_STRING2("dial", dial, wan_3g, (wan_3g+wan_3g_Idx)->dial, 0);   //STRING_T
XML_ENTRY_STRING2("username", username, wan_3g, (wan_3g+wan_3g_Idx)->username, 0);   //STRING_T
XML_ENTRY_STRING2("password", password, wan_3g, (wan_3g+wan_3g_Idx)->password, 0);   //STRING_T
XML_ENTRY_PRIMITIVE2("backup", backup, wan_3g, "%hhu", (wan_3g+wan_3g_Idx)->backup, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("backup_timer", backup_timer, wan_3g, "%hu", (wan_3g+wan_3g_Idx)->backup_timer, 0);   //WORD_T
/* CHAIN WAN_3G_TBL END*/
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
/* CHAIN DHCPV6S_NAME_SERVER_TBL*/
static int name_server_Idx=0;
MIB_DHCPV6S_NAME_SERVER_T _name_server[MAX_DHCPV6_CHAIN_ENTRY];
MIB_DHCPV6S_NAME_SERVER_Tp name_server=_name_server;
XML_DIR_ARRAY("DHCPV6S_NAME_SERVER_TBL", name_server, root, name_server_Idx, MAX_DHCPV6_CHAIN_ENTRY, MIB_DHCPV6S_NAME_SERVER_TBL);
XML_ENTRY_STRING2("v6_IPAddr", nameServer, name_server, (name_server+name_server_Idx)->nameServer, 0);   //STRING_T
/* CHAIN DHCPV6S_NAME_SERVER_TBL END*/

/* CHAIN DHCPV6S_DOMAIN_SEARCH_TBL*/
static int domain_search_Idx=0;
MIB_DHCPV6S_DOMAIN_SEARCH_T _domain_search[MAX_DHCPV6_CHAIN_ENTRY];
MIB_DHCPV6S_DOMAIN_SEARCH_Tp domain_search=_domain_search;
XML_DIR_ARRAY("DHCPV6S_DOMAIN_SEARCH_TBL", domain_search, root, domain_search_Idx, MAX_DHCPV6_CHAIN_ENTRY, MIB_DHCPV6S_DOMAIN_SEARCH_TBL);
XML_ENTRY_STRING2("domain", domain, name_server, (domain_search+domain_search_Idx)->domain, 0);   //STRING_T
/* CHAIN DHCPV6S_DOMAIN_SEARCH_TBL END*/
#endif
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
/* CHAIN PPTP_TBL*/
static int pptp_Idx=0;
MIB_PPTP_T _pptp[MAX_PPTP_NUM];
MIB_PPTP_Tp pptp=_pptp;
XML_DIR_ARRAY("PPTP_TBL", pptp, root, pptp_Idx, MAX_PPTP_NUM, MIB_PPTP_TBL);
XML_ENTRY_PRIMITIVE2("index", idx, pptp, "%u", (pptp+pptp_Idx)->idx, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, pptp, "%u", (pptp+pptp_Idx)->ifIndex, 0);   //INTEGER_T
#ifdef CONFIG_USER_PPTPD_PPTPD
XML_ENTRY_STRING2("name", name, pptp, (pptp+pptp_Idx)->name, 0);	 //STRING_T
#endif
XML_ENTRY_STRING2("server", server, pptp, (pptp+pptp_Idx)->server, 0);	 //STRING_T
XML_ENTRY_STRING2("username", username, pptp, (pptp+pptp_Idx)->username, 0);	 //STRING_T
XML_ENTRY_STRING2("password", password, pptp, (pptp+pptp_Idx)->password, 0);	 //STRING_T
XML_ENTRY_PRIMITIVE2("authtype", authtype, pptp, "%hhu", (pptp+pptp_Idx)->authtype, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("enctype", enctype, pptp, "%hhu", (pptp+pptp_Idx)->enctype, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("dgw", dgw, pptp, "%hhu", (pptp+pptp_Idx)->dgw, 0);   //BYTE_T
#ifdef CONFIG_IPV6_VPN
XML_ENTRY_PRIMITIVE2("IpProtocol", IpProtocol, pptp, "%hhu", (pptp+pptp_Idx)->IpProtocol, 0);   //BYTE_T
#endif
#ifdef CONFIG_RTK_RG_INIT
XML_ENTRY_PRIMITIVE2("CallID", callid, pptp, "%hu", (pptp+pptp_Idx)->callid, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Gateway_CallID", peer_callid, pptp, "%hu", (pptp+pptp_Idx)->peer_callid, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("RGWANIndex", rg_wan_idx, pptp, "%d", (pptp+pptp_Idx)->rg_wan_idx, 0);   //INTEGER_T
#endif
/* CHAIN PPTP_TBL END*/
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
/* CHAIN VPN_SERVER_TBL*/
static int vpn_server_Idx=0;
MIB_VPND_T _vpn_server[MAX_VPND_NUM];
MIB_VPND_Tp vpn_server=_vpn_server;
XML_DIR_ARRAY("VPN_SERVER_TBL", vpn_server, root, vpn_server_Idx, MAX_VPND_NUM, MIB_VPN_SERVER_TBL);
XML_ENTRY_PRIMITIVE2("type", type, vpn_server, "%hhu", (vpn_server+vpn_server_Idx)->type, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("authtype", authtype, vpn_server, "%hhu", (vpn_server+vpn_server_Idx)->authtype, 0);   //BYTE_T
XML_ENTRY_PRIMITIVE2("enctype", enctype, vpn_server, "%hhu", (vpn_server+vpn_server_Idx)->enctype, 0);   //BYTE_T
#ifdef CONFIG_USER_L2TPD_LNS
XML_ENTRY_PRIMITIVE2("tunnel_auth", tunnel_auth, vpn_server, "%hhu", (vpn_server+vpn_server_Idx)->tunnel_auth, 0);   //BYTE_T
XML_ENTRY_STRING2("tunnel_key", tunnel_key, vpn_server, (vpn_server+vpn_server_Idx)->tunnel_key, 0);	 //STRING_T
#endif
XML_ENTRY_PRIMITIVE2("peeraddr", peeraddr, vpn_server, "%u", (vpn_server+vpn_server_Idx)->peeraddr, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("localaddr", localaddr, vpn_server, "%u", (vpn_server+vpn_server_Idx)->localaddr, 0);   //INTEGER_T
/* CHAIN VPN_SERVER_TBL END*/

/* CHAIN VPN_ACCOUNT_TBL*/
static int vpn_account_Idx=0;
MIB_VPN_ACCOUNT_T _vpn_account[MAX_VPN_ACCOUNT_NUM];
MIB_VPN_ACCOUNT_Tp vpn_account=_vpn_account;
XML_DIR_ARRAY("VPN_ACCOUNT_TBL", vpn_account, root, vpn_account_Idx, MAX_VPN_ACCOUNT_NUM, MIB_VPN_ACCOUNT_TBL);
XML_ENTRY_PRIMITIVE2("index", idx, vpn_account, "%u", (vpn_account+vpn_account_Idx)->idx, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("type", type, vpn_account, "%hhu", (vpn_account+vpn_account_Idx)->type, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("enable", enable, vpn_account, "%hhu", (vpn_account+vpn_account_Idx)->enable, 0);	//BYTE_T
XML_ENTRY_STRING2("name", name, vpn_account, (vpn_account+vpn_account_Idx)->name, 0);	 //STRING_T
XML_ENTRY_STRING2("username", username, vpn_account, (vpn_account+vpn_account_Idx)->username, 0);	 //STRING_T
XML_ENTRY_STRING2("password", password, vpn_account, (vpn_account+vpn_account_Idx)->password, 0);	 //STRING_T
/* CHAIN VPN_ACCOUNT_TBL END*/
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
static int l2tp_Idx=0;
MIB_L2TP_T _l2tp[MAX_L2TP_NUM];
MIB_L2TP_Tp l2tp=_l2tp;
XML_DIR_ARRAY("L2TP_TBL", l2tp, root, l2tp_Idx, MAX_L2TP_NUM, MIB_L2TP_TBL);
XML_ENTRY_PRIMITIVE2("index", idx, l2tp, "%u", (l2tp+l2tp_Idx)->idx, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, l2tp, "%u", (l2tp+l2tp_Idx)->ifIndex, 0);	//INTEGER_T
#ifdef CONFIG_USER_L2TPD_LNS
XML_ENTRY_STRING2("name", name, l2tp, (l2tp+l2tp_Idx)->name, 0);	 //STRING_T
#endif
XML_ENTRY_STRING2("server", server, l2tp, (l2tp+l2tp_Idx)->server, 0);	 //STRING_T
XML_ENTRY_PRIMITIVE2("tunnel_auth", tunnel_auth, l2tp, "%hhu", (l2tp+l2tp_Idx)->tunnel_auth, 0);	//BYTE_T
XML_ENTRY_STRING2("secret", secret, l2tp, (l2tp+l2tp_Idx)->secret, 0);	 //STRING_T
XML_ENTRY_PRIMITIVE2("authtype", authtype, l2tp, "%hhu", (l2tp+l2tp_Idx)->authtype, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("enctype", enctype, l2tp, "%hhu", (l2tp+l2tp_Idx)->enctype, 0);	//BYTE_T
XML_ENTRY_STRING2("username", username, l2tp, (l2tp+l2tp_Idx)->username, 0);	 //STRING_T
XML_ENTRY_STRING2("password", password, l2tp, (l2tp+l2tp_Idx)->password, 0);	 //STRING_T
XML_ENTRY_PRIMITIVE2("conntype", conntype, l2tp, "%hhu", (l2tp+l2tp_Idx)->conntype, 0);	//BYTE_T
XML_ENTRY_PRIMITIVE2("idletime", idletime, l2tp, "%u", (l2tp+l2tp_Idx)->idletime, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("mtu", mtu, l2tp, "%u", (l2tp+l2tp_Idx)->mtu, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("dgw", dgw, l2tp, "%hhu", (l2tp+l2tp_Idx)->dgw, 0); //BYTE_T
#ifdef CONFIG_IPV6_VPN
XML_ENTRY_PRIMITIVE2("IpProtocol", IpProtocol, l2tp, "%hhu", (l2tp+l2tp_Idx)->IpProtocol, 0); //BYTE_T
#endif
#ifdef CONFIG_RTK_RG_INIT
XML_ENTRY_PRIMITIVE2("RGWANIndex", rg_wan_idx, l2tp, "%d", (l2tp+l2tp_Idx)->rg_wan_idx, 0);   //INTEGER_T
XML_ENTRY_PRIMITIVE2("Outer_Port", outer_port, l2tp, "%hu", (l2tp+l2tp_Idx)->outer_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Tunnel_ID", tunnel_id, l2tp, "%hu", (l2tp+l2tp_Idx)->tunnel_id, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Session_ID", session_id, l2tp, "%hu", (l2tp+l2tp_Idx)->session_id, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Gateway_Outer_Port", gateway_outer_port, l2tp, "%hu", (l2tp+l2tp_Idx)->gateway_outer_port, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Gateway_Tunnel_ID", gateway_tunnel_id, l2tp, "%hu", (l2tp+l2tp_Idx)->gateway_tunnel_id, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Gateway_Session_ID", gateway_session_id, l2tp, "%hu", (l2tp+l2tp_Idx)->gateway_session_id, 0);   //WORD_T
#endif
#endif //end of CONFIG_USER_L2TPD_L2TPD

#ifdef CONFIG_XFRM
static int ipsec_Idx=0;
MIB_IPSEC_T _ipsec[MAX_IPSEC_NUM];
MIB_IPSEC_Tp ipsec=_ipsec;
XML_DIR_ARRAY("IPSEC_TBL", ipsec, root, ipsec_Idx, MAX_IPSEC_NUM, MIB_IPSEC_TBL);
XML_ENTRY_PRIMITIVE2("enable", enable, ipsec, "%hhu", (ipsec+ipsec_Idx)->enable, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("state", state, ipsec, "%hhu", (ipsec+ipsec_Idx)->state, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("negotiationType", negotiationType, ipsec, "%hhu", (ipsec+ipsec_Idx)->negotiationType, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("transportMode", transportMode, ipsec, "%hhu", (ipsec+ipsec_Idx)->transportMode, 0); //BYTE_T
XML_ENTRY_ADDR("remoteTunnel", remoteTunnel, ipsec, (ipsec+ipsec_Idx)->remoteTunnel, 0);   //IA_T
XML_ENTRY_ADDR("remoteIP", remoteIP, ipsec, (ipsec+ipsec_Idx)->remoteIP, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("remoteMask", remoteMask, ipsec, "%hhu", (ipsec+ipsec_Idx)->remoteMask, 0); //BYTE_T
XML_ENTRY_ADDR("localTunnel", localTunnel, ipsec, (ipsec+ipsec_Idx)->localTunnel, 0);   //IA_T
XML_ENTRY_ADDR("localIP", localIP, ipsec, (ipsec+ipsec_Idx)->localIP, 0);   //IA_T
XML_ENTRY_PRIMITIVE2("localMask", localMask, ipsec, "%hhu", (ipsec+ipsec_Idx)->localMask, 0); //BYTE_T

XML_ENTRY_PRIMITIVE2("encapMode", encapMode, ipsec, "%hhu", (ipsec+ipsec_Idx)->encapMode, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("filterProtocol", filterProtocol, ipsec, "%hhu", (ipsec+ipsec_Idx)->filterProtocol, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("filterPort", filterPort, ipsec, "%u", (ipsec+ipsec_Idx)->filterPort, 0); //INTEGER_T

XML_ENTRY_PRIMITIVE2("espEncrypt", espEncrypt, ipsec, "%hhu", (ipsec+ipsec_Idx)->espEncrypt, 0); //BYTE_T
XML_ENTRY_STRING2("espEncryptKey", espEncryptKey, ipsec, (ipsec+ipsec_Idx)->espEncryptKey, 0);	 //STRING_T
XML_ENTRY_PRIMITIVE2("espAuth", espAuth, ipsec, "%hhu", (ipsec+ipsec_Idx)->espAuth, 0); //BYTE_T
XML_ENTRY_STRING2("espAuthKey", espAuthKey, ipsec, (ipsec+ipsec_Idx)->espAuthKey, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("ahAuth", ahAuth, ipsec, "%hhu", (ipsec+ipsec_Idx)->ahAuth, 0); //BYTE_T
XML_ENTRY_STRING2("ahAuthKey", ahAuthKey, ipsec, (ipsec+ipsec_Idx)->ahAuthKey, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("espINSPI", espINSPI, ipsec, "%u", (ipsec+ipsec_Idx)->espINSPI, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("espOUTSPI", espOUTSPI, ipsec, "%u", (ipsec+ipsec_Idx)->espOUTSPI, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("ahINSPI", ahINSPI, ipsec, "%u", (ipsec+ipsec_Idx)->ahINSPI, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("ahOUTSPI", ahOUTSPI, ipsec, "%u", (ipsec+ipsec_Idx)->ahOUTSPI, 0); //INTEGER_T

XML_ENTRY_PRIMITIVE2("ikeMode", ikeMode, ipsec, "%hhu", (ipsec+ipsec_Idx)->ikeMode, 0); //BYTE_T
XML_ENTRY_STRING2("psk", psk, ipsec, (ipsec+ipsec_Idx)->psk, 0);  //STRING_T
XML_ENTRY_BYTE_ARRAY("ikeProposal", ikeProposal, ipsec, (ipsec+ipsec_Idx)->ikeProposal, 0); //BYTE_ARRAY_T
XML_ENTRY_BYTE_ARRAY("saProposal", saProposal, ipsec, (ipsec+ipsec_Idx)->saProposal, 0); //BYTE_ARRAY_T
XML_ENTRY_PRIMITIVE2("ikeAliveTime", ikeAliveTime, ipsec, "%u", (ipsec+ipsec_Idx)->ikeAliveTime, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("saAliveTime", saAliveTime, ipsec, "%u", (ipsec+ipsec_Idx)->saAliveTime, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("saAliveByte", saAliveByte, ipsec, "%u", (ipsec+ipsec_Idx)->saAliveByte, 0); //INTEGER_T
#endif//end of CONFIG_XFRM

#ifdef CONFIG_NET_IPIP
static int ipip_Idx=0;
MIB_IPIP_T _ipip[MAX_IPIP_NUM];
MIB_IPIP_Tp ipip=_ipip;
XML_DIR_ARRAY("IPIP_TBL", ipip, root, ipip_Idx, MAX_IPIP_NUM, MIB_IPIP_TBL);
XML_ENTRY_PRIMITIVE2("index", idx, ipip, "%u", (ipip+ipip_Idx)->idx, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, ipip, "%u", (ipip+ipip_Idx)->ifIndex, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("remote", daddr, ipip, "%u", (ipip+ipip_Idx)->daddr, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("local", saddr, ipip, "%u", (ipip+ipip_Idx)->saddr, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("dgw", dgw, ipip, "%hhu", (ipip+ipip_Idx)->dgw, 0); //BYTE_T
#endif//end of CONFIG_NET_IPIP

#ifdef CONFIG_IPV6
static int v6_ippf_Idx=0;
MIB_CE_V6_IP_PORT_FILTER_T _v6_ippf[MAX_FILTER_NUM];
MIB_CE_V6_IP_PORT_FILTER_Tp v6_ippf=_v6_ippf;
XML_DIR_ARRAY("V6_IP_PORT_FILTER_TBL", v6_ippf, root, v6_ippf_Idx, MAX_FILTER_NUM, MIB_V6_IP_PORT_FILTER_TBL);
XML_ENTRY_PRIMITIVE2("FilterMode", action, v6_ippf, "%hhu", (v6_ippf+v6_ippf_Idx)->action, 0); //BYTE_T
#ifdef CONFIG_IPV6_OLD_FILTER
XML_ENTRY_ADDR6("Sip6Start", sip6Start, v6_ippf, (v6_ippf+v6_ippf_Idx)->sip6Start, 0);   //IA6_T
XML_ENTRY_ADDR6("Sip6End", sip6End, v6_ippf, (v6_ippf+v6_ippf_Idx)->sip6End, 0);   //IA6_T
XML_ENTRY_ADDR6("Dip6Start", dip6Start, v6_ippf, (v6_ippf+v6_ippf_Idx)->dip6Start, 0);   //IA6_T
XML_ENTRY_ADDR6("Dip6End", dip6End, v6_ippf, (v6_ippf+v6_ippf_Idx)->dip6End, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("Sip6PrefixLen", sip6PrefixLen, v6_ippf, "%hhu", (v6_ippf+v6_ippf_Idx)->sip6PrefixLen, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("Dip6PrefixLen", dip6PrefixLen, v6_ippf, "%hhu", (v6_ippf+v6_ippf_Idx)->dip6PrefixLen, 0); //BYTE_T
#else
XML_ENTRY_ADDR6("SIfId6Start", sIfId6Start, v6_ippf, (v6_ippf+v6_ippf_Idx)->sIfId6Start, 0);   //IA6_T
XML_ENTRY_ADDR6("SIfId6End", sIfId6End, v6_ippf, (v6_ippf+v6_ippf_Idx)->sIfId6End, 0);   //IA6_T
XML_ENTRY_ADDR6("DIfId6Start", dIfId6Start, v6_ippf, (v6_ippf+v6_ippf_Idx)->dIfId6Start, 0);   //IA6_T
XML_ENTRY_ADDR6("DIfId6End", dIfId6End, v6_ippf, (v6_ippf+v6_ippf_Idx)->dIfId6End, 0);   //IA6_T
#endif
XML_ENTRY_PRIMITIVE2("SrcPortStart", srcPortFrom, v6_ippf, "%hu", (v6_ippf+v6_ippf_Idx)->srcPortFrom, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("DstPortStart", dstPortFrom, v6_ippf, "%hu", (v6_ippf+v6_ippf_Idx)->dstPortFrom, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("SrcPortEnd", srcPortTo, v6_ippf, "%hu", (v6_ippf+v6_ippf_Idx)->srcPortTo, 0);	//WORD_T
XML_ENTRY_PRIMITIVE2("DstPortEnd", dstPortTo, v6_ippf, "%hu", (v6_ippf+v6_ippf_Idx)->dstPortTo, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("Direction", dir, v6_ippf, "%hhu", (v6_ippf+v6_ippf_Idx)->dir, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("protocol", protoType, v6_ippf, "%hhu", (v6_ippf+v6_ippf_Idx)->protoType, 0); //BYTE_T
#endif

#ifdef WLAN_WDS
static int wl_wds_Idx=0;
WDS_T _wl_wds[MAX_WDS_NUM];
WDS_Tp wl_wds=_wl_wds;
XML_DIR_ARRAY("WLAN_WDS_TBL", wl_wds, root, wl_wds_Idx, MAX_WDS_NUM, MIB_WDS_TBL);
XML_ENTRY_ADDR6("MacAddr", macAddr, wl_wds, (wl_wds+wl_wds_Idx)->macAddr, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("fixedTxRate", fixedTxRate, wl_wds, "%u", (wl_wds+wl_wds_Idx)->fixedTxRate, 0); //INTEGER_T
XML_ENTRY_STRING2("Comment", comment, wl_wds, (wl_wds+wl_wds_Idx)->comment, 0);	//STRING_T

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
static int wl1_wds_Idx=0;
WDS_T _wl1_wds[MAX_WDS_NUM];
WDS_Tp wl1_wds=_wl1_wds;
XML_DIR_ARRAY("WLAN1_WDS_TBL", wl1_wds, root, wl1_wds_Idx, MAX_WDS_NUM, MIB_WLAN1_WDS_TBL);
XML_ENTRY_ADDR6("MacAddr", macAddr, wl1_wds, (wl1_wds+wl1_wds_Idx)->macAddr, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("fixedTxRate", fixedTxRate, wl1_wds, "%u", (wl1_wds+wl1_wds_Idx)->fixedTxRate, 0); //INTEGER_T
XML_ENTRY_STRING2("Comment", comment, wl1_wds, (wl1_wds+wl1_wds_Idx)->comment, 0); //STRING_T
#endif
#endif

#ifdef CONFIG_IPV6
static int ip6_route_Idx=0;
MIB_CE_IPV6_ROUTE_T _ip6_route[8];
MIB_CE_IPV6_ROUTE_Tp ip6_route=_ip6_route;
XML_DIR_ARRAY("IPV6_ROUTE_TBL", ip6_route, root, ip6_route_Idx, 8, MIB_IPV6_ROUTE_TBL);
XML_ENTRY_ADDR6("Dstination", Dstination, ip6_route, (ip6_route+ip6_route_Idx)->Dstination, 0);   //IA6_T
XML_ENTRY_ADDR6("NextHop", NextHop, ip6_route, (ip6_route+ip6_route_Idx)->NextHop, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("Enable", Enable, ip6_route, "%hhu", (ip6_route+ip6_route_Idx)->Enable, 0); //BYTE_T
XML_ENTRY_PRIMITIVE2("Type", Type, ip6_route, "%hhu", (ip6_route+ip6_route_Idx)->Type, 0); //BYTE_T
XML_ENTRY_ADDR6("SourceIP", SourceIP, ip6_route, (ip6_route+ip6_route_Idx)->SourceIP, 0);   //IA6_T
XML_ENTRY_PRIMITIVE2("DstInf", DstIfIndex, ip6_route, "%u", (ip6_route+ip6_route_Idx)->DstIfIndex, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("SrcInf", SrcIfIndex, ip6_route, "%u", (ip6_route+ip6_route_Idx)->SrcIfIndex, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("FWMetric", FWMetric, ip6_route, "%d", (ip6_route+ip6_route_Idx)->FWMetric, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("MTU", Mtu, ip6_route, "%u", (ip6_route+ip6_route_Idx)->Mtu, 0);	 //DWORD_T
XML_ENTRY_PRIMITIVE2("InstanceNum", InstanceNum, ip6_route, "%u", (ip6_route+ip6_route_Idx)->InstanceNum, 0);   //DWORD_T
#endif

#ifdef CONFIG_EPON_FEATURE
static int epon_llid_Idx=0;
MIB_CE_MIB_EPON_LLID_T _epon_llid[8];
MIB_CE_MIB_EPON_LLID_Tp epon_llid=_epon_llid;
XML_DIR_ARRAY("EPON_LLID_TBL", epon_llid, root, epon_llid_Idx, 8, MIB_EPON_LLID_TBL);
XML_ENTRY_ADDR6("macAddr", macAddr, epon_llid, (epon_llid+epon_llid_Idx)->macAddr, 0);   //IA6_T
#endif

#ifdef CONFIG_DSL_VTUO
static int vtuo_dpbo_Idx=0;
MIB_CE_VTUO_DPBO_T _vtuo_dpbo[16];
MIB_CE_VTUO_DPBO_Tp vtuo_dpbo=_vtuo_dpbo;
XML_DIR_ARRAY("VTUO_DPBO_TBL", vtuo_dpbo, root, vtuo_dpbo_Idx, 8, MIB_VTUO_DPBO_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_dpbo, "%hu", (vtuo_dpbo+vtuo_dpbo_Idx)->ToneId, 0);   //WORD_T
XML_ENTRY_PRIMITIVE2("PsdLevel", PsdLevel, vtuo_dpbo, "%hu", (vtuo_dpbo+vtuo_dpbo_Idx)->PsdLevel, 0);   //WORD_T

static int vtuo_psd_ds_Idx=0;
MIB_CE_VTUO_PSD_T _vtuo_psd_ds[32];
MIB_CE_VTUO_PSD_Tp vtuo_psd_ds=_vtuo_psd_ds;
XML_DIR_ARRAY("VTUO_PSD_DS_TBL", vtuo_psd_ds, root, vtuo_psd_ds_Idx, 32, MIB_VTUO_PSD_DS_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_psd_ds, "%hu", (vtuo_psd_ds+vtuo_psd_ds_Idx)->ToneId, 0);  //WORD_T
XML_ENTRY_PRIMITIVE2("PsdLevel", PsdLevel, vtuo_psd_ds, "%hu", (vtuo_psd_ds+vtuo_psd_ds_Idx)->PsdLevel, 0);  //WORD_T

static int vtuo_psd_us_Idx=0;
MIB_CE_VTUO_PSD_T _vtuo_psd_us[16];
MIB_CE_VTUO_PSD_Tp vtuo_psd_us=_vtuo_psd_us;
XML_DIR_ARRAY("VTUO_PSD_US_TBL", vtuo_psd_us, root, vtuo_psd_us_Idx, 16, MIB_VTUO_PSD_US_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_psd_us, "%hu", (vtuo_psd_us+vtuo_psd_us_Idx)->ToneId, 0);  //WORD_T
XML_ENTRY_PRIMITIVE2("PsdLevel", PsdLevel, vtuo_psd_us, "%hu", (vtuo_psd_us+vtuo_psd_us_Idx)->PsdLevel, 0);  //WORD_T
/*
static int vtuo_vn_ds_Idx=0;
MIB_CE_VTUO_VN_T _vtuo_vn_ds[32];
MIB_CE_VTUO_VN_Tp vtuo_vn_ds=_vtuo_vn_ds;
XML_DIR_ARRAY("VTUO_VN_DS_TBL", vtuo_vn_ds, root, vtuo_vn_ds_Idx, 32, MIB_VTUO_VN_DS_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_vn_ds, "%hu", (vtuo_vn_ds+vtuo_vn_ds_Idx)->ToneId, 0);  //WORD_T
XML_ENTRY_PRIMITIVE2("NoiseLevel", NoiseLevel, vtuo_vn_ds, "%hu", (vtuo_vn_ds+vtuo_vn_ds_Idx)->NoiseLevel, 0);  //WORD_T

static int vtuo_vn_us_Idx=0;
MIB_CE_VTUO_VN_T _vtuo_vn_us[16];
MIB_CE_VTUO_VN_Tp vtuo_vn_us=_vtuo_vn_us;
XML_DIR_ARRAY("VTUO_VN_US_TBL", vtuo_vn_us, root, vtuo_vn_us_Idx, 16, MIB_VTUO_VN_US_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_vn_us, "%hu", (vtuo_vn_us+vtuo_vn_us_Idx)->ToneId, 0);  //WORD_T
XML_ENTRY_PRIMITIVE2("NoiseLevel", NoiseLevel, vtuo_vn_us, "%hu", (vtuo_vn_us+vtuo_vn_us_Idx)->NoiseLevel, 0);  //WORD_T

static int vtuo_rfi_Idx=0;
MIB_CE_VTUO_RFI_T _vtuo_rfi[16];
MIB_CE_VTUO_RFI_Tp vtuo_rfi=_vtuo_rfi;
XML_DIR_ARRAY("VTUO_RFI_TBL", vtuo_rfi, root, vtuo_rfi_Idx, 16, MIB_VTUO_RFI_TBL);
XML_ENTRY_PRIMITIVE2("ToneId", ToneId, vtuo_rfi, "%hu", (vtuo_rfi+vtuo_rfi_Idx)->ToneId, 0);  //WORD_T
XML_ENTRY_PRIMITIVE2("ToneIdEnd", ToneIdEnd, vtuo_rfi, "%hu", (vtuo_rfi+vtuo_rfi_Idx)->ToneIdEnd, 0);  //WORD_T
*/
#endif /*CONFIG_DSL_VTUO*/

#ifdef CONFIG_USER_DOT1AG_UTILS
static int dot1ag_Idx=0;
MIB_CE_DOT1AG_T _dot1ag[8];
MIB_CE_DOT1AG_Tp dot1ag=_dot1ag;
XML_DIR_ARRAY("DOT1AG_TBL", dot1ag, root, dot1ag_Idx, 8, MIB_DOT1AG_TBL);
XML_ENTRY_PRIMITIVE2("ifIndex", ifIndex, dot1ag, "%u", (dot1ag+dot1ag_Idx)->ifIndex, 0);  //INTEGER_T
XML_ENTRY_STRING2("md_name", md_name, dot1ag, (dot1ag+dot1ag_Idx)->md_name, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("md_level", md_level, dot1ag, "%hhu", (dot1ag+dot1ag_Idx)->md_level, 0);  //BYTE_T
XML_ENTRY_PRIMITIVE2("mep_id", mep_id, dot1ag, "%hhu", (dot1ag+dot1ag_Idx)->mep_id, 0);  //WORD_T
XML_ENTRY_STRING2("ma_name", ma_name, dot1ag, (dot1ag+dot1ag_Idx)->ma_name, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("ccm_enable", ccm_enable, dot1ag, "%hhu", (dot1ag+dot1ag_Idx)->ccm_enable, 0);  //BYTE_T
XML_ENTRY_PRIMITIVE2("ccm_interval", ccm_interval, dot1ag, "%u", (dot1ag+dot1ag_Idx)->ccm_interval, 0);  //DWORD_T
#endif

#ifdef OSGI_SUPPORT
static int osgi_bundle_Idx=0;
MIB_CE_OSGI_BUNDLE_T _osgi_bundle[32];
MIB_CE_OSGI_BUNDLE_Tp osgi_bundle=_osgi_bundle;
XML_DIR_ARRAY("OSGI_BUNDLE_TBL", osgi_bundle, root, osgi_bundle_Idx, 32, MIB_OSGI_BUNDLE_TBL);
XML_ENTRY_PRIMITIVE2("bundle_id", bundle_id, osgi_bundle, "%u", (osgi_bundle+osgi_bundle_Idx)->bundle_id, 0); //INTEGER_T
XML_ENTRY_STRING2("bundle_name", bundle_name, osgi_bundle, (osgi_bundle+osgi_bundle_Idx)->bundle_name, 0);  //STRING_T
XML_ENTRY_STRING2("bundle_file", bundle_file, osgi_bundle, (osgi_bundle+osgi_bundle_Idx)->bundle_file, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("bundle_action", bundle_action, osgi_bundle, "%u", (osgi_bundle+osgi_bundle_Idx)->bundle_action, 0); //INTEGER_T
#endif//end of CONFIG_NET_IPIP

#ifdef CONFIG_CWMP_TRANSFER_QUEUE
static int cwmp_transfer_queue_Idx = 0;
MIB_CE_CWMP_TRANSFER_T _cwmp_transfer_queue[16];
MIB_CE_CWMP_TRANSFER_Tp cwmp_transfer_queue = _cwmp_transfer_queue;
XML_DIR_ARRAY("CWMP_TRANSFER_QUEUE", cwmp_transfer_queue, root, cwmp_transfer_queue_Idx, 16, CWMP_TRANSFER_QUEUE_TBL);
XML_ENTRY_PRIMITIVE2("STATE", State, cwmp_transfer_queue, "%d", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->State, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("IS_DOWNLOAD", IsDownload, cwmp_transfer_queue, "%hhu", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->IsDownload, 0); //BTYE_T
XML_ENTRY_STRING2("COMMANDKEY", CommandKey, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->CommandKey, 0);  //STRING_T
XML_ENTRY_STRING2("FILETYPE", FileType, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->FileType, 0);  //STRING_T
XML_ENTRY_STRING2("URL", URL, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->URL, 0);  //STRING_T
XML_ENTRY_STRING2("USERNAME", Username, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->Username, 0);  //STRING_T
XML_ENTRY_STRING2("PASSWORD", Password, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->Password, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("FILESIZE", FileSize, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->FileSize, 0); //DWORD_T
XML_ENTRY_STRING2("TARGETFILENAME", TargetFileName, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->TargetFileName, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("DELAYSECONDS", DelaySeconds, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->DelaySeconds, 0); //DWORD_T
XML_ENTRY_STRING2("SUCCESSURL", SuccessURL, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->SuccessURL, 0);  //STRING_T
XML_ENTRY_STRING2("FAILUREURL", FailureURL, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->FailureURL, 0);  //STRING_T

/* For ScheduleDownload RPC */
XML_ENTRY_PRIMITIVE2("TIME_WIN_START1", time_win_start1, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_start[0], 0); //DWORD_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_END1", time_win_end1, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_end[0], 0); //DWORD_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_MODE1", time_win_mode1, cwmp_transfer_queue, "%hhu", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_mode[0], 0); //BTYE_T
XML_ENTRY_STRING2("TIME_WIN_USER_MSG1", time_win_user_msg1, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_user_msg[0], 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_RETRIES1", time_win_retires1, cwmp_transfer_queue, "%d", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_retires[0], 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_START2", time_win_start2, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_start[1], 0); //DWORD_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_END2", time_win_end2, cwmp_transfer_queue, "%u", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_end[1], 0); //DWORD_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_MODE2", time_win_mode2, cwmp_transfer_queue, "%hhu", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_mode[1], 0); //BTYE_T
XML_ENTRY_STRING2("TIME_WIN_USER_MSG2", time_win_user_msg2, cwmp_transfer_queue, (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_user_msg[1], 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("TIME_WIN_RETRIES2", time_win_retires2, cwmp_transfer_queue, "%d", (cwmp_transfer_queue+cwmp_transfer_queue_Idx)->time_win_retires[1], 0); //INTEGER_T
#endif //end of CONFIG_CWMP_TRANSFER_QUEUE

#ifdef _SUPPORT_L2BRIDGING_PROFILE_
static int l2bridging_bridge_group_Idx = 0;
MIB_L2BRIDGE_GROUP_T _l2bridging_bridge_group[5];
MIB_L2BRIDGE_GROUP_Tp l2bridging_bridge_group = _l2bridging_bridge_group;
XML_DIR_ARRAY("L2BRIDGING_BRIDGE_GROUP_TBL", l2bridging_bridge_group, root, l2bridging_bridge_group_Idx, 5, MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
XML_ENTRY_PRIMITIVE2("enable", enable, l2bridging_bridge_group, "%hhu", (l2bridging_bridge_group+l2bridging_bridge_group_Idx)->enable, 0);  //BYTE_T
XML_ENTRY_PRIMITIVE2("groupnum", groupnum, l2bridging_bridge_group, "%hhu", (l2bridging_bridge_group+l2bridging_bridge_group_Idx)->groupnum, 0);  //BYTE_T
XML_ENTRY_STRING2("name", name, l2bridging_bridge_group, (l2bridging_bridge_group+l2bridging_bridge_group_Idx)->name, 0);  //STRING_T
XML_ENTRY_PRIMITIVE2("vlanid", vlanid, l2bridging_bridge_group, "%u", (l2bridging_bridge_group+l2bridging_bridge_group_Idx)->vlanid, 0); //INTEGER_T
XML_ENTRY_PRIMITIVE2("instnum", instnum, l2bridging_bridge_group, "%u", (l2bridging_bridge_group+l2bridging_bridge_group_Idx)->instnum, 0); //INTEGER_T
#endif //end of _SUPPORT_L2BRIDGING_PROFILE_

typedef struct _chainInfo {
	unsigned int id;	//id
	void *pos;	//position
	int len;
} chainInfo_t;

//#define CM(id, pos) {id, pos, sizeof(pos)}
chainInfo_t chainInfo[]={
	{MIB_ATM_VC_TBL, _atmvc, sizeof(MIB_CE_ATM_VC_T)},
#ifdef IP_PORT_FILTER
    {MIB_IP_PORT_FILTER_TBL, _ipportfilter, sizeof(MIB_CE_IP_PORT_FILTER_T)},
#endif
#ifdef MAC_FILTER
    {MIB_MAC_FILTER_TBL, _macfilter, sizeof(MIB_CE_MAC_FILTER_T)},
#endif
#ifdef PORT_FORWARD_GENERAL
	{MIB_PORT_FW_TBL, _portfw, sizeof(MIB_CE_PORT_FW_T)},
#endif
#ifdef PORT_FORWARD_ADVANCE
	{MIB_PFW_ADVANCE_TBL, _ipfw_advance, sizeof(MIB_CE_PORT_FW_ADVANCE_T)},
#endif
#ifdef VIRTUAL_SERVER_SUPPORT
	{MIB_VIRTUAL_SVR_TBL, _mmvtlsvr, sizeof(MIB_CE_VTL_SVR_T)},
#endif
#ifdef ROUTING
    {MIB_IP_ROUTE_TBL, _iproute, sizeof(MIB_CE_IP_ROUTE_T)},
#endif
#ifdef IP_ACL
    {MIB_ACL_IP_TBL, _aclip, sizeof(MIB_CE_ACL_IP_T)},
#endif
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
    {MIB_DHCP_SERVER_OPTION_TBL, _dhcpopt_server, sizeof(MIB_CE_DHCP_OPTION_T)},
    {MIB_DHCP_CLIENT_OPTION_TBL, _dhcpopt_client, sizeof(MIB_CE_DHCP_OPTION_T)},
#endif
#ifdef MAC_ACL
    {MIB_ACL_MAC_TBL, _aclmac, sizeof(MIB_CE_ACL_MAC_T)},
#endif
#ifdef NAT_CONN_LIMIT
    {MIB_CONN_LIMIT_TBL, _connlimit, sizeof(MIB_CE_CONN_LIMIT_T)},
#endif
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
    {MIB_WLAN_AC_TBL, _wlac, sizeof(MIB_CE_WLAN_AC_T)},
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	{MIB_WLAN1_AC_TBL, _w1lac, sizeof(MIB_CE_WLAN_AC_T)},
#endif
#endif
#ifdef WLAN_11R
	{MIB_WLAN_FTKH_TBL, _wlan_ftkh, sizeof(MIB_CE_WLAN_FTKH_T)},
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	{MIB_WLAN1_FTKH_TBL, _wlan1_ftkh, sizeof(MIB_CE_WLAN_FTKH_T)},
#endif
#endif
#endif
	{MIB_SW_PORT_TBL, _swport, sizeof(MIB_CE_SW_PORT_T)},
#ifdef VLAN_GROUP
	{MIB_VLAN_TBL, _vlan, sizeof(MIB_CE_VLAN_T)},
#endif
#if defined(CONFIG_USER_IP_QOS) || defined(TELEFONICA_DEFAULT_CFG)
    {MIB_IP_QOS_TBL, _ipqos, sizeof(MIB_CE_IP_QOS_T)},
#endif
#ifdef CONFIG_USER_IP_QOS
    {MIB_IP_QOS_TC_TBL, _ipqos_tc, sizeof(MIB_CE_IP_TC_T)},
#endif
#ifdef CONFIG_USER_IP_QOS
	{MIB_IP_QOS_QUEUE_TBL, _ipqos_queue, sizeof(MIB_CE_IP_QOS_QUEUE_T)},
#endif
#ifdef REMOTE_ACCESS_CTL
	{MIB_ACC_TBL, _acc, sizeof(MIB_CE_ACC_T)},
#endif
#ifdef PORT_TRIGGERING
	{MIB_PORT_TRG_TBL, _porttrg, sizeof(MIB_CE_PORT_TRG_T)},
#endif
#ifdef NATIP_FORWARDING
	{MIB_IP_FW_TBL, _ipfw, sizeof(MIB_CE_IP_FW_T)},
#endif
#ifdef URL_BLOCKING_SUPPORT
	{MIB_URL_FQDN_TBL, _urlfqdn, sizeof(MIB_CE_URL_FQDN_T)},
	{MIB_KEYWD_FILTER_TBL, _keywdfilter, sizeof(MIB_CE_KEYWD_FILTER_T)},
#endif
#ifdef URL_ALLOWING_SUPPORT
	{MIB_URL_ALLOW_FQDN_TBL, _urlalwfqdn, sizeof(MIB_CE_URL_ALLOW_FQDN_T)},
#endif
#ifdef _CWMP_MIB_
#ifdef WLAN_SUPPORT
	{CWMP_PSK_TBL, _cwmp_psk, sizeof(CWMP_PSK_T)},
#endif
#endif /*_CWMP_MIB_*/
	{MIB_MAC_BASE_DHCP_TBL, _macBaseDhcp, sizeof(MIB_CE_MAC_BASE_DHCP_T)},
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
	{MIB_AUTO_PVC_SEARCH_TBL, _AutoPvcSearch, sizeof(MIB_AUTO_PVC_SEARCH_T)},
#endif
#ifdef CONFIG_USER_DDNS
	{MIB_DDNS_TBL, _ddns, sizeof(MIB_CE_DDNS_T)},
#endif
	{MIB_PPPOE_SESSION_TBL,  _pppoeSession, sizeof(MIB_CE_PPPOE_SESSION_T)},
#ifdef ACCOUNT_CONFIG
	{MIB_ACCOUNT_CONFIG_TBL, _accountConfig, sizeof(MIB_CE_ACCOUNT_CONFIG_T)},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	{MIB_DOMAIN_BLOCKING_TBL, _domainblk, sizeof(MIB_CE_DOMAIN_BLOCKING_T)},
#endif
	{MIB_RIP_TBL, _rip, sizeof(MIB_CE_RIP_T)},
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	{MIB_OSPF_TBL, _ospf, sizeof(MIB_CE_OSPF_T)},
#endif
#ifdef WLAN_SUPPORT
	{MIB_MBSSIB_TBL, _mbssid, sizeof(MIB_CE_MBSSIB_T)},
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	{MIB_WLAN1_MBSSIB_TBL, _wlan1_mbssid, sizeof(MIB_CE_MBSSIB_T)},
#endif
#endif
#ifdef LAYER7_FILTER_SUPPORT
	{MIB_LAYER7_FILTER_TBL, _layer7, sizeof(LAYER7_FILTER_T)},
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
	{MIB_QOS_SPEED_LIMIT, _ipqos_speedrank, sizeof(MIB_CE_IP_QOS_SPEEDRANK_T)},
#endif
#ifdef VOIP_SUPPORT
	{MIB_VOIP_CFG_TBL, _voipCfgParam, sizeof(voipCfgParam_t)},
#endif /*VOIP_SUPPORT*/
#ifdef PARENTAL_CTRL
	{MIB_PARENTAL_CTRL_TBL, _parentctrl, sizeof(MIB_PARENT_CTRL_T)},
#endif
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	{MIB_DHCPS_SERVING_POOL_TBL, _dhcp_sp, sizeof(DHCPS_SERVING_POOL_T)},
#endif
#ifdef TCP_UDP_CONN_LIMIT
	{MIB_TCP_UDP_CONN_LIMIT_TBL, _tucl, sizeof(MIB_CE_TCP_UDP_CONN_LIMIT_T)},
#endif
#ifdef WEB_REDIRECT_BY_MAC
	{MIB_WEB_REDIR_BY_MAC_TBL, _wrbm, sizeof(MIB_WEB_REDIR_BY_MAC_T)},
#endif
#ifdef MULTI_ADDRESS_MAPPING
	{MULTI_ADDRESS_MAPPING_LIMIT_TBL, _muladdrmap, sizeof(MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T)},
#endif //MULTI_ADDRESS_MAPPING
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	{CWMP_CAPTIVEPORTAL_ALLOWED_LIST, _cpal, sizeof(CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T)},
#endif
#ifdef WLAN_QoS
	{MIB_WLAN_QOS_AP_TBL, _wlan_qos_ap, sizeof(MIB_WLAN_QOS_T)},
	{MIB_WLAN_QOS_STA_TBL, _wlan_qos_sta, sizeof(MIB_WLAN_QOS_T)},
#endif //WLAN_QoS
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
	{MIB_DHCP_RESERVED_IPADDR_TBL, _dria, sizeof(MIB_DHCP_RESERVED_IPADDR_T)},
#endif //SUPPORT_DHCP_RESERVED_IPADDR
#ifdef CONFIG_USER_PPPOMODEM
	{MIB_WAN_3G_TBL, _wan_3g, sizeof(MIB_WAN_3G_T)},
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	{MIB_DHCPV6S_NAME_SERVER_TBL, _name_server, sizeof(MIB_DHCPV6S_NAME_SERVER_T)},
	{MIB_DHCPV6S_DOMAIN_SEARCH_TBL, _domain_search, sizeof(MIB_DHCPV6S_DOMAIN_SEARCH_T)},
#endif
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	{MIB_PPTP_TBL, _pptp, sizeof(MIB_PPTP_T)},
#endif
#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	{MIB_VPN_SERVER_TBL, _vpn_server, sizeof(MIB_VPND_T)},
	{MIB_VPN_ACCOUNT_TBL, _vpn_account, sizeof(MIB_VPN_ACCOUNT_T)},
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
	{MIB_L2TP_TBL, _l2tp, sizeof(MIB_L2TP_T)},
#endif
#ifdef CONFIG_XFRM
	{MIB_IPSEC_TBL, _ipsec, sizeof(MIB_IPSEC_T)},
#endif
#ifdef CONFIG_NET_IPIP
	{MIB_IPIP_TBL, _ipip, sizeof(MIB_IPIP_T)},
#endif
#ifdef CONFIG_IPV6
	{MIB_V6_IP_PORT_FILTER_TBL, _v6_ippf, sizeof(MIB_CE_V6_IP_PORT_FILTER_T)},
#endif
#ifdef WLAN_WDS
	{MIB_WDS_TBL, _wl_wds, sizeof(WDS_T)},
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	{MIB_WLAN1_WDS_TBL, _wl1_wds, sizeof(WDS_T)},
#endif
#endif
#ifdef _SUPPORT_L2BRIDGING_PROFILE_
	{MIB_L2BRIDGING_BRIDGE_GROUP_TBL, _l2bridging_bridge_group, sizeof(MIB_L2BRIDGE_GROUP_T)},
#endif
#ifdef CONFIG_IPV6
	{MIB_IPV6_ROUTE_TBL, _ip6_route, sizeof(MIB_CE_IPV6_ROUTE_T)},
#endif
#ifdef CONFIG_EPON_FEATURE
	{MIB_EPON_LLID_TBL, _epon_llid, sizeof(MIB_CE_MIB_EPON_LLID_T)},
#endif
#ifdef CONFIG_DSL_VTUO
	{MIB_VTUO_DPBO_TBL, _vtuo_dpbo, sizeof(MIB_CE_VTUO_DPBO_T)},
	{MIB_VTUO_PSD_DS_TBL, _vtuo_psd_ds, sizeof(MIB_CE_VTUO_PSD_T)},
	{MIB_VTUO_PSD_US_TBL, _vtuo_psd_us, sizeof(MIB_CE_VTUO_PSD_T)},
	/*
	{MIB_VTUO_VN_DS_TBL, _vtuo_vn_ds, sizeof(MIB_CE_VTUO_VN_T)},
	{MIB_VTUO_VN_US_TBL, _vtuo_vn_us, sizeof(MIB_CE_VTUO_VN_T)},
	{MIB_VTUO_RFI_TBL, _vtuo_rfi, sizeof(MIB_CE_VTUO_RFI_T)},
	*/
#endif /*CONFIG_DSL_VTUO*/
#ifdef CONFIG_USER_DOT1AG_UTILS
	{MIB_DOT1AG_TBL, _dot1ag, sizeof(MIB_CE_DOT1AG_T)},
#endif
#ifdef OSGI_SUPPORT
	{MIB_OSGI_BUNDLE_TBL, _osgi_bundle, sizeof(MIB_CE_OSGI_BUNDLE_T)},
#endif
#ifdef CONFIG_CWMP_TRANSFER_QUEUE
	{CWMP_TRANSFER_QUEUE_TBL, _cwmp_transfer_queue, sizeof(MIB_CE_CWMP_TRANSFER_T)},
#endif
    {0}
};

// for get chain position by chain id
int
getPositionById(unsigned int ID, char **buf)
{
	int i = 0;
	while (chainInfo[i].id != 0) {
		if (chainInfo[i].id == ID) {
			//printf("return i=%d\n", i);
			*buf = chainInfo[i].pos;
			return chainInfo[i].len;
		} else	++i;
	}
	return 0;
}

