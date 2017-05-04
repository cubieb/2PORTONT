#ifndef __RCM_VOIPLOG_H__
#define __RCM_VOIPLOG_H__

#include "linphonecore.h"
#include "rcm_voiplog_api.h"



#define SLIC_EVENT_TXT		"[SLIC]"
#define OSIP_EVENT_TXT		"[OSIP]"
#define CALL_MGNT_TXT		"[CM]"
#define VOIP_MISC_TXT		"[VOIP_MISC]"
#define VOIP_DEBUG_TEXT		"[DEBUG]"
#define DSP_EVENT_TXT		"[DSP]"


#define LOG_TEXT_MAXLEN		6000
#define VOIP_LOG_LEN 200

void rcm_LinphoneSyslog(const int category,LinphoneCore *lc,char *chfr,...);




#endif /* __RCM_VOIPLOG_H__ */

