#include "usbctl.h"

unsigned int s2i(char  *str_P)
{
    unsigned int  val;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;
        }
    }
    
    return val;
}

static void usage(void)
{
	printf("Usage:\n");
	printf("get_usb3phy [page] [hex-reg]\n");
	printf("	Get value of usb3 phy reg 0xXX at the page.\n");
	printf("	[page] = 0 | 1, [hex-reg] = 0xXX\n\n");
	printf("set_usb3phy [page] [hex-reg] [hex-value]\n");
	printf("	Set 0xYYYY to usb3 phy reg 0xXX at the page.\n");
	printf("	[page] = 0 | 1, [hex-reg] = 0xXX, [hex-value] = 0xYYYY\n\n");
	printf("get_phy [port] [hex-reg]\n");
	printf("	Get value of usb2 phy reg 0xXX on the port.\n");
	printf("	[port] = 0 | 1, [hex-reg] = 0xXX\n\n");
	printf("set_phy [port] [hex-reg] [hex-value]\n");
	printf("	Set 0xYY to usb2 phy reg 0xXX on the port.\n");
	printf("	[port] = 0 | 1, [hex-reg] = 0xXX, [hex-value] = 0xYY\n\n");
	printf("dbg [on/off]\n");
	printf("	Turn on/off debug message\n");
	printf("	[on/off] = on | off\n\n");
	printf("test_pkt [port]\n");
	printf("	Continually send test packet on the port, for host only.\n");
	printf("	[port] = 0 | 1\n\n");
	printf("r [hex-addr] [len]\n");
	printf("	Dump memory data from memory address 0xXXXXXXXX to (0xXXXXXXXX + N -1).\n");
	printf("	[hex-addr] = 0xXXXXXXXX, [len] = N\n\n");
	printf("w [hex-addr] [hex-value]\n");
	printf("	Write a word of data, 0xYYYYYYYY , at memory address 0xXXXXXXXX.\n");
	printf("	[hex-addr] = 0xXXXXXXXX, [hex-value] = 0xYYYYYYYY\n\n");
	printf("get_dev_desc [steps]\n");
	printf("	Single step of get device descriptor, for EHCI only.\n");
	printf("	[steps] = 1 | 2 | 3 (1: SETUP stage, 2: DATA stage, 3: STATUS stage)\n\n");

	return;
}

static int check_usb_vfs(const char *dirname)
{
  DIR *dir;
  struct dirent *entry;
  int found = 0;

  dir = opendir(dirname);
  if (!dir)
    return 0;

  while ((entry = readdir(dir)) != NULL) {
    /* Skip anything starting with a . */
    if (entry->d_name[0] == '.')
      continue;

    /* We assume if we find any files that it must be the right place */
    found = 1;
    break;
  }

  closedir(dir);

  return found;
}

static int usb_exec_cmd(unsigned int cmd, void *data, char *path)
{
	int skfd;

	skfd = open(path, O_WRONLY);
	if (skfd < 0) {
		printf(USBCTL "Error! USB HUB cannot be opened!\n");
		goto ERR_OUT;
    }
	
	if (ioctl(skfd, cmd, data)< 0) {
		printf(USBCTL "Error! usb ioctl failed!\n");
		goto ERR_OUT;
    }

	if (skfd != (int)NULL)
		close(skfd);

	return 0;

ERR_OUT:
	if (skfd != (int)NULL)
		close(skfd);

	return -1;
}

/*
 *	Description: USB3 PHY register access
 *	Option: 
 *		get_phy3: get the value of a specific PHY register
 *		set_phy3: set a specific  value of a specific PHY register
 */
static int usb3_phy_access(char **spp, unsigned char is_read)
{
	struct usbdevfs_usb3phy phy;

	phy.page = s2i(spp[1]);
	if ((phy.page != 0) && (phy.page != 1)) {
		printf(USBCTL "usb phy page number should be 0 or 1!\n");
		return -1;
	}
	
	phy.reg = s2i(spp[2]);
	if (phy.reg > 0x1F) {
		printf(USBCTL "bad register address: 0x%02x\n", phy.reg);
		return -1;
	}

	phy.is_read = is_read;
	
	if (!is_read) {
		phy.value = s2i(spp[3]);
	}

	return usb_exec_cmd(USBDEVFS_USB3PHY, &phy, USBHUBPATH1);
}

/*
 *	Description: USB PHY register access
 *	Option: 
 *		get_phy: get the value of a specific PHY register
 *		set_phy: set a specific  value of a specific PHY register
 */
static int usb2_phy_access(char **spp, unsigned char is_read)
{
	struct usbdevfs_usbphy phy;

	phy.port = s2i(spp[1]);
	if ((phy.port != 0) && (phy.port != 1)) {
		printf(USBCTL "usb phy port number should be 0 or 1!\n");
		return -1;
	}
	
	phy.reg = s2i(spp[2]);
	if ((phy.reg < 0xE0) || (phy.reg > 0xF6) || ((phy.reg > 0xE7) && (phy.reg < 0xF0))) {
		printf(USBCTL "bad register address: 0x%02x\n", phy.reg);
		return -1;
	}

	phy.is_read = is_read;
	
	if (!is_read) {
		phy.value = s2i(spp[3]);
	}

	return usb_exec_cmd(USBDEVFS_USBPHY, &phy, USBHUBPATH1);
}

/*
 *	Description: USB debug message switch
 *	Option: 
 *		dbg on: turn on debug message
 *		dbg off: turn off debug message
 */
static int usb_dbg(char **spp, unsigned char reserved)
{
	unsigned int dbg; 
	
	if (!strcmp(*(spp+1), "on")) {
		dbg = 1;
	}
	else if (!strcmp(*(spp+1), "off")) {
		dbg = 0;
	}
	else {
		printf(USBCTL "usb debug error on or off?\n");
		return -1;
	}

	return usb_exec_cmd(USBDEVFS_DBG, &dbg, USBHUBPATH1);
}

/*
 *	Description: Send TEST PACKET from EHCI or OTG host on the specific port
 *	Option:
 *		test_pkt 0: Send TEST PACKET on port 0
 *		test_pkt 1: Send TEST PACKET on port 1
 */
static int usb_test_packet(char **spp, unsigned char reserved)
{
	unsigned char port;

	port = s2i(spp[1]);
	if ((port != 0) && (port != 1)) {
		printf(USBCTL "usb phy port number should be 0 or 1!\n");
		return -1;
	}

    return usb_exec_cmd(USBDEVFS_HCTESTPKT, &port, USBHUBPATH1);
}

/*
 *	Description: Memory access
 *	Option:
 *		r: Dump the specific length of data at a specific memory address
 *		w: Write a word of data at a specific memory address
 */
static int usb_mem_access(char **spp, unsigned char is_read)
{
	struct usbdevfs_mem mem;
	
	mem.addr = s2i(spp[1]);
	if ((((mem.addr & 0xF0000000) != 0xA0000000)
		&& ((mem.addr & 0xF0000000) != 0x80000000)
		&& ((mem.addr & 0xF0000000) != 0xB0000000)
	)) {
		printf("USBCTL Wrong address: 0x%08x\n", mem.addr);
		return -1;
	}

	mem.is_read = is_read;
	
	if (!is_read) {
		mem.value = s2i(spp[2]);
	}
	else {
		mem.len = s2i(spp[2]);
	}

	return usb_exec_cmd(USBDEVFS_MEM, &mem, USBHUBPATH1);
}

/*
 *	Description: Single step get device descriptor
 *	Option:
 *		get_dev_desc 1: SETUP stage of GET DEVICE descriptor
 *		get_dev_desc 2: DATA stage of GET DEVICE descriptor
 *		get_dev_desc 3: STATUS stage of GET DEVICE descriptor
 */
static int usb_get_dev_desc(char **spp, unsigned char reserved)
{
	struct usbdevfs_ctrl_stage ctrl_stage;
	unsigned char buf[64];
	int i;

	memset(&ctrl_stage, 0, sizeof(struct usbdevfs_ctrl_stage));
	memset(buf, 0, 64);
	
	ctrl_stage.stage = s2i(spp[1]);
	if (ctrl_stage.stage < 1 || ctrl_stage.stage > 3) {
		printf(USBCTL "step should be 1~3\n");
		return -1;
	}

	//setup command of GET DEVICE descriptor
	ctrl_stage.ctrl.bRequestType = 0x80;
	ctrl_stage.ctrl.bRequest = 0x06;
	ctrl_stage.ctrl.wValue = 0x0100;
	ctrl_stage.ctrl.wLength = 0x0040;
	ctrl_stage.ctrl.data = buf;
	ctrl_stage.ctrl.timeout = 100;

	if ((usb_exec_cmd(USBDEVFS_CTRL_STAGE, &ctrl_stage, USBHUBPATH1) < 0) &&
		(usb_exec_cmd(USBDEVFS_CTRL_STAGE, &ctrl_stage, USBHUBPATH2) < 0) &&
		(usb_exec_cmd(USBDEVFS_CTRL_STAGE, &ctrl_stage, USBHUBPATH3) < 0))
		return -1;
	else
		return 0;
}

struct cmd_item cmdTable[] = {
	{"get_usb3phy", usb3_phy_access, 3, IS_READ},
	{"set_usb3phy", usb3_phy_access, 4, IS_WRITE},
	{"get_phy", usb2_phy_access, 3, IS_READ},
	{"set_phy", usb2_phy_access, 4, IS_WRITE},
	{"dbg", usb_dbg, 2, IGNORE},
	{"test_pkt", usb_test_packet, 2, IGNORE},
	{"r", usb_mem_access, 3, IS_READ},
	{"w", usb_mem_access, 3, IS_WRITE},
	{"get_dev_desc", usb_get_dev_desc, 2, IGNORE},
	{"", NULL, 0, 0}
};

int main(int argc, char *argv[])
{
	char **spp;
	unsigned char mount_retry = 5;
	unsigned char mountOK = 0;
	struct cmd_item *cmdp;

	argc--;
	argv++;

	if (argc == 0) goto USAGE_END;

	while (mount_retry) {
		if (!check_usb_vfs(USBPATH)) {
			system("mount -t usbfs none /proc/bus/usb");
			usleep(10000);
		}
		else {
			mountOK = 1;
			break;
		}
		mount_retry--;
	}

	if (!mountOK) {
		printf("mount usbfs fail!\n");
		goto ERROR_END;
	}

	spp = argv;
	for (cmdp = &cmdTable[0]; cmdp->fn != NULL; cmdp++) {
		if (!strcmp(*spp, cmdp->display_name)) {
			if (argc != cmdp->argc) {
				printf(USBCTL "wrong input!\n");
				goto ERROR_END;
			}
			if (cmdp->fn(spp, cmdp->rw_type) < 0)
				goto ERROR_END;
			break;
		}
	}

	if (cmdp->fn == NULL)
		goto USAGE_END;

	exit(0);

USAGE_END:
	usage();

ERROR_END:
	//printf("ioctl error\n"); 
	exit(1);
}

