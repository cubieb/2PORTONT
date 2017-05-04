#include <stdio.h>
#include <string.h>
#include "boot_param.h"

//default values
#define BOARD_PARAM_MAC		"\x00\x23\x79\x11\x22\x33"
#define BOARD_PARAM_IP		0xc0a80111 //192.168.1.17
#define BOARD_PARAM_IPMASK	0xffffff00
#define BOARD_PARAM_SER_IP	0 //0xc0a80132 //0xc0a80101 //192.168.1.1
#define BOARD_PARAM_FNAME	"" //"vmlinux.lzma"


unsigned char bp_mac[6]=BOARD_PARAM_MAC;
unsigned long bp_ip=BOARD_PARAM_IP;
unsigned long bp_ipmask=BOARD_PARAM_IPMASK;
unsigned long bp_serverip=BOARD_PARAM_SER_IP;
unsigned char bp_filename[24]=BOARD_PARAM_FNAME;


int bootconf_get_default( bootconf_t *buf)
{
	if(!buf) return -1;
	memset(buf, 0, sizeof(bootconf_t));
	buf->magic = BOOTCONF_MAGIC;
	memcpy(buf->mac, BOARD_PARAM_MAC, 6);
	buf->ip = BOARD_PARAM_IP;
	buf->ipmask = BOARD_PARAM_IPMASK;
	buf->serverip = BOARD_PARAM_SER_IP;
	strcpy( (char*)buf->filename, BOARD_PARAM_FNAME);
	
	return 0;
} 

int bootconf_get(bootconf_t *buf)
{
	if(!buf) return -1;

	bootconf_get_default(buf);
	
	//change the default values, ex: mac/ip/server/filename
	memcpy(buf->mac, bp_mac, 6);
	buf->ip=bp_ip;
	buf->ipmask=bp_ipmask;
	buf->serverip=bp_serverip;
	strcpy( (char*)buf->filename, (char*)bp_filename);

	return 0;
}

void bootparam_set_ip(unsigned long  p)
{
	bp_ip=p;
}

void bootparam_set_ipmask(unsigned long  p)
{
	bp_ipmask=p;
}


void bootparam_set_serverip(unsigned long  p)
{
	bp_serverip=p;
}

void bootparam_set_filename(char *p)
{
	if(p && (strlen(p)<sizeof(bp_filename)) )
		strcpy( (char*)bp_filename, p);
}

void bootparam_set_mac(char *p)
{
	if(p)
		memcpy(bp_mac, p, sizeof(bp_mac));
}


