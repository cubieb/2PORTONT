/*
 *      Adsl Driver Interface
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

// from hrchen adslctrl.c
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include "adsl_drv.h"
#include "utility.h"
#include "mib.h"
// from hrchen adslctrl.c end

//for RLCM_GET_LINE_RATE
typedef struct {
	unsigned short	upstreamRate;
	unsigned short	downstreamRate;
} Modem_LineRate;

static char rValueAdslDrv[ADSL_DRV_RETURN_LEN];
static char rGetFail[] = "n/a";

adsl_drv_get_cmd get_cmd_list[] = {
	{"snr-ds", ADSL_GET_SNR_DS},
	{"snr-us", ADSL_GET_SNR_US},
	{"version", ADSL_GET_VERSION},
	{"mode", ADSL_GET_MODE},
	{"state", ADSL_GET_STATE},
	{"power-ds", ADSL_GET_POWER_DS},
	{"power-us", ADSL_GET_POWER_US},
	{"attrate-ds", ADSL_GET_ATTRATE_DS},
	{"attrate-us", ADSL_GET_ATTRATE_US},
	{"rate-ds", ADSL_GET_RATE_DS},
	{"rate-us", ADSL_GET_RATE_US},
	{"latency", ADSL_GET_LATENCY},
  //       {"latency-us", ADSL_GET_LATENCY_US},
  //       {"latency-ds", ADSL_GET_LATENCY_DS},
	{"lpatt-ds", ADSL_GET_LPATT_DS},
	{"lpatt-us", ADSL_GET_LPATT_US},
	{"trellis", ADSL_GET_TRELLIS},
	{"pwlevel", ADSL_GET_POWER_LEVEL},
	{"pms-k-ds", ADSL_GET_K_DS},
	{"pms-k-us", ADSL_GET_K_US},
	{"pms-r-ds", ADSL_GET_R_DS},
	{"pms-r-us", ADSL_GET_R_US},
	{"pms-s-ds", ADSL_GET_S_DS},
	{"pms-s-us", ADSL_GET_S_US},
	{"pms-d-ds", ADSL_GET_D_DS},
	{"pms-d-us", ADSL_GET_D_US},
	{"pms-delay-ds", ADSL_GET_DELAY_DS},
	{"pms-delay-us", ADSL_GET_DELAY_US},
	{"fec-ds", ADSL_GET_FEC_DS},
	{"fec-us", ADSL_GET_FEC_US},
	{"crc-ds", ADSL_GET_CRC_DS},
	{"crc-us", ADSL_GET_CRC_US},
	{"es-ds", ADSL_GET_ES_DS},
	{"es-us", ADSL_GET_ES_US},
	{"ses-ds", ADSL_GET_SES_DS},
	{"ses-us", ADSL_GET_SES_US},
	{"uas-ds", ADSL_GET_UAS_DS},
	{"uas-us", ADSL_GET_UAS_US},
	{"los-us", ADSL_GET_LOS_US},
	{"los-ds", ADSL_GET_LOS_DS},
	{"lnk-fi", ADSL_GET_FULL_INIT},
	{"lnk-lfi", ADSL_GET_FAILED_FULL_INIT},
	{"lnk-llds", ADSL_GET_LAST_LINK_DS},
	{"lnk-llus", ADSL_GET_LAST_LINK_US},
	{"tx-frames", ADSL_GET_TX_FRAMES},
	{"rx-frames", ADSL_GET_RX_FRAMES},
	{"synchronized-time", ADSL_GET_SYNCHRONIZED_TIME}, 
	{"synchronized-number", ADSL_GET_SYNCHRONIZED_NUMBER},

#ifdef CONFIG_VDSL
//CONFIG_DSL_VTUO will use these
	{"trellis-ds", DSL_GET_TRELLIS_DS},
	{"trellis-us", DSL_GET_TRELLIS_US},	
	{"ginp-ds", DSL_GET_PHYR_DS},
	{"ginp-us", DSL_GET_PHYR_US},
	{"n-ds", DSL_GET_N_DS},
	{"n-us", DSL_GET_N_US},
	{"l-ds", DSL_GET_L_DS},
	{"l-us", DSL_GET_L_US},
	{"inp-ds", DSL_GET_INP_DS},
	{"inp-us", DSL_GET_INP_US},
	{"tps", DSL_GET_TPS},
#endif /*CONFIG_VDSL*/

#ifdef CONFIG_DSL_VTUO
	{"pmd-mode", DSL_GET_PMD_MODE},
	{"link-type", DSL_GET_LINK_TYPE},
	{"limit-mask", DSL_GET_LIMIT_MASK},
	{"us0-mask", DSL_GET_US0_MASK},
	{"upbokle-ds", DSL_GET_UPBOKLE_DS},
	{"upbokle-us", DSL_GET_UPBOKLE_US},
	{"actualce", DSL_GET_ACTUALCE},

	{"signal-atn-ds", DSL_GET_SIGNAL_ATN_DS},
	{"signal-atn-us", DSL_GET_SIGNAL_ATN_US},
	{"line-atn-ds", DSL_GET_LINE_ATN_DS},
	{"line-atn-us", DSL_GET_LINE_ATN_US},
	{"rx-power-ds", DSL_GET_RX_POWER_DS},
	{"rx-power-us", DSL_GET_RX_POWER_US},
	{"pmspara-i-ds", DSL_GET_PMSPARA_I_DS},
	{"pmspara-i-us", DSL_GET_PMSPARA_I_US},
	{"data-lpid-ds", DSL_GET_DATA_LPID_DS},
	{"data-lpid-us", DSL_GET_DATA_LPID_US},
	{"ptm-status", DSL_GET_PTM_STATUS},
	{"ra-mode-ds", DSL_GET_RA_MODE_DS},
	{"ra-mode-us", DSL_GET_RA_MODE_US},
	{"retx-ohrate-ds", DSL_GET_RETX_OHRATE_DS},
	{"retx-ohrate-us", DSL_GET_RETX_OHRATE_US},
	{"retx-fram-type-ds", DSL_GET_RETX_FRAM_TYPE_DS},
	{"retx-fram-type-us", DSL_GET_RETX_FRAM_TYPE_US},
	{"retx-actinp-rein-ds", DSL_GET_RETX_ACTINP_REIN_DS},
	{"retx-actinp-rein-us", DSL_GET_RETX_ACTINP_REIN_US},
	{"retx-h-ds", DSL_GET_RETX_H_DS},
	{"retx-h-us", DSL_GET_RETX_H_US},
#endif /*CONFIG_DSL_VTUO*/
	{"vector-mode", DSL_GET_VECTOR_MODE},

	{NULL, 0}
};

/*
 *	Get adsl string according to the id.
 *	The string is put into buf, and this function return the number of
 *	characters in the string (not including the trailing '\0')
 */
static int _getAdslInfo(ADSL_GET_ID id, char *buf, int len, XDSL_OP *d)
{
	int ret;
	int intVal[2];
	char chVal[2];
	Modem_NearEndLineOperData vNeld;
	Modem_FarEndLineOperData vFeld;
	Modem_AttRate vAr;
	Modem_LinkSpeed vLs;
	Modem_PMSParm pms;
	Modem_AvgLoopAttenuation vAla;
	Modem_Config vMc;
	Modem_MgmtCounter counter;
#ifdef CONFIG_VDSL
	int msgval[4];
#endif /*CONFIG_VDSL*/	
#ifdef LOOP_LENGTH_METER
	unsigned short loop_length;
#endif
	ret = 0;
	
	memset(buf, 0x00, len);
	if(d==NULL) return ret;

	switch (id) {
		case ADSL_GET_SNR_DS:
			if(d->xdsl_drv_get(RLCM_GET_SNR_MARGIN, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", intVal[0]/10, intVal[0]%10);
			}
			break;
		case ADSL_GET_SNR_US:
			if(d->xdsl_drv_get(RLCM_GET_SNR_MARGIN, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", intVal[1]/10, intVal[1]%10);
			}
			break;
		case ADSL_GET_VERSION:
			if(d->xdsl_drv_get(RLCM_GET_DRIVER_VERSION, (void *)rValueAdslDrv, RLCM_DRIVER_VERSION_SIZE)) {
				ret = snprintf(buf, len, "%s", rValueAdslDrv);
			}
			break;
		case ADSL_GET_MODE:
			#ifdef CONFIG_VDSL
			{
				int mval=0, pval=0;

				if( d->xdsl_msg_get(GetPmdMode,&mval) &&
					d->xdsl_msg_get(GetVdslProfile,&pval) )
				{
					//the prefix should keep the same as ADSL
					//printf( "\nmval=0x%08x, pval=0x%08x\n", mval,pval );
					if(mval&MODE_ANSI) ret=snprintf(buf, len, "%s ", "T1.413-ANSI");
					else if(mval&MODE_ETSI) ret=snprintf(buf, len, "%s ", "T1.413-ETSI");
					else if(mval&MODE_GDMT) ret=snprintf(buf, len, "%s ", "G.dmt");
					else if(mval&MODE_GLITE) ret=snprintf(buf, len, "%s ", "G.Lite");
					else if(mval&MODE_ADSL2) ret=snprintf(buf, len, "%s ", "ADSL2");
					else if(mval&MODE_ADSL2PLUS) ret=snprintf(buf, len, "%s ", "ADSL2+");
					else if(mval&MODE_VDSL2)
					{
						const char *pfmt="VDSL2-%s ";
						if(pval&VDSL2_PROFILE_8A) ret=snprintf(buf, len, pfmt, "8A");
						else if(pval&VDSL2_PROFILE_8B) ret=snprintf(buf, len, pfmt, "8B");
						else if(pval&VDSL2_PROFILE_8C) ret=snprintf(buf, len, pfmt, "8C");
						else if(pval&VDSL2_PROFILE_8D) ret=snprintf(buf, len, pfmt, "8D");
						else if(pval&VDSL2_PROFILE_12A) ret=snprintf(buf, len, pfmt, "12A");
						else if(pval&VDSL2_PROFILE_12B) ret=snprintf(buf, len, pfmt, "12B");
						else if(pval&VDSL2_PROFILE_17A) ret=snprintf(buf, len, pfmt, "17A");
						else if(pval&VDSL2_PROFILE_30A) ret=snprintf(buf, len, pfmt, "30A");
						else ret=snprintf(buf, len, pfmt, "N/A");
					}

					if(mval&MODE_ANX_A) ret=snprintf(buf, len, "%s %s", buf, "Annex A");
					else if(mval&MODE_ANX_B) ret=snprintf(buf, len, "%s %s", buf, "Annex B");
					else if(mval&MODE_ANX_I) ret=snprintf(buf, len, "%s %s", buf, "Annex I");
					else if(mval&MODE_ANX_J) ret=snprintf(buf, len, "%s %s", buf, "Annex J");
					else if(mval&MODE_ANX_L) ret=snprintf(buf, len, "%s %s", buf, "Annex L");
					else if(mval&MODE_ANX_M) ret=snprintf(buf, len, "%s %s", buf, "Annex M");
					else if(buf[0]) ret=snprintf(buf, len, "%s %s", buf, "Annex A");		
				}
			}
			#else /*CONFIG_VDSL*/
			if(d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
				//jim suit for annex mode append to showtime mode...
				unsigned int annexMode;
				char *ExpMode[]=
				{
					"",
					" AnnexB",
					" AnnexM",
					"",
					" AnnexL",
					0
				};
				annexMode=((unsigned char)chVal[0])>>5;
				if(annexMode >4)
					annexMode=0;
				chVal[0]&=0x1F;
				if (chVal[0] == 1) { // ADSL1
					d->xdsl_drv_get(RLCM_GET_ADSL_MODE, (void *)&intVal[0], 4);
					if (intVal[0]==1)
						ret = snprintf(buf, len, "%s", "T1.413");
					if (intVal[0]==2)
						ret = snprintf(buf, len, "%s", "G.dmt");
				}
				else if (chVal[0] == 2)  // G.992.2 -- G.Lite
						ret = snprintf(buf, len, "%s", "G.Lite");
				else if (chVal[0] == 3)  // ADSL2
						ret = snprintf(buf, len, "%s", "ADSL2");
				else if (chVal[0] == 5)  // ADSL2+
						ret = snprintf(buf, len, "%s", "ADSL2+");
				//printf("annex mode =%s\n", ExpMode[annexMode]);
				strcat(buf, ExpMode[annexMode]);
			}
			#endif /*CONFIG_VDSL*/
			break;
		// get string
		case ADSL_GET_STATE:
			if(d->xdsl_drv_get(RLCM_REPORT_MODEM_STATE, (void *)buf, len)) {
				ret = strlen(buf);
			}
			else
				ret = snprintf(buf, len, "%s", "IDLE");
			break;
		case ADSL_GET_POWER_DS:
			if(d->xdsl_drv_get(RLCM_MODEM_FAR_END_LINE_DATA_REQ, (void *)&vFeld, RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE)) {
				ret = snprintf(buf, len, "%d.%d" ,vFeld.outputPowerDnstr/2, ((vFeld.outputPowerDnstr&1)*5)%10);
			}
			break;
		case ADSL_GET_POWER_US:
			if(d->xdsl_drv_get(RLCM_MODEM_NEAR_END_LINE_DATA_REQ, (void *)&vNeld, RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE)) {
				ret = snprintf(buf, len, "%d.%d" ,vNeld.outputPowerUpstr/2, ((vNeld.outputPowerUpstr&1)*5)%10);
			}
			break;
		case ADSL_GET_ATTRATE_DS:
			#ifdef CONFIG_VDSL
			if(d->xdsl_msg_get_array(GetAttNDR, msgval))
				ret = snprintf(buf, len, "%d", msgval[0]);			
			#else
			if(d->xdsl_drv_get(RLCM_GET_ATT_RATE, (void *)&vAr, sizeof(Modem_AttRate))) {
				ret = snprintf(buf, len, "%d"
					,vAr.downstreamRate);
			}
			#endif /*CONFIG_VDSL*/
			break;
		case ADSL_GET_ATTRATE_US:
			#ifdef CONFIG_VDSL
			if(d->xdsl_msg_get_array(GetAttNDR, msgval))
				ret = snprintf(buf, len, "%d", msgval[1]);			
			#else
			if(d->xdsl_drv_get(RLCM_GET_ATT_RATE, (void *)&vAr, sizeof(Modem_AttRate))) {
				ret = snprintf(buf, len, "%d"
					,vAr.upstreamRate);
			}
			#endif /*CONFIG_VDSL*/
			break;
		case ADSL_GET_RATE_DS:
			if(d->xdsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)) {
				ret = snprintf(buf, len, "%d"
					,vLs.downstreamRate);
			}
			break;
		case ADSL_GET_RATE_US:
			if(d->xdsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)) {
				ret = snprintf(buf, len, "%d"
					,vLs.upstreamRate);
			}
			break;
		case ADSL_GET_LATENCY:
			if(d->xdsl_drv_get(RLCM_GET_CHANNEL_MODE, (void *)&intVal[0], sizeof(int))) {
				if (intVal[0]==1)
					ret = snprintf(buf, len, "%s", "Fast");
				else if(intVal[0]==2)
					ret = snprintf(buf, len, "%s", "Interleave");
			}
			break;		
		case ADSL_GET_LPATT_DS:
			if(d->xdsl_drv_get(RLCM_GET_LOOP_ATT, (void *)&vAla, RLCM_GET_LOOP_ATT_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", vAla.downstreamAtt/2, (vAla.downstreamAtt*5)%10);
			}
			break;
		case ADSL_GET_LPATT_US:
			if(d->xdsl_drv_get(RLCM_GET_LOOP_ATT, (void *)&vAla, RLCM_GET_LOOP_ATT_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", vAla.upstreamAtt/2, (vAla.upstreamAtt*5)%10);
			}
			break;
		case ADSL_GET_TRELLIS:
			if(d->xdsl_drv_get(RLCM_MODEM_READ_CONFIG, (void *)&vMc, RLCM_MODEM_CONFIG_SIZE)) {
				ret = snprintf(buf, len, "%s", (vMc.TrellisEnable==1)?"Enable":"Disable");
			}
			break;
		case ADSL_GET_POWER_LEVEL:
			if(d->xdsl_drv_get(RLCM_GET_LINK_POWER_STATE, (void *)&chVal[0], 1)) {
				if (chVal[0] == 0)
					ret = snprintf(buf, len, "%s", "L0");
				else if (chVal[0] == 2)
					ret = snprintf(buf, len, "%s", "L2");
				else if (chVal[0] == 3)
					ret = snprintf(buf, len, "%s", "L3");
			}
			break;
		case ADSL_GET_K_DS:
		case ADSL_GET_R_DS:
		case ADSL_GET_S_DS:
		case ADSL_GET_D_DS:
		case ADSL_GET_DELAY_DS:
			memset((void *)&pms, 0, sizeof(pms));
			if(d->xdsl_drv_get(RLCM_GET_DS_PMS_PARAM1, (void *)&pms, sizeof(pms))) {
				switch (id) {
					case ADSL_GET_K_DS:
						ret = snprintf(buf, len, "%d", pms.K);
						break;
					case ADSL_GET_R_DS:
						ret = snprintf(buf, len, "%d", pms.R);
						break;
					case ADSL_GET_S_DS:
						ret = snprintf(buf, len, "%d.%02d", pms.S/100, pms.S%100);
						break;
					case ADSL_GET_D_DS:
						ret = snprintf(buf, len, "%d", pms.D);
						break;
					case ADSL_GET_DELAY_DS:
						ret = snprintf(buf, len, "%d.%02d", pms.Delay/100, pms.Delay%100);
						break;
					default:break;
				}
			}
			break;
		case ADSL_GET_K_US:
		case ADSL_GET_R_US:
		case ADSL_GET_S_US:
		case ADSL_GET_D_US:
		case ADSL_GET_DELAY_US:
			memset((void *)&pms, 0, sizeof(pms));
			if(d->xdsl_drv_get(RLCM_GET_US_PMS_PARAM1, (void *)&pms, sizeof(pms))) {
				switch (id) {
					case ADSL_GET_K_US:
						ret = snprintf(buf, len, "%d", pms.K);
						break;
					case ADSL_GET_R_US:
						ret = snprintf(buf, len, "%d", pms.R);
						break;
					case ADSL_GET_S_US:
						ret = snprintf(buf, len, "%d.%02d", pms.S/100, pms.S%100);
						break;
					case ADSL_GET_D_US:
						ret = snprintf(buf, len, "%d", pms.D);
						break;
					case ADSL_GET_DELAY_US:
						ret = snprintf(buf, len, "%d.%02d", pms.Delay/100, pms.Delay%100);
						break;
					default:break;
				}
			}
			break;
		case ADSL_GET_CRC_DS:
		case ADSL_GET_FEC_DS:
		case ADSL_GET_ES_DS:
		case ADSL_GET_SES_DS:
		case ADSL_GET_UAS_DS:
		case ADSL_GET_LOS_DS:
			#if !SUPPORT_TR105
			if (ADSL_GET_LOS_DS==id) {
				ret = snprintf(buf, len, "--");
				break;
			}
			
			#endif
			memset((void *)&counter, 0, sizeof(counter));
			if(d->xdsl_drv_get(RLCM_GET_DS_ERROR_COUNT, (void *)&counter, sizeof(counter))) {
				unsigned long *cnt;
				cnt = (unsigned long *)&counter;
				ret = snprintf(buf, len, "%lu", cnt[id-ADSL_GET_CRC_DS]);
			}
			break;
		case ADSL_GET_CRC_US:
		case ADSL_GET_FEC_US:
		case ADSL_GET_ES_US:
		case ADSL_GET_SES_US:
		case ADSL_GET_UAS_US:
		case ADSL_GET_LOS_US:
			#if !SUPPORT_TR105		
			if (ADSL_GET_LOS_US==id) {
				ret = snprintf(buf, len, "--");
				break;
			}
			#endif
			memset((void *)&counter, 0, sizeof(counter));
			if(d->xdsl_drv_get(RLCM_GET_US_ERROR_COUNT, (void *)&counter, sizeof(counter))) {
				unsigned long *cnt;
				cnt = (unsigned long *)&counter;
				ret = snprintf(buf, len, "%lu", cnt[id-ADSL_GET_CRC_US]);
			} else {
				fprintf(stderr, "get us failed!!!!!\n\n");
			}
			break;
		case ADSL_GET_FULL_INIT:
		case ADSL_GET_FAILED_FULL_INIT:
		case ADSL_GET_LAST_LINK_DS:
		case ADSL_GET_LAST_LINK_US:
			{
				unsigned long lnk_counters[4];
				memset(lnk_counters, 0, sizeof(lnk_counters));				
				if(d->xdsl_drv_get(RLCM_GET_INIT_COUNT_LAST_LINK_RATE, (void *)lnk_counters, sizeof(lnk_counters))) {
					ret = snprintf(buf, len, "%lu", lnk_counters[id-ADSL_GET_FULL_INIT]);
				}				
			}
			break;
		case ADSL_GET_TX_FRAMES:				
			if(d->xdsl_drv_get(RLCM_GET_FRAME_COUNT, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d", intVal[0]);
			}			
			break;
		case ADSL_GET_RX_FRAMES:
			if(d->xdsl_drv_get(RLCM_GET_FRAME_COUNT, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d", intVal[1]);
			}			
			break;
		case ADSL_GET_SYNCHRONIZED_TIME:
			{
				unsigned int synchronized_time[3];
				if(d->xdsl_drv_get(RLCM_GET_DSL_ORHERS, synchronized_time, 3*sizeof(int))) {
					// jim according paget's advise the accuracy equation is : actual time= showtime - showtime/69;
					synchronized_time[0]=synchronized_time[0]-synchronized_time[0]/68;
					ret = snprintf(buf, len, "%d", synchronized_time[0]);
				}
			}
			break;
		case ADSL_GET_SYNCHRONIZED_NUMBER:
			{
				int synchronized_number;
				if (d->xdsl_drv_get(RLCM_GET_REHS_COUNT, &synchronized_number, sizeof(int)))
					ret = snprintf(buf, len, "%d", synchronized_number);
			}
			break;

#ifdef CONFIG_VDSL
		case DSL_GET_TRELLIS_DS:
			if(d->xdsl_msg_get_array(GetTrellis, msgval))
				ret = snprintf(buf, len, "%s", msgval[0]?"On":"Off");
			break;
		case DSL_GET_TRELLIS_US:	
			if(d->xdsl_msg_get_array(GetTrellis, msgval))
				ret = snprintf(buf, len, "%s", msgval[1]?"On":"Off");
			break;
		case DSL_GET_PHYR_DS:
			if(d->xdsl_msg_get_array(GetPhyR, msgval))
				ret = snprintf(buf, len, "%s", msgval[0]?"On":"Off");
			break;
		case DSL_GET_PHYR_US:
			if(d->xdsl_msg_get_array(GetPhyR, msgval))
				ret = snprintf(buf, len, "%s", msgval[1]?"On":"Off");
			break;
		case DSL_GET_N_DS:
			if(d->xdsl_msg_get_array(GetPmsPara_Nfec, msgval))
				ret = snprintf(buf, len, "%d", msgval[0]);
			break;
		case DSL_GET_N_US:
			if(d->xdsl_msg_get_array(GetPmsPara_Nfec, msgval))
				ret = snprintf(buf, len, "%d", msgval[1]);
			break;
		case DSL_GET_L_DS:
			if(d->xdsl_msg_get_array(GetPmsPara_L, msgval))
				ret = snprintf(buf, len, "%d", msgval[0]);
			break;
		case DSL_GET_L_US:
			if(d->xdsl_msg_get_array(GetPmsPara_L, msgval))
				ret = snprintf(buf, len, "%d", msgval[1]);
			break;
		case DSL_GET_INP_DS:
			if(d->xdsl_msg_get_array(GetPmsPara_INP, msgval))
				ret = snprintf(buf, len, "%d.%03d", msgval[0]/1000, msgval[0]%1000);
			break;
		case DSL_GET_INP_US:
			if(d->xdsl_msg_get_array(GetPmsPara_INP, msgval))
				ret = snprintf(buf, len, "%d.%03d", msgval[1]/1000, msgval[1]%1000);
			break;
		case DSL_GET_TPS:
			{
				int tpsmode=0;
				if(d->xdsl_msg_get_array(GetTpsMode, &tpsmode))
				{
					if(tpsmode==2) ret=snprintf(buf, len, "PTM");
					else if(tpsmode==4) ret=snprintf(buf, len, "ATM");
					else if(tpsmode==8) ret=snprintf(buf, len, "STM");
					else ret=snprintf(buf, len, "");
				}else
					ret=snprintf(buf, len, "");
			}
			break;
#endif /*CONFIG_VDSL*/

#ifdef CONFIG_DSL_VTUO
		case DSL_GET_PMD_MODE:
			{
				int mval=0;

				if( d->xdsl_msg_get(GetPmdMode,&mval) )
				{
					if(mval&MODE_ANSI) ret=snprintf(buf, len, "%s", "ANSI T1.413");
					else if(mval&MODE_ETSI) ret=snprintf(buf, len, "%s", "ETSI TS101 388");
					else if(mval&MODE_GDMT) ret=snprintf(buf, len, "%s", "G.992.1(G.dmt)");
					else if(mval&MODE_GLITE) ret=snprintf(buf, len, "%s", "G.992.2(G.Lite)");
					else if(mval&MODE_ADSL2) ret=snprintf(buf, len, "%s", "G.992.3(ADSL2)");
					else if(mval&MODE_ADSL2PLUS) ret=snprintf(buf, len, "%s", "G.992.5(ADSL2+)");
					else if(mval&MODE_VDSL2) ret=snprintf(buf, len, "%s", "G.993.2(VDSL2)");
					else ret=snprintf(buf, len, "%s", "NONE");
				}
			}
			break;
		case DSL_GET_LINK_TYPE:
			{
				int mval=0, pval=0;

				if( d->xdsl_msg_get(GetPmdMode,&mval) &&
					d->xdsl_msg_get(GetVdslProfile,&pval) )
				{
					if(mval&MODE_VDSL2)
					{
						const char *pfmt="%s";
						if(pval&VDSL2_PROFILE_8A) ret=snprintf(buf, len, pfmt, "8A");
						else if(pval&VDSL2_PROFILE_8B) ret=snprintf(buf, len, pfmt, "8B");
						else if(pval&VDSL2_PROFILE_8C) ret=snprintf(buf, len, pfmt, "8C");
						else if(pval&VDSL2_PROFILE_8D) ret=snprintf(buf, len, pfmt, "8D");
						else if(pval&VDSL2_PROFILE_12A) ret=snprintf(buf, len, pfmt, "12A");
						else if(pval&VDSL2_PROFILE_12B) ret=snprintf(buf, len, pfmt, "12B");
						else if(pval&VDSL2_PROFILE_17A) ret=snprintf(buf, len, pfmt, "17A");
						else if(pval&VDSL2_PROFILE_30A) ret=snprintf(buf, len, pfmt, "30A");
						else ret=snprintf(buf, len, "---");
					}else 
						ret=snprintf(buf, len, "---");
				}
			}
			break;
		case DSL_GET_LIMIT_MASK:
			if(d->xdsl_msg_get_array(GetLimitMask, msgval))
			{
				if(msgval[0]==0) ret = snprintf(buf, len, "D-32");
				else if(msgval[0]==1) ret = snprintf(buf, len, "D-48");
				else if(msgval[0]==2) ret = snprintf(buf, len, "D-64");
				else if(msgval[0]==3) ret = snprintf(buf, len, "D-128");
				else ret = snprintf(buf, len, "");
			}
			break;
		case DSL_GET_US0_MASK:
			if(d->xdsl_msg_get_array(GetUs0Mask, msgval))
			{
				if(msgval[0]==0) ret = snprintf(buf, len, "EU-32");
				else if(msgval[0]==1) ret = snprintf(buf, len, "EU-36");
				else if(msgval[0]==2) ret = snprintf(buf, len, "EU-40");
				else if(msgval[0]==3) ret = snprintf(buf, len, "EU-44");
				else if(msgval[0]==4) ret = snprintf(buf, len, "EU-48");
				else if(msgval[0]==5) ret = snprintf(buf, len, "EU-52");
				else if(msgval[0]==6) ret = snprintf(buf, len, "EU-56");
				else if(msgval[0]==7) ret = snprintf(buf, len, "EU-60");
				else if(msgval[0]==8) ret = snprintf(buf, len, "EU-64");
				else if(msgval[0]==9) ret = snprintf(buf, len, "EU-128");
				else ret = snprintf(buf, len, "");
			}
			break;
		case DSL_GET_UPBOKLE_DS:
			if(d->xdsl_msg_get_array(GetUPBOKLE, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[0]/10, msgval[0]%10 );
			break;
		case DSL_GET_UPBOKLE_US:
			if(d->xdsl_msg_get_array(GetUPBOKLE, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[1]/10, msgval[1]%10 );
			break;
		case DSL_GET_ACTUALCE:
			if(d->xdsl_msg_get_array(GetACTUALCE, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;



		case DSL_GET_SIGNAL_ATN_DS:
			if(d->xdsl_msg_get_array(GetSATN, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[0]/10, msgval[0]%10 );
			break;
		case DSL_GET_SIGNAL_ATN_US:
			if(d->xdsl_msg_get_array(GetSATN, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[1]/10, msgval[1]%10 );
			break;
		case DSL_GET_LINE_ATN_DS:
			if(d->xdsl_msg_get_array(GetLATN, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[0]/10, msgval[0]%10 );
			break;
		case DSL_GET_LINE_ATN_US:
			if(d->xdsl_msg_get_array(GetLATN, msgval))
				ret = snprintf(buf, len, "%u.%u", msgval[1]/10, msgval[1]%10 );
			break;
		case DSL_GET_RX_POWER_DS:
			if(d->xdsl_msg_get_array(GetRxPwr, msgval))
			{
				double tmpD;
				tmpD=(double)msgval[0]/10;
				ret = snprintf(buf, len, "%.1f", tmpD );
			}
			break;
		case DSL_GET_RX_POWER_US:
			if(d->xdsl_msg_get_array(GetRxPwr, msgval))
			{
				double tmpD;
				tmpD=(double)msgval[1]/10;
				ret = snprintf(buf, len, "%.1f", tmpD );
			}
			break;
		case DSL_GET_PMSPARA_I_DS:
			if(d->xdsl_msg_get_array(GetPmsPara_I, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_PMSPARA_I_US:
			if(d->xdsl_msg_get_array(GetPmsPara_I, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
		case DSL_GET_DATA_LPID_DS:
			if(d->xdsl_msg_get_array(GetDataLpId, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_DATA_LPID_US:
			if(d->xdsl_msg_get_array(GetDataLpId, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
		case DSL_GET_PTM_STATUS:
			if(d->xdsl_msg_get_array(GetPTMStatus, msgval))
			{
				if(msgval[0]==0) ret = snprintf(buf, len, "noDefect");
			}
			break;
		case DSL_GET_RA_MODE_DS:
			if(d->xdsl_msg_get_array(GetRAMode, msgval))
			{
				if(msgval[0]==-1) ret = snprintf(buf, len, "n/a" );
				else if(msgval[0]==0) ret = snprintf(buf, len, "Manual" );
				else if(msgval[0]==1) ret = snprintf(buf, len, "AdaptInit" );
				else if(msgval[0]==2) ret = snprintf(buf, len, "Dynamic" );
				else if(msgval[0]==3) ret = snprintf(buf, len, "SOS" );
			}
			break;
		case DSL_GET_RA_MODE_US:
			if(d->xdsl_msg_get_array(GetRAMode, msgval))
			{
				if(msgval[1]==-1) ret = snprintf(buf, len, "n/a" );
				else if(msgval[1]==0) ret = snprintf(buf, len, "Manual" );
				else if(msgval[1]==1) ret = snprintf(buf, len, "AdaptInit" );
				else if(msgval[1]==2) ret = snprintf(buf, len, "Dynamic" );
				else if(msgval[1]==3) ret = snprintf(buf, len, "SOS" );
			}
			break;
		case DSL_GET_RETX_OHRATE_DS:
			if(d->xdsl_msg_get_array(GetReTxOhRate, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_RETX_OHRATE_US:
			if(d->xdsl_msg_get_array(GetReTxOhRate, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
		case DSL_GET_RETX_FRAM_TYPE_DS:
			if(d->xdsl_msg_get_array(GetReTxFrmType, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_RETX_FRAM_TYPE_US:
			if(d->xdsl_msg_get_array(GetReTxFrmType, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
		case DSL_GET_RETX_ACTINP_REIN_DS:
			if(d->xdsl_msg_get_array(GetReTxActInpRein, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_RETX_ACTINP_REIN_US:
			if(d->xdsl_msg_get_array(GetReTxActInpRein, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
		case DSL_GET_RETX_H_DS:
			if(d->xdsl_msg_get_array(GetReTxH, msgval))
				ret = snprintf(buf, len, "%u", msgval[0]);
			break;
		case DSL_GET_RETX_H_US:
			if(d->xdsl_msg_get_array(GetReTxH, msgval))
				ret = snprintf(buf, len, "%u", msgval[1]);
			break;
#endif /*CONFIG_DSL_VTUO*/
#ifdef CONFIG_VDSL
		case DSL_GET_VECTOR_MODE:
			if(d->xdsl_msg_get_array(GetVectorMode, msgval))
				ret = snprintf(buf, len, "%s", msgval[0] ? "On" : "Off");
			break;
#endif
#ifdef LOOP_LENGTH_METER
		case ADSL_GET_LOOP_LENGTH_METER:
			if(d->xdsl_drv_get(RLCM_GET_LOOP_LENGTH_METER, (void *)&loop_length, sizeof(unsigned short))) {
				ret = snprintf(buf, len, "%u", loop_length);
			}
			break;
#endif
		default:
			break;
	}
	
	return ret;
}
		
int getAdslInfo(ADSL_GET_ID id, char *buf, int len)		
{
	XDSL_OP *d=xdsl_get_op(0);
	
	return _getAdslInfo( id, buf, len, d );
}
		
int getAdslDrvInfo(char *name, char *buff, int len)
{
	int idx, length;
	
	length = 0;
	
	for (idx=0; get_cmd_list[idx].cmd != NULL; idx++) {
		if (!strcmp(name, get_cmd_list[idx].cmd)) {
			length=getAdslInfo(get_cmd_list[idx].id, buff, len);
			break;
		}
	}
	
	if (length == 0)
		length = snprintf(buff, len, "%s", BLANK);
	
	return length;
}


#ifdef CONFIG_USER_XDSL_SLAVE
int getAdslSlvInfo(ADSL_GET_ID id, char *buf, int len)
{
	XDSL_OP *d=xdsl_get_op(1);
	
	return _getAdslInfo( id, buf, len, d );
}
		
int getAdslSlvDrvInfo(char *name, char *buff, int len)
{
	int idx, length;
	
	length = 0;
	
	for (idx=0; get_cmd_list[idx].cmd != NULL; idx++) {
		if (!strcmp(name, get_cmd_list[idx].cmd)) {
			length=getAdslSlvInfo(get_cmd_list[idx].id, buff, len);
			break;
		}
	}
	
	if (length == 0)
		length = snprintf(buff, len, "%s", BLANK);
	
	return length;
}
#endif /*CONFIG_USER_XDSL_SLAVE*/

