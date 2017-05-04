#ifndef INCLUDE_UC_MDIO_H
#define INCLUDE_UC_MDIO_H

extern unsigned char* mdio_wait_netinfo(int sec);
extern void mdio_event_handler(int sig_no);
extern int mdio_set_host_pid(void);
extern void mdio_close(void);
extern int mdio_open(void);

#endif //INCLUDE_UC_MDIO_H

