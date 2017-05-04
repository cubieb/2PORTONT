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
 * Feature test macro for struct in6_addr */
#define _BSD_SOURCE

#include <errno.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>

#include <net/if.h>

#include <sys/stat.h>

#include "configuration.h"
#include "constants.h"
#include "debug.h"
#include "tools.h"

#include "ezxml/ezxml.h"

/**
 * Convert the period from the configuration file to usec.
 */
static uint64_t oam_convert_period_config(const char const *precision, 
                                          const char const *period) 
{
    uint64_t usec = 0;

    if (precision[0] == 's') {
        usec= atoi(period);
        usec = usec * OAM_USEC_IN_S;
    } else if (precision[0] == 'm' &&
               precision[1] == 's') {
        usec = atoi(period);        
        usec = usec * OAM_USEC_IN_MS;
        /* add the needed 0,33 ms */
        if (usec == 3000) {
            usec += 330;
        }
    } else if (precision[0] == 'm' &&
               precision[1] == 'i' &&
               precision[2] == 'n') {
        usec = atoi(period);
        usec = usec * OAM_USEC_IN_MIN;
    } else {
        OAM_ERROR("Unknown precision, returning default\n");
        usec = OAM_CCM_DEFAULT_USEC;
    }
    return usec;
}

/**
 * @short Debug print the configuration information.
 * 
 * @param print_me Configuration to be printed.
 */
void oam_print_configuration(const struct oam_configuration const *print_me)
{
    OAM_DEBUG("\n"
              "\nConfiguration information\n"
              "=========================\n"
              /* CCM */  
              "Dynamic              : %s (%d)\n"
              "CCM Period (usec)    : %d\n"
              "CCM one-way LM       : %s\n"
              "CCM Pulses           : %d\n"
              /* LT */
              "LT TTL               : %d\n"
              /* AIS */
              "AIS Period (usec)    : %d\n"
              /* LCK */
              "LCK Period (usec)    : %d\n"
              /* MCC */
              "MCC OUI              : %s\n"
              /* LM */
              "LM                   : %s\n"
              "LM Period (usec)     : %d\n"
              "LM error limit       : %d\n"
              "LM warning limit     : %d\n"
              "LM samples           : %d\n"
              "LM warning threshold : %d\n"
              "LM error threshold   : %d\n"
              /* DM */
              "DM                   : %s\n"
              "DM opt. fields       : %s\n"
              "DM Period (usec)     : %d\n"
              "DM error limit       : %d\n"
              "DM warning limit     : %d\n"
              "DM samples           : %d\n"
              "DM warning threshold : %d\n"
              "DM error threshold   : %d\n"
              /* exclude */
              "Include IFs          : %s\n\n", 
              ((print_me->dynamic == 1) ? "ON" : ((print_me->dynamic == 2) ? "ID" : "OFF")), print_me->dynamic, 
              print_me->ccm_period,
              ((print_me->ccm_one_way_lm == 1) ? "ON" : "OFF"),
              print_me->ccm_pulses,
              print_me->lt_ttl,
              print_me->ais_period, 
              print_me->lck_period,
              print_me->mcc_oui,
              ((print_me->lm == 1) ? "ON" : "OFF"),
              print_me->lm_period,
              print_me->lm_warning,
              print_me->lm_error, 
              print_me->lm_samples,
              print_me->lm_th_warning,
              print_me->lm_th_error,
              ((print_me->dm == 1) ? "ON" : "OFF"),
              ((print_me->dm_optional_fields == 1) ? "ON" : "OFF"),
              print_me->dm_period,
              print_me->dm_warning,
              print_me->dm_error, 
              print_me->dm_samples,
              print_me->dm_th_warning,
              print_me->dm_th_error,
              print_me->include_string);

    OAM_STAT("Dynamic %d\n", print_me->dynamic);
    OAM_STAT("CCM Period (usec) %d\n", print_me->ccm_period);
    OAM_STAT("CCM One-Way LM %d\n", 
             ((print_me->ccm_one_way_lm == 1) ? "ON" : "OFF")); 
    OAM_STAT("CCM Pulses %d\n", print_me->ccm_pulses); 

    OAM_STAT("LT TTL %d\n", print_me->lt_ttl);

    OAM_STAT("AIS Period (usec) %d\n", print_me->ais_period);

    OAM_STAT("LCK Period (usec) %d\n", print_me->lck_period);

    OAM_STAT("MCC OUI %s\n", print_me->mcc_oui);

    OAM_STAT("LM %s\n", 
             ((print_me->lm == 1) ? "ON" : "OFF"));

    OAM_STAT("LM Period (usec) %d\n", print_me->lm_period);

    OAM_STAT("DM %s\n", 
             ((print_me->dm == 1) ? "ON" : "OFF"));
    OAM_STAT("DM optional fields %s\n", 
             ((print_me->dm_optional_fields == 1) ? "ON" : "OFF"));
    OAM_STAT("DM warning %d\n", print_me->dm_warning);
    OAM_STAT("DM error (usec) %d\n", print_me->dm_error);
    OAM_STAT("DM samples %d\n", print_me->dm_samples);
    OAM_STAT("DM warning threshold %d\n", print_me->dm_th_warning);
    OAM_STAT("DM error threshold %d\n", print_me->dm_th_error);

    OAM_STAT("LM Period (usec) %d\n", print_me->lm_period);
    OAM_STAT("LM warning %d\n", print_me->lm_warning);
    OAM_STAT("LM error (usec) %d\n", print_me->lm_error);
    OAM_STAT("LM samples %d\n", print_me->lm_samples);
    OAM_STAT("LM warning threshold %d\n", print_me->lm_th_warning);
    OAM_STAT("LM error threshold %d\n", print_me->lm_th_error);
}

/**
 * This file opens the given file and returns a pointer to it.
 *
 * @param config_file File path to open.
 *
 * @return ezxm_t pointer to the configuration on success, NULL else.
 *
 * @note User is responsible of freeing the file.
 */
ezxml_t oam_open_configuration(const char *const config_file)
{
    ezxml_t     config_xml = NULL;
    config_xml = ezxml_parse_file(config_file);

    if (config_xml == NULL) {
        OAM_ERROR("%s, does not exist %d\n", config_file);
    }

    return config_xml;
}

/**
 * Close the configuration file.
 * 
 * @param config_xml Configuration file to close.
 */
void oam_close_configuration(ezxml_t config_xml)
{
     ezxml_free(config_xml);
}

/**
 * This macro is used in oam_read_config_from_xml for opening and checking
 * the existence of a XML tag
 */
#define OAM_XML_TAG(xml_tag, tag_name, storage) \
    if ((tag = ezxml_child(xml_tag, tag_name)) == NULL) { \
    OAM_ERROR("No %s TAG in configuration\n", tag_name); \
        return -1; \
    }

/** 
 * This macro is used in oam_read_config_from_xml for opening and checking
 * the existence of XML tags attribute
 */
#define OAM_XML_ATTR(xml_tag, attr_name, pointer) \
    if ((pointer = (char *)ezxml_attr(xml_tag, attr_name)) == NULL) { \
        OAM_ERROR("No %s attribute in meg TAG\n", attr_name); \
        return -1; \
    }

/**
 * Toggle the parameter based on configuration
 *
 * @param attribute On or off
 * @param used Pointer whose content should be changed
 *
 * @note defaults to OFF
 */
static void oam_toggle_used(char *attribute, enum oam_onoff *used)
{
    if (strcmp(attribute, "on") == 0 ) {
        *used = ON;
    } else {
        *used = OFF;
    }
}

/**
 * This function reads the configuration file from disk. 
 * 
 * @param config_xml ezxml_t configuration.
 * @param storage where the configuration is stored.
 *
 * @return zero on success, non-zero else.
 * 
 * @note Also sets the configuration
 */
int oam_read_config_from_xml(const ezxml_t config_xml,
                            struct oam_configuration *storage) {
    int                 i = 0;
    char               *attribute;
    char               *attribute_precision;
    ezxml_t             tag;
    
    extern struct oam_entity *entity;

    /* MEG */
    OAM_XML_TAG(config_xml, "meg", tag);
    OAM_XML_ATTR(tag, "id", attribute);
    memset(entity->id_icc, 0, sizeof(entity->id_icc));
    sprintf(entity->id_icc, "%s", attribute); 
    OAM_XML_ATTR(tag, "level", attribute);
    entity->meg_level = atoi(attribute);
    if (entity->meg_level < 0 || entity->meg_level > 7) {
        OAM_ERROR("MEG level must be between 1-7\n");
        return -1;
    }    

    /* MYID */
    OAM_XML_TAG(config_xml, "myid", tag);
    OAM_XML_ATTR(tag, "id", attribute);
    entity->mepid = htons(atol(attribute));

    /* config */
    OAM_XML_TAG(config_xml, "config", tag);
    OAM_XML_ATTR(tag, "log", attribute);
    if (!strcmp(attribute, "syslog")) {
        OAM_INFO("Logtype set to syslog\n");
        oam_set_logtype(LOGTYPE_SYSLOG);
    } else {
        /* default */
        OAM_INFO("Logtype set to stderr\n");
        oam_set_logtype(LOGTYPE_STDERR);
    }
    OAM_XML_ATTR(tag, "loglevel", attribute);
    if (strcmp(attribute, "all") == 0) {
        oam_set_loglevel(attribute);
    } else if (strcmp(attribute, "medium") == 0 ) {
        oam_set_loglevel(attribute);
    } else if (strcmp(attribute, "xtra") == 0 ) {
        oam_set_loglevel(attribute);
    } else if (strcmp(attribute, "none") == 0 ) {
        oam_set_loglevel(attribute);
    } else {
        oam_set_loglevel("medium");        
    }
    OAM_XML_ATTR(tag, "pulses", attribute);
    i = atoi(attribute);
    if (i > 0 && i < 32) {
        storage->ccm_pulses = i;
    } else {
        storage->ccm_pulses = 5;
    }
    OAM_XML_ATTR(tag, "dynamic", attribute);
    if (strcmp(attribute, "on") == 0 ) {
        storage->dynamic = 1;
    } else if (strcmp(attribute, "id") == 0 ) {
            storage->dynamic = 2;
    } else {
        storage->dynamic = 0;
    } 

    OAM_XML_TAG(config_xml, "ccd", tag);
    OAM_XML_ATTR(tag, "use", attribute);
    oam_toggle_used(attribute, &entity->ccd);
    if (strcmp(attribute, "on") == 0 ) {
        entity->ccd = ON;
    } else {
        entity->ccd = OFF;
    }  
    OAM_XML_ATTR(tag, "ip", attribute);
    sprintf(entity->srv_ip, "%s\n", attribute);
    OAM_XML_ATTR(tag, "port", attribute);
    entity->srv_port = atoi(attribute);
 
    /* ccm */
    OAM_XML_TAG(config_xml, "ccm", tag);
    OAM_XML_ATTR(tag, "period", attribute); 
    OAM_XML_ATTR(tag, "precision", attribute_precision);
    storage->ccm_period = 
         oam_convert_period_config(attribute_precision, attribute);    
    OAM_XML_ATTR(tag, "one-way-loss-measurement", attribute);
    oam_toggle_used(attribute, &storage->ccm_one_way_lm);

    /* lt */
    OAM_XML_TAG(config_xml, "lt", tag);
    OAM_XML_ATTR(tag, "ttl", attribute);
    i = atoi(attribute);
    if (i < 1) {
        OAM_ERROR("TTL cannot be zero or less, defaulted to 1\n");
        i = 1;
    }
    storage->lt_ttl = i;

    /* ais */
    OAM_XML_TAG(config_xml, "ais", tag);
    OAM_XML_ATTR(tag, "period", attribute);
    OAM_XML_ATTR(tag, "precision", attribute_precision);
    storage->ais_period = 
         oam_convert_period_config(attribute_precision, attribute);

    /* lck */
    OAM_XML_TAG(config_xml, "lck", tag);
    OAM_XML_ATTR(tag, "period", attribute);
    OAM_XML_ATTR(tag, "precision", attribute_precision);
    storage->lck_period = 
         oam_convert_period_config(attribute_precision, attribute);

    /* mcc */
    OAM_XML_TAG(config_xml, "mcc", tag);
    OAM_XML_ATTR(tag, "oui", attribute);
    memcpy(storage->mcc_oui, attribute, sizeof(storage->mcc_oui));

    /* lm */
    OAM_XML_TAG(config_xml, "lm", tag);
    OAM_XML_ATTR(tag, "used", attribute);
    oam_toggle_used(attribute, &storage->lm);
    OAM_XML_ATTR(tag, "period", attribute);
    OAM_XML_ATTR(tag, "precision", attribute_precision);
    storage->lm_period = 
         oam_convert_period_config(attribute_precision, attribute);
    OAM_XML_ATTR(tag, "warning", attribute); 
    storage->lm_warning = atoi(attribute); 
    OAM_XML_ATTR(tag, "error", attribute); 
    storage->lm_error = atoi(attribute); 
    OAM_XML_ATTR(tag, "samples", attribute); 
    storage->lm_samples = atoi(attribute); 
    OAM_XML_ATTR(tag, "th_warning", attribute); 
    storage->lm_th_warning = atoi(attribute); 
    OAM_XML_ATTR(tag, "th_error", attribute); 
    storage->lm_th_error = atoi(attribute);  

    /* dm */
    OAM_XML_TAG(config_xml, "dm", tag);
    OAM_XML_ATTR(tag, "used", attribute);
    oam_toggle_used(attribute, &storage->dm);
    OAM_XML_ATTR(tag, "optional_fields", attribute);
    if (strcmp(attribute, "on") == 0 ) {
         storage->dm_optional_fields = ON;
    } else {
        storage->dm_optional_fields = OFF;
    }
    OAM_XML_ATTR(tag, "period", attribute);
    OAM_XML_ATTR(tag, "precision", attribute_precision);
    storage->dm_period = 
         oam_convert_period_config(attribute_precision, attribute);
    OAM_XML_ATTR(tag, "warning", attribute); 
    storage->dm_warning = atoi(attribute); 
    OAM_XML_ATTR(tag, "error", attribute); 
    storage->dm_error = atoi(attribute); 
    OAM_XML_ATTR(tag, "samples", attribute); 
    storage->dm_samples = atoi(attribute); 
    OAM_XML_ATTR(tag, "th_warning", attribute); 
    storage->dm_th_warning = atoi(attribute); 
    OAM_XML_ATTR(tag, "th_error", attribute); 
    storage->dm_th_error = atoi(attribute);  

    OAM_XML_TAG(config_xml, "include", tag);
    OAM_XML_ATTR(tag, "ifs", attribute);
    snprintf(storage->include_string,
             sizeof(storage->include_string), 
             "%s", 
             attribute);
    return 0;
}

/**
 * This function reads the participants from the configuration file. 
 * 
 * @param meg_xml ezxml_t configuration file for participants.
 * @param db A pointer where the shared memory pointer is stored.
 * @param storage where the configuration is.
 *
 * @return Number of participants (>= 0 on success as dynamic may be on).
 *
 * @note The config.xml tag MEG and its attribut max tells the
 *       maximum number of participants that can be handled.
 */
int oam_read_participants_from_xml(const ezxml_t meg_xml, 
                                   struct oam_db *db,
                                   struct oam_configuration *storage)
{
    int                      i          = 0;
    ezxml_t                  meg;
    ezxml_t                  mep;
    ezxml_t                  tag;
    char                    *attribute;
    unsigned char           *ptr        = NULL;
    struct oam_entity_stat  *ptr_stat   = NULL;
    struct oam_send_control *sc         = NULL; 

    extern struct oam_entity *entity;

    if (storage->dynamic == 1) {
        OAM_INFO("Dynamic is on, skipping the configuration file\n");
        return 0;
    } else if (storage->dynamic < 0 || storage->dynamic > 2) {
        OAM_ERROR("");
        return -1;
    }

    ptr = (unsigned char *)db;
    ptr = ptr + sizeof(struct oam_db);
    for (meg = ezxml_child(meg_xml, "meg"); meg; meg = meg->next) {	 
        i = 0;
        for (mep = ezxml_child(meg, "mep"); mep; mep = mep->next) {
            ptr = (unsigned char *)db + 
                sizeof(struct oam_db) + 
                (i * sizeof(struct oam_entity_stat));
            ptr_stat = (struct oam_entity_stat *)ptr;
            /* id */
            OAM_XML_ATTR(mep, "id", attribute);
            ptr_stat->mepid = atoi(attribute);
           
            if (storage->dynamic == 0) {
                /* mac */
                OAM_XML_TAG(mep, "mac", tag);             
                oam_mac_pton(ptr_stat->mac, 
                             (const char *)tag->txt); 
                /* ifname */
                OAM_XML_TAG(mep, "ifname", tag);
                sprintf(ptr_stat->ifindex_name, "%s",tag->txt);
                ptr_stat->ifindex = if_nametoindex(ptr_stat->ifindex_name);
            }
 
            /* ID ICC */
            strcpy(ptr_stat->id_icc, entity->id_icc);  

            /* LM */
            sc = &ptr_stat->lm_send_control;
            sc->onoff = storage->lm;
            sc->interval = storage->lm_period;
            if (sc->onoff == ON) {
                oam_get_internal_time(&sc->next_send);
            }
            /* DM */
            sc = &ptr_stat->dm_send_control;
            sc->onoff = storage->dm;
            sc->interval = storage->dm_period;
            if (sc->onoff == ON) {
                oam_get_internal_time(&sc->next_send);
            }

            i++;
        }
    }
    db->participants = i;
    return i;
} 

/**
 * This function reads and sets the configuration. 
 * 
 * @param config_file Where to read the configuration.
 * @param meg_file Where to read the participants from.
 * @param storage where the configuration is stored.
 * @param db A pointer where the shared memory pointer is stored.
 *
 * @return zero on success, non-zero else.
 */
int oam_configure(const char const *config_file,
                  const char const *meg_file,
                  struct oam_configuration *storage,
                  struct oam_db *db)
{
    int     err        = 0;
    ezxml_t config_xml = NULL;
    ezxml_t meg_xml = NULL;
    
    config_xml = oam_open_configuration(config_file);
    if (config_xml == NULL) {
        err = -1;
        goto out_err;
    }
    meg_xml = oam_open_configuration(meg_file);
    if (meg_xml == NULL) {
        err = -1;
        goto out_err;
    }
    if (oam_read_config_from_xml(config_xml, storage)) { 
        OAM_ERROR("Failed to read the configuration file.\n");
        err = -1;
        goto out_err;
    }
    OAM_DEBUG("Configuration read\n"); 
    if (oam_read_participants_from_xml(meg_xml, db, storage) < 0) { 
        OAM_ERROR("Failed to read the participants.\n");
        err = -1;
        goto out_err;
    }
    OAM_DEBUG("Participants read\n");

    oam_close_configuration(config_xml);
    oam_close_configuration(meg_xml);
    
    oam_print_configuration(storage);

out_err:
     return err;
}
