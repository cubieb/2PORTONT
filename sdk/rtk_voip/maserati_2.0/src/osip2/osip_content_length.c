/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Aymeric MOIZARD jack@atosc.org
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>

uint32_t rcm_decimal_digit_count(uint32_t decimal_number);

int osip_content_length_init(osip_content_length_t ** cl)
{
	*cl = (osip_content_length_t *) osip_malloc(sizeof(osip_content_length_t));
	if (*cl == NULL)
		return OSIP_NOMEM;
	(*cl)->value = NULL;
	return OSIP_SUCCESS;
}

//[SD6, bohungwu, exosip 3.5 integration
//This is a compact/simplified version of osip_message_set_content_length
/* adds the content_length header to message.       */
/* INPUT : int contetn_length  | integer value of content length.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int rcm_osip_message_set_content_length(osip_message_t * sip, int content_length)
{
	int i;
	uint32_t decimal_digit;
	char clen[10];

	if(content_length <= 0)
		return OSIP_SYNTAXERROR;

	snprintf(clen,sizeof(clen),"%i",content_length);
	i=osip_message_set_content_length(sip,clen);
	return i;
	
#if 0	
	if (sip->content_length != NULL)
		return OSIP_SYNTAXERROR;

	i = osip_content_length_init(&(sip->content_length));
	if (i != 0)
		return i;
	sip->message_property = 2;

	decimal_digit = rcm_decimal_digit_count((uint32_t)content_length);
	if (decimal_digit+ 1 < 2)
		return OSIP_SYNTAXERROR;
	sip->content_length->value = (char *) osip_malloc(decimal_digit + 1);
	if (sip->content_length->value == NULL)
		return OSIP_NOMEM;
	snprintf(sip->content_length->value, decimal_digit, "%d", content_length);

	return OSIP_SUCCESS;
#endif
}

uint32_t rcm_decimal_digit_count(uint32_t decimal_number)
{
	uint32_t ret_val = 0;
	uint32_t mask = 0xFFFF0000;

	//Calcualte the digit in hexdeciaml 
	if((decimal_number & mask) != 0)
	{
		ret_val |= (0x1 << 2);
		decimal_number >>= 16;
	}
		
	mask = 0xFF00;
	if((decimal_number & mask) != 0)
	{
		ret_val |= (0x1 << 1);
		decimal_number >>= 8;
	}

	mask = 0xF0;
	if((decimal_number & mask) != 0)
	{
		ret_val |= (0x1 << 0);
	}

	ret_val++;

	//Convert hex digit to decimal digit
	//Decimal digit of x =lg(x)+1=lg(16)*log16(x)+1=(log16(x)*5/4) +1
	ret_val = (ret_val * 5)/4;
	ret_val++;

	
	return ret_val;

}
//]

/* adds the content_length header to message.       */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int osip_message_set_content_length(osip_message_t * sip, const char *hvalue)
{
	int i;

	if (hvalue == NULL || hvalue[0] == '\0')
		return OSIP_SUCCESS;

	if (sip->content_length != NULL)
		return OSIP_SYNTAXERROR;
	i = osip_content_length_init(&(sip->content_length));
	if (i != 0)
		return i;
	sip->message_property = 2;
	i = osip_content_length_parse(sip->content_length, hvalue);
	if (i != 0) {
		osip_content_length_free(sip->content_length);
		sip->content_length = NULL;
		return i;
	}

	return OSIP_SUCCESS;
}

int
osip_content_length_parse(osip_content_length_t * content_length,
						  const char *hvalue)
{
	size_t len;

	if (hvalue == NULL)
		return OSIP_BADPARAMETER;
	len = strlen(hvalue);
	if (len + 1 < 2)
		return OSIP_SYNTAXERROR;
	content_length->value = (char *) osip_malloc(len + 1);
	if (content_length->value == NULL)
		return OSIP_NOMEM;
	osip_strncpy(content_length->value, hvalue, len);
	return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the content_length header.            */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
osip_content_length_t *osip_message_get_content_length(const osip_message_t * sip)
{
	return sip->content_length;
}
#endif

/* returns the content_length header as a string.          */
/* INPUT : osip_content_length_t *content_length | content_length header.  */
/* returns null on error. */
int osip_content_length_to_str(const osip_content_length_t * cl, char **dest)
{
	if (cl == NULL)
		return OSIP_BADPARAMETER;
	*dest = osip_strdup(cl->value);
	if (*dest == NULL)
		return OSIP_NOMEM;
	return OSIP_SUCCESS;
}

/* deallocates a osip_content_length_t strcture.  */
/* INPUT : osip_content_length_t *content_length | content_length header. */
void osip_content_length_free(osip_content_length_t * content_length)
{
	if (content_length == NULL)
		return;
	osip_free(content_length->value);
	osip_free(content_length);
}

int
osip_content_length_clone(const osip_content_length_t * ctl,
						  osip_content_length_t ** dest)
{
	int i;
	osip_content_length_t *cl;

	*dest = NULL;
	if (ctl == NULL)
		return OSIP_BADPARAMETER;
	/*
	   empty headers are allowed:
	   if (ctl->value==NULL) return -1;
	 */
	i = osip_content_length_init(&cl);
	if (i != 0)					/* allocation failed */
		return i;
	if (ctl->value != NULL) {
		cl->value = osip_strdup(ctl->value);
		if (cl->value == NULL) {
			osip_content_length_free(cl);
			return OSIP_NOMEM;
		}
	}

	*dest = cl;
	return OSIP_SUCCESS;
}
