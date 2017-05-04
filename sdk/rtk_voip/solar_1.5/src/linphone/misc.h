/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MISC_H
#define MISC_H

#include "linphonecore.h"
#include "../oRTP/ortp.h"

int linphone_arts_suspend(void);
int linphone_arts_restore(void);
gint do_registration(LinphoneCore *lc, gboolean doit);
void check_for_registration(LinphoneCore *lc);
int from_2char_without_params(osip_from_t *from,char **str);
void linphone_core_check_codecs_for_bandwidth(LinphoneCore *lc);
void linphone_core_setup_local_rtp_profile(LinphoneCore *lc);
gboolean host_has_ipv6_network(void);

#define PAYLOAD_TYPE_ENABLED	(1<<7)
#define PAYLOAD_TYPE_USABLE		(1<<8)


#endif
