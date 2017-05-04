#include <stdio.h>
#include <stdlib.h>
#include "webform.h"
#include "multilang.h"
#include "multilang_set.h"

#if MULTI_LANG_DL == 1
void *dl_handle = NULL;
#endif

void langSel (request *wp, char *path, char *query)
{
	char *strVal;
	char *submitUrl;

	strVal = (char *)boaGetVar (wp, "selinit", "");
	if (strVal) {
		g_language_state = atoi (strVal);
		if (g_language_state != g_language_state_prev) {
			//printf ("Switch to lan_%d\n", g_language_state);
			g_language_state_prev = g_language_state;

#if MULTI_LANG_DL == 1
			if (dl_handle != NULL) dlclose (dl_handle);

			char *lib_name = (char *) malloc (sizeof(char) * 25);
			if (lib_name == NULL) {
				fprintf (stderr, "lib_name malloc failed!\n"); return;
			}
			char *strtbl_name = (char *) malloc (sizeof(char) * 15);
			if (strtbl_name == NULL) {
				fprintf (stderr, "strtbl_name malloc failed!\n"); return;
			}
			snprintf (lib_name, 25, "libmultilang_%s.so", lang_set[g_language_state].langType);
			snprintf (strtbl_name, 15, "strtbl_%s", lang_set[g_language_state].langType);

			printf("[%s:%d]\n",__func__,__LINE__);
			printf("lib_name=%s, strtbl_name=%s\n",lib_name,strtbl_name);
			dl_handle = dlopen (lib_name, RTLD_LAZY);
			printf("[%s:%d]\n",__func__,__LINE__);
			strtbl = (const char **) dlsym (dl_handle, strtbl_name);

			printf("[%s:%d]\n",__func__,__LINE__);
			free (lib_name);
			free (strtbl_name);
#else
			strtbl = strtbl_name[g_language_state];
#endif
			// Save mib to xmlconfig.
			if (!mib_set (MIB_MULTI_LINGUAL, (void *)lang_set[g_language_state].langType)) {
				ERR_MSG (strSetMultiLangError);
				return;
			}
			#ifdef COMMIT_IMMEDIATELY
			Commit ();
			#endif
		}
	}

//	submitUrl = (char *)boaGetVar(wp, "submit-url", "");
//	if (submitUrl[0])
//		boaRedirect(wp, submitUrl);
//	else
//		boaDone(wp, 200);
}

