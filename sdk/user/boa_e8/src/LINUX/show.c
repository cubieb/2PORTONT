#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <ctype.h>

#include "mib.h"
#include "sysconfig.h"
#include "mibtbl.h"

void usage(void);
static void string_upper(char *s)
{	
	while (*s != '\0') {		
		if (islower(*s))			
			*s = toupper(*s);		
		s++;	
	}
}

int getOUIfromMAC(char* oui)
{
        if (oui==NULL)
                return -1;
        unsigned char macAddr[MAC_ADDR_LEN];
        mib_get( MIB_ELAN_MAC_ADDR, (void *)macAddr);
        sprintf(oui,"%02X%02X%02X",macAddr[0],macAddr[1],macAddr[2]);
        return 0;
}
#if 0
/*
 * Get MIB Value
 */
int mib_get_by_name(char *name, void *value)	// get default value
{
	int i, j, id = -1;
	unsigned char buffer[1024], tmp[16];

	for (i = 0; mib_table[i].id; i++) {
		if (!strcmp(mib_table[i].name, name)) {
			id = mib_table[i].id;
			break;
		}
	}

	if (id != -1) {
		memset(buffer, 0x00, sizeof(buffer));

		/* fails */
		if (_mib_get(id, buffer) == 0)
			return 0;

		switch (mib_table[i].type) {
		case IA_T:
			strcpy(value, inet_ntoa(*(struct in_addr *)buffer));
			return 1;

#ifdef CONFIG_IPV6
		case IA6_T:
			inet_ntop(PF_INET6, buffer, value, 48);
			return 1;
#endif

		case BYTE_T:
			sprintf(value, "%hhu", *(unsigned char *)buffer);
			return 1;

		case WORD_T:
			sprintf(value, "%hu", *(unsigned short *)buffer);
			return 1;

		case DWORD_T:
			sprintf(value, "%u", *(unsigned int *)buffer);
			return 1;

		case INTEGER_T:
			sprintf(value, "%d", *(int *)buffer);
			return 1;

		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			((unsigned char *)value)[0] = '\0';
			for (j = 0; j < mib_table[i].size; j++) {
				sprintf(tmp, "%02x", buffer[j]);
				strcat(value, tmp);
			}
			return 1;

		case BYTE_ARRAY_T:
			((unsigned char *)value)[0] = '\0';
			for (j = 0; j < mib_table[i].size; j++) {
#if 0//def USE_11N_UDP_SERVER
				if (udp_server_cmd) {
					sprintf(tmp, "%02x", buffer[j]);
				} else {
#endif
					sprintf(tmp, "%02x", buffer[j]);
					if (j != (mib_table[i].size - 1))
						strcat(tmp, ",");
#if 0//def USE_11N_UDP_SERVER
				}
#endif
				strcat(value, tmp);
			}
			return 1;

		case STRING_T:
			strcpy(value, buffer);
			return 1;

		default:
			return 0;
		}
	}

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	int argNum=1;

	if (argc<=1)
		goto arg_err_rtn;
	if ( !strcmp(argv[argNum], "system") ) {
		unsigned char buffer[512];
		
		if (argc != 3) {
			usage();
			return 0;
		}
		argNum++;
	
		if ( !strcmp(argv[argNum], "version")) {
			char * oui[20] = {0};
			struct sysinfo info;
			FILE * fp;
			char buf[256]={0};
			mib_get( MIB_DEVICE_NAME, (void *)buffer);
			
			printf("Model Number: %s\n",buffer);
			
			fp = fopen("/proc/version", "r");
			if(fp)
			{
				char month[5];
				char day[5];
				char time[20];
				char year[5];
				char * pBuf = NULL;
				memset(buf, 0, sizeof(buf));
				fgets(buf, sizeof(buf), fp);
	
				pBuf = buf;
				while(*pBuf)
				{
					if(*pBuf == '#')
					{
						break;
					}
					pBuf++;
				}
				sscanf(pBuf, "%*s %*s %s %s %s %*s %s",month,day,time,year);
				printf("Build Date: %s %s %s %s\n", month, day, year, time);
				fclose(fp);
			}
	
		#define _FIRMVERSION(s) #s
		#define FIRMVERSION(s) _FIRMVERSION(s)
			printf("Firmware Version: %s\n", FIRMVERSION(FIRMVER));
			
			//mib_get_by_name("ELAN_MAC_ADDR", buffer);
			mib_get( MIB_ELAN_MAC_ADDR, (void *)buffer);
			//string_upper(buffer);
			printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	
			if (sysinfo(&info) == 0) 
			{
				int day,hour,minute,second;
				day = info.uptime / 86400;
				hour = (info.uptime % 86400) / 3600;
				minute = (info.uptime % 3600) / 60;
				second = info.uptime % 60;
				printf("SysUpTime: %d %d:%d:%d\n", day, hour, minute, second);
			}
	
			//mib_get_by_name("HW_SERIAL_NO", buffer);
	        mib_get(MIB_HW_SERIAL_NUMBER, (void *)buffer);
			printf("HW Serial Number: %s\n", buffer);
			
			getOUIfromMAC(oui);
			printf("ManufacturerOUI: %s\n",oui);
	
			printf("Manufacturer: Realtek Semiconductor Corp.\n");
			return 0;
		}
		else if ( !strcmp(argv[argNum], "isdefault")) {
			unsigned char vChar;
			
			mib_get(MIB_MP_FIN, (void *)&vChar);
			if (vChar)
				printf("system is default setting\n");
			else
				printf("system is not default setting\n");
			return 0;
		}
	}

	return 0;
	arg_err_rtn:
	usage();
	exit(1);

}

void usage(void)
{
	printf("show system version\n");
	printf("show system isdefault\n");
}



