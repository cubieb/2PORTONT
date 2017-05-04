/* -- updatedd: ods.c --
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

#include <dprintf.h>
#include <get_connection.h>
#include <libexception_handle.h>
#include <ret_codes.h>
#include <version.h>

#include "ods.h"

static void
print_usage(char *pname, FILE *fp)
{
	(void)fprintf(fp,
		      "\nUsage: %s [...] %s -- [OPTION]... [USERNAME:PASSWORD] HOSTNAME\n\n",
		      pname, COLORED("ods"));
	(void)fprintf(fp,
		      "For security reasons use the environment variable LOGIN instead of\n"
		      "passing the login information directly.\n\n"
		      "Options:\n"
		      "   -4    --ipv4 <address>        ip address version 4\n"
		      "         --help                  print help and exit\n"
		      "         --version               display version information and exit\n\n"
		      "Report bugs to <"EMAIL">.\n\n");

	return;
}

static void
print_version(FILE *fp)
{

	(void)fprintf(fp,
                      "\n" PNAME " plugin for ods.org version " VERSION ",\n"
                      "Copyright (C) 2005 Philipp Benner.\n"
                      HOMEPAGE "\n\n"

                      "This is free software, and you are welcome to redistribute it\n"
                      "under certain conditions; see the source for copying conditions.\n"
                      "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
                      "FOR A PARTICULAR PURPOSE.\n\n");

	return;

}

static void
ret_msg(int mode, const char *fmt, ...)
{

	va_list az;

	va_start(az, fmt);
	(void)vs_warn(ret_msg_buf, BUFSIZE, mode, fmt, az);
	va_end(az);

	return;

}

int
dyndns(int argc, char *argv[])
{

	struct arguments args;
	const char *ptr;
	int s, ret;

	(void)memset(&args, 0, sizeof(struct arguments));
	
	if(get_flags(&args, argc, argv) != RET_OK) {
		return RET_WRONG_USAGE;
	}

	s = get_connection(DYNDNSHOST, PORT, &ptr);
	if(s == -1) {
		ret_msg(HERR, "%s: %s", ptr, DYNDNSHOST);
		ret = RET_WARNING;
	} else {
		ret = update_dyndns(s, &args);
		(void)close(s);
	}

	return ret;

}

static int
get_flags(struct arguments *args, int argc, char *argv[])
{

	int c;

	for(;;) {

		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",		1, 0, '4' },
			{ "help",		0, 0, 'h' },
			{ "version",		0, 0, 'v' },
			{ NULL,			0, 0, 0   }
		};

		c = getopt_long(argc, argv, "4:",
				long_options, &option_index);

		if(c == -1) break;

		switch(c) {
		case '4':
			args->ipv4 = optarg;
			break;
		case 'h':
			print_usage(argv[ARGV_PNAME], stdout);
			exit(EXIT_SUCCESS);
		case 'v':
			print_version(stdout);
			exit(EXIT_SUCCESS);
		}
	}

	switch(argc-optind) {
        default:
		ret_msg(NONE, "wrong usage");
		return RET_WRONG_USAGE;

        case 2:
		args->login = getenv("LOGIN");
		if(args->login == NULL) {
			ret_msg(NONE,
				"environment variable LOGIN is empty");
			return RET_WRONG_USAGE;
		}
		break;
        case 3:
		args->login = argv[ARGV_LOGIN];
	}
	args->hostname = argv[ARGV_HOSTNAME];

	return RET_OK;

}

static char *
server_msg(int s, char *msg, int err_code)
{

	(void)memset(msg, 0, BUFSIZE);
	(void)read(s, msg, BUFSIZE-1);

	*(msg+3) = '\0';
	if(err_code != atoi(msg))
		return (msg+4);
	return NULL;

}

static int
update_dyndns(int s, struct arguments *args)
{

	char *user, *pass, *ip_addr, *ptr;
	char msg[BUFSIZE];
        int len = strlen(args->login);
        char login[len];

        strcpy(login, args->login);
	user = strtok(login, ":");
	pass = strtok(NULL, "");

	if(args->ipv4) {
		ip_addr = args->ipv4;
	} else {
		ip_addr = (char *)"CONNIP";
	}

	if((ptr = server_msg(s, msg, 100)) != NULL) {
		ret_msg(NONE, "%s: %s", args->hostname, ptr);
		return RET_ERROR;
	} else {
		(void)dprintf(s, "LOGIN %s %s\n", user, pass);
		if((ptr = server_msg(s, msg, 225)) != NULL) {
			ret_msg(NONE, "%s: %s", args->hostname, ptr);
			return RET_ERROR;
		} else {
			(void)dprintf(s, "DELRR %s A\n", args->hostname);
			if((ptr = server_msg(s, msg, 901)) != NULL) {
				ret_msg(NONE, "%s: %s", args->hostname, ptr);
				return RET_ERROR;
			} else {
				(void)dprintf(s, "ADDRR %s A %s\n", args->hostname, ip_addr);
				if((ptr = server_msg(s, msg, 795)) != NULL) {
					ret_msg(NONE, "%s: %s", args->hostname, ptr);
					return RET_ERROR;
				} else {
					ret_msg(NONE, "%s: update successful", args->hostname);
				}
			}
		}
	}

	return RET_OK;

}
