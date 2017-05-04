#ifndef RTL867x_QOSRULE_H
#define RTL867x_QOSRULE_H

#include <linux/if.h>
#include <net/rtl/rtl_types.h>

#define RTL867x_IPQos_Format_Ethernet		0x01
#define RTL867x_IPQos_Format_IP				0x02
#define RTL867x_IPQos_Format_IP_Range		0x03
#define RTL867x_IPQos_Format_TCP				0x04
#define RTL867x_IPQos_Format_TCP_Range		0x05
#define RTL867x_IPQos_Format_UDP				0x06
#define RTL867x_IPQos_Format_UDP_Range		0x07
#define RTL867x_IPQos_Format_8021p			0x08
#define RTL867x_IPQos_Format_srcPort			0x09
#define RTL867x_IPQos_Format_srcPort_setMask	0x10
#define RTL867x_IPQos_Format_srcFilter		0x11
#define RTL867x_IPQos_Format_srcFilter_Range	0x12
#define RTL867x_IPQos_Format_8367Ext_IPMask	0x13
#define RTL867x_IPQos_Format_8367Ext_IPRange	0x14

typedef struct rtl867x_hwnat_qos_rule{ 	

		uint8	rule_type; /* RTL867x_IPQos_Format_XXXX  */
		char outIfname[IFNAMSIZ]; 	/* If assign this,  we will add acl rule at the specific netif.
										Otherwise ,add it on each netif by default .*/	
		struct
		{	
			union {
				char 			_lan_netifname[IFNAMSIZ];	
				uint8			_lan_portmask;
			} PHY;

			struct {
				ether_addr_t	_dstMac;
				ether_addr_t	_dstMacMask;
				ether_addr_t	_srcMac;
				ether_addr_t	_srcMacMask;			
				uint16 			_ethType;
				uint16			_ethTypeMask;
				uint16			_vlanid;
				uint16			_vlanidMask;
				uint8			_8021p;
				uint8			_8021pMask;
			} L2;		

			struct {
				union
				{
					struct
					{
						uint32	_sip;
						uint32	_sipMask;
						uint32	_dip;
						uint32	_dipMask;
					} mask;

					struct
					{
						uint32	_sip_start;
						uint32	_sip_end;
						uint32	_dip_start;
						uint32	_dip_end;
					} range;
				}ip;

				uint8	_tos;
				uint8	_tosMask;
				uint8	_ipProto;
				uint8	_ipProtoMask;
			} L3;	


			struct {
				uint16 	_sport_start;
				uint16	_sport_end;
				uint16 	_dport_start;
				uint16	_dport_end;	
			}L4;
		}match_field;	


}rtl867x_hwnat_qos_rule_t;


#define Format_Ethernet_DMAC			match_field.L2._dstMac
#define Format_Ethernet_DMAC_M		match_field.L2._dstMacMask
#define Format_Ethernet_SMAC			match_field.L2._srcMac
#define Format_Ethernet_SMAC_M		match_field.L2._srcMacMask
#define Format_Ethernet_TYPE			match_field.L2._ethType
#define Format_Ethernet_TYPE_M		match_field.L2._ethTypeMask

#define Format_IP_SIP				match_field.L3.ip.mask._sip
#define Format_IP_SIP_M				match_field.L3.ip.mask._sipMask
#define Format_IP_DIP				match_field.L3.ip.mask._dip
#define Format_IP_DIP_M				match_field.L3.ip.mask._dipMask
#define Format_IP_TOS				match_field.L3._tos
#define Format_IP_TOS_M				match_field.L3._tosMask
#define Format_IP_Proto				match_field.L3._ipProto
#define Format_IP_Proto_M			match_field.L3._ipProtoMask

#define Format_IPRange_SIP_Sta		match_field.L3.ip.range._sip_start
#define Format_IPRange_SIP_End		match_field.L3.ip.range._sip_end
#define Format_IPRange_DIP_Sta		match_field.L3.ip.range._dip_start
#define Format_IPRange_DIP_End		match_field.L3.ip.range._dip_end
#define Format_IPRange_TOS			match_field.L3._tos
#define Format_IPRange_TOS_M			match_field.L3._tosMask
#define Format_IPRange_Proto			match_field.L3._ipProto
#define Format_IPRange_Proto_M		match_field.L3._ipProtoMask

#define Format_TCP_SIP				match_field.L3.ip.mask._sip
#define Format_TCP_SIP_M				match_field.L3.ip.mask._sipMask
#define Format_TCP_DIP				match_field.L3.ip.mask._dip
#define Format_TCP_DIP_M				match_field.L3.ip.mask._dipMask
#define Format_TCP_TOS				match_field.L3._tos
#define Format_TCP_TOS_M				match_field.L3._tosMask
#define Format_TCP_SPORT_Sta			match_field.L4._sport_start
#define Format_TCP_SPORT_End			match_field.L4._sport_end
#define Format_TCP_DPORT_Sta			match_field.L4._dport_start
#define Format_TCP_DPORT_End			match_field.L4._dport_end

#define Format_TCPRange_SIP_Sta		match_field.L3.ip.range._sip_start
#define Format_TCPRange_SIP_End		match_field.L3.ip.range._sip_end
#define Format_TCPRange_DIP_Sta		match_field.L3.ip.range._dip_start
#define Format_TCPRange_DIP_End		match_field.L3.ip.range._dip_end
#define Format_TCPRange_TOS			match_field.L3._tos
#define Format_TCPRange_TOS_M		match_field.L3._tosMask
#define Format_TCPRange_SPORT_Sta	match_field.L4._sport_start
#define Format_TCPRange_SPORT_End	match_field.L4._sport_end
#define Format_TCPRange_DPORT_Sta	match_field.L4._dport_start
#define Format_TCPRange_DPORT_End	match_field.L4._dport_end

#define Format_UDP_SIP				match_field.L3.ip.mask._sip
#define Format_UDP_SIP_M				match_field.L3.ip.mask._sipMask
#define Format_UDP_DIP				match_field.L3.ip.mask._dip
#define Format_UDP_DIP_M				match_field.L3.ip.mask._dipMask
#define Format_UDP_TOS				match_field.L3._tos
#define Format_UDP_TOS_M				match_field.L3._tosMask
#define Format_UDP_SPORT_Sta			match_field.L4._sport_start
#define Format_UDP_SPORT_End			match_field.L4._sport_end
#define Format_UDP_DPORT_Sta			match_field.L4._dport_start
#define Format_UDP_DPORT_End			match_field.L4._dport_end

#define Format_UDPRange_SIP_Sta		match_field.L3.ip.range._sip_start
#define Format_UDPRange_SIP_End		match_field.L3.ip.range._sip_end
#define Format_UDPRange_DIP_Sta		match_field.L3.ip.range._dip_start
#define Format_UDPRange_DIP_End		match_field.L3.ip.range._dip_end
#define Format_UDPRange_TOS			match_field.L3._tos
#define Format_UDPRange_TOS_M		match_field.L3._tosMask
#define Format_UDPRange_SPORT_Sta	match_field.L4._sport_start
#define Format_UDPRange_SPORT_End	match_field.L4._sport_end
#define Format_UDPRange_DPORT_Sta	match_field.L4._dport_start
#define Format_UDPRange_DPORT_End	match_field.L4._dport_end

#define Format_8021P_PRIORITY		match_field.L2._8021p
#define Format_8021P_PRIORITY_M		match_field.L2._8021pMask

#define Format_SRCPort_NETIFNAME		match_field.PHY._lan_netifname

#define Format_srcPort_setMask_LanPortMask	match_field.PHY._lan_portmask

#define Format_SRCFILTER_SMAC		match_field.L2._srcMac
#define Format_SRCFILTER_SMAC_M		match_field.L2._srcMacMask
#define Format_SRCFILTER_VLANID		match_field.L2._vlanid
#define Format_SRCFILTER_VLANID_M	match_field.L2._vlanidMask
#define Format_SRCFILTER_SIP			match_field.L3.ip.mask._sip
#define Format_SRCFILTER_SIP_M		match_field.L3.ip.mask._sipMask
#define Format_SRCFILTER_SPORT_Sta	match_field.L4._sport_start
#define Format_SRCFILTER_SPORT_End	match_field.L4._sport_end

#define Format_SRCFILTERRange_SMAC		match_field.L2._srcMac
#define Format_SRCFILTERRange_SMAC_M		match_field.L2._srcMacMask
#define Format_SRCFILTERRange_VLANID		match_field.L2._vlanid
#define Format_SRCFILTERRange_VLANID_M	match_field.L2._vlanidMask
#define Format_SRCFILTERRange_SIP_Sta	match_field.L3.ip.range._sip_start
#define Format_SRCFILTERRange_SIP_End	match_field.L3.ip.range._sip_end
#define Format_SRCFILTERRange_SPORT_Sta	match_field.L4._sport_start
#define Format_SRCFILTERRange_SPORT_End	match_field.L4._sport_end


#define Format_8367Ext_IPMask_NETIFNAME		match_field.PHY._lan_netifname
#define Format_8367Ext_IPMask_DMAC			match_field.L2._dstMac
#define Format_8367Ext_IPMask_DMAC_M			match_field.L2._dstMacMask
#define Format_8367Ext_IPMask_SMAC			match_field.L2._srcMac
#define Format_8367Ext_IPMask_SMAC_M			match_field.L2._srcMacMask
#define Format_8367Ext_IPMask_VLANID			match_field.L2._vlanid
#define Format_8367Ext_IPMask_VLANID_M		match_field.L2._vlanidMask
#define Format_8367Ext_IPMask_1P_PRIO		match_field.L2._8021p
#define Format_8367Ext_IPMask_1P_PRIO_M		match_field.L2._8021pMask
#define Format_8367Ext_IPMask_ETHTYPE		match_field.L2._ethType
#define Format_8367Ext_IPMask_ETHTYPE_M		match_field.L2._ethTypeMask
#define Format_8367Ext_IPMask_SIP			match_field.L3.ip.mask._sip
#define Format_8367Ext_IPMask_SIP_M			match_field.L3.ip.mask._sipMask
#define Format_8367Ext_IPMask_DIP			match_field.L3.ip.mask._dip
#define Format_8367Ext_IPMask_DIP_M			match_field.L3.ip.mask._dipMask
#define Format_8367Ext_IPMask_TOS			match_field.L3._tos
#define Format_8367Ext_IPMask_TOS_M			match_field.L3._tosMask
#define Format_8367Ext_IPMask_IpProto		match_field.L3._ipProto
#define Format_8367Ext_IPMask_IpProto_M		match_field.L3._ipProtoMask
#define Format_8367Ext_IPMask_SPORT_Sta		match_field.L4._sport_start
#define Format_8367Ext_IPMask_SPORT_End		match_field.L4._sport_end
#define Format_8367Ext_IPMask_DPORT_Sta		match_field.L4._dport_start
#define Format_8367Ext_IPMask_DPORT_End		match_field.L4._dport_end

#define Format_8367Ext_IPRange_NETIFNAME		match_field.PHY._lan_netifname
#define Format_8367Ext_IPRange_DMAC			match_field.L2._dstMac
#define Format_8367Ext_IPRange_DMAC_M		match_field.L2._dstMacMask
#define Format_8367Ext_IPRange_SMAC			match_field.L2._srcMac
#define Format_8367Ext_IPRange_SMAC_M		match_field.L2._srcMacMask
#define Format_8367Ext_IPRange_VLANID		match_field.L2._vlanid
#define Format_8367Ext_IPRange_VLANID_M		match_field.L2._vlanidMask
#define Format_8367Ext_IPRange_1P_PRIO		match_field.L2._8021p
#define Format_8367Ext_IPRange_1P_PRIO_M		match_field.L2._8021pMask
#define Format_8367Ext_IPRange_ETHTYPE		match_field.L2._ethType
#define Format_8367Ext_IPRange_ETHTYPE_M		match_field.L2._ethTypeMask
#define Format_8367Ext_IPRange_SIP_Sta		match_field.L3.ip.range._sip_start
#define Format_8367Ext_IPRange_SIP_End		match_field.L3.ip.range._sip_end
#define Format_8367Ext_IPRange_DIP_Sta		match_field.L3.ip.range._dip_start
#define Format_8367Ext_IPRange_DIP_End		match_field.L3.ip.range._dip_end
#define Format_8367Ext_IPRange_TOS			match_field.L3._tos
#define Format_8367Ext_IPRange_TOS_M			match_field.L3._tosMask
#define Format_8367Ext_IPRange_IpProto		match_field.L3._ipProto
#define Format_8367Ext_IPRange_IpProto_M		match_field.L3._ipProtoMask
#define Format_8367Ext_IPRange_SPORT_Sta		match_field.L4._sport_start
#define Format_8367Ext_IPRange_SPORT_End		match_field.L4._sport_end
#define Format_8367Ext_IPRange_DPORT_Sta		match_field.L4._dport_start
#define Format_8367Ext_IPRange_DPORT_End		match_field.L4._dport_end


#endif
