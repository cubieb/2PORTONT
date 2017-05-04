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
 * File:		si_dial_plan_api.c
 * Purpose:		
 * Created:	19/10/2009
 * By:			EFilippatos
 * Country:	Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>

#include <si_print_api.h>

#include "si_configfiles_api.h"
#include "si_config_file.h"
#include "si_dial_plan_api.h"

/*========================== Local macro definitions ========================*/


/*========================== Global definitions =============================*/


/*========================== Local function prototypes ======================*/
static void dp_init_tables( void );
static void dp_read_list( char *dp_list_ptr );
static int dp_match( const char *dp_str, const char *dn_str );

/*========================== Local data definitions =========================*/
/* Holds the Dial Plan strings from the Dial Plan list specified by the user */ 
dial_plan_t dp[DP_MAX_STRINGS];
char dp_enable = 0;

/*========================== Function definitions ===========================*/
int dp_init(void)
{
	int ret = 0;
	DialPlan_settings pDialPlan_settings;

	dp_init_tables();
	
 	ret = sc_fileapi_LoadDialPlanSettings(&pDialPlan_settings);
	if (ret == 0){
	 dp_read_list(pDialPlan_settings.dial_plan);
	 dp_enable = pDialPlan_settings.dp_enable;
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Dial Plan related functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Function:  dp_init_tables()
 *
 * Description: Initialises the dial plan structure after a 
 * number is sent to call control.
*/
static void dp_init_tables( void )
{
  unsigned char idx;

  for( idx = 0; idx <= DP_MAX_STRINGS; idx++ )
  {
    if( dp[idx].status != DP_INACTIVE )
      dp[idx].status = DP_INACTIVE;
  }
}

/**
 * Function:  dp_read_list()
 *
 * Description: Reads the dial pan list entered by the user via BOA and 
 * initialises the dial plan array with the dial plan strings.
*/

static void dp_read_list( char *dp_list_ptr )
{
  unsigned char idx = 0;
  char token_sep[] = "|";
  char *dp_str;
  int  len;
	
	if (!(dp_enable)) return ;

  /* init dial plan array */
  memset( dp, 0, sizeof(dial_plan_t) );
  
  if( !strlen(dp_list_ptr) )
  {
    si_print(PRINT_LEVEL_INFO,  ("Empty Dial Plan list !!\n" ) );
    return;
  }
  /* Get first dial plan string separated by token '|'  */
  dp_str = strtok( dp_list_ptr, token_sep );
  
  do{
    /* Ignore opening and closing brackets from a dial paln string */
    if( dp_str[0] == '(' )
      dp_str++;
    
    len = strlen(dp_str);

    if( dp_str[len-1] == ')' )
      len--;

    /* Store dial paln string in dial plan array and init status */
    if( len <= DP_MAX_STRING_LEN && len )
    {
      strncpy( dp[idx].str, dp_str, len );
      dp[idx].status  = DP_ACTIVE;
      idx++;
    }
  }while( (dp_str = strtok( NULL, token_sep )) != NULL );

#if DP_DEBUG   
  for( idx = 0; idx <= DP_MAX_STRINGS; idx++ )
  {
    if( dp[idx].status  == DP_ACTIVE )
      si_print(PRINT_LEVEL_DEBUG,  "Dial Plan [%d]: %s\n", idx, dp[idx].str );
  }
#endif
}

/**
 * Function:  ata_dp_match()
 *
 * Description: Checks for a match between the dial plan string 
 * and the user dialed number string.
*/
static int dp_match( const char *dp_str, const char *dn_str )
{
  unsigned char c, dn, dp;
  int match = DP_NOMATCH;
  int dp_len, dn_len;

  dp_len = strlen( dp_str );
  dn_len = strlen( dn_len );

  while( (dp = *dp_str++) != '\0' )
  { 
    if( (dn = *dn_str++) == '\0')
    {
      if( dp == 't' || dp == 'T' )
      {
        match = DP_SET_TIMER;
      }
      else
      { 
        match = DP_NOMATCH;
      }

      si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
      return( match);
    }

    if( (dp == 'x') || (dp == 'X') )
    {
      match = DP_MATCH;
    }
    else if( dp == '.' )
    {
      dp = *dp_str;
      if( dp == 't' || dp == 'T' )
      {
        match = DP_SET_TIMER;
      }
      else
      { 
        match = DP_MATCH;
      }

      si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
      return( match);
    }
    else if( dp == '[' )
    {
      char *temp;

      match = DP_NOMATCH;
      while( (dp = *dp_str++) != ']' )
      {
        if( dp == '\0' )
        {
          si_print(PRINT_LEVEL_DEBUG,  "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
          return match;
        }

        if( dp == '-' )
        {
          temp = dp_str - 2;
          if( dn >= *temp && dn <= *(temp+2) )
            dp = dn;
          else
            dp = dp_str++;
        }
        
        if( dp == dn )
        {
          temp = strrchr( dp_str, ']');
          
          if( temp != NULL )
          {
            dp_str = ++temp;
            si_print(PRINT_LEVEL_DEBUG, "Return str: %s\n", dp_str);
          }
          else
          {
            si_print(PRINT_LEVEL_DEBUG, " Did not Find ]\n");
            return DP_NOMATCH;
          }

          match = DP_MATCH;
          si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
          break;
        }
      }
      if( match == DP_NOMATCH )
        return DP_NOMATCH;
    }
    else if( dp == 't' || dp == 'T' )
    {
      if( match == DP_MATCH )
      {
        match = DP_SET_TIMER;
      }

			si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
      return( match );
    }
    else
    {
      /* check for regular digit match */
      if( dn == dp )
      {
        match = DP_MATCH;
        si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
      }
      else
      {
        match = DP_NOMATCH;
        si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );
        return( match );
      }
    }
  }

  if( dp == 't' || dp == 'T' )
  {
    if( match == DP_MATCH )
    {
      match = DP_SET_TIMER;
    }    
  }

  si_print(PRINT_LEVEL_DEBUG, "[%s] line: %d. Match Found: %d\n", __FUNCTION__, __LINE__, match  );

  return( match );
}


int dp_check( const char *dial_str )
{
  unsigned char idx;
  int match = 0;
  for( idx = 0; idx <= DP_MAX_STRINGS; idx++ )
  {
    if( dp[idx].status == DP_ACTIVE )
    {
      if( DP_NOMATCH != (match = dp_match( dp[idx].str, dial_str )) )
      {
        si_print(PRINT_LEVEL_INFO, "Ent [%d] Match Found: %d Dial_str: %s \n", idx, match, dial_str );
        return( match );
      }
    }
  }
}

	