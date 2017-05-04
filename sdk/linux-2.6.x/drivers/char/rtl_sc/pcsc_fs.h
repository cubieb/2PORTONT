#ifndef __SCIOFS__
#define __SCIOFS__

#include "pcsc_t0.h"

#define SELECT		0xA4
#define STATUS		0xF2

#define READBINARY	0xB0
#define UPDATEBINARY 0xD6

#define READRECORD	0xB2
#define UPDATERECORD 0xDC

#define GETRESPONSE	0xC0
#define TERMINALPROFILE	0x10
#define ENVELOPE	0xC2
#define FETCH	0x12
#define TERMINALRESPONSE	0x14

#define SEEK	0xA2
#define INCREASE	0x32

#define VERIFYCHV	0x20
#define CHANGECHV	0x24
#define DISABLECHV	0x26
#define ENABLECHV	0x28
#define UNBLOCKCHV	0x2C

#define INVALIDATE	0x04
#define REHABILITATE	0x44

#define	RUNGSMALORITHM	0x88

#define SLEEP	0xFA

int T0selectFS(T0cmd *t0cmd);
int T0getStatus(T0cmd *t0cmd);

void setBufData(unsigned char *data, int count);
void getBufData(unsigned char *data, int count);
int T0getResponse(T0cmd *t0cmd);
int T0read(T0cmd *t0cmd);
int T0write(T0cmd *t0cmd);
void getSW(unsigned char *sw);

#endif
