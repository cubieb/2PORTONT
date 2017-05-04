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

#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/time.h>

#include "constants.h"
#include "debug.h"
#include "output.h"
#include "tools.h"

#define MAC_SEPARATOR ':'/**< Separator used with MAC addresses */
#define BASE10        10 /**< to be added to the A to FF when converted */

/**
 * Conversion of the given string MAC address to bytes
 *
 * @param mac      Where the converted mac will be stored.
 * @param mac_addr Address containing the MAC to convert.
 *
 * @return Zero on success, non-zero else.
 *
 * @note Assumes that the mac can hold 6 bytes.
 */
int oam_mac_pton(unsigned char *mac, const char *mac_addr)
{
    int i               = 0;
    unsigned int number = 0;
    char ch;

    for (i = 0; i < 6; ++i) {

        ch = tolower (*mac_addr++);
        if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f')) {
            return -1;
        }
        
        number = isdigit (ch) ? (ch - '0') : (ch - 'a' + BASE10);
        ch = tolower (*mac_addr);
        
        if ((i < 5 && ch != MAC_SEPARATOR) || 
            (i == 5 && ch != '\0' && !isspace (ch))) {
            ++mac_addr;
            
            if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f')) {
                return -1;
            }
            
            number <<= 4;
            number += isdigit (ch) ? (ch - '0') : (ch - 'a' + BASE10);
            ch = *mac_addr;
            
            if (i < 5 && ch != MAC_SEPARATOR) {
                return -1;
            }
        }
        mac[i] = (unsigned char) number;        
        ++mac_addr; // Skip separator 
    }
    return 0;
}

/**
 * Converts the given byte format MAC address
 * to a string presentation format
 *
 * @param mac_addr Where the presentation format of MAC is stored.
 * @param mac      MAC address to be converted.
 *
 * @note Assumes that the mac_addr can hold 17 bytes.
 */
void oam_mac_ntop(char *const mac_addr, const unsigned char *const mac)
{
    sprintf(mac_addr, "%02x%c%02x%c%02x%c%02x%c%02x%c%02x", 
            mac[0] & 0xff,
            MAC_SEPARATOR,
            mac[1]& 0xff, 
            MAC_SEPARATOR,
            mac[2]& 0xff, 
            MAC_SEPARATOR,
            mac[3]& 0xff, 
            MAC_SEPARATOR,
            mac[4]& 0xff, 
            MAC_SEPARATOR,
            mac[5]& 0xff);
    
    return;
} 

/**
 * Converts the given MAC to presentation format and prints 
 * the converted MAC and given message.
 * 
 * @param msg Message that is printed with the converted MAC
 * @param mac MAC address that needs to be printed
 */
void oam_print_mac(const char * msg, const unsigned char *const mac) 
{
   char mac_addr[] = "00:00:00:00:00:00";
   oam_mac_ntop((char *const)mac_addr,  
            (const unsigned char *const)mac);
   OAM_DEBUG("%s %s\n", msg, mac_addr); 
}

/**
 * Converts the given MAC to presentation format and prints 
 * the converted MAC and given message to stderr.
 * 
 * @param msg Message that is printed with the converted MAC
 * @param mac MAC address that needs to be printed
 */
void oam_print_mac_stderr(const char * msg, const unsigned char *const mac) 
{
   char mac_addr[] = "00:00:00:00:00:00";
   oam_mac_ntop((char *const)mac_addr,  
            (const unsigned char *const)mac);
   fprintf(stderr, "%s %s\n", msg, mac_addr); 
}

/**
 * Return a string for a given opcode.
 *
 * @param opcode Operation code 
 *
 * @return the name of the operation code
 */
const char *oam_packet_type_name(const int opcode)
{
    switch (opcode) {   
    case OAM_OPCODE_CCM:  return "Continuity Check Message";
    case OAM_OPCODE_LBM:  return "LoopBack Message";
    case OAM_OPCODE_LBR:  return "LoopBack Reply";
    case OAM_OPCODE_LTM:  return "Link Trace Message"; 
    case OAM_OPCODE_LTR:  return "Link Trace Reply"; 
    case OAM_OPCODE_AIS:  return "Alarm Indication Signal"; 
    case OAM_OPCODE_LCK:  return "Locked signal"; 
    case OAM_OPCODE_TST:  return "Test signal"; 
    case OAM_OPCODE_LAPS: return "Linear Automatic Protection Switching"; 
    case OAM_OPCODE_RAPS: return "Ring Automatic Protection Switching"; 
    case OAM_OPCODE_MCC:  return "Maintenance Communication Channel"; 
    case OAM_OPCODE_LMM:  return "Loss Measurement Message"; 
    case OAM_OPCODE_LMR:  return "Loss Measurement Reply"; 
    case OAM_OPCODE_1DM:  return "One-way Delay Measurement"; 
    case OAM_OPCODE_DMM:  return "Delay Measurement Message"; 
    case OAM_OPCODE_DMR:  return "Delay Measurement Reply"; 
    case OAM_OPCODE_EXM:  return "Experimental Message"; 
    case OAM_OPCODE_EXR:  return "Experimental Reply"; 
    case OAM_OPCODE_VSR:  return "Vendor-Specific Message"; 
    case OAM_OPCODE_VSM:  return "Vendor-Specific Reply"; 
    case OAM_OPCODE_SLR:  return "Synthetic Loss Reply";
    case OAM_OPCODE_SLM:  return "Synthetic Loss Message";
    default:
        return "UNDEFINED";
    }
    return "UNDEFINED";
} 

/**
 * Return a string for a given opcode.
 *
 * @param opcode Operation code 
 *
 * @return the name of the operation code
 */
const char *oam_packet_type_name_short(const int opcode)
{
    switch (opcode) {   
    case OAM_OPCODE_CCM:  return "CCM";
    case OAM_OPCODE_LBM:  return "LBM";
    case OAM_OPCODE_LBR:  return "LBR";
    case OAM_OPCODE_LTM:  return "LTM"; 
    case OAM_OPCODE_LTR:  return "LTR"; 
    case OAM_OPCODE_AIS:  return "AIS"; 
    case OAM_OPCODE_LCK:  return "LCK"; 
    case OAM_OPCODE_TST:  return "TST"; 
    case OAM_OPCODE_LAPS: return "LAPS"; 
    case OAM_OPCODE_RAPS: return "RAPS"; 
    case OAM_OPCODE_MCC:  return "MCC"; 
    case OAM_OPCODE_LMM:  return "LMM"; 
    case OAM_OPCODE_LMR:  return "LMR"; 
    case OAM_OPCODE_1DM:  return "1DM"; 
    case OAM_OPCODE_DMM:  return "DMM"; 
    case OAM_OPCODE_DMR:  return "DMR"; 
    case OAM_OPCODE_EXM:  return "EXM"; 
    case OAM_OPCODE_EXR:  return "EXR"; 
    case OAM_OPCODE_VSR:  return "VSM"; 
    case OAM_OPCODE_VSM:  return "VSR"; 
    case OAM_OPCODE_SLR:  return "SLR";
    case OAM_OPCODE_SLM:  return "SLM";
    default:
        return "UNDEFINED";
    }
    return "UNDEFINED";
}

/**
 * Return a string for a given subopcode
 * 
 * @param subopcode Sub operation code
 *
 * @return the name of the sub operation code
 */
const char *oam_mcc_subopcode_name(const int subopcode)
{
    switch (subopcode) {
    case OAM_SUB_OPCODE_LMM: return "LMM";
    case OAM_SUB_OPCODE_1DM: return "1DM";
    case OAM_SUB_OPCODE_DMM: return "DMM";
    case OAM_SUB_OPCODE_LBM: return "LBM";
    case OAM_SUB_OPCODE_LTM: return "LTM";
    case OAM_SUB_OPCODE_TST: return "TST";
    case OAM_SUB_OPCODE_LCK: return "LCK";
    case OAM_SUB_OPCODE_SLM: return "SLM";
    default:
        return "UNDEFINED";
    }
    return "UNDEFINED";
}

/**
 * This function returns the number of transmitted or received 
 * packets on the given interface.
 *
 * @param name Name of the interface.
 * @param direction OAM_RX or OAM_TX 
 *
 * @return the number of packets in the requested direction
 */
uint32_t oam_ask_rtx_packets(const char *const name, 
                             const enum oam_directions direction)
{
    uint32_t packets;
    char path[50];
    char line[20];
    FILE *fp;
    
    /* 
     *  /sys/class/net/eth0/statistics/rx_packets 
     *  /sys/class/net/eth0/statistics/tx_packets 
     */
    if (direction == OAM_RX) {
        sprintf(path, "/sys/class/net/%s/statistics/rx_packets", name);
    } else if (direction == OAM_TX) {
        sprintf(path, "/sys/class/net/%s/statistics/tx_packets", name);
    } 

    if ((fp = fopen(path, "r")) == NULL) {
        perror("PERROR");
        OAM_ERROR("Failed to open statistics\n");
        return 0;
    }

    if (!fgets(line, sizeof(line), fp)) {
        OAM_ERROR("Failed to read the packets value\n");
        packets = 0;
    }

    packets = atoll(line);
    
    //OAM_DEBUG("\nCOMPARE: %s vs %llu\n", line, packets);
    
    fclose(fp);

    return packets;
}

/**
 * Find a pointer to specific other entity's statistics.
 *
 * @param mepid MEP ID to find
 *
 * @return Pointer to the found struct oam_entity_stat or NULL else.
 *
 * @note the byte order! A wrapper for oam_find_from_db
 */
struct oam_entity_stat *oam_find(const uint16_t mepid)
{
    struct oam_entity_stat    *match = NULL;
    
    extern struct oam_db *oam_control_info;

    match = oam_find_from_db(oam_control_info, mepid);

    return match;
}

/**
 * Find a pointer to specific other entity's statistics
 *
 * @param db_ptr Pointer to the shared memory containing the statistics
 * @param mepid MEP ID to find
 *
 * @return Pointer to the found struct oam_entity_stat or NULL else.
 *
 * @note the byte order!!
 */
struct oam_entity_stat *oam_find_from_db(const struct oam_db *const db_ptr, 
                                          const uint16_t mepid)
{
    int                        i                   = 0; 
    int                        participants;
    unsigned char             *ptr;
    struct oam_entity_stat    *match               = NULL;

    ptr = (unsigned char *)db_ptr;
    participants = db_ptr->participants;
    ptr = ptr + sizeof(struct oam_db);
    for (i=0; i < participants; i++) {
        ptr = (unsigned char *)db_ptr + 
              sizeof(struct oam_db) +
              (i * sizeof(struct oam_entity_stat));  
        /*
        OAM_DEBUG("IN %d, COMPARE %d\n", 
                  mepid, htons(((struct oam_entity_stat *)ptr)->mepid));
        */
        if (mepid == htons(((struct oam_entity_stat *)ptr)->mepid)) {
            match = (struct oam_entity_stat *)ptr;
            break;
        }
    }

    return match;
}

/**
 * Find a pointer to specific other entity by mac
 *
 * @param from_mac MAC to search for
 *
 * @return Pointer to the found struct oam_entity_stat or NULL else.
 *
 * @note the byte order!!
 */
struct oam_entity_stat *oam_find_by_mac(const unsigned char *const from_mac)
{
    int                     i                   = 0; 
    unsigned char          *ptr;
    struct oam_entity_stat *match               = NULL;

    extern struct oam_db  *oam_control_info;
 
    ptr = (unsigned char *)oam_control_info;
    ptr = ptr + sizeof(struct oam_db);
    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
              sizeof(struct oam_db) +
              (i * sizeof(struct oam_entity_stat));       
        if (memcmp(from_mac, 
                   ((struct oam_entity_stat *)ptr)->mac, 
                   OAM_ETH_MAC_LENGTH) == 0) {
            match = (struct oam_entity_stat *)ptr;
            break;
        }
    }
    return match;
}

/**
 * Make the timestamp to the given location
 *
 * @param *stamp Pointer to the timestamp
 */
void oam_timestamp(struct timeval *stamp)
{
    gettimeofday(stamp, NULL);
}

/**
 * Check whether the given timestamp is newer than the given interval in seconds
 *
 * @param msg Message that will be printed with the alarm
 * @param *prev Pointer to the timestamp
 * @param interval_sec Interval in seconds 
 * @param interval_usec Interval in microseconds
 *
 * @return Diff usecs, zero for not in use, negative on error.
 *
 * @note The intervals will be calculated sec*1000000+usec.
 */
int oam_inside_interval(const char *const msg, 
                        struct timeval *const prev, 
                        const int interval_sec, 
                        const int interval_usec)
{
    int diff;
    int interval; /* in usecs */
    struct timeval now;
    int prev_usec, now_usec;

    prev_usec = prev->tv_sec * OAM_USEC_IN_S + prev->tv_usec;

    if (prev_usec == 0) {
        //OAM_DEBUG("Stamp was not initialized, so not in use\n");
        return 0;
    }
    gettimeofday(&now, NULL);

    /*
    OAM_DEBUG("Prev %ld.%06ld\n", prev->tv_sec, prev->tv_usec);
    OAM_DEBUG("Now %ld.%06ld\n", now.tv_sec, now.tv_usec);
    */

    interval = interval_sec * OAM_USEC_IN_S + interval_usec;

    now_usec = now.tv_sec * OAM_USEC_IN_S + now.tv_usec;

    diff = now_usec - prev_usec;

    /*
    OAM_DEBUG("DIFF %d INTERVAL %d\n",diff, interval);
    */

    if (diff < interval) { 
        //OAM_DEBUG("No worries inside the given interval\n);
        return diff;
    } else {
        OAM_DEBUG("Raise alarm (%s)\n", msg);
        /* Should this be cleared here or not? */
        prev->tv_sec = 0;
        prev->tv_usec = 0;
        return -1;
    }
}

/**
 * Convert the internal presentation of the interval to the 
 * usec equivalent.
 */
enum oam_alarms give_legit_interval(int interval)
{
    switch (interval) {        
    case 1:
        return OAM_INTERVALS_IN_USEC_333MS;
    case 2:
        return OAM_INTERVALS_IN_USEC_10MS;
    case 3:
        return OAM_INTERVALS_IN_USEC_100MS;
    case 4:
        return OAM_INTERVALS_IN_USEC_1S;
    case 5:
        return OAM_INTERVALS_IN_USEC_10S;
    case 6:
        return OAM_INTERVALS_IN_USEC_1MIN;
    case 7:
        return OAM_INTERVALS_IN_USEC_10MIN;
    default:
        return OAM_INTERVALS_IN_USEC_1S;
    }
}

/**
 * Set the alarm CCM on or off.
 *
 * @param db_ptr Pointer to the db so we can set our RDI
 * @param entity to set the alarm to 
 * @param onoff set the alarm on or off
 *
 * @see enum oam_alarms
 */
void oam_set_alarm_ccm(struct oam_db *db_ptr,
                       struct oam_entity_stat *const entity,
                       const enum oam_alarms onoff)
    
{
        entity->alarm_ccm = onoff;
        db_ptr->rdi = onoff;
}

/**
 * Set the alarm LB on or off.
 *
 * @param db_ptr Pointer to the db so we can set our RDI
 * @param entity to set the alarm to
 * @param onoff set the alarm on or off
 *
 * @see enum oam_alarms
 */
void oam_set_alarm_lb(struct oam_db *db_ptr,
                      struct oam_entity_stat *const entity,
                      const enum oam_alarms onoff)
{
        entity->alarm_lb = onoff;
        db_ptr->rdi = onoff;
} 

/**
 * Function that checks the internal table of the interfaces
 * if the given MAC is ours.
 * 
 * @param mac to be checked against our knowledge of our MAC.
 * 
 * @return Zero on success (MAC is ours), non-zero else.
 */
static int oam_mac_is_ours(const unsigned char *const mac) 
{
    int            i;
    struct oam_if *ifs_ptr;
    char           mac_addr[]  = "00:00:00:00:00:00";

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = 0; i < ifs_len; ifs_ptr++, i++) {
        oam_mac_ntop((char *const)mac_addr,
                     ifs_ptr->mac);
        /*
        OAM_DEBUG("IF index %d, MAC %s/%-10s\n", 
                  ifs_ptr->ifindex, 
                  mac_addr,
                  ifs_ptr->ifindex_name);
        */
        if (memcmp((unsigned char *)ifs_ptr->mac, 
                   mac, 
                   OAM_ETH_MAC_LENGTH) == 0) {
            return 0;
        }
    }
    return -1;
}

/**
 * This function checks if the packet was meant for us and that
 * the Ethertype is correct.
 * 
 * Other checks done for specific opcodes: 
 * CCM, MEG level and MEG ID are checked 
 *
 * @param pdu Buffer containing the packet
 * @param recv_info Where the multicast is marked.
 * @param mip are we a MIP
 * @param socket_out which socket to use for sending.
 * 
 * @return One if the packet is to be handled zero else.
 */
int oam_check_packet(const struct oam_pdu_common *const pdu,
                     struct oam_recv_information *recv_info,
                     const enum oam_onoff mip,
                     const int socket_out)
{
    int                     err;
    int                     send_length;
    unsigned char          *ptr;
    uint16_t                type;
    struct oam_entity_stat *found;

    extern unsigned char      da1[6];
    extern unsigned char      da2[6];

    err = 1;
    ptr = (unsigned char *)pdu;
    type = htons(OAM_ETH_P_CFM);
    
    /* Check if this is a CFM frame */
    ptr = ptr + 2 * OAM_ETH_MAC_LENGTH;
    err = memcmp(ptr, &type, sizeof(type));
    if (err) {
        /* not OAM frame */
        return 0;
    } 

    /* CFM frame has to be checked */
    /* Rewind the frame as we need the addresses */
    ptr = (unsigned char *)pdu;

    found = NULL;
    recv_info->multicast = OFF;

    /* it was for us? */
    err = oam_mac_is_ours((const unsigned char *const)pdu);
    if (!err) {
        /* DA was ours and is CFM so handle */
        return 1;
    } else {
        /* Not our DA but is a CFM so 
           Check if the DA matches class 1 or 2 multicast DAs */
        err = memcmp(&da1, pdu, OAM_ETH_MAC_LENGTH);
        if (err == 0) {
            recv_info->multicast = ON;
            return 1;
        }
        err = memcmp(&da2, pdu, OAM_ETH_MAC_LENGTH);
        if (err == 0) {
            recv_info->multicast = ON;
            return 1;
        }
        /* is CFM frame but DA not ours or multicast DA, 
           see if we are a MIP and forward if we know the DA */
        if (mip == ON) {
            /* it was from us? */
            ptr = (unsigned char *)pdu;
            ptr = ptr + OAM_ETH_MAC_LENGTH;
            err = oam_mac_is_ours((const unsigned char *const)ptr);
            if (!err) {
                //OAM_DEBUG("We sent this skipping\n");
                return 0;
            } 
            /* get the DA */
            ptr = (unsigned char *)pdu;
            found = oam_find_by_mac(ptr);
            if (found) {               
                /* Send the packet out. */
                /* search the real pdu start */
                ptr = (unsigned char *)pdu;
                ptr = ptr + (2 * OAM_ETH_MAC_LENGTH) + sizeof(type);
                send_length = recv_info->rcv_len - 
                    ((2 * OAM_ETH_MAC_LENGTH) + sizeof(type));
                err = oam_forward_eth(socket_out, 
                                      found->mac, 
                                      (const struct oam_pdu_common *)ptr, 
                                      send_length, 
                                      found->ifindex);
            } else {
                OAM_DEBUG("Dropping as we do not know the MAC\n");
            } 
        }
    }
    /* did not check out do not handle or was forwarded 
       and needs no further handling */
    return 0;   
}

/**
 * Get the PDU length with the TLVs (eth header not included)
 * 
 * @param pdu PDU which length we are interested in.
 *
 * @return length > 0 on success negative else.
 */
int oam_packet_length(const struct oam_pdu_common *const pdu) 
{
    int length = 0;
    unsigned char *ptr;
    
    ptr = (unsigned char *)pdu;
    /* Find where the first TLV is */
    ptr = ptr + sizeof(pdu->mel_ver);
    ptr = ptr + sizeof(pdu->opcode);
    ptr = ptr + sizeof(pdu->flags);
    /* Add melversion, opcode, flags, tlv_offset */
    length = *ptr + sizeof(struct oam_pdu_common);
    ptr = ptr + sizeof(pdu->tlv_offset);
    while (*ptr != 0) {
        OAM_DEBUG("ptr %d\n", *ptr);
        /* find the next TLV */
        ptr = ptr + 1; //type
        length = length + 1 + *ptr; // length
        ptr = ptr + *ptr; // past value
    }
    /* remember the end-TLV */
    length = length + sizeof(uint8_t);
    return length;
}

/**
 * Has the timestamp expired
 * 
 * @param stamp internal_time struct to be checked
 *
 * @return 0 if the stamp has expired and non-zero else
 */
int oam_internal_time_expired(const struct oam_internal_time *const stamp)
{ 
    struct oam_internal_time now;
    oam_get_internal_time(&now);
    if (stamp->seconds < now.seconds) {
        return 0;
    }

    if (stamp->seconds == now.seconds &&
        stamp->nanoseconds <= now.nanoseconds) {
        return 0;
    }

    return -1;
}

/**
 * Add given microseconds to given internal time struct.
 * 
 * @param usec How many microseconds are added
 * @param stamp internal_time struct to which the usec is added
 */
void oam_add_usecs_to_internal_time(const uint64_t usec, 
                                    struct oam_internal_time * stamp) 
{
    uint64_t temporary_nanoseconds = 0;

    //OAM_DEBUG("BEFORE %d:%d\n", stamp->seconds, stamp->nanoseconds);

    temporary_nanoseconds = (uint64_t)stamp->nanoseconds + 
        (uint64_t)usec * (uint64_t)OAM_USECS_TO_NANO;
    while (temporary_nanoseconds > OAM_NANOSEC_IN_S) {
        temporary_nanoseconds -= OAM_NANOSEC_IN_S;
        stamp->seconds++;
    }
    stamp->nanoseconds = temporary_nanoseconds;

   // OAM_DEBUG("AFTER %d:%d\n", stamp->seconds, stamp->nanoseconds);
}

/**
 * Function to get time now to the internal presentation
 *
 * @param now Container for the internal time.
 *
 * @note Base heavily on the similar functionality in ptpd
 */
void oam_get_internal_time(struct oam_internal_time *now) 
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    now->seconds = tv.tv_sec;
    now->nanoseconds = tv.tv_usec * OAM_USECS_TO_NANO;
}

/**
 * Function to get time now to the internal presentation
 *
 * @param tv the timeval to convert to internal time.
 * @param now Container for the internal time.
 *
 * @note Base heavily on the similar functionality in ptpd
 */
void oam_tv_to_internal_time(const struct timeval *const tv, 
                             struct oam_internal_time *now) 
{
    now->seconds = tv->tv_sec;
    now->nanoseconds = tv->tv_usec * OAM_USECS_TO_NANO;
}

/**
 * Function to print the time to given buffer.
 * 
 * @param buffer The buffer where the time is printed.
 * @param max The length of the buffer.
 * @param it The time to be printed in internal presentation
 *
 * @return 0 on success, non-zero else.
 *
 * @note Base heavily on the similar functionality in ptpd
 */
int oam_internal_time_str(char *buffer, 
                          int max, 
                          struct oam_internal_time *it)
{
    int length = 0;
    length = snprintf(buffer, max, "%d.%09d",
                      abs(it->seconds), abs(it->nanoseconds));
    return length;
}

/**
 * Function that calculates the diff of two times and returns the 
 * result in nanosecond precision.
 *
 * @param start What was the measured start time
 * @param stop What is the stop time from which the start is subtracted
 *
 * @note !Negative diff is allowed
 */ 
int32_t oam_internal_time_diff(const struct oam_internal_time *const start,
                                const struct oam_internal_time *const stop) {
    return (int32_t)(((stop->seconds - start->seconds) * OAM_NANOSEC_IN_S) + 
                     (stop->nanoseconds - start->nanoseconds));
}

/**
 * Function that returns the verdict if we accept the subopcode.
 *
 * @param subopcode Sub operation code that needs to be checked
 *
 * @return Zero on success, non-zero else.
 */
int oam_acceptable_subopcode(const int subopcode)
{
    if (subopcode == OAM_SUB_OPCODE_LMM ||
        subopcode == OAM_SUB_OPCODE_1DM ||
        subopcode == OAM_SUB_OPCODE_DMM ||
        subopcode == OAM_SUB_OPCODE_LBM ||
        subopcode == OAM_SUB_OPCODE_TST ||
        subopcode == OAM_SUB_OPCODE_SLM ||
        subopcode == OAM_SUB_OPCODE_LCK ||
        subopcode == OAM_SUB_OPCODE_LTM) {
        return 0;
    }
    OAM_ERROR("Unknown sub operation code in send MCC\n");
    return -1;
}

/**
 * Calculate the CRC32 checksum.
 *
 * @param data The content that the checsum is calculated over
 * @param length of the data.
 * @param crc32 the resulting CRC is stored here.
 *
 * @note Based on implementation of ElysiuM deeZine and Krzysztof Dabrowski 
 */
void oam_do_crc32_checksum(const unsigned char * data, 
                           const uint32_t length, 
                           uint32_t *crc32)
{
    uint32_t i;
    
    extern uint32_t crc32_table[256];

    OAM_HEXDUMP("\nCRC DATA ", data, length);
    
    *crc32 = 0xFFFFFFFF;
    for (i = 0; i < length; i++)
    {
        *crc32 = ((*crc32 >> 8) & 0x00FFFFFF) ^ 
            crc32_table[(*crc32 ^ *data++) & 0xFF];
    }
    *crc32 = (*crc32 ^ 0xFFFFFFFF);
    OAM_HEXDUMP("CRC-32 ", crc32, sizeof(*crc32));
}

/**
 * Generate the crc32 lookup table.
 *
 * @note !DO this always before using CRC
 *
 * @note Based on implementation of ElysiuM deeZine and Krzysztof Dabrowski 
 */
void oam_generate_crc32_table(void)
{
    uint32_t crc32;
    uint32_t polynomial;
    int      i;
    int      j;
    
    extern uint32_t crc32_table[256];
    
    polynomial = 0xEDB88320L;
    for (i = 0; i < 256; i++) {
        crc32 = i;
        for (j = 8; j > 0; j--) {
            if (crc32 & 1) {
                crc32 = (crc32 >> 1) ^ polynomial;
            } else {
                crc32 >>= 1;
            }
        }
        crc32_table[i] = crc32;
    }
}
 
/**
 * Check the received Sequence num against the previous 
 *
 * @param msg Message to print infront of the fault e.g. LBR, TST, LTR
 * @param received_seq What is the seq that needs checking (Remember Byte order)
 * @param previous_seq Against what the received seq is tested,
 *                     it successfull this is updated.
 * @param from_mac From what MAC was this message (presentation format)
 */
void oam_check_received_seq(const char *const msg,
                            const uint32_t received_seq, 
                            uint32_t *const previous_seq,
                            const char *const from_mac)
{
    uint32_t expected = *previous_seq + 1;

    if (received_seq < expected) {
        OAM_STAT_INFO("%s Fault, Seq already seen, received SEQ %d, "
                      "expected SEQ %d, MAC %s\n",
                      msg, received_seq, expected, from_mac);
    } else if (received_seq > expected) {
        OAM_STAT_INFO("%s Fault, Seq missing, received SEQ %d, "
                      "expected SEQ %d, MAC %s\n",
                      msg, received_seq, expected, from_mac);
    } else {
        *previous_seq = expected;
    }
}

/**
 * Function that converts the given pattern type to string
 *
 * @param type to be converted
 */
char * oam_tst_pattern_name(const int type) 
{
    switch (type) {
    case OAM_PATTERN_TYPE_ZERO_NO_CRC: return "All-Zeroes, no CRC-32";
    case OAM_PATTERN_TYPE_ZERO_CRC: return "All-Zeroes, with CRC-32";
    case OAM_PATTERN_TYPE_PRBS_NO_CRC: return "PRBS 2^31-1, no CRC-32"; 
    case OAM_PATTERN_TYPE_PRBS_CRC: return "PRBS 2^31-1, with CRC-32";
    }
    return "UNKNOWN";
}

/**
 * Debug print the bytes bits
 *
 * @param byte to be printed in bits
 *
 * @note DO NOT USE IN PRODUCTION STATE DAEMON, PRINTS TO STDOUT
 */
void oam_print_byte_in_bits(char byte)
{
    int i;
    int k;
    int mask;
    
    /* from 0-7 as i is used for mask */
    for( i = 7 ; i >= 0 ; i--) {
        mask = 1 << i;
        k = byte & mask;
        if(k == 0) {
            printf("0");
        } else {
            printf("1");
        }
    }
    printf(" ");
}

/**
 * Function to generate Pseudo Random Bit Sequence 2^31-1
 *
 * This function should follow the requirements set in 
 * ITU-T O.150 (05/96):
 * SERIES O: SPECIFICATIONS OF MEASURING EQUIPMENT 
 * Equipment for the measurement of digital and analogue/digital parameters
 *
 * GENERAL REQUIREMENTS FOR INSTRUMENTATION FOR PERFORMANCE MEASUREMENTS ON
 * DIGITAL TRANSMISSION EQUIPMENT
 *
 * - Shift register is 32 bits long
 * - Feedback taps are in 28th and 31st 
 *   (index 27 and 30 as the indexing starts at 0)
 * - Maximum sequence length is 2^31 - 1 bits
 * 
 * @param shift_register prefilled register to use
 * @param length How long in bytes 
 *
 * @return generated PRBS sequence or NULL on error
 *
 * @note The caller is responsible of freeing the PRBS buffer returned
 *
 * @note - TODO: 30 bit consequtive zeroes is allowed but no longer?
 *         should this be checked and the 31st 0 flipped?
 */
unsigned char* oam_generate_prbs(unsigned char* shift_register, 
                                 int length)
{
    int i;
    int j;
    unsigned char *sequence;
    unsigned char new_bit;
    
    sequence = malloc(length);
    if (sequence == NULL) {
        return sequence;                
    }  
    memset(sequence, 0, length);    

    for (i = 0; i < OAM_PRBS_BYTE * length; i++) {
        if(shift_register[OAM_PRBS_FIRST_TAP] != 
           shift_register[OAM_PRBS_SECOND_TAP] &&
           (shift_register[OAM_PRBS_FIRST_TAP] == OAM_PRBS_BIT_ZERO || 
            shift_register[OAM_PRBS_SECOND_TAP] == OAM_PRBS_BIT_ZERO)) {
            new_bit = OAM_PRBS_BIT_ONE;
        } else {
            new_bit = OAM_PRBS_BIT_ZERO;
        }

        for(j = OAM_PRBS_SECOND_TAP; j > 0; j--) {
            shift_register[j] = shift_register[j - 1];
        }

        shift_register[0] = new_bit;
        sequence[(uint8_t)(i / OAM_PRBS_BYTE)] = 
            sequence[(uint8_t)(i / OAM_PRBS_BYTE)] << 1;
        if(new_bit == OAM_PRBS_BIT_ONE) {
            sequence[(uint8_t)(i / OAM_PRBS_BYTE)] = 
                sequence[(uint8_t)(i / OAM_PRBS_BYTE)] | 0x1;
        }
    }
    return sequence;
}

/**
 * Function to calculate the frame loss (far-end). 
 *
 * @param ct1 Transmit counter was 
 * @param ct2 Transmit counter now
 * @param cr1 Receive counter was 
 * @param cr2 Receive counter now
 *
 * @return the calculated frame loss
 */
uint32_t oam_calculate_frame_loss(uint32_t ct1,
                                  uint32_t ct2, 
                                  uint32_t cr1,
                                  uint32_t cr2) 
{
/*
    OAM_DEBUG("\nCT1 %d,\nCT2 %d, \nCR1 %d, \nCR2 %d\n"
              "CT2-CT1 = %d \nCR2-CR1 = %d\nFrame loss = %d\n",
              ct1,
              ct2, 
              cr1, 
              cr2, 
              abs(ct2 - ct1),
              abs(cr2 - cr1), 
              (abs(ct2 - ct1) - abs(cr2 - cr1)));
*/

    OAM_DEBUG("Frame loss = %d\n",
              (abs(ct2 - ct1) - abs(cr2 - cr1)));

    return (abs(ct2 - ct1) - abs(cr2 - cr1));
}

static void oam_create_multicast_da(int level, 
                                       unsigned char *addr,
                                       int class) {
    unsigned char da[6];

    da[0] = 0x01;
    da[1] = 0x80;
    da[2] = 0xC2;
    da[3] = 0x0;
    da[4] = 0x0;

    switch (class) {
    case 1:
        da[5] = 0x30;
        da[5] += level;
        break;
    case 2:
        da[5] = 0x38;
        da[5] += level;        
        break;
    default:
        da[5] = 0x0;
        OAM_ERROR("Invalid multiclass address class\n");
    }
    memcpy(addr, &da, sizeof(da));
}

/**
 * Function that creates the class 1 multicast destination address.
 * 01-80-C2-00-00-3x where x is 0-7
 *
 * @param level What is the current MEG level
 * @param addr buffer for the addr
 *
 * @note Be sure that addr is long enough to hold the multicast addr.
 */
void oam_create_multicast_da1(int level, unsigned char *addr) {
    oam_create_multicast_da(level, addr, 1);
}

/**
 * Function that creates the class 2 multicast destination address.
 * 01-80-C2-00-00-3y where y is 8-F
 *
 * @param level What is the current MEG level
 * @param addr buffer for the addr
 *
 * @note Be sure that addr is long enough to hold the multicast addr.
 */
void oam_create_multicast_da2(int level, unsigned char *addr) {
    oam_create_multicast_da(level, addr, 2);
}

/**
 * Function that prints out the internal table of the interface information
 */
void oam_print_interfaces(void) {
    int            i;
    struct oam_if *ifs_ptr;
    char           mac_addr[]  = "00:00:00:00:00:00";

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = 0; i < ifs_len; ifs_ptr++, i++) {
        oam_mac_ntop((char *const)mac_addr,
                     ifs_ptr->mac);
        OAM_DEBUG("IF index %d, MAC %s/%-10s\n", 
                  ifs_ptr->ifindex, 
                  mac_addr,
                  ifs_ptr->ifindex_name);
    }
}

/**
 * Add a MEP with given information to the DB
 *
 * @param meg_level Level of MEG
 * @param mepid MEP ID of MEP
 * @param mac MAC of MEP
 * @param megid MEG ID of MEP
 * @param ifindex Interface index from which we heard from this MEP
 * @param ifindex_name Name of the interface from which we heard from this MEP
 *
 * @return Zero on success, non-zero else.
 */
int oam_add_mep_to_db(const int meg_level,
                       const uint16_t mepid, 
                       const unsigned char *const mac,
                       const char *const megid,
                       const int ifindex,
                       const char *const ifindex_name)
{
    int                     i;
    int                     what     = 0;
    unsigned char          *ptr      = NULL;
    struct oam_entity_stat *found    = NULL;
    struct oam_entity_stat *ptr_stat = NULL;

    extern        int      max_meps;
    extern struct oam_db *oam_control_info;

    /* Check again if MEP really does not exist */
    found = oam_find(htons(mepid));
    if (found) {
        what = 1;
    }
    found = oam_find_by_mac(mac);
    if (found) {
         what += 2;
    }
    switch (what) {
    case 1:
        OAM_ERROR("MEP ID already exists, not adding\n");
        return -1;
        break;
    case 2:
        OAM_ERROR("MAC already exists, not adding?\n");
        return -1;
        break;
    case 3:
        OAM_ERROR("MEP ID and MAC already exists, not adding\n");
        return -1;
        break;
    default:
        /* Alles klar, more or less */
        OAM_DEBUG("Adding MEP\n");
        break;
    }

    if (oam_control_info->participants < max_meps) {
        /* find the *last* thats why its !equal! */
        ptr = ptr + sizeof(struct oam_db);
        for (i=0; i <= oam_control_info->participants; i++) {
            ptr = (unsigned char *)oam_control_info + 
                sizeof(struct oam_db) +
                (i * sizeof(struct oam_entity_stat));  
        }
        ptr_stat = (struct oam_entity_stat *)ptr;

        /* save the information */
        ptr_stat->mepid = mepid;
        ptr_stat->meg_level = meg_level;
        memcpy(&ptr_stat->mac, mac, OAM_ETH_MAC_LENGTH);
        memset(ptr_stat->id_icc, 0, sizeof(ptr_stat->id_icc));
        sprintf(ptr_stat->id_icc, "%s", megid); 
        ptr_stat->ifindex = ifindex;
        strcpy(ptr_stat->ifindex_name, ifindex_name);

        oam_control_info->participants++;
    } else {
        OAM_ERROR("Too many MEPs, reconfigure\n");
    }

    return 0;
}

/**
 * Update a MEP in the DB with given information
 *
 * @param meg_level Level of MEG
 * @param mepid MEP ID of MEP
 * @param mac MAC of MEP
 * @param megid MEG ID of MEP
 * @param ifindex Interface index from which we heard from this MEP
 * @param ifindex_name Name of the interface from which we heard from this MEP
 *
 * @return Zero on success, non-zero else.
 */
int oam_update_mep_in_db(const int meg_level,
                          const uint16_t mepid, 
                          const unsigned char *const mac,
                          const char *const megid,
                          const int ifindex,
                          const char *const ifindex_name)
{
    int                     what     = 0;
    struct oam_entity_stat *found    = NULL;

    /* Check again if MEP really does exist */
    found = oam_find(htons(mepid));
    if (found) {
        what = 1;
    }
    switch (what) {
    case 1:
        /* Alles klar, more or less */
        OAM_DEBUG("Updating MEP\n");
        break;
    default:
        /*
        OAM_ERROR("MEP ID did not exist (fault case)\n");
        */
        return -1;
        break;
    } 
    
    /* save the information */
    found->mepid = mepid;
    found->meg_level = meg_level;
    memcpy(&found->mac, mac, OAM_ETH_MAC_LENGTH);
    memset(found->id_icc, 0, sizeof(found->id_icc));
    sprintf(found->id_icc, "%s", megid); 
    found->ifindex = ifindex;
    strcpy(found->ifindex_name, ifindex_name);
    
    return 0;
}

/**
 * Enable promiscuous mode for all interfaces.
 *
 * @param fd_in Inbound socket
 * @param enable Are we turning the promiscuous mode on or off.
 * 
 * @return Zero on success, non-zero else
 *
 * @note Use this ONLY AFTER the interfaces are gathered.
 */
int oam_toggle_promiscuous_mode(const int *const fd_in, 
                                const enum oam_onoff enable)
{
    int                  i;
    struct oam_if       *ifs_ptr;
    struct ifreq         ifr;

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = 0; i < ifs_len; ifs_ptr++, i++) { 
 
        /* Getting the NIC name */
        strcpy(ifr.ifr_name, ifs_ptr->ifindex_name);            

        if(ioctl(*fd_in, SIOCGIFFLAGS, &ifr) < 0) {
            OAM_ERROR("Failed to get flags\n"); 
            return -1;
        }
        
        if (enable == ON) {
            ifr.ifr_flags |= IFF_PROMISC; 
        } else {
            ifr.ifr_flags ^= IFF_PROMISC; 
        }
        /* Checking for PROMISCUOUS mode */
        if(ioctl(*fd_in, SIOCSIFFLAGS, &ifr) < 0) {      
            OAM_ERROR("Failed to set promiscuous mode on interface %s\n", 
                      ifs_ptr->ifindex_name);
            return -1;
        } 

        if (enable == ON) {
            OAM_DEBUG("Promiscuous mode enabled on %s\n", 
                      ifs_ptr->ifindex_name); 
        } else {
            OAM_DEBUG("Promiscuous mode disabled on %s\n", 
                      ifs_ptr->ifindex_name); 
        }
    }

    return 0;
}
