#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "aipc_util.h"

//#define AIPC_DEBUG

#ifdef AIPC_DEBUG
#define DPRINT(fmt, args...) fprintf(stderr , "%s(%d) "fmt, __FUNCTION__, __LINE__ ,## args)
#else
#define DPRINT( ... ) do { } while( 0 )
#endif

#define AIPC_DEV 		"/dev/aipc_dev"

#define OK    0
#define NOK   -1

//unsigned char read_buf[AIPC_BUF_SIZE] = {0};
//unsigned char *read_buf = NULL;

int display_help(void);

int aipc_boot_dsp(int dev_fd)
{
	return ioctl(dev_fd , IOCTL_BOOT_DSP);	
}

int aipc_unmap_dram(int dev_fd)
{	
	return ioctl(dev_fd , IOCTL_CPU_DRAM_UNMAP);
}

int aipc_dsp_entry(int dev_fd)
{	
	return ioctl(dev_fd , IOCTL_DSP_ENTRY);
}
				
int aipc_soc_sram_backup(int dev_fd)
{
	return ioctl(dev_fd , IOCTL_SOC_SRAM_BACKUP);
}

int aipc_zone_set(int dev_fd)
{	
	return ioctl(dev_fd , IOCTL_ZONE_SET);
}

int aipc_rom_set(int dev_fd , unsigned long rom_addr)
{	
	return ioctl(dev_fd , IOCTL_ROM_SET , rom_addr);
}

int aipc_soc_sram_set(int dev_fd , unsigned long sram_addr)
{	
	return ioctl(dev_fd , IOCTL_SOC_SRAM_SET , sram_addr);
}

int aipc_init_ipc(int dev_fd)
{	
	return ioctl(dev_fd , IOCTL_INIT_IPC);
}

int aipc_reset_slic(int dev_fd)
{
	return ioctl(dev_fd , IOCTL_RESET_SLIC);	
}

int aipc_mem_set(int dev_fd)
{
	return ioctl(dev_fd , IOCTL_DSP_SRAM_MAP);
}

int aipc_trigger(int dev_fd , int trigger)
{
	return ioctl(dev_fd , IOCTL_TRIGGER , trigger);
}

int aipc_dbg_print(int dev_fd , unsigned long dbg_print)
{
	return ioctl(dev_fd , IOCTL_DBG_PRINT , dbg_print);
}

int aipc_write_length(int dev_fd , void *dst_addr , void *src_addr , unsigned long length , int padding)
{
	ioctl(dev_fd , IOCTL_WF);
	ioctl(dev_fd , IOCTL_DST_ADDR  	, 	dst_addr);  //set write address
	ioctl(dev_fd , IOCTL_SRC_ADDR 	,  	src_addr);  //set src address
	ioctl(dev_fd , IOCTL_LENGTH 	,	length);  
	if (!padding)
		return ioctl(dev_fd , IOCTL_COPY);
	else
		return ioctl(dev_fd , IOCTL_COPY_PADDING);
}


int aipc_write_input(int dev_fd , int ifd , void *addr)
{
	off_t wc=0, rc=0, remain=0;
	void *da = addr;
	struct stat sb;
	int result = NOK;
	unsigned char *read_buf = NULL;

	if (fstat(ifd , &sb) == -1) {
    	perror("stat");
        exit(EXIT_FAILURE);
	}
	
	read_buf = (unsigned char *)malloc(AIPC_BUF_SIZE);
	if (read_buf==NULL) {
		return printf("aipc_util malloc failed.\n");
	}

	remain = sb.st_size;

	ioctl(dev_fd , IOCTL_WF);	

	while (remain > AIPC_BUF_SIZE){
		ioctl(dev_fd , IOCTL_DST_ADDR , da);  //set write address
		
		rc     +=  read(ifd     , read_buf , AIPC_BUF_SIZE);
		wc     +=  write(dev_fd , read_buf , AIPC_BUF_SIZE);
		da     +=  AIPC_BUF_SIZE;
		remain -=  AIPC_BUF_SIZE;
	}
	
	if (remain>0){
        ioctl(dev_fd , IOCTL_DST_ADDR , da);  //set write address

        rc     +=  read(ifd     , read_buf , remain);
        wc     +=  write(dev_fd , read_buf , remain);
        da     +=  AIPC_BUF_SIZE;
        remain -=  remain;
	}

	if( wc==rc ) {
		fprintf(stderr , "aipc: write image successful. size=%lld rc=%lld wc=%lld da=%p remain=%lld\n" , 
					sb.st_size , rc , wc , da , remain);
		result = OK;
	}
	else {
		fprintf(stderr , "aipc: write image failed. size=%lld rc=%lld wc=%lld da=%p remain=%lld\n", 
					sb.st_size , rc , wc , da , remain);
		result = NOK;
	}

	if( read_buf )
		free( read_buf );

	return result;
}

int aipc_write_word(int dev_fd , unsigned long word , void *addr)
{	
	ioctl(dev_fd , IOCTL_WW);	
	ioctl(dev_fd , IOCTL_DST_ADDR , addr);  //set write address
	return write(dev_fd , &word , sizeof(unsigned long));
}

int aipc_write_bitop(int dev_fd , unsigned long word , void *addr , BITOP op)
{
	switch(op){
	case BOP_AND:
		ioctl(dev_fd, IOCTL_BITOP_AND);
		break;
	case BOP_OR:
		ioctl(dev_fd, IOCTL_BITOP_OR);
		break;
	case BOP_XOR:
		ioctl(dev_fd, IOCTL_BITOP_XOR);
		break;
	case BOP_NOT:
		ioctl(dev_fd, IOCTL_BITOP_NOT);
		break;
	default:
		break;
	}
	
	ioctl(dev_fd , IOCTL_DST_ADDR , addr);  //set write address
	return write(dev_fd , &word , sizeof(unsigned long));
}

int aipc_read_ram(int ofd , int dev_fd , void *addr , unsigned long length)
{
	off_t rc=0 , wc=0;
	unsigned char *buf=NULL;
	int result = NOK;

	buf = (unsigned char *)malloc(length);
	if( buf==NULL ){
		fprintf(stderr , "read ram malloc failed\n" );
		return OK;
	}

	ioctl(dev_fd , IOCTL_DST_ADDR , addr);  //set read address
	
	rc = read(dev_fd , buf , length);
	wc = write(ofd ,buf , length);
	DPRINT("rc=%lld wc=%lld length=%lu\n" , rc , wc , length);
	
	if( wc==rc )
		result = OK;
	else
		result = NOK;	
	
	if( buf )
		free( buf );
	
	return result;
}

int aipc_read_put_cnsl(int dev_fd , void *addr , unsigned long length)
{
	off_t rc=0;
	int i=0;
	unsigned long *ulp = NULL;
	unsigned char *read_buf = NULL;
	
	read_buf = (unsigned char *)malloc(length);
	if (read_buf==NULL) {
		return printf("aipc_util malloc failed.\n");
	}

	ioctl(dev_fd , IOCTL_DST_ADDR , addr);  //set read address

	ulp = (unsigned long *)read_buf;

	rc = read(dev_fd , read_buf , length);

#if 1	//word display
	i=0;
	do{
		printf("%08lx ", ulp[i]);
		if((i+1)%4==0)
			printf("\n");
		++i;
	}while(i<(rc/4));
	printf("\n");
#endif

#if 0 //byte display	
	for(i=0 ; i<rc ; i++){
		printf( "%02x ", read_buf[i]);
		if((i+1)%32==0)
			printf("\n");
	}
	printf("\n");
#endif

	if( read_buf )
		free( read_buf );

	return OK;
}

void aipc_proc_ioctl_num(aipc_ioctl *ioctl_num , char *optarg)
{
	/*
	*	Control Plane
	*/
	if	(strstr(optarg , "IOCTL_CTRL_2DSP_SEND"))
		*ioctl_num = IOCTL_CTRL_2DSP_SEND;
	else if	(strstr(optarg , "IOCTL_CTRL_2DSP_POLL"))
		*ioctl_num = IOCTL_CTRL_2DSP_POLL;
	else if	(strstr(optarg , "IOCTL_CTRL_2CPU_SEND"))
		*ioctl_num = IOCTL_CTRL_2CPU_SEND;
	else if	(strstr(optarg , "IOCTL_CTRL_2CPU_POLL"))
		*ioctl_num = IOCTL_CTRL_2CPU_POLL;

	/*
	*	Data Plane
	*/
	else if	(strstr(optarg , "IOCTL_MBOX_2DSP_SEND"))
		*ioctl_num = IOCTL_MBOX_2DSP_SEND;
	else if	(strstr(optarg , "IOCTL_MBOX_2DSP_POLL"))
		*ioctl_num = IOCTL_MBOX_2DSP_POLL;
	else if	(strstr(optarg , "IOCTL_MBOX_2CPU_SEND"))
		*ioctl_num = IOCTL_MBOX_2CPU_SEND;
	else if	(strstr(optarg , "IOCTL_MBOX_2CPU_RECV"))
		*ioctl_num = IOCTL_MBOX_2CPU_RECV;

	/*
	*	Debug function
	*/
	else if	(strstr(optarg , "IOCTL_DBG_PRINT"))
		*ioctl_num = IOCTL_DBG_PRINT;
	else if (strstr(optarg , "IOCTL_DBG_DUMP"))
		*ioctl_num = IOCTL_DBG_DUMP;

	/*
	*	MUTEX
	*/
	else if	(strstr(optarg , "IOCTL_IPC_MUTEX_LOCK"))
		*ioctl_num = IOCTL_IPC_MUTEX_LOCK;
	else if (strstr(optarg , "IOCTL_IPC_MUTEX_TRYLOCK"))
		*ioctl_num = IOCTL_IPC_MUTEX_TRYLOCK;
	else if	(strstr(optarg , "IOCTL_IPC_MUTEX_UNLOCK"))
		*ioctl_num = IOCTL_IPC_MUTEX_UNLOCK;
	else if (strstr(optarg , "IOCTL_IPC_MUTEX_OWN"))
		*ioctl_num = IOCTL_IPC_MUTEX_OWN;
		
	/*
	*	Wrong setting
	*/
	else{
		*ioctl_num = IOCTL_BASE; //Not found
		printf( "wrong ioctl number\n");
	}
}

void aipc_proc_ioctl_flag(unsigned int *ioctl_flag, char *optarg)
{
	/*
	*	Control Plane
	*/
	if	(strstr(optarg , "CTRL_2DSP_SEND_NR"))
		*ioctl_flag &= ~CTRL_2DSP_SEND_NR;
	else if(strstr(optarg , "CTRL_2DSP_SEND_START"))
		*ioctl_flag |= CTRL_2DSP_SEND_START;
	else if(strstr(optarg , "CTRL_2DSP_SEND_STOP"))
		*ioctl_flag |= CTRL_2DSP_SEND_STOP;
	else if(strstr(optarg , "CTRL_2DSP_POLL_NR"))
		*ioctl_flag &= ~CTRL_2DSP_POLL_NR;
	else if(strstr(optarg , "CTRL_2DSP_POLL_START"))
		*ioctl_flag |= CTRL_2DSP_POLL_START;
	else if(strstr(optarg , "CTRL_2DSP_POLL_STOP"))
		*ioctl_flag |= CTRL_2DSP_POLL_STOP;
	else if(strstr(optarg , "CTRL_2CPU_SEND_NR"))
		*ioctl_flag &= ~CTRL_2CPU_SEND_NR;
	else if(strstr(optarg , "CTRL_2CPU_SEND_START"))
		*ioctl_flag |= CTRL_2CPU_SEND_START;
	else if(strstr(optarg , "CTRL_2CPU_SEND_STOP"))
		*ioctl_flag |= CTRL_2CPU_SEND_STOP;
	else if(strstr(optarg , "CTRL_2CPU_POLL_NR"))
		*ioctl_flag &= ~CTRL_2CPU_POLL_NR;
	else if(strstr(optarg , "CTRL_2CPU_POLL_START"))
		*ioctl_flag |= CTRL_2CPU_POLL_START;
	else if(strstr(optarg , "CTRL_2CPU_POLL_STOP"))
		*ioctl_flag |= CTRL_2CPU_POLL_STOP;

	/*
	*	Data Plane
	*/
	else if(strstr(optarg , "MBOX_2DSP_SEND_NR"))
		*ioctl_flag &= ~MBOX_2DSP_SEND_NR;
	else if(strstr(optarg , "MBOX_2DSP_SEND_START"))
		*ioctl_flag |= MBOX_2DSP_SEND_START;
	else if(strstr(optarg , "MBOX_2DSP_SEND_STOP"))
		*ioctl_flag |= MBOX_2DSP_SEND_STOP;
	else if(strstr(optarg , "MBOX_2DSP_POLL_NR"))
		*ioctl_flag &= ~MBOX_2DSP_POLL_NR;
	else if(strstr(optarg , "MBOX_2DSP_POLL_START"))
		*ioctl_flag |= MBOX_2DSP_POLL_START;
	else if(strstr(optarg , "MBOX_2DSP_POLL_STOP"))
		*ioctl_flag |= MBOX_2DSP_POLL_STOP;
	else if(strstr(optarg , "MBOX_2CPU_SEND_NR"))
		*ioctl_flag &= ~MBOX_2CPU_SEND_NR;
	else if(strstr(optarg , "MBOX_2CPU_SEND_START"))
		*ioctl_flag |= MBOX_2CPU_SEND_START;
	else if(strstr(optarg , "MBOX_2CPU_SEND_STOP"))
		*ioctl_flag |= MBOX_2CPU_SEND_STOP;
	else if(strstr(optarg , "MBOX_2CPU_RECV_NR"))
		*ioctl_flag &= ~MBOX_2CPU_RECV_NR;
	else if(strstr(optarg , "MBOX_2CPU_RECV_START"))
		*ioctl_flag |= MBOX_2CPU_RECV_START;
	else if(strstr(optarg , "MBOX_2CPU_RECV_STOP"))
		*ioctl_flag |= MBOX_2CPU_RECV_STOP;


	/*
	*	Debug function
	*/
	else if(strstr(optarg , "DBG_NONE"))
		*ioctl_flag = DBG_NONE;
	else if(strstr(optarg , "DBG_INTQ"))
		*ioctl_flag |= DBG_INTQ;
	else if(strstr(optarg , "DBG_MBOX"))
		*ioctl_flag |= DBG_MBOX;
	else if(strstr(optarg , "DBG_CTRL"))
		*ioctl_flag |= DBG_CTRL;
	else if(strstr(optarg , "DBG_ALL"))
		*ioctl_flag |= DBG_ALL;


	else if(strstr(optarg , "DUMP_NONE"))
		*ioctl_flag = DUMP_NONE;
	else if(strstr(optarg , "DUMP_CTRL"))
		*ioctl_flag |= DUMP_CTRL;
	else if(strstr(optarg , "DUMP_MBOX"))
		*ioctl_flag |= DUMP_MBOX;
	else if(strstr(optarg , "DUMP_INTQ"))
		*ioctl_flag |= DUMP_INTQ;

	/*
	*	MUTEX
	*/
	else if(strstr(optarg , "PROCESSOR_ID_CPU"))
		*ioctl_flag = PROCESSOR_ID_CPU;
	else if(strstr(optarg , "PROCESSOR_ID_DSP"))
		*ioctl_flag = PROCESSOR_ID_DSP;

	/*
	*	Wrong setting
	*/
	else{
		*ioctl_flag = IOCTL_FLAG_MAX;
		printf( "wrong ioctl flag. set as maximum\n");
	}
}

int aipc_send_ioctl(int dev_fd , aipc_ioctl ioctl_num , unsigned int ioctl_flag)
{
	return ioctl(dev_fd , ioctl_num , ioctl_flag);
}

int main( int argc, char *argv[])
{
	int wr=0 , rd=0;
	int dev_fd=0, ifd=0 , ofd=0;
	int padding=0;
	int trigger=0;
	void *dst_addr=NULL;
	void *src_addr=NULL;
	const char *input=NULL; 
	const char *output=NULL;
	unsigned long length=0;
	unsigned long word=0, and=0, or=0, xor=0 , not=0;
	unsigned long op_word=0, op_and=0, op_or=0, op_xor=0 , op_not=0;
	unsigned long rom_addr=0;
	unsigned int  sram_addr=0;
	unsigned long dbg_print=0;
	int result = NOK;
	
	aipc_ioctl	  ioctl_num  = IOCTL_BASE;
	unsigned int  ioctl_flag = 0;
	

	if (argc==1)
		goto fail;

	dev_fd = open(AIPC_DEV , O_RDWR);
	if (dev_fd<0){
		printf("open aipc device failed. errno=%d\n" , errno);
		goto fail;
	}

    for (;;) {
        int option_index = 0;
        static const char *short_options = "hwrbupzeBmISR:M:d:s:i:l:o:W:A:O:X:N:t:D:C:F:";
        static const struct option long_options[] = {
            {"help", 		no_argument, 		0, 'h'},
            {"write", 		no_argument, 		0, 'w'},
            {"read", 		no_argument, 		0, 'r'},
            {"boot_dsp", 	no_argument, 		0, 'b'},
            {"unmap_dram", 	no_argument, 		0, 'u'},
            {"padding", 	no_argument, 		0, 'p'},
            {"zone", 		no_argument, 		0, 'z'},
            {"entry", 		no_argument, 		0, 'e'},
            {"sram_backup",	no_argument, 		0, 'B'},
            {"mem",			no_argument, 		0, 'm'},
            {"init_ipc",	no_argument, 		0, 'I'},
            {"slic_reset",	no_argument, 		0, 'S'},
            {"rom",			required_argument,	0, 'R'},
            {"soc_sram",    required_argument, 	0, 'M'},
            {"dst_addr",	required_argument, 	0, 'd'},
            {"src_addr",	required_argument, 	0, 's'},
            {"input",		required_argument, 	0, 'i'},
            {"output",		required_argument, 	0, 'o'},
            {"length",		required_argument, 	0, 'l'},
            {"word",		required_argument, 	0, 'W'},
            {"and",			required_argument, 	0, 'A'},
            {"or",			required_argument, 	0, 'O'},
            {"xor",			required_argument, 	0, 'X'},
            {"not",			required_argument, 	0, 'N'},
            {"trigger",		required_argument, 	0, 't'},
            {"dbg_print",	required_argument, 	0, 'D'},
            {"ioctl_num",	required_argument, 	0, 'C'},
            {"ioctl_flag",	required_argument, 	0, 'F'},
            {0, 0, 0, 0},
        };

        int c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == EOF) {
            break;
        }

        switch (c) {
			case 'h':
				goto fail;

            case 'w':
				wr = 1;
                break;

            case 'r':
				rd = 1;
                break;

            case 'b':
				aipc_boot_dsp(dev_fd);
				goto success;

            case 'u':
				aipc_unmap_dram(dev_fd);
				goto success;

            case 'p':
				padding=1;
                break;

            case 'z':
				aipc_zone_set(dev_fd);
				goto success;

            case 'e':
				aipc_dsp_entry(dev_fd);
				goto success;

            case 'B':
				aipc_soc_sram_backup(dev_fd);
				goto success;

            case 'R':
				if (strstr(optarg,"0x")){
					rom_addr = strtoul(optarg , '\0' , 16);
				}
				else{
					rom_addr = strtoul(optarg , '\0' , 10);
				}
				
				if (!rom_addr){
					printf("Wrong rom address\n");
					goto fail;
					}

				aipc_rom_set(dev_fd , rom_addr);
				goto success;

            case 'M':
				if (strstr(optarg,"0x")){
					sram_addr = strtoul(optarg , '\0' , 16);
				}
				else{
					sram_addr = strtoul(optarg , '\0' , 10);
				}
				
			#if 0 // remove this check to kernel space	
				if (!sram_addr){
					printf("Wrong sram address\n");
					goto fail;
					}
			
				if (sram_addr & 0x00007fff){
					printf("Input is not 32K aligned\n");
					goto fail;
					}
			#endif

				aipc_soc_sram_set(dev_fd , sram_addr);
				goto success;

			case 'I':
				aipc_init_ipc(dev_fd);
				goto success;

			case 'S':
				aipc_reset_slic(dev_fd);
				goto success;

            case 'm':
				aipc_mem_set(dev_fd);
				goto success;

            case 'd':
				if (strstr(optarg,"0x")){
					dst_addr = (void *)strtoul(optarg , '\0' , 16);
				}
				else{
					dst_addr = (void *)strtoul(optarg , '\0' , 10);
				}
				if (!dst_addr){
					printf("Wrong address asignment\n");
					goto fail;
					}
				DPRINT ("dst_addr=%p\n" , dst_addr);
				break;

            case 's':
				if (strstr(optarg,"0x")){
					src_addr = (void *)strtoul(optarg , '\0' , 16);
				}
				else{
					src_addr = (void *)strtoul(optarg , '\0' , 10);
				}
				if (!src_addr){
					printf("Wrong address asignment\n");
					goto fail;
					}
				DPRINT ("src_addr=%p\n" , src_addr);
				break;

            case 'i':
                input = optarg;
				ifd = open(input , O_RDONLY);
				if (ifd<0){
					printf("Wrong file name\n");
					goto fail;
					}
				DPRINT("input=%s\n" , input);
				break;

            case 'o':
                output = optarg;
				ofd = open(output , O_RDWR|O_CREAT);
				if (ofd<0){
					printf("Wrong output file name\n");
					goto fail;
					}
				DPRINT("output=%s\n" , output);
				break;

            case 'l':
                if (strstr(optarg,"0x")){
					length = strtoul(optarg , '\0' , 16);
				}
				else{
					length = strtoul(optarg , '\0' , 10);
				}
				if (!length){
					printf("Wrong length\n");
					goto fail;
					}
				DPRINT("length=%lu\n" , length);
				break;

			case 'W':
				if (strstr(optarg,"0x")){
					word = strtoul(optarg , '\0' , 16);
				}
				else{
					word = strtoul(optarg , '\0' , 10);
				}
				op_word=1;
				DPRINT("word=%lu\n" , word);
				break;

			case 'A':
				if (strstr(optarg,"0x")){
					and = strtoul(optarg , '\0' , 16);
				}
				else{
					and = strtoul(optarg , '\0' , 10);
				}
				op_and=1;
				DPRINT("and=%lu\n" , and);
				break;

			case 'O':
				if (strstr(optarg,"0x")){
					or = strtoul(optarg , '\0' , 16);
				}
				else{
					or = strtoul(optarg , '\0' , 10);
				}
				op_or=1;
				DPRINT("or=%lu\n" , or);
				break;

			case 'X':
				if (strstr(optarg,"0x")){
					xor = strtoul(optarg , '\0' , 16);
				}
				else{
					xor = strtoul(optarg , '\0' , 10);
				}
				op_xor=1;
				DPRINT("xor=%lu\n" , xor);
				break;

			case 'N':
				if (strstr(optarg,"0x")){
					not = strtoul(optarg , '\0' , 16);
				}
				else{
					not = strtoul(optarg , '\0' , 10);
				}
				op_not=1;
				break;

			case 't':
				if (strstr(optarg,"0x")){
					trigger = strtoul(optarg , '\0' , 16);
				}
				else{
					trigger = strtoul(optarg , '\0' , 10);
				}
                aipc_trigger(dev_fd , trigger);
                goto success;

			case 'D':
				if (strstr(optarg,"0x")){
					dbg_print = strtoul(optarg , '\0' , 16);
				}
				else{
					dbg_print = strtoul(optarg , '\0' , 10);
				}
                aipc_dbg_print(dev_fd , dbg_print);
                goto success;
            
            case 'C':
            	aipc_proc_ioctl_num( &ioctl_num , optarg );
            	break;
            	
            case 'F':
            	aipc_proc_ioctl_flag( &ioctl_flag , optarg );
            	break;

			case '?':
			default:
				break;
			}
		}
	if (ioctl_num){
		result = aipc_send_ioctl(dev_fd , ioctl_num , ioctl_flag );
	}
	else if (dst_addr){
		if (wr){
			if (ifd>0){
				result = aipc_write_input(dev_fd , ifd , dst_addr);
				sync();
			}
			else if (length && src_addr){
				result = aipc_write_length(dev_fd , dst_addr , src_addr , length , padding);
				sync();
			}
			else if (op_word){	
				result = aipc_write_word(dev_fd , word , dst_addr);
				sync();
			}
			else if (op_and || op_or || op_xor || op_not){
				if (op_and)
					result = aipc_write_bitop(dev_fd , and , dst_addr , BOP_AND);
				if (op_or)
					result = aipc_write_bitop(dev_fd , or  , dst_addr , BOP_OR);
				if (op_xor)
					result = aipc_write_bitop(dev_fd , xor , dst_addr , BOP_XOR);
				if (op_not)
					result = aipc_write_bitop(dev_fd , not , dst_addr , BOP_NOT);
				
				DPRINT("\n");
			}
			else{
				printf("unknown operation\n");
				goto fail;
			}
		}else if (rd){
			if ((ofd>0) && length){
				result = aipc_read_ram(ofd , dev_fd , dst_addr , length);
				sync();
			}
			else if (length){
				result = aipc_read_put_cnsl(dev_fd , dst_addr , length);
				sync();
			}
			else{
				result = aipc_read_put_cnsl(dev_fd , dst_addr , sizeof(unsigned long));
				sync();
			}
		}else{
			display_help();
			printf("unknown operation\n");
			goto fail;
		}
	}
	else{
		printf("wrong setting\n");
		goto fail;
	}

	if(result==NOK)
		goto fail;

success:
	//free fd
	if(dev_fd) close(dev_fd);
	if(ifd)    close(ifd);
	if(ofd)    close(ofd);
	printf("\n");
	return OK;

fail:
	display_help();
	//free fd
	if(dev_fd) close(dev_fd);
	if(ifd)    close(ifd);
	if(ofd)    close(ofd);
	printf("\n");
	return NOK;
}


int display_help(void)
{
	printf("\nWrite operation:\n");
	printf("  aipc_util -w -d dst_address [-i input | -W word | -A word | -O word | -X word | -A word -O word |-A word -O word -X word]\n");
	printf("\nRead operation:\n");
	printf("  aipc_util -r -d dst_address [-o output] [-l length]\n");

	printf("\nParameters:\n");
	printf("-w,  --write    write to kernel\n");
	printf("-r,  --read     read from kernel\n");
	printf("-d,  --dst_addr specify dst_address\n");
	printf("-s,  --src_addr specify src_address\n");
	printf("-i,  --input    input file\n");
	printf("-o,  --output   output as file\n");
	printf("-l,  --length   specify length\n");
	printf("-W,  --word     set word value\n");
	printf("-A,  --and      and with a word\n");
	printf("-O,  --or       or with a word\n");
	printf("-X,  --xor      xor with a word\n");
	printf("-N,  --not      not with a word\n");

	printf("\nExample:\n");
	printf("Write operation\n");
	printf("aipc_util -w -d 0xb0000000 -s 0xbd510000 -l 0x10000		//no add padding in tail\n");
	printf("aipc_util -w -d 0xb0000000 -W 0x01       //write 1 word 0x1 to dst_address 0xb0000000\n");

	printf("\nBit operation\n");
	printf("aipc_util -w -d 0xb0000000 -A 0x01\n");
	printf("aipc_util -w -d 0xb0000000 -O 0x10\n");
	printf("aipc_util -w -d 0xb0000000 -X 0x01\n");
	printf("aipc_util -w -d 0xb0000000 -A 0x01 -O 0x10 -X 0x01\n");

	printf("\nRead operation\n");
	printf("aipc_util -r -d 0xb0000000 -l 0x800000              //read 8MB from 0xb0000000 and display in console\n");
	printf("aipc_util -r -d 0xb0000000                          //read 1 word from 0xb0000000 and display in console \n");

	printf("\nMisc\n");
	printf("aipc_util -R rom_addr   //setup ROM\n");
	printf("aipc_util -M sram_addr  //sram_addr must 32K align\n");
	return OK;
}

