#ifndef LOGGING_H
#define LOGGING_H

#undef ENABLE_BFTP_DEBUG_LOG

extern FILE *logfile;
extern FILE *statuslog;
extern FILE *statuslogforreading;

#ifdef ENABLE_BFTP_DEBUG_LOG
#define DEB_BFTP_LOGFILE "/var/log/debftp.log"
extern FILE *deb_logfile;
#endif
void log_init();

/* Status codes */
#define SL_UNDEF 0
#define SL_SUCCESS 1
#define SL_FAILURE 2

/* Message type */
#define SL_INFO 1
#define SL_COMMAND 2
#define SL_REPLY 3

void bftpd_statuslog(char type, char type2, char *format, ...);

void bftpd_log(char *format, ...);
#ifdef ENABLE_BFTP_DEBUG_LOG
void deb_log(char *format, ...);
void deb_log_init();
#endif
void log_end();

#endif
