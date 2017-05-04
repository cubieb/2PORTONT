#ifndef OPENWRT_BUILD
#include "../../../config/autoconf.h"
#else
#endif
#undef CONFIG_RTK_VOIP_IVR
//#ifndef __RCM_CUSTOMIZE_H
#define __RCM_CUSTOMIZE_H

//#undef CONFIG_RTK_VOIP_G7231
#undef CONFIG_RTK_VOIP_RTK_VOIP_SPEEX_NB
//#undef CONFIG_RTK_VOIP_G726
#undef CONFIG_RTK_VOIP_GSMFR
#undef CONFIG_RTK_VOIP_ILBC
//#undef CONFIG_RTK_VOIP_G722


//support RUSSIAN tone
#define CUSTOMER_COUNTRY_RUSSIAN
//#define CUSTOMER_COUNTRY_GUATEMALA
//#define CUSTOMER_COUNTRY_SPAIN
/*
#if defined (CUSTOMER_COUNTRY_RUSSIAN)
	#if defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW)
		#define COUNTRY_EXT1 "‰øÑÁ???
	#elif defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN)
		#define COUNTRY_EXT1 "‰øÑÁ???
	#else
		#define COUNTRY_EXT1 "RUSSIAN"
	#endif
#elif defined (CUSTOMER_COUNTRY_GUATEMALA)
	#if defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW)
		#define COUNTRY_EXT1 "?úÂú∞È¶¨Ê?"
	#elif defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN)
		#define COUNTRY_EXT1 "?úÂú∞È©¨Ê?"
	#else
		#define COUNTRY_EXT1 "GUATEMALA"
	#endif

#elif defined (CUSTOMER_COUNTRY_SPAIN)
        #if defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW)
                #define COUNTRY_EXT1 "Ë•øÁè≠??
        #elif defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN)
                #define COUNTRY_EXT1 "Ë•øÁè≠??
        #else
                #define COUNTRY_EXT1 "SPAIN"
        #endif

#else
	#if defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW)
		#define COUNTRY_EXT1 "È°çÂ??ãÂÆ∂"
	#elif defined (CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN)
		#define COUNTRY_EXT1 "È¢ùÂ??ΩÂÆ∂"
	#else
		#define COUNTRY_EXT1 "EXT1"
	#endif
#endif
*/
#define COUNTRY_EXT1 "RUSSIAN"

#if defined (CONFIG_E8B)
#define MAX_DIALPLAN_LENGTH		(4096+2)
#define NOANSWER_TIMER 60
#undef CONFIG_RTK_VOIP_G726
#undef CONFIG_RTK_VOIP_G7231
#define OFFHOOKWARNING 1
#define FAX_G711_CODEC 0
#define FLASH_HOOKTIME 500
#define FLASH_HOOKTIME_MIN 90
#define INTER_DIGIT_TIMER_LONG 20
#define M_DEFAULT_PRECEDENACE_LIST 	\
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#else
#define MAX_DIALPLAN_LENGTH		1024
#define NOANSWER_TIMER 0
#define OFFHOOKWARNING 0
#define FAX_G711_CODEC 1
#define FLASH_HOOKTIME 500
#define FLASH_HOOKTIME_MIN 80
#define INTER_DIGIT_TIMER_LONG 16
#define M_DEFAULT_PRECEDENACE_LIST 	\
	0, 1, 2, 8, 4, 5, 6, 7, 3, 9, 10, 11, 12, 13
#endif

//#endif
