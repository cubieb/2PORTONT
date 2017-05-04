

int32 testParser_PktFromMAC(uint32 caseNo);
int32 testParser_PktFromSAR(uint32 caseNo);
int32 testParser_PktFromWLAN(uint32 caseNo);
int32 testParser_PktTrapToCPU(uint32 caseNo);
int32 testParser_RandomPkt_MAP_SRAM (uint32 caseNo);
int32 hsbInit(hsb_param_t *hsb, int32 IfType);
int32 _sim_DMAPktToMEM(int srcportidx, void *desc, struct rtl_pkt *pPkt);
int32 testParser_PktFrom8139ToMACvRx(uint32 caseNo);
int32 testParser_PktFrom8139ToEXTpTx(uint32 caseNo);
int32 testParser_PktFrom8139ToMACpTx(uint32 caseNo);
int32 testParser_PktFrom8139ToSARpTx(uint32 caseNo);

