#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sc_ctrl.h"
#include "sc_ioctl.h"
#include "sc_filesystem.h"
#include "sc_util.h"

#define SUCCESS 0
#define FAIL -1
extern int devfd;

unsigned char *buf;

// GET IMSI 
int getIMSI(unsigned char *imsi){
	int ret = 0;
	unsigned short fileID = 0x0;
	//unsigned char imsi[9];
	printf("Get IMSI...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	fileID = 0x6F07;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (readBinary(0x00, 0x00, 9)) == FAIL )
		return ret;
}

int getEFKc(unsigned char *kc){
	int ret = 0;
	unsigned short fileID = 0x0;
	//unsigned char imsi[9];
	printf("Get IMSI...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	fileID = 0x6F07;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (readBinary(0x00, 0x00, 9)) == FAIL )
		return ret;
}

int getEFLP(unsigned char *lp){
	int ret = 0;
	unsigned short fileID = 0x0;
	
	printf("Get LP...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	fileID = 0x6F05;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	if ( (readBinary(0x00, 0x00, 9)) == FAIL )
		return ret;
}

// GET ICC
int getICC(unsigned char *icc){
	int ret = 0;
	unsigned short fileID = 0x0;
	//unsigned char imsi[9];
	printf("Get ICC...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x2fe2;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	if ( (readBinary(0x00, 0x00, 10)) == FAIL )
		return ret;

}

// GET LP
int getEFELP(unsigned char *elp){
	int ret = 0, len = 0;
	unsigned short fileID = 0x0;
	//unsigned char imsi[9];
	printf("Get ELP...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x2f05;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	len = getFileLen();
	
	if ( (readBinary(0x00, 0x00, len)) == FAIL )
		return ret;
}

// GET EF SST (SIM service table)
int getEFSST(unsigned char *SST){
	int ret = 0;
	unsigned short fileID = 0x0;	
	//unsigned char SST[9];
	
	printf("Get EF SST...\n");
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F38;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;
	
	if ( (readBinary(0x00, 0x00, 2)) == FAIL )
		return ret;
}

// GET EF SPN	
int getEFSPN(unsigned char *SPN){
	int ret = 0;
	unsigned short fileID = 0x0;	
	//unsigned char SPN[9];	
	
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	printf("Get EF SPN...\n");
	fileID = 0x6F46;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;
	
	if ( (readBinary(0x00, 0x00, 17)) == FAIL )
		return ret;	
}

int getEFLND(unsigned char *LND){
	int phoneBook_alphaLen = 0, recordNumber = 0, i = 0, j = 0, buflength = 0;;
	int ret = 0, count = 0;
	unsigned short fileID = 0x0;	
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F44;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		buf = (unsigned char *) malloc(buflength);
		memset(buf, 0xff, buflength);
		phoneBook_alphaLen = getEF_LenOfRecord() - ADN_NUMBER_LEN;
		for ( i = 0 ; i < phoneBook_alphaLen - 5; i ++ ){
			buf[i] = 0x5a;
		}
		
		buf[phoneBook_alphaLen] = 0x6;
		buf[phoneBook_alphaLen + 1] = 0x81;
		buf[phoneBook_alphaLen + 2] = 0x90;
		buf[phoneBook_alphaLen + 3] = 0x99;
		buf[phoneBook_alphaLen + 4] = 0x65;
		buf[phoneBook_alphaLen + 5] = 0x34;
		buf[phoneBook_alphaLen + 6] = 0x21;
		
		count = write(devfd, buf, buflength);
		//printf("count = %d\n", count);	
		free(buf);
		
		if ( (updateRecord(00, 0x03, buflength, &buf[0])) == FAIL)
			return ret;
		
		if ( (readRecord(00, 0x03, buflength)) == FAIL )
			return ret;

		usleep(1000);
	
	}	
}

// test phone book rw	
int testPhoneBook(unsigned char *pb){
	unsigned short fileID = 0x0;	
	int ret = 0, i = 0;
	int phoneBook_alphaLen = 0, recordNumber = 0, j = 0, buflength = 0;
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F3A;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	
	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		buf = (unsigned char *) malloc(buflength);
		memset(buf, 0xff, buflength);
		phoneBook_alphaLen = getEF_LenOfRecord() - ADN_NUMBER_LEN;
		for ( i = 0 ; i < phoneBook_alphaLen - 5; i ++ ){
			buf[i] = 0x61;
		}
		
		buf[phoneBook_alphaLen] = 0x06;
		buf[phoneBook_alphaLen + 1] = 0x31;
		buf[phoneBook_alphaLen + 2] = 0x32;
		buf[phoneBook_alphaLen + 3] = 0x33;
		buf[phoneBook_alphaLen + 4] = 0x34;
		buf[phoneBook_alphaLen + 5] = 0x35;
		buf[phoneBook_alphaLen + 6] = 0x36;
		
		//count = write(devfd, buf, buflength);
		//printf("count = %d\n", count);	
		
		if ( (updateRecord(j, 0x04, buflength, &buf[0])) == FAIL)
			return ret;
		free(buf);
		if ( (readRecord(j, 0x04, buflength)) == FAIL )
			return ret;

		usleep(1000);	
	}
}

// test phone book rw	
int getPhoneBook(unsigned char *pb){
	unsigned short fileID = 0x0;	
	int ret = 0, i = 0;
	int phoneBook_alphaLen = 0, recordNumber = 0, j = 0, buflength = 0;
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F3A;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	
	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		if ( (readRecord(j, 0x04, buflength)) == FAIL )
			return ret;

		usleep(1000);	
	}
}

// get sms
int getSMS(unsigned char *sms){
	int buflength = 0, recordNumber = 0, ret = 0, i = 0, j = 0;
	unsigned short fileID = 0x0;	
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F3C;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		if ( (readRecord(j, 0x04, buflength)) == FAIL )
			return ret;

		usleep(1000);
	}	
}

// test sms rw
int testSMS(unsigned char *sms){
	int buflength = 0, recordNumber = 0, ret = 0, i = 0, j = 0;
	unsigned short fileID = 0x0;	
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F3C;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		buf = (unsigned char *) malloc(buflength);
		memset(buf, 0xff, buflength);
		
		for ( i = 0 ; i < buflength - 1; i ++ ){
			buf[i] = 0x5a;
		}	
		//count = write(devfd, buf, buflength);
		//printf("count = %d\n", count);			
		if ( (updateRecord(j, 0x04, buflength, &buf[0])) == FAIL)
			return ret;
		free(buf);
		if ( (readRecord(j, 0x04, buflength)) == FAIL )
			return ret;

		usleep(1000);
	}	
}

int getEFSDN(unsigned char *sdn){
	int ret = 0, buflength = 0, recordNumber = 0, phoneBook_alphaLen = 0, j = 0, i = 0, count = 0;
	unsigned short fileID = 0x0;	
	
	fileID = 0x3f00;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x7F10;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	fileID = 0x6F49;
	if ( (selectFS(fileID)) == FAIL )
		return ret;

	if ( (getResponse()) == FAIL )
		return ret;

	recordNumber = getEF_FileSize() / getEF_LenOfRecord();
	
	for ( j = 0 ; j < recordNumber ; j ++ ){
	
		buflength = getEF_LenOfRecord();
		buf = (unsigned char *) malloc(buflength);
		memset(buf, 0xff, buflength);
		phoneBook_alphaLen = getEF_LenOfRecord() - ADN_NUMBER_LEN;
		for ( i = 0 ; i < phoneBook_alphaLen - 5; i ++ ){
			buf[i] = 0x5a;
		}
		
		buf[phoneBook_alphaLen] = 0x6;
		buf[phoneBook_alphaLen + 1] = 0x81;
		buf[phoneBook_alphaLen + 2] = 0x90;
		buf[phoneBook_alphaLen + 3] = 0x99;
		buf[phoneBook_alphaLen + 4] = 0x65;
		buf[phoneBook_alphaLen + 5] = 0x34;
		buf[phoneBook_alphaLen + 6] = 0x21;
		
		count = write(devfd, buf, buflength);
		//printf("count = %d\n", count);	
		free(buf);
		
		if ( (updateRecord(j, 0x04, buflength, &buf[0])) == FAIL)
			return ret;
		
		if ( (readRecord(j, 0x04, buflength)) == FAIL )
			return ret;

		usleep(1000);
	}
}

// GET ICC number
int getICCnumber(unsigned char *icc){
	int ret = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (getStatus(0x0d)) == FAIL )
		return ret;
	
	if ( (getStatus(0x23)) == FAIL )
		return ret;	
	
	printf("Get ICC number...\n");
	fileID = 0x2FE2;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (readBinary(0x00, 0x00, ICCID_NUM)) == FAIL )
		return ret;	
}

// GET EF Dir
int getEFDir(unsigned char *dir){
	int ret = 0, buflength = 0, j = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (getStatus(0x0d)) == FAIL )
		return ret;
	
	if ( (getStatus(0x23)) == FAIL )
		return ret;	
	
	printf("Get EF Dir...\n");
	fileID = 0x2F00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (getResponse()) == FAIL )
		return ret;

	if ( getEF_LenOfRecord() != 0 )
		buflength = getEF_LenOfRecord();
	else
		buflength = getEF_FileSize();
	
	if ( (readRecord(j, 0x02, buflength)) == FAIL )
		return ret;
}

// GET EF ATR
int getEFATR(unsigned char *atr){
	int ret = 0, buflength = 0, j = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (getStatus(0x0d)) == FAIL )
		return ret;
	
	if ( (getStatus(0x23)) == FAIL )
		return ret;	
	
	printf("Get EF ATR...\n");
	fileID = 0x2F01;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (getResponse()) == FAIL )
		return ret;

	if ( getEF_LenOfRecord() != 0 )
		buflength = getEF_LenOfRecord();
	else
		buflength = getEF_FileSize();
	
	if ( (readRecord(j, 0x04, buflength)) == FAIL )
		return ret;	
}

// GET EF PL
int getEFPL(unsigned char *pl){
	int ret = 0, buflength = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;
	
	if ( (getStatus(0x0d)) == FAIL )
		return ret;
	
	if ( (getStatus(0x23)) == FAIL )
		return ret;	
	
	printf("Get EF PL...\n");
	fileID = 0x2F05;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (getResponse()) == FAIL )
		return ret;

	if ( getEF_LenOfRecord() != 0 )
		buflength = getEF_LenOfRecord();
	else
		buflength = getEF_FileSize();
	
	
	if ( (readBinary(0x00, 0x00, buflength)) == FAIL )
		return ret;	
}

int enablePIN(unsigned char *pin){
	int ret = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (ret = endisCHV(0, pin)) == FAIL){
		printf("enable CHV fail\n");
		return ret;	
	}
	
	return ret;	
}

int verifyPIN(unsigned char *pin){
	int ret = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (ret = verifyCHV(0x01, &pin[0])) == FAIL){
		printf("verify CHV fail\n");
		return ret;	
	}
	
	return ret;	
}

int changePIN(unsigned char *newpin){
	int ret = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (ret = changeCHV(0x01, &newpin[0])) == FAIL){
		printf("change CHV fail\n");
		return ret;	
	}
	return ret;	
}

int disablePIN(unsigned char *pin){
	int ret = 0;
	unsigned short fileID = 0x0;	

	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (ret = endisCHV(1, &pin[0])) == FAIL){
		printf("disable CHV fail\n");
		return ret;	
	}	
	return ret;	
}

int unblockPIN(unsigned char *puk, unsigned char *newchv){
	int ret = 0;
	unsigned short fileID = 0x0;	
	
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		return ret;	
	
	if ( (ret = unblockCHV(1, &puk[0], &newchv[0])) == FAIL){
		printf("unblockCHV CHV fail\n");
		return ret;	
	}	
	return ret;	
}
