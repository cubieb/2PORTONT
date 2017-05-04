#include <config/autoconf.h>
#include "../prmt_wandsldiagnostics.h"

#include "prmt_dsl_diag.h"

/****** Device.DSL.Diagnostics.ADSLLineTest. *********************************/
struct CWMP_OP tDSLLineTestLeafOP = { getDSLLineTest, setDSLLineTest };
struct CWMP_PRMT tDSLLineTestLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"DiagnosticsState",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ, &tDSLLineTestLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ, &tDSLLineTestLeafOP},
};

enum eDSLDiagLeaf
{
	eDSLDI_DiagnosticsState,
	eDSLDI_Interface,
};

//defiend in ../prmt_wandsldiagnostics.c
extern struct CWMP_LEAF tWANDSLDIAGLeaf[];
extern struct CWMP_PRMT tWANDSLDIAGLeafInfo[];

struct CWMP_LEAF tDSLLineTestLeaf[] =
{
{ &tDSLLineTestLeafInfo[eDSLDI_DiagnosticsState] },
{ &tDSLLineTestLeafInfo[eDSLDI_Interface] },
{ &tWANDSLDIAGLeafInfo[eDI_ACTPSDds] },
{ &tWANDSLDIAGLeafInfo[eDI_ACTPSDus] },
{ &tWANDSLDIAGLeafInfo[eDI_ACTATPds] },
{ &tWANDSLDIAGLeafInfo[eDI_ACTATPus] },
{ &tWANDSLDIAGLeafInfo[eDI_HLINSCds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLINSCus] },
#ifdef CONFIG_VDSL
{ &tWANDSLDIAGLeafInfo[eDI_HLINGds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLINGus] },
{ &tWANDSLDIAGLeafInfo[eDI_HLOGGds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLOGGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLDIAGLeafInfo[eDI_HLOGpsds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLOGpsus] },
{ &tWANDSLDIAGLeafInfo[eDI_HLOGMTds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLOGMTus] },
{ &tWANDSLDIAGLeafInfo[eDI_LATNpbds] },
{ &tWANDSLDIAGLeafInfo[eDI_LATNpbus] },
{ &tWANDSLDIAGLeafInfo[eDI_SATNds] },
{ &tWANDSLDIAGLeafInfo[eDI_SATNus] },
{ &tWANDSLDIAGLeafInfo[eDI_HLINpsds] },
{ &tWANDSLDIAGLeafInfo[eDI_HLINpsus] },
#ifdef CONFIG_VDSL
{ &tWANDSLDIAGLeafInfo[eDI_QLNGds] },
{ &tWANDSLDIAGLeafInfo[eDI_QLNGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLDIAGLeafInfo[eDI_QLNpsds] },
{ &tWANDSLDIAGLeafInfo[eDI_QLNpsus] },
{ &tWANDSLDIAGLeafInfo[eDI_QLNMTds] },
{ &tWANDSLDIAGLeafInfo[eDI_QLNMTus] },
#ifdef CONFIG_VDSL
{ &tWANDSLDIAGLeafInfo[eDI_SNRGds] },
{ &tWANDSLDIAGLeafInfo[eDI_SNRGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLDIAGLeafInfo[eDI_SNRpsds] },
{ &tWANDSLDIAGLeafInfo[eDI_SNRpsus] },
{ &tWANDSLDIAGLeafInfo[eDI_SNRMTds] },
{ &tWANDSLDIAGLeafInfo[eDI_SNRMTus] },
{ &tWANDSLDIAGLeafInfo[eDI_BITSpsds] },
{ NULL }
};

/****** Device.DSL.Diagnostics. **********************************************/
struct CWMP_PRMT tDSLDiagObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"ADSLLineTest",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLDiagObject
{
	eADSLLineTest,
};

struct CWMP_NODE tDSLDiagObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLDiagObjectInfo[eADSLLineTest],		tDSLLineTestLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/**** Operations *************************************************************/

//defiend in ../prmt_wandsldiagnostics.c
extern char *strDSLDiagState[];
extern struct WANDSLDiag gWANDSLDiag;
int getDSLLineTest(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	char		*pChar=NULL;
	int		vINT=0;
	unsigned int	vUINT=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		*data = strdup( strDSLDiagState[gWANDSLDiag.DiagState] );
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{	
		*data = strdup("Device.DSL.Channel.1");
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDSLLineTest(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		char *buf=data;		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, strDSLDiagState[DSLDIAG_REQUESTED] )!=0 ) return ERR_9007;
		if( gWANDSLDiag.DiagState==DSLDIAG_REQUESTED ) return ERR_9001;
		
		reset_WANDSLDiag( &gWANDSLDiag );
		gStartDSLDiag=1;
		gWANDSLDiag.DiagState = DSLDIAG_REQUESTED;
	}
	else if(strcmp( lastname, "Interface" )==0)
	{
		char *buf=data;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		// Only "Device.DSL.Channel.1" is accepted
		if( strcmp( buf, "Device.DSL.Channel.1" )!=0 ) return ERR_9007;

		return 0;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

