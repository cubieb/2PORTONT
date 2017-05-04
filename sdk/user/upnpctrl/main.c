#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netdb.h>
#include <string.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#include <config/autoconf.h>
#include <signal.h>
#include <arpa/inet.h>

#ifdef CONFIG_RTK_RG_INIT
const char RG_UPNP_CONNECTION_FILE[] = "/var/rg_upnp_idx";
const char RG_UPNP_TMP_FILE[] = "/var/rg_upnp_tmp";
#endif

char g_IpFlag[5];
char g_extInterfaceName[IFNAMSIZ];
char g_intInterfaceName[10];
#define MINIUPNPDPID  "/var/run/linuxigd.pid"

#ifdef CONFIG_USER_UPNPD
// 2008-12-12
// Magician: UPnP Daemon is obsoleted, use MiniUPnP Daemon instead.
#endif

#ifdef CONFIG_USER_MINIUPNPD
int main(int argc, char **argv)
{
	FILE *fp, *fp2;
	//char ext_ifname[6];
	char ext_ifname[IFNAMSIZ];
	unsigned char upnpdEnable;
	unsigned int upnpItf;
	unsigned char value[6];
	unsigned char ipaddr[16];
	struct in_addr inAddr;
	char old_extIfName[10];
	int int_pid = 0, wscpid = 0, run_mini_upnpd = 0, i = 0;
	char *cmd_argv[9];

	// Magician: Sync WAN channel on the fly.
	if(argc == 2)
	{
		if(!strcmp(argv[1], "sync"))
		{
			if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable) != 0)
			{
				if( upnpdEnable != 1 ) // MINIUPNP Daemon is not enabled.
				{
					printf("upnpctrl(sync): miniupnpd is not enabled.\n");
					exit(0);
				}
			}
			else
			{
				printf("upnpctrl(sync): Get MIB_UPNP_DAEMON failed!\n");
				exit(0);
			}

			if(!(fp = fopen("/var/run/linuxigd.pid", "r")))
				return 0;

			fscanf(fp, "%d", &int_pid);  // Get miniupnpd pid.
			fclose(fp);

			if (int_pid <= 0)
			{
				printf("upnpctrl(sync): Get miniupnpd pid failed!\n");
				return 0;
			}

			if(!mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
			{
				printf("upnpctrl(sync): Get MIB_UPNP_EXT_ITF failed!\n");
				return 0;
			}

			ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

			fp = fopen("/var/upnp_extif", "r");
			fscanf(fp, "%s", old_extIfName);
			fclose(fp);

			fp = fopen("/var/upnp_extif", "w");
			fprintf(fp, "%s", ext_ifname);
			fclose(fp);

			// iptables -t nat -D PREROUTING -i vc0 -j MINIUPNPD
			va_cmd("/bin/iptables", 8, 1, "-t", "nat", "-D", "PREROUTING", "-i", old_extIfName, "-j", "MINIUPNPD");

			// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
			va_cmd("/bin/iptables", 11, 1, "-t", "filter", "-D", "FORWARD", "-i", old_extIfName, "!", "-o", old_extIfName, "-j", "MINIUPNPD");

			// iptables -t nat -A PREROUTING -i vc0 -j MINIUPNPD
			va_cmd("/bin/iptables", 8, 1, "-t", "nat", "-A", "PREROUTING", "-i", ext_ifname, "-j", "MINIUPNPD");

			// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
			va_cmd("/bin/iptables", 12, 1, "-t", "filter", "-I", "FORWARD", "1", "-i", ext_ifname, "!", "-o", ext_ifname, "-j", "MINIUPNPD");

			kill(int_pid, SIGUSR1);

			printf("upnpctrl(sync): MiniUPnP is sync. Sync from %s to %s\n", old_extIfName, ext_ifname);
		}
		else
		{
			printf("Usage: upnpctrl <up|down|sync> [<external ifname> <internal ifname>]\n");
			printf("Example: upnpctrl up ppp0 br0\n");
			printf("Example: upnpctrl down ppp0 br0\n");
			printf("Example: upnpctrl sync\n");
			exit(0);
		}

		return 1;
	}

	if(argc != 4)
	{
		printf("Usage: upnpctrl <up|down|sync> [<external ifname> <internal ifname>]\n");
		printf("Example: upnpctrl up ppp0 br0\n");
		printf("Example: upnpctrl down ppp0 br0\n");
		printf("Example: upnpctrl sync\n");
		exit(0);
	}

	// Save the IpFlag and interface names for later uses
	strcpy(g_IpFlag, argv[1]);
	strcpy(g_extInterfaceName, argv[2]);
	strcpy(g_intInterfaceName, argv[3]);
	//printf("g_IpFlag=%s g_extInterfaceName=%s g_intInterfaceName=%s\n", g_IpFlag, g_extInterfaceName, g_intInterfaceName);

	if(mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr[15] = '\0';
	}

	if(strcmp(g_IpFlag, "up") == 0)
	{
		if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable) != 0)
		{
			if( upnpdEnable != 1 ) // MINIUPNP Daemon is not enabled.
				exit(0);
		}
		else
		{
			printf("upnpctrl: Get MIB_UPNP_DAEMON failed!\n");
		}

		// Mason Yu
		if((fp=fopen("/var/run/linuxigd.pid", "r")) == NULL)
		{
			if(!mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
			{
				printf("upnpctrl(up): Get MIB_UPNP_EXT_ITF failed!\n");
				return 0;
			}

			if( upnpItf != DUMMY_IFINDEX )
			{
				ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

				if(strcmp(ext_ifname, g_extInterfaceName) == 0)
				{
					printf("upnpctrl(up): Start MINIUPNPD on %s interface\n", g_extInterfaceName);

					// We should add multicast route and INPUT relue for Netfilter.
					// route add -net 239.0.0.0 netmask 255.0.0.0 br0
					va_cmd("/bin/route", 6, 1, "add", "-net", "239.0.0.0", "netmask", "255.0.0.0", "br0");
					// iptables -I INPUT 1 -p TCP --dport 49152 -j ACCEPT
					va_cmd("/bin/iptables", 9, 1, "-I", "INPUT", "1", "-p", "TCP", "--dport", "49152", "-j", "ACCEPT");

					// iptables -t nat -N MINIUPNPD
					va_cmd("/bin/iptables", 4, 1, "-t", "nat", "-N", "MINIUPNPD");

					// iptables -t nat -A PREROUTING -i vc0 -j MINIUPNPD
					va_cmd("/bin/iptables", 8, 1, "-t", "nat", "-A", "PREROUTING", "-i", g_extInterfaceName, "-j", "MINIUPNPD");

					// iptables -t filter -N MINIUPNPD
					va_cmd("/bin/iptables", 4, 1, "-t", "filter", "-N", "MINIUPNPD");

					// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
					va_cmd("/bin/iptables", 12, 1, "-t", "filter", "-I", "FORWARD", "1", "-i", g_extInterfaceName, "!", "-o", g_extInterfaceName, "-j", "MINIUPNPD");

					// Start miniupnpd
					// miniupnpd -i vc0 -a 192.168.1.1 -p 49152
					va_cmd("/bin/miniupnpd", 6, 0, "-i", g_extInterfaceName, "-a", ipaddr, "-p", "49152");

					fp2=fopen("/var/upnp_extif", "w");
					fprintf(fp2, "%s", g_extInterfaceName);
					fclose(fp2);

					while(read_pid((char*)MINIUPNPDPID) < 0)
					{
						//printf("MINIUPNPD(IGD) is not running. Please wait!\n");
						usleep(300000);
					}
					startSSDP();
					// The end of miniupnpd starting.
				}
				else
					printf("upnpctrl(up): The %s interface is not binded for MINIUPNPD.\n", g_extInterfaceName);
			}
			else
			{
				printf("upnpctrl(up): MINIUPNP binded WAN interface not set(error)!\n");
				return;
			}
		}
		else
		{
			fclose(fp);
			printf("upnpctrl(up): MINIUPNP is start already!\n");
		}
	}
	else if(strcmp(g_IpFlag, "down") == 0)
	{
		if((fp=fopen("/var/run/linuxigd.pid", "r")) != NULL)
		{
			if(!mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
			{
				printf("upnpctrl(down): Get MIB_UPNP_EXT_ITF failed!\n");
				fclose(fp);
				return 0;
			}

			if( upnpItf != DUMMY_IFINDEX )
			{
				ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

				if( strcmp(ext_ifname, g_extInterfaceName) == 0 )
				{
					fscanf(fp, "%d", &int_pid);  // Get miniupnpd pid.

					if (int_pid > 0)
					{
						// Magician: we should delete all rules of miniupnp for instant effect.
						// route del -net 239.0.0.0 netmask 255.0.0.0 br0
						va_cmd("/bin/route", 6, 1, "del", "-net", "239.0.0.0", "netmask", "255.0.0.0", "br0");
						// iptables -D INPUT -p TCP --dport 49152 -j ACCEPT
						va_cmd("/bin/iptables", 8, 1, "-D", "INPUT", "-p", "TCP", "--dport", "49152", "-j", "ACCEPT");

						// iptables -t nat -D PREROUTING -i vc0 -j MINIUPNPD
						va_cmd("/bin/iptables", 8, 1, "-t", "nat", "-D", "PREROUTING", "-i", g_extInterfaceName, "-j", "MINIUPNPD");
						// iptables -t nat -N MINIUPNPD
						va_cmd("/bin/iptables", 4, 1, "-t", "nat", "-X", "MINIUPNPD");

						// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
						va_cmd("/bin/iptables", 11, 1, "-t", "filter", "-D", "FORWARD", "-i", g_extInterfaceName, "!", "-o", g_extInterfaceName, "-j", "MINIUPNPD");

						// iptables -t filter -N MINIUPNPD
						va_cmd("/bin/iptables", 4, 1, "-t", "filter", "-X", "MINIUPNPD");

#ifdef CONFIG_RTK_RG_INIT
						FILE *rgfp = NULL, *rgfp_tmp = NULL;
						int upnp_idx = 0, upnp_eport = 0, upnp_proto = 0;
						char line[24] = {0};

						if ((rgfp = fopen(RG_UPNP_CONNECTION_FILE, "r")) != NULL) {
							if ((rgfp_tmp = fopen(RG_UPNP_TMP_FILE, "w")) != NULL) {
								while (fgets(line, 23, rgfp) != NULL)
								{
									sscanf(line, "%d %d %d\n", &upnp_idx, &upnp_eport, &upnp_proto);

									if (rtk_rg_upnpConnection_del(upnp_idx)) {
										printf("upnpctrl(down): rtk_rg_upnpConnection_del failed! upnp_idx = %d", upnp_idx);
									}
								}
								fclose(rgfp_tmp);
							}
							fclose(rgfp);
							unlink(RG_UPNP_CONNECTION_FILE);
							rename(RG_UPNP_TMP_FILE, RG_UPNP_CONNECTION_FILE);
						}
#endif

						kill(int_pid, 9);
						unlink("/var/run/linuxigd.pid");
						printf("upnpctrl(down): MINIUPNPD is down\n");
					}
					else
						printf("upnpctrl(down): MINIUPNPD shutdown failed!");
					// modification for instant effect end.

					startSSDP();
				}
				else
					printf("upnpctrl(down): The %s interface is not binded for MINIUPNPD.\n", g_extInterfaceName);
			}
			else
				printf("upnpctrl(down): MINIUPNPD binded WAN interface is not set(error)!\n");
			fclose(fp);
		}
		else
			printf("upnpctrl(down): MINIUPNPD did not start!\n");
	}
	else
		printf("Invalid argument: \"%s\"\n", g_IpFlag);

	return 1;
}
#endif  // CONFIG_USER_MINIUPNPD

