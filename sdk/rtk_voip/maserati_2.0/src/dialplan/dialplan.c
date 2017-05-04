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
	
		if(( ch == '+' )||( ch == '|' )) {
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
			case '.':
				*pszReDest ++ = '*';
				break;

			case '*':
			case '#':
				*pszReDest ++ = '\\';
				*pszReDest ++ = ch;
				break;
				
			case '[':
				*pszReDest ++ = ch;
					/* find next char ] */
					pszSrc ++;
					while(( ch = *pszSrc ) != ']' ){
						if(( ch = *pszSrc ) == '\x0' )
						{
							g_message("dialplan format is not valid![%s] %d \n",__FUNCTION__,__LINE__);
							return 0;
						}
					
						*pszReDest ++ = ch;
						pszSrc ++;
					}
					
					*pszReDest ++ = ch;
					break;
			case '<':
					/* find next char ] */
					pszSrc ++;
					while(( ch = *pszSrc ) != ':' ){
						if( ( ch >= '0' ) && ( ch <= '9' ) ) {
							*pszReDest ++ = ch;
							pszSrc ++;
						}
						else
						{
							g_message("dialplan format is not valid![%s] %d \n",__FUNCTION__,__LINE__);
							return 0;
						}
					}
					while(( ch = *pszSrc ) != '>' ){
						if(( ch = *pszSrc ) == '\x0' )
						{
							g_message("dialplan format is not valid![%s] %d \n",__FUNCTION__,__LINE__);
							return 0;
						}
						pszSrc ++;
					}					
					break;
			case '|':
			case '+':
				if( !bPrefixRE )
					*pszReDest ++ = '$';
				*pszReDest ++ = '|';
				*pszReDest ++ = '^';
				break;
				
			case '(':
			case ')':
			case 'T':
			case 'S':
			case 'L':
			case 's':
			case 't':
			case 'l':
			case ']':	
				//not thing

				break;
			default:
				g_message("dialplan format [ ch =%c ] is not valid![%s] %d \n", ch, __FUNCTION__,__LINE__);
				return 0;
				break;
	  		} // end of switch()
	  	}
	  	pszSrc ++;
	} // end of while loop
	
	if( !bPrefixRE )
		*pszReDest ++ = '$';
	*pszReDest ++ = '\x0';
	
 	//g_message( "The new dial plan: %s\n", pszReDestOrg );

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
							 int *idx_end ,unsigned char *target)
{
	int z, i;	
	regmatch_t pm[ 10 ];
	const size_t nmatch = 10;
	replace_list_t *replace_en;
	replace_en=me->replace;
	/* check use or not? */
	if( me ->bUseReplaceRule == 0 ) {
		return 0;
	}

	/* Use RE to match the input data */
	#if 0/*support common dial plan format, Anson, 20150112*/
	z = regexec( &( me ->regReplaceRule ), pszDialString, 
				 nmatch, pm, 0);

	if( z == 0 ) {
		*idx_start = pm[ 0 ].rm_so;
		*idx_end = pm[ 0 ].rm_eo;
		return 1;
	}
	#else
	g_message("me ->bUseReplaceRule=%d\n",me ->bUseReplaceRule);

	for(i=1;i<=me ->bUseReplaceRule;i++)
	{
		if(replace_en!=NULL)
		{
			z = regexec( &( replace_en->regReplace), pszDialString, 
						 nmatch, pm, 0);
			if( z == 0 )
			{
				//*idx_start = pm[ 0 ].rm_so;				
				strcpy( target, replace_en->p_replace_target );
				
				*idx_start=replace_en->p_replace_start;
				*idx_end =replace_en->p_replace_soure_length;
				g_message("MatchReplaceRuleProcess match i=%d target =%s\n", i ,target);
				g_message("*idx_start=%d  *idx_end=%d\n",*idx_start, *idx_end);
				return 1;
			}
			replace_en=replace_en->next;
		}else
		{
			g_message("replace entry is NULL\n");
			return 0;	
		}
	}
	#endif


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
		/* Check for partial Match*/
        if ( me->bUsePMatchDialPlan ){
			z = regexec( &( me ->regPMatchDialPlan), pdpMatchParam ->pDialData, 
					 nmatch, pm, 0);
	        if (z == 0) {
	            g_message("Partial Match! Still Wait for Input\n");
	            return 2;
	        }
        }
    	
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
			return -1;
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
	unsigned char szRegularExpression[ REG_MAX_LENGTH ];
	int z;

	memset(szRegularExpression, 0 , sizeof(szRegularExpression));
	
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

/* For process partial match */
static int AllocateRegularExpression2( regex_t *pRegex1, regex_t *pRegex2,
									  const unsigned char *pszDialPlan,
									  unsigned int *pMaxElements,
									  int bPrefixRE )
{
	if( *pszDialPlan == '\0' )
		return 0;
	else {
		unsigned int len=strlen(pszDialPlan);
		char szRegEx1[ MAX_DIALPLAN_LENGTH], szRegEx2[ MAX_DIALPLAN_LENGTH ], tmpstr[ REG_MAX_LENGTH ];
		char *idx, *token, *index1=szRegEx1, *index2=szRegEx2;
		int z = 0;
        	memset(tmpstr, 0 , sizeof(tmpstr));
		/* get its max elements */
		if( pMaxElements )
			*pMaxElements = dialplan_counter( pszDialPlan );

		/* if string is too long*/
		if(len > MAX_DIALPLAN_LENGTH)
		{
			g_message("len > MAX_DIALPLAN_LENGTH [%s] %d \n",__FUNCTION__,__LINE__);
			return 0;
		}
        
		strncpy(tmpstr, pszDialPlan, len-1);
		tmpstr[len-1]='\0';
        
		if (*tmpstr=='('){
			if(*(tmpstr+len-1)==')')
				*(tmpstr+len-1)='\0';
			token=tmpstr+1;
		} else {
			token=tmpstr;
		}
		g_message("dialplan length %d now\n", len);

		memset(szRegEx1, 0, sizeof(szRegEx1));
		memset(szRegEx2, 0, sizeof(szRegEx2));

		/* add '(' for each string */
		*index1 ++= '('; *index2 ++= '(';

		token = strtok(token, "|");
		while( token!=NULL ){
			//printf("processing %s \n", token);
			len = strlen(token);
			idx = strchr(token, '.');
			/*	Partial Match Cases
			'.' exists in the last position, ex: ab. ,ax.*/
			if(idx && ((*(token+len-1) == '.')||((*(token+len-1) == 'T')&&(*(token+len-2) == '.'))))
			{
				memcpy(index2, token, len);
				index2 += len;
				*index2 ++= '|';
				//printf("Add %s szRegEx2 %s\n", token, szRegEx2);
			} else {
				memcpy(index1, token, len);
				index1 += len;
				*index1 ++= '|';
				//printf("Add %s szRegEx1 %s\n", token, szRegEx1);

			}
			token = strtok(NULL,"|");
		}

		/* append ')' for each string */
		memcpy(index1-1, ")\0", 2); 	memcpy(index2-1, ")\0", 2);

//g_message("dialplan is separated as %s and %s\n", szRegEx1, szRegEx2);

		/* convert to regular expression */
		if( !dialplan_parser( tmpstr, szRegEx1, bPrefixRE ) ) {
			g_message("dialplan is not valid!\n");
			return 0;
		}

		if (0 != regcomp( pRegex1, tmpstr, REG_EXTENDED )) {
			regerror(z, pRegex1, tmpstr, sizeof(tmpstr));
			fprintf(stderr, "%s: pattern '%s' \n", tmpstr, szRegEx1);
			return 0;
		}

		if( !dialplan_parser( tmpstr, szRegEx2, bPrefixRE ) ) {
			g_message("dialplan is not valid!\n");
			return 0;
		}

		if (0 != regcomp( pRegex2, tmpstr, REG_EXTENDED )) {
			regerror(z, pRegex2, tmpstr, sizeof(tmpstr));
			fprintf(stderr, "%s: pattern '%s' \n", tmpstr, szRegEx2);
			return 0;
		} 
	}
	
	return 1;
}

/*support common dial plan format, Anson, 20150112*/
replace_list_t *
RegularEx(char *token)
{
	if( *token == '\0' )
		return NULL;

	unsigned char szRegEx[ MAX_DIALPLAN_LENGTH];	
	char regstr[ REG_MAX_LENGTH ];
	char  *index1, *index2;
	int z = 0;
	replace_list_t *replace_sz=NULL;

	
	replace_sz=(replace_list_t *)malloc(sizeof(replace_list_t));
	if( replace_sz == NULL ) {
		g_message("No enough memory for Dial Plan![%s] %d \n",__FUNCTION__,__LINE__);
		goto failed;
	}
	memset(replace_sz, 0, sizeof(replace_list_t));
	memset(szRegEx, 0, sizeof(szRegEx));
	memset(regstr, 0, sizeof(regstr));
	
	replace_sz->p_replace_start=0;
	replace_sz->p_replace_soure_length=0;
	index1=replace_sz->p_replace_target;
	index2=szRegEx;

	if(strchr(token, '<')!=NULL)
	{

		if(strchr(token, ':')==NULL||strchr(token, '>')==NULL)
		{
			g_message("dialplan format is not valid![%s] %d \n",__FUNCTION__,__LINE__);
			goto failed;
		}
		
		//parse prefix  before replace '<' for replace_rule
		while(*token!='<')
		{

			replace_sz->p_replace_start++;
			*index2++=*token++;
		}

		//parse string  between '<'  to ':' for replace_rule
		token++;
		while(*token!=':')
		{	
			replace_sz->p_replace_soure_length++;
			*index2++=*token++;
		}

		//parse string  between ':'  to '>'  for replace_target 
		token++;
		while(*token!='>')
		{
			*index1++=*token++;
		}
		*index1='\0';


		//parse string  between '>'  to 'end'  for replace_rule
		token++;
		while(*token!='\0')
		{	
			*index2++=*token++;
		}
		*index2='\0';

		if(strlen(replace_sz->p_replace_target)>MAX_REPLACE_RULE_TARGET)
		{
			g_message("dialplan format is not valid![%s] %d \n",__FUNCTION__,__LINE__);
			goto failed;
		}
			
	}
	else
	{
		strncpy(szRegEx, token, strlen(token));			
	}
	
	g_message("p_replace_start=%d\n", replace_sz->p_replace_start);
	g_message("p_replace_target=%s\n", replace_sz->p_replace_target);
	g_message("p_replace_rule=%s\n", szRegEx);
	g_message("p_replace_soure_length=%d\n", replace_sz->p_replace_soure_length);
	
	if( !dialplan_parser( regstr, szRegEx, 0 ) ) {
		printf("dialplan is not valid!\n");
		goto failed;
	}
	
	if (0 != regcomp( &(replace_sz->regReplace), regstr, REG_EXTENDED )) {
		regerror(z, &(replace_sz->regReplace), regstr, sizeof(regstr));
		fprintf(stderr, "%s: pattern '%s' \n", regstr, &(replace_sz->regReplace));
		goto failed;
	}
	
	return replace_sz;
	
failed:
	if( replace_sz != NULL ) {
		free(replace_sz);
		replace_sz=NULL;
	}
	return NULL;
}
/*support common dial plan format, Anson, 20150112*/
/* For process replace < : > match */
replace_list_t *
ParseReplaceList(const unsigned char *pszDialPlan, unsigned int *number_of_replace)
{	
	if( *pszDialPlan == '\0' )
	{
		*number_of_replace=0;
		return NULL;
	}

	int number=0;
	replace_list_t *replace_head=NULL, *replace_tmp=NULL;
	char tmpstr[MAX_DIALPLAN_LENGTH];
	char* token=NULL;
	unsigned int len=strlen(pszDialPlan);
	/* if string is too long*/
	if(len > MAX_DIALPLAN_LENGTH)
	{
		g_message("len > MAX_DIALPLAN_LENGTH [%s] %d \n",__FUNCTION__,__LINE__);
		*number_of_replace=0;
		return NULL;
	}
    
	strncpy(tmpstr, pszDialPlan, len);
	tmpstr[len]='\0';
        
        if (*tmpstr=='('){
            if(*(tmpstr+len-1)==')')
                *(tmpstr+len-1)='\0';
			token=tmpstr+1;
        } else {
			token=tmpstr;
        }
		
	// g_message("replace parse dialplan=%s length=%d\n", tmpstr, len);
        
        token = strtok(token, "|");
        while( token!=NULL ){
			
		g_message("processing %s \n", token);

		if(number==0)
		{
			replace_head=replace_tmp=RegularEx(token);
		}
		else
		{
			replace_tmp->next=RegularEx(token);
			replace_tmp =replace_tmp->next;
		}

		if(replace_tmp!=NULL)
		{
			replace_tmp->next=NULL;
			number++;
			g_message("parse new replace entry success:number_of_replace=%d\n\n", number);
		}
		else
		{
			if(replace_head!=NULL)
				freeReplace(replace_head);
			*number_of_replace=0;
			
			g_message("parse new replace entry fail [%s] %d \n",__FUNCTION__,__LINE__);
			return NULL;
		}
		token = strtok(NULL,"|");
        }
	*number_of_replace=	number;
	return replace_head;

}
void
freeReplace(replace_list_t* head)
{
	replace_list_t *ptrRule, *nextRule;

	ptrRule = head;
	while( ptrRule != NULL ) {
		regfree( &(ptrRule->regReplace)) ;
		nextRule = ptrRule->next;
		free(ptrRule);
		ptrRule = nextRule;
	}
	head = NULL;
	return;
}
static inline void FreeDialPlan( dialplan_vars_t *me )
{
#define M_DO_REGEXFREE( var, reg )	\
	if( ( var ) ) {					\
		regfree( ( reg ) );			\
		( var ) = 0;				\
	} {}


	#if 1/*support common dial plan format, Anson, 20150112*/
	if (me ->bUseReplaceRule!=0)
	{
		if (me->replace!=NULL)
			freeReplace(me->replace);
		me->replace=NULL;
		me ->bUseReplaceRule=0;
	}
	#else
	M_DO_REGEXFREE( me ->bUseReplaceRule, &( me ->regReplaceRule ) );
	#endif
	M_DO_REGEXFREE( me ->bUseDialplan, &( me ->regDialPlan ) )
	M_DO_REGEXFREE( me ->bUsePrefixUnset, &( me ->regPrefixUnsetPlan ) )
	M_DO_REGEXFREE( me ->bUsePMatchDialPlan, &( me ->regPMatchDialPlan) )

#undef M_DO_REGFREE
}

void InitializeDialPlan( dialplan_vars_t *me, const dpInitParam_t *pdpInitParam)
{

	/* keep target of replace rule */
	//me ->p_replace_rule_target = pdpInitParam ->p_replace_rule_target;
	
	/* free previous allocate regular expressions */
	FreeDialPlan( me );

	/* allocate regular expressions */
#if 0/*support common dial plan format, Anson, 20150112*/
	if( pdpInitParam ->replace_rule_option ) {
		me ->bUseReplaceRule =
			AllocateRegularExpression( &( me ->regReplaceRule ),//index
									   pdpInitParam ->p_replace_rule_source,//rule
									   NULL,//rule max length 
									   1 /* prefix RE */ );
	} else {
		me ->bUseReplaceRule = 0;


		if(!strchr(pdpInitParam ->p_dialplan, '.')){
	    		me ->bUsePMatchDialPlan = 0;
			me ->bUseDialplan =
				AllocateRegularExpression( &( me ->regDialPlan ),
									   pdpInitParam ->p_dialplan,
									   &me ->max_elements_dialplan,
									   0 /* match whole string */ );
	    	} else {
			me ->bUseDialplan =
				AllocateRegularExpression2( &( me ->regDialPlan ),
									   &( me ->regPMatchDialPlan),
									   pdpInitParam ->p_dialplan,
									   &me ->max_elements_dialplan,
									   0 /* match whole string */ );
			if(me ->bUseDialplan!=0)
				me ->bUsePMatchDialPlan = 1;
	    	}

	}	
#else
	if( pdpInitParam ->dial_plan_enable ) {
		if(!strchr(pdpInitParam ->p_dialplan, '.')){
	    		me ->bUsePMatchDialPlan = 0;
			me ->bUseDialplan =
				AllocateRegularExpression( &( me ->regDialPlan ),
									   pdpInitParam ->p_dialplan,
									   &me ->max_elements_dialplan,
									   0 /* match whole string */ );
	    	} else {
			me ->bUseDialplan =
				AllocateRegularExpression2( &( me ->regDialPlan ),
									   &( me ->regPMatchDialPlan),
									   pdpInitParam ->p_dialplan,
									   &me ->max_elements_dialplan,
									   0 /* match whole string */ );
			if(me ->bUseDialplan!=0)
				me ->bUsePMatchDialPlan = 1;
	    	}
			
		if (me ->bUseDialplan!=0)
		{
			me ->replace =
				ParseReplaceList( pdpInitParam ->p_dialplan, &(me->bUseReplaceRule));

			/*parse replace error, free all dial plan*/
			if(me->bUseReplaceRule==0)
			{
				FreeDialPlan( me );
			}

		}
	}
#endif

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
    	g_message( "Partial Match Dial Plan: %d\n", me ->bUsePMatchDialPlan);
	g_message( "Unset prefix: %d\n", me ->bUsePrefixUnset );
	g_message( "------------------------\n" );

}

void UninitializeDialPlan( dialplan_vars_t *me )
{
	FreeDialPlan( me );
}

