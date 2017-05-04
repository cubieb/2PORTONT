#ifndef CSV_PARSER_H
#define CSV_PARSER_H
#include <stdio.h>

#define CSV_MAX_ENTRY_ALLOW 256
#define CSV_MAX_ITEM_LEN    1024
#define CSV_MAX_LINE_LEN    8127
#define CSV_MAX_FILE_NUM    10

typedef struct {
    int max_entry;
    int num_entry;
    char ***data_array;
} content_t;

typedef struct {
    unsigned int key;
    const char *file_name;
    content_t content;
} csv_file_t;

typedef int (csv_item_comparator_t)(const char *a, const char *b);
typedef int (csv_line_comparator_t)(const char **a, const char **b);

// memory allocator
extern void* (*csv_malloc) (size_t);
extern void (*csv_free) (void *);


// related to csv parser
extern int csv_cf_init(csv_file_t *cf, const char *key, const char *fn, int max);
extern int csv_cf_replace_line_text(content_t *content, int line_no, const char *line_text);
extern int csv_cf_replace_line(content_t *content, int line_no, const char **line, int num_item);
extern void csv_cf_free_content(content_t *content);
extern void csv_cf_release(csv_file_t *cf);
extern int csv_cf_read_all(csv_file_t *cf);
extern int csv_cf_read_all_content(content_t *content, FILE *fh);
extern int csv_cf_write_all_content(content_t *content, FILE *fh);
extern int csv_cf_move_line_up(content_t *content, int from_line);

// related to csv manager
extern int csv_load_config(const char *conf_file_name);
extern void csv_done(void);
extern const char *csv_get_key_filename(unsigned int key);
extern csv_file_t *csv_lookup_csv_file(unsigned int key, int try_to_load);
#define csv_get_csv_file(key) (csv_lookup_csv_file(key, 1))
extern content_t *csv_get_content(csv_file_t *f);
extern int csv_write_back(csv_file_t *f);
extern int csv_remove_a_line(csv_file_t *f, int line_no);
extern int csv_replace_items(csv_file_t *f, int line_no, ...);
extern int csv_replace_a_line(csv_file_t *f, int line_no, const char **line, int num_item);
extern int csv_replace_a_line_arg(csv_file_t *f, int line_no, ...);
extern int csv_roll_in_a_line(csv_file_t *f, const char **item, int num_item);
extern int csv_roll_in_a_line_arg(csv_file_t *f, ...);
extern char **csv_search_line_by_item(csv_file_t *f, int column, const char *item, csv_item_comparator_t *cmp, int *result_line_no);
extern void csv_sort_line_by_item(csv_file_t *f, int column, csv_item_comparator_t *cmp);
extern void csv_sort_line_by_line(csv_file_t *f, csv_line_comparator_t *cmp);
extern void csv_api_call_log_get_total_num(int *total);
extern void csv_api_call_log_dump_by_line_num(int line_no);
#if 0
extern void csv_api_call_log_get_by_line_num(int line_no , char *status , char *caller , char *callee , 
	char *direction , unsigned int *start , unsigned int *end);
#else
extern void csv_api_call_log_get_by_line_num(int line_no , char *status , char *caller , char *callee , 
	char *direction , time_t *start , time_t *end);
#endif
extern void csv_api_call_log_remove_by_line_num(int line_no);

typedef enum CALL_LOG_END{
	CALL_LOG_END_SUCCESS = 0 
	, CALL_LOG_END_MISSED 
	, CALL_LOG_END_ABORTED 
}CALL_LOG_END;

#define CDBG_SWITCH
#ifdef  CDBG_SWITCH
#  define CDBG(fmt, args...)  printf("CDBG: (%04d)%s  " fmt , __LINE__ , __FUNCTION__ , ## args)
#else
#  define CDBG(fmt, args...) /* not debugging: nothing */
#endif


extern char *itoa(int n);
extern char *utoa(unsigned int n);

#endif //CSV_PARSER_H


