#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sc_ctrl.h"
#include "sc_ioctl.h"
#include "sc_filesystem.h"
#include "sc_util.h"
#include "sc_api.h"
#define DEVICEPATH	"/dev/simcard"

#define SUCCESS 0
#define FAIL -1

//#define TESTPHONEBOOK
//#define TESTSMS
//#define TESTPIN
int devfd = 0;



int main(int argc, unsigned char *argv[]){
    int i = 0, ret = 0, count, len = 0, j = 0, option = 0;
	unsigned char cardIsPlugged = 0, isGetATRPPS = 0, *buf, *buf2;
	unsigned short fileID = 0x0;

	int type = 0;
	unsigned char pincode[CHV_LEN], pincode2[CHV_LEN];
	printf("13\n");
	
	if ( argc > 2 ){
		memset(pincode, 0xff, CHV_LEN);
		memset(pincode2, 0xff, CHV_LEN);
	}
	
	if ( argc < 2 ){
		printf("%s option\n", argv[0]);
		printf("option list\n");
		printf("0 : getICCnumber\n");
		printf("1 : getICC\n");
		printf("2 : getEFLP\n");
		printf("3 : getEFELP\n");
		printf("4 : getEFSST\n");
		printf("5 : getIMSI\n");
		printf("6 : getEFSPN\n");
		printf("7 : getEFLND\n");
		printf("8 : getPhoneBook\n");
		printf("9 : getSMS\n");
		printf("10 : getEFSDN\n");
		printf("11 : getEFDir\n");
		printf("12 : getEFATR\n");
		printf("13 : getEFPL\n");
		printf("14 : getSMS\n");
		printf("15 : testPhoneBook\n");
		printf("16 : enableCHV\n");
		printf("17 : verifyPIN\n");
		printf("18 : changePIN\n");
		printf("19 : disablePIN\n");
		printf("20 : unblockPIN\n");
		printf("21 : getEFPL\n");
		return -1;
	}
	
	sscanf(argv[1], "%d", &option);
	if ( argc > 2 ){
		sscanf(argv[2], "%s", &pincode[0]);
		printf("pincode [%s]\n", &pincode[0]);
	}
#ifdef TESTPIN	
	if ( argc != 4 ){
		printf("%s $type $pincode1 $pincode2\n", argv[0]);
		printf("type 0 : enable chv\n");
		printf("type 1 : verify chv\n");
		printf("type 2 : change chv\n");		
		printf("type 3 : disable chv\n");
		printf("type 4 : unblock chv\n");
		return 0;
	}
	
	memset(pincode, 0xff, CHV_LEN);
	memset(pincode2, 0xff, CHV_LEN);
	
	sscanf(argv[1], "%d", &type);
	memcpy(pincode, &argv[2][0], strlen(&argv[2][0]));
	memcpy(pincode2, &argv[3][0], strlen(&argv[3][0]));
		
	for ( i = 0 ; i < CHV_LEN ; i ++ ){
		printf("0x%02x ", pincode[i]);
	}
	printf("\n");
	
	for ( i = 0 ; i < CHV_LEN ; i ++ ){
		printf("0x%02x ", pincode2[i]);
	}
	printf("\n");
#endif	
	
	//sscanf(argv[2], "%d", &pincode);
	//sscanf(argv[3], "%d", &pincode2);
	
	if ( (ret = fs_init()) == FAIL ){
		return -1;
	}
	
	devfd = open(DEVICEPATH, O_RDWR);
    if (devfd == -1) {
		printf("Can't open %s %d\n", DEVICEPATH, devfd);
		printf("errno : %d\n", errno);
		goto FAILURE;
    }

    ret = ioctl(devfd, SC_IOCTL_CHECKCARD, &cardIsPlugged);
	if ( ret == -1 ){
		printf("SC_IOCTL_CHECKCARD fail\n");
		printf("errno : %d\n", errno);
		goto FAILURE;
	}
	//printf("Card is 0x%x\n", cardIsPlugged);

	if ( ! cardIsPlugged ){
		printf("Card is not plugged\n");
		goto FAILURE;
	}

#if 0
    ret = ioctl(devfd, SC_IOCTL_CHECKATRPPS, &isGetATRPPS);
	if ( ret == -1 ){
		printf("SC_IOCTL_CHECKATRPPS fail\n");
		printf("errno : %d\n", errno);
		goto FAILURE;
	}
	//printf("Card is 0x%x\n", isGetATRPPS);

	if ( ! isGetATRPPS ){
		ret = ioctl(devfd, SC_IOCTL_DOWARMRESET);
		if ( ret == -1 ){
			printf("SC_IOCTL_DOWARMRESET fail\n");
			printf("errno : %d\n", errno);
			goto FAILURE;
		}		
		printf("isGetATRPPS isn't get\n");
		//goto FAILURE;
	}
#endif
		
	
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		goto FAILURE;
	
	if ( (getStatus(0x0d)) == FAIL )
		goto FAILURE;
	
	if ( (getStatus(0x23)) == FAIL )
		goto FAILURE;


	// enable CHV
#ifdef TESTPIN	
	if ( type == 0 ){
		buf = (unsigned char *) malloc(CHV_LEN);
		memset(buf, 0xff, CHV_LEN);
		memcpy(buf, &pincode[0], CHV_LEN);
		//for (i = 0 ; i < 4 ; i ++){
		//	buf[i] = 0x30;
			//printf("%d : 0x%02x\n", i, buf[i]);
		//}
		
		if ( (ret = endisCHV(0, &buf[0])) == FAIL){
			printf("enable CHV fail\n");
			goto FAILURE;
		}
		free(buf);
		goto END;
	}
#endif
	
	// verify CHV	
#ifdef TESTPIN	
	if ( type == 1 ){
		buf = (unsigned char *) malloc(CHV_LEN);
		memset(buf, 0xff, CHV_LEN);
		for (i = 0 ; i < 4 ; i ++)
			buf[i] = 0x30;

		//count = write(devfd, buf, CHV_LEN);
		if ( (ret = verifyCHV(0x01, &buf[0])) == FAIL){
			printf("verify CHV fail\n");
			goto FAILURE;
		}
		free(buf);	
		goto END;
	}
#endif	
	
	// change CHV	
#ifdef TESTPIN	
	if ( type == 2 ){
		buf = (unsigned char *) malloc(CHV_LEN*2);
		memset(buf, 0xff, CHV_LEN*2);
		for (i = 0 ; i < 4 ; i ++)
			buf[i] = 0x30;

		for (i = CHV_LEN ; i < CHV_LEN + 4; i ++)
			buf[i] = 0x31;	

		//count = write(devfd, buf, CHV_LEN*2);
		if ( (ret = changeCHV(0x01, &buf[0])) == FAIL){
			printf("change CHV fail\n");
			goto FAILURE;
		}
		free(buf);
	}
#endif	

	// disable CHV	
#ifdef TESTPIN	
	if ( type == 3 ) {
		buf = (unsigned char *) malloc(CHV_LEN);
		memset(buf, 0xff, CHV_LEN);
		for (i = 0 ; i < 4 ; i ++ )
			buf[i] = 0x31;

		//count = write(devfd, buf, CHV_LEN);	
		if ( (ret = endisCHV(1, &buf[0])) == FAIL){
			printf("disable CHV fail\n");
			goto FAILURE;
		}
		free(buf);
		goto END;
	}
#endif	

	// unblock CHV	
#ifdef TESTPIN	
	if ( type == 4 ) {
		buf = (unsigned char *) malloc(CHV_LEN);	// unblock chv value
		buf2 = (unsigned char *) malloc(CHV_LEN);	// new chv value
		memset(buf, 0xff, CHV_LEN);
		memset(buf2, 0xff, CHV_LEN);
		{	// set unblock chv value
			buf[0] = 0x34;
			buf[1] = 0x32;
			buf[2] = 0x32;
			buf[3] = 0x31;
			buf[4] = 0x36;
			buf[5] = 0x34;
			buf[6] = 0x31;
			buf[7] = 0x32;
		}
		
		{	// set new chv value
			buf[0] = 0x30;
			buf[1] = 0x30;
			buf[2] = 0x30;
			buf[3] = 0x30;
		}
		
		//count = write(devfd, buf, CHV_LEN);	
		if ( (ret = unblockCHV(1, &buf[0], &buf2[0])) == FAIL){
			printf("unblockCHV CHV fail\n");
			goto FAILURE;
		}
		free(buf);
		free(buf2);
		goto END;
	}
#endif	

#if 0
	fileID = 0x3f00;
	if ( (ret = selectFS(fileID)) == FAIL )
		goto FAILURE;

	fileID = 0x7f20;
	if ( (ret = selectFS(fileID)) == FAIL )
		goto FAILURE;	
#endif	
	
	switch (option){
		case 0:
			getICCnumber(buf);
			break;
		case 1:
			getICC(buf);
			break;
		case 2:
			getEFLP(buf);
			break;
		case 3:
			getEFELP(buf);
			break;
		case 4:
			getEFSST(buf);
			break;
		case 5:
			getIMSI(buf);
			break;
		case 6:
			getEFSPN(buf);
			break;
		case 7:
			getEFLND(buf);
			break;
		case 8:
			getPhoneBook(buf);
			break;
		case 9:
			getSMS(buf);
			break;		
		case 10:
			getEFSDN(buf);
			break;	
		case 11:
			getEFDir(buf);
			break;	
		case 12:
			getEFATR(buf);
			break;	
		case 13:
			getEFPL(buf);
			break;	
		case 14:
			getSMS(buf);
			break;		
		case 15:
			testPhoneBook(buf);
			break;
		case 16:
			enablePIN(&pincode[0]);
			break;
		case 17:
			verifyPIN(&pincode[0]);
			break;
		case 18:
			changePIN(&pincode[0]);
			break;
		case 19:
			disablePIN(&pincode[0]);
			break;
		case 20:
			unblockPIN(&pincode[0], &pincode2[0]);
			break;
		default:
			break;
	}
	
#if 0	
	printf("Get Status 0x23..\n");
	{
	t0cmd.Class = SIMCLASS;
	t0cmd.Ins = STATUS;
	t0cmd.P1 = 0x0;
	t0cmd.P2 = 0x0;
	t0cmd.Le = 0x23;
	
	ret = ioctl(devfd, SC_IOCTL_SETCMD, &t0cmd);
	if ( ret == -1 ){
		printf("SC_IOCTL_GETSTATUS fail\n");
		printf("errno : %d\n", errno);
		goto FAILURE;
	}		
	
	if (( ret = checkDataStatus(devfd) ) == FAIL ){
		printf("checkDataStatus fail\n");
		goto FAILURE;
	}	
	
	len = 0x23 + SWLEN;
	buf = (unsigned char *) malloc(len);
	count = read(devfd, buf, len);
	printf("count = %d\n", count);
	printData(buf, len);
	


	free(buf);
	}
#endif

END:	
	close(devfd);
	fs_deinit();
    return 0;
	
FAILURE:
	close(devfd);
	fs_deinit();
	return FAIL;	
}
