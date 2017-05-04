#include <jni.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h> /* for strncpy */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/vfs.h>

#include "Gateway_jni.h"


// Get WAN IP Address and Netmask
JNIEXPORT jstring JNICALL Java_Gateway_1jni_getWanIp
  (JNIEnv *env, jclass class)
{
	int fd;
        struct ifreq ifr;
	char WanIP[256] = {0x0};
	jstring jWanIP = (*env)->NewStringUTF(env, WanIP);
	int buf_len = 0;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Get an IPv4 IP address */
        ifr.ifr_addr.sa_family = AF_INET;
	/* IP address attached to "eth0" */
        strncpy(ifr.ifr_name, "nas0_0", IFNAMSIZ-1);
	// Get WAN IP by IOCTL
        ioctl(fd, SIOCGIFADDR, &ifr);
	if(strcmp("0.0.0.0" ,  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr))!= 0 && 
		strcmp("0.82.20.24", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)) != 0)
	{
		sprintf(&WanIP[buf_len], "%s|" , inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		buf_len = strlen(WanIP);
		// Get WAN IP Netmask by IOCTL
       	 	ioctl(fd, SIOCGIFNETMASK, &ifr);
		sprintf(&WanIP[buf_len] , "%s" , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
	}

        close(fd);
	jWanIP = (*env)->NewStringUTF(env, WanIP);
	return jWanIP;
}

// Get Loid and Password
JNIEXPORT jstring JNICALL Java_Gateway_1jni_getLoid
  (JNIEnv *env, jclass class)
{
        char Loid[256];
        jstring jLoid = (*env)->NewStringUTF(env, Loid);
	printf("Get Loid \n");
	return jLoid;
}

// Get USB device file system information

struct mount_cont
{
	char dev_path[256];
	char mount_path[256];
	char fs_name[32];
};

static void parse_mount(struct mount_cont mc[], int *total_cnt)
{
	FILE *f = fopen("/proc/mounts" , "r");
	if(f)
	{
		while(fscanf(f, "%s %s %s %*s %*d %*d\n",  
			mc[*total_cnt].dev_path, 
			mc[*total_cnt].mount_path, 
			mc[*total_cnt].fs_name)!= EOF)
		{
			(*total_cnt) ++;
		}
		fclose(f);
	}

}

JNIEXPORT jstring JNICALL Java_Gateway_1jni_getUSB
  (JNIEnv *env , jclass class)
{
        char usb_dev_str[1024] = {0x0};
	int usb_len = 0;
	int i ;
	int total = 0;
	struct mount_cont mc[64];
	parse_mount(mc, &total);
	for(i = 0 ; i < total; i ++)
	{
		if(strncmp("/var/mnt/" , mc[i].mount_path , 9) == 0)
		{
			printf("dev:%s,mount:%s,fs:%s\n", mc[i].dev_path, mc[i].mount_path, mc[i].fs_name);
			// get file ssytem status
			struct statfs fs;
			if(statfs(mc[i].mount_path , &fs) == 0)
			{

				sprintf(&usb_dev_str[usb_len], "%s: %s, %lu, %lu ",
						mc[i].dev_path,mc[i].fs_name, 
						fs.f_bsize*fs.f_blocks, 
						(fs.f_bsize*fs.f_blocks - fs.f_bsize*fs.f_bfree));
				usb_len = strlen(usb_dev_str);		
			}
			else printf("can't get %s info\n" , mc[i].mount_path);
		}
	}
	jstring jUSB = (*env)->NewStringUTF(env, usb_dev_str);
	return jUSB;

}
