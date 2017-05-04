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
 * Feature test macro for usleep */
#define _BSD_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include "constants.h"
#include "debug.h"
#include "init.h"
#include "output.h"
#include "ife.h"
#include "input.h"
#include "pdu.h"
#include "tools.h"

/**
 * Input local socket. Used to receive from the mepd
 */
static int local_fd_in = -1; 

/**
 * Output local socket. Used to send commands to the mepd
 */
static int local_fd_out = -1; 

/**
 * Message used by all commands
 */
static struct oam_pdu_common *out_common_pdu = NULL;

/**
 * print mepd usage instructions on stderr
 */
static void usage(void)
{
    fprintf(stderr, "\nUsage: oamtool [options]\n\n"
            "  --slm [MEP ID], \n"
            "       Start synthetic loss measurement towards MEP ID.\n"
            "  --lmm [MEP ID],\n"
            "       Start loss measurement towards MEP ID.\n"
            "  --dmm [MEP ID],  \n"
            "       Start the delay measurements towards MEP ID\n"
            "  --1dm [MEP ID],  \n"
            "       Start the one-way delay measurements towards MEP ID\n"
            "  --ltm [MEP ID],  \n"
            "       Start the link trace towards the MEP ID\n"
            "  --tst [MEP ID],\n"
            "       Send a test signal towards the MEP ID."
            "  --lck [MEP ID],\n"
            "       We are configured for out-of-service test send LCKs"
            "  --count [count],  \n"
            "       Count how many e.g. LBMs are sent.\n"
            "  --pattern [pattern], \n"
            "       What pattern is used for the TST\n"
            "       (1 = Zeroes and 2 = PRBS) \n"
            "  --octets [octets],\n"
            "       How long the TST pattern will be in octets.\n"
            "  --until,  \n"
            "       Continuously send e.g. LBMs\n"
            "       (\"until\" called again with this on)\n"
            "  --interval [interval],  \n"
            "       Interval on how often the e.g. lmm is sent.\n"
            "       (allowed values for most, 3,33ms, 2 = 10ms, 3 = 100ms\n"
            "       4 = 1s, 5 = 10s, 6 = 1min, 7 = 10min, defaults to 1s)\n"
            "       For LCK/CSF 4 and 6 are allowed.\n"
            "  --interval_us [interval in usecs] \n"
            "       Interval with more precision. Usable only with SLM\n"
            "\n");
}

/**
 * Miminum length of the test pattern 
 */
#define OAM_MINIMUM_TST_PATTERN_LEN 4

/**
 * Maximum length of the test pattern 
 */
#define OAM_MAXIMUM_TST_PATTERN_LEN 1480

static void oam_kick_x(struct oam_tool_information *tool_info,
                       int subopcode)
{
    if (oam_acceptable_subopcode(subopcode)) {
        return;
    }

    if (tool_info->count < 1) {
        OAM_INFO("Incorrect count defaulting to 1\n");
        tool_info->count = 1;
    }

    if (subopcode == OAM_SUB_OPCODE_LCK) {
        if (tool_info->interval != 4 &&
            tool_info->interval != 6) {
            OAM_INFO("Invalid interval for LCK defaulting to 1s\n");
            tool_info->interval = 4;
        }
    } 

    if (tool_info->pattern_len < OAM_MINIMUM_TST_PATTERN_LEN &&
        subopcode == OAM_SUB_OPCODE_TST) {
        OAM_INFO("Too short test pattern given, using (%d)\n", 
                 OAM_MINIMUM_TST_PATTERN_LEN);
        tool_info->pattern_len = OAM_MINIMUM_TST_PATTERN_LEN;
    }

     if (tool_info->pattern_len > OAM_MAXIMUM_TST_PATTERN_LEN &&
        subopcode == OAM_SUB_OPCODE_TST) {
        OAM_INFO("Too long test pattern given, using (%d)\n", 
                 OAM_MAXIMUM_TST_PATTERN_LEN);
        tool_info->pattern_len = OAM_MAXIMUM_TST_PATTERN_LEN;
    }

    if (oam_send_mcc_x(local_fd_out, 
                       out_common_pdu, 
                       tool_info->mepid, 
                       tool_info->count,
                       tool_info->interval,
                       tool_info->loop,
                       tool_info->pattern,
                       tool_info->pattern_len,
                       tool_info->interval_us,
                       subopcode)) {
        OAM_ERROR("Failed to send local msg to mepd.\n");
    }  
}

static void oam_kick_dmm(struct oam_tool_information *tool_info)
{
    OAM_INFO("Starting DMM measurements towards %d with "
             "count %d, loop %d, pattern %d, pattern " 
             "length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_DMM);
} 

static void oam_kick_1dm(struct oam_tool_information *tool_info) 
{
    OAM_INFO("Starting 1DM measurements towards %d with "
             "count %d, loop %d, pattern %d, pattern " 
             "length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_1DM);
} 

static void oam_kick_tst(struct oam_tool_information *tool_info)
{
    if (tool_info->pattern != OAM_PATTERN_TYPE_ZERO_NO_CRC &&
        tool_info->pattern != OAM_PATTERN_TYPE_ZERO_CRC &&
        tool_info->pattern != OAM_PATTERN_TYPE_PRBS_NO_CRC && 
        tool_info->pattern != OAM_PATTERN_TYPE_PRBS_CRC) {
        OAM_INFO("Unknown pattern (%d), defaulting to All zeroes\n", 
                 tool_info->pattern);
        tool_info->pattern = 0;
    }

    OAM_INFO("Starting TST measurements towards %d with "
             "count %d, loop %d, pattern %d, pattern "
             "length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_TST);
}

static void oam_kick_ltm(struct oam_tool_information *tool_info)
{
    OAM_INFO("Starting LTM towards %d with "
             "count %d, loop %d, pattern %d, pattern "
             "pattern length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_LTM);
} 

static void oam_kick_lck(struct oam_tool_information *tool_info)
{
    OAM_INFO("Starting LCK towards %d with "
             "count %d, loop %d, pattern %d, pattern "
             "pattern length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_LCK); 
} 

static void oam_kick_slm(struct oam_tool_information *tool_info)
{
    OAM_INFO("Starting SLM measurements towards %d with "
             "count %d, loop %d, pattern %d, pattern "
             "pattern length %d and interval %d "
             "interval us %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval,
             tool_info->interval_us);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_SLM);
}  

static void oam_kick_lmm(struct oam_tool_information *tool_info)
{
    OAM_INFO("Starting LMM measurements towards %d with "
             "count %d, loop %d, pattern %d, pattern "
             "pattern length %d and interval %d\n", 
             tool_info->mepid, 
             tool_info->count,
             tool_info->loop,
             tool_info->pattern,
             tool_info->pattern_len,
             tool_info->interval);

    oam_kick_x(tool_info, OAM_SUB_OPCODE_LMM);
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
    int                          c;
    char                        *pattern_len_value = NULL;
    char                        *mepid_value = NULL; 
    struct oam_tool_information  tool_info;

    enum oam_onoff slm    = OFF;
    enum oam_onoff lmm    = OFF;
    enum oam_onoff dmm    = OFF;
    enum oam_onoff dm1    = OFF; 
    enum oam_onoff ltm    = OFF;
    enum oam_onoff tst    = OFF;
    enum oam_onoff lck    = OFF;

    extern struct oam_entity *entity;

    /* You need at least one argument */
    if (argc < 2) {
        usage();
        goto out_err;
    }

    /* After the parse cmdline opts so the usage can be viewed
     * as non-root */
    if (getuid()) {
        OAM_ERROR("Root privileges needed!\n");
	  return EXIT_FAILURE;
    }

    memset(&tool_info, 0, sizeof(struct oam_tool_information));

    out_common_pdu = oam_alloc_pdu(); 
                                                        
    oam_set_logtype(LOGTYPE_STDERR);
    oam_set_daemontype(DAEMONTYPE_OAMTOOL);
    
    if (!(entity = calloc(1, sizeof(struct oam_entity)))) {
        OAM_ERROR("Failed to calloc memory for the identity of this entity\n");
        goto out_err;
    }
    sprintf(entity->id_icc, "-");
    
    /* Assign send function */
    entity->send = &oam_send_eth;

    OAM_INFO("Logtype set to stderr\n");
    oam_set_logtype(LOGTYPE_STDERR);
    oam_set_loglevel("all");

    if (oam_init_local_sockets(&local_fd_in, &local_fd_out)) {
        OAM_ERROR("Failed in the initialization of the local sockets.\n");
        goto out_err;
    }

    struct option longopts[] = {
        {"slm", 1, NULL, 'a'},
        {"lmm", 1, NULL, 'm'},
        {"dmm", 1, NULL, 'd'},
        {"1dm", 1, NULL, '1'},
        {"lt", 1, NULL, 'l'},
        {"tst", 1, NULL, 't'},
        {"lck", 1, NULL, 'k'},
        {"interval", 1, NULL, 'i'},
        {"interval_us", 1, NULL, 'n'},
        {"count", 1, NULL, 'c'},
        {"until", 0, NULL, 'u'},
        {"pattern", 1, NULL, 'p'},
        {"octets", 1, NULL, 'o'},
        {0, 0, 0, 0}
    };

    opterr = 0;   
    while ((c = getopt_long_only(argc, argv, "a:m:n:d:t:l:k:c:i:p:o:u", 
                                 longopts, NULL)) != -1) {
        switch (c) {
        case 'c':
            tool_info.count = atoi(optarg);
            break;
        case 'o':
            pattern_len_value = optarg;
            tool_info.pattern_len= strtol(pattern_len_value, NULL, 0);
            break;
        case 'p':
            tool_info.pattern = atoi(optarg);
            break;
        case 'i':
            tool_info.interval = atoi(optarg);
            break;
        case 'n':
            tool_info.interval_us = strtol(optarg, NULL, 0);
            break;
        case 'u':
            tool_info.loop = 1;
            break;
        case 'm':
            /* get MEP ID */ 
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            lmm = ON;
            break;
        case 'a':
            /* get MEP ID */ 
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            slm = ON;
            break;
        case 'd':
            /* get MEP ID */
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            dmm = ON;
            break;
        case '1':
            /* get MEP ID */
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            dm1 = ON;
            break;
        case 'l':
            /* get MEP ID */
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            ltm = ON;
            break;
        case 't':
            /* get MEP ID */
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            tst = ON;
            break;
        case 'k':
            /* get MEP ID */
            mepid_value = optarg;
            tool_info.mepid = strtol(mepid_value, NULL, 0);
            lck = ON;
            break;  
        case ':':
            fprintf (stderr, "\nOption needs a value\n");
            break;
        case '?':
            fprintf (stderr, 
                     "\nUnknown option `-%c'.\n", optopt);
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

    if (slm == ON) {
        oam_kick_slm(&tool_info);
    }
    if (lmm == ON) {
        oam_kick_lmm(&tool_info);
    }
    if (dmm == ON) {
        oam_kick_dmm(&tool_info);
    }
    if (dm1 == ON) {
        oam_kick_1dm(&tool_info);
    }
    if (ltm == ON) {
        oam_kick_ltm(&tool_info);
    }
    if (tst == ON) {
        oam_kick_tst(&tool_info);
    }     
    if (lck == ON) {
        oam_kick_lck(&tool_info);
    }     

    free(out_common_pdu);
    close(local_fd_in);
    close(local_fd_out);
    OAM_FREE(entity);
    return 0;

out_err:
    free(out_common_pdu);
    close(local_fd_in);
    close(local_fd_out);
    OAM_FREE(entity);
    return -1;
}
