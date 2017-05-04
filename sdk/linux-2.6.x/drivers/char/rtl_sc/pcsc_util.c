#include <linux/kernel.h>

#ifdef __KERNEL__
#define PCSCUTIL_PRINT printk
#else
#define PCSCUTIL_PRINT printf
#endif

char getChar(char ch){
	char c = 0x0;
	if ( ch >= 0x30 && ch <= 0x39 )
		c = ch - 0x30;
	else if (ch >= 0x41 && ch <= 0x46)
		c = ch - 64 + 9;
	else if (ch >= 0x61 && ch <= 0x66)
		c = ch - 96 + 9;
	return c;
}

/*
 * transfer string to hex and check if correct
 */
int asciitobin(unsigned char *atrs, unsigned char *atrh, int len){
	int space = 0, step = 0, count = 0;
	unsigned char c0 = 0x0, c1 = 0x0;
	
	while(len > 0){	
		if ( *(atrs+1) == 0x20){
			space = 1;
			//PCSCUTIL_PRINT("%c:", *atrs);
		}else{
			space = 0;
			//PCSCUTIL_PRINT("%c%c:", *atrs, *(atrs+1));
		}
			
		
		if ( space ){
			if ( *atrs >= 0x30 && *atrs <= 0x39 )
				*atrh = *atrs - 0x30;
			else if (*atrs >= 0x41 && *atrs <= 0x46)
				*atrh = *atrs - 64 + 9;
			else if (*atrs >= 0x61 && *atrs <= 0x66)
				*atrh = *atrs - 96 + 9;
			else{
				PCSCUTIL_PRINT("return 1\n");
				return -1;
			}
			step = 2;
		}else{
			if (	( *atrs < 0x30 ) || ( *atrs > 0x66 ) || 
					( *atrs > 0x39 && *atrs < 0x40 ) || 
					( *atrs > 0x46 && *atrs < 0x61 )) {
				PCSCUTIL_PRINT("return 2:0x[%x]\n", *atrs);
				return -1;
			}else if (	( *(atrs+1) < 0x30 ) || ( *(atrs+1) > 0x66 ) || 
						( *(atrs+1) > 0x39 && *(atrs+1) < 0x40 ) || 
						( *(atrs+1) > 0x46 && *(atrs+1) < 0x61 )) {
				PCSCUTIL_PRINT("return 3:0x[%x]\n", *(atrs+1));
				return -1;
			}else{
				//PCSCUTIL_PRINT("enter 3 0x%x, 0x%x\n", *atrs, *(atrs+1));
				c0 = getChar(*(atrs+1));
				c1 = getChar(*atrs);
				//PCSCUTIL_PRINT("enter 3 c0 0x%x, 0x%x\n", c0, c0<<4);
				//PCSCUTIL_PRINT("enter 3 c1 0x%x, 0x%x\n", c1, c1<<4);
				*atrh = (c1 <<4) | c0;
			}
			step = 3;
		}	
		//PCSCUTIL_PRINT("0x%x\n", *atrh);
#if 0
		if ( *atrs == 0x30 ){	// 0
			if ( space )
				*atrh = 0x0;
			else {
			}
		}else if ( *atrs == 0x31 ){	// 1
			if ( space )
				*atrh = 0x1;
			else {
			}			
		}else if ( *atrs == 0x32 ){	// 2
			if ( space )
				*atrh = 0x2;
			else {
			}			
		}else if ( *atrs == 0x33 ){	// 3
			if ( space )
				*atrh = 0x3;
			else {
			}			
		}else if ( *atrs == 0x34 ){	// 4
			if ( space )
				*atrh = 0x4;
			else {
			}			
		}else if ( *atrs == 0x35 ){	// 5
			if ( space )
				*atrh = 0x5;
			else {
			}			
		}else if ( *atrs == 0x36 ){	// 6
			if ( space )
				*atrh = 0x6;
			else {
			}			
		}else if ( *atrs == 0x37 ){	// 7
			if ( space )
				*atrh = 0x7;
			else {
			}			
		}else if ( *atrs == 0x38 ){	// 8
			if ( space )
				*atrh = 0x8;
			else {
			}			
		}else if ( *atrs == 0x39 ){	// 9
			if ( space )
				*atrh = 0x9;
			else {
			}			
		}else if ( *atrs == 0x41 || *atrh == 0x61 ){	// a
			if ( space )
				*atrh = 0xa;
			else {
			}			
		}else if ( *atrs == 0x42 || *atrh == 0x62 ){	// b
			if ( space )
				*atrh = 0xb;
			else {
			}		
		}else if ( *atrs == 0x43 || *atrh == 0x63 ){	// c
			if ( space )
				*atrh = 0xc;
			else {
			}			
		}else if ( *atrs == 0x44 || *atrh == 0x64 ){	// d
			if ( space )
				*atrh = 0xd;
			else {
			}			
		}else if ( *atrs == 0x45 || *atrh == 0x65 ){	// e
			if ( space )
				*atrh = 0xe;
			else {
			}			
		}else if ( *atrs == 0x46 || *atrh == 0x66 ){	// f
			if ( space )
				*atrh = 0xf;
			else {
			}	
		}
#endif		
		space = 0;
		atrh ++;
		atrs += step;
		count++;
		len -= step;
	}
	return count;
}
