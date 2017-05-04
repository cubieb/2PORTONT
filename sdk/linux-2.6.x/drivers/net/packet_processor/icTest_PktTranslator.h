extern uint8 	*pVirtualPktBuffer;

/*  Prototype of testModel case */
int32 testModel_PktTranslator(uint32 pItem, uint32 pValue);
int32 testTranslator_MACToMAC(uint32 caseNo);
int32 testTranslator_SARToMAC(uint32 caseNo);
int32 testTranslator_WLANToMAC(uint32 caseNo);
int32 testTranslator_MACToSAR(uint32 caseNo);
int32 testTranslator_SARToSAR(uint32 caseNo);
int32 testTranslator_WLANToSAR(uint32 caseNo);
int32 testTranslator_MACToWLAN(uint32 caseNo);
int32 testTranslator_SARToWLAN(uint32 caseNo);
int32 testTranslator_WLANToWLAN(uint32 caseNo);
int32 testTranslator_RandomPkt_MAP_SRAM(uint32 caseNo);
int32 _PP_memComp(int srcportidx, void *desc, struct rtl_pkt *pPkt);

/* interface table setup function for test case */
int32 _setup_pkttbl(struct pktGen_Param_Table *iptbl);

