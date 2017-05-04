#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"
#include "voip_control.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116
#define COEFFLEN	52
#define COEFFPATH	"/var/config/slicoeff"
typedef struct _Coeff{
	unsigned char coeff1[COEFFLEN];
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2
	unsigned char coeff2[COEFFLEN];
#endif	
}Coeff;


int main(int argc, char* argv[]){
	FILE *fp;
	int opt = 0, len = 0, chid = 0, type = 0;
	size_t result;
	Coeff *pCoeff;
	TstVoipCalibrationCoeff stVoipCalibrationCoeff;
	TstVoipDoCalibration stVoipDoCalibration;
	
	fp = fopen(COEFFPATH, "rb");
	if ( fp == NULL ){
		printf("open file(r) %s fail\n", COEFFPATH);
		fp = fopen(COEFFPATH, "wb");
		if ( fp == NULL ){
			printf("open file(w) %s fail\n", COEFFPATH);
			return -1;
		}else
			printf("open file(w) ok\n");
	}
	
	pCoeff  = (Coeff *)malloc(sizeof(Coeff));
	if ( pCoeff == NULL ) {
		printf("malloc pCoeff fail\n");
		return -1;		
	}
	
	memset(pCoeff, 0x0, sizeof(Coeff));
	memset(&stVoipCalibrationCoeff, 0x0, sizeof(TstVoipCalibrationCoeff));
	memset(&stVoipDoCalibration, 0x0, sizeof(TstVoipDoCalibration));
	if ( argc != 5 )
	{
		printf("Please input channel id and function\n");
		printf("\t-c : channel id = 0 or 1\n");
		printf("\t-t : \n");
		return 0;
	}
	
	while (( opt = getopt(argc, argv, "c:t:")) != -1 )
	{
		switch (opt)
		{
			case 'c':
				sscanf(optarg, "%d", &chid);
				break;				
			case 't':
				sscanf(optarg, "%d", &type);
				break;								
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if ( len == 0 ){
		freopen(COEFFPATH, "wb", fp);
		stVoipDoCalibration.mode = 0;
		stVoipDoCalibration.ch_id = 0;		
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);

		stVoipCalibrationCoeff.ch_id = 0;		
		rtk_GetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		memcpy(pCoeff->coeff1, stVoipCalibrationCoeff.coeff, COEFFLEN);
	
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2
		stVoipDoCalibration.mode = 0;
		stVoipDoCalibration.ch_id = 1;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
		
		stVoipCalibrationCoeff.ch_id = 1;		
		rtk_GetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		memcpy(pCoeff->coeff2, stVoipCalibrationCoeff.coeff, COEFFLEN);
#endif	
		fwrite(pCoeff, sizeof(char), sizeof(Coeff), fp);
	}else if ( len > 0 ) {
		result = fread(pCoeff, 1, sizeof(Coeff), fp);
		if ( result != sizeof(Coeff)){
			printf("read error\n");
			return -1;
		}
		memcpy(stVoipCalibrationCoeff.coeff, pCoeff->coeff1, COEFFLEN);
		stVoipCalibrationCoeff.ch_id = 0;
		rtk_SetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		
		stVoipDoCalibration.ch_id = 0;
		stVoipDoCalibration.mode = 1;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);		
		
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR == 2
		memcpy(stVoipCalibrationCoeff.coeff, pCoeff->coeff2, COEFFLEN);
		stVoipCalibrationCoeff.ch_id = 1;
		rtk_SetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		
		stVoipDoCalibration.ch_id = 1;
		stVoipDoCalibration.mode = 1;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
#endif		
	}
	
	
#if 0	
	if ( type == 0 ){ // GetCoefficients ch0
		stVoipCalibrationCoeff.ch_id = 0;
		rtk_GetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		printf("result : 0x%x\n", stVoipCalibrationCoeff.result);
		  for ( i = 0 ; i < 52 ; i ++ ){
			  if ( i % 8  == 0 )
				  printf("\n");
			  if ( i == 51 )
				printf("0x%02x ", stVoipCalibrationCoeff.coeff[i]);
			  else
				printf("0x%02x, ", stVoipCalibrationCoeff.coeff[i]);		 
		  }	
		 printf("\n");		  
	}else if ( type == 1 ){ // GetCoefficients ch1
		stVoipCalibrationCoeff.ch_id = 1;
		rtk_GetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		printf("result : 0x%x\n", stVoipCalibrationCoeff.result);
		  for ( i = 0 ; i < 52 ; i ++ ){
			  if ( i % 8  == 0 )
				  printf("\n");
			  if ( i == 51 )
				printf("0x%02x ", stVoipCalibrationCoeff.coeff[i]);
			  else
				printf("0x%02x, ", stVoipCalibrationCoeff.coeff[i]);		 
		  }			  
		  printf("\n");				
	}else if ( type == 2 ){ // SetCoefficients ch0
		stVoipCalibrationCoeff.ch_id = 0;
		memcpy(stVoipCalibrationCoeff.coeff, CAL_COEFF2, COEFFLEN);
		rtk_SetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		printf("result : 0x%x\n", stVoipCalibrationCoeff.result);
	}else if ( type == 3 ){ // SetCoefficients ch1
		stVoipCalibrationCoeff.ch_id = 1;
		memcpy(stVoipCalibrationCoeff.coeff, CAL_COEFF2, COEFFLEN);
		rtk_SetCalibrationCoeff(stVoipCalibrationCoeff.ch_id, &stVoipCalibrationCoeff);
		printf("result : 0x%x\n", stVoipCalibrationCoeff.result);
	}else if ( type == 4 ){ // DoCalibration ch0 mode 0
		stVoipDoCalibration.ch_id = 0;
		stVoipDoCalibration.mode = 0;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
		printf("result : 0x%x\n", stVoipDoCalibration.result);
	}else if ( type == 5 ){ // DoCalibration ch1 mode 0
		stVoipDoCalibration.ch_id = 1;
		stVoipDoCalibration.mode = 0;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
		printf("result : 0x%x\n", stVoipDoCalibration.result);
	}else if ( type == 6 ){ // DoCalibration ch0 mode 1
		stVoipDoCalibration.ch_id = 0;
		stVoipDoCalibration.mode = 1;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
		printf("result : 0x%x\n", stVoipDoCalibration.result);
	}else if ( type == 7 ){ // DoCalibration ch1 mode 1
		stVoipDoCalibration.ch_id = 1;
		stVoipDoCalibration.mode = 1;
		rtk_DoCalibration(stVoipDoCalibration.ch_id, &stVoipDoCalibration);
		printf("result : 0x%x\n", stVoipDoCalibration.result);
	}
#endif	

	fclose(fp);
	free(pCoeff);
	return 0;
}
#else
int main(void){
	return 0;
}
#endif
