#include "mib_tool.h"

#include	<stdio.h>

unsigned int create_mib_tbl(struct mib_oid_tbl *tbl, unsigned int total, unsigned int max_oid_name)
{
	tbl->oid = (struct mib_oid *) calloc(total, sizeof(struct mib_oid));

	if(tbl->oid != 0)
	{
		tbl->total = total;
		tbl->oid_name_pool = (unsigned char*) malloc(total * max_oid_name);

		if(tbl->oid_name_pool != 0)
		{
			unsigned int idx;
			unsigned char *ptr = tbl->oid_name_pool;
			struct mib_oid * oid_ptr = tbl->oid;
			for(idx=0;idx<tbl->total;idx++, oid_ptr++)
			{
				oid_ptr->name = ptr;
				ptr += max_oid_name;
			}
		}
		else
		{
			free(tbl->oid);
			tbl->total = 0;		
		}	
	}
	else
		tbl->total = 0;		

//	printf("create_mib_tbl: tbl:%x, tbl->oid:%x, tbl->oid_name_pool:%x\n",(unsigned int)tbl ,(unsigned int)tbl->oid, (unsigned int)tbl->oid_name_pool);

	return tbl->total;
}

void free_mib_tbl(struct mib_oid_tbl *tbl)
{
#if 0
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;
	
	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		printf("free_mib_tbl(%d) oid(%x) name(%x)\n", idx,(unsigned int)oid_ptr ,(unsigned int)oid_ptr->name);
	}
#endif

//	printf("free_mib_tbl: tbl:%x, tbl->oid:%x, tbl->oid_name_pool:%x\n",(unsigned int)tbl ,(unsigned int)tbl->oid, (unsigned int)tbl->oid_name_pool);
	if(tbl->oid)
		free(tbl->oid);
	if(tbl->oid_name_pool)
		free(tbl->oid_name_pool);

	tbl->oid = 0;
	tbl->total = 0;	
	tbl->oid_name_pool = 0;	
}

void print_mib_tbl(struct mib_oid_tbl *tbl)
{
	unsigned int idx, item;
	struct mib_oid * oid_ptr = tbl->oid;
	unsigned char buf[256];

	printf("print_mib_tbl: tbl:%x total:%u\n",(unsigned int)tbl ,tbl->total);

	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		memset(buf,0x00,256);
		if((oid_ptr->name) && (oid_ptr->length))
		{
			for(item=0;item<oid_ptr->length;item++)
			{
				unsigned char temp[8];			
				sprintf(temp,"%d-",oid_ptr->name[item]);
				strcat(buf, temp);
			}

			printf("print_mib_tbl(%d) oid(%s)\n", idx, buf);
		}
	}
}

/** lexicographical compare two object identifiers.
 * 
 * Caution: this method is called often by
 *          command responder applications (ie, agent).
 *
 * @return -1 if name1 < name2, 0 if name1 = name2, 1 if name1 > name2
 */
int
snmp_oid_compare(const unsigned char * in_name1,
                 unsigned int len1, const unsigned char * in_name2, unsigned int len2)
{
    register int    len;
    register const unsigned char *name1 = in_name1;
    register const unsigned char *name2 = in_name2;

    /*
     * len = minimum of len1 and len2 
     */
    if (len1 < len2)
        len = len1;
    else
        len = len2;
    /*
     * find first non-matching OID 
     */
    while (len-- > 0) {
        /*
         * these must be done in seperate comparisons, since
         * subtracting them and using that result has problems with
         * subids > 2^31. 
         */
        if (*(name1) != *(name2)) {
            if (*(name1) < *(name2))
                return -1;
            return 1;
        }
        name1++;
        name2++;
    }
    /*
     * both OIDs equal up to length of shorter OID 
     */
    if (len1 < len2)
        return -1;
    if (len2 < len1)
        return 1;
    return 0;
}

int snmp_oid_getnext(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result)
{
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;

	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		if(snmp_oid_compare(oid_ptr->name, oid_ptr->length, in_name, len) == 1)
		{
			*result = idx;
			return 1;
		}
	}
	return 0;
}

int snmp_oid_get(struct mib_oid_tbl *tbl, const unsigned char * in_name, unsigned int len, unsigned int *result)
{
	int i;
	unsigned int idx;
	struct mib_oid * oid_ptr = tbl->oid;

	for(idx=0;idx<tbl->total;idx++, oid_ptr++)
	{
		i = snmp_oid_compare(oid_ptr->name, oid_ptr->length, in_name, len);
		if(i == 0)
		{
			*result = idx;
			return 1;
		}

		if(i == 1)
		{
			return 0;
		}
		
	}
	return 0;
}

