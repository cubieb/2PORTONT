#include "xml_config_define.h"

#define XML_DIR_LIST_END() \
    static entry_t *dir_list_end_p \
    	__attribute__((aligned(4), unused, __section__(".dir_list")))

#define XML_ENTRY_LIST_END() \
    static entry_t *entry_list_end_p \
    	__attribute__((aligned(4), unused, __section__(".entry_list")))

/* End of XML ENTRY (Do not modify here)*/
XML_ENTRY_LIST_END();
/* End of XML DIR (Do not modify here)*/
XML_DIR_LIST_END();


void *get_last_dir_ptr(void){
	return &dir_list_end_p;
}

void *get_last_entry_ptr(void){
	return &entry_list_end_p;
}
