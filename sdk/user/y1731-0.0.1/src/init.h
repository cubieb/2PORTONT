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

#ifndef OAM_INIT_H
#define OAM_INIT_H

#include "configuration.h"

/** 
 * Maximum length of the character value of the bootcount value. 
 * Maybe 20 digit value is an overkill. 
 */
#define BOOTCOUNT_MAX_LENGTH_IN_CHAR   20

void oam_init_send_intervals_in_db(struct oam_configuration *configuration);
int oam_initialize_db(char * config_file);
void oam_bootcount(void);
int oam_init_raw_sockets(int *fd_in, int *fd_out, enum oam_onoff mip);
int oam_init_local_sockets(int *fd_in, int *fd_out);
int oam_bind_local(int *fd_in);
int oam_bind_multicast(int *fd_in,  enum oam_onoff enable);

#endif /* OAM_INIT_H */

