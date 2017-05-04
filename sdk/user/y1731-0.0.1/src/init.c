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
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include "configuration.h"
#include "constants.h"
#include "debug.h"
#include "extendedpsquared.h"
#include "ife.h"
#include "init.h"
#include "tools.h"

#include "ezxml/ezxml.h" 

/**
 * Goes through the db initialized db and sets the send intervals properly
 *
 * @param configuration Configuration containing the intervals read from configuration files.  
 *
 * @note Also initiates the next send if the feature is set on 
 */
void oam_init_send_intervals_in_db(struct oam_configuration *configuration)
{
    int                       i;
    unsigned char            *ptr;
    struct oam_entity_stat   *ptr_stat;

    extern struct oam_db    *oam_control_info;

    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
                                sizeof(struct oam_db) + 
                                (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;

        ptr_stat->lm_send_control.interval = configuration->lm_period;
        if (ptr_stat->lm_send_control.onoff == ON) {
            oam_get_internal_time(&ptr_stat->lm_send_control.next_send);
            oam_add_usecs_to_internal_time((int64_t)configuration->lm_period, 
                                           &ptr_stat->lm_send_control.next_send);
        }
        ptr_stat->dm_send_control.interval = configuration->dm_period;
        if (ptr_stat->dm_send_control.onoff == ON) {
            oam_get_internal_time(&ptr_stat->dm_send_control.next_send);
            oam_add_usecs_to_internal_time((int64_t)configuration->dm_period, 
                                           &ptr_stat->dm_send_control.next_send);
        }
    }
}

/**
 * Goes through the db initialized by the function below this 
 * and mallocs and sets the quantiles to all ep2s
 */
static int oam_init_ep2s_in_db()
{
    int                       i;
    int                       err;
    unsigned char            *ptr;
    struct oam_entity_stat   *ptr_stat;

    extern struct oam_db    *oam_control_info;

    err = 0;
    for (i=0; i < oam_control_info->participants; i++) {
        ptr = (unsigned char *)oam_control_info + 
                                sizeof(struct oam_db) + 
                                (i * sizeof(struct oam_entity_stat));
        ptr_stat = (struct oam_entity_stat *)ptr;

	err = oam_ep2_allocate(OAM_EP2_M, 
		                 (int)sizeof(double), 
		                 &ptr_stat->delay_results);
        if (err != 0) {
            OAM_DEBUG("Something wrong in allocation (%d)\n", err);
            err = -1;
            goto out_err;
	} 
        oam_ep2_set_quantiles(ptr_stat->delay_results, 
			      OAM_EP2_M, 0.25, 0.5, 0.75);
        //oam_ep2_pretty_print(ptr_stat->delay_results);
    }
out_err:
    return err;
}

/**
 * @brief Initializes the memory for oamd (db)
 *
 * @param config_file From where to read how many participants there are
 *
 * @return 0 on success, non-zero else.
 */
int oam_initialize_db(char * config_file) 
{
    int         participants; 
    size_t      size       = 0;
    ezxml_t     config_xml = NULL;
    ezxml_t     meg;

    extern struct oam_db *oam_control_info;    
    extern        int     max_meps;

    config_xml = oam_open_configuration(config_file);
    if (config_xml == NULL) {
        OAM_DEBUG("Error with configuration file in db init\n");
        return -1;
    }
    meg = ezxml_child(config_xml, "meg");    
    participants = atoi(ezxml_attr(meg, "max")); 
    max_meps = participants;
    oam_close_configuration(config_xml);

    size = sizeof(struct oam_db) + 
           participants * 
           sizeof(struct oam_entity_stat);
    
    oam_control_info = calloc(1, size);
    if (!oam_control_info) {
        OAM_DEBUG("Error in calloc for db.\n");
        return -1;
    }

    /* Show the ep2 malloc how many participants max and clear it for later use */
    oam_control_info->participants = max_meps;
    oam_init_ep2s_in_db();
    oam_control_info->participants = 0;
    OAM_DEBUG("DB init success (size == %d).\n", size);

    return 0;
}

/**
 * @brief Increases the bootcount.
 *
 * Reads the current number of boots from a file and increases it
 * and stores the increased value in to the same file. 
 *
* @note The file containing the bootcount is named according to 
 *       the given MEP ID.
 */
void oam_bootcount(void) 
{
     char        line[20];
     char       *ret;
     char       *bootcount_file = NULL;
     FILE       *fp             = NULL;
     struct stat status;

     extern int                daemon_bootcount;
     extern char              *oam_bootcount_path;
     extern struct oam_entity *entity;
     
     if (!(bootcount_file = malloc(strlen(oam_bootcount_path) + 
                                   strlen(entity->id_icc))))  {
         OAM_ERROR("Failed to malloc for bootcount file name\n");
             return; /* -ENOMEM */
     }
     memset(bootcount_file, '\0', sizeof(bootcount_file));
     strcat(bootcount_file, oam_bootcount_path);
     strcat(bootcount_file, entity->id_icc);
     OAM_DEBUG("Bootcount file %s\n", bootcount_file);
     daemon_bootcount = 0;
     if (stat(bootcount_file, &status)  == 0) {
	  fp = fopen(bootcount_file, "r");
          if(!feof(fp)) { 
	       ret = fgets(line, BOOTCOUNT_MAX_LENGTH_IN_CHAR, fp);
               if (!ret) {
                   OAM_ERROR("Failed to read the bootcount file\n");
                   return;
               }
	       daemon_bootcount = atoi(line);
	       OAM_INFO("Previous boot %s\n", ctime(&status.st_atime));
	  } 
	  if (fp) {
	       fclose(fp);
	  }
     }
     fp = fopen(bootcount_file, "w");
     daemon_bootcount = daemon_bootcount + 1;
     fprintf(fp, "%d", daemon_bootcount);
     if (fp) {
         fclose(fp);
     }
     OAM_DEBUG("Bootcount %d\n", daemon_bootcount);
     OAM_STAT("Bootcount %d\n", daemon_bootcount);
     OAM_FREE(bootcount_file);
} 

/**
 * Initializes one raw Ethernet socket for inbound traffic and one 
 * raw Ethernet socket for outbound traffic
 *
 * @param *fd_in A pointer for the inbound raw socket fd is stored 
 * @param *fd_out A pointer for the inbound raw socket fd is stored
 * @param *mip Are we a MIP, changes the recv filter to from CFM to ALL.
 *
 * @return zero on success, non-zero else.
 */
int oam_init_raw_sockets(int *fd_in, int *fd_out, enum oam_onoff mip) 
{     
    if (mip == OFF) {
        *fd_in = socket(AF_PACKET, SOCK_RAW, htons(OAM_ETH_P_CFM));
    } else {
        *fd_in = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    }
    if(*fd_in == -1) {
        OAM_ERROR("Failed to create the inbound raw Ethernet socket.\n");
        return -1;
    }   
    *fd_out = socket(AF_PACKET, SOCK_RAW, htons(OAM_ETH_P_CFM));
    if (*fd_out == -1) { 
         OAM_ERROR("Failed to create the outbound raw Ethernet socket.\n"); 
         return -1;
    }
    OAM_DEBUG("Raw sockets %d and %d\n",
              *fd_in, *fd_out);
    return 0;
}

/**
 * Initializes one local socket for inbound traffic and one 
 * local socket for outbound traffic
 *
 * @param *fd_in A pointer for the inbound local socket fd is stored 
 * @param *fd_out A pointer for the inbound local socket fd is stored
 *
 * @return zero on success, non-zero else.
 */
int oam_init_local_sockets(int *fd_in, int *fd_out)
{
    *fd_in = socket(AF_INET, SOCK_DGRAM, 0);
    if(*fd_in == -1) { 
        OAM_ERROR("Failed to create the inbound local socket.\n");
        return -1;
    }
    *fd_out = socket(AF_INET, SOCK_DGRAM, 0);
    if (*fd_out == -1) { 
        OAM_ERROR("Failed to create the outbound local socket.\n"); 
        return -1;
    }
    OAM_DEBUG("Local sockets %d and %d\n",
              *fd_in, *fd_out);
    return 0; 
}

/**
 * This function binds the given socket to the loopback address
 * using the mepd port
 *
 * @param fd_in Inbound socket
 * 
 * @return Zero on success, non-zero else
 */
int oam_bind_local(int *fd_in)
{
    struct sockaddr_in mepd_addr;    
    memset(&mepd_addr, 0, sizeof(struct sockaddr_in));

    mepd_addr.sin_family      = AF_INET;
    mepd_addr.sin_port        = htons(OAM_MEPD_LOCAL_PORT);
    mepd_addr.sin_addr.s_addr = inet_addr(OAM_LOOPBACK);

    if (bind(*fd_in, (struct sockaddr *)&mepd_addr,
             sizeof(mepd_addr))) {
        OAM_ERROR("Bind on mepd addr failed\n");
        return -1;
    }
    OAM_DEBUG("Successfully bound to loopback\n");
    return 0;
}

/**
 * Binds the given socket and all the interfaces to the
 * multicast addresses DA Class 1 and Class 2
 *
 * @param fd_in Inbound socket
 * @param enable Turning it on or off
 * 
 * @return Zero on success, non-zero else
 *
 * @note Use this ONLY AFTER the interfaces are gathered.
 */
int oam_bind_multicast(int *fd_in, enum oam_onoff enable)
{
    int                 i;
    struct oam_if      *ifs_ptr;
    struct packet_mreq  mreq;

    extern unsigned char      da1[6];
    extern unsigned char      da2[6];

    extern int           ifs_len;
    extern struct oam_if ifs[20];

    ifs_ptr = ifs;
    for (i = 0; i < ifs_len; ifs_ptr++, i++) {
        memset(&mreq, 0, sizeof(mreq));
        mreq.mr_ifindex = ifs_ptr->ifindex; 
        mreq.mr_type = PACKET_MULTICAST | PACKET_HOST;
        mreq.mr_alen = sizeof(da1);
        memcpy(&mreq.mr_address, &da1, OAM_ETH_MAC_LENGTH); 

        if (enable == ON) {
            if (setsockopt(*fd_in, 
                           SOL_PACKET, 
                           PACKET_ADD_MEMBERSHIP, 
                           &mreq, 
                           sizeof(mreq)) < 0) {
                OAM_ERROR("setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP]");
                return -1;
            }
            OAM_DEBUG("Multicast (DA1) recv enabled on %s\n", 
                      ifs_ptr->ifindex_name);
        } else {
            if (setsockopt(*fd_in, 
                           SOL_PACKET, 
                           PACKET_DROP_MEMBERSHIP, 
                           &mreq, 
                           sizeof(mreq)) < 0) {
                OAM_ERROR("setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP]");
                return -1;
            }
            OAM_DEBUG("Multicast (DA1) recv disabled on %s\n", 
                      ifs_ptr->ifindex_name);
        }
        memcpy(&mreq.mr_address, &da2, OAM_ETH_MAC_LENGTH); 

        if (enable == ON) {
            if (setsockopt(*fd_in, 
                           SOL_PACKET, 
                           PACKET_ADD_MEMBERSHIP, 
                           &mreq, 
                           sizeof(mreq)) < 0) {
                OAM_ERROR("setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP]");
                return -1;
            }
            OAM_DEBUG("Multicast (DA2) recv enabled on %s\n", 
                      ifs_ptr->ifindex_name);
        } else {
            if (setsockopt(*fd_in, 
                           SOL_PACKET, 
                           PACKET_DROP_MEMBERSHIP, 
                           &mreq, 
                           sizeof(mreq)) < 0) {
                OAM_ERROR("setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP]");
                return -1;
            }
            OAM_DEBUG("Multicast (DA2) recv disabled on %s\n", 
                      ifs_ptr->ifindex_name);
        }
    }

    return 0;
}


