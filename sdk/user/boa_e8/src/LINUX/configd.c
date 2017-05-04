/*
 * configd.c --- main file for configuration server
 * --- By Kaohj
 */
#include <semaphore.h>
#include "../msgq.h"
#include "mib.h"	// for FLASH_BLOCK_SIZE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/shm.h>
#include "../syslog.h"
// Added by Mason Yu
#include "utility.h"
#ifdef CONFIG_CPU_BIG_ENDIAN
#include <linux/byteorder/big_endian.h>
#else
#include <linux/byteorder/little_endian.h>
#endif
#include <linux/magic.h>
#include <linux/cramfs_fs.h>

//ccwei
#if defined(CONFIG_MTD_NAND) || defined(CONFIG_RTL8686)
/* for open(), lseek() */
#include <mtd/mtd-user.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
//end ccwei
#include <pthread.h>
#include <arpa/inet.h>

#define _SET_MSGQ_SIZE_AND_NONBLOCK_
#ifdef _SET_MSGQ_SIZE_AND_NONBLOCK_
#define MSGMNB 		16384   /* default max size of a message queue in linux kernel*/
#define MAX_MSGQ_MSGNUM	8
#define MAX_MSGQ_SIZE 	((sizeof(struct mymsgbuf)*MAX_MSGQ_MSGNUM)<MSGMNB?MSGMNB:(sizeof(struct mymsgbuf)*MAX_MSGQ_MSGNUM))
#endif //_SET_MSGQ_SIZE_AND_NONBLOCK_

#define CONFIGD_RUNFILE	"/var/run/configd.pid"
int msgqueue_id;
int shm_id; // shared memory ID
char *shm_start; // attaching address
int this_pid;

#ifdef CONFIG_USER_DDNS
extern int g_killprocess;

#define AUTO_STARTDDNS_INTERVAL	60*60*24	/* start auto-startDDNSC every 24 hours */
//#define AUTO_STARTDDNS_INTERVAL	5	/* start auto-startDDNSC every 24 hours */
#endif

// Mason Yu On True
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef SEND_LOG
sem_t semSyslogC;
#define AUTO_STARTSYSLOGC_INTERVAL 60*60*24	/* start auto_startSYSLOGC every 24 hours */
#endif
#endif

char g_upload_post_file_name[MAX_SEND_SIZE];
int g_upload_startPos;
int g_upload_fileSize;

#ifdef CONFIG_USER_DDNS
void *ddnsC(struct ddns_info *tinfo)
{
	unsigned int entryNum, i;
	char account[70];
	MIB_CE_DDNS_Tp pEntry;
	struct in_addr inAddr;    // Mason Yu. Specify IP Address
	char *itfIP;

#ifdef CONFIG_IPV6
	struct ipv6_ifaddr ip6_addr[6];
	unsigned char itfIPv6[64];
#endif

	entryNum = _mib_chain_total(MIB_DDNS_TBL);

	for (i=0; i<entryNum; i++) {
		char getV4IP=0;
#ifdef CONFIG_IPV6
		char count=0, getV6IP=0;
		int num=0;
#endif

		if (!(pEntry=(MIB_CE_DDNS_T *)_mib_chain_get(MIB_DDNS_TBL, i)))
		{
			printf("ddnsC:Get chain record error!\n");
			continue;
		}

		// Check all variables that updatedd need
		if ( strlen(pEntry->username) == 0 ) {
			printf("ddnsC: username/email is NULL!!\n");
			continue;
		}

		if ( strlen(pEntry->password) == 0 ) {
			printf("ddnsC: password/key is NULL!!\n");
			continue;
		}

		if ( strlen(pEntry->hostname) == 0 ) {
			printf("ddnsC: Hostname is NULL!!\n");
			continue;
		}

		if ( strlen(pEntry->interface) == 0 ) {
			printf("ddnsC: Interface is NULL!!\n");
			continue;
		}

		if ( pEntry->Enabled != 1 ) {
			printf("ddnsC: The account is disabled!!\n");
			continue;
		}

		if ( strcmp(pEntry->provider, "dyndns") == 0 || strcmp(pEntry->provider, "tzo") == 0
			|| strcmp(pEntry->provider, "oray") == 0) {
			sprintf(account, "%s:%s", pEntry->username, pEntry->password);
		}else {
			//sprintf(account, "%s:%s", Entry.email, Entry.key);
			printf("ddnsC: Not support this provider\n");
			syslog(LOG_INFO, "ddnsC: Not support this provider %s\n", pEntry->provider);
			continue;
		}

		if (strcmp("Any", pEntry->interface) == 0) {
			if ( g_killprocess == KILL_PROCESS_OFF ) {
				// updatedd dyndns yufc:663333 mason2.dyndns.info
				va_cmd("/bin/updatedd", 3, 0, pEntry->provider, account, pEntry->hostname);
			}
		}
		else { // Specify IP Address
			if ( (strcmp(tinfo->ifname, pEntry->interface) != 0) && (strcmp(tinfo->ifname, "all") != 0) ) {
				printf("ddnsC: This rule is for %s not for %s rule!!\n", pEntry->interface, tinfo->ifname);
			continue;
		}

		// For IPv4
			if ( tinfo->ipversion & IPVER_IPV4 ) {
				if (getInAddr(pEntry->interface, IP_ADDR, (void *)&inAddr) == 1) {
				itfIP = inet_ntoa(inAddr);
				getV4IP = 1;
					printf("ddnsC: The %s IPv4 address is %s\n", pEntry->interface, itfIP);
			}
			else {
					printf("ddnsC: Can not get IPv4 address for %s interface\n", pEntry->interface);
				//continue;
			}
		}

#ifdef CONFIG_IPV6
		// For IPv6
			// only support IPv6 DDNS on dual stack(must go ipv4 connection).		
			if (tinfo->ipversion == IPVER_IPV4_IPV6 ) {
			// for IPv6 PPP dynamically
			if (strncmp(tinfo->ifname, "ppp", 3) == 0) {
					while ((getifip6(pEntry->interface, IPV6_ADDR_UNICAST, ip6_addr, 6) ==0) && (num < 5))
				{
						printf("ddnsC: Can not get IPv6 PPP address for %s interface\n", pEntry->interface);
					sleep(3);
					num++;
				}
				if ( num != 5) {
					inet_ntop(PF_INET6, &ip6_addr[0].addr, itfIPv6, 256);
					getV6IP = 1;
						printf("ddnsC: The %s IPv6 address is %s\n", pEntry->interface, itfIPv6);
				}
			}
			// For static link or  all interface update
			else
			{
					count=getifip6(pEntry->interface, IPV6_ADDR_UNICAST, ip6_addr, 6);
				if (count) {
						inet_ntop(PF_INET6, &ip6_addr[0].addr, itfIPv6, 256);
						getV6IP = 1;
							printf("ddnsC: The %s IPv6 address is %s\n", pEntry->interface, itfIPv6);
				}
				else {
						printf("ddnsC: Can not get IPv6 address for %s interface\n", pEntry->interface);
					//continue;
				}
			}
		}
#endif

		if ( g_killprocess == KILL_PROCESS_OFF ) {
#ifdef CONFIG_IPV6
			if (getV4IP || getV6IP) {
					printf("ddnsC: Start updatedd with %s interface\n", pEntry->interface);
					syslog(LOG_INFO, "ddnsC: %s Start updatedd - provider is %s\n", tinfo->ifname, pEntry->provider);
			}
			if (getV4IP && getV6IP)
				// updatedd dyndns -- -4 61.11.22.33 -6 2001:b010:7030:1a00:90:1a00:a2:8506 yufc:663333 mason2.dyndns.info
					va_cmd("/bin/updatedd", 8, 0, pEntry->provider, "--", "-4", itfIP, "-6", itfIPv6, account, pEntry->hostname);
			else if (getV4IP)
				// updatedd dyndns -- -4 61.11.22.33 yufc:663333 mason2.dyndns.info
					va_cmd("/bin/updatedd", 6, 0, pEntry->provider, "--", "-4", itfIP, account, pEntry->hostname);
#else
				if (getV4IP) {
					printf("ddnsC: Start updatedd with %s interface\n", pEntry->interface);
					syslog(LOG_INFO, "ddnsC: %s Start updatedd - provider is %s\n", tinfo->ifname, pEntry->provider);
				}
			if (getV4IP)
				// updatedd dyndns -- -4 61.11.22.33 yufc:663333 mason2.dyndns.info
					va_cmd("/bin/updatedd", 6, 0, pEntry->provider, "--", "-4", itfIP, account, pEntry->hostname);
#endif
		}
	}
	}
}

void startDdnsc(struct ddns_info tinfo)
{
#ifdef CONFIG_E8B
	unsigned char ddnsEnable;
#endif

#ifdef CONFIG_E8B
	_mib_get(MIB_DDNS_ENABLE, &ddnsEnable);
	if (!ddnsEnable)
		return;
#endif
	//printf("startDdnsc: tinfo.ifname is %s. tinfo.ipversion is %d.\n", tinfo.ifname, tinfo.ipversion);
	ddnsC(&tinfo);
}

static void auto_startDDNSC(int signo)
{
	// Mason Yu.  create DDNS thread dynamically
	struct ddns_info tinfo;
	strcpy(tinfo.ifname, "all");
#ifdef CONFIG_IPV6
	tinfo.ipversion= 3;
#else
	tinfo.ipversion= 1;
#endif
	startDdnsc(tinfo);

	signal(SIGALRM, auto_startDDNSC);
	alarm(AUTO_STARTDDNS_INTERVAL);
}
#endif

static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CONFIGD_RUNFILE;

	pid = getpid();
	this_pid = pid;

	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

void sigterm_handler(int signal)
{
	remove_queue(msgqueue_id);
}

void init_signals(void)
{
	struct sigaction sa;

	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGSEGV);
	sigaddset(&sa.sa_mask, SIGBUS);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGPIPE);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigaddset(&sa.sa_mask, SIGUSR1);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &sa, NULL);

	// Mason Yu.  create DDNS thread dynamically
#ifdef CONFIG_USER_DDNS
	/* Establish a handler for SIGALRM signals. */
	signal(SIGALRM, auto_startDDNSC);
	alarm(AUTO_STARTDDNS_INTERVAL);
#endif

}

static int initMsgQ()
{
	key_t key;
	int   queue_id;

	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');

	if ((queue_id = open_queue(key, MQ_CREATE)) == -1) {
		perror("open_queue");
		return -1;
	}

#ifdef _SET_MSGQ_SIZE_AND_NONBLOCK_
	set_msgqueue_max_size( queue_id, MAX_MSGQ_SIZE );
#endif //_SET_MSGQ_SIZE_AND_NONBLOCK_

	//printf("queue_id=%d\n", queue_id);
	return queue_id;
}

/*
 *	Init shared memory segment; attach the shared memory segment to address space.
 *	-1: error
 *	 0: successful
 */
static int initShm()
{
	if ((shm_id = shmget((key_t)1234, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
		perror("shmget");
		return -1;
	}
	if ((shm_start = (char *)shmat ( shm_id , NULL , 0 ) )==(char *)(-1)) {
		perror("shmat");
		return -1;
	}
	//printf("initShm: shm_id=%d, shm_start=0x%x\n", shm_id, shm_start);
	return 0;
}

#include <sys/syscall.h>
inline int checkPidExist(int tid, int tgid)
{
	int ret;
	ret = syscall(SYS_tgkill, tgid, tid, 0);

//	printf("check pid %d, ret=%d\n",tid, ret);
	if((ret == -1) && (errno == ESRCH))
	{
		printf("tid %d not existed!\n",tid);
		return 0;
	}
	else
		return 1;
}

void msg_handler() {
	struct mymsgbuf qbuf;
	int ret;
	int isPidExisted;

	while (1) {
		ret = read_message(msgqueue_id, &qbuf, MSG_CONFIGD_PID);

		if (ret>0) {
			//printf("process message...\n");
			msgProcess(&qbuf);
			isPidExisted = checkPidExist(qbuf.mtype, qbuf.tgid);
			if (isPidExisted) { // Send the message only if the receiving process alive.
#ifdef _SET_MSGQ_SIZE_AND_NONBLOCK_
				send_message_nonblock(msgqueue_id, qbuf.mtype, qbuf.request, 0, &qbuf.msg);
#else
				send_message(msgqueue_id, qbuf.mtype, qbuf.request, 0, &qbuf.msg);
#endif //_SET_MSGQ_SIZE_AND_NONBLOCK_
			}
		}
	}
}

#if defined(CONFIG_RTL8686) || defined(CONFIG_MTD_NAND)
/*return value 0 --> success, -1 --> fail*/
static int get_mtd_meminfo(struct mtd_info_user *mtd_info,
			   const char *part_name)
{
	int fd, ret = 0;

	fd = get_mtd_fd(part_name);
	if (fd >= 0) {
		ret = ioctl(fd, MEMGETINFO, mtd_info);
		if (ret < 0) {
			fprintf(stderr, "error MEMGETINFO! ret %d\n", ret);
			goto fail;
		}
		printf("%s(%d): A block size = %d, A mtd size %d\n",
		       __FUNCTION__, __LINE__, mtd_info->erasesize,
		       mtd_info->size);
	} else {
		fprintf(stderr, "%s(%d):get_mtd_fd fails fd %d\n", __FUNCTION__,
			__LINE__, fd);
		ret = -1;
		goto fail;
	}

fail:
	if (fd >= 0)
		close(fd);

	return ret;
}

static int flash_block_erase(struct erase_info_user *erase_info,
			     char *part_name)
{
	int fd, ret = 0;

	fd = get_mtd_fd(part_name);
	if (fd >= 0) {
		ret = ioctl(fd, MEMERASE, erase_info);
		if (ret < 0) {
			fprintf(stderr, "error MEMERASE %d ret %d!\n",
				erase_info->start, ret);
			goto fail;
		}
	} else {
		fprintf(stderr, "%s(%d):get_mtd_fd fails fd %d\n", __FUNCTION__,
			__LINE__, fd);
		ret = -1;
		goto fail;
	}

fail:
	if (fd >= 0)
		close(fd);

	return ret;
}
#endif

#ifdef CONFIG_RTL8686
int flash_filewrite(FILE * fp, int size, const char *part_name)
{
	int block_count;
	int image_size;
	int nWritten = 0, nRead, ret = 0;
	struct erase_info_user erase_info;
	struct mtd_info_user mtd_info;
	unsigned int flash_pos_count = 0;
	void *block = NULL;

	if ((ret = get_mtd_meminfo(&mtd_info, part_name)) < 0)
		goto fail;

	block_count = (size + (mtd_info.erasesize - 1)) / mtd_info.erasesize;
	image_size = block_count * mtd_info.erasesize;
	printf("%s-(%d):: block_count:%d image_size:%d\n", __FUNCTION__,
	       __LINE__, block_count, image_size);

	block = malloc(mtd_info.erasesize);
	if (!block) {
		fprintf(stderr, "%s-%d::malloc mem fail!\n", __FUNCTION__,
			__LINE__);
		ret = -1;
		goto fail;
	}

	while (nWritten < size) {
		nRead = (mtd_info.erasesize >
			 (size - nWritten)) ? (size -
					       nWritten) : mtd_info.erasesize;
		nRead = fread(block, 1, nRead, fp);
		if (nRead < mtd_info.erasesize)
			nRead = mtd_info.erasesize;
		erase_info.start = flash_pos_count;
		erase_info.length = nRead;
		if (erase_info.start >= mtd_info.size) {
			fprintf
			    (stderr,
			     "Error: you can't erase the region exceed mtd partition size\n");
			ret = -1;
			goto fail;
		}

		if ((ret = flash_block_erase(&erase_info, (char *)part_name)) < 0)
			goto fail;

		ret = __mib_flash_part_write(block,
					     erase_info.start, nRead,
					     part_name);
		if (ret < 0) {	//write data fail!
			fprintf(stderr, "%s(%d): flash_write fail!\n",
				__FUNCTION__, __LINE__);
			goto fail;
		}

		flash_pos_count += mtd_info.erasesize;
		nWritten += nRead;
	}

fail:
	free(block);

	return ret;
}

//end 120313
#endif /*CONFIG_RTL8686 */

#ifdef CONFIG_MTD_NAND
int flashdrv_filewrite(FILE * fp, int size, void *dstP)
{
	FILE *fp_http;
	int block_count;
	int fd = -1, http_upgrade = 0, fp_pos = 0;
	int nWritten = 0, nRead, ret = 0;
	struct erase_info_user erase_info;
	struct mtd_info_user mtd_info;
	unsigned int backup = 0, flash_pos_count = 0;
	void *block = NULL;

	fp_http = fopen("/var/http_upgrade", "r");
	if (fp_http) {
		fscanf(fp_http, "%d", &http_upgrade);
		fclose(fp_http);
	}
	printf("http_upgrade %d\n", http_upgrade);

	if (!http_upgrade) {
		ret = -1;
		goto fail;
	}

	if ((ret = get_mtd_meminfo(&mtd_info, "rootfs")) < 0)
		goto fail;

	block_count = (size + (mtd_info.erasesize - 1)) / mtd_info.erasesize;
	size = block_count * mtd_info.erasesize;

	block = malloc(mtd_info.erasesize);
	if (!block) {
		fprintf(stderr, "%s-%d::malloc mem fail!\n", __FUNCTION__,
			__LINE__);
		ret = -1;
		goto fail;
	}

	fp_pos = ftell(fp);
	printf("flashWrite --> total size %d fp_pos %d\n", size, fp_pos);
	if (fp_pos < 0) {
		fprintf(stderr, "error ftell(fp) %d\n", fp_pos);
		ret = -1;
		goto fail;
	}

UPGRADE_IMGB:
	if (backup) {
		/*note: start_addr + offset: can't exceed
		   more than mtd_info.size (mtd partition size) */

		system("echo 2 > /var/http_upgrade");

		if ((ret = get_mtd_meminfo(&mtd_info, "rootfs2")) < 0)
			goto fail;

		fseek(fp, fp_pos, SEEK_SET);	//seek data ptr to ori position.
		flash_pos_count = 0;	//reset flash position count!
		nWritten = 0;	//reset buffer data count!
	}

	while (nWritten < size) {
		nRead = (mtd_info.erasesize >
			 (size - nWritten)) ? (size -
					       nWritten) : mtd_info.erasesize;
		nRead = fread(block, 1, nRead, fp);

		//ccwei: write one block size into nand flash one time!
		if (nRead < mtd_info.erasesize)
			nRead = mtd_info.erasesize;

		fd = get_mtd_fd(backup ? "rootfs2" : "rootfs");
		if (fd < 0) {
			fprintf(stderr, "%s(%d):get_mtd_fd fails fd %d\n", __FUNCTION__,
					__LINE__, fd);
			ret = -1;
			goto fail;
		}

ERASE_NEXT_BLOCK:
		erase_info.start = flash_pos_count;
		erase_info.length = nRead;
		flash_pos_count += nRead;	//calculate next block addr.
		if (erase_info.start >= mtd_info.size) {
			fprintf
			    (stderr,
			     "Error: you can't erase the region exceed mtd partition size\n");
			ret = -1;
			goto fail;
		}
		ret = ioctl(fd, MEMERASE, &erase_info);
		if (ret < 0) {
			fprintf(stderr, "error MEMERASE %d ret %d!\n",
				erase_info.start, ret);
			goto ERASE_NEXT_BLOCK;
		}

		ret =
		    __mib_nand_flash_write(block, erase_info.start, nRead, fd);
		if (ret < 0) {	//write data fail!
			fprintf(stderr,
				"%s(%d): flash_write fail! addr %d\n",
				__FUNCTION__, __LINE__, erase_info.start);
			goto ERASE_NEXT_BLOCK;
		}

		close(fd);
		fd = -1;

		nWritten += nRead;
//ccwei
		//printf("flashWrite --> nWritten %d\n", nWritten);
	}

	if (backup == 0) {
		backup = 1;
		printf("goto upgrade img B! backup %d, block_count %d\n",
		       backup, block_count);
		goto UPGRADE_IMGB;
	}

fail:
	if (fd >= 0)
		close(fd);

	free(block);

	return ret;
}
#elif !defined(CONFIG_BLK_DEV_INITRD)
int flashdrv_filewrite(FILE * fp, int size, void *dstP)
{
	int nWritten = 0, nRead, ret = 0;
	uint16_t rData16;
	uint32_t rData32;
	void *vmlinuxAddr, *block = NULL;

	vmlinuxAddr = dstP;

	/* Read the magic number */
	flash_read(&rData32, dstP, sizeof(rData32));
	__le32_to_cpus(&rData32);

	/* Skip rootfs */
	if (CRAMFS_MAGIC == rData32) {
		/* Read size */
		flash_read(&rData32, dstP + 4, sizeof(rData32));
		__le32_to_cpus(&rData32);

		/* 16Byte alignment */
		vmlinuxAddr += (rData32 + 0x000FUL) & ~0x000FUL;
	} else if (SQUASHFS_MAGIC == rData32) {
		/* Read s_major */
		flash_read(&rData16, dstP + 28, sizeof(rData16));
		__le16_to_cpus(&rData16);

		/* s_major >= 4 */
		if (rData16 >= 4) {
			/* Read bytes_used */
			flash_read(&rData32, dstP + 40, sizeof(rData32));
			__le32_to_cpus(&rData32);
		} else {
			/* Read bytes_used */
			flash_read(&rData32, dstP + 8, sizeof(rData32));
			__le32_to_cpus(&rData32);
		}

		/* 64KB alignment */
		vmlinuxAddr += (rData32 + 0x0FFFFUL) & ~0x0FFFFUL;
	}

	block = malloc(FLASH_BLOCK_SIZE);
	if (!block) {
		ret = -1;
		goto fail;
	}
	// Kaohj -- destroying image
	memset(block, 1, FLASH_BLOCK_SIZE);
	flash_write(block, vmlinuxAddr, FLASH_BLOCK_SIZE);

	while (nWritten < size) {
		nRead = (FLASH_BLOCK_SIZE >
			 (size - nWritten)) ? (size -
					       nWritten) : FLASH_BLOCK_SIZE;
		nRead = fread(block, 1, nRead, fp);

		printf("flashWrite --> 0x%08x (len 0x%x)\n", dstP, nRead);
		if (!flash_write(block, (int)dstP, nRead)) {
			ret = -1;
			goto fail;
		}

		dstP += nRead;
		nWritten += nRead;
	}

fail:
	free(block);

	return ret;
}
#endif

// Mason Yu On True
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef SEND_LOG
static void startSendLogToServer()
{
	FILE *fp;
	unsigned char username[30], password[30];
	char ipaddr[20], tmpStr[5], tmpBuf[30];

	//strcpy(g_ddns_ifname, "all");
	//sem_post(&semDdnsc);

	if ((fp = fopen("/var/ftpput.txt", "w")) == NULL)
	{
		printf("***** Open file /var/ftpput.txt failed !\n");
		return;
	}

	if ( !mib_get(MIB_LOG_SERVER_IP, (void *)tmpStr)) {
		printf("Get LOG Server IP error!\n");
		fclose(fp);
		return;
	}
	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpStr)), 16);
	ipaddr[15] = '\0';
	//printf("ipaddr=%s\n", ipaddr);

	if ( !mib_get(MIB_LOG_SERVER_NAME, (void *)username)) {
		printf("Get user name for LOG Server IP error!\n");
		fclose(fp);
		return;
	}
	//printf("username=%s\n", username);

	if ( !mib_get(MIB_LOG_SERVER_PASSWORD, (void *)password)) {
		printf("Get user name for LOG Server IP error!\n");
		fclose(fp);
		return;
	}
	//printf("username=%s\n", password);

	fprintf(fp, "open %s\n", ipaddr);
	fprintf(fp, "user %s %s\n", username, password);
	fprintf(fp, "lcd /var/log\n");
	fprintf(fp, "bin\n");
	fprintf(fp, "put messages\n");
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpput.txt");
	//va_cmd("/bin/ftp", 3, 0, "-inv", "-f", "/var/ftpput.txt");

	return;
}

static void auto_startSYSLOGC(void)
{
	//strcpy(g_ddns_ifname, "all");
	sem_post(&semSyslogC);
	signal(SIGALRM, auto_startSYSLOGC);
	alarm(AUTO_STARTSYSLOGC_INTERVAL);
}

static void log_syslogCpid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = "/var/run/syslogC.pid";

	pid = getpid();
	printf("\nsyslogC=%d\n",pid);

	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);

}

static void syslogC(void)
{
	unsigned int entryNum, i;
	char account[70];
	MIB_CE_DDNS_T Entry;


	/* Establish a handler for SIGALRM signals. */
	signal(SIGALRM, auto_startSYSLOGC);
	alarm(AUTO_STARTSYSLOGC_INTERVAL);

	log_syslogCpid();

	signal(SIGUSR1, startSendLogToServer);

	// wait for user start Syslog Client
	while(1) {
		sem_wait(&semSyslogC);
		printf("Auto send syslog file to FTP Server\n");
		startSendLogToServer();
	}
}

static void startSyslogC()
{
	pthread_t ptSyslogCId;

	pthread_create(&ptSyslogCId, NULL, syslogC, NULL);
	pthread_detach(ptSyslogCId);
}
#endif  // #ifdef SEND_LOG
#endif  // #ifdef CONFIG_USER_RTK_SYSLOG

#ifdef CONFIG_REMOTE_CONFIGD
#ifdef CONFIG_DSL_VTUO
#ifdef CONFIG_SPC
static void *remote_configd(void *arg)
{
	int rconfigd_socket, ret;
	struct mymsgbuf qbuf;

	rconfigd_socket = socket(PF_SPC, SOCK_DGRAM, 0);

	if (rconfigd_socket < 0) {
		perror("socket");
		return NULL;
	}

	while (1) {
		if ((ret = recv(rconfigd_socket, &qbuf, sizeof(qbuf), 0)) < 0)
			fprintf(stderr, "%s:%d recv: %s\n", __FUNCTION__, __LINE__, strerror(errno));
		msgProcess(&qbuf);
		if (send(rconfigd_socket, &qbuf, ret, 0) < 0)
			fprintf(stderr, "%s:%d send: %s\n", __FUNCTION__, __LINE__, strerror(errno));
	}

	close(rconfigd_socket);
	return NULL;
}
#else
static void *remote_configd(void *arg)
{
	int rconfigd_socket, ret;
	socklen_t len;
	struct mymsgbuf qbuf;
	struct sockaddr_in addr;

	rconfigd_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (rconfigd_socket < 0) {
		perror("socket");
		return NULL;
	}

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8809);
	addr.sin_family = PF_INET;

	if (bind(rconfigd_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		fprintf(stderr, "%s:%d bind: %s\n", __FUNCTION__, __LINE__, strerror(errno));

	while (1) {
		len = sizeof(addr);
		if ((ret = recvfrom(rconfigd_socket, &qbuf, sizeof(qbuf), 0, (struct sockaddr *)&addr, &len)) < 0)
			fprintf(stderr, "%s:%d recvfrom: %s\n", __FUNCTION__, __LINE__, strerror(errno));
		msgProcess(&qbuf);
		if (sendto(rconfigd_socket, &qbuf, ret, 0, &addr, len) < 0)
			fprintf(stderr, "%s:%d sendto: %s\n", __FUNCTION__, __LINE__, strerror(errno));
	}

	close(rconfigd_socket);
	return NULL;
}
#endif

static void start_remote_configd(void)
{
	pthread_t id;

	pthread_create(&id, NULL, remote_configd, NULL);
	pthread_detach(id);
}
#endif
#endif

int main(void)
{
	init_signals();

/*
 *	Init MIB
 */
#ifndef CONFIG_USER_XMLCONFIG
	if ( mib_init() == 0 ) {
		printf("[configd] Initialize MIB failed!\n");
//		error(E_L, E_LOG, "Initialize MIB failed!\n");
		return -1;
	}
#endif	/*CONFIG_USER_XMLCONFIG*/

	if ((msgqueue_id=initMsgQ()) < 0) {
		return -1;
	}
	if (initShm() < 0) {
		printf("Init shared memory fail !\n");
		return -1;
	}
	//printf("[configd] Initialize 'MsgQ' and 'Shm' finished.\n");
	log_pid();

// Mason Yu on True
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef SEND_LOG
	sem_init(&semSyslogC, 0, 0);
	startSyslogC();
#endif
#endif

#ifdef CONFIG_REMOTE_CONFIGD
#ifdef CONFIG_DSL_VTUO
	start_remote_configd();
#endif
#endif

	msg_handler();
	printf("[configd]: should not be here !!\n");
	return 0;
}

