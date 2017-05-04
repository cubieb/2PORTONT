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

#ifndef OAM_OUTPUT_H
#define OAM_OUTPUT_H

#include "constants.h"

/* General sending functions */
int oam_send_eth(const int socket, 
                 const void *const dst, 
                 const struct oam_pdu_common *const pdu, 
                 const int pdu_len,
                 const int if_index);
int oam_send_eth_sa(const int socket, 
                    const void *const dst, 
                    const struct oam_pdu_common *const pdu, 
                    const int pdu_len,
                    const int if_index);
int oam_forward_eth(const int socket, 
                    const void *const dst, 
                    const struct oam_pdu_common *const pdu, 
                    const int pdu_len,
                    const int if_index);
int oam_send_local(int fd_out, 
                   struct oam_pdu_common *local_common_pdu,
                   int length);

/* Not so general */
int oam_send_ccd_stat(struct oam_entity_stat *report);
int oam_send_ccd(const char *fmt, ...);

unsigned char *oam_get_mac_of_interface_name(char * ifrname);
unsigned char *oam_get_mac_of_interface_index(int ifindex); 

/* Specific local functions */
int oam_send_mcc_x(int fd_out_local,
                   struct oam_pdu_common *out_common_pdu,
                   uint16_t mepid, 
                   uint8_t count,
                   uint8_t interval,
                   uint8_t loop,
                   uint8_t pattern,
                   uint16_t pattern_len,
                   uint32_t interval_us,
                   int subopcode);

/* "maintenance" loops*/
int oam_send_ccms(const int fd_out, 
                  int counters,
                  int period,
                  int dynamic);
int oam_send_ccm_multi(const int fd_out, 
                       int period);
void oam_send_the_rest(const int fd_out,
                       uint8_t ttl);
int oam_send_lbm(const int fd_out,
                 struct oam_entity_stat *ptr_stat); 
int oam_send_slm(const int fd_out,
                 struct oam_entity_stat *ptr_stat);
int oam_send_lmm(const int fd_out,
                 struct oam_entity_stat *ptr_stat);
int oam_send_dmm(const int fd_out,
                 struct oam_entity_stat *ptr_stat);
int oam_send_1dm(const int fd_out,
                 struct oam_entity_stat *ptr_stat);
int oam_send_ltm(const int fd_out,
                 struct oam_entity_stat *ptr_stat,
                 uint8_t ttl);
int oam_send_tst(const int fd_out,
                 struct oam_entity_stat *ptr_stat);
int oam_send_lck(const int fd_out,
                 struct oam_entity_stat *ptr_stat);

/* Functions that start measurement */
int oam_start_x(struct oam_tool_information *tool_info, int subopcode);

int oam_send_to_other_directions(const int fd_out,
                                 struct oam_pdu_common * pdu_in,
                                 const unsigned char *const from_mac);

#endif /* OAM_OUTPUT_H */
