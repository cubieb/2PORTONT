#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include "csv_parser.h"

void* (*csv_malloc) (size_t )=(void*)malloc;
void (*csv_free) (void *)=(void*)free;

#define CSV_MALLOC  (*csv_malloc)
#define CSV_FREE    (*csv_free)


static char *
csv_strdup(const char *src) {
    int n=strlen(src);
    char *ret=CSV_MALLOC(n+1);
    strcpy(ret, src);
    return ret;
}

int
csv_cf_init(csv_file_t *cf, const char *key, const char *fn, int max) {
    bzero(cf, sizeof(csv_file_t));
    cf->key=strtoul(key, NULL, 0);
    cf->file_name=fn;
    content_t *content=&(cf->content);
    content->max_entry=max;
    
    int sz=sizeof(char**)*(max+1);
    char ***data_array;
    if ((data_array=(char***)CSV_MALLOC(sz))==NULL) return -1;
    bzero((content->data_array=data_array), sz);
    //data_array[max]=NULL;
    return 0;
}
static void
csv_cf_free_line_array(char **line) {
    if (line==NULL) return ;
    char **entry=line;
    while (*entry!=NULL) {
        CSV_FREE(*(entry++));
    }
    CSV_FREE(line);
}
static int
csv_cf_free_line(content_t *content, int line_no) {
    if (line_no>=content->max_entry) return -1;
    char ***line=&(content->data_array[line_no]);
    csv_cf_free_line_array(*line);
    *line=NULL;
    return 0;
}
static const char *
cvs_cf_copy_item_before_separator(char *dst, int *m, const char *src) {
    int n=0;
    int in_doublequote=0;
    if (*src=='"') {
        in_doublequote=1;
        ++src;
    }
    while (1) {
        char chr=*(src++);
        if (chr=='\0') {
            break;
        } else if (chr=='"') {
            if (in_doublequote) {
                if (*src=='"') {
                    ++src;
                } else {
                    in_doublequote=0;
                    continue;
                }
            }
        } else if (chr==',') {
            if (in_doublequote==0) break;
        }
        if (dst!=NULL) *(dst++)=chr;
        ++n;
    }
    if (dst!=NULL) *dst='\0';
    if (m!=NULL) *m=n;
    return src-1;
}
static int
csv_cf_parser_line(char **line_dst, const char *line_text) {
    char _tmp_item[CSV_MAX_ITEM_LEN+1], *item;
    const char *s=line_text;
    int i=0, l;

    while (i<CSV_MAX_ENTRY_ALLOW) {
        s=cvs_cf_copy_item_before_separator(_tmp_item, &l, s);
        item=CSV_MALLOC(l+1);
        strcpy(item, _tmp_item);
        line_dst[i++]=item;
        if (*(s++)=='\0') break;
    }
    return i;
}
static int 
csv_cf_assign_line(content_t *content, int line_no, const char **line, int num_item) {
    int sz=sizeof(char*)*(num_item+1);
    char **new_line=CSV_MALLOC(sz);
    bzero(new_line, sz);

    int c, i;
    for (c=0, i=0;c<num_item;++c,++i) {
        const char *item=line[c];
        if (item==NULL) continue;
        int len=strlen(item);
        char *dst=CSV_MALLOC(len+1);
        strcpy(dst, item);
        new_line[i]=dst;
    }

    content->data_array[line_no]=new_line;
    return 0;
}
static int 
csv_cf_assign_line_text(content_t *content, int line_no, const char *line_text) {
    if (line_text==NULL) return -1;

    char *_tmp[CSV_MAX_ENTRY_ALLOW+1];
    int n=csv_cf_parser_line(_tmp, line_text);
    return csv_cf_assign_line(content, line_no, (const char **)_tmp, n);
}
int 
csv_cf_replace_line_text(content_t *content, int line_no, const char *line_text) {
    if (line_no>=content->num_entry) return -1;
    char **replaced_line=content->data_array[line_no];
    content->data_array[line_no]=NULL;
    int ret=csv_cf_assign_line_text(content, line_no, line_text);
    csv_cf_free_line_array(replaced_line);
    return ret;
}
int 
csv_cf_replace_line(content_t *content, int line_no, const char **line, int num_item) {
    if (line_no>=content->num_entry) return -1;
    char **replaced_line=content->data_array[line_no];
    content->data_array[line_no]=NULL;
    int ret=csv_cf_assign_line(content, line_no, line, num_item);
    csv_cf_free_line_array(replaced_line);
    return ret;
}

static void
_csv_cf_free_data_array(content_t *content) {
    int max_entry=content->max_entry;
    char ***data_array=content->data_array;
    int l;
    for (l=0; l<max_entry; ++l, ++data_array) {
        csv_cf_free_line_array(*data_array);
        *data_array=NULL;
    }
}

void
csv_cf_free_content(content_t *content) {
    _csv_cf_free_data_array(content);
    CSV_FREE(content->data_array);
    content->data_array=NULL;
    content->num_entry=0;
    content->max_entry=0;
}
void
csv_cf_release(csv_file_t *cf) {
    csv_cf_free_content(&cf->content);
    cf->file_name=NULL;
    cf->key=0;
}
int 
csv_cf_read_all_content(content_t *content, FILE *fh) {
    char line_buf[CSV_MAX_LINE_LEN+1];
    int ln=0;
    _csv_cf_free_data_array(content);
    while (fgets(line_buf, sizeof(line_buf), fh)!=NULL) {
        int sz=strlen(line_buf);
        if (line_buf[sz-1]==0x0a) {
            line_buf[sz-1]='\0';
            if (line_buf[sz-2]==0x0d) {
                line_buf[sz-2]='\0';
            }
        }
        csv_cf_assign_line_text(content, ln++, line_buf);
        if (ln>=content->max_entry) break;
    }
    content->num_entry=ln;
    return 0;
}

int 
csv_cf_read_all(csv_file_t *cf) {
    FILE *fh=(cf->file_name!=NULL)?fopen(cf->file_name, "r"):stdin;
    if (fh==NULL) return -1;
    int ret=csv_cf_read_all_content(&cf->content, fh);
    fclose(fh);
    return ret;
}
static void
csv_cf_write_item(FILE *fh, const char *item) {
    const char *p=item;
    char chr;
    
    int to_use_doublequote=(*item=='"');
    while ((to_use_doublequote==0)&&((chr=*(p++))!='\0')) {
        if ((chr==',') || (chr=='\n') || (chr=='\a')) {
            to_use_doublequote=1;
        }
    }
    
    if (to_use_doublequote) {
        fputc('"', fh);
        p=item;
        while ((chr=*(p++))!='\0') {
            if (chr=='"') fputc('"', fh);
            fputc(chr, fh);
        }
        fputc('"', fh);
    } else {
        fputs(item, fh);
    }
}
int
csv_cf_write_all_content(content_t *content, FILE *fh) {
    char **line, *item;
    int ln, n=content->num_entry;
    for (ln=0;ln<n;++ln) {
        if ((line=content->data_array[ln])==NULL) break;
        item=*(line++);
        if (item!=NULL) {
            while (1) {
                csv_cf_write_item(fh, item);
                if ((item=*(line++))==NULL) break;
                fputc(',', fh);
            }
        }
        fputs("\r\n", fh);
    }
    return 0;
}
int
csv_cf_move_line_up(content_t *content, int from_line) {
    int num=content->num_entry;
    if (from_line>=num) return -1;
    
    // move up by a line
    csv_cf_free_line(content, from_line);
    int c;
    for (c=from_line+1;c<num;++c) {
        content->data_array[c-1]=content->data_array[c];
    }
    content->data_array[--num]=NULL;
    content->num_entry=num;
    
    return 0;
}

// --------------------------------------------------------------------------
//         CSV Configuration Manager: Basic
// --------------------------------------------------------------------------
static csv_file_t conf_csv;
static csv_file_t csv_files[CSV_MAX_FILE_NUM];
static int csv_file_num=0;
static FILE *csv_conf_file_handle=NULL;
int
csv_load_config(const char *conf_file_name) {
    csv_file_num=0;

    int ret;
    if ((ret=csv_cf_init(&conf_csv, "0x7fffffff", NULL, CSV_MAX_FILE_NUM))!=0) return ret;
    FILE *conf_fh=fopen(conf_file_name, "r");
    if (conf_fh==NULL) return -1;
	
    int conf_fd=fileno(conf_fh);
    flock(conf_fd, LOCK_EX);
    
    if ((ret=csv_cf_read_all_content(&conf_csv.content, conf_fh))!=0) {
        fclose(conf_fh);
        return ret;
    }

    int n=conf_csv.content.num_entry;
	
    n=(n<CSV_MAX_FILE_NUM)?n:CSV_MAX_FILE_NUM;
    char ***fdesc=conf_csv.content.data_array;
    csv_file_t *file=csv_files;
    int c=0;

    while (c<n) {
        char **file_desc=*(fdesc++);
        if (file_desc==NULL) break;
        
        if (*file_desc==NULL) continue;
        const char *key=*(file_desc++);
        if (*file_desc==NULL) continue;
        const char *fn=*(file_desc++);
        if (*file_desc==NULL) continue;
        const char *max=*file_desc;
        int max_entry=strtoul(max, NULL, 0);
        
        if (csv_cf_init(file, key, fn, max_entry)!=0) continue;
        ++file;
        ++c;
    }
    csv_file_num=c;
    csv_conf_file_handle=conf_fh;

	return 0;
}

void
csv_done(void) {
    int c, n=conf_csv.content.num_entry;
    for (c=0; c<n; ++c) {
        csv_cf_release(csv_files+c);
    }
    csv_file_num=0;
    
    csv_cf_release(&conf_csv);
    
    if (csv_conf_file_handle!=NULL) {
        fclose(csv_conf_file_handle);
    }
    csv_conf_file_handle=NULL;
}

csv_file_t *
csv_lookup_csv_file(unsigned int key, int try_to_load) {
    int c, n=csv_file_num;
    csv_file_t *file=csv_files;
    for (c=0; c<n; ++c, ++file) {
        if (file->key==key) {
            if (try_to_load && (file->content.num_entry==0)) csv_cf_read_all(file);
            return file;
        }
    }    
    return NULL;
}

content_t *
csv_get_content(csv_file_t *f) {
    if (f->content.num_entry==0) csv_cf_read_all(f);
    return &f->content;
}

int 
csv_write_back(csv_file_t *f) {
    FILE *fh=(f->file_name!=NULL)?fopen(f->file_name, "w"):stdout;
    if (fh==NULL) return -1;
    content_t *content=&f->content;
    int ret=csv_cf_write_all_content(content, fh);
    fclose(fh);
    return ret;
}

int 
csv_roll_in_a_line(csv_file_t *f, const char **line, int num_item){
    content_t *content=&f->content;
    if (content->num_entry>=content->max_entry) csv_cf_move_line_up(content, 0);
    return csv_cf_assign_line(content, content->num_entry++, line, num_item);
}
int
csv_remove_a_line(csv_file_t *f, int line_no) {
    content_t *content=&f->content;
    return csv_cf_move_line_up(content, line_no);
}
int 
csv_replace_items(csv_file_t *f, int line_no, ...) {
    content_t *content=&f->content;
    if (line_no>=content->max_entry) return -1;
    
    char **line =content->data_array[line_no];
    
    // count number of item
    int n=0;
    while (line[n]!=NULL) ++n;
    
    va_list ap;
    va_start(ap, line_no);
    while (1) {
        int col=va_arg(ap, int);
        if ((col<0)||(col>n)) break;
        const char *new_item=va_arg(ap, const char *);
        if (new_item==NULL) break;
        
        char *cur_item=line[col];
        line[col]=csv_strdup(new_item);
        CSV_FREE(cur_item);
    }
    va_end(ap);
    return 0;
}
int 
csv_replace_a_line(csv_file_t *f, int line_no, const char **line, int num_item) {
    content_t *content=&f->content;
    return csv_cf_replace_line(content, line_no, line, num_item);
}

#define PARSER_ARG_TO_ARRAY(__last_arg, __array, __num) ({\
    const char *__p;\
    const char **__q=__array;\
    va_list ap;\
    va_start(ap, __last_arg);\
    __num=0;\
    while ((__p=va_arg(ap, const char *))!=NULL) {\
        *(__q++)=__p;\
        if ((++__num)>=CSV_MAX_ENTRY_ALLOW) break;\
    }\
    *__q=NULL;\
    va_end(ap);\
})
int
csv_roll_in_a_line_arg(csv_file_t *f, ...) {   
    const char *_tmp[CSV_MAX_ENTRY_ALLOW+1];
    int n;
    PARSER_ARG_TO_ARRAY(f, _tmp, n);
    return csv_roll_in_a_line(f, _tmp, n);
}
int
csv_replace_a_line_arg(csv_file_t *f, int line_no, ...) {   
    const char *_tmp[CSV_MAX_ENTRY_ALLOW+1];
    int n;
    PARSER_ARG_TO_ARRAY(line_no, _tmp, n);
    return csv_replace_a_line(f, line_no, _tmp, n);
}
// --------------------------------------------------------------------------
//         CSV Configuration Manager: Sort and serach
// --------------------------------------------------------------------------

static csv_item_comparator_t *_item_comparator_in_use=NULL;
static csv_line_comparator_t *_line_comparator_in_use=NULL;
static int _column_in_use=0;
const char *
csv_get_key_filename(unsigned int key) {
    csv_file_t *f=csv_lookup_csv_file(key, 0);
    if (f==NULL) return NULL;
    return f->file_name;
}
const char *
csv_get_filename(csv_file_t *f) {
    return f->file_name;
}

char **
csv_search_line_by_item(csv_file_t *f, int column, const char *item, csv_item_comparator_t *cmp, int *result_line_no) {
    content_t *content=&f->content;
    int c, n=content->num_entry;
    char ***line=content->data_array;
    for (c=0;c<n;++c, ++line) {
        char **this_line=*line;
        if ((*cmp)(item, this_line[column])==0) {
            if (result_line_no!=NULL) *result_line_no=c;
            return this_line;
        }
    }
    if (result_line_no!=NULL) *result_line_no=-1;
    return NULL;
}

static int
_csv_comp_line(const void *a, const void *b) {
    const char **a_line=*(const char ***)a;
    const char **b_line=*(const char ***)b;    
    return (*_line_comparator_in_use)(a_line, b_line);
}
void
csv_sort_line_by_line(csv_file_t *f, csv_line_comparator_t *cmp) {
    content_t *content=&f->content;

    _line_comparator_in_use=cmp;
    qsort(content->data_array, content->num_entry, sizeof(char**), _csv_comp_line);
}

static int
_csv_comp_item(const char **a_line, const char **b_line) {
    const char *a_item=a_line[_column_in_use];
    const char *b_item=b_line[_column_in_use];
    return (*_item_comparator_in_use)(a_item, b_item);
}

void
csv_sort_line_by_item(csv_file_t *f, int column, csv_item_comparator_t *cmp) {
    _item_comparator_in_use=cmp;
    _column_in_use=column;
    csv_sort_line_by_line(f, _csv_comp_item);    
}


