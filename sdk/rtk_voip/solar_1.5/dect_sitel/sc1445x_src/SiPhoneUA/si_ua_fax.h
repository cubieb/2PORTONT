
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
 * File:		si_ua_fax.h
 * Purpose:		
 * Created:		Fax 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_FAX_H
#define SI_UA_FAX_H
#include "si_ua_init.h"

/*========================== Local macro definitions & typedefs =============*/

/* This type is used to pass T38 packets between the T38 Fax Relay and the IP stack
*/

/* - For packets passed to IP stack, the app should send the contexts of the 
** FR38_T38Obj.data unmodified to the IP link.  The valid number of octets in the 
** buffer is specified by FR38_T38Obj.numBits.  The FR38_T38Obj.data is defined as an 
** array of short.  The high octet of each short should be sent first.  An odd number 
** of octets is possible.  The app must unpack this array if the byte ordering is not 
** as required by the IP stack.  The FR38_T38Obj.seqNumber field is for informational 
** purposes only and the app can ignore this value for normal operations.
** - For packets passed to Fax Relay, the app must ensure that the buffer is packed 
** with high octets received first.  The numBits must be set.  The seqNumber field is 
** ignored by the relay except for RTP operation.  For RTP operation, the seqNumber 
** field should be set by the app to the RTP sequence number.
*/
typedef struct
{
  
  unsigned short  seqNumber;
  unsigned short  numBits;  /* count of valid bits in data */

  /* Buffer of size T38FaxMaxDatagram must be allocated by the sender 
  ** and freed by the receiver 
  */
  unsigned short  *data; /*-octets are packed */
} T38Packet_t;

/* This data type is used for passing PCM data and T38 packet to the T38 algorithm. 
** The algorithm  will process the input PCM samples and T38 packet and will return 
** a PCM output buffer to be sent to PCM interface and a T38 packet to be send to the IP network. 
*/
typedef struct 
{
  unsigned short  *src_ptr;             /* input PCM buffer pointer. From PCM */
  T38Packet_t     *fax_packetIn_ptr;    /* Input fax packet pointer. From IP network */
  unsigned short  *dst_ptr;             /* output PCM buffer pointer. To PCM     */
  T38Packet_t     *fax_packetOut_ptr;   /* output fax packet pointer. To IP network */
} T38Obj_t;

/* Type definitions for T38 SDP extensions. 
*/
typedef enum
{
  localTCF,     
  transferredTCF
}T38FaxRateManagement_t;

typedef enum
{
  t38UDPNone,
  t38UDPFEC,
  t38UDPRedundancy
}T38FaxUdpEC_t;

/* Type definitions for T38 SDP extensions. 
*/
typedef enum
{
  /* DO NOT change values. They match #defines in fr38.h */
  t38TCP = 0,
  t38UDP = 1,
  t38RTP = 2
}T38LinkMode_t;

typedef struct
{
  char T38FaxRateManagement[15];  /* Max string "transferredTCF" */
  char T38FaxVersion[2];          /* String of 1 digit max. i.e "0" */
  char T38MaxBitRate[6];          /* String of 5 digits max. i.e. "14400" */
  char T38FaxFillBitRemoval[2];   /* Not supported by T38 alg. Holds "0" */
  char T38FaxTranscodingMMR[2];   /* Not supported by T38 alg. Holds "0" */
  char T38FaxTranscodingJBIG[2];  /* Not supported by T38 alg. Holds "0" */
  char T38FaxMaxBuffer[4];        /* String of 3 digits max. i.e "400" */
  char T38FaxMaxDatagram[4];      /* String of 3 digits max. i.e "280" */
  char T38FaxUdpEC[17];              /* Max string "t38UDPRedundancy" */
  unsigned short          T38VendorInfo[3];
  unsigned short          T38FaxUdpEnabled;
  T38LinkMode_t           T38LinkMode;

}ata_t38_sdp_params_type;

int sc1445x_fax_params(void *params, int size);
int sc1445x_fax_switch_mode(sicall_t* pCall);

int sc1445x_fax_open_connection(sicall_t* pCall) ;
  
#endif //SI_UA_FAX_H
