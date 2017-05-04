#ifdef CONFIG_USER_PPPOMODEM
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <utmp.h>
#include <mntent.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include <asm/types.h>		/* glibc 2 conflicts with linux/types.h */
#include <linux/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/if_ether.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/version.h>
#include <linux/kdev_t.h>

#include "pppomodem.h"

#ifndef SERIAL_TTY_MAJOR
#define SERIAL_TTY_MAJOR	188	/* Nice legal number now */
#endif

#define POM_SCRIPT_DIR	"/var/ppp"
#define POM_CON_PRE	POM_SCRIPT_DIR "/pppom.con"
#define POM_DISCON_PRE	POM_SCRIPT_DIR "/pppom.discon"
#define POM_RESULT_PRE	POM_SCRIPT_DIR "/pppom.result"
#define POM_MODEM_NAME	POM_SCRIPT_DIR "/modem"

/*************debug****************************************/
#define POM_PRINTF(...) 	do{printf( "PPPoM:%s:%d> ", __FUNCTION__,__LINE__ );printf(__VA_ARGS__);}while(0)
#define DBGFLAG			(p && p->debug)
/*********************************************************/


/* We can get an EIO error on an ioctl if the modem has hung up */
#define ok_error(num) ((num)==EIO)

/********************************************************************
 *
 * List of valid speeds.
 */

struct speed {
    int speed_int, speed_val;
} speeds[] = {
#ifdef B50
    { 50, B50 },
#endif
#ifdef B75
    { 75, B75 },
#endif
#ifdef B110
    { 110, B110 },
#endif
#ifdef B134
    { 134, B134 },
#endif
#ifdef B150
    { 150, B150 },
#endif
#ifdef B200
    { 200, B200 },
#endif
#ifdef B300
    { 300, B300 },
#endif
#ifdef B600
    { 600, B600 },
#endif
#ifdef B1200
    { 1200, B1200 },
#endif
#ifdef B1800
    { 1800, B1800 },
#endif
#ifdef B2000
    { 2000, B2000 },
#endif
#ifdef B2400
    { 2400, B2400 },
#endif
#ifdef B3600
    { 3600, B3600 },
#endif
#ifdef B4800
    { 4800, B4800 },
#endif
#ifdef B7200
    { 7200, B7200 },
#endif
#ifdef B9600
    { 9600, B9600 },
#endif
#ifdef B19200
    { 19200, B19200 },
#endif
#ifdef B38400
    { 38400, B38400 },
#endif
#ifdef B57600
    { 57600, B57600 },
#endif
#ifdef B115200
    { 115200, B115200 },
#endif
#ifdef EXTA
    { 19200, EXTA },
#endif
#ifdef EXTB
    { 38400, EXTB },
#endif
#ifdef B230400
    { 230400, B230400 },
#endif
#ifdef B460800
    { 460800, B460800 },
#endif
    { 0, 0 }
};

/********************************************************************
 *
 * Translate from bits/second to a speed_t.
 */
static int translate_speed (int bps)
{
    struct speed *speedp;

    if (bps != 0) {
	for (speedp = speeds; speedp->speed_int; speedp++) {
	    if (bps == speedp->speed_int)
		return speedp->speed_val;
	}
	POM_PRINTF("speed %d not supported\n", bps);
    }
    return 0;
}

/********************************************************************
 *
 * Translate from a speed_t to bits/second.
 */
static int baud_rate_of (int speed)
{
    struct speed *speedp;
    
    if (speed != 0) {
	for (speedp = speeds; speedp->speed_int; speedp++) {
	    if (speed == speedp->speed_val)
		return speedp->speed_int;
	}
    }
    return 0;
}

/********************************************************************
 *
 * setdtr - control the DTR line on the serial port.
 * This is called from die(), so it shouldn't call die().
 */
static void setdtr (int tty_fd, int on)
{
    int modembits = TIOCM_DTR;

    ioctl(tty_fd, (on ? TIOCMBIS : TIOCMBIC), &modembits);
}

/********************************************************************
 *
 * set_up_tty: Set up the serial port on `fd' for 8 bits, no parity,
 * at the requested speed, etc.  If `local' is true, set CLOCAL
 * regardless of whether the modem option was specified.
 */
static void set_up_tty(struct pppomodem_param_s *p, int tty_fd, int local)
{
    int speed;
    struct termios tios;

    setdtr(tty_fd, 1);
    if (tcgetattr(tty_fd, &tios) < 0) {
	if (!ok_error(errno))
	    POM_PRINTF("tcgetattr: %s(%d)\n", strerror(errno), errno);
	return;
    }
    
    if (!p->restore_term)
	p->inittermios = tios;
    
    tios.c_cflag     &= ~(CSIZE | CSTOPB | PARENB | CLOCAL);
    tios.c_cflag     |= CS8 | CREAD | HUPCL;

    tios.c_iflag      = IGNBRK | IGNPAR;
    tios.c_oflag      = 0;
    tios.c_lflag      = 0;
    tios.c_cc[VMIN]   = 1;
    tios.c_cc[VTIME]  = 0;
    
    if (local || !p->modem)
	tios.c_cflag ^= (CLOCAL | HUPCL);

    switch (p->crtscts) {
    case 1:
	tios.c_cflag |= CRTSCTS;
	break;

    case -2:
	tios.c_iflag     |= IXON | IXOFF;
	tios.c_cc[VSTOP]  = 0x13;	/* DC3 = XOFF = ^S */
	tios.c_cc[VSTART] = 0x11;	/* DC1 = XON  = ^Q */
	break;

    case -1:
	tios.c_cflag &= ~CRTSCTS;
	break;

    default:
	break;
    }
    
    speed = translate_speed(p->inspeed);
    if (speed) {
	cfsetospeed (&tios, speed);
	cfsetispeed (&tios, speed);
    }
/*
 * We can't proceed if the serial port speed is B0,
 * since that implies that the serial port is disabled.
 */
    else {
	speed = cfgetospeed(&tios);
	if (speed == B0)
	    POM_PRINTF("Baud rate for %s is 0; need explicit baud rate\n", (p->devnam[0]==0)?"N/A":p->devnam );
    }

    if (tcsetattr(tty_fd, TCSAFLUSH, &tios) < 0)
	if (!ok_error(errno))
	    POM_PRINTF("tcsetattr: %s\n", strerror(errno) );
    
    p->baud_rate    = baud_rate_of(speed);
    p->restore_term = 1;
}

/********************************************************************
 *
 * restore_tty - restore the terminal to the saved settings.
 */
static void restore_tty (struct pppomodem_param_s *p, int tty_fd)
{
    if (p->restore_term) {
	p->restore_term = 0;
/*
 * Turn off echoing, because otherwise we can get into
 * a loop with the tty and the modem echoing to each other.
 * We presume we are the sole user of this tty device, so
 * when we close it, it will revert to its defaults anyway.
 */
	//if (!default_device)
	if(1)
	    p->inittermios.c_lflag &= ~(ECHO | ECHONL);
	
	if (tcsetattr(tty_fd, TCSAFLUSH, &p->inittermios) < 0) {
	    if (! ok_error (errno))
		POM_PRINTF("tcsetattr: %s\n", strerror(errno) );
	}
    }
}

/********************************************************************
 *
 * close_tty - restore the terminal device and close it.
 */
static void close_tty( struct pppomodem_param_s *p )
{
    /* drop dtr to hang up */
    //if (!default_device && modem)
    if(p->modem) 
    {
	setdtr(p->ttyfd, 0);
	/*
	 * This sleep is in case the serial port has CLOCAL set by default,
	 * and consequently will reassert DTR when we close the device.
	 */
	sleep(1);
    }

    restore_tty(p, p->ttyfd);
    
    close(p->ttyfd);
    p->ttyfd = -1;
}

/********************************************************************
 *
 * device_script - run a program to talk to the serial device
 * (e.g. to run the connector or disconnector script).
 */
static int device_script(char *program, int in, int out, int *ret_pid, int debugflag )
{
	int pid;
	int status = -1;
	int errfd;

	if(ret_pid) *ret_pid=0;	
	pid = vfork();
	if (pid < 0) {
		POM_PRINTF("Failed to create child process: %s\n", strerror(errno) );
		return -1;
	}

	if (pid == 0)
	{
		//fd 2 for err msg
		if(!debugflag) //default should output the message on console
		{
			int errfd;
			if (in == 2) {
				/* aargh!!! */
				int newin = dup(in);
				if (in == out)
					out = newin;
				in = newin;
			} else if (out == 2) {
				out = dup(out);
			}			
			close(2);
			errfd = open("/dev/null", O_WRONLY | O_APPEND | O_CREAT, 0600);
			if (errfd >= 0 && errfd != 2) {
				dup2(errfd, 2);
				close(errfd);
			}
		}
		
		if (in != 0) {
			if (out == 0)	out = dup(out);
			dup2(in, 0);
		}
		if (out != 1) {
			dup2(out, 1);
		}
		

		/*
		 *      On uClinux we don't have a full shell, just call chat
		 *      program directly (obviously it can't be a sh script!).
		 */
		{
			char    *argv[16], *sp;
			int     argc = 0, prevspace = 1;

			for (sp = program; (*sp != 0); ) 
			{
				if (prevspace && !isspace(*sp))
				{
					argv[argc++] = sp;
				}

				if ((prevspace = isspace(*sp)))
					*sp = 0;

				sp++;
			}
			argv[argc] = 0;
			execv(argv[0], argv);
			perror( "execv" );
		}
		fprintf( stderr, "could not exec /bin/sh: %s\n", strerror(errno) );
		exit(99);
		/* NOTREACHED */		
	}

	POM_PRINTF( "parent process, pid=%d\n", pid );
	if(ret_pid) *ret_pid=pid;
	return 0;
}

/********************************************************************
 *
 * Functions to read and set the flags value in the device driver
 */
static int get_flags (int fd)
{    
    int flags;

    if (ioctl(fd, PPPIOCGFLAGS, (caddr_t) &flags) < 0) {
	if ( ok_error (errno) )
	    flags = 0;
	else{
	    POM_PRINTF("ioctl(PPPIOCGFLAGS): %s\n", strerror(errno) );
	}
    }

    //SYSDEBUG ((LOG_DEBUG, "get flags = %x\n", flags));
    POM_PRINTF( "get flags = %x\n", flags);
    return flags;
}

static void set_flags (int fd, int flags)
{    
    //SYSDEBUG ((LOG_DEBUG, "set flags = %x\n", flags));
    POM_PRINTF("set flags = %x\n", flags);

    if (ioctl(fd, PPPIOCSFLAGS, (caddr_t) &flags) < 0) {
	if (! ok_error (errno) )
	    POM_PRINTF("ioctl(PPPIOCSFLAGS, %x): %s(%d)\n", flags, strerror(errno), errno);
    }
}

/********************************************************************
 *
 * tty_disestablish_ppp - Restore the serial port to normal operation.
 * This shouldn't call die() because it's called from die().
 */
static void tty_disestablish_ppp(struct pppomodem_param_s *p, int tty_fd)
{
    int tty_disc = N_TTY;	/* The TTY discipline */
    
    //generic_disestablish_ppp(tty_fd);

    if (!p->hungup) {
/*
 * Flush the tty output buffer so that the TIOCSETD doesn't hang.
 */
	if (tcflush(tty_fd, TCIOFLUSH) < 0)
	    POM_PRINTF("tcflush failed: %s\n", strerror(errno) );
/*
 * Restore the previous line discipline
 */
	if (ioctl(tty_fd, TIOCSETD, &tty_disc) < 0) {
	    if ( ! ok_error (errno))
		POM_PRINTF("ioctl(TIOCSETD, N_TTY): %s\n", strerror(errno) );
	}

	if (ioctl(tty_fd, TIOCNXCL, 0) < 0) {
	    if ( ! ok_error (errno))
		POM_PRINTF("ioctl(TIOCNXCL): %s(%d)\n", strerror(errno), errno);
	}

#if 0 /*for oldstyledriver*/
	/* Reset non-blocking mode on fd. */
	if (initfdflags != -1 && fcntl(tty_fd, F_SETFL, initfdflags) < 0) {
	    if ( ! ok_error (errno))
		//warn("Couldn't restore device fd flags: %m");
		POM_PRINTF("Couldn't restore device fd flags: %m");
	}
#endif //if 0
    }
}

/*
 * Most 3G dongles use the first interface that has interrupt endpoint as the modem interface,
 * but some use 2nd interface that has interrupt endpoint to be. We record the vid and pid for
 * these dongles in id_table and if id matches an entry of id_table in find_3g_tty function, findIntfAgain
 * will be set and the 2nd interface number that has interrupt endpoint will be returned.
*/
struct id_entry id_table[] = {
	{"0408", "ea16"},
	{NULL, NULL}
};

/*
usb device file will be....
T:  Bus=01 Lev=01 Prnt=01 Port=01 Cnt=01 Dev#=  3 Spd=480 MxCh= 0
D:  Ver= 2.00 Cls=00(>ifc ) Sub=00 Prot=00 MxPS=64 #Cfgs=  1
P:  Vendor=1da5 ProdID=4512 Rev= 0.00
S:  Manufacturer=HSUPA USB Device
S:  Product=H21 HSUPA USB Device
S:  SerialNumber=353031020363931
C:* #Ifs= 4 Cfg#= 1 Atr=e0 MxPwr=500mA
I:  If#= 0 Alt= 0 #EPs= 2 Cls=ff(vend.) Sub=ff Prot=ff Driver=option
E:  Ad=81(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=01(O) Atr=02(Bulk) MxPS= 512 Ivl=4ms
I:  If#= 1 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
E:  Ad=02(O) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=82(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
I:  If#= 2 Alt= 0 #EPs= 3 Cls=ff(vend.) Sub=ff Prot=ff Driver=option
E:  Ad=83(I) Atr=03(Int.) MxPS=  64 Ivl=2ms
E:  Ad=84(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=03(O) Atr=02(Bulk) MxPS= 512 Ivl=4ms
we should find which If# has an Interrupt endpoint.

ret: which option interface is 3g, if 0, means no one is 3g.
*/
int find_3g_tty(FILE* fp){
	char *p = NULL;
	char tmp[256];
	char *optionDriver = "Driver=option";
	char *interruptAttr = "Atr=03";
	char vid[8], pid[8];
	enum states{
		NOT_FIND = 0,
		FIND_T,
		FIND_P,
		FIND_OPTION_I,
		FIND_INT_END
	}state;
	state = NOT_FIND;
	int updateIntNum = 0;//update interface number
	int resetVar = 0;//reset variable
	int optionIntfNum = NOT_FIND;//option interface number
	int findIntfAgain = 0;

	if(!fp){
		return NOT_FIND;
	}
	while( fgets( tmp, sizeof(tmp), fp ) ){
		switch(state){
			case NOT_FIND:
				/*can only go to FIND_T*/
				if(tmp[0] == 'T'){
					state = FIND_T;
					resetVar = 1;
				}
				break;
			case FIND_T:
				if(tmp[0] == 'P'){
					state = FIND_P;
					p = strstr(tmp, "Vendor=");
					if(p){
						strncpy(vid, p+strlen("Vendor="), 4);
						vid[4]='\0';
					}
					p = strstr(tmp, "ProdID=");
					if(p){
						strncpy(pid, p+strlen("ProdID="), 4);
						pid[4]='\0';
					}
					if(p){
						struct id_entry *idp;
						findIntfAgain = 0;
						for(idp=&id_table[0]; idp->vid!=NULL; idp++) {
							if(!strcmp(vid, idp->vid) && !strcmp(pid, idp->pid)){
								findIntfAgain = 1;
								break;
							}
						}
					}
				}
				else if(tmp[0] == 'T'){
					resetVar = 1;
				}
				else{/*still in FIND_T*/
				}
				break;
			case FIND_P:
				if(tmp[0] == 'I' && strstr(tmp, optionDriver)){
					state = FIND_OPTION_I;
					updateIntNum = 1;
				}
				else if(tmp[0] == 'T'){
					state = FIND_T;
					resetVar = 1;
				}
				else{/*still in FIND_P*/
				}
				break;
			case FIND_OPTION_I:
				if(tmp[0] == 'T'){/*goto FIND_T*/
					state = FIND_T;
					resetVar = 1;
				}
				else if(tmp[0] == 'E' && strstr(tmp, interruptAttr)){/*finish*/
					if(!findIntfAgain) {
						state = FIND_INT_END;
						return optionIntfNum;
					}
					else {
						findIntfAgain = 0;
					}
				}
				else if(tmp[0] == 'I' && strstr(tmp, optionDriver)){/*find new option interface*/
					updateIntNum = 1;
				}
				else{/*still in FIND_OPTION_I*/
				}
				break;
			default:
				break;
		}
		if(resetVar){
			optionIntfNum = NOT_FIND;
			resetVar = 0;
		}
		if(updateIntNum){
			optionIntfNum++;
			updateIntNum = 0;
		}
	}
	return NOT_FIND;
}

#include <dirent.h>

int check_usb_vfs(const char *dirname)
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

/*ret: -1:fail, 1:success. plz don't return 0(means do nothing)*/
int mount_usbfs(void){
	#define USBPATH "/proc/bus/usb"
	unsigned char mount_retry = 5;
	char mountOK =-1;//default fail
	while(mount_retry) {
		   if(!check_usb_vfs(USBPATH)) {
			   system("mount -t usbfs none /proc/bus/usb");
			   usleep(10000);
		   }
		   else {
			   mountOK = 1;
			   break;
		   }
		   mount_retry--;
	}
	if(mountOK == -1){
		POM_PRINTF( "<Warning!>No usb filesystem, dial maybe fail!!!!!!!!!!!!!!!!!!!!\n" );
	}
	return mountOK;
}

/********************************************************************************/
static int pppomodem_finddev( struct pppomodem_param_s *p )
{
	int ret=-1, tty_num = 1;//default use first one
	static int alreadyMount = 0;
	char *pUSBSerialFile="/proc/tty/driver/usbserial";
	char *pUsbFileSystem = "/proc/bus/usb/devices";
	FILE *fp;
	
	//POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	fp=fopen( pUsbFileSystem, "r" );
	if(fp){
		tty_num = find_3g_tty(fp);
		fclose(fp);
		if(!tty_num){
			return ret;
		}
		else{
			if(DBGFLAG) POM_PRINTF( "choose ttyUsb%d\n", tty_num - 1 );
		}
	}
	else{
		if(alreadyMount == 0){//not mount
			alreadyMount = mount_usbfs();
			/*fopen fail is because not mount, so we return and wait for open again!*/
			return ret;
		}
		else{
			/*no matter mount success or fail, we all continue to do our wrok*/
		}
	}
	
	p->devnam[0]=0;
	fp=fopen( pUSBSerialFile, "r" );
	if(fp)
	{
		char tmp[320];
		fgets( tmp, sizeof(tmp), fp );
		while( fgets( tmp, sizeof(tmp), fp ) )
		{
			char *pstr;

			if(DBGFLAG) POM_PRINTF( "%s", tmp );
			
			pstr=strstr(tmp, "\n");
			if(pstr) *pstr=0;
			
			//idx: module: name: vendor: product: num_ports: port: path: str4product: str4manufacturer: str4serial: (no "module:" in linux 2.6.30)
			if( isdigit(tmp[0])&&strstr(tmp, ": ") )
			{
				char *pidx, *pmodule, *pname, *pvendor, *pproduct, *pnum_ports, *pport, *ppath, *pstr4product, *pstr4manufacturer, *pstr4serial;

				tty_num--;
				if(tty_num){
					/*still need to skip*/
					continue;
				}
				
				pidx=tmp;
				pstr=strstr(tmp, ": ");
				pstr[0]=0;
				pstr++;
								
				pmodule=strstr(pstr, " module:" );
				pname=strstr(pstr, " name:" );
				pvendor=strstr(pstr, " vendor:" );
				pproduct=strstr(pstr, " product:" );
				pnum_ports=strstr(pstr, " num_ports:" );
				pport=strstr(pstr, " port:" );
				ppath=strstr(pstr, " path:" );
				pstr4product=strstr(pstr, " str4product:" );
				pstr4manufacturer=strstr(pstr, " str4manufacturer:" );
				pstr4serial=strstr(pstr, " str4serial:" );

				if(pmodule) {
					pmodule[0]=0;
					pmodule=pmodule+strlen(" module:");
				}

				if(pname) {
					pname[0]=0;
					pname=pname+strlen(" name:");
				}

				if(pvendor) {
					pvendor[0]=0;
					pvendor=pvendor+strlen(" vendor:");
				}

				if(pproduct) {
					pproduct[0]=0;
					pproduct=pproduct+strlen(" product:");
				}

				if(pnum_ports) {
					pnum_ports[0]=0;
					pnum_ports=pnum_ports+strlen(" num_ports:");
				}

				if(pport) {
					pport[0]=0;
					pport=pport+strlen(" port:");
				}

				if(ppath) {
					ppath[0]=0;
					ppath=ppath+strlen(" path:");
				}

				if(pstr4product) {
					pstr4product[0]=0;
					pstr4product=pstr4product+strlen(" str4product:");
				}

				if(pstr4manufacturer) {
					pstr4manufacturer[0]=0;
					pstr4manufacturer=pstr4manufacturer+strlen(" str4manufacturer:");
				}

				if(pstr4serial) {
					pstr4serial[0]=0;
					pstr4serial=pstr4serial+strlen(" str4serial:");
				}
				
				if(DBGFLAG) 
				{
					POM_PRINTF( "idx=%s\n", pidx );
					POM_PRINTF( "module=%s\n", pmodule );
					POM_PRINTF( "name=%s\n", pname );
					POM_PRINTF( "vendor=%s\n", pvendor );
					POM_PRINTF( "product=%s\n", pproduct );
					POM_PRINTF( "num_ports=%s\n", pnum_ports );
					POM_PRINTF( "port=%s\n", pport );
					POM_PRINTF( "path=%s\n", ppath );
					POM_PRINTF( "str4product=%s\n", pstr4product );
					POM_PRINTF( "str4manufacturer=%s\n", pstr4manufacturer );
					POM_PRINTF( "str4serial=%s\n", pstr4serial );
				}

				//currently only support "GSM modem (1-port)" module
				if( pname && strstr( pname, "GSM modem (1-port)" ) )
				{
					dev_t tmpdev;
					unsigned int minor_no;
					
					minor_no=atoi(pidx);
					if(DBGFLAG) POM_PRINTF( "create %s with major:minor=%d:%d\n", 
											POM_MODEM_NAME, SERIAL_TTY_MAJOR, minor_no );
					
					if( unlink( POM_MODEM_NAME )<0 )
						POM_PRINTF( "unlink: %s\n", strerror(errno) );
					
					tmpdev=MKDEV( SERIAL_TTY_MAJOR,  minor_no);
					if( mknod( POM_MODEM_NAME, S_IFCHR, tmpdev )<0 )
					{
						POM_PRINTF( "mknod: %s\n", strerror(errno) );
					}else{
						strcpy( p->devnam, POM_MODEM_NAME );
						ret=0;
					}
					
					break;
				}
			}
		}
		fclose(fp);
	}else{
		if(DBGFLAG) POM_PRINTF( "fopen: %s\n", strerror(errno) );
	}
	
	
	return ret; 
}

static int pppomodem_opendev( struct pppomodem_param_s *p )
{
	int ttyfd=-1;
	int fdflags;
	static int err_stat=0; //reduce the debug message
	static int err_ischr=0; //reduce the debug message
	static int err_open=0; //reduce the debug message

	//POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	
	//find the modem device if the finddev flag is set.
	if(p->finddev)
	{
		if( pppomodem_finddev(p)<0 )
			goto ret_fail;
	}

	if(p->devnam && (p->devnam[0]!=0))
	{
		char *devnam=p->devnam;
		struct stat statbuf;

		/*
		 * Check if there is a character device by this name.
		 */
		if (stat(devnam, &statbuf) < 0) 
		{
			if(err_stat!=errno) 
			{	
				err_stat=errno;
				POM_PRINTF("Couldn't stat %s: %s\n", devnam, strerror(errno) );
			}
			goto ret_fail;
		}else
			err_stat=0;

		if (!S_ISCHR(statbuf.st_mode)) 
		{
			err_ischr=1;
			POM_PRINTF("%s is not a character device\n", devnam);
			goto ret_fail;
		}else
			err_ischr=0;


		for (;;)
		{
			int err;
			/* If the user specified the device name, become the
			   user before opening it. */
			ttyfd = open(devnam, O_NONBLOCK | O_RDWR, 0);
			err = errno;
			if (ttyfd >= 0)
			    break;
			errno = err;
			if (err != EINTR)
			{
				if(err_open!=err) //reduce the debug message
				{	
					err_open=err;
					POM_PRINTF("Failed to open %s, errno=%d, %s\n", devnam, err, strerror(err) );
				}
			    goto ret_fail;
			}
		}
		err_open=0;

		if((fdflags = fcntl(ttyfd, F_GETFL)) == -1
			|| fcntl(ttyfd, F_SETFL, fdflags & ~O_NONBLOCK) < 0)
			POM_PRINTF( "Couldn't reset non-blocking mode on device, %s\n", strerror(errno) );

		/*
		 * Set line speed, flow control, etc.
		 * If we have a non-null connection script,
		 * on most systems we set CLOCAL for now so that we can talk
		 * to the modem before carrier comes up.  But this has the
		 * side effect that we might miss it if CD drops before we
		 * get to clear CLOCAL below.  On systems where we can talk
		 * successfully to the modem with CLOCAL clear and CD down,
		 * we could clear CLOCAL at this point.
		 */
		set_up_tty(p, ttyfd, 1);
	}

ret_fail:
	p->ttyfd=ttyfd;
	return ttyfd;	
}

static void pppomodem_closedev( struct pppomodem_param_s *p )
{
	if( p && (p->ttyfd>=0) )
	{
		close_tty( p );
	}
}

static int pppomodem_dial_start( struct pppomodem_param_s *p, int unit )
{
	char connector[72];
	char connector_script[32];
	FILE *fd;
	pid_t cpid;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	if(p==NULL) return -1;
	p->cpid=0;
	p->wait_child=0;

	//create a script file
	sprintf( connector_script, "%s%d", POM_CON_PRE, unit );
	fd=fopen( connector_script, "w" );
	if(fd==NULL) return -1;
	fprintf( fd, "ABORT 'BUSY'\n" );
	fprintf( fd, "ABORT 'ERROR'\n" );
	fprintf( fd, "ABORT 'NO CARRIER'\n" );
	fprintf( fd, "ABORT 'NO DIALTONE'\n" );
	fprintf( fd, "ABORT 'NO DIAL TONE'\n" );
	fprintf( fd, "TIMEOUT 60\n" );
	fprintf( fd, "'' ATZ\n" );
	fprintf( fd, "OK AT+CFUN=1\n" );
	
	if(p->has_simpin && (strlen(p->simpin)==SIM_PIN_LEN))
	{
		fprintf( fd, "OK AT+CPIN?\n" );
		fprintf( fd, "TIMEOUT 5\n" );
		fprintf( fd, "READY-AT+CPIN=%s-", p->simpin );
	}
	if(p->has_apn && (strlen(p->apn)>0))
		fprintf( fd, "OK 'AT+CGDCONT=1,\"IP\",\"%s\"'\n", p->apn );
	fprintf( fd, "OK ATDT%s\n", p->dial );
	fprintf( fd, "TIMEOUT 60\n" );
	fprintf( fd, "CONNECT ''\n" );
	fclose(fd);

	if(DBGFLAG)
	{
		FILE *dumpfp;
		dumpfp=fopen( connector_script, "r" );
		POM_PRINTF("********dump file=%s****\n", connector_script);
		if(dumpfp)
		{
			int c;
			do
			{
				c=fgetc(dumpfp);
				if(c!=EOF)printf( "%c", (unsigned char)c );
			}while( c!=EOF );
			fclose(dumpfp);
		}
		POM_PRINTF("********end dump file=%s\n", connector_script);
	}

	/* run connection script */
	sprintf( connector, "/bin/chat -v -s -f %s -R %s%d", connector_script, POM_RESULT_PRE, unit );
	if(DBGFLAG) POM_PRINTF("Connecting with <%s>\n", connector);
	if(p->ttyfd!=-1)
	{
		//if (!default_device && modem)
		if (p->modem) 
		{
		    setdtr(p->ttyfd, 0);	/* in case modem is off hook */
		    sleep(1);
		    setdtr(p->ttyfd, 1);
		}
	}

	if (device_script(connector, p->ttyfd, p->ttyfd, &cpid, DBGFLAG) < 0)
	{
		POM_PRINTF("device_script() return error\n");
		return -1;
	}
	if(DBGFLAG) POM_PRINTF("return from device_script(), child pid=%d, set  wait_child=1\n", cpid);

	p->cpid=cpid;
	p->wait_child=1;
	return 0;
}

static void pppomodem_dial_end( struct pppomodem_param_s *p )
{
	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	POM_PRINTF("Serial connection established.\n");

	/* set line speed, flow control, etc.;
	clear CLOCAL if modem option */
	if (p->ttyfd != -1)
		set_up_tty(p, p->ttyfd, 0);

	return;
}

static int pppomodem_dial_isfinished(pid_t cpid)
{
	pid_t wpid;
	int st, ret=0;
	
	wpid=waitpid(cpid, &st, WNOHANG);
	if( (wpid==-1) && (errno==ECHILD) )
		ret=1;
	else if(wpid==cpid)
		ret=1;
	
	//POM_PRINTF( "cpid=%d, ret=%d\n", cpid, ret);
	return ret;	
}

static int pppomodem_dial_isOK( struct pppomodem_param_s *p, int unit )
{
	char fileresult[32];
	FILE *fp;
	int ret=0;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	sprintf( fileresult, "%s%d", POM_RESULT_PRE, unit );
	fp=fopen( fileresult, "r" );
	if(fp)
	{
		int status;
		if( fscanf( fp, "%d", &status )==1 )
		{
			if(DBGFLAG) POM_PRINTF( "status=%d\n", status );
			if(status==0) ret=1;
		}
		fclose(fp);
	}

	return ret;
}

static int pppomodem_disconnect( struct pppomodem_param_s *p, int unit )
{
	char disconnector[72];
	char disconnector_script[32];
	FILE *fd;
	pid_t cpid;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	if(p==NULL) return -1;
	p->cpid=0;
	p->wait_child=0;

	if( p && (p->ttyfd>=0) )
	{
		sprintf( disconnector_script, "%s%d", POM_DISCON_PRE, unit );
		fd=fopen( disconnector_script, "w" );
		if(fd==NULL) return -1;
		fprintf( fd, "ABORT 'BUSY'\n" );
		fprintf( fd, "ABORT 'ERROR'\n" );
		fprintf( fd, "ABORT 'NO DIALTONE'\n" );
		fprintf( fd, "TIMEOUT 30\n" );
		fprintf( fd, "'' '+++\\c'\n" );
		fprintf( fd, "SAY ' + sending break'\n" );
		fprintf( fd, "'NO CARRIER' 'ATH'\n" );
		fprintf( fd, "SAY '\\n + dropping data connection'\n" );
		fprintf( fd, "OK '\\c'\n" );
		fprintf( fd, "SAY '\\n + disconnected.\\n'\n" );		
		fclose(fd);

		sprintf( disconnector, "/bin/chat -v -s -f %s -R %s%d", disconnector_script, POM_RESULT_PRE, unit );
		if (disconnector && !p->hungup) 
		{
			if (p->ttyfd >= 0)
				set_up_tty(p,p->ttyfd, 1);
			if(DBGFLAG) POM_PRINTF("Disconnecting with <%s>\n", disconnector);
			if (device_script(disconnector, p->ttyfd, p->ttyfd, &cpid, DBGFLAG) < 0) {
				POM_PRINTF("disconnect script failed\n");
			}else{
				//POM_PRINTF("Serial link disconnected.\n");
				POM_PRINTF("waiting Serial link disconnected. (pid=%d)\n", cpid);
				p->cpid=cpid;
				p->wait_child=1;
			}
		}
	}

	return 0;
}

static int pppomodem_tty_establish_ppp_start(struct pppomodem_param_s *p)
{
	int tty_fd;
	int ppp_disc; /* The PPP discpline */

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	if(p==NULL) return -1;

	tty_fd=p->ttyfd;
	/*
	 * Ensure that the tty device is in exclusive mode.
	 */
	if (ioctl(tty_fd, TIOCEXCL, 0) < 0) {
		if ( ! ok_error ( errno ))
			POM_PRINTF("Couldn't make tty exclusive: %s\n", strerror(errno) );
	}
	/*
	 * Set the current tty to the PPP discpline
	 */
#ifndef N_SYNC_PPP
#define N_SYNC_PPP 14
#endif
	//ppp_disc = (new_style_driver && sync_serial)? N_SYNC_PPP: N_PPP;
	ppp_disc = p->sync_serial? N_SYNC_PPP: N_PPP;
	if (ioctl(tty_fd, TIOCSETD, &ppp_disc) < 0) {
		if ( ! ok_error (errno) ) {
			POM_PRINTF("Couldn't set tty to PPP discipline: %s\n", strerror(errno) );
			return -1;
		}
	}	

	return 0;
}

int pppomodem_tty_establish_ppp_end(struct pppomodem_param_s *p, int ppp_fd)
{
	int kdebugflag=0; /* Tell kernel to print debug messages */
	
	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

#if 0
#define SC_RCVB	(SC_RCV_B7_0 | SC_RCV_B7_1 | SC_RCV_EVNP | SC_RCV_ODDP)
#define SC_LOGB	(SC_DEBUG | SC_LOG_INPKT | SC_LOG_OUTPKT | SC_LOG_RAWIN \
		 | SC_LOG_FLUSH)

	set_flags(ppp_fd, ((get_flags(ppp_fd) & ~(SC_RCVB | SC_LOGB))
		| ((kdebugflag * SC_DEBUG) & SC_LOGB)));

	//??????
	//lcp_lowerup(0);
	//wait_input(&t);
#endif //if 0

	return 0;
}

static int pppomodem_tty_establish_ppp_fail(struct pppomodem_param_s *p)
{
	int tty_disc = N_TTY;	/* The TTY discipline */

	if( p && (p->ttyfd>=0) )
	{
		if (ioctl(p->ttyfd, TIOCSETD, &tty_disc) < 0 && !ok_error(errno))
			if(DBGFLAG) POM_PRINTF("Couldn't reset tty to normal line discipline: %s\n", strerror(errno) );
	}
	return 0;
}

static void pppomodem_tty_disestablish_ppp(struct pppomodem_param_s *p)
{
	if( p && (p->ttyfd>=0) )
	{
		tty_disestablish_ppp( p, p->ttyfd );
	}
}

/********************************************************************************/
char* strPPPoModemState[]=
{
	"POM_ST_DEAD",
	"POM_ST_INIT",
	"POM_ST_DIAL",
	"POM_ST_CONNECT",
	"POM_ST_DISCONNECT",
	
	"POM_ST_(ILLEGAL)" /*last*/
};

static char *pppomodem_get_state_str( int s )
{
	//error
	if( (s<0) || (s>=POM_ST_NUM) )
		s=POM_ST_NUM;

	return strPPPoModemState[s];
}

static int pppomodem_set_state(struct pppomodem_param_s *p, int newstate)
{
	if(p)
	{
		int oldstate;

		if(DBGFLAG) POM_PRINTF( "%s(%d) ==> %s(%d)\n",
					pppomodem_get_state_str(p->state), p->state,
					pppomodem_get_state_str(newstate), newstate);

		oldstate = p->state;
		p->state = newstate;
		return oldstate;
	}

	return -1;
}

static int pppomodem_reset_value(struct pppomodem_param_s *p)
{
	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	if(p==NULL) return -1;

	//some values should be reset before change to DEAD state
	p->wait_child=0;
	p->cpid=0;	
	p->ttyfd=-1;
	p->restore_term=0;
	p->hungup=0;

	return 0;
}

int pppomodem_default(struct pppomodem_param_s *p)
{
	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );

	if(p==NULL) return -1;

	pppomodem_set_state(p, POM_ST_DEAD);
	p->wait_child=0;
	p->cpid=0;
	p->debug=0;
	p->has_simpin=0;
	p->simpin[0]=0;
	
	p->ttyfd=-1;
	p->restore_term=0;
	p->crtscts=0;
	p->inspeed=0;
	p->baud_rate=0;
	p->hungup=0;
	p->modem=1;
	p->sync_serial=0;
	p->devnam[0]=0;
	p->finddev=1;

	return 0;
}

int attached_3g_usb() //check if 3g usb is attached; 0:no, 1:yes
{
	//char *pUSBSerialFile="/proc/tty/driver/usbserial";
	char *pUsbFileSystem = "/proc/bus/usb/devices";
	FILE *fp;
	int tty_num;
	
	//POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	fp=fopen( pUsbFileSystem, "r" );
	if(fp){
		tty_num = find_3g_tty(fp);
		fclose(fp);
		if(!tty_num){
			return 0;
		}
		else
			return 1;
	}
	return 0;

}

extern int ppp_up_flag[N_SPPP];

static void pppomodem_handle_state( struct sppp *sp )
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;
	
	if( (sp==NULL) || (sp->over != SPPP_PPPOMODEM) || (p==NULL) ) return;

	//POM_PRINTF( "state=%s\n", pppomodem_get_state_str(p->state) );
	switch(p->state)
	{
	case POM_ST_DEAD:
		{
			
			//paula, 3g backup PPP
			//if 3g dongle is not plugged, change state to INIT for immediate link start if adsl ppp connection
			//else 3g is plugged, start timer to provide backup function
			if(ppp_backup_flag && !attached_3g_usb()){ 
				pppomodem_set_state(p, POM_ST_INIT);
				pppomodem_handle_state( sp );
			}
			
			break;
		}
	case POM_ST_INIT:
		{
			if( pppomodem_opendev(p)>=0 )
			{
				if(DBGFLAG) POM_PRINTF( "pppomodem_opendev() OK, p->ttyfd=%d\n", p->ttyfd );
				//paula, 3g backup automatic connection for adsl ppp disconnection
				if(ppp_backup_flag && (!ppp_up_flag[sp->if_unit]) ){ 
					if(poll_3g_backup_link_status()){
						pppomodem_closedev(p);
						pppomodem_reset_value(p);
						pppomodem_set_state(p, POM_ST_DEAD); 
					}
					else{
						ppp_up_flag[sp->if_unit] = 1;
						pppomodem_set_state(p, POM_ST_DIAL);
						pppomodem_handle_state( sp );
					}
				}
				else{
					pppomodem_set_state(p, POM_ST_DIAL);
					pppomodem_handle_state( sp );
				}	
			}//else fail
			break;
		}
	case POM_ST_DIAL:
		{
			//POM_PRINTF( "p->wait_child=%d, p->cpid=%u\n", p->wait_child, p->cpid );
			if(p->wait_child)
			{
				if( pppomodem_dial_isfinished(p->cpid) )
				{
					if(DBGFLAG) POM_PRINTF( "finish dialing\n" );
					
					p->wait_child=0;
					p->cpid=0;
					if( pppomodem_dial_isOK(p, sp->unit) )
					{
						pppomodem_dial_end(p);
						pppomodem_set_state(p, POM_ST_CONNECT);
						pppomodem_handle_state( sp );
					}else{
						//dial fail  => to init state to restart????
						pppomodem_closedev(p);
						pppomodem_reset_value(p);
						pppomodem_set_state(p, POM_ST_INIT);
						return;
					}					
				}
			}else{
				pppomodem_dial_start(p, sp->unit);
			}		
			break;
		}
	case POM_ST_CONNECT:
		{
			if( (sp->fd<0) && (p->ttyfd>=0) )
			{
				//start ppp
				sp->fd = p->ttyfd;
				//like pppd:tty_establish_ppp() 
				pppomodem_tty_establish_ppp_start( p );
				if( start_ppp_real(sp)<0 )
				{
					//ppp fail  => to init state to restart????
					pppomodem_tty_establish_ppp_fail( p );
					pppomodem_closedev(p);
					pppomodem_reset_value(p);
					pppomodem_set_state(p, POM_ST_INIT);
					return;
				}
				pppomodem_tty_establish_ppp_end( p, sp->fd );
			}else if(  (sp->fd<0) && (p->ttyfd<0) )
			{
				if(DBGFLAG) POM_PRINTF( "!!!!!!!!bug fixme!!!!!!!\n");
			}
			break;
		}
	case POM_ST_DISCONNECT:
		{
#if 1
{
			if( p->ttyfd>=0 )
			{
				pppomodem_tty_disestablish_ppp( p );
				pppomodem_closedev(p);
			}else{
				if(DBGFLAG) POM_PRINTF( "error, p=0x%x, p->ttyfd=%d\n", p, p?p->ttyfd:-1 );
			}

			pppomodem_reset_value(p);
			pppomodem_set_state(p, POM_ST_DEAD);
			break;
}
#else
{			//take longer time
			int end_discon=0;
			
			if( p->ttyfd>=0 )
			{
			#if 1
					pppomodem_tty_disestablish_ppp( p );
					pppomodem_disconnect(p, sp->unit);
					if(p->wait_child)
					{
						if(DBGFLAG) POM_PRINTF( "checking disconnector\n" );
						while( pppomodem_dial_isfinished(p->cpid)==0 ) sleep(1);
						if(DBGFLAG) POM_PRINTF( "finish disconnecting\n" );
						p->wait_child=0;
						p->cpid=0;
					}
					end_discon=1;
			#else
				//using this way, sp had removed before finishing disconnector
				if(p->wait_child)
				{
					if(DBGFLAG) POM_PRINTF( "checking disconnector\n" );
					if( pppomodem_dial_isfinished(p->cpid) )
					{
						if(DBGFLAG) POM_PRINTF( "finish disconnecting\n" );
						p->wait_child=0;
						p->cpid=0;
						end_discon=1;
					}
				}else{
					pppomodem_tty_disestablish_ppp( p );
					pppomodem_disconnect(p, sp->unit);
					//fail or hungup
					if(p->wait_child==0) end_discon=1;
				}
			#endif
			}else{
				if(DBGFLAG) POM_PRINTF( "error, p=0x%x, p->ttyfd=%d\n", p, p?p->ttyfd:-1 );
				end_discon=1;
			}


			if(end_discon)
			{
					if(p->ttyfd>=0) pppomodem_closedev(p);
					pppomodem_reset_value(p);
					pppomodem_set_state(p, POM_ST_DEAD);
			}
			break;
}
#endif
		}
	default:
		{
			break;
		}
	}
	return;
}

static void pppomodem_handle_event( struct sppp *sp, int e )
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	if( (sp==NULL) || (sp->over != SPPP_PPPOMODEM) || (p==NULL) ) return;
	
	switch(e)
	{
	case POM_EVENT_OPEN:
		{
			if(DBGFLAG) POM_PRINTF( "POM_EVENT_OPEN, state=%s\n", pppomodem_get_state_str(p->state) );
			switch(p->state)
			{
			case POM_ST_DEAD:
				{
					pppomodem_set_state(p, POM_ST_INIT);
					pppomodem_handle_state( sp );
					break;
				}
			case POM_ST_INIT:
				break;
			case POM_ST_DIAL:
				break;
			case POM_ST_CONNECT:
				{
					pppomodem_handle_state( sp );
					break;
				}
			case POM_ST_DISCONNECT:
			default:
				break;
			}			
			break;
		}
	case POM_EVENT_CLOSE:
		{
			if(DBGFLAG) POM_PRINTF( "POM_EVENT_CLOSE, state=%s\n", pppomodem_get_state_str(p->state) );
			switch(p->state)
			{
			case POM_ST_DEAD:
				break;
			case POM_ST_INIT:
				{
					pppomodem_reset_value(p);
					pppomodem_set_state(p, POM_ST_DEAD);
					break;
				}
			case POM_ST_DIAL:
				{
					if(p->wait_child)
					{
						kill( p->cpid, SIGTERM );
						while( pppomodem_dial_isfinished(p->cpid)==0 ) sleep(1);
						p->wait_child=0;
						p->cpid=0;
					}else{
						//nothing
					}

					pppomodem_closedev(p);
					pppomodem_reset_value(p);
					pppomodem_set_state(p, POM_ST_DEAD);
					break;
				}
			case POM_ST_CONNECT:
				{
					pppomodem_set_state(p, POM_ST_DISCONNECT);
					pppomodem_handle_state( sp );
					break;
				}
			case POM_ST_DISCONNECT:
			default:
				break;
			}
			break;
		}
	default:
		{
			POM_PRINTF( "POM_EVENT_(ILLEGAL) , state=%s\n", pppomodem_get_state_str(p->state) );
			break;
		}
	}

	return;
}

extern struct sppp *spppq;
void pppomodem_process(void)
{
	struct sppp *sp;

	sp = spppq;
	while(sp)
	{
		if (sp->over == SPPP_PPPOMODEM) 
			pppomodem_handle_state(sp);
		sp = sp->pp_next;
	}
   	return;
}

int pppomodem_init(struct sppp *sp)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	if(DBGFLAG) POM_PRINTF( "p->fd=%d, sp->fd=%d\n", p->ttyfd, sp->fd );

	pppomodem_handle_event( sp, POM_EVENT_OPEN );
	return 0;
}

int pppomodem_close(struct sppp *sp)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	pppomodem_handle_event( sp, POM_EVENT_CLOSE );
	return 0;
}

void pppomodem_set_hangup(struct sppp *sp)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(DBGFLAG) POM_PRINTF( "<enter %s>\n", __FUNCTION__ );
	if(p) p->hungup=1;
	return;
}

/*****lcp option, accm***************************************************************/
/********************************************************************
 *
 * ppp_set_xaccm - set the extended transmit ACCM for the interface.
 */
void ppp_set_xaccm (struct sppp *sp, ext_accm accm)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;
	
	if(sp==NULL) return;

	if(DBGFLAG) POM_PRINTF( "set_xaccm: %08lx %08lx %08lx %08lx\n",
							accm[0], accm[1], accm[2], accm[3]);

	if (ioctl(sp->fd, PPPIOCSXASYNCMAP, accm) < 0 && errno != ENOTTY) {
		if ( ! ok_error (errno))
			POM_PRINTF("ioctl(set extended ACCM): %s(%d)", strerror(errno), errno);
	}
}
/********************************************************************
 *
 * ppp_send_config - configure the transmit characteristics of
 * the ppp interface.
 * part of generic_ppp_send_config() for setting send asyncmap
 */
void ppp_send_accm_config (struct sppp *sp, u_int32_t asyncmap)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(sp==NULL) return;

	if(DBGFLAG) POM_PRINTF( "send_config: asyncmap = %lx\n", asyncmap );

	if (ioctl(sp->fd, PPPIOCSASYNCMAP, (caddr_t) &asyncmap) < 0) {
		if (!ok_error(errno))
			POM_PRINTF("ioctl(PPPIOCSASYNCMAP): %s(%d)", strerror(errno), errno);
		return;
	}
}
/********************************************************************
 *
 * ppp_recv_config - configure the receive-side characteristics of
 * the ppp interface.
 * part of generic_ppp_recv_config() for setting recv asyncmap
 */
void ppp_recv_accm_config (struct sppp *sp, u_int32_t asyncmap)
{
	struct pppomodem_param_s *p=sp?sp->dev:NULL;

	if(sp==NULL) return;

	if(DBGFLAG) POM_PRINTF( "recv_config: asyncmap = %lx\n", asyncmap);
	
	if (ioctl(sp->fd, PPPIOCSRASYNCMAP, (caddr_t) &asyncmap) < 0) {
		if (!ok_error(errno))
			POM_PRINTF("ioctl(PPPIOCSRASYNCMAP): %s(%d)", strerror(errno),errno);
	}
}
/*****end lcp option, accm***************************************************************/




#endif //CONFIG_USER_PPPOMODEM
