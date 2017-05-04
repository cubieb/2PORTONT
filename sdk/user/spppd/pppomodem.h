#ifdef CONFIG_USER_PPPOMODEM
#ifndef _PPPOMODEM_H_
#define _PPPOMODEM_H_

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include "if_sppp.h"

enum PPPOMODEM_STATE{
	POM_ST_DEAD=0,
	POM_ST_INIT,
	POM_ST_DIAL,
	POM_ST_CONNECT,
	POM_ST_DISCONNECT,

	POM_ST_NUM /*last*/
};

enum PPPOMODEM_EVENT{
	POM_EVENT_OPEN=0,
	POM_EVENT_CLOSE,

	POM_EVENT_NUM /*last*/
};

#define MAX_MODEM_DEV_NAME 32
#define SIM_PIN_LEN	4
struct pppomodem_param_s {
	int state; /*modem state*/
	int wait_child; /*wait child's process*/
	pid_t cpid; /*child process pid*/
	int debug; /*debug flag*/

	char has_simpin;
	char simpin[SIM_PIN_LEN+1];
	char has_apn;
	char apn[64];
	char dial[16+1];
	
	int ttyfd; /* Serial port file descriptor */ /* fd for actual serial port (not pty) */
	int restore_term; /* 1 => we've munged the terminal */
	int crtscts; /* Use hardware flow control */
	int inspeed; /* Input/Output speed requested */
	int baud_rate; /* Actual bits/second for serial device */
	int hungup; /* terminal has been hung up */
	struct termios inittermios;	/* Initial TTY termios */
	unsigned char modem; /* Use modem control lines */
	unsigned char sync_serial; /* Device is synchronous serial device */
	char devnam[MAX_MODEM_DEV_NAME]; /* Device name */
	char finddev; /*flag to find the device name automatically*/
};

struct id_entry {
	char *vid;
	char *pid;
};
void pppomodem_process(void);
int pppomodem_init(struct sppp *sp);
int pppomodem_close(struct sppp *sp);
int pppomodem_default(struct pppomodem_param_s *p);
void pppomodem_set_hangup(struct sppp *sp);

//lcp option, accm
void ppp_set_xaccm (struct sppp *sp, ext_accm accm);
void ppp_send_accm_config (struct sppp *sp, u_int32_t asyncmap);
void ppp_recv_accm_config (struct sppp *sp, u_int32_t asyncmap);

#endif //_PPPOMODEM_H_
#endif //CONFIG_USER_PPPOMODEM
