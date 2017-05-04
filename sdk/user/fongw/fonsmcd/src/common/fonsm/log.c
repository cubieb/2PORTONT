/*
 * Fonsm log module. Used inside the fonsm backend module and on the client.
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070202 Pablo Martin Medrano <pablo@fon.com>
 *
 * $Id: log.c,v 1.1 2012/09/20 03:52:23 paula Exp $
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#include "log.h"

#define MAX_SESSION_PREFIX 256

typedef struct {
	char domain[256];
	LG_LEVEL watermark;
	int mode;
	LG_log_function_pointer function;
} ST_fslog;

static ST_fslog fslog;
static void LG_log_string(LG_LEVEL level, const char *message);

/*! 
 \brief Starts the log subsystem, redirecting glog() to stderr/syslog depending
 on mode
 \retval FSLOG_ERROR : FSLOG_OK if everything goes well
 \param lg : handle that will be returned
 \param ident : program identifier, any string 
 \param low_watermark : if the log level is less than this value, it will not be logged
 \param mode : FSLOG_MODE_SYSLOG (log to syslog) or FSLOG_MODE_STDERR (log to stderr)
*/
FSLOG_ERROR LG_start(const char *domain, LG_LEVEL watermark, int mode,
	LG_log_function_pointer log_function, int facility)
{
	strncpy(fslog.domain, domain, MAX_LG_DOMAIN);
	fslog.domain[MAX_LG_DOMAIN - 1] = '\0';
	fslog.watermark = watermark;
	fslog.mode = mode;
	fslog.function = log_function?log_function:LG_log_string;
	if (fslog.mode & LG_SYSLOG)
		openlog(domain, LOG_NDELAY, facility);
	return FSLOG_OK;
}

/*! 
 \brief Set the low watermark
 \retval FSLOG_ERROR : FSLOG_OK
 \param lg : log handle
 \param low_watermark : new watermark 
*/
FSLOG_ERROR LG_set_loglevel(LG_LEVEL watermark)
{
	fslog.watermark = watermark;
	return FSLOG_OK;
}

/*! 
 \brief  Ends the log subsystem, unregisteing glog handle
 \retval FSLOG_ERROR : FSLOG_OK if everything goes well
 \param handle : log handle to free
*/
FSLOG_ERROR LG_end(void)
{
	if (fslog.mode & LG_SYSLOG)
		closelog();
	return FSLOG_OK;
}


void LG_log(LG_LEVEL loglevel, const char *message, ...)
{
	va_list ap;
	char buffer[4096];
	int n;

	va_start(ap, message);
	n = vsnprintf(buffer, MAX_LOG_STRING, message, ap);
	va_end(ap);
	if (n > -1 && n < MAX_LOG_STRING)
		fslog.function(loglevel, buffer);
	else
		fon_critical("%s: Message too big to be logged", __FUNCTION__); 
}

/* Default log function (when mode is LG_SYSLOG or LG_STDERR) */
static void LG_log_string(LG_LEVEL level, const char *message) 
{
        static struct {
                int syslog_level;
                char *log_string;
        } fonlog_to_syslog[] = {
                [LG_DEBUG] = {LOG_ERR, "DEBUG"},
                [LG_MESSAGE] = {LOG_ERR, "MESSAGE"},
                [LG_WARNING] = {LOG_ERR, "WARNING"},
                [LG_CRITICAL] = {LOG_ERR, "CRITICAL"},
                [LG_ERROR] = {LOG_ERR, "ERROR"}
        };

	if (level < fslog.watermark)
		return;
	if (fslog.mode & LG_SYSLOG) {
		if (level == LG_MESSAGE) {
			syslog(LOG_INFO, "%s", message);
		} else {
			syslog(fonlog_to_syslog[level].syslog_level, "%s: %s", fonlog_to_syslog[level].log_string, message);
		}
	}
	if (fslog.mode & LG_STDERR) {
		fprintf(stderr, "%s[%d]: (%s) %8.8s: %s\n", fslog.domain, 
				getpid(), "Main", fonlog_to_syslog[level].log_string, 
				message);
	}
}

