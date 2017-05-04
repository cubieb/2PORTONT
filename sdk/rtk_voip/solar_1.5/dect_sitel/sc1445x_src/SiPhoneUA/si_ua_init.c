/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 si_ua_init.c
 * Purpose:		 		 
 * Created:		 		 Oct 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <si_print_api.h>
#include "si_ua_registrar.h"
#include "si_ua_events.h"
#include "si_ua_hold.h"
#include "si_ua_dtmf.h"
#include "../common/si_phone_api.h"
#include "../common/si_configfiles_api.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "si_ua_init.h"
#include "si_ua_api.h"
#include "si_sdp_negotiation.h"
#include "si_ua_callcontrol.h"
#include "si_ua_dns.h"
#include "../CCFSM/ccfsm_api.h"
#include "../exosip2/src/eX_setup.h"

//#include "si_ua_filesAPI.h"

/*========================== Global Variables definitions ========================*/
  
SICORE siphoneCore;
char SipAllowHeader[128];

sicall_t SiCall[MAX_SUPPORTED_CALLS];
 
extern ConfigSettings m_Config_Settings;
void InitiateAllCalls(void);

//============================================================================
// Initiate the User Agent
// port: SIP port Default : 5060
//
//============================================================================
void convertIP2DotStr(char *pDest, unsigned char *pSrc);
void convertIP2DotStr( char *pDest, unsigned char *pSrc)
{
	sprintf(pDest, "%d.%d.%d.%d",pSrc[0],pSrc[1],pSrc[2],pSrc[3]);
}

#include "si_ua_stun.h"
char StunResponse[SIPHONE_IPADDR_SIZE];
int StunOutPort=0;


void si_ua_reset(void) 
{
	int accountID ;
	
	si_print(PRINT_LEVEL_INFO, "SI UA RESET \n");
	for (accountID=0;accountID<MAX_SUPPORTED_ACCOUNTS;accountID++)
	{
		if (siphoneCore.siregistration[accountID].state == CCFSM_REG_SUCCESS)
		{
			if (si_ua_unregister(&siphoneCore, &siphoneCore.siregistration[accountID]))
			{
 				siphoneCore.siregistration[accountID].state = CCFSM_REG_INIT;
			}
 		}else siphoneCore.siregistration[accountID].state = CCFSM_REG_INIT;
 	}
 	checkToReRegister(&siphoneCore);
}

int si_net_startup(unsigned char *IPAddress) 
{
  static int cnt=0;
 	int ret ;
 
 _start:
	//  Get IP Address
#ifdef SIP_IPv6
		si_ua_getAddress6(IPAddress);//siphoneCore.IPAddress);
#else 
		si_ua_getAddress4(IPAddress);//siphoneCore.IPAddress);
#endif
    ret = strcmp((char*)IPAddress,(char*)"0.0.0.0");
  
	if (!ret) {
		sleep(1);
    cnt++;
    if (cnt==20){
    
      si_ua_SetNetworkSettingsInfo((unsigned char*)"0.0.0.0");
      cnt=0;
      si_print(PRINT_LEVEL_INFO, "DHCP NOT FOUND. Please assign a static IP address or retry \n\n");
      return -1;
    }
		goto _start;
	}
//
 	si_ua_SetNetworkSettingsInfo(IPAddress);
  return 0;
}

int si_net_init(unsigned char *IPAddress) 
{
  char *args[] = {"/etc/udhcpc.scr", "renew" ,(char *) 0 };
  pid_t pid;
   int ret=0;

  //read net files to see if DHCP is enabled
	ret = sc_fileapi_LoadNetworkSettings(&m_Config_Settings.m_NetworkSettings); 
  if (ret<0){
    si_print(PRINT_LEVEL_ERR, "\n\n========== Cannot load network settings file \n\n");
    /* Set dhcp as default */
    m_Config_Settings.m_NetworkSettings.DHCP=1;
  }

  if(m_Config_Settings.m_NetworkSettings.DHCP==0){
		si_print(PRINT_LEVEL_INFO, "\n\n =========== Using static IP\n");
  }else si_print(PRINT_LEVEL_INFO, "Acquiring DHCP..\n");

	ret=si_net_startup(IPAddress);

  if (ret==-1){

		si_print(PRINT_LEVEL_ERR, "DHCP Failed\n");
    m_Config_Settings.m_NetworkSettings.DHCP = 0;
    si_system_configuration_api(&m_Config_Settings.m_NetworkSettings);
    si_print(PRINT_LEVEL_INFO, "Parent process %d running...\n", getpid());

    /* Call vfork to create a child process since the parent process */
    /* will be replaced by execv */
    if ((pid = vfork())== 0) {
      si_print(PRINT_LEVEL_INFO, "Parent process stopped...\n");
      si_print(PRINT_LEVEL_INFO, "Child process %d is running...\n", getpid());
      ret=execv("/etc/udhcpc.scr", args);
      if (ret==-1){
        si_print(PRINT_LEVEL_ERR, "\n\n udhcpc.scr did not run \n\n");
        si_print(PRINT_LEVEL_ERR, "execv failed \n");
        si_print(PRINT_LEVEL_ERR, "Error code: %s\n", strerror(errno));
      }
   }else if (pid > 0) { // Here the parent process is been replaced after the end of ev
     si_print(PRINT_LEVEL_INFO, "Udhcpc.scr script updated succesfully with static IP settings\n");
     si_print(PRINT_LEVEL_INFO, "Child process completed successfully\n");
     si_print(PRINT_LEVEL_INFO, "Parent process %d re-running...\n", getpid());
   }else {
     si_print(PRINT_LEVEL_ERR, "vfork failed\n"); // error handling
     si_print(PRINT_LEVEL_ERR, "Unable to set a static IP\n"); // error handling   
     //writeLCD("
    }

   ret=si_net_startup(IPAddress);
   si_print(PRINT_LEVEL_INFO, "Using static IP\n");
   //writeLCD("Using static IP",1,1);

 
   m_Config_Settings.m_NetworkSettings.DHCP = 1;
   si_system_configuration_api(&m_Config_Settings.m_NetworkSettings);
  }

  return 0;
}

int si_ua_start_ntpclient(void)
{
 	system("ntpclient -s -h ntp.mfa.gr &");
	return 0;
}
int initSIPparameters(SICORE* pCore)
{
	sprintf(SipAllowHeader, "%s", SIPHONE_UA_ALLOW_HEADER);
 	if (pCore->Prack) 
 		strcat(SipAllowHeader, ",PRACK");

 	if (pCore->Update) 
		strcat(SipAllowHeader, ",UPDATE");

	return 0;
 }

int si_ua_init(SICORE* pCore)
{
	int ret;
  int inet=AF_INET;
	int i;
	 TRACE_INITIALIZE (10, stdout);
	if (pCore->sipStatus)  eXosip_quit();
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	  	siphoneCore.siregistration[i].state = CCFSM_REG_INIT;

   	InitiateAllCalls();
  	siphoneCore.firewallIP[0]='\0';
	//Init ALL SIP parameters
	initSIPparameters(pCore);
 
	//FIX BROADWORKS
 	if (siphoneCore.NATMode == (int)NAT_STUN ) 
	{
 		stun_get_nattype(&siphoneCore);
   		ret =stun_get_localip(&siphoneCore, StunResponse, &StunOutPort);
		if (ret){
  			strcpy(siphoneCore.firewallIP, StunResponse);
 		} 
	}
   	if (StunOutPort>0) 
 		pCore->SipServerPort= StunOutPort;
	
  //FIX BROADWORKS FIX
 //	pCore->SipServerPort=5060;

	// Initiate DSPnegotiationData for each Account ID
	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
		eXosip_sdp_negotiation_init(i,  &sdpNegotiationData[i], &siphoneCore); 
	
	// si_media_vprofile_init( ); 
	// Initialize Codecs, Media and Audio Path
	// Initialize QOS
	// Initialize Cipher - sRTP
 
	if(eXosip_init()) 
 	    {FatalError(EXOSIP_INIT_FAILED, "exoSip initiation failed ");return -1;}
 
  if (siphoneCore.IPv6)
	{
		inet= AF_INET6;
    printf(" #########  AF_INET6; ########\n");
		eXosip_enable_ipv6(1);
	}else
		eXosip_enable_ipv6(0);

	si_print(PRINT_LEVEL_INFO, "TRY eXosip_listen_addr  [%s]\n",siphoneCore.IPAddress);
 
	eXosip_set_option (EXOSIP_OPT_SRV_WITH_NAPTR,&siphoneCore.Naptr);
	// pass ip address to exosip library
  	//  eXosip_masquerade_contact(siphoneCore.IPAddress,pCore->SipServerPort);
 	if (siphoneCore.firewallIP[0]!='\0'){
  		eXosip_masquerade_contact(pCore->firewallIP,  pCore->SipServerPort);
 	}

    if (siphoneCore.transport == TRANSPORT_TCP)
	{
		if (eXosip_listen_addr(IPPROTO_TCP, (char*)siphoneCore.IPAddress, pCore->SipServerPort, inet,0))
		 {si_print(PRINT_LEVEL_CRIT, "FATAL eXosip_listen_addr TCP[%s]\n",siphoneCore.IPAddress); return -1 ;}
	}else if (siphoneCore.transport == TRANSPORT_TLS)
	{
		if (eXosip_listen_addr(IPPROTO_TCP, (char*)siphoneCore.IPAddress, pCore->SipServerPort, inet,1))
		{si_print(PRINT_LEVEL_CRIT, "FATAL eXosip_listen_addr TLS[%s]\n",siphoneCore.IPAddress); return -1 ;}
    }else {
		if (eXosip_listen_addr(IPPROTO_UDP, (char*)siphoneCore.IPAddress, pCore->SipServerPort, inet,0))
      {si_print(PRINT_LEVEL_CRIT, "FATAL eXosip_listen_addr UDP [%s]\n",siphoneCore.IPAddress); return -1 ;}
	}
	
	
	// UnderDev
	/*
		strcpy(siphoneCore.Gateway,"192.168.1.1");//YPAPA FIXEDVALUE
 		if (siphoneCore.Gateway[0]!='\0') 
			eXosip_set_option(EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, siphoneCore.Gateway);
	*/

  int value=1;
  eXosip_set_option(EXOSIP_OPT_DONT_SEND_101, &value);

	// eXosip_force_localip(pCore->firewallIP);
 	// Initialize Proxy
 	// Set User Agent string
 	eXosip_set_user_agent(SITEL_PHONE_USER_AGENT_LABEL);
 	sc1445x_mcu_create();

 	for (i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if (siphoneCore.useRegistration[i])	
		{
			strcpy(pCore->siregistration[i].regusername,&pCore->regusername[i][0]);
			strcpy(pCore->siregistration[i].regpassword,&pCore->regpassword[i][0]);
			strcpy(pCore->siregistration[i].reglogin,&pCore->reglogin[i][0]);
 			if (eXosip_add_authentication_info(	&pCore->regusername[i][0],
				&pCore->reglogin[i][0],&pCore->regpassword[i][0],NULL,pCore->regrealm))
		  	 {
					si_print(PRINT_LEVEL_CRIT, "\n\n\nFatalError(EXOSIP_SET_PARAMETER_FAILED REGISTRATION \n\n ");
 					return -1;
				}
		} 
	} 


   	// Initialize CONTACT url
 	// Initialize Payload
	//  	si_payload_init();
 	// Initialize Auto Configuration (Answer)

  	pCore->sipStatus = STATUS_SIP_INIT;
 
	return 0;
} 

  #include <net/if.h>

#include <sys/socket.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

int si_ua_readMACAddress(unsigned char *MACAddress)
{
	int sfd;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

	memset(&ifr, 0, sizeof ifr);

	if (0 > (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		si_print(PRINT_LEVEL_ERR, "Error opening socket to read MAC Address \n");
		return -1;
	}

	//strcpy(ifr.ifr_name, "eth0");
	strcpy(ifr.ifr_name, "br0");
	sin->sin_family = AF_INET;

	if (0 > ioctl(sfd, SIOCGIFHWADDR, &ifr)) {
		si_print(PRINT_LEVEL_ERR, "Unable to get MAC Address \n");
		return -1;
	}

	memcpy(MACAddress, (unsigned char *) &ifr.ifr_addr.sa_data, 6);

	if (MACAddress[0] + MACAddress[1] + MACAddress[2] + MACAddress[3] + MACAddress[4] + MACAddress[5]) {
		si_print(PRINT_LEVEL_INFO, "\n\nHW Address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n\n",
								MACAddress[0], MACAddress[1], MACAddress[2], 
								MACAddress[3], MACAddress[4], MACAddress[5]);

	}else memcpy(MACAddress, "00:01:02:03:04:05", 6);

	return 0;
}

 // gets interface addresses and displays them using getifaddrs()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <net/if.h>
#ifdef SIP_IPv6
#include <ifaddrs.h>
#endif

void
print_addr(struct sockaddr *sa, unsigned char* host)
{
 int err;
  if (sa == NULL) {
  printf("\n");
  return;
 }
 
 switch (sa->sa_family) {
 case AF_INET6:
   err = getnameinfo(sa, sizeof(struct sockaddr_in6), (char*) host, 64, NULL, 0, NI_NUMERICHOST);
   if (err>0) {
		perror("getnameinfo");
	    exit(EXIT_FAILURE);
	}
  break;
 default:
 //case AF_INET:
   err = getnameinfo(sa, sizeof(struct sockaddr_in),
      (char*) host, 64, NULL, 0, NI_NUMERICHOST);
  if (err>0) {
   perror("getnameinfo");
   exit(EXIT_FAILURE);
  }
 
  printf("\n");
  return;
 }

  printf("[%s]\n", host);
}
 
#ifdef SIP_IPv6
void si_ua_getAddress6(unsigned char* address)
{
	unsigned char host[64];
	struct ifaddrs *ifap0 = NULL, *ifap = NULL;
	char buf[BUFSIZ];

	memset(buf, 0, sizeof(buf));

	if (getifaddrs(&ifap0)) {
 		goto fin;
	}

	for (ifap = ifap0; ifap; ifap=ifap->ifa_next) 
	{
		if (ifap->ifa_addr == NULL)
			continue;

		if( ifap->ifa_addr->sa_family == AF_INET6 && (strcmp(ifap->ifa_name, "eth0") == 0) )
		{
			printf("Interface: %s\n", ifap->ifa_name);      
			printf("\tAddress: ");
			print_addr(ifap->ifa_addr, address);
			printf("\tNetmask: ");
			print_addr(ifap->ifa_netmask, host);
			printf("\tBcast: ");
			print_addr(ifap->ifa_broadaddr, host);
			printf("------\n");
		}
	}   
fin:
 freeifaddrs(ifap);   
} 
#endif

void si_ua_getAddress4(unsigned char* address)
{
int fd;
struct ifreq ifr;
struct sockaddr_in *sin;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd )
	{
		memset(&ifr, 0, sizeof(struct ifreq));
		ifr.ifr_addr.sa_family = AF_INET;
		//strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name));
		strncpy(ifr.ifr_name, "br0", sizeof(ifr.ifr_name));
 		if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) < 0) 
		{
			si_print(PRINT_LEVEL_ERR, "\nUnable to read IP Address \n");
			strcpy((char*)address, "0.0.0.0");
			close (fd);
 			return ;
		}
 
	}else{
		si_print(PRINT_LEVEL_ERR, "\nUnable to read IP Address \n");
		strcpy((char*)address, "0.0.0.0");
		return ;
	}
	sin = (struct sockaddr_in *)&ifr.ifr_addr;

	snprintf((char*)address,16,"%s", inet_ntoa(sin->sin_addr));
	close (fd);
}
 
void FatalError(int error, const char* ErrorString)
{
	si_print(PRINT_LEVEL_ERR, "\nFatal Error : %s [%d] \n", ErrorString, error);
}



void InitiateAllCalls(void)
{
	int i;
	sicall_t *pCall;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++) 
	{
		pCall=&SiCall[i];
 		memset ((char*)pCall, 0, sizeof (sicall_t));

		pCall->tid =(int)-1;
 		pCall->cid =(int)-1;
 
		pCall->did =(int)-1;
 		pCall->accountID =(int)0;
  	pCall->port =  -1;
		pCall->callid =0xffffffff;
	}
}



