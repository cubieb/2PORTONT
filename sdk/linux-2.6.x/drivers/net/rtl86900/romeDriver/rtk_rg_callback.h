
int _rtk_rg_arpAddByHwCallBack(rtk_rg_arpInfo_t* arpInfo);
int _rtk_rg_arpDelByHwCallBack(rtk_rg_arpInfo_t* arpInfo);
int _rtk_rg_macAddByHwCallBack(rtk_rg_macEntry_t* macInfo);
int _rtk_rg_macDelByHwCallBack(rtk_rg_macEntry_t* macInfo);
int _rtk_rg_naptAddByHwCallBack(rtk_rg_naptInfo_t* naptInfo);
int _rtk_rg_naptDelByHwCallBack(rtk_rg_naptInfo_t* naptInfo);
int _rtk_rg_routingAddByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo);
int _rtk_rg_routingDelByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo);
int _rtk_rg_bindingAddByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo);
int _rtk_rg_bindingDelByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo);

int _rtk_rg_initParameterSetByHwCallBack(void);
int _rtk_rg_interfaceAddByHwCallBack(rtk_rg_intfInfo_t* intfInfo,int* intfIdx);
int _rtk_rg_interfaceDelByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx);
int _rtk_rg_pppoeBeforeDiagByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t* before_diag, int* intfIdx);
int _rtk_rg_pptpBeforeDialByHwCallBack(rtk_rg_pptpClientInfoBeforeDial_t* before_dial, int* intfIdx);
int _rtk_rg_l2tpBeforeDialByHwCallBack(rtk_rg_l2tpClientInfoBeforeDial_t* before_dial, int* intfIdx);
int _rtk_rg_neighborAddByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo);
int _rtk_rg_neighborDelByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo);
int _rtk_rg_v6RoutingAddByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et);
int _rtk_rg_v6RoutingDelByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et);
int _rtk_rg_dhcpRequestByHwCallBack(int* intfIdx);
int _rtk_rg_pppoeDsliteBeforeDialByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t *before_diag, int *intfIdx);
int _rtk_rg_softwareNaptInfoAddCallBack(rtk_rg_naptInfo_t* naptInfo);
int _rtk_rg_softwareNaptInfoDeleteCallBack(rtk_rg_naptInfo_t* naptInfo);
int _rtk_rg_naptPreRouteDPICallBack(void *data, rtk_rg_naptDirection_t direct);
int _rtk_rg_naptForwardDPICallBack(void *data, rtk_rg_naptDirection_t direct);

