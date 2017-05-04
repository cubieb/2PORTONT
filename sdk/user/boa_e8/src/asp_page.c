#include "asp_page.h"
#include "port.h" // ANDREW
#ifdef SUPPORT_ASP
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../include/linux/autoconf.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boa.h"
//ANDREW #include "rtl865x/rome_asp.c"

asp_name_t *root_asp=NULL;
form_name_t *root_form=NULL;
temp_mem_t root_temp;
char *query_temp_var=NULL;

//ANDREW int boaASPDefine(char* name, int (*fn)(request * req,  int argc, char **argv))
int boaASPDefine(const char* const name, int (*fn)(int eid, request * req,  int argc, char **argv))
{
	asp_name_t *asp;
	asp=(asp_name_t*)malloc(sizeof(asp_name_t));
	if(asp==NULL) return ERROR;
	/*
	asp->pagename=(char *)malloc(strlen(name)+1);
	strcpy(asp->pagename,name);
	*/
	asp->pagename = name; // andrew, use the constant directly

	asp->next=NULL;
	asp->function=fn;


	if(root_asp==NULL)
		root_asp=asp;
	else
	{
		// prepend the newly-created asp to the list pointed by root_asp
		asp->next = root_asp;
		root_asp = asp;
	}
	return SUCCESS;
}

#ifdef MULTI_USER_PRIV
typedef enum {PRIV_USER=0, PRIV_ENG=1, PRIV_ROOT=2};	// copy from mib.h
struct formPrivilege formPrivilegeList[] = {
	{"formTcpipLanSetup", PRIV_ROOT},
#ifdef _CWMP_MIB_
	{"formTR069Config", PRIV_ROOT},
	{"formTR069CPECert", PRIV_ROOT},
	{"formTR069CACert", PRIV_ROOT},
	{"formTR069CACertDel", PRIV_ROOT},
	{"formMidwareConfig", PRIV_ROOT},
#endif
#ifdef PORT_FORWARD_GENERAL
	{"formPortFw", PRIV_ROOT},
#endif
#ifdef NATIP_FORWARDING
	{"formIPFw", PRIV_ROOT},
#endif
#ifdef PORT_TRIGGERING
	{"formGaming", PRIV_ROOT},
#endif
	{"formFilter", PRIV_ROOT},
#ifdef LAYER7_FILTER_SUPPORT
	{"formlayer7", PRIV_ROOT},
#endif
#ifdef PARENTAL_CTRL
	{"formParentCtrl", PRIV_ROOT},
#endif
	{"formDMZ", PRIV_ROOT},
	{"formPasswordSetup", PRIV_ROOT},
	{"formUserPasswordSetup", PRIV_USER},
#ifdef ACCOUNT_CONFIG
	{"formAccountConfig", PRIV_ROOT},
#endif
#ifdef WEB_UPGRADE
	{"formUpload", PRIV_ENG},
#endif
#ifdef SUPPORT_WEB_PUSHUP
	{"formUpgradePop", PRIV_USER},
#endif
	{"formSaveConfig", PRIV_ENG},
	{"formSnmpConfig", PRIV_ROOT},
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
	{"formRadvdSetup", PRIV_ROOT},
#endif
	{"formSetAdsl", PRIV_ROOT},
	{"formAdslSafeMode", PRIV_ROOT},
	{"formStatAdsl", PRIV_USER},
	{"formDiagAdsl", PRIV_ROOT},
	{"formStats", PRIV_USER},
	{"formRconfig", PRIV_ROOT},
	{"formSysCmd", PRIV_ROOT},
#ifdef CONFIG_USER_RTK_SYSLOG
	{"formSysLog", PRIV_USER},
#endif
	{"formSetAdslTone", PRIV_ROOT},
	{"formStatus", PRIV_USER},
	{"formStatusModify", PRIV_ROOT},
	{"formWanAtm", PRIV_ROOT},
	{"formWanAdsl", PRIV_ENG},
	{"formPPPEdit", PRIV_ENG},
	{"formIPEdit", PRIV_ENG},
	{"formBrEdit", PRIV_ENG},
	{"formBridging", PRIV_ROOT},
	{"formRefleshFdbTbl", PRIV_USER},
	{"formRouting", PRIV_ENG},
#ifdef CONFIG_IPV6
	{"formIPv6Routing", PRIV_ENG},
#endif
	{"formRefleshRouteTbl", PRIV_USER},
#ifdef CONFIG_IPV6
	{"formIPv6RefleshRouteTbl", PRIV_USER},
#endif
	{"formDhcpServer", PRIV_ROOT},
	{"formReflashClientTbl", PRIV_USER},
	{"formDNS", PRIV_ROOT},
#ifdef CONFIG_USER_DDNS
	{"formDDNS", PRIV_USER},
#endif
	{"formDhcrelay", PRIV_ROOT},
	{"formPing", PRIV_USER},
#ifdef CONFIG_USER_TCPDUMP_WEB
	{"formCapture", PRIV_ROOT},
#endif
	{"formOAMLB", PRIV_USER},
#ifdef ADDRESS_MAPPING
	{"formAddressMap", PRIV_ROOT},
#endif
	{"formFinishMaintenance", PRIV_ROOT},
#ifdef USE_LOGINWEB_OF_SERVER
	{"formLogin", PRIV_USER},
#endif
	{"formReboot", PRIV_USER},
	{"formDhcpMode", PRIV_ROOT},
#ifdef CONFIG_USER_IGMPPROXY
	{"formIgmproxy", PRIV_ROOT},
#endif
	{"formMLDSnooping", PRIV_ROOT},    // Mason Yu. MLD snooping
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"formUpnp", PRIV_ROOT},
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
	{"formRip", PRIV_ROOT},
#endif
#ifdef IP_ACL
	{"formACL", PRIV_ROOT},
#endif
#ifdef NAT_CONN_LIMIT
	{"formConnlimit", PRIV_ROOT},
#endif
	{"formmacBase", PRIV_ROOT},
#ifdef URL_BLOCKING_SUPPORT
	{"formURL", PRIV_ROOT},
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER
	{"formLedTimer", PRIV_ROOT},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	{"formDOMAINBLK", PRIV_ROOT},
#endif
#ifdef TIME_ZONE
	{"formNtp", PRIV_ROOT},
#endif
	{"formOthers", PRIV_ROOT},
#ifdef AUTO_PROVISIONING
	{"formAutoProvision", PRIV_ROOT},
#endif
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
	{"formLinkMode", PRIV_ROOT},
#endif
#else // of CONFIG_RTL_MULTI_LAN_DEV
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	{"formLinkMode", PRIV_ROOT},
#endif
#endif	// of CONFIG_RTL_MULTI_LAN_DEV
#ifdef IP_QOS
	{"formIPQoS", PRIV_ROOT},
#endif
	{"formSAC", PRIV_ROOT},
#ifdef WLAN_SUPPORT
	{"formWlanSetup", PRIV_ROOT},
	{"formWirelessTbl", PRIV_USER},
#ifdef WLAN_ACL
	{"formWlAc", PRIV_ROOT},
#endif
	{"formAdvanceSetup", PRIV_ROOT},
#ifdef WLAN_WPA
	{"formWlEncrypt", PRIV_ROOT},
#endif
#ifdef WLAN_WDS
	{"formWlWds", PRIV_ROOT},
	{"formWdsEncrypt", PRIV_ROOT},
#endif
#ifdef WLAN_CLIENT
	{"formWlSiteSurvey", PRIV_ROOT},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	{"formWsc", PRIV_ROOT},
#endif
	{"formWlanSchedule", PRIV_ROOT},
#endif	// of WLAN_SUPPORT
#ifdef DIAGNOSTIC_TEST
	{"formDiagTest", PRIV_USER},
#endif
#ifdef DOS_SUPPORT
	{"formDosCfg", PRIV_ROOT},
#endif
#ifdef WLAN_MBSSID
	{"formWlanMBSSID", PRIV_ROOT},
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
	{"formALGOnOff", PRIV_ROOT},
#endif
	{"\0", 0}
};

unsigned char getPagePriv(const char* const name)
{
	struct formPrivilege * pformPrivilege = formPrivilegeList;
	while(strlen(pformPrivilege->pagename)) {
		if (strcmp(pformPrivilege->pagename, name) == 0)
			return pformPrivilege->privilege;
		pformPrivilege ++;
	}
	return PRIV_ROOT;
}
#endif

int boaFormDefine(const char* const name, void (*fn)(request * req,  char* path, char* query))
{

	form_name_t *form;
	form=(form_name_t *)malloc(sizeof(form_name_t));
	if(form==NULL) return ERROR;
	/*
	form->pagename=(char *)malloc(strlen(name)+1);
	strcpy(form->pagename,name);
	*/
	form->pagename=name;// andrew, use the constant directly
#ifdef MULTI_USER_PRIV
//	form->privilege = priv;
	form->privilege = getPagePriv(form->pagename);
#endif
	form->next=NULL;
	form->function=fn;

	if(root_form==NULL)
		root_form=form;
	else
	{
		//modify the previous's next pointer
		form_name_t *now_form;
		now_form=root_form;
		while(now_form->next!=NULL) now_form=now_form->next;
		now_form->next=form;
	}
//	printf("boaFormDefine: form->pagename=%s form->privilege=%d\n", form->pagename, form->privilege);
	return SUCCESS;

}

int addTempStr(char *str)
{
	temp_mem_t *temp,*newtemp;
	temp=&root_temp;
	while(temp->next)
	{
		temp=temp->next;
	}
	newtemp=(temp_mem_t *)malloc(sizeof(temp_mem_t));
	if(newtemp==NULL) return FAILED;
	newtemp->str=str;
	newtemp->next=NULL;
	temp->next=newtemp;
	return SUCCESS;
}

void freeAllTempStr(void)
{
	temp_mem_t *temp,*ntemp;
	temp=root_temp.next;
	root_temp.next=NULL;
	while(1)
	{
		if(temp==NULL) break;
		ntemp=temp->next;
		free(temp->str);
		free(temp);
		temp=ntemp;
	}
}

int getcgiparam_adv(char *dst,char *query_string,char *param,int maxlen, int index)
{
	int len,plen;
	int y;
    int count =0;

	plen=strlen(param);
	while (*query_string)
	{
		len=strlen(query_string);

		if ((len=strlen(query_string))>plen)
			if (!strncmp(query_string,param,plen))
				if (query_string[plen]=='=')
				{//copy parameter
					query_string+=plen+1;
					y=0;
					while ((*query_string)&&(*query_string!='&'))
					{
						if ((*query_string=='%')&&(strlen(query_string)>2))
							if ((isxdigit(query_string[1]))&&(isxdigit(query_string[2])))
							{
								if (y<maxlen)
									dst[y++]=((toupper(query_string[1])>='A'?toupper(query_string[1])-'A'+0xa:toupper(query_string[1])-'0') << 4)
									+ (toupper(query_string[2])>='A'?toupper(query_string[2])-'A'+0xa:toupper(query_string[2])-'0');
								query_string+=3;
								continue;
							}
						if (*query_string=='+')
						{
							if (y<maxlen)
								dst[y++]=' ';
							query_string++;
							continue;
						}
						if (y<maxlen)
							dst[y++]=*query_string;
						query_string++;
					}
	
					if (y<maxlen && (index==count)) 
                    {
                          dst[y]=0;
					      return y;
					}
                    count ++;
				}
		while ((*query_string)&&(*query_string!='&')) query_string++;
		query_string++;
	}
	if (maxlen) dst[0]=0;
	return -1;
}

int getcgiparam(char *dst,char *query_string,char *param,int maxlen)
{
	int len,plen;
	int y;

	plen=strlen(param);
	while (*query_string)
	{
		len=strlen(query_string);

		if ((len=strlen(query_string))>plen)
			if (!strncmp(query_string,param,plen))
				if (query_string[plen]=='=')
				{//copy parameter
					query_string+=plen+1;
					y=0;
					while ((*query_string)&&(*query_string!='&'))
					{
						if ((*query_string=='%')&&(strlen(query_string)>2))
							if ((isxdigit(query_string[1]))&&(isxdigit(query_string[2])))
							{
								if (y<maxlen)
									dst[y++]=((toupper(query_string[1])>='A'?toupper(query_string[1])-'A'+0xa:toupper(query_string[1])-'0') << 4)
									+ (toupper(query_string[2])>='A'?toupper(query_string[2])-'A'+0xa:toupper(query_string[2])-'0');
								query_string+=3;
								continue;
							}
						if (*query_string=='+')
						{
							if (y<maxlen)
								dst[y++]=' ';
							query_string++;
							continue;
						}
						if (y<maxlen)
							dst[y++]=*query_string;
						query_string++;
					}
					if (y<maxlen) dst[y]=0;
					return y;
				}
		while ((*query_string)&&(*query_string!='&')) query_string++;
		query_string++;
	}
	if (maxlen) dst[0]=0;
	return -1;
}


char *boaGetVar_adv(request *req, char *var, char *defaultGetValue, int index)
{
	char *buf;
	struct stat statbuf;
	int ret=-1;

	if (req->method == M_POST)
	{
		int i;
		fstat(req->post_data_fd, &statbuf);
		buf=(char *)malloc(statbuf.st_size+1);
		if(buf==NULL) return defaultGetValue;
		lseek(req->post_data_fd, SEEK_SET, 0);
		read(req->post_data_fd,buf,statbuf.st_size);
		buf[statbuf.st_size]=0;
		i=statbuf.st_size - 1;
		while(i > 0)
		{
			if((buf[i]==0x0a)||(buf[i]==0x0d)) buf[i]=0;
			else break;
			i--;
		}
	}
	else
	{
		buf=req->query_string;
        
	}

	if(buf!=NULL)
	{
        memset(query_temp_var, 0 ,MAX_QUERY_TEMP_VAL_SIZE);
		ret=getcgiparam_adv(query_temp_var,buf,var,MAX_QUERY_TEMP_VAL_SIZE,index);
//        printf("==[%s:%d] index=%d ,query_temp_var=[%s]==\n",__func__,__LINE__,index,query_temp_var);
	}

	if(req->method == M_POST) free(buf);
	if(ret<0) return defaultGetValue;
	buf=(char *)malloc(ret+1);
	memcpy(buf,query_temp_var,ret);
	buf[ret]=0;
	addTempStr(buf);
    
	return (char *)buf;	//this buffer will be free by freeAllTempStr().

}

char *boaGetVar(request *req, char *var, char *defaultGetValue)
{
	char *buf;
	struct stat statbuf;
	int ret=-1;

	if (req->method == M_POST)
	{
		int i;
		fstat(req->post_data_fd, &statbuf);
		buf=(char *)malloc(statbuf.st_size+1);
		if(buf==NULL) return defaultGetValue;
		lseek(req->post_data_fd, SEEK_SET, 0);
		read(req->post_data_fd,buf,statbuf.st_size);
		buf[statbuf.st_size]=0;
		i=statbuf.st_size - 1;
		while(i > 0)
		{
			if((buf[i]==0x0a)||(buf[i]==0x0d)) buf[i]=0;
			else break;
			i--;
		}
	}
	else
	{
		buf=req->query_string;
	}

	if(buf!=NULL)
	{
		ret=getcgiparam(query_temp_var,buf,var,MAX_QUERY_TEMP_VAL_SIZE);
	}

	if(req->method == M_POST) free(buf);
	//printf("query str=%s ret=%d var=%s query_temp_var=%s\n",buf,ret,var,query_temp_var);
	if(ret<0) return defaultGetValue;
	buf=(char *)malloc(ret+1);
	memcpy(buf,query_temp_var,ret);
	buf[ret]=0;
	addTempStr(buf);
	return (char *)buf;	//this buffer will be free by freeAllTempStr().

}

unsigned int __flash_base;
unsigned int __crmr_addr;

//ANDREW int32 board_cfgmgr_init(uint8 syncAll);

void boaASPInit(int argc,char **argv)
{
	flashdrv_init();
	rtl8670_AspInit();
	query_temp_var=(char *)malloc(MAX_QUERY_TEMP_VAL_SIZE);
	if(query_temp_var==NULL) exit(0);

#ifdef ANDREW
	int shmid=0;
#ifndef __uClinux__
	__flash_base=rtl865x_mmap(0x1e000000,0x1000000); /* 0xbe000000~0xbeffffff */
	__crmr_addr=rtl865x_mmap(0x1c805000,0x1000)+0x0104; /* 0xbc805104 */
	printf("__flash_base=0x%08x\n",__flash_base);
	printf("__crmr_addr=0x%08x\n",__crmr_addr);
#endif
	shmid=shmget(SHM_PROMECFGPARAM,sizeof(romeCfgParam_t),0666|IPC_CREAT);
	pRomeCfgParam=shmat(shmid,(void*)0,0);

/*
 *	Initialize the memory allocator. Allow use of malloc and start
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */

	if (strstr(argv[0],"ip-up") || strstr(argv[0],"ip-down"))
	{
		ipupdown(argc,argv);
		goto main_end;
	}

	/* =============================================
		We always init cfgmgr module before all procedures
	============================================= */
	#if defined(CONFIG_RTL865X_DIAG_LED)
	if (board_cfgmgr_init(TRUE) != SUCCESS)
	{
		//re865xIoctl("eth0",SET_VLAN_IP,*(uint32*)(&(pRomeCfgParam->pptpCfgParam.ipAddr)),netmask,0);
		//re865xIoctl(char * name, uint32 arg0, uint32 arg1, uint32 arg2, uint32 arg3);
		re865xIoctl("eth0",RTL8651_IOCTL_DIAG_LED,2,0,0);
	}
	#else
	board_cfgmgr_init(TRUE); //before any flash read/write
	#endif /* CONFIG_RTL865X_DIAG_LED */

	if(argc>1)
	{

		/* =============================================
			Process argument
		============================================= */

		if (!strcmp(argv[1],"default"))
		{
			/*
				user call "boa default"
				--> We must do factory default after init cfgmgr system.
			*/
			cfgmgr_factoryDefault(0);
			printf("cfgmgr_factoryDefault() done!\n");
		}

		else if (!strcmp(argv[1],"save"))
		{
			cfgmgr_task();
			printf("call cfgmgr_task() to save romeCfgParam to flash done!\n");
		}

		else if (!strcmp(argv[1],"-c")) //start
		{
			/* demo system configuration inialization entry point
			 * defined in board.c
			 */
			sysInit();
			goto conti;
		}

		else if (!strcmp(argv[1],"bound"))
		{
			dhcpcIPBound();
		}

		else if (!strcmp(argv[1], "renew"))
		{
			dhcpcIPRenew();
		}

		else if (!strcmp(argv[1],"deconfig"))
		{
			dhcpcDeconfig();
		}

		else if (!strcmp(argv[1],"addacl"))
		{
			acl_tableDriverSingleHandle(ACL_SINGLE_HANDLE_ADD,atoi(argv[2]),atoi(argv[3]));
		}

		else if (!strcmp(argv[1],"delacl"))
		{
			acl_tableDriverSingleHandle(ACL_SINGLE_HANDLE_DEL,atoi(argv[2]),atoi(argv[3]));
		}

		goto main_end;
	}

conti:
	root_temp.next=NULL;
	root_temp.str=NULL;

	boaFormDefine("asp_setLan", asp_setLan);
	boaFormDefine("asp_setDmz", asp_setDmz);
	boaFormDefine("asp_dhcpServerAdvance", asp_dhcpServerAdvance);
	boaFormDefine("asp_setDhcpClient", asp_setDhcpClient);
	boaFormDefine("asp_setPppoeWizard", asp_setPppoeWizard);
	boaFormDefine("asp_setPppoe", asp_setPppoe);
	boaFormDefine("asp_setUnnumberedWizard", asp_setUnnumberedWizard);
	boaFormDefine("asp_setNat", asp_setNat);
	boaFormDefine("asp_setPing", asp_setPing);
	boaFormDefine("asp_setRouting", asp_setRouting);
	boaFormDefine("asp_setPort", asp_setPort);
	boaFormDefine("asp_setStaticWizard", asp_setStaticWizard);
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
	boaFormDefine("asp_setPptpWizard", asp_setPptpWizard);
	boaFormDefine("asp_setL2tpWizard", asp_setL2tpWizard);
	boaFormDefine("asp_setDhcpL2tpWizard", asp_setDhcpL2tpWizard);
#endif
	boaFormDefine("asp_upload", asp_upload);
	boaFormDefine("asp_setUrlFilter", asp_setUrlFilter);
	boaFormDefine("asp_restart", asp_restart);
	boaFormDefine("asp_systemDefault", asp_systemDefault);
	boaFormDefine("asp_wanIp", asp_wanIp);
	boaFormDefine("asp_setAcl", asp_setAcl);
	boaFormDefine("asp_setTZ", asp_setTZ);
	boaFormDefine("asp_setDdns", asp_setDdns);
	boaFormDefine("asp_setSpecialApplication", asp_setSpecialApplication);
	boaFormDefine("asp_setGaming", asp_setGaming);
	boaFormDefine("asp_setServerPort", asp_setServerPort);
	boaFormDefine("asp_setEventLog", asp_setEventLog);
	boaFormDefine("asp_setUpnp", asp_setUpnp);
	boaFormDefine("asp_setDos", asp_setDos);
	boaFormDefine("asp_setDosProc", asp_setDosProc);
	boaFormDefine("asp_setAlg", asp_setAlg);
	boaFormDefine("asp_setBt", asp_setBt);
	boaFormDefine("asp_setUser", asp_setUser);
	boaFormDefine("asp_setMailAlert", asp_setMailAlert);
	boaFormDefine("asp_setRemoteLog", asp_setRemoteLog);
	boaFormDefine("asp_setRemoteMgmt", asp_setRemoteMgmt);
	boaFormDefine("asp_setWlanBasic", asp_setWlanBasic);
	boaFormDefine("asp_setWlanAdvance", asp_setWlanAdvance);
	boaFormDefine("asp_setWlanSecurity", asp_setWlanSecurity);
	boaFormDefine("asp_setWlanAccessCtrl", asp_setWlanAccessCtrl);
	boaFormDefine("asp_selectWlanNeighborAp", asp_selectWlanNeighborAp);
	boaFormDefine("asp_setUdpBlocking", asp_setUdpBlocking);
	boaFormDefine("asp_setQos", asp_setQos);
	boaFormDefine("asp_setQos1", asp_setQos1);
	boaFormDefine("asp_setpseudovlan", asp_setpseudovlan);
	boaFormDefine("asp_setPbnat", asp_setPbnat);
	boaFormDefine("asp_setFlash", asp_setFlash);
	boaFormDefine("asp_setRateLimit", asp_setRateLimit);
	boaFormDefine("asp_setRatio_qos", asp_setRatio_qos);
	boaFormDefine("asp_setModeConfig", asp_setModeConfig);
	boaFormDefine("asp_setRipConfig", asp_setRipConfig);
	boaFormDefine("asp_setPassthru", asp_setPassthru);
	boaFormDefine("asp_setMcast", asp_setMcast);
	boaFormDefine("asp_setNaptOpt", asp_setNaptOpt);
	boaASPDefine("asp_rateLimit", asp_rateLimit);
	boaASPDefine("asp_acl", asp_acl);
	boaASPDefine("asp_dmz", asp_dmz);
	boaASPDefine("asp_ddns", asp_ddns);
	boaASPDefine("asp_upnp", asp_upnp);
	boaASPDefine("asp_dos", asp_dos);
	boaASPDefine("asp_dosProc", asp_dosProc);
	boaASPDefine("asp_alg", asp_alg);
	boaASPDefine("asp_bt", asp_bt);
	boaASPDefine("asp_user", asp_user);
	boaASPDefine("asp_mailAlert", asp_mailAlert);
	boaASPDefine("asp_remoteLog", asp_remoteLog);
	boaASPDefine("asp_remoteMgmt", asp_remoteMgmt);
	boaASPDefine("asp_urlFilter", asp_urlFilter);
	boaASPDefine("asp_pppoe", asp_pppoe);
	boaASPDefine("asp_ping", asp_ping);
	boaASPDefine("asp_routing", asp_routing);
	boaASPDefine("asp_port", asp_port);
	boaASPDefine("asp_TZ", asp_TZ);
	boaASPDefine("asp_TZ2", asp_TZ2);
	boaASPDefine("asp_eventLog", asp_eventLog);
	boaASPDefine("asp_serverPort", asp_serverPort);
	boaASPDefine("asp_specialApplication", asp_specialApplication);
	boaASPDefine("asp_gaming", asp_gaming);
	boaASPDefine("asp_statusExtended", asp_statusExtended);
	boaASPDefine("asp_configChanged", asp_configChanged);
	boaASPDefine("asp_flashGetCfgParam", asp_flashGetCfgParam);
	boaASPDefine("asp_dhcpServerLeaseList", asp_dhcpServerLeaseList);
	boaASPDefine("asp_flashGetPppoeParam", asp_flashGetPppoeParam);
	boaASPDefine("asp_getWanAddress", asp_getWanAddress);
	boaASPDefine("asp_flashGetCloneMac", asp_flashGetCloneMac);
	boaASPDefine("asp_flashGetString", asp_flashGetString);
	boaASPDefine("asp_flashGetIpElement", asp_flashGetIpElement);
	boaASPDefine("asp_wlanBasic", asp_wlanBasic);
	boaASPDefine("asp_wlanAdvance", asp_wlanAdvance);
	boaASPDefine("asp_wlanSecurity", asp_wlanSecurity);
	boaASPDefine("asp_wlanAccessCtrl", asp_wlanAccessCtrl);
	boaASPDefine("asp_wlanClientList", asp_wlanClientList);
	boaASPDefine("asp_wlanNeighborApList", asp_wlanNeighborApList);
	boaASPDefine("asp_updateFW", asp_updateFW);
	boaASPDefine("asp_reboot", asp_reboot);
	boaASPDefine("asp_udpBlocking", asp_udpBlocking);
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
	boaASPDefine("asp_pptpWizard", asp_pptpWizard);
	boaASPDefine("asp_l2tpWizard", asp_l2tpWizard);
	boaASPDefine("asp_dhcpL2tpWizard", asp_dhcpL2tpWizard);
#endif
	boaASPDefine("asp_qos", asp_qos);
	boaASPDefine("asp_pseudovlan", asp_pseudovlan);
	boaASPDefine("asp_getLanPortStatus", asp_getLanPortStatus);
	boaASPDefine("asp_getWanPortStatus", asp_getWanPortStatus);
	boaASPDefine("asp_pbnat", asp_pbnat);
	boaASPDefine("asp_flash", asp_flash);
	boaASPDefine("asp_ratio_qos", asp_ratio_qos);
	boaASPDefine("asp_webcam", asp_webcam);
	boaASPDefine("asp_naptOpt", asp_naptOpt);
	boaASPDefine("asp_modeConfig", asp_modeConfig);
	boaASPDefine("asp_RipConfig", asp_RipConfig);
//	boaASPDefine("asp_aclPage", asp_aclPage);
	boaASPDefine("asp_serverpPage", asp_serverpPage);
	boaASPDefine("asp_urlfilterPage", asp_urlfilterPage);
	boaASPDefine("asp_qosPage", asp_qosPage);
	boaASPDefine("asp_ratelimitPage", asp_ratelimitPage);
	boaASPDefine("asp_ratio_qosPage", asp_ratio_qosPage);
	boaASPDefine("asp_routingPage", asp_routingPage);
	boaASPDefine("asp_ripPage", asp_ripPage);
	boaASPDefine("asp_ddnsPage", asp_ddnsPage);
	boaASPDefine("asp_specialapPage", asp_specialapPage);
	boaASPDefine("asp_gamingPage", asp_gamingPage);
	boaASPDefine("asp_algPage", asp_algPage);
	boaASPDefine("asp_dmzPage", asp_dmzPage);
	boaASPDefine("asp_dosPage", asp_dosPage);
	boaASPDefine("asp_udpblockingPage", asp_udpblockingPage);
	boaASPDefine("asp_pbnatPage", asp_pbnatPage);
	boaASPDefine("asp_pingPage", asp_pingPage);
	boaASPDefine("asp_naptoptPage", asp_naptoptPage);
	boaASPDefine("asp_pseudovlanPage", asp_pseudovlanPage);
	boaASPDefine("asp_passthruPage", asp_passthruPage);
	boaASPDefine("asp_mcastPage", asp_mcastPage);
	boaASPDefine("asp_bittorrentPage", asp_bittorrentPage);
	boaASPDefine("asp_passthru", asp_passthru);
	boaASPDefine("asp_mcast", asp_mcast);
	boaASPDefine("asp_IpsecExist", asp_IpsecExist);
#ifdef CONFIG_KLIPS
	boaASPDefine("asp_flashGetIpsec", asp_flashGetIpsec);
	boaASPDefine("asp_GetIpsecStatus", asp_GetIpsecStatus);
	boaFormDefine("asp_setIpsecBasic", asp_setIpsecBasic);
	boaFormDefine("asp_IpsecConnect", asp_IpsecConnect);
#endif

	query_temp_var=(char *)malloc(MAX_QUERY_TEMP_VAL_SIZE);
	if(query_temp_var==NULL) exit(0);

	return;

main_end:
	shmdt(pRomeCfgParam);
	exit(0);
#endif // UNUSED
}

int boaRedirect(request *req,char *url)
{
	req->buffer_end=0;
	send_redirect_perm(req,url);
	return SUCCESS;
}

int boaRedirectTemp(request *req,char *url)
{
	req->buffer_end=0;
	send_redirect_temp(req,url);
	return SUCCESS;
}


int allocNewBuffer(request *req)
{
	char *newBuffer;
	newBuffer=(char *)malloc(req->max_buffer_size*2+1);
	if(newBuffer==NULL) return FAILED;
	memcpy(newBuffer,req->buffer,req->max_buffer_size);
	req->max_buffer_size<<=1;
	free(req->buffer);
	req->buffer=newBuffer;
	return SUCCESS;
}

int boaWriteBlock(request *req, char *buf, int nChars)
{
	int bob=nChars;
	int pos = 0;

#ifndef SUPPORT_ASP
	if((bob+req->buffer_end)>BUFFER_SIZE) bob=BUFFER_SIZE- req->buffer_end;
#else
	while((bob+req->buffer_end)>req->max_buffer_size)
	{
		int ret;
#if MINIMIZE_RAM_USAGE
		int byte_to_send;

		byte_to_send = req->max_buffer_size - req->buffer_end;
		if (byte_to_send > bob)
			byte_to_send = bob;

		memcpy(req->buffer + req->buffer_end,	buf + pos, byte_to_send);
		req->buffer_end += byte_to_send;
		pos += byte_to_send;
		bob -= byte_to_send;
		ret = req_flush_retry(req);
#else  // MINIMIZE_RAM_USAGE
		ret=allocNewBuffer(req);
		if(ret==FAILED) {bob=BUFFER_SIZE- req->buffer_end; break;}
#endif // MINIMIZE_RAM_USAGE
	}
#endif

	if(bob>0)
	{
		memcpy(req->buffer + req->buffer_end,	buf+pos, bob);
		req->buffer_end+=bob;
		return 0;
	}
	return -1;
}

void handleForm(request *req)
{
	char *ptr;
	#define SCRIPT_ALIAS "/boaform/"
	// kaotest
	char *myalias;
	#define SCRIPT_USERALIAS "/boaform/admin/"
	ptr=strstr(req->request_uri,SCRIPT_USERALIAS);
	if (ptr==NULL) {
	ptr=strstr(req->request_uri,SCRIPT_ALIAS);
		myalias = "/boaform/";
	}
	else
		myalias = "/boaform/admin/";
//	printf("into handleForm %s........\n",req->request_uri);

	if(ptr==NULL)
	{
		send_r_not_found(req);
		return;
	}
	else
	{
		form_name_t *now_form;
		// kaotest
		ptr+=strlen(myalias);
		//ptr+=strlen(SCRIPT_ALIAS);

		now_form=root_form;
		while(1)
		{
			if (	(strlen(ptr) == strlen(now_form->pagename)) &&
				(memcmp(ptr,now_form->pagename,strlen(now_form->pagename))==0))
			{
 #ifdef VOIP_SUPPORT
  /* for VoIP config load page */
				if(0 == strncmp(now_form->pagename,"voip_config_load", strlen("voip_config_load"))){
					now_form->function(req,NULL,NULL);
					send_r_request_ok(req);
				}else{
					send_r_request_ok(req);
					now_form->function(req,NULL,NULL);
				}
#else
#ifdef MULTI_USER_PRIV
				//printf("handleForm: req->user=%s form->pagename=%s form->privilege=%d\n", req->user, now_form->pagename, now_form->privilege);
				//boaRedirect(req, now_form->pagename);
				if ((int)now_form->privilege > getAccPriv(req->user)) {
					boaHeader(req);
					boaWrite(req, "<body><blockquote><h2><font color=\"#FF0000\">Access Denied !!</font></h2><p>\n");
					boaWrite(req, "<h4>Sorry, you don't have enough privilege to take this action.</h4>\n");
					boaWrite(req, "<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>");
					boaFooter(req);
					boaDone(req, 200);
				}
				else {
					send_r_request_ok(req);		/* All's well */
					now_form->function(req,NULL,NULL);
				}
#else
				send_r_request_ok(req);		/* All's well */
				now_form->function(req,NULL,NULL);
#endif
#endif
				freeAllTempStr();
				//boaWrite(req,"okokok\n");
				return;
			}

			if(now_form->next==NULL) break;
			now_form=now_form->next;
		}

		send_r_not_found(req);
		return;
	}
}

void handleScript(request * req, char *left, char *right)
{
	asp_name_t *now_asp;
	uint32 funcNameLength;
	int i;

	/*
	 * left = strstr(last_right, "<%");
	 * right = strstr(left, "%>");
	 */
	left += 2;
	right -= 1;
	while (left < right) {
		left += strspn(left, " ;(),");
		if (left >= right)
			break;

		/* count the length of the function name */
		funcNameLength = strcspn(left, "( ");

		for (now_asp = root_asp; now_asp; now_asp = now_asp->next) {
			if (strlen(now_asp->pagename) == funcNameLength &&
			    strncmp(left, now_asp->pagename, funcNameLength) == 0) {
				char *start, *end, *semicolon;
				int argc = 0;
				char *argv[8] =
				    { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
					NULL
				};

				/* 
				 *       start   end right
				 *         |      |    |
				 *         v      v    v
				 * <% func(" args ");   %>
				 *    ^             ^
				 *    |             |
				 *   left        semicolon
				 */ 
				left += funcNameLength;
				semicolon = strchr(left, ';');
				for (; left < right && left < semicolon; argc++) {
					start = strchr(left, '"');
					if (start == NULL || start >= semicolon || start >= right)
						break;

					end = strchr(start + 1, '"');
					if (end == NULL || end >= semicolon || end >= right)
						break;

					left = end + 1;

					argv[argc] = strndup(start + 1, end - start - 1);
					if (argv[argc] == NULL)
						break;
				}

				now_asp->function(0, req, argc, argv);	// ANDREW

				for (i = 0; i < argc; i++)
					free(argv[i]);
				break;
			}
		}
		left += strcspn(left, " ") + 1;
	}
}
#endif

