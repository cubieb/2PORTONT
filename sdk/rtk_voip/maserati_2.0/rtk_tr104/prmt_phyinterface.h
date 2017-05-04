#ifndef __PRMT_VOICE_PHYINTERFACE_H__
#define __PRMT_VOICE_PHYINTERFACE_H__

#include "prmt_igd.h"
extern struct CWMP_LEAF tTestsXCT_SimulateTestEntityLeaf[];
extern struct CWMP_LEAF tXCT_STPoorQualityListEntityLeaf[];
extern struct CWMP_LINKNODE tXCT_STPoorQualityListObject[];
extern struct CWMP_NODE tXCT_StatusEntityObject[];
extern struct CWMP_LEAF tPhyInfTestsEntityLeaf[];
extern struct CWMP_NODE tPhyInfTestsEntityObject[];
extern struct CWMP_LEAF tPhyInterfaceEntityLeaf[];
extern struct CWMP_NODE tPhyInterfaceEntityObject[];
extern struct CWMP_LINKNODE tPhyInterfaceObject[];

int objPhyInterface(char *name, struct CWMP_LEAF *e, int type, void *data);

/* VoiceService.{i}.PhyInterface.{i}.InterfaceID */
int getPhyInterfaceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPhyInterfaceEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.PhyInterface.{i}.Tests. */
int getPhyInfTestsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPhyInfTestsEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

/* VoiceService.{i}.PhyInterface.{i}.Tests.X_SimulateTest. */
int getXCTSimulateTestEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setXCTSimulateTestEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int objPhyPoorQualityList(char *name, struct CWMP_LEAF *e, int type, void *data);

/* VoiceService.{i}.PhyInterface.{i}.X_Stats.PoorQualityList.{i}. */
int getXCT_STPoorQualityListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#endif /* __PRMT_VOICE_PHYINTERFACE_H__ */
