
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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <asm/sockios.h>

#include <arpa/inet.h>

#include <net/if.h>

#include <netinet/in.h>

#include <netpacket/packet.h>

#include <sys/ioctl.h>
#include <sys/un.h>

#include "configuration.h"
#include "constants.h"
#include "debug.h"
#include "extendedpsquared.h"
#include "ife.h"
#include "input.h"
#include "output.h"
#include "tools.h"

/* Why I cannot include this correctly from asm/sockios.h */
#ifndef SIOCGSTAMP 
#define SIOCGSTAMP   0x8906 /**< Number of the timestamp for the ioctl, There should be no need to define it here, see the bug for this */
#endif 
 
/**
 * If there is data in the socket the packet is read and checked
 * that it belongs to us and then passed to the correct handler.
 *
 * @param pdu A buffer into which the packet is read
 * @param fd File descriptor for the input socket
 * @param fd_out The file descriptor used for sending
 * @param configuration Pointer to the configuration information
 * @param mip are we a MIP?
 *
 * @return Zero on success, non-zero else.
 *
 * @note Use with poll, with select call the wrapper
 * @see oam_handle_raw_in
 */ 
int oam_handle_raw_read(struct oam_pdu_common *pdu, 
                        int fd,  
                        int fd_out,
                        struct oam_configuration *configuration,
                        enum oam_onoff mip)
{
    int                          err;
    int                          checked;
    uint8_t                      opcode;
    char                         buf[64];
    unsigned char               *ptr;
    socklen_t                    fromlen;
    struct sockaddr_ll           mac;
    struct oam_pdu_common       *pdu_in;
    struct oam_recv_information  recv_info;
    struct timeval               tv;

    ptr = (unsigned char *)pdu;

    fromlen = sizeof(mac);
    recv_info.rcv_len = recvfrom(fd,  
                                 pdu,
                                 OAM_MAX_PACKET, 
                                 0, 
                                 (struct sockaddr *)&mac, 
                                 &fromlen);
    
    if (recv_info.rcv_len == -1) { 
        OAM_ERROR("Failed to receive from the raw socket\n");    
        err = -1;
        goto out_err;
    }
    
    if (mac.sll_family == PF_PACKET) {
        recv_info.from_ifindex = mac.sll_ifindex;
        memcpy(&recv_info.from_mac, 
               &mac.sll_addr,
               sizeof(recv_info.from_mac));
    } 

    /* If we are a mip and the packet is going out from this
       machine skip this packet as its going out from this machine */
    if (mip == ON && mac.sll_pkttype == PACKET_OUTGOING) {
        err = 0;
	goto out_err;
    }   
 
    /* Get the timestamp here */
    ioctl(fd, SIOCGSTAMP, &tv);
    oam_tv_to_internal_time(&tv, &recv_info.hw_time); 
    oam_internal_time_str(buf, sizeof(buf), &recv_info.hw_time);
    //OAM_XTRA("HW Timestamp %s\n", buf);
    
    gettimeofday(&tv, NULL);
    oam_tv_to_internal_time(&tv, &recv_info.sw_time); 
    oam_internal_time_str(buf, sizeof(buf), &recv_info.sw_time);
    //OAM_XTRA("SW Timestamp %s\n", buf);
 
    checked = oam_check_packet(pdu, &recv_info, mip, fd_out);                   

    if (checked) {  
        ptr = ptr + OAM_ETH_HEADER_LENGTH + 1; /* 1 = Level/version */
        opcode = *ptr;           
        OAM_DEBUG("%s (%d)\n", 
                  oam_packet_type_name(opcode), opcode);
        /* get the PDU start minus the Eth header */
        ptr = (unsigned char *)pdu + OAM_ETH_HEADER_LENGTH;
        pdu_in = (struct oam_pdu_common *)ptr; 
        switch (opcode) {   
        case OAM_OPCODE_CCM:  
            err = oam_handle_ccm(pdu_in, &recv_info, configuration);
            break;
        case OAM_OPCODE_SLM:
            err = oam_handle_slm(pdu_in, &recv_info, fd_out);
            break;
        case OAM_OPCODE_SLR:
            err = oam_handle_slr(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_LBM:
            err = oam_handle_lbm(pdu_in, &recv_info, fd_out);
            break;
        case OAM_OPCODE_LBR:
            err = oam_handle_lbr(&recv_info);
            break;
        case OAM_OPCODE_LTM:
            err = oam_handle_ltm(pdu_in, &recv_info, fd_out);
            break;
        case OAM_OPCODE_LTR:
            err = oam_handle_ltr(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_AIS:
            err = oam_handle_ais(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_LCK:
            err = oam_handle_lck(pdu_in, &recv_info);
                break;
        case OAM_OPCODE_TST:
            err = oam_handle_tst(pdu_in, &recv_info);
            break;    
        case OAM_OPCODE_LAPS:
            err = oam_handle_laps(&recv_info);
            break;
        case OAM_OPCODE_RAPS:
            err = oam_handle_raps(&recv_info);
            break;
        case OAM_OPCODE_MCC:
            err = oam_handle_mcc(pdu_in, &recv_info, ON);
            break;
        case OAM_OPCODE_LMM:
            err = oam_handle_lmm(pdu_in, &recv_info, fd_out);
            break;
        case OAM_OPCODE_LMR:
            err = oam_handle_lmr(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_1DM:
            err = oam_handle_1dm(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_DMM:
            err = oam_handle_dmm(pdu_in, &recv_info, fd_out, 
                                 configuration->dm_optional_fields);
            break;
        case OAM_OPCODE_DMR:
            err = oam_handle_dmr(pdu_in, &recv_info, configuration);
            break;
        case OAM_OPCODE_EXM:
            err = oam_handle_exm(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_EXR:
            err = oam_handle_exr(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_VSR:
            err = oam_handle_vsr(pdu_in, &recv_info);
            break;
        case OAM_OPCODE_VSM: 
            err = oam_handle_vsm(pdu_in, &recv_info);
            break;
        default:
            err = 0;
            break;
        }
        /* Say we failed but do not cause an error */
        if (err) { 
            OAM_ERROR("Failed to handle packet %s (%d)\n", 
                      oam_packet_type_name(opcode), opcode);
            err = 0;
            goto out_err;
        }
    } else {
        /* Do not handle packet and let's be silent about it. */
        err = 0;            
        }
out_err:
    /* reset the buffer */
    memset(pdu, 0, OAM_MAX_PACKET); 
    return err;
}
 
/**
 * If there is data in the socket the packet is read and checked
 * that it belongs to us and then passed to the correct handler.
 *
 * @param pdu A buffer into which the packet is read
 * @param fd File descriptor for the input socket
 * @param fdset File descriptor set to which the socket belongs to
 * @param fd_out The file descriptor used for sending
 * @param configuration Pointer to the configuration information
 * @param mip are we a MIP?
 *
 * @return Zero on success, non-zero else.
 */
int oam_handle_raw_in(struct oam_pdu_common *pdu, 
                      int fd, 
                      fd_set *fdset, 
                      int fd_out,
                      struct oam_configuration *configuration,
                      enum oam_onoff mip)
{
    int err = 0;

    if (FD_ISSET(fd, fdset)) {
        err = oam_handle_raw_read(pdu, fd, fd_out, configuration, mip);
        if (err != 0) {
            OAM_ERROR("Error in raw read\n");
        }
    }
    return err;
}

/**
 * Function to check the fault cases for the PDU CCM.
 */
static int oam_ccm_faults(unsigned char period, 
                          int level, 
                          struct oam_pdu_ccm *pdu,
                          const struct oam_configuration *const configuration,
                          char *mac_addr,
                          struct oam_entity_stat *found) 
{
    extern struct oam_entity *entity;
/* XXX REPORT ALL ERRORS AND THEN EXIT SO ERR = -1 and go through all */
    if (give_legit_interval(period) != (unsigned int)configuration->ccm_period) {
        OAM_STAT_INFO("CCM Fault, Unexpected period (%d),"
                      " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                      period, level, pdu->meg_id, pdu->mepid, mac_addr);
        oam_send_ccd("CCM Fault, Unexpected period (%d),"
                     " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                     period, level, pdu->meg_id, pdu->mepid, mac_addr);
        return -1;
    }

    if (entity->meg_level > level) {
        OAM_STAT_INFO("CCM Fault, Unexpected MEG level,"
                      " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                      level, pdu->meg_id, pdu->mepid, mac_addr);
        oam_send_ccd("CCM Fault, Unexpected MEG level,"
                     " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                     level, pdu->meg_id, pdu->mepid, mac_addr);
        return -1;
    }
    
    if (entity->meg_level == level &&
        strcmp(entity->id_icc, (const char *)pdu->meg_id) != 0) {
        OAM_STAT_INFO("CCM Fault, Mismerge,"
                      " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                      level, pdu->meg_id, pdu->mepid, mac_addr);
        oam_send_ccd("CCM Fault, Mismerge,"
                     " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                     level, pdu->meg_id, pdu->mepid, mac_addr);
        return -1;
    }

    if (entity->meg_level == level &&
        strcmp(entity->id_icc, (const char *)pdu->meg_id) == 0) {
        /* Check that the MEP ID was not ours */ 
        if ((found == NULL) && pdu->mepid != htons(entity->mepid)) { 
            OAM_STAT_INFO("CCM Fault, Unexpected MEP,"
                          " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                          level, pdu->meg_id, pdu->mepid, mac_addr);
            oam_send_ccd("CCM Fault, Unexpected MEP,"
                         " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                         level, pdu->meg_id, pdu->mepid, mac_addr);
            return -1;
        }
    }
    return 0;
}

/**
 * Handle packet CCM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param configuration Pointer to the configuration information
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_ccm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const struct oam_configuration *const configuration)
{
    int                     length; 
    int                     version;
    int                     mask        = 0x7;
    uint8_t                 opcode      = 0;
    uint8_t                 rdi         = 0;
    uint8_t                 tlvoffset   = 0;
    int                     level       = 0;
    unsigned char           flags       = 0;
    unsigned char           period      = 0;
    static unsigned char    rdi_on      = 128;
    uint32_t                rx_packets  = 0;
    uint32_t                frame_loss  = 0;
    unsigned char          *ptr;
    double                  loss_rate   = 0;
    char                    index_name[6]; 
    char                    mac_addr[]  = "00:00:00:00:00:00"; 
    uint8_t                 empty_mac[6];
    struct oam_pdu_ccm      pdu_ccm;
    struct oam_pdu_ccm     *pdu; 
    struct oam_entity_stat *found       = NULL;

    extern struct oam_db    *oam_control_info;
    extern struct oam_entity *entity;

    ptr = (unsigned char *)common_pdu;
    pdu = &pdu_ccm;

    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);
    if (recv_info->multicast == ON) {
        OAM_DEBUG("Received via Multicast\n");
    }

    /* level / version (0)*/
    version = *ptr & 0x1F;
    level = *ptr >> 5;
    ptr = ptr + 1;

    /* opcode */
    opcode = *ptr; 
    ptr = ptr + 1;

    /* RDI (1st (MSB) bit) and period Flags*/
    flags = *ptr;
    period = *ptr;
    period &= mask;
    rdi = flags & (1 << 7); 
    ptr = ptr + 1;
    /* TLV offset */
    tlvoffset = *ptr;
    if (!(tlvoffset ==  OAM_TLVOFFSET_CCM)) {
        OAM_ERROR("Incorrect TLV offset for CCM\n");
        return -1;
    }
    ptr = ptr + 1;

    /* Sequence number*/
    length = sizeof(pdu->seq); 
    memcpy(&pdu->seq, ptr, length);
    ptr = ptr + length;    

    /* MEP ID */
    length = sizeof(pdu->mepid); 
    memcpy(&pdu->mepid, ptr, length);
    pdu->mepid = htons(pdu->mepid);
    ptr = ptr + length;  

    /* MEG ID */
    length = sizeof(pdu->meg_reserved);
    ptr = ptr + length;

    length = sizeof(pdu->meg_format); 
    pdu->meg_format = *ptr;
    if (!(*ptr == OAM_MEG_ID_FORMAT)) {
        OAM_ERROR("Unknown MEG ID format (%d)\n", *ptr);
        return -1;
    }
    ptr = ptr + length;

    length = sizeof(pdu->meg_length); 
    pdu->meg_length = *ptr;
    if (!(*ptr == OAM_MEG_ID_LENGTH)) {
        OAM_ERROR("Incorrect MEG ID length\n");
        return -1;
    }
    ptr = ptr + length;

    length = sizeof(pdu->meg_id); 
    memcpy(&pdu->meg_id, ptr, length);
    OAM_DEBUG("MEG ID: %s\n", pdu->meg_id);
    ptr = ptr + length;

    length = sizeof(pdu->meg_unused);
    ptr = ptr + length;

    /* TxFCf */
    length = sizeof(pdu->txfcf); 
    memcpy(&pdu->txfcf, ptr, length);
    ptr = ptr + length;    

    /* RxFCb */
    length = sizeof(pdu->rxfcb); 
    memcpy(&pdu->rxfcb, ptr, length);
    ptr = ptr + length;   
 
    /* TxFCb */
    length = sizeof(pdu->txfcb); 
    memcpy(&pdu->txfcb, ptr, length);
    ptr = ptr + length;

    OAM_DEBUG("Level: %d\n", level);
    OAM_DEBUG("Version: %d\n", version);
    OAM_DEBUG("Opcode: %s (%d)\n", oam_packet_type_name(opcode), opcode);
    OAM_DEBUG("FLAGS %d\n", flags);
    OAM_DEBUG("Period (usec) %d\n", give_legit_interval(period));
    OAM_DEBUG("RDI bit %d\n", rdi);
    OAM_DEBUG("TLV offset %d\n", *ptr);
    OAM_DEBUG("Seq # %d\n", pdu->seq);
    OAM_DEBUG("MEP ID: %d\n", pdu->mepid);

    /* full dynamic MEG creation */ 
    found = oam_find(htons(pdu->mepid));

    if (
	#if 0 /* Created for unicast request, too */
	recv_info->multicast == ON &&
	#endif
        configuration->dynamic == 1 &&
        !found) { 
        if (!if_indextoname(recv_info->from_ifindex, index_name)) {
            OAM_ERROR("Failed to convert the IF index to name\n");
            return -1;
        } 
        if (oam_add_mep_to_db(level,
                               pdu->mepid, 
                               recv_info->from_mac,
                               (char *)pdu->meg_id,
                               recv_info->from_ifindex,
                               index_name) != 0) {
            OAM_ERROR("Failed to add MEP\n");
            return -1;
        }
        /* Requery for later use see below */
        found = oam_find(htons(pdu->mepid));
    }

    /* Store the information about the MEP if needed */

    /* dynamic but the MEP IDs are known */
    if (configuration->dynamic == 2 && found) {
        memset(&empty_mac, 0, sizeof(empty_mac));

        if (memcmp(found->mac, &empty_mac, sizeof(empty_mac)) == 0) { 

            if (!if_indextoname(recv_info->from_ifindex, index_name)) {
                OAM_ERROR("Failed to convert the IF index to name\n");
                return -1;
            } 
            if (oam_update_mep_in_db(level,
                                      found->mepid, 
                                      recv_info->from_mac,
                                      (char *)pdu->meg_id,
                                      recv_info->from_ifindex,
                                      index_name) != 0) {
                OAM_STAT_INFO("CCM Fault, Unexpected MEP,"
                              " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                              level, 
                              pdu->meg_id, 
                              found->mepid, 
                              recv_info->from_mac);
                oam_send_ccd("CCM Fault, Unexpected MEP,"
                             " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                             level, 
                             pdu->meg_id, 
                             found->mepid, 
                             recv_info->from_mac);
                return -1; 
            }
        }
    }

    if (found) {
        OAM_DEBUG("TxFCf; %d (we sent)\n", found->ccm_my_txfcf); 
        oam_timestamp(&found->ccm_stamp);

        frame_loss = oam_calculate_frame_loss(pdu->txfcb,
                                              found->ccm_txfcb_tp,
                                              pdu->rxfcb,
                                              found->ccm_rxfcb_tp);
        if (pdu->txfcb > 0) {
            loss_rate = (double)abs(frame_loss) / (double)pdu->txfcb;
        } else {   
            loss_rate = 0;
        }
        OAM_STAT_INFO("CCM MEP ID %d, TxFCb_tp %d, TxFCb_tc %d, "
                      "RxFCf_tp %d, RxFCb_tc %d\n", 
                      found->mepid, 
                      found->ccm_txfcb_tp, 
                      pdu->txfcb,
                      found->ccm_rxfcb_tp,
                      pdu->rxfcb);        
        OAM_STAT_INFO("CCM: Frame loss %d, rate %f%%\n", frame_loss, loss_rate);
            
        /* store the counters */ 
        memcpy(&found->ccm_txfcf_tp, &pdu->txfcf, sizeof(found->ccm_txfcf_tp));
        memcpy(&found->ccm_rxfcb_tp, &pdu->rxfcb, sizeof(found->ccm_rxfcb_tp));
        memcpy(&found->ccm_txfcb_tp, &pdu->txfcb, sizeof(found->ccm_txfcb_tp));
        if (recv_info->multicast == OFF) {
            rx_packets = oam_ask_rtx_packets(found->ifindex_name, OAM_RX); 
            memcpy(&found->ccm_rx, &rx_packets, sizeof(found->ccm_rx));  
        }
        
        if (found->alarm_ccm == 1) {
            OAM_STAT_INFO("CCM ALARM condition lowered for MEP ID %d\n", 
                          pdu->mepid);
            oam_send_ccd("CCM ALARM condition lowered for MEP ID %d\n", 
                          pdu->mepid);
            oam_set_alarm_ccm(oam_control_info, found, OAM_ALARM_OFF);
        }
        if (flags == rdi_on) {
            OAM_STAT_INFO("RDI received from MEP ID %d\n", pdu->mepid);
            oam_send_ccd("RDI received from MEP ID %d\n", pdu->mepid);
            found->rdi = OAM_ALARM_ON;
        } else if (found->rdi == OAM_ALARM_ON) {
            OAM_STAT_INFO("RDI lowered from MEP ID %d\n", pdu->mepid);
            oam_send_ccd("RDI lowered from MEP ID %d\n", pdu->mepid);
            found->rdi = OAM_ALARM_OFF;
        }
       
        /* Check if the interface has changed since the last time */
        if (recv_info->from_ifindex != found->ifindex && found->ifindex != 0) {
            OAM_STAT_INFO("CCM Fault, Unexpected interface,"
                          " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                          level, pdu->meg_id, pdu->mepid, mac_addr);
            oam_send_ccd("CCM Fault, Unexpected interface,"
                         " MEG level %d, MEG ID %s, MEP ID %d, MAC %s\n",
                         level, pdu->meg_id, pdu->mepid, mac_addr);
        }
    }
        
    return oam_ccm_faults(period, level, pdu, configuration, mac_addr, found);
}

/**
 * Handle packet SLM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param fd_out The file desctiptor for outputting the LBR
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_slm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info, 
                   const int fd_out)
{ 
    int err                             = 0;
    int ifindex                         = 0;
    int returned_length                 = 0;
    unsigned char              *ptr;
    unsigned char              *dst_mac;
    char mac_addr[] = "00:00:00:00:00:00";
    struct oam_pdu_slm_no_end  *slm_in  = NULL;
    struct oam_pdu_slr_no_end  *slr_ptr = NULL;
    struct oam_pdu_common      *pdu_out = NULL;
    struct oam_entity_stat     *found = NULL;
    
    extern struct oam_entity *entity;
 
    ptr = (unsigned char *)common_pdu;
    dst_mac = (unsigned char *)recv_info->from_mac;
    oam_mac_ntop((char *const)mac_addr, dst_mac);
    
    /* Handle the packet*/    
    slm_in = (struct oam_pdu_slm_no_end *)ptr;
    if (!slm_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } else {
        pdu_out = oam_alloc_pdu(); 
        slr_ptr = calloc(1, sizeof(struct oam_pdu_slr_no_end));
    }
    
    OAM_DEBUG("Handling SLM with Test ID %d\n", slm_in->test_id);
    OAM_DEBUG("SLM, txfcf %d\n", htonl(slm_in->txfcf));
    
    slr_ptr->opcode  = OAM_OPCODE_SLR; 
    memcpy(&slr_ptr->test_id, 
           &slm_in->test_id, 
           sizeof(slr_ptr->test_id));
    memcpy(&slr_ptr->source_mepid, 
           &slm_in->source_mepid, 
           sizeof(slr_ptr->source_mepid));
    memcpy(&slr_ptr->target_mepid, 
           &entity->mepid, 
           sizeof(slr_ptr->target_mepid)); 
    /* Mirror the SLM txfcf */
    memcpy(&slr_ptr->txfcf, 
           &slm_in->txfcf, 
           sizeof(slr_ptr->txfcf)); 
   
    found = oam_find_by_mac(dst_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        goto out_err;
    } else {
        ifindex = found->ifindex;

        /* How many SLR have we sent (including this one) to this test-id */
        found->slm_txfcl_slr++;
        OAM_DEBUG("SLR txfcb %d\n", found->slm_txfcl_slr);
        memcpy(&slr_ptr->txfcb, 
               &found->slm_txfcl_slr, 
               sizeof(slr_ptr->txfcb));

    }

    returned_length = oam_build_pdu(pdu_out,
                                   (const struct oam_pdu_common *const) 
                                    slr_ptr, 
                                    *dst_mac);
    returned_length = oam_build_end_tlv((struct oam_pdu_common *)common_pdu, 
                                        returned_length);
    /* Send SLR */
    if (entity->send(fd_out, 
                     dst_mac, 
                     (const struct oam_pdu_common *const)pdu_out, 
                     returned_length,
                     ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        err = -1; 
    } 

out_err:

    free(pdu_out);
    return err;
}

/**
 * Handle packet SLR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else.  
 */
int oam_handle_slr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{ 
    int                        err = 0;
    unsigned char             *ptr;
    struct oam_pdu_slr_no_end *slr_in;
    unsigned char             *dst_mac;
    char                       mac_addr[] = "00:00:00:00:00:00";
    uint32_t                   tx = 0;
    uint32_t                   rx = 0;
    
    extern struct oam_entity  *entity;

    dst_mac = (unsigned char *)recv_info->from_mac;
    oam_mac_ntop((char *const)mac_addr, dst_mac);
 
    ptr = (unsigned char *)common_pdu;
    
    /* Handle the packet*/    
    slr_in = (struct oam_pdu_slr_no_end *)ptr;
    if (!slr_in) {
        OAM_DEBUG("No inbound packet to handle\n");
        err = -1;
    } else {    
        tx = htonl(slr_in->txfcf);
        rx = htonl(slr_in->txfcb);
        OAM_STAT_INFO("SLR, Test ID %d, txfcf %d, txfcb %d, loss %d\n", 
                      slr_in->test_id ,tx, rx, (tx - rx));
    }
    
    return err;
}

/**
 * Handle packet LBM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param fd_out The file desctiptor for outputting the LBR
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_lbm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info, 
                   const int fd_out)
{ 
    int err                             = 0;
    int ifindex                         = 0;
    int returned_length                 = 0;
    unsigned char              *ptr;
    unsigned char              *dst_mac;
    char mac_addr[] = "00:00:00:00:00:00";
     struct oam_pdu_lbm_no_data *lbm_in  = NULL;
    struct oam_pdu_lbr_no_data *lbr_ptr = NULL;
    struct oam_pdu_common      *pdu_out = NULL;
    struct oam_entity_stat     *found = NULL;

    extern struct oam_entity *entity;
 
    ptr = (unsigned char *)common_pdu;
    dst_mac = (unsigned char *)recv_info->from_mac;
    oam_mac_ntop((char *const)mac_addr, dst_mac);

    /* Handle the packet*/    
    lbm_in = (struct oam_pdu_lbm_no_data *)ptr;
    if (!lbm_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } else {
        pdu_out = oam_alloc_pdu(); 
        lbr_ptr = calloc(1, sizeof(struct oam_pdu_lbr_no_data));
    }

    OAM_DEBUG("Handling LBM with TID/SEQ %d\n", ntohl(lbm_in->tid_seq));

    lbr_ptr->opcode  = OAM_OPCODE_LBR; 
    memcpy(&lbr_ptr->tid_seq, &lbm_in->tid_seq, sizeof(lbr_ptr->tid_seq));

    returned_length = oam_build_pdu(pdu_out,
                                    (const struct oam_pdu_common *const) 
                                    lbr_ptr, 
                                    *dst_mac);
    /* Send LBR */
    found = oam_find_by_mac(dst_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        goto out_err;
    } else {
        ifindex = found->ifindex;
    }

    oam_check_received_seq("LBR",
                           ntohl(lbr_ptr->tid_seq), 
                           &found->lb_seq_in,
                           mac_addr);

    if (entity->send(fd_out, 
                     dst_mac, 
                     (const struct oam_pdu_common *const)pdu_out, 
                     returned_length,
                     ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        err = -1; 
    } 

out_err:

    free(pdu_out);
    return err;
}

/**
 * Handle packet LBR
 * 
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_lbr(const struct oam_recv_information *recv_info)
{
    int err;
    unsigned char              *from_mac;
    struct oam_entity_stat     *found = NULL;
    struct timeval             *stamp = NULL;
    
    extern struct oam_db *oam_control_info;

    from_mac = (unsigned char *)recv_info->from_mac;

    found = oam_find_by_mac(from_mac);
    if (found == NULL) {
        OAM_ERROR("Did not find the sender information from others\n");
        return -1;
    }
    stamp = &found->lb_stamp;
    found->lb_send_control.received++;
    err = oam_inside_interval("LB", stamp, OAM_LBR_TIMEOUT_SEC, 0);
    if (err < 0) {
        OAM_STAT_INFO("LB ALARM from MEP ID %d\n", found->mepid);
        oam_send_ccd("LB ALARM from MEP ID %d\n", found->mepid);
        oam_set_alarm_lb(oam_control_info, found, OAM_ALARM_ON);
    } else {
        /* is this statistically sane or should I add all and then 
         when done divide TICKET #87 */

        /* timeval time */
        OAM_XTRA("LB RTT SW %d usec\n", err); 
        found->lb_usecs_rtt = err;
        err = 0; /* Reset the return value as the caller expects 0 success */
        
        /* internal time */
        OAM_XTRA("LB RTT HW %d ns\n", 
                 oam_internal_time_diff(&found->lb_stamp_int, 
                 &recv_info->hw_time)); 
    }
    stamp->tv_sec = 0;
    stamp->tv_usec = 0;
    return err;
}

/**
 * Handle packet LTM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param fd_out FD used for sending the LTR / forwarded LTM
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_ltm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out)
{
    int                        err             = 0;
    int                        ifindex         = 0;
    int                        returned_length = 0;
    uint8_t                    ttl             = 0;
    //    uint32_t                   seq             = 0;
    unsigned char             *ptr;
    unsigned char             *from_mac;
    unsigned char             *origin_mac;
    unsigned char             *target_mac;
    char                       mac_addr[]  = "00:00:00:00:00:00";
    struct oam_pdu_ltm_no_end *ltm_in          = NULL; 
    struct oam_pdu_ltr_no_end *ltr_ptr         = NULL;
    struct oam_pdu_common     *pdu_out         = NULL;
    struct oam_entity_stat    *found           = NULL;

    extern struct oam_entity *entity;
 
    ptr = (unsigned char *)common_pdu;
    from_mac = (unsigned char *)recv_info->from_mac;
    oam_mac_ntop((char *const)mac_addr, from_mac);

    /* Handle the packet*/    
    ltm_in = (struct oam_pdu_ltm_no_end *)ptr;
    if (!ltm_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } else {
        pdu_out = oam_alloc_pdu(); 
        ltr_ptr = calloc(1, sizeof(struct oam_pdu_ltr_no_end));
    }

    /* get the ingress ifindex with the from_mac*/
    found = oam_find_by_mac(from_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity for the sender\n");
        err = -1;
        goto out_err;
    } else {
        ifindex = found->ifindex;
    }

    /* skip the commons */
    ptr = ptr + sizeof(struct oam_pdu_common);

    //seq = *ptr;
    ptr = ptr + sizeof(ltm_in->transid);
    ttl = *ptr;
    OAM_DEBUG("TTL %d\n", ttl);
    /* one away from the TTL */
    *ptr = *ptr - 1;
    ptr = ptr + sizeof(ltm_in->ttl);

    /* get the other macs */
    origin_mac = ptr;
    ptr = ptr + OAM_ETH_MAC_LENGTH;
    target_mac = ptr;

    oam_print_mac("Origin MAC",origin_mac);
    oam_print_mac("Target MAC",target_mac);

/* XXX FIX
    if (memcmp(target_mac, entity->mac, sizeof(entity->mac)) != 0) {
        found = oam_find_by_mac(target_mac);
        if (!found) {
            OAM_ERROR("Did not find the target mac form MEG\n");
            err = -1;
            goto out_err;
        }
        oam_check_received_seq("LTR",
                               ntohl(seq), 
                               &found->lt_seq_in,
                               mac_addr);
    }
*/
    ltr_ptr->opcode  = OAM_OPCODE_LTR; 
    
    returned_length = oam_build_pdu(pdu_out,
                                    (const struct oam_pdu_common *const) 
                                    ltr_ptr, 
                                    *origin_mac);
    /* Send LTR */
    if (entity->send(fd_out, 
                     origin_mac, 
                     (const struct oam_pdu_common *const)pdu_out, 
                     returned_length,
                     ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        err = -1; 
        goto out_err;
    } 
    
    /* forward the LTM if it wasn't addressed to us */
    /* XXX if not MEP we do not do this MIP does check if we are a mip and 
     * add a call to this handle from the forwarder. */
    if (!found) {
        OAM_ERROR("Did not find correct entity target mac\n");
        err = -1;
        goto out_err;
    } else {
        ifindex = found->ifindex;
    }
    if (ttl == 0) {
        OAM_STAT_INFO("TTL exceeded, dropping LTM towards MEP ID %s, %s\n",
                      found->mepid, found->mac);
        
    }
    /* XXX FIX
    if (memcmp(target_mac, entity->mac, sizeof(entity->mac)) != 0) {
        if (oam_send_to_other_directions(fd_out,
                                         (struct oam_pdu_common *)ltm_in,
                                         from_mac)) {
            OAM_ERROR("Failed to forward the LTM to one or more interfaces\n");
            err = -1;
            goto out_err;
        }
    }
    */    
out_err:

    free(pdu_out);
    return err;
}

/**
 * Handle packet LTR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_ltr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char *ptr;
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    ptr = (unsigned char *)common_pdu;
    OAM_DEBUG("DO SOMETHING WITH THIS %d\n",ptr);
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented\n");
    return -1;
}

/**
 * Handle packet AIS
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_ais(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    int                        level             = 0;
    int                        version           = 0;
    int                        mask              = 0x7;
    uint8_t                    period            = 0;
    unsigned char             *ptr               = NULL;
    char                       mac_addr[]        = "00:00:00:00:00:00"; 
    struct oam_pdu_ais        *ais_in            = NULL;
    struct oam_entity_stat    *found             = NULL;

    extern struct oam_entity *entity;

    ptr = (unsigned char *)common_pdu;
 
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);

    OAM_DEBUG("MAC %s\n", mac_addr);

    /* level / version (0)*/
    version = *ptr & 0x1F;
    level = *ptr >> 5;
    OAM_DEBUG("Level: %d\n", level);
    if (entity->meg_level > level) {
        OAM_STAT_INFO("AIS Fault, Unexpected MEG level,"
                      " MEG level %d, MAC %s\n",
                      level, mac_addr);
        oam_send_ccd("AIS Fault, Unexpected MEG level,"
                      " MEG level %d, MAC %s\n",
                      level, mac_addr);
        return -1;
    }
    OAM_DEBUG("Version: %d\n", version);
    ptr = ptr + sizeof(ais_in->mel_ver);
    ptr = ptr + sizeof(ais_in->opcode);
    period = *ptr;
    period &= mask;

    OAM_DEBUG("Recv period: %d", give_legit_interval(period));

    found = oam_find_by_mac(recv_info->from_mac);
    if (!found) {
        OAM_ERROR("Did not find the MEP ID with its MAC\n");
        return -1;
    }

    /* store the period so it can be checked that we have not seen 
       any ais's beyond the period * 3.5 and drop the state. */
    if (found->ais_recv_period != period && found->ais_recv_period != 0) {
        OAM_STAT_INFO("AIS Fault, Unexpected period (earlier %d, now %d),"
                      " MEG level %d, MAC %s\n",
                      found->ais_recv_period, period, level, mac_addr);
        oam_send_ccd("AIS Fault, Unexpected period (earlier %d, now %d),"
                     " MEG level %d, MAC %s\n",
                     found->ais_recv_period, period, level, mac_addr);
        return -1;
    }
    found->ais_recv_period = period;
    return 0;
}

/**
 * Handle packet LCK
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_lck(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    int                        level             = 0;
    int                        version           = 0;
    int                        mask              = 0x7;
    uint8_t                    period            = 0;
    unsigned char             *ptr               = NULL;
    char                       mac_addr[]        = "00:00:00:00:00:00"; 
    struct oam_pdu_lck        *lck_in            = NULL;
    struct oam_entity_stat    *found             = NULL;

    extern struct oam_entity *entity;

    ptr = (unsigned char *)common_pdu;
 
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);

    OAM_DEBUG("MAC %s\n", mac_addr);

    /* level / version (0)*/
    version = *ptr & 0x1F;
    level = *ptr >> 5;
    OAM_DEBUG("Level: %d\n", level);
    if (entity->meg_level > level) {
        OAM_STAT_INFO("LCK Fault, Unexpected MEG level,"
                      " MEG level %d, MAC %s\n",
                      level, mac_addr);
        oam_send_ccd("LCK Fault, Unexpected MEG level,"
                     " MEG level %d, MAC %s\n",
                     level, mac_addr);
        return -1;
    }
    OAM_DEBUG("Version: %d\n", version);
    ptr = ptr + sizeof(lck_in->mel_ver);
    ptr = ptr + sizeof(lck_in->opcode);
    period = *ptr;
    period &= mask;

    OAM_DEBUG("Recv period: %d", give_legit_interval(period));

    found = oam_find_by_mac(recv_info->from_mac);
    if (!found) {
        OAM_ERROR("Did not find the MEP ID with its MAC\n");
        return -1;
    }

    /* store the period so it can be checked that we have not seen 
       any lck's beyond the period * 3.5 and drop the state. */
    if (found->lck_recv_period != period && found->lck_recv_period != 0) {
        OAM_STAT_INFO("LCK Fault, Unexpected period (earlier %d, now %d),"
                      " MEG level %d, MAC %s\n",
                      found->lck_recv_period, period, level, mac_addr);
        oam_send_ccd("LCK Fault, Unexpected period (earlier %d, now %d),"
                     " MEG level %d, MAC %s\n",
                     found->lck_recv_period, period, level, mac_addr);
        return -1;
    }
    found->lck_recv_period = period;
    return 0;
} 

/**
 * Handle packet TST
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_tst(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    int                        i                 = 0;
    int                        level             = 0;
    int                        version           = 0;
    unsigned char              test_zero         = 0;
    uint8_t                    type              = 0;
    uint16_t                   pattern_length    = 0;
    uint16_t                   pattern_length_in = 0;
    uint8_t                    pattern_type      = 0;
    uint32_t                   checksum          = 0;
    unsigned char             *ptr               = NULL;
    char                       mac_addr[]        = "00:00:00:00:00:00"; 
    struct oam_pdu_tst_no_end *tst_in            = NULL;
    struct oam_entity_stat    *found             = NULL;
    unsigned char             *prbs = NULL; 
    unsigned char  prbs_state_register[OAM_PRBS_SHIFT_REGISTER_LENGTH];  

    extern struct oam_entity *entity;

    ptr = (unsigned char *)common_pdu;
 
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);

    OAM_DEBUG("MAC %s\n", mac_addr);

    /* level / version (0)*/
    version = *ptr & 0x1F;
    level = *ptr >> 5;
    OAM_DEBUG("Level: %d\n", level);

    OAM_DEBUG("Version: %d\n", version);
    ptr = ptr + 1;

    if (entity->meg_level > level) {
        OAM_STAT_INFO("TST Fault, Unexpected MEG level,"
                      " MEG level %d, MAC %s\n",
                      level, mac_addr);
        oam_send_ccd("TST Fault, Unexpected MEG level,"
                     " MEG level %d, MAC %s\n",
                     level, mac_addr);
        return -1;
    }

    tst_in = (struct oam_pdu_tst_no_end *)common_pdu;

    found = oam_find_by_mac(recv_info->from_mac);
    if (!found) {
        OAM_ERROR("Did not find the MEP ID with its MAC\n");
        return -1;
    }

    oam_check_received_seq("TST",
                           ntohl(tst_in->seq), 
                           &found->tst_seq_in,
                           mac_addr);
    
    /* Test the test pattern for faults. */    
    /* get the tlv start */
    ptr = ptr + sizeof(tst_in->opcode) + sizeof(tst_in->flags) + 
        sizeof(tst_in->tlv_offset) + sizeof(tst_in->seq);
    type = *ptr;
    OAM_DEBUG("TYPE %d\n", type);
    ptr = ptr + sizeof(type); 
    memcpy(&pattern_length_in, ptr, sizeof(pattern_length_in));
    pattern_length = htons(pattern_length_in);

    pattern_length = pattern_length - sizeof(pattern_type) - sizeof(checksum); 
    ptr = ptr + sizeof(pattern_length);

    pattern_type = *ptr;
    ptr = ptr + sizeof(pattern_type);

    OAM_DEBUG("Pattern type: %s (%d), pattern length %d\n", 
              oam_tst_pattern_name(pattern_type), pattern_type, pattern_length);
    
    /* Test the pattern */ 
    if (pattern_type == OAM_PATTERN_TYPE_ZERO_NO_CRC || 
        pattern_type == OAM_PATTERN_TYPE_ZERO_CRC) { 
        for (i = 0; i < pattern_length; i++) { 
            test_zero += ptr[i]; 
        } 
        if (test_zero != 0) { 
            OAM_STAT_INFO("TST Fault, All-zero test pattern was not zeroes," 
                          " MEG level %d, MAC %s\n", 
                          level, mac_addr);
            oam_send_ccd("TST Fault, All-zero test pattern was not zeroes," 
                         " MEG level %d, MAC %s\n", 
                         level, mac_addr); 
        }
    } 
    if (pattern_type == OAM_PATTERN_TYPE_PRBS_NO_CRC || 
        pattern_type == OAM_PATTERN_TYPE_PRBS_CRC) { 
        for (i = 0; i < OAM_PRBS_SHIFT_REGISTER_LENGTH; i++) {  
            prbs_state_register[i] = OAM_PRBS_BIT_ONE; 
        }         
        prbs = oam_generate_prbs(prbs_state_register, pattern_length); 
        if (prbs == NULL) { 
            OAM_ERROR("Could not create PRBS\n"); 
            return -1; 
        } 
        OAM_HEXDUMP("PRBS ", prbs, pattern_length); 
        OAM_HEXDUMP("PTR  ", ptr, pattern_length); 
        if (memcmp(prbs, ptr, pattern_length) != 0) { 
            OAM_STAT_INFO("TST Fault, PRBS mismatch," 
                          " MEG level %d, MAC %s\n", 
                          level, mac_addr);
            oam_send_ccd("TST Fault, PRBS mismatch," 
                         " MEG level %d, MAC %s\n", 
                         level, mac_addr);
        } 	
    } 
    
    /* Test the CRC */
    if (pattern_type == OAM_PATTERN_TYPE_ZERO_CRC ||
        pattern_type == OAM_PATTERN_TYPE_PRBS_CRC) {
        //OAM_HEXDUMP("Pattern data ", ptr, pattern_length);
        oam_do_crc32_checksum(ptr, pattern_length, &checksum);
        
        /* find the received checsum */
        ptr = ptr + pattern_length;
        OAM_HEXDUMP("Checksum (OURS) ", &checksum, sizeof(checksum));
        OAM_HEXDUMP("Checksum (RECV) ", ptr, sizeof(checksum));
        if (memcmp(&checksum, ptr, sizeof(checksum)) != 0) {
            OAM_STAT_INFO("TST Fault, Checksum mismatch,"
                          " MEG level %d, MAC %s\n",
                          level, mac_addr);
            oam_send_ccd("TST Fault, Checksum mismatch,"
                         " MEG level %d, MAC %s\n",
                         level, mac_addr);
        }
    }
    free(prbs);
    return 0;
}

/**
 * Handle packet LAPS
 *
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_laps(const struct oam_recv_information *recv_info)
{
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented\n");
    return -1;
}

/**
 * Handle packet RAPS
 *
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_raps(const struct oam_recv_information *recv_info)
{
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);       
    OAM_DEBUG("Packet handling not implemented\n");
    return -1;
}

/**
 * Handle packet MCC
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param ethernet if ON then we received the msg from raw sock
 *                 and this is not a local message. 
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_mcc(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const enum oam_onoff ethernet)
{
    int                         err              = 0;
    struct oam_tool_information tool_info;
    uint8_t                     subopcode        = 0; 
    unsigned char              *ptr;
    struct oam_pdu_common      *out_common_pdu;
    char           mac_addr[]        = "00:00:00:00:00:00"; 

    /* See output and definition of oam_add_tool_tlv() for more info */
    const int common_parts_len = 4;
    const int oui_len          = 3;
    const int tl_len           = 3;    

    if (ethernet == ON) {
        oam_mac_ntop((char *const)mac_addr,  
                     (const unsigned char *const)recv_info->from_mac);
        OAM_DEBUG("Remote MAC %s\n", mac_addr);    
    }
    ptr = (unsigned char *)common_pdu;
    out_common_pdu = oam_alloc_pdu();

    /* Common parts and the oui */
    ptr = ptr + common_parts_len + oui_len; 
    /* subopcode */
    subopcode = *ptr;
    ptr = ptr + sizeof(subopcode);
    /* TOOL TLV */
    ptr = ptr + tl_len; /* TLV type and length */
    /* Count */
    tool_info.count = *ptr;
    ptr = ptr + sizeof(tool_info.count); 
    /* Mepid */
    memcpy(&tool_info.mepid, ptr, sizeof(tool_info.mepid));
    ptr = ptr + sizeof(tool_info.mepid);
    /* loop */
    tool_info.loop = *ptr;
    ptr = ptr + sizeof(tool_info.loop);
    /* interval */ 
    tool_info.interval = *ptr;
    ptr = ptr + sizeof(tool_info.interval);
    /* pattern */ 
    tool_info.pattern = *ptr;
    ptr = ptr + sizeof(tool_info.pattern);
    /* pattern len */
    memcpy(&tool_info.pattern_len, ptr, sizeof(tool_info.pattern_len));
    ptr = ptr + sizeof(tool_info.pattern_len);
    /* interval us */
    memcpy(&tool_info.interval_us, ptr, sizeof(tool_info.interval_us));
    ptr = ptr + sizeof(tool_info.interval_us);
    
    OAM_DEBUG("Handle %s for MEP ID %d, count %d, interval %d, loop %d, "
              "pattern %d, and pattern length %d, interval us %d\n", 
              oam_mcc_subopcode_name(subopcode), 
              tool_info.mepid, tool_info.count, tool_info.interval, 
              tool_info.loop, tool_info.pattern, tool_info.pattern_len,
              tool_info.interval_us);

    switch (subopcode) {
    case OAM_SUB_OPCODE_SLM:  
        oam_start_x(&tool_info, OAM_SUB_OPCODE_SLM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start SLM\n");
            err =  -1;
            goto out_err;
        }
        break;   
    case OAM_SUB_OPCODE_LMM:  
        oam_start_x(&tool_info, OAM_SUB_OPCODE_LMM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start LMM\n");
            err =  -1;
            goto out_err;
        }
        break;
    case OAM_SUB_OPCODE_1DM:
        oam_start_x(&tool_info, OAM_SUB_OPCODE_1DM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start 1DM\n");
            err =  -1;
            goto out_err;
        }
        break;
    case OAM_SUB_OPCODE_DMM:
        oam_start_x(&tool_info, OAM_SUB_OPCODE_DMM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start DMM\n");
            err =  -1;
            goto out_err;
        }
        break;
    case OAM_SUB_OPCODE_LBM:
        err = oam_start_x(&tool_info, OAM_SUB_OPCODE_LBM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start LBM\n"); 
            err = -1;
            goto out_err;
        }
        break; 
    case OAM_SUB_OPCODE_LTM:
        err = oam_start_x(&tool_info, OAM_SUB_OPCODE_LTM);
        if (err < 0) {
            OAM_ERROR("Failed to handle start LTM\n"); 
            err = -1;
            goto out_err;
        }
        break;
    case OAM_SUB_OPCODE_TST:
        err = oam_start_x(&tool_info, OAM_SUB_OPCODE_TST);
        if (err < 0) {
            OAM_ERROR("Failed to handle start TST\n"); 
            err = -1;
            goto out_err;
        }
        break;         
    case OAM_SUB_OPCODE_LCK:
        err = oam_start_x(&tool_info, OAM_SUB_OPCODE_LCK);
        if (err < 0) {
            OAM_ERROR("Failed to handle start LCK\n"); 
            err = -1;
            goto out_err;
        }
        break;   
    default:
        break;
    }
    if (err) {
        err = -1;
    }
out_err: 
    free(out_common_pdu);
    return err;
}

/**
 * Handle packet LMM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param fd_out The file desctiptor for outputting the LMR
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_lmm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out)
{ 
    int                     err             = 0;
    int                     ifindex         = 0;
    int                     returned_length = 0;
    unsigned char          *ptr;
    unsigned char          *dst_mac;
    uint32_t                rx_packets;
    struct oam_pdu_lmm     *lmm_in          = NULL; 
    struct oam_pdu_lmr     *lmr_ptr         = NULL;
    struct oam_pdu_common  *pdu_out         = NULL;
    struct oam_entity_stat *found           = NULL;

    extern struct oam_entity *entity;
 
    ptr = (unsigned char *)common_pdu;
    dst_mac = (unsigned char *)recv_info->from_mac;

    /* Handle the packet*/    
    lmm_in = (struct oam_pdu_lmm *)ptr;
    if (!lmm_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } else {
        pdu_out = oam_alloc_pdu(); 
        lmr_ptr = calloc(1, sizeof(struct oam_pdu_lmr));
    }

    found = oam_find_by_mac(dst_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        goto out_err;
    } else {
        ifindex = found->ifindex;
    }

    lmr_ptr->opcode  = OAM_OPCODE_LMR; 

    /* the last received txfcf */
    memcpy(&lmr_ptr->txfcf, &found->lmm_txfcf, sizeof(found->lmm_txfcf));
    OAM_DEBUG("Prev recvd TxFCf: %d\n", lmr_ptr->txfcf);
    /* store the current rxfcf */
    OAM_DEBUG("Now recvd TxFCf: %d\n", lmm_in->txfcf);
    memcpy(&lmr_ptr->txfcb, &found->lmm_txfcf, sizeof(found->lmm_txfcf));
    memcpy(&found->lmm_txfcf, &lmm_in->txfcf, sizeof(found->lmm_txfcf));
    OAM_DEBUG("Stored TxFCf: %d\n", found->lmm_txfcf);
    /* RX packets */
    rx_packets = oam_ask_rtx_packets(found->ifindex_name, OAM_RX);
    memcpy(&lmr_ptr->rxfcf, &rx_packets, sizeof(lmr_ptr->rxfcf));

    returned_length = oam_build_pdu(pdu_out,
                                    (const struct oam_pdu_common *const) 
                                    lmr_ptr, 
                                    *dst_mac);
    /* Send LMR */
    if (entity->send(fd_out, 
                     dst_mac, 
                     (const struct oam_pdu_common *const)pdu_out, 
                     returned_length,
                     ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        err = -1; 
    } 

out_err:

    free(pdu_out);
    return err;
}

/**
 * Handle packet LMR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_lmr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char          *ptr;
    unsigned char          *from_mac;
    double                  loss_rate       = 0;
    uint32_t                frame_loss      = 0;
    struct oam_pdu_lmr     *lmr_in          = NULL; 
    struct oam_entity_stat *found           = NULL;
 
    ptr = (unsigned char *)common_pdu;
    from_mac = (unsigned char *)recv_info->from_mac;

    /* Handle the packet*/    
    lmr_in = (struct oam_pdu_lmr *)ptr;
    if (!lmr_in) {
        OAM_DEBUG("No inbound packet to handle\n");
        return -1;
    }

    found = oam_find_by_mac(from_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        return -1;
    }
    
    OAM_DEBUG("MEP ID %d\n",found->mepid);
    OAM_DEBUG("TxFCf %d\n",lmr_in->txfcf);
    OAM_DEBUG("TxFCf %d\n",lmr_in->rxfcf);
    OAM_DEBUG("TxFCf %d\n",lmr_in->txfcb);

    OAM_STAT("LMR MEP ID %d, TxFCf %d, RxFCf %d, TxFCb\n", 
             found->mepid, lmr_in->txfcf, lmr_in->rxfcf, lmr_in->txfcf);

    /* Frame loss measurement */

    frame_loss = oam_calculate_frame_loss(lmr_in->txfcb,
                                          lmr_in->txfcf,
                                          found->lmm_rxfcb,
                                          lmr_in->rxfcf);

    if (lmr_in->txfcb > 0) {
        loss_rate = (double)abs(frame_loss) / (double)lmr_in->txfcb;
    } else {
        loss_rate = 0;
    }

    OAM_STAT_INFO("LMM: frame loss %d, rate %f%%\n", frame_loss, loss_rate);

    memcpy(&found->lmm_rxfcb, &lmr_in->rxfcf, sizeof(found->lmm_rxfcb));

    return 0;
}

/**
 * Handle packet 1DM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_1dm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char           *ptr;
    unsigned char           *dst_mac;
    uint32_t                 diff = 0;
    char                     print_time_txf[OAM_INTERNAL_TIME_TXT_LEN];
    char                     print_time_recv[OAM_INTERNAL_TIME_TXT_LEN];
    struct oam_pdu_dmm      *dm1_in          = NULL; 
    struct oam_entity_stat  *found           = NULL;
    struct oam_internal_time received_txf;
    struct oam_internal_time received;

    ptr = (unsigned char *)common_pdu;
    dst_mac = (unsigned char *)recv_info->from_mac;

    oam_get_internal_time(&received);

    /* Handle the packet*/    
    dm1_in = (struct oam_pdu_dmm *)ptr;
    if (!dm1_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } 
    found = oam_find_by_mac(dst_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        return -1;
    }
    memcpy(&received_txf.seconds, &dm1_in->txtimestampf_sec, 
           sizeof(received_txf.seconds));
    memcpy(&received_txf.nanoseconds, &dm1_in->txtimestampf_nano, 
           sizeof(received_txf.nanoseconds));
    oam_internal_time_str(print_time_txf, 
                          sizeof(print_time_txf), 
                          &received_txf);
    oam_internal_time_str(print_time_recv, 
                          sizeof(print_time_recv), 
                          &received);
    diff = oam_internal_time_diff(&received_txf, &received);
    OAM_STAT_INFO("1DM MED ID %d, Recvd TxTimestampf: %s "
                  "recvd at %s, delay %d (usecs)\n", 
                  found->mepid, 
                  print_time_txf, 
                  print_time_recv,
                  diff / OAM_USECS_TO_NANO);
    return 0; 
}

/**
 * Handle packet DMM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 * @param fd_out Socket that is used to send the DMR
 * @param dm_optional_fields Are the optional fields in DMR filled
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_dmm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out,
                   enum oam_onoff dm_optional_fields)
{ 
    int                      err             = 0;
    int                      ifindex         = 0;
    int                      returned_length = 0;
    unsigned char           *ptr;
    unsigned char           *dst_mac;
    char                     print_time[OAM_INTERNAL_TIME_TXT_LEN];
    struct oam_pdu_dmm      *dmm_in          = NULL; 
    struct oam_pdu_dmr      *dmr_ptr         = NULL;
    struct oam_pdu_common   *pdu_out         = NULL;
    struct oam_entity_stat  *found           = NULL;
    struct oam_internal_time received;
    struct oam_internal_time sent; 
    struct oam_internal_time recv_txf;

    extern struct oam_entity        *entity;

    oam_get_internal_time(&received);

    ptr = (unsigned char *)common_pdu;
    dst_mac = (unsigned char *)recv_info->from_mac;

    /* Handle the packet*/    
    dmm_in = (struct oam_pdu_dmm *)ptr;
    if (!dmm_in) {
        OAM_DEBUG("No inbound packet to handle\n");
    } else {
        pdu_out = oam_alloc_pdu(); 
        dmr_ptr = calloc(1, sizeof(struct oam_pdu_dmr));
    }

    found = oam_find_by_mac(dst_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        goto out_err;
    } else {
        ifindex = found->ifindex;
    }

    dmr_ptr->opcode  = OAM_OPCODE_DMR; 

    memcpy(&dmr_ptr->txtimestampf_sec, 
           &dmm_in->txtimestampf_sec, 
           sizeof(dmm_in->txtimestampf_sec));
    memcpy(&dmr_ptr->txtimestampf_nano, 
           &dmm_in->txtimestampf_nano, 
           sizeof(dmm_in->txtimestampf_nano));

    memcpy(&recv_txf.seconds, &dmr_ptr->txtimestampf_sec, 
           sizeof(recv_txf.seconds));
    memcpy(&recv_txf.nanoseconds, &dmr_ptr->txtimestampf_nano, 
           sizeof(recv_txf.nanoseconds));
    oam_internal_time_str(print_time, sizeof(print_time), &recv_txf);
    OAM_DEBUG("Recvd TxTimestampf: %s\n", print_time);

    /* Do we fill the DMM reception time ourselves or not. */
    if (dm_optional_fields == ON) {
        memcpy(&dmr_ptr->rxtimestampf_sec, 
               &received.seconds, 
               sizeof(dmr_ptr->txtimestampf_sec));
        memcpy(&dmr_ptr->rxtimestampf_nano, 
               &received.nanoseconds, 
               sizeof(dmr_ptr->txtimestampf_nano));

        oam_get_internal_time(&sent);

        memcpy(&dmr_ptr->txtimestampb_sec, 
               &sent.seconds, 
               sizeof(dmr_ptr->txtimestampb_sec));
        memcpy(&dmr_ptr->txtimestampb_nano, 
               &sent.nanoseconds, 
               sizeof(dmr_ptr->txtimestampb_nano));
    } else {
        memcpy(&dmr_ptr->rxtimestampf_sec, 
               &dmm_in->reserved_dmm_secs, 
               sizeof(dmr_ptr->rxtimestampf_sec));
        memcpy(&dmr_ptr->rxtimestampf_nano, 
               &dmm_in->reserved_dmm_nano, 
               sizeof(dmr_ptr->rxtimestampf_nano));
    }
    
    returned_length = oam_build_pdu(pdu_out,
                                    (const struct oam_pdu_common *const) 
                                    dmr_ptr, 
                                    *dst_mac);

    /* Send DMR */
    if (entity->send(fd_out, 
                     dst_mac, 
                     (const struct oam_pdu_common *const)pdu_out, 
                     returned_length,
                     ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        err = -1; 
    } 

out_err:
 
    free(pdu_out);
    return err;
}

/**
 * Handle packet DMR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family etc.
 * @param configuration contains the configuration information. 
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_dmr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const struct oam_configuration *const configuration)
{
    unsigned char           *ptr;
    unsigned char           *from_mac;
    char                     time_rx[OAM_INTERNAL_TIME_TXT_LEN];
    char                     time_txf[OAM_INTERNAL_TIME_TXT_LEN];
    char                     time_rxf[OAM_INTERNAL_TIME_TXT_LEN];
    char                     time_txb[OAM_INTERNAL_TIME_TXT_LEN];
    struct oam_pdu_dmr      *dmr_in          = NULL; 
    struct oam_entity_stat  *found; 
    struct oam_internal_time rx;
    struct oam_internal_time txf;
    struct oam_internal_time rxf;
    struct oam_internal_time txb;
    uint32_t                 frame_delay = 0;
    uint32_t                 processing_delay = 0;
    struct oam_ep2           *ep2_ptr;

    oam_get_internal_time(&rx);
    oam_internal_time_str(time_rx, sizeof(time_rx), &rx);

    ptr = (unsigned char *)common_pdu;
    from_mac = (unsigned char *)recv_info->from_mac;

    /* Handle the packet*/    
    dmr_in = (struct oam_pdu_dmr *)ptr;
    if (!dmr_in) {
        OAM_DEBUG("No inbound packet to handle\n");
        return -1;
    }

    found = oam_find_by_mac(from_mac);
    if (!found) {
        OAM_ERROR("Did not find correct entity\n");
        return -1;
    }

    memcpy(&txf.seconds, 
           dmr_in->txtimestampf_sec,
           sizeof(txf.seconds));
    memcpy(&txf.nanoseconds, 
           dmr_in->txtimestampf_nano,
           sizeof(txf.nanoseconds));
    oam_internal_time_str(time_txf, sizeof(time_txf), &txf);

    /* optional RxTimestampF */
    memcpy(&rxf.seconds, 
           dmr_in->rxtimestampf_sec,
           sizeof(rxf.seconds));
    memcpy(&rxf.nanoseconds, 
           dmr_in->rxtimestampf_nano,
           sizeof(rxf.nanoseconds));
    oam_internal_time_str(time_rxf, sizeof(time_rxf), &rxf);

    /* optional TxTimestampb */
    memcpy(&txb.seconds, 
           dmr_in->txtimestampb_sec,
           sizeof(txb.seconds));
    memcpy(&txb.nanoseconds, 
           dmr_in->txtimestampb_nano,
           sizeof(txb.nanoseconds));
    oam_internal_time_str(time_txb, sizeof(time_txb), &txb);

    processing_delay = oam_internal_time_diff(&rxf, &txb) / OAM_USECS_TO_NANO;
    frame_delay = (oam_internal_time_diff(&txf, &rx) - processing_delay) / 
        OAM_USECS_TO_NANO; 
    OAM_STAT_INFO("DMR MEP ID %d, Recv time %s, Txtimestampf %s,"
                  " Rxtimestampf %s, Txtimestampb %s, "
                  "frame delay (usec) %d, processing delay (usec) %d\n", 
                  found->mepid, 
                  time_rx, 
                  time_txf, 
                  time_rxf,
                  time_txb,
                  frame_delay,
                  processing_delay);

    /* add the frame delay to ep2 structure */
    ep2_ptr = found->delay_results;
    oam_ep2_add_value(frame_delay, ep2_ptr); 
    //oam_ep2_pretty_print(ep2_ptr);
    /* Check to see if we have enough samples we hit a threshold or limit for error or warning */
    if (ep2_ptr->num_of_observations % configuration->dm_samples == 0) {
	oam_send_ccd_stat(found);
    }
    if (frame_delay > configuration->dm_th_warning) {
        found->dm_num_of_warnings++;
    }
    if (frame_delay > configuration->dm_th_error) {
        found->dm_num_of_errors++;
    }
    if (found->dm_num_of_errors >= configuration->dm_error) {
        oam_send_ccd("DM to %d error limit exceeded\n", found->mepid);
        oam_send_ccd_stat(found);
        found->dm_num_of_errors = 0;
    }
    if (found->dm_num_of_warnings >= configuration->dm_warning) {
        oam_send_ccd("DM to %d warning limit exceeded\n", found->mepid);
        oam_send_ccd_stat(found);
        found->dm_num_of_warnings = 0;
    }
    OAM_DEBUG("DM Info samples %d, observations %d, errors %d, warnings %d\n", 
              configuration->dm_samples, ep2_ptr->num_of_observations, 
              found->dm_num_of_errors, found->dm_num_of_warnings);

    return 0;
}

/**
 * Handle packet EXM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_exm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char *ptr;
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    ptr = (unsigned char *)common_pdu;
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented (%d)\n", ptr);
    return -1;
}

/**
 * Handle packet EXR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_exr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char *ptr;
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    ptr = (unsigned char *)common_pdu;
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented (%d)\n", ptr);
    return -1;
}

/**
 * Handle packet VSR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_vsr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char *ptr;
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    ptr = (unsigned char *)common_pdu;
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented (%d)\n", ptr);
    return -1;
}

/**
 * Handle packet VSM
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
int oam_handle_vsm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info)
{
    unsigned char *ptr;
    char           mac_addr[]        = "00:00:00:00:00:00"; 
    ptr = (unsigned char *)common_pdu;
    oam_mac_ntop((char *const)mac_addr,  
                 (const unsigned char *const)recv_info->from_mac);
    OAM_DEBUG("MAC %s\n", mac_addr);    
    OAM_DEBUG("Packet handling not implemented (%d)\n", ptr);
    return -1;
}
