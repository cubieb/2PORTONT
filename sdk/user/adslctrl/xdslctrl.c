#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "obcif.h"

static void print_mem(unsigned char *start, unsigned int size)
{
	int     row, column, index, index2, max;
	unsigned char    *buf, ascii[17];
	char    empty = '.';
	
	buf = (char*) start;
	/*
	   16 bytes per line
	 */	
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, buf += 16)
	{
		memset(ascii, 0, 17);
		printf("%08x ", (unsigned int)(buf-start));

		max = (index == row - 1) ? column : 16;

		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
				printf("  ");
			printf("%02x ", (unsigned char) buf[index2]);
			ascii[index2] = (!isprint(buf[index2])) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printf("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printf("   ");
		}

		//ASCII
		printf("  %s\n", ascii);
	}	
}

static struct option longopts[] = {
	/* { name  has_arg  *flag  val } */
	{"raw",	        0, 0, 'r'},	/* print raw data. */	
	{"device",	    required_argument, 0, 'd'},	/*  */
	{"help",		0, 0, 'h'},	/* Give help */
	{"usage",		0, 0, 'u'},	/* Give usage */
	{"verbose",		0, 0, 'v'},	/* Report each action taken. */
	{"version",		0, 0, 'V'},	/* Emit version information. */
	{ 0, 0, 0, 0}
};

struct dsldrv_cmd;
typedef int data_handler_func_t(struct dsldrv_cmd *e, unsigned char *buf, unsigned int buflen);
typedef int set_handler_func_t(struct dsldrv_cmd *e, int argc, char *argv[]);

struct dsldrv_cmd {
	char *cmd;
	int  cmdId;
	unsigned short flags;	/* Get or Set */
	unsigned short size;	/* for GET, sizeof(arg), for SET, number of bytes params required */
	void *handler; /* */
};
#define CMD_FLAG_GET 0x00
#define CMD_FLAG_SET 0x01

static int handle_get_string(struct dsldrv_cmd *e, unsigned char *buf, unsigned int buflen) {
	//char *fmt = NULL;
	int i;
	
	//switch(e->cmdId) {
	//case RLCM_GET_DRIVER_VERSION: fmt = "Driver Version = "; break;
	//case RLCM_GET_DRIVER_BUILD:   fmt = "Driver Build = "; break;
	//}
	//
	//if (fmt) {	
	//	printf(fmt);
	//}
	printf("\"%s\"\n", buf);

	return 0;
}

static int handle_get_bin(struct dsldrv_cmd *e, unsigned char *buf, unsigned int buflen) {
	printf("Returned %d bytes\n", buflen);
	print_mem(buf, buflen);
	return 0;
}

static int handle_get_uint(struct dsldrv_cmd *e, unsigned char *buf, unsigned int buflen) {
	int n, i;
	unsigned int val;
	unsigned char *p;
	char *fmt; 
	
	if ((buflen!=1) && (buflen!=2) && (buflen&0x3)) {
		printf("%s: cannot handle size %d\n", __func__,buflen);
		return -1;
	}
	
	p = buf;
	
	n = (buflen+3)>>2;
	for (i=0; i<n; i++) {
		switch(buflen) {
		case 1:
			val = buf[0];
			fmt = "0x%x";
			break;
		case 2:
			val = *((unsigned short *)&buf[0]);
			val = ntohs(val);
			fmt = "0x%x";
			break;
		default:
			val = *((unsigned int *)p);
			val = ntohl(val);
			fmt = "0x%x ";
			p += 4;
		}
		printf(fmt, val);
	}
	printf("\n");
	return 0;
}

static int handle_get_auto(struct dsldrv_cmd *e, unsigned char *buf, unsigned int buflen) {
	if (buflen > 4)
		return handle_get_bin(e,buf,buflen);
	else
		return handle_get_uint(e,buf,buflen);
}

static struct dsldrv_cmd cmdtable[] = {
	{"startAdsl",           RLCM_PHY_START_MODEM,     CMD_FLAG_SET,	 4, NULL },
	{"EnableModemLine",     RLCM_PHY_ENABLE_MODEM,               0,	 0, NULL },
	{"DisableModemLine",    RLCM_PHY_DISABLE_MODEM,              0,  0, NULL },
	{"GetDriverVersion",    RLCM_GET_DRIVER_VERSION,             0,  RLCM_DRIVER_VERSION_SIZE, (void *)handle_get_string },
	{"GetDriverBuild",      RLCM_GET_DRIVER_BUILD,               0,  RLCM_DRIVER_BUILD_SIZE,   (void *)handle_get_string },
	{"ReTrain",             RLCM_MODEM_RETRAIN,                  0,  0, NULL},
	{"GetReHSKCount",       RLCM_GET_REHS_COUNT,                 0,  RLCM_GET_REHS_COUNT_SIZE, (void *)handle_get_uint},
	{"GetChannelSNR",       RLCM_GET_ADSL_CHANNEL_SNR,           0,  RLCM_GET_ADSL_CHANNEL_SNR_SIZE, (void *)handle_get_bin},
	//{"GetAverageSNR",       RLCM_GET_AVERAGE_SNR,                0,  0, NULL},
	{"GetSNRMargin",        RLCM_GET_ADSL_SNR_MARGIN,            0,  RLCM_GET_ADSL_SNR_MARGIN_SIZE, (void *)handle_get_uint},
	{"ReportModemStates",   RLCM_REPORT_MODEM_STATE,             0,  RLCM_REPORT_MODEM_STATE_SIZE, (void *)handle_get_bin},	
	{"ReportPMdata",        RLCM_REPORT_PM_DATA,                 0,  RLCM_REPORT_PM_DATA_SIZE,   (void *)handle_get_auto},
    {"NearEndIdentification", RLCM_MODEM_NEAR_END_ID_REQ,        0,  RLCM_MODEM_ID_REQ_SIZE,     (void *)handle_get_auto},
    //{"FarEndIdentification",  RLCM_MODEM_FAR_END_ID_REQ          0,  RLCM_MODEM_ID_REQ_SIZE,     (void *)handle_get_auto},
    {"NearEndLineData", 	RLCM_MODEM_NEAR_END_LINE_DATA_REQ,   0,  RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE, (void *)handle_get_auto},
    //{"FarEndLineData", 	    RLCM_MODEM_FAR_END_LINE_DATA_REQ,    0,  RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE,  (void *)handle_get_auto},
    {"NearEndChannelDataFastMode",        RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ, 0, RLCM_MODEM_CH_DATA_REQ_SIZE, (void *)handle_get_auto},
    {"NearEndChannelDataInterleavedMode", RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ,  0, RLCM_MODEM_CH_DATA_REQ_SIZE, (void *)handle_get_auto},
    {"FarEndChannelDataFastMode", 	      RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ,  0, RLCM_MODEM_CH_DATA_REQ_SIZE, (void *)handle_get_auto},
    {"FarEndChannelDataInterleavedMode",  RLCM_MODEM_FAR_END_INT_CH_DATA_REQ,   0, RLCM_MODEM_CH_DATA_REQ_SIZE, (void *)handle_get_auto},
    {"SetAdslMode", 	    RLCM_SET_ADSL_MODE,       CMD_FLAG_SET,  4, NULL},
    {"GetAdslMode", 	    RLCM_GET_ADSL_MODE,                  0,	 4, (void *)handle_get_auto },
    {"GetLossData", 	    RLCM_GET_CURRENT_LOSS_DATA,          0,	 RLCM_GET_LOSS_DATA_SIZE, (void *)handle_get_auto },
    {"GetLinkSpeed", 	    RLCM_GET_LINK_SPEED,                 0,	 RLCM_GET_LINK_SPEED_SIZE, (void *)handle_get_auto },
    {"GetChannelMode", 	    RLCM_GET_CHANNEL_MODE,               0,	 RLCM_GET_CHANNEL_MODE_SIZE, (void *)handle_get_auto },
    {"GetLoopAtt", 		    RLCM_GET_ADSL_LOOP_ATT,              0,	 0, (void *)handle_get_auto },
    {"IncTxPower", 		    RLCM_INC_TX_POWER,        CMD_FLAG_SET,	 4, (void *)handle_get_auto },
    {"TunePerf", 		    RLCM_TUNE_PERF,           CMD_FLAG_SET,  4, (void *)handle_get_auto },
    {"DbgData", 		    RLCM_DBG_DATA,            CMD_FLAG_SET,	 4, (void *)handle_get_auto },
    {"EnableBitSwap", 	    RLCM_ENABLE_BIT_SWAP,                0,	 0, NULL },
    {"DisableBitSwap", 	    RLCM_DISABLE_BIT_SWAP,               0,	 0, NULL },
    {"EnablePilotRelocation", RLCM_ENABLE_PILOT_RELOCATION,      0,	 0, NULL },
    {"DisablePilotRelocation",RLCM_DISABLE_PILOT_RELOCATION,     0,	 0, NULL },
    {"EnableTrellis",       RLCM_ENABLE_TRELLIS,                 0,	 0, NULL },
    {"DisableTrellis",      RLCM_DISABLE_TRELLIS,                0,	 0, NULL },
    {"SetVendorID",         RLCM_SET_VENDOR_ID,       CMD_FLAG_SET,	 4, NULL },
    {"ReadCfg",             RLCM_MODEM_READ_CONFIG,              0,  RLCM_MODEM_CONFIG_SIZE, (void *)handle_get_auto },
	{"DebugMode",           RLCM_DEBUG_MODE,          CMD_FLAG_SET,	 4, NULL },
	{"TestPSD",             RLCM_TEST_PSD,            CMD_FLAG_SET,	 4, NULL },
	{"GetChannelBH",        RLCM_GET_CHANNEL_BH,                 0,	 0, NULL },
	{"msgmode",	            RLCM_MSGMODE,             CMD_FLAG_SET,	 4, NULL },	
    {"SetLoopback",	        RLCM_SET_LOOPBACK,                   0,	 0, NULL },    
    {"MaskTones",           RLCM_MASK_TONE,           CMD_FLAG_SET,	 8, NULL },//masktones
    {"GetCapability",       RLCM_GET_CAPABILITY,      CMD_FLAG_SET,	 4, NULL },
    {"VerifyHW",            RLCM_VERIFY_HW,           CMD_FLAG_SET,	 4, NULL },    
    {"TrigOlrType1",        RLCM_TRIG_OLR_TYPE1,      CMD_FLAG_SET,	 4, NULL },    
    {"EnableHPF",           RLCM_ENABLE_HPF,          CMD_FLAG_SET,	 4, NULL },
    {"SetHybrid",           RLCM_SET_HYBRID,          CMD_FLAG_SET,	 4, NULL },
    {"SetRxGain",           RLCM_SET_RX_GAIN,         CMD_FLAG_SET,	 4, NULL },
    {"SetAFEReg",           RLCM_SET_AFE_REG,         CMD_FLAG_SET,	 4, NULL },
	{"SetXdslMode",         RLCM_SET_XDSL_MODE,       CMD_FLAG_SET,	 4, NULL 	},
    {"GetShowtimeXdslMode", RLCM_GET_SHOWTIME_XDSL_MODE,         0,	 XDSL_MODE_SIZE, (void *)handle_get_auto },
    {"GetXdslMode",         RLCM_GET_XDSL_MODE,                  0,	 XDSL_MODE_SIZE, (void *)handle_get_auto },
    {"SetOLRType",          RLCM_SET_OLR_TYPE,        CMD_FLAG_SET,	 SET_OLR_TYPE_SIZE, NULL },
    {"GetOLRType",          RLCM_GET_OLR_TYPE,                   0,	 GET_OLR_TYPE_SIZE, (void *)handle_get_auto },
    {"GetLineRate",         RLCM_GET_LINE_RATE,                  0,	 GET_LINE_RATE_SIZE, (void *)handle_get_auto },
    {"GetDsErrorCount",     RLCM_GET_DS_ERROR_COUNT,             0,	 GET_DS_ERROR_COUNT_SIZE, (void *)handle_get_auto },
    {"GetUsErrorCount",     RLCM_GET_US_ERROR_COUNT,             0,	 GET_US_ERROR_COUNT_SIZE, (void *)handle_get_auto },
    {"GetDiagQLN",          RLCM_GET_ADSL_DIAG_QLN,              0,	 GET_ADSL_DIAG_QLN_SIZE, (void *)handle_get_auto },
    {"GetDiagHlog",         RLCM_GET_ADSL_DIAG_HLOG,             0,	 GET_ADSL_DIAG_HLOG_SIZE, (void *)handle_get_auto },
    {"GetDiagSNR",          RLCM_GET_ADSL_DIAG_SNR,              0,	 GET_ADSL_DIAG_SNR_SIZE, (void *)handle_get_auto },
    {"GetDsPMSParam1",      RLCM_GET_XDSL_DS_PMS_PARAM1,         0,	 GET_XDSL_PMS_SIZE, (void *)handle_get_auto	},
    {"GetUsPMSParam1",      RLCM_GET_XDSL_US_PMS_PARAM1,         0,	 GET_XDSL_PMS_SIZE, (void *)handle_get_auto	},
    {"SetAnnexB",           RLCM_SET_ANNEX_B,         CMD_FLAG_SET,	 GET_ANNEX_B_SIZE, NULL	},
    {"GetAnnexB",           RLCM_GET_ANNEX_B,                    0,	 GET_ANNEX_B_SIZE, (void *)handle_get_auto	},
    {"SetAnnexL",           RLCM_SET_ANNEX_L,         CMD_FLAG_SET,	 GET_ANNEX_L_SIZE, NULL },
    {"GetAnnexL",           RLCM_GET_ANNEX_L,                    0,	 GET_ANNEX_L_SIZE, (void *)handle_get_auto		},
    {"SetAnnexM",           RLCM_SET_ANNEX_M,         CMD_FLAG_SET,	 GET_ANNEX_M_SIZE, NULL },
    {"GetAnnexM",           RLCM_GET_ANNEX_M,                    0,	 GET_ANNEX_M_SIZE, (void *)handle_get_auto		},
    {"GetLinkPowerState",   RLCM_GET_LINK_POWER_STATE,           0,	 GET_LINK_POWER_STATE_SIZE, (void *)handle_get_auto	},
    {"GetAttainableRate",   RLCM_GET_ATT_RATE,                   0,	 GET_LINE_RATE_SIZE, (void *)handle_get_auto	},
    {"LoadCarrierMask",     RLCM_LOADCARRIERMASK,                0,	 GET_LOADCARRIERMASK_SIZE, (void *)handle_get_auto	},
    {"Set8671Rev",          RLCM_SET_8671_REV,        CMD_FLAG_SET,	 1, NULL },
    {"Get8671Rev",          RLCM_GET_8671_REV,                   0,	 1, (void *)handle_get_auto		},
    {"SetHighInp",          RLCM_SET_HIGH_INP,        CMD_FLAG_SET,	 GET_HIGH_INP_SIZE, NULL		},
    {"GetHighInp",          RLCM_GET_HIGH_INP,                   0,  GET_HIGH_INP_SIZE, (void *)handle_get_auto},
	{ 0, 0, 0, 0, 0 }
};

static int opt_r = 0;
static int opt_h = 0;
static int opt_v = 0;
static int opt_V = 0;
static char opt_devname[64] = "/dev/adsl0";
static int devfd;


static inline int cmd_is_get(struct dsldrv_cmd *e) {
	return ((e->flags & 0x01)==0);
}


static void usage(void) {
	printf("\nxdslctrl <command> <arg1> <arg2> ...\n");
}

static void help(void) {
	struct dsldrv_cmd *e;
	int i;
	printf("command list:\n");
	for (i=0; cmdtable[i].cmd; i++) {
		e = &cmdtable[i];
		printf("  %s ", e->cmd);
		if (cmd_is_get(e)) {
			printf("\n");
		} else {
			printf("<arg>\n");
		}
	}
}

/* Find the entry with matching cmd */
/* Return pos of entry found, -1 if no entry found, -2 if more than two matching */
static int get_cmd_entry(struct dsldrv_cmd *tbl, const char *cmd) {
	struct dsldrv_cmd *e;
	int i, len, ret = -1;
	
	len = strlen(cmd);
	for (i=0; tbl[i].cmd; i++) {
		e = &tbl[i];
		if (strncasecmp(e->cmd, cmd, len))
			continue;
		if (ret >= 0)
			return -2;
		ret = i;
	}
	return ret;	
}

static int process_get_command(struct dsldrv_cmd *e, int argc, char *argv[]) {
	unsigned char buffer[64];
	obcif_arg *pifarg, ifarg;
	int ret;
	
	memset((void *)&ifarg, 0, sizeof(ifarg));
	if (e->size) {			
		ifarg.argsize = htonl(e->size);
		ifarg.arg     = (int) ((e->size > sizeof(buffer)) ? calloc(1, e->size) : buffer);
		if (!ifarg.arg) {
			printf("Cannot alloc memory, %d bytes required\n", e->size);
			return -1;
		}
		pifarg = &ifarg;
	} else {
		pifarg = NULL;
	}
	
	ret = ioctl(devfd, e->cmdId, pifarg);
	
	if (ret < 0) {
		printf("ioctl %x failed\n", e->cmdId);
		perror("");
		return -2;
	}
	
	if (opt_r) {
		if (e->size) {
			printf("Raw Data: len = %d\n", e->size);
			print_mem((unsigned char *)ifarg.arg, ifarg.argsize);			
		}
	}
	if (e->handler) {
		data_handler_func_t *handler = e->handler;
		handler(e, (unsigned char *)ifarg.arg, ntohl(ifarg.argsize));
	}
	
	if (ifarg.arg && ((unsigned char *)ifarg.arg != buffer))
		free ((void *)ifarg.arg);
	
	return 0;
}


static int process_set_command(struct dsldrv_cmd *e, int argc, char *argv[]) {
	unsigned char buffer[64];
	obcif_arg *pifarg, ifarg;
	int i, n, ret = 0;
		
	if (e->handler) {
		set_handler_func_t *setfunc  = e->handler;
		return setfunc(e, argc, argv);
	}
	
	/* make sure there is enough parameters */
	n = (e->size+3)>>2;
	if (argc < n) {
		printf("Requires %d argument(s)\n", n);
		return -1;
	}
	
	/* only allow 1,2,4n */
	if ((e->size!=1) && (e->size!=2) && (e->size&0x3)) {
		printf("Argument size is invalid %d\n", e->size);
		return -1;
	}
	
	if (e->size) {
		unsigned int val;
		unsigned char *p;
		
		ifarg.argsize = htonl(e->size);
		ifarg.arg     = (int) ((e->size > sizeof(buffer)) ? calloc(1, e->size) : buffer);
		if (!ifarg.arg) {
			printf("Cannot alloc memory, %d bytes required\n", e->size);
			return -1;
		}
		pifarg = &ifarg;
		p = (unsigned char *)ifarg.arg;
		/* parse each argument as number */
		for (i=0; i<n; i++) {
			errno = 0;			
			val = strtoul(argv[i], NULL, 0);
			if (errno) {
				printf("Arg %s is not valid (%s)\n", i, argv[i]);
				perror("");
				return -1;
			}
			
			switch(e->size) {
			case 1: 
				val &= 0xff;
				*p = val; 
				break;
			case 2: 
				val &= 0xffff;
				*((unsigned short *)p) = htons(val); 
				break;
			default:
				*((unsigned int *)p) = htonl(val);
				p += 4;
				break;			
			}
		}		
	} else {
		pifarg = NULL;
	}	
	
	if (opt_r) {
		if (e->size) {
			printf("Raw Data: len = %d\n", ifarg.argsize);
			print_mem((unsigned char *)ifarg.arg, ifarg.argsize);			
		}
	}	
	
	if (ioctl(devfd, e->cmdId, pifarg) < 0) {
		printf("ioctl %x failed: ", e->cmdId);
		perror("");
		ret = -2;
		goto out;
	}
out:
	if (ifarg.arg && ((unsigned char *)ifarg.arg != buffer))
		free ((void *)ifarg.arg);
		
	return ret;	
}


int main(int argc, char *argv[])
{
	int c, index, ret = 0;
	char *cmd;
	struct dsldrv_cmd *e;
	
	while ((c = getopt_long(argc, argv, "rd:hvV", longopts, 0)) != EOF)  {
		switch (c) {
		case 'r': opt_r = 1; break;
		case 'd': strncpy(opt_devname, optarg, sizeof(opt_devname)); break;
		case 'h': opt_h = 1; break;		
		case 'v': opt_v = 1; break;
		case 'V': opt_V = 1; break;
		}
	}

	if (opt_h) {
		usage();
		help();
		exit(0);
	}
	
	if (optind >= argc) {
		usage();
		exit(0);	
	}
		
	devfd = open(opt_devname, 0);
	if (devfd < 0) {
		printf("Cannot open %s\n", opt_devname);
		perror("");
		ret = 2;
		goto out;
	}
	
	cmd = argv[optind++];
	
	index = get_cmd_entry(cmdtable, cmd);
	switch(index) {
	case -1:
		printf("Unknown command: %s\n", cmd);
		break;
	case -2:
		printf("Ambiguous command: %s\n", cmd);
		break;
	default:
		e = &cmdtable[index];
		if (opt_v) {
			printf("Cmd: %s ID:%x %s(%d)\n", e->cmd, e->cmdId, cmd_is_get(e) ? "GET" : "SET", e->size);
		}
		if (cmd_is_get(e))
			ret = process_get_command(e, argc - optind, &argv[optind]);
		else
			ret = process_set_command(e, argc - optind, &argv[optind]);
	}
	
	close(devfd);
out:		
	return ret;
}
