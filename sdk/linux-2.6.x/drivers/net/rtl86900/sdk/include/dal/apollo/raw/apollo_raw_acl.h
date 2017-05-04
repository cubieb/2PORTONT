#ifndef _APOLLO_RAW__ACL_H_
#define _APOLLO_RAW_ACL_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_ACLRULENO					64
#define APOLLO_ACLRULEMAX					(APOLLO_ACLRULENO-1)
#define APOLLO_ACLRULEMASK 					0x3f
#define APOLLO_ACLRULEFIELDNO			    8
#define APOLLO_ACLTEMPLATENO				4
#define APOLLO_ACLTEMPLATEMAX			    (APOLLO_ACLTEMPLATENO-1)

#define APOLLO_ACLACTIONNO				    128
#define APOLLO_ACLACTIONMAX					(APOLLO_ACLACTIONNO-1)


#define APOLLO_ACLRULETBLEN				    5
#define APOLLO_ACLACTTBLEN				    2
#define APOLLO_ACLRULETBADDR(type, rule)	((type << 6) | rule)

#define APOLLO_ACLRANGENO1					16
#define APOLLO_ACLRANGEMAX1				    (APOLLO_ACLRANGENO1-1)
#define APOLLO_ACLRANGENO2					8
#define APOLLO_ACLRANGEMAX2				    (APOLLO_ACLRANGENO2-1)


#define APOLLO_RAW_ACL_ENACT_CVLAN_MASK         0x01
#define APOLLO_RAW_ACL_ENACT_SVLAN_MASK         0x02
#define APOLLO_RAW_ACL_ENACT_PRIORITY_MASK    	0x04
#define APOLLO_RAW_ACL_ENACT_POLICING_MASK    	0x08
#define APOLLO_RAW_ACL_ENACT_FWD_MASK    		0x10
#define APOLLO_RAW_ACL_ENACT_INTGPIO_MASK    	0x20
#define APOLLO_RAW_ACL_ENACT_NOT_MASK    	    0x40
#define APOLLO_RAW_ACL_ENACT_INIT_MASK			0x7F

#define APOLLO_RAW_ACL_FIELD_SELECTOR_NO		16
#define APOLLO_RAW_ACL_FIELD_SELECTOR_MAX		(APOLLO_RAW_ACL_FIELD_SELECTOR_NO-1)

#define APOLLO_RAW_ACL_PKTLEN_MAX				((1<<14)-1)


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef enum apollo_raw_acl_fwdAct_e
{
    APOLLO_ACL_FWDACT_COPY = 0,
    APOLLO_ACL_FWDACT_REDIRECT,
    APOLLO_ACL_FWDACT_MIRROR,
    APOLLO_ACL_FWDACT_TRAP,
    APOLLO_ACL_FWDACT_END
} apollo_raw_acl_fwdAct_t;

typedef enum apollo_raw_acl_cvlanAct_e
{
    APOLLO_ACL_CACT_INGRESS = 0,
    APOLLO_ACL_CACT_EGRESS,
    APOLLO_ACL_CACT_SVID,
    APOLLO_ACL_CACT_POLICE,
    APOLLO_ACL_CACT_LOGGING,
    APOLLO_ACL_CACT_1PREMARK,
    APOLLO_ACL_CACT_END
} apollo_raw_acl_cvlanAct_t;

typedef enum apollo_raw_acl_svlanAct_e
{
    APOLLO_ACL_SACT_INGRESS = 0,
    APOLLO_ACL_SACT_EGRESS,
    APOLLO_ACL_SACT_CVID,
    APOLLO_ACL_SACT_POLICE,
    APOLLO_ACL_SACT_LOGGING,
    APOLLO_ACL_SACT_1PREMARK,
    APOLLO_ACL_SACT_DSCPREMARK,
    APOLLO_ACL_SACT_END
} apollo_raw_acl_svlanAct_t;

typedef enum apollo_raw_acl_policeAct_e
{
    APOLLO_ACL_PACT_POLICE = 0,
    APOLLO_ACL_PACT_LOGGING,
    APOLLO_ACL_PACT_END
} apollo_raw_acl_policeAct_t;


typedef enum apollo_raw_acl_priAct_e
{
    APOLLO_ACL_PRIACT_PRI = 0,
    APOLLO_ACL_PRIACT_DSCPREMARK,
    APOLLO_ACL_PRIACT_1PREMARK,
    APOLLO_ACL_PRIACT_POLICE,
    APOLLO_ACL_PRIACT_LOGGING,
    APOLLO_ACL_PRIACT_END
} apollo_raw_acl_priAct_t;

typedef enum apollo_raw_acl_cfAct_e
{
    APOLLO_ACL_CFACT_NONE = 0,
    APOLLO_ACL_CFACT_SID,
    APOLLO_ACL_CFACT_LLID,
    APOLLO_ACL_CFACT_VCEXT,
    APOLLO_ACL_CFACT_END,
} apollo_raw_acl_cfAct_t;

typedef enum apollo_raw_acl_fieldTypes_e
{
	ACL_UNUSED,
	ACL_DMAC0,
	ACL_DMAC1,
	ACL_DMAC2,
	ACL_SMAC0,
	ACL_SMAC1,
	ACL_SMAC2,
	ACL_ETHERTYPE,
	ACL_STAG,
	ACL_CTAG,
	ACL_GEMIDXLLIDX,
	ACL_IP4SIP0 = 0x10,
	ACL_IP4SIP1,
	ACL_IP4DIP0,
	ACL_IP4DIP1,
	ACL_IP6SIP0WITHIPV4 = 0x20,
	ACL_IP6SIP1WITHIPV4,
	ACL_IP6DIP0WITHIPV4 = 0x28,
	ACL_IP6DIP1WITHIPV4,
	ACL_IP6NH,
	ACL_VIDRANGE = 0x30,
	ACL_IPRANGE,
	ACL_PORTRANGE,
	ACL_PKTLENRANGE,
	ACL_FIELDVALID,
	ACL_FIELD_SELECT00 = 0x40,
	ACL_FIELD_SELECT01,
	ACL_FIELD_SELECT02,
	ACL_FIELD_SELECT03,
	ACL_FIELD_SELECT04,
	ACL_FIELD_SELECT05,
	ACL_FIELD_SELECT06,
	ACL_FIELD_SELECT07,
	ACL_FIELD_SELECT08,
	ACL_FIELD_SELECT09,
	ACL_FIELD_SELECT10,
	ACL_FIELD_SELECT11,
	ACL_FIELD_SELECT12,
	ACL_FIELD_SELECT13,
	ACL_FIELD_SELECT14,
	ACL_FIELD_SELECT15,
	ACL_TYPE_END
}apollo_raw_acl_fieldTypes_t;

typedef enum apollo_raw_acl_rngVidTypes_e
{
    APOLLO_ACL_RNGVIDTYPE_NONVALID = 0,
    APOLLO_ACL_RNGVIDTYPE_CVLAN,
    APOLLO_ACL_RNGVIDTYPE_SVLAN,
    APOLLO_ACL_RNGVIDTYPE_END,
}apollo_raw_rngVidTypes_t;

typedef enum apollo_raw_acl_rngIpTypes_e
{
    APOLLO_ACL_RNGIPTYPE_NONVALID = 0,
    APOLLO_ACL_RNGIPTYPE_V4SIP,
    APOLLO_ACL_RNGIPTYPE_V4DIP,
    APOLLO_ACL_RNGIPTYPE_V6SIP,
    APOLLO_ACL_RNGIPTYPE_V6DIP,
    APOLLO_ACL_RNGIPTYPE_END,
}apollo_raw_rngIpTypes_t;

typedef enum apollo_raw_acl_rngL4PortTypes_e
{
    APOLLO_ACL_RNGL4PORTTYPE_NONVALID = 0,
    APOLLO_ACL_RNGL4PORTTYPE_SRC,
    APOLLO_ACL_RNGL4PORTTYPE_DEST,
    APOLLO_ACL_RNGL4PORTTYPE_END,
}apollo_raw_rngL4PortTypes_t;

typedef enum apollo_raw_acl_rngPktLenTypes_e
{
    APOLLO_ACL_RNGPKTLENTYPE_NOTREVISE = 0,
    APOLLO_ACL_RNGPKTLENTYPE_REVISE,
    APOLLO_ACL_RNGPKTLENTYPE_END,
}apollo_raw_rngPktLenTypes_t;


typedef enum apollo_raw_acl_modeTypes_e
{
    APOLLO_ACL_MODE_64ENTRIES = 0,
    APOLLO_ACL_MODE_128ENTRIES,
    APOLLO_ACL_MODE_END
}apollo_raw_acl_modeTypes_t;

typedef enum apollo_raw_acl_ruleTag_e
{
    APOLLO_ACL_RULETAG_PPPOE = 0,
    APOLLO_ACL_RULETAG_CTAG,
    APOLLO_ACL_RULETAG_STAG,
    APOLLO_ACL_RULETAG_IP4,
    APOLLO_ACL_RULETAG_IP6,
    APOLLO_ACL_RULETAG_TCP,
    APOLLO_ACL_RULETAG_UDP,
    APOLLO_ACL_RULETAG_END    
}apollo_raw_acl_ruleTag_t;



typedef struct apollo_raw_acl_rule_s
{
	uint32 active_portmsk;
	uint32 type;
	uint32 tag_exist;
	uint32 field[APOLLO_ACLRULEFIELDNO];
    
}apollo_raw_acl_rule_t;

typedef struct apollo_raw_acl_ruleEntry_s
{
    uint32 idx;
    apollo_raw_acl_modeTypes_t mode;
    
    apollo_raw_acl_rule_t data_bits;
	uint32 valid;
    apollo_raw_acl_rule_t care_bits;

}apollo_raw_acl_ruleEntry_t;


typedef struct apollo_raw_acl_template_s{
    uint32 idx;
	apollo_raw_acl_fieldTypes_t field[APOLLO_ACLRULEFIELDNO];
}apollo_raw_acl_template_t;

typedef struct apollo_raw_acl_act_s{

    uint32 idx;
    
    uint32 cfidx;
    apollo_raw_acl_cfAct_t cfact;
    uint32 aclint;
    uint32 pridx;
    apollo_raw_acl_priAct_t priact;
    uint32 fwdpmask;
    apollo_raw_acl_fwdAct_t fwdact;
    uint32 meteridx;
    apollo_raw_acl_policeAct_t policact;
    uint32 svidx_sact;
    apollo_raw_acl_svlanAct_t sact;
    uint32 cvidx_cact;
    apollo_raw_acl_cvlanAct_t cact;
}apollo_raw_acl_act_t;

typedef struct apollo_raw_acl_actCtrl_s{

    uint32 idx;
    
    rtk_enable_t valid;
    rtk_enable_t not;
    rtk_enable_t int_cf_en;
    rtk_enable_t pri_remark_en;
    rtk_enable_t fwd_en;
    rtk_enable_t polic_en;
    rtk_enable_t svlan_en;
    rtk_enable_t cvlan_en;
}apollo_raw_acl_actCtrl_t;

typedef struct apollo_raw_acl_portRange_s{

    uint32 idx;
    apollo_raw_rngL4PortTypes_t type;
    uint32 upperPort;
    uint32 lowerPort;
}apollo_raw_acl_portRange_t;


typedef struct apollo_raw_acl_vidRange_s{

    uint32 idx;
    apollo_raw_rngVidTypes_t type;
    uint32 upperVid;
    uint32 lowerVid;
}apollo_raw_acl_vidRange_t;

typedef struct apollo_raw_acl_ipRange_s{

    uint32 idx;

    apollo_raw_rngIpTypes_t type;
    ipaddr_t upperIp;
    ipaddr_t lowerIp;
}apollo_raw_acl_ipRange_t;

typedef struct apollo_raw_acl_pktLenRange_s{

    uint32 idx;

    apollo_raw_rngPktLenTypes_t type;
    uint32 upperPktLen;
    uint32 lowerPktLen;
}apollo_raw_acl_pktLenRange_t;


extern int32 apollo_raw_acl_state_set(uint32 port, uint32 enabled);
extern int32 apollo_raw_acl_state_get(uint32 port, uint32 *pEnabled);
extern int32 apollo_raw_acl_permit_set(uint32 port, uint32 enabled);
extern int32 apollo_raw_acl_permit_get(uint32 port, uint32 *pEnabled);
extern int32 apollo_raw_acl_actCtrl_set(apollo_raw_acl_actCtrl_t *pAclActCtrl);
extern int32 apollo_raw_acl_actCtrl_get(apollo_raw_acl_actCtrl_t *pAclActCtrl);
extern int32 apollo_raw_acl_act_set(apollo_raw_acl_act_t *pAclAct);
extern int32 apollo_raw_acl_act_get(apollo_raw_acl_act_t *pAclAct);
extern int32 apollo_raw_acl_rule_set(apollo_raw_acl_ruleEntry_t *pAclRule);
extern int32 apollo_raw_acl_rule_get(apollo_raw_acl_ruleEntry_t *pAclRule);
extern int32 apollo_raw_acl_template_set(apollo_raw_acl_template_t *pAclType);
extern int32 apollo_raw_acl_template_get(apollo_raw_acl_template_t *pAclType);
extern int32 apollo_raw_acl_portRange_set(apollo_raw_acl_portRange_t *pPortRange);
extern int32 apollo_raw_acl_portRange_get(apollo_raw_acl_portRange_t *pPortRange);
extern int32 apollo_raw_acl_vidRange_set(apollo_raw_acl_vidRange_t *pVidRange);
extern int32 apollo_raw_acl_vidRange_get(apollo_raw_acl_vidRange_t *pVidRange);
extern int32 apollo_raw_acl_ipRange_set(apollo_raw_acl_ipRange_t *pIpRange);
extern int32 apollo_raw_acl_ipRange_get(apollo_raw_acl_ipRange_t *pIpRange);
extern int32 apollo_raw_acl_pktLenRange_set(apollo_raw_acl_pktLenRange_t *pPktLenRange);
extern int32 apollo_raw_acl_pktLenRange_get(apollo_raw_acl_pktLenRange_t *pPktLenRange);

extern int32 apollo_raw_acl_mode_set(apollo_raw_acl_modeTypes_t mode);
extern int32 apollo_raw_acl_mode_get(apollo_raw_acl_modeTypes_t *pMode);

#endif /*_APOLLO_RAW_ACL_H_*/


