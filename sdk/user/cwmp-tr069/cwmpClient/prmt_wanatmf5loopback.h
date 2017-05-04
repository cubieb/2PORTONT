#ifndef _PRMT_WANATMF5LOOPBACK_H_
#define _PRMT_WANATMF5LOOPBACK_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

// also used in tr181/prmt_atm.c
struct WANATMF5Loopback
{
	int DiagState;
	int vpi;
	int vci;
	unsigned int NumOfRep;
	unsigned int Timeout;
	unsigned int SuccessCount;
	unsigned int FailureCount;
	unsigned int AvaRespTime;
	unsigned int MinRespTime;
	unsigned int MaxRespTime;
};

// also used in tr181/prmt_atm.c
enum eWANATMF5LBLeaf
{
	eF5_DiagnosticsState,
	eF5_NumberOfRepetitions,
	eF5_Timeout,
	eF5_SuccessCount,
	eF5_FailureCount,
	eF5_AverageResponseTime,
	eF5_MinimumResponseTime,
	eF5_MaximumResponseTime
};


extern struct CWMP_LEAF tWANATMF5LBLeaf[];

int getWANATMF5LB(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANATMF5LB(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern int gStartATMF5LB;
void cwmpStartATMF5LB(void);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANATMF5LOOPBACK_H_*/
