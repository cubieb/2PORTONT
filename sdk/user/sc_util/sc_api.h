#ifndef __SCAPI__
#define __SCAPI__

int getIMSI(unsigned char *imsi);
int getEFSST(unsigned char *SST);
int getEFSPN(unsigned char *SPN);
int getEFLND(unsigned char *LND);
int testPhoneBook(unsigned char *pb);
int getPhoneBook(unsigned char *pb);

int getSMS(unsigned char *sms);
int testSMS(unsigned char *sms);

int getEFSDN(unsigned char *sdn);
int getICCnumber(unsigned char *icc);
int getEFDir(unsigned char *dir);
int getEFATR(unsigned char *atr);
int getEFPL(unsigned char *pl);
int getICC(unsigned char *icc);
int getEFELP(unsigned char *elp);
int getEFLP(unsigned char *lp);

int enablePIN(unsigned char *pin);
int verifyPIN(unsigned char *pin);
int changePIN(unsigned char *newpin);
int disablePIN(unsigned char *pin);
int unblockPIN(unsigned char *puk, unsigned char *newchv);

#endif
