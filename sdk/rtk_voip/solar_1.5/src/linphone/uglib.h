/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef UGLIB_H
#define UGLIB_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <stdarg.h>

#include <string.h>

#include "osip_port.h"

/* integer types */
typedef  uint64_t guint64;
typedef  int64_t gint64;
typedef  uint16_t guint16;
typedef  uint32_t guint32;
typedef  signed short gint16;
typedef  int32_t gint32;
typedef  unsigned int guint;
typedef  int gint;
typedef  char gchar;
typedef  unsigned char guchar;
typedef  unsigned char guint8;
typedef  void* gpointer;
typedef int gboolean;
typedef double gdouble;
typedef int gsize;
typedef ssize_t gssize;

#define TRUE 1
#define FALSE 0

#ifdef __cplusplus
#define G_BEGIN_DECLS extern"C"{
#define G_END_DECLS }
#else
#define G_BEGIN_DECLS 
#define G_END_DECLS 
#endif



typedef gchar GError;
GError *g_error_new(gint dummy1, gint dummy2, const gchar* errormsg,...);
void    g_set_error(GError **err,gint dummy1,gint dummy2,const gchar *format,...);

/*misc*/
#define g_return_if_fail(expr) if (!(expr)) {fprintf(stderr,"%s:%i- assertion" #expr "failed\n",__FILE__,__LINE__); return;}
#define g_return_val_if_fail(expr,ret) if (!(expr)) {fprintf(stderr,"%s:%i- assertion #expr failed\n",__FILE__,__LINE__); return (ret);}
#define g_assert(expr) if (!(expr)) fprintf(stderr,"%s:%i- assertion #expr failed\n",__FILE__,__LINE__)
#define g_assert_not_reached() fprintf(stderr,"%s:%i- assert not reached failed\n",__FILE__,__LINE__)


#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include "voip_manager.h"
#include "voip_debug.h"

#define DEBUG_B_ERROR			0x00000001	/* g_error */
#define DEBUG_B_WARNING			0x00000002	/* g_warning */
#define DEBUG_B_MESSAGE			0x00000004	/* g_message */
#define DEBUG_B_SOLAR_D			0x10000000	/* solar -d */

extern unsigned long g_SystemDebug;
extern FILE *g_dbgfile;
extern int rtk_print(int level, char *module, char *msg);

static inline void g_print(int type, const gchar *fmt, ...)
{
	va_list args;
	char msg[DBG_PRINT_MAX];
	int level;

	if( ( g_SystemDebug & type ) == 0 )
		return;

	if (type == DEBUG_B_ERROR)
	{
		level = RTK_DBG_ERROR;
	}
	else if (type == DEBUG_B_WARNING)
	{
		level = RTK_DBG_WARNING;
	}
	else
	{
		level = RTK_DBG_INFO;
	}

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	rtk_print(level, "SIP", msg);

	if (type == DEBUG_B_ERROR)
		exit(-1);
}

#define g_warning(...) g_print(DEBUG_B_WARNING, __VA_ARGS__)
#define g_error(...) g_print(DEBUG_B_ERROR, __VA_ARGS__)
#define g_message(...) g_print(DEBUG_B_MESSAGE, __VA_ARGS__)

#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

/* memory allocation functions */
#define g_malloc(sz)  osip_malloc(sz)

#ifdef MALLOC_DEBUG
#define g_malloc0(S)  osip_calloc(1,S)
#else
static inline void * g_malloc0(int sz)
{
	void *p=malloc(sz);
	if (p==NULL) {
		exit(0);
	}
	memset(p,0,sz);
	return p;
}
#endif

#define g_alloca(s)    alloca(s)
#define g_new(type,count)   (type *)g_malloc(sizeof(type)*(count))
#define g_new0(type, count)	(type *)g_malloc0(sizeof(type)*(count))
#define g_realloc(p,sz) osip_realloc((p),(sz))
#define g_free(p) osip_free(p)

typedef pthread_mutex_t GMutex;
static inline GMutex *g_mutex_new(void)
{
	int i;

	pthread_mutex_t *mutex=g_new(pthread_mutex_t,1);
	i = pthread_mutex_init(mutex,NULL);
	if (i)
	{
		fprintf(stderr, "g_mutex_new failed\n");
		exit(0);
	}

	return mutex;
}

static inline void g_mutex_free(GMutex *mut)
{
	int i;

	if (mut)
	{
		i = pthread_mutex_destroy(mut);
		if (i)
		{
			fprintf(stderr, "g_mutex_free failed\n");
			exit(0);
		}

		g_free(mut);
	}
}

typedef enum
{
  G_THREAD_PRIORITY_LOW,
  G_THREAD_PRIORITY_NORMAL,
  G_THREAD_PRIORITY_HIGH,
  G_THREAD_PRIORITY_URGENT
} GThreadPriority;

typedef pthread_t GThread;

typedef gpointer (*GThreadFunc)(gpointer data);

static inline GThread *g_thread_create(GThreadFunc func, gpointer data, gboolean joinable, void **error){
	GThread *thread=g_new(GThread,1);
	pthread_create(thread,NULL,func,data);
	return thread;
}

static inline void g_thread_join(GThread *thread){
	pthread_join(*thread,NULL);
	g_free(thread);
}

static inline void g_thread_set_priority(GThread *thread,GThreadPriority prio){
	if (prio>G_THREAD_PRIORITY_NORMAL){
		struct sched_param param;
		param.sched_priority=sched_get_priority_min(SCHED_RR);
		sched_setscheduler(*thread,SCHED_RR,&param);
	}
}

#define g_mutex_lock(mutex)	  pthread_mutex_lock((mutex))
#define g_mutex_unlock(mutex)  pthread_mutex_unlock((mutex))
#define g_mutex_free(mutex)		pthread_mutex_destroy((mutex));g_free((mutex))

typedef pthread_cond_t GCond;
static inline GCond * g_cond_new(void)
{
	pthread_cond_t *cond=g_new(pthread_cond_t,1);
	pthread_cond_init(cond,NULL);
	return cond;
}
#define g_cond_wait(cond,mutex)	pthread_cond_wait((cond),(mutex))
#define g_cond_signal(cond)		pthread_cond_signal((cond))
#define g_cond_broadcast(cond)	pthread_cond_broadcast((cond))
#define g_cond_free(cond)		pthread_cond_destroy((cond)); g_free((cond))

#define g_thread_init(vtable)
#define g_thread_supported()	(1)

#include <time.h>
typedef struct timeval GTimer;
#define g_timer_new() g_new(GTimer,1)
#define g_timer_destroy(t) g_free((t))
#define g_timer_start(t)  gettimeofday((t),NULL)
#define g_timer_reset(t)  g_timer_start(t)

gdouble g_timer_elapsed(GTimer *t,guint *us);

/*GString */

typedef struct _GString{
	gchar *str;
	gint curindex;
	gint len;
	gint allocated_len;
} GString ;

G_BEGIN_DECLS
GString *g_string_new(const gchar *init);
GString *g_string_append_c(GString *string, gchar c);
GString *g_string_append(GString *obj, const gchar *val);
gchar * g_string_free(GString *string, gboolean free_segment);
GString *g_string_append_len(GString *obj, const gchar *val, gssize len);
G_END_DECLS

#include "glist.h"

#define G_GNUC_CONST

G_BEGIN_DECLS
gchar *g_strdup_printf(const char *fmt,...);
gchar* g_strconcat(const gchar *string1, ...);
void g_strfreev(gchar **strarray);

G_END_DECLS
#define g_stpcpy(d,s) stpcpy((d),(s))
#define g_strdup(machin)	osip_strdup(machin)
#define g_strndup(s,n)  strndup((s),(n))
#define g_snprintf  snprintf
#define g_getenv	getenv
#define g_strerror(errnum) strerror(errnum)


typedef enum {G_FILE_TEST_EXISTS} GFileTest;

#define g_get_home_dir() getenv("HOME")
#define g_get_current_dir() get_current_dir_name()

#define g_build_filename(a,b,c) g_strdup_printf("%s/%s",a,b)

typedef enum
{
  /* log flags */
  G_LOG_FLAG_RECURSION          = 1 << 0,
  G_LOG_FLAG_FATAL              = 1 << 1,

  /* GLib log levels */
  G_LOG_LEVEL_ERROR             = 1 << 2,       /* always fatal */
  G_LOG_LEVEL_CRITICAL          = 1 << 3,
  G_LOG_LEVEL_WARNING           = 1 << 4,
  G_LOG_LEVEL_MESSAGE           = 1 << 5,
  G_LOG_LEVEL_INFO              = 1 << 6,
  G_LOG_LEVEL_DEBUG             = 1 << 7,

  G_LOG_LEVEL_MASK              = ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;

#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN ""
#endif
void g_log(const gchar *log_domain,GLogLevelFlags log_level,const gchar *format,...);
void g_logv(const gchar *log_domain,GLogLevelFlags log_level,const gchar *format,va_list args);
typedef void (*GLogFunc)  (const gchar *log_domain,
                                          GLogLevelFlags log_level,
                                           const gchar *message,
                                           gpointer user_data);
void g_log_set_handler(const gchar *log_domain,GLogLevelFlags log_levels, GLogFunc log_func, gpointer user_data);


#include "gspawn.h"
#include "gshell.h"

#define GINT_TO_POINTER(a)	((void*)(long)a)

gboolean g_file_test(const char *filename, gint test);

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n);
#endif

#ifndef HAVE_GET_CURRENT_DIR_NAME
char *get_current_dir_name(void);
#endif

#ifndef HAVE_STPCPY
char *stpcpy(char *dest, const char *src);
#endif

#ifdef MALLOC_DEBUG
#include "memwatch.h"
#endif

char *int_2char(int a);

#endif
