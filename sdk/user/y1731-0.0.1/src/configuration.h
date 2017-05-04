
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

#ifndef OAM_CONFIGURATION_H
#define OAM_CONFIGURATION_H

#include <stdint.h>

#include "constants.h"

#include "ezxml/ezxml.h"

/**
 * Structure that contains all the configurations
 */
struct oam_configuration {
    int dynamic; /**< Do we use dynamic participant finding */
    /* CCM */
    int  ccm_period; /**< CCM period in ms */
    int  ccm_priority; /**< CCM priority */
    enum oam_onoff ccm_one_way_lm; /**< Send counters in CCM or not */
    int ccm_pulses; /**< How many multicast pulses on startup */
    /* LB */
    int  lb_priority; /**< LB priority */
    int  lb_drop; /**< LB Drop eligibility */
    int  lb_data; /**< 0 = all zeroes pattern, 1 = PRBS */
    /* LT */ 
    int  lt_priority; /**< LT priority */
    int  lt_ttl; /**< LT TTL */ 
    /* AIS */
    int  ais_period; /**< AIS period in ms */
    int  ais_priority; /**< AIS priority */
    /* LCK */
    int  lck_period; /**< LCK period in ms */
    int  lck_priority; /**<  LCK priority */
    /* TST */ 
    int  tst_priority; /**< TST priority */
    int  tst_drop; /**< TST Drop eligibility */
    int  tst_data; /**< 0 = all zeroes pattern, 1 = PRBS */
    /* MCC */
    int  mcc_priority; /**< MCC priority */
    char mcc_oui[10]; /**< Organizationally Unique Identifier */
    /* LM */
    enum oam_onoff lm; /**< LM 1=on/0=off */
    int  lm_period; /**< LM period in ms */
    int  lm_priority; /**< LM priority */
    uint32_t  lm_warning; /**< LM warning limit */
    uint32_t  lm_error; /**< LM error limit */
    uint32_t  lm_samples; /**< LM reference period */
    uint32_t  lm_th_warning; /**< How many times the warning limit may be exceeded befor reporting */
    uint32_t  lm_th_error; /**< How many times the error limit may be exceeded before reporting */
    /* DM */
    enum oam_onoff dm; /**< LM 1=on/0=off */
    int  dm_priority; /**< DM priority */
    enum oam_onoff dm_optional_fields; /**< Do we send optional fields or not */
    int  dm_period; /**< LM period in ms */
    uint32_t  dm_warning; /**< LM warning limit */
    uint32_t  dm_error; /**< LM error limit */
    uint32_t  dm_samples; /**< LM reference period */
    uint32_t  dm_th_warning; /**< How many times the warning limit may be exceeded befor reporting */
    uint32_t  dm_th_error; /**< How many times the error limit may be exceeded before reporting */
    /* include */
    char include_string[126]; /**< which interfaces will be excluded upon init */
} __attribute__ ((packed));

void oam_print_configuration(const struct oam_configuration const *print_me);

ezxml_t oam_open_configuration(const char *const config_file);

int oam_configure(const char const *config_file,
                  const char const *meg_file, 
                  struct oam_configuration *storage,
                  struct oam_db *shm);

int oam_read_config_from_xml(const ezxml_t config_xml,
                             struct oam_configuration *storage);
int oam_read_participants_from_xml(const ezxml_t meg_file,
                                   struct oam_db *shm,
                                   struct oam_configuration *storage);

void oam_close_configuration(ezxml_t config_xml);

#endif /* OAM_CONFIGURATION_H */
