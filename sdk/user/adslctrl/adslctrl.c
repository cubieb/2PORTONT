/*****************************************************************************/

/*
 *	cpu.c -- simple CPU usage reporting tool.
 *
 *	(C) Copyright 2000, Greg Ungerer (gerg@snapgear.com)
 */

/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <rtk/options.h>
#include <rtk/adslif.h>
#ifdef FIELD_TRY_SAFE_MODE
#include <rtk/utility.h>
#endif

typedef struct CMD_TYPE {
  char *cmd;
  int setFlag;  //=2, use cmd arg to set DMT, =1, no arg set
  int argsize;
  int funcID;
} cmd_t;

cmd_t cmdTable[]={
{"startAdsl", 		2,	4, 	RLCM_PHY_START_MODEM},
{"EnableModemLine", 	1,	0, 	RLCM_PHY_ENABLE_MODEM},
{"DisableModemLine", 	1,	0, 	RLCM_PHY_DISABLE_MODEM},
{"GetDriverVersion", 	1,	0, 	RLCM_GET_DRIVER_VERSION},
{"GetDriverBuild", 	1,	0, 	RLCM_GET_DRIVER_BUILD},
{"ReTrain", 		1,	0, 	RLCM_MODEM_RETRAIN},
{"GetReHSKCount", 	1,	0, 	RLCM_GET_REHS_COUNT},
{"GetChannelSNR", 	1,	0, 	RLCM_GET_CHANNEL_SNR},
{"GetAverageSNR", 	1,	0, 	RLCM_GET_AVERAGE_SNR},
{"GetSNRMargin", 	1,	0, 	RLCM_GET_SNR_MARGIN},
{"ReportModemStates", 	1,	0, 	RLCM_REPORT_MODEM_STATE},
{"ReportPMdata", 	1,	0, 	RLCM_REPORT_PM_DATA},
{"NearEndIdentification", 	1,	0, 	RLCM_MODEM_NEAR_END_ID_REQ},
{"FarEndIdentification", 	1,	0, 	RLCM_MODEM_FAR_END_ID_REQ},
{"NearEndLineData", 	1,	0, 	RLCM_MODEM_NEAR_END_LINE_DATA_REQ},
{"FarEndLineData", 	1,	0, 	RLCM_MODEM_FAR_END_LINE_DATA_REQ},
{"NearEndChannelDataFastMode", 	1,	0, 	RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ},
{"NearEndChannelDataInterleavedMode", 	1,	0, 	RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ},
{"FarEndChannelDataFastMode", 	1,	0, 	RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ},
{"FarEndChannelDataInterleavedMode", 	1,	0, 	RLCM_MODEM_FAR_END_INT_CH_DATA_REQ},
{"SetAdslMode", 	2,	4, 	RLCM_SET_ADSL_MODE},
{"GetAdslMode", 	1,	0, 	RLCM_GET_ADSL_MODE},
{"GetLossData", 	1,	0, 	RLCM_GET_CURRENT_LOSS_DATA},
{"GetLinkSpeed", 	1,	0, 	RLCM_GET_LINK_SPEED},
{"GetChannelMode", 	1,	0, 	RLCM_GET_CHANNEL_MODE},
{"GetLoopAtt", 		1,	0, 	RLCM_GET_LOOP_ATT},
{"IncTxPower", 		2,	4, 	RLCM_INC_TX_POWER},
{"TunePerf", 		2,	4, 	RLCM_TUNE_PERF},
{"DbgData", 		2,	4, 	RLCM_DBG_DATA},
{"EnableBitSwap", 	1,	0, 	RLCM_ENABLE_BIT_SWAP},
{"DisableBitSwap", 	1,	0, 	RLCM_DISABLE_BIT_SWAP},
{"EnablePilotRelocation", 	1,	0, 	RLCM_ENABLE_PILOT_RELOCATION},
{"DisablePilotRelocation", 	1,	0, 	RLCM_DISABLE_PILOT_RELOCATION},
{"EnableTrellis", 	1,	0, 	RLCM_ENABLE_TRELLIS},
{"DisableTrellis", 	1,	0, 	RLCM_DISABLE_TRELLIS},
{"SetVendorID", 	2,	4, 	RLCM_SET_VENDOR_ID},
{"ReadCfg", 		1,	0, 	RLCM_MODEM_READ_CONFIG},
//{"WriteCfg", 		2,	RLCM_MODEM_CONFIG_SIZE, 	RLCM_MODEM_WRITE_CONFIG},
{"DebugMode", 		2,	4, 	RLCM_DEBUG_MODE},
{"TestPSD", 		2,	4, 	RLCM_TEST_PSD},
{"GetChannelBH",	1,	0, 	RLCM_GET_CHANNEL_BH},
{"msgmode",		2,	4, 	RLCM_MSGMODE},
{"reset",		2,	4, 	RLCM_SET_ADSL_MODE},
//tylo, for 8671
{"SendDyingGasp",	1,	0,	RLCM_SEND_DYING_GASP},
{"TestHW",			1,	0,	RLCM_TEST_HW},
{"SetLoopback",		1,	0,	RLCM_SET_LOOPBACK},
{"InitAdslMode",		2,	4,	RLCM_INIT_ADSL_MODE},
{"EnablePOMAccess",	1,	0,	RLCM_ENABLE_POM_ACCESS},
{"DisablePOMAccess",	1,	0,	RLCM_DISABLE_POM_ACCESS},
{"MaskTones",		2,	8,	RLCM_MASK_TONE},//masktones
{"GetCapability",		2,	4,	RLCM_GET_CAPABILITY},
{"VerifyHW",			2,	4,	RLCM_VERIFY_HW},
{"TrigOlrType",		2,	4,	RLCM_TRIG_OLR_TYPE},
{"TrigOlrType1",		2,	4,	RLCM_TRIG_OLR_TYPE1},
{"EnableAEQ",			2,	4,	RLCM_ENABLE_AEQ},
{"EnableHPF",			2,	4,	RLCM_ENABLE_HPF},
{"SetHPFfc",			2,	4,	RLCM_SET_HPF_FC},
{"SetHybrid",			2,	4,	RLCM_SET_HYBRID},
{"SetRxGain",			2,	4,	RLCM_SET_RX_GAIN},
{"SetAFEReg",			2,	4,	RLCM_SET_AFE_REG},
{"SetFOBase",			2,	4,	RLCM_SET_FOBASE},
//yaru + for web data
{"SetXdslMode"		   ,2,4,        RLCM_SET_XDSL_MODE	},
{"GetShowtimeXdslMode"     ,1,0,        RLCM_GET_SHOWTIME_XDSL_MODE  },
{"GetXdslMode"             ,1,0,        RLCM_GET_XDSL_MODE	        },
{"SetOLRType"              ,2,4,        RLCM_SET_OLR_TYPE		},
{"GetOLRType"              ,1,0,        RLCM_GET_OLR_TYPE 	        },
{"GetLineRate"             ,1,0,        RLCM_GET_LINE_RATE	        },
{"GetDsErrorCount"         ,1,0,        RLCM_GET_DS_ERROR_COUNT	},
{"GetUsErrorCount"         ,1,0,        RLCM_GET_US_ERROR_COUNT	},
{"GetDiagQLN"              ,1,0,        RLCM_GET_DIAG_QLN		},
{"GetDiagHlog"             ,1,0,        RLCM_GET_DIAG_HLOG	        },
{"GetDiagSNR"              ,1,0,        RLCM_GET_DIAG_SNR		},
{"GetDsPMSParam1"          ,1,0,        RLCM_GET_DS_PMS_PARAM1	},
{"GetUsPMSParam1"          ,1,0,        RLCM_GET_US_PMS_PARAM1	},
{"SetAnnexB"               ,2,4,        RLCM_SET_ANNEX_B		},
{"GetAnnexB"               ,1,0,        RLCM_GET_ANNEX_B	},
{"SetAnnexL"               ,2,4,        RLCM_SET_ANNEX_L		},
{"GetAnnexL"               ,1,0,        RLCM_GET_ANNEX_L		},
{"SetAnnexM"               ,2,4,        RLCM_SET_ANNEX_M		},
{"GetAnnexM"               ,1,0,        RLCM_GET_ANNEX_M		},
{"GetLinkPowerState"       ,1,0,        RLCM_GET_LINK_POWER_STATE	},
{"GetAttainableRate"       ,1,0,        RLCM_GET_ATT_RATE	},
{"LoadCarrierMask"         ,1,0,        RLCM_LOADCARRIERMASK	},
{"Set8671Rev"              ,2,1,        RLCM_SET_8671_REV		},
{"Get8671Rev"              ,1,0,        RLCM_GET_8671_REV		},
{"SetHighInp"              ,2,4,        RLCM_SET_HIGH_INP		},
{"GetHighInp"              ,1,0,        RLCM_GET_HIGH_INP		},
//-yaru
//TR069
{"GetDslShowtime"	,1,0, 	RLCM_GET_DSL_STAT_SHOWTIME},
{"GetDslTotal" 		,1,0,	RLCM_GET_DSL_STAT_TOTAL},
{"GetDslPSD" 		,1,0,	RLCM_GET_DSL_PSD},
{"GetDslOthers"		,1,0,	RLCM_GET_DSL_ORHERS},
{"GetDslGi"		,1,0,	RLCM_GET_DSL_GI},
//end of tr069
#if defined(CONFIG_DSL_ON_SLAVE)
{"InitSAR"      ,2,4,   RLCM_INIT_ATMSAR},
{"InitPTM"      ,2,4,   RLCM_INIT_PTMMAC},
#endif
{0,1,0,0}
};

const char err_mag[] = "ADSL ioctl failed!\n";
/*****************************************************************************/
//return 0 success, -1 no such command, -2 arg number error, -3 command fail

void argv_to_lower_case(char *str){
	int len,i;
	len=strlen(str);
	for(i=0;i<len;i++){
		if(str[i]>='A' && str[i]<='Z')
				str[i]-=('A'-'a');
	}
}
char *cmd_to_lower_case(char *str){
	int len,i;
	char *retstr;
	len=strlen(str);
	retstr=malloc(len*sizeof(char));
	for(i=0;i<len;i++){
		if(str[i]>='A' && str[i]<='Z')
				retstr[i]=str[i]-('A'-'a');
		else
				retstr[i]=str[i];
	}
	return retstr;
}

char *get_cmd_string(char *str){
	char *retstr=NULL;
	char *TableCmd;
	cmd_t *pCmd=cmdTable;
	int max_matching_len=0,len;
	//argv_to_lower_case(str); //change argv[1] to lower case
	while(pCmd->cmd!=NULL){
		TableCmd=cmd_to_lower_case(pCmd->cmd);
		if(strstr(TableCmd,str)==TableCmd){
			if(strlen(str)>max_matching_len){
				max_matching_len=strlen(str);
				retstr=pCmd->cmd;
			}
			else if(strlen(str)==max_matching_len){
				printf("Ambiguous command!\n");
				exit(0);
			}
		}
		free(TableCmd);
		pCmd++;
	}
	if(retstr==NULL){
		printf("No such command!\n");
		exit(0);
	}
	return retstr;
}

int AdslCMD(FILE *fp, int argc, char *argv[])
{
  cmd_t *pCmd=cmdTable;
  obcif_arg	myarg;
  char *cmdstr;

  cmdstr=get_cmd_string(argv[1]);//tylo, pCmd->cmd to lowercase string cmpstr
    while (pCmd->cmd!=NULL) {
	//if (strcmp(argv[1],pCmd->cmd)==0) {
	if(cmdstr==pCmd->cmd){	//tylo, cmdstr compare with pCmd->cmd
	    if (pCmd->argsize!=0) {  //use arg for API
	    	char *ptr;
	    	int *ptr32;
	    	int *ptr32_2;
		ptr32=ptr=malloc(pCmd->argsize);
	    	if (pCmd->setFlag==2) {  //need set arg from console
	    	    	if(pCmd->argsize==8){
				if (argc!=4) {
				printf("Error! Command need argument!\n");
				free(ptr);
	    	    		return -2;
	    	    		};
						ptr32_2=ptr32+4;
	    	    		*ptr32 = strtol(argv[2],0,0); //small arguments, only WriteCfg can not work
	    	    		*ptr32_2=strtol(argv[3],0,0);
	    	    		myarg.arg = (int)ptr32;
	    	    	}
	    	    	else{
	    	    		if (argc!=3) {
					printf("Error! Command need argument!\n");
					free(ptr);
	    	    			return -2;
	    	    		};
	    	    		*ptr32 = strtol(argv[2],0,0); //small arguments, only WriteCfg can not work
	    	    		myarg.arg = (int)ptr32;
	    	    	}
		} else {  //get from DMT
	    	    myarg.arg = (int)ptr;
		};
	    	myarg.argsize = pCmd->argsize;
	        if (ioctl(fileno(fp), pCmd->funcID, &myarg) < 0) {
		    free(ptr);
	    	    return -3;
	    	};
		free(ptr);
	    } else {  //no arg
	    	if (ioctl(fileno(fp), pCmd->funcID, NULL)<0)
	    	    return -3;
	    };
	    break;
	};
    	pCmd++;
    };
    if (pCmd->cmd!=NULL) {
        return 0;
    } else {
	printf("No such command!\n");
        return -1;
    };
}

int main(int argc, char *argv[])
{
	FILE		*fp;
	char		*procdevice, data[8];
	obcif_arg	myarg;
	int		arg;
	char rValueAdslDrv[64];

#if defined(CONFIG_DSL_ON_SLAVE)
    procdevice = "/dev/xdsl_ipc";
#elif defined(CONFIG_XDSL_CTRL_PHY_IS_SOC)
	procdevice = "/dev/xdsl0";
#else
    procdevice = "/dev/adsl0";
#endif

	if (argc<2) {
	  printf("usage:adslctrl command args %d %s\n", argc, argv[1]);
	  exit(0);
	};

	argv_to_lower_case(argv[1]); //tylo, change argv[1] to lower case
	//printf("command args %d %s\n", argc, argv[1]);

	if ((fp = fopen(procdevice, "r")) == NULL) {
	    printf(stderr, "ERROR: failed to open %s, errno=%d\n",
		procdevice, errno);
	    exit(0);
	};
	memset(&myarg, 0, sizeof(obcif_arg));
if (strcmp(argv[1],"test")==0) {
	    //for v1.0.278, auto reconnect if rate is OK, disconnect if rate is below input arg
	    int rate_drop=0;
	    int mode, last_rate=0;
	    Modem_LinkSpeed Link_Speed;

	    if (argc<3) {
	    	printf("usage: adslctrl test mode\n");
	        goto stop_adslctrl;
	    };
	    mode = atoi(argv[2]);
	    if ((mode<1)||(mode>3)) {
	    	printf("adsl link mode error, only 1, 2, 3 is allowed\n");
	        goto stop_adslctrl;
	    };

	    while (rate_drop==0) {
retry_test:
	    	//disconnect
	        if (ioctl(fileno(fp), RLCM_PHY_DISABLE_MODEM, NULL) < 0) {
	    	    printf("ADSL disconnect failed! %x\n", RLCM_PHY_DISABLE_MODEM);
	        };
	        //wait 1 sec
	        sleep(3);
	        //reconnect
	        myarg.argsize = 4;
	        myarg.arg = (int)&mode;
	        if (ioctl(fileno(fp), RLCM_PHY_ENABLE_MODEM, &myarg) < 0) {
	    	    printf("ADSL connect failed %x!\n", RLCM_PHY_ENABLE_MODEM);
	    	    goto retry_test;
	        };
	        //wait show time
wait_showtime:
	        sleep(1);
	        Link_Speed.downstreamRate = 0;
	        myarg.argsize = sizeof(Link_Speed);
	        myarg.arg = (int)&Link_Speed;
	        if (ioctl(fileno(fp), RLCM_GET_LINK_SPEED, &myarg) < 0) {
	    	    printf("List API command failed! %x\n", RLCM_GET_LINK_SPEED);
	        };
	        if (Link_Speed.downstreamRate==0)
	            goto wait_showtime;
	        //show time
	        sleep(10);
	        if (last_rate!=0) {
	            int diff;
	            diff = Link_Speed.downstreamRate-last_rate;
	            if ( (diff<0)&&(abs(diff)>(last_rate/2)) ) {
	                printf("Stop adsl test! Link rate too low. Last: %d\n", last_rate);
	                rate_drop=1;
	                goto stop_adslctrl;
	            }
	        };
	        last_rate = Link_Speed.downstreamRate;
	    };
	} else if (strcmp(argv[1],"ioctl")==0) {
	    if (argc!=3) {
	        printf("ioctl argument number error!\n");
	    } else {
	        unsigned int id=atoi(argv[2]);
	        unsigned char ifr[256];

	        if (id==-1) {
	            printf("No such ioctl command!\n");
	        } else {
	    	    myarg.argsize = 256;
	    	    myarg.arg = (int)ifr;
	    	    memset(ifr, 0, 256);
	    	    printf("ADSL ioctl %x\n", id);
	            if (ioctl(fileno(fp), id, &myarg) < 0) {
	    	        printf("ADSL ioctl failed! %x\n", id);
	            } else {
	            	int i;
	                for (i=0;i<32;i++) {
	                  printf("%02X: %02X %02X %02X %02X %02X %02X %02X %02X\n", i*8,
	                      ifr[i*8+0], ifr[i*8+1], ifr[i*8+2], ifr[i*8+3],
	                      ifr[i*8+4], ifr[i*8+5], ifr[i*8+6], ifr[i*8+7]);
	                };
	            };
	        };
	    };
	} else if (strcmp(argv[1],"adslver")==0) {
		myarg.argsize = RLCM_DRIVER_VERSION_SIZE;
		myarg.arg = (int)&rValueAdslDrv[0];
		if (ioctl(fileno(fp), RLCM_GET_DRIVER_VERSION, &myarg) < 0) {
		    printf(err_mag);
		};
		printf("Version: %s\n", rValueAdslDrv);
	} else if (strcmp(argv[1],"adslbuild")==0) {
		myarg.argsize = RLCM_DRIVER_BUILD_SIZE;
		myarg.arg = (int)&rValueAdslDrv[0];
		if (ioctl(fileno(fp), RLCM_GET_DRIVER_BUILD, &myarg) < 0) {
		    printf(err_mag);
		};
		printf("Build: %s\n", rValueAdslDrv);
#ifdef FIELD_TRY_SAFE_MODE
	//} else if (strcmp(argv[1], "getsafemode") == 0) {
	} else if (strstr("getsafemode", argv[1]) == "getsafemode") {
		SafeModeData vSmd;

		memset((void *)&vSmd, 0, sizeof(vSmd));
		myarg.argsize = SAFEMODE_DATA_SIZE;
		myarg.arg = (int)&vSmd;
		if (ioctl(fileno(fp), RLCM_GET_SAFEMODE_CTRL, &myarg) < 0)
			printf(err_mag);
		printf("Field Try Safe Mode : %s\n", !vSmd.FieldTrySafeMode? "Disable" : "Enable");
		printf("Field Try Test PSD Times : %d\n", vSmd.FieldTryTestPSDTimes);
		printf("Field Try CtrlIn : 0x%x\n", vSmd.FieldTryCtrlIn);
		printf("Safe Mode Note : %s\n", vSmd.SafeModeNote);
	//} else if (strcmp(argv[1], "setsafemode") == 0) {
	} else if (strstr("setsafemode", argv[1]) == "setsafemode") {
		char mode = 0;
		int i;
		SafeModeData vSmd;
		char ptr[10];

		if (argc != 5) {
			printf("setsafemode argument number error!\n");
			goto stop_adslctrl;
		};
		memset((void *)&vSmd, 0, sizeof(vSmd));
		if (!strcmp(argv[2], "1"))
			mode = 1;
		vSmd.FieldTrySafeMode = (int)mode;
		vSmd.FieldTryTestPSDTimes = atoi(argv[3]);
		if (!(!strncmp(argv[4], "0x", 2) || !strncmp(argv[4], "0X", 2))) {
			printf("FieldTryCtrlIn argument must be HEX!\n");
			goto stop_adslctrl;
		}
		if (strlen(argv[4]) > 10) {
			printf("FieldTryCtrlIn argument can't be more than 8 chars !\n");
			goto stop_adslctrl;
		}
		memset(ptr, 0, 10);
		memmove(ptr, argv[4] + 2, strlen(argv[4]) - 2);
		for (i = 0; i < strlen(ptr); i ++) {
			if ((ptr[i] >= '0' && ptr[i] <= '9') || (ptr[i] >= 'A' && ptr[i] <= 'F') || (ptr[i] >= 'a' && ptr[i] <= 'f'))
				continue;
			printf("Invalid FieldTryCtrlIn argument !\n");
			goto stop_adslctrl;
		}
		vSmd.FieldTryCtrlIn = strtoul(ptr, 0, 16);
		mib_set(MIB_ADSL_FIELDTRYSAFEMODE, (void *)&mode);
		mib_set(MIB_ADSL_FIELDTRYTESTPSDTIMES, (void *)&vSmd.FieldTryTestPSDTimes);
		mib_set(MIB_ADSL_FIELDTRYCTRLIN, (void *)&vSmd.FieldTryCtrlIn);
		Commit();
		myarg.argsize = SAFEMODE_DATA_SIZE;
		myarg.arg = (int)&vSmd;
		if (ioctl(fileno(fp), RLCM_SET_SAFEMODE_CTRL, &myarg) < 0)
			printf(err_mag);
#endif
	} else {
	    //Search cmd table
	    AdslCMD(fp, argc, argv);
	};

stop_adslctrl:
	fclose(fp);
	exit(0);
}

/*****************************************************************************/
