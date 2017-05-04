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
 * File:			phbook_api.h
 * Purpose:		
 * Created:		10/10/2007
 * By:			  KF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_PHBOOK_API_H
#define SI_PHBOOK_API_H
 
/*========================== Include files ==================================*/


/*========================== Local macro definitions & typedefs =============*/
 
#define PHBOOK_VCARD_FILEPATH			"/mnt/flash/ApplicationData/storage/sc14450_fs/phbook/phbook.vcf"

#define MAX_NUM_OF_CONTACTS 16

#define ERROR_PhFileNotFound		  (-101)
#define ERROR_PhNoMemoryAvailable	(-102)
#define ERROR_PhContactExists		  (-103)
#define ERROR_PhEmptyBook			    (-104)
#define ERROR_PhContactNotFound		(-105)

typedef struct _phbookContact {
	char name[32];
	char home_number[64];
	char office_number[64];
	char mobile_number[64];
	char home_2_number[64];
	char office_2_number[64];
	char mobile_2_number[64];
	char speed_dial_number;
	char ring_tone;
	char e_mail[64];
	char photo[256];
	char reserved[2];
} phbookContact;

typedef struct _phBookErrorCode {
   int ErrorId;
   char ErrorString[35];
}phBookErrorCode;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
int phbookLoadContacts(void);
int phbookStoreContacts(void);
int phbookAddContact(phbookContact *contact_to_add);
int phbookEditContact(char *name, phbookContact *new_contact);
int phbookDeleteContact(char *name);
int phbookFindContact(char *name);
phbookContact *phbookGetContactDetails(char *name);
int phbookMemSize(void);
int phbookMemUsage(void);
phbookContact* phbookGotoFirst(void);
phbookContact* phbookGotoLast(void);
phbookContact* phbookGotoNext(void);
phbookContact* phbookGotoPrevious(void);
char* phbookGetErrorString(int errCode);
int sc1445x_phoneapi_init_pcm_ports(void);
int phbook_vcardLoadContacts(int *num_of_contacts_ptr, phbookContact *pb_contacts_ptr);
int phbook_vcardStoreContacts(int *num_of_contacts_ptr, phbookContact *pb_contacts_ptr);
void phbook_vcardSort(int num_of_contacts, phbookContact *pb_contacts_ptr);

#endif /* SI_PHBOOK_API_H */
