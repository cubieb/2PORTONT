#include <linux/module.h>
#include "pcsc_fs.h"
#include "pcsc_if.h"
#include "pcsc_t0.h"
#include "pcsc_io.h"
#include "pcsc_reg.h"

#ifdef __KERNEL__
#define PCSCFS_PRINT printk
#else
#define PCSCFS_PRINT printf
#endif

#define TIMEOUT 10000

static void getData(int len){
	unsigned int data = 0x0;
	int i ;
	for ( i = 0 ; i < len ; i ++ ){
		data = sc_read(DATABUF + i);
		PCSCFS_PRINT("%02x", data);
	}
	PCSCFS_PRINT("\n");
}

static void printPama(T0cmd *t0cmd){
	PCSCFS_PRINT("Class : 0x%02x\n", t0cmd->Class);
	PCSCFS_PRINT("Ins : 0x%02x\n", t0cmd->Ins);
	PCSCFS_PRINT("P1 : 0x%02x\n", t0cmd->P1);
	PCSCFS_PRINT("P2 : 0x%02x\n", t0cmd->P2);
	PCSCFS_PRINT("Le : 0x%02x\n", t0cmd->Le);
}

void getSW(unsigned char *sw){
	//PCSCFS_PRINT("Enter %s..\n", __FUNCTION__);
	sw[0] = sc_read(ICC_SW1_STATUS);
	sw[1] = sc_read(ICC_SW2_STATUS);
	//PCSCFS_PRINT("SW1 : 0x%02x\n", sw[0]);
	//PCSCFS_PRINT("SW2 : 0x%02x\n", sw[1]);	
}

void getBufData(unsigned char *data, int count){
	int i = 0;
	
	for ( i = 0 ; i < count ; i ++ ){
		data[i] = sc_read(DATABUF + i);
		//PCSCFS_PRINT("%02x", data[i]);
	}
	
}

void setBufData(unsigned char *data, int count){
	int i = 0;
	int offset = 5; // because the first five bytes are T0cmd
	
	for ( i = 0 ; i < count ; i ++ ){
		//PCSCFS_PRINT("%02x", data[offset + i]);
		sc_write(DATABUF + i, data[ offset + i ]);
	}
	//PCSCFS_PRINT("\n");
}

int T0selectFS(T0cmd *t0cmd){
	int ret = 0;
	
	if ( (ret = checkCardIsPlugged()) == 0)
		return -1;
	
	sc_write(ICC_T0_CLA, t0cmd->Class);
	sc_write(ICC_T0_INS, t0cmd->Ins);
	sc_write(ICC_T0_P1, t0cmd->P1);
	sc_write(ICC_T0_P2, t0cmd->P2);
	sc_write(ICC_T0_P3, t0cmd->Le);
	
	addSession();
	//PCSCFS_PRINT("%02x:%02x:%02x:%02x:%02x\n", t0cmd->Class, t0cmd->Ins, t0cmd->P1, t0cmd->P2, t0cmd->Le);
	//PCSCFS_PRINT("%02x:%02x\n", sc_read(DATABUF), sc_read(DATABUF + 1));
	//PCSCFS_PRINT("ICCTRANSFER : 0x%02x\n", START_TRANSFER|T0_WRITE);
	sc_write(ICC_TRANSFER, START_TRANSFER|T0_WRITE); 

	return 0;
}

int T0getStatus(T0cmd *t0cmd){
	int ret = 0;
	
	if ( (ret = checkCardIsPlugged()) == 0)
		return -1;	
	
	sc_write(ICC_T0_CLA, t0cmd->Class);
	sc_write(ICC_T0_INS, t0cmd->Ins);
	sc_write(ICC_T0_P1, t0cmd->P1);
	sc_write(ICC_T0_P2, t0cmd->P2);
	sc_write(ICC_T0_P3, t0cmd->Le);

	//PCSCFS_PRINT("%02x:%02x:%02x:%02x:%02x\n", t0cmd->Class, t0cmd->Ins, t0cmd->P1, t0cmd->P2, t0cmd->Le);
	
	addSession();
	sc_write(ICC_TRANSFER, START_TRANSFER|T0_READ); 

	return 0;
}

int T0getResponse(T0cmd *t0cmd){
	int ret = 0;
	
	if ( (ret = checkCardIsPlugged()) == 0)
		return -1;	
	
	sc_write(ICC_T0_CLA, t0cmd->Class);
	sc_write(ICC_T0_INS, t0cmd->Ins);
	sc_write(ICC_T0_P1, t0cmd->P1);
	sc_write(ICC_T0_P2, t0cmd->P2);
	sc_write(ICC_T0_P3, t0cmd->Le);

	//PCSCFS_PRINT("%02x:%02x:%02x:%02x:%02x\n", t0cmd->Class, t0cmd->Ins, t0cmd->P1, t0cmd->P2, t0cmd->Le);
	
	addSession();
	sc_write(ICC_TRANSFER, START_TRANSFER|T0_READ); 

	return 0;
}

int T0read(T0cmd *t0cmd){
	int ret = 0;
	
	if ( (ret = checkCardIsPlugged()) == 0)
		return -1;	
	
	sc_write(ICC_T0_CLA, t0cmd->Class);
	sc_write(ICC_T0_INS, t0cmd->Ins);
	sc_write(ICC_T0_P1, t0cmd->P1);
	sc_write(ICC_T0_P2, t0cmd->P2);
	sc_write(ICC_T0_P3, t0cmd->Le);

	//PCSCFS_PRINT("%02x:%02x:%02x:%02x:%02x\n", t0cmd->Class, t0cmd->Ins, t0cmd->P1, t0cmd->P2, t0cmd->Le);
	
	addSession();
	sc_write(ICC_TRANSFER, START_TRANSFER|T0_READ); 

	return 0;
}

int T0write(T0cmd *t0cmd){
	int ret = 0;
	
	if ( (ret = checkCardIsPlugged()) == 0)
		return -1;
	
	sc_write(ICC_T0_CLA, t0cmd->Class);
	sc_write(ICC_T0_INS, t0cmd->Ins);
	sc_write(ICC_T0_P1, t0cmd->P1);
	sc_write(ICC_T0_P2, t0cmd->P2);
	sc_write(ICC_T0_P3, t0cmd->Le);
		
	addSession();
	//PCSCFS_PRINT("%02x:%02x:%02x:%02x:%02x\n", t0cmd->Class, t0cmd->Ins, t0cmd->P1, t0cmd->P2, t0cmd->Le);
	//PCSCFS_PRINT("ICCTRANSFER : 0x%02x\n", START_TRANSFER|T0_WRITE);
	sc_write(ICC_TRANSFER, START_TRANSFER|T0_WRITE); 

	return 0;
}

