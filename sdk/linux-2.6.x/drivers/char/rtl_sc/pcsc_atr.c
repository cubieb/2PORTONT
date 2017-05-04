#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/smp_lock.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "pcsc_atr.h"
#include "pcsc_io.h"
#include "pcsc_reg.h"

#ifdef __KERNEL__
#define SCATR_PRINT printk
#else
#define SCATR_PRINT printf
#endif
/* TA1 */
static int Fi[] = {372, 372, 558, 744, 1116, 1488, 1860, RFU, RFU, 512, 768, 1024, 1536, 2048, RFU, RFU};
static int Di[] = {RFU, 1, 2, 4, 8, 16, 32, 64, 12, 20, RFU, RFU, RFU, RFU, RFU, RFU};

/* For first TA for T = 15*/
#if 0
static char ClockIndicator[4][32] = {"Clock stop not supported", "State L", "State H", "No preference"};
static char ClassIndicator[9][30] = {"RFU", "A(5V) only", "B(3V) only", "A(5V) and B(3V)", 
									  "C(1.8V) only", "RFU", "B(3V) and C(1.8V)", "A(5V), B(3V) and C(1.8V)", "RFU"};
#else
static char *ClockIndicator[] = {"Clock stop not supported", "State L", "State H", "No preference"};
static char *ClassIndicator[] = {"RFU", "A(5V) only", "B(3V) only", "A(5V) and B(3V)", 
									  "C(1.8V) only", "RFU", "B(3V) and C(1.8V)", "A(5V), B(3V) and C(1.8V)", "RFU"};	
#endif

static ATRStruct gATR;
static unsigned char origPPS[6];

static void resetgATR(void){
	gATR.hasTA = 0;
	gATR.hasTB = 0;
	gATR.hasTC = 0;
	gATR.hasTD = 0;
	gATR.hasCount = 0;
}

static int analyzeTS(unsigned char *atr){
	if ( *atr == 0x3b ){
		SCATR_PRINT("direct convention\n");
		gATR.isDirect = 1;
	}else if ( *atr == 0x3f )
		SCATR_PRINT("inverse convention\n");
	else
		return -1;
	return 0;
}

static int analyzeT0(unsigned char *atr){
	unsigned char Y1 = 0, K = 0;
	Y1 = ( (*atr) & 0xf0 ) >> 4;
	K = (*atr) & 0xf;
	SCATR_PRINT("Y1 = %d, K = %d\n", Y1, K);
	gATR.hasTA = Y1 & 0x1;
	gATR.hasTB = ( Y1 & 0x2 ) >> 1;
	gATR.hasTC = ( Y1 & 0x4 ) >> 2;
	gATR.hasTD = ( Y1 & 0x8 ) >> 3;
	
	while ( Y1 != 0){
		if ( Y1 & 0x1 )
			gATR.hasCount++;
		Y1 = Y1 >> 1;
	}
	SCATR_PRINT("analyzeT0 = A : %d, B : %d, C : %d, D : %d, count : %d\n", gATR.hasTA, gATR.hasTB, gATR.hasTC,gATR.hasTD,gATR.hasCount);

	gATR.historyBytes = K;
	
	return 0;
}

static int analyzeTA(unsigned char *atr, int index){
	SCATR_PRINT("analyzeTA[%d]  = 0x%02x\n", index, *atr);
	if ( index == 1 ){
		gATR.FI = (*atr) >> 4;
		gATR.DI = ( (*atr) & 0x0f );

		SCATR_PRINT("\t- Fi = %d, Di = %d\n", Fi[gATR.FI], Di[gATR.DI]);
	}else if ( index == 2 ){
		gATR.modeChange = ( (*atr) & 0x80 ) >> 8;
		gATR.useTA1FiDi = ( (*atr) & 0x20 ) >> 5;
		gATR.TofTA2 = (*atr) & 0xf;
		SCATR_PRINT("\t- modeChange = %d, useTA1FiDi = %d, TofTA2 = %d\n", 
						gATR.modeChange, gATR.useTA1FiDi, gATR.TofTA2);
	}
	
	if ( gATR.T1 == 1 && index == 3 ){
		gATR.IFSC = (*atr);
		SCATR_PRINT("\t- gATR.IFSC = %d\n", gATR.IFSC);
	}
		
	if ( gATR.T15 ){
		gATR.ClockIndex = ( (*atr) & 0xc0 ) >> 6;
		gATR.ClassIndex	= (*atr) & 0x3f ;
		SCATR_PRINT("\t- ClockIndicator = [%s], Class Indicator : [%s]\n", 
						ClockIndicator[gATR.ClockIndex], ClassIndicator[gATR.ClassIndex]);
	}
	gATR.hasTA = 0;
	return 0;
}

//SCATR_PRINT("TB1 and TB2 are deprecated\n");
static int analyzeTB(unsigned char (*atr), int index){
	SCATR_PRINT("analyzeTB[%d]  = 0x%02x\n", index, *atr);
	if ( index == 1 ){ //|| index == 2 )
		gATR.PI1 = (*atr) & 0x1f;
		gATR.II = ( (*atr) & 0x60 ) >> 6;
		if ( gATR.PI1 == 0x0 )
			SCATR_PRINT("\t- VPP is not connected in the ICC\n");
	}else if ( index == 2 ){
		if ( (*atr) > 49 || (*atr) < 251 ){
			gATR.PI2 = (*atr);
			SCATR_PRINT("\t- programming voltage\n");
		}else
			SCATR_PRINT("\t- other value is RFU\n");
	}else if ( index >= 3 ){
		gATR.BCWISet = 1;
		// The first TB for T = 1 encode CWI ( 0~15, default : 13 ) and BWI ( 0 ~ 9, default : 4 )
		gATR.BWI = (*atr) >> 4;
		gATR.CWI = ( (*atr) & 0xf );
		SCATR_PRINT("\t- BWI : %d, CWI : %d\n", gATR.BWI, gATR.CWI);
	}
	gATR.hasTB = 0;
	return 0;
}

static int analyzeTC(unsigned char (*atr), int index){
	SCATR_PRINT("analyzeTC[%d]  = 0x%02x\n", index, *atr);
	if ( index == 1 ){
		gATR.N = (*atr);
		SCATR_PRINT("\t- N = [%u]\n", gATR.N);
	}else if ( index == 2 ){
		gATR.WorkWaitTime = (*atr);
		SCATR_PRINT("\t- WorkWaitTime = [%u]\n", gATR.WorkWaitTime);
	}else if ( index == 3 ){
		if ( gATR.T1 == 1 ){
			if ( (*atr) == 1 ){
				SCATR_PRINT("\t- use CRC\n");
				gATR.EDCmode = 1;
			}else if ( (*atr) == 0 ){
				SCATR_PRINT("\t- use LRC\n");
				gATR.EDCmode = 0;
			}
		}
	}
	gATR.hasTC = 0;
	return 0;
}

static int analyzeTD(unsigned char *atr, int index){
	unsigned char ms = 0, T = 0;
	ms = (*atr) >> 4;
	resetgATR();
	gATR.hasTA = ms & 0x1;
	gATR.hasTB = (ms & 0x2) >> 1;
	gATR.hasTC = (ms & 0x4) >> 2;
	gATR.hasTD = (ms & 0x8) >> 3;
	SCATR_PRINT("analyzeTD[%d]  = 0x%02x\n", index, *atr);
	while ( ms != 0){
		if ( ms & 0x1 )
			gATR.hasCount++;
		ms = ms >> 1;
	}
	
	T = (*atr) & 0xf;
	
	if ( index == 1 && T == 15 )
		return -1;
	
	if ( T == 0 ){
		gATR.T0 = 1;
		SCATR_PRINT("\t- T = 0\n");
	}else if ( T == 1 ){
		gATR.T1 = 1;
		SCATR_PRINT("\t- T = 1\n");
	}else if ( T == 14 ){
		gATR.T14 = 1;
		SCATR_PRINT("\t- T = 14\n");
	}else if ( T == 15 ){
		gATR.T15 = 1;
		SCATR_PRINT("\t- T = 15\n");
	}
	
	//SCATR_PRINT("analyzeTD[%02x] A : %d, B : %d, C : %d, D : %d, count : %d\n", *atr, gATR.hasTA, gATR.hasTB, gATR.hasTC,gATR.hasTD,gATR.hasCount);
	//SCATR_PRINT("analyzeTD[%02x] T0 : %d, T1 : %d, T14 : %d, T15 : %d\n", *atr, gATR.T0,gATR.T1,gATR.T14,gATR.T15);
	//gATR.hasTD = 0;
	return 0;
}

// check TCK and PCK
static int analyzeTCKPCK(unsigned char *atr, int num, int type){
	unsigned char c = 0;
	if ( type == 0 )
		atr ++; // bypass TS
	while ( num!= 1 ){
		//SCATR_PRINT("[%x]:[%x]\n", c, *atr);
		c ^= (*atr);
		atr ++;
		num --;
	}
	//SCATR_PRINT("TCK = 0x[%x]\n", c);
	if ( c == *atr ){
		if ( type == 0 )
			SCATR_PRINT("TCK OK!\n");
		else
			SCATR_PRINT("PCK OK!\n");
	}else{
		if ( type == 0 )
			SCATR_PRINT("TCK NOK! 0x[%x]\n", *atr);
		else
			SCATR_PRINT("PCK NOK! 0x[%x]\n", *atr);
	}
	return 1;
}

/* 0x1Y */
static int analyzeCountryCode(unsigned char *atr, int num){
	unsigned char *str, len = 0;
	if ( atr == NULL )
		return -1;
	SCATR_PRINT("- Country code\n");
	str = atr;
	return 0;
}

/* 0x2Y */
static int analyzeIssuerIdentifyNum(unsigned char *atr, int num){
	unsigned char *str, len = 0;
	if ( atr == NULL )
		return -1;
	SCATR_PRINT("- Issuer identification number\n");
	str = atr;
	return 0;
}

/* 0x3Y */
static int analyzeCardSerData(unsigned char *atr, int num){
	unsigned char *str;
	if ( atr == NULL )
		return -1;		
	SCATR_PRINT("- Card Service data\n");
	str = atr;
	if ( *str == 0x31 ) {
		str++;
		if ( *(str) & 0xc0 )
			SCATR_PRINT("\t- Application selection :\n");
		if ( *(str) & 0x80 ){
			SCATR_PRINT("\t\t- Full DF name\n");
		}
		if ( *(str) & 0x40 ){
			SCATR_PRINT("\t\t- Partial DF name\n");
		}
		if ( *(str) & 0x30 )
			SCATR_PRINT("\t- BER-TLV data objects available\n");
		if ( *(str) & 0x20 ){
			SCATR_PRINT("\t\t- EF.DIR\n");
		}				
		if ( *(str) & 0x10 ){
			SCATR_PRINT("\t\t- EF.ATR\n");
		}				
		if ( (*(str) & 0x8)>>1 == 0x4 ){
			SCATR_PRINT("\t- by the READ BINARY command ( transparent structure )\n");
		}				
		if ( (*(str) & 0x4)>>1 == 0x0 ){
			SCATR_PRINT("\t- by the READ RECORD(S) command ( record structure )\n");
		}						
		if ( (*(str) & 0x0)>>1 == 0x2 ){
			SCATR_PRINT("\t- by the GET DATA command ( TLV structure)\n");
		}	
		if ( *(str) & 0x1 ){
			SCATR_PRINT("\t- Card without MF\n");
		}else{
			SCATR_PRINT("\t- Card with MF\n");
		}					
	}
	return 0;
}

/* 0x4Y */
static int analyzeInitAccessData(unsigned char *atr, int num){
	unsigned char *str, len = 0;

	if ( atr == NULL )
		return -1;	
	
	str = atr;
	len = num;
	
	SCATR_PRINT("- Initial access data\n");
	str ++;
	if ( len == 1 ){
	}else if ( len == 2 ){
		if ( *str & 0x80 )
			SCATR_PRINT("EF structure : Record Structure\n");
		else
			SCATR_PRINT("EF structure : Transparent Structure\n");
	}
	return 0;
}

/* 0x6Y */
static int analyzePreIssueData(unsigned char *atr, int num){
	unsigned char *str, i = 0;

	if ( atr == NULL )
		return -1;
	str = atr;
	SCATR_PRINT("- Pre-issuing data\n");
	str++;
	SCATR_PRINT("\t");
	SCATR_PRINT("0x[");
	for ( i = 0 ; i < num ; i ++ ){
		SCATR_PRINT("%02x", *str);
		str++;
	}
	SCATR_PRINT("]\n");
	return 0;
}

/* 0x7Y */
static int analyzeCardCapabilities(unsigned char *atr, int num){
	unsigned char *str, ch;

	if ( atr == NULL )
		return -1;
	str = atr;
	
	if ( num >= 1 ){
		str ++;
		SCATR_PRINT("Len 1 : 0x%02x\n", *str);		
		if ( (*str & 0xf0) != 0x0 )
			SCATR_PRINT("\t- DF selection\n");
		if ( *str & 0x80 )
			SCATR_PRINT("\t\t- by full DF name\n");
		if ( *str & 0x40 )
			SCATR_PRINT("\t\t- by partial DF name\n");
		if ( *str & 0x20 )
			SCATR_PRINT("\t\t- by path\n");
		if ( *str & 0x10 )
			SCATR_PRINT("\t\t- by file identifier\n");
		if ( *str & 0x08 )
			SCATR_PRINT("\t- Implicit DF selection\n");
		if ( *str & 0x04 )
			SCATR_PRINT("\t- Short EF identifier supported\n");
		if ( *str & 0x02 )
			SCATR_PRINT("\t- Record number supported\n");
		if ( *str & 0x01 )
			SCATR_PRINT("\t- Record identifier supported\n");		
	}
	if ( num >= 2 ){
		str ++;
		SCATR_PRINT("Len 2 : 0x%02x\n", *str);
		if ( *str & 0x80 )
			SCATR_PRINT("\tEFs of TLV structure supported\n");		

		SCATR_PRINT("\t- Behaviour of write functions\n");

		ch = (*str & 0x70) >> 5;
		if ( ch == 0x0 )
			SCATR_PRINT("\t\t- One-time write\n");
		if ( ch == 0x1 )
			SCATR_PRINT("\t\t- Proprietary\n");
		if ( ch == 0x2 )
			SCATR_PRINT("\t\t- Write OR\n");
		if ( ch == 0x3 )
			SCATR_PRINT("\t\t- Write AND\n");
		
		SCATR_PRINT("\t- Value 'FF' for the first byte of BER-TLV tag fields\n");		
		ch = (*str & 0x10 ) >> 4;
		if ( ch )
			SCATR_PRINT("\t\t- Valid (long private tags, constructed encoding)\n");
		else
			SCATR_PRINT("\t\t- Invalid (used for padding, default value)\n");
		ch = ( *str & 0xf ) ;
			SCATR_PRINT("\t- Data unit size in quartets : %d\n", ch);			
	}
	if ( num >= 3 ){
		str ++;
		SCATR_PRINT("Len 3 : 0x%02x\n", *str);		
		if ( *str & 0x80 )
			SCATR_PRINT("\tCommand chaining\n");
		if ( *str & 0x40 )
			SCATR_PRINT("\tExtendex Lc and Le fields\n");

		
		ch = ( *str & 0x18 ) >> 3;
		SCATR_PRINT("\t- Logical channel number assignment : \n");
		if ( ch & 0x2 )
			SCATR_PRINT("\t\t- by the Card\n");
		if ( ch & 0x1 )
			SCATR_PRINT("\t\t- by the interface\n");		
		if ( ch == 0x0 )
			SCATR_PRINT("\tNo logical channel\n");				

		SCATR_PRINT("\t- Maximum number of logical channels : ");
		ch = ( *str & 0x7 );
		if ( ch == 0x7 )
			SCATR_PRINT("eight or more bytes\n");
		else 
			SCATR_PRINT("%d\n", ch+1);
	}

	
	return 0;
}

/* 0x8Y */
static int analyzeStatusIndicator(unsigned char *atr, int num){
	unsigned char *str;

	if ( atr == NULL )
		return -1;	
	
	str = atr;
	
	if ( num == 1 ){
		str++;
		SCATR_PRINT("LCS = 0x[%x]\n", *str);
	}
	
	if ( num == 2 ){
		str++;
		SCATR_PRINT("SW1 = 0x[%x], SW2 = 0x[%x]\n", *str, *(str+1));
	}

	if ( num == 3 ){
		str++;
		SCATR_PRINT("LCS = 0x[%x], SW1 = 0x[%02x], SW2 = 0x[%02x]\n", *str, *(str+1), *(str+2));
	}
	
	return 0;
}

static int analyzeHistoricalBytes(unsigned char *atr, int num){
	unsigned char *str, len = 0, ch, mandatory_indicator = 0;
	int ret = 0;

	if ( atr == NULL )
		return -1;
	
	str = atr;
	
	if ( *str == 0x80 )
		SCATR_PRINT("Compact-TLV\n");
	else if ( *str == 0x0 ){
		SCATR_PRINT("Compact-TLV\n");
		mandatory_indicator = 1;
	}else if ( ((*str & 0x80)>>7) && ((*str & 0x7f) >= 1 )  && ((*str & 0x7f) <= 15 )){ // 0x81~0x8f
		SCATR_PRINT("Reserved for future use\n");
		return 0;
	}else{
		SCATR_PRINT("proprietary format : 0x[");
		for (len = 0 ; len < num ; len ++ )
			SCATR_PRINT("%02x", *(str+len));
		SCATR_PRINT("]\n");
		return 0;
	}
	
	if ( mandatory_indicator )
		num -= 4; // the last three bytes is mandatory status indicator 
	else
		num --;
	str ++ ;
	
	while ( num != 0 ){
		SCATR_PRINT("str = 0x[%x]\n", *str);
		ch = ( *str & 0xf0) >> 4;
		len = ( *str & 0x0f );
		if ( ch == 0x1 ){
			SCATR_PRINT("Country code\n");
			if ( ( ret = analyzeCountryCode(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x2 ){
			SCATR_PRINT("Issuer Identification number\n");
			if ( ( ret = analyzeIssuerIdentifyNum(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x3 ){
			if ( ( ret = analyzeCardSerData(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x4 ){
			if ( ( ret = analyzeInitAccessData(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x5 ){
		}else if ( ch == 0x6 ){
			if ( ( ret = analyzePreIssueData(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x7 ){
			if ( ( ret = analyzeCardCapabilities(str, len) ) == -1 )
				return -1;
		}else if ( ch == 0x8 ){
			if ( ( ret = analyzeStatusIndicator(str, len) ) == -1 )
				return -1;			
		}else if ( ch == 0xf ){
		}else if ( ch == 0xf ){
			
		}
		num -= (len+1);
		str += (len+1);
	}
	
	if ( mandatory_indicator ) {
		SCATR_PRINT("Mandatory status indicator\n");
		SCATR_PRINT("Life Cycle byte = 0x[%02x]\n", *str);
		SCATR_PRINT("Status bytes = 0x[%02x%02x]\n", *(str+1), *(str+2));
	}
		
	return 0;
}

void setETU(void){
	gATR.ETU = Fi[gATR.FI] / Di[gATR.DI];
	SCATR_PRINT("FI = %d, Di = %d, etu = 0x%x\n", 
				Fi[gATR.FI], Di[gATR.DI], gATR.ETU);
	sc_write(ICC_ETU0,  gATR.ETU & 0x00ff);
	sc_write(ICC_ETU1, (gATR.ETU & 0xff00) >> 8);	
}

void resetETU(void){
	sc_write(ICC_ETU0, 0x74);
	sc_write(ICC_ETU1, 0x01);	
}

void setBWI(void){
	if ( gATR.BCWISet ){
		gATR.BWT = 11 + ((372*Di[gATR.DI]) / Fi[gATR.FI] ) * 960 * (2^(gATR.BWI)) + Di[gATR.DI]*960;
		SCATR_PRINT("BWI= %d, BWT = 0x%04x\n", 
				gATR.BWI, gATR.BWT);		
		sc_write(ICC_T1_BWT0,  gATR.BWT & 0x000000ff);
		sc_write(ICC_T1_BWT1, (gATR.BWT & 0x0000ff00) >> 8);		
		sc_write(ICC_T1_BWT2, (gATR.BWT & 0x00ff0000) >> 16);		
		sc_write(ICC_T1_BWT3, (gATR.BWT & 0x0000ff00) >> 24);		
	}
}

void setCWI(void){
	if ( gATR.BCWISet ){
		gATR.CWT = 11 + (2^(gATR.CWI)) + 4;
		SCATR_PRINT("CWI= %d, CWT = 0x%04x\n", 
				gATR.CWI, gATR.CWT);		
		sc_write(ICC_T1_CWT0,  gATR.CWT & 0x00ff);
		sc_write(ICC_T1_CWT1, (gATR.CWT & 0xff00) >> 8);
	}
}

void setWorkWaitTime(void){
	if ( gATR.WorkWaitTime ){
		gATR.WWT = gATR.WorkWaitTime * 960 * Di[gATR.DI];
		SCATR_PRINT("WWI= %d, WWT = 0x%04x\n", 
				gATR.WorkWaitTime, gATR.WWT);		
		sc_write(ICC_T0_WWT0,  gATR.WWT & 0x000000ff);
		sc_write(ICC_T0_WWT1, (gATR.WWT & 0x0000ff00) >> 8);		
		sc_write(ICC_T0_WWT2, (gATR.WWT & 0x00ff0000) >> 16);				
	}
}

int analyzeATR(int num, ATRStruct *patr){
	unsigned char atr[36], *str, *strtmp, level = 1, count = 0;
	int ret = 0, i = 0;
	
	strtmp = atr;
	count = num;
	SCATR_PRINT("count = %d\n", count);
	
	if ( count <= 0 ||  count >= 34 ){
		SCATR_PRINT("ATR Len isn't correct %d\n", num);
		return -1;
	}
	if ( patr == NULL ){
		SCATR_PRINT("Invalid paramter\n");
		return -1;
	}

	memset(atr, 0x0, 36);
	for ( i = 0 ; i < num ; i ++ )
		atr[i] = sc_read(DATABUF+i);
	
	for ( i = 0 ; i < num ; i ++ )
		SCATR_PRINT("%02x ", atr[i]);
	SCATR_PRINT("\n");
	
	str = atr;
	//SCATR_PRINT("str = 0x[%x]\n", *str);
	if ( (ret =analyzeTS(str) ) == -1 )	{
		SCATR_PRINT("Error TS\n");
		return -1;
	}
		
	memset(&gATR, 0, sizeof(gATR));
	memset(origPPS, 0, 6);
	
	str++;
	count --;

	analyzeT0(str);
	
	while ( gATR.hasCount != 0 ){
		str++;
		count --;
		gATR.hasCount --;

		if ( gATR.hasTA )
			analyzeTA(str, level);

		else if ( gATR.hasTB )
			analyzeTB(str, level);

		else if ( gATR.hasTC )
			analyzeTC(str, level);

		else if ( gATR.hasTD ){
			if ( (ret = analyzeTD(str, level)) == -1 ){
				SCATR_PRINT("T of TD1 is 15, it's illegal\n");
				return -1;
			}
			level++;
		}
	}
	
	if ( gATR.T1 == 1 ){
		if ( ( ret = analyzeTCKPCK(strtmp, num, 0) ) == -1 )
			return -1;
	}
	str++;
	count -= 2; // also decrease TCK
	if ( count > 15 ){
		SCATR_PRINT("Historical bytes count %d is over 15\n", count);
		return -1;
	}
	SCATR_PRINT("count = %d\n", count);
	analyzeHistoricalBytes(str, count);

	memcpy(patr, &gATR, sizeof(ATRStruct));
	
	return 0;
}

void setOriginalPPS(unsigned char pps, int index){
	origPPS[index] = pps;
}

void genPPS(void){
	unsigned char pck = 0x0, T = 0;
	unsigned int data = 0x0, i = 0;
	
	// generate PPS request
	SCATR_PRINT("Gen PPS request\n");
	sc_write(DATABUF, 0xff); //PPSS
	setOriginalPPS(0xff, 0);
	
	// PPS0
	if ( gATR.T0 ) 
		T = 0;
	else if ( gATR.T1 )
		T = 1;
	
	sc_write(DATABUF + 1, 0x70 | T);
	setOriginalPPS(0x70 | T, 1);
	
	sc_write(DATABUF + 2, (gATR.FI << 4) | gATR.DI); // PPS1
	setOriginalPPS((gATR.FI << 4) | gATR.DI, 2);
	sc_write(DATABUF + 3, 0x0); // PPS2
	setOriginalPPS(0x0, 3);
	sc_write(DATABUF + 4, 0x0); // PPS3
	setOriginalPPS(0x0, 4);
#if 0
	sc_write(ICC_CTL0, sc_read(ICC_CTL0)|PSS_AUTO_PCK_ON); // Enable auto PCK
#else	
	for ( i = 0 ; i < 5 ; i ++ ){
		pck ^= sc_read(DATABUF+i);
	}
	//SCATR_PRINT("pck = 0x%02x\n", pck);
	sc_write(DATABUF + 5, pck); // PCK
	
	sc_write(ICC_CTL0, sc_read(ICC_CTL0)&(~PSS_AUTO_PCK_ON)); // disable auto PCK

#endif
	sc_write(ICC_PPS_RQST_LEN, 6); // fill PPS request length = 5

	data = START_TRANSFER|PPS_REQ;
	sc_write(ICC_TRANSFER, START_TRANSFER|PPS_REQ);
}

int analyzePPS(int num){
	unsigned char *str, len = 0, T = 0, pps[6], i = 0;
	int ret = 0;
	str = pps;
	
	if ( num <= 0 || num >= 7 ){
		SCATR_PRINT("PPS : Error Length\n");
		return -1;
	}
	
	for ( i = 0 ; i < num ; i ++ )
		pps[i] = sc_read(DATABUF+i);
	
	for ( i = 0 ; i < num ; i ++ ){
		SCATR_PRINT("%02x:", pps[i]);
	}
	SCATR_PRINT("\n");	
	
	if ( (ret = analyzeTCKPCK(str, num, 1)) == -1){
		SCATR_PRINT("PPS : Error PCK\n");		
		return -1;
	}
	
	if ( *str != 0xFF ){
		SCATR_PRINT("PPS : Error PPSS\n");
		return -1;		
	}
	
	str ++;
	len = ((*str) & 0xf0)>>4;
	T = ((*str) & 0x0f );
	
	if ( T != (origPPS[1] & 0x0f) ){ // PPS0
		SCATR_PRINT("PPS : T isn't same as original\n");
		return -1;
	}
	
	if ( ((*str) & 0x10 ) >> 4 ){ // bit 5 is 1
		if ( *(str+1) != origPPS[2] ){ //PPS1
			SCATR_PRINT("PPS : PPS1 (Fi and Di) isn't same\n");
			return -1;		
		}
		
	}

	if ( ((*str) & 0x20 ) >> 5 ){ // bit 6 is 1
		if ( *(str+2) != origPPS[3] ){ //PPS2
			SCATR_PRINT("PPS : PPS2 isn't same\n");
			return -1;		
		}	
	}else
		SCATR_PRINT("PPS : Card don't use SPU\n");

	if ( ((*str) & 0x40 ) >> 6 ){ // bit 7 is 1
		if ( *(str+3) != origPPS[4] ){ //PPS3
			SCATR_PRINT("PPS : PPS3 isn't same\n");
			return -1;		
		}		
	}
	
	setETU();
	if ( T == 1 ){
		setCWI();
		setBWI();
	}else if ( T == 0 )
		setWorkWaitTime();
	return 0;
}

