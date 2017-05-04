#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "libnv.h"

#define dbg(f, a...) if (debug) printf("[rtknv]" f, ##a)
#define DEVNAME "/dev/nvram"

#define RTK_NV_MTDWRITE      8
#define RTK_NV_MTDERASE		 9
#define RTK_NV_GETENV		11
#define RTK_NV_SETENV		12

//#define MEM_LEAK_TEST

struct nvram_arg {
	int argc;
	char *argv[4];
};

struct cmd_type {
	char *cmd;
	int (*func)(int, char **);
};

static void usage() {
	char *cmd = "nv";
	printf("Usage: %s getenv [parameter_name]\n", cmd);
	printf("       %s setenv parameter_name value \n", cmd);	
#ifdef MEM_LEAK_TEST
	printf("       %s gettest test memory leak \n", cmd);	
#endif
	//exit(0);
}


#if 0 
static void mem_dump (void *start, unsigned int size, unsigned int virt_addr)
{
	int row, column, index, index2, max;
	unsigned char *buf, *line, ascii[17];

	if(!start ||(size==0))
		return;
	line = (unsigned char*)start;

	/*
	16 bytes per line
	*/
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printf("\n%08x ", (unsigned int) virt_addr);
		virt_addr += 16;
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printf("  ");
			printf("%02x ", (unsigned char) buf[index2]);
			ascii[index2] = isprint(buf[index2]) ? buf[index2] : '.';
		}

		if (max != 16)
		{
			if (max < 8)
				printf("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printf("   ");
		}

		//ASCII
		printf("  %s", ascii);
	}
} 

static int _erase(int argc, char **argv) {
	char *ptr;
	int num;
	if (1!=argc) {
		printf("Require 1 arg\n");
		return -1;
	}
	num = strtol(argv[0], &ptr, 0);
	if (*ptr != '\0') {
		printf("%s not a valid number\n", argv[0]);
		return -1;
	}
	printf("erase %d\n", num);
	return rtk_nv_erase(num);	
}

static int _testwrite(int argc, char **argv) {
	char *buffer, *ptr;
	int i, num;
	if (1!=argc) {
		printf("Require 1 arg\n");
		return -1;
	}
	num = strtol(argv[0], &ptr, 0);
	if (*ptr != '\0') {
		printf("%s not a valid number\n", argv[0]);
		return -1;
	}
	/* populate a buffer and write */
	buffer = malloc(0x10000);
	if (!buffer) {
		printf("No buffer\n");
		return -1;
	}
	for (i=0; i<0x10000; i++) {
		buffer[i] = i & 0xff;
	}
	return rtk_nv_write(num, 0, buffer, 0x10000);
}

#endif /* #if 0  */

static int _setenv(int argc, char **argv) {
	char *name = 0, *value = 0;
	int rv;

	switch (argc) {
	case 2:
		value = argv[1];
	case 1:
		name = argv[0];
		break;
	default:
		printf("Require 1 or 2 arg\n");
		return EINVAL;
	}

	rv = rtk_env_set(name, value);
	if(rv !=0) {
		printf("Error: rtk_env_set failed. rv=%d\n", rv);
		rv = EIO;
	}	

	return rv;
}

static int _getenv(int argc, char **argv) {
	char *name = 0, *buf;
	int buflen;
	int rv;

	switch (argc) {
	case 0:
		buflen = 0x10000;
		break;
	case 1:
		name = argv[0];
		buflen = 0x1000;
		break;
	default:
		printf("Require 0 or 1 arg\n");
		return EINVAL;
	}
	buf = malloc(buflen);
	if (!buf) {
		printf("No memory\n");
		return ENOMEM;
	}
	rv = rtk_env_get(name, buf, buflen);
	if(rv != 0) {
		printf("Error: rtk_env_get failed. rv=%d\n", rv);
		rv = EIO;
	}
	printf("%s\n", buf);
	free(buf);
	return rv;
}

#ifdef MEM_LEAK_TEST
static int _gettest(int argc, char **argv) {
	char *name = 0, *buf;
	int buflen;
	int rv = 0;

	buflen = 0x10000;
	buf = malloc(buflen);
	if (!buf) {
		printf("No memory\n");
		return ENOMEM;
	}

	while(1) {
		rv = rtk_env_get(name, buf, buflen);
		if(rv != 0) {
			printf("Error: rtk_env_get failed. rv=%d\n", rv);
			rv = EIO;
			break;
		}
		sleep(1);
		rv = rtk_env_set("abc", "123");
		if(rv != 0) {
			printf("Error: rtk_env_get failed. rv=%d\n", rv);
			rv = EIO;
			break;
		}

	}

	free(buf);
	return rv;
}
#endif

static struct cmd_type cmd[] = {
	//{ "erase", _erase   },
	//{ "write", _testwrite },
	{ "setenv", _setenv },
	{ "getenv", _getenv },
#ifdef MEM_LEAK_TEST
	{ "gettest", _gettest },
#endif
	{ 0, 0 }
};

int main(int argc, char *argv[])
{	
	struct cmd_type *p;
	int rv = EINVAL;
	
	if (argc < 2) {
		usage();
		goto done;
	}
	
	for (p = cmd; p->cmd; p++) {
		if (strcmp(p->cmd, argv[1]))
			continue;
		rv = p->func(argc - 2, &argv[2]);
		break;
	}
	
	if(!(p->cmd)) {
		usage();
	}
done:
	return rv;
}

/*****************************************************************************/

