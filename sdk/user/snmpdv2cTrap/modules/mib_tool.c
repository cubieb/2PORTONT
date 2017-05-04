#include "mib_tool.h"

#include	<stdio.h>
#include	<string.h>
#include	<netdb.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>


static int test_i=1;
extern int g_pppTestFlag;


#ifdef CONFIG_USER_SNMPD_MODULE_RM 
// Added by Mason Yu for Remote Management
// return TRUE if PVC2 is greater than PVC1
int pvc_is_greater(int vpi1, int vci1, int vpi2, int vci2) {
   if ((vpi2 > vpi1) || ((vpi2 == vpi1) && (vci2 > vci1)))
      return 1;

   return 0;
}


// Added by Mason Yu for Remote Management
//MIB_CE_ATM_VC_Tp vclTableSort(int vpi, int vci)
void vclTableSort(int vpi, int vci, MIB_CE_ATM_VC_Tp tmp_pEntry)
{
	int i, tmp_vpi, tmp_vci, tmp_ch;
	int entryNum;
	//MIB_CE_ATM_VC_Tp pEntry, tmp_pEntry;
	//MIB_CE_ATM_VC_Tp tmp_pEntry;
	MIB_CE_ATM_VC_T Entry;
	
	char pEntry_vpi[6], pEntry_vci[6];
	
	
	tmp_vpi = tmp_vci = tmp_ch = -1;
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	 
	for (i=0; i<entryNum; i++){
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);			
		snprintf(pEntry_vpi, 6, "%u", Entry.vpi);
		snprintf(pEntry_vci, 6, "%u", Entry.vci);		
		
		//tmp_pEntry = pEntry;
				
		if (!pvc_is_greater(vpi, vci, atoi(pEntry_vpi), atoi(pEntry_vci) )) 
         		continue;
         		
		
		if ((-1) == tmp_ch) { // first time here.			
         		tmp_vpi = atoi(pEntry_vpi); // just set it.
         		tmp_vci = atoi(pEntry_vci);
        		tmp_ch  = 1;
        		//tmp_pEntry = pEntry;
        		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
      		} 
      		 
         	if (pvc_is_greater(atoi(pEntry_vpi), atoi(pEntry_vci), tmp_vpi, tmp_vci)) {         		
            		tmp_vpi = atoi(pEntry_vpi);
            		tmp_vci = atoi(pEntry_vci);
            		tmp_ch  = 1;
            		//tmp_pEntry = &Entry;
            		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
         	} 
	}
	//return tmp_pEntry;
}	

// Added by Mason Yu for Remote Management
// return TRUE if ifindex2 is greater than ifindex1
int pvc_ifindex_is_greater(int ifindex1, int ifindex2) {
   if ( (ifindex2 > ifindex1) )
      return 1;

   return 0;
}


// Added by Mason Yu for Remote Management
//MIB_CE_ATM_VC_T* pppTableSort(unsigned long ifindex)
void pppTableSort(unsigned long ifindex, MIB_CE_ATM_VC_Tp tmp_pEntry)
{
	int i, tmp_ch;
	int entryNum;
	MIB_CE_ATM_VC_T Entry;	
	unsigned long tmp_ifindex;
	
	//printf("pppTableSort(Before): ifindex=%d\n", ifindex);
	
	tmp_ch = -1;
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	 
	for (i=0; i<entryNum; i++){
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);	
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);			
		
		//printf("pppTableSort(Before): ifindex=%d  Entry.cpePppIfIndex=%d\n", ifindex, Entry.cpePppIfIndex);		
		if ( !pvc_ifindex_is_greater(ifindex, Entry.cpePppIfIndex) ) 
         		continue;
         		
		//printf("pppTableSort(After): Entry.cpePppIfIndex=%d\n", Entry.cpePppIfIndex);
		
		if ((-1) == tmp_ch) { // first time here.         		
         		tmp_ifindex = Entry.cpePppIfIndex;
        		tmp_ch  = 1;
        		//tmp_pEntry = pEntry;
        		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
      		} 
      		 
         	if ( pvc_ifindex_is_greater(Entry.cpePppIfIndex, tmp_ifindex) ) {             		
            		tmp_ifindex = Entry.cpePppIfIndex;
            		tmp_ch  = 1;
            		//tmp_pEntry = pEntry;
            		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
         	} 
	}
	//return tmp_pEntry;
}


// Added by Mason Yu for Remote Management
//MIB_CE_ATM_VC_T* ipTableSort(unsigned long ifindex)
void ipTableSort(unsigned long ifindex, MIB_CE_ATM_VC_Tp tmp_pEntry)
{
	int i, tmp_ch;
	int entryNum;
	MIB_CE_ATM_VC_T Entry;	
	unsigned long tmp_ifindex;
	
	
	tmp_ch = -1;
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	 
	for (i=0; i<entryNum; i++){
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i);	
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);			
				
		if ( !pvc_ifindex_is_greater(ifindex, Entry.cpeIpIndex) ) 
         		continue;         		
		
		if ((-1) == tmp_ch) { // first time here.         		
         		tmp_ifindex = Entry.cpeIpIndex;
        		tmp_ch  = 1;
        		//tmp_pEntry = pEntry;
        		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
      		} 
      		 
         	if ( pvc_ifindex_is_greater(Entry.cpeIpIndex, tmp_ifindex) ) {             		
            		tmp_ifindex = Entry.cpeIpIndex;
            		tmp_ch  = 1;
            		//tmp_pEntry = pEntry;
            		memcpy(tmp_pEntry, &Entry, sizeof(Entry));
         	}     	
	}
	//return tmp_pEntry;
}

int deleteAtmVc(int vpi, int vci)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;	
	char tmp_vpi[6], tmp_vci[6];
	MIB_CE_ATM_VC_T entry;
	
	//printf("deleteAtmVc: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}			
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );	
		
		if ( (selected == -1) && ((atoi(tmp_vpi) == vpi) && (atoi(tmp_vci) == vci))  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}
		
	}
	
	
	if (selected == -1)
	{
		printf("deleteAtmVc Please select a VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	
	resolveServiceDependency(selected);
	if(mib_chain_delete(MIB_ATM_VC_TBL, selected) != 1) {
		printf("deleteAtmVcl: Delete chain record error!");		
	}
	
	return 1;
	
}	


//MIB_CE_ATM_VC_T searchpEntrybyVpiVci(int vpi, int vci, MIB_CE_ATM_VC_Tp pEntry)
int searchpEntrybyVpiVci(int vpi, int vci, MIB_CE_ATM_VC_Tp pEntry)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int drflag=0;		// Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	MIB_CE_ATM_VC_T entry;
	
	//printf("modifyChannelConf: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
		
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );	
		
		if ( (selected == -1) && ((atoi(tmp_vpi) == vpi) && (atoi(tmp_vci) == vci))  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}	
	
	if (selected == -1)
	{
		printf("searchpEntrybyVpiVci: Can not find this VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	snprintf(tmp_vpi, 6, "%u", Entry.vpi);
	snprintf(tmp_vci, 6, "%u", Entry.vci);
	//printf("searchpEntrybyVpiVci: atoi(tmp_vpi)=%d atoi(tmp_vci)=%d\n", atoi(tmp_vpi), atoi(tmp_vci));
		
	//return pEntry;
	return 1;	
}	


// Added by Mason Yu for Remote Management
// Search PVC by cpePppIfIndex
//MIB_CE_ATM_VC_Tp searchpEntrybyPppIfindex(unsigned long pppIfIndex)
MIB_CE_ATM_VC_T searchpEntrybyPppIfindex(unsigned long pppIfIndex, MIB_CE_ATM_VC_Tp pEntry)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int drflag=0;	        // Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	char vpi[6], vci[6];
	MIB_CE_ATM_VC_T entry;
	
	//printf("modifyChannelConfPPP: inputvpi = %d  inputvci = %d\n", inputvpi, inputvci);
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}		
		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );			
		
		if ( (selected == -1) && (pppIfIndex == Entry.cpePppIfIndex)  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}
	
	
	if (selected == -1)
	{
		printf("searchpEntrybyPppIfindex: Please select a VC channel!\n");
		return;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	snprintf(tmp_vpi, 6, "%u", Entry.vpi);
	snprintf(tmp_vci, 6, "%u", Entry.vci);
	//printf("searchpEntrybyPppIfindex: atoi(tmp_vpi)=%d atoi(tmp_vci)=%d\n", atoi(tmp_vpi), atoi(tmp_vci));
	
	//return pEntry;
}

// Added by Mason Yu for Remote Management
// Search PVC by VPI and VCI
int modifyChannelConf(struct channel_conf_para *para)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int drflag=0;		// Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	MIB_CE_ATM_VC_T entry;	
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}		
		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );	
		
		if ( (selected == -1) && ((atoi(tmp_vpi) == para->inputvpi) && (atoi(tmp_vci) == para->inputvci))  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}
	
	
	if (selected == -1)
	{
		printf("modifyChannelConf: Please select a VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	snprintf(tmp_vpi, 6, "%u", Entry.vpi);
	snprintf(tmp_vci, 6, "%u", Entry.vci);
	
	// Retrive the old value
	memcpy(&entry, &Entry, sizeof(entry));
	
	// cpePppIfIndex
	if ( para->pppIfIndex == 0 )
		entry.cpePppIfIndex = Entry.cpePppIfIndex;
	else {		
		entry.cpePppIfIndex = para->pppIfIndex;
	}	
	
	
	// cpeIpIndex
	if ( para->IpIndex == 0 )
		entry.cpeIpIndex = Entry.cpeIpIndex;
	else {		
		entry.cpeIpIndex = para->IpIndex;
	}
	
	
	// Retrive original value
	entry.vpi = para->inputvpi;
	entry.vci = para->inputvci;
	
	// set default Qos
	entry.qos = 0;
	entry.pcr = 2400;
	
	// Encaptulation	
	if ( para->encap == 100 )	
		entry.encap = Entry.encap;
	else if ( para->encap == 7 )	
		entry.encap = ENCAP_LLC;
	else
		entry.encap = ENCAP_VCMUX;	
		
	// Enable NAPT		
	if ( para->natmode == 1 )             // Enable
		entry.napt = 1;
	else if ( para->natmode == 2 )        // Disable
		entry.napt = 0;
	else 
		entry.napt = Entry.napt;
		
		
	// Enabled		
	if ( para->admin == 1 )             // up
		entry.enable = 1;
	else if ( para->admin == 2 )        // down
		entry.enable = 0;
	else 
		entry.enable = Entry.enable;
		
		
	if ( para->cmode != 0 ) {
		//printf("modifyChannelConf: para->cmode != 0  atoi(tmp_vpi)=%d atoi(tmp_vci)=%d\n", atoi(tmp_vpi), atoi(tmp_vci));
		if ( atoi(tmp_vpi) == 1 && atoi(tmp_vci) == 39 ) {
			printf("***** Stop %d/%d PVC connection *****\n",atoi(tmp_vpi), atoi(tmp_vci) );
			stopConnection(&Entry);				
		}
		
	}
	
	// Connection mode				
	//entry.cmode = pEntry->cmode;
	if ( para->cmode == 1 ) {            // PPPoA
		entry.cmode = 3;		
		
		// If the connection mode change from br1483 to PPPoA
		if ( Entry.cmode == 0 ) {
			entry.cpePppIfIndex = 0xff;
			entry.cpeIpIndex = 0xff;
		}
	}else if ( para->cmode == 2 ) {       // PPPoE
		entry.cmode = 2;		
		
		// If the connection mode change from br1483 to PPPoE
		if ( Entry.cmode == 0 ) {				
			entry.cpePppIfIndex = 0xff;
			entry.cpeIpIndex = 0xff;
		}
	}else if ( para->cmode == 3 ) {       // IPoA
		entry.cmode = 4; 		
		
		// If the connection mode change from br1483 to IPoA 
		if ( Entry.cmode == 0 ) {
			entry.cpeIpIndex = 0xff;  
		}  
	}else if ( para->cmode == 4 )        // Bridge1483
		entry.cmode = 0;     
	else if ( para->cmode == 5 )  {      // MER
		entry.cmode = 1;  
		
		// If the connection mode change from br1483 to MER
		if ( Entry.cmode == 0 ) {
			entry.cpeIpIndex = 0xff;  
		}  
	}else 
		entry.cmode = Entry.cmode;
	
	// Default Route	
	entry.dgw = Entry.dgw;
/*	
#ifdef PPPOE_PASSTHROUGH
	entry.brmode = 1;
	
	// 1483 bridged
	if (entry.cmode == CHANNEL_MODE_BRIDGE)
	{
		entry.brmode = 0;
	}
	else // PPP connection
#endif
*/
	if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA)
	{
		// PPP user name		
		strncpy(entry.pppUsername, Entry.pppUsername, MAX_NAME_LEN-1);
		entry.pppUsername[MAX_NAME_LEN]='\0';		

		// PPP password		
		strncpy(entry.pppPassword, Entry.pppPassword, MAX_NAME_LEN-1);
		entry.pppPassword[MAX_NAME_LEN]='\0';		

		// PPP connection type			
		entry.pppCtype = Entry.pppCtype;					
			
		// PPP idle time				
		entry.pppIdleTime = Entry.pppIdleTime;		
				
	}
	else // Wan IP setting
	{		
		entry.ipunnumbered = Entry.ipunnumbered;		

		// IP mode		
		//entry.ipDhcp = Entry.ipDhcp;			
		if ( para->dhcpmode == 1 )             // up
			entry.ipDhcp = 1;
		else if ( para->dhcpmode == 2 )        // down
			entry.ipDhcp = 0;
		else 
			entry.ipDhcp = Entry.ipDhcp;			
			
		// Local IP address
		((struct in_addr *)entry.ipAddr)->s_addr = ((struct in_addr *)Entry.ipAddr)->s_addr;			
		
		// Remote IP address
		((struct in_addr *)entry.remoteIpAddr)->s_addr = ((struct in_addr *)Entry.remoteIpAddr)->s_addr;
			
		
		// Subnet Mask, added by Jenny
		((struct in_addr *)entry.netMask)->s_addr = ((struct in_addr *)Entry.netMask)->s_addr;
					
	}
	
	
	// restore stuff not posted in this form
	entry.ifIndex = Entry.ifIndex;
	entry.qos = Entry.qos;
	entry.pcr = Entry.pcr;
	entry.scr = Entry.scr;
	entry.mbs = Entry.mbs;
	entry.cdvt = Entry.cdvt;
	entry.pppAuth = Entry.pppAuth;
	//entry.pppAuth = PPP_AUTH_AUTO;
	entry.rip = Entry.rip;
//	entry.dgw = Entry.dgw;
	entry.mtu = Entry.mtu;
	// Added by Mason Yu for Port Mapping	
	//entry.itfGroup = Entry.itfGroup;
//#ifdef PPPOE_PASSTHROUGH
	if ( para->brmode == 100 )
		entry.brmode = Entry.brmode;
	else
		entry.brmode = para->brmode;
//#endif
#ifdef CONFIG_RE8305
	// VLAN
	entry.vlan = Entry.vlan;
	entry.vid = Entry.vid;
	entry.vprio = Entry.vprio;
	entry.vpass = Entry.vpass;
#endif
	//strcpy(entry.pppACName, Entry.pppACName);
	strcpy(entry.pppACName, "");
  
	// disable this interface
	if (entry.enable == 0 && Entry.enable == 1)
		resolveServiceDependency(selected);
	
	
	// set default route flag
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
		if (!entry.dgw && Entry.dgw)
			drflag =0;
	else
		if (Entry.dgw)
			drflag = 0;

	// find the ifIndex
	if (entry.cmode != Entry.cmode)
	{
		entry.ifIndex = if_find_index(entry.cmode, ifMap);
		if (entry.ifIndex == NA_VC)
		{
			printf("Error(modifyChannelConf): Maximum number of VC exceeds !\n");
			return 0;
		}
		else if (entry.ifIndex == NA_PPP)
		{
			printf("Error(modifyChannelConf): Maximum number of PPP exceeds !\n");
			return 0;
		}
		
		// mode changed, restore to default
		if (entry.cmode == CHANNEL_MODE_PPPOE) {
			entry.mtu = 1492;
			entry.pppAuth = 0;
		}
		else {
//			entry.dgw = 1;
			entry.mtu = 1500;
		}
		
//		entry.dgw = 1;
#ifdef C8305
		// VLAN
		entry.vlan = 0;	// disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
	}
	
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
	{
		if (drflag && entry.dgw && !Entry.dgw)
		{
			printf("modifyChannelConf: Default route already exists !\n");
			return 0;
		}
		if (entry.dgw && !Entry.dgw)
			drflag = 1;
	}
	
	memcpy(&Entry, &entry, sizeof(entry));
	// log message
	mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
			
	return 1;
}

// Added by Mason Yu for Remote Management
// Search PVC by cpePppIfIndex
int modifyChannelConfPPP(struct channel_conf_ppp_para *para)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int drflag=0;		// Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	char vpi[6], vci[6];
	MIB_CE_ATM_VC_T entry;	
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}		
		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );			
		
		if ( (selected == -1) && (para->pppIfIndex == Entry.cpePppIfIndex)  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}
	
	
	if (selected == -1)
	{
		printf("modifyChannelConfPPP: Please select a VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	// Retrive the old value
	memcpy(&entry, &Entry, sizeof(entry));
	
	// To start connection(1/39) for PPPoE Testing
	if ( g_pppTestFlag == 1 ) {
		goto APPLY;
	}	
	
	// cpePppIfIndex
	entry.cpePppIfIndex = Entry.cpePppIfIndex;
	
	// cpeIpIndex
	entry.cpeIpIndex = Entry.cpeIpIndex;
	
	// Retrive original value
	snprintf(vpi, 6, "%u", Entry.vpi);
	snprintf(vci, 6, "%u", Entry.vci);
	//printf("modifyChannelConfPPP: atoi(vpi)=%d\n", atoi(vpi) );
	//printf("modifyChannelConfPPP: atoi(vci)=%d\n", atoi(vci) );
	
	entry.vpi = atoi(vpi);
	entry.vci = atoi(vci);
	
	// set default Qos
	entry.qos = 0;
	entry.pcr = 2400;
	
	// Encaptulation	
	entry.encap = Entry.encap;
	
	// Enabled NAPT
	entry.napt = Entry.napt;
		
		
	// Enabled		
	if ( para->admin == 1 )             // up
		entry.enable = 1;
	else if ( para->admin == 2 )        // down
		entry.enable = 0;
	else 
		entry.enable = Entry.enable;
	
	// Connection mode	
	entry.cmode = Entry.cmode;
	
	// Default Route	
	entry.dgw = Entry.dgw;
/*	
#ifdef PPPOE_PASSTHROUGH
	entry.brmode = 1;
	
	// 1483 bridged
	if (entry.cmode == CHANNEL_MODE_BRIDGE)
	{
		entry.brmode = 0;
	}
	else // PPP connection
#endif
*/
	if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA)
	{
		// PPP user name	
		if ( strcmp(para->pppUsername, "") ==0 ) {	
			strncpy(entry.pppUsername, Entry.pppUsername, MAX_NAME_LEN-1);
			entry.pppUsername[MAX_NAME_LEN]='\0';
		}else {
			strncpy(entry.pppUsername, para->pppUsername, MAX_NAME_LEN-1);
			entry.pppUsername[MAX_NAME_LEN]='\0';
		}		

		// PPP password		
		if ( strcmp(para->pppPassword, "") ==0 ) {
			strncpy(entry.pppPassword, Entry.pppPassword, MAX_NAME_LEN-1);
			entry.pppPassword[MAX_NAME_LEN]='\0';
		} else {
			strncpy(entry.pppPassword, para->pppPassword, MAX_NAME_LEN-1);
			entry.pppPassword[MAX_NAME_LEN]='\0';			
		}	

		// PPP connection type			
		entry.pppCtype = Entry.pppCtype;
			
			
		// PPP idle time
		if ( para->IdleTime!=0 ){
			// If Remote Management set IdleTime, we must set connection type as CONNECT_ON_DEMAND.	
			entry.pppCtype = CONNECT_ON_DEMAND;		
			entry.pppIdleTime = para->IdleTime;
		}else
			entry.pppIdleTime = Entry.pppIdleTime;
				
	}
	else // Wan IP setting
	{		
		entry.ipunnumbered = Entry.ipunnumbered;		

		// IP mode					 
		entry.ipDhcp = Entry.ipDhcp;			
			
		// Local IP address
		((struct in_addr *)entry.ipAddr)->s_addr = ((struct in_addr *)Entry.ipAddr)->s_addr;				
		
		// Remote IP address
		((struct in_addr *)entry.remoteIpAddr)->s_addr = ((struct in_addr *)Entry.remoteIpAddr)->s_addr;			
		
		// Subnet Mask, added by Jenny
		((struct in_addr *)entry.netMask)->s_addr = ((struct in_addr *)Entry.netMask)->s_addr;
					
	}
	
	
	// restore stuff not posted in this form
	entry.ifIndex = Entry.ifIndex;
	entry.qos = Entry.qos;
	entry.pcr = Entry.pcr;
	entry.scr = Entry.scr;
	entry.mbs = Entry.mbs;
	entry.cdvt = Entry.cdvt;
	entry.pppAuth = Entry.pppAuth;
	entry.rip = Entry.rip;
//	entry.dgw = Entry.dgw;
	// Added by Mason Yu for Port Mapping	
	//entry.itfGroup = Entry.itfGroup;
	
	// PPPMSS
	if ( para->mtu != 0 ) {		
		entry.mtu = para->mtu;
	}else {		
		entry.mtu = Entry.mtu;
	}
		
//#ifdef PPPOE_PASSTHROUGH
	entry.brmode = Entry.brmode;
//#endif
#ifdef CONFIG_RE8305
	// VLAN
	entry.vlan = Entry.vlan;
	entry.vid = Entry.vid;
	entry.vprio = Entry.vprio;
	entry.vpass = Entry.vpass;
#endif
	strcpy(entry.pppACName, Entry.pppACName);
  
	// disable this interface
	if (entry.enable == 0 && Entry.enable == 1)
		resolveServiceDependency(selected);
	
	
	// set default route flag
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
		if (!entry.dgw && Entry.dgw)
			drflag =0;
	else
		if (Entry.dgw)
			drflag = 0;

	// find the ifIndex
	if (entry.cmode != Entry.cmode)
	{
		entry.ifIndex = if_find_index(entry.cmode, ifMap);
		if (entry.ifIndex == NA_VC)
		{
			printf("Error(modifyChannelConfPPP): Maximum number of VC exceeds !\n");
			return 0;
		}
		else if (entry.ifIndex == NA_PPP)
		{
			printf("Error(modifyChannelConfPPP): Maximum number of PPP exceeds !\n");
			return 0;
		}
		
		// mode changed, restore to default
		//printf("modifyChannelConfPPP: Mode change and set PPPMSS\n");
		if (entry.cmode == CHANNEL_MODE_PPPOE) {
			entry.mtu = 1492;
			entry.pppAuth = 0;
		}
		else {
//			entry.dgw = 1;
			entry.mtu = 1500;
		}
		
//		entry.dgw = 1;
#ifdef C8305
		// VLAN
		entry.vlan = 0;	// disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
	}
	
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
	{
		if (drflag && entry.dgw && !Entry.dgw)
		{
			printf("modifyChannelConfPPP: Default route already exists !\n");
			return 0;
		}
		if (entry.dgw && !Entry.dgw)
			drflag = 1;
	}	
	
	
	memcpy(&Entry, &entry, sizeof(entry));
	// log message
	mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
	
	return 1;
	
APPLY:
	// Apply
	// When RM set PppAdminStatus = 3, we should start this pppoe connection on time.		
		startConnection(&Entry);
		//va_cmd("/bin/sarctl",1,1,"enable");			
	return 1;
}

// Added by Mason Yu for Remote Management
// Search PVC by cpeIpIndex
int modifyChannelConfIP(struct channel_conf_ip_para *para)
{
	int i, selected;
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int drflag=0;		// Jenny, check if default route exists
	char tmp_vpi[6], tmp_vci[6];
	char vpi[6], vci[6];
	MIB_CE_ATM_VC_T entry;	
	
	selected = -1;
	ifMap = 0;
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */
		
		//if(pEntry == NULL)
		//{			
		//	return 0;
		//}		
		
		
		snprintf(tmp_vpi, 6, "%u", Entry.vpi);
		snprintf(tmp_vci, 6, "%u", Entry.vci);
		//printf("atoi(tmp_vpi)=%d\n", atoi(tmp_vpi) );
		//printf("atoi(tmp_vci)=%d\n", atoi(tmp_vci) );			
		
		if ( (selected == -1) && (para->IpIndex == Entry.cpeIpIndex)  )
			selected = i;
		else
		{
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			if (Entry.dgw)
				drflag = 1;
	}
	
	
	if (selected == -1)
	{
		printf("modifyChannelConfIP: Please select a VC channel!\n");
		return 0;
	}
	
	//pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
	mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry); /* get the specified chain record */
	
	// Retrive the old value
	memcpy(&entry, &Entry, sizeof(entry));
	
	// cpePppIfIndex
	entry.cpePppIfIndex = Entry.cpePppIfIndex;
	
	// cpeIpIndex
	entry.cpeIpIndex = Entry.cpeIpIndex;
	
	// Retrive original value
	snprintf(vpi, 6, "%u", Entry.vpi);
	snprintf(vci, 6, "%u", Entry.vci);
	//printf("modifyChannelConfIP: atoi(vpi)=%d\n", atoi(vpi) );
	//printf("modifyChannelConfIP: atoi(vci)=%d\n", atoi(vci) );
	
	entry.vpi = atoi(vpi);
	entry.vci = atoi(vci);
	
	// set default Qos
	entry.qos = 0;
	entry.pcr = 2400;
	
	// Encaptulation	
	entry.encap = Entry.encap;
	
	// Enabled NAPT
	entry.napt = Entry.napt;
		
		
	// Enabled	
	entry.enable = Entry.enable;
	
	// Connection mode	
	entry.cmode = Entry.cmode;
	
	// Default Route	
	entry.dgw = Entry.dgw;
/*	
#ifdef PPPOE_PASSTHROUGH
	entry.brmode = 1;
	
	// 1483 bridged
	if (entry.cmode == CHANNEL_MODE_BRIDGE)
	{
		entry.brmode = 0;
	}
	else // PPP connection
#endif
*/
	// Marked by Mason Yu
	// PPPoE or PPPoA connection does not modify his VC via this function.
        
/*
	if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA)
	{
		// PPP user name			
		strncpy(entry.pppUsername, Entry.pppUsername, MAX_NAME_LEN-1);
		entry.pppUsername[MAX_NAME_LEN]='\0';
			
		

		// PPP password			
		strncpy(entry.pppPassword, Entry.pppPassword, MAX_NAME_LEN-1);
		entry.pppPassword[MAX_NAME_LEN]='\0';
			

		// PPP connection type			
		entry.pppCtype = Entry.pppCtype;
			
			
		// PPP idle time		
		entry.pppIdleTime = Entry.pppIdleTime;
				
	}
	else // Wan IP setting
*/	
	{		
		entry.ipunnumbered = Entry.ipunnumbered;
		

		// IP mode							 
		entry.ipDhcp = Entry.ipDhcp;		
			
			
		// Local IP address
		if ( strcmp(para->ipAddr, "") == 0 )				
			((struct in_addr *)entry.ipAddr)->s_addr = ((struct in_addr *)Entry.ipAddr)->s_addr;	
		else
			((struct in_addr *)entry.ipAddr)->s_addr = ((struct in_addr *)para->ipAddr)->s_addr;
		//printf("((struct in_addr *)entry.ipAddr)->s_addr=0x%x\n", ((struct in_addr *)entry.ipAddr)->s_addr);		
			
		
		// Remote IP address
		if ( strcmp(para->remoteIpAddr, "") == 0 )				
			((struct in_addr *)entry.remoteIpAddr)->s_addr = ((struct in_addr *)Entry.remoteIpAddr)->s_addr;	
		else
			((struct in_addr *)entry.remoteIpAddr)->s_addr = ((struct in_addr *)para->remoteIpAddr)->s_addr;
		//printf("((struct in_addr *)entry.remoteIpAddr)->s_addr=0x%x\n", ((struct in_addr *)entry.remoteIpAddr)->s_addr);		
			
		
		// Subnet Mask, added by Jenny
		if ( strcmp(para->netMask, "") == 0 )				
			((struct in_addr *)entry.netMask)->s_addr = ((struct in_addr *)Entry.netMask)->s_addr;	
		else
			((struct in_addr *)entry.netMask)->s_addr = ((struct in_addr *)para->netMask)->s_addr;
		//printf("((struct in_addr *)entry.netMask)->s_addr=0x%x\n", ((struct in_addr *)entry.netMask)->s_addr);
					
	}
	
	
	// restore stuff not posted in this form
	entry.ifIndex = Entry.ifIndex;
	entry.qos = Entry.qos;
	entry.pcr = Entry.pcr;
	entry.scr = Entry.scr;
	entry.mbs = Entry.mbs;
	entry.cdvt = Entry.cdvt;
	entry.pppAuth = Entry.pppAuth;
	entry.rip = Entry.rip;
//	entry.dgw = Entry.dgw;
	entry.mtu = Entry.mtu;
	// Added by Mason Yu for Port Mapping	
	//entry.itfGroup = Entry.itfGroup;
//#ifdef PPPOE_PASSTHROUGH
	entry.brmode = Entry.brmode;
//#endif
#ifdef CONFIG_RE8305
	// VLAN
	entry.vlan = Entry.vlan;
	entry.vid = Entry.vid;
	entry.vprio = Entry.vprio;
	entry.vpass = Entry.vpass;
#endif
	strcpy(entry.pppACName, Entry.pppACName);
  
	// disable this interface
	if (entry.enable == 0 && Entry.enable == 1)
		resolveServiceDependency(selected);
	
	
	// set default route flag
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
		if (!entry.dgw && Entry.dgw)
			drflag =0;
	else
		if (Entry.dgw)
			drflag = 0;

	// find the ifIndex
	if (entry.cmode != Entry.cmode)
	{
		entry.ifIndex = if_find_index(entry.cmode, ifMap);
		if (entry.ifIndex == NA_VC)
		{
			printf("Error(modifyChannelConfIP): Maximum number of VC exceeds !\n");
			return 0;
		}
		else if (entry.ifIndex == NA_PPP)
		{
			printf("Error(modifyChannelConfIP): Maximum number of PPP exceeds !\n");
			return 0;
		}
		
		// mode changed, restore to default
		if (entry.cmode == CHANNEL_MODE_PPPOE) {
			entry.mtu = 1492;
			entry.pppAuth = 0;
		}
		else {
//			entry.dgw = 1;
			entry.mtu = 1500;
		}
		
//		entry.dgw = 1;
#ifdef C8305
		// VLAN
		entry.vlan = 0;	// disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
	}
	
	if (entry.cmode != CHANNEL_MODE_BRIDGE)
	{
		if (drflag && entry.dgw && !Entry.dgw)
		{
			printf("modifyChannelConfIP: Default route already exists !\n");
			return 0;
		}
		if (entry.dgw && !Entry.dgw)
			drflag = 1;
	}
	
	memcpy(&Entry, &entry, sizeof(entry));
	// log message
	mib_chain_update(MIB_ATM_VC_TBL, (char *)&Entry, selected);
			
	return 1;
}

// Added by Mason Yu for Remote Management
int deleteAllPvc()
{
	unsigned int i;
	unsigned int idx;
	unsigned int totalEntry;
	
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	for (i=0; i<totalEntry; i++) {
		idx = totalEntry-i-1;			
		
		resolveServiceDependency(idx);
		if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
			printf("deleteAllPvc: Delete chain record error!");				
		}
		
	}
	
	
	
}	
#endif


unsigned int create_mib_tbl(struct mib_oid_tbl *tbl, unsigned int total, unsigned int max_oid_name)
{
	tbl->oid = (struct mib_oid *) calloc(total, sizeof(struct mib_oid));

	if(tbl->oid != 0)
	{
		tbl->total = total;
		tbl->oid_name_pool = (unsigned char*) malloc(total * max_oid_name);

		if(tbl->oid_name_pool != 0)
		{
			unsigned int idx;
			unsigned char *ptr = tbl->oid_name_pool;
			struct mib_oid * oid_ptr = tbl->oid;
			for(idx=0;idx<tbl->total;idx++, oid_ptr++)
			{
				oid_ptr->name = ptr;
				ptr += max_oid_name;
			}
		}
		else
		{
			free(tbl->oid);
			tbl->total = 0;		
		}	
	}
	else
		tbl->total = 0;		

//	printf("create_mib_tbl: tbl:%x, tbl->oid:%x, tbl->oid_name_pool:%x\n",(unsigned int)tbl ,(unsigned int)tbl->oid, (unsigned int)tbl->oid_name_pool);

	return tbl->total;
}

void free_mib_tbl(struct mib_oid_tbl *tbl)
{
#if 0
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;
	
	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		printf("free_mib_tbl(%d) oid(%x) name(%x)\n", idx,(unsigned int)oid_ptr ,(unsigned int)oid_ptr->name);
	}
#endif

//	printf("free_mib_tbl: tbl:%x, tbl->oid:%x, tbl->oid_name_pool:%x\n",(unsigned int)tbl ,(unsigned int)tbl->oid, (unsigned int)tbl->oid_name_pool);
	if(tbl->oid)
		free(tbl->oid);
	if(tbl->oid_name_pool)
		free(tbl->oid_name_pool);

	tbl->oid = 0;
	tbl->total = 0;	
	tbl->oid_name_pool = 0;	
}

void print_mib_tbl(struct mib_oid_tbl *tbl)
{
	unsigned int idx, item;
	struct mib_oid * oid_ptr = tbl->oid;
	unsigned char buf[256];

	printf("print_mib_tbl: tbl:%x total:%u\n",(unsigned int)tbl ,tbl->total);

	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		memset(buf,0x00,256);
		if((oid_ptr->name) && (oid_ptr->length))
		{
			for(item=0;item<oid_ptr->length;item++)
			{
				unsigned char temp[8];			
				sprintf(temp,"%d-",oid_ptr->name[item]);
				strcat(buf, temp);
			}

			printf("print_mib_tbl(%d) oid(%s)\n", idx, buf);
		}
	}
}

/** lexicographical compare two object identifiers.
 * 
 * Caution: this method is called often by
 *          command responder applications (ie, agent).
 *
 * @return -1 if name1 < name2, 0 if name1 = name2, 1 if name1 > name2
 */
int
snmp_oid_compare(const unsigned char * in_name1,
                 unsigned int len1, const unsigned char * in_name2, unsigned int len2)
{
    register int    len;
    register const unsigned char *name1 = in_name1;
    register const unsigned char *name2 = in_name2;

    /*
     * len = minimum of len1 and len2 
     */
    if (len1 < len2)
        len = len1;
    else
        len = len2;
    /*
     * find first non-matching OID 
     */
    while (len-- > 0) {
        /*
         * these must be done in seperate comparisons, since
         * subtracting them and using that result has problems with
         * subids > 2^31. 
         */
        // Mason Yu. The source code is correct. we need not to modify.         
        if (  *(name1) != *(name2)  ) {
        //if (  (*(name1) != *(name2)) && (*(name1+1) == *(name2+1)) ) {
            if (*(name1) < *(name2))
            //if (   (*(name1) < *(name2))  && (*(name1+1) == *(name2+1))   )
                return -1;
            return 1;
        }
        name1++;
        name2++;
    }
#if 0    
    /*
     * both OIDs equal up to length of shorter OID 
     */
    if (len1 < len2)
        return -1;
    if (len2 < len1)
        return 1;
#endif
    return 0;
}

int snmp_oid_getnext(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result)
{
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;	
	int i;
	
	
	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		//for (i=0; i<6; i++)
		//	printf("oid_ptr->name[%d]=%d  oid_ptr->length=%d in_name[%d]=%d len=%d\n", i, oid_ptr->name[i], oid_ptr->length, i, in_name[i], len);
		
		if(snmp_oid_compare(oid_ptr->name, oid_ptr->length, in_name, len) == 1)
		{
			*result = idx;			
			return 1;
		}
	}
	return 0;
}

int snmp_oid_get(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result)
{
	int i,j;
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;

	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		//for (j=0; j<6; j++)
		//	printf("Get: oid_ptr->name[%d]=%d  oid_ptr->length=%d in_name[%d]=%d len=%d\n", j, oid_ptr->name[j], oid_ptr->length, j, in_name[j], len);

		i = snmp_oid_compare(oid_ptr->name, oid_ptr->length, in_name, len);
		if(i == 0)
		{
			*result = idx;
			return 1;
		}

		if(i == 1)
		{
			return 0;
		}
		
	}
	return 0;
}







