		/******************************************************
		 *						      *
		 *	Realtek VoIP-ATA DTMF DECODER HEADER FILE     *
		 *						      *
		 *	Author : thlin@realtek.com.tw		      *
		 *						      *
		 *	Date: 2005-10-11			      *
		 *						      *
		 *	Copyright 2005 Realtek Semiconductor Corp.    *
		 *						      *
		 ******************************************************/

#ifndef DTMF_DEC_H
#define DTMF_DEC_H

#include "../../include/rtk_voip.h"
#include "fsk_det.h"

/*
 ***************************************************************************************************************************
 * Description of sturcture Dtmf_det_out:
 * Assume DTMF key "6" is pressed for several seconds, digit response "6" only one time when DTMF detecter detect it. 
 * However, digitOnOff response result of each detection. I.e. digitOnOff response "6" while DTMF key "6" is still pressed.
 ***************************************************************************************************************************
 */

typedef enum
{
	DTMF_POWER_LEVEL_0DBM = 0,
	DTMF_POWER_LEVEL_MINUS_1DBM,
	DTMF_POWER_LEVEL_MINUS_2DBM,
	DTMF_POWER_LEVEL_MINUS_3DBM,
	DTMF_POWER_LEVEL_MINUS_4DBM,
	DTMF_POWER_LEVEL_MINUS_5DBM,
	DTMF_POWER_LEVEL_MINUS_6DBM,
	DTMF_POWER_LEVEL_MINUS_7DBM,
	DTMF_POWER_LEVEL_MINUS_8DBM,
	DTMF_POWER_LEVEL_MINUS_9DBM,
	DTMF_POWER_LEVEL_MINUS_10DBM,
	DTMF_POWER_LEVEL_MINUS_11DBM,
	DTMF_POWER_LEVEL_MINUS_12DBM,
	DTMF_POWER_LEVEL_MINUS_13DBM,
	DTMF_POWER_LEVEL_MINUS_14DBM,
	DTMF_POWER_LEVEL_MINUS_15DBM,
	DTMF_POWER_LEVEL_MINUS_16DBM,
	DTMF_POWER_LEVEL_MINUS_17DBM,
	DTMF_POWER_LEVEL_MINUS_18DBM,
	DTMF_POWER_LEVEL_MINUS_19DBM,
	DTMF_POWER_LEVEL_MINUS_20DBM,
	DTMF_POWER_LEVEL_MINUS_21DBM,
	DTMF_POWER_LEVEL_MINUS_22DBM,
	DTMF_POWER_LEVEL_MINUS_23DBM,
	DTMF_POWER_LEVEL_MINUS_24DBM,
	DTMF_POWER_LEVEL_MINUS_25DBM,
	DTMF_POWER_LEVEL_MINUS_26DBM,
	DTMF_POWER_LEVEL_MINUS_27DBM,
	DTMF_POWER_LEVEL_MINUS_28DBM,
	DTMF_POWER_LEVEL_MINUS_29DBM,
	DTMF_POWER_LEVEL_MINUS_30DBM,
	DTMF_POWER_LEVEL_MINUS_31DBM,
	DTMF_POWER_LEVEL_MINUS_32DBM,
	DTMF_POWER_LEVEL_MINUS_33DBM,
	DTMF_POWER_LEVEL_MINUS_34DBM,
	DTMF_POWER_LEVEL_MINUS_35DBM,
	DTMF_POWER_LEVEL_MINUS_36DBM,
	DTMF_POWER_LEVEL_MINUS_37DBM,
	DTMF_POWER_LEVEL_MINUS_38DBM,
	DTMF_POWER_LEVEL_MINUS_39DBM,
	DTMF_POWER_LEVEL_MINUS_40DBM

}TdtmfPowerLevel;

#if 0
typedef enum
{
	DTMF_TWIST_1DB = 0x65810624,		//0x80000000*0.79432
	DTMF_TWIST_2DB = 0x50A3D70A,		//0x80000000*0.63
	DTMF_TWIST_3DB = 0x40000000,		//0x80000000*0.5
	DTMF_TWIST_4DB = 0x33333333,		//0x80000000*0.4  
	DTMF_TWIST_5DB = 0x2872B020,		//0x80000000*0.316
	DTMF_TWIST_6DB = 0x20000000,            //0x80000000*0.25
	DTMF_TWIST_7DB = 0x19999999,            //0x80000000*0.2  
	DTMF_TWIST_8DB = 0x14467381,            //0x80000000*0.1584 
	DTMF_TWIST_9DB = 0x10000000,            //0x80000000*0.125   
	DTMF_TWIST_10DB = 0x0CCCCCCC,           //0x80000000*0.1 
	DTMF_TWIST_11DB = 0x0A26809D,           //0x80000000*0.0793  
	DTMF_TWIST_12DB	= 0x0810624D,           //0x80000000*0.063 
	DTMF_TWIST_13DB = 0x06666666,           //0x80000000*0.05  
	DTMF_TWIST_14DB	= 0x04FDF3B6,           //0x80000000*0.039  
	DTMF_TWIST_15DB = 0x040B7803,           //0x80000000*0.0316  
	DTMF_TWIST_16DB	= 0x03333333,	        //0x80000000*0.025
	DTMF_TWIST_ERR = 0

}TdtmfTwistLevel;
#endif
#if 0
typedef enum
{
	DTMF_TWIST_1DB = 0x5A7EF9DB,		//0x80000000*0.707 (1.5dB)
	DTMF_TWIST_2DB = 0x47EF9DB2,		//0x80000000*0.562 (2.5dB)
	DTMF_TWIST_3DB = 0x3916872B,		//0x80000000*0.446 (3.5dB)
	DTMF_TWIST_4DB = 0x2DB22D0E,		//0x80000000*0.357
	DTMF_TWIST_5DB = 0x24189374,		//0x80000000*0.282
	DTMF_TWIST_6DB = 0x1C8B4395,		//0x80000000*0.223
	DTMF_TWIST_7DB = 0x16D5CFAA,		//0x80000000*0.1784
	DTMF_TWIST_8DB = 0x12161E4F,		//0x80000000*0.1413
	DTMF_TWIST_9DB = 0xE45A1CA,			//0x80000000*0.1115  
	DTMF_TWIST_10DB = 0xB6AE7D5,		//0x80000000*0.0892
	DTMF_TWIST_11DB = 0x90CB295,		//0x80000000*0.0707
	DTMF_TWIST_12DB	= 0x7318FC5,		//0x80000000*0.0562
	DTMF_TWIST_13DB = 0x5B573EA,		//0x80000000*0.0446
	DTMF_TWIST_14DB	= 0x474538E,		//0x80000000*0.0348 
	DTMF_TWIST_15DB = 0x39C0EBE,		//0x80000000*0.0282
	DTMF_TWIST_16DB	= 0x2DAB9F5,		//0x80000000*0.0223
	DTMF_TWIST_ERR = 0
}TdtmfTwistLevel;
#endif
#if 0
typedef enum
{
	DTMF_TWIST_1DB = 0x578D4FDF,		//0x80000000*0.6840 (1.65dB)
	DTMF_TWIST_2DB = 0x458793DD,		//0x80000000*0.5432 (2.65dB)
	DTMF_TWIST_3DB = 0x373B645A,		//0x80000000*0.4315 (3.65dB)
	DTMF_TWIST_4DB = 0x2BDD97F6,		//0x80000000*0.3427
	DTMF_TWIST_5DB = 0x22D7E40A,		//0x80000000*0.272213464
	DTMF_TWIST_6DB = 0x1BAD3F64,		//0x80000000*0.216224598
	DTMF_TWIST_7DB = 0x15FBF431,		//0x80000000*0.171751522
	DTMF_TWIST_8DB = 0x11766575,		//0x80000000*0.136425669
	DTMF_TWIST_9DB = 0xDDEECD9,			//0x80000000*0.108365637
	DTMF_TWIST_10DB = 0xB049226,		//0x80000000*0.086076993
	DTMF_TWIST_11DB = 0x8C06F95,		//0x80000000*0.068372677
	DTMF_TWIST_12DB	= 0x6F39F7F,		//0x80000000*0.054309785
	DTMF_TWIST_13DB = 0x5859714,		//0x80000000*0.043139348
	DTMF_TWIST_14DB	= 0x462D7CA,		//0x80000000*0.034266447
	DTMF_TWIST_15DB = 0x37BE587,		//0x80000000*0.027218524
	DTMF_TWIST_16DB	= 0x2C47388,		//0x80000000*0.021620218
	DTMF_TWIST_ERR = 0
}TdtmfTwistLevel;
#endif
#if 1
typedef enum	//0.794319997338
{
	DTMF_TWIST_1DB = 0x52A454DE,		//0x80000000*0.64564 (1.9dB)
	DTMF_TWIST_2DB = 0x41A4BDBA,		//0x80000000*0.51284 (2.9dB)
	DTMF_TWIST_3DB = 0x34240B78,		//0x80000000*0.40735 (3.9dB)
	DTMF_TWIST_4DB = 0x296ADADB,		//0x80000000*0.3235734533
	DTMF_TWIST_5DB = 0x20E60F47,		//0x80000000*0.2570208646
	DTMF_TWIST_6DB = 0x1A21CF78,		//0x80000000*0.2041568124
	DTMF_TWIST_7DB = 0x14C1D9A6,		//0x80000000*0.1621658387
	DTMF_TWIST_8DB = 0x107CE5C1,		//0x80000000*0.1288115686
	DTMF_TWIST_9DB = 0xD18BE47,			//0x80000000*0.1023176048
	DTMF_TWIST_10DB = 0xA6726A9,		//0x80000000*0.0812729196
	DTMF_TWIST_11DB = 0x84364E4,		//0x80000000*0.0645567052
	DTMF_TWIST_12DB	= 0x6904CC2,		//0x80000000*0.0512786819
	DTMF_TWIST_13DB = 0x536B21E,		//0x80000000*0.0407316825
	DTMF_TWIST_14DB	= 0x4242CF0,		//0x80000000*0.0323539899
	DTMF_TWIST_15DB = 0x34A1E5E,		//0x80000000*0.0256994212
	DTMF_TWIST_16DB	= 0x29CE963,		//0x80000000*0.0204135641
	DTMF_TWIST_ERR = 0
}TdtmfTwistLevel;
#endif

/* larger percent, more strict DTMF detection */
typedef enum
{
	DTMF_95_PERCENT = 122,
	DTMF_90_PERCENT = 115,
	DTMF_85_PERCENT = 109,
	DTMF_80_PERCENT = 102,
	DTMF_75_PERCENT = 96,
	DTMF_70_PERCENT = 90,
	DTMF_65_PERCENT	= 83,
	DTMF_60_PERCENT = 77,
	DTMF_55_PERCENT	= 70,
	DTMF_50_PERCENT	= 65,
	DTMF_45_PERCENT	= 58,
	DTMF_40_PERCENT	= 52,
	DTMF_35_PERCENT	= 45,	
	DTMF_30_PERCENT	= 40,
	DTMF_25_PERCENT	= 32,
	DTMF_20_PERCENT	= 27,
	DTMF_15_PERCENT	= 19,
	DTMF_10_PERCENT	= 15,
	DTMF_5_PERCENT	= 6,
	DTMF_0_PERCENT = 0

}TdtmfRowColPercent;


/* larger multiple, more strict DTMF detection */
typedef enum
{
#ifdef DTMF_DET_DURATION_HIGH_ACCURACY
	DTMF_1_MULTIPLE = 1,
	DTMF_2_MULTIPLE,
	DTMF_3_MULTIPLE,
	DTMF_4_MULTIPLE,
	DTMF_5_MULTIPLE,
	DTMF_6_MULTIPLE,
	DTMF_7_MULTIPLE,
	DTMF_8_MULTIPLE,
	DTMF_9_MULTIPLE,
	DTMF_10_MULTIPLE,
	DTMF_11_MULTIPLE,
	DTMF_12_MULTIPLE,
	DTMF_13_MULTIPLE,
	DTMF_14_MULTIPLE,
	DTMF_15_MULTIPLE,
	DTMF_16_MULTIPLE,
	DTMF_17_MULTIPLE,
	DTMF_18_MULTIPLE,
	DTMF_19_MULTIPLE,
	DTMF_20_MULTIPLE,
	DTMF_21_MULTIPLE,
	DTMF_22_MULTIPLE,
	DTMF_23_MULTIPLE,
	DTMF_24_MULTIPLE,
	DTMF_25_MULTIPLE,
	DTMF_26_MULTIPLE,
	DTMF_27_MULTIPLE,
	DTMF_28_MULTIPLE,
	DTMF_29_MULTIPLE,
	DTMF_30_MULTIPLE,
	DTMF_31_MULTIPLE,
	DTMF_32_MULTIPLE,
	DTMF_33_MULTIPLE,
	DTMF_34_MULTIPLE,
	DTMF_35_MULTIPLE,
	DTMF_36_MULTIPLE,
	DTMF_37_MULTIPLE,
	DTMF_38_MULTIPLE,
	DTMF_39_MULTIPLE,
	DTMF_40_MULTIPLE,
	DTMF_41_MULTIPLE,
	DTMF_42_MULTIPLE,
	DTMF_43_MULTIPLE,
	DTMF_44_MULTIPLE,
	DTMF_45_MULTIPLE,
	DTMF_46_MULTIPLE,
	DTMF_47_MULTIPLE,
	DTMF_48_MULTIPLE,
	DTMF_49_MULTIPLE,
	DTMF_50_MULTIPLE,
	DTMF_51_MULTIPLE,
	DTMF_52_MULTIPLE,
	DTMF_53_MULTIPLE,
	DTMF_54_MULTIPLE,
	DTMF_55_MULTIPLE,
	DTMF_56_MULTIPLE,
	DTMF_57_MULTIPLE,
	DTMF_58_MULTIPLE,
	DTMF_59_MULTIPLE,
	DTMF_60_MULTIPLE,
	DTMF_61_MULTIPLE,
	DTMF_62_MULTIPLE,
	DTMF_63_MULTIPLE,
	DTMF_64_MULTIPLE,
	DTMF_65_MULTIPLE,
	DTMF_66_MULTIPLE,
	DTMF_67_MULTIPLE,
	DTMF_68_MULTIPLE,
	DTMF_69_MULTIPLE,
	DTMF_70_MULTIPLE,
	DTMF_71_MULTIPLE,
	DTMF_72_MULTIPLE,
	DTMF_73_MULTIPLE,
	DTMF_74_MULTIPLE,
	DTMF_75_MULTIPLE,
	DTMF_76_MULTIPLE,
	DTMF_77_MULTIPLE,
	DTMF_78_MULTIPLE,
	DTMF_79_MULTIPLE,
	DTMF_80_MULTIPLE,
	DTMF_81_MULTIPLE,
	DTMF_82_MULTIPLE,
	DTMF_83_MULTIPLE,
	DTMF_84_MULTIPLE,
	DTMF_85_MULTIPLE,
	DTMF_86_MULTIPLE,
	DTMF_87_MULTIPLE,
	DTMF_88_MULTIPLE,
	DTMF_89_MULTIPLE,
	DTMF_90_MULTIPLE
#else
	DTMF_512_MULTIPLE = 0,
	DTMF_341_MULTIPLE,
	DTMF_256_MULTIPLE,
	DTMF_205_MULTIPLE,
	DTMF_171_MULTIPLE,
	DTMF_128_MULTIPLE = 5,
	DTMF_114_MULTIPLE,
	DTMF_102_MULTIPLE,
	DTMF_85_MULTIPLE,
	DTMF_64_MULTIPLE,
	DTMF_60_MULTIPLE = 10,
	DTMF_57_MULTIPLE,
	DTMF_51_MULTIPLE,
	DTMF_43_MULTIPLE,
	DTMF_32_MULTIPLE,
	DTMF_30_MULTIPLE = 15,
	DTMF_28_MULTIPLE,
	DTMF_26_MULTIPLE,
	DTMF_21_MULTIPLE,
	DTMF_16_MULTIPLE,
	DTMF_13_MULTIPLE = 20,
	DTMF_11_MULTIPLE,
	DTMF_8_MULTIPLE,
	DTMF_4_MULTIPLE,
	DTMF_2_MULTIPLE,
	DTMF_1_MULTIPLE,
	DTMF_0P67_MULTIPLE,
	DTMF_0P5_MULTIPLE
#endif

}TdtmfRowColPowerMultiple;

typedef struct
{
	char digit;	 
	char digitOnOff; 

}Dtmf_det_out;

typedef struct
{
	unsigned short fax_v21_data[512];
	unsigned short fax_v21_windex;
	unsigned short hs_fax_v21_data[512]; //high speed fax
	unsigned short hs_fax_v21_windex;
	unsigned short hs_fax_v21_data_tx[512]; //high speed fax
	unsigned short hs_fax_v21_windex_tx;
	unsigned v21flag_det_local_detected:1;
	unsigned v21flag_det_remote_detected:1;
	unsigned hs_fax_send_v21flag_det_local_detected:1;
	unsigned hs_fax_send_v21flag_det_remote_detected:1;
	unsigned hs_fax_recv_v21flag_det_local_detected:1;
	unsigned hs_fax_recv_v21flag_det_remote_detected:1;
}
TstVoipFaxV21;//For fax v21 data
#define FAX_V21_HEADER	0x5501
#define FAX_V21_GEP	0x5502
#define FAX_V21_NO_SIGNAL	0x5503

#define HS_FAX_V21_HEADER	0x6601
#define HS_FAX_V21_CH2_HEADER	0x6602

extern TstVoipFaxV21 stVoipFaxV21[];
extern unsigned char dtmf_chid[MAX_DSP_RTK_CH_NUM][DTMF_DIR_SIZE];

void init_fax_v21(unsigned int chid);
void exit_fax_v21(unsigned int chid);
void fax_v21_time_power_ths_set(int power);

extern long coef_acu_fra[];
#ifdef DTMF_DET_FREQ_OFFSET_REFINE
extern long* coef_tbl[];
#endif

/* dtmf_dec.c function prototype */

Dtmf_det_out dtmf_decoder(unsigned char chid, unsigned char dir, unsigned char *pBuf, long dtmf_pwr_level);

//#ifdef DTMF_DET_FREQ_OFFSET_REFINE
//Dtmf_det_out dtmf_dec(unsigned char *adr, uint32 page_size, unsigned char CH, long dtmf_pwr_level, unsigned char dir, long* p_accu_coef_in, unsigned char buf_update);
//#else
//Dtmf_det_out dtmf_dec(unsigned char *adr, uint32 page_size, unsigned char CH, long dtmf_pwr_level, unsigned char dir, long* p_accu_coef_in);
//#endif
void dtmf_start(unsigned char CH, unsigned int dir);
void dtmf_stop(unsigned char CH, unsigned int dir);
//void dtmf_cid_det(unsigned char *adr, uint32 page_size, unsigned char daa_chid, long level);
TstVoipciddet dtmf_cid_check(unsigned char daa_chid, char digit);
void dmtf_cid_det_init(int chid);

/* Set the dtmf det threshld */
void dtmf_det_threshold_set(int chid, TdtmfPowerLevel threshold, unsigned int dir);

/* Get the current dtmf det threshld(dBm) */
TdtmfPowerLevel dtmf_det_threshold_dBm_get(int chid, unsigned int dir);

/* Get the dtmf det threshld value(hex) from dBm */
long dtmf_det_threshold_dBm2Hex(TdtmfPowerLevel threshold);

/*  lower dtmf detect level : */
void dtmf_lower_level(void);

/*  lower dtmf caller id detect level : */
void dtmf_cid_lower_level(int chid, int mode, unsigned int dir);

/* dtmf det twist setting */
void dtmf_det_twist_set(unsigned int chid,unsigned int fortwist, unsigned int revtwist, unsigned int dir);
void dtmf_det_twist_dB_set(unsigned int chid,unsigned int fortwist_dB, unsigned int revtwist_dB, unsigned int dir);

/* For max row, col power percentage setting */
void dtmf_det_hilo_percentage_set(unsigned int chid, unsigned int row_stage1, unsigned int col_stage1, unsigned int row_stage2, unsigned int col_stage2, unsigned int dir);

/* For the setting of multiple of time power which "max (row+col) power" sould be */
void dtmf_det_time_power_multiple(unsigned int chid, unsigned int multiple, unsigned int dir);

/* set the dtmf detection minium on time unit: 10ms */
void dtmf_det_on_time_set(int chid, unsigned int dir, unsigned int on_time_10ms);

/* get the dtmf detection minium on time unit: 10ms */
unsigned int dtmf_det_on_time_get(int chid, unsigned int dir);

unsigned char dtmf_det_get_index(unsigned int chid, unsigned dir);

#ifdef DTMF_DET_DURATION_HIGH_ACCURACY
/* get more accuracy DTMF detected duration */
unsigned int dtmf_det_duration_get(unsigned int chid, unsigned dir, unsigned int* pDuration, int coef_index);
#ifdef DTMF_DET_FREQ_OFFSET_REFINE
Dtmf_det_out dtmf_decoder_v2(unsigned char chid, unsigned char dir, unsigned char *pBuf, long dtmf_pwr_level);
/* get current DTMF detected M value */
unsigned int get_cur_Mval(unsigned char chid, unsigned int dir);
/* Set and get the DTMF detection with freq. offset */
// Fellow ETSI. ITU-T Q.24 freq. offset standard
// ETSI: +-(1.5% + 2Hz) must be detected
// Q.24: within +-1.8% must be detected, over +-3% must be not detected
int dtmf_freq_offset_det_set(unsigned char chid, unsigned int dir, unsigned int val);
unsigned int dtmf_freq_offset_det_get(unsigned char chid, unsigned int dir);
#endif
void dtmf_det_set_index(unsigned int chid, unsigned dir, unsigned char index);
#endif // DTMF_DET_DURATION_HIGH_ACCURACY

#endif // DTMF_DEC_H

