#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <string.h>
#include <fcntl.h>
#include "bus.h"

unsigned char tx_1[5] = { 0x10, 0x00, 0x01, 0xC0, 0xC0 };
unsigned char tx_2[5] = { 0x10, 0x00, 0x01, 0x81, 0x81 };
unsigned char tx_3[32] =
			{ 0x10, 0x00, 0x1C, 0x01, 0x00, 0x0F, 0x0C, 0x40, 0x01, 
				0x07, 0x01, 0x64, 0x00, 0x00, 0x64, 0x00, 0x01, 0x64,
				0x00, 0x00, 0x64, 0x00, 0x01, 0x64, 0x00, 0x00, 0xE8,
				0x03, 0x02, 0xFF, 0xFF, 0x46 };
unsigned char tx_4[9] = { 0x10, 0x00, 0x05, 0x11, 0x00, 0x0F, 0x00, 0x41, 0x61 };
unsigned char tx_5[9] = { 0x10, 0x00, 0x05, 0x29, 0x00, 0x0F, 0x02, 0x40, 0x7A };

/******  for CMBS *******/
unsigned char dspg_hello[18]  = {0x11, 0xDA, 0xDA, 0xDA, 0xDA, 0x0D, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x05, 0x00, 0x00, 0x03, 0x03, 0xff, 0xff};
//unsigned char dspg_hello[17]  = {0xDA, 0xDA, 0xDA, 0xDA, 0x0D, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x05, 0x00, 0x00, 0x03, 0x03, 0xff, 0xff};//-->fail
unsigned char dspg_hello2[18]  = {0x11, 0xDA, 0xDA, 0xDA, 0xDA, 0x0D, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x03, 0x03, 0xff, 0xff};// --> fail

unsigned char dspg_read_req[2] = {0x00, 0xFF};
unsigned char dspg_read_req2[2] = {0xAA, 0xFF}; // --> fail
/******  for CMBS *******/

int main(int argc, char *argv[])
{
	int i;
	unsigned char buf[255];
	
	/* open device */
	if (Bus_Open() < 0)
		printf("failed to open %s\n", MODEMDEVICE);
	else
		printf(" %s opened for O_RDWR \n", MODEMDEVICE);

	/* test case */
	if (atoi(argv[1]) == 1)
	{
		for (i=0; i<6; i++) Bus_Read_byte_blocking(&buf[i]);
	}
	else if  (atoi(argv[1]) == 2)
   	{
   		for (i=0; i<6; i++) Bus_Read_byte_blocking(&buf[i]);
   		
   		Bus_Write_byte_blocking(tx_1, sizeof(tx_1));//SAMB
   		
		for (i=0; i<11; i++) Bus_Read_byte_blocking(&buf[i]);
		
		Bus_Write_byte_blocking(tx_2, sizeof(tx_2));
		Bus_Write_byte_blocking(tx_3, sizeof(tx_3));;
		Bus_Write_byte_blocking(tx_4, sizeof(tx_4));
		Bus_Write_byte_blocking(tx_5, sizeof(tx_5));
		
		for (i=0; i<35; i++) Bus_Read_byte_blocking(&buf[i]);
	}
	else if (argv[1][0] == 'd')	// for "d"spg CMBS
	{
		if  (atoi(argv[2]) == 1)
		{
   			Bus_Write_byte_blocking(dspg_hello, sizeof(dspg_hello));
			printf("send hello\n");
		}
		else if  (atoi(argv[2]) == 2)
		{
   			Bus_Write_byte_blocking(dspg_read_req, sizeof(dspg_read_req));
			printf("send read req\n");
		}
		else if  (atoi(argv[2]) == 3)
		{
			Bus_Write_byte_blocking(dspg_hello2, sizeof(dspg_hello2));
			printf("send hello2\n");
		}
		else if (atoi(argv[2]) == 4)
		{
   			Bus_Write_byte_blocking(dspg_read_req2, sizeof(dspg_read_req2));
			printf("send read req2\n");
		}
		else if (argv[2][0] == 'r')
		{
			Bus_Read_blocking(&buf[0], atoi(argv[3]));
			printf("Read: ");
			for (i=0; i < (atoi(argv[3])); i++)
				printf("%x, ", buf[i]);
			printf("\n");
		}
		else if (argv[2][0] == 'w')
		{
			buf[0] = atoi(argv[3]);
			Bus_Write_byte_blocking( &buf[0], 1);
			printf("W: 0x%x\n", atoi(argv[3]));
		}
	}


	/* close device */
	Bus_Close();
	
	return 0;
}

