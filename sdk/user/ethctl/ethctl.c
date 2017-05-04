
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#include <signal.h>      // Added by Mason Yu for p2r_test
#include "../../include/linux/autoconf.h"

struct arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
	unsigned int cmd5;
}pass_arg;
#define tx_cmd 1
#define rx_cmd 2
#define enable_cmd 3
#define disable_cmd 4

#define SIOCETHTEST	0x89b1

#ifdef CONFIG_RTL_MULTI_WAN
#define SIOCSIFSMUX	0x89c1

enum smux_ioctl_cmds {
  ADD_SMUX_CMD,
  REM_SMUX_CMD,
};

enum smux_proto_types {
  SMUX_PROTO_PPPOE,
  SMUX_PROTO_IPOE,
  SMUX_PROTO_BRIDGE,
};

struct smux_ioctl_args {
  int cmd; /* Should be one of the smux_ioctl_cmds enum above. */
  int proto;
  int vid; /* vid==-1 means vlan disabled on this dev. */
  int napt;
  int brpppoe;
  char ifname[IFNAMSIZ]; /* real device */
  union {
    char ifname[IFNAMSIZ]; /* overlay device */
  } u;
};
#endif

#ifdef CONFIG_RTL867X_VLAN_MAPPING
#define SIOCSIFVLANMAP	0x89c2

enum re_vlan_ioctl_cmds {
	RE_VLAN_ADD_VLANPAIR_CMD = 1,
	RE_VLAN_DEL_VLANPAIR_CMD,
	RE_VLAN_DEL_VLANDEV_CMD,
	RE_VLAN_SHOW_ALL_CMD
};

struct re_vlan_ioctl_args {
	enum re_vlan_ioctl_cmds cmd;
	char ifname[IFNAMSIZ];
	unsigned short lanVid;
	unsigned short wanVid;
};
#endif


struct portmir
{
	/*
	*	0: mirror off;
	*	1: mirror out;
	*	2: mirror in;
	*	3: both direction;
	*/
	int  port_mirror; 
	char mir_dev_name[10];
};



#define PATH_MFCV6D_PID "/var/run/mfcv6d.pid"   // Added by Mason Yu for p2r_test
#define PATH_RADVD_PID "/var/run/radvd.pid"
#define PATH_ECMH_PID "/var/run/ecmh.pid"		// Added by Mason Yu for ecmh

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

void usage(){
	printf("Usage:\n");
	printf("tx <packet_count>\n\tsend number of <packet_count> packets\n");
	printf("rx <time>\n\tgather statistics of rx packets in <time> seconds\n");
	printf("on <phy>\n\tturn on the <phy> port of the switch\n");
	printf("off <phy>\n\tturn off the <phy> port of the switch\n");
	printf("r <phy> <reg> [<page>]\n\tread <reg> page <page> value of the <phy> port of the switch\n");
	printf("w <phy> <reg> <hex-val> [<page>]\n\twrite <hex-val> to <reg> page <page> of the <phy> port of the switch\n");
	printf("led\n\tturn on all LED\n");
	printf("gpio <index> <0/1>\n\tGPIO pin test\n");
	printf("conntrack <killall/killtcpudp/killicmp>\n\tkill all/tcpand udp/icmp conntrack sessions\n");
//	printf("conntrack <killall/killtcpudp/killicmp/killdns>\n\tkill all/tcpand udp/icmp/dns conntrack sessions\n");
	printf("route <add/delete>\n\tadd/delete static Route\n");
	printf("dumpqos\n\tdump RTL8306 QoS\n");
	printf("resetcnt\n\treset RTL8306 MIB counter\n");
	printf("dumpcnt\n\tdump RTL8306 MIB counter\n");
	printf("vlaninfo\n\tdump VLAN info.\n");
	printf("igmpinfo\n\tdump IGMP info.\n");
	printf("debug igmp <0/1>\n");
	printf("switch <vi/le>\n\tdump switch's vlan info/learning entry\n");
	printf("pkt <no/all/tx/rx>\n\tdump nic <no/all/tx only/rx only> packets\n");
#ifdef CONFIG_RTL_MULTI_WAN
	printf("addsmux protocol real_dev smux_dev [napt] [brpppoe] [vlan vid]\n\tprotocol: bridge/ipoe/pppoe\n\treal_dev: smux device name\n\tsmux_dev: real dev name\n");
	printf("remsmux protocol real_dev smux_dev [vlan vid]\n\tprotocol: bridge/ipoe/pppoe\n\treal_dev: smux device name\n\tsmux_dev: real dev name\n");
#endif
#ifdef CONFIG_RTL867X_VLAN_MAPPING
	printf("vlanmap <add_pair/del_pair> <interface> <lanVid> <wanVid>\n\t<del_dev> <interface>\n\t<show>\n");
#endif
#if defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
	printf("get_mac <hex-reg>\n\t<hex-reg> = 0xXXXX\n");
	printf("set_mac <hex-reg> <hex-val>\n\t<hex-reg> = 0xXXXX, <hex-val> = 0xXXXX\n");
#endif
	printf("mirror <wan_dev> <lan_dev> <action>\n\tMirror <wan_dev> traffic to <lan_dev>.\n\taction = 0: off, 1: mirror tx, 2: mirror rx, 3: mirror both\n");
#ifdef CONFIG_RTL8685_PTM_MII
	printf("ptmqmap nas0_x <path> <pri0-q>...<pri7-q>\n");
	printf("\t<path>=0/1\n");
	printf("\t<prix-q>=0~3\n");
#endif /*CONFIG_RTL8685_PTM_MII*/
	printf("enable_usb0_wan 0/1 \n");
}

int main(int argc, char *argv[])
{
	char *pEnd;
  	struct ifreq	ifr;
  	int fd=-1,i,repeat=0,times;

	if (argc<=1)
		goto arg_err_rtn;
#ifdef CONFIG_RTL_MULTI_WAN
	if (!strcmp(argv[1], "addsmux") || !strcmp(argv[1], "remsmux"))
	{
		struct smux_ioctl_args sifr = {0};
		int idx = 0;

		if (argc < 5)
			goto arg_err_rtn;
		
		fd = socket(AF_INET, SOCK_DGRAM, 0);
  		if(fd< 0){
			printf("Error!Socket create fail in ethctl.\n");
        		return 1;
        	}

		if (!strcmp(argv[1], "addsmux"))
			sifr.cmd = ADD_SMUX_CMD;
		else
			sifr.cmd = REM_SMUX_CMD;
		
		if (!strcmp(argv[2], "pppoe"))
			sifr.proto = SMUX_PROTO_PPPOE;
		else if (!strcmp(argv[2], "ipoe"))
			sifr.proto = SMUX_PROTO_IPOE;
		else if (!strcmp(argv[2], "bridge"))
			sifr.proto = SMUX_PROTO_BRIDGE;
		else
			goto arg_err_rtn;

		strcpy(sifr.ifname, argv[3]);
		strcpy(sifr.u.ifname, argv[4]);

		idx =5;
		sifr.vid = -1;
		while(idx < argc)
		{
			if(!strcmp(argv[idx],"napt"))
				sifr.napt=1;
			else if(!strcmp(argv[idx],"brpppoe"))
				sifr.brpppoe = 1;
			else if(!strcmp(argv[idx], "vlan") && idx+1 < argc)
			{
				idx++;
				sifr.vid = atoi(argv[idx]);
#if 0
				if (sifr.vid <= 9)
				{
					printf("vid 0-9 are reserved.\n");
					goto arg_err_rtn;
				}
#endif
			}
			idx++;
		}

		if (ioctl(fd, SIOCSIFSMUX, &sifr) < 0) {
			printf("Error ioctl(SIOCSIFSMUX) in ethctl\n");
			close(fd);
			return 1;
		}
		close(fd);
		return 0;
	}
#endif
	if (strcmp(argv[1],"tx")==0) {
	    if (argc!=3) goto arg_err_rtn;
		pass_arg.cmd=1;
		if(atoi(argv[2])>0){
			pass_arg.cmd2=atoi(argv[2]);
			repeat=pass_arg.cmd2;
		}
	}
	if (strcmp(argv[1],"rx")==0) {
	    if (argc!=3) goto arg_err_rtn;
		pass_arg.cmd=2;
		repeat=1;
		if(atoi(argv[2])>0)
			pass_arg.cmd2=atoi(argv[2]);
	}
	if (strcmp(argv[1],"on")==0) {
	    if (argc!=3) goto arg_err_rtn;
		pass_arg.cmd=3;
		repeat=1;
		if(atoi(argv[2])>=0&&atoi(argv[2])<4)
			pass_arg.cmd2=atoi(argv[2]);
		else{
			goto arg_err_rtn;
		}
	}
	if (strcmp(argv[1],"off")==0) {
	    if (argc!=3) goto arg_err_rtn;
		pass_arg.cmd=4;
		repeat=1;
		if(atoi(argv[2])>=0&&atoi(argv[2])<4)
			pass_arg.cmd2=atoi(argv[2]);
		else{
			goto arg_err_rtn;
		}
	}
	if (strcmp(argv[1],"r")==0) {
	    if (argc<4) goto arg_err_rtn;
		pass_arg.cmd=5;
		repeat=1;
		if (atoi(argv[2]) < 0 || atoi(argv[2]) > 6)
			goto arg_err_rtn;
		pass_arg.cmd2=atoi(argv[2]);  //port #
		pass_arg.cmd3=atoi(argv[3]);  //reg #
		if (argc >= 5)
			pass_arg.cmd4=atoi(argv[4]);  //reg page #
	}
	if (strcmp(argv[1],"w")==0) {
	    if (argc<5) goto arg_err_rtn;
		pass_arg.cmd=6;
		repeat=1;
		if (atoi(argv[2]) < 0 || atoi(argv[2]) > 6)
			goto arg_err_rtn;
		pass_arg.cmd2=atoi(argv[2]);  //port #
		pass_arg.cmd3=atoi(argv[3]);  //reg # 		
		pass_arg.cmd4=strtol(argv[4], 0, 16); // hexdecimal value
		if (argc >= 6)
			pass_arg.cmd5=atoi(argv[5]);  //reg page #
	}
	if (strcmp(argv[1],"led")==0) {
		if (argc!=3) goto arg_err_rtn;
		if(strcmp(argv[2],"on")==0){
			system("ifconfig wlan0 down");
			pass_arg.cmd=7;
			repeat=1;
		}
		if(strcmp(argv[2],"flash")==0){
			system("ifconfig wlan0 down");
			pass_arg.cmd=9;
			repeat=1;
		}
        	
	}	
        if (strcmp(argv[1],"wifi")==0) {
		pass_arg.cmd=8;
		pass_arg.cmd2=atoi(argv[2]); //0: normal case 1:wifi test
		repeat=1;
	}
        if (strcmp(argv[1],"gpio")==0) {
		if (argc!=4) goto arg_err_rtn;
		pass_arg.cmd=10;
		pass_arg.cmd2=atoi(argv[2]); // gpio index
		pass_arg.cmd3=atoi(argv[3]); // value
		repeat=1;
	}
	// Added by Mason Yu for PPP LED on/off
	if (strcmp(argv[1],"ppp")==0) {
		if (argc!=3) goto arg_err_rtn;
			repeat=1;
		if(strcmp(argv[2],"on")==0){
			pass_arg.cmd=12;
		}
		if(strcmp(argv[2],"off")==0){
			pass_arg.cmd=13;
		}
	}
	
	// Added by Mason Yu for Kill ip conntrack session
	if (strcmp(argv[1],"conntrack")==0) {
		if (argc!=3) goto arg_err_rtn;
		repeat=1;
		if(strcmp(argv[2],"killall")==0){
			pass_arg.cmd=21;
		}
		if(strcmp(argv[2],"killtcpudp")==0){
			pass_arg.cmd=22;
		}
		if(strcmp(argv[2],"killicmp")==0){
			pass_arg.cmd=23;
		}
#if 0
		if(strcmp(argv[2],"killdns")==0){
			pass_arg.cmd=24;
		}
#endif
	}
	
#ifdef ROUTING
#ifdef CONFIG_USER_BOA_SRC_BOA
	// Mason Yu
	// Added by Mason Yu for static route on/off
	if (strcmp(argv[1],"route")==0) {
		if (argc!=3) goto arg_err_rtn;
		repeat=1;
		if(strcmp(argv[2],"add")==0){		    		    
			addStaticRoute();
			return 0; 
		}
		if(strcmp(argv[2],"delete")==0){		   
			deleteStaticRoute();
			return 0; 
		}
	}
#endif //CONFIG_USER_BOA_SRC_BOA
#endif

	// Kaohj --- dump 8306 qos values
        if (strcmp(argv[1],"dumpqos")==0) {
		pass_arg.cmd=26;
		repeat=1;
        }
        // Kaohj --- reset 8306 MIB Counter
        if (strcmp(argv[1],"resetcnt")==0) {
		pass_arg.cmd=27;
		repeat=1;
        }
        // Kaohj --- dump 8306 MIB Counter
        if (strcmp(argv[1],"dumpcnt")==0) {
		pass_arg.cmd=28;
		repeat=1;
        }
        if (strcmp(argv[1],"vlaninfo")==0) {
		pass_arg.cmd=29;
		repeat=1;
        }
        if (strcmp(argv[1],"igmpinfo")==0) {
		pass_arg.cmd=30;
		repeat=1;
        }

	//jim reserve cmd  31-39 for polling link status in e8b's diagnosing ethernet ports' status......
	//
        if (strcmp(argv[1],"debug")==0) {
		if (argc!=4) goto arg_err_rtn;
		pass_arg.cmd=40;
	        if (strcmp(argv[2],"igmp")==0)
			pass_arg.cmd2=1; // igmp
		else
			goto arg_err_rtn;
		pass_arg.cmd3=atoi(argv[3]); // value
		repeat=1;
        }
        if (strcmp(argv[1], "enable")==0) { // NIC phy power up
		pass_arg.cmd=41;
		repeat=1;
        }
        if (strcmp(argv[1], "disable")==0) { // NIC phy power down
		pass_arg.cmd=42;
		repeat=1;
        }

	// Added by Mason Yu for p2r_test to start/stop mfcv6d
#ifdef CONFIG_IPV6
#if defined(CONFIG_USER_IPV6READYLOGO_ROUTER) || defined(CONFIG_USER_IPV6READYLOGO_HOST)
	// Added by Mason Yu for ecmh
	if (strcmp(argv[1],"ecmh")==0) {
		int pid;
		
		if (argc!=3) goto arg_err_rtn;
		repeat=1;
		if(strcmp(argv[2],"start")==0){		
			va_cmd("/bin/ecmh", 4, 0, "-i", "nas0_0","-o","br0");
			return 0; 
		}
		if(strcmp(argv[2],"stop")==0){		
			pid = read_pid(PATH_ECMH_PID);
			if ( pid > 0)
				kill(pid, SIGKILL);
			return 0; 
		}
	}
	
	if (strcmp(argv[1],"mfcv6d")==0) {
		int pid;
		
		if (argc!=3) goto arg_err_rtn;
		repeat=1;
		if(strcmp(argv[2],"start")==0){		
			va_cmd("/bin/mfcv6d", 3, 0, "br0", "vc0", "&");
			return 0; 
		}
		if(strcmp(argv[2],"stop")==0){		
			pid = read_pid(PATH_MFCV6D_PID);
			if ( pid > 0)
				kill(pid, SIGTERM);
			return 0; 
		}
	}
	
#ifdef CONFIG_USER_RADVD
	if (strcmp(argv[1],"radvd")==0) {
		int pid;
		unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
		unsigned char vChar;
		
		if (argc!=3 && argc!=4) goto arg_err_rtn;
		repeat=1;
		if(strcmp(argv[2],"start")==0){
			// Setup radvd.conf for radvd
			setup_radvd_conf(0);
			// /bin/radvd -s -C /var/radvd.conf		
			va_cmd("/bin/radvd", 3, 0, "-s", "-C", "/var/radvd.conf");
			return 0; 
		}
		if(strcmp(argv[2],"stop")==0){		
			pid = read_pid(PATH_RADVD_PID);
			if ( pid > 0)
				kill(pid, SIGTERM);
			return 0; 
		}
		if(strcmp(argv[2],"reset")==0){		
			init_radvd_conf_mib();
			return 0; 
		}
		if(strcmp(argv[2],"maxinterval")==0){		
			mib_set(MIB_V6_MAXRTRADVINTERVAL, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"mininterval")==0){		
			mib_set(MIB_V6_MINRTRADVINTERVAL, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"chlim")==0){		
			mib_set(MIB_V6_ADVCURHOPLIMIT, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"rltime")==0){		
			mib_set(MIB_V6_ADVDEFAULTLIFETIME, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"rtime")==0){		
			mib_set(MIB_V6_ADVREACHABLETIME, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"retrans")==0){		
			mib_set(MIB_V6_ADVRETRANSTIMER, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"linkmtu")==0){		
			mib_set(MIB_V6_ADVLINKMTU, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"flagO")==0){
			vChar = argv[3][0] - '0';		
			mib_set(MIB_V6_MANAGEDFLAG, (void *)&vChar);		
			return 0; 
		}
		if(strcmp(argv[2],"flagM")==0){
			vChar = argv[3][0] - '0';		
			mib_set(MIB_V6_OTHERCONFIGFLAG, (void *)&vChar);		
			return 0; 
		}
		if(strcmp(argv[2],"flagOnlink")==0){
			vChar = argv[3][0] - '0';	    	
			mib_set(MIB_V6_ONLINK, (void *)&vChar);		
			return 0; 
		}
		if(strcmp(argv[2],"flagAuto")==0){
			vChar = argv[3][0] - '0';	
			mib_set(MIB_V6_AUTONOMOUS, (void *)&vChar);		
			return 0; 
		}
		if(strcmp(argv[2],"prefixIP")==0){		
			mib_set(MIB_V6_PREFIX_IP, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"prefixLen")==0){		
			mib_set(MIB_V6_PREFIX_LEN, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"pltime")==0){		
			mib_set(MIB_V6_PREFERREDLIFETIME, (void *)argv[3]);		
			return 0; 
		}
		if(strcmp(argv[2],"vltime")==0){		
			mib_set(MIB_V6_VALIDLIFETIME, (void *)argv[3]);		
			return 0; 
		}
	}
#endif
#endif
#endif // CONFIG_IPV6

	if (strcmp(argv[1], "switch")==0) { // 
		if(strcmp(argv[2], "vi")==0){//switch vlan info
			pass_arg.cmd=43;
		}
		if(strcmp(argv[2], "le")==0){//switch learning entry
			pass_arg.cmd=44;
		}
		repeat=1;
	}
	if (strcmp(argv[1], "pkt")==0) { //
		pass_arg.cmd=45;
		if(strcmp(argv[2], "no")==0){//
			pass_arg.cmd2=0; 
		}
		if(strcmp(argv[2], "all")==0){//
			pass_arg.cmd2=1;
		}
		if(strcmp(argv[2], "tx")==0){//
			pass_arg.cmd2=2;
		}
		if(strcmp(argv[2], "rx")==0){//
			pass_arg.cmd2=3;
		}
		repeat=1;
	}

	//set enable_ipqos flag when GMAC driver is not built-in
	if (strcmp(argv[1],"setipqos")==0) {
		if (argc!=3) goto arg_err_rtn;
		pass_arg.cmd=50;
		repeat=1;
		pass_arg.cmd2=atoi(argv[2]);
	}

#ifdef CONFIG_RTL867X_VLAN_MAPPING
	if (!strcmp(argv[1], "vlanmap")) {
		struct re_vlan_ioctl_args arg;
		
		if (argc <= 2)
			goto arg_err_rtn;
		if (!strcmp(argv[2], "add_pair")) {
			if (argc != 6)
				goto arg_err_rtn;
			arg.cmd = RE_VLAN_ADD_VLANPAIR_CMD;
		}
		else if (!strcmp(argv[2], "del_pair")) {
			if (argc != 6)
				goto arg_err_rtn;
			arg.cmd = RE_VLAN_DEL_VLANPAIR_CMD;
		}
		else if (!strcmp(argv[2], "del_dev")) {
			if (argc != 4)
				goto arg_err_rtn;
			arg.cmd = RE_VLAN_DEL_VLANDEV_CMD;
			strcpy(arg.ifname, argv[3]);
		}
		else if (!strcmp(argv[2], "show")) {
			if (argc != 3)
				goto arg_err_rtn;
			arg.cmd = RE_VLAN_SHOW_ALL_CMD;
		}
		else
			goto arg_err_rtn;

		fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(fd< 0){
			printf("Error!Socket create fail in ethctl.\n");
			return 1;
        	}

		if (argc == 6) {
			strcpy(arg.ifname, argv[3]);
			arg.lanVid = atoi(argv[4]);
			arg.wanVid = atoi(argv[5]);
		}
		
		if (ioctl(fd, SIOCSIFVLANMAP, &arg) < 0) {
			printf("Error ioctl(SIOCSIFVLANMAP) in ethctl\n");
			close(fd);
			return 1;
		}
		close(fd);
		return 0;
	}
#endif
#if defined(CONFIG_RTL_819X_SWCORE) && defined(CONFIG_RTL_8367B)
	if (!strcmp(argv[1], "get_mac")) {
		if (argc != 3) goto arg_err_rtn;
		pass_arg.cmd=51;
		repeat=1;
		pass_arg.cmd2=s2i(argv[2]);  //reg #: 0xXXXX
	}
	if (!strcmp(argv[1], "set_mac")) {
		if (argc != 4) goto arg_err_rtn;
		pass_arg.cmd=52;
		repeat=1;
		pass_arg.cmd2=s2i(argv[2]);  //reg #: 0xXXXX		
		pass_arg.cmd3=s2i(argv[3]); // hexdecimal value
	}
#endif

	if (!strcmp(argv[1], "enable_usb0_wan")) {
		if (argc != 3) goto arg_err_rtn;
		repeat=1;
		pass_arg.cmd=53;
		pass_arg.cmd2=atoi(argv[2]);
	}
	/* --------------------------------- for test -------------------------------------*/
	if (strcmp(argv[1],"link")==0) {
		if (argc!=3) goto arg_err_rtn;
		repeat = get_net_link_status(argv[2]);
		printf("link %s: %d\n", argv[2], repeat);
		return;
	}
	/* -------------------------------- end of test -----------------------------------*/


#ifndef SIOCPORTMIRROR
#define SIOCPORTMIRROR (0x89F0 + 0xf)
#endif
	if (!strcmp(argv[1], "mirror"))
	{
		int val; 

		if (argc != 5)
		{
			printf("error 1: the param must be 5!\n");
			goto arg_err_rtn;
		}
			

		val = atoi(argv[4]);

		if((val < 0) || (val > 3))
		{
			printf("error 2: Invalid direction!\n");
			goto arg_err_rtn;
		}
			

		// smux wan interface, ex. nas0_1, ptm0_0 ...
		if (strstr(argv[2], "_"))
		{
			int skfd, ret;
			struct ifreq ifr;
			struct portmir pmir;

			strcpy(ifr.ifr_name, argv[2]);

			//AUG_PRT("the ifr.name is %s\n", ifr.ifr_name);
			
			ifr.ifr_data = (char *)&pmir;
			pmir.port_mirror = val;

			strcpy(pmir.mir_dev_name, argv[3]);

			if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				printf("socket create failed!\n");
				return 1;
			}

			if((ret = ioctl(skfd, SIOCPORTMIRROR, &ifr)) != 0){
				printf("some inner error! err is %d\n", ret);
				close(skfd);
				return 1;
			}

			printf("command succeed!\n");
			close(skfd);
			return 0;
		}

		printf("error 3: Invalid device name!\n");
		return 0;
	}
#ifdef CONFIG_RTL8685_PTM_MII
#ifndef SIOCSPTMQMAP
#define SIOCSPTMQMAP 0x89FE
#endif
	//ethctl ptmqmap nas0_0  <path> <pri0-q>...<pri7-q>
	if (!strcmp(argv[1], "ptmqmap"))
	{	
		int skfd, ret, i;
		struct ifreq ifr;
		unsigned char qmap[9], tmp;

		if(argc!=5)
		{
			printf("error 1: the parameter num must be 5!\n");
			goto arg_err_rtn;
		}

		strcpy(ifr.ifr_name, argv[2]);
		ifr.ifr_data = (char *)qmap;
		memset(qmap, 0, sizeof(qmap));

		//path
		tmp=(unsigned char)atoi(argv[3]);
		qmap[0]=tmp;
		if(tmp<200) //tmp>=200 for debug
		{
			if(tmp>1)
			{
				printf("error 2: path=0 or 1!\n");
				goto arg_err_rtn;
			}

			//qid
			if(strlen(argv[4])<8)
			{
				printf("error 3: the qid parameter must be 8 in length!\n");
				goto arg_err_rtn;
			}
			for(i=1; i<9; i++)
			{
				unsigned char strval[2];
				strval[0]=argv[4][i-1];
				strval[1]=0;
				tmp=(unsigned char)atoi(strval);
				if(tmp>3)
				{
					printf("error 4: qid=0~3!\n");
					goto arg_err_rtn;
				}
				qmap[i]=tmp;
			}
		}
		if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
		{
			printf("socket create failed!\n");
			return 1;
		}
		
		if((ret = ioctl(skfd, SIOCSPTMQMAP, &ifr)) != 0)
		{
			printf("some inner error! err is %d\n", ret);
			close(skfd);
			return 1;
		}
		
		printf("command succeed!\n");
		close(skfd);
		return 0;
	}
#endif /*CONFIG_RTL8685_PTM_MII*/

	if (!repeat)
		goto arg_err_rtn;
#ifdef CONFIG_USER_BOA_SRC_BOA
	pEnd = ELANVIF[0];
#else
	pEnd="eth0";
#endif

	fd = socket(AF_INET, SOCK_DGRAM, 0);
  	if(fd< 0){
		printf("Error!Socket create fail in ethctl.\n");
  	}
	
	strncpy(ifr.ifr_name, pEnd, IFNAMSIZ);
	ifr.ifr_data = (void *)&pass_arg;

	for(i=0;i<repeat;i++) {
		if (ioctl(fd, SIOCETHTEST, &ifr) < 0) {
			printf("Error ioctl in ethctl\n");
			close(fd);
			return 1;
		};
	}
	close(fd);
	return 0;        


arg_err_rtn:
	if (fd >=0)
		close(fd);
	usage();
	exit(1);
}

