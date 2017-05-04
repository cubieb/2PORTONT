#ifndef NETLOG_H_
#define NETLOG_H_

#define NETLOG_PORT             0x1234
#define NETLOG_MAX_BUFSIZE      (32*1024)
#define NETLOG_WATCHDOGVAL		(20 * HZ)
#define NETLOG_PRINT_INTERVAL	(HZ / 4)
#define NETLOG_SEND_THRESHOLD   (NETLOG_MAX_BUFSIZE / 2) /* Threshold to trigger send net log */


#define NETLOG_NULL    0
#define NETLOG_START   1
#define NETLOG_STOP    2
#define NETLOG_INPUT   3
#define NETLOG_POLLING 4

void netlog_emit_char(char c);

#endif // NETLOG_H_

