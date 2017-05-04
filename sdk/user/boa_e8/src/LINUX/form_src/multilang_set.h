/* This file is for language sets define. */
#ifndef MULTILANG_SET_H
#define MULTILANG_SET_H

#include "../options.h"

typedef enum langlist
{
#ifdef CONFIG_USER_BOA_WITH_MULTILANG_EN
	LANG_EN,
#define LANG_STR_EN "English"
#define LANG_TYPE_EN "en"
#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW
//	LANG_CHINESE_TW,
//#define LANG_STR_CHINESE_TW "繁體中文"
//#define LANG_TYPE_TW "tw"
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN
//	LANG_CHINESE_CN,
//#define LANG_STR_CHINESE_CN "簡体中文"
//#define LANG_TYPE_CN "cn"
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_XX_XX
//	LANG_XX_XX,
//#define LANG_STR_XX_XX "XXXX"
//#define LANG_TYPE_XX "xx"
//#endif
	LANG_MAX
} LANG_LIST;


//typedef struct langset {
//	char *langType;
//	char *langStr;
//} LANG_SET;
//static LANG_SET lang_set[] = {
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_EN
//	{LANG_TYPE_EN, LANG_STR_EN},
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW
//	{LANG_TYPE_TW, LANG_STR_CHINESE_TW},
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN
//	{LANG_TYPE_CN, LANG_STR_CHINESE_CN},
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_XX_XX
//	{LANG_TYPE_XX, LANG_STR_XX_XX},
//#endif
//};


//#if MULTI_LANG_DL == 0
#ifdef CONFIG_USER_BOA_WITH_MULTILANG_EN
extern const char *strtbl_en[];
#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW
//extern const char *strtbl_tw[];
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN
//extern const char *strtbl_cn[];
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_XX_XX
//extern const char *strtbl_xx[];
//#endif

static const char **strtbl_name[] = {
#ifdef CONFIG_USER_BOA_WITH_MULTILANG_EN
	strtbl_en,
#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_TW
//	strtbl_tw,
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_ZH_CN
//	strtbl_cn
//#endif
//#ifdef CONFIG_USER_BOA_WITH_MULTILANG_XX_XX
	//strtbl_xx
//#endif
};
//#endif

#endif
