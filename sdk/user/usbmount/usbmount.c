/* 
USB Mount Handler
andrew 2008/02/21

handles linux 2.6 hotplug event
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <config/autoconf.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#ifndef CONFIG_ARCH_LUNA_SLAVE
#include <rtk/options.h>
#endif
#include <libgen.h>
#include <assert.h>
#include <err.h>
#ifndef CONFIG_ARCH_LUNA_SLAVE
#include <rtk/utility.h>
#endif

/*
Environment Variables:
DEVPATH
ACTION
*/

#define MNT_ROOT "/mnt"
#define BUFFER_SIZE 32
/* #define USBMOUNT_DEBUG */
#define PROC_MOUNTS "/proc/mounts"

struct action_table {
	char *action_str;
	int	(*action_func)(int, char **, char *);
};

#ifdef CONFIG_USER_MINIDLNA
int stopDLNA_wait()
{
	int count = 0;
	
	stopMiniDLNA();		
	while(read_pid((char*)MINIDLNAPID) > 0 && (count < 40))
	{
#ifdef USBMOUNT_DEBUG		
		warn("MINIDLNA is running. Please wait!\n");
#endif
		usleep(600000);
		count++;
	}
	
	if (count>=40)
		unlink(MINIDLNAPID);
}

int restart_minidlna()
{		
	stopDLNA_wait();
	startMiniDLNA();	
}
#endif

static int gen_source_target(char *source_buffer, char *target_buffer, char *bname)
{
#ifdef CONFIG_E8B
	unsigned int i, j;
#endif

	if (strlen(bname) < 3) {
		return 1;
	}

	snprintf(source_buffer, BUFFER_SIZE, "/dev/%s", bname);

#ifdef CONFIG_E8B
	/* takes sdb1 as a example */

	/* i = 'b' - 'a' + 1, so that i == 2*/
	i = bname[2] - 'a' + 1;

	/* j = '1' - '0', so that j == 1*/
	j = strtoul(bname + 3, NULL, 10);

	snprintf(target_buffer, BUFFER_SIZE, MNT_ROOT"/usb%u_%u", i, j);
#else
	snprintf(target_buffer, BUFFER_SIZE, MNT_ROOT"/%s", bname);
#endif

	return 0;
}

static int action_add(int argc, char **argv, char *devpath) 
{
	int ret, retry_count = 0;
	char source_buffer[BUFFER_SIZE], target_buffer[BUFFER_SIZE];

	if (gen_source_target(source_buffer, target_buffer, basename(devpath))) {
		return 1;
	}

#ifdef USBMOUNT_DEBUG
	warnx("action add: %s", devpath);
#endif

	ret = mkdir(target_buffer, 0755);
	if (ret && errno != EEXIST) {
#ifdef USBMOUNT_DEBUG
		warn("%s:%d", __FUNCTION__, __LINE__);
#endif
		return ret;
	}

retry:

#ifdef CONFIG_USER_NTFS_NTFS3G
	ret = va_cmd("/bin/ntfs-3g", 4, 1, "-o", "noatime", 
			source_buffer, target_buffer);
#else
	ret = 1;
#endif

	if (ret) {
#ifndef CONFIG_ARCH_LUNA_SLAVE
	#ifndef CONFIG_E8B
		ret = va_cmd("/bin/mount", 4, 1, "-o", "noatime,utf8", 
				source_buffer, target_buffer);
	#else
		ret = va_cmd("/bin/mount", 4, 1, "-o", "noatime,iocharset=gb2312,codepage=936",
                                source_buffer, target_buffer);
	#endif
#else
		char buf[128];
		sprintf(buf, "/bin/mount -o noatime,utf8 %s %s", source_buffer, target_buffer);
		system(buf);
#endif
	}

	if (ret && retry_count++ < 2) {
#ifdef USBMOUNT_DEBUG
		warn("%s:%d ret %d, errno %d", __FUNCTION__, __LINE__, ret, errno);
#endif
		sleep(1 << retry_count);
		goto retry;
	}

	if (ret) {
#ifdef USBMOUNT_DEBUG
		warn("%s:%d", __FUNCTION__, __LINE__);
#endif
		rmdir(target_buffer);
	}

#ifdef CONFIG_USER_MINIDLNA	
	if (!ret)
	{
#ifdef USBMOUNT_DEBUG			
		warn("action add: restart_minidlna, %s:%d", __FUNCTION__, __LINE__);
#endif
		restart_minidlna();
	}
#endif
	return ret;
}

static int action_remove(int argc, char **argv, char *devpath) 
{
	int ret, retry_count = 0;;
	char source_buffer[BUFFER_SIZE], target_buffer[BUFFER_SIZE];

	if (gen_source_target(source_buffer, target_buffer, basename(devpath))) {
		return 1;
	}

#ifdef USBMOUNT_DEBUG
	warnx("action remove: %s\n", devpath);
#endif
	if ((ret = access(target_buffer, F_OK)))
		return ret;
	
#ifdef CONFIG_USER_MINIDLNA
	stopDLNA_wait();
#endif

retry:

	ret = umount2(target_buffer, MNT_DETACH);
	ret |= rmdir(target_buffer);

	if (ret && retry_count++ < 3) {
#ifdef USBMOUNT_DEBUG
		warn("%s:%d ret %d, errno %d", __FUNCTION__, __LINE__, ret, errno);
#endif
		sleep(1 << retry_count);
		goto retry;
	}

#ifdef CONFIG_USER_MINIDLNA
	if (!ret)
	{		
#ifdef USBMOUNT_DEBUG	
		warn(" action remove: restart_minidlna, %s:%d", __FUNCTION__, __LINE__);
#endif
		startMiniDLNA();
	}
#endif
	return ret;
}

static const struct action_table hotplug_table[] = {
	{ "add", action_add },
	{ "remove", action_remove },	
	{ NULL, NULL },
};

int main(int argc, char **argv)
{
	char *devpath;
	char *action;
	const struct action_table *p;
#ifdef USBMOUNT_DEBUG
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

