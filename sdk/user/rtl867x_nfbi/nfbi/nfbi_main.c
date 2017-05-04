#include <stdio.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nfbi_api.h"
#include "boot_param.h"
#include "nfbi_frame.h"


#define NFBI_DEV_NAME  ("/dev/rtl_nfbi")
#define NFBI_VERSION	"1.4"


//--------------------------------------------------------------------------------
struct reg_table_t 
{
	const char*	name;
	int 		val;
	int 		dval;
};
static struct reg_table_t nfbi_reg_table[]=
{
	//{name,     val,                  	dval }
	{"UACR",   NFBI_REG_UACR,	0x0000},
	{"UASR",   NFBI_REG_UASR,	0x0000},
	{"PHYID1", NFBI_REG_PHYID1, 0x001c},
	{"PHYID2", NFBI_REG_PHYID2, 0xcb81},
	{"FDCR",   NFBI_REG_FDCR,	0x0000},
	{"SDCR",   NFBI_REG_SDCR,	0x0000},
	{"WDOGCNT",NFBI_REG_WDOGCNT,0x0000},
	{"DDCH",   NFBI_REG_DDCH,	0x0000},
	{"DDCL",   NFBI_REG_DDCL,	0x0000},
	{"CMD",    NFBI_REG_CMD,	0x0000},
	{"ADDH",   NFBI_REG_ADDH,	0x1fc0},
	{"ADDL",   NFBI_REG_ADDL,	0x0000},
	{"DH",     NFBI_REG_DH,		0x0000},
	{"DL",     NFBI_REG_DL,		0x0000},
	{"SCR",    NFBI_REG_SCR,	0x0000},
	{"RSR",    NFBI_REG_RSR,	0x0000},
	{"SYSSR",  NFBI_REG_SYSSR,	0x0000},
	{"SYSCR",  NFBI_REG_SYSCR,	0x1000},
	{"IMR",    NFBI_REG_IMR,	0x0001},
	{"ISR",    NFBI_REG_ISR,	0x0000},
	{"DCH",    NFBI_REG_DCH,	0x5808},
	{"DCL",    NFBI_REG_DCL,	0x0000},
	{"DTH",    NFBI_REG_DTH,	0xffff},
	{"DTL",    NFBI_REG_DTL,	0x0fc0},
	{"RR",     NFBI_REG_RR, 	0x0000},
	{NULL,     0,				0}
};

//-----------------------------------------------------------------------------
int cmd_version(int argc, char *argv[]);
//int cmd_getcmd(int argc, char *argv[]);
//int cmd_setcmd(int argc, char *argv[]);
int cmd_hwreset(int argc, char *argv[]);
int cmd_eqreset(int argc, char *argv[]);
int cmd_dump_regs(int argc, char *argv[]);
int cmd_probephyaddr(int argc, char *argv[]);
int cmd_phyaddr(int argc, char *argv[]);
int cmd_testphyaddr(int argc, char *argv[]);
int cmd_tx_cmdword_interval(int argc, char *argv[]);
int cmd_interrupt_timeout(int argc, char *argv[]);
//int cmd_handshake_polling(int argc, char *argv[]);
int cmd_bootdownload(int argc, char *argv[]);
int cmd_memread(int argc, char *argv[]);
int cmd_memwrite(int argc, char *argv[]);
int cmd_fwdownload_w_boot(int argc, char *argv[]);
int cmd_regwrite(int argc, char *argv[]);
int cmd_regread(int argc, char *argv[]);
int cmd_fwdownload_wo_boot(int argc, char *argv[]);
int cmd_bootdownload_ip(int argc, char *argv[]);
int cmd_bootdownload_ipmac(int argc, char *argv[]);
int cmd_fwdownload_w_boot_ip(int argc, char *argv[]);
int cmd_tftp_fw_w_boot(int argc, char *argv[]);
int cmd_fw_w_boot(int argc, char *argv[]);
int cmd_dw(int argc, char *argv[]);
int cmd_ew(int argc, char *argv[]);
int cmd_dump_priv_data(int argc, char *argv[]);
int cmd_dump_eq(int argc, char *argv[]);
int cmd_bulkmemwrite(int argc, char *argv[]);
int cmd_bulkmemread(int argc, char *argv[]);
int cmd_memtest(int argc, char *argv[]);
int cmd_memtest2(int argc, char *argv[]);
int cmd_phyidtest(int argc, char *argv[]);
int cmd_downloadtest(int argc, char *argv[]);
int cmd_downloadtest2(int argc, char *argv[]);
int cmd_downloadtest3(int argc, char *argv[]);


// Command Table
struct nfbi_cmd_table_t {
	const char*	cmd;	// Input command string
	int			debug;	//command for debugging
	int			(*func)(int argc, char *argv[]);
	const char*	msg;	// Help message
};

struct nfbi_cmd_table_t nfbi_cmd_table[]=
{
    {"version",         1,		cmd_version,       "nfbi version"},
	{"eqreset", 		0,		cmd_eqreset, 	   "nfbi eqreset"},
	{"hwreset", 		0,		cmd_hwreset, 	   "nfbi hwreset"},
	{"probephyaddr",	1,		cmd_probephyaddr,  "nfbi probephyaddr"},
	{"phyaddr", 		1,		cmd_phyaddr, 	   "nfbi phyaddr 1"},
	{"testphyaddr", 	0,		cmd_testphyaddr,   "nfbi testphyaddr"},
	{"regread", 		0,		cmd_regread, 	   "nfbi regread SCR [phyaddr]"},
	{"regwrite",		0,		cmd_regwrite,	   "nfbi regwrite SCR 0xff00 0x1234 [phyaddr] or nfbi regwrite SCR 0x1234 [phyaddr] "},
    {"memread",         0,		cmd_memread,       "nfbi memread 0x00008000 1"},
    {"memwrite",        0,		cmd_memwrite,      "nfbi memwrite 0x00008000 0x1234 1"},
	{"membulkwrite",	0,		cmd_bulkmemwrite,  "nfbi membulkwrite 0x00008000 0x1234567890 1"},
	{"membulkread", 	0,		cmd_bulkmemread,   "nfbi membulkread 0x00008000 12 1"},
//	{"getcmd",			0,		cmd_getcmd,		   "nfbi getcmd cmd_timeout"},
//	{"setcmd",			0,		cmd_setcmd,		   "nfbi setcmd cmd_timeout 1"},
	{"dumppriv",		0,		cmd_dump_priv_data,"nfbi dumppriv"},
	{"dumpregs",		0,		cmd_dump_regs,	   "nfbi dumpregs [phyaddr]"},
	{"dumpeq",			0,		cmd_dump_eq, 	   "nfbi dumpeq"},
	{"dw",				0,		cmd_dw,		       "nfbi dw 0x80008000"},
	{"ew",				0,		cmd_ew,		       "nfbi ew 0x80008000 0x1234"},

    //jiunming test
    {"memtest",         0,		cmd_memtest,       "nfbi memtest 0x1fc00000 0x10"},
    {"memtest2",        0,		cmd_memtest2,      "nfbi memtest2 0x1fc00000 0x0 0x10000"},
    {"phyidtest",       0,		cmd_phyidtest,     "nfbi phyidtest [0 or 1]"},
    {"downloadtest",    0,		cmd_downloadtest,  "nfbi downloadtest [single or bulk]"},
    {"downloadtest2",   0,		cmd_downloadtest2, "nfbi downloadtest2 1 /etc/fw/boot.bin /etc/fw/linux.bin"},
	{"downloadtest3",	0,		cmd_downloadtest3, "nfbi downloadtest3 1 /etc/fw/boot.bin /etc/fw/linux.bin"},
    
    

	//timer setting
	{"tx_cmdword_interval",  0,	cmd_tx_cmdword_interval, "nfbi tx_cmdword_interval 0"},
	{"interrupt_timeout",    0,	cmd_interrupt_timeout,   "nfbi interrupt_timeout 10"},
	//{"cmd_handshake_polling",1,	cmd_handshake_polling,   "nfbi cmd_handshake_polling 1"},

    //nfbi bootcode/runcode download command
    {"bootdownload",         1,	cmd_bootdownload,   	"nfbi bootdownload 0 /etc/fw/boot.bin"},
    {"bootdownload_w_ip",    1, cmd_bootdownload_ip,	"nfbi bootdownload_w_ip 192.168.1.17 /etc/fw/boot.bin"},
	{"bootdownload_w_ipmac", 1, cmd_bootdownload_ipmac, "nfbi bootdownload_w_ipmac 00e04c867701 192.168.1.17 /etc/fw/boot.bin"},
    {"tftp_fw_w_boot",       1,	cmd_tftp_fw_w_boot,     "nfbi tftp_fw_w_boot 00e04c867701 192.168.1.17 255.255.255.0 192.168.1.1 /etc/fw/boot.bin /etc/fw/linux.bin"},
    {"fw_w_boot",            1,	cmd_fw_w_boot,          "nfbi fw_w_boot 0 /etc/fw/boot.bin /etc/fw/linux.bin"},
    

    {NULL, 0, NULL, NULL},
};


//-----------------------------------------------------------------------------
static int mac_hexstr2bin( char *p, char *m )
{
	int i;

	if(!p || !m) return -1;
	if( strlen(p)!=12 ) return -1;

	for(i=0; i<6; i++)
	{
		char buf[3];
		long int val;
		
		buf[0]=p[i*2];
		buf[1]=p[i*2+1];
		buf[2]=0;
		
		errno = 0;
		val=strtol(buf, NULL, 16);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
				 || (errno != 0 && val == 0)) 
			return -1;
		m[i]=(unsigned char)(val&0xff);	
	}

	return 0;
}

#if 0
static int bin2hexstr( unsigned char *b, int len, unsigned char *h )
{
	int i;
	if(!b || len<0 || !h) return -1;
	h[0]=0;
	for(i=0; i<len; i++)
		sprintf( h+i*2, "%02x", b[i] );

	return 0;
}
#endif

static int regname2val(char *name, struct reg_table_t *table)
{
    int i;

    if (0 == strncmp(name, "0x", 2)) {
        i = _atoi(name+2 ,16);
        return i;
    }

    i = 0;
    while (table[i].name != NULL) {
        if (0 == strcmp(name, table[i].name))
            return table[i].val;
        i++;
    }
    return -1;
}

static void print_regwrite_list(void)
{
    int i;

    i = 0;
    while (nfbi_reg_table[i].name != NULL) {
        printf("nfbi regwrite %s 0x%04x\n", nfbi_reg_table[i].name, nfbi_reg_table[i].dval);
        i++;
        }
}

static void print_regread_list(void)
{
    int i;

    i = 0;
    while (nfbi_reg_table[i].name != NULL) {
        printf("nfbi regread %s\n", nfbi_reg_table[i].name);
        i++;
        }
}

int cmd_dump_regs(int argc, char *argv[])
{
    int i;
    int param;
    int val;
    int tmp;
    nfbi_register_read(NFBI_REG_PHYID2, &val);
    i = 0;
    printf("Name   Val    Default\n");
    printf("---------------------\n");
    if (argc==3) tmp = atoi(argv[2]) + 5 ;
    ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp);
    while (nfbi_reg_table[i].name != NULL) {
        if (val == NFBI_REG_PHYID2_DEFAULT2) {
            if ((strcmp(nfbi_reg_table[i].name, "DCH")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DCL")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DTH")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DTL")==0)) {
                i++;
                continue;
            }
        }
        param = nfbi_reg_table[i].val << 16; //put register address to high word
    	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
    	    printf("%6s 0x%04x 0x%04x\n", nfbi_reg_table[i].name, param&0xffff, nfbi_reg_table[i].dval);
    	else
            printf("%6s  Fail  0x%04x\n", nfbi_reg_table[i].name, nfbi_reg_table[i].dval);
        i++;
	}
    dump_misc("cmd_dump_regs");
    return 0;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_hwreset(int argc, char *argv[])
{
    int param;
    
    if (argc > 2)
        param = atoi(argv[2]);
    else
        param = 2;
    if ((param != 0) && (param != 1))
        param = 2; //hardware reset

	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 1;
	else
	    return -1;
}

int cmd_eqreset(int argc, char *argv[])
{
    int param;
    
    param = 5; //event queue reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 1;
	else
	    return -1;
}


int cmd_probephyaddr(int argc, char *argv[])
{
    int param;

    	param = 3; //probe phyaddr
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
	return 0;
}

int cmd_phyaddr(int argc, char *argv[])
{
    int param;

	if(argc<3) return -1;
	
    param = atoi(argv[2]);
   	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &param))
        return 1;
    else
        return -1;
}

int cmd_testphyaddr(int argc, char *argv[])
{
    int param, i, found, addr[32], verbose;

    verbose = 0;
    if (argc > 2) {
        if (strcmp(argv[2], "-v")==0)
            verbose = 1;
    }
    
    found = 0;
    for (i=0; i<32; i++) {
    	ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &i);
    	addr[i] = 0;
        param = NFBI_REG_PHYID2 << 16; //put register address to high word
    	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
    	    param = param&0xffff;
    	    if (verbose)
    	        printf("Probe PHY ADDR=%d  reg3=0x%04x\n", i, param);
    	    if ((param==NFBI_REG_PHYID2_DEFAULT) ||
    	        (param==NFBI_REG_PHYID2_DEFAULT2)||
    	        (param==NFBI_REG_PHYID2_DEFAULT3)||
    	        (param==NFBI_REG_PHYID2_DEFAULT4)){
    	        found++;
    	        addr[i] = 1;
    	    }
    	}
    }
    //update the phy numbers found
    if(found==0) return -1;
    else return found;
    //update the phy address to the driver
#if 0 
    i = 0xff;
    if (found == 1) {
        if (addr[1]==1)
            i = 1;
        else if (addr[2]==1)
            i = 2;
        else if (addr[3]==1)
            i = 3;
    }
    ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &i);
    
    if ((found == 1) && ((addr[1]==1)||(addr[2]==1)||(addr[3]==1)))
        return 1;
    else
        return -1;
#endif
}



int cmd_memtest(int argc, char *argv[])
{
    int addr, val, val_ret;
	int len,i,j;
	int r_count=0, e_count=0;
	int flag=-1;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    // parsing <len>
    if (0 == strncmp(argv[3], "0x", 2))
        len = _atoi(argv[3]+2 ,16);
    else
        return -1; //wrong command format

	addr=addr&0xfffffffc;
	printf( "cmd_memtest start, addr=0x%08x, len=0x%x\n", addr, len );
	val=0xa5b4c3d1;
	j=0;

	for(i=0; i<len; i=i+4)
	{
		//write
		if( nfbi_mem32_write(addr+i, val+j) )
			printf( "write fail:  0x%08x=0x%08x\n", addr+i, val+j );

		//read	
		val_ret=0;
	    if( nfbi_mem32_read(addr+i, &val_ret) ) 
		    printf( "read fail:  0x%08x\n", addr+i);

		if( (val+j)!=(val_ret) )
		{
			e_count++;
			if(flag!=0)
			{
				printf( "error:  0x%08x=0x%08x\n", addr+i, val+j );
				flag=0;
			}
		}else{
			r_count++;
			if(flag!=1)
			{
				printf( "OK:  0x%08x=0x%08x\n", addr+i, val+j );
				flag=1;
			}			
		}


		j++;
		if(j==97) j=0;
	}
	printf( "cmd_memtest stop, addr=0x%08x, right=%d, error=%d\n", addr+i-4, r_count, e_count);


	return 0;
}


int cmd_memtest2(int argc, char *argv[])
{
    int addr, val, val_ret;
	unsigned int len,i;

    if (argc != 5)
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

    // parsing <value len>
    if (0 == strncmp(argv[4], "0x", 2))
        len = _atoi(argv[4]+2 ,16);
    else
        return -1; //wrong command format

	addr=addr&0xfffffffc;
	printf( "cmd_memtest2 start, addr=0x%08x, val=0x%08x, value len=0x%08x\n", addr, val, len );
	for(i=val; i<val+len; i++)
	{
		if( (i&0xffff)==0 )
		{
			printf( "." );
			fflush(stdout);
		}
	
		//write
		if(nfbi_mem32_write(addr, i))
			printf( "write fail:  0x%08x=0x%08x\n", addr, i);

		//read	
		val_ret=0;
	    if(nfbi_mem32_read(addr, &val_ret)) 
		    printf( "read fail:  0x%08x\n", addr+i);


		if( (i)!=(val_ret) )
			printf( "error: 0x%08x\n", i );


	}
	printf( "cmd_memtest2 stop, addr=0x%08x\n", addr);
	return 0;
}


//test at nfbi 100Mhz----data shift right 1 bit
int cmd_phyidtest(int argc, char *argv[])
{
	int i,j;
    int reg;
    int param, mask;
	int shift_bit;

#define V_PHYID1 0x1c
#define V_PHYID2 0xcb81
#define V_TEST_TIME 100000

    if (argc != 3)
        return -1;      //wrong command format 

	if( atoi(argv[2])==0 )
		shift_bit=0;
	else
		shift_bit=1;
	printf( "PHYID1/ID2's value will shift right %d bit\n", shift_bit );
	printf( "PHYID1= 0x%04x\n", V_PHYID1>>shift_bit);
	printf( "PHYID2= 0x%04x\n", V_PHYID2>>shift_bit);


	printf( "\nread phyid1\n" );
	j=0;
	for(i=0; i<V_TEST_TIME; i++)
	{
	    mask = 0xffff;
	    reg = NFBI_REG_PHYID1;
	    param = reg << 16; //put register address to high word
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) 
		{
	        //printf("0x%04x\n", param&mask);
	        if( (param&mask)!=(V_PHYID1>>shift_bit)) //0x1c ==> 0xe
	        {
	        	j++;
	        	printf( "i=%d error phyid1=0x%04x\n", i, param&mask );
	        }
	    }else{
	        printf("ioctl NFBI_IOCTL_REGREAD phyid1 error\n");
	    }
		//if( (i&0xff)==0x0 )
		//	printf( "i=%x\n", i );
	}
	printf("error count=%d\n\n", j);


	printf( "\nread phyid2\n" );
	j=0;
	for(i=0; i<V_TEST_TIME; i++)
	{
	    mask = 0xffff;
	    reg = NFBI_REG_PHYID2;
	    param = reg << 16; //put register address to high word
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) 
		{
	        //printf("0x%04x\n", param&mask);
	        if( (param&mask)!=(V_PHYID2>>shift_bit)) //0xcb81 ==> 0x65c0
	        {
	        	j++;
	        	printf( "i=%d error phyid2=0x%04x\n", i, param&mask );
	        }
	    }else{
	        printf("ioctl NFBI_IOCTL_REGREAD phyid2 error\n");
	    }
		//if( (i&0xff)==0x0 )
		//	printf( "i=%x\n", i );
	}
	printf("error count=%d\n\n", j);


	printf( "\nread phyid1 & phyid2\n" );
	j=0;
	for(i=0; i<V_TEST_TIME; i++)
	{
	    mask = 0xffff;
	    reg = NFBI_REG_PHYID1;
	    param = reg << 16; //put register address to high word
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) 
		{
	        //printf("0x%04x\n", param&mask);
	        if( (param&mask)!=(V_PHYID1>>shift_bit)) //0x1c ==> 0xe
	        {
	        	j++;
	        	printf( "i=%d error phyid1=0x%04x\n", i, param&mask );
	        }
	    }else{
	        printf("ioctl NFBI_IOCTL_REGREAD phyid1 error\n");
	    }


	    mask = 0xffff;
	    reg = NFBI_REG_PHYID2;
	    param = reg << 16; //put register address to high word
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) 
		{
	        //printf("0x%04x\n", param&mask);
	        if( (param&mask)!=(V_PHYID2>>shift_bit)) //0xcb81 ==> 0x65c0
	        {
	        	j++;
	        	printf( "i=%d error phyid2=0x%04x\n", i, param&mask );
	        }
	    }else{
	        printf("ioctl NFBI_IOCTL_REGREAD phyid2 error\n");
	    }

		//if( (i&0xff)==0x0 )
		//	printf( "i=%x\n", i );
	}
	printf("error count=%d\n\n", j);

	return 0;
}


int cmd_downloadtest(int argc, char *argv[])
{
	int single_way=0;
	if(argc==3)
	{
		if(strcmp(argv[2],"single")==0)
			single_way=1;
	}
	
    if (0 == bootcode_download_test(single_way, "/etc/fw/boot.bin"))
        return 1;
    else
        return -1;
}


/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
//nfbi memread 0x00008000 <phyaddr>
int cmd_memread(int argc, char *argv[])
{
    int addr, val;

    if ((argc != 3) && (argc !=4))
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format
    if (argc==4) {//set phyaddr 
	int tmp = atoi(argv[3]) + 5 ; 
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp); 
	}
    if (0 == nfbi_mem32_read(addr, &val)) {
        printf("0x%08x\n", val);
	    return 0;
	}
	else
	    return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
//nfbi memwrite <addr> <value> <phyaddr>
int cmd_memwrite(int argc, char *argv[])
{
    int addr, val;

    if ((argc != 4) && (argc != 5))
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
    if (argc ==5 ) {
	int tmp = atoi(argv[4]) + 5 ;	
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp); 
    }
    if (0 == nfbi_mem32_write(addr, val))
	    return 1;
	else
	    return -1;
}

//nfbi regread <name> <mask> <phyaddr>
int cmd_regread(int argc, char *argv[])
{
    int reg;
    int param, mask;

    if (argc < 3) {
        print_regread_list();
        return 0;
    }

    if (argc==3)
        //nfbi regread <name>
        mask = 0xffff;
    else {
        //nfbi regread <name> <mask>
        // parsing <mask>
        if (0 == strncmp(argv[3], "0x", 2)) {
            mask = _atoi(argv[3]+2 ,16);
            mask &= 0xffff;
	    if (argc==5) {  
		int tmp = atoi(argv[4]) + 5 ;
		ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp);
	    }
        }
	//nfbi regread <name> <phyaddr>
	else if ( (atoi(argv[3])) > 0 && (atoi(argv[3])) < 4 ) {
		int tmp = atoi(argv[3]) + 5 ;
		mask = 0xffff;
		ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp);
	}
        else return -1; //wrong command format
    }
    // parsing <name>
    reg = regname2val(argv[2], nfbi_reg_table);
    if (reg < 0) {
        return -1; //wrong command format
    }

    param = reg << 16; //put register address to high word
    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
        printf("0x%04x\n", param&mask);
        return 0;
    }
    else
        return -1;
}

//nfbi regwrite <name> <value> <phyaddr>
int cmd_regwrite(int argc, char *argv[])
{
    int reg, val, index;
    int param, mask;

    if (argc < 4) {
        print_regwrite_list();
        return 0;
    }
    if (argc==4) {
        //nfbi regwrite <name> <value>
        index = 3; //fourth argument is <value>
        mask = 0xffff;
    }
    else {
	if (argc==5) {
		//nfbi regwrite <name> <mask> <value>
		if(0 == strncmp(argv[4], "0x", 2))  //value
		{
			index =4;
        		if (0 == strncmp(argv[3], "0x", 2)) {
		            mask = _atoi(argv[3]+2 ,16);
		            mask &= 0xffff;
		        }
			else return -1;	
		}
		//nfbi regwrite <name> <value> <phyaddr>
		else if ( (atoi(argv[4])) > 0 && (atoi(argv[4])) < 4 )
		{
			int tmp = atoi(argv[4])+5;
			index = 3;
			mask = 0xffff;
			ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp);
		}
		else return -1;
	}
	else if (argc==6) {
		//nfbi regwrite <name> <mask> <value> <phyaddr>
		int tmp = argv[5]+5;
		index = 4;
        	if (0 == strncmp(argv[3], "0x", 2)) {
	            mask = _atoi(argv[3]+2 ,16);
	            mask &= 0xffff;
	        }
		else return -1;
		ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &tmp);
	}
	else return -1;
    }
    // parsing <name>
    reg = regname2val(argv[2], nfbi_reg_table);
    if (reg < 0) {
        return -1; //wrong command format
    }
    // parsing <value>
    if (0 == strncmp(argv[index], "0x", 2)) {
        val = _atoi(argv[index]+2 ,16);
        val &= 0xffff;
    }
    else {
        return -1; //wrong command format
    }

    // read register first for the 1st kind of command
    if (mask != 0xffff) {
        param = reg << 16; //put register address to high word
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
            val = ((param&0xffff)&(~mask)) | (val&mask);
        }
        else
            return -1;
    }

    //put register address to high word and the value to low word
#if 0
    if (reg == NFBI_REG_BMCR)
        val &= 0x7dff; //filter out reset bit and restart auto nego bit
#endif
    param = (reg << 16) | (val & 0xffff);
    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
            return 1;
        else
            return -1;
}
	


int cmd_bootdownload(int argc, char *argv[])
{
    if (argc < 4)
        return -1;
    if (0 == bootcode_download(atoi(argv[2]), argv[3]))
        return 1;
    else
        return -1;
}


int cmd_bootdownload_ip(int argc, char *argv[])
{		
	int ip_addr;
    if (argc < 4)
        return -1;	
	
	ip_addr = inet_addr(argv[2]); //ip
	if(ip_addr == -1) return -1;
	bootparam_set_ip(ip_addr);
	//printf("ip_addr=%s\n",argv[2]);

    if (0 == bootcode_download(0, argv[3]))
        return 1;
    else
        return -1;
}

//nfbi bootdownload_w_ipmac 00e04c867701 192.168.1.17 /etc/fw/boot.bin
 int cmd_bootdownload_ipmac(int argc, char *argv[])
 {		 
	 int ip_addr;
	 unsigned char mac[6];

	 if (argc!=5) return -1;  

	 //mac
	 if( mac_hexstr2bin( argv[2], mac )<0 )
		 return -1;
	 bootparam_set_mac( mac );

	 //ip
	 ip_addr = inet_addr(argv[3]);
	 if(ip_addr == -1) return -1;
	 bootparam_set_ip(ip_addr); 
	 //printf("ip_addr=%s\n",argv[2]);
 
	 if (0 == bootcode_download(0, argv[4]))
		 return 1;
	 else
		 return -1;
 }
   

#if 0  
 int cmd_fwdownload_w_boot(int argc, char *argv[])
 {
     int retval;

     if (argc < 5) return -1;
     retval = firmware_download_w_boot(atoi(argv[2]), argv[3], argv[4]);
     if (0 == retval)
         return 1;
     else {
         printf("retval=%d\n", retval);
         return -1;
     }
 }

 int cmd_fwdownload_w_boot_ip(int argc, char *argv[])
 {
     int retval;
	 int ip_addr;

	 ip_addr = inet_addr(argv[2]); //ip
	 if(ip_addr == -1) return -1;

     if (argc < 5) return -1;
     retval = firmware_download_w_boot(0, argv[3], argv[4]);
     if (0 == retval)
         return 1;
     else {
         printf("retval=%d\n", retval);
         return -1;
     }
 }
#endif

//nfbi tftp_fw_w_boot 00e04c867701 192.168.1.17 255.255.255.0 192.168.1.1 /etc/fw/boot.bin /etc/fw/linux.bin 
int cmd_tftp_fw_w_boot(int argc, char *argv[])
{
	int retval;
	int ip_addr, ip_mask, sip_addr;
	unsigned char mac[6];

	printf( "nfbi start time %d\n", (int)time(NULL) );
	if (argc!=8) return -1;

	//mac
	if( mac_hexstr2bin( argv[2], mac )<0 )
		return -1;
	bootparam_set_mac( mac );

	//ip
	ip_addr = inet_addr(argv[3]); 
    if(ip_addr == -1) return -1;
	bootparam_set_ip(ip_addr);

	//mask
	ip_mask = inet_addr(argv[4]); 
    if(ip_mask == -1) return -1;
	bootparam_set_ipmask(ip_mask);

	//tftp server ip
	sip_addr = inet_addr(argv[5]); 
    if(sip_addr == -1) return -1;
	bootparam_set_serverip(sip_addr);


	retval = firmware_download_w_boot(0, argv[7], argv[6]);
	if (0 == retval)
	{
		//unsigned char b[14], bhex[32];

		//after download boot, IMR=0 => reset to default value.
		nfbi_register_write(NFBI_REG_IMR, NFBI_DEF_IMR_INT);
		#if 0	/*use nfbi msg channel, not udp */	
		memcpy(b, mac, 6);
		memcpy(b+6, &ip_addr, 4);
		memcpy(b+10, &ip_mask, 4);
		bin2hexstr( b, 14, bhex );
		printf( "set netinfo to slave:\n" );
		printf( "\tmac=%02x%02x%02x%02x%02x%02x\n", b[0],b[1],b[2],b[3],b[4],b[5] );
		printf( "\tip=%u.%u.%u.%u\n", b[6],b[7],b[8],b[9] );
		printf( "\tmask=%u.%u.%u.%u\n", b[10],b[11],b[12],b[13] );
		printf( "\thexstr=%s\n", bhex );
		nc_handle_setcmd( "netinfo", bhex );
		#endif
		printf( "nfbi end time %d\n", (int)time(NULL) );
		return 1;
	}else{
		printf( "failed, return code=%d\n", retval );
	}

	return -1;
}


//nfbi fw_w_boot 0 /etc/fw/boot.bin /etc/fw/linux.bin 
int cmd_fw_w_boot(int argc, char *argv[])
{
	int retval,i,param,retboot,slv_num;
	//auto detect slave number
	slv_num=cmd_testphyaddr(2,"/bin/nfbi testphyaddr");

	retboot=0;
	for (i=0;slv_num!=retboot;i++)
	{
		param=i+6;
		//modify nfbi_phy_addr
		ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
		nfbi_register_write(NFBI_REG_CMD,~BM_START_RUN_BOOTCODE);

		printf( "nfbi start time %d\n", (int)time(NULL) );
		if (argc!=5) return -1;

		retval = firmware_download_w_boot_nfbi( atoi(argv[2]), argv[4], argv[3]);
		if (0 == retval)
		{		
			//after download boot, IMR=0 => reset to default value.
			nfbi_register_write(NFBI_REG_IMR, NFBI_DEF_IMR_INT);
			printf( "nfbi end time %d\n", (int)time(NULL) );
			retboot++;
		}else{
			printf( "failed, return code=%d\n", retval );
		}
	}
	if (retboot==0) return -1;
	return retboot;
}

//nfbi downloadtest2 0 /etc/fw/boot.bin /etc/fw/linux.bin 
int cmd_downloadtest2(int argc, char *argv[])
{
	int retval;

	printf( "nfbi start time %d\n", (int)time(NULL) );
	if (argc!=5) return -1;

	retval = firmware_downloadtest( atoi(argv[2]), argv[4], argv[3]);
	if (0 == retval)
	{		
		//after download boot, IMR=0 => reset to default value.
		nfbi_register_write(NFBI_REG_IMR, NFBI_DEF_IMR_INT);
		printf( "nfbi end time %d\n", (int)time(NULL) );
		return 1;
	}else{
		printf( "failed, return code=%d\n", retval );
	}

	return -1;
}

//nfbi downloadtest3 0 /etc/fw/boot.bin /etc/fw/linux.bin 
//this test must use hardware reset pin
int cmd_downloadtest3(int argc, char *argv[])
{
	int i;
	
	while(1)
	{
		i++;
		printf( "\n\n%s: i=%d\n", __FUNCTION__, i );
		if( cmd_fw_w_boot(argc, argv)<0 )
			break;
		if(i==1500) break;
	}
	printf( "%s: done\n", __FUNCTION__ );
}

int cmd_tx_cmdword_interval(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if ((param<0) || (param>1000))
            return -1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_TX_CMDWORD_INTERVAL, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_TX_CMDWORD_INTERVAL, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}

int cmd_interrupt_timeout(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if ((param<0) || (param>1000))
            return -1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_INTERRUPT_TIMEOUT, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_INTERRUPT_TIMEOUT, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}

#if 0
int cmd_handshake_polling(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if (param != 0)
            param = 1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_CMD_HANDSHAKE_POLLING, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_CMD_HANDSHAKE_POLLING, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}
#endif

int cmd_dump_priv_data(int argc, char *argv[])
{
    int param;

    param = 4; //dump private data
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
	return 0;
}

int cmd_dump_eq(int argc, char *argv[])
{
    int ret;
    struct evt_msg evt;
    
    ret = nfbi_get_event_msg(&evt);
    while ((ret==0) && (evt.id==1)) {
        printf("evt.id=%d evt.value=%x evt.status=%x\n", evt.id, evt.value, evt.status);
        if (evt.status & IP_CHECKSUM_DONE) {
            if (evt.value & BM_CHECKSUM_DONE)
                printf("Checksum Done: 1\n");
            else
                printf("Checksum Done: 0\n");
        }
        if (evt.status & IP_CHECKSUM_OK) {
            if (evt.value & BM_CHECKSUM_OK)
                printf("Checksum OK: 1\n");
            else
                printf("Checksum OK: 0\n");
        }
        if (evt.status & IP_DSLLINK) {
            if (evt.value & BM_DSLLINK)
                printf("DSL link: up\n");
            else
                printf("DSLlink: down\n");
        }
        if (evt.status & IP_ALLSOFTWARE_READY) {
            if (evt.value & BM_ALLSOFTWARE_READY)
                printf("All Software: ready\n");
            else
                printf("All Software: not ready\n");
        }
        if (evt.status & IP_BOOTCODE_READY) {
            if (evt.value & BM_BOOTCODE_READY)
                printf("Bootcode: ready\n");
            else
                printf("Bootcode: not ready\n");
        }
        if (evt.status & IP_NEED_BOOTCODE) {
            printf("Need Bootcode\n");
        }
        ret = nfbi_get_event_msg(&evt);
    }
    return 0;
}

static int read_word(int addr, int *pval)
{
    struct nfbi_mem32_param param;
    
    param.addr = addr;
    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_DW, &param)) {
        *pval = param.val;
	    return 0;
	}
	else
	    return -1;
}

static int write_word(int addr, int val)
{
    struct nfbi_mem32_param param;
    
    param.addr = addr;
    param.val = val;
    return ioctl(nfbi_fd, NFBI_IOCTL_EW, &param);
}

int cmd_dw(int argc, char *argv[])
{
    int addr, val;

    if (argc != 3)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == read_word(addr, &val)) {
        printf("0x%08x\n", val);
	    return 0;
	}
	else
	    return -1;
}

int cmd_ew(int argc, char *argv[])
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

    if (0 == write_word(addr, val))
	    return 1;
	else
	    return -1;
}

int cmd_bulkmemwrite(int argc, char *argv[])
{
    int addr;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == nfbi_bulk_mem_write(addr, strlen(argv[3]), argv[3]))
	    return 1;
	else
	    return -1;
}

int cmd_bulkmemread(int argc, char *argv[])
{
    int addr, i, len;
    char tmp[128];

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format
    
    len = atoi(argv[3]);
    if (0 == nfbi_bulk_mem_read(addr, len, tmp)) {
        printf("=================================\n");
        for (i=0; i< len; i++) {
	        printf("%c", tmp[i]);
	    }
	    printf("\n");
	    printf("=================================\n");
	    return 0;
	}
	else
	    return -1;
}

#if 0
int cmd_setcmd(int argc, char *argv[])
{
    int ret;

    if (argc < 4) 
	{
        nc_print_cmd_list(NC_W);
        return 0;
    }

	ret=nc_handle_setcmd(argv[2], argv[3]);
	if(ret==0)
	    return 1;
	else
	    return -1;
}

int cmd_getcmd(int argc, char *argv[])
{
	int ret;

	if (argc < 3) 
	{
		nc_print_cmd_list(NC_R);
		return 0;
    }

	ret=nc_handle_getcmd(argv[2]);
	if (0 == ret)
		return 0;
	else
		return -1;
}
#endif

int cmd_version(int argc, char *argv[])
{
    printf("version=%s\n", NFBI_VERSION);
    return 0;
}

void print_command_list(void)
{
	int i;

	printf("\n==========commands for debugging============\n");
	i = 0;
	while (nfbi_cmd_table[i].cmd != NULL) 
	{
		if (!nfbi_cmd_table[i].debug)
		printf("%s\n", nfbi_cmd_table[i].msg);
		i++;
	}

	i = 0;
	printf("\n===========commands list===================\n");
	while (nfbi_cmd_table[i].cmd != NULL) 
	{
		if (nfbi_cmd_table[i].debug)
		printf("%s\n", nfbi_cmd_table[i].msg);
		i++;
	}
}

int main(int argc, char *argv[])
{
	int i, ret=-1;

	if (argc < 2)
	{
		print_command_list();
		return ret;
	}


	//open device file
	nfbi_fd = open(NFBI_DEV_NAME, O_RDWR); 
	if(-1 == nfbi_fd)
	{
		perror("open driver failed");
		return ret;
	}


	i = 0;
	while (nfbi_cmd_table[i].cmd != NULL) 
	{
		if( strcmp(argv[1], nfbi_cmd_table[i].cmd)==0 ) 
		{
			if (nfbi_cmd_table[i].func) 
			{
				ret = nfbi_cmd_table[i].func(argc, argv);
				if(ret>=0)
				{
					if(ret>0) printf("ret=%d,OK\n",ret);
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

	close(nfbi_fd);
	return ret;
}


