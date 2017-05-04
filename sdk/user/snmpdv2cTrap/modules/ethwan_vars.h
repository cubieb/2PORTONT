#ifndef _ETHWAN_H_
#define _ETHWAN_H_

#include "../../boa/src/LINUX/options.h"

#ifndef AUG_SNMP_DBG
#define AUG_SNMP_DBG
#ifdef AUG_SNMP_DBG
#define AUG_SNMP_PRT(fmt,args...)  printf("\033[1;33;46m<%s %d %s> \033[m"fmt, __FILE__, __LINE__, __func__, ##args)
#endif
#endif

CVoidType	ethwanInit(void);

#endif
