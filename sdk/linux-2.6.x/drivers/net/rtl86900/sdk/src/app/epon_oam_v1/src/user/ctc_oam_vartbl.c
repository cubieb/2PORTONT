
static ctc_varCb_t ctc_stdAttrCb[] = {
    /*  The leaf value should be sorted
     *  { varBranch, varLeaf },
     *  allowed_op,
     *  target,
     *  get_callback, set_callback
     */
    {
        /* aPhyAdminState */
        { 0x07, 0x0025 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_aPhyAdminState_get, NULL
    },
    {
        /* aAutoNegAdminState */
        { 0x07, 0x004F },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_aAutoNegAdminState_get, NULL
    },
    {
        /* aAutoNegLocalTechnologyAbility */
        { 0x07, 0x0052 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_aAutoNegLocalTechnologyAbility_get, NULL
    },
    {
        /* aAutoNegAdvertisedTechnologyAbility */
        { 0x07, 0x0053 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_aAutoNegAdvertisedTechnologyAbility_get, NULL
    },
    {
        /* aFECAbility */
        { 0x07, 0x0139 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_aFECAbility_get, NULL
    },
    {
        /* aFECmode */
        { 0x07, 0x013A },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_aFecMode_get, ctc_oam_varCb_aFecMode_set
    },
    {
        /* End indicator - must be the last one */
        { 0x00, 0x0000 },
        0,
        0,        
        NULL, NULL
    }
};
    
static ctc_varCb_t ctc_stdActCb[] = {
    /*  The leaf value should be sorted
     *  { varBranch, varLeaf },
     *  allowed_op,
     *  target,
     *  get_callback, set_callback
     */
    {
        /* acPhyAdminControl */
        { 0x09, 0x0005 },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_acPhyAdminControl_set
    },
    {
        /* acAutoNegRestartAutoConfig */
        { 0x09, 0x000B },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_acAutoNegAdminControl_set
    },
    {
        /* acAutoNegAdminControl */
        { 0x09, 0x000C },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_acAutoNegAdminControl_set
    },
    {
        /* End indicator - must be the last one */
        { 0x00, 0x0000 },
        0,
        0,        
        NULL, NULL
    }
};

static ctc_varCb_t ctc_extAttrCb[] = {
    /*  The leaf value should be sorted
     *  { varBranch, varLeaf },
     *  allowed_op,
     *  target,
     *  get_callback, set_callback
     */
    {
        /* ONU SN */
        { 0xC7, 0x0001 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuSn_get, NULL
    },
    {
        /* FirmwareVer */
        { 0xC7, 0x0002 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_firmwareVer_get, NULL
    },
    {
        /* Chipset ID */
        { 0xC7, 0x0003 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_chipsetId_get, NULL
    },
    {
        /* ONU Capabilities-1 */
        { 0xC7, 0x0004 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuCapabilities1_get, NULL
    },
    {
        /* OpticalTransceiverDiagnosis */
        { 0xC7, 0x0005 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_opticalTransceiverDiagnosis_get, NULL
    },
    {
        /* Sevice SLA */
        { 0xC7, 0x0006 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_serviceSla_get, ctc_oam_varCb_serviceSla_set
    },
    {
        /* ONU Capabilities-2 */
        { 0xC7, 0x0007 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuCapabilities2_get, NULL
    },
    {
        /* HoldoverConfig */
        { 0xC7, 0x0008 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_holdoverConfig_get, ctc_oam_varCb_holdoverConfig_set
    },
    {
        /* MxUMngGlobalParameter */
        { 0xC7, 0x0009 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_mxUMngGlobalParameter_get, ctc_oam_varCb_mxUMngGlobalParameter_set
    },
    {
        /* MxUMngSNMPParameter */
        { 0xC7, 0x000A },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_mxUMngSNMPParameter_get, ctc_oam_varCb_mxUMngSNMPParameter_set
    },
    {
        /* Active PON_IFAdminstate */
        { 0xC7, 0x000B },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_activePonIfAdminstate_get, ctc_oam_varCb_activePonIfAdminstate_set
    },
    {
        /* ONU Capabilities-3 */
        { 0xC7, 0x000C },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuCapabilities3_get, NULL
    },
    {
        /* ONU power saving capabilities */
        { 0xC7, 0x000D },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuPowerSavingCapabilities_get, NULL
    },
    {
        /* ONU power saving config */
        { 0xC7, 0x000E },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuPowerSavingConfig_get, ctc_oam_varCb_onuPowerSavingConfig_set
    },
    {
        /* ONU Protection Parameters */
        { 0xC7, 0x000F },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        ctc_oam_varCb_onuProtectionParameters_get, ctc_oam_varCb_onuProtectionParameters_set
    },
    {
        /* ethPort State */
        { 0xC7, 0x0011 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_ethLinkState_get, NULL
    },
    {
        /* ethPort Pause */
        { 0xC7, 0x0012 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_ethPortPause_get, ctc_oam_varCb_ethPortPause_set
    },
    {
        /* ethPortUs Policing */
        { 0xC7, 0x0013 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_ethPortUsPolicing_set
    },
    {
        /* EthPortDs RateLimiting */
        { 0xC7, 0x0016 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, NULL
    },
    {
        /* PortLoopDetect */
        { 0xC7, 0x0017 },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_portLoopDetect_set
    },
    {
        /* Port MAC Aging Time */
        { 0xC7, 0x0019 },
        CTC_VAR_OP_SET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        ctc_oam_varCb_portMacAgingTime_get, ctc_oam_varCb_portMacAgingTime_set
    },
    {
        /* VLAN */
        { 0xC7, 0x0021 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_vlan_set
    },
    {
        /* Classification&Marking */
        { 0xC7, 0x0031 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_UNIPORT,
        NULL, ctc_oam_varCb_calssificationMarking_set
    },
    {
        /* Add/Del Multicast VLAN */
        { 0xC7, 0x0041 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_MULTICAST,
        NULL, ctc_oam_varCb_addDelMulticastVlan_set
    },
    {
        /* MulticastTagOper */
        { 0xC7, 0x0042 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_MULTICAST,
        NULL, ctc_oam_varCb_multicastTagOper_set
    },
    {
        /* MulticastSwitch */
        { 0xC7, 0x0043 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_MULTICAST,
        NULL, ctc_oam_varCb_multicastSwitch_set
    },
    {
        /* Group Num Max */
        { 0xC7, 0x0045 },
        CTC_VAR_OP_GET | CTC_VAR_OP_SET,
        CTC_VAR_TARGET_MULTICAST,
        NULL, ctc_oam_varCb_groupNumMax_set
    },
    {
        /* aFastLeaveAbility */
        { 0xC7, 0x0046 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_MULTICAST,
        ctc_oam_varCb_aFastLeaveAbility_get, NULL
    },
    {
        /* aFastLeaveAdminState */
        { 0xC7, 0x0047 },
        CTC_VAR_OP_GET,
        CTC_VAR_TARGET_MULTICAST,
        ctc_oam_varCb_aFastLeaveAdminState_get, NULL
    },
    {
        /* Propritey OAM? */
        { 0xC7, 0x0081 },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        NULL, ctc_oam_varCb_dummy_set
    },
    {
        /* End indicator - must be the last one */
        { 0x00, 0x0000 },
        0,
        0,        
        NULL, NULL
    }
};

static ctc_varCb_t ctc_extActCb[] = {
    /*  The leaf value should be sorted
     *  { varBranch, varLeaf },
     *  allowed_op,
     *  target,
     *  get_callback, set_callback
     */
    {
        /* Reset ONU */
        { 0xC9, 0x0001 },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        NULL, ctc_oam_varCb_resetOnu_set
    },
    {
        /* acFastLeaveAdminControl */
        { 0xC9, 0x0048 },
        CTC_VAR_OP_SET,
        CTC_VAR_TARGET_ONU,
        NULL, ctc_oam_varCb_acFastLeaveAdminControl_set
    },
    {
        /* End indicator - must be the last one */
        { 0x00, 0x0000 },
        0,
        0,        
        NULL, NULL
    }
};

