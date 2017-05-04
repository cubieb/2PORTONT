/* ------------------------------------------------------------------------- */
/* tftp.c                                                                    */
/*                                                                           */
/* A simple tftp client for busybox.                                         */
/* Tries to follow RFC1350.                                                  */
/* Only "octet" mode supported.                                              */
/* Optional blocksize negotiation (RFC2347 + RFC2348)                        */
/*                                                                           */
/* Copyright (C) 2001 Magnus Damm <damm@opensource.se>                       */
/*                                                                           */
/* Parts of the code based on:                                               */
/*                                                                           */
/* atftp:  Copyright (C) 2000 Jean-Pierre Lefebvre <helix@step.polymtl.ca>   */
/*                        and Remi Lefebvre <remi@debian.org>                */
/*                                                                           */
/* utftp:  Copyright (C) 1999 Uwe Ohse <uwe@ohse.de>                         */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          */
/* General Public License for more details.                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
// Kaohj
#include <rtk/sysconfig.h>
#include <rtk/options.h>
#include <rtk/utility.h>

#include "busybox.h"

// Kaohj
#define	XFER_IMAGE	1
#define XFER_CONF	2
#define	IMG_FILE	"/tmp/tftpImg"
#define CNF_FILE	"/tmp/config.xml"

//#define BB_FEATURE_TFTP_GET
//#define BB_FEATURE_TFTP_PUT
//#define BB_FEATURE_TFTP_BLOCKSIZE
//#define BB_FEATURE_TFTP_DEBUG

#define TFTP_BLOCKSIZE_DEFAULT 512 /* according to RFC 1350, don't change */
#define TFTP_TIMEOUT 5             /* seconds */

/* opcodes we support */

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5
#define TFTP_OACK  6

#define UPD_IMAGE	0
#define UPD_CONF	1
#define UPD_NONE	5

static const char *tftp_error_msg[] = {
	"Undefined error",
	"File not found",
	"Access violation",
	"Disk full or allocation error",
	"Illegal TFTP operation",
	"Unknown transfer ID",
	"File already exists",
	"No such user"
};

const int tftp_cmd_get = 1;
const int tftp_cmd_put = 2;

// Kaohj
int xfertype = 0;
int xferok = 0;
int updateType;

#ifdef BB_FEATURE_TFTP_BLOCKSIZE

static int tftp_blocksize_check(int blocksize, int bufsize)  
{
        /* Check if the blocksize is valid: 
	 * RFC2348 says between 8 and 65464,
	 * but our implementation makes it impossible
	 * to use blocksizes smaller than 22 octets.
	 */

        if ((bufsize && (blocksize > bufsize)) || 
	    (blocksize < 8) || (blocksize > 65464)) {
	        error_msg("bad blocksize");
	        return 0;
	}

	return blocksize;
}

static char *tftp_option_get(char *buf, int len, char *option)  
{
        int opt_val = 0;
	int opt_found = 0;
	int k;
  
	while (len > 0) {

	        /* Make sure the options are terminated correctly */

	        for (k = 0; k < len; k++) {
		        if (buf[k] == '\0') {
			        break;
			}
		}

		if (k >= len) {
		        break;
		}

		if (opt_val == 0) {
			if (strcasecmp(buf, option) == 0) {
			        opt_found = 1;
			}
		}      
		else {
		        if (opt_found) {
				return buf;
			}
		}
    
		k++;
		
		buf += k;
		len -= k;
		
		opt_val ^= 1;
	}
	
	return NULL;
}

#endif

// Kaohj
static void tftpUpdateImage(const char *fname)
{
	FILE *fp;
	struct stat st;
	
	printf("Updating image ...\n");
	fflush(0);
#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	if (cmd_check_image(fname, 0)) {
#endif
		if ((fp = fopen(fname, "rb")) == NULL) {
			printf("File %s open fail\n", fname);
			return;
		}

		if (fstat(fileno(fp), &st) < 0) {
			printf("File %s get status fail\n", fname);
			fclose(fp);
			return;	
		}		
		
		if (st.st_size <= 0) {
			printf("File %s size error\n", fname);
			fclose(fp);
			return;
		}
		fclose(fp);		
		cmd_upload(fname, 0, st.st_size);
#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	}
#endif
}

static void tftpUpdateConf(const char *fname)
{
	int ret;
	
	printf("Updating configuration ...\n");
	fflush(0);
	cmd_file2xml(fname, CNF_FILE);
	//ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
	ret = call_cmd("/bin/loadconfig", 0, 1);
	if (ret == 0) { // load ok
		/* upgdate to flash */
		printf("Writing ...\n");
		va_cmd("/bin/adslctrl",1,1,"disablemodemline");
		sleep(1);	
		sync();
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	else { // load fail
		printf("Parsing error!\n");
	}
	va_cmd("/bin/sarctl",1,1,"disable");
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	/* reboot the system */
	printf("Rebooting ...\n");
	cmd_reboot();
}

// Kaohj -- verify file header for configuration file
static int isConfile(char *buf)
{
	int i, j, hlen;
	char cnfHdr[128];
	
	j = 0;
	hlen = strlen(CONFIG_HEADER);
	if (hlen > 128)
		hlen = 128;
	for (i=0; i<hlen; i++) {
#ifdef XOR_ENCRYPT
		cnfHdr[i] = buf[i]^XOR_KEY[j++];
		if (XOR_KEY[j] == '\0')
			j = 0;
#else
		cnfHdr[i] = buf[i];
#endif
	}
	cnfHdr[hlen] = 0;
	if (strcmp(cnfHdr, CONFIG_HEADER))
		return 0;
	return 1;
}

static inline int tftp(const int cmd, const struct hostent *host,
	const char *remotefile, int localfd, const int port, int tftp_bufsize)
{
	const int cmd_get = cmd & tftp_cmd_get;
	const int cmd_put = cmd & tftp_cmd_put;
	const int bb_tftp_num_retries = 5;

	struct sockaddr_in sa;
	struct sockaddr_in from;
	struct timeval tv;
	socklen_t fromlen;
	fd_set rfds;
	char *cp;
	unsigned short tmp;
	int socketfd;
	int len;
	int opcode = 0;
	int finished = 0;
	int timeout = bb_tftp_num_retries;
	int block_nr = 1;
	// Kaohj
	int checked = 0;

#ifdef BB_FEATURE_TFTP_BLOCKSIZE
	int want_option_ack = 0;
#endif

	/* Can't use RESERVE_BB_BUFFER here since the allocation
	 * size varies meaning BUFFERS_GO_ON_STACK would fail */
	char *buf=xmalloc(tftp_bufsize + 4);

	tftp_bufsize += 4;

	if ((socketfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror_msg("socket");
		return EXIT_FAILURE;
	}

	len = sizeof(sa);

	memset(&sa, 0, len);
	bind(socketfd, (struct sockaddr *)&sa, len);

	sa.sin_family = host->h_addrtype;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr, (struct in_addr *) host->h_addr,
		   sizeof(sa.sin_addr));

	/* build opcode */

	if (cmd_get) {
		opcode = TFTP_RRQ;
	}

	if (cmd_put) {
		opcode = TFTP_WRQ;
	}

	while (1) {

		cp = buf;

		/* first create the opcode part */

		*((unsigned short *) cp) = htons(opcode);

		cp += 2;

		/* add filename and mode */

		if ((cmd_get && (opcode == TFTP_RRQ)) ||
			(cmd_put && (opcode == TFTP_WRQ))) {
                        int too_long = 0; 

			/* see if the filename fits into buf */
			/* and fill in packet                */

			len = strlen(remotefile) + 1;

			if ((cp + len) >= &buf[tftp_bufsize - 1]) {
			        too_long = 1;
			}
			else {
			        safe_strncpy(cp, remotefile, len);
				cp += len;
			}

			if (too_long || ((&buf[tftp_bufsize - 1] - cp) < 6)) {
				error_msg("too long remote-filename");
				break;
			}

			/* add "mode" part of the package */

			memcpy(cp, "octet", 6);
			cp += 6;

#ifdef BB_FEATURE_TFTP_BLOCKSIZE

			len = tftp_bufsize - 4; /* data block size */

			if (len != TFTP_BLOCKSIZE_DEFAULT) {

			        if ((&buf[tftp_bufsize - 1] - cp) < 15) {
				        error_msg("too long remote-filename");
					break;
				}

				/* add "blksize" + number of blocks  */

				memcpy(cp, "blksize", 8);
				cp += 8;

				cp += snprintf(cp, 6, "%d", len) + 1;

				want_option_ack = 1;
			}
#endif
		}

		/* add ack and data */

		if ((cmd_get && (opcode == TFTP_ACK)) ||
			(cmd_put && (opcode == TFTP_DATA))) {

			*((unsigned short *) cp) = htons(block_nr);

			cp += 2;

			block_nr++;

			if (cmd_put && (opcode == TFTP_DATA)) {
				len = read(localfd, cp, tftp_bufsize - 4);

				if (len < 0) {
					perror_msg("read");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
					// Kaohj -- put ok
					xferok = 1;
				}

				cp += len;
			} else if (finished) {
				// Kaohj -- get ok
				xferok = 1;
				break;
			}
		}


		/* send packet */


		do {

			len = cp - buf;

#ifdef BB_FEATURE_TFTP_DEBUG
			printf("sending %u bytes\n", len);
			for (cp = buf; cp < &buf[len]; cp++)
				printf("%02x ", *cp);
			printf("\n");
#endif
			if (sendto(socketfd, buf, len, 0,
					(struct sockaddr *) &sa, sizeof(sa)) < 0) {
				perror_msg("send");
				len = -1;
				break;
			}


			/* receive packet */


			memset(&from, 0, sizeof(from));
			fromlen = sizeof(from);

			tv.tv_sec = TFTP_TIMEOUT;
			tv.tv_usec = 0;

			FD_ZERO(&rfds);
			FD_SET(socketfd, &rfds);

			switch (select(FD_SETSIZE, &rfds, NULL, NULL, &tv)) {
			case 1:
				len = recvfrom(socketfd, buf, tftp_bufsize, 0,
						(struct sockaddr *) &from, &fromlen);

				if (len < 0) {
					perror_msg("recvfrom");
					break;
				}

				timeout = 0;

				if (sa.sin_port == htons(port)) {
					sa.sin_port = from.sin_port;
				}
				if (sa.sin_port == from.sin_port) {
					break;
				}

				/* fall-through for bad packets! */
				/* discard the packet - treat as timeout */
				timeout = bb_tftp_num_retries;

			case 0:
				error_msg("timeout");

				if (timeout == 0) {
					len = -1;
					error_msg("last timeout");
				} else {
					timeout--;
				}
				break;

			default:
				perror_msg("select");
				len = -1;
			}

		} while (timeout && (len >= 0));

		if (len < 0) {
			break;
		}

		/* process received packet */


		opcode = ntohs(*((unsigned short *) buf));
		tmp = ntohs(*((unsigned short *) &buf[2]));

#ifdef BB_FEATURE_TFTP_DEBUG
		printf("received %d bytes: %04x %04x\n", len, opcode, tmp);
#endif

		if (opcode == TFTP_ERROR) {
			char *msg = NULL;

			if (buf[4] != '\0') {
				msg = &buf[4];
				buf[tftp_bufsize - 1] = '\0';
			} else if (tmp < (sizeof(tftp_error_msg) 
					  / sizeof(char *))) {

				msg = (char *) tftp_error_msg[tmp];
			}

			if (msg) {
				error_msg("server says: %s", msg);
			}

			break;
		}

#ifdef BB_FEATURE_TFTP_BLOCKSIZE
		if (want_option_ack) {

			 want_option_ack = 0;

		         if (opcode == TFTP_OACK) {

			         /* server seems to support options */

			         char *res;

				 res = tftp_option_get(&buf[2], len-2, 
						       "blksize");

				 if (res) {
				         int foo = atoi(res);
			     
					 if (tftp_blocksize_check(foo,
							   tftp_bufsize - 4)) {

					         if (cmd_put) {
				                         opcode = TFTP_DATA;
						 }
						 else {
				                         opcode = TFTP_ACK;
						 }
#ifdef BB_FEATURE_TFTP_DEBUG
						 printf("using blksize %u\n");
#endif
					         tftp_bufsize = foo + 4;
						 block_nr = 0;
						 continue;
					 }
				 }
				 /* FIXME:
				  * we should send ERROR 8 */
				 error_msg("bad server option");
				 break;
			 }

			 error_msg("warning: blksize not supported by server"
				   " - reverting to 512");

			 tftp_bufsize = TFTP_BLOCKSIZE_DEFAULT + 4;
		}
#endif

		if (cmd_get && (opcode == TFTP_DATA)) {

			if (tmp == block_nr) {
			    
			    	// Kaohj
				IMGHDR *phdr;
				
				if (!checked) {
					checked = 1;
					phdr = (IMGHDR *)&buf[4];
					updateType = UPD_NONE;
					if (xfertype == XFER_CONF && isConfile(&buf[4]))
						updateType = UPD_CONF;
					if (xfertype == XFER_IMAGE && phdr->key == APPLICATION_IMAGE) {
						// Telnet_CLI will use tftp, not kill
						cmd_killproc(ALL_PID & ~((1<<PID_CLI)|(1<<PID_TELNETD)));
						updateType = UPD_IMAGE;
					}
					if (updateType == UPD_NONE) {
						error_msg("Invalid File !");
						break;
					}
				}
				len = write(localfd, &buf[4], len - 4);

				if (len < 0) {
					perror_msg("write");
					break;
				}

				if (len != (tftp_bufsize - 4)) {
					finished++;
				}

				opcode = TFTP_ACK;
				continue;
			}
		}

		if (cmd_put && (opcode == TFTP_ACK)) {

			if (tmp == (block_nr - 1)) {
				if (finished) {
					break;
				}

				opcode = TFTP_DATA;
				continue;
			}
		}
	}

#ifdef BB_FEATURE_CLEAN_UP
	close(socketfd);

        RELEASE_BB_BUFFER(buf);
#endif

	return finished ? EXIT_SUCCESS : EXIT_FAILURE;
}

int tftp_main(int argc, char **argv)
{
	struct hostent *host = NULL;
	char *localfile = NULL;
	char *remotefile = NULL;
	int port = 69;
	int cmd = 0;
	int fd = -1;
	int flags = 0;
	int opt;
	int result;
	int blocksize = TFTP_BLOCKSIZE_DEFAULT;

	/* figure out what to pass to getopt */

#ifdef BB_FEATURE_TFTP_BLOCKSIZE
#define BS "b:"
#else
#define BS
#endif

#ifdef BB_FEATURE_TFTP_GET
#define GET "g"
#else
#define GET 
#endif

#ifdef BB_FEATURE_TFTP_PUT
#define PUT "p"
#else
#define PUT 
#endif
// Kaohj
#define IMG "i"
#define CONF "c"

	// Kaohj -- tftp update image and configuration or backup configuration
	while ((opt = getopt(argc, argv, GET PUT IMG CONF "f:")) != -1) {
	//while ((opt = getopt(argc, argv, BS GET PUT "l:r:")) != -1) {
		switch (opt) {
// Kaohj
#if 0
#ifdef BB_FEATURE_TFTP_BLOCKSIZE
		case 'b':
			blocksize = atoi(optarg);
			if (!tftp_blocksize_check(blocksize, 0)) {
                                return EXIT_FAILURE;
			}
			break;
#endif
#endif
#ifdef BB_FEATURE_TFTP_GET
		case 'g':
			cmd = tftp_cmd_get;
			flags = O_WRONLY | O_CREAT;
			break;
#endif
#ifdef BB_FEATURE_TFTP_PUT
		case 'p':
			cmd = tftp_cmd_put;
			flags = O_RDONLY;
			break;
#endif
// Kaohj
#if 0
		case 'l': 
			localfile = xstrdup(optarg);
			break;
		case 'r':
			remotefile = xstrdup(optarg);
			break;
// Kaohj
#endif
		case 'i':
			xfertype = XFER_IMAGE;
			break;
		case 'c':
			xfertype = XFER_CONF;
			break;
		case 'f':
			remotefile = xstrdup(optarg);
			break;
		}
	}

	if ((cmd == 0) || (optind == argc)) {
		show_usage();
	}
	// Kaohj --- remote file and transfer type must be set
	if (remotefile == NULL || xfertype == 0) {
		show_usage();
	}
	if(localfile && strcmp(localfile, "-") == 0) {
	    fd = fileno((cmd==tftp_cmd_get)? stdout : stdin);
	}
	if(localfile == NULL)
	    localfile = remotefile;
	if(remotefile == NULL)
	    remotefile = localfile;
	// Kaohj
	if (xfertype == XFER_IMAGE)
		localfile = IMG_FILE;
	else
		localfile = CNF_FILE;
	// Kaohj
	if (cmd == tftp_cmd_put && xfertype == XFER_CONF) { // generate xml file
		//if (call_cmd("/bin/CreatexmlConfig", 0, 1))
		if (call_cmd("/bin/saveconfig", 0, 1))
			return EXIT_FAILURE;
		cmd_xml2file(CNF_FILE, CNF_FILE);
	}
	printf("Transferring ...\n");
	//printf("cmd:%d, local:%s, remote:%s, type=%d\n", cmd, localfile, remotefile, xfertype);
	if (fd==-1) {
	    fd = open(localfile, flags, 0644);
	}
	if (fd < 0) {
		perror_msg_and_die("local file");
	}

	host = xgethostbyname(argv[optind]);

	if (optind + 2 == argc) {
		port = atoi(argv[optind + 1]);
	}

#ifdef BB_FEATURE_TFTP_DEBUG
	printf("using server \"%s\", remotefile \"%s\", "
		"localfile \"%s\".\n",
		inet_ntoa(*((struct in_addr *) host->h_addr)),
		remotefile, localfile);
#endif

	result = tftp(cmd, host, remotefile, fd, port, blocksize);
#ifdef BB_FEATURE_CLEAN_UP
	if (!(fd == fileno(stdout) || fd == fileno(stdin))) {
	    close(fd);
	}
#endif
	// Kaohj -- remove the local file
	if (xferok == 1)
		printf("Ok!\n");
	if (cmd == tftp_cmd_get && xferok) { // updating
		if (xfertype == XFER_IMAGE) {
			if (updateType == UPD_IMAGE)
				tftpUpdateImage(IMG_FILE);
			else
				printf("Invalid File !\n");
		}
		else if (xfertype == XFER_CONF) {
			if (updateType == UPD_CONF)
				tftpUpdateConf(CNF_FILE);
			else
				printf("Invalid File !\n");
		}
	}
	//unlink(localfile);

	return(result);
}
