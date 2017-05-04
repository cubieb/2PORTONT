#ifndef _SUBR_NFBI_API_H_
#define _SUBR_NFBI_API_H_
#include "user/rtl867x_nfbi/ucd/uc_mib.h"
#include "user/rtl867x_nfbi/ucd/uc_udp.h"

extern char adsl_slv_drv_get(unsigned int id, void *rValue, unsigned int len);
#ifdef CONFIG_VDSL
char dsl_slv_msg_set_array(int msg, int *pval);
char dsl_slv_msg_set(int msg, int val);
char dsl_slv_msg_get_array(int msg, int *pval);
char dsl_slv_msg_get(int msg, int *pval);
#endif /*CONFIG_VDSL*/

extern int mib_slv_get(int id, void *value);
extern int mib_slv_set(int id, void *value);
extern int sys_slv_init( char *cmd );
extern int mib_slv_sync_dsl(void);
extern int mib_slv_sync_all(void);
#endif /*_SUBR_NFBI_API_H_*/


