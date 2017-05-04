#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "xml_config.h"
#include "xml_config_gramma.h"

//#define ONE_DIR
typedef enum {
	ST_CONFIG = 0,
	ST_CONFIG_END,
	ST_ENTRY,
	ST_DIR,
	ST_DIR_END,
	_ST_CONFIG,
	_ST_CONFIG_END,
	_ST_DIR,
	_ST_DIR_END,
	ST_FILE_END,
	ST_COMMENT,
	ST_INVALID,
	ST_UNUSED,
	ST_UNKOWN
} stoken_t;

//int depth=0;
char *g_config=NULL;
char current_filename[MAX_NAME_STRING];
unsigned int linenum=1;
//static char current_token_string[MAX_TOKEN_STRING];
static char *_cindex=NULL;
//static stoken_t _saved_token=ST_INVALID;

static char modal_name[MAX_NAME_STRING]="RTK";
static char dir_name[MAX_NAME_STRING];
static char ent_name[MAX_NAME_STRING];
static char ent_value[MAX_VALUE_STRING];
static char scomment[MAX_VALUE_STRING];
//static unsigned int dir_index=0;

#define DIR_STACK_MAX 6
dir_t *dir_stack[DIR_STACK_MAX]={NULL};
int dir_sp=0;
int dir_ig=0; // ignore dir depth
#define push_dir(d) (dir_stack[dir_sp++]=d)
#define pop_dir() if(dir_sp>=1) {--dir_sp;}	// pop dir_stack

extern dir_t *root_dir_p;
dir_t *parsing_dir=NULL;
entry_t *parsing_ent=NULL;

static cfg_err_t g_process(void);
#if ONE_DIR
static int one_dir_ff(stoken_t st);
static cfg_err_t g_one_dir(void);
static cfg_err_t g_list(void);
#endif


/*
tokenName 		tokenContent
----------------------------------------------
ST_CONFIG		<Config Name="">
ST_CONFIG_END	</Config>
ST_ENTRY		<Value Name="" Value=""/>
ST_DIR			<Dir Name="">
ST_DIR_END		</Dir>
_ST_CONFIG		<Config_Information_File_8671>
_ST_CONFIG_END	</Config_Information_File_8671>
_ST_DIR			<chain chainName="">
_ST_DIR_END		</chain>
ST_COMMENT		<!--""-->
ST_FILE_END		<end-of-file>
*/

static struct {
    stoken_t st;
    const char *stString;
    unsigned int length;
} stoken_list[] = {
	{ST_CONFIG, "<Config Name=\"\">", 13},
	{ST_CONFIG_END, "</Config>", 9},
	{ST_ENTRY, "<Value Name="" Value=\"\"/>", 12},
	{ST_DIR, "<Dir Name=\"\">", 10},
	{ST_DIR_END, "</Dir>", 6},
	{_ST_CONFIG, "<Config_Information_File_8671>", 30},
	{_ST_CONFIG_END, "</Config_Information_File_8671>", 31},
	{_ST_DIR, "<chain chainName=\"\">", 17},
	{_ST_DIR_END, "</chain>", 8},
	{ST_COMMENT, "<!--""-->", 4},
	{ST_INVALID, 0, 0}
};

// init stoken parameter
void
stoken_init(void) {
	XML_DBG ("!! stoken init....\n");
	int i = 0;
	_cindex = g_config;
	linenum = 1;
	parsing_dir = NULL;
	parsing_ent = NULL;
	dir_sp = 0;
	dir_ig = 0;
	for (i = 0; i < DIR_STACK_MAX; i++)
	    dir_stack[i] = NULL;
}

// Nils: reset root_dir_p->firstChild->sibling index number
void
Index_cnt_init(void) {
	parsing_dir = root_dir_p->firstChild;
	while (parsing_dir != NULL) {
		if (parsing_dir->index != NULL) {
			if (0 != *(parsing_dir->index)) {
				XML_DBG ("%s(): [%s] index=[%d]\n",__FUNCTION__, parsing_dir->matchname, *(parsing_dir->index)); 
				*(parsing_dir->index) = 0;
			}
		}
		parsing_dir = parsing_dir->sibling;
	}
}

static inline int
token_match_n_move (char **p, const char *s) {
	int len = strlen (s);
	if (strncasecmp ((*p), s, len) == 0) {
		XML_DBG ("\"%s\" len is %d\n", s, len);
		(*p) += len;
		return 0;
	} else {
		XML_DBG ("unmatched string for %s\n", s);
		return -1;
	}
}

static inline void
space_skip (char **p) {
	while(**p == ' ' || **p == '\n')
		(*p)++;
}

static stoken_t
_get_stoken (void) {
	int status = ST_INVALID;
	char *ptr = _cindex;
	char *q = ptr, *r = ptr;

	// skip typesetting characters, space, tab, newline
	while (1) {
		if ((*ptr == '\n'))
			{++linenum;}
		else if (*ptr == '\0') 
			{return ST_FILE_END;}
		else if (*ptr == '<') {
			q = ptr + 1;
			break;
		}
		++ptr;
	}

	// check '>'
	while ((*q != '>') && (*q != '\0') && (*q != '<')) {q++;}
	if ((*q == '\0') || (*q == '<')) return ST_UNKOWN;

	if (token_match_n_move (&ptr, "<Config_Information_File_8671>") == 0) {
		XML_DBG ("<Config_Information_File_8671>\n");
		status = _ST_CONFIG;
	} else if (token_match_n_move (&ptr, "<Config") == 0) {
		space_skip (&ptr);
		if (token_match_n_move (&ptr, "Name=") == 0) {
			if (*ptr == '"') {
				r = ++ptr;
				while ((*r != '"') && (*r != '\0')) {r++;}
				if (*r == '\0') {return ST_UNKOWN;}
				snprintf (modal_name, (r - ptr + 1), "%s", ptr);
				XML_DBG ("<Config Name=\"%s\">\n", modal_name);
			}
        		status = ST_CONFIG;
		} else
			return ST_UNKOWN;
	} else if (token_match_n_move (&ptr, "</Config>") == 0) {
		XML_DBG ("</Config>\n");
		status = ST_CONFIG_END;
	} else if (token_match_n_move (&ptr, "</Config_Information_File_8671>") == 0) {
		XML_DBG ("</Config_Information_File_8671>\n");
		status = _ST_CONFIG_END;
	} else if (token_match_n_move (&ptr, "<Dir") == 0) {
		/* <Dir Name=""> */
		space_skip (&ptr);
		if (token_match_n_move (&ptr, "Name=") == 0) {
			if (*ptr == '"') {
				r = ++ptr;
				while ((*r != '"') && (*r != '\0')) {r++;}
				if (*r == '\0') {return ST_UNKOWN;}
				snprintf (dir_name, (r - ptr + 1), "%s", ptr);
				XML_DBG ("<Dir Name=\"%s\">\n", dir_name);
			}
			status = ST_DIR;
		} else 
			return ST_UNKOWN;
	} else if (token_match_n_move (&ptr, "<chain") == 0) {
		space_skip (&ptr);
		if (token_match_n_move (&ptr, "chainName=") == 0) {
			if (*ptr == '"') {
				r = ++ptr;
				while ((*r != '"') && (*r != '\0')) {r++;}
				if (*r == '\0') {return ST_UNKOWN;}
				snprintf (dir_name, (r - ptr + 1), "%s", ptr);
				XML_DBG ("<chain chainName=\"%s\">\n", dir_name);
			}
			status = _ST_DIR;
		} else 
			return ST_UNKOWN;
	} else if (token_match_n_move (&ptr, "</Dir>") == 0) {
		XML_DBG ("</Dir>\n");
		status = ST_DIR_END;
	} else if (token_match_n_move (&ptr, "</chain>")==0) {
		XML_DBG ("</chain>\n");
		status = _ST_DIR_END;
	} else if (token_match_n_move(&ptr, "<Value") == 0) {
		/* <Value Name="" Value=""/> */
		space_skip (&ptr);
		if (token_match_n_move (&ptr, "Name=") == 0) {
			if (*ptr == '"') {
				r = ++ptr;
				while ((*r != '"') && (*r != '\0')) {r++;}
				if (*r == '\0') {return ST_UNKOWN;}
				snprintf (ent_name, (r - ptr + 1), "%s", ptr);
			}
		} else
			return ST_UNKOWN;
		//move on
		ptr = r + 1;
		space_skip (&ptr);
		if (token_match_n_move (&ptr, "Value=") == 0) {
			if (*ptr == '"') {
				r = ++ptr;
				while ((*r != '"') && (*r != '\0')) {r++;}
				if (*r == '\0') {return ST_UNKOWN;}
				if ((r - ptr + 1) > MAX_VALUE_STRING) {
					return ST_UNKOWN;
				}
				snprintf (ent_value, (r - ptr + 1), "%s", ptr);
			}
		} else
			return ST_UNKOWN;
		XML_DBG ("<Value Name=\"%s\" Value=\"%s\"/>\n", ent_name, ent_value);
		status = ST_ENTRY;
	} else if (token_match_n_move (&ptr, "<!--") == 0) {
		/* <!--""--> */
		r = ptr;
		while ((*r != '>') && (*r != '\0')) {r++;}
		if (*r == '\0') {return ST_UNKOWN;}
		if (*r == '>' && *(r - 1) != '-' && *(r - 2) != '-') {
			// ignore this strange comment
			ptr = r + 1;
			return ST_INVALID;
		}
		snprintf (scomment, ((r - 2) - ptr + 1), "%s", ptr);
		XML_DBG ("<!--%s-->\n", scomment);
		status = ST_COMMENT;
	} else {
		// unexpected string
		++ptr;
		XML_DBG ("unexpected string\n");
	}

	_cindex = ptr;

	return status;
}

static stoken_t _returned_token = ST_UNUSED;
static stoken_t
get_stoken (void) {
	if (_returned_token != ST_UNUSED) {
		stoken_t s =_returned_token;
		_returned_token = ST_UNUSED;
		return s;
	} else {
		return _get_stoken ();
	}
}
void return_token (stoken_t s) {
	assert (_returned_token == ST_UNUSED);
	_returned_token = s;
}


// enter_dir()
// 1. find dir in top of dir_stack by dir_name, and push into dir_stack as parsing_dir
// 2. visit all entry's init 
// 3. reset parsing_dir's children's index if exists
static void 
enter_dir (void) {

	if (dir_sp > 0) {
		XML_DBG ("using stack information\n");
		parsing_dir = dir_stack[dir_sp - 1]->firstChild;
	}
	else
		parsing_dir = root_dir_p->firstChild;

	if (parsing_dir == NULL) { 
		fprintf (stderr, "ERR: Directory parsing error\n"); return;
	}
	while (1) {
		if (0 == strncmp (parsing_dir->matchname, dir_name, 
			strlen (parsing_dir->matchname))) {
			parsing_dir->empty_chain = 1;
			push_dir (parsing_dir);			
			XML_DBG ("enter %s(sp=%d)\n", parsing_dir->matchname, dir_sp - 1);
			parsing_ent = parsing_dir->firstEntry;

			// reset current dir's entry
			while (parsing_ent != NULL) {
				if (parsing_ent->init_index)
					parsing_ent->init_index ();
				parsing_ent = parsing_ent->sibling;
			}

			// reset current dir's child            
			parsing_dir = parsing_dir->firstChild;
			while (parsing_dir != NULL) {
				XML_DBG ("[enter_dir] reset child index=[%d]\n", *(parsing_dir->index)); 
				if (0 < *(parsing_dir->index))
					*(parsing_dir->index) = 0;
				parsing_dir = parsing_dir->sibling;
			}

			break;
		} else {
			if (parsing_dir->sibling != NULL) {
				parsing_dir = parsing_dir->sibling;
				continue;
			} else {
				dir_ig = dir_sp + 1;
				XML_DBG ("Warning, unknown dir %s\n", dir_name);                
				break;
			}
		}
	}
}
static void
leave_dir (void) {
	// ignore unrecognizable directory
	if (dir_ig > 0) {--dir_ig; return;}

	parsing_dir = dir_stack[dir_sp - 1];
	XML_DBG ("%s(): sp=[%d] empty=[%d]\n",__FUNCTION__, (dir_sp - 1), (parsing_dir->empty_chain)); 

	if (!(parsing_dir->empty_chain) && (parsing_dir->index)) {	//For DIR have entries.
		parsing_dir->final_idx = (++(*(parsing_dir->index)));
	} else {		//For "single DIR table" and "<DIR Name="...."> </DIR>",  final_idx set to 1. 
		parsing_dir->final_idx = 1;
	}
	XML_DBG ("%s():)[%20s] sp=[%d] final_idx[%3d] max=[%d]\n", __FUNCTION__, 
		parsing_dir->matchname, (dir_sp - 1), parsing_dir->final_idx, parsing_dir->index_max);

	pop_dir ();

	if (dir_sp && !(parsing_dir->empty_chain)) {
		parsing_dir = dir_stack[dir_sp - 1];
		parsing_dir->empty_chain = 0;
	}
}

// ent_set()
// 1. find entry in parsing_dir as parsing_entry by ent_name
// 2. parsing_entry->set(parsing_entry, ent_value);
// 3. if fail, show error message
static void
ent_set (void) {
	// ignore unrecognizable directory
	if (dir_sp >= 1) {
		parsing_dir = dir_stack[dir_sp - 1];
	} else if ((dir_sp == 0) && (dir_ig == 0)) {
		parsing_dir = root_dir_p->firstChild;
	} else 	{return;}

	assert (parsing_dir != NULL);
	parsing_ent = parsing_dir->firstEntry;

	while (parsing_ent != NULL) {
		if (0 == strcmp (parsing_ent->matchname, ent_name)) {
			parsing_dir->empty_chain = 0;
			XML_DBG ("process entry(%s) in dir(%s)\n",
				parsing_ent->matchname, parsing_dir->matchname);
			if (parsing_ent->set (parsing_ent, ent_value) != XML_OK) {
				fprintf (stderr, "ERR: %s:%d: Failed to set %s(%s) value\n",
					current_filename, linenum, parsing_dir->matchname, parsing_ent->matchname);
			}
			return;
		} else {
			parsing_ent = parsing_ent->sibling;
			continue;
		}
	}
}

#define match_token(expected_token) \
    if (get_stoken()!=expected_token) { \
        fprintf(stderr, "ERR: can't get expected token type %s\n",\
        	stoken_list[expected_token].stString);\
        return CFG_SYNTAX_ERROR;}




#ifdef ONE_DIR

/*
program		->	ST_CONFIG process ST_CONFIG_END ST_FILE_END
    		|	_ST_CONFIG process _ST_CONFIG_END ST_FILE_END ;
process		->	one_dir process
			|	;
one_dir     ->  ST_DIR list ST_DIR_END
            |   _ST_DIR list _ST_DIR_END ;
list        ->  ST_ENTRY list
            |   one_dir list
            |   ;
*/

cfg_err_t
g_program (void) {
	stoken_t st;

	st = get_stoken ();
	if (st == ST_CONFIG) {
		if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (ST_CONFIG_END);
		match_token (ST_FILE_END);
	} else if (st ==_ST_CONFIG) {
		if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (_ST_CONFIG_END);
		match_token (ST_FILE_END);
	} else {
		return CFG_SYNTAX_ERROR;
	}
	return CFG_OK;
}


static cfg_err_t 
g_process (void) {
	stoken_t st;
	while (1) {
		st = get_stoken ();
		if (one_dir_ff (st)) {
			return_token (st);
			if (g_one_dir () != CFG_OK) return CFG_SYNTAX_ERROR;
			continue;
		} else {
			return_token (st);
			return CFG_OK;
        	}
    	}
}

static int
one_dir_ff (stoken_t st) {
    return (st == ST_DIR) || (st == _ST_DIR);
}

static cfg_err_t 
g_one_dir (void) {
	stoken_t st;
	st = get_stoken ();
	if (st == ST_DIR) {
		enter_dir ();
		if (g_list () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (ST_DIR_END);
		leave_dir ();
	} else if (st == _ST_DIR) {
		enter_dir ();
		if (g_list () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (_ST_DIR_END);
		leave_dir ();
	} else if (st == ST_COMMENT) {
		// do nothing	
	} else {
		return CFG_SYNTAX_ERROR;
	}
	return CFG_OK;
}

static cfg_err_t 
g_list(void) {
	stoken_t st;
	while (1) {
	st = get_stoken();
        if (st==ST_ENTRY) {
            ent_set();
            continue;
        } else if (one_dir_ff(st)) {
        	return_token(st);
            if (g_one_dir()!=CFG_OK) return CFG_SYNTAX_ERROR;
            continue;
        } else if (st==ST_COMMENT) {
			// do nothing	
			continue;
    	} else {
            return_token(st);
            return CFG_OK;
        }
    }
}
#else

/*
program		->	ST_CONFIG process ST_CONFIG_END ST_FILE_END
    		|	_ST_CONFIG process _ST_CONFIG_END ST_FILE_END ;
process		->  ST_DIR process ST_DIR_END
            |   _ST_DIR process _ST_DIR_END ;
            |   ST_ENTRY process
            |   ;
*/

cfg_err_t 
g_program (void) {
	stoken_t st;

	st = get_stoken ();
	if (st == ST_CONFIG) {
		if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (ST_CONFIG_END);
		match_token (ST_FILE_END);
	} else if (st == _ST_CONFIG) {
		if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
		match_token (_ST_CONFIG_END);
		match_token (ST_FILE_END);
	} else {
		return CFG_SYNTAX_ERROR;
	}
	return CFG_OK;
}


static cfg_err_t 
g_process (void) {
	stoken_t st;
	while (1) {
		st = get_stoken ();

		if (st == ST_DIR) {
			enter_dir ();
			if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
			match_token (ST_DIR_END);
			leave_dir ();
		} else if (st == _ST_DIR) {
			enter_dir ();
			if (g_process () != CFG_OK) return CFG_SYNTAX_ERROR;
			match_token (_ST_DIR_END);
			leave_dir ();
		} else if (st == ST_ENTRY) {
			ent_set ();
			continue;
		} else if (st == ST_COMMENT) {
			// do nothing	
			continue;
		} else {
			return_token (st);
			return CFG_OK;
		}
	}
	return CFG_OK;
}

#endif
