#ifndef __RFC2833_TRUNKEVENT__
#define __RFC2833_TRUNKEVENT__

void RFC2833TrunkEventEnable(int chid, unsigned char bEnable);
int RFC2833GenerateTrunkEvent(int chid, unsigned char signal);
int RFC2833TrunkEventOut(int chid, int status);
int RFC2833TrunkEventIn(int chid, int event);

#define RFC2833TrunkEventBeginID 128
#define RFC2833TrunkEventEndID 173
#define RFC2833TrunkABCDEventBeginID 144
#define RFC2833TrunkABCDEventEndID 159

#endif