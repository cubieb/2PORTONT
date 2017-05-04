/**
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology
 * and University of Helsinki.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

/** 
 * @file
 *
 * @author Samu Varjonen
 */ 

#ifndef OAM_DEBUG_H
#define OAM_DEBUG_H

#include <stdio.h>
#include <time.h>

/**
 * Maximum length for the debug prefix printed before the debug message.
 * Includes filename, line number and max(debug_prefix[]) 
 */
#define DEBUG_PREFIX_MAX  128

/**
 * Maximum length for the whole debug printout.
 * Stderror includes prefix, separator, msg and a terminator character.
 * Syslog includes msg and terminator character.
 */
#define DEBUG_MSG_MAX_LEN     1024

/**
 * Macro to log statistics lines to a file 
 */
#define OAM_STAT(...) stat_print(__VA_ARGS__);
 
/**
 * Macro to log statistics lines to a file, 
 * also prints the same info wit OAM_INFO 
 */
#define OAM_STAT_INFO(...) stat_print(__VA_ARGS__);oam_print_str(DEBUG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * Maps the sockaddr_in to sockaddr_in6
 */
#define IPV4_TO_IPV6_MAP(in_addr_from, in6_addr_to)                       \
    { (in6_addr_to)->s6_addr32[0] = 0;                                \
      (in6_addr_to)->s6_addr32[1] = 0;                                \
      (in6_addr_to)->s6_addr32[2] = htonl(0xffff);                    \
      (in6_addr_to)->s6_addr32[3] = (uint32_t) ((in_addr_from)->s_addr); }

/**
 * differentiate between die(), error() and debug() error levels 
 */
enum debug_level { DEBUG_LEVEL_DIE, DEBUG_LEVEL_ERROR, DEBUG_LEVEL_INFO,
                   DEBUG_LEVEL_DEBUG, DEBUG_LEVEL_XTRA, DEBUG_LEVEL_MAX };

/**
 * \brief OAM_ERROR a macro that *MUST* be used for printing.
 *
 * Almost the same as the OAM_DEBUG but the printed line starts with 
 * an ERR rather than DBG.
 *
 * @see OAM_DEBUG
 */
#define OAM_ERROR(...) oam_print_str(DEBUG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * \brief OAM_INFO a macro that *MUST* be used for printing.
 *
 * Almost the same as the OAM_DEBUG but the printed line starts with 
 * an INF rather than DBG.
 *
 * @see OAM_DEBUG
 */
#define OAM_INFO(...) oam_print_str(DEBUG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * \brief OAM_XTRA a macro that *MUST* be used for printing.
 *
 * Almost the same as the OAM_DEBUG but the printed line starts with 
 * an XTR rather than DBG.
 *
 * @see OAM_DEBUG
 */
#define OAM_XTRA(...) oam_print_str(DEBUG_LEVEL_XTRA, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * \brief OAM_DIE a macro that *MUST* be used to fatal error and exit.
 */
#define OAM_DIE(...) oam_die(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * \brief OAM_DEBUG a macro that *MUST* be used for printing.
 *
 * This macro is the main printing macro that produces the debug lines and 
 * the extra info correctly. The main idea behind this macro is that we 
 * can simply reroute the messages from stderr to syslog for example.
 *
 */
#define OAM_DEBUG(...) oam_print_str(DEBUG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/**
 * \brief OAM_HEXDUMP a macro that *MUST* be used for printing.
' */
#define OAM_HEXDUMP(prefix, str, len) \
    oam_hexdump(__FILE__, __LINE__, __FUNCTION__, prefix, str, len)

/**
 * Logtypes in use.
 */
enum logtype_t { LOGTYPE_NOLOG, LOGTYPE_SYSLOG, LOGTYPE_STDERR };
/**
 * Types for debug. Note LOGDEBUG_NONE is not in use.
 */
enum logdebug_t {LOGDEBUG_ALL, LOGDEBUG_MEDIUM, LOGDEBUG_XTRA, LOGDEBUG_NONE };
/**
 * Daemon type definitions.
 * @see daemon_prefix[]
 * @note Must be in the same order as enum daemon_type_t_level (straight mapping)
 */
enum daemontype_t { DAEMONTYPE_NONE, DAEMONTYPE_MEP, DAEMONTYPE_MIP, DAEMONTYPE_OAMSTAT, DAEMONTYPE_OAMTOOL };

void oam_set_logtype(int logtype);
int oam_get_logtype();
void oam_set_loglevel(char *level);
void oam_set_daemontype(int new_daemontype);
void oam_print_str(int debug_level, 
                   const char *file,
                   int line, 
                   const char *function,
                   const char *fmt,
                   ...);
void oam_hexdump(const char *file, 
                 int line, 
                 const char *function,
                 const char *prefix, 
                 const void *str, 
                 const int len);
void oam_die(const char *file, 
             int line, 
             const char *function,
             const char *fmt, ...);
void stat_print(const char *fmt, 
                ...);
int oam_format_time(time_t *time_to_format, 
                    char *time_string,
                    int length);
void oam_print_current_loglevel(void);
#endif /* OAM_DEBUG_H */
