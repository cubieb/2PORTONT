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

#ifndef OAM_PDU_H
#define OAM_PDU_H

#include <stdint.h>

#define OAM_MAX_PACKET 1510 /**< Maximum size of the Ethernet packet */

/**
 * Struct describing the common parts of all the OAM PDUs
 *
 * @note Priority and drop eligibility are conveyed in frames.
 */
struct oam_pdu_common {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
} __attribute__ ((packed));

/**
 * Continuity check PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |RDI|    Reserved   |   Period  |
 * +---+---------------+-----------+
 * </pre>
 *
 * RDI, Bit 8 is set to 1 to indicate RDI, otherwise 0, Reserved (0),
 * Period, bits 3 to 1 are set according to group ccm_period.
 */
struct oam_pdu_ccm {
     uint8_t           mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t           opcode;     /**< Operation code indicating the PDU type */
     uint8_t           flags;      /**< Flags dependent on the used PDU type */
     uint8_t           tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint32_t          seq;        /**< Set to all-ZEROes */
     uint16_t          mepid;      /**< 13 bit int identifying the transmitting MEP (bits 16 to 14 are set to zero) */
     /* MEG ID */
     uint8_t           meg_reserved; /**< Reserved (01)*/
     uint8_t           meg_format; /**< Format (for ICC its 32) */
     uint8_t           meg_length; /**< Length (for ICC its 13) */
     uint8_t           meg_id[13]; /**< MEG ID */
     uint8_t           meg_unused[32]; /**< all-ZEROes */
     uint32_t          txfcf;      /**< Number of transmitted frames toward peer MEP (0s when not in use) */
     uint32_t          rxfcb;      /**< Number of received frames from peer MEP (0s when not in use) */
     uint32_t          txfcb;      /**< Value of the txfcf in the last CCM (0s when not in use) */
     uint32_t          reserved;   /**< Reserved (0) */
     uint8_t           endtlv;     /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Loopback message PDU. Does not contain the optional field data.
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 */ 
struct oam_pdu_lbm_no_data {
     uint8_t  mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;     /**< Operation code indicating the PDU type */
     uint8_t  flags;      /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint32_t tid_seq;    /**< Transaction ID/Sequence number */
     uint8_t  endtlv;     /**< End TLV (0) */
} __attribute__ ((packed));

/* Tracker ticket 35 */

/**
 * Loopback response PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 */
struct oam_pdu_lbr_no_data {
     uint8_t  mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;     /**< Operation code indicating the PDU type */
     uint8_t  flags;      /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint32_t tid_seq;    /**< Transaction ID/Sequence number (copy from lbm)*/    
     uint8_t  endtlv;     /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Link trace message PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 *
 * @note No end-TLV (0) here it is handled in the builder because of the additional TLVs.
 * 
 * @see struct oam_ltm_egress_tlv
 */
struct oam_pdu_ltm_no_end {
     uint8_t  mel_ver;         /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;          /**< Operation code indicating the PDU type */
     uint8_t  flags;           /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset;      /**< Offset in octets to the next OAM PDU */
     uint32_t transid;         /**< Transaction ID */
     uint8_t  ttl;             /**< Time-to-live */
     uint8_t  original_mac[6]; /**< Original MAC */
     uint8_t  target_mac[6];   /**< Target MAC */
     /* TLVs */
     /* End TLV */
} __attribute__ ((packed));

/**
 * Contains the identification of the initiating MEP or responder relaying modified frame.
 *
 * @see struct oam_ltm_pdu
 */
struct oam_ltm_egress_tlv {
     uint8_t type;      /**< Type */
     uint8_t length;    /**< Length */
     uint8_t egress_id; /**< Egress identifier */
} __attribute__ ((packed));

/**
 * Link trace response PDU
 *
 * <pre>
 * Flags format: 
 *      8        7           6         5   4   3   2   1
 * +--------+--------+--------------+--------------------+
 * | Hwonly | Fwdyes | Terminal MEP |     Reserved (0)   |
 * +--------+--------+--------------+--------------------+
 * </pre>
 *
 * @note Remember! to put the end TLV after the TLVs
 *
 * @see struct oam_ltr_pdu and struct oam_ltr_egress_tlv, struct oam_ltr_reply_ingress_tlv, and struct oam_ltr_reply_egress_tlv
 */
struct oam_pdu_ltr_no_end {
     uint8_t  mel_ver;       /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;       /**< Operation code indicating the PDU type */
     uint8_t  flags;        /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset;   /**< Offset in octets to the next OAM PDU */
     uint32_t transid;      /**< Transaction ID */
     uint8_t  ttl;          /**< Time-to-live */
     uint8_t  relay_action; /**< Relay action */
     /* TLVs */
     /* End TLV */
} __attribute__ ((packed));

/**
 * Contains the identification of the initiating MEP and the the responder
 * that transmitter this LTR.
 *
 * @see struct oam_ltr_pdu, struct oam_ltr_reply_ingress_tlv, and struct oam_ltr_reply ingress_tlv
 */
struct oam_ltr_egress_tlv {
     uint8_t type;           /**< Type */
     uint8_t length;         /**< Length */
     uint8_t last_egress_id; /**< Egress identifier */
     uint8_t next_egress_id; /**< Egress identifier */
} __attribute__ ((packed));

/**
 * Contains the identification of the MEP at the ingress port.
 *
 * @see struct oam_ltr_pdu, struct oam_ltr_egress_tlv, and struct oam_ltr_reply_egress_tlv
 */
struct oam_ltr_reply_ingress_tlv {
     uint8_t type;           /**< Type */
     uint8_t length;         /**< Length */
     uint8_t ingress_action; /**< Ingress action */
     uint8_t ingress_mac;     /**< Ingress MAC */
} __attribute__ ((packed));

/**
 * Contains the identification of the MEP at the egress port.
 *
 * @see struct oam_ltr_pdu, struct oam_ltr_egress_tlv, and struct oam_ltr_ingress_tlv
 */
struct oam_ltr_reply_egress_tlv {
     uint8_t type;          /**< Type */
     uint8_t length;        /**< Length */
     uint8_t egress_action; /**< Type */
     uint8_t egress_mac;     /**< Egress MAC */
} __attribute__ ((packed));

/**
 * Alarm indication signal PDU
 *
 * <pre>
 * Flags format: 
 *   8  7  6   5   4   3   2   1
 * +-----------------+-----------+
 * |   Reserved (0)  |   Period  |
 * +-----------------+-----------+
 * </pre>
 */
struct oam_pdu_ais {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU (0)*/
     uint8_t endtlv;      /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Ethernet locked signal PDU
 *
 * <pre>
 * Flags format: 
 *   8  7  6   5   4   3   2   1
 * +-----------------+-----------+
 * |   Reserved (0)  |   Period  |
 * +-----------------+-----------+
 * </pre>
 */
struct oam_pdu_lck {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU (0) */
     uint8_t endtlv;       /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Test PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_tst_no_end {
     uint8_t  mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;     /**< Operation code indicating the PDU type */
     uint8_t  flags;      /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint32_t seq;        /**< The sequence number incremented for successive TST PDUs */
     /* DATA TLV or TEST TLV */
     /* End TLV */
} __attribute__ ((packed));

/**
 * Automatic protection swithching PDU 
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 *
 * @note !!OUTSIDE-OF-SCOPE!! is not used
 */
struct oam_pdu_aps {
     uint8_t  mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;     /**< Operation code indicating the PDU type */
     uint8_t  flags;      /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint32_t aps_data;   /**< Outside-of-scope */
     uint8_t  endtlv;     /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Maintenance communication channel PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_mcc_no_end {
     uint8_t mel_ver;    /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;     /**< Operation code indicating the PDU type */
     uint8_t flags;      /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset; /**< Offset in octets to the next OAM PDU */
     uint8_t oui[3];     /**< Organizationally unique identifier defines the subopcodes and TLVs */
     uint8_t subopcode;  /**< sub operation code indicating the TLV type */
     /* TLVs */
     /* End TLV (0) */
} __attribute__ ((packed));

/**
 * Frame loss measurement message PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 */
struct oam_pdu_lmm {
     uint8_t  mel_ver;        /**< 3 bit MEL and 5 bit version */
     uint8_t  opcode;         /**< Operation code indicating the PDU type */
     uint8_t  flags;          /**< Flags dependent on the used PDU type */
     uint8_t  tlv_offset;     /**< Offset in octets to the next OAM PDU */ 
     uint32_t txfcf;          /**< Number of transmitted frames toward peer MEP */
     uint32_t reserved_rxfcf; /**< Reserved for RxFCf in LMR, set to all ZEROEs*/
     uint32_t reserved_txfcb; /**< Reserved for TxFCb in LMR, set to all ZEROEs*/
     uint8_t  endtlv;         /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Frame loss measurement response PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 *
 * (copied from the LMM)
 * </pre>
 */
struct oam_pdu_lmr {
    uint8_t  mel_ver;    /**< 3 bit MEL and 5 bit version */
    uint8_t  opcode;     /**< Operation code indicating the PDU type */
    uint8_t  flags;      /**< Flags dependent on the used PDU type */
    uint8_t  tlv_offset; /**< Offset in octets to the next OAM PDU */
     /**
      *  Number of transmitted frames toward peer MEP (0s when not in use) */
    uint32_t txfcf;     
    /**
     * Number of received frames from peer MEP (0s when not in use) */
    uint32_t rxfcf;      
    /**
     * Value of the txfcf in the last CCM (0s when not in use) */
    uint32_t txfcb;      
    uint8_t  endtlv;     /**< End TLV (0) */
} __attribute__ ((packed));

/** 
 * One-way delay measurement PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 *
 * @note The name is oam_*one*md_pdu
 */
struct oam_pdu_1dm {
    uint8_t mel_ver;         /**< 3 bit MEL and 5 bit version */
    uint8_t opcode;          /**< Operation code indicating the PDU type */
    uint8_t flags;           /**< Flags dependent on the used PDU type */
    uint8_t tlv_offset;      /**< Offset in octets to the next OAM PDU */
    uint8_t txtimestampf_sec[4]; /**< TxTimeStampf secs */
    uint8_t txtimestampf_nano[4]; /**< TxTimeStampf nanosecs */
    /**
     * Reserved for 1DM receiving equipment (0) (for RxTimeStampf) */
    uint8_t reserved[8];    
    uint8_t endtlv;          /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Two-way delay measurement message PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 * </pre>
 */
struct oam_pdu_dmm {
    uint8_t mel_ver;         /**< 3 bit MEL and 5 bit version */
    uint8_t opcode;          /**< Operation code indicating the PDU type */
    uint8_t flags;           /**< Flags dependent on the used PDU type */
    uint8_t tlv_offset;      /**< Offset in octets to the next OAM PDU */
    uint8_t txtimestampf_sec[4]; /**< TxTimeStampf secs*/
    uint8_t txtimestampf_nano[4]; /**< TxTimeStampf nanosecs*/
    /**
     * Reserved for DMM receiving equipment (0) (for RxTimeStampf) secs */
    uint8_t reserved_dmm_secs[4]; 
    /**
     * Reserved for DMM receiving equipment (0) (for RxTimeStampf) nanosecs */
    uint8_t reserved_dmm_nano[4]; 
    /**
     * Reserved for DMR (0) (for TxTimeStampf) secs */
    uint8_t reserved_dmr_secs[4]; 
    /**
     * Reserved for DMR (0) (for TxTimeStampf) nanosecs */
    uint8_t reserved_dmr_nano[4]; 
    uint8_t reserved[8];     /**< Reserved for DMR receiving equipment (0) */
    uint8_t endtlv;          /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Two-way delay measurement response PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |         Reserved (0)          |
 * +---+---------------+-----------+
 *
 * (copied from the DMM)
 * </pre>
 */
struct oam_pdu_dmr {
    uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
    uint8_t opcode;      /**< Operation code indicating the PDU type */
    uint8_t flags;       /**< Flags dependent on the used PDU type */
    uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
    uint8_t txtimestampf_sec[4]; /**< TxTimeStampf secs */
    uint8_t txtimestampf_nano[4]; /**< TxTimeStampf nanosecs */
    uint8_t rxtimestampf_sec[4]; /**< RxTimeStampf secs */
    uint8_t rxtimestampf_nano[4]; /**< RxTimeStampf nanosecs */
    uint8_t txtimestampb_sec[4]; /**< TxTimeStampb secs */     
    uint8_t txtimestampb_nano[4]; /**< TxTimeStampb nanosecs */
    /**
     *  Reserved for DMR receiving equipment (0) (for RxTimeStampb) */
    uint8_t reserved[8];     
     uint8_t endtlv;      /**< End TLV (0) */
} __attribute__ ((packed));

/**
 * Experimental message PDU
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_exm_no_end {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
     uint8_t oui[3];     /**< Organizationally unique identifier defines the subopcodes and TLVs */
     uint8_t subopcode;  /**< sub operation code indicating the TLV type */
     /* TLVs */
     /* End TLV (0) */
} __attribute__ ((packed));

/**
 * Experimental response PDU
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_exr_no_end {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
     uint8_t oui[3];     /**< Organizationally unique identifier defines the subopcodes and TLVs */
     uint8_t subopcode;  /**< sub operation code indicating the TLV type */
     /* TLVs */
     /* End TLV (0) */
} __attribute__ ((packed));

/**
 * Vendor specific message PDU
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_vsm_no_end {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
     uint8_t oui[3];     /**< Organizationally unique identifier defines the subopcodes and TLVs */
     uint8_t subopcode;  /**< sub operation code indicating the TLV type */
     /* TLVs */
     /* End TLV (0) */ 
} __attribute__ ((packed));

/**
 * Vendor specific response PDU
 *
 * @note Remember! to put the end TLV after the TLVs
 */
struct oam_pdu_vsr_no_end {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU */
     uint8_t oui[3];     /**< Organizationally unique identifier defines the subopcodes and TLVs */
     uint8_t subopcode;  /**< sub operation code indicating the TLV type */
     /* TLVs */
     /* End TLV (0) */
} __attribute__ ((packed));

/**
 * Client Signal Fail PDU
 *
 * <pre>
 * Flags format: 
 *   8  7   6   5   4   3   2   1
 * +------+-----------+----------+
 * | R(0) |   Type    | Period   |
 * +------+-----------+----------+
 * </pre>
 */
struct oam_pdu_csf {
     uint8_t mel_ver;     /**< 3 bit MEL and 5 bit version */
     uint8_t opcode;      /**< Operation code indicating the PDU type */
     uint8_t flags;       /**< Flags dependent on the used PDU type */
     uint8_t tlv_offset;  /**< Offset in octets to the next OAM PDU (0)*/
     uint8_t endtlv;      /**< End TLV (0) */
} __attribute__ ((packed));


/**
 * Synthetic Loss Message PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |        Reserved (0)           |
 * +---+---------------+-----------+
 * </pre>
 */
struct oam_pdu_slm_no_end {
    uint8_t           mel_ver;    /**< 3 bit MEL and 5 bit version */
    uint8_t           opcode;     /**< Operation code indicating the PDU type */
    uint8_t           flags;      /**< Flags dependent on the used PDU type */
    uint8_t           tlv_offset; /**< Offset in octets to the next OAM PDU */
    uint16_t          source_mepid; /**< 13 bit int identifying the transmitting MEP (bits 16 to 14 are set to zero) */
    uint16_t          target_mepid; /**< 13 bit int identifying the transmitting MEP (bits 16 to 14 are set to zero) */
    uint32_t          test_id;     /**< Unique test ID among MEPs */
    uint32_t          txfcf;      /**< Number of transmitted frames toward peer MEP (0s when not in use) */
    uint32_t          reserved_txfcb; /**< Reserved (0) */
} __attribute__ ((packed));

/**
 * Synthetic Loss Reply PDU
 *
 * <pre>
 * Flags format: 
 *   8   7   6   5   4   3   2   1
 * +---+---------------+-----------+
 * |        Reserved (0)           |
 * +---+---------------+-----------+
 * </pre>
 */
struct oam_pdu_slr_no_end {
    uint8_t           mel_ver;    /**< 3 bit MEL and 5 bit version */
    uint8_t           opcode;     /**< Operation code indicating the PDU type */
    uint8_t           flags;      /**< Flags dependent on the used PDU type */
    uint8_t           tlv_offset; /**< Offset in octets to the next OAM PDU */
    uint16_t          source_mepid; /**< 13 bit int identifying the transmitting MEP (bits 16 to 14 are set to zero) */
    uint16_t          target_mepid; /**< 13 bit int identifying the transmitting MEP (bits 16 to 14 are set to zero) */
    uint32_t          test_id;     /**< Unique test ID among MEPs */
    uint32_t          txfcf;      /**< Number of transmitted frames toward peer MEP (0s when not in use) */
    uint32_t          txfcb;      /**< Value of the txfcf in the last CCM (0s when not in use) */ 
} __attribute__ ((packed));

void oam_init_pdu(struct oam_pdu_common *pdu);
struct oam_pdu_common *oam_alloc_pdu();
int oam_build_end_tlv(struct oam_pdu_common *pdu, int length);
int oam_build_pdu(struct oam_pdu_common *const common_pdu, 
                  const struct oam_pdu_common *const pdu,
                  uint16_t dst_mepid);

#endif /* OAM_PDU_H */
