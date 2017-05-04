/*
  *   Module to access xdsl driver
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: uc_xdsl.c,v 1.1 2012/07/09 07:57:29 jiunming Exp $
  */
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include "uc_adslif.h"
#include "uc_xdsl.h"
#include "uc_mib.h"


static char adslDevice[] = "/dev/adsl0";
static FILE* adslFp = NULL;

//--------------------------------------------------------
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) 
	{
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

static char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) 
	{
		obcif_arg	myarg;
		myarg.argsize = (int) len;
		myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) 
		{
			//printf("ADSL ioctl failed! id=%x\n",id );
			close_adsl_drv();
			return 0;
		};

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}

#if 0
#define ADSL_DRV_RETURN_LEN	512
void test_adsl_ver(void)
{
	char rValueAdslDrv[ADSL_DRV_RETURN_LEN];
	char buf[100]={0};
	int len=100, ret;

	printf( "RLCM_GET_DRIVER_VERSION=%d\n", RLCM_GET_DRIVER_VERSION );
	printf( "RLCM_DRIVER_VERSION_SIZE=%d\n", RLCM_DRIVER_VERSION_SIZE );

	if(adsl_drv_get(RLCM_GET_DRIVER_VERSION, (void *)rValueAdslDrv, RLCM_DRIVER_VERSION_SIZE)) 
	{
		ret = snprintf(buf, len, "%s", rValueAdslDrv);
	}
	buf[ret]=0;
	printf( "test_adsl_ver()=%s\n", buf );
}
#endif


#ifdef CONFIG_VDSL
/*pval: must be an int[4]-arrary pointer*/
static char dsl_msg(unsigned int id, int msg, int *pval)
{
	MSGTODSL msg2dsl;
	char ret=0;

	msg2dsl.message=msg;
	msg2dsl.intVal=pval;
	ret=adsl_drv_get(id, &msg2dsl, sizeof(MSGTODSL));

	return ret;
}

static char dsl_msg_set_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_msg(RLCM_UserSetDslData, msg, pval);
	}
	return ret;
}

static char dsl_msg_set(int msg, int val)
{
	int tmpint[4];
	char ret=0;

	tmpint[0]=val;
	ret=dsl_msg_set_array(msg, tmpint);
	return ret;
}

static char dsl_msg_get_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_msg(RLCM_UserGetDslData, msg, pval);
	}
	return ret;
}

static char dsl_msg_get(int msg, int *pval)
{
	int tmpint[4];
	char ret=0;

	if(pval)
	{
		ret=dsl_msg_get_array(msg, tmpint);
		if(ret) *pval=tmpint[0];
	}
	return ret;
}
#endif /*#CONFIG_VDSL*/

static char tone[64];
static int setupDsl(void)
{
	unsigned char init_line, olr;
	unsigned short dsl_mode;
	int val;
	int ret=1;

	// check INIT_LINE
	if (uc_mib_get(UC_MIB_INIT_LINE, (void *)&init_line) != 0)
	{
		if (init_line == 1)
		{
#ifdef CONFIG_VDSL
			unsigned int mode;
			unsigned short profile;

			//disable modem,20130203
			printf("xDSL disable modem\n");
			adsl_drv_get(RLCM_PHY_DISABLE_MODEM, NULL, 0);

			//Pmd mode
			mode=0;
			uc_mib_get(UC_MIB_ADSL_MODE, (void *)&dsl_mode);
			if(dsl_mode&ADSL_MODE_GDMT)		mode |= MODE_GDMT;
			if(dsl_mode&ADSL_MODE_GLITE)	mode |= MODE_GLITE;
			if(dsl_mode&ADSL_MODE_ADSL2)	mode |= MODE_ADSL2;
			if(dsl_mode&ADSL_MODE_ADSL2P)	mode |= MODE_ADSL2PLUS;
			if(dsl_mode&ADSL_MODE_VDSL2)	mode |= MODE_VDSL2;

			if(dsl_mode&ADSL_MODE_ANXB) 
			{
				// Annex B
				mode |= MODE_ANX_B;
				if(dsl_mode&ADSL_MODE_T1413)	mode |= MODE_ETSI;
			}else{
				// Annex A
				mode |= MODE_ANX_A;
				if(dsl_mode&ADSL_MODE_T1413)	mode |= MODE_ANSI;
				if(dsl_mode&ADSL_MODE_ANXL)		mode |= MODE_ANX_L;
				if(dsl_mode&ADSL_MODE_ANXM)		mode |= MODE_ANX_M;
			}
			printf("SetPmdMode=0x%08x (dsl_mode=0x%04x)\n",  mode, dsl_mode);
			dsl_msg_set(SetPmdMode, mode);

			//vdsl2 profile
			uc_mib_get(UC_MIB_VDSL2_PROFILE, (void *)&profile);
			printf("SetVdslProfile=0x%08x\n", (unsigned int)profile);
			dsl_msg_set(SetVdslProfile, (unsigned int)profile);


			// G.INP,	 default by dsplib
#ifdef ENABLE_ADSL_MODE_GINP
			val=0;
			if (dsl_mode & ADSL_MODE_GINP)	// G.INP
				val=3;
			printf("SetGInp=0x%08x\n", val);	
			dsl_msg_set(SetGInp, val);
#endif /*ENABLE_ADSL_MODE_GINP*/

			// set OLR Type
			uc_mib_get(UC_MIB_ADSL_OLR, (void *)&olr);
			val = (int)olr;
			// SRA (should include bitswap)
			if(val == 2) val = 3;
			printf("SetOlr=0x%08x (olr=0x%02x)\n", val, olr);	
			dsl_msg_set(SetOlr, val);

			// Start handshaking; should be the last command
			mode=0;
			adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&mode, 4);


			//enable modem,20130203
			printf("xDSL enable modem\n");
			adsl_drv_get(RLCM_PHY_ENABLE_MODEM, NULL, 0);

			ret = 1;
#else /*CONFIG_VDSL*/
			char mode[3], inp;
			int xmode,adslmode, axB, axM, axL;

			// start adsl
			uc_mib_get(UC_MIB_ADSL_MODE, (void *)&dsl_mode);

			adslmode=(int)(dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT));	// T1.413 & G.dmt
			#if 0
			adsl_drv_get(RLCM_PHY_START_MODEM, (void *)&adslmode, 4);
			#endif

			if (dsl_mode & ADSL_MODE_ANXB) {	// Annex B
				axB = 1;
				axL = 0;
				axM = 0;
			}
			else {	// Annex A
				axB = 0;
				if (dsl_mode & ADSL_MODE_ANXL)	// Annex L
					axL = 3; // Wide-Band & Narrow-Band Mode
				else
					axL = 0;
				if (dsl_mode & ADSL_MODE_ANXM)	// Annex M
					axM = 1;
				else
					axM = 0;
			}

			adsl_drv_get(RLCM_SET_ANNEX_B, (void *)&axB, 4);
			adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&axL, 4);
			adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&axM, 4);

#ifdef ENABLE_ADSL_MODE_GINP
			if (dsl_mode & ADSL_MODE_GINP)	// G.INP
				xmode = DSL_FUNC_GINP;
			else
				xmode = 0;
			adsl_drv_get(RLCM_SET_DSL_FUNC, (void *)&xmode, 4);
#endif

			xmode=0;
			if (dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT))
				xmode |= 1;	// ADSL1
			if (dsl_mode & ADSL_MODE_ADSL2)
				xmode |= 2;	// ADSL2
			if (dsl_mode & ADSL_MODE_ADSL2P)
				xmode |= 4;	// ADSL2+
			adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

			// set OLR Type
			uc_mib_get(UC_MIB_ADSL_OLR, (void *)&olr);

			val = (int)olr;
			if (val == 2) // SRA (should include bitswap)
				val = 3;

			adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&val, 4);

			// set Tone mask
			uc_mib_get(UC_MIB_ADSL_TONE, (void *)tone);
			adsl_drv_get(RLCM_LOADCARRIERMASK, (void *)tone, GET_LOADCARRIERMASK_SIZE);

			uc_mib_get(UC_MIB_ADSL_HIGH_INP, (void *)&inp);
			xmode = inp;
			adsl_drv_get(RLCM_SET_HIGH_INP, (void *)&xmode, 4);

			// new_hibrid
			uc_mib_get(UC_MIB_DEVICE_TYPE, (void *)&inp);
			xmode = inp;
			switch(xmode) {
				case 1:
					xmode = 1052;
					break;
				case 2:
					xmode = 2099;
					break;
				case 3:
					xmode = 3099;
					break;
				case 4:
					xmode = 4052;
					break;
				case 5:
					xmode = 5099;
					break;
				case 9:
					xmode = 9099;
					break;
				default:
					xmode = 9099;
			}
			adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
			// Start handshaking; should be the last command.
			adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&adslmode, 4);
			ret = 1;
#endif /*CONFIG_VDSL*/
		}
		else
			ret = 0;
	}
	else
		ret = -1;

#ifdef FIELD_TRY_SAFE_MODE
	unsigned char mode;
	SafeModeData vSmd;

	memset((void *)&vSmd, 0, sizeof(vSmd));
	uc_mib_get(UC_MIB_ADSL_FIELDTRYSAFEMODE, (void *)&mode);
	vSmd.FieldTrySafeMode = (int)mode;
	uc_mib_get(UC_MIB_ADSL_FIELDTRYTESTPSDTIMES, (void *)&vSmd.FieldTryTestPSDTimes);
	uc_mib_get(UC_MIB_ADSL_FIELDTRYCTRLIN, (void *)&vSmd.FieldTryCtrlIn);
	adsl_drv_get(RLCM_SET_SAFEMODE_CTRL, (void *)&vSmd, SAFEMODE_DATA_SIZE);
#endif

	return ret;
}

static int startDsl()
{
	unsigned char init_line;
	unsigned short dsl_mode;
	int adslmode;
	int ret;
	
#ifdef CONFIG_VDSL
	//enable/disable dsl log
	system("/bin/adslctrl debug 9");
#endif /*CONFIG_VDSL*/

	
	ret = 1;
	if (uc_mib_get(UC_MIB_INIT_LINE, (void *)&init_line) != 0) {
		if (init_line == 1) {
			// start adsl
		  #ifdef CONFIG_VDSL
			adslmode=0;
		  #else
			uc_mib_get(UC_MIB_ADSL_MODE, (void *)&dsl_mode);
			adslmode=(int)(dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT));	// T1.413 & G.dmt
		  #endif /*CONFIG_VDSL*/
			adsl_drv_get(RLCM_PHY_START_MODEM, (void *)&adslmode, 4);
			ret = setupDsl();
		}
		else
			ret = 0;
	}
	else
		ret = -1;
	return ret;
}

/**uc_xdsl wrapper api********************************************************/
char uc_xdsl_drv_get(unsigned int id, void *data, unsigned int len)
{
#ifdef CONFIG_VDSL
	DEF_MSGTODSL *dm;
	MSGTODSL m;

	if( (id==RLCM_UserSetDslData)|| (id==RLCM_UserGetDslData) )
	{
		//printf( "id=RLCM_UserSetDslData/RLCM_UserGetDslData\n" );
		if(len!=sizeof(DEF_MSGTODSL))
			printf( "%s: id=%u, len=%u, error\n", __FUNCTION__, id, len );
		dm=(DEF_MSGTODSL *)data;
		m.message=dm->message;
		m.intVal=dm->intVal;
		return adsl_drv_get( id, &m, sizeof(m) );
	}
#endif /*CONFIG_VDSL*/

	return adsl_drv_get( id, data, len );
}

int uc_xdsl_setup(void)
{
	setupDsl();	
#ifdef CONFIG_VDSL
	/*when ad<->vd, need retrain*/
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif /*CONFIG_VDSL*/
	return 0;
}



int uc_xdsl_init(void)
{
	startDsl();
	return 0;
}
/**end uc_xdsl wrapper api********************************************************/


