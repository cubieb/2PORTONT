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
 * File:		si_dial_plan_api.h
 * Purpose:		
 * Created:	19/10/2009
 * By:			Efilippatos
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_DIAL_PLAN_API_H
#define SI_DIAL_PLAN_API_H

/*========================== Include files ==================================*/


/*========================== Local macro definitions & typedefs =============*/
/* Dial Paln definitions
*/
#define DP_MAX_LIST_LEN    200
#define DP_MAX_STRING_LEN  50
#define DP_MAX_STRINGS     10
#define DP_INACTIVE        0
#define DP_ACTIVE          1
#define DP_NOMATCH         0
#define DP_MATCH           1
#define DP_SET_TIMER       2
#undef  DP_DEBUG

typedef struct
{
  unsigned char status;
  char  str[DP_MAX_STRING_LEN + 1];
} dial_plan_t;

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/
int dp_check( const char *dial_str );
int dp_init(void);

#endif /* SI_DIAL_PLAN_API_H */
