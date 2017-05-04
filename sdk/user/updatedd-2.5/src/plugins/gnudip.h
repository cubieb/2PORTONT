/* -- updatedd: ods.h --
 *
 * Copyright (C) 2004, 2005 Philipp Benner
 *
 * This file is part of UpdateDD - http://updatedd.philipp-benner.de.
 *
 * UpdateDD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * UpdateDD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UpdateDD; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ret_codes.h>

#define USE_HTTP  

#define DYNDNSHOST	"ddns.local"
#ifdef USE_HTTP
#define PORT        80
#define GNUDIPCGI  "/gnudip/cgi-bin/gdipupdt.cgi"
#else
#define PORT		3495
#endif

#define BUFSIZE		1024

#define ARGV_PNAME	0
#define ARGV_LOGIN	argc-2
#define ARGV_HOSTNAME	argc-1

#define COLORED(x)	"\033[0;38;1m"x"\033[0m"

static char ret_msg_buf[BUFSIZE];

struct arguments {
	char *hostname;
	char *ipv4;
	char *mx;
	char *login;
};
 #ifdef USE_HTTP
typedef struct ServerMessage{
	char *salt;
	char *time;
	char *sign;
}SERVERM;
#endif
static int gnudip_get_flags(struct arguments *args, int argc, char *argv[]);

#ifdef USE_HTTP
static int gnudip_update_dyndns_1(int s, struct arguments *args);
static int gnudip_check_server_msg_1(int s, const char *hostnames,SERVERM *servermessage);
static int gnudip_update_dyndns_2(int s, struct arguments *args, SERVERM *servermessage);
static int gnudip_check_server_msg_2(int s, const char *hostnames);
#else
static int gnudip_update_dyndns(int s, struct arguments *args);
static int gnudip_check_server_msg(int s, const char *hostnames);
#endif

char *
gnudip_get_retmsg(void)
{
	return ret_msg_buf;
}

