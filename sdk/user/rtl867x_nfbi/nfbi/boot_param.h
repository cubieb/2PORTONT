#ifndef BOOT_PARAM_H
#define BOOT_PARAM_H


#define BOOTCONF_MAGIC (('b'<<24) | ('t'<<16) | ('c'<<8) | ('f')) //0x62746366
struct bootconf
{
	unsigned long	magic;
	unsigned char	mac[6];
	unsigned short	flag;
	unsigned long	ip;
	unsigned long	ipmask;
	unsigned long	serverip;
	unsigned char	filename[24];
	unsigned char	res[16];
};
typedef struct bootconf bootconf_t, *bootconf_p;


int bootconf_get(bootconf_t *buf);

void bootparam_set_mac(char *p);
void bootparam_set_ip(unsigned long  p);
void bootparam_set_ipmask(unsigned long  p);
void bootparam_set_serverip(unsigned long  p);
void bootparam_set_filename(char *p);

#endif /*BOOT_PARAM_H*/
