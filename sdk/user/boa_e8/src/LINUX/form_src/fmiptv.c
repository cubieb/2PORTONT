
#include <config/autoconf.h>
#include "../webs.h"
#include "fmdefs.h"
#include "mib.h"
#include "utility.h"
#ifdef CONFIG_RTK_RG_INIT
#include "../rtusr_rg_api.h"
#endif

#ifdef CONFIG_MCAST_VLAN
void formMcastVlanMapping(request * wp, char *path, char *query)
{
	char *strData;
	char *submitUrl;
	char *sWanName;
	int IgmpVlan;
	int IfIndex;
	int ifidx, entryNum, i, chainNum=-1;
	MIB_CE_ATM_VC_T entry;
	
	strData = boaGetVar(wp, "if_index", "");
	ifidx = atoi(strData);
	strData = boaGetVar(wp, "mVlan", "");
	IgmpVlan = atoi(strData);
	if(ifidx == -1 || IgmpVlan ==-1)
		goto back2add;
	sWanName = boaGetVar(wp, "WanName", "");
//printf("%s-%d ifidx=%d IgmpVlan=%d strData=%s\n",__func__,__LINE__,ifidx,IgmpVlan,sWanName);
	IfIndex = getifIndexByWanName(sWanName);
//printf("%s-%d IfIndex=%x\n",__func__,__LINE__,IfIndex);

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			return -1;
		}
		if(IfIndex == entry.ifIndex){
			printf("%s-%d i=%x\n",__func__,__LINE__,i);
			chainNum = i;
			break;
		}
	}
	entry.mVid = IgmpVlan;
//printf("%s-%d IgmpVlan=%d entry.mVid=%d chainNum=%d\n",__func__,__LINE__,IgmpVlan, entry.mVid,chainNum);
	if(chainNum != -1)
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, chainNum);
#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
	RTK_RG_ACL_Flush_mVlan();
	RTK_RG_ACL_Add_mVlan();
#endif
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	back2add: /*mean user cancel modify, refresh web page again!*/
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;


}
#endif



