#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_mdio.h"



////////////////////////////////////////////////////////////////////////////
static int _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {			
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				printf("error hex format [%x]!\n", *s);
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}


////////////////////////////////////////////////////////////////////////////
#define MDIO_DEV_NAME		("/dev/rtl_mdio")
static int mdio_fd=-1;
static int mdio_open(void)
{
	mdio_fd=open(MDIO_DEV_NAME, O_RDWR);
	if(mdio_fd==-1)
	{
		perror("mdio_open(): open");
		return -1;
	}
	return 0;
}

static void mdio_close(void)
{
	if(mdio_fd!=-1) close(mdio_fd);
	mdio_fd=-1;
}

static int mdio_set_host_pid(int pid)
{	
	// Set daemon pid to driver. 
	// When this ioctl is set, driver will set AllSoftwareReady bit to 'CPU System Status' Register
	if(ioctl(mdio_fd, MDIO_IOCTL_SET_HOST_PID, &pid)<0) 
	{
		//printf( "%s(): failed\n", __FUNCTION__ );
		return -1;
	}

	return 0;
}

static int mdio_set_cmd_timeout(unsigned char t)
{	
	if(ioctl(mdio_fd, MDIO_IOCTL_SET_CMD_TIMEOUT, &t)<0)
	{
		//printf( "%s(): failed\n", __FUNCTION__ );
		return -1;
	}

	return 0;
}

static int mdio_set_phy_poll_time(unsigned char t)
{	
	if(ioctl(mdio_fd, MDIO_IOCTL_SET_PHY_POLL_TIME, &t)<0)
	{
		//printf( "%s(): failed\n", __FUNCTION__ );
		return -1;
	}

	return 0;
}

static int mdio_read_word(int addr, int *pval)
{
    int param;
    
    param= addr;
    if (0 == ioctl(mdio_fd, MDIO_IOCTL_READ_MEM, &param)) {
        *pval = param;
	    return 0;
	}
	else
	    return -1;
}

static int mdio_write_word(int addr, int val)
{
    struct mdio_mem32_param param;
    
    param.addr = addr;
    param.val = val;
    return ioctl(mdio_fd, MDIO_IOCTL_WRITE_MEM, &param);
}

static int mdio_set_priv_cmd(int p)
{	
	if(ioctl(mdio_fd, MDIO_IOCTL_PRIV_CMD, &p)<0)
	{
		printf( "%s(): failed\n", __FUNCTION__ );
		return -1;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////
struct mdio_reg_table_t 
{
	const char*		name;
	unsigned int	addr;
};

static struct mdio_reg_table_t mdio_reg_table[]=
{
	//{name,     val}
	{"RCR",		REG_RCR},
	{"SSR",		REG_SSR},
	{"SYSCR",	REG_SYSCR},
	{"SYSSR", 	REG_SYSSR},
	{"IMR", 	REG_IMR},
	{"ISR",		REG_ISR},
	{"UACR",	REG_UACR},
	{"UASR", 	REG_UASR},
	{"FDCR",	REG_FDCR},
	{"SDCR",	REG_SDCR},
	{"NFBIRR",	REG_NFBIRR},
	{"WDOGCNT",	REG_WDOGCNT},
	{NULL,		0}
};

static void reg_print_list(void)
{
    int i;

	printf( "mdio register list:\n" );
    i = 0;
    while (mdio_reg_table[i].name != NULL) 
	{
        printf("\t0x%08x %s\n", mdio_reg_table[i].addr, mdio_reg_table[i].name);
        i++;
    }
}

static unsigned int reg_name2addr(char *name)
{
    int i;

    i = 0;
    while (mdio_reg_table[i].name != NULL) {
        if (0 == strcmp(name, mdio_reg_table[i].name))
            return mdio_reg_table[i].addr;
        i++;
    }
    return -1;
}

static int reg_dump(void)
{
    int i;

    i = 0;
    while (mdio_reg_table[i].name != NULL) 
	{
		int reg, val;	

		reg=mdio_reg_table[i].addr;
		if(mdio_read_word(reg, &val)==0) 
			printf("%7s 0x%08x\n", mdio_reg_table[i].name, val);
		else
			printf("%7s failed\n", mdio_reg_table[i].name);

        i++;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////
static int cmd_regread(int index,int argc, char *argv[])
{
	int reg, val;

	if (argc < 3) 
	{
		reg_print_list();
		return -1;
	}
	reg = reg_name2addr(argv[2]);
	if (reg==-1)
	{
		reg_print_list();
		return -1;
	}
	if (0 == mdio_read_word(reg, &val)) 
	{
		printf("0x%08x\n", val);
		return 0;
	}
	return -1;
}

static int cmd_regwrite(int index,int argc, char *argv[])
{
	int reg, val;

	if (argc < 4) 
	{
		reg_print_list();
		return -1;
	}

	reg = reg_name2addr(argv[2]);
	if (reg==-1)
	{
		reg_print_list();
		return -1;
	}

	if (0 == strncmp(argv[3], "0x", 2))
		val = _atoi(argv[3]+2 ,16);
	else
		return -1;

	if (0 == mdio_write_word(reg, val))
		return 1;

	return -1;
}

static int cmd_regdump(int index,int argc, char *argv[])
{
	int ret=0;
	reg_dump();
	return ret;    	
}

int cmd_timeout(int index, int argc, char *argv[])
{
    int param;
	unsigned char t;
    
    if (argc==3)
	{
        param = atoi(argv[2]);
        if ((param<0) || (param>255))
            return -1;
        t=  (unsigned char)(param&0xff);
		if( mdio_set_cmd_timeout(t)==0 )
			return 1;
		else
			return -1;
	}
	return -1;
}

int cmd_phy_poll_time(int index, int argc, char *argv[])
{
    int param;
	unsigned char t;
    
    if (argc==3)
	{
        param = atoi(argv[2]);
        if ((param<0) || (param>255))
            return -1;
        t=  (unsigned char)(param&0xff);
		if( mdio_set_phy_poll_time(t)==0 )
			return 1;
		else
			return -1;
	}
	return -1;
}

static int cmd_dw(int index,int argc, char *argv[])
{
    unsigned int addr, val;

    if (argc != 3)
        return -1;      //wrong command format 

    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == mdio_read_word(addr, &val)) {
        printf("0x%08x\n", val);
	    return 0;
	}
	else
	    return -1;
}

static int cmd_ew(int index,int argc, char *argv[])
{
    int addr, val;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    // parsing <value>
    if (0 == strncmp(argv[3], "0x", 2))
        val = _atoi(argv[3]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == mdio_write_word(addr, val))
	    return 1;
	else
	    return -1;
}


int cmd_poll(int index, int argc, char *argv[])
{
    int param;
    
    if (argc==3)
	{
		if( strcmp(argv[2],"stop")==0 )
			param=0;
		else if( strcmp(argv[2],"start")==0 )
			param=1;
		if( mdio_set_priv_cmd(param)==0 )
			return 1;
		else
			return -1;
	}
	return -1;
}


int cmd_dsllink(int index, int argc, char *argv[])
{
    int param;
    
    if (argc==3)
	{
		if( strcmp(argv[2],"up")==0 )
			param=3;
		else if( strcmp(argv[2],"down")==0 )
			param=4;
		if( mdio_set_priv_cmd(param)==0 )
			return 1;
		else
			return -1;
	}
	return -1;
}


int cmd_dump_priv(int index, int argc, char *argv[])
{
    int param;
    
	param=2;
	return mdio_set_priv_cmd(param);
}



////////////////////////////////////////////////////////////////////////////
struct mdio_cmd_table_t
{
	const char *cmd;
	int (*func)(int index,int argc, char *argv[]);
	const char *msg;
};


static struct mdio_cmd_table_t mdio_cmd_table[]=
{
    {"regread",			cmd_regread,		"mdioctrl regread IMR"},
	{"regwrite", 		cmd_regwrite,		"mdioctrl regwrite IMR 0x1234"},
	{"regdump",			cmd_regdump,		"mdioctrl regdump"},
	
	{"cmd_timeout",		cmd_timeout,		"mdioctrl cmd_timeout 0"},
	{"phy_poll_time",	cmd_phy_poll_time,	"mdioctrl phy_poll_time 1"},

	{"dw",				cmd_dw,				"mdioctrl dw 0xb8019010"},
	{"ew",				cmd_ew,				"mdioctrl ew 0xb8019010 0x1234"},

	{"poll",			cmd_poll,			"mdioctrl poll start/stop"},
	{"dsllink", 		cmd_dsllink,		"mdioctrl dsllink up/down"},
	{"dump_priv",		cmd_dump_priv, 		"mdioctrl dump_priv"},
    {NULL, NULL,NULL},
};


static void cmd_print_list(void)
{
    int i;

    printf("\n========== commands ============\n");
    i = 0;
    while (mdio_cmd_table[i].cmd != NULL) 
	{         
            printf("%s\n", mdio_cmd_table[i].msg);
        i++;
	}
}


////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int i, ret=-1;

	if(argc<2) 
	{
		cmd_print_list();
		return ret;
	}

	if( mdio_open()<0 ) return ret;

    i = 0;
    while (mdio_cmd_table[i].cmd != NULL) 
	{
        if (0 == strcmp(argv[1], mdio_cmd_table[i].cmd)) 
		{
			if (mdio_cmd_table[i].func) 
			{
			    ret = mdio_cmd_table[i].func(i,argc, argv);
				if(ret>=0)
				{
					if(ret>0) printf("OK\n");
					ret=0;
				}else if (ret < 0){
					printf("FAIL\n");
					ret=-1;
				}
		    }
			break;
        }
        i++;
	}
	if(mdio_cmd_table[i].cmd==NULL)
		cmd_print_list();

	mdio_close();
	return ret;
}


