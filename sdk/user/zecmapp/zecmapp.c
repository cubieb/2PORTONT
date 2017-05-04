#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define TTY_DEV_NAME "/dev/ttyUSB1"
#define ECM_NET_DEV_NAME "usb0"
#define SPEED 4098 //if baud rate is 115200


extern const char DHCPC_SCRIPT_NAME[]; 
extern const char DHCPC_PID[];
extern const char DHCPC_ROUTERFILE[];
extern const char DHCPC_SCRIPT[];
extern const char DHCPC_SCRIPT_NAME[];
extern const char IFCONFIG[];
extern const char ARG_I[];
extern const char DHCPC[];

// set raw tty mode
static void xget1(int fd, struct termios *t, struct termios *oldt)
{
	tcgetattr(fd, oldt);
	*t = *oldt;
	cfmakeraw(t);
}


static int xset1(int fd, struct termios *tio, const char *device)
{
	int ret = tcsetattr(fd, TCSAFLUSH, tio);

	if (ret) {
		perror("can't tcsetattr for %s", device);
	}
	return ret;
}

static int atcommand_io(char *command, char *result, int result_len)
{
	int sfd;
	struct termios tio0, tiosfd, tio;
	int timeout = 5000;
	int len=0;
	char c;

	// open device
	sfd = open(TTY_DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK, 0666);
	if (sfd < 0){
		goto open_error;
	}
	fcntl(sfd, F_SETFL, 0);

	// put device to "raw mode"
	xget1(sfd, &tio, &tiosfd);
	// set device speed
	cfsetspeed(&tio, SPEED);
	if (xset1(sfd, &tio, TTY_DEV_NAME))
		goto done;

	//send command
	write(sfd, command, strlen(command));
	c=13; //End of Command
	write(sfd, &c, 1);

	sleep(3);
	//read result
	memset(result,0,result_len);
	len = read(sfd, result, result_len);
	if (len > 0)
		;
	// restore device mode
	tcsetattr(sfd, TCSAFLUSH, &tiosfd);

done:

	close(sfd);
	
	printf("->%s\n",command);
	printf("<-%s\n", result);

	return 1;

open_error:
	printf("Error! Open %s Fail!\n",TTY_DEV_NAME);
	return 0;
}


static void generate_udhcpc_script(char *fname)
{
	int fh;
	char buff[64];

	fh = open(fname, O_RDWR|O_CREAT|O_TRUNC, S_IXUSR);
	if (fh == -1) {
		printf("Create udhcpc script file %s error!\n", fname);
		return;
	}

	WRITE_DHCPC_FILE(fh, "#!/bin/sh\n");
	snprintf(buff, 64, "RESOLV_CONF=\"/var/udhcpc/resolv.conf.%s\"\n", ECM_NET_DEV_NAME);
	WRITE_DHCPC_FILE(fh, buff);
	WRITE_DHCPC_FILE(fh, "[ \"$broadcast\" ] && BROADCAST=\"broadcast $broadcast\"\n");
	WRITE_DHCPC_FILE(fh, "[ \"$subnet\" ] && NETMASK=\"netmask $subnet\"\n");
	WRITE_DHCPC_FILE(fh, "ifconfig $interface $ip $BROADCAST $NETMASK -pointopoint\n");
	WRITE_DHCPC_FILE(fh, "MER_GW_INFO=\"/tmp/MERgw.\"$interface\n");	// Jenny, write MER1483 gateway info

#ifdef NEW_PORTMAPPING
	tableId = caculate_tblid(pEntry->ifIndex);
	snprintf(iproutecmd, 80, "\tip route add default dev $interface via $i table %d\n", tableId);
#endif
	WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
#ifdef DEFAULT_GATEWAY_V2
	if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
		WRITE_DHCPC_FILE(fh, "\troute del default\n");
#endif
	WRITE_DHCPC_FILE(fh, "\twhile route del -net default gw 0.0.0.0 dev $interface\n");
	WRITE_DHCPC_FILE(fh, "\tdo :\n");
	WRITE_DHCPC_FILE(fh, "\tdone\n\n");
	WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
	WRITE_DHCPC_FILE(fh, "\tdo\n");
	WRITE_DHCPC_FILE(fh, "\tifconfig $interface pointopoint $i\n");
	WRITE_DHCPC_FILE(fh, "\troute add -net default gw $i dev $interface\n");
#ifdef NEW_PORTMAPPING
	WRITE_DHCPC_FILE(fh, iproutecmd);
#endif
	WRITE_DHCPC_FILE(fh, "\tdone\n");
	WRITE_DHCPC_FILE(fh, "\tifconfig $interface -pointopoint\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	WRITE_DHCPC_FILE(fh, "if [ \"$dns\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "\trm $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, "\tfor i in $dns\n");
	WRITE_DHCPC_FILE(fh, "\tdo\n");
	WRITE_DHCPC_FILE(fh, "\techo 'DNS=' $i\n");
	WRITE_DHCPC_FILE(fh, "\techo nameserver $i >> $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, "\tdone\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	//napt
	WRITE_DHCPC_FILE(fh, "iptables -t nat -A POSTROUTING -j MASQUERADE\n");



	close(fh);
	chmod(fname,0777);
}

static int startDhcpc(char *inf)
{
	unsigned char value[32], value2[32];
	char * argv[9];

	argv[1] = inf;
	argv[2] = "up";
	argv[3] = NULL;
	do_cmd(IFCONFIG, argv, 1);

	// udhcpc -i vc0 -p pid -s script
	argv[1] = (char *)ARG_I;
	argv[2] = inf;
	argv[3] = "-p";
	snprintf(value2, 32, "%s.%s", (char*)DHCPC_PID, inf);
	argv[4] = (char *)value2;
	argv[5] = "-s";
	snprintf(value, 32, "%s.%s",(char *)DHCPC_SCRIPT_NAME, ECM_NET_DEV_NAME);
	generate_udhcpc_script(value);
	argv[6] = (char *)DHCPC_SCRIPT;
	argv[7] = NULL;

	do_cmd(DHCPC, argv, 0);

	return 1;
}

int do_at_commands()
{
	char result[256]={0};
	char *command[] = {
	"at+cgdcont=1,\"ip\",\"internet\"",
	"at+zsnt=0,0,0",
	"at+cfun=1",
	"at+zpas?",
	"at+zecmcall=1",
	"at+zecmcall?"};	

	#define PASSCODE "+ZECMCALL: IPV4"
	
	int ret=0;

	atcommand_io(command[0],result, sizeof(result));
	atcommand_io(command[1],result, sizeof(result));
	atcommand_io(command[2],result, sizeof(result));
	atcommand_io(command[3],result, sizeof(result));
	atcommand_io(command[4],result, sizeof(result));
	atcommand_io(command[5],result, sizeof(result));
	
	if(strstr(result,PASSCODE)>0){
		printf("Connected!\n");	
		ret =1;
	}
	else
		printf("Connected fail!\n");

	return ret;
}

int main(int argc, char **argv)
{	
	int ret;

retry:
	//Step 1, First using at commands to change zm8620 to ecm mode
	ret = do_at_commands();
	if(ret == 1)
	{
		int pid;
		//Step 2, then use dhcp client to request IP. This step is necessary
		//			after this step, then coulud connect to the Internet.
		pid = read_pid((char *)DHCPC_PID);
		if (pid > 0)
		{
			kill(pid, 9);
		}

		//enable IFF_DOMAIN_WAN for interface usb0, then could flash INTERNET LED
		system("/bin/ethctl enable_usb0_wan 1");
		startDhcpc(ECM_NET_DEV_NAME);
	}
	else{ 
		sleep(3);
		goto retry;
	}

	return 0;
}
