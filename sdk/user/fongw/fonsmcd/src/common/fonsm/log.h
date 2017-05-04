/*
 * Fonsm log module. Used inside the fonsm backend module and on the client.
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070202 Pablo Martin Medrano <pablo@fon.com>
 *
 * $Id: log.h,v 1.1 2012/09/20 03:52:23 paula Exp $
 */
#ifndef _LOG_H
#define _LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef enum {
        FSLOG_OK = 0,
        FSLOG_UNKNOWN = -1
} FSLOG_ERROR;


typedef void * LG_HANDLE;

typedef enum {
        LG_DEBUG = 0,
        LG_MESSAGE = 1,
        LG_INFO = 2,
        LG_WARNING = 3,
        LG_CRITICAL = 4,
        LG_ERROR = 5
} LG_LEVEL;

#define LG_SYSLOG 0x01
#define LG_STDERR 0x02
#define LG_CUSTOM 0x04

#define MAX_LG_DOMAIN 256
#define MAX_LOG_STRING 4096

#ifndef NDEBUG
#ifndef LOGPRINTF
#define fon_debug(...) LG_log (LG_DEBUG, __VA_ARGS__)
#else
#define fon_debug(...) { printf("DEBUG: "); printf(__VA_ARGS__); printf("\n"); }
#endif
#else
#define fon_debug(...)
#endif
#ifndef LOGPRINTF
#define fon_message(...) LG_log (LG_MESSAGE, __VA_ARGS__)
#define fon_warning(...) LG_log (LG_WARNING, __VA_ARGS__) 
#define fon_critical(...) LG_log (LG_CRITICAL, __VA_ARGS__)
#define fon_error(...) LG_log (LG_ERROR, __VA_ARGS__)
#else
#define fon_message(...) { printf("MESSAGE: "); printf(__VA_ARGS__); printf("\n"); }
#define fon_warning(...)  { printf("WARNING: "); printf(__VA_ARGS__); printf("\n"); }
#define fon_critical(...) { printf("CRITICAL: "); printf(__VA_ARGS__); printf("\n"); }
#define fon_error(...) { printf("ERROR: "); printf(__VA_ARGS__); printf("\n"); }
#endif

typedef void (*LG_log_function_pointer)(LG_LEVEL level, const char *message);

FSLOG_ERROR LG_start(const char *domain, LG_LEVEL watermark, int mode, LG_log_function_pointer log_function, int facility);
FSLOG_ERROR LG_set_loglevel(LG_LEVEL watermark);
FSLOG_ERROR LG_end(void);
void LG_log(LG_LEVEL loglevel, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif

