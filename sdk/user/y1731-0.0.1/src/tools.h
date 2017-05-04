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

#ifndef OAM_TOOLS_H
#define OAM_TOOLS_H

/** 
 * Allowed directions for oam_ask_rtx_packets() 
 */
enum oam_directions {OAM_TX, OAM_RX};

int oam_mac_pton(unsigned char *mac, 
                 const char *mac_addr);
void oam_mac_ntop(char *const mac_addr, 
                  const unsigned char *const mac);
void oam_print_mac(const char * msg, 
                   const unsigned char *const mac);
void oam_print_mac_stderr(const char * msg, 
                          const unsigned char *const mac); 

const char *oam_packet_type_name(const int opcode);
const char *oam_packet_type_name_short(const int opcode);
const char *oam_mcc_subopcode_name(const int subopcode);

uint32_t oam_ask_rtx_packets(const char *const name,
                             const enum oam_directions direction);

struct oam_entity_stat *oam_find(const uint16_t mepid);
struct oam_entity_stat *oam_find_from_db(const struct oam_db *const shm_ptr,
                                          const uint16_t mepid);
struct oam_entity_stat *oam_find_by_mac(const unsigned char *const from_mac);

void oam_timestamp(struct timeval *stamp);
int oam_inside_interval(const char *const msg, 
                        struct timeval *const stamp, 
                        const int interval_sec,
                        const int interval_usec);
enum oam_alarms give_legit_interval(int interval);

void oam_set_alarm_lb(struct oam_db *shm_ptr,
                      struct oam_entity_stat *const entity,
                      const enum oam_alarms onoff);
void oam_set_alarm_ccm(struct oam_db *shm_ptr,
                       struct oam_entity_stat *const entity,
                       const enum oam_alarms onoff);
int oam_check_packet(const struct oam_pdu_common *const pdu,
                     struct oam_recv_information *recv_info,
                     const enum oam_onoff mip,
                     const int socket_out);

int oam_packet_length(const struct oam_pdu_common *const pdu);

int oam_internal_time_expired(const struct oam_internal_time *const stamp);
void oam_add_usecs_to_internal_time(const uint64_t usec, 
                                    struct oam_internal_time * stamp);
void oam_get_internal_time(struct oam_internal_time *now);
void oam_tv_to_internal_time(const struct timeval *const tv, 
                             struct oam_internal_time *now); 

int oam_internal_time_str(char *buffer, 
                          int max, 
                          struct oam_internal_time *it);
int32_t oam_internal_time_diff(const struct oam_internal_time *const start,
                               const struct oam_internal_time *const stop);

int oam_acceptable_subopcode(const int subopcode);

void oam_generate_crc32_table(void);
void oam_do_crc32_checksum(const unsigned char * data, 
                               const uint32_t length, 
                               uint32_t *crc32);

void oam_check_received_seq(const char *const msg,
                            const uint32_t received_seq, 
                            uint32_t *const previous_seq,
                            const char *const from_mac);

char * oam_tst_pattern_name(const int type);
 
void oam_print_byte_in_bits(char byte);
unsigned char* oam_generate_prbs(unsigned char * shift_register, 
                                 int length);

uint32_t oam_calculate_frame_loss(uint32_t ct1,
                                  uint32_t ct2, 
                                  uint32_t cr1,
                                  uint32_t cr2);

void oam_create_multicast_da2(int level, unsigned char *addr);
void oam_create_multicast_da1(int level, unsigned char *addr);

void oam_print_interfaces(void);

int oam_add_mep_to_db(const int meg_level,
                       const uint16_t mepid, 
                       const unsigned char *const mac,
                       const char *const megid,
                       const int ifindex,
                       const char *const ifindex_name);
int oam_update_mep_in_db(const int meg_level,
                          const uint16_t mepid, 
                          const unsigned char *const mac,
                          const char *const megid,
                          const int ifindex,
                          const char *const ifindex_name);
int oam_toggle_promiscuous_mode(const int *const fd_in,  
                                const enum oam_onoff enable);
#endif /* OAM_TOOLS_H */
