/***************************************************************************
 *            private.h
 *
 *  Mon Jun 13 14:23:23 2005
 *  Copyright  2005  Simon Morlat
 *  Email simon dot morlat at linphone dot org
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _PRIVATE_H
#define _PRIVATE_H

void linphone_auth_info_write_config(struct _LpConfig *config, LinphoneAuthInfo *obj, int pos);

void linphone_core_update_proxy_register(LinphoneCore *lc);
void linphone_core_refresh_subscribes(LinphoneCore *lc);

#if 0

int linphone_proxy_config_send_publish(LinphoneProxyConfig *cfg, LinphoneOnlineStatus os);

int linphone_online_status_to_eXosip(LinphoneOnlineStatus os);

void linphone_friend_set_sid(LinphoneFriend *lf, int cid);
void linphone_friend_set_nid(LinphoneFriend *lf, int cid);
void linphone_friend_notify(LinphoneFriend *lf, int ss, int os);

#endif


#endif /* _PRIVATE_H */
