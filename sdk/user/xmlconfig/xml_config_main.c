#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "sysconfig.h"
#include "xml_config.h"
#include "xml_config_gramma.h"

/* Utility Declaration*/
dir_t root_dir = { .matchname="ROOT" }, *root_dir_p
	__attribute__((aligned(4), unused, __section__(".dir_list")))
	= &root_dir;

static entry_t *entry_list_p
    __attribute__((unused, __section__(".entry_list")));

/* End of Declaration*/

#define CHECK_XMLCONFIG_FILE
#define XML_MIB_ACCESS
#define DEFAULT_XML_FILE "/etc/config_default.xml"
#undef CHAIN_USE_STATIC_SH_BUF

typedef enum opt_type {
    OPT_NONE=0,
    OPT_IN_FILE,
    OPT_IN_STDIN,
    OPT_OUT_FILE,
    OPT_OUT_STDOUT,
    OPT_DEBUG,
#ifdef CHECK_XMLCONFIG_FILE
    OPT_CHECK_FILE,
#endif
#ifdef XML_MIB_ACCESS
    OPT_GET_ENTRY,
    OPT_SET_ENTRY,
#endif
    OPT_DEFAULT,
    OPT_DEFAULT_VOIP
} opt_type_t;

static int
tree_entry_init()
{
    entry_t **ent_pp = get_last_entry_ptr();
    dir_t *pDir_p = NULL;

    while(ent_pp!=&entry_list_p) {
        ent_pp --;

        if(*ent_pp==NULL)
            break;

        //parent
        if((pDir_p=((*ent_pp)->dir))!=NULL){
            XML_DBG("entry(%s),dir(%s)", (*ent_pp)->matchname, pDir_p->matchname);
			if(pDir_p->firstEntry!= NULL){
				// Dir have sibling already
				XML_DBG(",sibling(%s)", pDir_p->firstEntry->matchname);
                (*ent_pp)->sibling = pDir_p->firstEntry;
			}
            // and entry is the first entry now
            pDir_p->firstEntry = *ent_pp;

            // init index if entry is array type
            if((*ent_pp)->init_index!=NULL){
                (*ent_pp)->init_index();
            }
            XML_DBG("\n");
        }
    }
    return 0;
}

static int
tree_data_init()
{
    dir_t **dir_pp = get_last_dir_ptr();
    dir_t *pDir_p = NULL;

	while(dir_pp!=&root_dir_p) {
        dir_pp --;

        if(*dir_pp==NULL)
            break;

        XML_DBG("name(%s)", (*dir_pp)->matchname);
        //parent
        if((pDir_p=(*dir_pp)->parent)!=NULL){
            XML_DBG(", parent(%s)", pDir_p->matchname);
			if(pDir_p->firstChild!=NULL){
				// Dir have sibling already
				XML_DBG("sibling(%s)", pDir_p->firstChild->matchname);
                (*dir_pp)->sibling = pDir_p->firstChild;
			}
            // and Dir is the first child now
            pDir_p->firstChild = *dir_pp;

            //init index if dir is array type
            if (pDir_p->index!=NULL) *(pDir_p->index)=0;
        }
        XML_DBG("\n");
    }
    return tree_entry_init();
}

static int
initXMLconfig()
{
    // init metadata tree
    if(0!=tree_data_init()){
        printf("Init metadata tree failed.\n");
        return XML_FAIL;
    }

    return XML_OK;
}

static char buf[SHM_SIZE];	// current SHM_SIZE is 32768 bytes
static char *outbuf;
static unsigned int len, g_index, gint;

extern MIB_T _config;
extern HW_MIB_T _hw_config;

#if 0
#ifdef CHECK_XMLCONFIG_FILE
const char XMLCONFIG_HEADER[] = "<Config Name=\"ROOT\">";
const char XMLCONFIG_TRAILER[] = "</Config>";

// Nils: This function is for checking the validation of whole config file.
static char
checkXmlConfigFile(char *config_file)
{
	FILE *fp;
	char strbuf[255], *pstr;
	int linenum = 1, len, dep = 0;
	char isEnd = 0, status = 1;
	unsigned char inChain[8]={0};	//maximum depth is set to 8

	if(!(fp = fopen(config_file, "r")))
	{
		printf("Open config file failed!! \n");
		return 0;
	}

	len = strlen(fgets(strbuf, 255, fp));
	if(strbuf[len-1] != '\n')
	{
		printf("Miss a newline at line %d!\n", linenum);
		fclose(fp);
		return 0;
	}

	if(strbuf[len-2] == '\r' && strbuf[len-1] == '\n') // Remove the CRLF(0x0D0A) which is at the last of a line.
		strbuf[len-2] = 0;
	else if(strbuf[len-1] == '\n') // Remove the LF(0x0A) which is at the last of a line.
		strbuf[len-1] = 0;

	if(strcmp(strbuf, XMLCONFIG_HEADER))
	{
		printf("Invalid header: %s\n", strbuf);
		fclose(fp);
		return 0;
	}

	while(fgets(strbuf, 255, fp))
	{
		linenum++;  // The counter of current line on handling.
		if (len = strlen(strbuf) <= 0){
			printf("Invalid line(empty). %d\n", linenum);
			status = 0;
			break;
		}

		XML_DBG("%d: %d: %s", linenum, len, strbuf);

		if(strbuf[len-1] != '\n')   // If this line does not end with a newline, return an error.
		{
			printf("Miss a newline at line %d!\n", linenum);
			status = 0;
			break;
		}

		if(strbuf[len-2] == '\r' && strbuf[len-1] == '\n') // Remove the CRLF(0x0D0A) which is at the last of a line.
			strbuf[len-2] = 0;
		else if(strbuf[len-1] == '\n') // Remove the LF(0x0A) which is at the last of a line.
			strbuf[len-1] = 0;

		if(isEnd)  // It should be at the end of this config file.
		{
			printf("Invalid end of config file at line %d\n", linenum);
			status = 0;
			break;
		}

		if(!(pstr = strchr(strbuf, '<')))
		{
			printf("[%c]\n", *pstr);
			printf("Invalid format[<] at line %d: %s\n", linenum, strbuf);
			status = 0;
			break;
		}

		if(!strncmp(pstr, "<Value Name=\"", 13))  // Check the validation of common <Value Name.... line.
		{
			if(!(pstr = strchr(pstr+13, '\"')))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(strncmp(pstr, " Value=\"", 8))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			if(!(pstr = strchr(pstr+8, '\"')))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(strncmp(pstr, "/>", 2))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr += 2;

			if (strstr(pstr, "index")) {	// For chain:  <Value Name="FRAME_SIZE" Value="1"/> <!--index=1-->
				if(strncmp(pstr, " <!--index=", 11)){
					printf("Invalid format at line %d: %s\n", linenum, strbuf);
					status = 0;
					break;
				}

				if(!(pstr = strchr(pstr+11, '>')))
				{
					printf("Invalid format at line %d: %s\n", linenum, strbuf);
					status = 0;
					break;
				}
				pstr++;
			}

			if(*pstr != '\0')
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			continue;
		}

		if(!strncmp(pstr, "<Dir Name=\"", 11))  // Enter in a chain table.	<Dir Name="ATM_VC_TBL"> <!--index=0--> 		<chain chainName="IP_QOS_QUEUE_TBL">
		{
			if(inChain[dep]) // It has been in another chain table currently.
			{
				printf("Invalid structure at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			if(!(pstr = strchr(pstr+11, '\"')))
			{
				printf("[1]Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(*pstr != '>')
			{
				printf("[2]Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if (strstr(pstr, "index")) {	// For chain:  <Dir Name="ATM_VC_TBL"> <!--index=0-->
				if(strncmp(pstr, " <!--index=", 11)){
					printf("[3]Invalid format at line %d: %s\n", linenum, strbuf);
					status = 0;
					break;
				}

				if(!(pstr = strchr(pstr+11, '>')))
				{
					printf("[4]Invalid format at line %d: %s\n", linenum, strbuf);
					status = 0;
					break;
				}
				pstr++;
			}

			if(*pstr != '\0' )
			{
				printf("[5]Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			inChain[dep] = 1;
			dep++;
			continue;
		}

		if(!strncmp(pstr, "</Dir>", 6))  // Leave from a chain table.
		{
			if(inChain[dep-1])
			{
				inChain[dep-1] = 0;
				dep--;
				continue;
			}
			else
			{
				printf("Invalid structure at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}
		}

		if(!strcmp(strbuf, XMLCONFIG_TRAILER))  // Reach the end of config file.
		{
			isEnd = 1;
			continue;
		}

		printf("Unknown format at line %d: %s\n", linenum, strbuf);
		status = 0;
		break;
	}

	fclose(fp);
	return status;
}
#endif
#endif

#ifdef XML_MIB_ACCESS
static int actStatus = 0;
static dir_t *check_dir = NULL;

/*For MIB Table only. Get from mib and update local memory then find out entry value.*/
static int
getOneEntry(int depth, char *name)
{
	entry_t *check_ent = NULL;
	char str[OUT_STR_LEN];

	len = 0;

	do {
		if (NULL == check_ent)
			check_ent = check_dir->firstEntry;
		else
			check_ent = check_ent->sibling;
		XML_DBG ("%s(): table_entry[%s] v.s [%s].\n", __FUNCTION__, check_ent->matchname, name);

		if (!strcmp (check_ent->matchname, name)) {
			memset (buf, 0, sizeof (buf));
			if (mib_get (check_ent->id, buf) == 0) {
				fprintf (stderr, "[ERR] mib_get failed for entry(%s)\n", check_ent->matchname);
				return XML_FAIL;
			}
			XML_DBG ("%s(): mib_get %s(%d), set_val=[%s] len=(%d)\n", __FUNCTION__, check_ent->matchname, check_ent->id, buf, strlen(buf));

			if (check_ent->mcp_set (check_ent, buf) != XML_OK) {
				fprintf (stderr, "[ERR] entry(%s) set failed\n", check_ent->matchname);
				return XML_FAIL;
			}
			XML_DBG ("%s(): mcp_set %s with value [%s]\n", __FUNCTION__, check_ent->matchname, buf);

			if (check_ent->get != NULL) {
				if (check_ent->get (check_ent, str, &len) != XML_OK) {
					fprintf (stderr, "[ERR] entry(%s) get failed\n", check_ent->matchname);
					return XML_FAIL;
				}
			}
			printf ("%s=%s\n", name, str);
			XML_DBG ("%s(): Get entry %s=[%s] len[%d]\n\n", __FUNCTION__, check_ent->matchname, str, len);

			actStatus = XML_GET;
			return XML_OK;
		}
	} while (check_ent->sibling != NULL);

	return XML_OK;
}

static int
setOneEntry (int depth, char *name, char *value)
{
	entry_t *check_ent = NULL;
	len = 0;

	do {
		if (NULL == check_ent)
			check_ent = check_dir->firstEntry;
		else
			check_ent = check_ent->sibling;
		XML_DBG ("%s(): table_entry[%s] v.s [%s].\n", __FUNCTION__, check_ent->matchname, name);

		if (!strcmp (check_ent->matchname, name)) {
			if (check_ent->set (check_ent, value) != XML_OK) {
				fprintf (stderr, "[ERR] Failed to set %s value\n", check_ent->matchname);
				return XML_FAIL;
			}
			XML_DBG ("%s(): set %s(%d), set_val=[%s]\n", __FUNCTION__, check_ent->matchname, check_ent->id, value);
			memset (buf, 0, sizeof (buf));
			if (check_ent->mcp_get (check_ent, buf, &len) != XML_OK) {
				fprintf (stderr, "[ERR] Entry(%s) get failed\n", check_ent->name);
				return XML_FAIL;
			}
			XML_DBG ("%s(): mcp_get %s with value [%s]\n", __FUNCTION__, check_ent->matchname, buf);
			if (!mib_set (check_ent->id, buf)) {
				fprintf (stderr, "[ERR] mib_set(%s) failed\n", check_ent->matchname);
				return XML_FAIL;
			}
			XML_DBG ("%s(): Set entry %s=[%s]\n\n", __FUNCTION__, check_ent->matchname, value);

			actStatus = XML_SET;
			return XML_OK;
		}
	} while (check_ent->sibling != NULL);

	return XML_OK;
}

/*Find out the chain entry value.*/
static int
getChainEntry(int depth, char *tok_str[])
{
	dir_t *entry_dir = NULL;
	entry_t *check_ent = NULL;
	char str[OUT_STR_LEN];

	entry_dir = check_dir;
	XML_DBG("%s(): dir(%s) index=[%d]/[%d]\n", __FUNCTION__,
			entry_dir->matchname, *(entry_dir->index), entry_dir->index_max);
#if 1
	//Specify the CHAIN index as <input_index>
	if (((*(entry_dir->index)) = atoi(tok_str[1])) >= entry_dir->index_max) {
		fprintf (stderr, "%s(): [ERR] Chain(%s) index out of range(0~%d).\n",
			__FUNCTION__, check_ent->matchname, entry_dir->index_max);
		return XML_FAIL;
	}

	//Compare with all entry name.
	do {
		len = 0;
		if (NULL == check_ent)
			check_ent = entry_dir->firstEntry;
		else
			check_ent = check_ent->sibling;

		if (0 == strlen (&tok_str[2])) {	/* For DIR.idx */
			memset (str, '\0', sizeof (str));
			if (check_ent->get != NULL) {
				if (check_ent->get (check_ent, str, &len) != XML_OK) {
					fprintf (stderr, "[ERR] entry(%s) get failed\n", check_ent->matchname);
					return XML_FAIL;
				}
				printf ("%s=%s\n", check_ent->matchname, str);
			}

			if (NULL == check_ent->sibling) {
				actStatus = XML_GET;
				return XML_OK;
			}
		} else {	/* For DIR.idx.entry */
			XML_DBG ("%s(): Entry is [%s] vs [%s]\n", __FUNCTION__, check_ent->matchname, tok_str[2]);
			if (0 == strncmp (check_ent->matchname, tok_str[2], strlen (check_ent->matchname) + 1)) {
				if (check_ent->get != NULL) {
					if (check_ent->get (check_ent, str, &len) != XML_OK) {
						fprintf (stderr, "[ERR] entry(%s) get failed\n", check_ent->matchname);
						return XML_FAIL;
					}
				}
				printf ("%s=%s\n", check_ent->matchname, str);
				XML_DBG ("%s(): get entry (%s) with value[%s] len[%d] success.\n\n",
						__FUNCTION__, check_ent->matchname, str, len);

				actStatus = XML_GET;
				return XML_OK;
			}
		}
	} while (NULL != check_ent->sibling);

	fprintf (stderr, "[ERR] Warning, unknown entry.\n");
	return XML_FAIL;
#else
//	getChainEntry(int depth, char *q_str[])

    while(entry_dir != NULL){
		if(entry_dir->index && (*(entry_dir->index)>=entry_dir->index_max)) break;
			printf("%s(): dir(%s) index=[%d]/[%d] to get [%s]\n", __FUNCTION__,
				entry_dir->matchname, *(entry_dir->index), entry_dir->index_max, q_str[3]);

        if(atoi(q_str[3]) == (*(entry_dir->index))){
			do {
				len = 0;
				if(NULL == check_ent)
					check_ent=entry_dir->firstEntry;
				else
					check_ent=check_ent->sibling;
				printf("%s():  entry is [%s]\n",
					__FUNCTION__, check_ent->matchname);

				if(0 == strncmp(check_ent->matchname, q_str[4], strlen(check_ent->matchname)+1)){
					if(check_ent->get!=NULL){
						if(check_ent->get(check_ent, str, &len) != XML_OK){
							fprintf(stderr, "[ERR] entry(%s) get failed\n", check_ent->matchname);
							return XML_FAIL;
						}
					}
					printf("%s():  get entry (%s) with value[%s] len[%d] success.\n\n",
						__FUNCTION__, check_ent->matchname, str, len);

					actStatus = XML_GET;
					return XML_OK;
				}
			} while(NULL != check_ent->sibling);

			fprintf(stderr, "[ERR] Warning, unknown entry [%s]\n", q_str[4]);
			return XML_FAIL;
		}

        (*(entry_dir->index))++;
        entry_dir=entry_dir->firstEntry->dir;
   	}

	return XML_OK;
#endif
}

/*Set a specific chain entry value.*/
/*tok_str : <0_dir_name>.<1_idx>.<2_ent_name>[.<3_idx>.<4_ent_name>...] */
static int
setChainEntry (int depth, char *tok_str[], char *value)
{
	dir_t *entry_dir = NULL;
	entry_t *check_ent = NULL;
	entry_dir = check_dir;

	if (0 == strlen (tok_str[2])) {
		fprintf (stderr, "%s(): [ERR] No Entry name for setting.\n", __FUNCTION__);
		return XML_FAIL;
	}
	XML_DBG ("%s(): dir=(%s) index=[%d]/[%d] entry=[%s]\n", __FUNCTION__,
			entry_dir->matchname, *(entry_dir->index), entry_dir->index_max, tok_str[2]);

	//Specify the CHAIN index as <input_index> and check max value.
	if (((*(entry_dir->index)) = atoi (tok_str[1])) >= entry_dir->index_max) {
		fprintf (stderr, "%s(): [ERR] Chain(%s) index out of range(0~%d).\n", __FUNCTION__,
			check_ent->matchname, entry_dir->index_max);
		return XML_FAIL;
	}

	//Compare with all entry name.
	do {
		if (NULL == check_ent)
			check_ent = entry_dir->firstEntry;
		else
			check_ent = check_ent->sibling;

		XML_DBG ("%s(): Entry is [%s] vs [%s]\n", __FUNCTION__, check_ent->matchname, tok_str[2]);
		if (0 == strncmp (check_ent->matchname, tok_str[2], strlen (check_ent->matchname) + 1)) {
			// Update local entry information
			if (check_ent->set != NULL) {
				if (check_ent->set (check_ent, value) != XML_OK) {
					fprintf (stderr, "[ERR] Failed to set %s value\n", check_ent->matchname);
					return XML_FAIL;
				}
			}
			XML_DBG ("%s(): Set entry %s=[%s] to local memory.\n", __FUNCTION__, check_ent->matchname, value);

			// Update specific chain setting from local to mib_chain
			len = getPositionById (entry_dir->id, &outbuf);
			if (len == 0) {
				fprintf (stderr, "[ERR] get dir(%s) information failed\n", entry_dir->matchname);
				return XML_FAIL;
			}
			if (mib_chain_update (entry_dir->id, (unsigned char*)outbuf + (atoi (tok_str[1]) * len), (atoi (tok_str[1]))) == 0) {
				fprintf (stderr, "[ERR] mib_chain_update failed for DIR=%s, idx=%d)\n", entry_dir->matchname, atoi (tok_str[1]));
				return XML_FAIL;
			}
			XML_DBG ("%s(): %s(%d)_%d mib_chain_update success.\n", __FUNCTION__, entry_dir->matchname, entry_dir->id, atoi (tok_str[1]));

			actStatus = XML_SET;
			return XML_OK;
		}
	} while (NULL != check_ent->sibling);

	fprintf (stderr, "[ERR] Warning, unknown dir.idx.entry\n");
	return XML_FAIL;
}

/*Get chain setting with index, then update local memory.*/
static int
getOneChain(int depth, char *chain_idx)
{
	if ((g_index = mib_chain_total (check_dir->id)) > 0) {
		// get total number of specific chain
		if ((check_dir->index_max >= 1) && (check_dir->index_max >= g_index)) {
			check_dir->index_max = g_index;
		}
		XML_DBG ("%s(): Query chain index=[%d]/[0~%d]\n", __FUNCTION__, atoi (chain_idx), g_index - 1);

		// get local chain information (position & size) from id
		len = getPositionById (check_dir->id, &outbuf);
		if (len == 0) {
			fprintf (stderr, "[ERR] get dir(%s) information failed\n", check_dir->matchname);
			return XML_FAIL;
		}
		XML_DBG ("%s(): get information id=%d len=%d buf=%p\n", __FUNCTION__, check_dir->id, len, outbuf);

		// get content with mib_chain_get ; copy to local position
		g_index = atoi (chain_idx);
		if (g_index < check_dir->index_max) {
			XML_DBG ("%s(): mib_chain_get(%s(id=%d), %d/%d)\n", __FUNCTION__,
				check_dir->matchname, check_dir->id, g_index, check_dir->index_max);
#ifdef CHAIN_USE_STATIC_SH_BUF
			memset (buf, 0, sizeof (buf));
			if (mib_chain_get (check_dir->id, g_index, buf) == 0) {
#else
			if (mib_chain_get (check_dir->id, g_index, outbuf + (len * g_index)) == 0) {
#endif
				fprintf (stderr,"[ERR] get chain(%s, %d) failed\n",
					check_dir->matchname, (check_dir->index)?(*(check_dir->index)):1);
				return XML_FAIL;
			}
#ifdef CHAIN_USE_STATIC_SH_BUF
			memcpy (outbuf + (len * g_index), buf, len);
#endif
			XML_DBG("%s(): DIR(%s) updated index=(%d)/(0~%d) success.\n", __FUNCTION__,
				check_dir->matchname, g_index, (check_dir->index_max) - 1);
			return XML_OK;
		} else {
			fprintf (stderr,"[ERR] Input chain index(%d) out of range(0~%d).\n", g_index, (check_dir->index_max) - 1);
			return XML_FAIL;
		}
	} else {
		fprintf (stderr, "[ERR] Get dir(%s) with an error index or empty.\n", check_dir->matchname);
		return XML_FAIL;
	}
}

static int
accessEntryVal(dir_t *dir, int depth, char *q_str[], int action)
{
	int i = 0;
	char dir_ent[100];
	char *token;
	char *tok_str[7] = {NULL};	/* <0_dir_name>.<1_idx>.<2_ent_name>[.<3_idx>.<4_ent_name>...] */

	if (dir == NULL) {
		fprintf (stderr, "[ERR] directory is null\n");
		return XML_FAIL;
	}
	XML_DBG ("%s(): DIR(%s) original %d/%d\n", __FUNCTION__,
		dir->matchname, (dir->index)?(*(dir->index)):0, dir->index_max);

	check_dir = dir->firstChild;

	/* Parser the input to tok_str[] --> Get the chain index number.  chian.0.~ */
	strcpy (dir_ent, q_str[2]);
	token = strtok (dir_ent, ".");
	while (token != NULL) {
		tok_str[i] = token;
		XML_DBG ("The token is: [%d]_[%s]\n", i, tok_str[i]);
		i++;
		token = strtok (NULL, ".");
	}

	/*while loop for access entry value.*/
	while (check_dir != NULL) {
		XML_DBG ("%s(): chain_[%s](%d) v.s input_[%s](%s)(%s)\n", __FUNCTION__,
			check_dir->matchname, check_dir->id, q_str[2], q_str[3], q_str[4]);

		//For mib_table
		if ((check_dir->id == 0) && (NULL == tok_str[1])) {
			XML_DBG ("%s(): For table[%s](%d) to find [%s]\n", __FUNCTION__,
				check_dir->matchname, check_dir->id, tok_str[0]);
			if (action == OPT_GET_ENTRY) {
				if (getOneEntry (depth, tok_str[0]) != XML_OK) {
					fprintf (stderr, "[ERR] getOneEntry() failed\n");
					return XML_FAIL;
				} else if (actStatus == XML_GET)
					return XML_OK;
			} else {	//action == OPT_SET_ENTRY
				if (setOneEntry (depth, tok_str[0], q_str[3]) != XML_OK) {
					fprintf (stderr, "[ERR] setOneEntry() failed\n");
					return XML_FAIL;
				} else if (actStatus == XML_SET)
					return XML_OK;
			}
		}

		//For mib_chain
		if ((0 == strncmp (check_dir->matchname, tok_str[0], strlen (check_dir->matchname))) &&
			(0 != strlen (tok_str[1]))) {
			XML_DBG ("%s(): For chain[%s](%d) to find [%s]\n", __FUNCTION__,
				check_dir->matchname, check_dir->id, tok_str[0]);

			/* Update Local chain data from mib by chain index number. */
			if (getOneChain (depth, tok_str[1]) != XML_OK) {
				fprintf (stderr, "[ERR] getOneChain() failed\n");
				return XML_FAIL;
			}

			/* Get Entry value from Local. */
			if (action == OPT_GET_ENTRY) {
				if (getChainEntry (depth, tok_str) != XML_OK){
					fprintf (stderr, "[ERR] getChainEntry() failed\n");
					return XML_FAIL;
				} else if (actStatus == XML_GET)
					return XML_OK;
			} else {	//action == OPT_SET_ENTRY
				if (setChainEntry (depth, tok_str, q_str[3]) != XML_OK) {
					fprintf (stderr, "[ERR] setChainEntry() failed\n");
					return XML_FAIL;
				} else if (actStatus == XML_SET)
					return XML_OK;
			}
		} else {
			if (check_dir->sibling != NULL) {
				check_dir = check_dir->sibling;
				continue;
			} else {
				fprintf (stderr, "[ERR] Warning, unknown dir name: %s\n", tok_str[0]);
				break;
			}
		}
	}

	return XML_OK;
}
#endif

static int
getEntry(entry_t *entry, int depth)
{
	if (entry == NULL) {
		fprintf (stderr, "[ERR] entry is null\n");
		return XML_FAIL;
	}

	// a while loop for setting entry
	// FIXME: check entry array
	if (entry->init_index) entry->init_index ();

	do {
		memset (buf, 0, sizeof (buf));
		if (mib_get (entry->id, buf) == 0) {
			fprintf (stderr, "[ERR] mib_get failed for entry(%s)\n", entry->matchname);
			return XML_FAIL;
		}
		XML_DBG ("getEntry: 1. mib_get with id (%d) value=[%s] len=(%d)\n", entry->id, buf, strlen (buf));
		if (entry->mcp_set (entry, buf) != XML_OK) {
			fprintf (stderr, "[ERR] entry(%s) set failed\n", entry->matchname);
			return XML_FAIL;
		}
		XML_DBG ("getEntry: 2. mcp_set entry (%s) with value [%s]\n\n", entry->matchname, buf);
	} while (entry->init_index != NULL);

	if (entry->sibling != NULL) {
		if (getEntry (entry->sibling, depth) != XML_OK) {
			//fprintf (stderr, "[ERR] entry(%s) get failed\n", entry->matchname);
			return XML_FAIL;
		}
	}
	return XML_OK;
}

/* get config from configd */
static int
getConfig(dir_t *dir, int depth)
{
	if (dir == NULL) {
		fprintf (stderr, "[ERR] directory is null\n");
		return XML_FAIL;
	}

	XML_DBG ("Debug: DIR(%s) original %d/%d\n", dir->matchname, (dir->index)?(*(dir->index)):0, dir->index_max);

	if (dir->id > CHAIN_ENTRY_TBL_ID && dir->id <= MIB_CHAIN_TBL_END) {
		if ((g_index = mib_chain_total (dir->id)) > 0) {
			// get total number of specific chain
			if ((dir->index_max >= 1) && (g_index <= dir->index_max)) {
				dir->empty_chain = 0;        // For genConfig() use, to show <Dir Name="..."> </Dir>
				dir->final_idx = g_index;    // To record chain index number.
			}
			XML_DBG ("Debug: DIR(%s) index=%d/%d/%d\n", dir->matchname, g_index, dir->final_idx, dir->index_max);

			// get local chain information (position & size) from id
			len = getPositionById (dir->id, &outbuf);
			if (len == 0) {
				fprintf (stderr, "get dir(%s) information failed\n", dir->matchname);
				return XML_FAIL;
			}
			XML_DBG ("get information id=%d len=%d buf=%p\n", dir->id, len, outbuf);

			// a while loop for setting chain
			// get content with mib_chain_get
			// copy to local position
			g_index = 0;
			while (g_index < dir->final_idx) {
				XML_DBG ("DIR(%s), index=%d/%d/%d\n", dir->matchname, g_index, dir->final_idx, dir->index_max);
#ifdef CHAIN_USE_STATIC_SH_BUF
				memset (buf, 0, sizeof (buf));
				if (mib_chain_get (dir->id, g_index, buf) == 0) {
#else
				if (mib_chain_get (dir->id, g_index, outbuf + (len * g_index)) == 0) {
#endif
					fprintf (stderr,"[ERR] get chain(%s, %d) failed\n",
						dir->matchname, (dir->index)?(*(dir->index)):1);
					return XML_FAIL;
				}
#ifdef CHAIN_USE_STATIC_SH_BUF
				memcpy (outbuf + (len * g_index), buf, len);
#endif
				XML_DBG("mib_chain_get(%d, %d)\n", dir->id, g_index);
				++g_index;
			}
        	} else {
			dir->empty_chain = 1;
			dir->final_idx = 0;
		}
        	XML_DBG ("Debug: DIR(%s) modified %d/%d E[%d]\n", dir->matchname,
			(dir->index)?(*(dir->index)):0, dir->index_max, dir->empty_chain);
	} else {
		if (dir->firstEntry) {
			if (getEntry (dir->firstEntry, depth + 1) != XML_OK) {
				fprintf (stderr, "[ERR] entry get failed in dir(%s).\n", dir->matchname);
				return XML_FAIL;
			}
	    	}

		if (dir->firstChild) {
	        	if (getConfig (dir->firstChild, depth + 1) != XML_OK) {
				fprintf (stderr, "[ERR] first child get failed in dir(%s).\n", dir->matchname);
				return XML_FAIL;
			}
		}
    	}

	if (dir->sibling) {
		if (getConfig (dir->sibling, depth) != XML_OK) {
			fprintf (stderr, "[ERR] sibling get failed in dir(%s).\n", dir->matchname);
			return XML_FAIL;
        	}
    	}

	return XML_OK;
}

static int
setEntry(entry_t *entry, int depth)
{
	if (entry == NULL) {
		fprintf (stderr, "[ERR] entry is null\n");
		return XML_FAIL;
	}

	// a while loop for setting entry
	memset (buf, 0, sizeof (buf));
	len = 0;
	if (entry->mcp_get (entry, buf, &len) != XML_OK) {
		fprintf (stderr, "[ERR] entry(%s) get failed\n", entry->name);
		return XML_FAIL;
	}

	XML_DBG ("setEntry(): mib_set(%s(%d),len=[%d])\n", entry->matchname, entry->id, len);
	if (!mib_set (entry->id, buf)) {
		fprintf (stderr, "[ERR] mib_set(%s) failed\n", entry->matchname);
		return XML_FAIL;
	}

	if (entry->sibling != NULL) {
		if (setEntry (entry->sibling, depth) != XML_OK) {
			fprintf (stderr, "[ERR] entry(%s) set failed\n", entry->name);
			return XML_FAIL;
		}
	}
	return XML_OK;
}

static int
setConfig(dir_t *dir, int depth)
{
	// set config to configd

	if (dir == NULL) {
		fprintf (stderr, "[ERR] directory is null.\n");
		return XML_FAIL;
	}
	XML_DBG ("DIR(%s) final_idx[%d] empty[%d]\n", dir->matchname, dir->final_idx, dir->empty_chain);

	if ((0 != dir->final_idx) && dir->id>CHAIN_ENTRY_TBL_ID && dir->id <= MIB_CHAIN_TBL_END) {

		// clear mib chain
		if (mib_chain_clear(dir->id) == 0) {
			fprintf (stderr, "[ERR] clear dir(%s) failed.\n", dir->matchname);
            		return XML_FAIL;
		}

		// get local chain information
		len = getPositionById (dir->id, &outbuf);
		if (len == 0) {
			fprintf (stderr, "[ERR] get dir(%s) information failed\n", dir->matchname);
			return XML_FAIL;
		}
		g_index = 0;
		// a while loop for setting chain
		while (1) {
			gint = (dir->empty_chain == 0)?(dir->final_idx):0;
			XML_DBG ("%s_%d/%d/%d, clean=[%d]\n", dir->matchname, g_index, gint, dir->index_max, dir->empty_chain);

			if (0 == gint) break;

			if (mib_chain_add (dir->id, (unsigned char*)outbuf + (g_index * len)) == 0) {
				fprintf (stderr, "[ERR] mib_chain_add failed for dir(%s, %d)\n",
					dir->matchname, gint);
					return XML_FAIL;
			}
			XML_DBG ("%s_%d/%d/%d, mib_chain_add done.\n", dir->matchname, g_index, gint, dir->index_max);

			++g_index;
			if (g_index >= dir->index_max || g_index >= gint) break;
		}
	} else {
		if (dir->firstEntry) {
			if ((0 != dir->final_idx) && (setEntry (dir->firstEntry, depth + 1) != XML_OK)) {
				fprintf (stderr, "[ERR] entry set failed in dir(%s).\n", dir->matchname);
				return XML_FAIL;
			}
		}

		if (dir->firstChild) {
			if (setConfig (dir->firstChild, depth + 1) != XML_OK) {
				fprintf (stderr, "[ERR] first child set failed in dir(%s)\n", dir->matchname);
				return XML_FAIL;
			}
		}
	}

	if (dir->sibling) {
		if (setConfig (dir->sibling, depth) != XML_OK) {
			fprintf (stderr, "[ERR] sibling set failed in dir(%s)\n", dir->matchname);
			return XML_FAIL;
		}
	}

	return XML_OK;
}

#define XMLCONFIG_LOCK "/var/run/xmlconfig.lock"
static int lock_by_flock (void)
{
	int lockfd;

	if ((lockfd = open (XMLCONFIG_LOCK, O_CREAT)) == -1) {
		perror ("open lockfile failed");
		return lockfd;
	}
	while (flock (lockfd, LOCK_EX) == -1 && errno == EINTR) {
		fprintf (stderr, "Lock failed by flock. errno = %d\n", errno);
	}

	return lockfd;
}

static int unlock_by_flock (int lockfd)
{
	while (flock (lockfd, LOCK_UN) == -1 && errno == EINTR) {
		fprintf (stderr, "Unlock failed by flock. errno = %d\n", errno);
	}
	close (lockfd);

	return 0;
}

static void
print_usage(void){
	printf("usage: xmlconfig [options] [default resource options]\n");
	printf("  options:\n");
#ifdef CHECK_XMLCONFIG_FILE
	printf("    -c <filen ame>:                 check xmlconfig format file\n");
#endif
#ifdef XML_MIB_ACCESS
	printf("    -g <DIR.entry>:                 get a specific entry from mib.\n");
	printf("    -s <DIR.entry> <value>:         set a specific entry from mib.\n");
#endif
	printf("    -h:                             show usage\n");
	printf("    -if <filename> -def <filename>: input from file && default.\n");
	printf("    -if <filename> -nodef:          input from file only.\n");
	printf("    -is:                            input from standard input\n");
	printf("    -of [-hs] <filename>:           output to file\n");
	printf("    -os [-hs]:                      output to standard output\n");
	printf("  default resource options:\n");
	printf("    -def_mib: [-hs]                 set default source from mib\n");
#ifdef VOIP_SUPPORT
	printf("    -def_voip_mib:                  set default voip source from mib\n");
#endif
}

int main(int argc, char *argv[]){
	opt_type_t opt=OPT_NONE;
	CONFIG_DATA_T xml_type=CURRENT_SETTING;
  	int def_flag=0, mib_type=0;
	int i=0, ret=0;
	char *fname=NULL, *dfname=NULL;
	int fd = 0;

	// err check
	if(argc<=1 || argc>6){
		print_usage();
		return XML_FAIL;
	}

	// option parsing
	for(i=1; i<argc; i++){
		//printf("%s(%d)\n", argv[i], i);
		if(0==strcasecmp(argv[i], "-h")){
			print_usage();
			return 0;
		} else if(0==strcasecmp(argv[i], "-if")){
			if(argc>(i+1)){
				fname=argv[++i];
//				opt=OPT_IN_FILE;
			} else {
				print_usage();
				return XML_FAIL;
			}
		} else if (0==strcasecmp(argv[i], "-is")) {
			opt=OPT_IN_STDIN;
		} else if (0==strcasecmp(argv[i], "-of")) {
			if(argc>(i+1)){
				if (0==strcasecmp(argv[i+1], "-hs")) {
					xml_type=HW_SETTING;
					i++;
				}
				fname=argv[++i];
				opt=OPT_OUT_FILE;
				continue;
			} else {
				print_usage();
				return XML_FAIL;
			}
		} else if (0==strcasecmp(argv[i], "-hs")) {
			xml_type=HW_SETTING;
		} else if ((0==strcasecmp(argv[i], "-os")) || (0==strcasecmp(argv[i], "all"))) {
			opt=OPT_OUT_STDOUT;
		} else if (0==strcasecmp(argv[i], "-def")) {
			def_flag=1;
			if(argc>(i+1)){
				dfname=argv[++i];
				opt=OPT_IN_FILE;
			} else {
				print_usage();
				return XML_FAIL;
			}
		} else if (0==strcasecmp(argv[i], "-nodef")) {
			def_flag=0;
			opt=OPT_IN_FILE;
		} else if (0==strcasecmp(argv[i], "-def_mib")) {
			opt=OPT_DEFAULT;
			mib_type = ((argc == 3) && !strcasecmp(argv[2], "-hs")) ? HW_SETTING : CURRENT_SETTING;
#ifdef VOIP_SUPPORT
		} else if(0==strcasecmp(argv[1], "-def_voip_mib")){
			opt=OPT_DEFAULT_VOIP;
#endif
		} else if (0==strcasecmp(argv[i], "-debug")){
			opt=OPT_DEBUG;
#ifdef CHECK_XMLCONFIG_FILE
		} else if (0==strcasecmp(argv[i], "-c")){
			if(argc>(i+1)){
				fname=argv[++i];
				opt=OPT_CHECK_FILE;
			} else {
				print_usage();
				return XML_FAIL;
			}
#endif
#ifdef XML_MIB_ACCESS
		} else if ((0==strcasecmp(argv[i], "-g"))||(0==strcasecmp(argv[i], "get"))) {
			if(argc>(i+1)){
				opt = OPT_GET_ENTRY;
				break;
			} else {
				print_usage();
				return XML_FAIL;
			}
		} else if ((0==strcasecmp(argv[i], "-s"))||(0==strcasecmp(argv[i], "set"))) {
			if(argc>(i+2)){
				opt = OPT_SET_ENTRY;
				break;
			} else {
				print_usage();
				return XML_FAIL;
			}
#endif
		} else {
			print_usage();
			return XML_FAIL;
		}
	}

	initXMLconfig();
	// FIXME: lock flash
	XML_DBG("===== xmlconfig opt[%d] fname=[%s] dfname=[%s] =====\n", opt, fname, dfname);
	switch(opt){
	case OPT_IN_FILE:
	case OPT_IN_STDIN:
		// The final configured value = orig_mib + dfname + fname.
		if(getConfig(root_dir_p, 0)==XML_FAIL){	//from mib, always load mib first when input file
			printf("getConfig() failed\n");
			return XML_FAIL;
		}
		if(def_flag) {
			// load input with customized default resource first
			if(dfname!=NULL){
				ret = config_parser(dfname); // from file
				printf("\n***** config_parser(): dfname=[%s](%d) loaded *****\n", dfname, ret);
			} else {
				getConfig(root_dir_p, 0); // from mib ...
			}
		}
		// replace configured value
		ret = config_parser(fname);
		if (fname) printf("\n***** config_parser(): fname=[%s](%d) loaded *****\n\n", fname, ret);
		setConfig(root_dir_p, 0);
		break;
	case OPT_OUT_FILE:
		fd = lock_by_flock ();
		if(fname=='\0'){
			printf("Error: Invalid filename\n");
			return XML_FAIL;
		}
		if(getConfig(root_dir_p, 0)==XML_FAIL){
			printf("getConfig() failed\n");
			return XML_FAIL;
		}
		gen_file(xml_type, fname);
		unlock_by_flock (fd);
		break;
	case OPT_OUT_STDOUT:
		// output to console
		if(getConfig(root_dir_p, 0)==XML_FAIL){
			printf("getConfig() failed\n");
			return XML_FAIL;
		}
		gen_file(xml_type, NULL);
		break;
	case OPT_DEBUG:
		printTree(root_dir_p, 0);
		break;
#ifdef CHECK_XMLCONFIG_FILE
	case OPT_CHECK_FILE:
		printf("Checking input file [%s].\n", fname);
		if(!config_parser(fname)){
			printf("[OK] Valid config file!\n");
			break;
		}
		printf("[FAIL] Invalid config file!\n");
		return XML_FAIL;
#endif
#ifdef XML_MIB_ACCESS
	case OPT_GET_ENTRY:
		if(accessEntryVal(root_dir_p, 0, argv, OPT_GET_ENTRY) == XML_FAIL){
			printf("[FAIL] GET_ENTRY failed\n");
			return XML_FAIL;
		}

		if(actStatus != XML_GET)
			printf("GET fail.\n");
		break;
	case OPT_SET_ENTRY:
		if(accessEntryVal(root_dir_p, 0, argv, OPT_SET_ENTRY) == XML_FAIL){
			printf("[FAIL] SET_ENTRY failed\n");
			return XML_FAIL;
		}

		if(actStatus == XML_SET)
			printf("%s=%s\n", argv[2], argv[3]);
		else
			printf("SET fail.\n");
		break;
#endif
	case OPT_DEFAULT:
		mib_sys_to_default(mib_type);
		printf("Fetch mib data (type %d) from program code.\n", mib_type);
		break;
#ifdef VOIP_SUPPORT
	case OPT_DEFAULT_VOIP:
		mib_voip_to_default();
		printf("Reset to default voip from program code.\n");
		break;
#endif
	default:
		//should not get here
		print_usage();
		break;
	}
	return ret;
}

