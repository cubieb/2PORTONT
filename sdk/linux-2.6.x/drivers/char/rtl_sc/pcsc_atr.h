#ifndef __PCSCATR__
#define __PCSCATR__

#define RFU -99

typedef struct _ATRStruct {
	unsigned char isDirect;			// TS 1:direct convention, 0:inverse convention
	unsigned char hasTA;
	unsigned char hasTB;
	unsigned char hasTC;
	unsigned char hasTD;
	unsigned char hasCount;			// count of sum (TA,TB,TC,TD)
	unsigned char historyBytes;		// T0 : least 4 bits
	unsigned char FI;				// TA1 : most 4 bits
	unsigned char DI;				// TA1 : least 4 bits
	unsigned char modeChange;		// TA2 : ability for changing negotiable / specific mode, unable : 0, capable : 1
	unsigned char useTA1FiDi;		// TA2 : 0 : apply Fi/Di in TA1, 1 use default
	unsigned char TofTA2;			// TA2 : least 4 bits means T
	unsigned char ClockIndex;		// TA3 : most 4 bits for T=15
	unsigned char ClassIndex;		// TA3 : least 4 bits for T=15
	unsigned char IFSC;				// TA3 : IFSC for T=1
	unsigned char PI1;				// TB1 : bits 7~8
	unsigned char II;				// TB1 : bits 0~5
	unsigned char PI2;				// TB2 : bits 7~8
	unsigned char BCWISet;			// if 1, means have BWI and CWI
	unsigned char BWI;				// most 4 bits in TB >= index 3
	unsigned char CWI;				// least 4 bits in TB >= index 3
	unsigned char N;				// TC1 value
	unsigned char WorkWaitTime;		// TC2 value
	unsigned char EDCmode;			// TC3 value when T=1, 0 : LRC, 1 : CRC
	unsigned short ETU;
	unsigned short CWT;
	unsigned int WWT;
	unsigned int BWT;
	unsigned char T0;
	unsigned char T1;
	unsigned char T14;
	unsigned char T15;
}ATRStruct;

void resetETU(void);
void genPPS(void);
int analyzeATR(int num, ATRStruct *patr);
int analyzePPS(int num);
#endif
