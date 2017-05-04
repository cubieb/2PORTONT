#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include "atm.h"
#include <linux/atmdev.h>
#include <linux/atmbr2684.h>
#include <linux/autoconf.h>
#ifdef CONFIG_ATM_CLIP	// Jenny, for IPoA
#include <linux/atmclip.h>
#include <linux/atmarp.h>
#endif
#include "rtk/options.h"

/* Written by Marcell GAL <cell@sch.bme.hu> to make use of the */
/* ioctls defined in the br2684... kernel patch */
/* Compile with cc -o br2684ctl br2684ctl.c -latm */

/*
  Modified feb 2001 by Stephen Aaskov (saa@lasat.com)
  - Added daemonization code
  - Added syslog
  
  TODO: Delete interfaces after exit?
*/


#define LOG_NAME "RFC1483/2684 bridge"
#define LOG_OPTION     LOG_PERROR
#define LOG_FACILITY   LOG_LOCAL0

#define DEV_NAME	"vc"

struct mpoa_cfg {
	struct mpoa_cfg *prev;
	struct mpoa_cfg *next;
	int sock;
	char ifname[IFNAMSIZ]; 
	int encaps;
	struct sockaddr_atmpvc addr;
	struct atm_qos	qos;
//#ifdef CONFIG_RE8305
	// vlan tagging
	struct vlan vlan_tag;
	// interface group
	struct itfgrp ifgrp;
#ifdef NEW_PORTMAPPING
	uint16_t fgroup;
#endif
	
//#endif
#ifdef CONFIG_ATM_CLIP
	unsigned long clip_ip;	// Jenny, ipoa host ip
	unsigned long srv_ip;	// Jenny, ipoa arp server ip
#endif
};

struct command
{
	int	needs_mpoa_arg;
	int	num_string_arg;
	char	*name;
	int	(*func)(struct mpoa_cfg *cfg, char *arg);
};



#define SERVER_FIFO_NAME "/tmp/serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/cli_fifo"
#define BUFFER_SIZE	256

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};

struct mpoa_cfg *cfg_head = NULL;

int lastsock, lastitf;

static char *qosstr = NULL;

void fatal(char *str, int i)
{
  syslog (LOG_ERR,"Fatal: %s",str);
  exit(-2);
};


void exitFunc(void)
{
  syslog (LOG_PID,"Daemon terminated\n");	
}

struct mpoa_cfg * find_cfg_ptr(char *arg)
{
struct mpoa_cfg *cfg;

	if(cfg_head == NULL)
		return NULL;
	else {
		cfg = cfg_head;
		do {
			if(!strcmp(cfg->ifname, arg))
				return cfg;
			cfg = cfg->next;
		} while(cfg);
		return NULL;
	}
}

int show_if(struct mpoa_cfg *cfg, char *arg)
{
#ifdef CONFIG_ATM_CLIP	// Jenny, for IPoA
char *encaps_str[] = {"BR1483 VCMUX", "BR1483 LLC", "BR1483 AUTO", "RT1483 VCMUX", "RT1483 LLC", "RT1483 AUTO", "RT1577 LLC" };
#else
char *encaps_str[] = {"BR1483 VCMUX", "BR1483 LLC", "BR1483 AUTO", "RT1483 VCMUX", "RT1483 LLC", "RT1483 AUTO" };
#endif
char *qos_str[] = {"none", "UBR", "CBR", "VBR"};
char *aal_str[] = {"AAL0", "AAL5"};

	cfg = cfg_head;
	while(cfg) {
		printf("%s\t%d.%d %s %s\n", cfg->ifname,
		   	cfg->addr.sap_addr.vpi,
		   	cfg->addr.sap_addr.vci,
		   	(cfg->qos.aal == ATM_AAL0) ? aal_str[0]: aal_str[1], 
		   	encaps_str[cfg->encaps]);
		printf("\t%s PCR = %d SCR = %d MBS = %d\n\n", 
			qos_str[cfg->qos.txtp.traffic_class],
			cfg->qos.txtp.pcr,
			cfg->qos.txtp.scr,
			cfg->qos.txtp.mbs);
		cfg = cfg->next;
	}
	return 0;
}

#ifdef CONFIG_ATM_CLIP
int create_clip(struct mpoa_cfg *cfg, char *arg)	// Jenny, create IPoA interface
{
	int num, err;
	unsigned int number;
	struct mpoa_cfg *new_cfg;
	
	if((new_cfg = malloc(sizeof(struct mpoa_cfg)))==NULL)
		return -1;

	new_cfg->sock = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
	if(new_cfg->sock < 0) {
		free(new_cfg);
		return -1;
	}

	strcpy(new_cfg->ifname, arg);
	sscanf(new_cfg->ifname, "vc%u", &number);
	num = ioctl (new_cfg->sock, SIOCMKCLIP, number);

	if (num >= 0) {
		if(cfg_head == NULL) {
			cfg = cfg_head = new_cfg;
			cfg->prev = NULL;
			cfg->next = NULL;
		}
		else {
			cfg = cfg_head;
			while(cfg->next!=NULL)
				cfg = cfg->next;
			cfg->next = new_cfg;
			new_cfg->prev = cfg;
			new_cfg->next = NULL;
			cfg = new_cfg;		
		}
		syslog(LOG_INFO, "Interface \"%s\" created sucessfully\n", cfg->ifname);
	}
	else {
		close(new_cfg->sock);
		free(new_cfg);
		syslog(LOG_INFO, "Interface \"%s\" could not be created, reason: %s\n",
			cfg->ifname, strerror(errno));
	}
	
	return 0;
}
#endif

int create_if(struct mpoa_cfg *cfg, char *arg)
{
	int num, err;
	struct mpoa_cfg *new_cfg;
	
	if((new_cfg = malloc(sizeof(struct mpoa_cfg)))==NULL)
		return -1;

	new_cfg->sock = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
	if(new_cfg->sock < 0) {
		free(new_cfg);
		return -1;
	}

	strcpy(new_cfg->ifname, arg);
	
	/* create the device with ioctl: */
	//num=atoi(arg);
	//if( num>=0 && num<16)
	{
		struct atm_newif_br2684 ni;
		ni.backend_num = ATM_BACKEND_BR2684;
		ni.media = BR2684_MEDIA_ETHERNET;
		ni.mtu = 1500;
		//sprintf(ni.ifname, "%s%d", DEV_NAME, num);
		strcpy(ni.ifname, new_cfg->ifname);
		err=ioctl (new_cfg->sock, ATM_NEWBACKENDIF, &ni);
		
		if (err == 0) {
			if(cfg_head == NULL) {
				cfg = cfg_head = new_cfg;
				cfg->prev = NULL;
				cfg->next = NULL;
			}
			else {
				cfg = cfg_head;
				while(cfg->next!=NULL)
					cfg = cfg->next;
				cfg->next = new_cfg;
				new_cfg->prev = cfg;
				new_cfg->next = NULL;
				cfg = new_cfg;		
			}
			syslog(LOG_INFO, "Interface \"%s\" created sucessfully\n",ni.ifname);
		}
		else {
			close(new_cfg->sock);
			free(new_cfg);
			syslog(LOG_INFO, "Interface \"%s\" could not be created, reason: %s\n",
				ni.ifname, strerror(errno));
		}
	} 
	//else {
	//  syslog(LOG_ERR,"err: strange interface number %d", num );
	//}
	
	return 0;
}

int delete_if(struct mpoa_cfg *cfg, char *arg)
{
	/* find new cfg instance */
	cfg = cfg_head;
	while (cfg != NULL) {
		if(!strcmp(cfg->ifname, arg))
			break;
		cfg = cfg->next;
	};

	if(cfg==NULL)
		return -1;

	close(cfg->sock);

	if(cfg->next)
		cfg->next->prev = cfg->prev;

	if(cfg->prev)
		cfg->prev->next = cfg->next;
	else
		cfg_head = cfg->next;
				
	free(cfg);	
	
	return 0;
}

int set_encaps(struct mpoa_cfg *cfg, char *arg)
{

	if(cfg==NULL)
		return -1;
	cfg->encaps = atoi(arg);
	return 0;
}

int set_qos(struct mpoa_cfg *cfg, char *arg)
{
    int err, errno;
    int sndbuf = 8192;

	if(cfg==NULL)
		return -1;

    memset(&cfg->qos, 0, sizeof(struct atm_qos));

	if (arg != NULL)
    	if (text2qos(arg, &cfg->qos, 0))
      		printf("Can't parse QoS: %s \n", arg);

    if(!cfg->qos.aal)
		cfg->qos.aal = ATM_AAL5;
	if(!cfg->qos.txtp.traffic_class)
    	cfg->qos.txtp.traffic_class = ATM_UBR;
    if(!cfg->qos.txtp.pcr)
    	cfg->qos.txtp.pcr = 0x1DFF;
    cfg->qos.txtp.max_sdu            = 1524;
	cfg->qos.rxtp = cfg->qos.txtp;

	if ( (err=setsockopt(cfg->sock,SOL_SOCKET,SO_SNDBUF, &sndbuf ,sizeof(sndbuf))) )
		syslog(LOG_ERR,"setsockopt SO_SNDBUF: (%d) %s\n",err, strerror(err));
    
	if ( setsockopt(cfg->sock, SOL_ATM, SO_ATMQOS, &cfg->qos, sizeof(struct atm_qos)) < 0)
		syslog(LOG_ERR,"setsockopt SO_ATMQOS: %d\n", errno);

	return 0;
}


int assign_vcc(struct mpoa_cfg *cfg, char *arg)
{
    int err, errno;
    int fd;
    struct atm_backend_br2684 be;

	if(cfg==NULL)
		return -1;

    memset(&cfg->addr, 0, sizeof(struct sockaddr_atmpvc));
    err=text2atm(arg,(struct sockaddr *)(&cfg->addr), sizeof(struct sockaddr_atmpvc), T2A_PVC);
    if (err!=0)
      syslog(LOG_ERR,"Could not parse ATM parameters (error=%d)\n",err);
    
    syslog(LOG_INFO,"Communicating over ATM %d.%d.%d, encapsulation: %d\n", 
    	cfg->addr.sap_addr.itf,
	   	cfg->addr.sap_addr.vpi,
	   	cfg->addr.sap_addr.vci,
	   	cfg->encaps);
	   	
	if(cfg->qos.aal == 0)
		set_qos(cfg, "ubr");
    
    err = connect(cfg->sock, (struct sockaddr*)&cfg->addr, sizeof(struct sockaddr_atmpvc));
    
    if (err < 0) {
		syslog (LOG_ERR,"failed to connect on socket");    
		return -1;
	}
    
    /* attach the vcc to device: */
    
#ifdef CONFIG_ATM_CLIP	// Jenny, for IPoA
	if (cfg->encaps == 6)
	{
		if (ioctl(cfg->sock, ATMARP_MKIP, CLIP_DEFAULT_AGINGTIMER) < 0) {
			err = -errno;
			syslog (LOG_ERR, "ioctl ATMARP_MKIP: %s", strerror(errno));
			exit(2);
		}
		else
			syslog (LOG_INFO,"Interface configured");
	}
	else {
#endif
    be.backend_num = ATM_BACKEND_BR2684;
    be.ifspec.method = BR2684_FIND_BYIFNAME;
    //sprintf(be.ifspec.spec.ifname, "%s%d", DEV_NAME, lastitf);
    strcpy(be.ifspec.spec.ifname, cfg->ifname);
    be.fcs_in = BR2684_FCSIN_NO;
    be.fcs_out = BR2684_FCSOUT_NO;
    be.fcs_auto = 0;
    be.encaps = cfg->encaps;
    be.has_vpiid = 0;
    be.send_padding = 0;
    be.min_size = 0;
    err=ioctl (cfg->sock, ATM_SETBACKEND, &be);
    if (err == 0)
      syslog (LOG_INFO,"Interface configured");
    else {
      syslog (LOG_ERR,"Could not configure interface:%s",strerror(errno));
      exit(2);
    }
#ifdef CONFIG_ATM_CLIP
	}
#endif
    
    return 0 ;
}

//#ifdef CONFIG_RE8305
// Vlan tagging
int set_vid(struct mpoa_cfg *cfg, char *arg)
{

	if(cfg==NULL)
		return -1;
	cfg->vlan_tag.vid = atoi(arg);
	return 0;
}

int set_vprio(struct mpoa_cfg *cfg, char *arg)
{

	if(cfg==NULL)
		return -1;
	cfg->vlan_tag.vlan_prio = atoi(arg);
	return 0;
}

int set_vpass(struct mpoa_cfg *cfg, char *arg)
{

	if(cfg==NULL)
		return -1;
	cfg->vlan_tag.vlan_pass = atoi(arg);
	return 0;
}

int assign_vlan(struct mpoa_cfg *cfg, char *arg)
{
	int err;
	int fd;
	struct atm_backend_br2684 be;
	
	if(cfg==NULL)
		return -1;
	
	cfg->vlan_tag.vlan = atoi(arg);
	
	//printf("assign_vlan: vlan=%d, vid=%d, vprio=%d, vpass=%d\n", cfg->vlan_tag.vlan, cfg->vlan_tag.vid, cfg->vlan_tag.vprio, cfg->vlan_tag.vpass);
	be.ifspec.method = BR2684_FIND_BYIFNAME;
	strcpy(be.ifspec.spec.ifname, cfg->ifname);
	be.vlan_tag.vlan = cfg->vlan_tag.vlan;
	be.vlan_tag.vid = cfg->vlan_tag.vid;
	be.vlan_tag.vlan_prio = cfg->vlan_tag.vlan_prio;
	be.vlan_tag.vlan_pass = cfg->vlan_tag.vlan_pass;
	err=ioctl (cfg->sock, ATM_SETVLAN, &be);
	return 0;
}

// Interface Group
int set_member(struct mpoa_cfg *cfg, char *arg)
{

	if(cfg==NULL)
		return -1;
	cfg->ifgrp.member = atoi(arg);
	return 0;
}

int assign_group(struct mpoa_cfg *cfg, char *arg)
{
	int err;
	int fd;
	struct atm_backend_br2684 be;
	
	if(cfg==NULL)
		return -1;
	
	cfg->ifgrp.flag = atoi(arg);
	
	be.ifspec.method = BR2684_FIND_BYIFNAME;
	strcpy(be.ifspec.spec.ifname, cfg->ifname);
	memcpy(&be.ifgrp, &cfg->ifgrp, sizeof(struct itfgrp));
	//printf("assign_group: group=%d, member=0x%x\n", be.ifgrp.flag, be.ifgrp.member);
	err=ioctl (cfg->sock, ATM_SETITFGRP, &be);
	return 0;
}
//#endif

#ifdef CONFIG_ATM_CLIP	// Jenny, for IPoA
int init_atmarp(struct mpoa_cfg *cfg, char *arg)	// Jenny, init atm arpd
{
	if(cfg==NULL) {
		printf("init_atmarp: cfg=NULL\n");
		return -1;
	}
	
	if (ioctl(cfg->sock, ATMARPD_CTRL, 0) < 0) {
		syslog(LOG_ERR, "ioctl ATMARPD_CTRL: %s", strerror(errno));
		exit(2);
	}
	else
		syslog (LOG_INFO,"atmarp init");
	return 0;
}

int set_ip(struct mpoa_cfg *cfg, char *arg)	// Jenny, set ipoa local host ip
{
	char *endptr;
	if(cfg==NULL)
		return -1;
	cfg->clip_ip = strtoul(arg, &endptr, 0);
	if (ioctl(cfg->sock, ATMARP_IP, cfg->clip_ip) < 0) {
		syslog(LOG_ERR, "ioctl ATMARP_IP: %s", strerror(errno));
		exit(2);
	}
	else
		syslog (LOG_INFO,"atmarp set_ip");
	return 0;
}

int send_inARPRep(struct mpoa_cfg *cfg, char *arg)	// Jenny, send inATMARP Reply
{
	char *endptr;
	if (cfg == NULL)
		return -1;
	cfg->srv_ip = strtoul(arg, &endptr, 0);
	if (ioctl(cfg->sock, ATMARP_INARPREP, cfg->srv_ip) < 0) {
		syslog(LOG_ERR, "ioctl ATMARP_INARPREP: %s", strerror(errno));
	}
	else
		syslog (LOG_INFO,"atmarp send_inARPRep");
	return 0;
}

/*
int set_srv(struct mpoa_cfg *cfg, char *arg)	// Jenny, set ipoa atm arp server ip
{
	char *endptr;
	if(cfg==NULL)
		return -1;
	cfg->srv_ip = strtoul(arg, &endptr, 0);
	if (ioctl(cfg->sock, ATMARP_SRV, cfg->srv_ip) < 0) {
		syslog(LOG_ERR, "ioctl ATMARP_IP: %s", strerror(errno));
		exit(2);
	}
	else
		syslog (LOG_INFO,"atmarp set_srv");
	return 0;
}*/
#endif


#ifdef NEW_PORTMAPPING
int set_fgroup(struct mpoa_cfg *cfg, char *arg)
{
	int err;
	int fd;
	struct atm_backend_br2684 be;
	
	if(cfg==NULL)
		return -1;

	cfg->fgroup = (uint16_t)atoi(arg);
	
	be.ifspec.method = BR2684_FIND_BYIFNAME;
	strcpy(be.ifspec.spec.ifname, cfg->ifname);
	//we just consider the switch lan port,
	//put away the wlan
	be.fgroup = (uint16_t)(cfg->fgroup & 0x1ff);
	err=ioctl (cfg->sock, ATM_SETFGROUP, &be);
	return 0;
}
#endif

/*-----------------------------------------------------------------
 * usage:
 * mpoactl add vc0 pvc 0.33 encaps 1 qos ubr:pcr=7600
 * mpoactl set vc0 vlan 1 vid 23 vprio 2 vpass 0
 *----------------------------------------------------------------*/

static struct command commands[] = {
	{0, 1, "show", show_if},
#ifdef CONFIG_ATM_CLIP
	{0, 1, "addclip", create_clip},	// Jenny, create IPoA interface
#endif
	{0, 1, "add", create_if},
	{0, 0, "set", 0},
	{1, 1, "del", delete_if},
	{1, 1, "encaps", set_encaps},
	{1, 1, "qos", set_qos},
	{1, 1, "pvc", assign_vcc},
//#ifdef CONFIG_RE8305
	// vlan tagging
	{1, 1, "vid", set_vid},
	{1, 1, "vprio", set_vprio},
	{1, 1, "vpass", set_vpass},
	{1, 1, "vlan", assign_vlan},
	// interface group
	{1, 1, "member", set_member},
	{1, 1, "group", assign_group},

#ifdef NEW_PORTMAPPING
	{1, 1, "fgroup", set_fgroup},
#endif
	
//#endif
#ifdef CONFIG_ATM_CLIP
	{1, 1, "atmarp", init_atmarp},	// Jenny, init atm arpd
	{1, 1, "cip", set_ip},	// Jenny, set ipoa client ip
	{1, 1, "inarprep", send_inARPRep},	// Jenny, send inATMARP Reply
//	{1, 1, "sip", set_srv},	// Jenny, set ipoa atm arp server ip
//	{1, 1, "entry", set_entry},
#endif
	{0, 0, NULL, NULL}
};

struct command *mpoa_command_lookup(char *arg)
{
	int i;
	for(i=0; commands[i].name!=NULL; i++)
		if(!strcmp(arg, commands[i].name))
			return (&commands[i]);
	return NULL;
}

void usage(char *s)
{
  printf("usage: %s [-b] [[-c number] [-e 0|1] [-a [itf.]vpi.vci]*]*\n", s);
  exit(1);
}


#define MAX_ARGS	20
#define MAX_ARG_LEN	50


#ifdef EMBED
static char mpoad_pidfile[] = "/var/run/mpoad.pid";
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = mpoad_pidfile;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void quit_signal(int sig)
{
	unlink(mpoad_pidfile);
	exit(1);
}
#endif

#ifdef _LINUX_2_6_
// Use this would cause busy waitting. Should it be removed ?
//#define _MPOA_NOT_CLOSE_FD_
#endif //_LINUX_2_6_


int main (void)
{
	int server_fifo_fd, client_fifo_fd;
	struct data_to_pass_st 	msg;
	int	read_res;
	int i, c;
  	int argc;
	char argv[MAX_ARGS][MAX_ARG_LEN+1];
  	char *arg_ptr;
	struct mpoa_cfg *cfg;
	
	lastsock=-1;
	lastitf=0;
	mkfifo(SERVER_FIFO_NAME, 0777);

	syslog (LOG_INFO, "RFC 1483/2684 bridge daemon started\n");
#ifdef EMBED
	signal (SIGTERM, quit_signal);
	log_pid();
#endif

  	while (1) {
		server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
		if (server_fifo_fd == -1) {
			fprintf(stderr, "Server fifo failure\n");
			exit(EXIT_FAILURE);
		}
#ifdef _MPOA_NOT_CLOSE_FD_
read_again:
#endif //_MPOA_NOT_CLOSE_FD_		
		argc = 0;
		//jiunming, this may be an unnecessary do{}while loop
		//if read for the second time, use the same buffer as one for the first time
		//note: for 8672, it occurs that 2 commands may be read in the do{} while loop.
		//do {
			read_res = read(server_fifo_fd, &msg, sizeof(msg));
			if(read_res > 0) {
				int arg_idx = 0;
				arg_ptr = msg.data;
				for(i=0; arg_ptr[i]!='\0'; i++) {
					if(arg_ptr[i]==' '){
						argv[argc][arg_idx]='\0';
						argc++;
						arg_idx=0;
					}
					else {
						if(arg_idx<MAX_ARG_LEN) {
							argv[argc][arg_idx]=arg_ptr[i];
							arg_idx++;
						}
					}
				}
				argv[argc][arg_idx]='\0';
			}
		//} while (read_res > 0);
#ifndef _MPOA_NOT_CLOSE_FD_
		close(server_fifo_fd);
#endif //_MPOA_NOT_CLOSE_FD_

		cfg = NULL;
		for(c=0; commands[c].name!=NULL; c++) {
			for(i=1; i<argc; i++) {
				if(!strcmp(argv[i], commands[c].name)) {
					if(!cfg)
						if(commands[c].needs_mpoa_arg)
							cfg = find_cfg_ptr(argv[2]);
					if(commands[c].num_string_arg) {
						commands[c].func(cfg, argv[i+1]);
						if (!cfg)
							break;
					}
				}
			}
		}
		
#ifdef _MPOA_NOT_CLOSE_FD_
		goto read_again;
#endif //_MPOA_NOT_CLOSE_FD_
	}
	
#ifdef EMBED
	unlink(mpoad_pidfile);
#endif
	atexit (exitFunc);
  
	return 0;
}

