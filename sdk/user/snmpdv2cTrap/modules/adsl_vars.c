
#include	<stdio.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<time.h>

#include	"ctypes.h"
#include	"error.h"
#include	"local.h"
#include	"adsl_vars.h"
#include	"mix.h"
#include	"mis.h"
#include	"asn.h"

#include "mib_tool.h"

#define ADSLATUCINVVENDORID_LEN			17
#define ADSLATUCINVVERSIONNUMBER_LEN		17

#define ADSLATURINVSERIALNUMBER_LEN		33
#define ADSLATURINVVENDORID_LEN			17
#define ADSLATURINVVERSIONNUMBER_LEN		17

struct adsl_mib {
	unsigned char adslLineCoding;	    // adslMIB.1.1.1.1
	
	char adslAtucInvVendorID[ADSLATUCINVVENDORID_LEN];  // adslMIB.1.1.2.1
	char adslAtucInvVersionNumber[ADSLATUCINVVERSIONNUMBER_LEN];

	char adslAturInvSerialNumber[ADSLATURINVSERIALNUMBER_LEN];  // adslMIB.1.1.3.1
	char adslAturInvVendorID[ADSLATURINVVENDORID_LEN];
	char adslAturInvVersionNumber[ADSLATURINVVERSIONNUMBER_LEN];
	int adslAturCurrSnrMgn;
	unsigned int adslAturCurrAtn;	
	int adslAturCurrOutputPwr;
	unsigned int adslAturCurrAttainableRate;
	
	unsigned int adslAturChanInterleaveDelay;		// adslMIB.1.1.5.1  // Mason Yu.  20130207.
	unsigned int adslAturChanCurrTxRate;
	unsigned int adslAturChanPrevTxRate;
	
	unsigned int adslAtucPerfLofs;		// adslMIB.1.1.6.1
	unsigned int adslAtucPerfLoss;
	unsigned int adslAtucPerfCurr15MinLofs;
	unsigned int adslAtucPerfCurr15MinLoss;
	unsigned int adslAtucPerfCurr1DayLofs;
	unsigned int adslAtucPerfCurr1DayLoss;

	unsigned int adslAturPerfLofs;		// adslMIB.1.1.7.1
	unsigned int adslAturPerfLoss;
	unsigned int adslAturPerfLprs;
	unsigned int adslAturPerfESs;
	unsigned int adslAturPerfCurr15MinLofs;
	unsigned int adslAturPerfCurr15MinLoss;
	unsigned int adslAturPerfCurr15MinLprs;
	unsigned int adslAturPerfCurr15MinESs;
	unsigned int adslAturPerfCurr1DayLofs;
	unsigned int adslAturPerfCurr1DayLoss;
	unsigned int adslAturPerfCurr1DayLprs;
	unsigned int adslAturPerfCurr1DayESs;	
	
	unsigned int adslAturChanReceivedBlks;  // adslMIB.1.1.11.1    // Mason Yu.  20130207.
	unsigned int adslAturChanTransmittedBlks; // 2
	unsigned int adslAturChanPerfCurr15MinReceivedBlks; // 8
	unsigned int adslAturChanPerfCurr15MinTransmittedBlks; // 9
	unsigned int adslAturChanPerfCurr1DayReceivedBlks; //13
	unsigned int adslAturChanPerfCurr1DayTransmittedBlks; // 14
	
	unsigned int adslAturThresh15MinESs;	// adslMIB.1.1.15.1	
	
    struct adsl_mib   *if_next;
 };

extern unsigned int Adsl_Interface_Idx(void);	// retrieve adsl interface id

static struct adsl_mib *adsl_mib_list = NULL;
static struct mib_oid_tbl adsl_mib_oid_tbl;
static unsigned int adsl_list_num;

static long adsl_last_scan = 0;

void Adsl_Mib_Get(unsigned int if_idx)
{
	struct adsl_mib *nnew;		
	/*
	 * free old list: 
	 */
	while (adsl_mib_list) {
	    struct adsl_mib   *old = adsl_mib_list;
	    adsl_mib_list = adsl_mib_list->if_next;
	    free(old);
	}
	
	nnew = (struct adsl_mib *) calloc(1, sizeof(struct adsl_mib));
	if (nnew == NULL)
	{
		adsl_mib_list = NULL;
		return;              /* alloc error */
	}

	adsl_mib_list = nnew;
	nnew->if_next = NULL;

// Mason Yu.  20130207
//#ifdef __uClinux__
#if	1
	// adslLineCoding
	nnew->adslLineCoding = 2;

	// adslAtucInvVendorID, adslAtucInvVersionNumber
	{
		Modem_Identification vMId;

		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)) {
			strncpy(nnew->adslAtucInvVendorID, (char *)&(vMId.ITU_VendorId.vendorCode), ADSLATUCINVVENDORID_LEN);
			snprintf(nnew->adslAtucInvVersionNumber, ADSLATUCINVVERSIONNUMBER_LEN, "%02X", vMId.ITU_StandardRevisionNbr);			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}

	// adslAturPhysTable 94.11.3, adslAturInvSerialNumber(1), adslAturInvVendorID(2), adslAturInvVersionNumber(3)
	{
		Modem_Identification vMId;

		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)) {
			snprintf(nnew->adslAturInvSerialNumber, ADSLATURINVSERIALNUMBER_LEN, "%02X", vMId.ITU_VendorId.vendorSpecific);			 
			strncpy(nnew->adslAturInvVendorID, (char *)&(vMId.ITU_VendorId.vendorCode), ADSLATURINVVENDORID_LEN);
			snprintf(nnew->adslAturInvVersionNumber, ADSLATURINVVERSIONNUMBER_LEN, "%02X", vMId.ITU_StandardRevisionNbr);			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}


	// adslAturCurrSnrMgn
	/*
	{
		int vInt;

		if(adsl_drv_get(RLCM_GET_SNR_MARGIN, (void *)&vInt, RLCM_GET_SNR_MARGIN_SIZE)) {
			nnew->adslAturCurrSnrMgn = vInt;
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	*/
	
	// adslAturPhysTable 94.11.3 ,adslAturCurrSnrMgn(4) ,adslAturCurrAtn(5)
	{
		Modem_FarEndLineOperData vFeld;

		if(adsl_drv_get(RLCM_MODEM_FAR_END_LINE_DATA_REQ, (void *)&vFeld, RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE)) {
			nnew->adslAturCurrSnrMgn = vFeld.noiseMarginUpstr;
			nnew->adslAturCurrAtn = vFeld.attenuationUpstr;		
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	// adslAturPhysTable 94.11.3 ,adslAturCurrOutputPwr(7) 
	{
		Modem_NearEndLineOperData vNeld;

		if(adsl_drv_get(RLCM_MODEM_NEAR_END_LINE_DATA_REQ, (void *)&vNeld, RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE)) {
			nnew->adslAturCurrOutputPwr = vNeld.outputPowerUpstr;				
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	// adslAturPhysTable 94.11.3 ,adslAturCurrAttainableRate(8) 
	{
		Modem_AttRate vAr;

		if(adsl_drv_get(RLCM_GET_ATT_RATE, (void *)&vAr, GET_LINE_RATE_SIZE)) {
			nnew->adslAturCurrAttainableRate =  vAr.upstreamRate;				
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	{
		Modem_LossData vLd;

		// adslAtucPerfLofs, adslAtucPerfLoss, adslAturPerfLofs, adslAturPerfLoss, adslAturPerfLprs, adslAturPerfESs
		if(adsl_drv_get(RLCM_GET_LOSS_DATA, (void *)&vLd, RLCM_GET_LOSS_DATA_SIZE)) {
			nnew->adslAtucPerfLofs = vLd.LossOfFrame_FE;
			nnew->adslAtucPerfLoss = vLd.LossOfSignal_FE;

			nnew->adslAturPerfLofs = vLd.LossOfFrame_NE;
			nnew->adslAturPerfLoss = vLd.LossOfSignal_NE;
			nnew->adslAturPerfLprs = vLd.LossOfPower_NE;
			nnew->adslAturPerfESs = vLd.ESs_NE;

		} else {			
			goto Adsl_Mib_Get_Fail;
		}


		// 15Min
		if(adsl_drv_get(RLCM_GET_15MIN_LOSS_DATA, (void *)&vLd, RLCM_GET_LOSS_DATA_SIZE)) {
			nnew->adslAtucPerfCurr15MinLofs = vLd.LossOfFrame_FE;
			nnew->adslAtucPerfCurr15MinLoss = vLd.LossOfSignal_FE;

			nnew->adslAturPerfCurr15MinLofs = vLd.LossOfFrame_NE;
			nnew->adslAturPerfCurr15MinLoss = vLd.LossOfSignal_NE;
			nnew->adslAturPerfCurr15MinLprs = vLd.LossOfPower_NE;
			nnew->adslAturPerfCurr15MinESs = vLd.ESs_NE;
		} else {			
			goto Adsl_Mib_Get_Fail;
		}

		// 1Day
		if(adsl_drv_get(RLCM_GET_1DAY_LOSS_DATA, (void *)&vLd, RLCM_GET_LOSS_DATA_SIZE)) {
			nnew->adslAtucPerfCurr1DayLofs = vLd.LossOfFrame_FE;
			nnew->adslAtucPerfCurr1DayLoss = vLd.LossOfSignal_FE;

			nnew->adslAturPerfCurr1DayLofs = vLd.LossOfFrame_NE;
			nnew->adslAturPerfCurr1DayLoss = vLd.LossOfSignal_NE;
			nnew->adslAturPerfCurr1DayLprs = vLd.LossOfPower_NE;
			nnew->adslAturPerfCurr1DayESs = vLd.ESs_NE;

		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}

	// adslAturThresh15MinESs
	{
		Modem_LossData vLd;

		if(adsl_drv_get(RLCM_GET_TRAP_THRESHOLD, (void *)&vLd, RLCM_GET_LOSS_DATA_SIZE)) {
			nnew->adslAturThresh15MinESs = vLd.ESs_NE;
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}

	// adslAturChanTable	94.1.1.5, adslAturChanInterleaveDelay(1)
	{
		Modem_PMSParm pms;

		if(adsl_drv_get(RLCM_GET_US_PMS_PARAM1, (void *)&pms, GET_PMS_SIZE)) {            
			nnew->adslAturChanInterleaveDelay = pms.Delay/100;  // 1			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	// adslAturChanTable	94.1.1.5, adslAturChanCurrTxRate(2)
	{
		Modem_LinkSpeed vLs;

		if(adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)) {			
			nnew->adslAturChanCurrTxRate =  vLs.upstreamRate;  // 2	
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	//adslAturChanPerfDataTable 94.1.1.11, adslAturChanReceivedBlks(1), adslAturChanTransmittedBlks(2)
	{
		Modem_DSLConfigStatus vStat; 

		if(adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturChanReceivedBlks = vStat.ReceiveBlocks;
			nnew->adslAturChanTransmittedBlks = vStat.TransmitBlocks;			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	//adslAturChanPerfDataTable 94.1.1.11, adslAturChanPerfCurr15MinReceivedBlks(8), adslAturChanPerfCurr15MinTransmittedBlks(9)
	{
		Modem_DSLConfigStatus vStat; 

		if(adsl_drv_get(RLCM_GET_DSL_STAT_15MIN, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturChanPerfCurr15MinReceivedBlks = vStat.ReceiveBlocks;
			nnew->adslAturChanPerfCurr15MinTransmittedBlks = vStat.TransmitBlocks;			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	
	//adslAturChanPerfDataTable 94.1.1.11, adslAturChanPerfCurr1DayReceivedBlks(13), adslAturChanPerfCurr1DayTransmittedBlks(14)
	{
		Modem_DSLConfigStatus vStat;

		if(adsl_drv_get(RLCM_GET_DSL_STAT_1DAY, (void *)&vStat, TR069_STAT_SIZE)) {			
			nnew->adslAturChanPerfCurr1DayReceivedBlks = vStat.ReceiveBlocks;
			nnew->adslAturChanPerfCurr1DayTransmittedBlks = vStat.TransmitBlocks;			
		} else {			
			goto Adsl_Mib_Get_Fail;
		}
	}
	return;
	
Adsl_Mib_Get_Fail:	
	// handle ioctl error
	while (adsl_mib_list) {
	    struct adsl_mib   *old = adsl_mib_list;
	    adsl_mib_list = adsl_mib_list->if_next;
	    free(old);
	}

	adsl_mib_list = NULL;

#else
	nnew->adslLineCoding = 2;

	strcpy(nnew->adslAtucInvVendorID, "ATU-C Vendor ID");
	strcpy(nnew->adslAtucInvVersionNumber, "ATU-C Version Num");

	strcpy(nnew->adslAturInvSerialNumber, "ATU-R Serial Num");
	strcpy(nnew->adslAturInvVendorID, "ATU-R Version ID");
	strcpy(nnew->adslAturInvVersionNumber, "ATU-R Version Num");
	nnew->adslAturCurrSnrMgn = -640;

	nnew->adslAtucPerfLofs = 100;
	nnew->adslAtucPerfLoss = 999;
	nnew->adslAtucPerfCurr15MinLofs = 777777;
	nnew->adslAtucPerfCurr15MinLoss = 8888888;
	nnew->adslAtucPerfCurr1DayLofs = 111;
	nnew->adslAtucPerfCurr1DayLoss = 11111;

	nnew->adslAturPerfLofs = 1;
	nnew->adslAturPerfLoss = 2;
	nnew->adslAturPerfLprs = 3;
	nnew->adslAturPerfESs = 4;
	nnew->adslAturPerfCurr15MinLofs = 8;
	nnew->adslAturPerfCurr15MinLoss = 9;
	nnew->adslAturPerfCurr15MinLprs = 10;
	nnew->adslAturPerfCurr15MinESs = 11;
	nnew->adslAturPerfCurr1DayLofs = 13;
	nnew->adslAturPerfCurr1DayLoss = 14;
	nnew->adslAturPerfCurr1DayLprs = 15;
	nnew->adslAturPerfCurr1DayESs = 16;

	nnew->adslAturThresh15MinESs = 900;
#endif	
}

void Adsl_Mib_Init(void)
{
	struct timeval now;
	gettimeofday(&now, 0);

	if((now.tv_sec - adsl_last_scan) > 10)	//  10 sec
	{
		unsigned int if_idx;

		adsl_last_scan = now.tv_sec;

		if_idx = Adsl_Interface_Idx();		
		if(if_idx != 0)
		{
			unsigned int oid_tbl_size;
			struct mib_oid *oid_ptr;
			
			Adsl_Mib_Get(if_idx);

			// only one adsl interface is support now
			adsl_list_num = 1;

			// create oid table
			free_mib_tbl(&adsl_mib_oid_tbl);
			
			oid_tbl_size = adsl_list_num * (ADSLLINETBLNUM  + ADSLATUCPHYSTBLNUM + ADSLATURPHYSTBLNUM + ADSLATURCHANTABLENUM + ADSLATUCPERFDATATBLNUM + ADSLATURPERFDATATBLNUM + ADSLATURCHANPERFDATATABLENUM+ ADSLLINEALARMCONFTBLNUM);
			create_mib_tbl(&adsl_mib_oid_tbl,  oid_tbl_size, 14);	// mib-2.10.94.1.1.15.1

			oid_ptr = adsl_mib_oid_tbl.oid;

			// adslLineTable 1.1 (adslLineCoding)
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLLINETBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLLINECODING;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			

			// adslAtucPhysTable 1.2 (adslAtucInvVendorID, adslAtucInvVersionNumber)
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCINVVENDORID;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCINVVERSIONNUMBER;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			

			// adslAturPhysTable 1.3 (adslAturInvSerialNumber, adslAturInvVendorID, adslAturInvVersionNumber, adslAturCurrSnrMgn)
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURINVSERIALNUMBER;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURINVVENDORID;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURINVVERSIONNUMBER;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCURRSNRMGN;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			// Mason Yu.  20130207. add 4 items
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCURRATN;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCURROUTPUTPWR;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPHYSTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCURRATTAINABLERATE;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			// Mason Yu.  20130207
			// adslAturChanTable 1.5
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANINTERLEAVEDELAY;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANCURRTXRATE;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANTABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANPREVTXRATE;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			// adslAtucPerfDataTable 1.6 (adslAtucPerfLofs, adslAtucPerfLoss, adslAtucPerfCurr15MinLofs, adslAtucPerfCurr15MinLoss, adslAtucPerfCurr1DayLofs, adslAtucPerfCurr1DayLoss)
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFCURR15MINLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFCURR15MINLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFCURR1DAYLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATUCPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATUCPERFCURR1DAYLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			

			// adslAturPerfDataTable 1.7 (adslAturPerfLofs, adslAturPerfLoss, adslAturPerfLprs, adslAturPerfESs, adslAturPerfCurr15MinLofs, adslAturPerfCurr15MinLoss, adslAturPerfCurr15MinLprs, adslAturPerfCurr15MinESs, adslAturPerfCurr1DayLofs, adslAturPerfCurr1DayLoss, adslAturPerfCurr1DayLprs, adslAturPerfCurr1DayESs)
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFLPRS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFESS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR15MINLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR15MINLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR15MINLPRS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR15MINESS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR1DAYLOFS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR1DAYLOSS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR1DAYLPRS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURPERFDATATBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURPERFCURR1DAYESS;
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			
			// Mason Yu.  20130207
			// adslAturChanPerfDataTable 1.11()			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;  
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANRECEIVEDBLKS;   // 1
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANTRANSMITTEDBLKS;  // 2
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANPERFCURR15MINRECEIVEDBLKS; //8
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANPERFCURR15MINTRANSMITTEDBLKS; //9
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANPERFCURR1DAYRECEIVEDBLKS; //13
			oid_ptr->name[3] = if_idx;
			oid_ptr++;
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLATURCHANPERFDATATABLE;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURCHANPERFCURR1DAYTRANSMITTEDBLKS; //14
			oid_ptr->name[3] = if_idx;
			oid_ptr++;			
		
			// adslLineAlarmConfProfileTable 1.15 ()
			oid_ptr->length = 4;
			oid_ptr->name[0] = ADSLLINEALARMCONFTBL;
			oid_ptr->name[1] = 1;
			oid_ptr->name[2] = ADSLATURTHRESH15MINESS;
			oid_ptr->name[3] = if_idx;	// length
		}	
	}
}

static	AsnIdType	adslRetrieveMibValue (unsigned int mibIdx)
{
	struct mib_oid * oid_ptr;

	if(adsl_mib_list == NULL)
	{
		return ((AsnIdType) 0);
	}

	oid_ptr = adsl_mib_oid_tbl.oid;

	oid_ptr += mibIdx;

	if(oid_ptr->name[0] == ADSLLINETBL)
	{
		if (oid_ptr->name[2] == ADSLLINECODING)
		{
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, adsl_mib_list->adslLineCoding);	
		}
	}
	else
	if(oid_ptr->name[0] == ADSLATUCPHYSTBL)
	{
		if (oid_ptr->name[2] == ADSLATUCINVVENDORID)
		{
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (adsl_mib_list->adslAtucInvVendorID), 
				(AsnLengthType) strlen ((char *) (adsl_mib_list->adslAtucInvVendorID)));
		}
		else
		if (oid_ptr->name[2] == ADSLATUCINVVERSIONNUMBER)
		{
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (adsl_mib_list->adslAtucInvVersionNumber), 
				(AsnLengthType) strlen ((char *) (adsl_mib_list->adslAtucInvVersionNumber)));
		}
	}
	else
	if(oid_ptr->name[0] == ADSLATURPHYSTBL)
	{
		if (oid_ptr->name[2] == ADSLATURINVSERIALNUMBER)
		{
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (adsl_mib_list->adslAturInvSerialNumber), 
				(AsnLengthType) strlen ((char *) (adsl_mib_list->adslAturInvSerialNumber)));
		}
		else
		if (oid_ptr->name[2] == ADSLATURINVVENDORID)
		{
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (adsl_mib_list->adslAturInvVendorID), 
				(AsnLengthType) strlen ((char *) (adsl_mib_list->adslAturInvVendorID)));
		}
		else
		if (oid_ptr->name[2] == ADSLATURINVVERSIONNUMBER)
		{
			return asnOctetString (asnClassUniversal, (AsnTagType) 4, 
				(CBytePtrType) (adsl_mib_list->adslAturInvVersionNumber), 
				(AsnLengthType) strlen ((char *) (adsl_mib_list->adslAturInvVersionNumber)));
		}
		else
		if (oid_ptr->name[2] == ADSLATURCURRSNRMGN)
		{
			return asnIntl (asnClassUniversal, (AsnTagType) 2, adsl_mib_list->adslAturCurrSnrMgn);	
		}
		// Mason Yu.  20130207
		if (oid_ptr->name[2] == ADSLATURCURRATN)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturCurrAtn);
		}
		if (oid_ptr->name[2] == ADSLATURCURROUTPUTPWR)
		{
			return asnIntl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturCurrOutputPwr);
		}
		if (oid_ptr->name[2] == ADSLATURCURRATTAINABLERATE)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturCurrAttainableRate);
		}
	}
	// Mason Yu.  20130207
	else
	if(oid_ptr->name[0] == ADSLATURCHANTABLE) // 94.1.1.5
	{
		if (oid_ptr->name[2] == ADSLATURCHANINTERLEAVEDELAY) // 1
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanInterleaveDelay);	
		}
		if (oid_ptr->name[2] == ADSLATURCHANCURRTXRATE) // 2
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanCurrTxRate);	
		}
		if (oid_ptr->name[2] == ADSLATURCHANPREVTXRATE) // 3
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanPrevTxRate);	
		}
	}
	else
	if(oid_ptr->name[0] == ADSLATUCPERFDATATBL)
	{
		if (oid_ptr->name[2] == ADSLATUCPERFLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAtucPerfLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATUCPERFLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAtucPerfLoss);	
		}
		else
		if (oid_ptr->name[2] == ADSLATUCPERFCURR15MINLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAtucPerfCurr15MinLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATUCPERFCURR15MINLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAtucPerfCurr15MinLoss);	
		}
		else
		if (oid_ptr->name[2] == ADSLATUCPERFCURR1DAYLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAtucPerfCurr1DayLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATUCPERFCURR1DAYLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAtucPerfCurr1DayLoss);	
		}
	}
	else
	if(oid_ptr->name[0] == ADSLATURPERFDATATBL)
	{

		if (oid_ptr->name[2] == ADSLATURPERFLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAturPerfLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAturPerfLoss);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFLPRS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAturPerfLprs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFESS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 1, adsl_mib_list->adslAturPerfESs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR15MINLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr15MinLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR15MINLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr15MinLoss);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR15MINLPRS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr15MinLprs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR15MINESS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr15MinESs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR1DAYLOFS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr1DayLofs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR1DAYLOSS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr1DayLoss);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR1DAYLPRS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr1DayLprs);	
		}
		else
		if (oid_ptr->name[2] == ADSLATURPERFCURR1DAYESS)
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturPerfCurr1DayESs);	
		}
	}
	// Mason Yu.  20130207
	else
	if(oid_ptr->name[0] == ADSLATURCHANPERFDATATABLE)  // 94.1.1.11
	{
		if (oid_ptr->name[2] == ADSLATURCHANRECEIVEDBLKS) // 1
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanReceivedBlks);
		}
		if (oid_ptr->name[2] == ADSLATURCHANTRANSMITTEDBLKS) // 2
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanTransmittedBlks);
		}
		if (oid_ptr->name[2] == ADSLATURCHANPERFCURR15MINRECEIVEDBLKS) // 8
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanPerfCurr15MinReceivedBlks);
		}
		if (oid_ptr->name[2] == ADSLATURCHANPERFCURR15MINTRANSMITTEDBLKS) // 9
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanPerfCurr15MinTransmittedBlks);
		}
		if (oid_ptr->name[2] == ADSLATURCHANPERFCURR1DAYRECEIVEDBLKS) // 13
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanPerfCurr1DayReceivedBlks);
		}
		if (oid_ptr->name[2] == ADSLATURCHANPERFCURR1DAYTRANSMITTEDBLKS) // 14
		{
			return asnUnsl (asnClassApplication, (AsnTagType) 2, adsl_mib_list->adslAturChanPerfCurr1DayTransmittedBlks);
		}
	}
	else
	if(oid_ptr->name[0] == ADSLLINEALARMCONFTBL)
	{
		if (oid_ptr->name[2] == ADSLATURTHRESH15MINESS)
		{
			return asnUnsl (asnClassUniversal, (AsnTagType) 2, adsl_mib_list->adslAturThresh15MinESs);
		}
	}

	return ((AsnIdType) 0);
}

static	MixStatusType	adslRelease (MixCookieType cookie)
{
	cookie = cookie;
	return (smpErrorGeneric);
}

static	MixStatusType adslCreate (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	//printf("ifaceCreate ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;
	return (smpErrorGeneric);
}

static	MixStatusType	adslDestroy (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	//printf("ifaceDestroy ()\n");    
	cookie = cookie;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	adslGet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen)
{
	unsigned int idx;

	Adsl_Mib_Init();

	cookie = cookie;
	if(snmp_oid_get(&adsl_mib_oid_tbl, name, namelen, &idx))
	{
		return adslRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixStatusType	adslSet (MixCookieType cookie, MixNamePtrType name, MixLengthType namelen, AsnIdType asn)
{
	unsigned int idx;

	cookie = cookie;
	name = name;
	namelen = namelen;
	asn = asn;

	if(snmp_oid_get(&adsl_mib_oid_tbl, name, namelen, &idx))
	{	
		struct mib_oid * oid_ptr;

		if(adsl_mib_list == NULL)
		{
			return (smpErrorReadOnly);
		}

		oid_ptr = adsl_mib_oid_tbl.oid;

		oid_ptr += idx;

		if(oid_ptr->name[0] == ADSLLINEALARMCONFTBL)
		{
			if (oid_ptr->name[2] == ADSLATURTHRESH15MINESS)
			{
				if(asnType (asn) == asnTypeInteger)
				{
#ifdef __uClinux__
					// adslAturThresh15MinESs
					{
						Modem_LossData vLd;

						vLd.ESs_NE = asnNumber (asnValue (asn), asnLength (asn));

						if(adsl_drv_get(RLCM_SET_TRAP_THRESHOLD, (void *)&vLd, RLCM_GET_LOSS_DATA_SIZE)) {
							return (smpErrorNone);
						} else {
							return (smpErrorGeneric);					
						}
					}
#else				
					adsl_mib_list->adslAturThresh15MinESs = asnNumber (asnValue (asn), asnLength (asn));
					
					return (smpErrorNone);
#endif					
				}
				else
				{
					return (smpErrorGeneric);					
				}
			}
		}
	}

	return (smpErrorReadOnly);
}

static	AsnIdType	adslNext (MixCookieType cookie, MixNamePtrType name, MixLengthPtrType namelenp)
{
	unsigned int idx = 0;

	Adsl_Mib_Init();
	
#if 0
	printf("adslNext %d\n", *namelenp);

	
	while(idx < *namelenp)
	{
		printf("adslNext %d %d\n",idx, *name++);		
		idx++;
	}
#endif

	cookie = cookie;
	if(snmp_oid_getnext(&adsl_mib_oid_tbl, name, *namelenp, &idx))
	{
		struct mib_oid * oid_ptr = adsl_mib_oid_tbl.oid;
		oid_ptr += idx;

		memcpy((unsigned char *)name, oid_ptr->name, oid_ptr->length);
		*namelenp = oid_ptr->length;
	
		return adslRetrieveMibValue(idx);
	}

	return ((AsnIdType) 0);
}

static	MixOpsType	adslOps = {

			adslRelease,
			adslCreate,
			adslDestroy,
			adslNext,
			adslGet,
			adslSet

			};


CVoidType		adslInit (void)
{
	(void) misExport ((MixNamePtrType) "\53\6\1\2\1\12\136\1\1",
		(MixLengthType) 9, & adslOps, (MixCookieType) 0);
}



