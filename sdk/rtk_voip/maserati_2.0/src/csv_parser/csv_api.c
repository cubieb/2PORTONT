#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "csv_parser.h"


#define CSV_CONF_FILE       "/etc/config.csv"
#define CSV_KEY_CONTACT               0x0001
#define CSV_KEY_CALL_LOG              0x0002
#define CSV_KEY_WAKEUP_CALL           0x0003
#define CSV_KEY_RINGING_SCHEDULE      0x0004

#define MAX_FILENAME_SZ     127

// in call log  

//#define CSV_CALL_NUMBER_ONLY_CALLER 1

#if CSV_CALL_NUMBER_ONLY_CALLER

#define CSV_COL_SEQ         0
#define CSV_COL_IS_ACTIVE   1
#define CSV_COL_NUMBER      2
#define CSV_COL_CALL_TYPE   3
#define CSV_COL_START_TIME  4
#define CSV_COL_END_TIME    5

#else /* incoming and outgoing */

#define CSV_COL_SEQ                  0
#define CSV_COL_IS_ACTIVE            1
#define CSV_COL_NUMBER_CALLER        2
#define CSV_COL_NUMBER_CALLEE        3
#define CSV_COL_CALL_TYPE            4
#define CSV_COL_START_TIME           5
#define CSV_COL_END_TIME             6

#endif

// in contact
#define CSV_COL_NAME        0
#define CSV_CONTACT_TYPE    1
#define CSV_CONTACT_NUM     2

// in wakeup call
#define CSV_COL_WUC_NAME    	     0
#define CSV_COL_WUC_RINGTIME         1
#define CSV_COL_WUC_DAYS             2
#define CSV_COL_WUC_ASSIGN_FXS       3
#define CSV_COL_WUC_ONOFF            4

// in ringing schedule
#define CSV_COL_RS_FROM    0
#define CSV_COL_RS_TO      1

void
csv_api_init(void) {
    // when call manager starts up, all active of call log should be clean.
    
    // redefine the memory allocator if necessary
    csv_malloc=malloc;
    csv_free=free;
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    content_t *content=csv_get_content(f);
    int l, num_entry=content->num_entry;
    int any_change=0;
    for (l=0; l<num_entry; ++l) {
        char **line=content->data_array[l];
        if (strcmp(line[CSV_COL_IS_ACTIVE], "active")==0) {
            csv_replace_items(f, l, CSV_COL_IS_ACTIVE, "success", -1, NULL);
            any_change=1;
        }	
    }
    if (any_change) csv_write_back(f);
    csv_done();
}

/*
* Call Log
*/

void
#if CSV_CALL_NUMBER_ONLY_CALLER
csv_api_call_start(const char *seq, const char *phone_number, const char *call_type, unsigned int start_unix_time) {
#else
csv_api_call_start(const char *seq, const char *phone_number_caller , const char *phone_number_callee, const char *call_type, unsigned int start_unix_time) {
#endif
    // when call manager makes a call
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    char buf[10];
    //sprintf(buf, "%ul", start_unix_time);
    sprintf(buf, "%u", start_unix_time);
    
#if CSV_CALL_NUMBER_ONLY_CALLER    
    csv_roll_in_a_line_arg(f, 
        seq, "active", phone_number, call_type, buf, "", NULL);
#else
    csv_roll_in_a_line_arg(f, 
        seq, "active", phone_number_caller, phone_number_callee, call_type, buf, "", NULL);
#endif
    	
    csv_write_back(f);  	
    csv_done();
}

void
csv_api_call_end(const char *seq, unsigned int end_unix_time , CALL_LOG_END end_type) {
    // when call manager finish a call
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    int ln;
    char **line=csv_search_line_by_item(f, CSV_COL_SEQ, seq, strcmp, &ln);
    if (line!=NULL) {
        char buf[10];
        //sprintf(buf, "%ul", end_unix_time);
        sprintf(buf, "%u", end_unix_time);
        
        switch(end_type){
			case CALL_LOG_END_SUCCESS:
	        	csv_replace_items(f, ln, CSV_COL_IS_ACTIVE, "success", CSV_COL_END_TIME, buf, -1, NULL);
				break;
			case CALL_LOG_END_MISSED:
	        	csv_replace_items(f, ln, CSV_COL_IS_ACTIVE, "missed", CSV_COL_END_TIME, buf, -1, NULL);
				break;
			case CALL_LOG_END_ABORTED:
	        	csv_replace_items(f, ln, CSV_COL_IS_ACTIVE, "aborted", CSV_COL_END_TIME, buf, -1, NULL);
				break;
			default:
				CDBG("other case\n");
				break;
        }
		
        csv_write_back(f);
    } else {
        // error may occur: the call log could be removed by scenario5 before finishing the call
    }
    csv_done();
}

#define is_phone_number(chr) ({char __chr=chr; ((__chr>='0')&&(__chr<='9'))||(__chr=='#')||(__chr=='*');})
static int
phone_comp(const char *pn1, const char *pn2) {
    while (*pn1 && *pn2) {
        if (is_phone_number(*pn1)!=0) {++pn1; continue;}
        if (is_phone_number(*pn2)!=0) {++pn2; continue;}
        int ret=*pn1 - *pn2;
        if (ret!=0) return ret;
        ++pn1; 
        ++pn2;
    }
    return *pn1 - *pn2;
}

void
csv_api_call_log_dump(void) {
    // dump call log
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    content_t *content=csv_get_content(f);
    int row, num_row=content->num_entry;
    for (row=0;row<num_row;++row) {
        char **line=content->data_array[row];
        if (line==NULL) break;
        
        const char *active=line[CSV_COL_IS_ACTIVE];
        const char *type=line[CSV_COL_CALL_TYPE];
#if CSV_CALL_NUMBER_ONLY_CALLER        
        const char *number=line[CSV_COL_NUMBER];
#else
        const char *number_caller=line[CSV_COL_NUMBER_CALLER];
        const char *number_callee=line[CSV_COL_NUMBER_CALLEE];        
#endif        
        time_t start_time_unix=strtoul(line[CSV_COL_START_TIME], NULL, 0);
        time_t end_time_unix=(strcmp(active,"active")==0)?time(NULL):strtoul(line[CSV_COL_END_TIME], NULL, 0);
        time_t duration=end_time_unix-start_time_unix;
        char start_time[27];
        strcpy(start_time, ctime(&start_time_unix));
#if CSV_CALL_NUMBER_ONLY_CALLER        		
        char **contact=csv_search_line_by_item(g, CSV_CONTACT_NUM, number, phone_comp, NULL);
        const char *person=(contact!=NULL)?contact[CSV_COL_NAME]:number;
        printf("%12s %s %12s %6s %ul(sec)\n",
            type, start_time, person, active, (unsigned int)duration);
#else
        char **contact_caller=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_caller, phone_comp, NULL);
        char **contact_callee=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_callee, phone_comp, NULL);
        const char *person_caller=(contact_caller!=NULL)?contact_caller[CSV_COL_NAME]:number_caller;
        const char *person_callee=(contact_callee!=NULL)?contact_callee[CSV_COL_NAME]:number_callee;
		#if 0
        printf("%12s %s %12s %6s %ul(sec)\n",
            type, start_time, person_caller, active, (unsigned int)duration);
        printf("%12s %s %12s %6s %ul(sec)\n",
            type, start_time, person_callee, active, (unsigned int)duration);
		#else
        printf("%12s %s %12s %6s %u(sec)\n",
            type, start_time, person_caller, active, (unsigned int)duration);
        printf("%12s %s %12s %6s %u(sec)\n",
            type, start_time, person_callee, active, (unsigned int)duration);

		#endif
#endif
    }    
    csv_done();
}

void
csv_api_call_log_get_total_num(int *total) {

    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    content_t *content=csv_get_content(f);

	if (!total){
		printf("%s(%d) total=%p\n" , __FUNCTION__ , __LINE__ , total);
		goto done;		
	}
	
	*total=content->num_entry;

done:
    csv_done();
}


void
csv_api_call_log_dump_by_line_num(int line_no){
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    content_t *content=csv_get_content(f);
    int row, num_row=content->num_entry;

	row = line_no;

	if (row>=num_row){
		printf("%s(%d) line_no=%d too large\n" , __FUNCTION__ , __LINE__ , line_no);
		goto done;
	}
	else {
        char **line=content->data_array[row];
        if (line==NULL) 
			goto done;
        
        const char *active=line[CSV_COL_IS_ACTIVE];
        const char *type=line[CSV_COL_CALL_TYPE];
#if CSV_CALL_NUMBER_ONLY_CALLER        
        const char *number=line[CSV_COL_NUMBER];
#else
        const char *number_caller=line[CSV_COL_NUMBER_CALLER];
        const char *number_callee=line[CSV_COL_NUMBER_CALLEE];        
#endif        
        time_t start_time_unix=strtoul(line[CSV_COL_START_TIME], NULL, 0);
        time_t end_time_unix=(strcmp(active,"active")==0)?time(NULL):strtoul(line[CSV_COL_END_TIME], NULL, 0);
        time_t duration=end_time_unix-start_time_unix;
        char start_time[27];
        strcpy(start_time, ctime(&start_time_unix));
#if CSV_CALL_NUMBER_ONLY_CALLER        		
        char **contact=csv_search_line_by_item(g, CSV_CONTACT_NUM, number, phone_comp, NULL);
        const char *person=(contact!=NULL)?contact[CSV_COL_NAME]:number;
        printf("%12s %s %12s %6s %ul(sec)\n",
            type, start_time, person, active, (unsigned int)duration);
#else
        char **contact_caller=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_caller, phone_comp, NULL);
        char **contact_callee=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_callee, phone_comp, NULL);
        const char *person_caller=(contact_caller!=NULL)?contact_caller[CSV_COL_NAME]:number_caller;
        const char *person_callee=(contact_callee!=NULL)?contact_callee[CSV_COL_NAME]:number_callee;

		printf("%12s %s %12s %12s %6s %u(sec)\n", type, start_time , person_caller , person_callee , active, (unsigned int)duration);

#endif
    }    

done:
    csv_done();		
}


void
#if 0
csv_api_call_log_get_by_line_num(int line_no , char *status , char *caller , char *callee , 
	char *direction , unsigned int *start , unsigned int *end){
#else
csv_api_call_log_get_by_line_num(int line_no , char *status , char *caller , char *callee , 
	char *direction , time_t *start , time_t *end){
#endif
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    content_t *content=csv_get_content(f);
    int row, num_row=content->num_entry;

	row = line_no;

	if (row>=num_row){
		printf("%s(%d) line_no=%d too large\n" , __FUNCTION__ , __LINE__ , line_no);
		goto done;
	}
	else {
        char **line=content->data_array[row];
        if (line==NULL) 
			goto done;

		const char *seq=line[CSV_COL_SEQ];
        const char *active=line[CSV_COL_IS_ACTIVE];
        const char *type=line[CSV_COL_CALL_TYPE];
#if CSV_CALL_NUMBER_ONLY_CALLER        
        const char *number=line[CSV_COL_NUMBER];
#else
        const char *number_caller=line[CSV_COL_NUMBER_CALLER];
        const char *number_callee=line[CSV_COL_NUMBER_CALLEE];        
#endif        
        time_t start_time_unix=strtoul(line[CSV_COL_START_TIME], NULL, 0);
        time_t end_time_unix=(strcmp(active,"active")==0)?time(NULL):strtoul(line[CSV_COL_END_TIME], NULL, 0);
        time_t duration_unix=end_time_unix-start_time_unix;
        char start_time[27];
        strcpy(start_time, ctime(&start_time_unix));
#if CSV_CALL_NUMBER_ONLY_CALLER        		
        char **contact=csv_search_line_by_item(g, CSV_CONTACT_NUM, number, phone_comp, NULL);
        const char *person=(contact!=NULL)?contact[CSV_COL_NAME]:number;
        printf("%12s %s %12s %6s %ul(sec)\n",
            type, start_time, person, active, (unsigned int)duration_unix);
#else
        char **contact_caller=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_caller, phone_comp, NULL);
        char **contact_callee=csv_search_line_by_item(g, CSV_CONTACT_NUM, number_callee, phone_comp, NULL);
        const char *person_caller=(contact_caller!=NULL)?contact_caller[CSV_COL_NAME]:number_caller;
        const char *person_callee=(contact_callee!=NULL)?contact_callee[CSV_COL_NAME]:number_callee;

		strcpy(status , active);
		strcpy(caller , person_caller);
		strcpy(callee , person_callee);
		strcpy(direction , type);
		*start = start_time_unix;
		*end   = end_time_unix;		

	    //printf("%12s %s %12s %12s %6s %u(sec)\n", type, start_time , person_caller , person_callee , active, (unsigned int)duration_unix);
#endif
    }    

done:
    csv_done();		
}



void
csv_api_call_log_remove(const char *seq) {
    // remove call log
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);

    int ln;
    char **line=csv_search_line_by_item(f, CSV_COL_SEQ, seq, strcmp, &ln);
    if (line!=NULL) {
        csv_remove_a_line(f, ln);
        csv_write_back(f);
    }
    csv_done();
}


void
csv_api_call_log_remove_by_line_num(int line_no) {
    // remove call log
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_CALL_LOG);
    content_t *content=csv_get_content(f);
    int row = line_no;

	int ln = line_no;
    char **line=content->data_array[row];

	//printf("%s(%d) ln=%d\n" , __FUNCTION__ , __LINE__ , ln);

    if (line!=NULL) {
        csv_remove_a_line(f, ln);
        csv_write_back(f);
    }
    csv_done();
}

const char *
csv_api_file_get_call_log(void) {
    // get the filename of call log
    static char csv_filename[MAX_FILENAME_SZ+1];
    csv_filename[0]='\0';
    
    csv_load_config(CSV_CONF_FILE);
    const char *fn=csv_get_key_filename(CSV_KEY_CALL_LOG);
    if (fn!=NULL) strncpy(csv_filename, fn, MAX_FILENAME_SZ);
    csv_done();
    
    return csv_filename;
}

/*
* Contact
*/
void
csv_api_contact_add(const char *name, const char *type, const char *number) {
    // add a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    csv_roll_in_a_line_arg(g, name, type, number, NULL);
    csv_sort_line_by_item(g, CSV_COL_NAME, strcmp);
    csv_write_back(g);
    csv_done();
}

void
csv_api_contact_remove(int line_no) {
    // remove a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    csv_remove_a_line(g, line_no);
    csv_write_back(g);
    csv_done();    
}

void
csv_api_contact_edit(int line_no, const char *name, const char *type, const char *number) {
    // edit a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_CONTACT);
    csv_replace_a_line_arg(g, line_no, name, type, number, NULL);
    csv_write_back(g);
    csv_done();        
}

const char *
csv_api_file_get_contact(void) {
    // get the filename of contact
    static char csv_filename[MAX_FILENAME_SZ+1];
    csv_filename[0]='\0';
    
    csv_load_config(CSV_CONF_FILE);
    const char *fn=csv_get_key_filename(CSV_KEY_CONTACT);
    if (fn!=NULL) strncpy(csv_filename, fn, MAX_FILENAME_SZ);
    csv_done();
    
    return csv_filename;
}


/*
* wakeup call
*/

void
csv_api_wakeup_call_get_total_num(int *total) {

    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_WAKEUP_CALL);
    content_t *content=csv_get_content(f);

	if (!total){
		printf("%s(%d) total=%p\n" , __FUNCTION__ , __LINE__ , total);
		goto done;		
	}
	
	*total=content->num_entry;

done:
    csv_done();
}

void
csv_api_wakeup_call_get_by_line_num(int line_no, char *name, char *ring_time, char *days , char *assign_fxs , char *on_off){
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_WAKEUP_CALL);
    content_t *content=csv_get_content(f);
    int row, num_row=content->num_entry;

	row = line_no;

	if (row>=num_row){
		printf("%s(%d) line_no=%d too large\n" , __FUNCTION__ , __LINE__ , line_no);
		goto done;
	}
	else {
        char **line=content->data_array[row];
        if (line==NULL) 
			goto done;

        const char *csv_wakeup_call_name=line[CSV_COL_WUC_NAME];
        const char *csv_wakeup_call_ringtime=line[CSV_COL_WUC_RINGTIME];
        const char *csv_wakeup_call_days=line[CSV_COL_WUC_DAYS];
        const char *csv_wakeup_call_assign_fxs=line[CSV_COL_WUC_ASSIGN_FXS];
        const char *csv_wakeup_call_on_off=line[CSV_COL_WUC_ONOFF];

		strcpy(name , csv_wakeup_call_name);
		strcpy(ring_time , csv_wakeup_call_ringtime);
		strcpy(days , csv_wakeup_call_days);
		strcpy(assign_fxs , csv_wakeup_call_assign_fxs);
		strcpy(on_off, csv_wakeup_call_on_off);
    }

done:
    csv_done();		
}


void
csv_api_wakeup_call_add(const char *name, const char *ring_time, const char *days , const char *assign_fxs , const char *on_off) {
    // add a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_WAKEUP_CALL);
    csv_roll_in_a_line_arg(g, name, ring_time, days, assign_fxs, on_off, NULL);
    csv_write_back(g);
    csv_done();
}

void
csv_api_wakeup_call_remove_by_line_num(int line_no) {
    // remove a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_WAKEUP_CALL);
    csv_remove_a_line(g, line_no);
    csv_write_back(g);
    csv_done();    
}

void
csv_api_wakeup_call_edit(int line_no, const char *name, const char *ring_time, const char *days , const char *assign_fxs , const char *on_off) {
    // edit a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_WAKEUP_CALL);
    csv_replace_a_line_arg(g, line_no, name, ring_time, days, assign_fxs, on_off, NULL);
    csv_write_back(g);
    csv_done();        
}

const char *
csv_api_file_get_wakeup_call(void) {
    // get the filename of wakeup_call
    static char csv_filename[MAX_FILENAME_SZ+1];
    csv_filename[0]='\0';
    
    csv_load_config(CSV_CONF_FILE);
    const char *fn=csv_get_key_filename(CSV_KEY_WAKEUP_CALL);
    if (fn!=NULL) strncpy(csv_filename, fn, MAX_FILENAME_SZ);
    csv_done();
    
    return csv_filename;
}

/*
* ringing schedule
*/

void
csv_api_ringing_schedule_get_total_num(int *total) {

    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_RINGING_SCHEDULE);
    content_t *content=csv_get_content(f);

	if (!total){
		printf("%s(%d) total=%p\n" , __FUNCTION__ , __LINE__ , total);
		goto done;		
	}
	
	*total=content->num_entry;

done:
    csv_done();
}

void
csv_api_ringing_schedule_get_by_line_num(int line_no, char *from , char *to){
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *f=csv_get_csv_file(CSV_KEY_RINGING_SCHEDULE);
    content_t *content=csv_get_content(f);
    int row, num_row=content->num_entry;

	row = line_no;

	if (row>=num_row){
		printf("%s(%d) line_no=%d too large\n" , __FUNCTION__ , __LINE__ , line_no);
		goto done;
	}
	else {
        char **line=content->data_array[row];
        if (line==NULL) 
			goto done;

        const char *csv_ringing_schedule_from=line[CSV_COL_RS_FROM];
        const char *csv_ringing_schedule_to=line[CSV_COL_RS_TO];

        strcpy(from , csv_ringing_schedule_from);
        strcpy(to , csv_ringing_schedule_to);
    }

done:
    csv_done();		
}


void
csv_api_ringing_schedule_add(const char *from , const char *to) {
    // add a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_RINGING_SCHEDULE);
    csv_roll_in_a_line_arg(g, from, to, NULL);
    csv_write_back(g);
    csv_done();
}

void
csv_api_ringing_schedule_remove_by_line_num(int line_no) {
    // remove a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_RINGING_SCHEDULE);
    csv_remove_a_line(g, line_no);
    csv_write_back(g);
    csv_done();    
}

void
csv_api_ringing_schedule_edit(int line_no, const char *from , const char *to) {
    // edit a contact
    csv_load_config(CSV_CONF_FILE);
    csv_file_t *g=csv_get_csv_file(CSV_KEY_RINGING_SCHEDULE);
    csv_replace_a_line_arg(g, line_no, from, to, NULL);
    csv_write_back(g);
    csv_done();        
}

const char *
csv_api_file_get_ringing_schedule(void) {
    // get the filename of ringing_schedule
    static char csv_filename[MAX_FILENAME_SZ+1];
    csv_filename[0]='\0';
    
    csv_load_config(CSV_CONF_FILE);
    const char *fn=csv_get_key_filename(CSV_KEY_RINGING_SCHEDULE);
    if (fn!=NULL) strncpy(csv_filename, fn, MAX_FILENAME_SZ);
    csv_done();
    
    return csv_filename;
}


extern char *itoa(int);
#if !defined(__UC_LIBC__)
char *itoa(int n)
{
    static char buf[32];
    sprintf(buf, "%d", n);
    return(buf);
}

char *utoa(unsigned int n)
{
    static char buf[32];
    sprintf(buf, "%u", n);
    return(buf);
}
#endif

