#ifndef _PRMT_WANDSLDIAGNOSTICS_H_
#define _PRMT_WANDSLDIAGNOSTICS_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

// re-used by tr181/prmt_dsl_diag.c
enum { 
	DSLDIAG_NONE=0,
	DSLDIAG_REQUESTED,
	DSLDIAG_COMPLETE,
	DSLDIAG_ERROR_INTERNAL,
	DSLDIAG_ERROR_OTHER
};

// re-used by tr181/prmt_dsl_diag.c
struct WANDSLDiag
{
	int DiagState;
};


// re-used by tr181/prmt_dsl_diag.c
enum eWANDSLDIAGLeaf
{
	eDI_LoopDiagnosticsState,
	eDI_ACTPSDds,
	eDI_ACTPSDus,
	eDI_ACTATPds,
	eDI_ACTATPus,
	eDI_HLINSCds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINSCus,
#ifdef CONFIG_VDSL
	eDI_HLINGds,
	eDI_HLINGus,
	eDI_HLOGGds,
	eDI_HLOGGus,
#endif /*CONFIG_VDSL*/
	eDI_HLOGpsds,
	eDI_HLOGpsus,
	eDI_HLOGMTds,
	eDI_HLOGMTus,
	eDI_LATNpbds,
	eDI_LATNpbus,
	eDI_SATNds,
	eDI_SATNus,
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINpsus,
#ifdef CONFIG_VDSL
	eDI_QLNGds,
	eDI_QLNGus,
#endif /*CONFIG_VDSL*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_QLNpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_QLNpsus,
	eDI_QLNMTds,
	eDI_QLNMTus,
#ifdef CONFIG_VDSL
	eDI_SNRGds,
	eDI_SNRGus,
#endif /*CONFIG_VDSL*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_SNRpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_SNRpsus,
	eDI_SNRMTds,
	eDI_SNRMTus,
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_BITSpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	/*BITSpsus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_GAINSpsds
};


extern struct CWMP_LEAF tWANDSLDIAGLeaf[];

int getWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern int gStartDSLDiag;
void cwmpStartDSLDiag(void);



enum
{
	GET_DSL_ACTPSDds=0,
	GET_DSL_ACTPSDus,
	GET_DSL_ACTATPds,
	GET_DSL_ACTATPus,
	
	GET_DSL_HLINpsds,
	GET_DSL_HLINpsus,
	GET_DSL_HLINSCds,
	GET_DSL_HLINSCus,

	GET_DSL_HLOGpsds,
	GET_DSL_HLOGpsus,
	GET_DSL_HLOGMTds,
	GET_DSL_HLOGMTus,

	GET_DSL_QLNpsds,
	GET_DSL_QLNpsus,
	GET_DSL_QLNMTds,
	GET_DSL_QLNMTus,

	GET_DSL_SNRpsds,
	GET_DSL_SNRpsus,
	GET_DSL_SNRMTds,
	GET_DSL_SNRMTus,
	
	GET_DSL_LATNds,
	GET_DSL_LATNus,
	GET_DSL_SATNds,
	GET_DSL_SATNus,
	
	GET_DSL_BITSpsds,
	GET_DSL_GAINSpsds,
	
#ifdef CONFIG_VDSL
	GET_DSL_HLINGds,
	GET_DSL_HLINGus,
	GET_DSL_HLOGGds,
	GET_DSL_HLOGGus,
	GET_DSL_QLNGds,
	GET_DSL_QLNGus,
	GET_DSL_SNRGds,
	GET_DSL_SNRGus,
	GET_DSL_SNRMpbds,
	GET_DSL_SNRMpbus,
#endif /*CONFIG_VDSL*/
	
	DSL_END
};
int getDSLParameterValue( unsigned int name_idx, void *pvalue );




#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANDSLDIAGNOSTICS_H_*/
