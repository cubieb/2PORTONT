#ifndef _NFBI_API_H_
#define _NFBI_API_H_
#include "linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_nfbi.h"

#if defined(CONFIG_RTL8685) || defined(CONFIG_RTL8685S)
#define NFBI_BOOTADDR			0x9fc00000
#else
#define NFBI_BOOTADDR			0x1fc00000
#endif /*CONFIG_RTL8685 8685S*/
#define NFBI_BOOTPARAM_OFFSET	0x7f80
#define NFBI_BOOTPARAM_SIZE		0x80
#define NFBI_BOOTPARAM_START	(NFBI_BOOTADDR+NFBI_BOOTPARAM_OFFSET)
#define NFBI_KERNADDR 			0x00400000
//#define NFBI_BOOTADDR 0x007f0000
//#define NFBI_KERNADDR 0x00700000


#define TAG_SET_COMMAND	0x80
#define TAG_GET_COMMAND	0xC0
#define TAG_GOOD_STATUS	0x80
#define TAG_BAD_STATUS	0xC0

#ifdef CONFIG_RTL8685S
#define BOOTCODE_DOWNLOAD_RETRY_MAX 10
#else
#define BOOTCODE_DOWNLOAD_RETRY_MAX 1
#endif

extern int nfbi_fd;

extern void real_sleep(unsigned int sec);
extern int _atoi(char *s, int base);

extern int hwreset(void);
extern int eqreset(void);
extern int nfbi_set_hcd_pid(int pid);
extern int nfbi_get_event_msg(struct evt_msg *evt);
extern int nfbi_register_read(int reg, int *pval);
extern int nfbi_register_write(int reg, int val);
extern int nfbi_register_mask_read(int reg, int mask, int *pval);
extern int nfbi_register_mask_write(int reg, int mask, int val);
extern int nfbi_mem32_write(int addr, int val);
extern int nfbi_mem32_read(int addr, int *pval);
extern int nfbi_bulk_mem_write(int addr, int len, char *buf);
extern int nfbi_bulk_mem_read(int addr, int len, char *buf);

extern int send_file_to_ram(char *filename, unsigned int ram_addr, int verify);
extern int bootcode_download(int verify, char *filename);
extern int firmware_download_w_boot(int verify, char *fw_filename, char *boot_filename);
extern int firmware_download_w_boot_nfbi(int verify, char *fw_filename, char *boot_filename);

extern void dump_misc(char *msg);
extern int bootcode_download_test(int verify, char *filename);
extern int firmware_downloadtest(int verify, char *fw_filename, char *boot_filename);
#endif //_NFBI_API_H_

