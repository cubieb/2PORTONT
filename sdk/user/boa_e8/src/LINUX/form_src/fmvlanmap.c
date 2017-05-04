/*
 *      Web server handler routines for NET
 *
 */

/*-- System inlcude files --*/
#include <config/autoconf.h>
#include "../webs.h"
#include "fmdefs.h"
#include "mib.h"
#include "utility.h"
#ifdef CONFIG_RTK_RG_INIT
#include "../rtusr_rg_api.h"
#endif

#define LANIF_NUM	9
struct vlan_pair {
	unsigned short vid_a;
	unsigned short vid_b;
#ifdef CONFIG_RTK_RG_INIT
	unsigned short rg_vbind_entryID;
#endif
};

int initPagePBind(int eid, request * wp, int argc, char ** argv)
{
	int total, i, k;
	char vlan_str[40];
	struct vlan_pair *vid_pair;
	MIB_CE_PORT_BINDING_T pbEntry;
	
	total = mib_chain_total(MIB_PORT_BINDING_TBL);
	
	for (i=0; i<total; i++) {
		mib_chain_get(MIB_PORT_BINDING_TBL, i, (void*)&pbEntry);
		vid_pair = (struct vlan_pair *)&pbEntry.pb_vlan0_a;
		boaWrite(wp, "setValue('Mode%d', %d);\n\t", i, pbEntry.pb_mode);
		if (pbEntry.pb_mode == 0) {
			boaWrite(wp, "setValue('VLAN%d', '');\n\t", i);
			continue;
		}
		vlan_str[0]='\0';
		for (k=0; k<4; k++) {
			if (vid_pair[k].vid_a) {
				if (k==0)
					sprintf(vlan_str, "%d/%d", vid_pair[k].vid_a, vid_pair[k].vid_b);
				else
					sprintf(vlan_str, "%s;%d/%d", vlan_str, vid_pair[k].vid_a, vid_pair[k].vid_b);
			}
		}
		boaWrite(wp, "setValue('VLAN%d', '%s');\n\t", i, vlan_str);
	}
}

void formVlanMapping(request * wp, char *path, char *query)
{
	char *strData;
	char 			*submitUrl;
	int total, i, ifidx, nVal, org_mode;
	char tmpBuf[100];
	MIB_CE_PORT_BINDING_T pbEntry;
	
	total = mib_chain_total(MIB_PORT_BINDING_TBL);
	if (total == 0) {
		memset(&pbEntry, 0, sizeof(MIB_CE_PORT_BINDING_T));
		for (i=0; i<LANIF_NUM; i++) {
			mib_chain_add(MIB_PORT_BINDING_TBL, (void *)&pbEntry);
		}
	}
	strData = boaGetVar(wp, "if_index", "");
	ifidx = atoi(strData);
	if (ifidx < 0 || ifidx >= LANIF_NUM) {
		strcpy(tmpBuf, strModChainerror);
		goto setErr_vmap;
	}
	mib_chain_get(MIB_PORT_BINDING_TBL, ifidx, (void*)&pbEntry);
#ifdef CONFIG_RTK_RG_INIT
	RG_flush_vlanBinding(ifidx);
#endif
	
	org_mode = pbEntry.pb_mode;
	strData = boaGetVar(wp, "Frm_Mode", "");
	nVal = atoi(strData);
	pbEntry.pb_mode = nVal;

	strData = boaGetVar(wp, "Frm_VLAN0a", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan0_a = nVal;
	strData = boaGetVar(wp, "Frm_VLAN0b", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan0_b = nVal;

	strData = boaGetVar(wp, "Frm_VLAN1a", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan1_a = nVal;
	strData = boaGetVar(wp, "Frm_VLAN1b", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan1_b = nVal;

	strData = boaGetVar(wp, "Frm_VLAN2a", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan2_a = nVal;
	strData = boaGetVar(wp, "Frm_VLAN2b", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan2_b = nVal;

	strData = boaGetVar(wp, "Frm_VLAN3a", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan3_a = nVal;
	strData = boaGetVar(wp, "Frm_VLAN3b", "");
	nVal = atoi(strData);
	pbEntry.pb_vlan3_b = nVal;
	
	mib_chain_update(MIB_PORT_BINDING_TBL, (void *)&pbEntry, ifidx);
	// sync with port-based mapping
	if (pbEntry.pb_mode!=0 && org_mode != pbEntry.pb_mode)
		sync_itfGroup(ifidx);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	setupnewEth2pvc();
	
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;
setErr_vmap:
	ERR_MSG(tmpBuf);
}

