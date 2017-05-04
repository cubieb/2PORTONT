
#ifdef __mips__
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mibtbl.h"
#include "utility.h"

int read_pid(char *filename)
{
	int fh;
	FILE *in;
	int pid;

	fh = open(filename, O_RDWR);
	if ( fh == -1 ) return -1;
	if ((in = fdopen(fh, "r")) == NULL) return -1;
	fscanf(in, "%d", &pid);
	fclose(in);
	close(fh);

	return pid;
}

int genscript_dhcpc(int argc, char** argv, int argNum)
{
#define _DHCPC_PIDFILE_				("/var/udhcpc/udhcpc.pid")
#define _DHCPC_SCRIPT_PATH_			("/var/udhcpc")

	char *enable;
	char *ifname;
	char *filename;
	int fh;
	FILE *out;
	int pid;

	if(argc < (argNum+2)) {
		printf("Usage: genscript dhcpc on/off interface filename\n");
		return -1;
	}

	enable = argv[argNum++];
	ifname = argv[argNum++];
	filename = argv[argNum++];
	pid = read_pid(_DHCPC_PIDFILE_);

	fh = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	if ( fh == -1 ) {
		printf("Error: File open fail! %s\n",filename);
		return -1;
	}

	if ((out = fdopen(fh, "w")) != NULL) {
		if(pid != -1)
			fprintf(out, "kill -9 %d\n", pid);
	
		if( strcmp(enable, "off") != 0) {		
			fprintf(out, "touch %s/%s.sh \n" ,_DHCPC_SCRIPT_PATH_, ifname); 
			fprintf(out, "udhcpc -a -i %s -p %s -s %s/%s.sh &\n"
				,ifname
				,_DHCPC_PIDFILE_
				,_DHCPC_SCRIPT_PATH_, ifname);
		}
		 
		fclose(out);
	}
	lockf(fh, F_UNLCK, 0);
	close(fh);
	
	return 0;
}

int genscript_dhcpd(int argc, char** argv, int argNum)
{
#define _DHCPD_PIDFILE_					("/var/udhcpd/udhcpd.pid")
#define _DHCPD_SCRIPT_PATH_			("/var/udhcpd")
#define _DHCPD_CONFIG_FILE_			("/var/udhcpd/udhcpd.conf")
#define _DHCPD_LEASE_FILE_				("/var/udhcpd/udhcpd.leases")

	char *enable;
	char *ifname;
	char *filename;
	int fh;
	FILE *out;
	int pid;

	unsigned char vChar;
	struct in_addr ip_addr;

	if(argc < (argNum+2)) {
		printf("Usage: genscript dhcpd on/off interface filename\n");
		return -1;
	}

	enable = argv[argNum++];
	ifname = argv[argNum++];
	filename = argv[argNum++];
	pid = read_pid(_DHCPD_PIDFILE_);

	if( strcmp(enable, "off") != 0) {		
		if(mib_init() != 1) goto fail; /* Initialize */

		mib_get(MIB_ADSL_LAN_DHCP, (void *)&vChar); /* get LAN DHCP MODE*/

		if(vChar != 2) { /* DHCP Server */
			goto fail;
		}

		/* Prepare DHCP Server Config File */
		fh = open(_DHCPD_CONFIG_FILE_, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	 	if ( fh == -1 ) goto fail;

		if ((out = fdopen(fh, "w")) != NULL) {
			fprintf(out, "interface %s\n", ifname);

			mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&ip_addr);
			fprintf(out, "start %s\n", inet_ntoa(ip_addr));

			mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&ip_addr);
			fprintf(out, "end %s\n", inet_ntoa(ip_addr));

			mib_get(MIB_ADSL_LAN_SUBNET, (void *)&ip_addr);
			fprintf(out, "opt subnet %s\n", inet_ntoa(ip_addr));

			mib_get(MIB_ADSL_LAN_IP, (void *)&ip_addr);
			fprintf(out, "opt router %s\n", inet_ntoa(ip_addr));

			mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar);
			if(vChar == 0) {	/* Auto DNS Mode, DNS relay  */
				fprintf(out, "opt dns %s\n", inet_ntoa(ip_addr));
			} else { /* Manual DNS Mode  */
				mib_get(MIB_ADSL_WAN_DNS1, (void *)&ip_addr);
				fprintf(out, "opt dns %s\n", inet_ntoa(ip_addr));
				mib_get(MIB_ADSL_WAN_DNS2, (void *)&ip_addr);
				fprintf(out, "opt dns %s\n", inet_ntoa(ip_addr));
				mib_get(MIB_ADSL_WAN_DNS3, (void *)&ip_addr);
				fprintf(out, "opt dns %s\n", inet_ntoa(ip_addr));
			}

			fclose(out);
		}
		lockf(fh, F_UNLCK, 0);
		close(fh);
	}

	/* Prepare DHCP Server Script File */
	fh = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
 	if ( fh == -1 ) goto fail;

	if ((out = fdopen(fh, "w")) != NULL) {
		if(pid != -1)
			fprintf(out, "kill -9 %d\n", pid);

		if( strcmp(enable, "off") != 0) {		
			fprintf(out, "rm %s\n", _DHCPD_LEASE_FILE_);
			fprintf(out, "touch %s\n", _DHCPD_LEASE_FILE_);
			fprintf(out, "udhcpd %s &\n", _DHCPD_CONFIG_FILE_);
		}
		fclose(out);
	}
	lockf(fh, F_UNLCK, 0);
	close(fh);
	
	return 0;

fail:
	return -1;
}

int genscript_initlan(int argc, char** argv, int argNum)
{
	char *enable;
	char *ifname;
	char *filename;
	int fh;
	FILE *out;
	struct in_addr ip_addr, subnet, gw;
	unsigned char vChar;

	if(argc < (argNum+2)) {
		printf("Usage: genscript initlan on/off interface filename\n");
		return -1;
	}

	enable = argv[argNum++];
	ifname = argv[argNum++];
	filename = argv[argNum++];

	if(mib_init() != 1) goto fail; /* Initialize */

	/* Prepare LAN Init File */
	fh = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if ( fh == -1 ) goto fail;

	if ((out = fdopen(fh, "w")) != NULL) {
		if( !strcmp(enable, "off") ) {
			fprintf(out, "ifconfig %s down\n" ,ifname);
			fprintf(out, "route del -net default gw 0.0.0.0 dev %s\n" ,ifname);			
		} else {
			mib_get(MIB_ADSL_LAN_IP, (void *)&ip_addr);
			mib_get(MIB_ADSL_LAN_SUBNET, (void *)&subnet);
			mib_get(MIB_ADSL_LAN_GATEWAY, (void *)&gw);
			mib_get(MIB_ADSL_LAN_DHCP, (void *)&vChar);

			fprintf(out, "ifconfig %s %s" ,ifname ,inet_ntoa(ip_addr));
			fprintf(out, " netmask %s\n" ,inet_ntoa(subnet));

			fprintf(out, "route del -net default gw 0.0.0.0 dev %s\n" ,ifname);

			if (( strcmp(inet_ntoa(gw), "0.0.0.0") != 0) && (vChar != 2)) {	/* Not DHCP Server Mode */
				fprintf(out, "route add -net default gw %s dev %s\n"
					,inet_ntoa(gw) ,ifname);
			}
		}
		fclose(out);
	}
	lockf(fh, F_UNLCK, 0);
	close(fh);

fail:
	return -1;
}

int file_write(char *filename, char* data)
{
	int fh;
	FILE *out;
	fh = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if ( fh == -1 )  return -1;
	if ((out = fdopen(fh, "w")) != NULL) {
		fprintf(out, data);
		fclose(out);
	}
	lockf(fh, F_UNLCK, 0);
	close(fh);
	return 0;
}

int genscript_fw(int argc, char** argv, int argNum)
{
#define _IP_FORWARD_FILE_			("/proc/sys/net/ipv4/ip_forward")
#define _IP_DYNADDR_FILE_			("/proc/sys/net/ipv4/ip_dynaddr")

	char *enable;
	char *lanIf;
	char *wanIf;
	char *filename;
	int fh;
	FILE *out;
	struct in_addr wan_ip, lan_ip;
	unsigned char wan[16];
	unsigned char lan[16];
	
	if(argc < (argNum+3)) {
		printf("Usage: genscript fw on/off lan wan filename\n");
		return -1;
	}

	enable = argv[argNum++];
	lanIf = argv[argNum++];
	wanIf = argv[argNum++];
	filename = argv[argNum++];

	if(mib_init() != 1) return -1; /* Initialize */

	if (getInAddr( lanIf, IP_ADDR, (void *)&lan_ip) != 1) return -1;
	if (getInAddr( wanIf, IP_ADDR, (void *)&wan_ip) != 1) return -1;

	memset(wan, 0x00, 16);
	strcpy(wan, inet_ntoa(wan_ip));
	memset(lan, 0x00, 16);
	strcpy(lan, inet_ntoa(lan_ip));

	printf("WAN interface %s %s\n",wanIf, wan);
	printf("WLAN interface %s %s\n",lanIf, lan);

	{
		unsigned char vChar;
		unsigned int entryNum;

		mib_get(MIB_PORT_FW_ENABLE, (void *)&vChar);
		entryNum = mib_chain_total(MIB_PORT_FW_TBL);
		printf("PORT FW %d %d\n", vChar, entryNum);

		mib_get(MIB_DMZ_ENABLE, (void *)&vChar);
		printf("DMZ %d\n", vChar, entryNum);

		mib_get(MIB_IP_PORT_FILTER_MODE, (void *)&vChar);
		entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		printf("IP PORT FILTER %d %d\n", vChar, entryNum);

		mib_get(MIB_MAC_FILTER_MODE, (void *)&vChar);
		entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
		printf("MAC FILTER %d %d\n", vChar, entryNum);
	}

	fh = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if ( fh == -1 )  return -1;

	if ((out = fdopen(fh, "w")) != NULL) {
		if( !strcmp(enable, "off") ) {
			fprintf(out, "echo \"Stoping IPTABLES firewall...\" \n");
		} else {
			fprintf(out, "echo \"Starting IPTABLES firewall...\" \n");
		}
		fprintf(out, "\n\n");

		// Attempt to Reset the iptables counters
		fprintf(out, "iptables -Z\n");
		fprintf(out, "iptables -t nat -Z\n");
		fprintf(out, "iptables -t mangle -Z\n");
		fprintf(out, "\n\n");

		// Attempt to Flush All Rules in Filter Table
		fprintf(out, "iptables -F\n");
		fprintf(out, "iptables -X\n");
		fprintf(out, "\n\n");

		// Flush Built-in Rules
		fprintf(out, "iptables -F INPUT\n");
		fprintf(out, "iptables -F OUTPUT\n");
		fprintf(out, "iptables -F FORWARD\n");
		fprintf(out, "\n\n");

		// Flush Built-in Rules
		fprintf(out, "iptables -t nat -F\n");
		fprintf(out, "iptables -t nat -X\n");
		fprintf(out, "iptables -t mangle -F\n");
		fprintf(out, "iptables -t mangle -X\n");
		fprintf(out, "\n\n");

		// Set default policies
		if( !strcmp(enable, "off") ) {
			fprintf(out, "iptables -P INPUT ACCEPT\n");
			fprintf(out, "iptables -P FORWARD ACCEPT\n");
		} else {
			fprintf(out, "iptables -P INPUT DROP\n");
			fprintf(out, "iptables -P FORWARD DROP\n");
		}
		fprintf(out, "iptables -P OUTPUT ACCEPT\n");
		fprintf(out, "iptables -t nat -P POSTROUTING ACCEPT\n");
		fprintf(out, "iptables -t nat -P PREROUTING ACCEPT\n");
		fprintf(out, "iptables -t mangle -P OUTPUT ACCEPT\n");
		fprintf(out, "iptables -t mangle -P PREROUTING ACCEPT\n");
		fprintf(out, "\n\n");

		if( !strcmp(enable, "on") ) {
			unsigned char vChar;
			unsigned char filterAllow = 0;
			struct in_addr ip_addr;

			//  Enable NAT
			fprintf(out, "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE\n", wanIf);
			file_write(_IP_FORWARD_FILE_, "1");
			fprintf(out, "\n\n");

			if(1) {	//// WAN DHCP Client
				fprintf(out, "echo \"Enabling support for a dynamically assigned IP (ISP DHCP)...\" \n");
				fprintf(out, "iptables -A INPUT -i %s -p udp --sport 67 --dport 68 -j ACCEPT\n", wanIf);
				file_write(_IP_DYNADDR_FILE_, "1");
				fprintf(out, "\n\n");
			} else {
				file_write(_IP_DYNADDR_FILE_, "0");
			}

			//  Create chain which blocks new connections, except if coming from inside(LAN)
			fprintf(out, "iptables -N block\n");
			fprintf(out, "iptables -A block -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
			fprintf(out, "iptables -A block -m state --state NEW -i ! %s -j ACCEPT\n", wanIf);
			fprintf(out, "iptables -A block -j DROP\n");
			fprintf(out, "\n\n");

			//  Accept SNMP, PING which is target to the local
			fprintf(out, "iptables -A INPUT -i %s -d %s -p UDP --dport 161:162 -m limit --limit 100/s --limit-burst 500 -j ACCEPT\n", wanIf, wan);
			fprintf(out, "iptables -A INPUT -i %s -d %s -p ICMP --icmp-type echo-request -m limit --limit 1/s -j ACCEPT\n", wanIf, wan);
			fprintf(out, "\n\n");

			mib_get(MIB_MAC_FILTER_MODE, (void *)&vChar);
			if(vChar == 2) { filterAllow = 1;}

			mib_get(MIB_IP_PORT_FILTER_MODE, (void *)&vChar);
			if(vChar == 2) { filterAllow = 1;}
			
			//  IP/PORT filters, filter specified IP, PORT from LAN
			if(vChar != 0) { // 0: Disable, 1:Deny, 2:Allow
				unsigned int entryNum, i;
				MIB_CE_IP_PORT_FILTER_Tp pEntry;
				char *policy, *ip;
				char	portRange[20];
				unsigned char filter_ip[20];

				entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
				
				if(vChar == 1)  {
					if(filterAllow) entryNum = 0;		// If one of the filter is Allow, Deny is disable
					
					policy = "DROP";
				} else if(vChar == 2) {
					policy = "ACCEPT";
				}
				
				if(entryNum > 0) fprintf(out, "echo \"Enabling IP/Port filtering...\" \n");

				for (i=0; i<entryNum; i++) {
					pEntry = (MIB_CE_IP_PORT_FILTER_Tp) mib_chain_get(MIB_IP_PORT_FILTER_TBL, i); /* get the specified chain record */
					if(pEntry == NULL) continue;

					if ( pEntry->fromPort == 0)
						memset(portRange, 0x00, 20);
					else if ( pEntry->fromPort == pEntry->toPort )
						snprintf(portRange, 20, "--sport %d", pEntry->fromPort);
					else
						snprintf(portRange, 20, "--sport %d:%d", pEntry->fromPort, pEntry->toPort);

					if ( !strcmp(inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), "0.0.0.0")) { 
						memset(filter_ip, 0x00, 20);
					} else {
						snprintf(filter_ip, 20, "-s %s",inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
					}

					if(( pEntry->protoType == PROTO_ICMP ) || ( pEntry->protoType == PROTO_TCP)) {
						fprintf(out, "iptables -A FORWARD -i %s -p tcp %s %s -j %s\n",lanIf, filter_ip, portRange, policy);
					}

					if(( pEntry->protoType == PROTO_ICMP ) || ( pEntry->protoType == PROTO_UDP)) {
						fprintf(out, "iptables -A FORWARD -i %s -p udp %s %s -j %s\n",lanIf, filter_ip, portRange, policy);
					}
				}

				if(entryNum > 0) fprintf(out, "\n\n");				
			}

			//  MAC filters, filter specified MAC from LAN
			mib_get(MIB_MAC_FILTER_MODE, (void *)&vChar);
			if(vChar != 0) { // 0: Disable, 1:Deny, 2:Allow
				unsigned int entryNum, i;
				MIB_CE_MAC_FILTER_Tp pEntry;
				char *policy;

				entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
				
				if(vChar == 1)  {
					if(filterAllow) entryNum = 0;		// If one of the filter is Allow, Deny is disable
					
					policy = "DROP";
				} else if(vChar == 2) {
					policy = "ACCEPT";
				}

				if(entryNum > 0) fprintf(out, "echo \"Enabling MAC filtering...\" \n");

				for (i=0; i<entryNum; i++) {
					pEntry = (MIB_CE_MAC_FILTER_Tp) mib_chain_get(MIB_MAC_FILTER_TBL, i); /* get the specified chain record */
					if(pEntry == NULL) continue;

					fprintf(out, "iptables -A FORWARD -i %s -m mac --mac-source %02X:%02X:%02X:%02X:%02X:%02X -j %s\n"
						,lanIf
						,pEntry->macAddr[0] ,pEntry->macAddr[1] ,pEntry->macAddr[2] ,pEntry->macAddr[3] ,pEntry->macAddr[4] ,pEntry->macAddr[5]
						,policy);
				}

				if(entryNum > 0) fprintf(out, "\n\n");								
			}

			if(filterAllow) {
				fprintf(out, "iptables -A FORWARD -i %s -j DROP\n", lanIf);
				fprintf(out, "\n\n");
			}

			//  PORT forwardig, forwarding specified port from WAN to FW_TARGET
			mib_get(MIB_PORT_FW_ENABLE, (void *)&vChar);
			if(vChar == 1) {
				unsigned int entryNum, i;
				MIB_CE_PORT_FW_Tp pEntry;
				char	*ip;
				char	portRange[20];

				entryNum = mib_chain_total(MIB_PORT_FW_TBL);
				if(entryNum > 0) fprintf(out, "echo \"Enabling Port forwarding...\" \n");

				for (i=0; i<entryNum; i++) {
					pEntry = (MIB_CE_PORT_FW_Tp) mib_chain_get(MIB_PORT_FW_TBL, i); /* get the specified chain record */
					if(pEntry == NULL) continue;

					ip = inet_ntoa(*((struct in_addr *)pEntry->ipAddr));
					if ( !strcmp(ip, "0.0.0.0")) continue;

					if ( pEntry->fromPort == 0)
						memset(portRange, 0x00, 20);
					else if ( pEntry->fromPort == pEntry->toPort )
						snprintf(portRange, 20, "--dport %d", pEntry->fromPort);
					else
						snprintf(portRange, 20, "--dport %d:%d", pEntry->fromPort, pEntry->toPort);

					if(( pEntry->protoType == PROTO_ICMP ) || ( pEntry->protoType == PROTO_TCP)) {
						fprintf(out, "iptables -t nat -A PREROUTING -i %s -d %s -p tcp %s -j DNAT --to %s\n", wanIf, wan, portRange, ip);
						fprintf(out, "iptables -t nat -A POSTROUTING -o %s -d %s -p tcp %s -j SNAT --to %s\n", lanIf, ip, portRange, lan);
						fprintf(out, "iptables -A FORWARD -i %s -o ! %s -p tcp %s -j ACCEPT\n", wanIf, wanIf, portRange);
						fprintf(out, "\n");
					}

					if(( pEntry->protoType == PROTO_ICMP ) || ( pEntry->protoType == PROTO_UDP)) {
						fprintf(out, "iptables -t nat -A PREROUTING -i %s -d %s -p udp %s -j DNAT --to %s\n", wanIf, wan, portRange, ip);
						fprintf(out, "iptables -t nat -A POSTROUTING -o %s -d %s -p udp %s -j SNAT --to %s\n", lanIf, ip, portRange, lan);
						fprintf(out, "iptables -A FORWARD -i %s -o ! %s -p udp %s -j ACCEPT\n", wanIf, wanIf, portRange);
						fprintf(out, "\n");
					}
				}

				if(entryNum > 0) fprintf(out, "\n\n");
			}

			//  DMZ, config Demilitarized Zone Area
			mib_get(MIB_DMZ_ENABLE, (void *)&vChar);
			if(vChar == 1) {
				unsigned char dmz[16];

				memset(dmz, 0x00, 16);
				mib_get(MIB_DMZ_IP, (void *)&ip_addr);
				strcpy(dmz, inet_ntoa(ip_addr));
				
				fprintf(out, "echo \"Enabling DMZ... DMZ HOST is %s\" \n", dmz);
				fprintf(out, "iptables -t nat -A PREROUTING -i %s -d %s -p UDP --dport 161:162 -j ACCEPT\n", wanIf, wan); // DO NOT forward snmp packet to DMZ
				fprintf(out, "\n");

				fprintf(out, "iptables -t nat -A PREROUTING -i %s -d %s -j DNAT --to %s\n", wanIf, wan, dmz);
				fprintf(out, "iptables -t nat -A POSTROUTING -d %s -j SNAT --to %s\n", dmz, lan);
				fprintf(out, "iptables -A FORWARD -i %s -o ! %s -d %s -j ACCEPT\n", wanIf, wanIf, dmz);
				fprintf(out, "\n\n");
			}

			//  link the block chain
			fprintf(out, "iptables -A INPUT -j block\n");
			fprintf(out, "iptables -A FORWARD -j block\n");
			fprintf(out, "\n\n");

			fprintf(out, "echo \"IPTABLES firewall ENABLE!\" \n");
		} else {
			file_write(_IP_FORWARD_FILE_, "0");
			file_write(_IP_DYNADDR_FILE_, "0");
			fprintf(out, "echo \"IPTABLES firewall DISABLE!\" \n");
		}
		
		fclose(out);
	}
	lockf(fh, F_UNLCK, 0);
	close(fh);



}

static void showHelp(void)
{
	printf("Usage: genscript cmd\n");
	printf("cmd:\n");
	printf("  initlan \ton/off \tinterface \tfilename\t Lan Init Script\n");
	printf("  dhcpc \ton/off \tinterface \tfilename\t Dhcp Client Script\n");
	printf("  dhcpd \ton/off \tinterface \tfilename\t Dhcp Server Script\n");
}

int genscript_main(int argc, char** argv)
{
	int argNum=1;

	if ( argc > 1 ) 
	{
		if ( !strcmp(argv[argNum], "dhcpc") ) {
			return (genscript_dhcpc(argc, argv, ++argNum));
		}
		else if ( !strcmp(argv[argNum], "dhcpd") ) {
			return (genscript_dhcpd(argc, argv, ++argNum));
		}
		else if ( !strcmp(argv[argNum], "initlan") ) {
			return (genscript_initlan(argc, argv, ++argNum));
		}
		else if ( !strcmp(argv[argNum], "fw") ) {
			return (genscript_fw(argc, argv, ++argNum));
		}
	}

	showHelp();
	return 0;
}
#else
int genscript_main(int argc, char** argv)
{
	return 0;
}
#endif

