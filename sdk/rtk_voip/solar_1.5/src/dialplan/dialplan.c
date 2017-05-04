#include <stdlib.h>
#include <regex.h>
#include "dialplan.h"
#include "linphonecore.h"

int dialplan_counter(const char *str)
{
	int max_rule_length = 0;
	int current_rule_length = 0;
	unsigned char ch;
	
	while( ( ch = ( *str ++ ) ) != '\x0' ) {
	
		if( ch == '+' ) {
			if( max_rule_length < current_rule_length )
				max_rule_length = current_rule_length;
				
			current_rule_length = 0;
		} else
			current_rule_length ++;
	}
	
	if( max_rule_length < current_rule_length )
		max_rule_length = current_rule_length;

	g_message( "The max. of rule length: %d.\n", max_rule_length );
	
	return max_rule_length;
}

/* 
  * Change the User input format to RE. format. 
  * TODO: Check syntax of user input.
  *
*/
static int dialplan_parser( unsigned char *pszReDest, 
							const unsigned char *pszSrc,
							int bPrefixRE )
{
	unsigned char ch;
	const unsigned char *pszReDestOrg = pszReDest;

	*pszReDest ++ = '^';
	
	while( ( ch = *pszSrc ) != '\x0' ) {
		/* for 0~9 */
		if( ( ch >= '0' ) && ( ch <= '9' ) ) {
			*pszReDest ++ = ch;
		}
		else {
			switch( ch ) {
			case 'X':
			case 'x':
				memcpy( pszReDest, "[0-9]", 5);
				pszReDest += 5;
				break;
				
			case 'N':
			case 'n':
				memcpy( pszReDest, "[1-9]", 5);
				pszReDest += 5;
				break;

			case 'M':
			case 'm':
				memcpy( pszReDest, "[2-9]", 5);
				pszReDest += 5;
				break;

			case '*':
			case '#':
				*pszReDest ++ = '\\';
				*pszReDest ++ = ch;
				break;
				
			case '+':
				if( !bPrefixRE )
					*pszReDest ++ = '$';
				*pszReDest ++ = '|';
				*pszReDest ++ = '^';
				break;
				
			default:
				g_message("dialplan parser error!\n");
				return 0;
				break;
	  		} // end of switch()
	  	}
	  	pszSrc ++;
	} // end of while loop
	
	if( !bPrefixRE )
		*pszReDest ++ = '$';
	*pszReDest ++ = '\x0';
	
 	g_message( "The new dial plan: %s", pszReDestOrg );
  
	return 1;
}

int IsNumericalString( const unsigned char *pszCheckNumericalString )
{
	unsigned char ch;
	
	while( ( ch = *pszCheckNumericalString ++ ) ) {
		if( ch < '0' || ch > '9' )
			return 0;	/* not a numerical string */
	}
	
	return 1;
}

int MatchPrefixUnsetProcess( dialplan_vars_t *me, 
							 const unsigned char *pszDialString  )
{
	int z;	
	regmatch_t pm[ 10 ];
	const size_t nmatch = 10;
	
	/* check use or not? */
	if( me ->bUsePrefixUnset == 0 ) {
		return 0;
	}

	/* Use RE to match the input data */
	z = regexec( &( me ->regPrefixUnsetPlan ), pszDialString, 
				 nmatch, pm, 0);

	if( z == 0 )
		return 1;		/* NOT do auto prefix */

	return 0;
}

int MatchReplaceRuleProcess( dialplan_vars_t *me, 
							 const unsigned char *pszDialString,
							 int *idx_start,
							 int *idx_end )
{
	int z;	
	regmatch_t pm[ 10 ];
	const size_t nmatch = 10;

	/* check use or not? */
	if( me ->bUseReplaceRule == 0 ) {
		return 0;
	}

	/* Use RE to match the input data */
	z = regexec( &( me ->regReplaceRule ), pszDialString, 
				 nmatch, pm, 0);

	if( z == 0 ) {
		*idx_start = pm[ 0 ].rm_so;
		*idx_end = pm[ 0 ].rm_eo;
		return 1;
	}

	return 0;
}

/*
 * If user dial match the dial plan, then return 1, 
 * else if user input too much return 2,
 * else return 0.
 */
int MatchDialProcess( dialplan_vars_t *me, const dpMatchParam_t *pdpMatchParam )
{
	int z;	
	regmatch_t pm[ 10 ];
	const size_t nmatch = 10;
	
	/* check use or not? */
	if( me ->bUseDialplan == 0 ) {
		g_message("Not use Dial Plan!\n");
		return 0;
	}

	/* Use RE to match the input data */
	z = regexec( &( me ->regDialPlan ), pdpMatchParam ->pDialData, 
				 nmatch, pm, 0);

	if (z == 0) {
		g_message(" Match!\n");
		/* Action: dial out */
		g_message("Do Action: dial out.\n");
		return 1;
	}
	else if (z != 0) {
		g_message(" Not Match...\n");
		//regerror(z, &reg, ebuf, sizeof(ebuf));
		//fprintf(stderr, "%s: regcom('%s')\n", ebuf, lbuf);
		g_message( "dial_data_index: %d, max_elements_dialplan: %d.\n", 
				pdpMatchParam ->nDialDataLen, 
				me ->max_elements_dialplan);
							
		if (pdpMatchParam ->nDialDataLen >= me ->max_elements_dialplan) 
		{
			/* User input too much */
			/* Action: busy tone */
			g_message("Do Action: busy tone...\n");
			return 2;
		}
	}

	return 0;
}

static int AllocateRegularExpression( regex_t *pRegex,
									  const unsigned char *pszDialPlan,
									  int *pMaxElements,
									  int bPrefixRE )
{
#define ebuf	szRegularExpression
	unsigned char szRegularExpression[ 512 ];
	int z;

	if( *pszDialPlan == '\0' )
		return 0;
	else {
		/* get its max elements */
		if( pMaxElements )
			*pMaxElements = dialplan_counter( pszDialPlan );

		/* convert to regular expression */
		if( !dialplan_parser( szRegularExpression, pszDialPlan, bPrefixRE ) ) {
			g_message("dialplan is not valid!\n");
			return 0;
		}
				
		z = regcomp( pRegex, szRegularExpression, REG_EXTENDED );
		
		if (z != 0) {
			regerror(z, pRegex, ebuf, sizeof(ebuf));
			fprintf(stderr, "%s: pattern '%s' \n", ebuf, szRegularExpression);
			return 0;
		}
	}
	
	return 1;
#undef ebuf
}

static inline void FreeDialPlan( dialplan_vars_t *me )
{
#define M_DO_REGEXFREE( var, reg )	\
	if( ( var ) ) {					\
		regfree( ( reg ) );			\
		( var ) = 0;				\
	} {}

	M_DO_REGEXFREE( me ->bUseReplaceRule, &( me ->regReplaceRule ) );
	M_DO_REGEXFREE( me ->bUseDialplan, &( me ->regDialPlan ) )
	M_DO_REGEXFREE( me ->bUsePrefixUnset, &( me ->regPrefixUnsetPlan ) )

#undef M_DO_REGFREE
}

void InitializeDialPlan( dialplan_vars_t *me, const dpInitParam_t *pdpInitParam)
{
	
	/* keep target of replace rule */
	me ->p_replace_rule_target = pdpInitParam ->p_replace_rule_target;
	
	/* free previous allocate regular expressions */
	FreeDialPlan( me );

	/* allocate regular expressions */
	if( pdpInitParam ->replace_rule_option ) {
		me ->bUseReplaceRule =
			AllocateRegularExpression( &( me ->regReplaceRule ),
									   pdpInitParam ->p_replace_rule_source,
									   NULL,
									   1 /* prefix RE */ );
	} else 
		me ->bUseReplaceRule = 0;
		
	me ->bUseDialplan =
			AllocateRegularExpression( &( me ->regDialPlan ),
									   pdpInitParam ->p_dialplan,
									   &me ->max_elements_dialplan,
									   0 /* match whole string */ );
		
	if( pdpInitParam ->p_auto_prefix[ 0 ] != '\x0' ) {
		me ->bUsePrefixUnset =
			AllocateRegularExpression( &( me ->regPrefixUnsetPlan ),
									   pdpInitParam ->p_prefix_unset_plan,
									   NULL,
									   1 /* prefix RE */ );
	} else
		me ->bUsePrefixUnset = 0;
	
	/* show result */
	g_message( "=== Dial Plan Initialization Summary (%p) ===\n", me );
	g_message( "Replace rule: %d\n", me ->bUseReplaceRule );
	g_message( "Dial plan: %d\n", me ->bUseDialplan );
	g_message( "Unset prefix: %d\n", me ->bUsePrefixUnset );
	g_message( "------------------------\n" );
	
}

void UninitializeDialPlan( dialplan_vars_t *me )
{
	FreeDialPlan( me );
}

