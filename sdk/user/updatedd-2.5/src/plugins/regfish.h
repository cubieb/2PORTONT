/* -- updatedd: regfish.h --
 *
 * Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
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

#define DYNDNSHOST	"www.regfish.com"
#define PORT		80

#define BUFSIZE		512

#define ARGV_PNAME	0
#define ARGV_LOGIN	argc-2
#define ARGV_HOSTNAME	argc-1

#define COLORED(x)	"\033[0;36;1m"x"\033[0m"

static char ret_msg_buf[BUFSIZE];

struct arguments {
	int   force_host;
	char *hostname;
	char *ipv4;
	char *ipv6;
	int   ismx;
	char *login_std;
	char *login_token;
	int   ttl;
};

static struct dyndns_return_codes {
	const char *code;
	const char *message;
	const int  error;
} return_codes[] = {
	{ "401", "standard authentication failed: wrong username or password",	1 },
	{ "402", "secure authentication failed: wrong security hash",		1 },
	{ "403", "standard authentication failed: domain name does not exist",	1 },
	{ "404", "regfish is not authorized to change the domain",		1 },
	{ "405", "invalid SOA-zone, please contact regfish.com",		1 },
	{ "406", "resource record does not exist, use force-host",		1 },
	{ "407", "invalid ttl value",						1 },
	{ "408", "invalid ipv4 address",					1 },
	{ "409", "invalid ipv6 address",					1 },
	{ "410", "invalid auth type, please send a bugreport to the author",	1 },
	{ "411", "could not update MX-host",					1 },
	{ "412", "invalid hostname",						1 },
	{ "413", "unsecure authentication is not admitted",			1 },
	{ "414", "unknown fault",						1 },
	{ "415", "multiple A-pointer found",					1 },
	{ "101", "no changes, update considered abusive",			0 },
	{ "100", "update good and successful, ip updated",			0 },
	{ NULL,  NULL,								0 }
};

static int get_flags(struct arguments *args, int argc, char *argv[]);
static int update_dyndns(int s, struct arguments *args);
static int check_server_msg(int s, char *hostnames);

char *
get_retmsg(void)
{
	return ret_msg_buf;
}
