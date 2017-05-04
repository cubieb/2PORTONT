#ifndef __MIB_RESERVE_H__
#define __MIB_RESERVE_H__

#ifndef NULL
#define NULL		0
#endif 

//ql add
int mib_chain_record_retrive(int id);
int mib_table_record_retrive(int id);
void mib_record_reserve(CONFIG_MIB_T type);
int mib__record_clear(CONFIG_MIB_T type);

#endif
