/*
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology
 * and University of Helsinki.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * @author Samu Varjonen
 */

#include <errno.h>
#include <stdlib.h> 
#include <string.h>

#include "constants.h"
#include "debug.h"
#include "ife.h"
#include "pdu.h"
#include "tools.h"

/** Length of the common parts in an OAM PDU,
 * Meg Level / version, opcode, flags TLV offset */
#define OAM_COMMON_HEADER_LENGTH 4 

/**
 * Builds the CCM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu        Pointer to the source PDU struct
 * @param dst_mepid  MEP ID of the destination used to match the statistics 
 *                   the correct entity
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_ccm(struct oam_pdu_common * common_pdu,
                             const struct oam_pdu_ccm *const pdu, 
                             uint16_t dst_mepid) {
    int                        length       = 0;
    int                        total_length = OAM_COMMON_HEADER_LENGTH; 
    unsigned char             *ptr          = (unsigned char *)common_pdu;
    struct oam_entity_stat    *found        = NULL;

    ptr = ptr + total_length;
    
    length = sizeof(pdu->seq);
    memcpy(ptr, &pdu->seq, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->mepid); 
    memcpy(ptr, &pdu->mepid, length);
    total_length += length;
    ptr = ptr + length;    

    /* MEG ID */
    length = sizeof(pdu->meg_reserved);
    memcpy(ptr, &pdu->meg_reserved, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->meg_format); 
    memcpy(ptr, &pdu->meg_format, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->meg_length); 
    memcpy(ptr, &pdu->meg_length, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->meg_id); 
    memcpy(ptr, &pdu->meg_id, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->meg_unused);
    memset(ptr, 0, length);
    total_length += length;
    ptr = ptr + length;

    /* Counters */
    length = sizeof(pdu->txfcf); 
    memcpy(ptr, &pdu->txfcf, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->rxfcb); 
    memcpy(ptr, &pdu->rxfcb, length);
    total_length += length;
    ptr = ptr + length;   

    /* Find the other so the counter can be saved */ 
    found = oam_find(htons(dst_mepid));
    if (found) {
        length = sizeof(pdu->txfcf); 
        memcpy(ptr, &found->ccm_txfcf_tp, length);
        /* Store what we are going to send as TX (debugging) */
        memcpy(&found->ccm_my_txfcf, &pdu->txfcf, length);
        total_length += length;
        ptr = ptr + length;
    } else {
        /* add the empty field to the packet  */
        total_length += length;
        ptr = ptr + length;
    }
    
    /* Reserved (0) */
    length = sizeof(pdu->reserved);
    memset(ptr, 0, length);
    total_length += length;
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;
    
    return total_length;
}

/**
 * Builds the LBM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_lbm(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_lbm_no_data *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->tid_seq); 
    memcpy(ptr, &pdu->tid_seq, length);
    total_length += length;
    ptr = ptr + length;     

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the LBR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_lbr(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_lbr_no_data *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->tid_seq); 
    memcpy(ptr, &pdu->tid_seq, length);
    total_length += length;
    ptr = ptr + length;     

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the LTM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_ltm_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_ltm_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->transid); 
    memcpy(ptr, &pdu->transid, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->ttl); 
    memcpy(ptr, &pdu->ttl, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->original_mac); 
    memcpy(ptr, &pdu->original_mac, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->target_mac); 
    memcpy(ptr, &pdu->target_mac, length);
    total_length += length;
    ptr = ptr + length;

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_ltm_add_tlv REMEMBER End-TLV */

/**
 * Builds the LTR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_ltr_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_ltr_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->transid); 
    memcpy(ptr, &pdu->transid, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->ttl); 
    memcpy(ptr, &pdu->ttl, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->relay_action); 
    memcpy(ptr, &pdu->relay_action, length);
    total_length += length;
    ptr = ptr + length;

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_ltr_add_tlv REMEMBER End-TLV */

/**
 * Builds the AIS PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_ais(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_ais *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr          = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the LCK PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_lck(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_lck *const pdu) {
    int            length       = 0;
    int            total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr          = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the TST PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_tst_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_tst_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->seq); 
    memcpy(ptr, &pdu->seq, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_tst_add_tlv REMEMBER End-TLV */
 
/**
 * Builds the MCC PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 *
 * @note Remember to add end-TLV.
 * @see oam_build_end_tlv
 * @see oam_add_mepid_tlv
 */ 
static int oam_build_pdu_mcc_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_mcc_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->oui); 
    memcpy(ptr, &pdu->oui, length) ;
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->subopcode); 
    memcpy(ptr, &pdu->subopcode, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/**
 * Builds the SLM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_slm(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_slm_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;
    uint32_t turn_it_around;
    
    ptr = ptr + total_length;

    length = sizeof(pdu->source_mepid); 
    memcpy(ptr, &pdu->source_mepid, length);
    total_length += length;
    ptr = ptr + length;    

    /* Zero-out responder MEP ID */
    length = sizeof(pdu->target_mepid);
    memset(ptr, 0, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->test_id); 
    memcpy(ptr, &pdu->test_id, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txfcf);
    turn_it_around = htonl(pdu->txfcf);
    memcpy(ptr, &turn_it_around, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->reserved_txfcb); 
    memcpy(ptr, &pdu->reserved_txfcb, length);
    total_length += length;
    ptr = ptr + length;

    return total_length;
}

/**
 * Builds the SLR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_slr(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_slr_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;
    uint32_t turn_it_around;

    ptr = ptr + total_length;

    length = sizeof(pdu->source_mepid); 
    memcpy(ptr, &pdu->source_mepid, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->target_mepid); 
    memcpy(ptr, &pdu->target_mepid, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->test_id); 
    memcpy(ptr, &pdu->test_id, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txfcf); 
    memcpy(ptr, &pdu->txfcf, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->txfcb); 
    turn_it_around = htonl(pdu->txfcb);
    memcpy(ptr, &turn_it_around, length);
    total_length += length;
    ptr = ptr + length;

    return total_length;
}

/**
 * Builds the LMM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_lmm(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_lmm *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->txfcf); 
    memcpy(ptr, &pdu->txfcf, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->reserved_rxfcf); 
    memcpy(ptr, &pdu->reserved_rxfcf, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->reserved_txfcb); 
    memcpy(ptr, &pdu->reserved_txfcb, length);
    total_length += length;
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the LMR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_lmr(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_lmr *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->txfcf); 
    memcpy(ptr, &pdu->txfcf, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->rxfcf); 
    memcpy(ptr, &pdu->rxfcf, length);
    total_length += length;
    ptr = ptr + length;

    length = sizeof(pdu->txfcb); 
    memcpy(ptr, &pdu->txfcb, length);
    total_length += length;
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the 1DM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_1dm(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_1dm *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->txtimestampf_sec); 
    memcpy(ptr, &pdu->txtimestampf_sec, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txtimestampf_nano); 
    memcpy(ptr, &pdu->txtimestampf_nano, length);
    total_length += length;
    ptr = ptr + length;     

    /* Reserved (0) */
    length = sizeof(pdu->reserved);
    memset(ptr, 0, length);
    total_length += length; 
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the DMM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_dmm(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_dmm *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->txtimestampf_sec); 
    memcpy(ptr, &pdu->txtimestampf_sec, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txtimestampf_nano); 
    memcpy(ptr, &pdu->txtimestampf_nano, length);
    total_length += length;
    ptr = ptr + length;    
 
    /* Reserved RXtimestampf (0) */
    length = sizeof(pdu->reserved_dmm_secs);
    memset(ptr, 0, length);
    total_length += length;

    /* Reserved RXtimestampf (0) */
    length = sizeof(pdu->reserved_dmm_nano);
    memset(ptr, 0, length);
    total_length += length;
    
    /* Reserved TXtimestampb (0) */
    length = sizeof(pdu->reserved_dmr_secs);
    memset(ptr, 0, length);
    total_length += length;

    /* Reserved TXtimestampb (0) */
    length = sizeof(pdu->reserved_dmr_nano);
    memset(ptr, 0, length);
    total_length += length;

    /* Reserved (0) */
    length = sizeof(pdu->reserved);
    memset(ptr, 0, length);
    total_length += length;
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the DMR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_dmr(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_dmr *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->txtimestampf_sec); 
    memcpy(ptr, &pdu->txtimestampf_sec, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txtimestampf_nano); 
    memcpy(ptr, &pdu->txtimestampf_nano, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->rxtimestampf_sec); 
    memcpy(ptr, &pdu->rxtimestampf_sec, length);
    total_length += length;
    ptr = ptr + length;    

    length = sizeof(pdu->rxtimestampf_nano); 
    memcpy(ptr, &pdu->rxtimestampf_nano, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txtimestampb_sec); 
    memcpy(ptr, &pdu->txtimestampb_sec, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->txtimestampb_nano); 
    memcpy(ptr, &pdu->txtimestampb_nano, length);
    total_length += length;
    ptr = ptr + length;    

    /* Reserved (0) */
    length = sizeof(pdu->reserved);
    memset(ptr, 0, length);
    total_length += length;
    ptr = ptr + length;

    /* End TLV (0)*/
    length = sizeof(pdu->endtlv);
    memset(ptr, 0, length);
    total_length += length;

    return total_length;
}

/**
 * Builds the EXM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_exm_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_exm_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->oui); 
    memcpy(ptr, &pdu->oui, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->subopcode); 
    memcpy(ptr, &pdu->subopcode, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_exm_add_tlv REMEMBER End-TLV */

/**
 * Builds the EXR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_exr_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_exr_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->oui); 
    memcpy(ptr, &pdu->oui, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->subopcode); 
    memcpy(ptr, &pdu->subopcode, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_exr_add_tlv REMEMBER End-TLV */

/**
 * Builds the VSM PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_vsm_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_vsm_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->oui); 
    memcpy(ptr, &pdu->oui, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->subopcode); 
    memcpy(ptr, &pdu->subopcode, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_vsm_add_tlv REMEMBER End-TLV */

/** 
 * Builds the VSR PDU's non-common parts.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu Pointer to the source PDU struct 
 *
 * @return Length of the built PDU in bytes
 */ 
static int oam_build_pdu_vsr_no_end(struct oam_pdu_common * common_pdu, 
                      const struct oam_pdu_vsr_no_end *const pdu) {
    int length       = 0;
    int total_length = OAM_COMMON_HEADER_LENGTH;
    unsigned char *ptr = (unsigned char *)common_pdu;

    ptr = ptr + total_length;

    length = sizeof(pdu->oui); 
    memcpy(ptr, &pdu->oui, length);
    total_length += length;
    ptr = ptr + length;     

    length = sizeof(pdu->subopcode); 
    memcpy(ptr, &pdu->subopcode, length);
    total_length += length;
    ptr = ptr + length;     

    /* No End-TLV as there will be additional TLVs */

    return total_length;
}

/* XXTODO oam_build_pdu_vsr_add_tlv REMEMBER End-TLV */

/**
 * Initializes a common PDU for all the OAM PDUs
 *
 * @param common_pdu A pointer to the structure to be initialized.
 */
void oam_init_pdu(struct oam_pdu_common *const common_pdu) {
     memset (common_pdu, 0, OAM_MAX_PACKET);
}
 
/**
 * Mallocs memory for the common PDU for all the OAM PDUs.
 *
 * @return Zeroed oam_pdu on success, NULL else.
 */
struct oam_pdu_common *oam_alloc_pdu() {
     struct oam_pdu_common *ptr = NULL;
     
     ptr = calloc(1, OAM_MAX_PACKET);
     return ptr;
}

/**
 * This function adds the end-TLV to the end. In other words
 * this function just clears the end.
 *
 * @param pdu PDU to where the end-TLV is added
 * @param length Total length this far
 *
 * @return Total length after the addition
 *  
 * @note !remember! to do this always after building the pdu
 *       and the addition of the needed tlvs.
 */
int oam_build_end_tlv(struct oam_pdu_common *pdu, int length)
{
    const int end_tlv_length = 1;
    unsigned char *ptr = (unsigned char*)pdu;
    ptr = ptr + length;
    memset(ptr, 0, end_tlv_length);
    return length + end_tlv_length;
}

/**
 * What to add to flags so the bit 8 is up.
 */
#define OAM_RDI_BIT_ADDITION 128;

/**
 * This function acts as an indirection for the individual PDU build functions.
 *
 * @param common_pdu Pointer to the destination buffer the BDU is built
 * @param pdu        Pointer to the source PDU struct
 * @param dst_mepid  MEP ID of the destination used to match the statistics 
 *                   the correct entity
 *
 * @return Length of the built PDU in bytes, zero or negative on error.
 *
 * @note The dst_mepid is only used in CCM so it can be zero for all else.
 */
int oam_build_pdu(struct oam_pdu_common * common_pdu, 
                  const struct oam_pdu_common *const pdu,
                  uint16_t dst_mepid) {
    int                       err;
    extern struct oam_entity *entity;
    extern struct oam_db    *oam_control_info;

    common_pdu->mel_ver = (entity->meg_level << 5);
    common_pdu->opcode = pdu->opcode;
    common_pdu->flags = pdu->flags;

    switch (pdu->opcode) {
    case OAM_OPCODE_CCM:
        /* RDI */
        if (oam_control_info->rdi == OAM_ALARM_ON) {
            common_pdu->flags += OAM_RDI_BIT_ADDITION; 
        }
        common_pdu->tlv_offset = OAM_TLVOFFSET_CCM;
        err = oam_build_pdu_ccm(common_pdu,
                                (const struct oam_pdu_ccm *const) pdu,
                                dst_mepid);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU CCM\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LBM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LBM;
        err = oam_build_pdu_lbm(common_pdu,
                                (const struct oam_pdu_lbm_no_data *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LBM\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LBR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LBR;
        err = oam_build_pdu_lbr(common_pdu,
                                (const struct oam_pdu_lbr_no_data *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LBR\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LTM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LTM;
        err = oam_build_pdu_ltm_no_end(common_pdu,
                                (const struct oam_pdu_ltm_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LTM\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LTR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LTR;
        err = oam_build_pdu_ltr_no_end(common_pdu,
                                (const struct oam_pdu_ltr_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LTR\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_AIS:
        common_pdu->tlv_offset = OAM_TLVOFFSET_AIS;
        err = oam_build_pdu_ais(common_pdu,
                                (const struct oam_pdu_ais *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU AIS\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LCK:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LCK;
        err = oam_build_pdu_lck(common_pdu,
                                (const struct oam_pdu_lck *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LCK\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_TST:
        common_pdu->tlv_offset = OAM_TLVOFFSET_TST;
        err = oam_build_pdu_tst_no_end(common_pdu,
                                (const struct oam_pdu_tst_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU TST\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LAPS:
        OAM_DEBUG("APS Out-of-scope unimplemented\n");
        err = -1;
        break;
    case OAM_OPCODE_RAPS:
        OAM_DEBUG("APS Out-of-scope unimplemented\n");
        err = -1;
        break;
    case OAM_OPCODE_MCC:
        common_pdu->tlv_offset = OAM_TLVOFFSET_MCC;
        err = oam_build_pdu_mcc_no_end(common_pdu, 
                                (const struct oam_pdu_mcc_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU MCC\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_SLM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_SLM;
        err = oam_build_pdu_slm(common_pdu,
                                (const struct oam_pdu_slm_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU SLM %d\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_SLR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_SLR;
        err = oam_build_pdu_slr(common_pdu,
                                (const struct oam_pdu_slr_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU SLR %d\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LMM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LMM;
        err = oam_build_pdu_lmm(common_pdu,
                                (const struct oam_pdu_lmm *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LMM %d\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_LMR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_LMR;
        err = oam_build_pdu_lmr(common_pdu,
                                (const struct oam_pdu_lmr *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU LMR\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_1DM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_1DM;
        err = oam_build_pdu_1dm(common_pdu,
                                (const struct oam_pdu_1dm *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU 1DM\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_DMM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_DMM;
        err = oam_build_pdu_dmm(common_pdu,
                                (const struct oam_pdu_dmm *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU DMM\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_DMR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_DMR;
        err = oam_build_pdu_dmr(common_pdu,
                                (const struct oam_pdu_dmr *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU DMR\n");
            err = -1;
        }
        break;
    case OAM_OPCODE_EXM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_EXM;
        err = oam_build_pdu_exm_no_end(common_pdu,
                                (const struct oam_pdu_exm_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU EXM\n");
            err = -1;
        }
        break; 
    case OAM_OPCODE_EXR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_EXR;
        err = oam_build_pdu_exr_no_end(common_pdu,
                                (const struct oam_pdu_exr_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU EXR\n");
            err = -1;
        }
        break; 
    case OAM_OPCODE_VSR:
        common_pdu->tlv_offset = OAM_TLVOFFSET_VSR;
        err = oam_build_pdu_vsr_no_end(common_pdu,
                                (const struct oam_pdu_vsr_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU VSR");
            err = -1;
        }
        break;
    case OAM_OPCODE_VSM:
        common_pdu->tlv_offset = OAM_TLVOFFSET_VSM;
        err = oam_build_pdu_vsm_no_end(common_pdu,
                                (const struct oam_pdu_vsm_no_end *const) pdu);
        if (err < 1) {
            OAM_DEBUG("Failed to build PDU VSM");
            err = -1;
        }
        break;
    default:
        OAM_INFO("Unknown PDU type (%d)\n", pdu->opcode);
        err = -1;
        break;
    }
    return err;
}

