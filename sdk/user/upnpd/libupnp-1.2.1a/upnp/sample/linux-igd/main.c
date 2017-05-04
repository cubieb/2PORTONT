#include "inc/upnp.h"
#include <stdio.h>
#include <stdlib.h>
//  #include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <time.h>
#include "config.h"
#include "gatedevice.h"
#include "util.h"
#include "pmlist.h"
#include "globals.h"

#include <sys/types.h>
#include <unistd.h>

char extIpAddress[16];     // Server internal ip address
int initUpnp;

int main (int argc, char** argv)
{
	int ret = UPNP_E_SUCCESS;
	int signum;
	char descDocUrl[50];
	char descDocName[20];
	char xmlPath[50];
	char intIpAddress[16];     // Server internal ip address
	sigset_t sigsToCatch;
	FILE *f, *f2;

	pid_t pid,sid;
	struct rlimit resourceLimit = { 0 };
	int i;
	int status;
	
	if (argc != 3)
   {
      printf("Usage: upnpd <external ifname> <internal ifname>\n");
      printf("Example: upnpd ppp0 eth0\n");
      printf("Example: upnpd eth1 eth0\n");
      exit(0);
   }
	
	// Mason Yu
   	initUpnp = 0;
   	
   	f=fopen("/var/run/linuxigd.pid","w");
   	fprintf(f,"%d",getpid());
   	fclose(f);

	parseConfigFile(&g_forwardRules,&g_debug,g_iptables,
		        g_forwardChainName,g_preroutingChainName,
			g_upstreamBitrate,g_downstreamBitrate,
			descDocName,xmlPath);
	// Save the interface names for later uses
	strcpy(g_extInterfaceName, argv[1]);
	strcpy(g_intInterfaceName, argv[2]);

	// Get the internal ip address to start the daemon on
	GetIpAddressStr(intIpAddress, g_intInterfaceName);
	GetIpAddressStr(extIpAddress, g_extInterfaceName);

	// Put igd in the background as a daemon process.
#if 0	
	pid = vfork();
	if (pid < 0)
	{
		perror("Error forking a new process.");
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
		exit(EXIT_SUCCESS);

	// become session leader
	if ((sid = setsid()) < 0)
	{
		perror("Error running setsid");
		exit(EXIT_FAILURE);
	}
#endif	

#if 0
if(rtl8651_queryUpnpMapTimeAge(1,0,0,0xc0a89a77,9000)==0)
  rtl8651_addNaptUpnpPortMapping(2,0xc0a89a77, 9000, 0xc0a80171, 8000,0,120);
rtl8651_queryUpnpMapTimeAge(1,0,0,0xc0a89a77,9000);
#endif
	
	// close all file handles
	resourceLimit.rlim_max = 0;
	status = getrlimit(RLIMIT_NOFILE, &resourceLimit);
	if (-1 == status) /* shouldn't happen */
	{
	    perror("error in getrlimit()");	    
	    exit(1);
	}
	if (0 == resourceLimit.rlim_max)
	{	   
	    fprintf(stderr, "Max number of open file descriptors is 0!!\n");
	    exit(1);
	}	
	for (i = 0; i < resourceLimit.rlim_max; i++)
	    close(i);

	
	
#if 0	
	// fork again so child can never acquire a controlling terminal
	pid = vfork();
	if (pid < 0)
	{
		perror("Error forking a new process.");
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
		exit(EXIT_SUCCESS);
	
	if ((chdir("/")) < 0)
	{
		perror("Error setting root directory");
		exit(EXIT_FAILURE);
	}
#endif
	
	umask(0);

// End Daemon initialization	
	
	// Initialize UPnP SDK on the internal Interface
	if (g_debug) printf( "Initializing UPnP SDK ... ");
	if ( (ret = UpnpInit(intIpAddress,0) ) != UPNP_E_SUCCESS)
	{
		printf( "Error Initializing UPnP SDK on IP %s ",intIpAddress);
		printf( "  UpnpInit returned %d", ret);
		UpnpFinish();
		exit(1);
	}
	if (g_debug) printf( "UPnP SDK Successfully Initialized.");

	// Set the Device Web Server Base Directory
	if (g_debug) printf( "Setting the Web Server Root Directory to %s",xmlPath);
	if ( (ret = UpnpSetWebServerRootDir(xmlPath)) != UPNP_E_SUCCESS )
	{
		printf( "Error Setting Web Server Root Directory to: %s", xmlPath);
		printf( "  UpnpSetWebServerRootDir returned %d", ret); 
		UpnpFinish();
		exit(1);
	}
	if (g_debug) printf( "Succesfully set the Web Server Root Directory.");


	// Form the Description Doc URL to pass to RegisterRootDevice
	sprintf(descDocUrl, "http://%s:%d/%s", UpnpGetServerIpAddress(),
				UpnpGetServerPort(), descDocName);

	// Register our IGD as a valid UPnP Root device
	if (g_debug) printf( "Registering the root device with descDocUrl %s", descDocUrl);
	if ( (ret = UpnpRegisterRootDevice(descDocUrl, EventHandler, &deviceHandle,
													&deviceHandle)) != UPNP_E_SUCCESS )
	{
		printf( "Error registering the root device with descDocUrl: %s", descDocUrl);
		printf( "  UpnpRegisterRootDevice returned %d", ret);
		UpnpFinish();
		exit(1);
	}

	printf( "IGD root device successfully registered.");
	
	// Initialize the state variable table.
	StateTableInit(descDocUrl);
	
	// Mason Yu
	sleep(5);
	
	// Record the startup time, for uptime
	startup_time = time(NULL);
	
	// Send out initial advertisements of our device's services with timeouts of 30 minutes
	if ( (ret = UpnpSendAdvertisement(deviceHandle, 1800) != UPNP_E_SUCCESS ))
	{
		printf( "Error Sending Advertisements.  Exiting ...");
		UpnpFinish();
		exit(1);
	}
	printf( "Advertisements Sent.  Listening for requests ... ");	
	
	// Loop until program exit signals recieved
	sigemptyset(&sigsToCatch);
	sigaddset(&sigsToCatch, SIGINT);
	sigaddset(&sigsToCatch, SIGTERM);
	//sigwait(&sigsToCatch, &signum);
	pthread_sigmask(SIG_SETMASK, &sigsToCatch, NULL);
	
	// Mason Yu
	initUpnp = 1;	
	
	sigwait(&sigsToCatch, &signum);
	printf( "Shutting down on signal %d...\n", signum);

	// Cleanup UPnP SDK and free memory
	pmlist_FreeList(); 

	UpnpUnRegisterRootDevice(deviceHandle);
	UpnpFinish();	
	
	// Exit normally
	return (1);
}
