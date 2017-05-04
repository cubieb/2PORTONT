#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>


// ioctl command called by protocols (system-independent)
#define PTM_MAGIC			(('R'+'T'+'L'+'P'+'T'+'M') << 8)  
#define PTM_SET_DEFAULT_TABLE	(PTM_MAGIC+1)
#define PTM_GET_TABLE			(PTM_MAGIC+2)
#define PTM_READ_DATA			(PTM_MAGIC+3)
#define PTM_WRITE_DATA			(PTM_MAGIC+4)
#define PTM_SET_HW				(PTM_MAGIC+5)
#define PTM_GET_QMAP			(PTM_MAGIC+6)
#define PTM_STOP_HW				(PTM_MAGIC+7)
#define PTM_START_HW			(PTM_MAGIC+8)
#define PTM_CLEAN_WANIF			(PTM_MAGIC+9)
#define PTM_CLEAN_QMAP			(PTM_MAGIC+10)
#define PTM_SET_WANIF			(PTM_MAGIC+11)
#define PTM_SET_QMAP			(PTM_MAGIC+12)
#define PTM_SET_STAGTYPE		(PTM_MAGIC+13)
#define PTM_SET_DUMMYVID		(PTM_MAGIC+14)
#define PTM_SET_QMAPATSTAG		(PTM_MAGIC+15)
#define PTM_SET_MODE			(PTM_MAGIC+16)
#define PTM_SET_SYSTEM			(PTM_MAGIC+17)


typedef struct {
	unsigned char cmd;
	union{
		struct{
			unsigned int cmd2;
			unsigned int cmd3;
			unsigned int cmd4;
		};
		struct{
			unsigned short a1;
			unsigned short a2;
			unsigned char a3[8];
		};
		unsigned char arg[12];
	};
}ptm_arg;


int s2i(char  *str_P)
{
    unsigned long  val;

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


static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;

	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static ptmctl_help(void)
{
	printf( "ptmctl usage:\n" );
	printf( "settbl_default\n" );
	printf( "rdata <addr> <len>\n" );
	printf( "wdata <addr> <value>\n" );
	printf( "set_hw\n" );
	printf( "    non-bonding or <none>\n" );
	printf( "    bonding <line number>\n" );
	printf( "stop_hw\n" );
	printf( "start_hw\n" );
	printf( "gettbl: get all wan interface\n" );
	printf( "clean_wanif: clean all wan interface\n" );
	printf( "set_wanif <idx> <svid> <cvid> <mac>\n" );
	printf( "get_qmap: get all qmap\n" );
	printf( "clean_qmap: clean all qmap\n" );
	printf( "set_qmap <idx> <map>\n" );
	printf( "set_stagtype <stag type value>\n" );
	printf( "set_dummyvid <vid>\n");
	printf( "set_qmapatstag <tx> <rx>\n" );
	printf( "set_mode <phy/route>\n" );
	printf( "\n" );
}

int main(int argc, char *argv[])
{
	FILE *fp=NULL;
	char	*procdevice;
	ptm_arg myarg;
	int i;

	procdevice = "/dev/ptm";

	if(argc<2)
		goto arg_err_rtn;

	if ((fp = fopen(procdevice, "r")) == NULL) {
		   fprintf(stderr, "ERROR: failed to open %s, errno=%d\n",  procdevice, errno);
		   exit(0);
	};
	
	memset(&myarg, 0, sizeof(ptm_arg));

	if (strcmp(argv[1],"settbl_default")==0) {

		if (ioctl(fileno(fp), PTM_SET_DEFAULT_TABLE, &myarg) < 0) {
			printf("PTM set table failed %x %x!\n", PTM_SET_DEFAULT_TABLE, myarg.cmd);
		}

	}else if(strcmp(argv[1],"gettbl")==0) {

		if (ioctl(fileno(fp), PTM_GET_TABLE, &myarg) < 0) {
			printf("PKTA turns off failed %x %x!\n", PTM_GET_TABLE, myarg.cmd);
		}
	}else if(strcmp(argv[1],"rdata")==0) {

		if (argc!=4) {
			printf("start argument number error!\n");
		}else{ 
			myarg.cmd2 = s2i(argv[2]);
			myarg.cmd3 = s2i(argv[3]);
			
			//printf("[%s, line %d] myarg.cmd2 = %d, myarg.cmd3 = %d\n",
			//	__func__,__LINE__,myarg.cmd2, myarg.cmd3);
		
			if (ioctl(fileno(fp), PTM_READ_DATA, &myarg) < 0) {
				printf("PTM read data failed %x %x!\n", PTM_READ_DATA, myarg.cmd);
			}
		}
	}else if(strcmp(argv[1],"wdata")==0) {

		if (argc!=4) {
			printf("start argument number error!\n");
		}else{ 
			myarg.cmd2 = s2i(argv[2]);
			myarg.cmd3 = s2i(argv[3]);
		
			//printf("[%s, line %d] myarg.cmd2 = %d, myarg.cmd3 = %d\n",
			//	__func__,__LINE__,myarg.cmd2, myarg.cmd3);
		
			if (ioctl(fileno(fp), PTM_WRITE_DATA, &myarg) < 0) {
				printf("PTM write data failed %x %x!\n", PTM_WRITE_DATA, myarg.cmd);
			}
		}
	}else if(strcmp(argv[1],"set_sys")==0){
		if (ioctl(fileno(fp), PTM_SET_SYSTEM, &myarg) < 0) {
			printf("PTM set system failed %x %x!\n", PTM_SET_SYSTEM, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"set_hw")==0){
		
		if(argc==2){
			myarg.cmd2 = 0;
		}else{
			if(strcmp(argv[2],"non-bonding")==0){
				myarg.cmd2 = 0;
			}else if(strcmp(argv[2],"bonding")==0){
				myarg.cmd2 = 1;
				myarg.cmd3 = s2i(argv[3]); /* Bonding line */
			}		
		}
		
		if (ioctl(fileno(fp), PTM_SET_HW, &myarg) < 0) {
			printf("PTM set table failed %x %x!\n", PTM_SET_HW, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"get_qmap")==0){
		
		if (ioctl(fileno(fp), PTM_GET_QMAP, &myarg) < 0) {
			printf("PTM set table failed %x %x!\n", PTM_GET_QMAP, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"stop_hw")==0){
		if (ioctl(fileno(fp), PTM_STOP_HW, &myarg) < 0) {
			printf("PTM stop hw failed %x %x!\n", PTM_STOP_HW, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"start_hw")==0){
		if (ioctl(fileno(fp), PTM_START_HW, &myarg) < 0) {
			printf("PTM start hw failed %x %x!\n", PTM_START_HW, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"clean_wanif")==0){
		if (ioctl(fileno(fp), PTM_CLEAN_WANIF, &myarg) < 0) {
			printf("PTM clean wanif failed %x %x!\n", PTM_CLEAN_WANIF, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"clean_qmap")==0){
		if (ioctl(fileno(fp), PTM_CLEAN_QMAP, &myarg) < 0) {
			printf("PTM clean qmap failed %x %x!\n", PTM_CLEAN_QMAP, myarg.cmd);
		}	
	}else if(strcmp(argv[1],"set_wanif")==0){
		if( argc<6 ) goto arg_err_rtn;
		myarg.cmd=(unsigned char)atoi(argv[2]);
		myarg.a1=(unsigned short)atoi(argv[3]);
		myarg.a2=(unsigned short)atoi(argv[4]);
		if( !string_to_hex( argv[5], myarg.a3, 12) )
		{
			printf( "mac format error!\n" );
			goto arg_err_rtn;
		}
		if (ioctl(fileno(fp), PTM_SET_WANIF, &myarg) < 0) {
			printf("PTM set wanif failed %x %x!\n", PTM_SET_WANIF, myarg.cmd);
		}
	}else if(strcmp(argv[1],"set_qmap")==0){		
		if( argc<4 ) goto arg_err_rtn;
		myarg.cmd=(unsigned char)atoi(argv[2]);
		if(strlen(argv[3])<8) goto arg_err_rtn;
		for(i=0; i<8; i++)
		{
			unsigned char tmp[2];
			tmp[0]=argv[3][i];
			tmp[1]=0;
			myarg.arg[i]= (unsigned char)atoi(tmp);
		}
		if (ioctl(fileno(fp), PTM_SET_QMAP, &myarg) < 0) {
			printf("PTM set qmap failed %x %x!\n", PTM_SET_QMAP, myarg.cmd);
		}
	}else if(strcmp(argv[1],"set_stagtype")==0){
		if( argc!=3 ) goto arg_err_rtn;
		myarg.a1=(unsigned short)s2i(argv[2]);
		if (ioctl(fileno(fp), PTM_SET_STAGTYPE, &myarg) < 0) {
			printf("PTM set stagtype failed %x %x!\n", PTM_SET_STAGTYPE, myarg.a1);
		}
	}else if(strcmp(argv[1],"set_dummyvid")==0){
		if( argc!=3 ) goto arg_err_rtn;
		myarg.a1=((unsigned short)s2i(argv[2]))&0xfff;
		if (ioctl(fileno(fp), PTM_SET_DUMMYVID, &myarg) < 0) {
			printf("PTM set dummyvid failed %x %x!\n", PTM_SET_DUMMYVID, myarg.a1);
		}
	}else if(strcmp(argv[1],"set_qmapatstag")==0){
		if( argc!=4 ) goto arg_err_rtn;
		myarg.a1= (s2i(argv[2]))?1:0;
		myarg.a2= (s2i(argv[3]))?1:0;
		if (ioctl(fileno(fp), PTM_SET_QMAPATSTAG, &myarg) < 0) {
			printf("PTM set qmapatstag failed %x %x %x!\n", PTM_SET_QMAPATSTAG, myarg.a1, myarg.a2);
		}
	}else if(strcmp(argv[1],"set_mode")==0){
		if( argc!=3 ) goto arg_err_rtn;
		if(strcmp(argv[2],"phy")==0)
			myarg.cmd=0;
		else if(strcmp(argv[2],"route")==0)
			myarg.cmd=1;
		else
			goto arg_err_rtn;
			
		if (ioctl(fileno(fp), PTM_SET_MODE, &myarg) < 0) {
			printf("PTM set mode failed %x %x!\n", PTM_SET_MODE, myarg.cmd);
		}
	}else{
		ptmctl_help();
	}
	
	fclose(fp);
	return 0;

arg_err_rtn:
	printf("Argument Error Return\n");
	ptmctl_help();
	if(fp)fclose(fp);
	exit(0);
}
