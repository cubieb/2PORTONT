#ifndef CHIP_DEPS_H
#define CHIP_DEPS_H
typedef enum {
     OMCI_MODE_PPPOE = 0,
     OMCI_MODE_IPOE,
     OMCI_MODE_BRIDGE,
     OMCI_MODE_PPPOE_V4NAPT_V6,
     OMCI_MODE_IPOE_V4NAPT_V6,
} OMCI_MODE_T;

//Weight Define for ACL Rules
//The larger number, the higher priority.
typedef enum 
{ 
	RG_RESERVED_ACL_WEIGHT=0, 
	RG_OMCI_QOS_ACL_WEIGHT=100,   //For OMCI set QoS rule
	RG_DEFAULT_ACL_WEIGHT=200,	  
	RG_QOS_LOW_ACL_WEIGHT=300,	  //For TR069, Web UI set QoS rule, Lower priority in QoS.
	RG_QOS_ACL_WEIGHT=350,		  //For TR069, Web UI set QoS rule.
	RG_FIREWALL_ACL_WEIGHT=400 
} RG_ACL_WEIGHT_T; 
#endif
