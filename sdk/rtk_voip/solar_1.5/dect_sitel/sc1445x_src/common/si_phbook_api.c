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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:			si_phbook_api.c
 * Purpose:		
 * Created:		10/10/2007
 * By:			  KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>

#include "si_phbook_api.h"
#include "../SiPhoneUA/si_ua_base64.h"

#define ERROR_FileNotFound	    -1
#define ERROR_SyntaxError	    -2
#define ERROR_FileNotUpdated	 1

/*========================== Local macro definitions ========================*/
#define DirWWW	"/mnt/flash/ApplicationData/boa/www"

/*========================== Global definitions =============================*/
int photo_index = 0;

/*========================== Local function prototypes ======================*/
static void phbookSort();
static void ConvertToUpperCase(char *str);

/*========================== Local data definitions =========================*/
phbookContact pb_contact[MAX_NUM_OF_CONTACTS];
int num_of_contacts = 0;

int PhBookCurrentPosition = -1;

char FilePHBOOK[] = PHBOOK_VCARD_FILEPATH;

phBookErrorCode PhBookErrorCodes[] = {
  {ERROR_PhFileNotFound, "Phone Book file not found"},
	{ERROR_PhNoMemoryAvailable, "List is full"},
  {ERROR_PhContactExists, "Contact exists"},
  {ERROR_PhEmptyBook, "List is empty"},
	{ERROR_PhContactNotFound, "Contact not found"}
};

/*========================== Function definitions ===========================*/
 
int phbookLoadContacts(void)
{
	int ret;
 	ret = phbook_vcardLoadContacts(&num_of_contacts, pb_contact);
 
	return ret;
}

int phbookStoreContacts(void)
{
	return phbook_vcardStoreContacts(&num_of_contacts, pb_contact);
}

int phbookAddContact(phbookContact *contact_to_add)
{
	int pos;
  
  if(num_of_contacts >= MAX_NUM_OF_CONTACTS)
   	return ERROR_PhNoMemoryAvailable;
 
  pos = phbookFindContact(contact_to_add->name);
  if(pos > -1)
		return ERROR_PhContactExists;
 
  memcpy(&pb_contact[num_of_contacts], contact_to_add, sizeof(phbookContact));
  num_of_contacts++;
  
  if(num_of_contacts > 1)
   	phbookSort();

 	return 0;
}


int phbookEditContact(char *name, phbookContact *new_contact)
{
	int pos, new_pos;
  
  if(num_of_contacts == 0)
	  return ERROR_PhEmptyBook;
  
  pos = phbookFindContact(name);
 	if(pos < 0 )
		return ERROR_PhContactNotFound;

  new_pos = phbookFindContact(new_contact->name);
  if((new_pos >= 0) && (new_pos != pos))
		return ERROR_PhContactExists;
  
  memcpy(&pb_contact[pos], new_contact, sizeof(phbookContact));

  if((num_of_contacts > 1) && (new_pos != pos))
   	phbookSort();

  return 0;
}


int phbookDeleteContact(char *name)
{
	int pos, i;
  
  if(num_of_contacts == 0)
		return ERROR_PhEmptyBook;
  
  pos = phbookFindContact(name);
 	if(pos < 0 )
		return ERROR_PhContactNotFound;
  
  for(i = pos ; i < num_of_contacts - 1 ; i++)
   	memcpy(&pb_contact[i], &pb_contact[i + 1], sizeof(phbookContact));
    
  num_of_contacts--; 

  return 0;
}


int phbookFindContact(char *name)
{
	int i;
  
  for(i = 0 ; i < num_of_contacts ; i++) {
  if(!strcmp(pb_contact[i].name, name))
   	return i;
  }

  return ERROR_PhContactNotFound;
}


phbookContact* phbookGetContactDetails(char *name)
{
	int pos;
 
  pos = phbookFindContact(name);
  if(pos < 0)
		return (phbookContact *)NULL;
   
  return (phbookContact *)&pb_contact[pos];
}


int phbookMemSize(void)
{
	return MAX_NUM_OF_CONTACTS;
}


int phbookMemUsage(void)
{
  return num_of_contacts;
}


phbookContact* phbookGotoFirst(void)
{
	if (!num_of_contacts)
		return (phbookContact*)NULL;

	PhBookCurrentPosition = 0;

	return (phbookContact*)&pb_contact[PhBookCurrentPosition];
}


phbookContact* phbookGotoLast(void)
{
	if (!num_of_contacts)
		return (phbookContact*)NULL;

	PhBookCurrentPosition = num_of_contacts -1;

	return (phbookContact*)&pb_contact[PhBookCurrentPosition];
}


phbookContact* phbookGotoNext(void)
{
	if (!num_of_contacts)
		return (phbookContact*)NULL;

	PhBookCurrentPosition++;
	if((PhBookCurrentPosition < 0) || (PhBookCurrentPosition > num_of_contacts-1))
		PhBookCurrentPosition = 0;

 	return (phbookContact*)&pb_contact[PhBookCurrentPosition];
}


phbookContact* phbookGotoPrevious(void)
{
	if (!num_of_contacts)
		return (phbookContact*)NULL;
 
  PhBookCurrentPosition--;
 	if ((PhBookCurrentPosition < 0) || (PhBookCurrentPosition > num_of_contacts-1))
		PhBookCurrentPosition = num_of_contacts-1;

	return (phbookContact*)&pb_contact[PhBookCurrentPosition];
}


char* phbookGetErrorString(int errCode)
{
	int i;

	for(i=0 ; PhBookErrorCodes[i].ErrorId ; i++)
	{
		if(PhBookErrorCodes[i].ErrorId == errCode)
      	return PhBookErrorCodes[i].ErrorString;
	}
      
  return (char*)NULL;
}

static void phbookSort(void)
{
	int i, j;
  phbookContact temp;

  for(i = num_of_contacts - 1 ; i >= 0 ; i--) {
   	for(j = 1 ; j <= i ; j++) {
     	if(strcmp(pb_contact[j - 1].name, pb_contact[j].name) > 0) {
     		memcpy(&temp, &pb_contact[j - 1], sizeof(phbookContact));
     		memcpy(&pb_contact[j - 1], &pb_contact[j], sizeof(phbookContact));
     		memcpy(&pb_contact[j], &temp, sizeof(phbookContact));
     	}
    }
  }
}

static char buffer[11000];
static unsigned char output[8300];

int phbook_vcardLoadContacts(int *num_of_contacts_ptr, phbookContact *pb_contacts_ptr)
{
	FILE *fp, *fp_photo;
	int output_length;
	char *ptr, *delimiter_ptr;
	char flag;
	//char command[100];
  
	//sprintf(command, "mkdir %s/photos", DirWWW);
	//system(command);

	fp = fopen(FilePHBOOK, "r");
  	if(fp == NULL) {
			*num_of_contacts_ptr = 0;
			return ERROR_FileNotFound;
  	}

	flag = 0;
	*num_of_contacts_ptr = 0;
	memset(pb_contacts_ptr, 0, sizeof(phbookContact) * MAX_NUM_OF_CONTACTS);
	while(1) {
		if(fgets(buffer, sizeof(buffer), fp) == NULL)
			break;

		if(buffer[strlen(buffer) - 2] == '\r')
			buffer[strlen(buffer) - 2] = '\0';
		else
			buffer[strlen(buffer) - 1] = '\0';

		delimiter_ptr = strchr(buffer, ':');
		if(delimiter_ptr == NULL)
			continue;

		ptr = &buffer[strlen(buffer) - 1];
		while(isspace(*ptr)) {
			*ptr = '\0';
			ptr--;
		}

		ptr = buffer;
		while(isspace(*ptr))
			ptr++;

		*delimiter_ptr = '\0';
		ConvertToUpperCase(ptr);

		if(flag == 0) {
			if(strcmp(ptr, "BEGIN")) {
				*num_of_contacts_ptr = 0;
				fclose(fp);
				return ERROR_SyntaxError;
			}else{
				ptr = delimiter_ptr + 1;

				ConvertToUpperCase(ptr);

				if(strcmp(ptr, "VCARD")) {
					*num_of_contacts_ptr = 0;
					fclose(fp);
					return ERROR_SyntaxError;
				}else
					flag = 1;
			}
		}else{
			if(!strcmp(ptr, "BEGIN")) {
				*num_of_contacts_ptr = 0;
				fclose(fp);
				return ERROR_SyntaxError;
			}else if(!strcmp(ptr, "FN")) {
				ptr = delimiter_ptr + 1;

				while(*ptr) {
					if(isspace(*ptr))
						*ptr = '_';

					ptr++;
				}

				ptr = delimiter_ptr + 1;
				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].name, ptr, 32);
				pb_contacts_ptr[*num_of_contacts_ptr].name[31] = '\0';

				if(pb_contacts_ptr[*num_of_contacts_ptr].name[0] == '\0') {
					*num_of_contacts_ptr = 0;
					fclose(fp);
					return ERROR_SyntaxError;
				}
			}else if(!strcmp(ptr, "END")) {
				ptr = delimiter_ptr + 1;

				ConvertToUpperCase(ptr);

				if(strcmp(ptr, "VCARD")) {
					*num_of_contacts_ptr = 0;
					fclose(fp);
					return ERROR_SyntaxError;
				}else{
					if(pb_contacts_ptr[*num_of_contacts_ptr].name[0] == '\0') {
						*num_of_contacts_ptr = 0;
						fclose(fp);
						return ERROR_SyntaxError;
					}

					flag = 0;
					(*num_of_contacts_ptr)++;

					if(*num_of_contacts_ptr >= MAX_NUM_OF_CONTACTS)
						break;
				}
			}else if(!strncmp(ptr, "TEL;", 4) && strstr(ptr, "HOME")) {
				ptr = delimiter_ptr + 1;

				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].home_number, ptr, 64);
				pb_contacts_ptr[*num_of_contacts_ptr].home_number[63] = '\0';
			}else if(!strncmp(ptr, "TEL;", 4) && strstr(ptr, "WORK")) {
				ptr = delimiter_ptr + 1;

				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].office_number, ptr, 64);
				pb_contacts_ptr[*num_of_contacts_ptr].office_number[63] = '\0';
			}else if(!strncmp(ptr, "TEL;", 4) && strstr(ptr, "CELL")) {
				ptr = delimiter_ptr + 1;

				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].mobile_number, ptr, 64);
				pb_contacts_ptr[*num_of_contacts_ptr].mobile_number[63] = '\0';
			}else if(!strncmp(ptr, "EMAIL;", 6) && strstr(ptr, "INTERNET")) {
				ptr = delimiter_ptr + 1;

				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].e_mail, ptr, 64);
				pb_contacts_ptr[*num_of_contacts_ptr].e_mail[63] = '\0';
			}else if(!strncmp(ptr, "PHOTO;", 6) && strstr(ptr, "VALUE=URI")) {
				ptr = delimiter_ptr + 1;

				strncpy(pb_contacts_ptr[*num_of_contacts_ptr].photo, ptr, 256);
				pb_contacts_ptr[*num_of_contacts_ptr].photo[255] = '\0';

				photo_index++;
			}else if(!strncmp(ptr, "PHOTO;", 6) && strstr(ptr, "ENCODING")) {
				if(pb_contacts_ptr[*num_of_contacts_ptr].name[0] == '\0') {
					*num_of_contacts_ptr = 0;
					fclose(fp);
					return ERROR_SyntaxError;
				}

				if(photo_index >= MAX_NUM_OF_CONTACTS)
					continue;

				ptr = delimiter_ptr + 1;

				output_length = sizeof(output);
				si_base64_decode(ptr, strlen(ptr), output, &output_length);

				ptr = buffer;
				if(strstr(ptr, "TYPE=BMP"))
					sprintf(pb_contacts_ptr[*num_of_contacts_ptr].photo, "photos/%s.bmp", pb_contacts_ptr[*num_of_contacts_ptr].name);
				else if(strstr(ptr, "TYPE=PNG"))
					sprintf(pb_contacts_ptr[*num_of_contacts_ptr].photo, "photos/%s.png", pb_contacts_ptr[*num_of_contacts_ptr].name);
				else
					sprintf(pb_contacts_ptr[*num_of_contacts_ptr].photo, "photos/%s.jpeg", pb_contacts_ptr[*num_of_contacts_ptr].name);

				sprintf(buffer, "%s/%s", DirWWW, pb_contacts_ptr[*num_of_contacts_ptr].photo);
				fp_photo = fopen(buffer, "wb");
			  	if(fp_photo != NULL) {
					fwrite(output, output_length, 1, fp_photo);
					fclose(fp_photo);
					photo_index++;
  				}
			}
		}
	}

	fclose(fp);

	if(*num_of_contacts_ptr > 1)
   	phbookSort();

	return 0;
}

int phbook_vcardStoreContacts(int *num_of_contacts_ptr, phbookContact *pb_contacts_ptr)
{
	FILE *fp;
	int i, j;
  
	fp = fopen(FilePHBOOK, "w");
  if(fp == NULL)
		return ERROR_FileNotFound;

	for(i = 0 ; i < *num_of_contacts_ptr ; i++) {
		fputs("BEGIN:VCARD\r\n", fp);
		fputs("VERSION:3.0\r\n", fp);

		for(j = 0 ; pb_contacts_ptr[i].name[j] != '\0' ; j++) {
			if(isspace(pb_contacts_ptr[i].name[j]))
				pb_contacts_ptr[i].name[j] = '_';
		}
		fprintf(fp, "N:%s;;;;\r\n", pb_contacts_ptr[i].name);
		fprintf(fp, "FN:%s\r\n", pb_contacts_ptr[i].name);

		if(pb_contacts_ptr[i].home_number[0] != '\0')
			fprintf(fp, "TEL;TYPE=HOME;TYPE=VOICE:%s\r\n", pb_contacts_ptr[i].home_number);
		if(pb_contacts_ptr[i].mobile_number[0] != '\0')
			fprintf(fp, "TEL;TYPE=CELL;TYPE=VOICE:%s\r\n", pb_contacts_ptr[i].mobile_number);

		if(pb_contacts_ptr[i].photo[0] != '\0')
			fprintf(fp, "PHOTO;VALUE=URI:%s\r\n", pb_contacts_ptr[i].photo);

		fputs("END:VCARD\r\n", fp);
	}

  	fclose(fp);
  
  	return 0;
}

static void ConvertToUpperCase(char *str)
{
	char *ptr;

	ptr = str;
	while(*ptr) {
		if((*ptr >= 'a') && (*ptr <= 'z'))
			*ptr -= 32;

		ptr++;
	}
}
