/* -- updatedd: dyndns.h --
 *
 * Copyright (C) 2016 Relatek
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

#define DYNDNSHOST	"ddns.oray.com"
#define PORT		80

#define BUFSIZE		512

#define ARGV_PNAME	0
#define ARGV_LOGIN	argc-2
#define ARGV_HOSTNAME	argc-1

#define COLORED(x)	"\033[0;33;1m"x"\033[0m"

static char ret_msg_buf[BUFSIZE];

struct arguments {
	char *hostname;
	char *ipv4;
	char *login;
};

static struct dyndns_return_codes {
	const char *code;
	const char *message;
	const int  error;
} return_codes[] = {
	{ "good",	"Update good and successful, IP updated.",              0 },
	{ "nochg",	"No changes, update considered abusive.",               0 },
	{ "notfqdn",	"A Fully-Qualified Domain Name was not provided.",      1 },
	{ "nohost",	"The hostname specified does not exist.",               1 },
	{ "!donator",	"The offline setting was set, when the user is "
          "not a donator.",                                                     1 },
	{ "abuse",      "The hostname specified is blocked for abuse",          1 },
	{ "911",	"Oray system error.",                               1 },
	{ NULL,		NULL,                                                   0 }
};

static int get_flags(struct arguments *args, int argc, char *argv[]);
static int update_dyndns(int s, struct arguments *args);
static int check_server_msg(int s, char *hostnames);

char *
oray_get_retmsg(void)
{
	return ret_msg_buf;
}

