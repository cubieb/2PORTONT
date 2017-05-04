#ifndef __SCT0__
#define __SCT0__

#if 0
unsigned char *SWMessage[] = {"command normally completed",	//9000
						   "CLA not supported",	// 6E00
						   "CLA supported, but INS not programmed or invalid",	//6D00
						   "CLA INS supported, but P1 P2 incorrect",	// 6B00
						   "CLA INS P1 P2 supported, but P3 incorrect",	// 6700
						   "command not supported and no precise diagnosis given"	// 6F00
						   };
#endif

typedef struct _StatusWord{
	unsigned char sw1;
	unsigned char sw2;
}StatusWord;

typedef struct _T0cmd{
	unsigned char Class;
	unsigned char Ins;
	unsigned char P1;
	unsigned char P2;
	unsigned char Le;
}T0cmd;

typedef struct _T0data{
	unsigned char data[256];
	StatusWord sw;
	int len;
}T0data;

#endif
