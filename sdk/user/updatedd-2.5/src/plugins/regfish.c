/* -- updatedd: regfish.c --
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

#include <get_connection.h>
#include <libexception_handle.h>
#include <ret_codes.h>
#include <version.h>

#include "regfish.h"

static void
print_usage(char *pname, FILE *fp)
{
	(void)fprintf(fp,
		      "\nUsage: %s [...] %s -- [OPTION]... [LOGIN] HOSTNAME\n\n",
		      pname, COLORED("regfish"));
	(void)fprintf(fp,
		      "LOGIN is either TOKEN=<token> or STD=<USER:PASS>\n\n"
		      "For security reasons use the environment variable LOGIN instead of\n"
		      "passing the login information directly.\n\n"
		      "Options:\n"
		      "   -4    --ipv4 <address>        ip address version 4\n"
		      "   -6    --ipv6 <address>        ip address version 6\n"
		      "   -f    --force-host            create hostname if it does not exist\n"
		      "   -i    --ismx                  use this host as your MX-host\n"
		      "   -t    --ttl <num>             time to live (300-86400)\n"
		      "         --help                  print help and exit\n"
		      "         --version               display version information and exit\n\n"
		      "Report bugs to <"EMAIL">.\n\n");

	return;
}

void
print_version(FILE *fp)
{

	(void)fprintf(fp,
		      "\n" PNAME " plugin for regfish.com version " VERSION ",\n"
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
		if(ret == RET_OK) {
			ret = check_server_msg(s, args.hostname);
		}
		(void)close(s);
	}

	return ret;

}

static int
get_flags(struct arguments *args, int argc, char *argv[])
{

	int c;
	char *ptr;

	for(;;) {

		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",		1, 0, '4' },
			{ "ipv6",		1, 0, '6' },
			{ "force-host",		0, 0, 'f' },
			{ "help",		0, 0, 'h' },
			{ "ismx",		0, 0, 'i' },
			{ "ttl",		1, 0, 't' },
			{ "version",		0, 0, 'v' },
			{ NULL,			0, 0, 0   }
		};

		c = getopt_long(argc, argv, "4:6:fit:",
				long_options, &option_index);

		if(c == -1) break;

		switch(c) {
		case '4':
			if(args->ipv6 != NULL) {
				ret_msg(NONE, "use either ipv4 or ipv6");
				return RET_WRONG_USAGE;
			}
			args->ipv4 = optarg;
			break;
		case '6':
			if(args->ipv4 != NULL) {
				ret_msg(NONE, "use either ipv4 or ipv6");
				return RET_WRONG_USAGE;
			}
			args->ipv6 = optarg;
			break;
		case 'f':
			args->force_host = 1;
			break;
		case 'h':
			print_usage(argv[ARGV_PNAME], stdout);
			exit(EXIT_SUCCESS);
		case 'i':
			args->ismx = 1;
			break;
		case 't':
			args->ttl = strtol(optarg, NULL, 0);
			if(args->ttl < 300 || args->ttl > 86400) {
				ret_msg(NONE, "invalid ttl");
				return RET_WRONG_USAGE;
			}
			break;
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
		ptr = getenv("LOGIN");
		if(ptr == NULL) {
			ret_msg(NONE,
				"environment variable LOGIN is empty");
			return RET_WRONG_USAGE;
		}
		break;
	case 3:
		ptr = argv[ARGV_LOGIN];
	}
	if(strncmp(ptr, "TOKEN=", 6) == 0) {
		args->login_token = ptr+6;
	} else if(strncmp(ptr, "STD=", 4) == 0) {
		args->login_std = ptr+4;
	} else {
		ret_msg(NONE, "invalid login information");
		return RET_WRONG_USAGE;
	}
	args->hostname = argv[ARGV_HOSTNAME];

	return RET_OK;

}

#define BUFLEN		4096
#define BUFFREE(name)	BUFLEN - strlen(name)

static int
update_dyndns(int s, struct arguments *args)
{

	char buffer[BUFLEN];
	char message[BUFLEN];

	(void)memset(buffer, 0, BUFLEN);
        (void)memset(message, 0, BUFLEN);
	if(args->ipv4) {
		strncat(buffer, "ipv4=", BUFFREE(buffer));
		strncat(buffer, args->ipv4, BUFFREE(buffer));
	} else {
		strncat(buffer, "thisipv4=1", BUFFREE(buffer));
	}
	if(args->ipv6) {
		strncat(buffer, "&ipv6=", BUFFREE(buffer));
		strncat(buffer, args->ipv6, BUFFREE(buffer));
        }

	if(args->login_std) {
		int i;
		int len = strlen(args->login_std);
                char login[len], *pass = NULL;

                strcpy(login, args->login_std);
		for(i = 0; i < len; i++) {
			if(login[i] == ':') {
				login[i] = '\0';
                                pass = login+1+i;
				break;
			}
		}
		if(pass == NULL) {
			ret_msg(NONE, "password is missing");
			return RET_WRONG_USAGE;
		}

		strncat(buffer, "&authtype=standard&username=", BUFFREE(buffer));
		strncat(buffer, login, BUFFREE(buffer));
		strncat(buffer, "&password=", BUFFREE(buffer));
		strncat(buffer, pass, BUFFREE(buffer));
	} else if(args->login_token) {
		strncat(buffer, "&authtype=secure&token=", BUFFREE(buffer));
		strncat(buffer, args->login_token, BUFFREE(buffer));
	} else {
		ret_msg(NONE, "login information is missing");
		return RET_WRONG_USAGE;
	}

	if(args->ttl) {
		char ttl[BUFSIZE];
		(void)snprintf(ttl, BUFSIZE, "%i", args->ttl);
		strncat(buffer, "&ttl=", BUFFREE(buffer));
		strncat(buffer, ttl, BUFFREE(buffer));
	}

	(void)snprintf(message, BUFLEN,
                       "GET /dyndns/2/?%s&fqdn=%s&forcehost=%i&ismx=%i HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "User-Agent: %s %s - %s\r\n"
                       "Connection: close\r\n"
                       "Pragma: no-cache\r\n\r\n",
                       buffer, args->hostname, args->force_host, args->ismx,
                       DYNDNSHOST, PNAME, VERSION, HOMEPAGE);

	print_debug("\n\nMessage:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
		    message);

	if(write(s, message, strlen(message)) == -1) {
		ret_msg(PERR, "write() failed");
		return RET_WARNING;
	}

	return RET_OK;

}

static int
check_server_msg(int s, char *hostname)
{

	int n;
	char server_msg[BUFSIZE], *ptr;

	/* get server_msg */
	(void)memset(server_msg, 0, sizeof(server_msg));
	if(read(s, server_msg, sizeof(server_msg) - 1) < 0) {
		ret_msg(PERR, "read() failed");
		return RET_WARNING;
	}

	print_debug("\n\nServer message:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
		    server_msg);

	if(strstr(server_msg, "HTTP/1.1 200 OK") ||
	   strstr(server_msg, "HTTP/1.0 200 OK")) {

		(void)strtok(server_msg, "\n");
		while((ptr = strtok(NULL, "\n")) != NULL) {
			for(n=0; return_codes[n].code != NULL; n++) {
				char buf[256];
				(void)memset(buf, 0, 256);
				snprintf(buf, 256, "|%s|", return_codes[n].code);
				if(strstr(ptr, buf)) {
					ret_msg(NONE, "%s: %s",
						hostname, return_codes[n].message);
					if(return_codes[n].error == 1) {
						return RET_ERROR;
					}
				}
			}
		}
	} else {
		ret_msg(NONE, "Internal Server Error");
		return RET_ERROR;
	}

	return RET_OK;
}
