#include "autoconf.h"


#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<unistd.h>

#include	<stdio.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<strings.h>

#include	"host.h"
#include	"ctypes.h"
#include	"debug.h"
#include	"rdx.h"
#include	"smp.h"
#include	"mis.h"
#include	"miv.h"
#include	"aps.h"
#include	"ap0.h"
#include	"asn.h"
#include	"asl.h"
#include	"avl.h"
#include	"udp.h"
#include	"systm.h"

#include <rtk/adslif.h>

// Added by Mason Yu for modify snmp agent
#include	<linux/atm.h>

// Added by Mason Yu for SystemName
#include 	<rtk/sysconfig.h>
#include 	<rtk/utility.h>

// Modified by Mason Yu for nclude 10 type contexts
//#define VARBIND_LENGTH 8  
#define VARBIND_LENGTH 10

//[OID TYPE VALUE]
typedef struct trapOIDTag {
  char *OID;
  char type;
  char *value;
}trapOIDType;

typedef struct trapArgTag {
  int genericTrap;
  int specificTrap;
  int argNum;
  //trapOIDType *trapOIDList;
  trapOIDType *trapOIDList[VARBIND_LENGTH];   // Modify by Mason Yu for include 10 type contexts
}trapArgType;


// Added by Mason Yu for Remote Management
#ifdef CONFIG_USER_SNMPD_MODULE_RM
extern char ConfigID[10];
extern char *productID;
extern char *vendorID;
extern char *systemVersion;
extern char *mibVersion;
extern char *serialNumber;
extern char *manufactureOUI;
// Kaohj
extern int	g_ftp_operStatus;
extern int	g_ftp_totalSize;
extern int	g_ftp_doneSize;
extern int	g_ftp_elapseTime;

#endif




SmpBindType VBList[VARBIND_LENGTH];
SmpBindPtrType VBPtr[VARBIND_LENGTH];
#define DATA_BUFFER_SIZE 1024  
char bindDataBfr[DATA_BUFFER_SIZE];  //value store buffer, increase it if you have many OID need put into a trap
//unsigned long bindDataBfr[DATA_BUFFER_SIZE];  //value store buffer, increase it if you have many OID need put into a trap

static struct  sockaddr        saremoteTrap;
static struct  sockaddr        salocalTrap;
static ApsIdType communityIdTrap;
static CCharPtrType lEOIDStringTrap;
static int sTrap;
// Added by Mason Yu
static int pvcsTrap;
static int eocsTrap;


static u_long lhostTrap;
static u_short lportTrap;

extern SmpStatusType	myUpCall (SmpIdType smp, SmpRequestPtrType req);

static SmpStatusType align_4_byte(int *loc, char *databfr)
{
            do {  //align to 4 byte
                if ((*loc)>DATA_BUFFER_SIZE) {
                    printf("Error! Trap data buffer full\n");
                    return (errBad);
                };
                databfr[(*loc)] = 0; (*loc)++;
            } while (((*loc)&0x03)!=0);
            
            return (errOk);
}

static SmpStatusType parseMIBvalue(trapOIDType *ptarpOID, SmpBindPtrType bind, int *loc, char *bindDataBfr)
{
  int k;
  
    //get type
    if (ptarpOID->type=='n') {
        bind->smpBindKind = smpKindNull;
    } else if (ptarpOID->type=='i') {
        bind->smpBindKind = smpKindInteger;
    } else if (ptarpOID->type=='c') {
        bind->smpBindKind = smpKindCounter;
    } else if (ptarpOID->type=='g') {
        bind->smpBindKind = smpKindGuage;
    } else if (ptarpOID->type=='t') {
        bind->smpBindKind = smpKindTimeTicks;
    } else if (ptarpOID->type=='s') {
        bind->smpBindKind = smpKindOctetString;
    } else if (ptarpOID->type=='a') {
        bind->smpBindKind = smpKindIPAddr;
    } else if (ptarpOID->type=='o') {
        bind->smpBindKind = smpKindObjectId;
    } else
        return errBad;
    
    //get value
    bind->smpBindValue = &bindDataBfr[*loc];
    bind->smpBindValueLen = DATA_BUFFER_SIZE - *loc;
    if ((k=smxTextToValue(bind, ptarpOID->value))==(CIntfType)-1) {
        if (bind->smpBindKind!=smpKindNull) return errBad;  //allow -1 when smpKindNull
    }
    
    *loc += k;
    if (errBad==align_4_byte(loc, bindDataBfr)) 
        return errBad;  //buffer full
        
    return errOk;
}

CByteType defaultEOID[]="1.3.1.4.1.3.1.1.0";  //enterprises.cmu.1.1
static SmpStatusType packTrapV1(trapArgType *Parameter, SmpRequestPtrType req, CCharPtrType lEOIDString)
{
  int loc;
  SmpBindPtrType bind;
  int i, k, req_build;
  FILE *fp;  	
  	
  	memset(bindDataBfr, 0, sizeof(bindDataBfr));
  	
	loc = 0; 
        k = smxTextToObjectId ((CBytePtrType) &bindDataBfr[loc],
                    (CIntfType) (DATA_BUFFER_SIZE-loc), lEOIDString);            
        req->smpRequestEnterprise = (char *) &bindDataBfr[loc];
        req->smpRequestEnterpriseLen = (int) k;
        loc += k;
        if (errBad==align_4_byte(&loc, bindDataBfr)) goto pack_error;  //buffer full
	
	
        //get Agent IP
        if (getIP("eth0", &bindDataBfr[loc])==0) {
            bindDataBfr[loc+0]=0xc0;
            bindDataBfr[loc+1]=0xa8;
            bindDataBfr[loc+2]=0x01;
            bindDataBfr[loc+3]=0x01;
        };
        req->smpRequestAgent = (SmpValueType)&bindDataBfr[loc];
        req->smpRequestAgentLen = 4;
        loc+=4;

	
        //get genericTrap
        if (Parameter->genericTrap>6)
            Parameter->genericTrap = 6;
        req->smpRequestGenericTrap = Parameter->genericTrap;

        //get specificTrap
        req->smpRequestSpecificTrap = Parameter->specificTrap;

	
        //time stamp
        req->smpRequestTimeStamp = 0;
 	fp = fopen("/proc/uptime", "r");
 	if (fp!=NULL) {
 	    char strTime[64];
 	    int i;
 	    fgets(strTime, sizeof(strTime), fp);
 	    fclose(fp);
 	    i = 0;
 	    while (strTime[i]!='.') i++;
 	    strTime[i]=0;
 	    req->smpRequestTimeStamp = atoi(strTime)*100;
 	};

	
        //get var bind, this is the information will be encoded into trap
        req_build = 0;
        req->smpRequestCount = 0; 
        for (i=0;i<VARBIND_LENGTH;i++)
            VBPtr[i] = &VBList[i];
        req->smpRequestBinds = (SmpBindPtrType)VBPtr;
        for (i=0;i<Parameter->argNum;i++) {
            bind = &VBList[req->smpRequestCount];
            //put OID into data buffer
            k = smxTextToObjectId ((CBytePtrType) &bindDataBfr[loc],
                    (CIntfType) (DATA_BUFFER_SIZE-loc), Parameter->trapOIDList[i]->OID);
            bind->smpBindName = &bindDataBfr[loc];
            bind->smpBindNameLen = (SmpLengthType) k;
            loc += k;
           
            if (errBad==align_4_byte(&loc, bindDataBfr)) goto pack_error;  //buffer full

	   
	    //value of var bind
	    if (parseMIBvalue(Parameter->trapOIDList[i], bind, &loc, bindDataBfr)==errOk) {
	    	req_build = 1;
                req->smpRequestCount++;
            } else {
                printf ("Parse MIB value failed for var bind %d\n", i);
            };
            
            
            // Modified by Mason Yu for include 10 type contexts
            //if (req->smpRequestCount>=8) break;
            if (req->smpRequestCount>=VARBIND_LENGTH) break;
        };
       
        if (req_build==0) goto pack_error;
        
        req->smpRequestBinds = &VBList[0];

        return (errOk);

pack_error:
        return (errBad);
}

#ifdef CONFIG_DEV_xDSL
//always use SIGUSR1 as TRAP ID
static void setTrapPID(void)
{
  FILE *adslFp;
  obcif_arg	myarg;
  Modem_LossData mld;
 
 	#if defined(CONFIG_DSL_ON_SLAVE)
	if ((adslFp = fopen("/dev/xdsl_ipc", "r")) == NULL) {
	#else
	if ((adslFp = fopen("/dev/adsl0", "r")) == NULL) {
	#endif
		printf("ERROR: failed to open ADSL device\n");
		return;
	};
	

	//set threshold
	/*memset(&mld, 0, sizeof(Modem_LossData));
	mld.ESs_NE = 2;
	myarg.argsize = (int) RLCM_TRAP_THRESHOLD_SIZE;
	myarg.arg = (int) &mld;
	if (ioctl(fileno(adslFp), RLCM_SET_TRAP_THRESHOLD, &myarg) < 0) {
	 	printf("setTrapPID set threshold failed!\n");
	 	goto set_stop;
	};*/
	
	//set trap process id
	myarg.argsize = (int) 4;
	myarg.arg = (int) getpid();
	if (ioctl(fileno(adslFp), RLCM_15MIN_WAIT_TRAP, &myarg) < 0) {
	 	printf("setTrapPID set process id failed!\n");
	};

set_stop:	
	fclose(adslFp);
	return;
}
#endif

void sendTrap(trapArgType *TrapParameter)
{
    SmpRequestType          req;
    struct  sockaddr_in     *sin;
    SmpIdType               smp;
    SmpSocketType           udp;
    SmpStatusType status;
	
	
        sin = (struct sockaddr_in *) & saremoteTrap;
        bzero ((char *) sin, sizeof (saremoteTrap));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = lhostTrap;
        sin->sin_port = htons(lportTrap);


        bzero ((char *) &req, sizeof (SmpRequestType));
        req.smpRequestCommunity = communityIdTrap;

        //if (lversion==0) {  //v1 trap
                req.smpRequestCmd = smpCommandTrap;
                status = packTrapV1(TrapParameter, &req, lEOIDStringTrap);
        //} else {  //v2 trap
        //        status = req.smpRequestCmd = smpCommandTrapV2;
        //};
       
        udp = udpNew (sTrap, sin->sin_addr.s_addr, sin->sin_port);
        smp = smpNew (udp, udpSend, myUpCall);     
  	
  	
        status = smpRequest(smp, &req);        
        smp = smpFree (smp);
        udp = udpFree (udp);	
        
}

#ifdef CONFIG_USER_SNMPD_MODULE_RM
// Added by Mason Yu for modify snmp agent
void sendPVCTrap(trapArgType *TrapParameter)
{
    SmpRequestType          req;
    struct  sockaddr_in     *sin;
    SmpIdType               smp;
    SmpSocketType           udp;
    SmpStatusType status;
    
	// Mason Yu
	// Although PVC channel not need the IP address and Port No.
	// But we still hold them here. Because the pvcSend() do not use them later. 
        sin = (struct sockaddr_in *) & saremoteTrap;
        bzero ((char *) sin, sizeof (saremoteTrap));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = lhostTrap;
        sin->sin_port = htons(lportTrap);


        bzero ((char *) &req, sizeof (SmpRequestType));
        req.smpRequestCommunity = communityIdTrap;

        //if (lversion==0) {  //v1 trap
                req.smpRequestCmd = smpCommandTrap;
                status = packTrapV1(TrapParameter, &req, lEOIDStringTrap);
        //} else {  //v2 trap
        //        status = req.smpRequestCmd = smpCommandTrapV2;
        //};
        
        udp = udpNew (pvcsTrap, sin->sin_addr.s_addr, sin->sin_port);
	smp = smpNew (udp, pvcSend, myUpCall);
     
  
        status = smpRequest(smp, &req);       
        smp = smpFree (smp);
        udp = udpFree (udp);	
}


// Added by Mason Yu for add EOC channel
void sendEOCTrap(trapArgType *TrapParameter)
{
    SmpRequestType          req;
    struct  sockaddr_in     *sin;
    SmpIdType               smp;
    SmpSocketType           udp;
    SmpStatusType status;
    
	// Mason Yu
	// Although EOC channel not need the IP address and Port No.
	// But we still hold them here. Because the pvcSend() do not use them later. 
        sin = (struct sockaddr_in *) & saremoteTrap;
        bzero ((char *) sin, sizeof (saremoteTrap));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = lhostTrap;
        sin->sin_port = htons(lportTrap);


        bzero ((char *) &req, sizeof (SmpRequestType));
        req.smpRequestCommunity = communityIdTrap;

        //if (lversion==0) {  //v1 trap
                req.smpRequestCmd = smpCommandTrap;
                status = packTrapV1(TrapParameter, &req, lEOIDStringTrap);
        //} else {  //v2 trap
        //        status = req.smpRequestCmd = smpCommandTrapV2;
        //};
        
        udp = udpNew (eocsTrap, sin->sin_addr.s_addr, sin->sin_port);
	smp = smpNew (udp, eocSend, myUpCall);
     
  
        status = smpRequest(smp, &req);        
        smp = smpFree (smp);
        udp = udpFree (udp);	
}
#endif

//register trap entry to driver
char TrapOUI[4]={ 0x01, 0xe0, 0x4c, 0x00};
SmpStatusType startTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int cnt)
{
    struct  sockaddr_in     *sin;
    int i;
    int	result;
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];
    char dataBfr[256];
    FILE *fp;
    char strVer[64], *strBld;      	
    	
   
    	if ( cnt == 1 ) {   			
        	//TRAP port
        	result=-1;
        	for (i=3000; ((i<3100)&&(result<0)); i++) {  //get a socket # for TRAP
        	        sin = (struct sockaddr_in *) & salocalTrap;
        	        bzero ((char *) sin, sizeof (salocalTrap));
        	        sin->sin_family = AF_INET;
        	        sin->sin_addr.s_addr = 0;
        	        sin->sin_port = htons(i);
        	
        	        result = bind (s, & salocalTrap, sizeof (*sin));
        	        i++;
        	};
        	if (result<0) {
        	        (void) perror ("Trap port");
        	        return (1);
        	}
	}	
				
	// Get Firmware Version		
	strVer[0]=0;
	strBld = 0;
	fp = fopen("/etc/version", "r");
	if (fp!=NULL) {
		fgets(strVer, sizeof(strVer), fp);  //main version
		fclose(fp);
		strBld = strchr(strVer, ' ');
		*strBld=0;
	}
	
	//community ID
	communityIdTrap = communityId;
	//enterprise OID string
	lEOIDStringTrap = lEOIDString;
	//s for bind
	sTrap = s;
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;	
	
	
	//build cold boot req
        //cold boot parameter
        //2 0 1.3.6.1.2.1.1.1.0 i 500
        ReBootParameter.genericTrap = 1;  //warmStart trap
        ReBootParameter.specificTrap = 0;
#ifdef CONFIG_USER_SNMPD_MODULE_RM       
        ReBootParameter.argNum = 9;    //# of inform to be send to trap host
#else
        ReBootParameter.argNum = 1;
#endif 
        //the information for warmStart trap
#ifdef CONFIG_USER_SNMPD_MODULE_RM        
        ReBootOID[0].OID = "1.3.6.1.4.1.16972.1.3.0";   // cpeSysInfoFirmware Version
        ReBootOID[0].type = 's';
        ReBootOID[0].value = strVer;
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];
        
        
        ReBootOID[1].OID = "1.3.6.1.4.1.16972.1.6.0";  //cpeSysInfoSystem Version
        ReBootOID[1].type = 's';
        ReBootOID[1].value = systemVersion;
        ReBootParameter.trapOIDList[1] = &ReBootOID[1];
        
        
        ReBootOID[2].OID = "1.3.6.1.4.1.16972.1.4.0";  //cpeSysInfoProductID
        ReBootOID[2].type = 's';
        ReBootOID[2].value = productID;
        ReBootParameter.trapOIDList[2] = &ReBootOID[2];
       
        
        ReBootOID[3].OID = "1.3.6.1.4.1.16972.1.5.0";  //cpeSysInfoVendorID
        ReBootOID[3].type = 's';
        ReBootOID[3].value = vendorID;
        ReBootParameter.trapOIDList[3] = &ReBootOID[3];
        
        
        ReBootOID[4].OID = "1.3.6.1.4.1.16972.1.10.0";  //cpeSysInfoManufacturerOui 
        ReBootOID[4].type = 's';         
        ReBootOID[4].value = TrapOUI;    //only accept string type        
        ReBootParameter.trapOIDList[4] = &ReBootOID[4];               
        //ReBootOID[4].type = 's';
        //ReBootOID[4].value = "00e04c";        
        //ReBootParameter.trapOIDList[4] = &ReBootOID[4];
        
        
        ReBootOID[5].OID = "1.3.6.1.4.1.16972.1.2.0";  //cpeSysInfoConfigID
        ReBootOID[5].type = 's';
        ReBootOID[5].value = ConfigID;
        ReBootParameter.trapOIDList[5] = &ReBootOID[5];
        
        
        ReBootOID[6].OID = "1.3.6.1.4.1.16972.1.7.0";  //cpeSysInfoWorkMode
        ReBootOID[6].type = 'i';
        sprintf(dataBfr, "%d", 2);
        ReBootOID[6].value = dataBfr;    //only accept string type        
        ReBootParameter.trapOIDList[6] = &ReBootOID[6];
        
        
        ReBootOID[7].OID = "1.3.6.1.4.1.16972.1.8.0";  //cpeSysInfoMibVersion
        ReBootOID[7].type = 's';
        ReBootOID[7].value = mibVersion;
        ReBootParameter.trapOIDList[7] = &ReBootOID[7];
        
        
        ReBootOID[8].OID = "1.3.6.1.4.1.16972.1.9.0";  //cpeSysInfoSerialNumber
        ReBootOID[8].type = 's';
        ReBootOID[8].value = serialNumber;
        ReBootParameter.trapOIDList[8] = &ReBootOID[8];
        
#else

	ReBootOID[0].OID = "1.3.6.1.2.1.1.1.0";  //systemDescr
        ReBootOID[0].type = 's';
        ReBootOID[0].value = "Realtek/RTL8670";
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];


#endif

	sendTrap(&ReBootParameter);
	
#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number        
        setTrapPID();
#endif        
       
        return status;
}

SmpStatusType warmStartTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int cnt)
{
    struct  sockaddr_in     *sin;
    int i;
    int	result;
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];
    char dataBfr[256];
    FILE *fp;
    char strVer[64], *strBld;    	
   
    	if ( cnt == 1 ) {    			
        	//TRAP port
        	result=-1;
        	for (i=3000; ((i<3100)&&(result<0)); i++) {  //get a socket # for TRAP
        	        sin = (struct sockaddr_in *) & salocalTrap;
        	        bzero ((char *) sin, sizeof (salocalTrap));
        	        sin->sin_family = AF_INET;
        	        sin->sin_addr.s_addr = 0;
        	        sin->sin_port = htons(i);
        	
        	        result = bind (s, & salocalTrap, sizeof (*sin));
        	        i++;
        	};
        	if (result<0) {
        	        (void) perror ("Trap port");
        	        return (1);
        	}
	}	
				
	// Get Firmware Version		
	strVer[0]=0;
	strBld = 0;
	fp = fopen("/etc/version", "r");
	if (fp!=NULL) {
		fgets(strVer, sizeof(strVer), fp);  //main version
		fclose(fp);
		strBld = strchr(strVer, ' ');
		*strBld=0;
	}
	
	//community ID
	communityIdTrap = communityId;
	//enterprise OID string
	lEOIDStringTrap = lEOIDString;
	//s for bind
	sTrap = s;
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;	
	
	
	//build cold boot req
        //cold boot parameter
        //2 0 1.3.6.1.2.1.1.1.0 i 500
        ReBootParameter.genericTrap = 1;  //warmStart trap
        ReBootParameter.specificTrap = 0;
        ReBootParameter.argNum = 1;
 
        //the information for warmStart trap
	ReBootOID[0].OID = "1.3.6.1.2.1.1.1.0";  //systemDescr
        ReBootOID[0].type = 's';        
        if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)dataBfr)) {			
		return (1);
	}       
        dataBfr[strlen(dataBfr)]= '\0';
        ReBootOID[0].value = dataBfr;      
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];

	sendTrap(&ReBootParameter);
	
#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number
        setTrapPID();
#endif
       
        return status;
}

#ifdef CONFIG_USER_SNMPD_MODULE_RM
// Added by Mason Yu for modify snmp agent
//register trap entry to driver
SmpStatusType startRMTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int channel)
{
    struct  sockaddr_in     *sin;
    int i;
    //int	result;
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];
    int err;
    char dataBfr[256];
    FILE *fp;
    struct sockaddr_atmpvc pvcaddr;
    char strVer[64], *strBld; 


// Mason Yu for multi pvc

#if 0
	pvcaddr.sap_family = AF_ATMPVC;
    	pvcaddr.sap_addr.itf = -1;
    	pvcaddr.sap_addr.vpi = 0;
    	pvcaddr.sap_addr.vci = 16; 

	err = connect(s, (struct sockaddr*)&pvcaddr, sizeof(pvcaddr));
    	if (err < 0) {
		printf("failed to connect on ATM socket(SnmpPVCTrap)\n");    
		return -1;
	}
#endif

	// Get Firmware Version		
	strVer[0]=0;
	strBld = 0;
	fp = fopen("/etc/version", "r");
	if (fp!=NULL) {
		fgets(strVer, sizeof(strVer), fp);  //main version
		fclose(fp);
		strBld = strchr(strVer, ' ');
		*strBld=0;
	}

	//community ID
	communityIdTrap = communityId;
	//enterprise OID string
	lEOIDStringTrap = lEOIDString;
	
	//s for bind
	if ( channel == ILMI_Channel )	
		pvcsTrap = s;
	else if ( channel == EOC_Channel )
		eocsTrap = s;
			
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;
	
	//build cold boot req
        //cold boot parameter
        //2 0 1.3.6.1.2.1.1.1.0 i 500
        ReBootParameter.genericTrap = 6;  //EnterpriseSpecfic trap
        ReBootParameter.specificTrap = 1;
        ReBootParameter.argNum = 9;    //# of inform to be send to trap host
        //the information for warmStart trap
        ReBootOID[0].OID = "1.3.6.1.4.1.16972.1.2.0";  //cpeSysInfoConfigID
        ReBootOID[0].type = 's';
        ReBootOID[0].value = ConfigID;
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];        
        
        ReBootOID[1].OID = "1.3.6.1.4.1.16972.1.6.0";  //cpeSysInfoSystem Version
        ReBootOID[1].type = 's';
        ReBootOID[1].value = systemVersion;
        ReBootParameter.trapOIDList[1] = &ReBootOID[1];
        
        
        ReBootOID[2].OID = "1.3.6.1.4.1.16972.1.4.0";  //cpeSysInfoProductID
        ReBootOID[2].type = 's';
        ReBootOID[2].value = productID;
        ReBootParameter.trapOIDList[2] = &ReBootOID[2];
       
        
        ReBootOID[3].OID = "1.3.6.1.4.1.16972.1.5.0";  //cpeSysInfoVendorID
        ReBootOID[3].type = 's';
        ReBootOID[3].value = vendorID;
        ReBootParameter.trapOIDList[3] = &ReBootOID[3];
        
        
        ReBootOID[4].OID = "1.3.6.1.4.1.16972.1.10.0";  //cpeSysInfoManufacturerOui        
        ReBootOID[4].type = 's';
        ReBootOID[4].value = TrapOUI;
        ReBootParameter.trapOIDList[4] = &ReBootOID[4];
        //ReBootOID[4].type = 's';
        //ReBootOID[4].value = "00e04c";        
        //ReBootParameter.trapOIDList[4] = &ReBootOID[4];
        
        
        ReBootOID[5].OID = "1.3.6.1.4.1.16972.1.3.0";   // cpeSysInfoFirmware Version
        ReBootOID[5].type = 's';
        ReBootOID[5].value = strVer;
        ReBootParameter.trapOIDList[5] = &ReBootOID[5];        
        
        ReBootOID[6].OID = "1.3.6.1.4.1.16972.1.7.0";  //cpeSysInfoWorkMode
        ReBootOID[6].type = 'i';
        sprintf(dataBfr, "%d", 1);
        ReBootOID[6].value = dataBfr;    //only accept string type        
        ReBootParameter.trapOIDList[6] = &ReBootOID[6];
        
        
        ReBootOID[7].OID = "1.3.6.1.4.1.16972.1.8.0";  //cpeSysInfoMibVersion
        ReBootOID[7].type = 's';
        ReBootOID[7].value = mibVersion;
        ReBootParameter.trapOIDList[7] = &ReBootOID[7];
        
        
        ReBootOID[8].OID = "1.3.6.1.4.1.16972.1.9.0";  //cpeSysInfoSerialNumber
        ReBootOID[8].type = 's';
        ReBootOID[8].value = serialNumber;
        ReBootParameter.trapOIDList[8] = &ReBootOID[8];

	
	if ( channel == ILMI_Channel )	
		sendPVCTrap(&ReBootParameter);
	else if ( channel == EOC_Channel )
		sendEOCTrap(&ReBootParameter);
	
	
#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number
        setTrapPID();
#endif
        
        return status;
}


extern long ftpAdmin;
extern char g_ftpFile[256];
extern const char OPERSTATUS[];
extern const char TOTALSIZE[];
extern const char ELAPSETIME[];
// Added by Mason Yu for Remote Management
//register trap entry to driver
SmpStatusType startFTPTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int channel)
{       
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];  				
    unsigned long value;
    char getfileName[100];
    long oper;
    FILE *fp;
    char dataBfr1[10], dataBfr2[10],dataBfr3[10],dataBfr4[10],dataBfr5[10];
    
	
	//community ID
	communityIdTrap = communityId;
	//enterprise OID string
	lEOIDStringTrap = lEOIDString;
	
	//s for bind
	if ( channel == ILMI_Channel )	
		pvcsTrap = s;
	else if ( channel == EOC_Channel )
		eocsTrap = s;	
			
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;
	
	// Kaohj
	//value=RMFtpRetrieveValueFromFile(OPERSTATUS, strlen(OPERSTATUS)+1);
	value = g_ftp_operStatus;
		
	if ( value == 2) {
		sprintf(getfileName, "/tmp/%s", g_ftpFile);
		if ( (fp = fopen(getfileName, "r")) == NULL )
		{
			//printf("file name=%s\n", getfileName);
			//printf("Open get file fail !\n");
			value=6;
		}
	}
	
	
        ReBootParameter.genericTrap = 6;  //EnterpriseSpecfic trap
        ReBootParameter.specificTrap = 2;       
        ReBootParameter.argNum = 5;    //# of inform to be send to trap host
 
        //the information for warmStart trap        
        ReBootOID[0].OID = "1.3.6.1.4.1.16972.4.5.0";   // cpeFtpAdminStatus
        ReBootOID[0].type = 'i';        
        sprintf(dataBfr1, "%d", ftpAdmin);
        //sprintf(dataBfr, "%d", 3);
        ReBootOID[0].value = dataBfr1;    //only accept string type        
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];
        
        
        ReBootOID[1].OID = "1.3.6.1.4.1.16972.4.6.0";  //cpeFtpOperStatus
        ReBootOID[1].type = 'i';
        sprintf(dataBfr2, "%d", value);
        //sprintf(dataBfr, "%d", 5);
        ReBootOID[1].value = dataBfr2;    //only accept string type       
        ReBootParameter.trapOIDList[1] = &ReBootOID[1];
	
	// Kaohj
	//value=RMFtpRetrieveValueFromFile(TOTALSIZE, strlen(TOTALSIZE)+1);
	value = g_ftp_totalSize;
	ReBootOID[2].OID = "1.3.6.1.4.1.16972.4.8.0";  //cpeFtpTotalSize
        ReBootOID[2].type = 'i';
        sprintf(dataBfr3, "%d", value);
        ReBootOID[2].value = dataBfr3;    //only accept string type       
        ReBootParameter.trapOIDList[2] = &ReBootOID[2];
	
	
	ReBootOID[3].OID = "1.3.6.1.4.1.16972.4.9.0";  //cpeFtpDoneSize
        ReBootOID[3].type = 'i';
        sprintf(dataBfr4, "%d", value);
        ReBootOID[3].value = dataBfr4;    //only accept string type       
        ReBootParameter.trapOIDList[3] = &ReBootOID[3];
        
        // Kaohj
        //value=RMFtpRetrieveValueFromFile(ELAPSETIME, strlen(ELAPSETIME)+1);
        value = g_ftp_elapseTime;
        ReBootOID[4].OID = "1.3.6.1.4.1.16972.4.10.0";  //cpeFtpElapseTime
        ReBootOID[4].type = 'i';
        sprintf(dataBfr5, "%d", value);
        ReBootOID[4].value = dataBfr5;    //only accept string type       
        ReBootParameter.trapOIDList[4] = &ReBootOID[4];
        
        
	if ( channel == ILMI_Channel )	
		sendPVCTrap(&ReBootParameter);
	else if ( channel == EOC_Channel )
		sendEOCTrap(&ReBootParameter);	
		
#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number        
        setTrapPID();
#endif
        
       
        return status;
}

extern 	int 	g_pingOperStatus;
extern  char	g_xPingString[500];
// Added by Mason Yu for Remote Management
//register trap entry to driver
SmpStatusType startPingTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int channel)
{       
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];    
    long oper;   
    char dataBfr[256];
    
	
	//community ID
	communityIdTrap = communityId;	
	lEOIDStringTrap = lEOIDString;
	
	//s for bind
	if ( channel == ILMI_Channel )	
		pvcsTrap = s;
	else if ( channel == EOC_Channel )
		eocsTrap = s;
		
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;	
	
	// Kaohj --- not set here
	#if 0
	if ( nreceived != 0 )
		g_pingOperStatus = 3;                 // Success
	else
		g_pingOperStatus = 4;	              // Failure
	#endif
	
        ReBootParameter.genericTrap = 6;  //EnterpriseSpecfic trap
        ReBootParameter.specificTrap = 3;       
        ReBootParameter.argNum = 2;    //# of inform to be send to trap host
 
        //the information for warmStart trap        
        ReBootOID[0].OID = "1.3.6.1.4.1.16972.5.1.5.0";   // cpePingOperStatus
        ReBootOID[0].type = 'i';        
        sprintf(dataBfr, "%d", g_pingOperStatus);
        ReBootOID[0].value = dataBfr;    //only accept string type        
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];
        
        
        ReBootOID[1].OID = "1.3.6.1.4.1.16972.5.1.6.0";   // cpePingResult              
        ReBootOID[1].type = 's';
        ReBootOID[1].value = g_xPingString;
        ReBootParameter.trapOIDList[1] = &ReBootOID[1];       
        
        
	//printf("startPingTrap: to ILMI or to Eoc\n");
	if ( channel == ILMI_Channel )	{
		//printf("startPingTrap: to ILMI\n");
		sendPVCTrap(&ReBootParameter);
	}else if ( channel == EOC_Channel ) {
		//printf("startPingTrap: to Eoc\n");
		sendEOCTrap(&ReBootParameter);
	}

#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number        
        setTrapPID();
#endif
       
        return status;
}



extern unsigned long 	g_pppTestRseult;
extern unsigned long 	g_cpePppIfIndex;
// Added by Mason Yu for Remote Management
//register trap entry to driver
SmpStatusType startPPPTrap(int s, u_long lhost, u_short lport, ApsIdType communityId, CCharPtrType lEOIDString, int channel)
{       
    SmpStatusType status;
    trapArgType ReBootParameter;
    trapOIDType ReBootOID[VARBIND_LENGTH];    
    unsigned long pppAuthStatus;   
    char dataBfr[256];
    char pppIfIndex[256];
	
	//community ID
	communityIdTrap = communityId;
	//enterprise OID string
	lEOIDStringTrap = lEOIDString;
	
	//s for bind
	if ( channel == ILMI_Channel )	
		pvcsTrap = s;
	else if ( channel == EOC_Channel )
		eocsTrap = s;
		
	//trap ip
	lhostTrap = lhost;
	//trap port num
	lportTrap = lport;
	
		
	
        ReBootParameter.genericTrap = 6;  //EnterpriseSpecfic trap
        ReBootParameter.specificTrap = 4;       
        ReBootParameter.argNum = 1;    //# of inform to be send to trap host
 
        //the information for warmStart trap   
        sprintf(pppIfIndex, "1.3.6.1.4.1.16972.2.4.1.9.%d", g_cpePppIfIndex);     
        //ReBootOID[0].OID = "1.3.6.1.4.1.16972.2.4.1.9";   // cpePppTestResult
        ReBootOID[0].OID = pppIfIndex;
        ReBootOID[0].type = 'i';        
        sprintf(dataBfr, "%d", g_pppTestRseult);
        ReBootOID[0].value = dataBfr;    //only accept string type        
        ReBootParameter.trapOIDList[0] = &ReBootOID[0];       
        
	
	if ( channel == ILMI_Channel )	
		sendPVCTrap(&ReBootParameter);
	else if ( channel == EOC_Channel )
		sendEOCTrap(&ReBootParameter);
	
#ifdef CONFIG_DEV_xDSL
        //tell obc pid & trap number        
        setTrapPID();
#endif
       
        return status;
}
#endif


//call if signal SIGUSR1 occurred, send trap to dst host
// Marked by Mason Yu. The following trapOccur() is original codes but it work fail. 
// I have moved this function to snmp.c and tested ok.
#if 0
void trapOccur(int sig)
{
  FILE *adslFp;
  obcif_arg	myarg;
  Modem_LossData mld;
  trapArgType trapParameter;
  trapOIDType trapOID[VARBIND_LENGTH];
  char dataBfr[256];  	
  	
  	signal(SIGUSR1, trapOccur);  //restart trap
	
	if ((adslFp = fopen("/dev/adsl0", "r")) == NULL) {
		printf("ERROR: failed to open ADSL device\n");
		return;
	};
	
	//get counter value
	memset(&mld, 0, sizeof(Modem_LossData));
	myarg.argsize = (int) RLCM_GET_LOSS_DATA_SIZE;
	myarg.arg = (int) &mld;
	if (ioctl(fileno(adslFp), RLCM_GET_TRAP_15MIN_LOSS_DATA, &myarg) < 0) {
	 	printf("trapOccur get threshold failed!\n");
	 	goto trapOccur_stop;
	};
	
	//pack trap
//build 15 min ES trap
        //ES trap parameter
        //6 0 1.3.6.1.2.1.1.1.0 i 500
        trapParameter.genericTrap = 6;  //warmStart trap
        trapParameter.specificTrap = 0;
        trapParameter.argNum = 1;    //# of inform to be send to trap host
        //the information for warmStart trap
        trapOID[0].OID = "1.3.6.1.2.1.10.94.1.1.7.1.11.0";  //15 min Es
        trapOID[0].type = 'i';
        sprintf(dataBfr, "%d", mld.ESs_NE);
        trapOID[0].value = dataBfr;  //only accept string type
        trapParameter.trapOIDList[0] = &trapOID[0];

	sendTrap(&trapParameter);
	
	// Added by Mason Yu for modify snmp agent
#ifdef CONFIG_USER_SNMPD_MODULE_RM 	
	sendPVCTrap(&trapParameter);
#endif
	
trapOccur_stop:	
	fclose(adslFp);
	return;	
}
#endif
