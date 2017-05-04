/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 si_ua_base64.c
 * Purpose:		 		 
 * Created:		 		 Feb 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>  

#include <si_print_api.h>

#include "si_ua_base64.h"
 
#define INV	    -1
#define PADDING	    '='
#define SI_ASSERT_RETURN(expr,retval)    assert(expr)

static void base256to64(unsigned char c1, unsigned char c2, unsigned char c3, int padding, char *output);

const char base64_char[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/' 
};

static int base256_char(char c)
{
    if (c >= 'A' && c <= 'Z')
	return (c - 'A');
    else if (c >= 'a' && c <= 'z')
	return (c - 'a' + 26);
    else if (c >= '0' && c <= '9')
	return (c - '0' + 52);
    else if (c == '+')
	return (62);
    else if (c == '/')
	return (63);
    else {
	assert(!"Should not happen as '=' should have been filtered");
	return INV;
    }
}


static void base256to64(unsigned char c1, unsigned char c2, unsigned char c3, 
			int padding, char *output)
{
    *output++ = base64_char[c1>>2];
    *output++ = base64_char[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)];
    switch (padding) {
    case 0:
	*output++ = base64_char[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
	*output = base64_char[c3 & 0x3F];
	break;
    case 1:
	*output++ = base64_char[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
	*output = PADDING;
	break;
    case 2:
    default:
	*output++ = PADDING;
	*output = PADDING;
	break;
    }
}


int si_base64_encode(const unsigned char *input, int in_len,
				     char *output, int *out_len)
{
    const unsigned char *pi = input;
    unsigned char c1, c2, c3;
    int i = 0;
    char *po = output;

    SI_ASSERT_RETURN(input && output && out_len, PJ_EINVAL);
    SI_ASSERT_RETURN(*out_len >= SI_BASE256_TO_BASE64_LEN(in_len), 
		     100);

    while (i < in_len) {
	c1 = *pi++;
	++i;

	if (i == in_len) {
	    base256to64(c1, 0, 0, 2, po);
	    po += 4;
	    break;
	} else {
	    c2 = *pi++;
	    ++i;

	    if (i == in_len) {
		base256to64(c1, c2, 0, 1, po);
		po += 4;
		break;
	    } else {
		c3 = *pi++;
		++i;
		base256to64(c1, c2, c3, 0, po);
	    }
	}
		po += 4;
    }
 
	*out_len = po - output;
    return 0;
}
 
int si_base64_decode(const char* input, int in_len,
				     unsigned char *out, int *out_len)
{
    const char *buf = input;
    int len = in_len;
    int i, j;
    int c1, c2, c3, c4;

    SI_ASSERT_RETURN(input && out && out_len, PJ_EINVAL);

    while (buf[len-1] == '=' && len)
	--len;

    SI_ASSERT_RETURN(*out_len >= SI_BASE64_TO_BASE256_LEN(len), 
		     100);

    for (i=0, j=0; i+3 < len; i+=4) {
	c1 = base256_char(buf[i]);
	c2 = base256_char(buf[i+1]);
	c3 = base256_char(buf[i+2]);
	c4 = base256_char(buf[i+3]);

	out[j++] = (unsigned char)((c1<<2) | ((c2 & 0x30)>>4));
	out[j++] = (unsigned char)(((c2 & 0x0F)<<4) | ((c3 & 0x3C)>>2));
	out[j++] = (unsigned char)(((c3 & 0x03)<<6) | (c4 & 0x3F));
    }

    if (i < len) {
	c1 = base256_char(buf[i]);

	if (i+1 < len)
	    c2 = base256_char(buf[i+1]);
	else 
	    c2 = (INV);

	if (i+2 < len)
	    c3 = base256_char(buf[i+2]);
	else
	    c3 = (INV);

	c4 = (INV);

	if (c2 != INV) {
	    out[j++] = (unsigned char)((c1<<2) | ((c2 & 0x30)>>4));
	    if (c3 != INV) {
		out[j++] = (unsigned char)(((c2 & 0x0F)<<4) | ((c3 & 0x3C)>>2));
		if (c4 != INV) {
		    out[j++] = (unsigned char)(((c3 & 0x03)<<6) | (c4 & 0x3F));
		}
	    }
	}
	
    }

    assert(j < *out_len);
    *out_len = j;

    return 0;
}


