
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

#ifndef OAM_INPUT_H
#define OAM_INPUT_H

#include "configuration.h"
#include "pdu.h"

int oam_handle_raw_read(struct oam_pdu_common *pdu, 
                        int fd,  
                        int fd_out,
                        struct oam_configuration *configuration,
                        enum oam_onoff mip);
int oam_handle_raw_in(struct oam_pdu_common *pdu, 
                      int fd, 
                      fd_set *fdset, 
                      int fd_out,
                      struct oam_configuration *configuration,
                      enum oam_onoff mip);

int oam_handle_ccm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const struct oam_configuration *const configuration);
int oam_handle_lbm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info, 
                   const int fd_out);
int oam_handle_lbr(const struct oam_recv_information *recv_info);
int oam_handle_ltm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out);
int oam_handle_ltr(const struct oam_pdu_common * common_pdu,
		   const struct oam_recv_information *recv_info);
int oam_handle_ais(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_lck(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_tst(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_laps(const struct oam_recv_information *recv_info);
int oam_handle_raps(const struct oam_recv_information *recv_info);
int oam_handle_mcc(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info, 
                   const enum oam_onoff ethernet);
int oam_handle_slm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info, 
                   const int fd_out);
int oam_handle_slr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_lmm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out);
int oam_handle_lmr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_1dm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_dmm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const int fd_out,
                   enum oam_onoff dm_optional_fields);
int oam_handle_dmr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info,
                   const struct oam_configuration *configuration);
int oam_handle_exm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_exr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_vsr(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);
int oam_handle_vsm(const struct oam_pdu_common * common_pdu,
                   const struct oam_recv_information *recv_info);

#endif /* OAM_INPUT_H */
