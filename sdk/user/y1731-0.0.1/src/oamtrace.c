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
 * Feature test macro struct ifreq */
#define _BSD_SOURCE 

#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <netpacket/packet.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include "constants.h"
#include "debug.h"
#include "extendedpsquared.h"
#include "init.h"
#include "output.h"
#include "tools.h"
#include "pdu.h"

/**
 * Output socket for Ethernet
 */
static int raw_fd_out = -1;

/**
 * Input socket for Ethernet
 */
static int raw_fd_in = -1;

/**
 * Message used by all commands
 */
static struct oam_pdu_common *out_common_pdu = NULL;

/**
 * Is the pinger running. 
 */
static int oam_daemon_state; 

/**
 * How many LBs are we going to send
 */
static int count = 1;

/**
 * Count how many LB responses we have received.
 */
static int received = 0;

/** 
 * Transaction id 
 */
static uint32_t tid = 0;

/**
 * The last send timestamp.
 */
struct oam_internal_time sent_internal_time;

/**
 * print usage instructions on stderr
 */
static void usage(void)
{
    fprintf(stderr, "\nUsage: oamtrace [options]\n\n"
            "  --mac / -m [MAC], \n"
            "       MAC of the target.\n"
            "  --ttl/ -t [TTL], \n"
            "       TTL.\n"
            "  --iifindex / -f [number of] \n"
            "       The IF index used for sending.\n"
            "  --mel / -l [MEG level], \n"
            "       MEG level of the target.\n"
            "\n");
}

/**
 * How long is the human readable MAC.
 */
#define OAM_PRESENTATION_LEN_MAC 18

/**
 * Get all the interfaces and the associated MACs.
 *
 * @param ifconf Where the information is stored,
 * @param sockfd Socket to ask from.
 *
 * @return negative if error, positive if (or 0) if success
 *
 * @note uses the configuration's exlude IFs to exclude IFs
 */
static int oam_get_interfaces(const int sockfd)
{
    int            err;
    int            i;
    struct ifreq   ifrequest;
    struct ifreq  *ifr_ptr;
    struct oam_if *ifs_ptr;
    
    struct ifreq  ifreqs[20];
    struct ifconf ifconf; 

    extern int                       ifs_len;
    extern struct oam_if             ifs[20];
//    extern struct oam_configuration *configuration;
 
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.ifc_buf = (char*)ifreqs;
    ifconf.ifc_len = sizeof(ifreqs);
    
    if ((err = ioctl(sockfd, SIOCGIFCONF, (char *)&ifconf)) < 0) {
        perror("Get all the interfaces");
    }
    ifs_len = 0;
    ifr_ptr = ifconf.ifc_req;
    ifs_ptr = ifs;
    for (i = ifconf.ifc_len / sizeof(struct ifreq); i >= 0; ifr_ptr++, i--) {
        strcpy(ifrequest.ifr_name, ifr_ptr->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifrequest) == 0) {
            if (! (ifrequest.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sockfd, SIOCGIFHWADDR, &ifrequest) == 0) {
                        memcpy(ifs_ptr->mac,
                               ifrequest.ifr_hwaddr.sa_data,
                               OAM_ETH_MAC_LENGTH);
                        ifs_ptr->ifindex = if_nametoindex(ifrequest.ifr_name);
                        strcpy(ifs_ptr->ifindex_name, ifrequest.ifr_name); 
                        ifs_len++;
                        ifs_ptr++;
                }
            }
        }
    }
    oam_print_interfaces();
    if (err < 0) {
        return err;
    } else {
        return ifconf.ifc_len/sizeof(struct ifreq);
    }
}

/**
 * Handle packet LTR
 *
 * @param common_pdu Packet to be handled
 * @param recv_info From addr, ifindex and family
 *
 * @return Zero on success, non-zero else. 
 */
static int oam_handle_ltr_sa(const struct oam_pdu_common * common_pdu,
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
 * If there is data in the socket the packet is read and checked
 * that it belongs to us and then passed to the correct handler.
 *
 * @param pdu A buffer into which the packet is read
 * @param fd File descriptor for the input socket
 * @param fdset File descriptor set to which the socket belongs to
 *
 * @return Zero on success, non-zero else.
 */
static int oam_handle_raw_in_sa(struct oam_pdu_common *pdu, 
                                int fd, 
                                fd_set *fdset)
{
    int                          err = 0;
    int                          length; 
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

    if (FD_ISSET(fd, fdset)) {
        fromlen = sizeof(mac);
        length = recvfrom(fd,  
                          pdu,
                          OAM_MAX_PACKET, 
                          0, 
                          (struct sockaddr *)&mac, 
                          &fromlen);

        if (length == -1) { 
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

        /* Get the timestamp here */
        ioctl(fd, SIOCGSTAMP, &tv);
        oam_tv_to_internal_time(&tv, &recv_info.hw_time); 
        oam_internal_time_str(buf, sizeof(buf), &recv_info.hw_time);
        //OAM_XTRA("HW Timestamp %s\n", buf);
         
        gettimeofday(&tv, NULL); 
        oam_tv_to_internal_time(&tv, &recv_info.sw_time); 
        oam_internal_time_str(buf, sizeof(buf), &recv_info.sw_time);
        //OAM_XTRA("SW Timestamp %s\n", buf);

        /* Add the IP version code here */

        /* stand alone trace is not a MIP so OFF and the socket to 0 */
        checked = oam_check_packet(pdu, &recv_info, OFF, 0);
        if (checked) {  
            ptr = ptr + OAM_ETH_HEADER_LENGTH + 1; /* 1 = Level/version */
            opcode = *ptr;           
            //OAM_DEBUG("%s (%d)\n", 
            //         oam_packet_type_name(opcode), opcode);

            /* get the PDU start minus the Eth header */
            ptr = (unsigned char *)pdu + OAM_ETH_HEADER_LENGTH;
            pdu_in = (struct oam_pdu_common *)ptr; 
            switch (opcode) {   
            case OAM_OPCODE_LTM:
                break;
            case OAM_OPCODE_LTR:
                err = oam_handle_ltr_sa(pdu_in, &recv_info);
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
    }
 out_err:
    /* reset the buffer */
    memset(pdu, 0, OAM_MAX_PACKET);
    return err;
}

/**
 * Main function. 
 *
 * @param argc number of arguments. 
 * @param argv a pointer to the arguments array.
 *
 * @return zero on success, non-zero else.
 */
int main (int argc, char **argv) {
    int                         i = 0;
    int                         c;  
    int                         err = 0;
    int                         ttl = 1;
    int                         mel = 1;
    int                         iifindex = 0;
    int                         highest_descriptor;
    int                         num_of_interfaces = 0;
    fd_set                      read_fdset;
    char                        buf[64];
    struct oam_internal_time    timer_stamp;
    int                         returned_length = 0;
    unsigned char               network_mac[OAM_ETH_MAC_LENGTH];
    unsigned char              *our_mac;
    char                        presentation_mac[] = "11:22:33:44:55:66";
    struct oam_pdu_common      *common_pdu_in;
    struct timeval              timeout;
    struct oam_pdu_ltm_no_end  *pdu;
    struct oam_pdu_common      *common_pdu;
    struct timeval              tv;

    extern struct oam_entity   *entity;
    extern unsigned char        da2[6];

    /* You need at least one argument */
    if (argc < 5) {
        usage();
        goto out_err;
    }

    /* After the parse cmdline opts so the usage can be viewed
     * as non-root */
    if (getuid()) {
        OAM_ERROR("Root privileges needed!\n");
	  return EXIT_FAILURE;
    }

    out_common_pdu = oam_alloc_pdu();  
    common_pdu_in = oam_alloc_pdu();

    count = 1;

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

    memset(common_pdu, 0, OAM_MAX_PACKET);             
                                       
    oam_set_logtype(LOGTYPE_STDERR);
    oam_set_daemontype(DAEMONTYPE_OAMTOOL);
    
    if (!(entity = calloc(1, sizeof(struct oam_entity)))) {
        OAM_ERROR("Failed to calloc memory for the identity of this entity\n");
        goto out_err;
    }
    sprintf(entity->id_icc, "-");
    
    /* Assign send function */
    entity->send = &oam_send_eth_sa;
 
    OAM_INFO("Logtype set to stderr\n");
    oam_set_logtype(LOGTYPE_STDERR);
    oam_set_loglevel("all");

    struct option longopts[] = {
        {"mel", 1, NULL, 'l'},
        {"ttl", 1, NULL, 't'},
        {"mac", 1, NULL, 'm'},
        {"iifindex", 1, NULL, 'f'},
        {0, 0, 0, 0}
    };

    /* Initialize and bind and listen and etc sockets... */
    if (oam_init_raw_sockets(&raw_fd_in, &raw_fd_out, OFF)) {
        OAM_ERROR("Failed in the initialization of the "
                  "communication sockets.\n");
        goto out_err;
    }   

    /* Get the interface list */
    num_of_interfaces = oam_get_interfaces(raw_fd_out);

    if (num_of_interfaces == 0) {
      OAM_ERROR("No interfaces, no reason to continue\n");
      err = -1;
      goto out_err;
    }

    /* Initialize the timer */
    oam_get_internal_time(&timer_stamp);

    opterr = 0;   
    while ((c = getopt_long_only(argc, argv, "l:m:f:t:", 
                                 longopts, NULL)) != -1) {
        switch (c) {
        case 'l':
            /* meg level */
            mel = atoi(optarg);
            if (mel < 1 || mel > 7) {
                OAM_ERROR("Invalid MEG Level\n");
                goto out_err;
            }
            break;
        case 'f':
            /* iifindex */
            iifindex = atoi(optarg);
            if (iifindex < 0) {
                OAM_ERROR("Invalid interface index, see above for allowed ones\n");
                goto out_err;
            }
            break;
        case 't':
            /* ttl */
            ttl = atoi(optarg);
            if (ttl < 0) {
                OAM_ERROR("Invalid MEG Level\n");
                goto out_err;
            }
            break;
        case 'm':
            /* mac */
            oam_mac_pton(network_mac, (const char *)optarg);
            snprintf(presentation_mac, OAM_PRESENTATION_LEN_MAC, "%s", optarg); 
            //oam_print_mac("Read MAC: ", network_mac);
            break;
        case ':':
            fprintf (stderr, "\nOption needs a value\n");
            break;
        case '?':
            fprintf (stderr, 
                     "\nUnknown option (%c).\n", optopt);
            usage();
            goto out_err;
            break;
        default: 
            fprintf (stderr, 
                     "No options\n");
            usage();
            goto out_err;
        }
    }

    oam_create_multicast_da2(entity->meg_level, da2);    
    oam_print_mac("Class 2 multicast DA ", da2);

    OAM_DEBUG("\n\nTarget MAC %s,\n"
              "TTL %d,\n"
              "MEG level %d,\n"
              "iifindex %d\n\n", 
              presentation_mac, ttl, mel, iifindex);

    oam_daemon_state = OAM_RUNNING;
    i = 0;

    while (oam_daemon_state != OAM_STOPPED) {
        FD_ZERO(&read_fdset);
        FD_SET(raw_fd_in, &read_fdset);

        timeout.tv_sec   = 0;
        timeout.tv_usec  = OAM_SELECT_TIMEOUT_TICKER_USECS;

        /* FIX THIS , using MAX? find max socket num and add one to it. */
        highest_descriptor = raw_fd_out + 1;
        
        err = select(highest_descriptor, &read_fdset, 
                     NULL, NULL, &timeout);
        
        if (err < 0) {
            OAM_ERROR("select() error: %s.\n", strerror(errno));
            //goto to_maintenance;
        } else if (err == 0) {
            /* idle cycle - select() timeout */
            //goto to_maintenance;
        } else if (err > 0) { 
            /* Receive from the Ethernet socket */
            if (oam_handle_raw_in_sa(common_pdu_in, 
                                     raw_fd_in, 
                                     &read_fdset)) { 
                OAM_ERROR("Failed to handle raw input socket.\n");
                err = -1;
                goto out_err;
            }
        }
        
        /* Is set to 1 so will only send one at the time. */
        if (i < count) {
            if (oam_internal_time_expired(&timer_stamp) == 0 ) {
                /* Build the PDU */
                tid++;
                pdu->opcode = OAM_OPCODE_LTM;
                pdu->transid = htonl(tid); 
                pdu->ttl = ttl;
                our_mac = oam_get_mac_of_interface_index(iifindex);
                memcpy(&pdu->original_mac, our_mac, sizeof(network_mac));
                memcpy(&pdu->target_mac, &network_mac, sizeof(network_mac));

                returned_length = 
                    oam_build_pdu((struct oam_pdu_common * const)common_pdu, 
                                  (const struct oam_pdu_common * const)pdu,
                                  0); /*MEP ID can be anything it is not used*/
                
                if (returned_length < 1) {
                    OAM_ERROR("Failed to build LTM\n"); 
                    return -1;
                }
                        
                if (entity->send(raw_fd_out, 
                                 &network_mac, //&da2 
                                 (const struct oam_pdu_common *const)common_pdu,
                                 returned_length,
                                 iifindex)) { 
                    OAM_ERROR("Failed to send RAW ETHERNET packet\n");
                    free(common_pdu);
                    free(pdu);
                    goto out_err; 
                }

                OAM_DEBUG("Sent TID %d\n", ntohl(pdu->transid)); 
                gettimeofday(&tv, NULL);  
                oam_tv_to_internal_time(&tv, &sent_internal_time); 

                oam_internal_time_str(buf, sizeof(buf), &sent_internal_time);
                OAM_XTRA("%s sent Timestamp %s\n", 
                         oam_packet_type_name_short(pdu->opcode), 
                         buf);

                i++;
                oam_get_internal_time(&timer_stamp);
            }
        }
        // check if the received is the same or more as the TTL
        if (ttl <= received) {
            // and stop if exceeded.
            oam_daemon_state = OAM_STOPPED;
        }
    } // while

 out_err:
    free(out_common_pdu);
    close(raw_fd_in);
    close(raw_fd_out);
    free(entity);
    return 0;
}
