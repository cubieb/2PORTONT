/** \file Ve886_api.c
 * 
 *
 * This file contains the major api for upper application
 * 
 *
 * Copyright (c) 2013, Realtek Semiconductor, Inc.
 *
 */
#include<linux/delay.h>
#include "vp_api_cfg.h"
#include "ve886_api.h"
#include "Ve_profile.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH
#include "snd_define.h"
#else
#include "Slic_api.h" 	/* for COUNTRY_USA ... */
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
/* LT-API (line test) library header */
#include "lt_api.h"
#endif

#undef DEBUG_API 
//#define DEBUG_API 

#if defined(DEBUG_API)
#define DEBUG_API_PRINT() printk("%s(%d) line #%d\n",__FUNCTION__,__LINE__,pLine->ch_id);
#else
#define DEBUG_API_PRINT()
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
// for line test
extern int zarlinkeventflag;
#define THRESHOLD 300
#define LINE_TEST_TIMEOUT (3000)
typedef union {
    LtLineVInputType lineVInp;
    LtRingerInputType ringersInp;
    LtRohInputType rohInp;
    LtResFltInputType resFltInp;
    /* ... Add other input structs here */
} TestInputType;
#endif

VpStatusType Ve886SetRingCadenceProfile(RTKLineObj *pLine, uint8 ring_cad)
{
	VpStatusType status;
	VpProfilePtrType ring_cadence = DEF_LE886_RING_CAD_PROFILE;

	DEBUG_API_PRINT();

	//kevin su add for zsi
	#if 1
	switch (ring_cad)
	{
		case 0:
			ring_cadence = LE886_RING_CAD_STD;
			PRINT_MSG("set LE886_RING_CAD_STD\n");
			break;
		case 1:
			ring_cadence = LE886_RING_CAD_SHORT;
			PRINT_MSG("set LE886_RING_CAD_SHORT\n");
			break;
		default:
			ring_cadence = LE886_RING_CAD_STD;
			PRINT_MSG("set LE886_RING_CAD_STD\n");
			break;
	}
	#endif
	status = VpInitRing( pLine->pLineCtx, 
						 ring_cadence,
						 VP_PTABLE_NULL);

	if ( status == VP_STATUS_SUCCESS )
		pLine->pRing_cad_profile = ring_cadence;

	return status;
}

VpStatusType Ve886SetImpedenceCountry(RTKLineObj *pLine, uint8 country)
{
	VpStatusType status;
	VpProfilePtrType AC_profile;

	DEBUG_API_PRINT();

	PRINT_MSG(" <<<<<<<<< %s Country %d >>>>>>>>>\n",__FUNCTION__, country);

#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
	if (pLine->codec_type == VP_OPTION_WIDEBAND) 
		PRINT_MSG("line#%d VP_OPTION_WIDEBAND\n",pLine->ch_id);
	else
#endif
		PRINT_MSG("line#%d VP_OPTION_NARROWBAND\n",pLine->ch_id);

	switch(country)
	{
		case COUNTRY_AUSTRALIA:	
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_AU; //LE886_AC_FXS_RF14_AU;
			else
#endif
				AC_profile = AC_FXS_RF14_AU; //LE886_AC_FXS_RF14_AU;
			break;

		case COUNTRY_BE:	/* Belgium*/
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_BE; //LE886_AC_FXS_RF14_BE;
			else
#endif
				AC_profile = AC_FXS_RF14_BE; //LE886_AC_FXS_RF14_BE;
			break;
			
		case COUNTRY_CN:	/* China  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_CN; //LE886_AC_FXS_RF14_CN;
			else
#endif
				AC_profile = AC_FXS_RF14_CN; //LE886_AC_FXS_RF14_CN;
			break;
		
		case COUNTRY_GR:	/* German */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_DE; //LE886_AC_FXS_RF14_DE;
			else
#endif
				AC_profile = AC_FXS_RF14_DE; //LE886_AC_FXS_RF14_DE;
			break;

		case COUNTRY_FL:	/* Finland*/
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_FI; //LE886_AC_FXS_RF14_FI;
			else
#endif
				AC_profile = AC_FXS_RF14_FI; //LE886_AC_FXS_RF14_FI;
			break;
			
		case COUNTRY_FR:	/* France */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_FR; //LE886_AC_FXS_RF14_FR;
			else
#endif
				AC_profile = AC_FXS_RF14_FR; //LE886_AC_FXS_RF14_FR;
			break;
			
		case COUNTRY_IT:	/* Italy  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_IT; //LE886_AC_FXS_RF14_IT;
			else
#endif
				AC_profile = AC_FXS_RF14_IT; //LE886_AC_FXS_RF14_IT;
			break;			

		case COUNTRY_JP:	/* Japan  */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_JP; //LE886_AC_FXS_RF14_JP;
			else
#endif
				AC_profile = AC_FXS_RF14_JP; //LE886_AC_FXS_RF14_JP;
			break;

		case COUNTRY_SE:	/* Sweden */
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = AC_FXS_RF14_SE; //LE886_AC_FXS_RF14_SE;
			else
#endif
				AC_profile = AC_FXS_RF14_SE; //LE886_AC_FXS_RF14_SE;
			break;

		case COUNTRY_HK:
		case COUNTRY_TW:	
		case COUNTRY_UK:
		case COUNTRY_USA:
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = DEF_LE886_AC_PROFILE;
			else
#endif
				AC_profile = DEF_LE886_AC_PROFILE;
			PRINT_MSG("Set to default SLIC impedance 600 ohm.\n");
			break;

		default:
#ifdef CONFIG_RTK_VOIP_SLIC_ZARLINK_886_SERIES_WIDEBAND
			if (pLine->codec_type == VP_OPTION_WIDEBAND)
				AC_profile = DEF_LE886_AC_PROFILE;
			else
#endif
				AC_profile = DEF_LE886_AC_PROFILE;
			PRINT_Y("country wasn't defined. \
						Set to default SLIC impedance 600 ohm.\n");
			break;
	}

	status = VpConfigLine( pLine->pLineCtx, 
						   AC_profile,
						   VP_PTABLE_NULL,
						   VP_PTABLE_NULL );

	if ( status == VP_STATUS_SUCCESS ) {
		pLine->pAC_profile = AC_profile;
		pLine->AC_country  = country;
	}
	
	return status;
}

VpStatusType Ve886SetFxsAcProfileByBand(RTKLineObj *pLine, int pcm_mode)
{
    VpStatusType status;

	DEBUG_API_PRINT();

	status = Ve886SetImpedenceCountry(pLine, pLine->AC_country);
	return status;
    
}

VpStatusType Ve886SetIODir(RTKLineObj *pLine, VPIO IO, int bOut)
{
	//unsigned char reg = 0x54;
	unsigned char regdir = 0;
	//unsigned char len = 1;
	VpOptionLineIoConfigType io_cfg_ctrl;
	
	DEBUG_API_PRINT();

	#ifdef DEBUG_API
	printk("%s(%d)dir=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dir);
	#endif

	/* update DEV record */
	if (bOut==1)
		pLine->pDev->gpio_dir |= IO;
	else
		pLine->pDev->gpio_dir &= ~IO;
	
	/* prepare data for register write. data is channel depend */
	if (pLine->channelId == 0) {
		if (pLine->pDev->gpio_dir | 0x1)
			regdir |= 0x1;

		if (pLine->pDev->gpio_dir | 0x2)
			regdir |= 0x4;

	} else if (pLine->channelId == 1) {
		if (pLine->pDev->gpio_dir | 0x4)
			regdir |= 0x1;

		if (pLine->pDev->gpio_dir | 0x8)
			regdir |= 0x4;
	}

	//VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdir);
	io_cfg_ctrl.direction = 3;
	io_cfg_ctrl.outputType = 0;
	VpSetOption(pLine->pLineCtx, VP_NULL, VP_OPTION_ID_LINE_IO_CFG, &io_cfg_ctrl);	

	#ifdef DEBUG_API
	printk("%s(%d)dir=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dir);
	#endif
}

VpStatusType Ve886SetIOState(RTKLineObj *pLine, VPIO IO, int bHigh )
{
	unsigned int gpio_new_dat;
    
	DEBUG_API_PRINT();

	#ifdef DEBUG_API
	printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dat);
	#endif

	gpio_new_dat = pLine->pDev->gpio_dat & GPIO_NEW_MASK;

	if (bHigh==1)
		pLine->pDev->gpio_dat |= IO;
	else
		pLine->pDev->gpio_dat &= ~IO;

	#ifdef DEBUG_API
	printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLine->pDev->gpio_dat);
	#endif
}

VpStatusType Ve886GetIOState(RTKLineObj *pLine, VPIO IO, int *bHigh)
{
	//unsigned char reg=0x53;
	//unsigned char len=1;
	unsigned char regdat;
	VpLineIoAccessType IoCtrl;
	IoCtrl.direction = VP_IO_READ;
	IoCtrl.ioBits.data &= 0xFE;
	IoCtrl.ioBits.mask = 3;	

	DEBUG_API_PRINT();

	/* Read data from SLIC */
	//VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdat);
	//VpLineIoAccess(pLine->pLineCtx, &IoCtrl, 0);
	regdat = IoCtrl.ioBits.data & 0x01;

	if (pLine->channelId ==0)
		*bHigh = (regdat & IO ? 1:0);
	else
		*bHigh = ((regdat<<2) & IO ? 1:0);

	#ifdef DEBUG_API
	printk("IO=0x%x, regdat=%d, bHigh=%d\n",IO, regdat, *bHigh);
	#endif
}

VpStatusType Ve886UpdIOState(RTKLineObj *pLinein)
{
	//unsigned char reg = 0x52;
	unsigned int gpio_new_dat;
	unsigned int gpio_cur_dat;
	//unsigned char len=1;
	unsigned char regdat = 0x0;
	RTKLineObj *pLine;
	int line;
	VpLineIoAccessType IoCtrl;
	IoCtrl.direction = VP_IO_WRITE;
	IoCtrl.ioBits.data &= 0xFE;
	IoCtrl.ioBits.mask = 3;
	
	if (pLinein->pDev->dev_st != DEV_S_READY) 
		return;

	gpio_new_dat = pLinein->pDev->gpio_dat & GPIO_NEW_MASK;
	gpio_cur_dat = pLinein->pDev->gpio_dat & GPIO_CUR_MASK;
	gpio_cur_dat = gpio_cur_dat >> 16;

	#if 0
	DEBUG_API_PRINT(); 
	/* noisy! should be called every 10ms */

	printk("%s(%d)pLine=0x%08X, pLineCtx=0x%08X\n",
			__FUNCTION__,__LINE__,pLine, pLine->pLineCtx);
	printk("%s(%d)cur=0x%08X, new=0x%08X, channelId=%d\n",
			__FUNCTION__,__LINE__,gpio_cur_dat,gpio_new_dat,pLine->channelId);
	#endif

	for (line=0; line < pLinein->pDev->max_line; line++) {

		/* IO of Zarlink 886 is line depend */
		pLine = pLinein->pDev->pLine[line];

		/* update if dat was changed */
		if (gpio_cur_dat != gpio_new_dat) {

			if (pLine->channelId == 0) {
				regdat = gpio_new_dat & 0x03; 			/* bit1, bit0 */

				gpio_cur_dat &=  ~(0x3);      			/* clear bit1, bit0 */
				gpio_cur_dat |= (gpio_new_dat & 0x3); 	/* update new to current */

			} else if  (pLine->channelId == 1) {
				regdat = gpio_new_dat & 0x0C; 			/* bit3, bit2 */
				regdat = regdat >> 2;

				gpio_cur_dat &= ~(0xC);       			/* clear bit3, bit2 */
				gpio_cur_dat |= (gpio_new_dat & 0xC); 	/* update new to current */
			} else {
				printk("%s(%d)Error\n",__FUNCTION__,__LINE__);
			}

			if ( regdat )
				IoCtrl.ioBits.data = 1;
			else
				IoCtrl.ioBits.data = 0;

			/* write register */
			//VpRegisterReadWrite(pLine->pLineCtx, reg, &len, &regdat);
			VpLineIoAccess(pLine->pLineCtx, &IoCtrl, 0);

			pLinein->pDev->gpio_dat = ((gpio_cur_dat<<16) | gpio_new_dat);

			#ifdef DEBUG_API
			printk("%s(%d)dat=0x%08X\n",__FUNCTION__,__LINE__,pLinein->pDev->gpio_dat);
			#endif
		}
	}
}


VpStatusType Ve886StartMeter(RTKLineObj *pLine, uint16 hz, uint16 onTime, uint16 offTime, uint16 numMeters)		
{
	VpProfileDataType *MerterProfile;
	VpStatusType status;

	if ( hz == 12 )
		MerterProfile = &DEF_LE886_METERING_12K_PROFILE;
	else if ( hz == 16)
		MerterProfile = &DEF_LE886_METERING_16K_PROFILE;

	status = VpInitMeter(pLine->pLineCtx, MerterProfile);

	if ( status == VP_STATUS_SUCCESS ){
		status = VpStartMeter(pLine->pLineCtx, onTime, offTime, numMeters);
	}
	
	return status;
}

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
//#define LT_DEBUG_MESSAGE
int Ve886StartLineTest(VpLineCtxType *pLineCtx, VpDevCtxType *pDevCtx, int tID, void *data)
{
	TestInputType gTestInputs;
	LtTestAttributesType gTestAtts;
	LtTestTempType gTestTempData;
	LtTestResultType gTestResults;
	LtTestCtxType gTestCtx;
    LtTestStatusType ltStatus = LT_STATUS_RUNNING;
	VpEventType event;
	VpResultsType vpResults;
	//VpStatusType status;
	bool vpApiEventPending = FALSE;
	LtEventType ltEvent;	
    zarlinkeventflag = 0;
	int32 i = 0, runtime = 0;
    /* user handle value (can be any number)*/
    uint16 ltParamHandle = 0xAAAA; //0xBEEF; 
    LtTestIdType testId = tID; //LT_TID_RINGERS;
	int *result = (int *)data;
#ifdef LT_DEBUG_MESSAGE	
	printk("test id = %d\n", testId);
#endif
    /* 
     * if the LtTestAttributesType argument to LtStartTest is NULL
     * then the LT-API will use a set of default inputs,
     * criteria, and topology value
     */
    LtTestAttributesType *pTestAtts = &gTestAtts;
    VpMemSet(pTestAtts, 0, sizeof(LtTestAttributesType));
	VpMemSet(&gTestTempData, 0, sizeof(LtTestTempType));
	VpMemSet(&gTestResults, 0, sizeof(LtTestResultType));
	VpMemSet(&gTestCtx, 0, sizeof(LtTestCtxType));

TESTAGAIN:
    /* 
     * The following shows examples of the different ways that input
     * parameters can be passed to the LT-API. These are for example
     * purposes and should be modified to fit the customer's needs.
     */
	switch (testId) {
	
		case LT_TID_RINGERS:
			/* configure the input params */
			//gTestInputs.ringersInp.ringerTestType = LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE;

			gTestInputs.ringersInp.ringerTestType = LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE;
#if 1
			gTestInputs.ringersInp.vRingerTest = 56000;
			gTestInputs.ringersInp.freq = 20000;
			gTestInputs.ringersInp.renFactor = 1210000;
#else
			gTestInputs.ringersInp.vRingerTest = 56000;
			gTestInputs.ringersInp.freq = 20000;
			gTestInputs.ringersInp.renFactor = 7000;
#endif
			
			/* set the input params pointer to the global variable */
			//pTestAtts->inputs.pRingersInp = &gTestInputs.ringersInp;
			
			if ( runtime == 0 )
				pTestAtts->inputs.pRingersInp = VP_NULL;
			else if ( runtime == 1 )
				pTestAtts->inputs.pRingersInp = &gTestInputs.ringersInp;

			//pTestAtts->inputs.pRingersInp = &gTestInputs.ringersInp;
			//pTestAtts->inputs.pRingersInp = VP_NULL;
			
			//gTestAtts->inputs.pRingersInp = &gTestInputs.ringersInp;
				
			pTestAtts->criteria.pRingersCrt 		= VP_NULL;
			pTestAtts->topology.dummyPtr 			= VP_NULL;
			pTestAtts->topology.pVp886Topology 		= VP_NULL;
			pTestAtts->calFactors.dummyPtr 			= VP_NULL;
			break;
		case LT_TID_ROH:
			pTestAtts->inputs.pRohInp 			= VP_NULL;
			pTestAtts->criteria.pRohCrt 		= VP_NULL;
			pTestAtts->topology.dummyPtr 		= VP_NULL;
			pTestAtts->topology.pVp886Topology 	= VP_NULL;
			pTestAtts->calFactors.dummyPtr 		= VP_NULL;
	
			break;

		case LT_TID_LINE_V:
			pTestAtts->inputs.pLineVInp 		= VP_NULL;
			pTestAtts->criteria.pLintVCrt 		= VP_NULL;
			pTestAtts->topology.dummyPtr 		= VP_NULL;
			pTestAtts->topology.pVp886Topology 	= VP_NULL;
			pTestAtts->calFactors.dummyPtr 		= VP_NULL;
	
			break;

		case LT_TID_RES_FLT:
			pTestAtts->inputs.pResFltInp 		= VP_NULL;
			pTestAtts->criteria.pResFltCrt 		= VP_NULL;
			pTestAtts->topology.dummyPtr 		= VP_NULL;
			pTestAtts->topology.pVp886Topology 	= VP_NULL;
			pTestAtts->calFactors.dummyPtr 		= VP_NULL;
	
			break;
		case LT_TID_CAP:
			pTestAtts->inputs.pCapInp 			= VP_NULL;
			pTestAtts->criteria.pCapCrt 		= VP_NULL;
			pTestAtts->topology.dummyPtr 		= VP_NULL;
			pTestAtts->topology.pVp886Topology	= VP_NULL;
			pTestAtts->calFactors.dummyPtr		= VP_NULL;
			
			break;
		case LT_TID_DC_FEED_ST:
			pTestAtts->inputs.pDcFeedSTInp		= VP_NULL;
			pTestAtts->criteria.pDcFeedSTCrt	= VP_NULL;
			pTestAtts->topology.dummyPtr		= VP_NULL;
			pTestAtts->topology.pVp886Topology	= VP_NULL;
			break;
		default: 
			printk("Unsupported quick start testid %i\n", testId);
			return 0;	 
	}
	
    /* enable/disable runtime debug */
    //pTestAtts->ltDbgFlag = LT_DBG_ALL;
	

	/* Start the line test */
	ltStatus = LtStartTest(pLineCtx, 
							testId, 
							ltParamHandle, 
							&gTestAtts, 
							&gTestTempData, 
							&gTestResults, 
							&gTestCtx);

	if (ltStatus != LT_STATUS_RUNNING) {
		printk("Unable to start testId %i: ltStatus = %i\n", testId, ltStatus);
		return 0;
	}

	//mdelay(500);
#if 1
	for (i = 0; (i < LINE_TEST_TIMEOUT) && (ltStatus != LT_STATUS_DONE); i++) {
		mdelay(10);		
		vpApiEventPending = FALSE;
		ltStatus = VpApiTick(pDevCtx, &vpApiEventPending);

		if ( vpApiEventPending == FALSE )
				continue;
		//if ((ltStatus == VP_STATUS_SUCCESS) && ) 
		{
			while ((ltStatus != LT_STATUS_DONE) && VpGetEvent(pDevCtx, &event)) {
#if 0						
				printk("%s:%d category %d eventId %d\n"
					, __FUNCTION__, __LINE__
					, event.eventCategory
					, event.eventId);
#endif					
				//mdelay(10);
				if (event.hasResults) {
					ltStatus = VpGetResults(&event, &vpResults);
					if ( ltStatus != VP_STATUS_SUCCESS ){
						PRINT_R("VpGetResut fail\n : %i\n",
ltStatus);
						return;
					}
					ltEvent.pResult = &vpResults;
				} else {
					ltEvent.pResult = NULL;
				}
#if 0				
				if( ltEvent.pResult )
				{
					LtRingersResultType *RRT =	(LtRingersResultType*)ltEvent.pResult;
					printk("fltMask = 0x[%x]\n", RRT->fltMask);
					printk("measStatus = [%d]\n", RRT->measStatus);
					printk("ren = [%d]\n", RRT->ren);
					printk("rentg = [%d]\n", RRT->rentg);
					printk("renrg = [%d]\n", RRT->renrg);
					printk("ringerTestType = [%d]\n", RRT->ringerTestType);
				}
#endif				
				//mdelay(100);
				ltEvent.pVpEvent = &event;
				/* process the event */
				ltStatus = LtEventHandler(&gTestCtx, &ltEvent);
				switch (ltStatus) {
					case LT_STATUS_DONE:
#ifdef LT_DEBUG_MESSAGE							
						printk("LT-API test result status :%i\n", ltStatus);
#endif
						break;
					case LT_STATUS_RUNNING:
					case LT_STATUS_ABORTED:
#ifdef LT_DEBUG_MESSAGE							
						printk("LT-API test result status :%i\n", ltStatus);
#endif
						break;
					default:
						break;
					}
				//mdelay(10);
			}
		}
	}
#endif
	
	
	switch (testId) {
		case LT_TID_RINGERS:
#ifdef LT_DEBUG_MESSAGE
	printk("ren = [%li]\n", gTestResults.result.ringers.ren);
	
	printk("ringerTestType = [%d]\n", gTestTempData.vp886Temp.tempData.ringers.input.ringerTestType);
	printk("renFactor = [%d]\n", gTestTempData.vp886Temp.tempData.ringers.input.renFactor);
	printk("vRingerTest = [%d]\n", gTestTempData.vp886Temp.tempData.ringers.input.vRingerTest);
	printk("freq = [%d]\n", gTestTempData.vp886Temp.tempData.ringers.input.freq);
#endif

	if (gTestResults.result.ringers.ren >= THRESHOLD){
		zarlinkeventflag = 1;
		result[0] = 1;
		return 1;
	}else if ( (gTestResults.result.ringers.ren < THRESHOLD) && (runtime == 0)){
		runtime = 1;
		goto TESTAGAIN;
	}
	else if ( (gTestResults.result.ringers.ren < THRESHOLD) && (runtime == 1)){
		zarlinkeventflag = 1;
		result[0] = 0;
		return 0;
	}
			break;
		case LT_TID_ROH:
#ifdef LT_DEBUG_MESSAGE
			printk("fltMask = 0x[%x]\n", gTestResults.result.roh.fltMask);			
			printk("measStatus = 0x[%x]\n", gTestResults.result.roh.measStatus);			
			printk("rLoop1 = [%d]\n", gTestResults.result.roh.rLoop1);			
			printk("rLoop2 = [%d]\n", gTestResults.result.roh.rLoop2);
#endif
			result[0] = gTestResults.result.roh.fltMask;
			result[1] = gTestResults.result.roh.measStatus;
			result[2] = gTestResults.result.roh.rLoop1;
			result[3] = gTestResults.result.roh.rLoop2;	
			break;
		case LT_TID_LINE_V: 
#ifdef LT_DEBUG_MESSAGE
			printk("fltMask = 0x[%x]\n", gTestResults.result.lineV.fltMask);			
			printk("measStatus = 0x[%x]\n", gTestResults.result.lineV.measStatus);			
			printk("vAcDiff = [%d]\n", gTestResults.result.lineV.vAcDiff);
			printk("vAcRing = [%d]\n", gTestResults.result.lineV.vAcRing);				
			printk("vAcTip = [%d]\n", gTestResults.result.lineV.vAcTip);
			printk("vDcDiff = [%d]\n", gTestResults.result.lineV.vDcDiff);				
			printk("vDcRing = [%d]\n", gTestResults.result.lineV.vDcRing);
			printk("vDcTip = [%d]\n", gTestResults.result.lineV.vDcTip);				
#endif
			result[0] = gTestResults.result.lineV.vAcDiff;
			result[1] = gTestResults.result.lineV.vAcRing;
			result[2] = gTestResults.result.lineV.vAcTip;
			result[3] = gTestResults.result.lineV.vDcDiff;
			result[4] = gTestResults.result.lineV.vDcRing;
			result[5] = gTestResults.result.lineV.vDcTip;
			result[6] = gTestResults.result.lineV.fltMask;
			result[7] = gTestResults.result.lineV.measStatus;
			break;
		case LT_TID_RES_FLT:
#ifdef LT_DEBUG_MESSAGE	
			printk("fltMask = 0x[%x]\n", gTestResults.result.resFlt.fltMask);			
			printk("measStatus = 0x[%x]\n", gTestResults.result.resFlt.measStatus);
			printk("rGnd = [%d]\n", gTestResults.result.resFlt.rGnd);
			printk("rrg = [%d]\n", gTestResults.result.resFlt.rrg);
			printk("rtg = [%d]\n", gTestResults.result.resFlt.rtg);				
			printk("rtr = [%d]\n", gTestResults.result.resFlt.rtr);
#endif
			result[0] = gTestResults.result.resFlt.rGnd;
			result[1] = gTestResults.result.resFlt.rrg;
			result[2] = gTestResults.result.resFlt.rtg;			
			result[3] = gTestResults.result.resFlt.rtr;
			result[4] = gTestResults.result.resFlt.fltMask;
			result[5] = gTestResults.result.resFlt.measStatus;			
			break;
		case LT_TID_CAP:
#ifdef LT_DEBUG_MESSAGE			
			printk("fltMask = 0x[%x]\n", gTestResults.result.cap.fltMask);			
			printk("measStatus = 0x[%x]\n", gTestResults.result.cap.measStatus);			
			printk("crg = [%d]\n", gTestResults.result.cap.crg);
			printk("ctg = [%d]\n", gTestResults.result.cap.ctg);				
			printk("ctr = [%d]\n", gTestResults.result.cap.ctr);
#endif			
			result[0]	= gTestResults.result.cap.crg;
			result[1]	= gTestResults.result.cap.ctg;
			result[2]	= gTestResults.result.cap.ctr;
			result[3]	= gTestResults.result.cap.fltMask;
			result[4]	= gTestResults.result.cap.measStatus;
			break;
		case LT_TID_DC_FEED_ST:
#ifdef LT_DEBUG_MESSAGE			
			printk("fltMask = 0x[%x]\n", gTestResults.result.dcFeedST.fltMask);			
			printk("measStatus = 0x[%x]\n", gTestResults.result.dcFeedST.measStatus);			
			printk("iTestLoad = [%d]\n", gTestResults.result.dcFeedST.iTestLoad);
			printk("rTestLoad = [%d]\n", gTestResults.result.dcFeedST.rTestLoad);				
			printk("vTestLoad = [%d]\n", gTestResults.result.dcFeedST.vTestLoad);
#endif						
			result[0]	= gTestResults.result.dcFeedST.iTestLoad;
			result[1]	= gTestResults.result.dcFeedST.rTestLoad;			
			result[2]	= gTestResults.result.dcFeedST.vTestLoad;			
			result[3]	= gTestResults.result.dcFeedST.fltMask;					
			result[4]	= gTestResults.result.dcFeedST.measStatus;								
			break;
        default: 
            printk("Unsupported quick start testid %i\n", testId);
            return 0;    
    	}			
	zarlinkeventflag = 1;
	return 0;

}
#endif

/*****************************************************************/
#if 0
VpProfilePtrType Ve886RingProfile(uint8 profileId)
{
    VpProfilePtrType ring_profile = VP_PTABLE_NULL;                                                        
	PRINT_Y("%s(%d) Not support yet!\n",__FUNCTION__,profileId);
	return ring_profile;
}
#endif

#if 0
VpProfilePtrType Ve886AcProfile(uint8 profileId)
{
    VpProfilePtrType AC_profile = DEF_LE886_AC_PROFILE;

	PRINT_Y("%s(%d) Not support yet!\n",__FUNCTION__,profileId);
    return AC_profile;
}   
#endif

#ifdef DEBUG_API
int Ve886_ver(int deviceId)
{
	#if 1 // read revision
    unsigned char res[14]={0};
    int i;
    uint8 reg, len;
        
    reg = 0x73;
    len= 2;
    VpMpiCmd(deviceId, 0x3, reg, len, res);
    printk("Revision: ");
        
    for (i=0; i<len; i++)
        printk("\nbyte%d = 0x%x", i, res[i]);
    printk("\n");
	#endif

	return 0;
}
#endif


