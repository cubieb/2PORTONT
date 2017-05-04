#ifndef _PRMT_WANDEVICE_H_
#define _PRMT_WANDEVICE_H_

#include "prmt_igd.h"
//#include "parameter_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _SUPPORT_ADSL2WAN_PROFILE_
enum eWANDSLIFTestParamsLeaf	//TR-181 need this
{
#ifdef CONFIG_VDSL
	eTP_HLOGGds,
	eTP_HLOGGus,
#endif /*CONFIG_VDSL*/
	eTP_HLOGpsds,
	eTP_HLOGpsus,
	eTP_HLOGMTds,
	eTP_HLOGMTus,
#ifdef CONFIG_VDSL
	eTP_QLNGds,
	eTP_QLNGus,
#endif /*CONFIG_VDSL*/
	eTP_QLNpsds,
	eTP_QLNpsus,
	eTP_QLNMTds,
	eTP_QLNMTus,
#ifdef CONFIG_VDSL
	eTP_SNRGds,
	eTP_SNRGus,
#endif /*CONFIG_VDSL*/
	eTP_SNRpsds,
	eTP_SNRpsus,
	eTP_SNRMTds,
	eTP_SNRMTus,
	eTP_LATNds,
	eTP_LATNus,
	eTP_SATNds,
	eTP_SATNus,
};

extern struct  CWMP_LEAF tWANDSLIFTestParamsLeaf[];
int getWANDSLIFTestParams(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif //_SUPPORT_ADSL2WAN_PROFILE_

extern struct CWMP_LINKNODE tWANDeviceObjectDSL[];
extern struct CWMP_NODE tWANDevEntityObjectDSL[];
extern struct CWMP_LINKNODE tWANDeviceObjectEth[];
extern struct CWMP_NODE tWANDevEntityObjectEth[];
extern struct CWMP_LEAF tWANDevEntityLeaf[];
extern struct CWMP_LEAF tWANCmnIfCfgLeaf[];

int getWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANEthStatsLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int objWANDev(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANDEVICE_H_*/
