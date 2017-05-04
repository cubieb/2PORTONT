#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sc_filesystem.h"
#include "sc_ctrl.h"
#include "sc_ioctl.h"
#include "sc_status.h"

extern int devfd;

#define RETRYCOUNT	10
#define SWLEN	2
#define SUCCESS 0
#define FAIL -1
#define INVALID_PARA	-2
#define DATABUF_LEN	256

static inline void printf_off( const char *fmt, ... ) {};

#define NEWARCH
#define SCFS_DEBUG

#ifdef SCFS_DEBUG
#define FS_PRINTF           printf
#else
#define FS_PRINTF            printf_off
#endif

// local definition

typedef enum {
	SC_STATUS0_START,

	SC_PARITY_ERR,
	SC_PCK_TS_ERR,
	SC_TCK_ATRLEN_ERR,
	SC_LRC_ERR,
	SC_CRC_ERR,
	SC_ICC_TIMEOUT_FLAG,	
	SC_PROCEDURE_ERR,
	
	SC_DATAFINISH,
	
	SC_STATUS0_END
}SC_STATUS_TYPE1;

typedef enum {
	SC_STATUS1_START,
	
	SC_ICC_T0_WWT_TIMEOUT_FLAG,
	SC_ICC_T1_CWT_TIMEOUT_FLAG,
	SC_ICC_T1_BWT_TIMEOUT_FLAG,
	SC_ICC_RESET_RECEIVE_TIMEOUT,
	SC_ICC_RESET_ATR_DURATION_TIMEOUT,
	
	SC_STATUS1_END
}SC_STATUS_TYPE2;

static const char *SC_STATUS1[] = {
	"SC_PARITY_ERR"	
	"SC_PCK_TS_ERR",	
	"SC_TCK_ATRLEN_ERR",	
	"SC_LRC_ERR",	
	"SC_CRC_ERR",	
	"SC_ICC_TIMEOUT_FLAG",	
	"SC_PROCEDURE_ERR",
};

static const char *SC_STATUS2[] = {
	"SC_ICC_T0_WWT_TIMEOUT_FLAG",	
	"SC_ICC_T1_CWT_TIMEOUT_FLAG",	
	"SC_ICC_T1_BWT_TIMEOUT_FLAG",	
	"SC_ICC_RESET_RECEIVE_TIMEOUT",	
	"SC_ICC_RESET_ATR_DURATION_TIMEOUT"
};

// local variable
static DFFileStatus *gpDF;
static EFFileStatus *gpEF;
static unsigned char databuf[DATABUF_LEN];

int getFileLen(void){
	//StatusWord sw;
	unsigned char sw[2]  = { 0x0 };
	unsigned short swfull = 0x00;
	int len = 0, count = 0, ret = 0;

	len = SWLEN;
	
	ret = ioctl(devfd, SC_IOCTL_GETSW, &sw[0]);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_GETSW fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}	
	
	FS_PRINTF("sw1sw2 : 0x%02x:0x%02x\n", sw[0], sw[1]);
	
	if ( sw[0] == 0x9f )
		return sw[1];
	else
		return -1;
}

int checkDataStatus(void){
	unsigned char status = 0x0;
	int i = 0, ret = 0;
	StatusWord sw;
	
	for ( i = 0 ; i < RETRYCOUNT ; i ++ ){
		ret = ioctl(devfd, SC_IOCTL_GETSYSTEMSTATUS, &status);
		if ( ret == -1 ){
			FS_PRINTF("SC_IOCTL_GETDATA fail\n");
			FS_PRINTF("errno : %d\n", errno);
		}	
		if ( status == SC_DATAFINISH ){
			//FS_PRINTF("status get\n");
			break;
		}//else 
			//FS_PRINTF("status 0x%02x\n", status);
		usleep(10);
	}	

	if ( i == RETRYCOUNT ){
		//FS_PRINTF("status get timeout\n");	
		return FAIL;
	}
	return SUCCESS;
}

int checkStateWord(void){
	//StatusWord sw;
	unsigned char sw[2]  = { 0x0 };
	unsigned short swfull = 0x00;
	int len = 0, count = 0, ret = 0;

	//memset(&sw, 0x0, sizeof(StatusWord));
	len = SWLEN;
	
//	count = read(devfd, &sw[0], len);
	//FS_PRINTF("count = %d\n", count);
	
	ret = ioctl(devfd, SC_IOCTL_GETSW, &sw[0]);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_GETSW fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}	
	
	FS_PRINTF("sw1sw2 : 0x%02x:0x%02x\n", sw[0], sw[1]);
	
	if ( sw[0] == 0x9f )
		return sw[1];
	
	swfull = ((sw[0] << 8) | sw[1]);
	ret = SimStatusWord(swfull);
	//FS_PRINTF("ret = %d\n", ret);
	
	return SUCCESS;
}

void printData(unsigned char *buf, int len){
	int i = 0;
	
	for (i = 0 ; i < len ; i ++ ){
		if ( i != 0 && i % 8 == 0 )
			FS_PRINTF("\n");
		FS_PRINTF("0x%02x ", buf[i]);
	}
	FS_PRINTF("\n");
}

void printDFFileStatus(DFFileStatus *p){
	FS_PRINTF("totalMemory : 0x%04x\n", p->totalMemory);
	FS_PRINTF("FileID : 0x%04x\n", p->FileID);
	FS_PRINTF("typeOfFile : 0x%02x\n", p->typeOfFile);
	FS_PRINTF("lengnthOfFollowingData : 0x%02x\n", p->lengnthOfFollowingData);
	FS_PRINTF("characteristics : 0x%02x\n", p->gsmdata.characteristics);
	FS_PRINTF("numberofDF : 0x%02x\n", p->gsmdata.numberofDF);
	FS_PRINTF("numberofEF : 0x%02x\n", p->gsmdata.numberofEF);
	FS_PRINTF("numberofCHV : 0x%02x\n", p->gsmdata.numberofCHV);
	FS_PRINTF("CHV1Status : 0x%02x\n", p->gsmdata.CHV1Status);		
	FS_PRINTF("UNBLOCKCHV1Status : 0x%02x\n", p->gsmdata.UNBLOCKCHV1Status);	
	FS_PRINTF("CHV2Status : 0x%02x\n", p->gsmdata.CHV2Status);
	FS_PRINTF("UNBLOCKCHV2Status : 0x%02x\n", p->gsmdata.UNBLOCKCHV2Status);
	memcpy(gpDF, p, sizeof(DFFileStatus));
}

void printEFFileStatus(EFFileStatus *p){
	FS_PRINTF("FileSize : 0x%04x\n", p->FileSize);

	FS_PRINTF("FileID : 0x%04x\n", p->FileID);
	FS_PRINTF("typeOfFile : 0x%02x : ", p->typeOfFile);
	if ( p->typeOfFile == FILETYPE_RFU )
		FS_PRINTF("RFU\n");
	else if ( p->typeOfFile == FILETYPE_MF )
		FS_PRINTF("MF\n");
	else if ( p->typeOfFile == FILETYPE_DF )
		FS_PRINTF("DF\n");
	else if ( p->typeOfFile == FILETYPE_EF )
		FS_PRINTF("EF\n");	
	
	FS_PRINTF("AccessConditions : 0x%02x%02x%02x\n", p->AccessConditions[0], p->AccessConditions[1], p->AccessConditions[2]);
	FS_PRINTF("fileStatus : 0x%02x\n", p->fileStatus);
	FS_PRINTF("lengnthOfFollowingData : 0x%02x\n", p->lengnthOfFollowingData);
	
	FS_PRINTF("strutureOfEF : 0x%02x : ", p->strutureOfEF);
	if ( p->strutureOfEF == EF_TRANSPARENT )
		FS_PRINTF("TRANSPARENT\n");
	else if ( p->strutureOfEF == EF_LINEAR_FIXED )
		FS_PRINTF("LINEAR_FIXED\n");
	else if ( p->strutureOfEF == EF_CYCLIC )
		FS_PRINTF("CYCLIC\n");	
	FS_PRINTF("lengthOfRecord : 0x%02x\n", p->lengthOfRecord);
	
	memcpy(gpEF, p, sizeof(EFFileStatus));
}

unsigned short getEF_FileSize(void){
	return gpEF->FileSize;
}

unsigned char getEF_LenOfRecord(void){
	return gpEF->lengthOfRecord;
}

int selectFS(unsigned short fileID){
	int ret = 0, count = 0, length = 0;
	int i = 0;
	T0cmd t0cmd;
	
	//FS_PRINTF("select 0x%04x..\n", fileID);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = SELECT;
	t0cmd.P1 = 0x0;
	t0cmd.P2 = 0x0;
	t0cmd.Le = sizeof(unsigned short);
	
#ifndef NEWARCH	
	t0cmd.FileID[0] = (fileID & 0xff00) >> 8;
	t0cmd.FileID[1] = fileID & 0x00ff;
	
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	databuf[sizeof(T0cmd) + 0] = (fileID & 0xff00) >> 8;
	databuf[sizeof(T0cmd) + 1] = fileID & 0x00ff;
	length = sizeof(T0cmd) + sizeof(unsigned short); // command + 2bytes file ID	

	count = write(devfd, databuf, length);
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	checkStateWord();

	return SUCCESS;
}

int getStatus(unsigned char len){
	int ret = 0, length = 0, count = 0;
	unsigned char *buf;
	DFFileStatus *pDF;
	T0cmd t0cmd;
	
	//FS_PRINTF("Get Status length %d..\n", len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = STATUS;
	t0cmd.P1 = 0x0;
	t0cmd.P2 = 0x0;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_GETSTATUS fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	
	count = write(devfd, databuf, sizeof(T0cmd));
#endif
	
	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}	

	length = len + SWLEN;
	buf = (unsigned char *) malloc(length);
	count = read(devfd, buf, length);
	//FS_PRINTF("count = %d\n", count);
	//printData(buf, length);

	pDF = (DFFileStatus *)buf;
	//printDFFileStatus(pDF);	
	
	free(buf);

	return SUCCESS;
}

int getResponse(void){
	int ret = 0, length = 0, count = 0;
	unsigned char *buf;	
	T0cmd t0cmd;
	EFFileStatus *pEF;
	
	FS_PRINTF("get response..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = GETRESPONSE;
	t0cmd.P1 = 0x0;
	t0cmd.P2 = 0x0;
	t0cmd.Le = 0x0f;
	
#ifndef NEWARCH	
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	
	count = write(devfd, databuf, sizeof(T0cmd));	
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	length = 0x0f + SWLEN;
	buf = (unsigned char *) malloc(length);
	count = read(devfd, buf, length);
	//FS_PRINTF("count = %d\n", count);
	pEF = (EFFileStatus *)buf;
	printEFFileStatus(pEF);
	//printData(buf, length);
	free(buf);	

	return SUCCESS;
}

int readRecord(unsigned char recNo, unsigned char mode, unsigned char len){
	int ret = 0, length = 0, count = 0;
	T0cmd t0cmd;	
	unsigned char *buf;		

	FS_PRINTF("read record..recNo%d, mode 0x%02x, len%d\n", recNo, mode, len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = READRECORD;
	t0cmd.P1 = recNo;
	t0cmd.P2 = mode;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else	
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	
	count = write(devfd, databuf, sizeof(T0cmd));
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	length = len + SWLEN;
	buf = (unsigned char *) malloc(length);
	count = read(devfd, buf, length);
	//FS_PRINTF("count = %d\n", count);

	printData(buf, length);

	free(buf);			

	return SUCCESS;
}

int updateRecord(unsigned char recNo, unsigned char mode, unsigned char len, unsigned char *data){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("update record..recNo%d, mode 0x%02x, len%d\n", recNo, mode, len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = UPDATERECORD;
	t0cmd.P1 = recNo;
	t0cmd.P2 = mode;
	t0cmd.Le = len;

#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);

	len += sizeof(T0cmd);
	
	count = write(devfd, databuf, len);	
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

int readBinary(unsigned char offset_H, unsigned char offset_L, unsigned char len){
	int ret = 0, length = 0, count = 0;
	T0cmd t0cmd;	
	unsigned char *buf;		

	FS_PRINTF("read binary..offset_H 0x%02x, offset_L 0x%02x, len%d\n", offset_H, offset_L, len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = READBINARY;
	t0cmd.P1 = offset_H;
	t0cmd.P2 = offset_L;
	t0cmd.Le = len;

#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	
	count = write(devfd, databuf, sizeof(T0cmd));	
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	length = len + SWLEN;
	buf = (unsigned char *) malloc(length);
	count = read(devfd, buf, length);
	//FS_PRINTF("count = %d\n", count);

	printData(buf, length);

	free(buf);			

	return SUCCESS;
}

int updateBinary(unsigned char offset_H, unsigned char offset_L, unsigned char len, unsigned char *data){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("update binary..offset_H 0x%02x, offset_L 0x%02x, len%d\n", offset_H, offset_L, len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = UPDATEBINARY;
	t0cmd.P1 = offset_H;
	t0cmd.P2 = offset_L;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	count = write(devfd, databuf, sizeof(T0cmd));	
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();
	
	return SUCCESS;
}

// Command + Parameters
// Reponse data : Record number
int do_seek(unsigned char type, unsigned char len, unsigned char *data){
	int ret = 0, length = 0, count = 0;
	unsigned char *buf;	
	T0cmd t0cmd;

	FS_PRINTF("seek..type 0x%02x, len%d\n", type, len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = SEEK;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = type;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);	
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	length = len + SWLEN;
	buf = (unsigned char *) malloc(length);
	count = read(devfd, buf, length);
	//FS_PRINTF("count = %d\n", count);

	printData(buf, length);

	free(buf);

	return SUCCESS;
}

// Command + Parameters
// Reponse data
int do_increase(unsigned char *data){
	int ret = 0, length, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("increase..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = INCREASE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = 0x03;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, 0x03);
	
	length = 0x03 + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);		
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	return SUCCESS;
}

int verifyCHV(unsigned char chvNo, unsigned char *data){
	int ret = 0, len = 0x8, length = 0, count = 0;
	unsigned char *buf;
	T0cmd t0cmd;

	FS_PRINTF("verifyCHV..chvNo 0x%02x\n", chvNo);
	
	if ( chvNo > 0x2 ){
		FS_PRINTF("Invalid CHV number\n");
		return INVALID_PARA;
	}

	if ( sizeof(data) > len ){
		FS_PRINTF("pin code length is too long [%d]\n", sizeof(data));
		return INVALID_PARA;
	}	
	
	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = VERIFYCHV;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = chvNo;
	t0cmd.Le = len;
	
#ifndef NEWARCH	
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, sizeof(data));
	
	length = len + sizeof(T0cmd);
	printData(databuf, length);
	count = write(devfd, databuf, length);		
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}
	
	checkStateWord();

	return SUCCESS;	
}

// Parameter :	byte 1~8 : old CHV value
//				byte 9~16: new CHV value
int changeCHV(unsigned char chvNo, unsigned char *data){
	int ret = 0, len = 0x10, length = 0, count = 0;
	int i = 0;
	T0cmd t0cmd;
	
	FS_PRINTF("changeCHV..chvNo 0x%02x\n", chvNo);
	
	if ( chvNo > 0x2 ){
		FS_PRINTF("Invalid CHV number\n");
		return INVALID_PARA;
	}

	if ( sizeof(data) > len ){
		FS_PRINTF("pin code length is too long [%d]\n", sizeof(data));
		return INVALID_PARA;
	}
	
	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = CHANGECHV;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = chvNo;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	printData(databuf, length);
	length = len + sizeof(T0cmd);
#if 0	
	FS_PRINTF("=========\n");
	for ( i = 0 ; i < length ; i ++ ){
		if ( i != 0 && i % 8 == 0)
			FS_PRINTF("\n");
		FS_PRINTF("0x%02x ", databuf[i]);
	}
	FS_PRINTF("\n");
	FS_PRINTF("=========\n");
	return SUCCESS;
#endif	
	count = write(devfd, databuf, length);		
#endif

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

// mode  : 0 enable, 1 disable
int endisCHV(unsigned char mode, unsigned char *data){
	int ret = 0, len = CHV_LEN, length = 0, count = 0;
	T0cmd t0cmd;

	if ( mode == 0 )
		FS_PRINTF("enable CHV..mode 0x%02x\n", mode);
	else if ( mode == 1 )
		FS_PRINTF("disable CHV..mode 0x%02x\n", mode);

	if ( mode > 1 ){
		FS_PRINTF("Invalid mode\n");
		return INVALID_PARA;
	}	
	
	if ( sizeof(data) > len ){
		FS_PRINTF("pin code length is too long [%d]\n", sizeof(data));
		return INVALID_PARA;
	}
	
	printf("data len %d\n", sizeof(data));
	printf("data : [%s]\n", data);
	
	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	if ( mode == 0 )
		t0cmd.Ins = ENABLECHV;
	else if ( mode == 1 )
		t0cmd.Ins = DISABLECHV;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x01;
	t0cmd.Le = len;
	
#ifndef NEWARCH	
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, sizeof(data));
	
	length = len + sizeof(T0cmd);
	printData(databuf, length);
	
	printf("data : [%s]\n", databuf);
	count = write(devfd, databuf, length);		
#endif
	
	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

int unblockCHV(unsigned char chvNo, unsigned char *unblockCHVdata, unsigned char *newCHVdata){
	int ret = 0, len = CHV_LEN*2, length = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("disable CHV..\n");

	if ( chvNo > 0x2 ){
		FS_PRINTF("Invalid CHV number\n");
		return INVALID_PARA;
	}	
	
	if ( (sizeof(unblockCHVdata) > CHV_LEN) || (sizeof(newCHVdata) > CHV_LEN) ){
		FS_PRINTF("pin code length is too long unblockCHVdata : [%d], newCHVdata : [%d]\n", sizeof(unblockCHVdata), sizeof(newCHVdata));
		return INVALID_PARA;		
	}
	
	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = UNBLOCKCHV;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = chvNo;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], unblockCHVdata, CHV_LEN);	// copy unblock chv key
	memcpy(&databuf[sizeof(T0cmd) + CHV_LEN], newCHVdata, CHV_LEN);	// copy unblock chv key
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);		
#endif
	
	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

int do_invalidate(void){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("do_invalidate..\n");
	
	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = INVALIDATE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = 0x00;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));

	count = write(devfd, databuf, sizeof(T0cmd));		
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

int do_rehabilitate(void){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("do_rehabilitate..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = REHABILITATE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = 0x00;
	
#ifndef NEWARCH			
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));

	count = write(devfd, databuf, sizeof(T0cmd));		
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

// Parameters : 16 bytes rand 
// Response data :	Byte 1~4 :SRES
// 					Byte 5~12:Cipher Key Kc
int do_runGSMAlgorithm(unsigned char *data){
	int ret = 0, len = GSM_ALGO_NUM, length = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("Run GSM algorithm..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = RUNGSMALORITHM;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);		
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	return SUCCESS;
}

int do_sleep(void){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("do_rehabilitate..\n");

	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = SLEEP;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = 0x00;

#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));

	count = write(devfd, databuf, sizeof(T0cmd));		
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;
}

int terminalProfile(unsigned char *data, unsigned char len){
	int ret = 0, length = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("Run GSM algorithm..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = TERMINALPROFILE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = len;
	
#ifndef NEWARCH			
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);			
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;	
}

int do_envelope(unsigned char *data, unsigned char len){
	int ret = 0, length = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("do envelope..len %d\n", len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = ENVELOPE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);			
#endif	

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	return SUCCESS;	
}

int do_fetch(unsigned char len){
	int ret = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("do fetch..len %d\n", len);

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = FETCH;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));

	count = write(devfd, databuf, sizeof(T0cmd));		
#endif			

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	return SUCCESS;	
}

int terminalResponse(unsigned char len, unsigned char *data){
	int ret = 0, length = 0, count = 0;
	T0cmd t0cmd;

	FS_PRINTF("Run GSM algorithm..\n");

	memset(databuf, 0xff, DATABUF_LEN);
	
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = TERMINALRESPONSE;
	t0cmd.P1 = 0x00;
	t0cmd.P2 = 0x00;
	t0cmd.Le = len;
	
#ifndef NEWARCH		
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == FAIL ){
		FS_PRINTF("SC_IOCTL_SETCMD fail\n");
		FS_PRINTF("errno : %d\n", errno);
		return FAIL;
	}
#else
	memcpy(&databuf[0], &t0cmd, sizeof(T0cmd));
	memcpy(&databuf[sizeof(T0cmd)], data, len);
	
	length = len + sizeof(T0cmd);
	
	count = write(devfd, databuf, length);			
#endif		

	if (( ret = checkDataStatus() ) == FAIL ){
		FS_PRINTF("checkDataStatus fail\n");
		return FAIL;
	}

	checkStateWord();

	return SUCCESS;	
}

int fs_init(void){
	gpDF = (DFFileStatus *)malloc(sizeof(DFFileStatus));
	if ( gpDF == NULL )
		return FAIL;
	
	gpEF = (EFFileStatus *)malloc(sizeof(EFFileStatus));
	if ( gpEF == NULL )
		return FAIL;	
	
	memset(databuf, 0x0, DATABUF_LEN);
	
	return SUCCESS;
}

int fs_deinit(void){
	if ( gpDF )
		free(gpDF);
	if ( gpEF )
		free(gpEF);
}
