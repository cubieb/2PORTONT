#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "xml_config.h"

unsigned int bc=0, idx=0;
char strbuf[OUT_STR_LEN]={0};
FILE *fp;

#define TYPE_SETTING_CHAR	' '
//Keyword
const char *NAME_KEYWORD = "Name";
const char *VALUE_KEYWORD = "Value";
const char *ENTRY_KEYWORD = "Value";
const char *DIR_KEYWORD = "Dir";
const char *ROOT_KEYWORD = "Config";

extern dir_t root_dir;

int pushchar(char *buf, char chr, int n)
{
	int i, len = 0;
	for (i = 0; i < n; i++)
		len += sprintf (buf + len, "%c", chr);
	return len;
}

XML_OPR_t
genConfigEntry(CONFIG_DATA_T xml_type, entry_t *entry, int depth)
{
	//assert(entry);
	if (entry == NULL) {
		fprintf (stderr, "[ERR] entry is null\n");
		return -1;
	}

	// get entry value
	XML_OPR_t status = XML_FAIL;
	idx = 0;
	do {
		memset (strbuf, 0, sizeof (strbuf));

		bc = pushchar (strbuf, TYPE_SETTING_CHAR, depth);

		bc += snprintf (strbuf + bc, OUT_STR_LEN - bc, "<%s %s=\"%s\" %s=\"",
			ENTRY_KEYWORD, NAME_KEYWORD, entry->matchname, VALUE_KEYWORD);
		if (bc >= OUT_STR_LEN) goto  _WRITE_NULL_VAL;

		if (entry->get != NULL) {
			status = entry->get (entry, strbuf + bc, &bc);
			if (bc >= OUT_STR_LEN) goto  _WRITE_NULL_VAL;
		}

		if (status != XML_FAIL) {
			bc += snprintf (strbuf + bc, OUT_STR_LEN - bc, "\"/>");
			if (bc >= OUT_STR_LEN) goto  _WRITE_NULL_VAL;
		}

		if (entry->init_index)
			bc += snprintf (strbuf + bc, OUT_STR_LEN - bc, " <!--index=%d-->\n", idx);
		else
			bc += snprintf (strbuf + bc, OUT_STR_LEN - bc, "\n");

_WRITE_NULL_VAL:
		if (bc >= OUT_STR_LEN) {
			memset (strbuf, 0, sizeof (strbuf));
			bc = pushchar (strbuf, TYPE_SETTING_CHAR, depth);
			snprintf (strbuf + bc, OUT_STR_LEN - bc, "<%s %s=\"%s\" %s=\"\"/>\n",
				ENTRY_KEYWORD, NAME_KEYWORD, entry->matchname, VALUE_KEYWORD);
			fprintf (stderr, "[ERR] Write to xmlfile buffer overflowed, "
				"\"%s\" value will be null.\n", entry->matchname);
		}

		if (fp) fprintf (fp, "%s", strbuf);

		if (status == XML_REACH_MAX) {
			status = XML_FAIL;
			break;
		}
		++idx;
	} while (entry->init_index != NULL);

	if (entry->sibling != NULL) {
		genConfigEntry (xml_type, entry->sibling, depth);
	}
	return status;
}

XML_OPR_t
genConfig(CONFIG_DATA_T xml_type, dir_t *dir, int depth)
{
	if (dir == NULL) {
		printf ("[ERR] dirctory is null\n");
		return -1;
	}
	XML_DBG ("%s(): %d_%s %d/%d/%d empty=[%d]\n",__FUNCTION__, depth, dir->matchname,
		dir->index?(*(dir->index)):0, dir->final_idx, dir->index_max, dir->empty_chain);

	bool hs = strcmp ("HW_MIB_TABLE", dir->matchname);
	while (1) {
		//Only for CS: Empty chain, show <Dir Name="..."> </Dir>
		if ((xml_type != HW_SETTING) && (dir->empty_chain)) {
			if (depth != 0) {
				memset (strbuf, 0, sizeof (strbuf));
				bc = pushchar (strbuf, TYPE_SETTING_CHAR, depth);
				bc += sprintf (strbuf + bc, "<%s %s=\"%s\">\n", DIR_KEYWORD, NAME_KEYWORD, dir->matchname);
				bc += pushchar (strbuf + bc, TYPE_SETTING_CHAR, depth);
				bc += sprintf (strbuf + bc, "</%s>\n", DIR_KEYWORD);
				if (fp) fprintf (fp, "%s", strbuf);
			}
		}

		// over max or no need to generate chain
		if (depth <= 1) {
			if (dir->index && (*(dir->index) >= dir->final_idx)) break;	//Level 1, refer final_idx
		} else {
			if (dir->index && (*(dir->index) >= dir->index_max)) break;	//Level 2 and above, refer index_max. Ex: VoiceService 
		}

		// get entry value
		if (!dir->matchname)
			return -1;

		// Show different configuration file
		if (xml_type == HW_SETTING) {	//Show Hardware setting , Ignore others DIR
			if ((depth == 1) && hs) {
				XML_DBG ("Show HS: Ignore dir_%s.\n", dir->matchname); break;
			}
		} else {	//Show Current setting, Ignore the HS_MIB_DIR
			if (!hs) {
				XML_DBG ("Show CS: Ignore dir_%s.\n", dir->matchname); break;
			}
		}

		if (depth != 0) {
			memset (strbuf, 0, sizeof (strbuf));
			bc = pushchar (strbuf, TYPE_SETTING_CHAR, depth);

			bc += sprintf (strbuf + bc, "<%s %s=\"%s\">",
	        		DIR_KEYWORD, NAME_KEYWORD, dir->matchname);

			if (dir->index)
				bc += sprintf (strbuf + bc, " <!--index=%d-->\n", *(dir->index));
			else
				bc += sprintf (strbuf + bc, "\n");
		} else {
			bc = sprintf (strbuf, "<%s %s=\"%s\">\n",
				ROOT_KEYWORD, NAME_KEYWORD, dir->matchname);
		}

		//printf("%s", strbuf);
		if (fp) fprintf (fp, "%s", strbuf);
		if (dir->firstEntry != NULL) {
			genConfigEntry (xml_type, dir->firstEntry, depth + 1);
		}

		if (dir->firstChild != NULL) {
			genConfig (xml_type, dir->firstChild, depth + 1);
		}

		if (depth != 0) {
			bc = pushchar (strbuf, TYPE_SETTING_CHAR, depth);
			bc += sprintf (strbuf + bc, "</%s>\n", DIR_KEYWORD);
		} else {
			bc = sprintf (strbuf, "</%s>\n", ROOT_KEYWORD);
		}
		if (fp) fprintf (fp, "%s", strbuf);
		//printf("</%s>\n", DIR_KEYWORD);
		if (dir->index == NULL) break; // single

		if (dir->index)	(*(dir->index))++;
	};

	//re-initialize
	if (dir->index != NULL)
		*(dir->index) = 0;

	if (dir->sibling != NULL) {
		genConfig (xml_type, dir->sibling, depth);
	}

	return XML_OK;
}

int gen_file(CONFIG_DATA_T xml_type, const char *filename)
{
	int idx = 0;
	if (filename != NULL) {
		fp = fopen (filename, "w+");
	} else {
		fp = fdopen (fileno (stdout), "w");
	}
	if (genConfig (xml_type, &root_dir, idx) != XML_OK) {
		fprintf (stderr, "ERR: file generated failed.\n");
	}
	fclose (fp);
	return 0;
}

/* Debug */
void pushTree(int n)
{
	switch(n){
		case 1:
            putchar('|');
            putchar('-');
            break;
        case 2:
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar('-');
            break;
        case 3:
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar('-');
            break;
         case 4:
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar(' ');
            putchar('|');
            putchar('-');
            break;
    }
}


//#define ENTRY_PRINT_TYPE "     "
int printEntry(entry_t *entry, int depth)
{
	if(entry==NULL){
        printf("[ERR] entry is null\n");
        return -1;
    }
    
    pushTree(depth);
    printf("ENTRY(%s),%s,%s,%s\n", entry->matchname, entry->name, 
        (entry->dir)?entry->dir->matchname:"NULL",
        (entry->sibling)?entry->sibling->matchname:"NULL");

    if(entry->sibling!=NULL){
		printEntry(entry->sibling, depth);
    } 
    return 0;
}


int printTree(dir_t *dir, int depth)
{
	if(dir==NULL){
		printf("[ERR] directory is null\n");
        return -1;
    }
    // root
    // |-sub1
    // |  |-sub1.1
    // |  |-sub1.2
    pushTree(depth);
    printf("DIR(%s)\n", dir->matchname);

    if(dir->firstEntry){
        printEntry(dir->firstEntry, depth+1);
    } 

    if(dir->firstChild){
        printTree(dir->firstChild, depth+1);
    }

    //pushchar(' ', depth);
    //printf("DIR(\"%s\")\n", dir->matchname);

    if(dir->sibling){
        printTree(dir->sibling, depth);
    }

    return 0;
}


