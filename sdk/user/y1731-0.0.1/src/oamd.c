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
 * Feature test macro for IPC */
#define _XOPEN_SOURCE 550 

/**
 * Feature test macro struct ifreq */
#define _BSD_SOURCE 

#include <errno.h>
#include <ifaddrs.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <net/if.h>

#include <netpacket/packet.h>

#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/un.h>
#include <fnmatch.h>
#include "configuration.h"
#include "debug.h"
#include "init.h"
#include "ife.h"
#include "input.h"
#include "constants.h"
#include "pdu.h"
#include "output.h"
#include "tools.h" 

/**
 * The default usec for rest of the PDUs check period (LT, DM, LB, LM).
 */
#define OAM_REST_CHECK_PERIOD 1

/**
 * Default location of the mepd configuration file
 */
#ifndef CFGFILE
#define CFGFILE "/usr/etc/oam/config.xml"
#endif 

/**
 * Compilation time. Helps to distinguish old versions.
 */
#ifndef COMPILETIME
#define COMPILETIME "No compile time given"
#endif 

/**
 * Time string when the binary is compiled.
 */
static char *compilation_time = COMPILETIME;

/**
 * SVN revision information. Helps to distinguish old versions.
 */
#ifndef REVISION
#define REVISION "information not given"
#endif 

/**
 * Time string when the binary is compiled.
 */
static char *revision = REVISION;

/**
 * Default location of the meg configuration file
 */
#ifndef MEGFILE
#define MEGFILE "/usr/etc/oam/meg.xml"
#endif 

/**
 * Default location of the meg configuration file
 */
#ifndef LOGFILE
#define LOGFILE "/var/log/meg.xml"
#endif 

/**
 * Location of the configuration XML file.
 */
static char *config_file = CFGFILE;

/**
 * Location of the meg XML file.
 */
static char *meg_file = MEGFILE;

/**
 * For test purposes, send CCMs or not.
 */
static enum oam_onoff no_ccm = OFF;

/**
 * Is this instance a MIP.
 */
static enum oam_onoff mip = OFF;

/**
 * For test purposes, turn the select/poll time out to 0 (no-block).
 */
static enum oam_onoff no_block = OFF;

/**
 * For selecting between the select and, poll.
 */
static int spe_method = 0;

/**
 * Output socket, 
 * depends on the configuration if this is IP or Ethernet
 */
static int fd_out = -1;

/**
 * Input socket, 
 * depends on the configuration if this is IP or Ethernet
 */
static int fd_in = -1;

/**
 * Input local socket. Used to receive commands from the oamstad
 */
static int local_fd_in = -1; 

/**
 * Output local socket. Used to reply commands from the oamstad
 */
static int local_fd_out = -1; 

///**
// * Process id for the child process (the CCM loop).
// */
//static pid_t child_pid;

/**
 * The chosen check period in usec for the rest of the PDUs (LT, DM, LB, LM).
 */ 
static uint64_t rest_period_usec = OAM_REST_CHECK_PERIOD;

/**
 * Structure holding the configuration information read from the file
 */
static struct oam_configuration configuration;    

/**
 * This function uninitializes (frees) all the resources claimed
 * on the initialization of this daemon.
 *
 * @note This is called at the end of normal termination and 
 *       when mepd terminates on an error.
 */
static void oam_oamd_uninit(void)
{

    /* If we are running as a MIP we need to turn promiscuous mode off */
    if (mip == ON) {
        if (oam_toggle_promiscuous_mode(&fd_in, OFF) < 0) {
            OAM_ERROR("Failed to enable multicast recv.\n");
        }
    }

    /* xxx unregister multicast */
    if (oam_bind_multicast(&fd_in, OFF) < 0) {
        OAM_ERROR("Failed to diasble multicast recv.\n");
    }

    /* sockets */
    close(fd_in);
    close(fd_out);
    close(local_fd_in);
    close(local_fd_out);
}
 
/**
 * print mepd usage instructions
 */
static void usage(void)
{
    fprintf(stderr, "\nUsage: oamd [options]\n\n");
    fprintf(stderr, "  -c [name] of the alternate configuration XML path.\n");
    fprintf(stderr, "  -m [name] of the alternate meg XML path.\n");
    fprintf(stderr, "  -x server MEP, i.e., turn of the CCM sending.\n");
    fprintf(stderr, "  -i MIP\n");
    fprintf(stderr, "  -s [select, or poll] Switch between select, and poll\n");
    fprintf(stderr, "  -n Switch the timeout to 0 for select, or poll (no-block)\n");
    fprintf(stderr, "  -a Clock accuracy correction\n");
    fprintf(stderr, "  -v Print the compilation time and the svn revision of this binary.\n");
    fprintf(stderr, "\n");
}


static int match_include_interface(const char *incl_str, const char *ifname) {
	char *s, *token;
	int ismatch = 0;
	s = strdup(incl_str);
	if (NULL==s)
		return 0;
	
	for(token = strtok(s, ","); token; token = strtok(NULL, ",")) {
		//printf("Pattern: [%s]\n", token);
		if (0==fnmatch(token, ifname, 0)) {
			ismatch = 1;
			break;
		}			
	}
		
	free(s);
	return ismatch;
}
/**
 * Get all the interfaces and the associated MACs.
 *
 * @return negative if error, positive if (or 0) if success
 *
 * @note uses the configuration's exlude IFs to exclude IFs
 */
static int oam_get_interfaces(void)
{
    int             err = 0;

    struct ifaddrs *index;
    struct ifaddrs *if_array;
    struct sockaddr_ll *s;

    struct oam_if  *ifs_ptr;

    struct ifreq    ifreqs[20];
    struct ifconf   ifconf; 

    extern int                       ifs_len;
    extern struct oam_if             ifs[20];

    if_array = NULL;
    index = NULL; 

    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.ifc_buf = (char*)ifreqs;
    ifconf.ifc_len = sizeof(ifreqs);

    ifs_ptr = ifs;
    ifs_len = 0;    
    err = getifaddrs(&if_array);
    if (err == 0) {
        for (index = if_array; index != NULL; index = index->ifa_next) {
            if (index->ifa_addr && index->ifa_addr->sa_family == AF_PACKET) {
                if (strstr("lo", index->ifa_name) != NULL) {
                    OAM_DEBUG("Excluded loopback\n");
                    continue;
                } 
				if (!match_include_interface(configuration.include_string,index->ifa_name)) {
					OAM_DEBUG("Excluded %s as instructed\n", index->ifa_name);
                    continue;
				}
                
                OAM_DEBUG("Interface name %s has MAC address\n",  
                          index->ifa_name);
                s = (struct sockaddr_ll *)index->ifa_addr;
                memcpy(ifs_ptr->mac,
                       s->sll_addr,
                       OAM_ETH_MAC_LENGTH);
                strcpy(ifs_ptr->ifindex_name, index->ifa_name);
                ifs_ptr->ifindex = if_nametoindex(index->ifa_name);
                ifs_len++;
                ifs_ptr++; 
            }
        }
		freeifaddrs(if_array);
    } else {
        err = -1;
        OAM_ERROR("Failed to get information on the interfaces\n");
        return err;
    }

    oam_print_interfaces();
    if (err < 0) {
        return err;
    } else {
        return ifconf.ifc_len/sizeof(struct ifreq);
    }
}
 
/**
 * Parses the command line options given to the mepd main function.
 *
 * @param argc number of arguments.
 * @param argv a pointer to the arguments array.
 * @return zero on success, non-zero else.
 */
static int oam_parse_cmdline_opts(int argc, char **argv) {
     int err = 0;
     int c;

     extern char *config_file;
     extern char *meg_file;
     extern struct oam_entity *entity;

     entity->accuracy_correction = OFF;
     
     while ((c = getopt (argc, argv, "c:m:xs:nvia")) != -1) {
         switch (c) {
         case 'c':
             config_file = optarg;
             break;
         case 'm':
             meg_file = optarg;
             break;
         case 's':
             if (strcmp(optarg, "select") == 0 ) {
                 spe_method = 0;
             } else if (strcmp(optarg, "poll") == 0) {
                 spe_method = 1;
             } else {
                 OAM_DEBUG("Unknown option, defaulting to select\n");
                 spe_method = 0;
             }
             break;
         case 'x':
             no_ccm = ON;
             break;
         case 'a':
             entity->accuracy_correction = ON;
             OAM_DEBUG("Using clock accuracy correction\n");
             break;
         case 'i':
             mip = ON;
             OAM_DEBUG("Runnning as MIP!\n");
             break;
         case 'n':
             no_block = ON;
             break;
         case 'v':
             OAM_DEBUG("Compilation time %s, SVN %s\n", compilation_time, revision);
             exit(0);
             break;
         case '?':
             fprintf (stderr,
                      "Unknown option `-%c'.\n", optopt);
             usage();
             err = -1;
             goto out_err;
         default:
             fprintf (stderr,
                      "No options\n");
             usage();
             err = -1;
             goto out_err;
         }
     }
out_err:
     return err;
}

/**
 * Debug print the information that the entity contains (MEP ID, MEG ID).
 */
static void oam_print_entity(void)
{
    extern struct oam_entity *entity;

    OAM_DEBUG("\n\nEntity information\n"
              "==================\n"
              "MEP ID              : %d\n"  
              "MEG ID (ICC)        : %s\n"
              "MEG Level           : %d\n\n",
              htons(entity->mepid), 
              entity->id_icc, 
              entity->meg_level);
    OAM_STAT("Entity information\n");
    OAM_STAT("MEP ID %d\n", entity->mepid);
    OAM_STAT("MEG ID (ICC) %s\n", entity->id_icc);
    OAM_STAT("MEG Level %d\n\n", entity->meg_level);
} 
 
/**
 * a Function to go through the stat table and see if any
 * entity has sent out an LB and if its stamp is over the 5 sec period.
 *
 * @return Zero on success, non-zero else.
 */
static int oam_check_lb_intervals()
{ 
    int                       i;
    int                       err;
    unsigned char            *ptr;
    struct oam_entity_stat   *ptr_stat;

    extern struct oam_db    *oam_control_info;
    extern struct oam_entity *entity;

    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
                                sizeof(struct oam_db) + 
                                (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;
        
        err = oam_inside_interval("LB", &ptr_stat->lb_stamp, 
                                  OAM_LBR_TIMEOUT_SEC, 0);
        if (err < 0) {
            OAM_STAT_INFO("LB ALARM for MEP ID %d\n", ptr_stat->mepid);
            oam_send_ccd("LB ALARM for MEP ID %d\n", ptr_stat->mepid);
            oam_set_alarm_lb(oam_control_info, ptr_stat, OAM_ALARM_ON);
        } else {
            /* Clearing the error as the oam_inside_interval returns 
             positive interval in usecs as successfull output */
            err = 0;
        } 
    }
    return err;
}

/**
 * A function to check that we got the CCMs we needed.
 *
 * @return Zero on success, non-zero else.
 */
static int oam_check_ccm_intervals()
{
    int                       i;
    int                       err;
    unsigned char            *ptr;
    struct oam_entity_stat   *ptr_stat;

    extern struct oam_db    *oam_control_info;
    extern struct oam_entity *entity;

    err = 0;

    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
                                sizeof(struct oam_db) + 
                                (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;

        err = oam_inside_interval("CCM", &ptr_stat->ccm_stamp, 
                                  0, (OAM_ERROR_INTERVAL_MULTIPLIER *
                                  configuration.ccm_period)); 
        if (err < 0) {
            OAM_STAT_INFO("CCM ALARM for MEP ID %d\n", ptr_stat->mepid);
            oam_send_ccd("CCM ALARM for MEP ID %d\n", ptr_stat->mepid);
            oam_set_alarm_ccm(oam_control_info, ptr_stat, OAM_ALARM_ON);
        } else {
            /* Clearing the error as the oam_inside_interval returns 
             positive interval in usecs as successfull output */
            err = 0;
        } 
    }
    return err;
}

/**
 * Handle periodic maintence tasks. 
 */
static void oam_periodic_maintenance() {
    int err; 
    err = 0; 
    
    err = oam_check_lb_intervals(); 
    err = oam_check_ccm_intervals();
    
    if (err != 0) {
        OAM_DEBUG("Error in maintenance (checking intervals)\n");
    }
}

/**
 * Signal handler: exit gracefully.
 *
 * @param signum signal the signal mepd received from OS
 */
static void oam_mepd_close(int signum) {
    static int terminate = 0;

    OAM_ERROR("Caught signal: %d\n", signum);
    /* Original terminate++ but I wanted mepd to terminat at once */
    terminate = 3;

    if (terminate == 1) {
	 OAM_DEBUG("Starting to close MEPD daemon...\n");
    } else if (terminate == 2) {
	 OAM_DEBUG("Send still once this signal to force daemon exit...\n");
    } else if (terminate > 2) {
        OAM_STAT_INFO("Terminating daemon (pid %d)\n", getpid());
        oam_oamd_uninit();
        exit(EXIT_SUCCESS);
    }
}

/**
 * If there is data in the socket the packet is read and checked
 * that it belongs to us and then passed to the correct handler.
 *
 * @param pdu A buffer into which the packet is read
 * @param fd File descriptor for the input socket
 *
 * @return Zero on success, non-zero else.
 */
static int oam_handle_local_read(struct oam_pdu_common *pdu, 
                                 int fd)
{
    int            err;
    int            length;
    uint8_t        opcode;
    unsigned char *ptr;

    err = 0;

    ptr = (unsigned char *)pdu;

    length = recvfrom(fd, pdu, OAM_MAX_PACKET, 0, NULL, NULL);
    if (length == -1) { 
        OAM_ERROR("Failed to receive from the raw socket\n");
        err = -1;
        goto out_err;
    }
    ptr = ptr + 1; /* 1 = Level/version */
    opcode = *ptr;           
    OAM_DEBUG("%s (%d)\n", 
              oam_packet_type_name(opcode), opcode);
    switch (opcode) {   
    case OAM_OPCODE_MCC:
        err = oam_handle_mcc(pdu, NULL, OFF);
        break;
    default:
        OAM_ERROR("Failed to handle packet %s (%d) not"
                  " used in local traffic\n", 
                  oam_packet_type_name(opcode), opcode);
        break;
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
 *
 * @return Zero on success, non-zero else.
 */
static int oam_handle_local_in(struct oam_pdu_common *pdu, 
                               int fd, fd_set *fdset)
{
    int            err;

    err = 0;

    if (FD_ISSET(fd, fdset)) { 
        err = oam_handle_local_read(pdu, fd);
        if (err != 0) { 
            OAM_ERROR("Failed to receive local socket\n");
        }
    }
    return err;
}

/**
 * Daemon "main" function, select version. 
 *
 * @return zero on success, non-zero else
 */
static int oam_mepd_main_select() { 
    int                       err; 
    int                       highest_descriptor;
    int                       diff;            
    fd_set                    read_fdset;
    time_t                    load_time;
    char                      time_string[OAM_TIME_STRING_LEN];
    struct timeval            timeout;
    struct oam_pdu_common    *common_pdu_in;
    struct oam_internal_time  ccm_timer_stamp;
    struct oam_internal_time  rest_timer_stamp;
    struct oam_internal_time  now;

    extern int                       oam_daemon_state;
    extern struct oam_configuration  configuration;
    extern struct oam_entity        *entity;

    err = 0;

    common_pdu_in = oam_alloc_pdu();

    if (oam_get_logtype() == LOGTYPE_SYSLOG) {
        if (fork() > 0) {
            return 0;
        }
    }

    time(&load_time);  
    oam_daemon_state = OAM_RUNNING;
    oam_format_time(&load_time, time_string, sizeof(time_string));
    OAM_STAT_INFO("MEPD, pid %d, start %s\n", 
                  getpid(),
                  time_string);
        
    oam_print_current_loglevel();
    
    /* Initialize timers */
    oam_get_internal_time(&ccm_timer_stamp);
    oam_get_internal_time(&rest_timer_stamp);
    
    oam_send_ccd("Starting");
    
    while (oam_daemon_state != OAM_STOPPED) {
        FD_ZERO(&read_fdset);
        FD_SET(fd_in, &read_fdset);
        FD_SET(local_fd_in, &read_fdset);
        
        timeout.tv_sec   = 0;
        if (no_block == OFF) {
            timeout.tv_usec = OAM_SELECT_TIMEOUT_TICKER_USECS;
        } else {
            timeout.tv_usec = 0;
        }

        // This could be handled in a smarter way. 
        highest_descriptor = local_fd_out;
        
        err = select(highest_descriptor + 1, &read_fdset, 
                     NULL, NULL, &timeout);
        
        if (err < 0) {
            OAM_ERROR("select() error: %s.\n", strerror(errno));
        } else if (err == 0) {
            if (oam_internal_time_expired(&ccm_timer_stamp) == 0 ) {
                /* is parent process running, if not stop */
                if (getppid() == 1) {
                    return 0;
                }
                if (entity->accuracy_correction == OFF) { 
                    oam_add_usecs_to_internal_time(
                        (int64_t)configuration.ccm_period, 
                                                   &ccm_timer_stamp);
                } else {
                    oam_get_internal_time(&ccm_timer_stamp);
                    oam_get_internal_time(&now);
                    diff = oam_internal_time_diff(&ccm_timer_stamp,
                                                  &now); 
                    oam_add_usecs_to_internal_time(
                        (int64_t)configuration.ccm_period - diff, 
                        &ccm_timer_stamp);
                    //OAM_DEBUG("interval added %d\n", (int64_t)configuration.ccm_period - diff);
                }
                if (configuration.ccm_pulses > 0 && 
                    (configuration.dynamic == 1 ||
                     configuration.dynamic == 2)) {
                    /* if there are pulses send multicast */
                    OAM_DEBUG("Pulse #%d\n", configuration.ccm_pulses);
                    if (oam_send_ccm_multi(fd_out, 
                                           configuration.ccm_period)) {
                        OAM_ERROR("Failed to send CCMs\n");
                        err = -1;
                        goto out_err;
                    } 
                    configuration.ccm_pulses--;
                    if (configuration.ccm_pulses == 0) {
                        OAM_DEBUG("Going to unicast CCM\n");
                    }
                } else if (no_ccm == OFF && mip == OFF) {
                    /* if no pulses send unicast */ 
                    if (oam_send_ccms(fd_out, 
                                      configuration.ccm_one_way_lm,
                                      configuration.ccm_period,
                                      configuration.dynamic)) {
                        OAM_ERROR("Failed to send CCMs\n");
                        err = -1;
                        goto out_err;
                    } 
                }
            }
            if (oam_internal_time_expired(&rest_timer_stamp) == 0 ) {
                oam_get_internal_time(&rest_timer_stamp);
                oam_add_usecs_to_internal_time((int64_t)rest_period_usec, 
                                               &rest_timer_stamp);
                
                oam_send_the_rest(fd_out, configuration.lt_ttl);
            }
        } else if (err > 0) { 
            /* Receive from the Ethernet socket */
            if (oam_handle_raw_in(common_pdu_in, 
                                  fd_in, 
                                  &read_fdset,
                                  fd_out,
                                  &configuration,
                                  mip)) { 
                OAM_ERROR("Failed to handle raw input socket.\n");
                err = -1;
                goto out_err;
            }
            /* Receive from the local socket traffic */ 
            if (oam_handle_local_in(common_pdu_in, 
                                    local_fd_in, 
                                    &read_fdset)) { 
                OAM_ERROR("Failed to handle local input socket.\n");
                /* We acknowledged that there was an error in the
                   handling of local MCC but its no reason to crash 
                   whole mepd */ 
                err = 0;
            }
        }
        oam_periodic_maintenance();
    }    

out_err:
    free(common_pdu_in);
    return err;
}

/**
 * How many fds do we have for the poll function
 */
#define OAM_POLL_MAX_FDS 4

/**
 * Timeout for the poll in milliseconds
 */
#define OAM_POLL_TIMEOUT 1

/**
 * Daemon "main" function, poll version.
 *
 * @return zero on success, non-zero else
 */
static int oam_mepd_main_poll() { 
    int                       err;
    int                       i;
    int                       timeout;
    struct pollfd            *my_fds;       
    time_t                    load_time;
    char                      time_string[OAM_TIME_STRING_LEN];
    struct oam_pdu_common    *common_pdu_in;
    struct oam_internal_time  ccm_timer_stamp;
    struct oam_internal_time  rest_timer_stamp;

    extern int                      oam_daemon_state;
    extern struct oam_configuration configuration;
    
    err = 0;
    common_pdu_in = oam_alloc_pdu();
    my_fds = (struct pollfd*) calloc(sizeof(struct pollfd*), OAM_POLL_MAX_FDS);
    
    if (oam_get_logtype() == LOGTYPE_SYSLOG) {
        if (fork() > 0) {
            return 0;
        }
    }

    my_fds[0].fd = fd_in;
    my_fds[0].events = POLLIN;
    my_fds[1].fd = fd_out;
    my_fds[2].fd = local_fd_in;
    my_fds[0].events = POLLIN;
    my_fds[3].fd = local_fd_out;

    for (i = 0; i < OAM_POLL_MAX_FDS; i++) {
        OAM_DEBUG("Poll fds[%d] = %d\n", i+1, my_fds[i]);
    }

    time(&load_time);  
    oam_daemon_state = OAM_RUNNING;
    oam_format_time(&load_time, time_string, sizeof(time_string));
    OAM_STAT_INFO("MEPD, pid %d, start %s\n", 
                  getpid(),
                  time_string);
        
    oam_print_current_loglevel();
    
    /* Initialize timers */
    oam_get_internal_time(&ccm_timer_stamp);
    oam_get_internal_time(&rest_timer_stamp);
    
    oam_send_ccd("Starting");
    
    if (no_block == OFF) {
        timeout = OAM_POLL_TIMEOUT;
    } else {
        timeout = 0;
    }

    while (oam_daemon_state != OAM_STOPPED) {
        
        // reset all the events
        for (i = 0; i < OAM_POLL_MAX_FDS; i++) {
            my_fds[i].events = POLLIN | POLLPRI;
            my_fds[i].revents = 0;
        }

        if (poll(my_fds, OAM_POLL_MAX_FDS, timeout) == -1)
        {
                perror("poll");
                exit(0);
        }

        // send everything that has triggered.
        if (oam_internal_time_expired(&ccm_timer_stamp) == 0 ) {
            /* is parent process running, if not stop */
            if (getppid() == 1) {
                return 0;
            }
            oam_get_internal_time(&ccm_timer_stamp);
            oam_add_usecs_to_internal_time((int64_t)configuration.ccm_period, 
                                           &ccm_timer_stamp);
            
            if (configuration.ccm_pulses > 0 && 
                (configuration.dynamic == 1 ||
                 configuration.dynamic == 2)) {
                /* if there are pulses send multicast */
                OAM_DEBUG("Pulse #%d\n", configuration.ccm_pulses);
                if (oam_send_ccm_multi(fd_out, 
                                       configuration.ccm_period)) {
                    OAM_ERROR("Failed to send CCMs\n");
                    err = -1;
                    goto out_err;
                } 
                configuration.ccm_pulses--;
                if (configuration.ccm_pulses == 0) {
                    OAM_DEBUG("Going to unicast CCM\n");
                }
            } else if (no_ccm == OFF && mip == OFF) {
                /* if no pulses send unicast */ 
                if (oam_send_ccms(fd_out, 
                                  configuration.ccm_one_way_lm,
                                  configuration.ccm_period,
                                  configuration.dynamic)) {
                    OAM_ERROR("Failed to send CCMs\n");
                    err = -1;
                        goto out_err;
                } 
            }
        }
        if (oam_internal_time_expired(&rest_timer_stamp) == 0 ) {
            oam_get_internal_time(&rest_timer_stamp);
            oam_add_usecs_to_internal_time((int64_t)rest_period_usec, 
                                           &rest_timer_stamp);
            
            oam_send_the_rest(fd_out, configuration.lt_ttl);
        }
        
        if (my_fds[0].revents != 0) {
            OAM_DEBUG("Raw in socket (poll)\n");
            /* Receive from the Ethernet socket */
            err = oam_handle_raw_read(common_pdu_in,
                                      fd_in,
                                      fd_out,
                                      &configuration,
                                      mip);
            if (err != 0) { 
                OAM_ERROR("Failed to handle raw input socket.\n");
                err = -1;
                goto out_err;
            }
        }
        if (my_fds[3].revents != 0) {
            OAM_DEBUG("Local in socket (poll)\n");
            /* Receive from the local socket traffic */ 
            err = oam_handle_local_read(common_pdu_in, local_fd_in);
            if (err != 0) { 
                OAM_ERROR("Failed to handle local input socket.\n");
                /* We acknowledged that there was an error in the
                   handling of local MCC but its no reason to crash 
                   whole mepd */ 
                err = 0;
            }
        }
    } // while
    
 out_err:
    free(common_pdu_in);
    return err;

}

#define PID_FILE "/var/run/oamd.pid"
static void log_pid(void) {
	FILE *fp;
	
	fp = fopen(PID_FILE, "w");
	if (fp) {
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
}
/**
 * Main function. 
 *
 * @param argc number of arguments. 
 * @param argv a pointer to the arguments array.
 * @return zero on success, non-zero else.
 */
int main (int argc, char **argv) {     
    int      num_of_interfaces = 0;
    int      err = 0;

    extern struct oam_entity *entity; 
    extern struct oam_db     *oam_control_info;    
    extern FILE              *stat_log;
    extern unsigned char      da1[6];
    extern unsigned char      da2[6];
       
    if (!(entity = calloc(1, sizeof(struct oam_entity)))) {
        OAM_ERROR("Failed to calloc memory for the identity of this entity\n");
        err =  -ENOMEM;
        goto out_err;
    }
    sprintf(entity->id_icc, "Unknwn ent");

    if (oam_parse_cmdline_opts(argc, argv)) {
        OAM_ERROR("Failed to parse the commandline options.\n");
        err = -1;
        goto out_err;
    }
    /* After the parse cmdline opts so the usage can be viewed
     * as non-root */
    if (getuid()) {
        OAM_ERROR("Root privileges needed!\n");
	  return EXIT_FAILURE;
    }    

    OAM_DEBUG("Using MEPD conf: %s\n", config_file);
    OAM_DEBUG("Using MEG conf: %s\n", meg_file);

    stat_log = fopen(LOGFILE, "a");
    if (stat_log == NULL) {
        OAM_ERROR("Could not open the statistic file\n");
    }
    
    oam_set_daemontype(DAEMONTYPE_MEP);

    if (oam_initialize_db(config_file)) {
        OAM_ERROR("Failed to initialize db for control information.\n");
        err = -1;
        goto out_err;
    } 
    if (oam_configure(config_file, 
                      meg_file, 
                      &configuration, 
                      oam_control_info)) { 
        OAM_ERROR("Failed to configure.\n");
        err = -1;
        goto out_err;
    }

    oam_init_send_intervals_in_db(&configuration);

    if (oam_control_info->participants < 1 && 
        configuration.dynamic == OFF) {
        OAM_ERROR("No participants\n");
        err = -1;
        goto out_err;
    }
    OAM_DEBUG("%d participants\n", oam_control_info->participants);

    if (entity->ccd == ON) {
        OAM_DEBUG("Using CCD server port %d and addr %s", 
                  entity->srv_port, entity->srv_ip);
    }

    OAM_STAT_INFO("Configuration complete\n");
    signal(SIGINT, oam_mepd_close);

    oam_create_multicast_da1(entity->meg_level, da1);
    oam_create_multicast_da2(entity->meg_level, da2);    
    oam_print_mac("Class 1 multicast DA ", da1);
    oam_print_mac("Class 2 multicast DA ", da2);

    oam_bootcount();    

    oam_print_entity();
    //oam_print_db(oam_control_info);  

    /* Initialize and bind and listen and etc sockets... */
    if (oam_init_raw_sockets(&fd_in, &fd_out, mip)) {
        OAM_ERROR("Failed in the initialization of the "
                  "communication sockets.\n");
        err = -1;
        goto out_err;
    }    
    if (oam_init_local_sockets(&local_fd_in, &local_fd_out)) {
        OAM_ERROR("Failed in the initialization of the local sockets.\n");
         err = -1;
         goto out_err;
    }
    if (oam_bind_local(&local_fd_in)) {
        OAM_ERROR("Failed to bind to local socket\n");
        err = -1;
        goto out_err;
    }
    /* Get the interface list */
    num_of_interfaces = oam_get_interfaces();
    if (num_of_interfaces == 0) {
        OAM_ERROR("No network interfaces\n");
        err = -1;
        goto out_err;
    }
    /* Enable the DA1 and DA2 recv on all the interfaces used */
    if (oam_bind_multicast(&fd_in, ON) < 0) {
        OAM_ERROR("Failed to enable multicast recv.\n");
        err = -1;
        goto out_err;
    }
    
    /* If we are running as a MIP we need promiscuous mode */
    if (mip == ON) {
        if (oam_toggle_promiscuous_mode(&fd_in, ON) < 0) {
            OAM_ERROR("Failed to enable multicast recv.\n");
            err = -1;
            goto out_err;
        }
    }

    /* Assign send function */
    entity->send = &oam_send_eth;

    oam_generate_crc32_table();

    if (no_block == ON && spe_method != 2) {
        OAM_DEBUG("Using 0 ms timeout (no-block)\n");
    } else {
        OAM_DEBUG("Using 1 ms timeout\n");
    }

	log_pid(); 
	
    if (spe_method == 1) {
        OAM_DEBUG("Using POLL\n");
        if (oam_mepd_main_poll()) {
            OAM_ERROR("Failed in the main loop\n");
            err = -1;
        }
    } else {
        OAM_DEBUG("Using SELECT\n");
        if (oam_mepd_main_select()) {
            OAM_ERROR("Failed in the main loop\n");
            err = -1;
        }
    }    
    
out_err:
    oam_oamd_uninit();
    OAM_FREE(entity)
    exit(err);
}
