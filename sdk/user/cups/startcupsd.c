/* 
USB Printer Handler
tsaitc 2012/08/01

handles linux 2.6 hotplug event for USB Printers
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <config/autoconf.h>
#include <fcntl.h>
#include <rtk/options.h>
#include <libgen.h>
#include <assert.h>
#include <err.h>
#include <glob.h>
#include <sys/inotify.h>
#include <errno.h>

/*
Environment Variables:
DEVPATH
ACTION
*/

#define BUFFER_SIZE 32
#define STARTCUPSD_DEBUG 

static const char CUPSDPID[] = "/var/run/cupsd.pid";

struct action_table {
	char *action_str;
	int (*action_func)(int, char **, const char *);
};

static int action_add(int argc, char **argv, char *devpath) 
{
	int ret, retry_count;
	pid_t pid;

	const char * const bname = basename(devpath);
	char uri[BUFFER_SIZE] = "parallel:/dev/";

#ifdef STARTCUPSD_DEBUG
	warnx("action add: %s", devpath);
#endif

	pid = read_pid((char *)CUPSDPID);
	if (pid <= 0) {
		/* cupsd is not running */

		/* /var/cups/conf/cupsd.conf must exist before executing cupsd */
		for (retry_count = 0; retry_count < 3 
				&& access("/var/cups/conf/cupsd.conf", F_OK); retry_count++) {
			/* cupsd.conf does not exist */
#ifdef STARTCUPSD_DEBUG
			warn("%s:%d, errno %d", __FUNCTION__, __LINE__, errno);
#endif
			sleep(1 << retry_count);
		}

		va_cmd("/bin/cupsd", 0, 0);
	}

	for (retry_count = 0; retry_count < 3 
			&& access("/var/run/cupsd.pid", F_OK); retry_count++) {
		/* cupsd.pid does not exist */
#ifdef STARTCUPSD_DEBUG
		warn("%s:%d, errno %d", __FUNCTION__, __LINE__, errno);
#endif
		sleep(1 << retry_count);
	}

	strcat(uri, bname);

	/* lpadmin -p lp0 -v parallel:/dev/lp0 -E */
	ret = va_cmd("/bin/lpadmin", 5, 0, "-p", bname, "-v", uri, "-E");

	return ret;
}

static int action_remove(int argc, char **argv, char *devpath) 
{
	int ret;
	glob_t globbuf;
	pid_t pid;

	const char * const bname = basename(devpath);

#ifdef STARTCUPSD_DEBUG
	warnx("action remove: %s", devpath);
#endif

	/* lpadmin -x lp0 */
	ret = va_cmd("/bin/lpadmin", 2, 1, "-x", bname);

	glob("/dev/*lp*", 0, NULL, &globbuf);

	/* the last USB Printer will be removed */
	if (globbuf.gl_pathc <= 1) {
		pid = read_pid((char *)CUPSDPID);
		if (pid > 0) {
			/* cupsd is running */
			kill(pid, 9);
			unlink(CUPSDPID);
		}
	}

	globfree(&globbuf);

	return ret;
}

static const struct action_table hotplug_table[] = {
	{ "add", action_add },
	{ "remove", action_remove },	
	{ NULL, NULL },
};

int main(int argc, char **argv)
{
	const char *devpath;
	const char *action;
	const struct action_table *p;
#ifdef STARTCUPSD_DEBUG
	int fd;

	fd = open("/dev/console", O_WRONLY);
	dup2(fd, STDERR_FILENO);
	close(fd);
#endif

	devpath = getenv("DEVPATH");
	assert(devpath != NULL);
	
	action = getenv("ACTION");
	assert(action != NULL);

	for (p = &hotplug_table[0]; p->action_str; p++) {
		if (strcmp(action, p->action_str) == 0) {
			return p->action_func(argc - 1, &argv[1], devpath);
		}
	}

	return 0;
}

