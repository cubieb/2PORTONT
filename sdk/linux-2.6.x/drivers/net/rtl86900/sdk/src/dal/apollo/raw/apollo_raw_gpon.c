/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : GPON MAC register access APIs
 *
 * Feature : Provide the APIs to access GPON MAC
 *
 */

#include <hal/mac/reg.h>
#include <hal/mac/mac_probe.h>
#include <common/rt_error.h>
#include <dal/apollo/gpon/gpon_defs.h>
#include <dal/apollo/raw/apollo_raw.h>
#include <dal/apollo/raw/apollo_raw_gpon.h>

#define GPON_NO_CHECK  0

#define GPON_REG_16BITS_LOW                     0
#define GPON_REG_16BITS_HIGH                    8

#define GPON_REG_32BITS_0                       0
#define GPON_REG_32BITS_1                       8
#define GPON_REG_32BITS_2                       16
#define GPON_REG_32BITS_3                       24

#define GPON_REG_STATUS_CLEAR                   0
#define GPON_REG_STATUS_SET                     1

#define GPON_REG_OPERRATION_NO                  0
#define GPON_REG_OPERRATION_WRITE               1
#define GPON_REG_OPERRATION_READ                2
#define GPON_REG_OPERRATION_CLEAN               3

#define GPON_REG_BITS_CAM_OP_IDX                0
#define GPON_REG_BITS_CAM_OP_MODE               8
#define GPON_REG_BITS_CAM_OP_HIT                13
#define GPON_REG_BITS_CAM_OP_COMPLETE           14
#define GPON_REG_BITS_CAM_OP_REQ                15

#define GPON_REG_BITS_INDIRECT_ACK              15

#define GPON_REG_EQD_FRAME_LEN                  (19440*8)

#define GPON_REG_PTN_GTC_US_WRPROTECT_ENA       0xCC19
#define GPON_REG_PTN_GTC_US_WRPROTECT_DIS       0x0000

#define GPON_REG_TYPE_GTC_US_PLOAM_NRM          0x0
#define GPON_REG_TYPE_GTC_US_PLOAM_URG          0x1
#define GPON_REG_TYPE_GTC_US_PLOAM_DYINGGASP    0x5
#define GPON_REG_TYPE_GTC_US_PLOAM_SN           0x6
#define GPON_REG_TYPE_GTC_US_PLOAM_NOMSG        0x7



#define WAIT_ZERO                               4096

#define MP_REG_OFFSET                          (GPON_INT_DLTr - APOLLOMP_GPON_INT_DLTr)

typedef struct gpon_field_wrap_s{
    uint32 apollo_filed;
    uint32 apollomp_filed;
}gpon_field_wrap_t;

static gpon_field_wrap_t gpon_wrap_field[] = {
    {GPON_IRQf,                   APOLLOMP_GPON_IRQf},
    {RST_DONEf,                   APOLLOMP_RST_DONEf},
    {SOFT_RSTf,                   APOLLOMP_SOFT_RSTf},
    {VER_IDf,                     APOLLOMP_VER_IDf},
    {TEST_REGf,                   APOLLOMP_TEST_REGf},
    {M_BYPASS_AES_MODf,           APOLLOMP_M_BYPASS_AES_MODf},
    {GEM_US_Mf,                   APOLLOMP_GEM_US_Mf},
    {GTC_US_Mf,                   APOLLOMP_GTC_US_Mf},
    {GEM_DS_Mf,                   APOLLOMP_GEM_DS_Mf},
    {AES_DECRYPT_Mf,              APOLLOMP_AES_DECRYPT_Mf},
    {GTC_DS_CAP_Mf,               APOLLOMP_GTC_DS_CAP_Mf},
    {GTC_DS_Mf,                   APOLLOMP_GTC_DS_Mf},
    {GEM_US_INTRf,                APOLLOMP_GEM_US_INTRf},
    {GTC_US_INTRf,                APOLLOMP_GTC_US_INTRf},
    {GEM_DS_INTRf,                APOLLOMP_GEM_DS_INTRf},
    {AES_DECRYPT_INTRf,           APOLLOMP_AES_DECRYPT_INTRf},
    {GTC_DS_CAP_INTRf,            APOLLOMP_GTC_DS_CAP_INTRf},
    {GTC_DS_INTRf,                APOLLOMP_GTC_DS_INTRf},
    {PLM_BUF_REQf,                APOLLOMP_PLM_BUF_REQf},
    {RNG_REQ_HISf,                APOLLOMP_RNG_REQ_HISf},
    {SN_REQ_HISf,                 APOLLOMP_SN_REQ_HISf},
    {LOM_DLTf,                    APOLLOMP_LOM_DLTf},
    {DS_FEC_STA_DLTf,             APOLLOMP_DS_FEC_STA_DLTf},
    {LOF_DLTf,                    APOLLOMP_LOF_DLTf},
    {LOS_DLTf,                    APOLLOMP_LOS_DLTf},
    {PLM_BUF_Mf,                  APOLLOMP_PLM_BUF_Mf},
    {RNG_REQ_Mf,                  APOLLOMP_RNG_REQ_Mf},
    {SN_REQ_Mf,                   APOLLOMP_SN_REQ_Mf},
    {LOM_Mf,                      APOLLOMP_LOM_Mf},
    {DS_FEC_STA_Mf,               APOLLOMP_DS_FEC_STA_Mf},
    {LOF_Mf,                      APOLLOMP_LOF_Mf},
    {LOS_Mf,                      APOLLOMP_LOS_Mf},
    {LOMf,                        APOLLOMP_LOMf},
    {DS_FEC_STSf,                 APOLLOMP_DS_FEC_STSf},
    {LOFf,                        APOLLOMP_LOFf},
    {LOSf,                        APOLLOMP_LOSf},
    {ONU_IDf,                     APOLLOMP_ONU_IDf},
    {ONU_STATEf,                  APOLLOMP_ONU_STATEf},
    {BWM_FILT_ONUIDf,             APOLLOMP_BWM_FILT_ONUIDf},
    {CHK_BWM_CRCf,                APOLLOMP_CHK_BWM_CRCf},
    {PLEND_STRICT_MODEf,          APOLLOMP_PLEND_STRICT_MODEf},
    {EXTRA_SN_TXf,                APOLLOMP_EXTRA_SN_TXf},
    {FEC_CORRECT_DISf,            APOLLOMP_FEC_CORRECT_DISf},
    {FEC_DET_THRSHf,              APOLLOMP_FEC_DET_THRSHf},
    {DESCRAM_DISf,                APOLLOMP_DESCRAM_DISf},
    {PLM_DROP_CRCEf,              APOLLOMP_PLM_DROP_CRCEf},
    {PLM_BC_ACC_ENf,              APOLLOMP_PLM_BC_ACC_ENf},
    {PLM_DS_ONUID_FLT_ENf,        APOLLOMP_PLM_DS_ONUID_FLT_ENf},
    {PLM_DS_NOMSG_IDf,            APOLLOMP_PLM_DS_NOMSG_IDf},
    {CDR_LOS_SIGf,                APOLLOMP_CDR_LOS_SIGf},
    {OPTIC_LOS_SIGf,              APOLLOMP_OPTIC_LOS_SIGf},
    {LOS_FILTER_ENf,              APOLLOMP_LOS_FILTER_ENf},
    {CDR_LOS_POLARf,              APOLLOMP_CDR_LOS_POLARf},
    {CDR_LOS_ENf,                 APOLLOMP_CDR_LOS_ENf},
    {OPTIC_LOS_POLARf,            APOLLOMP_OPTIC_LOS_POLARf},
    {OPTIC_LOS_ENf,               APOLLOMP_OPTIC_LOS_ENf},
    {SF_CNTRf,                    APOLLOMP_SF_CNTRf},
    {PLM_BUF_EMPTYf,              APOLLOMP_PLM_BUF_EMPTYf},
    {PLM_BUF_FULLf,               APOLLOMP_PLM_BUF_FULLf},
    {PLM_DEQf,                    APOLLOMP_PLM_DEQf},
    {PLOAM_RDATAf,                APOLLOMP_PLOAM_RDATAf},
    {ALLOCID_OP_REQf,             APOLLOMP_ALLOCID_OP_REQf},
    {ALLOCID_OP_COMPLf,           APOLLOMP_ALLOCID_OP_COMPLf},
    {ALLOCID_OP_HITf,             APOLLOMP_ALLOCID_OP_HITf},
    {ALLOCID_OP_MODEf,            APOLLOMP_ALLOCID_OP_MODEf},
    {ALLOCID_OP_IDXf,             APOLLOMP_ALLOCID_OP_IDXf},
    {ALLOCID_OP_WDATAf,           APOLLOMP_ALLOCID_OP_WDATAf},
    {ALLOCID_OP_RDATAf,           APOLLOMP_ALLOCID_OP_RDATAf},
    {PORTID_OP_REQf,              APOLLOMP_PORTID_OP_REQf},
    {PORTID_OP_COMPLf,            APOLLOMP_PORTID_OP_COMPLf},
    {PORTID_OP_HITf,              APOLLOMP_PORTID_OP_HITf},
    {PORTID_OP_MODEf,             APOLLOMP_PORTID_OP_MODEf},
    {PORTID_OP_IDXf,              APOLLOMP_PORTID_OP_IDXf},
    {PORTID_OP_WDATAf,            APOLLOMP_PORTID_OP_WDATAf},
    {PORTID_OP_RDATAf,            APOLLOMP_PORTID_OP_RDATAf},
    {GEM_CNTR_R_ACKf,             APOLLOMP_GEM_CNTR_R_ACKf},
    {GEM_CNTR_RSELf,              APOLLOMP_GEM_CNTR_RSELf},
    {GEM_CNTR_IDXf,               APOLLOMP_GEM_CNTR_IDXf},
    {GEM_CNTRf,                   APOLLOMP_GEM_CNTRf},
    {CNTR_DS_BIP_ERR_BLOCKf,      APOLLOMP_CNTR_DS_BIP_ERR_BLOCKf},
    {CNTR_DS_BIP_ERR_BITSf,       APOLLOMP_CNTR_DS_BIP_ERR_BITSf},
    {CNTR_FEC_CORRECTED_BITSf,    APOLLOMP_CNTR_FEC_CORRECTED_BITSf},
    {CNTR_FEC_CORRECTED_BYTESf,   APOLLOMP_CNTR_FEC_CORRECTED_BYTESf},
    {CNTR_FEC_CORRETED_CWf,       APOLLOMP_CNTR_FEC_CORRETED_CWf},
    {CNTR_FEC_UNCORRETABLE_CWf,   APOLLOMP_CNTR_FEC_UNCORRETABLE_CWf},
    {CNTR_PLEND_FAILf,            APOLLOMP_CNTR_PLEND_FAILf},
    {CNTR_SUPERFRAME_LOSf,        APOLLOMP_CNTR_SUPERFRAME_LOSf},
    {CNTR_PLOAMD_ACCEPTEDf,       APOLLOMP_CNTR_PLOAMD_ACCEPTEDf},
    {CNTR_PLOAMD_OVERFLOWf,       APOLLOMP_CNTR_PLOAMD_OVERFLOWf},
    {CNTR_PLOAMD_CRC_ERRf,        APOLLOMP_CNTR_PLOAMD_CRC_ERRf},
    {CNTR_BWMAP_OVERFLOWf,        APOLLOMP_CNTR_BWMAP_OVERFLOWf},
    {CNTR_BWMAP_CRC_ERRf,         APOLLOMP_CNTR_BWMAP_CRC_ERRf},
    {CNTR_BWMAP_INV1f,            APOLLOMP_CNTR_BWMAP_INV1f},
    {CNTR_BWMAP_INV0f,            APOLLOMP_CNTR_BWMAP_INV0f},
    {CNTR_RANGING_REQf,           APOLLOMP_CNTR_RANGING_REQf},
    {CNTR_SN_REQf,                APOLLOMP_CNTR_SN_REQf},
    {CNTR_BWMAP_ACCPTEDf,         APOLLOMP_CNTR_BWMAP_ACCPTEDf},
    {CNTR_GEM_LOSf,               APOLLOMP_CNTR_GEM_LOSf},
    {CNTR_HEC_CORRECTEDf,         APOLLOMP_CNTR_HEC_CORRECTEDf},
    {CNTR_GEM_IDLEf,              APOLLOMP_CNTR_GEM_IDLEf},
    {CNTR_PORTID_MMATCHf,         APOLLOMP_CNTR_PORTID_MMATCHf},
    {CNTR_GEM_LEN_MISMf,          APOLLOMP_CNTR_GEM_LEN_MISMf},
    {CNTR_RX_GEM_NON_IDLEf,       APOLLOMP_CNTR_RX_GEM_NON_IDLEf},
    {CNTR_PLEND_CORRECTIONSf,     APOLLOMP_CNTR_PLEND_CORRECTIONSf},
    {RESERVEDf,                   APOLLOMP_RESERVEDf},
    {OMCI_PTI_MASKf,              APOLLOMP_OMCI_PTI_MASKf},
    {OMCI_END_PTIf,               APOLLOMP_OMCI_END_PTIf},
    {ETH_PTI_MASKf,               APOLLOMP_ETH_PTI_MASKf},
    {ETH_END_PTIf,                APOLLOMP_ETH_END_PTIf},
    {TRAFFIC_TYPE_CFGf,           APOLLOMP_TRAFFIC_TYPE_CFGf},
    {AES_DECRYPT_INTRf,           APOLLOMP_AES_DECRYPT_INTRf},
    {INFO_FIFO_OVERFL_DLTf,       APOLLOMP_INFO_FIFO_OVERFL_DLTf},
    {DATA_FIFO_OVERFL_DLTf,       APOLLOMP_DATA_FIFO_OVERFL_DLTf},
    {INFO_FIFO_OVERFL_Mf,         APOLLOMP_INFO_FIFO_OVERFL_Mf},
    {DATA_FIFO_OVERFL_Mf,         APOLLOMP_DATA_FIFO_OVERFL_Mf},
    {INFO_FIFO_OVERFLf,           APOLLOMP_INFO_FIFO_OVERFLf},
    {DATA_FIFO_OVERFLf,           APOLLOMP_DATA_FIFO_OVERFLf},
    {KEY_CFG_REQf,                APOLLOMP_KEY_CFG_REQf},
    {CFG_ACTIVE_KEYf,             APOLLOMP_CFG_ACTIVE_KEYf},
    {SWITCH_SUPERFRAMEf,          APOLLOMP_SWITCH_SUPERFRAMEf},
    {KEY_WR_REQf,                 APOLLOMP_KEY_WR_REQf},
    {KEY_WR_COMPLf,               APOLLOMP_KEY_WR_COMPLf},
    {KEY_USE_INDf,                APOLLOMP_KEY_USE_INDf},
    {KEY_WORD_IDXf,               APOLLOMP_KEY_WORD_IDXf},
    {KEY_DATAf,                   APOLLOMP_KEY_DATAf},
    {ETH_PKT_RX_R_ACKf,           APOLLOMP_ETH_PKT_RX_R_ACKf},
    {ETH_PKT_RX_IDXf,             APOLLOMP_ETH_PKT_RX_IDXf},
    {ETH_PKT_RXf,                 APOLLOMP_ETH_PKT_RXf},
    {ETH_PKT_FWD_R_ACKf,          APOLLOMP_ETH_PKT_FWD_R_ACKf},
    {ETH_PKT_FWD_IDXf,            APOLLOMP_ETH_PKT_FWD_IDXf},
    {ETH_PKT_FWDf,                APOLLOMP_ETH_PKT_FWDf},
    {MISC_CNTR_IDXf,              APOLLOMP_MISC_CNTR_IDXf},
    {MISC_CNTRf,                  APOLLOMP_MISC_CNTRf},
    {IPV6_MC_FORCE_PASSf,         APOLLOMP_IPV6_MC_FORCE_PASSf},
    {IPV6_MC_FORCE_DROPf,         APOLLOMP_IPV6_MC_FORCE_DROPf},
    {BROADCAST_PASSf,             APOLLOMP_BROADCAST_PASSf},
    {NON_MULTICAST_PASSf,         APOLLOMP_NON_MULTICAST_PASSf},
    {FCS_CHK_ENf,                 APOLLOMP_FCS_CHK_ENf},
    {IPV4_MC_FORCE_PASSf,         APOLLOMP_IPV4_MC_FORCE_PASSf},
    {IPV4_MC_FORCE_DROPf,         APOLLOMP_IPV4_MC_FORCE_DROPf},
    {MC_EXCL_MODEf,               APOLLOMP_MC_EXCL_MODEf},
    {MC_ITEM_OP_REQf,             APOLLOMP_MC_ITEM_OP_REQf},
    {MC_ITEM_OP_COMPLf,           APOLLOMP_MC_ITEM_OP_COMPLf},
    {MC_ITEM_OP_HITf,             APOLLOMP_MC_ITEM_OP_HITf},
    {MC_ITEM_OP_MODEf,            APOLLOMP_MC_ITEM_OP_MODEf},
    {MC_ITEM_OP_IDXf,             APOLLOMP_MC_ITEM_OP_IDXf},
    {MC_ITEM_OP_WDATAf,           APOLLOMP_MC_ITEM_OP_WDATAf},
    {MC_ITEM_OP_RDATAf,           APOLLOMP_MC_ITEM_OP_RDATAf},
    {OMCI_TR_MODEf,               APOLLOMP_OMCI_TR_MODEf},
    {ASSM_TIMEOUT_FRMf,           APOLLOMP_ASSM_TIMEOUT_FRMf},
    {IPV4_MC_MAC_PREFIXf,         APOLLOMP_IPV4_MC_MAC_PREFIXf},
    {IPV6_MC_MAC_PREFIXf,         APOLLOMP_IPV6_MC_MAC_PREFIXf},
    {GTC_US_INTRf,                APOLLOMP_GTC_US_INTRf},
    {OPTIC_SD_MISM_DLTf,          APOLLOMP_OPTIC_SD_MISM_DLTf},
    {OPTIC_SD_TOOLONG_DLTf,       APOLLOMP_OPTIC_SD_TOOLONG_DLTf},
    {PLM_NRM_EMPTY_DLTf,          APOLLOMP_PLM_NRM_EMPTY_DLTf},
    {PLM_URG_EMPTY_DLTf,          APOLLOMP_PLM_URG_EMPTY_DLTf},
    {US_FEC_STS_DLTf,             APOLLOMP_US_FEC_STS_DLTf},
    {DG_MSG_TX_DLTf,              APOLLOMP_DG_MSG_TX_DLTf},
    {OPTIC_SD_MISM_Mf,            APOLLOMP_OPTIC_SD_MISM_Mf},
    {OPTIC_SD_TOOLONG_Mf,         APOLLOMP_OPTIC_SD_TOOLONG_Mf},
    {PLM_NRM_EMPTY_Mf,            APOLLOMP_PLM_NRM_EMPTY_Mf},
    {PLM_URG_EMPTY_Mf,            APOLLOMP_PLM_URG_EMPTY_Mf},
    {US_FEC_STS_Mf,               APOLLOMP_US_FEC_STS_Mf},
    {DG_MSG_TX_Mf,                APOLLOMP_DG_MSG_TX_Mf},
    {US_FEC_STSf,                 APOLLOMP_US_FEC_STSf},
    {ONU_IDf,                     APOLLOMP_ONU_IDf},
    {FS_LONf,                     APOLLOMP_FS_LONf},
    {FS_LOFFf,                    APOLLOMP_FS_LOFFf},
    {IND_NRM_PLMf,                APOLLOMP_IND_NRM_PLMf},
    {PLM_DISf,                    APOLLOMP_PLM_DISf},
    {DBRU_DISf,                   APOLLOMP_DBRU_DISf},
    {ENA_AUTO_DGf,                APOLLOMP_ENA_AUTO_DGf},
    {US_BEN_POLARf,               APOLLOMP_US_BEN_POLARf},
    {SCRM_DISf,                   APOLLOMP_SCRM_DISf},
    {RSV_REG_WRITE_PROTECTIONf,   APOLLOMP_RSV_REG_WRITE_PROTECTIONf},
    {TX_PATTERN_MODE_NO_FGf,      APOLLOMP_TX_PATTERN_MODE_NO_FGf},
    {TX_PATTERN_MODE_BGf,         APOLLOMP_TX_PATTERN_MODE_BGf},
    {TX_PATTERN_MODE_FGf,         APOLLOMP_TX_PATTERN_MODE_FGf},
    {TX_PATTERN_BGf,              APOLLOMP_TX_PATTERN_BGf},
    {TX_PATTERN_FGf,              APOLLOMP_TX_PATTERN_FGf},
    {MIN_DELAY1f,                 APOLLOMP_MIN_DELAY1f},
    {MIN_DELAY2f,                 APOLLOMP_MIN_DELAY2f},
    {EQD1_MULTFRAMEf,             APOLLOMP_EQD1_MULTFRAMEf},
    {EQD1_INFRAMEf,               APOLLOMP_EQD1_INFRAMEf},
    {LON_TIMEf,                   APOLLOMP_LON_TIMEf},
    {LOFF_TIMEf,                  APOLLOMP_LOFF_TIMEf},
    {BOH_REPEATf,                 APOLLOMP_BOH_REPEATf},
    {BOH_LENGTHf,                 APOLLOMP_BOH_LENGTHf},
    {PLM_TYPEf,                   APOLLOMP_PLM_TYPEf},
    {PLM_NRM_EMPTYf,              APOLLOMP_PLM_NRM_EMPTYf},
    {PLM_NRM_FULLf,               APOLLOMP_PLM_NRM_FULLf},
    {PLM_URG_EMPTYf,              APOLLOMP_PLM_URG_EMPTYf},
    {PLM_URG_FULLf,               APOLLOMP_PLM_URG_FULLf},
    {PLM_ENQf,                    APOLLOMP_PLM_ENQf},
    {PLM_DATAf,                   APOLLOMP_PLM_DATAf},
    {PLM_FLUSH_BUFf,              APOLLOMP_PLM_FLUSH_BUFf},
    {PLM_US_CRC_GEN_ENf,          APOLLOMP_PLM_US_CRC_GEN_ENf},
    {PLM_US_ONUID_OVRD_ENf,       APOLLOMP_PLM_US_ONUID_OVRD_ENf},
    {MISC_IDXf,                   APOLLOMP_MISC_IDXf},
    {MISC_CNTRf,                  APOLLOMP_MISC_CNTRf},
    {ONU_RDIf,                    APOLLOMP_ONU_RDIf},
    {DG_STATUSf,                  APOLLOMP_DG_STATUSf},
    {DG_MSG_TX_CNTf,              APOLLOMP_DG_MSG_TX_CNTf},
    {DG_MSG_TX_CNT_THRESHOLDf,    APOLLOMP_DG_MSG_TX_CNT_THRESHOLDf},
    {OPTIC_SD_MISM_THREHf,        APOLLOMP_OPTIC_SD_MISM_THREHf},
    {OPTIC_SD_TOOLONG_THRESHf,    APOLLOMP_OPTIC_SD_TOOLONG_THRESHf},
    {OPTIC_AUTO_SUPRESS_DISf,     APOLLOMP_OPTIC_AUTO_SUPRESS_DISf},
    {AUTO_PROC_SSTARTf,           APOLLOMP_AUTO_PROC_SSTARTf},
    {GEM_US_INTRf,                APOLLOMP_GEM_US_INTRf},
    {SD_VALID_LONG_DLTf,          APOLLOMP_SD_VALID_LONG_DLTf},
    {SD_DIFF_HUGE_DLTf,           APOLLOMP_SD_DIFF_HUGE_DLTf},
    {REQUEST_DELAY_DLTf,          APOLLOMP_REQUEST_DELAY_DLTf},
    {BC_LESS6_DLTf,               APOLLOMP_BC_LESS6_DLTf},
    {ERR_PLI_DLTf,                APOLLOMP_ERR_PLI_DLTf},
    {BURST_TM_LARGER_GTC_DLTf,    APOLLOMP_BURST_TM_LARGER_GTC_DLTf},
    {BANK_TOO_MUCH_AT_END_DLTf,   APOLLOMP_BANK_TOO_MUCH_AT_END_DLTf},
    {BANK_REMAIN_AFRD_DLTf,       APOLLOMP_BANK_REMAIN_AFRD_DLTf},
    {BANK_OVERFL_DLTf,            APOLLOMP_BANK_OVERFL_DLTf},
    {BANK_UNDERFL_DLTf,           APOLLOMP_BANK_UNDERFL_DLTf},
    {SD_VALID_LONG_Mf,            APOLLOMP_SD_VALID_LONG_Mf},
    {SD_DIFF_HUGE_Mf,             APOLLOMP_SD_DIFF_HUGE_Mf},
    {REQUEST_DELAY_Mf,            APOLLOMP_REQUEST_DELAY_Mf},
    {BC_LESS6_Mf,                 APOLLOMP_BC_LESS6_Mf},
    {ERR_PLI_Mf,                  APOLLOMP_ERR_PLI_Mf},
    {BURST_TM_LARGER_GTC_Mf,      APOLLOMP_BURST_TM_LARGER_GTC_Mf},
    {BANK_TOO_MUCH_AT_END_Mf,     APOLLOMP_BANK_TOO_MUCH_AT_END_Mf},
    {BANK_REMAIN_AFRD_Mf,         APOLLOMP_BANK_REMAIN_AFRD_Mf},
    {BANK_OVERFL_Mf,              APOLLOMP_BANK_OVERFL_Mf},
    {BANK_UNDERFL_Mf,             APOLLOMP_BANK_UNDERFL_Mf},
    {SD_VALID_LONG_INDf,          APOLLOMP_SD_VALID_LONG_INDf},
    {SD_DIFF_HUGE_INDf,           APOLLOMP_SD_DIFF_HUGE_INDf},
    {REQUEST_DELAY_INDf,          APOLLOMP_REQUEST_DELAY_INDf},
    {BC_LESS6_INDf,               APOLLOMP_BC_LESS6_INDf},
    {ERR_PLI_INDf,                APOLLOMP_ERR_PLI_INDf},
    {BURST_TM_LARGER_GTC_INDf,    APOLLOMP_BURST_TM_LARGER_GTC_INDf},
    {BANK_TOO_INDUCH_AT_END_INDf, APOLLOMP_BANK_TOO_INDUCH_AT_END_INDf},
    {BANK_REMAIN_AFRD_INDf,       APOLLOMP_BANK_REMAIN_AFRD_INDf},
    {BANK_OVERFL_INDf,            APOLLOMP_BANK_OVERFL_INDf},
    {BANK_UNDERFL_INDf,           APOLLOMP_BANK_UNDERFL_INDf},
    {FS_GEM_IDLEf,                APOLLOMP_FS_GEM_IDLEf},
    {PTI_VECTOR3f,                APOLLOMP_PTI_VECTOR3f},
    {PTI_VECTOR2f,                APOLLOMP_PTI_VECTOR2f},
    {PTI_VECTOR1f,                APOLLOMP_PTI_VECTOR1f},
    {PTI_VECTOR0f,                APOLLOMP_PTI_VECTOR0f},
    {ETH_GEM_RX_R_ACKf,           APOLLOMP_ETH_GEM_RX_R_ACKf},
    {ETH_GEM_RX_IDXf,             APOLLOMP_ETH_GEM_RX_IDXf},
    {ETH_GEM_RX_CNTRf,            APOLLOMP_ETH_GEM_RX_CNTRf},
    {DEBUG_BUS_SELf,              APOLLOMP_DEBUG_BUS_SELf},
    {GEM_PTN_MODEf,               APOLLOMP_GEM_PTN_MODEf},
    {GEM_PTN_BYTEf,               APOLLOMP_GEM_PTN_BYTEf},
    {PORT_CFG_DATAf,              APOLLOMP_PORT_CFG_DATAf},
    {CNTR_LOW32f,                 APOLLOMP_CNTR_LOW32f},
    {CNTR_HIGH32f,                APOLLOMP_CNTR_HIGH32f},
    {CNTR_LOW32f,                 APOLLOMP_CNTR_LOW32f},
    {CNTR_HIGH32f,                APOLLOMP_CNTR_HIGH32f},
};

#define TOTAL_FIELD_NO (sizeof(gpon_wrap_field)/sizeof(gpon_field_wrap_t))

static uint32  _mp_field(uint32 field)
{
    uint32 i;

    for(i = 0; i < TOTAL_FIELD_NO; i ++)
        if(field == gpon_wrap_field[i].apollo_filed)
            return gpon_wrap_field[i].apollomp_filed;

    return 0;
}

static uint32 isGetchipId = 0;
static uint32 keepChipId = 0xFFFFFFFF;
static uint32 _apollo_wrapper_chip_id_get(void)
{
    uint32 chipId;
    uint32 ChipRevId;
    uint32 ret;
    if(0 == isGetchipId)
    {
    	isGetchipId = 1;
    	if (( ret = drv_swcore_cid_get(&chipId,&ChipRevId)) != RT_ERR_OK )
        {
            keepChipId = 0xFFFFFFFF;
            return 0xFFFFFFFF;
        }
        keepChipId = chipId;
        return chipId;
    }
    else
    {
        return keepChipId;
    }
}

static int32 _apollo_wrap_reg(uint32 reg, uint32 *pWrapReg)
{
    switch(_apollo_wrapper_chip_id_get())
    {
        case APOLLO_CHIP_ID:
            *pWrapReg = reg;
            break;

        case APOLLOMP_CHIP_ID:
            *pWrapReg = reg - MP_REG_OFFSET;
            break;

        default:
            return RT_ERR_CHIP_NOT_FOUND;
            break;
    }
    //osal_printf("\n\r reg %d wrap to reg %d", reg, *pWrapReg);
    return RT_ERR_OK;

}

static int32 _apollo_wrap_field(uint32 field, uint32 *pWrapField)
{
    switch(_apollo_wrapper_chip_id_get())
    {
        case APOLLO_CHIP_ID:
            *pWrapField = field;
            break;

        case APOLLOMP_CHIP_ID:
            *pWrapField = _mp_field(field);
            break;

        default:
            return RT_ERR_CHIP_NOT_FOUND;
            break;
    }
    //osal_printf("\n\r field %d wrap to field %d", field, *pWrapField);
    return RT_ERR_OK;

}


static int32 _apollo_wrap_reg_read(uint32 reg, uint32 *pValue)
{
    uint32 reg_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_read: _apollo_wrap_reg Failed!!");
        return ret;
    }
    return reg_read(reg_wrap, pValue);
}

static int32 _apollo_wrap_reg_write(uint32 reg, uint32 *pValue)
{
    uint32 reg_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_write: _apollo_wrap_reg Failed!!");
        return ret;
    }
    return reg_write(reg_wrap, pValue);
}

static int32 _apollo_wrap_reg_field_read(uint32 reg, uint32 field, uint32 *pValue)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_read: _apollo_wrap_reg Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_read: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_field_read(reg_wrap, field_wrap, pValue);
}

static int32 _apollo_wrap_reg_field_write(uint32 reg, uint32 field, uint32 *pValue)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_write: _apollo_wrap_reg Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_write: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_field_write(reg_wrap, field_wrap, pValue);
}

static int32 _apollo_wrap_reg_field_get(uint32 reg, uint32 field, uint32 *pValue, uint32 *pData)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_get: _apollo_wrap_reg Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_get: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_field_get(reg_wrap, field_wrap, pValue, pData);
}

static int32 _apollo_wrap_reg_field_set(uint32 reg, uint32 field, uint32 *pValue, uint32 *pData)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_set: _apollo_wrap_reg Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_field_set: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_field_set(reg_wrap, field_wrap, pValue, pData);
}

static int32 _apollo_wrap_reg_array_read(uint32 reg, int32 index_1, int32 index_2, uint32 *pValue)
{
    uint32 reg_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_read: _apollo_wrap_reg Failed!!");
        return ret;
    }
    return reg_array_read(reg_wrap, index_1, index_2, pValue);
}

static int32 _apollo_wrap_reg_array_write(uint32 reg, int32 index_1, int32 index_2, uint32 *pValue)
{
    uint32 reg_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_write: _apollo_wrap_reg Failed!!");
        return ret;
    }
    return reg_array_write(reg_wrap, index_1, index_2, pValue);
}

static int32 _apollo_wrap_reg_array_field_read(uint32 reg, int32 index_1, int32 index_2, uint32 field, uint32 *pValue)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_field_read: _apollo_wrap_field Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_field_read: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_array_field_read(reg_wrap, index_1, index_2, field_wrap, pValue);
}

static int32 _apollo_wrap_reg_array_field_write(uint32 reg, int32 index_1, int32 index_2, uint32 field, uint32 *pValue)
{
    uint32 reg_wrap, field_wrap;
    int32  ret;

    if((ret = _apollo_wrap_reg(reg, &reg_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_field_write: _apollo_wrap_reg Failed!!");
        return ret;
    }
    if((ret = _apollo_wrap_field(field, &field_wrap)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_GPON, "apollo_wrap_reg_array_field_write: _apollo_wrap_field Failed!!");
        return ret;
    }
    return reg_array_field_write(reg_wrap, index_1, index_2, field_wrap, pValue);
}




static void apollo_raw_gpon_delay(uint32 step)
{
    uint32 delay = step;
    while(delay)
    {
        delay--;
    }
}


static uint32 gpon_get_bit(uint32 value, uint32 bit)
{
    return ((value>>bit)&0x01);
}

#define WAIT_FOREVER                            (0xFFFFFFFF - 1)
#define WAIT_MAX_TIMES                          WAIT_FOREVER

static int32 apollo_raw_gpon_indirect_wait(uint32 reg)
{
#if GPON_NO_CHECK /*defined(CONFIG_SDK_ASICDRV_TEST)*/ /* for init pass, scott */
    apollo_raw_gpon_delay(WAIT_ZERO);
    return TRUE;
#else
    uint32 timeout;
    uint32 ind;

    for(timeout=0;timeout<=WAIT_MAX_TIMES;timeout++)
    {
        _apollo_wrap_reg_read(reg, &ind);
        if(gpon_get_bit(ind,GPON_REG_BITS_INDIRECT_ACK))
        {
            return TRUE;
        }
        osal_printf("apollo_raw_gpon_indirect_wait %d\n\r",timeout);
    }
    return FALSE;
#endif
}

static int32 apollo_raw_gpon_cam_wait(uint32 reg, int32 waithit)
{
#if GPON_NO_CHECK /*defined(CONFIG_SDK_ASICDRV_TEST)*/ /* for init pass, scott */
    apollo_raw_gpon_delay(WAIT_ZERO);
    return TRUE;
#else
    uint32 timeout;
    uint32 ind;

    for(timeout=0;timeout<=WAIT_MAX_TIMES;timeout++)
    {
        _apollo_wrap_reg_read(reg, &ind);
        if(gpon_get_bit(ind,GPON_REG_BITS_CAM_OP_COMPLETE))
        {
            if(!waithit || gpon_get_bit(ind,GPON_REG_BITS_CAM_OP_HIT))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        /*osal_printf("apollo_raw_gpon_cam_wait %d\n\r",timeout);*/
    }
      return FALSE;
#endif
}

void apollo_raw_gpon_intr_disableAll(void)
{
	uint32 data;
    uint32 tmp;

    data = GPON_REG_STATUS_CLEAR;

    /* close top int mask */
    _apollo_wrap_reg_write(GPON_INTR_MASKr,&data);

    /* clear all mask */
    _apollo_wrap_reg_write(GPON_GTC_DS_INTR_MASKr,&data);
#if 0 /* GEM DS interrupt is removed */
    _apollo_wrap_reg_write(GPON_GEM_DS_INTR_MASKr,&data);
#endif
    _apollo_wrap_reg_write(GPON_GTC_US_INTR_MASKr,&data);

    /* clear unused mask */
	data = GPON_REG_STATUS_CLEAR;
	_apollo_wrap_reg_write(GPON_AES_INTR_MASKr, &data);

    /* open top int mask */
    data=0;
    tmp = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_INTR_MASKr, GTC_DS_Mf, &tmp, &data);
    _apollo_wrap_reg_field_set(GPON_INTR_MASKr, GEM_DS_Mf, &tmp, &data);
    _apollo_wrap_reg_field_set(GPON_INTR_MASKr, GTC_US_Mf, &tmp, &data);
    _apollo_wrap_reg_write(GPON_INTR_MASKr,&data);
}

int32 apollo_raw_gpon_top_gtcDsIntr_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_INTR_STSr, GTC_DS_INTRf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_top_gemDsIntr_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_INTR_STSr, GEM_DS_INTRf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_top_gtcUsIntr_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_INTR_STSr, GTC_US_INTRf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_top_gemUsIntr_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_INTR_STSr, GEM_US_INTRf, &value, &status);
    return value;
}

void apollo_raw_gpon_reset_write(int32 reset)
{
	_apollo_wrap_reg_write(GPON_RESETr,&reset);
}

int32 apollo_raw_gpon_restDone_wait(void)
{
#if GPON_NO_CHECK /*defined(CONFIG_SDK_ASICDRV_TEST)*/ /* for init pass, scott */
    apollo_raw_gpon_delay(WAIT_ZERO);
    return TRUE;
#else
    uint32 timeout;
    uint32 ind=0;

    for(timeout=0;timeout<=WAIT_MAX_TIMES;timeout++)
    {
		_apollo_wrap_reg_field_read(GPON_RESETr,RST_DONEf,&ind);
        if(1 == ind)
        {
            return TRUE;
        }
    }
    return FALSE;
#endif
}

uint32 apollo_raw_gpon_version_read(void)
{
	uint32 data=0;
	_apollo_wrap_reg_read(GPON_VERSIONr, &data);
	return data;
}

uint32 apollo_raw_gpon_test_read(void)
{
	uint32 data=0;
	_apollo_wrap_reg_read(GPON_TESTr, &data);
	return data;
}

void apollo_raw_gpon_test_write(uint32 value)
{
	_apollo_wrap_reg_write(GPON_TESTr, &value);
}

int32 apollo_raw_gpon_top_mask_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_INTR_MASKr, pValue);
}

void apollo_raw_gpon_top_mask_write(uint32 value)
{
    _apollo_wrap_reg_write(GPON_INTR_MASKr,&value);
}

int32 apollo_raw_gpon_top_intr_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_INTR_STSr, pValue);
}

int32 apollo_raw_gpon_gtcDs_intr_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GTC_DS_INTR_DLTr, pValue);
}

int32 apollo_raw_gpon_gtcDsIntr_snReq_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, SN_REQ_HISf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_rangReq_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, RNG_REQ_HISf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_ploam_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, PLM_BUF_REQf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_los_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, LOS_DLTf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_lof_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, LOF_DLTf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_lom_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, LOM_DLTf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDsIntr_fec_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_INTR_DLTr, DS_FEC_STA_DLTf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcDs_mask_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GTC_DS_INTR_MASKr, pValue);
}

void apollo_raw_gpon_gtcDs_mask_write(uint32 value)
{
    _apollo_wrap_reg_write(GPON_GTC_DS_INTR_MASKr,&value);
}

void apollo_raw_gpon_gtcDsMask_ploam_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,PLM_BUF_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_rangReq_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,RNG_REQ_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_snReq_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,SN_REQ_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_los_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,LOS_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_lof_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,LOF_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_lom_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,LOM_Mf,&value);
}

void apollo_raw_gpon_gtcDsMask_fec_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_INTR_MASKr,DS_FEC_STA_Mf,&value);
}

int32 apollo_raw_gpon_gtcDs_los_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_INTR_STSr,LOSf,pValue);
}

int32 apollo_raw_gpon_gtcDs_lof_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_INTR_STSr,LOFf,pValue);
}

int32 apollo_raw_gpon_gtcDs_lom_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_INTR_STSr,LOMf,pValue);
}

int32 apollo_raw_gpon_gtcDs_fec_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_INTR_STSr,DS_FEC_STSf,pValue);
}

void apollo_raw_gpon_onuId_write(uint8 value)
{
    uint32 tmp = value;

	_apollo_wrap_reg_field_write(GPON_GTC_DS_ONU_ID_STATUSr,ONU_IDf,&tmp);

	_apollo_wrap_reg_field_write(GPON_GTC_US_ONU_IDr,ONU_IDf,&tmp);
}

int apollo_raw_gpon_onuId_read(uint8 *pValue)
{
    uint32 tmp;
    int32 ret;

	ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_ONU_ID_STATUSr,ONU_IDf,&tmp);

    *pValue = tmp;

    return ret;
}

void apollo_raw_gpon_onuStatus_write(uint8 value)
{
    uint32 tmp = value;

	_apollo_wrap_reg_field_write(GPON_GTC_DS_ONU_ID_STATUSr,ONU_STATEf,&tmp);
}

int apollo_raw_gpon_onuStatus_read(uint8 *pValue)
{
    uint32 tmp;
    int32 ret;

	ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_ONU_ID_STATUSr,ONU_STATEf,&tmp);

    *pValue = tmp;

    return ret;
}

void apollo_raw_gpon_dsBwmap_crcCheck_enable(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,CHK_BWM_CRCf,&value);
}

int32 apollo_raw_gpon_dsBwmap_crcCheck_enable_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,CHK_BWM_CRCf,value);
}

/* Added in GPON_MAC_SWIO_v1.1 */
void apollo_raw_gpon_dsBwmap_filterOnuId_write(int32 value)
{
    _apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,BWM_FILT_ONUIDf,&value);
}

int32 apollo_raw_gpon_dsBwmap_filterOnuId_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,BWM_FILT_ONUIDf,value);
}

void apollo_raw_gpon_dsBwmap_plend_write(int32 value)
{

    _apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,PLEND_STRICT_MODEf,&value);
}

int32 apollo_raw_gpon_dsBwmap_plend_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,PLEND_STRICT_MODEf,value);
}

void apollo_raw_gpon_dsScramble_enable(int32 value)
{
    uint32 tmp = !value;

	_apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,DESCRAM_DISf,&tmp);
}

int32 apollo_raw_gpon_dsScramble_enable_read(int32 *value)
{
    uint32 data;
    int32 ret;
    ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,DESCRAM_DISf,&data);
    *value = !data;
    return ret;
}

void apollo_raw_gpon_dsFec_bypass(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,FEC_CORRECT_DISf,&value);
}

int32 apollo_raw_gpon_dsFec_bypass_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,FEC_CORRECT_DISf,value);
}

void apollo_raw_gpon_dsFecThrd_write(uint8 value)
{
    uint32 tmp = value;

	_apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,FEC_DET_THRSHf,&tmp);
}

int32 apollo_raw_gpon_dsFecThrd_read(uint8 *value)
{
    uint32 tmp ;
    int32 ret;

    ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_CFGr,FEC_DET_THRSHf,&tmp);

    *value = (uint8)tmp;
    return ret;
}

void apollo_raw_gpon_extraSN_write(uint8 value)
{
    uint32 tmp = value;

	_apollo_wrap_reg_field_write(GPON_GTC_DS_CFGr,EXTRA_SN_TXf,&tmp);
}

void apollo_raw_gpon_dsPloam_nomsg_write(uint8 value)
{
    uint32 tmp=value;

    _apollo_wrap_reg_field_write(GPON_GTC_DS_PLOAM_CFGr,PLM_DS_NOMSG_IDf,&tmp);
}

void apollo_raw_gpon_dsPloam_onuIdFilter_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_PLOAM_CFGr,PLM_DS_ONUID_FLT_ENf,&value);
}

int32 apollo_raw_gpon_dsPloam_onuIdFilter_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_PLOAM_CFGr,PLM_DS_ONUID_FLT_ENf,value);
}

void apollo_raw_gpon_dsPloam_bcAccept_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_DS_PLOAM_CFGr,PLM_BC_ACC_ENf,&value);
}

int32 apollo_raw_gpon_dsPloam_bcAccept_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_PLOAM_CFGr,PLM_BC_ACC_ENf,value);
}

/* Added in GPON_MAC_SWIO_v1.1 */
void apollo_raw_gpon_dsPloam_dropCrc_write(int32 value)
{
    _apollo_wrap_reg_field_write(GPON_GTC_DS_PLOAM_CFGr,PLM_DROP_CRCEf,&value);
}

int32 apollo_raw_gpon_dsPloam_dropCrc_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_PLOAM_CFGr,PLM_DROP_CRCEf,value);
}

int32 apollo_raw_gpon_cdrLos_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_LOS_CFG_STSr, CDR_LOS_SIGf, pValue);
}

int32 apollo_raw_gpon_optLos_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_DS_LOS_CFG_STSr, OPTIC_LOS_SIGf, pValue);
}

void apollo_raw_gpon_losCfg_write(int32 opten, int32 optpolar, int32 cdren, int32 cdrpolar, int32 filter)
{
    uint32 value;
	_apollo_wrap_reg_read(GPON_GTC_DS_LOS_CFG_STSr,&value);
	_apollo_wrap_reg_field_set(GPON_GTC_DS_LOS_CFG_STSr,OPTIC_LOS_ENf,&opten,&value);
	_apollo_wrap_reg_field_set(GPON_GTC_DS_LOS_CFG_STSr,OPTIC_LOS_POLARf,&optpolar,&value);
	_apollo_wrap_reg_field_set(GPON_GTC_DS_LOS_CFG_STSr,CDR_LOS_ENf,&cdren,&value);
	_apollo_wrap_reg_field_set(GPON_GTC_DS_LOS_CFG_STSr,CDR_LOS_POLARf,&cdrpolar,&value);
	_apollo_wrap_reg_field_set(GPON_GTC_DS_LOS_CFG_STSr,LOS_FILTER_ENf,&filter,&value);
	_apollo_wrap_reg_write(GPON_GTC_DS_LOS_CFG_STSr,&value);
}

int32 apollo_raw_gpon_losCfg_read(int32 *opten, int32 *optpolar, int32 *cdren, int32 *cdrpolar, int32 *filter)
{
    uint32 data;
	_apollo_wrap_reg_read(GPON_GTC_DS_LOS_CFG_STSr,&data);
	_apollo_wrap_reg_field_get(GPON_GTC_DS_LOS_CFG_STSr,OPTIC_LOS_ENf,(uint32*)opten,&data);
	_apollo_wrap_reg_field_get(GPON_GTC_DS_LOS_CFG_STSr,OPTIC_LOS_POLARf,(uint32*)optpolar,&data);
	_apollo_wrap_reg_field_get(GPON_GTC_DS_LOS_CFG_STSr,CDR_LOS_ENf,(uint32*)cdren,&data);
	_apollo_wrap_reg_field_get(GPON_GTC_DS_LOS_CFG_STSr,CDR_LOS_POLARf,(uint32*)cdrpolar,&data);
	_apollo_wrap_reg_field_get(GPON_GTC_DS_LOS_CFG_STSr,LOS_FILTER_ENf,(uint32*)filter,&data);

    return RT_ERR_OK;
}

int32 apollo_raw_gpon_superframe_read(uint32 * value)
{
    _apollo_wrap_reg_read(GPON_GTC_DS_SUPERFRAME_CNTr,value);
    return RT_ERR_OK;
}

int32 apollo_raw_gpon_dsPloam_read(uint8 *value)
{
    uint32 ploam,i;
    uint32 tmp,data;

    _apollo_wrap_reg_field_read(GPON_GTC_DS_PLOAM_INDr,PLM_BUF_EMPTYf,&tmp);
    if(tmp==GPON_REG_STATUS_CLEAR)
    {
        for(i=0;i<6;i++)
        {
            _apollo_wrap_reg_array_field_read(GPON_GTC_DS_PLOAM_MSGr,
                             REG_ARRAY_INDEX_NONE,
                             i,
                             PLOAM_RDATAf,
                             &ploam);
            value[2*i+0] = (ploam>>GPON_REG_16BITS_HIGH)&0xFF;
            value[2*i+1] = (ploam>>GPON_REG_16BITS_LOW)&0xFF;
        }
		data = GPON_REG_STATUS_CLEAR;
        _apollo_wrap_reg_write(GPON_GTC_DS_PLOAM_INDr,&data);
		data = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_write(GPON_GTC_DS_PLOAM_INDr,&data);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_PLOAM_QUEUE_EMPTY;
    }
}

int32 apollo_raw_gpon_tcont_read(uint32 idx, uint32 *value)
{
    uint32 op = 0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_READ;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_ALLOC_INDr,TRUE))
    {
		_apollo_wrap_reg_read(GPON_GTC_DS_ALLOC_RDr,value);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_tcont_write(uint32 idx, uint32 value)
{
    uint32 op = 0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_WRITE;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

    _apollo_wrap_reg_field_write(GPON_GTC_DS_ALLOC_WRr,ALLOCID_OP_WDATAf,&value);

	tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_ALLOC_INDr,FALSE))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_tcont_clean(uint32 idx)
{
    uint32 op = 0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_CLEAN;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

	tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ALLOC_INDr, ALLOCID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_ALLOC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_ALLOC_INDr,FALSE))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_read(uint32 idx, uint32 *value, uint32 *cfg)
{
    uint32 op = 0;
    uint32 tmp_val;
    int32  ret;

    tmp_val = GPON_REG_OPERRATION_READ;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_PORT_INDr,TRUE))
    {
#if defined(CONFIG_SDK_ASICDRV_TEST)
		_apollo_wrap_reg_field_read(GPON_GTC_DS_PORT_WRr,PORTID_OP_WDATAf,value);
#else
		_apollo_wrap_reg_field_read(GPON_GTC_DS_PORT_RDr,PORTID_OP_RDATAf,value);
#endif
        ret = _apollo_wrap_reg_array_field_read(GPON_GTC_DS_TRAFFIC_CFGr,
                             REG_ARRAY_INDEX_NONE,
                             idx,
                             TRAFFIC_TYPE_CFGf,
                             cfg);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_write(uint32 idx, uint32 value, uint32 cfg)
{
    uint32 op = 0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_WRITE;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    _apollo_wrap_reg_field_write(GPON_GTC_DS_PORT_WRr,PORTID_OP_WDATAf,&value);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_PORT_INDr,FALSE))
    {
        _apollo_wrap_reg_array_field_write(GPON_GTC_DS_TRAFFIC_CFGr,
                              REG_ARRAY_INDEX_NONE,
                              idx,
                              TRAFFIC_TYPE_CFGf,
                              &cfg);

        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_clean(uint32 idx)
{
    uint32 op = 0;
    uint32 tmp_val;
    uint32 cfg=0;

    tmp_val = GPON_REG_OPERRATION_CLEAN;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_IDXf, &tmp_val, &op);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_INDr, PORTID_OP_REQf, &tmp_val, &op);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GTC_DS_PORT_INDr,FALSE))
    {
        _apollo_wrap_reg_array_field_write(GPON_GTC_DS_TRAFFIC_CFGr,
                              REG_ARRAY_INDEX_NONE,
                              idx,
                              TRAFFIC_TYPE_CFGf,
                              &cfg);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_pktCnt_read(uint32 idx, uint32 *value)
{
    uint32 ind = 0;
    uint32 tmp_val;

    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_CNTR_INDr, GEM_CNTR_IDXf, &tmp_val, &ind);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_CNTR_INDr, GEM_CNTR_RSELf, &tmp_val, &ind);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_CNTR_INDr,&ind);

    if(apollo_raw_gpon_indirect_wait(GPON_GTC_DS_PORT_CNTR_INDr))
    {
		return _apollo_wrap_reg_field_read(GPON_GTC_DS_PORT_CNTR_STATr,GEM_CNTRf,value);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_byteCnt_read(uint32 idx, uint32 *value)
{
    uint32 ind = 0;
    uint32 tmp_val;

    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_CNTR_INDr, GEM_CNTR_IDXf, &tmp_val, &ind);
    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_PORT_CNTR_INDr, GEM_CNTR_RSELf, &tmp_val, &ind);

    _apollo_wrap_reg_write(GPON_GTC_DS_PORT_CNTR_INDr,&ind);

    if(apollo_raw_gpon_indirect_wait(GPON_GTC_DS_PORT_CNTR_INDr))
    {
		return _apollo_wrap_reg_field_read(GPON_GTC_DS_PORT_CNTR_STATr,GEM_CNTRf,value);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsGtc_miscCnt_read(apollo_raw_gpon_dsGtc_pmMiscType_t idx, uint32 *pValue)
{
    int32 ret;

    switch(idx)
    {
        case GPON_REG_DSGTCPMMISC_BIP_BLOCK_ERR:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_BIP_ERR_BLKr,CNTR_DS_BIP_ERR_BLOCKf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_BIP_BITS_ERR:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_BIP_ERR_BITr,CNTR_DS_BIP_ERR_BITSf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_FEC_CORRECTED_BITS:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_FEC_CORRECT_BITr,CNTR_FEC_CORRECTED_BITSf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_FEC_CORRECTED_BYTES:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_FEC_CORRECT_BYTEr,CNTR_FEC_CORRECTED_BYTESf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_FEC_CORRETED_CWS:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_FEC_CORRECT_CWr,CNTR_FEC_CORRETED_CWf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_FEC_UNCOR_CWS:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_FEC_UNCOR_CWr,CNTR_FEC_UNCORRETABLE_CWf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_LOM:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_LOMr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_PLOAM_ACCEPT:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_PLOAM_ACPTr,CNTR_PLOAMD_ACCEPTEDf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_PLOAM_FAIL:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_PLOAM_FAILr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_BWMAP_FAIL:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_BWM_FAILr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_BWMAP_INV:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_BWM_INVr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_ACTIVE:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_ACTIVEr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_BWMAP_ACPT:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_BWM_ACPTr,CNTR_BWMAP_ACCPTEDf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_GEM_LOS:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_GEM_LOSr,CNTR_GEM_LOSf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_HEC_COR:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_HEC_CORRECTr,CNTR_HEC_CORRECTEDf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_GEM_IDLE:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_GEM_IDLEr,CNTR_GEM_IDLEf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_GEM_FAIL:
            ret = _apollo_wrap_reg_read(GPON_GTC_DS_MISC_CNTR_GEM_FAILr,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_RX_GEM_NON_IDLE:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_GEM_NON_IDLEr,CNTR_RX_GEM_NON_IDLEf,pValue);
            break;
        case GPON_REG_DSGTCPMMISC_PLEND_CORRECTIONS:
            ret = _apollo_wrap_reg_field_read(GPON_GTC_DS_MISC_CNTR_PLEN_CORRECTr,CNTR_PLEND_CORRECTIONSf,pValue);
            break;
        default:
            ret = RT_ERR_OUT_OF_RANGE;
            break;
    }
    return ret;
}

void apollo_raw_gpon_dsOmci_pti_write(uint8 mask, uint8 end)
{
    uint32 data=0;
    uint32 value;

    value = mask;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_OMCI_PTIr, OMCI_PTI_MASKf, &value, &data);
    value = end;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_OMCI_PTIr, OMCI_END_PTIf, &value, &data);

    _apollo_wrap_reg_write(GPON_GTC_DS_OMCI_PTIr,&data);
}

int32 apollo_raw_gpon_dsOmci_pti_read(uint8 *mask, uint8 *end)
{
    uint32 value, data;
    int32 ret;

    if((ret = _apollo_wrap_reg_read(GPON_GTC_DS_OMCI_PTIr, &data))!=RT_ERR_OK)
        return ret;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_OMCI_PTIr,OMCI_PTI_MASKf,&value,&data);
    *mask = value&0x7;
    _apollo_wrap_reg_field_get(GPON_GTC_DS_OMCI_PTIr,OMCI_END_PTIf,&value,&data);
    *end = value&0x7;

    return RT_ERR_OK;
}

void apollo_raw_gpon_dsEth_pti_write(uint8 mask, uint8 end)
{
    uint32 data=0;
    uint32 value;

    value = mask;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ETH_PTIr, ETH_PTI_MASKf, &value, &data);
    value = end;
    _apollo_wrap_reg_field_set(GPON_GTC_DS_ETH_PTIr, ETH_END_PTIf, &value, &data);

    _apollo_wrap_reg_write(GPON_GTC_DS_ETH_PTIr,&data);
}

int32 apollo_raw_gpon_dsEth_pti_read(uint8 *mask, uint8 *end)
{
    uint32 value, data;
    int32 ret;

    if((ret = _apollo_wrap_reg_read(GPON_GTC_DS_ETH_PTIr, &data))!=RT_ERR_OK)
        return ret;

    _apollo_wrap_reg_field_get(GPON_GTC_DS_ETH_PTIr,ETH_PTI_MASKf,&value,&data);
    *mask = value&0x7;
    _apollo_wrap_reg_field_get(GPON_GTC_DS_ETH_PTIr,ETH_END_PTIf,&value,&data);
    *end = value&0x7;

    return RT_ERR_OK;
}

void apollo_raw_gpon_aes_keyswitch_write(uint32 value)
{
    uint32 data;
    data = value;
    _apollo_wrap_reg_field_write(GPON_AES_KEY_SWITCH_TIMEr,SWITCH_SUPERFRAMEf,&data);
}

void apollo_raw_gpon_aes_keyswitch_read(uint32 *pValue)
{
    uint32 data;

    _apollo_wrap_reg_field_read(GPON_AES_KEY_SWITCH_TIMEr,SWITCH_SUPERFRAMEf,&data);
    *pValue = data;
}

int32 apollo_raw_gpon_aes_keyword_write(uint8 *value)
{
    uint32 data = 0;
    uint32 tmp_val;
    uint32 idx,ind;

    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_AES_KEY_SWITCH_REQr, CFG_ACTIVE_KEYf, &tmp_val, &data);
    tmp_val = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_AES_KEY_SWITCH_REQr, KEY_CFG_REQf, &tmp_val, &data);

    _apollo_wrap_reg_write(GPON_AES_KEY_SWITCH_REQr,&data);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_AES_KEY_SWITCH_REQr, KEY_CFG_REQf, &tmp_val, &data);

    _apollo_wrap_reg_write(GPON_AES_KEY_SWITCH_REQr,&data);

    for(idx=0;idx<8;idx++)
    {
        data = (value[2*idx+0]<<GPON_REG_16BITS_HIGH)|(value[2*idx+1]<<GPON_REG_16BITS_LOW);
        _apollo_wrap_reg_field_write(GPON_AES_WORD_DATAr,KEY_DATAf,&data);

        ind = 0;
        tmp_val = idx;
        _apollo_wrap_reg_field_set(GPON_AES_KEY_WORD_INDr, KEY_WORD_IDXf, &tmp_val, &ind);
        tmp_val = GPON_REG_STATUS_CLEAR;
        _apollo_wrap_reg_field_set(GPON_AES_KEY_WORD_INDr, KEY_WR_REQf, &tmp_val, &ind);
        _apollo_wrap_reg_write(GPON_AES_KEY_WORD_INDr,&ind);

        tmp_val = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_field_set(GPON_AES_KEY_WORD_INDr, KEY_WR_REQf, &tmp_val, &ind);
		_apollo_wrap_reg_write(GPON_AES_KEY_WORD_INDr,&ind);

        if(apollo_raw_gpon_cam_wait(GPON_AES_KEY_WORD_INDr,FALSE))
        {
        }
        else
        {
        }
    }
    return RT_ERR_OK;
}

int32 apollo_raw_gpon_gemDs_intr_read(uint32 *pValue)
{
#if defined(OLD_FPGA_DEFINED)
    return _apollo_wrap_reg_read(GPON_GEM_DS_INTR_DLTr, pValue);
#else
    if(pValue)
        return FALSE;

    return FALSE;
#endif
}

int32 apollo_raw_gpon_gemDsIntr_omci_check(uint32 status)
{
#if defined(OLD_FPGA_DEFINED)
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_DS_INTR_DLTr, OMCI_RX_REQf, &value, &status);
    return value;
#else
    if(status)
        return FALSE;
    return FALSE;
#endif
}

int32 apollo_raw_gpon_omci_read(uint8 *value)
{
#if defined(OLD_FPGA_DEFINED)
    uint32 omci,i;
    uint32 tmp,data;

    _apollo_wrap_reg_field_read(GPON_GEM_DS_OMCI_INDr, OMCI_BUF_EMPTYf, &tmp);
    if(tmp==GPON_REG_STATUS_CLEAR)
    {
        for(i = 0; i<(RTK_GPON_OMCI_MSG_LEN / 4); i++)
        {
            _apollo_wrap_reg_array_read(GPON_GEM_DS_OMCI_MSGr,
                             REG_ARRAY_INDEX_NONE,
                             i,
                             &omci);
            value[4 * i + 0] = (omci >> GPON_REG_32BITS_3) & 0xFF;
            value[4 * i + 1] = (omci >> GPON_REG_32BITS_2) & 0xFF;
            value[4 * i + 2] = (omci >> GPON_REG_32BITS_1) & 0xFF;
            value[4 * i + 3] = (omci >> GPON_REG_32BITS_0) & 0xFF;
        }
		data = GPON_REG_STATUS_CLEAR;
        _apollo_wrap_reg_write(GPON_GEM_DS_OMCI_INDr, &data);
		data = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_write(GPON_GEM_DS_OMCI_INDr, &data);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_OMCI_QUEUE_EMPTY;
    }
#else
    if(value)
        return RT_ERR_GPON_OMCI_QUEUE_EMPTY;

    return RT_ERR_GPON_OMCI_QUEUE_EMPTY;
#endif
}

int32 apollo_raw_gpon_dsPort_ethRxCnt_read(uint32 idx, uint32 *value)
{
    _apollo_wrap_reg_write(GPON_GEM_DS_RX_CNTR_INDr,&idx);

    if(apollo_raw_gpon_indirect_wait(GPON_GEM_DS_RX_CNTR_INDr))
    {
		return _apollo_wrap_reg_field_read(GPON_GEM_DS_RX_CNTR_STATr,ETH_PKT_RXf,value);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsPort_ethFwdCnt_read(uint32 idx, uint32 *value)
{
    _apollo_wrap_reg_write(GPON_GEM_DS_FWD_CNTR_INDr,&idx);

    if(apollo_raw_gpon_indirect_wait(GPON_GEM_DS_FWD_CNTR_INDr))
    {
		return _apollo_wrap_reg_field_read(GPON_GEM_DS_FWD_CNTR_STATr,ETH_PKT_FWDf,value);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_dsGem_miscCnt_read(apollo_raw_gpon_dsGem_pmMiscType_t idx, uint32 *pValue)
{
    int32 ret;

    RT_PARAM_CHK((idx>GPON_REG_DSGEMPMMISC_OMCI_RX),RT_ERR_OUT_OF_RANGE);
    ret = _apollo_wrap_reg_field_write(GPON_GEM_DS_MISC_INDr,MISC_CNTR_IDXf,&idx);
    if(ret != RT_ERR_OK)
        return ret;

    return _apollo_wrap_reg_field_read(GPON_GEM_DS_MISC_CNTR_STATr,MISC_CNTRf,pValue);
}

void apollo_raw_gpon_dsEth_fcsCheck_enable(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GEM_DS_MC_CFGr,FCS_CHK_ENf,&value);
}

int32 apollo_raw_gpon_dsEth_fcsCheck_enable_read(uint32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_CFGr,FCS_CHK_ENf,value);
}

void apollo_raw_gpon_dsEth_bcPAss_write(int32 value)
{
    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_CFGr,BROADCAST_PASSf,&value);
}

int32 apollo_raw_gpon_dsEth_bcPAss_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_CFGr,BROADCAST_PASSf,value);
}

void apollo_raw_gpon_dsEth_nonMcPass_write(int32 value)
{
    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_CFGr,NON_MULTICAST_PASSf,&value);
}

int32 apollo_raw_gpon_dsEth_nonMcPass_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_CFGr,NON_MULTICAST_PASSf,value);
}

void apollo_raw_gpon_dsEth_macFilterMode_write(rtk_gpon_macTable_exclude_mode_t exclude)
{
    uint32 value;
    if(exclude == RTK_GPON_MACTBL_MODE_EXCLUDE)
        value = 1;
    else
        value =0;

    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_CFGr,MC_EXCL_MODEf,&value);
}

int32 apollo_raw_gpon_dsEth_macFilterMode_read(rtk_gpon_macTable_exclude_mode_t *value)
{
    uint32 excl_mode;
    int32 ret;

    if( (ret = _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_CFGr,MC_EXCL_MODEf,&excl_mode)) != RT_ERR_OK )
        return ret;

    if(excl_mode == 0)
        *value = RTK_GPON_MACTBL_MODE_INCLUDE;
    else
        *value = RTK_GPON_MACTBL_MODE_EXCLUDE;
    return RT_ERR_OK;
}

void apollo_raw_gpon_dsEth_mcForceMode_write(rtk_gpon_mc_force_mode_t ipv4,
                                      rtk_gpon_mc_force_mode_t ipv6)
{
    uint32 drop_ipv4, pass_ipv4, drop_ipv6, pass_ipv6;
    uint32 data;

    switch(ipv4)
    {
        case RTK_GPON_MCFORCE_MODE_PASS:
            drop_ipv4 = 0;
            pass_ipv4 = 1;
        break;

        case RTK_GPON_MCFORCE_MODE_DROP:
            drop_ipv4 = 1;
            pass_ipv4 = 0;
        break;

        case RTK_GPON_MCFORCE_MODE_NORMAL:
        default:
            drop_ipv4 = 0;
            pass_ipv4 = 0;
        break;
    }
    switch(ipv6)
    {
        case RTK_GPON_MCFORCE_MODE_PASS:
            drop_ipv6 = 0;
            pass_ipv6 = 1;
        break;

        case RTK_GPON_MCFORCE_MODE_DROP:
            drop_ipv6 = 1;
            pass_ipv6 = 0;
        break;

        case RTK_GPON_MCFORCE_MODE_NORMAL:
        default:
            drop_ipv6 = 0;
            pass_ipv6 = 0;
        break;
    }

    _apollo_wrap_reg_read(GPON_GEM_DS_MC_CFGr, &data);

    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_CFGr,IPV4_MC_FORCE_PASSf,&pass_ipv4,&data);
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_CFGr,IPV4_MC_FORCE_DROPf,&drop_ipv4,&data);
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_CFGr,IPV6_MC_FORCE_PASSf,&pass_ipv6,&data);
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_CFGr,IPV6_MC_FORCE_DROPf,&drop_ipv6,&data);

    _apollo_wrap_reg_write(GPON_GEM_DS_MC_CFGr,&data);

}

int32 apollo_raw_gpon_dsEth_mcForceMode_read(rtk_gpon_mc_force_mode_t *ipv4,
                                      rtk_gpon_mc_force_mode_t *ipv6)
{
    uint32 drop_ipv4, pass_ipv4, drop_ipv6, pass_ipv6;
    uint32 data;
    int32  ret;

    if((ret = _apollo_wrap_reg_read(GPON_GEM_DS_MC_CFGr,&data)) != RT_ERR_OK )
        return ret;

    if((ret = _apollo_wrap_reg_field_get(GPON_GEM_DS_MC_CFGr,IPV4_MC_FORCE_PASSf,&pass_ipv4,&data)) != RT_ERR_OK)
        return ret;
    if((ret = _apollo_wrap_reg_field_get(GPON_GEM_DS_MC_CFGr,IPV4_MC_FORCE_DROPf,&drop_ipv4,&data)) != RT_ERR_OK)
        return ret;
    if((ret = _apollo_wrap_reg_field_get(GPON_GEM_DS_MC_CFGr,IPV6_MC_FORCE_PASSf,&pass_ipv6,&data)) != RT_ERR_OK)
        return ret;
    if((ret = _apollo_wrap_reg_field_get(GPON_GEM_DS_MC_CFGr,IPV6_MC_FORCE_DROPf,&drop_ipv6,&data)) != RT_ERR_OK)
        return ret;

    if((pass_ipv4 == 0) && (drop_ipv4 == 0))
        *ipv4 = RTK_GPON_MCFORCE_MODE_NORMAL;
    else if(pass_ipv4 == 1)
        *ipv4 = RTK_GPON_MCFORCE_MODE_PASS;
    else
        *ipv4 = RTK_GPON_MCFORCE_MODE_DROP;

    if((pass_ipv6 == 0) && (drop_ipv6 == 0))
        *ipv6 = RTK_GPON_MCFORCE_MODE_NORMAL;
    else if(pass_ipv6 == 1)
        *ipv6 = RTK_GPON_MCFORCE_MODE_PASS;
    else
        *ipv6 = RTK_GPON_MCFORCE_MODE_DROP;

    return RT_ERR_OK;
}

int32 apollo_raw_gpon_macEntry_read(uint32 idx, uint32 *value)
{
    uint32 op=0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_READ;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_IDXf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_REQf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GEM_DS_MC_INDr,TRUE))
    {
#if defined (CONFIG_SDK_ASICDRV_TEST)
		return _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_WRr,MC_ITEM_OP_WDATAf,value);
#else
		return _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_RDr,MC_ITEM_OP_RDATAf,value);
#endif
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_macEntry_write(uint32 idx, uint32 value)
{
    uint32 op=0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_WRITE;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_IDXf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_WRr,MC_ITEM_OP_WDATAf,&value);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_REQf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GEM_DS_MC_INDr,FALSE))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

int32 apollo_raw_gpon_macEntry_clean(uint32 idx)
{
    uint32 op=0;
    uint32 tmp_val;

    tmp_val = GPON_REG_OPERRATION_CLEAN;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_MODEf, &tmp_val, &op);
    tmp_val = idx;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_IDXf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    tmp_val = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GEM_DS_MC_INDr, MC_ITEM_OP_REQf, &tmp_val, &op);
    _apollo_wrap_reg_write(GPON_GEM_DS_MC_INDr,&op);

    if(apollo_raw_gpon_cam_wait(GPON_GEM_DS_MC_INDr,FALSE))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

void apollo_raw_gpon_assembleTimeout_write(uint8 value)
{
    uint32 tmp = value;
	_apollo_wrap_reg_field_write(GPON_GEM_DS_FRM_TIMEOUTr, ASSM_TIMEOUT_FRMf,&tmp);
}

int32 apollo_raw_gpon_assembleTimeout_read(uint8 *value)
{
    uint32 tmp;
    int32 ret;
    ret=_apollo_wrap_reg_field_read(GPON_GEM_DS_FRM_TIMEOUTr, ASSM_TIMEOUT_FRMf,&tmp);
    *value = (tmp&0x1F);
    return ret;
}

/* rename MAC_H24B_xxx to IPV4_MC_MAC_PREFIX in GPON_MAC_SWIO_r1.2 */
void apollo_raw_gpon_ipv4McAddrPtn_write(uint32 value)
{
    uint32 pattern=value&0xFFFFFF;
    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_ADDR_PTN_IPV4r, IPV4_MC_MAC_PREFIXf,&pattern);
}

int32 apollo_raw_gpon_ipv4McAddrPtn_read(uint32 *value)
{
    uint32 pattern;
    int32  ret;

    ret = _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_ADDR_PTN_IPV4r, IPV4_MC_MAC_PREFIXf,&pattern);
    *value = pattern&0xFFFFFF;
    return ret;
}
/* add IPV6_MC_MAC_PREFIX in GPON_MAC_SWIO_r1.2 */
void apollo_raw_gpon_ipv6McAddrPtn_write(uint32 value)
{
    uint32 pattern=value&0xFFFF;
    _apollo_wrap_reg_field_write(GPON_GEM_DS_MC_ADDR_PTN_IPV6r,IPV6_MC_MAC_PREFIXf,&pattern);
}

int32 apollo_raw_gpon_ipv6McAddrPtn_read(uint32 *value)
{
    uint32 pattern;
    int32  ret;

    ret = _apollo_wrap_reg_field_read(GPON_GEM_DS_MC_ADDR_PTN_IPV6r,IPV6_MC_MAC_PREFIXf,&pattern);
    *value = pattern&0xFFFF;
    return ret;
}

#if 0 /* wellknown addr is removed in GPON_MAC_SWIO_r1.1 */
void apollo_raw_gpon_wellKnownAddr_write(uint32 value)
{
    apollo_raw_gpon_register_write(GPON_GEM_DS_WELLKWN_ADDRr,value);
}

int32 apollo_raw_gpon_wellKnownAddr_read(uint32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_DS_WELLKWN_ADDRr,WELL_KNOWN_ADDRf,value);
}
#endif

int32 apollo_raw_gpon_gtcUs_intr_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GTC_US_INTR_DLTr, pValue);
}

int32 apollo_raw_gpon_gtcUsIntr_fec_check(uint32 status)
{
    uint32 value;

    _apollo_wrap_reg_field_get(GPON_GTC_US_INTR_DLTr, US_FEC_STS_DLTf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcUsIntr_urgPloamEmpty_check(uint32 status)
{
    uint32 value;

    _apollo_wrap_reg_field_get(GPON_GTC_US_PLOAM_INDr, PLM_URG_EMPTYf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcUsIntr_nrmPloamEmpty_check(uint32 status)
{
    uint32 value;

    _apollo_wrap_reg_field_get(GPON_GTC_US_PLOAM_INDr, PLM_NRM_EMPTYf, &value, &status);
    return value;
}

int32 apollo_raw_gpon_gtcUs_mask_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GTC_US_INTR_MASKr, pValue);
}

void apollo_raw_gpon_gtcUs_mask_write(uint32 value)
{
    _apollo_wrap_reg_write(GPON_GTC_US_INTR_MASKr,&value);
}

void apollo_raw_gpon_gtcUsMask_fec_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_INTR_MASKr,US_FEC_STS_Mf,&value);
}

void apollo_raw_gpon_gtcUsMask_urgPloamEmpty_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_INTR_MASKr,PLM_URG_EMPTY_Mf,&value);
}

void apollo_raw_gpon_gtcUsMask_nrmPloamEmpty_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_INTR_MASKr,PLM_NRM_EMPTY_Mf,&value);
}

int32 apollo_raw_gpon_gtcUs_fec_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_INTR_STSr,US_FEC_STSf,pValue);
}

int apollo_raw_gpon_usOnuId_read(uint8 *pValue)
{
    uint32 tmp;
    int32 ret;

	ret = _apollo_wrap_reg_field_read(GPON_GTC_US_ONU_IDr, ONU_IDf, &tmp);

    *pValue = tmp;

    return ret;
}

void apollo_raw_gpon_forceLaser_write(rtk_gpon_laser_status_t value)
{
    uint32 data,tmp;

	_apollo_wrap_reg_read(GPON_GTC_US_CFGr,&data);

	if(value==RTK_GPON_LASER_STATUS_FORCE_ON)
    {
        tmp = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_field_set(GPON_GTC_US_CFGr, FS_LONf, &tmp, &data);
    }
    else
    {
        tmp = GPON_REG_STATUS_CLEAR;
        _apollo_wrap_reg_field_set(GPON_GTC_US_CFGr, FS_LONf, &tmp, &data);
    }

    if(value==RTK_GPON_LASER_STATUS_FORCE_OFF)
    {
        tmp = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_field_set(GPON_GTC_US_CFGr, FS_LOFFf, &tmp, &data);
    }
    else
    {
        tmp = GPON_REG_STATUS_CLEAR;
        _apollo_wrap_reg_field_set(GPON_GTC_US_CFGr, FS_LOFFf, &tmp, &data);
    }
	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

	_apollo_wrap_reg_write(GPON_GTC_US_CFGr,&data);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_forceLaser_read(rtk_gpon_laser_status_t *pValue)
{
    uint32 data,value;
    int32 ret;

    ret = _apollo_wrap_reg_read(GPON_GTC_US_CFGr, &data);
    _apollo_wrap_reg_field_get(GPON_GTC_US_CFGr,FS_LOFFf,&value,&data);
    if(value)
    {
        *pValue = RTK_GPON_LASER_STATUS_FORCE_OFF;
        return ret;
    }
    _apollo_wrap_reg_field_get(GPON_GTC_US_CFGr,FS_LONf,&value,&data);
    if(value)
    {
        *pValue = RTK_GPON_LASER_STATUS_FORCE_ON;
        return ret;
    }
    *pValue = RTK_GPON_LASER_STATUS_NORMAL;

    return ret;
}

void apollo_raw_gpon_txPtnMode_write(rtk_gpon_tx_ptn_mode_t fg_mode, rtk_gpon_tx_ptn_mode_t bg_mode)
{
    uint32 no_fg;
    uint32 data=0;
    uint32 tmp;

    if(fg_mode == RTK_GPON_TX_PTN_MODE_NONE)
    {
        no_fg = 1;
        fg_mode = 0;
    }
    else
    {
        no_fg = 0;
    }

    _apollo_wrap_reg_field_set(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_NO_FGf, &no_fg, &data);
    _apollo_wrap_reg_field_set(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_FGf, &fg_mode, &data);
    _apollo_wrap_reg_field_set(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_BGf, &bg_mode, &data);

    tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_write(GPON_GTC_US_TX_PATTERN_CTLr,&data);

    tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_txPtnMode_read(rtk_gpon_tx_ptn_mode_t *pFg_mode, rtk_gpon_tx_ptn_mode_t *pBg_mode)
{
    uint32 val, data;
    uint32 ret;

    if((ret = _apollo_wrap_reg_read(GPON_GTC_US_TX_PATTERN_CTLr, &data)) != RT_ERR_OK)
        return ret;

    if((ret = _apollo_wrap_reg_field_get(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_BGf, &val, &data)) != RT_ERR_OK)
        return ret;

    *pBg_mode = val;

    if((ret = _apollo_wrap_reg_field_get(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_NO_FGf, &val, &data)) != RT_ERR_OK)
        return ret;

    if(val == 1)
        *pFg_mode = RTK_GPON_TX_PTN_MODE_NONE;
    else
    {
        if((ret = _apollo_wrap_reg_field_get(GPON_GTC_US_TX_PATTERN_CTLr, TX_PATTERN_MODE_FGf, &val, &data)) != RT_ERR_OK)
            return ret;
        *pFg_mode = val;
    }

    return ret;
}

void apollo_raw_gpon_txPtn_write(uint32 fg_ptn, uint32 bg_ptn)
{
    uint32 tmp;

    tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_TX_PATTERN_FGr, TX_PATTERN_FGf, &fg_ptn);
    _apollo_wrap_reg_field_write(GPON_GTC_US_TX_PATTERN_BGr, TX_PATTERN_BGf, &bg_ptn);

    tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_txPtn_read(uint32 *pFg_ptn, uint32 *pBg_ptn)
{
    uint32 ret;

    if((ret = _apollo_wrap_reg_field_read(GPON_GTC_US_TX_PATTERN_FGr, TX_PATTERN_FGf, pFg_ptn)) != RT_ERR_OK)
        return ret;

    if((ret = _apollo_wrap_reg_field_read(GPON_GTC_US_TX_PATTERN_BGr, TX_PATTERN_BGf, pBg_ptn)) != RT_ERR_OK)
        return ret;

    return ret;
}

void apollo_raw_gpon_RogueOnt_write(int32 value)
{
    if(1 == value)			//diagnose enable
    {
        /*osal_print("Rcv Ds ploam DiagNose enable %d \r\n",value);*/
        apollo_raw_gpon_txPtn_write(0x0, 0x55555555);
        apollo_raw_gpon_txPtnMode_write(RTK_GPON_TX_PTN_MODE_NORMAL, RTK_GPON_TX_PTN_MODE_CUSTOM);
    }
    else if(0 == value)		//diagnose disable
    {
        /*osal_print("Rcv Ds ploam DiagNose enable %d \r\n",value);*/
        apollo_raw_gpon_txPtn_write(0x0, 0x0);
        apollo_raw_gpon_txPtnMode_write(RTK_GPON_TX_PTN_MODE_NORMAL, RTK_GPON_TX_PTN_MODE_NORMAL);
    }
}

void apollo_raw_gpon_forceUsPrbs_write(int32 value)
{
    if(1 == value)			//diagnose enable
    {
        apollo_raw_gpon_txPtnMode_write(RTK_GPON_TX_PTN_MODE_NONE, RTK_GPON_TX_PTN_MODE_PRBS);
        apollo_raw_gpon_forceLaser_write(RTK_GPON_LASER_STATUS_FORCE_ON);
    }
    else if(0 == value)		//diagnose disable
    {
        apollo_raw_gpon_txPtnMode_write(RTK_GPON_TX_PTN_MODE_NORMAL, RTK_GPON_TX_PTN_MODE_NORMAL);
        apollo_raw_gpon_forceLaser_write(RTK_GPON_LASER_STATUS_NORMAL);
    }
}

#if 0
void apollo_raw_gpon_forceUsGemIdle_write(int32 value)
{
	uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,FS_GEM_IDLEf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_forceUsGemIdle_read(int32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,FS_GEM_IDLEf,pValue);
}

void apollo_raw_gpon_forceUsPrbs_write(int32 value)
{
	uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,FS_PRBSf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_forceUsPrbs_read(int32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,FS_PRBSf,pValue);
}
#endif

void apollo_raw_gpon_ploam_enable(int32 value)
{
    uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

	value = !value;
    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,PLM_DISf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_ploam_enable_read(int32 *pValue)
{
    uint32 data;
    int32 ret;

    ret = _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,PLM_DISf,&data);
    *pValue = !data;
    return ret;
}

void apollo_raw_gpon_ind_nrm_ploam_enable(int32 value)
{
    uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,IND_NRM_PLMf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_ind_nrm_ploam_enable_read(int32 *pValue)
{
    uint32 data;
    int32 ret;

    ret = _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,IND_NRM_PLMf,&data);
    *pValue = data;
    return ret;
}

void apollo_raw_gpon_dbru_enable(int32 value)
{
    uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

	value = !value;
    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,DBRU_DISf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_dbru_enable_read(int32 *pValue)
{
    uint32 data;
    int32 ret;

    ret = _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,DBRU_DISf,&data);
    *pValue = !data;
    return ret;
}

void apollo_raw_gpon_usScramble_enable(int32 value)
{
	uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

	value = !value;
    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,SCRM_DISf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_usScramble_enable_read(int32 *pValue)
{
    uint32 data;
    int32 ret;

    ret = _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,SCRM_DISf,&data);
    *pValue = !data;
    return ret;
}

void apollo_raw_gpon_usBurstPolarity_write(int32 value)
{
    uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_CFGr,US_BEN_POLARf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_usBurstPolarity_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_CFGr,US_BEN_POLARf,value);
}

extern gpon_drv_obj_t *g_gponmac_drv;

void apollo_raw_gpon_eqd_write(uint32 value)
{
    uint32 data=0,min_delay1;
    uint32 eqd1,multi,intra;

	_apollo_wrap_reg_field_read(GPON_GTC_US_MIN_DELAYr,MIN_DELAY1f,&min_delay1);

    eqd1 = value + (min_delay1*16*8) + (g_gponmac_drv->dev->eqd_offset);
    multi = (eqd1 /GPON_REG_EQD_FRAME_LEN);
    intra = (eqd1-multi*GPON_REG_EQD_FRAME_LEN);

    _apollo_wrap_reg_field_set(GPON_GTC_US_EQDr,EQD1_MULTFRAMEf,&multi,&data);
	_apollo_wrap_reg_field_set(GPON_GTC_US_EQDr,EQD1_INFRAMEf,&intra,&data);

	_apollo_wrap_reg_write(GPON_GTC_US_EQDr,&data);

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"set eqd(%d) offset(%d) eqd1(%d) multi(%d) intra(%d)",value,(g_gponmac_drv->dev->eqd_offset),eqd1,multi,intra);
}

int32 apollo_raw_gpon_eqd_read(uint32 *pMultiFrame, uint32 *pIntraFrame)
{
    uint32 data;
    int32 ret;

    ret =  _apollo_wrap_reg_read(GPON_GTC_US_EQDr, &data);

    _apollo_wrap_reg_field_get(GPON_GTC_US_EQDr, EQD1_MULTFRAMEf, pMultiFrame, &data);
    _apollo_wrap_reg_field_get(GPON_GTC_US_EQDr, EQD1_INFRAMEf, pIntraFrame, &data);

    return ret;
}

void apollo_raw_gpon_laserTime_write(uint8 on, uint8 off)
{
	uint32 data=0, value;

	value = on;
	_apollo_wrap_reg_field_set(GPON_GTC_US_LASERr,LON_TIMEf,&value,&data);
	value = off;
	_apollo_wrap_reg_field_set(GPON_GTC_US_LASERr,LOFF_TIMEf,&value,&data);

	_apollo_wrap_reg_write(GPON_GTC_US_LASERr,&data);
}

int32 apollo_raw_gpon_laserTime_read(uint8 *on, uint8 *off)
{
    uint32 data,value;
	int32  ret;

    ret = _apollo_wrap_reg_read(GPON_GTC_US_LASERr,&data);

	_apollo_wrap_reg_field_get(GPON_GTC_US_LASERr,LON_TIMEf,&value,&data);
    *on  = (uint8)value;
    _apollo_wrap_reg_field_get(GPON_GTC_US_LASERr,LOFF_TIMEf,&value,&data);
    *off  = (uint8)value;

	return ret;
}

void apollo_raw_gpon_burstOverhead_write(uint8 rep, uint8 len, uint8 size, uint8 *oh)
{
    uint8 i;
    uint32 data=0, tmp;

    if (rep) {}

#if 1 /* change the repeat to the last byte before delimiter */
    tmp = (size-4)&0x0F;
#else
    tmp = rep&0x0F;
#endif
    _apollo_wrap_reg_field_set(GPON_GTC_US_BOH_CFGr, BOH_REPEATf, &tmp, &data);

    tmp = len;
    _apollo_wrap_reg_field_set(GPON_GTC_US_BOH_CFGr, BOH_LENGTHf, &tmp, &data);

    _apollo_wrap_reg_write(GPON_GTC_US_BOH_CFGr,&data);

    for(i=0;i<size;i++)
    {
        data = oh[i];
        _apollo_wrap_reg_array_write(GPON_GTC_US_BOH_DATAr, REG_ARRAY_INDEX_NONE, i, &data);
    }
}

int32 apollo_raw_gpon_usPloam_write(int32 urgent, uint8 *value)
{
    uint32 data, ind, bit, tmp;
    uint8  i;

    ind = 0;
    if(urgent)
    {
        tmp = GPON_REG_TYPE_GTC_US_PLOAM_URG;
        _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_TYPEf, &tmp, &ind);
        bit = PLM_URG_FULLf;
    }
    else
    {
        tmp = GPON_REG_TYPE_GTC_US_PLOAM_NRM;
        _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_TYPEf, &tmp, &ind);
        bit = PLM_NRM_FULLf;
    }
    tmp = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_ENQf, &tmp, &ind);
    _apollo_wrap_reg_write(GPON_GTC_US_PLOAM_INDr,&ind);

	_apollo_wrap_reg_field_read(GPON_GTC_US_PLOAM_INDr,bit,&data);
    if(data==GPON_REG_STATUS_CLEAR)
    {
        for(i=0;i<6;i++)
        {
            data = (value[2*i+0]<<GPON_REG_16BITS_HIGH)|(value[2*i+1]<<GPON_REG_16BITS_LOW);
            _apollo_wrap_reg_array_write(GPON_GTC_US_PLOAM_DATAr, REG_ARRAY_INDEX_NONE, i, &data);
        }
        tmp = GPON_REG_STATUS_SET;
        _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_ENQf, &tmp, &ind);
        _apollo_wrap_reg_write(GPON_GTC_US_PLOAM_INDr,&ind);
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_GPON_PLOAM_QUEUE_FULL;
    }
}

void apollo_raw_gpon_usAutoPloam_write(apollo_raw_gpon_usAutoPloamType_t type, uint8 *value)
{
    uint32 data, ind, tmp;
    uint8 i;

    ind = 0;
    if(type==GPON_REG_USAUTOPLOAM_SN)
    {
        tmp = GPON_REG_TYPE_GTC_US_PLOAM_SN;

    }
    else if(type==GPON_REG_USAUTOPLOAM_DYINGGASP)
    {
        tmp = GPON_REG_TYPE_GTC_US_PLOAM_DYINGGASP;
    }
	else
    {
        tmp = GPON_REG_TYPE_GTC_US_PLOAM_NOMSG;
    }
    _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_TYPEf, &tmp, &ind);

    tmp = GPON_REG_STATUS_CLEAR;
    _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_ENQf, &tmp, &ind);

    _apollo_wrap_reg_write(GPON_GTC_US_PLOAM_INDr,&ind);

    for(i=0;i<6;i++)
    {
        data = (value[2*i+0]<<GPON_REG_16BITS_HIGH)|(value[2*i+1]<<GPON_REG_16BITS_LOW);
        _apollo_wrap_reg_array_write(GPON_GTC_US_PLOAM_DATAr, REG_ARRAY_INDEX_NONE, i, &data);
    }

    tmp = GPON_REG_STATUS_SET;
    _apollo_wrap_reg_field_set(GPON_GTC_US_PLOAM_INDr, PLM_ENQf, &tmp, &ind);
    _apollo_wrap_reg_write(GPON_GTC_US_PLOAM_INDr,&ind);
}

void apollo_raw_gpon_usPloam_crcGen_enable(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_PLOAM_CFGr,PLM_US_CRC_GEN_ENf,&value);
}

int32 apollo_raw_gpon_usPloam_crcGen_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_PLOAM_CFGr,PLM_US_CRC_GEN_ENf,value);
}

void apollo_raw_gpon_usPloam_onuIdFilter_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_PLOAM_CFGr,PLM_US_ONUID_OVRD_ENf,&value);
}

int32 apollo_raw_gpon_usPloam_onuIdFilter_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_PLOAM_CFGr,PLM_US_ONUID_OVRD_ENf,value);
}
#if 1
void apollo_raw_gpon_usPloam_buf_flush_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_PLOAM_CFGr,PLM_FLUSH_BUFf,&value);
}

int32 apollo_raw_gpon_usPloam_buf_flush_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_PLOAM_CFGr,PLM_FLUSH_BUFf,value);
}
#endif
int32 apollo_raw_gpon_usGtc_miscCnt_read(apollo_raw_gpon_usGtc_pmMiscType_t idx, uint32 *pValue)
{
    int32 ret;

    RT_PARAM_CHK((idx>GPON_REG_USGTCPMMISC_GEM_BYTE_TX),RT_ERR_OUT_OF_RANGE);
    ret = _apollo_wrap_reg_field_write(GPON_GTC_US_MISC_CNTR_IDXr,MISC_IDXf,&idx);
    if(ret != RT_ERR_OK)
        return ret;

    return _apollo_wrap_reg_read(GPON_GTC_US_MISC_CNTR_STATr,pValue);
}

void apollo_raw_gpon_rdi_write(int32 value)
{
	_apollo_wrap_reg_field_write(GPON_GTC_US_RDIr,ONU_RDIf,&value);
}

int32 apollo_raw_gpon_rdi_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_RDIr,ONU_RDIf,value);
}

void apollo_raw_gpon_usSmalSstartProc_enable(int32 value)
{
	uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_PROC_MODEr,AUTO_PROC_SSTARTf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_usSmalSstartProc_enable_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_PROC_MODEr,AUTO_PROC_SSTARTf,value);
}

void apollo_raw_gpon_usSuppressLaser_enable(int32 value)
{
	uint32 tmp;

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_ENA;
	_apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);

    _apollo_wrap_reg_field_write(GPON_GTC_US_PROC_MODEr,OPTIC_AUTO_SUPRESS_DISf,&value);

	tmp = GPON_REG_PTN_GTC_US_WRPROTECT_DIS;
    _apollo_wrap_reg_write(GPON_GTC_US_WRITE_PROTECTr,&tmp);
}

int32 apollo_raw_gpon_usSuppressLaser_enable_read(int32 *value)
{
    return _apollo_wrap_reg_field_read(GPON_GTC_US_PROC_MODEr,OPTIC_AUTO_SUPRESS_DISf,value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_intr_read
 * Description:
 *      Read GPON GEM upstream interrupt indicator register
 * Input:
 *      reg_addr 	- void
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_intr_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GEM_US_INTR_DLTr, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_sdLong_check
 * Description:
 *      Indicate the Signal Detect valid more than 125us.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_sdLong_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, SD_VALID_LONG_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_sdDiff_check
 * Description:
 *      Indicate the Signal Detect and the TX Burst enable has
 *      the huge difference for the Transmit Optical Module.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_sdDiff_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, SD_DIFF_HUGE_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_reqDelay_check
 * Description:
 *      Indicate the GTC_US need request the next 125 us packets but
 *      the SWITCH also is busy on the last 125 us.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_reqDelay_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, REQUEST_DELAY_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_bcLess6_check
 * Description:
 *      Indicate the byte counter don't take less than 6 and
 *      as Empty TCONT by Switch
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_bcLess6_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BC_LESS6_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_errPLI_check
 * Description:
 *      Indicate the PLI mismatching with the input cycle from the Switch.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_errPLI_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, ERR_PLI_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_burstLarge_check
 * Description:
 *      Indicate the TM(SWITCH) has the larger burst bytes
 *      than the GTC indication. In this case, GEM module will
 *      insert IDLE until bank (125us) end.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_burstLarge_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BURST_TM_LARGER_GTC_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_bankTooMuch_check
 * Description:
 *      Indicate the 1k bytes Bank has too much data at the time
 *      of leave 800 cycles (bytes). It will force the signal buffer
 *      full indication to un-valid in order to let the Switch can
 *      send the next bank data. It means the Switch gives too many
 *      traffic from request and full at the last 800 cycles of band read.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_bankTooMuch_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BANK_TOO_MUCH_AT_END_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_bankRemain_check
 * Description:
 *      Indicate the Bank has the remained data after the 125us bank read.
 *      It means the Switch gives too many traffic from request.
 *      (Too many response times or too many bytes in one response, etc.)
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_bankRemain_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BANK_REMAIN_AFRD_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_bankOverflow_check
 * Description:
 *      Indicate Bank overflow. It means the Switch doesn¡¦t have the
 *      true back press machine.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_bankOverflow_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BANK_OVERFL_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUsIntr_bankUnderflow_check
 * Description:
 *      Indicate Bank underflow. It is as timeout of request to response
 *      of interface. Insert IDLE in this case.
 * Input:
 *      status 	    - GEM upstream interrupt indicator value
 * Output:
 *      None
 * Return:
 *      1 		    - indicator is happened
 *      0 		    - indicator is not happened
 * Note:
 */
uint32 apollo_raw_gpon_gemUsIntr_bankUnderflow_check(uint32 status)
{
    uint32 value=0;

    _apollo_wrap_reg_field_get(GPON_GEM_US_INTR_DLTr, BANK_UNDERFL_DLTf, &value, &status);

    return value;
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_mask_read
 * Description:
 *      Read the GEM upstream interrupt mask.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_mask_read(uint32 *pValue)
{
    return _apollo_wrap_reg_read(GPON_GEM_US_INTR_MASKr, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_mask_write
 * Description:
 *      Write the GEM upstream interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUs_mask_write(uint32 value)
{
    return _apollo_wrap_reg_write(GPON_GEM_US_INTR_MASKr,&value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_sdLong_write
 * Description:
 *      Turn on/off the SD long valid interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_sdLong_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, SD_VALID_LONG_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_sdDiff_write
 * Description:
 *      Turn on/off the SD huge difference interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_sdDiff_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, SD_DIFF_HUGE_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_reqDelay_write
 * Description:
 *      Turn on/off the request delay interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_reqDelay_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, REQUEST_DELAY_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_bcLess6_write
 * Description:
 *      Turn on/off the 'byte count less than 6' interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_bcLess6_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BC_LESS6_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_errPLI_write
 * Description:
 *      Turn on/off the error PLT interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_errPLI_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, ERR_PLI_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_burstLarge_write
 * Description:
 *      Turn on/off the burst large interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_burstLarge_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BURST_TM_LARGER_GTC_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_bankTooMuch_write
 * Description:
 *      Turn on/off the bank too much data interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_bankTooMuch_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BANK_TOO_MUCH_AT_END_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_bankRemain_write
 * Description:
 *      Turn on/off the bank remain data interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_bankRemain_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BANK_REMAIN_AFRD_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_bankOverflow_write
 * Description:
 *      Turn on/off the bank overflow interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_bankOverflow_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BANK_OVERFL_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUsrMask_bankUnderflow_write
 * Description:
 *      Turn on/off the bank underflow interrupt mask.
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUsrMask_bankUnderflow_write(int32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_INTR_MASKr, BANK_UNDERFL_Mf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_sdLong_read
 * Description:
 *      None
 * Input:
 *      value       - interrupt mask value
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_sdLong_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, SD_VALID_LONG_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_sdDiff_read
 * Description:
 *      Read the SD huge difference status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_sdDiff_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, SD_DIFF_HUGE_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_sdLong_read
 * Description:
 *      Read the request delay status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_reqDelay_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, REQUEST_DELAY_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_bcLess6_read
 * Description:
 *      Read the 'byte count less than 6' status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_bcLess6_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BC_LESS6_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_errPLI_read
 * Description:
 *      Read the error PLI status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_errPLI_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, ERR_PLI_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_burstLarge_read
 * Description:
 *      Read the burst large status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_burstLarge_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BURST_TM_LARGER_GTC_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_bankTooMuch_read
 * Description:
 *      Read the bank too much data status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_bankTooMuch_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BANK_TOO_INDUCH_AT_END_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_bankRemain_read
 * Description:
 *      Read the bank remain data status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_bankRemain_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BANK_REMAIN_AFRD_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_bankOverflow_read
 * Description:
 *      Read the bank overflow status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_bankOverflow_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BANK_OVERFL_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_bankUnderflow_read
 * Description:
 *      Read the bank underflow status.
 * Input:
 *      None
 * Output:
 *      pValue              - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_bankUnderflow_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_INTR_STSr, BANK_UNDERFL_INDf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_forceIdle_write
 * Description:
 *      Turn on/off force to send IDLE GEM only.
 * Input:
 *      value       - on/off value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUs_forceIdle_write(uint32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_PTI_CFGr, FS_GEM_IDLEf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_forceIdle_read
 * Description:
 *      Read the setting of force IDLE GEM.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_forceIdle_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_PTI_CFGr, FS_GEM_IDLEf,  pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_PTI_vector_write
 * Description:
 *      Set the PTI value vector mapping base on (OMCI,END_FRAG).
 * Input:
 *      pti_v0       - the PTI value base on (OMCI,END_FRAG)=(0,0)
 *      pti_v1       - the PTI value  base on (OMCI,END_FRAG)=(0,1)
 *      pti_v2       - the PTI value  base on (OMCI,END_FRAG)=(1,0)
 *      pti_v3       - the PTI value  base on (OMCI,END_FRAG)=(1,1)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 */
int32 apollo_raw_gpon_gemUs_PTI_vector_write(uint8 pti_v0, uint8 pti_v1, uint8 pti_v2, uint8 pti_v3)
{
    uint32 data=0;
    int32  ret;
    uint32 pti_value;

    pti_value = pti_v0&0x7;
    ret = _apollo_wrap_reg_field_set(GPON_GEM_US_PTI_CFGr, PTI_VECTOR0f, &pti_value, &data);
    if (ret != RT_ERR_OK)
        return ret;

    pti_value = pti_v1&0x7;
    ret = _apollo_wrap_reg_field_set(GPON_GEM_US_PTI_CFGr, PTI_VECTOR1f, &pti_value, &data);
    if (ret != RT_ERR_OK)
        return ret;

    pti_value = pti_v2&0x7;
    ret = _apollo_wrap_reg_field_set(GPON_GEM_US_PTI_CFGr, PTI_VECTOR2f, &pti_value, &data);
    if (ret != RT_ERR_OK)
        return ret;

    pti_value = pti_v3&0x7;
    ret = _apollo_wrap_reg_field_set(GPON_GEM_US_PTI_CFGr, PTI_VECTOR3f, &pti_value, &data);
    if (ret != RT_ERR_OK)
        return ret;

    return _apollo_wrap_reg_write(GPON_GEM_US_PTI_CFGr, &data);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_PTI_vector_read
 * Description:
 *      Read the setting of force IDLE GEM.
 * Input:
 *      None
 * Output:
 *      pPti_v0             - the PTI vector 0
 *      pPti_v1             - the PTI vector 1
 *      pPti_v2             - the PTI vector 2
 *      pPti_v3             - the PTI vector 3
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_PTI_vector_read(uint8 *pPti_v0, uint8 *pPti_v1, uint8 *pPti_v2, uint8 *pPti_v3)
{
    uint32 data=0;
    int32  ret;
    uint32 pti_value;

    ret = _apollo_wrap_reg_read(GPON_GEM_US_PTI_CFGr, &data);
    if (ret != RT_ERR_OK)
        return ret;

    _apollo_wrap_reg_field_get(GPON_GEM_US_PTI_CFGr, PTI_VECTOR0f, &pti_value, &data);
    *pPti_v0 = pti_value&0x7;
    _apollo_wrap_reg_field_get(GPON_GEM_US_PTI_CFGr, PTI_VECTOR1f, &pti_value, &data);
    *pPti_v1 = pti_value&0x7;
    _apollo_wrap_reg_field_get(GPON_GEM_US_PTI_CFGr, PTI_VECTOR2f, &pti_value, &data);
    *pPti_v2 = pti_value&0x7;
    _apollo_wrap_reg_field_get(GPON_GEM_US_PTI_CFGr, PTI_VECTOR3f, &pti_value, &data);
    *pPti_v3 = pti_value&0x7;

    return ret;
}

#if 0 /* GPON_GEM_US_OPT_CFG is removed in GPON_MAC_SWIO_r1.1 */
/* Function Name:
 *      apollo_raw_gpon_gemUs_txDis_write
 * Description:
 *      Set the use of TX disable for power saving.
 * Input:
 *      use_tx_dis  - specify to use TX disable or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_txDis_write(uint32 use_tx_dis)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_OPT_CFGr, USE_TX_OPT_DISABLEf, &use_tx_dis);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_txDis_read
 * Description:
 *      Read the setting of TX disable for power saving.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_txDis_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_CFGr, USE_TX_OPT_DISABLEf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_optBehindCycle_write
 * Description:
 *      Set the optical behind cycle.
 * Input:
 *      cycle   - specify the cycle value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_optBehindCycle_write(uint32 cycle)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_OPT_CFGr, OPT_BEHIND_CYCLESf, &cycle);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_optBehindCycle_read
 * Description:
 *      Read the setting of optical behind cycle.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_optBehindCycle_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_CFGr, OPT_BEHIND_CYCLESf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_optAheadCycle_write
 * Description:
 *      Set the optical ahead cycle.
 * Input:
 *      cycle   - specify the cycle value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_optAheadCycle_write(uint32 cycle)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_OPT_CFGr, OPT_AHEAD_CYCLESf, &cycle);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_optAheadCycle_read
 * Description:
 *      Read the setting of optical ahead cycle.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_optAheadCycle_read(uint32 *pValue)
{
    _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_CFGr, OPT_AHEAD_CYCLESf, pValue);
}
#endif

#if 0 /* GPON_GEM_US_OPT_SD will move to GTC_US */
/* Function Name:
 *      apollo_raw_gpon_gemUs_txSd_read
 * Description:
 *      Indicate the TX Signal Detect vaild.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of status
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_txSd_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_SDr, TX_OPT_SD_HVf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_txSdPolar_write
 * Description:
 *      Set the palarity setting of TX Signal Detect vaild.
 * Input:
 *      value       - specify the polarity value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_txSdPolar_write(uint32 value)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_OPT_SDr, TX_OPT_SD_POLARf, &value);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_txSdPolar_read
 * Description:
 *      Read the palarity setting of TX Signal Detect vaild.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_txSdPolar_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_SDr, TX_OPT_SD_POLARf, pValue);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_sdDiffCycle_write
 * Description:
 *      Set the setting of SD difference cycle.
 * Input:
 *      cycle       - specify the cycle value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_sdDiffCycle_write(uint32 cycle)
{
    return _apollo_wrap_reg_field_write(GPON_GEM_US_OPT_SDr, SD_DIFF_CYCLESf, &cycle);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_sdDiffCycle_read
 * Description:
 *      Read the setting of SD difference cycle.
 * Input:
 *      None
 * Output:
 *      pValue      - return value of setting
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_sdDiffCycle_read(uint32 *pValue)
{
    return _apollo_wrap_reg_field_read(GPON_GEM_US_OPT_SDr, SD_DIFF_CYCLESf, pValue);
}
#endif

/* Function Name:
 *      apollo_raw_gpon_gemUs_ethCnt_read
 * Description:
 *      Read the ether counter for local port.
 * Input:
 *      local_idx           - local port index
 * Output:
 *      pEth_cntr           - counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 */
int32 apollo_raw_gpon_gemUs_ethCnt_read(uint32 local_idx, uint32 *pEth_cntr)
{
    uint32 data;

    data = 0x80 | (local_idx & 0x7F);

    _apollo_wrap_reg_field_write(GPON_GEM_US_ETH_GEM_RX_CNTR_IDXr, ETH_GEM_RX_IDXf, &data);

    if(apollo_raw_gpon_indirect_wait(GPON_GEM_US_ETH_GEM_RX_CNTR_IDXr))
    {
		return _apollo_wrap_reg_read(GPON_GEM_US_ETH_GEM_RX_CNTR_STATr, pEth_cntr);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_gemCnt_read
 * Description:
 *      Read the ether counter for local port.
 * Input:
 *      local_idx           - local port index
 * Output:
 *      pGem_cntr           - counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 */
int32 apollo_raw_gpon_gemUs_gemCnt_read(uint32 local_idx, uint32 *pGem_cntr)
{
    uint32 data;

    data = (local_idx & 0x7F);

    _apollo_wrap_reg_field_write(GPON_GEM_US_ETH_GEM_RX_CNTR_IDXr, ETH_GEM_RX_IDXf, &data);

    if(apollo_raw_gpon_indirect_wait(GPON_GEM_US_ETH_GEM_RX_CNTR_IDXr))
    {
		return _apollo_wrap_reg_read(GPON_GEM_US_ETH_GEM_RX_CNTR_STATr, pGem_cntr);
    }
    else
    {
        return RT_ERR_GPON_INVALID_HANDLE;
    }
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_portCfg_write
 * Description:
 *      Set the mapping of local_idx and gem_port_id.
 * Input:
 *      cycle       - specify the cycle value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      Others              - Failed
 * Note:
 */
int32 apollo_raw_gpon_gemUs_portCfg_write(uint32 local_idx, uint32 gem_port_id)
{
    return _apollo_wrap_reg_array_field_write(GPON_GEM_US_PORT_MAPr,
                                 REG_ARRAY_INDEX_NONE,
                                 local_idx,
                                 PORT_CFG_DATAf,
                                 &gem_port_id);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_portCfg_read
 * Description:
 *      Read the mapping of local_idx and gem_port_id.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 */
int32 apollo_raw_gpon_gemUs_portCfg_read(uint32 local_idx, uint32 *pGem_port_id)
{
    return _apollo_wrap_reg_array_field_read(GPON_GEM_US_PORT_MAPr,
                                REG_ARRAY_INDEX_NONE,
                                local_idx,
                                PORT_CFG_DATAf,
                                pGem_port_id);
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_dataByteCnt_read
 * Description:
 *      Read the data byte counter for local port.
 * Input:
 *      local_idx           - local port index
 * Output:
 *      pByte_cntr          - counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 */
int32 apollo_raw_gpon_gemUs_dataByteCnt_read(uint32 local_idx, uint64 *pByte_cntr)
{
    int32 ret;

    ret = _apollo_wrap_reg_array_read(GPON_GEM_US_BYTE_STATr,
                         REG_ARRAY_INDEX_NONE,
                         local_idx,
                         (uint32 *)pByte_cntr);
    *pByte_cntr = ((*pByte_cntr >> 32) & 0xFFFFFFFF) |
                  ((*pByte_cntr & 0xFFFFFFFF) << 32);

    return ret;
}

/* Function Name:
 *      apollo_raw_gpon_gemUs_idleByteCnt_read
 * Description:
 *      Read the IDLE byte counter for local TCONT.
 * Input:
 *      local_idx           - local TCONT index
 * Output:
 *      pIdle_cntr          - counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 */
int32 apollo_raw_gpon_gemUs_idleByteCnt_read(uint32 local_idx, uint64 *pIdle_cntr)
{
    int32 ret;

    ret = _apollo_wrap_reg_array_read(TCONT_IDLE_BYTE_STATr,
                         REG_ARRAY_INDEX_NONE,
                         local_idx,
                         (uint32 *)pIdle_cntr);
    *pIdle_cntr = ((*pIdle_cntr >> 32) & 0xFFFFFFFF) |
                  ((*pIdle_cntr & 0xFFFFFFFF) << 32);

    return ret;
}



#if defined(OLD_FPGA_DEFINED)
#define PKT_GEN_W_OFFSET 0xC000
#define PKT_GEN_R_OFFSET 0x8000

int32 packet_gen_read(uint32 address, uint16 *pValue)
{
    io_mii_phy_reg_write(11, 0, PKT_GEN_R_OFFSET|(address&0xFFF));
    io_mii_phy_reg_read(11, 2, pValue);

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"packet_gen_read addr=0x%x, value=0x%04x\n\r", address, *pValue);
}

int32 packet_gen_write(uint32 address, uint16 data)
{
    uint16 value;
    io_mii_phy_reg_write(11, 1, data);
    io_mii_phy_reg_write(11, 0, PKT_GEN_W_OFFSET|(address&0xFFF));

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"packet_gen_write addr=0x%x, data=0x%04x\n\r", address, data);

    packet_gen_read(address, &value);
}




#define PKT_GEN_ADDR_BUF_BASE           0x0
#define PKT_GEN_ADDR_BUF_OFFSET         5

#define PKT_GEN_ADDR_TCONT_BASE         0x40
#define PKT_GEN_ADDR_TCONT_OFFSET       1
#define PKT_GEN_FIELD_TCONT_OFFSET              11
#define PKT_GEN_FIELD_BC_OFFSET                 0

#define PKT_GEN_ADDR_GEM_BASE           0x41
#define PKT_GEN_ADDR_GEM_OFFSET         1
#define PKT_GEN_FIELD_EN_OFFSET                 10
#define PKT_GEN_FIELD_OMCI_OFFSET               9
#define PKT_GEN_FIELD_GEM_OFFSET                0

#define PKT_GEN_ADDR_RAM_BUF_BASE_1     0x46
#define PKT_GEN_ADDR_RAM_BUF_BASE_2     0x47
#define PKT_GEN_ADDR_RAM_CTRL_BASE      0x48
#define PKT_GEN_FIELD_RAM_RES_OFFSET            8
#define PKT_GEN_FIELD_RAM_DELAY_OFFSET          0


#define PKT_GEN_REQ_DELAY               0x2

#define ITEM_BUF_ADDR(__item)       (PKT_GEN_ADDR_BUF_BASE   | (__item<<PKT_GEN_ADDR_BUF_OFFSET))
#define ITEM_TCONT_ADDR(__item)     (PKT_GEN_ADDR_TCONT_BASE | (__item<<PKT_GEN_ADDR_TCONT_OFFSET))
#define ITEM_GEM_ADDR(__item)       (PKT_GEN_ADDR_GEM_BASE   | (__item<<PKT_GEN_ADDR_GEM_OFFSET))


int32 apollo_raw_gpon_pktGen_cfg_write(uint32 item, uint32 tcont, uint32 len, uint32 gem, int32 omci)
{
    int32  enable;
    uint16 data;

    /* set TCONT */
    data = (((tcont&0x1F)<<PKT_GEN_FIELD_TCONT_OFFSET) |
            ((len&0x3FF)<<PKT_GEN_FIELD_BC_OFFSET));
    packet_gen_write(ITEM_TCONT_ADDR(item), data);

    /* set GEM */
    if(gem == 0xFF)
        enable = 0;
    else
        enable = 1;

    data = (((gem&0x7F)<<PKT_GEN_FIELD_GEM_OFFSET) |
            ((omci&0x1)<<PKT_GEN_FIELD_OMCI_OFFSET) |
            ((enable&0x1)<<PKT_GEN_FIELD_EN_OFFSET));
    packet_gen_write(ITEM_GEM_ADDR(item), data);

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"pktGen cfg: item=%d, tcont=%d, len=%d, gem=%d, omci=%d\n\r",item,tcont,len,gem,omci);

    return RT_ERR_OK;
}

int32 apollo_raw_gpon_pktGen_buf_write(uint32 item, uint8 *buf, uint32 len)
{
    int32 i;
    uint16 data;

    /* set buf */
    if((item == 0) || (item == 1))
    {
        for(i=0;i<len;i+=2)
        {
            data = (buf[i]) | (buf[i+1] << 8);
            packet_gen_write(((ITEM_BUF_ADDR(item))+(i/2)), data);
        }
    }
    else if(item == 2)
    {
        data = ((1<<PKT_GEN_FIELD_RAM_RES_OFFSET) |
                (PKT_GEN_REQ_DELAY<<PKT_GEN_FIELD_RAM_DELAY_OFFSET));
        packet_gen_write(PKT_GEN_ADDR_RAM_CTRL_BASE, data);

        for(i=0;i<len;i+=4)
        {
            data = (buf[i]<<8) | (buf[i+1]);
            packet_gen_write(PKT_GEN_ADDR_RAM_BUF_BASE_1, data);
            data = (buf[i+2]<<8) | (buf[i+3]);
            packet_gen_write(PKT_GEN_ADDR_RAM_BUF_BASE_2, data);
        }
    }

    GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"pktGen buf: \n\r");
    for(i=0;i<len;i++)
        GPON_OS_Log(GPON_LOG_LEVEL_NORMAL,"%02x-",buf[i]);

    return RT_ERR_OK;
}
#endif /* OLD_FPGA_DEFINED */
