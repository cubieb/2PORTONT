#ifndef __RCM_VOIPLOG_API_H__
#define __RCM_VOIPLOG_API_H__


#define LOG_SLIC_EVENT				(1<<0)
#define LOG_OSIP_EVENT				(1<<1)
#define LOG_CALL_MGNT				(1<<2)
#define LOG_VOIP_MISC				(1<<3)
#define LOG_VOIP_DEBUG				(1<<4)
#define LOG_DSP_EVENT				(1<<5)

void rcm_voiplog_write(const int category, char *message);
void rcm_voipSyslog(const int category,char *chfr, ...);

#endif /* __RCM_VOIPLOG_API_H__ */
