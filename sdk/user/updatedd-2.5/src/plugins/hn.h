/* -- updatedd: hn.h --
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

#define DYNDNSHOST	"dup.hn.org"
#define PORT		80

#define BUFSIZE		512

#define ARGV_PNAME	0
#define ARGV_LOGIN	argc-2
#define ARGV_HOSTNAME	argc-1

#define COLORED(x)	"\033[0;34;1m"x"\033[0m"

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
        { "101", "Successfully Updated",						0 },
	{ "201", "Failure because previous update occured less than 300 seconds ago",	1 },
	{ "202", "Failure because of server error",					1 },
	{ "203", "Failure because account is frozen (by admin)",			1 },
	{ "204", "Failure because account is locked (by user)",				1 },
	{ NULL, NULL, 0 }
};

static int get_flags(struct arguments *args, int argc, char *argv[]);
static int update_dyndns(int s, struct arguments *args);
static int check_server_msg(int s, char *hostnames);

char *
get_retmsg(void)
{
	return ret_msg_buf;
}
