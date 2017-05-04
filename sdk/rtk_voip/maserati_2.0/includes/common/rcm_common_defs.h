#ifndef RCM_SIM_DEFS_H
#define RCM_SIM_DEFS_H

//#include "sysconfig.h"

#define rcm_sim_log( fmt, args... )	{}

#define SETSOCKOPT(optid, varptr, vartype, qty) \
        { \
                int     sockfd; \
                if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) { \
                        return -1; \
                } \
                if (setsockopt(sockfd, IPPROTO_IP, optid, (void *)varptr, sizeof(vartype)*qty) != 0) { \
                        close(sockfd); \
                        return -2; \
                } \
                close(sockfd); \
        }

/* for debug usage */
char                 flash_filename[4096];
char                 log_filename[4096];
char                 case_name[512];

#ifdef RCM_DEBUG
#define rcm_dp(fmt, args...) fprintf(stderr, "[%ld:%03ld] <%s:%d>\t" fmt, log_tick/600, (log_tick)%600, __FILE__, __LINE__ , ## args)
#else
#define rcm_dp(fmt, args...)
#endif
#ifdef VOIP_FLASH_ARCH	//Alex, original architecture
#define rcm_voip_flash_client_init(a,b) voip_flash_client_init(a,b)
#else
#define rcm_voip_flash_client_init(a,b) voip_flash_get(a)
#endif
#define rcm_init_voip_interface() rcm_init_linphone_voip_interface()
#define rcm_set_exosip_listen_port(a,b,c,d,e)	rcm_set_linphone_exosip_listen_port(a,b,c,d,e)
#define rcm_set_conctrol_fifo() rcm_set_linphone_conctrol_fifo()
#define rcm_set_pidfile() rcm_set_linphone_pidfile()


#endif//RCM_SIM_DEFS_H

