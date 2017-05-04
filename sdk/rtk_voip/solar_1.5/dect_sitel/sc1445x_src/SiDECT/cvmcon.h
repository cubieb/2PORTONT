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
 * File:			cvmcon.h
 * Purpose:		
 * Created:		9/2/2008
 * By:			  EF
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SVMCON_H
#define SVMCON_H
 
/*========================== Include files ==================================*/

/*========================== Switches ==================================*/
#define SET_RFPI //Adds support for changing the RFPI of the basestation

/*========================== Local macro definitions & typedefs =============*/

/* Registration mode flags */
#define NORMAL_MODE 		0x00 //Normal operation
#define REGISTER_MODE	 	0x01 //Handsets can be registered
#define DELETE_ALL_MODE		0x02 //All registered handsets are deleted
#define FWU_UPGRADE_MODE	0x03 //Firmware upgrade mode

/*========================== Global variables ===============================*/

extern unsigned long ref_VersionHex;
extern unsigned char ref_AccessCode[ 2 ];	// BCD format 

/*========================== Global function prototypes =====================*/

void dect_main(void);
void SetRegistrationMode(unsigned char mode);
int HandleSendConnectReq (int codec, unsigned char accountid, const char* dialednum, const char* callerid);
void HandleSendReleaseReq(unsigned char accountid);

/* call state check function */
extern int CheckIfCallStateIsBusy( unsigned char HandsetId );
extern int CheckIfCallStateIsRinging( unsigned char HandsetId );

/* init functions */
extern int FwuSetFile( const char *filename );


#endif /* SVMCON_H */


