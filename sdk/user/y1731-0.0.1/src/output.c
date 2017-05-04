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

/**
 * Feature test macro for struct ifreq */
#define _BSD_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <asm/sockios.h>

#include <netpacket/packet.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdarg.h>

#include "extendedpsquared.h"
#include "constants.h"
#include "ife.h"
#include "pdu.h"
#include "output.h"
#include "tools.h"

#define OAM_ARPHRD_ETHER       1 /**< Ethernet hardware format */

#define BUFLEN 1024 /**< buffer length for the oam_send_ccd */
#define MAX_MSG 512 /**< Maximum msg leng in oam_send_ccd */

/**
 * This is a wrapper for the messages containing statistics information
 * uses the function defined below to actually send the message
 *
 * @see oam_send_ccd
 */
int oam_send_ccd_stat(struct oam_entity_stat *report)
{
    struct oam_ep2 *ep2;
    
    ep2 = report->delay_results;
    //oam_ep2_pretty_print(report->delay_results);
    oam_send_ccd("STAT:%d::%f:::%f::::%f:::::\n", 
                 report->mepid,
                 ep2->q[0],
                 ep2->q[ep2->num_of_markers / 2],
                 ep2->q[ep2->num_of_markers - 1]);
   return 0;
}

/** 
 * This is the sending of the UDP message to the central server.
 * 
 * @param fmt printf formatting options
 * @param ... variable number of strings or integers to print
 *        according to the @c fmt parameter
 *
 * @return Zero on success, non-zero else.
 */
int oam_send_ccd(const char *fmt, ...)
{

    va_list            args;
    char               ccd_msg[MAX_MSG];
    int                s;
    socklen_t          slen;
    char               buf[BUFLEN];
    struct sockaddr_in si_other;

    extern struct  oam_entity *entity;
    
    if (entity->ccd == OFF) {
        return 0;
    }

    slen = sizeof(si_other);

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        perror("socket");
        return -1;
    }
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(entity->srv_port);
    if (inet_aton(entity->srv_ip, &si_other.sin_addr)==0) {
        OAM_ERROR("inet_aton() failed\n");
        return -1;
    }

    memset(ccd_msg, '\0', sizeof(ccd_msg));
    va_start(args, fmt);
    vsnprintf(ccd_msg, MAX_MSG, fmt, args);    
    va_end(args);
    
    OAM_DEBUG("CCD: MEP ID: %d, %s\n", htons(entity->mepid), ccd_msg);
    sprintf(buf, "MEP ID: %d, %s\n", htons(entity->mepid), ccd_msg);
    if (sendto(s, 
               buf, 
               BUFLEN, 
               0, 
               (const struct sockaddr *)&si_other, 
               slen)==-1) {
        perror("sendto()");
        return -1;
    }
    
    close(s);
    return 0;
}

/**
 * Function that checks the internal table for mac of the interface
 * 
 * @param ifrname the interface name of the interface we need the MAC for
 * 
 * @return Pointer to MAC as unsigned char or NULL if not found.
 * 
 * @note Remember not to read beyond the MAC!
 */
unsigned char *oam_get_mac_of_interface_name(char * ifrname) 
{
    int            i;
    struct oam_if *ifs_ptr;

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = ifs_len; --i >= 0; ifs_ptr++) {
        if (strcmp(ifs_ptr->ifindex_name, ifrname) == 0) {
            return (unsigned char *)ifs_ptr->mac;
        }
    }
    return NULL;
}

/**
 * Function that checks the internal table for mac of the interface
 * 
 * @param ifindex the interface name of the interface we need the MAC for
 * 
 * @return Pointer to MAC as unsigned char or NULL if not found.
 * 
 * @note Remember not to read beyond the MAC!
 */
unsigned char *oam_get_mac_of_interface_index(int ifindex) 
{
    int            i;
    struct oam_if *ifs_ptr;

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = ifs_len; --i >= 0; ifs_ptr++) {
        if (ifs_ptr->ifindex == ifindex) {
            return (unsigned char *)ifs_ptr->mac;
        }
    }
    return NULL;
}
 
/**
 *
 * @param socket Socket used to send the packet
 * @param dst Destination address (unsigned char)
 * @param pdu PDU that has to be sent.
 * @param pdu_len Length of the PDU
 * @param if_index Interface index for sending
 *
 * @return Zero on success, non-zero else.
 *
 * @note DO NOT use OAM_STAT inside here as this is called
 *       also from the oamtool side and the file pointer does
 *       not exist there.
 */
int oam_send_eth(const int socket, 
                 const void *const dst, 
                 const struct oam_pdu_common *const pdu, 
                 const int pdu_len,
                 const int if_index)
{
    int                      err         = 0;
    unsigned char           *buffer;
    char                     sa_mac[20];
    char                     da_mac[20];
    char                     buf[64];
    unsigned char           *our_mac;
    unsigned char           *payload     = NULL;
    unsigned char           *etherheader = NULL;
    struct ethhdr           *eh          = NULL;
    struct sockaddr_ll       socket_address; //target address
    struct oam_entity_stat  *found       = NULL;
    struct timeval           tv;
    struct oam_internal_time internal_time;

    extern unsigned char da1[6];
    extern unsigned char da2[6];
    //extern struct oam_entity *entity;

    memset(&socket_address, 0, sizeof(socket_address));

    if (!(buffer = calloc(1, OAM_MAX_PACKET))) { 
        OAM_ERROR("Failed to calloc memory for the outgoing packet.\n");
        return -ENOMEM;
    }
    etherheader = buffer;
    payload = buffer + OAM_ETH_HEADER_LENGTH;
    eh = (struct ethhdr *)etherheader;
    
    socket_address.sll_family   = PF_PACKET;	    
    socket_address.sll_halen    = ETH_ALEN;
    socket_address.sll_ifindex = if_index;
    memcpy(&socket_address.sll_addr, dst, ETH_ALEN);
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    if (memcmp(&da1, dst, OAM_ETH_MAC_LENGTH) != 0 &&
        memcmp(&da2, dst, OAM_ETH_MAC_LENGTH) != 0) {

        found = oam_find_by_mac(dst);
 
        if (found == NULL) {
            OAM_ERROR("Could not find the target\n");
            return -1;
        }
        our_mac = oam_get_mac_of_interface_name(found->ifindex_name);
    } else {
        our_mac = oam_get_mac_of_interface_index(if_index);
    }

    if (our_mac == NULL) {
        OAM_ERROR("Failed to find our send information\n");
        return -1;
    }
    memcpy((void*)buffer, (void*)dst, ETH_ALEN);
    memcpy((void*)(buffer + ETH_ALEN), (void*)our_mac, ETH_ALEN);
    eh->h_proto = htons(OAM_ETH_P_CFM);

    /* copy the payload */
    memcpy(payload, pdu, pdu_len);

    /*send the packet*/
    err = sendto(socket, 
                 buffer, 
                 OAM_ETH_HEADER_LENGTH + pdu_len, 
                 0, 
                 (struct sockaddr*)&socket_address, 
                 sizeof(socket_address)); 

    if (err < 0) {
        perror("SendTo PERROR ");
    }
    oam_mac_ntop((char * const)&da_mac, 
                 (unsigned char * const)&socket_address.sll_addr);
    oam_mac_ntop((char * const)&sa_mac, (unsigned char * const)our_mac);
    
    OAM_DEBUG("%s S %s D %s\n", 
              oam_packet_type_name_short(pdu->opcode), 
              sa_mac, da_mac);

    gettimeofday(&tv, NULL);
    oam_tv_to_internal_time(&tv, &internal_time); 
    oam_internal_time_str(buf, sizeof(buf), &internal_time);
    OAM_XTRA("%s SW Timestamp %s\n", 
             oam_packet_type_name_short(pdu->opcode), 
             buf);
    err = 0;
    
    free(buffer);
    return err; 
}

/**
 * Stand alone version. Used only in oampinger and oamtrace. 
 *
 * @param socket Socket used to send the packet
 * @param dst Destination address (unsigned char)
 * @param pdu PDU that has to be sent.
 * @param pdu_len Length of the PDU
 * @param if_index Interface index for sending
 *
 * @return Zero on success, non-zero else.
 *
 * @note DO NOT use OAM_STAT inside here as this is called
 *       also from the oamtool side and the file pointer does
 *       not exist there.
 */
int oam_send_eth_sa(const int socket,  
                    const void *const dst, 
                    const struct oam_pdu_common *const pdu, 
                    const int pdu_len,
                    const int if_index)
{
    int                      err         = 0;
    unsigned char           *buffer;
    char                     sa_mac[20];
    char                     da_mac[20];
    char                     buf[64];
    unsigned char           *our_mac;
    unsigned char           *payload     = NULL;
    unsigned char           *etherheader = NULL;
    struct ethhdr           *eh          = NULL;
    struct sockaddr_ll       socket_address; //target address
    struct timeval           tv;
    struct oam_internal_time internal_time;

    extern unsigned char da1[6];
    extern unsigned char da2[6];
    //extern struct oam_entity *entity;

    memset(&socket_address, 0, sizeof(socket_address));

    if (!(buffer = calloc(1, OAM_MAX_PACKET))) { 
        OAM_ERROR("Failed to calloc memory for the outgoing packet.\n");
        return -ENOMEM;
    }
    etherheader = buffer;
    payload = buffer + OAM_ETH_HEADER_LENGTH;
    eh = (struct ethhdr *)etherheader;
    
    socket_address.sll_family   = PF_PACKET;	    
    socket_address.sll_halen    = ETH_ALEN;
    socket_address.sll_ifindex = if_index;
    memcpy(&socket_address.sll_addr, dst, ETH_ALEN);
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    our_mac = oam_get_mac_of_interface_index(if_index);

    if (our_mac == NULL) {
        OAM_ERROR("Failed to find our send information\n");
        return -1;
    }
    memcpy((void*)buffer, (void*)dst, ETH_ALEN);
    memcpy((void*)(buffer + ETH_ALEN), (void*)our_mac, ETH_ALEN);
    eh->h_proto = htons(OAM_ETH_P_CFM);

    /* copy the payload */
    memcpy(payload, pdu, pdu_len);

    /*send the packet*/
    err = sendto(socket, 
                 buffer, 
                 OAM_ETH_HEADER_LENGTH + pdu_len, 
                 0, 
                 (struct sockaddr*)&socket_address, 
                 sizeof(socket_address)); 

    if (err < 0) {
        perror("SendTo PERROR ");
    }
    oam_mac_ntop((char * const)&da_mac, 
                 (unsigned char * const)&socket_address.sll_addr);
    oam_mac_ntop((char * const)&sa_mac, (unsigned char * const)our_mac);
    
    /*
    OAM_DEBUG("%s S %s D %s\n", 
              oam_packet_type_name_short(pdu->opcode), 
              sa_mac, da_mac);
    */

    gettimeofday(&tv, NULL);
    oam_tv_to_internal_time(&tv, &internal_time); 
    oam_internal_time_str(buf, sizeof(buf), &internal_time);
    /*
    OAM_XTRA("%s SW Timestamp %s\n", 
             oam_packet_type_name_short(pdu->opcode), 
             buf);
    */
    err = 0;
    
    free(buffer);
    return err; 
}

/**
 * Packet forwarding version. Used only in MIP. 
 *
 * @param socket Socket used to send the packet
 * @param dst Destination address (unsigned char)
 * @param pdu PDU that has to be sent.
 * @param pdu_len Length of the PDU
 * @param if_index Interface index for sending
 *
 * @return Zero on success, non-zero else.
 *
 * @note NICs have to be in promiscuous mode.
 */
int oam_forward_eth(const int socket,  
                    const void *const dst, 
                    const struct oam_pdu_common *const pdu, 
                    const int pdu_len,
                    const int if_index)
{
    int                      err         = 0;
    unsigned char           *buffer;
    char                     sa_mac[20];
    char                     da_mac[20];
    char                     buf[64];
    unsigned char           *our_mac;
    unsigned char           *payload     = NULL;
    unsigned char           *etherheader = NULL;
    struct ethhdr           *eh          = NULL;
    struct sockaddr_ll       socket_address; //target address
    struct timeval           tv;
    struct oam_internal_time internal_time;

    extern unsigned char da1[6];
    extern unsigned char da2[6];

    memset(&socket_address, 0, sizeof(socket_address));

    if (!(buffer = calloc(1, OAM_MAX_PACKET))) { 
        OAM_ERROR("Failed to calloc memory for the outgoing packet.\n");
        return -ENOMEM;
    }
    etherheader = buffer;
    payload = buffer + OAM_ETH_HEADER_LENGTH;
    eh = (struct ethhdr *)etherheader;
    
    socket_address.sll_family   = PF_PACKET;	    
    socket_address.sll_halen    = ETH_ALEN;
    socket_address.sll_ifindex = if_index;
    memcpy(&socket_address.sll_addr, dst, ETH_ALEN);
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    /* XXX need to change this in the forwarding see the 
       addr from below  but will have problems in promisc mode */
    our_mac = oam_get_mac_of_interface_index(if_index);

    if (our_mac == NULL) {
        OAM_ERROR("Failed to find our send information\n");
        return -1;
    }
    memcpy((void*)buffer, (void*)dst, ETH_ALEN);
    memcpy((void*)(buffer + ETH_ALEN), (void*)our_mac, ETH_ALEN);
    eh->h_proto = htons(OAM_ETH_P_CFM);

    /* copy the payload */
    memcpy(payload, pdu, pdu_len);

    /*send the packet*/
    err = sendto(socket, 
                 buffer, 
                 OAM_ETH_HEADER_LENGTH + pdu_len, 
                 0, 
                 (struct sockaddr*)&socket_address, 
                 sizeof(socket_address));  
    if (err < 0) {
        perror("SendTo PERROR ");
    }

    oam_mac_ntop((char * const)&da_mac, 
                 (unsigned char * const)&socket_address.sll_addr);
    oam_mac_ntop((char * const)&sa_mac, (unsigned char * const)our_mac);
    
    OAM_DEBUG("Forwarded a frame %s S %s D %s\n", 
              oam_packet_type_name_short(pdu->opcode), 
              sa_mac, da_mac);

    gettimeofday(&tv, NULL);
    oam_tv_to_internal_time(&tv, &internal_time); 
    oam_internal_time_str(buf, sizeof(buf), &internal_time);
    
    OAM_XTRA("%s SW Timestamp %s\n", 
             oam_packet_type_name_short(pdu->opcode), 
             buf);
    
    err = 0;
    free(buffer);
    return err; 
}

/**
 * This function handles the sending of the packet locally
 *
 * @param fd_out Outbound local socket to be used in sending.
 * @param local_common_pdu Msg to send
 * @param length Length of the msg
 *
 * @return Zero on success, non-zero else.
 */
int oam_send_local(int fd_out, 
                   struct oam_pdu_common *local_common_pdu,
                   int length)
{
    struct sockaddr_in         mepd_addr;

    memset(&mepd_addr, 0, sizeof(struct sockaddr_in));

    mepd_addr.sin_family      = AF_INET;
    mepd_addr.sin_port        = htons(OAM_MEPD_LOCAL_PORT);
    mepd_addr.sin_addr.s_addr = inet_addr(OAM_LOOPBACK);
    
    if (sendto(fd_out, local_common_pdu, length,
               0, (struct sockaddr *)&mepd_addr, 
               sizeof(struct sockaddr_in)) == -1) {
        OAM_ERROR("Failed to send local message\n");
        return -1;
    }

    OAM_DEBUG("Successfully sent local message\n");
    return 0;
}

/**
 * This function adds the tool TLV to the packet
 *
 * TOOL TLV 
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Type          | Length                        | Count         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | MEP ID                        |Loop           | Interval      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Pattern       |  Pattern Len                  | Interval us   \
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \ interval us                                   | Reserved      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Count: How many times PDUs are sent.
 *        If zero the sending will continue until stopped with 
 *        another local MCC with PDU subopcode to the corresponding
 *        MEP ID.
 *        
 * MEP ID: uint16_t representation of the mepid
 * 
 * Loop: is the PDU to be sent continuously 
 *
 * Interval: uint8_t representation of the interval
 *
 * Pattern: This is used by TST. indicates the OAM_PATTERN_TYPE
 * 
 * Pattern Len: Length of the pattern
 * 
 * Interval us: SLM interval, arbitary us interval. 
 *
 * @param local_common_pdu Msg to which we are adding
 * @param length Start of the place where to add
 * @param mepid MEP ID
 * @param count How many messages will be sent
 * @param loop 1 if it should the sending be continuous
 * @param interval Interval
 * @param pattern Which pattern to use with TST. 
 * @param len the length of the pattern
 * @param interval_us Interval for SLM
 *
 * @return the length of the packet after the addition
 */
static int oam_add_tool_tlv(struct oam_pdu_common *local_common_pdu, 
                            int length, 
                            uint16_t mepid, 
                            uint8_t count,
                            uint8_t loop,
                            uint8_t interval,
                            uint8_t pattern,
                            uint16_t pattern_len,
                            uint32_t interval_us)
{
    const uint8_t type_len  = 3;
    const uint16_t tlv_len = sizeof(count) + sizeof(mepid) + 
                             sizeof(loop) + sizeof(interval) + 
                             sizeof(pattern) + sizeof(pattern_len) + 
                             sizeof(interval_us);  
                             /*see above, just the V from TLV */
    uint8_t interval_in = interval; 
    uint32_t interval_us_in = interval_us;
    unsigned char * ptr = (unsigned char *)local_common_pdu;

    ptr = ptr + length; /* Go to start of the TLV */

    /* Type */
    *ptr = OAM_TLV_TYPE_TOOL;
    ptr = ptr + sizeof(type_len);

    /* Length */
    memcpy(ptr, &tlv_len, sizeof(tlv_len));
    ptr = ptr + sizeof(tlv_len);

    /* Count */
    *ptr = count;
    ptr = ptr + sizeof(count);

    /* MEP ID */
    memcpy(ptr, &mepid, sizeof(mepid));
    ptr = ptr + sizeof(mepid);
    
    /* Loop */
    *ptr = loop;
    ptr = ptr + sizeof(loop);

    /* interval */
    *ptr = interval_in;
    ptr = ptr + sizeof(interval_in);

    /* Pattern */
    *ptr = pattern;
    ptr = ptr + sizeof(pattern);

    /* Pattern length */
    memcpy(ptr, &pattern_len, sizeof(pattern_len));
    ptr = ptr + sizeof(pattern_len);

    /* Interval us */
    memcpy(ptr, &interval_us_in, sizeof(interval_us_in));
    ptr = ptr + sizeof(interval_us_in);

    return length + tlv_len + type_len;
}

/**
 * This function adds the test TLV to the packet
 *
 * TST TLV 
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Type          | Length                        | Pattern type  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \ Test Pattern                                                  \
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Checksum                                                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Pattern: Pattern type NULL (all zeroes), PRBS,...
 * 
 * checksum: CRC-2 optional checksum
 *
 * @param local_common_pdu Msg to which we are adding
 * @param length Start of the place where to add
 * @param pattern Which pattern to use with TST. 
 * @param len the length of the pattern
 *
 * @return the length of the packet after the addition
 */
static int oam_add_tst_tlv(struct oam_pdu_common *local_common_pdu, 
                           int length, 
                           uint8_t pattern,
                           uint16_t pattern_len)
{
    const uint8_t  type_len  = 3;
    const uint8_t  checksum_len = 4;
    uint16_t       tlv_len = 0;
    int            i = 0;
    uint16_t       tlvl = 0;
    unsigned char *pattern_data;
    unsigned char *ptr;
    unsigned char *prbs = NULL;
    unsigned char  prbs_state_register[OAM_PRBS_SHIFT_REGISTER_LENGTH]; 

    ptr = (unsigned char *)local_common_pdu;     
    ptr = ptr + length; /* Go to start of the TLV */

    tlv_len = pattern_len + checksum_len + sizeof(pattern);    

    /* Type */
    *ptr = OAM_TYPE_TEST_TLV;
    ptr = ptr + sizeof(type_len);

    /* Length */
    tlvl = htons(tlv_len);
    memcpy(ptr, &tlvl, sizeof(tlv_len));
    ptr = ptr + sizeof(tlv_len);

    switch (pattern) {
    case 0:
        /* All zeroes (NULL) pattern */
        *ptr = OAM_PATTERN_TYPE_ZERO_NO_CRC;
        ptr++;
        memset(ptr, 0, pattern_len);
        /* Checksum (0) */
        ptr = ptr + pattern_len;
        break;
    case 1:
        *ptr = OAM_PATTERN_TYPE_ZERO_CRC;
        ptr++;
        /* save pointer to checksum input data */
        pattern_data = ptr;
        memset(ptr, 0, pattern_len);
        /* Move to checksum */
        ptr = ptr + pattern_len;
        oam_do_crc32_checksum(pattern_data, pattern_len, (uint32_t *)ptr);
        OAM_HEXDUMP("CRC32 ", ptr, 4);
        ptr = ptr + checksum_len;
        break;
    case 2:
        *ptr = OAM_PATTERN_TYPE_PRBS_NO_CRC;
        ptr++;    
        pattern_data = ptr;
        for (i = 0; i < OAM_PRBS_SHIFT_REGISTER_LENGTH; i++) { 
            prbs_state_register[i] = OAM_PRBS_BIT_ONE;
        }	 
        prbs = oam_generate_prbs(prbs_state_register, pattern_len);
        if (prbs == NULL) {
            OAM_ERROR("Could not create PRBS\n");
            return -1;
        }
        memcpy(pattern_data, prbs, pattern_len);
        ptr = ptr + pattern_len;
        break;
    case 3:
        *ptr = OAM_PATTERN_TYPE_PRBS_CRC;
        ptr++;
        pattern_data = ptr;
        for (i = 0; i < OAM_PRBS_SHIFT_REGISTER_LENGTH; i++) { 
            prbs_state_register[i] = OAM_PRBS_BIT_ONE;
        }	 
        prbs = oam_generate_prbs(prbs_state_register, pattern_len);
        if (prbs == NULL) {
            OAM_ERROR("Could not create PRBS\n");
            return -1;
        }
        memcpy(pattern_data, prbs, pattern_len);
        ptr = ptr + pattern_len;
        oam_do_crc32_checksum(pattern_data, pattern_len, (uint32_t *)ptr);
        OAM_HEXDUMP("CRC32 ", ptr, 4);
        ptr = ptr + checksum_len;
        break;
    default:
        /* All zeroes (NULL) pattern */
        *ptr = OAM_PATTERN_TYPE_ZERO_NO_CRC;
        ptr++;
        memset(ptr, 0, pattern_len);
        ptr = ptr + pattern_len;
    }
    
    free(prbs);
    return length + tlv_len + type_len;
}

/**
 * This function sends the local control message
 *
 * @param fd_out_local Local socket for sending 
 * @param out_common_pdu Msg buffer to use
 * @param mepid towards the msg is sent
 * @param count How many times this is sent
 * @param interval Interval to send in
 * @param loop Is the message to be sent continuously
 * @param pattern Pattern type
 * @param pattern_len Lenght of the pattern 
 * @param interval_us Interval for SLM
 * @param subopcode What message is sent.
 *
 * @return Zero on success, non-zero else.
 *
 * @see allowed pattern types OAM_PATTERN_TYPE_*
 */
int oam_send_mcc_x(int fd_out_local,
                   struct oam_pdu_common *out_common_pdu,
                   uint16_t mepid,
                   uint8_t count, 
                   uint8_t interval,
                   uint8_t loop,
                   uint8_t pattern,
                   uint16_t pattern_len,
                   uint32_t interval_us,
                   int subopcode)
{
    int                        err             = 0;
    int                        returned_length = 0;
    struct oam_pdu_mcc_no_end *mcc_ptr;
    
    extern struct oam_entity *entity;

    if (oam_acceptable_subopcode(subopcode)) {
        return -1;
    } 

    mcc_ptr = calloc(1, sizeof(struct oam_pdu_mcc_no_end));

    mcc_ptr->opcode       = OAM_OPCODE_MCC;
    mcc_ptr->subopcode    = subopcode;
       
    memset(out_common_pdu, 0, OAM_MAX_PACKET);
    
    returned_length = oam_build_pdu(out_common_pdu,
                                    (const struct oam_pdu_common *const)mcc_ptr,
                                    0);

    /* Add the TOOL TLV */
    returned_length = oam_add_tool_tlv(out_common_pdu, 
                                       returned_length, 
                                       mepid,
                                       count,
                                       loop,
                                       interval,
                                       pattern,
                                       pattern_len,
                                       interval_us);

    returned_length  = oam_build_end_tlv(out_common_pdu, returned_length);

    if (oam_send_local(fd_out_local, out_common_pdu, returned_length)) {
        OAM_ERROR("Failed to send local packet\n");
        err = -1;
        goto out_err;
    }

 out_err:
    free(mcc_ptr);
    return err;
}

/**
 * Fills the PDUs send control info.
 *
 * @param sc send control of the PDU where some of the tool info is strored.
 * @param tool_info Information sent from the tool
 *
 * @note if loop is 1 then the count is ignored.
 */ 
static void oam_do_start(struct oam_send_control *const sc,
                         struct oam_tool_information *const tool_info)
{
    /* If there is a interval defined override interval_us */
    if (tool_info->interval > 0) {
        tool_info->interval_us = give_legit_interval(tool_info->interval);
        OAM_DEBUG("Using interval %d us\n", tool_info->interval_us);     
    }
   
    if (tool_info->loop == 1) {
        /* if already on turn off*/
        if (sc->onoff == ON) {
            sc->onoff = OFF;
            sc->interval = 0;
            sc->count = 0;
            /* if not on then turn on */    
        } else {
            sc->onoff = ON; 
            sc->interval = tool_info->interval_us;     
            sc->count = 0;
            oam_get_internal_time(&sc->next_send);
        }
    } else {
        /* no loop flag */ 
        if (tool_info->count > 1) { 
            sc->onoff = OFF;
            sc->count = tool_info->count;
            sc->interval = tool_info->interval_us;    
            oam_get_internal_time(&sc->next_send); 
        } else {
            /* Mark a single */
            sc->onoff = OFF;
            sc->count = 1;
            sc->interval = 0;
        }
    }
}
/**
 * Marks the sending of the defined PDU for the given MEP ID
 *
 * @param tool_info Information sent from the tool
 * @param subopcode What PDU is in case
 *
 * @return Zero on success, non-zero else.
 */
int oam_start_x(struct oam_tool_information *tool_info, int subopcode)
{
    struct oam_entity_stat *ptr        = NULL;

    ptr = oam_find(htons(tool_info->mepid));
    if (ptr == NULL)  {
        OAM_ERROR("Did not find the MEP ID %d\n", tool_info->mepid);
        return -1;
    } else {
        switch (subopcode) {
        case OAM_SUB_OPCODE_SLM:
            oam_do_start(&ptr->sl_send_control, tool_info);
            /* figure out the is this global to what extent */
            ptr->slm_test_id = 1; 
            ptr->slm_txfcl_slm = 0; 
            ptr->slm_txfcl_slr = 0; 
            break;
        case OAM_SUB_OPCODE_LMM:
            oam_do_start(&ptr->lm_send_control, tool_info);
            break;
        case OAM_SUB_OPCODE_LTM:
            oam_do_start(&ptr->lt_send_control, tool_info);
            break;
        case OAM_SUB_OPCODE_DMM:
            oam_do_start(&ptr->dm_send_control, tool_info);
            break;
        case OAM_SUB_OPCODE_1DM:
            oam_do_start(&ptr->dm1_send_control, tool_info);
            break;
        case OAM_SUB_OPCODE_LBM:
            oam_do_start(&ptr->lb_send_control, tool_info);
            break;
        case OAM_SUB_OPCODE_TST:
            oam_do_start(&ptr->tst_send_control, tool_info);
            break;       
        case OAM_SUB_OPCODE_LCK:
            oam_do_start(&ptr->lck_send_control, tool_info);
            break;
        default:
            OAM_INFO("Unknown subopcode, could not start measurement\n");
            break;
        }
    }
   
    return 0;
}

/**
 * Function that translates the internal interval to the period used in 
 * the CCM flags.
 */
static uint8_t oam_get_ccm_flags(int ccm_period)
{
    switch (ccm_period) {
    case OAM_INTERVALS_IN_USEC_333MS: return OAM_CCM_PERIOD_THREEISH_MS;
    case OAM_INTERVALS_IN_USEC_10MS: return OAM_CCM_PERIOD_10MS;
    case OAM_INTERVALS_IN_USEC_100MS: return OAM_CCM_PERIOD_100MS;
    case OAM_INTERVALS_IN_USEC_1S: return OAM_CCM_PERIOD_1S;
    case OAM_INTERVALS_IN_USEC_10S: return OAM_CCM_PERIOD_10S;
    case OAM_INTERVALS_IN_USEC_1MIN: return OAM_CCM_PERIOD_1MIN;
    case OAM_INTERVALS_IN_USEC_10MIN: return OAM_CCM_PERIOD_10MIN;
    }
    return 0;
}

/**
 * Sends out the multicast CCM
 *
 * @param fd_out The socket for the output.
 * @param period from the configuration
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_ccm_multi(const int fd_out, int period)
{
    int                      err = 0;
    int                      returned_length;
    int                      i;
    unsigned char           *ptr;
    struct oam_pdu_common   *common_pdu;     
    struct oam_pdu_ccm      *ccm_ptr;
    struct oam_if           *ifs_ptr;

    extern int                ifs_len;
    extern struct oam_if      ifs[20];
    extern struct oam_db     *oam_control_info;
    extern unsigned char      da1[6];
    extern struct oam_entity *entity;

    common_pdu = oam_alloc_pdu();
    ccm_ptr = calloc(1, sizeof(struct oam_pdu_ccm));

    /* CCM PDU Raw sending loop */
    
    ptr = (unsigned char *)oam_control_info;
    ptr = ptr + sizeof(struct oam_db);

    ifs_ptr = ifs;
    for (i = 0; i < ifs_len; i++, ifs_ptr++) {
        memset(common_pdu, 0, OAM_MAX_PACKET);
        
        ccm_ptr->opcode       = OAM_OPCODE_CCM; 
        ccm_ptr->flags        = oam_get_ccm_flags(period);
        ccm_ptr->meg_reserved = OAM_MEG_ID_RESERVED;
        ccm_ptr->meg_format   = OAM_MEG_ID_FORMAT;
        ccm_ptr->meg_length   = OAM_MEG_ID_LENGTH; 

        memcpy(&ccm_ptr->mepid, &entity->mepid, 
               sizeof(ccm_ptr->mepid));
        memcpy(&ccm_ptr->meg_id, &entity->id_icc, 
               sizeof(ccm_ptr->meg_id)); 
        
        /* Last parameter dst mepid is zero as we do not need the counters */
        returned_length = oam_build_pdu(common_pdu,
                                        (const struct oam_pdu_common *const)
                                        ccm_ptr, 
                                        0);
        if (entity->send(fd_out, 
                         &da1, 
                         (const struct oam_pdu_common *const)common_pdu, 
                         returned_length,
                         ifs_ptr->ifindex)) {
            OAM_ERROR("Failed to send RAW ETHERNET packet\n");
            err = -1; 
            goto out_err;
        } 
    }

out_err:
    free(common_pdu);
    free(ccm_ptr);
    return err;
}

/**
 * Sends out all the CCMs to all the MEPs in the MEG
 *
 * @param fd_out The socket for the output.
 * @param one_way_lm 1 if sent 0 if not
 * @param period from the configuration
 * @param dynamic from the configuration
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_ccms(const int fd_out, int one_way_lm, int period, int dynamic)
{
    int                     err = 0;
    int                     returned_length;
    int                     i;
    unsigned char          *ptr;
    struct oam_entity_stat *ptr_stat;
    uint32_t                tx_packets;
    struct oam_pdu_common  *common_pdu;     
    struct oam_pdu_ccm     *ccm_ptr;
    uint8_t                 empty_mac[6];

    extern struct oam_entity *entity;
    extern struct oam_db    *oam_control_info;

    common_pdu = oam_alloc_pdu();
    ccm_ptr = calloc(1, sizeof(struct oam_pdu_ccm));

    /* CCM PDU Raw sending loop */
    
    ptr = (unsigned char *)oam_control_info;
    ptr = ptr + sizeof(struct oam_db);
    for (i=0; i < oam_control_info->participants; i++) {

        ptr = (unsigned char *)oam_control_info + 
            sizeof(struct oam_db) + 
            (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;

        /* The MEP ID may be read from the file without any 
         information on the MAC or the interface beyond the MEP 
        can be found. */
        if (dynamic == 2) {
            memset(&empty_mac, 0, sizeof(empty_mac));
            if (memcmp(ptr_stat->mac, &empty_mac, sizeof(empty_mac)) == 0) {
                /* skipping this one */
                continue;
            }
        }

        memset(common_pdu, 0, OAM_MAX_PACKET);
        
        ccm_ptr->opcode       = OAM_OPCODE_CCM; 
        ccm_ptr->flags        = oam_get_ccm_flags(period);
        ccm_ptr->meg_reserved = OAM_MEG_ID_RESERVED;
        ccm_ptr->meg_format   = OAM_MEG_ID_FORMAT;
        ccm_ptr->meg_length   = OAM_MEG_ID_LENGTH; 

        memcpy(&ccm_ptr->mepid, &entity->mepid, 
               sizeof(ccm_ptr->mepid));
        memcpy(&ccm_ptr->meg_id, &entity->id_icc, 
               sizeof(ccm_ptr->meg_id)); 

        if (one_way_lm == 1) {
            tx_packets = oam_ask_rtx_packets(ptr_stat->ifindex_name, OAM_TX);
            memcpy(&ccm_ptr->txfcf, 
                   &tx_packets, 
                   sizeof(ccm_ptr->txfcf));  
            memcpy(&ccm_ptr->rxfcb, 
                   &((struct oam_entity_stat *)ptr)->ccm_rx, 
                   sizeof(ccm_ptr->rxfcb)); 
        } 

        returned_length = oam_build_pdu(common_pdu,
                                        (const struct oam_pdu_common *const) 
                                        ccm_ptr, 
                                        ptr_stat->mepid);

        if (entity->send(fd_out, 
                         ptr_stat->mac, 
                         (const struct oam_pdu_common *const)common_pdu, 
                         returned_length,
                         ptr_stat->ifindex)) {
            OAM_ERROR("Failed to send RAW ETHERNET packet\n");
            err = -1; 
            goto out_err;
        } 
    }
out_err:
    free(common_pdu);
    free(ccm_ptr);
    return err;
}

/**
 * Evaluates the sending of the PDU
 * 
 * @param sc The control information
 *
 * @return 0 if send is necessary, non-zero else.
 */
static int oam_send_evaluate(struct oam_send_control *sc)
{
    if (sc->count > 0 || sc->onoff == ON) {
        if (oam_internal_time_expired(&sc->next_send) == 0 ) {
            return 0;
        }
    }
    return -1;    
}

/**
 * What has to be done after the send
 *
 * @param sc Send control information to be updated 
 */
static void oam_send_done(struct oam_send_control *sc)
{
    sc->sent++;
    if (sc->onoff != ON) {
        sc->count -= 1;
    }   

    oam_add_usecs_to_internal_time((int64_t)sc->interval, &sc->next_send);
}

/**
 * Checks if there is a need to send other PDUs.
 *
 * @param fd_out The socket for the output.
 * @param ttl Hopcount for the msg (used with LTM)
 */
void oam_send_the_rest(const int fd_out,
                       uint8_t ttl)
{
    int                         i;
    int                         err;
    unsigned char              *ptr;
    struct oam_entity_stat     *ptr_stat;
    struct oam_send_control    *ptr_sc;
   
    extern struct oam_db    *oam_control_info;    
    
    ptr = (unsigned char *)oam_control_info;
    ptr = ptr + sizeof(struct oam_db);

    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
            sizeof(struct oam_db) + 
            (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;
        ptr_sc = &ptr_stat->lb_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_lbm(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending LBM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->dm_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_dmm(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending DMM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->dm1_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_1dm(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending 1DM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->sl_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_slm(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending SLM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->lm_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_lmm(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending LMM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->lt_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_ltm(fd_out, ptr_stat, ttl);
            if (err) {
                OAM_ERROR("Problem sending LTM (moving on)\n");
            }
        }
        ptr_sc = &ptr_stat->tst_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_tst(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending LTM (moving on)\n");
            }
        }    
        ptr_sc = &ptr_stat->lck_send_control;
        if (!oam_send_evaluate(ptr_sc)) {
            err = oam_send_lck(fd_out, ptr_stat);
            if (err) {
                OAM_ERROR("Problem sending LCK (moving on)\n");
            }
        }        
    }
}

/**
 * Checks if there is a need to send LBs and sends them.
 *
 * @param fd_out The socket for the output.
 * @param ptr_stat Entitys stat table entry
 *
 * @return Zero on success, non-zero else.
 */
int oam_send_lbm(const int fd_out,
                 struct oam_entity_stat *ptr_stat) 
{
    int                         returned_length;
    struct oam_pdu_lbm_no_data *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;
     
    pdu = calloc(1, sizeof(struct oam_pdu_lbm_no_data));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }
    ptr_sc = &ptr_stat->lb_send_control;
    
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    /* Build LBM */    
    pdu->opcode = OAM_OPCODE_LBM;
    
     /*  Check that we did not hit the limit */
    if (ptr_stat->lb_seq < UINT32_MAX) {
        ptr_stat->lb_seq += 1;
    } else {
        ptr_stat->lb_seq = 0;
    }
    
    pdu->tid_seq = htonl(ptr_stat->lb_seq); 
    OAM_DEBUG("Sending TID/SEQ %d\n", ntohl(pdu->tid_seq));

    oam_timestamp(&ptr_stat->lb_stamp);  
    oam_get_internal_time(&ptr_stat->lb_stamp_int);
    
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu, 
                      ptr_stat->mepid);

    if (returned_length < 1) {
        OAM_ERROR("Failed to build LBM\n"); 
        return -1;
    }
    
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the SLMs 
 *
 * @param fd_out The socket for the output.
 * @param ptr_stat Entitys stat table entry
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_slm(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    struct oam_pdu_slm_no_end  *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_entity_stat     *found;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_slm_no_end));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->sl_send_control;        
    memset(common_pdu, 0, OAM_MAX_PACKET);
     
    found = oam_find(htons(ptr_stat->mepid));
    if (found != NULL) {
        memcpy(&pdu->test_id, 
               &found->slm_test_id, 
               sizeof(found->slm_test_id));
        found->slm_txfcl_slm++;
        memcpy(&pdu->txfcf, 
               &found->slm_txfcl_slm, 
               sizeof(found->slm_txfcl_slm));
    } else {
        OAM_ERROR("Failed to find entity in others"
                  " sending 0 TxFCf\n");
    }

    pdu->opcode = OAM_OPCODE_SLM;
    memcpy(&pdu->source_mepid, &entity->mepid, sizeof(pdu->source_mepid)); 
    
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);
    returned_length  = oam_build_end_tlv(common_pdu, returned_length);
   
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }     
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the LMMs 
 *
 * @param fd_out The socket for the output.
 * @param ptr_stat Entitys stat table entry
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_lmm(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    uint32_t                    tx_packets = 0;
    struct oam_pdu_lmm         *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_entity_stat     *found;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_lbm_no_data));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->lm_send_control;        
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    found = oam_find(htons(ptr_stat->mepid));
    if (found != NULL) {
        tx_packets =  oam_ask_rtx_packets(found->ifindex_name, 
                                          OAM_TX);
    } else {
        OAM_ERROR("Failed to find entity in others"
                  " sending 0 TxFCf\n");
    }
    pdu->opcode = OAM_OPCODE_LMM;
    memcpy(&pdu->txfcf, &tx_packets, sizeof(pdu->txfcf)); 
    /* Store as what we sent */
    memcpy(&ptr_stat->lmm_my_txfcf, 
           &tx_packets, 
           sizeof(pdu->txfcf));
    
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);
    
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }     
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the 1DM
 *
 * @param fd_out The socket for the output. 
 * @param ptr_stat Entitys stat table entry
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_1dm(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    struct oam_internal_time    send_time;
    struct oam_pdu_1dm         *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_1dm));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->dm1_send_control;
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    pdu->opcode = OAM_OPCODE_1DM; 
    oam_get_internal_time(&send_time);
    memcpy(&pdu->txtimestampf_sec, 
           &send_time.seconds, 
           sizeof(pdu->txtimestampf_sec)); 
    memcpy(&pdu->txtimestampf_nano, 
           &send_time.nanoseconds, 
           sizeof(pdu->txtimestampf_nano)); 
     
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);
    
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }
    free(common_pdu);
    free(pdu);
    return 0;
}
 
/**
 * Sends out the DMMs
 *
 * @param fd_out The socket for the output. 
 * @param ptr_stat Entitys stat table entry
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_dmm(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    struct oam_internal_time    send_time;
    struct oam_pdu_dmm         *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_dmm));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->dm_send_control;
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    pdu->opcode = OAM_OPCODE_DMM; 
    oam_get_internal_time(&send_time);
    memcpy(&pdu->txtimestampf_sec, 
           &send_time.seconds, 
           sizeof(pdu->txtimestampf_sec)); 
    memcpy(&pdu->txtimestampf_nano, 
           &send_time.nanoseconds, 
           sizeof(pdu->txtimestampf_nano)); 
    
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);
    
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the LTMs
 *
 * @param fd_out The socket for the output. 
 * @param ptr_stat Entitys stat table entry
 * @param ttl Hopcount for the msg
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_ltm(const int fd_out,
                 struct oam_entity_stat *ptr_stat,
                 uint8_t ttl)
{
    int                         returned_length = 0;
    struct oam_pdu_ltm_no_end  *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_ltm_no_end));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->lt_send_control;
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    pdu->opcode = OAM_OPCODE_LTM; 
    /*  Check that we did not hit the limit */
    if (ptr_stat->lt_seq < UINT32_MAX) {
        ptr_stat->lt_seq += 1;
    } else {
        ptr_stat->lt_seq = 0;
    }                
    pdu->transid = htonl(ptr_stat->lt_seq); 
    OAM_DEBUG("Sending TID/SEQ %d\n", ntohl(pdu->transid));
    pdu->ttl = ttl;
//XX_FIX 
//    memcpy(&pdu->original_mac, &entity->mac, sizeof(entity->mac));
    memcpy(&pdu->target_mac, &ptr_stat->mac, sizeof(ptr_stat->mac));    
    
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);       
    returned_length  = oam_build_end_tlv(common_pdu, returned_length);
    
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the TSTs
 *
 * @param fd_out The socket for the output.
 * @param ptr_stat Entitys stat table entry
 *
 * @return Zero on success, non-zero else.
 */
int oam_send_tst(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    struct oam_pdu_tst_no_end  *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;
    
    extern struct oam_db    *oam_control_info;   
    extern struct oam_entity *entity;
    
    pdu = calloc(1, sizeof(struct oam_pdu_tst_no_end));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }
    
    ptr_sc = &ptr_stat->tst_send_control;
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    pdu->opcode = OAM_OPCODE_TST;
    /*  Check that we did not hit the limit */
    if (ptr_stat->tst_seq < UINT32_MAX) {
        ptr_stat->tst_seq += 1;
    } else {
        ptr_stat->tst_seq = 0;
    }               
    pdu->seq = htonl(ptr_stat->tst_seq);
    OAM_DEBUG("Sending TID/SEQ %d\n", ntohl(pdu->seq));
    
    returned_length =
        oam_build_pdu((struct oam_pdu_common * const)common_pdu,
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);     
    
    returned_length =
        oam_add_tst_tlv(common_pdu,
                        returned_length,
                        ptr_stat->tst_pattern,
                        ptr_stat->tst_pattern_len);
    returned_length  = oam_build_end_tlv(common_pdu, returned_length);
    if (entity->send(fd_out,
                     ptr_stat->mac,
                     (const struct oam_pdu_common *const)common_pdu,
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1;
    } else {
        oam_send_done(ptr_sc);
    }    
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * Sends out the LCK
 *
 * @param fd_out The socket for the output. 
 * @param ptr_stat Entitys stat table entry
 * 
 * @return Zero on success, non-zero else.
 */
int oam_send_lck(const int fd_out,
                 struct oam_entity_stat *ptr_stat)
{
    int                         returned_length = 0;
    struct oam_pdu_lck         *pdu;
    struct oam_pdu_common      *common_pdu;
    struct oam_send_control    *ptr_sc;

    extern struct oam_db    *oam_control_info;    
    extern struct oam_entity *entity;

    pdu = calloc(1, sizeof(struct oam_pdu_tst_no_end));
    if (!pdu) {
        OAM_ERROR("Failed to calloc memory for the PDU\n");
        return -ENOMEM;
    }
    common_pdu = oam_alloc_pdu();
    if (!common_pdu) {
        OAM_ERROR("Failed to alloc memory for common PDU\n");
        return -ENOMEM;
    }

    ptr_sc = &ptr_stat->lck_send_control;
    memset(common_pdu, 0, OAM_MAX_PACKET);
    
    pdu->opcode = OAM_OPCODE_LCK;  
    
    /* Stick the interval to correct place */
    if (ptr_sc->interval == 4) {
        pdu->flags = OAM_CCM_PERIOD_1S;
    } else if (ptr_sc->interval == 6) {
        pdu->flags = OAM_CCM_PERIOD_1MIN;
    } else {
        pdu->flags = OAM_CCM_PERIOD_1S;
    }
    returned_length = 
        oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                      (const struct oam_pdu_common * const)pdu,
                      ptr_stat->mepid);     
    if (entity->send(fd_out, 
                     ptr_stat->mac, 
                     (const struct oam_pdu_common *const)common_pdu, 
                     returned_length,
                     ptr_stat->ifindex)) {
        OAM_ERROR("Failed to send RAW ETHERNET packet\n");
        free(common_pdu);
        free(pdu);
        return -1; 
    } else {
        oam_send_done(ptr_sc);
    }
    free(common_pdu);
    free(pdu);
    return 0;
}

/**
 * This function sends the packet forward to all interfaces
 * defined in the configuration, !except! to ours.
 *
 * @param fd_out Fd to use for outbound packets
 * @param pdu_in Packet that came in
 * @param from_mac Who sent it
 *
 * @return Zero on success non-zero else.
 */
int oam_send_to_other_directions(const int fd_out,
                                 struct oam_pdu_common * pdu_in,
                                 const unsigned char *const from_mac)
{
    int                     i;
    int                     length      = 0;
    int                     interfaces;
    unsigned char          *ptr          = NULL;
    struct oam_entity_stat *found        = NULL;
    struct oam_if          *ifs_ptr;

    extern int                ifs_len;
    extern struct oam_if      ifs[20]; 
    extern struct oam_entity *entity;

    ptr = (unsigned char *)pdu_in;
    /* skip mel version opcode flags */ 
    ptr = ptr + 3;
    length = *ptr;

    found = oam_find_by_mac(from_mac);
    interfaces = ifs_len;
    
    if (interfaces == 2) {
        /* one egress/ingress + lo */
        /* No point in looping */
        return 0;
    }

    ifs_ptr = ifs;
    for (i = ifs_len; --i >= 0; ifs_ptr++) {
        if (strncmp(ifs_ptr->ifindex_name, 
                    OAM_LOOPBACK_NAME, 
                    OAM_IFNAME_MAX) == 0) {
            continue;
        }
        if (strncmp(ifs_ptr->ifindex_name, 
                    found->ifindex_name, 
                    OAM_IFNAME_MAX) == 0) {
            continue;
        }
        /* Not the interface it came in or "lo" so send it out */
        length = oam_packet_length(pdu_in);
        if (entity->send(fd_out, 
                         found->mac, 
                         (const struct oam_pdu_common *const)pdu_in, 
                         length,
                         found->ifindex)) {
            OAM_ERROR("Failed to send RAW ETHERNET packet\n");
            return -1; 
        }
    }

    return 0;
}
