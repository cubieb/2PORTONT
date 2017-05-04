//#include <config/autoconf.h>
#include "autoconf.h"


#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <signal.h>
#include        <unistd.h>

#include        <stdio.h>
#include        <stdlib.h>
#include        <netdb.h>
#include        <strings.h>

#include        "host.h"
#include        "ctypes.h"
#include        "debug.h"
#include        "rdx.h"
#include        "smp.h"
#include        "mis.h"
#include        "miv.h"
#include        "aps.h"
#include        "ap0.h"
#include        "asn.h"
#include        "asl.h"
#include        "avl.h"
#include        "udp.h"
#include        "systm.h"
#include        "tcp_vars.h"
#include        "ip_vars.h"
#include        "iface_vars.h"
#include        "icmp_vars.h"
#include        "udp_vars.h"

#include        "adsl_vars.h"

#ifdef CONFIG_USER_SNMPD_MODULE_FRAMEWORK
#include        "framework_vars.h"
#endif


#define         cmdBufferSize           (2048)

static void done(int sig)
{
  exit(0);
}

static  void    cmdInit (void)
{
        aslInit ();
        asnInit ();
        misInit ();
        avlInit ();
        mixInit ();
        apsInit ();
        ap0Init ();
        smpInit ();

#ifdef CONFIG_USER_SNMPD_MODULE_SYSTEM
        systmInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_IP
        ipInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_IFACE
        ifaceInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_TCP
        tcpInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_ICMP
        icmpInit ();
#endif
#ifdef CONFIG_USER_SNMPD_MODULE_UDP
        udpInit ();
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_ADSL
        adslInit ();
#endif

#ifdef CONFIG_USER_SNMPD_MODULE_ATM
        atmInit ();
#endif


#ifdef CONFIG_USER_SNMPD_MODULE_FRAMEWORK
        frameworkInit ();
#endif
}

static  CIntfType       usage (CCharPtrType s)
{
        fprintf (stderr, "snmptrap -v 1 [COMMON OPTIONS] generic-trap specific-trap [OID TYPE VALUE]\n");
//ex: snmptrap -v 1 -h 172.19.31.181 -e 1.3.6.1.4.1.3.1.1.0 6 0 1.3.6.1.2.1.1.1.0 i 500
//        fprintf (stderr, "trap -v 2c [COMMON OPTIONS] trap-oid [OID]\n");
//ex: snmptrap -v 2c -h 172.19.31.181 -c public .1.3.6.1.2.1.10.94.1.2.2.0.4 .1.3.6.1.2.1.10.94.1.1.15.1.15 i 6
        fprintf (stderr, "\nCOMMON OPTIONS:\n");
        fprintf (stderr, " [-h lhost]\n");
        fprintf (stderr, " [-p lport]\n");
        fprintf (stderr, " [-c community]\n");
        fprintf (stderr, " [-e enterprise-oid]\n");
        fprintf (stderr, "\nOID TYPE\n \
  i: integer\n \
  c: counter\n \
  g: guage\n \
  t: time ticks\n \
  s: octet string\n \
  a: ip address\n \
  o: object id\n \
");
        return (1);
}

#define VARBIND_LENGTH 8  
SmpBindType VBList[VARBIND_LENGTH];
SmpBindPtrType VBPtr[VARBIND_LENGTH];
#define DATA_BUFFER_SIZE 1024  
char bindDataBfr[DATA_BUFFER_SIZE];  //value store buffer, increase it if you have many OID need put into a trap

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

static SmpStatusType parseMIBvalue(int *argc, char ***argv, SmpBindPtrType bind, int *loc, char *bindDataBfr)
{
  int k;
  
    if ((*argc)<=0) return errBad;
    
    //get type
    if (strcmp(**argv, "n")==0) {
        bind->smpBindKind = smpKindNull;
    } else if (strcmp(**argv, "i")==0) {
        bind->smpBindKind = smpKindInteger;
    } else if (strcmp(**argv, "c")==0) {
        bind->smpBindKind = smpKindCounter;
    } else if (strcmp(**argv, "g")==0) {
        bind->smpBindKind = smpKindGuage;
    } else if (strcmp(**argv, "t")==0) {
        bind->smpBindKind = smpKindTimeTicks;
    } else if (strcmp(**argv, "s")==0) {
        bind->smpBindKind = smpKindOctetString;
    } else if (strcmp(**argv, "a")==0) {
        bind->smpBindKind = smpKindIPAddr;
    } else if (strcmp(**argv, "o")==0) {
        bind->smpBindKind = smpKindObjectId;
    } else
        return errBad;
    (*argv)++; 
    (*argc)--; 
    if ((bind->smpBindKind!=smpKindNull)&&((*argc)<=0)) return errBad;  //no more args
    
    //get value
    bind->smpBindValue = &bindDataBfr[*loc];
    bind->smpBindValueLen = DATA_BUFFER_SIZE - *loc;
    if ((k=smxTextToValue(bind, **argv))==(CIntfType)-1) {
        if (bind->smpBindKind!=smpKindNull) return errBad;  //allow -1 when smpKindNull
    } else {
        (*argv)++; (*argc)--;
    };
    
    *loc += k;
    if (errBad==align_4_byte(loc, bindDataBfr)) 
        return errBad;  //buffer full
        
    return errOk;
}

static CByteType defaultEOID[]="1.3.1.4.1.3.1.1.0";  //enterprises.cmu.1.1
static SmpStatusType packTrapV1(int restArgc, char **argv, SmpRequestPtrType req, CCharPtrType lEOIDString)
{
  int genericTrap, specificTrap, loc;
  SmpBindPtrType bind;
  int i, k, req_build;
  FILE *fp;
  
        if (restArgc<2)
            return (errBad);
            
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
        genericTrap = atoi(*argv);
        if (genericTrap>6)
            genericTrap = 6;
        argv++; restArgc--;
        req->smpRequestGenericTrap = genericTrap;

        //get specificTrap
        specificTrap = atoi(*argv);
        argv++; restArgc--;
        req->smpRequestSpecificTrap = specificTrap;

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
        while (restArgc != 0) {
            bind = &VBList[req->smpRequestCount];
            //put OID into data buffer
            k = smxTextToObjectId ((CBytePtrType) &bindDataBfr[loc],
                    (CIntfType) (DATA_BUFFER_SIZE-loc), *argv);
            argv++;
            bind->smpBindName = &bindDataBfr[loc];
            bind->smpBindNameLen = (SmpLengthType) k;
            loc += k;
            if (errBad==align_4_byte(&loc, bindDataBfr)) goto pack_error;  //buffer full

	    //value of var bind
	    if (parseMIBvalue(&restArgc, &argv, bind, &loc, bindDataBfr)==errOk) {
	    	req_build = 1;
                req->smpRequestCount++;
            } else {
                printf ("Parse MIB value failed for %s\n", *argv);
            };
            
            restArgc--;
            if (req->smpRequestCount>=8) break;
        };
        
        if (req_build==0) goto pack_error;
        
        req->smpRequestBinds = &VBList[0];

        return (errOk);

pack_error:
        return (errBad);
}

static  SmpStatusType   myUpCall (SmpIdType smp, SmpRequestPtrType req)
{
        smp = smp;
        req = req;
        printf ("Upcall:\n");
        return (errOk);
}

extern int getIP(char *devName, int *pIp);

int trapCommand (int argc, char **argv)
{
        int                     s;
        int                     salen;
        int                     result;
        struct  sockaddr        salocal;
        struct  sockaddr        saremote;
        struct  sockaddr_in     *sin;
        struct  servent         *svp;

        u_long                  lhost;
        u_short                 lport;

        CByteType               buf [ cmdBufferSize ];
        CBytePtrType            bp;
        SmpIdType               smp;
        SmpSocketType           udp;
        ApsIdType               communityId;
        CCharPtrType            *ap;
        CCharPtrType            cp;
        CBoolType               noerror;
        CUnslType               number;

        CCharPtrType            communityString;
        CCharPtrType            lhostString;
        CCharPtrType            lportString;
        CCharPtrType            lversionString;
        CCharPtrType            lEOIDString;
        int                     lversion;
        int                     i;
        SmpStatusType           status;
        SmpRequestType          req;
        SmpErrorType            error;
        
        communityString = (CCharPtrType) 0;
        lhostString = (CCharPtrType) 0;
        lportString = (CCharPtrType) 0;
        lversionString = (CCharPtrType) 0;
        lEOIDString = (CCharPtrType) 0;

        if (argc<5)
                return ((int) usage ((CCharPtrType) argv [ 0 ]));
        
        ap = (CCharPtrType *) argv + 1;
        argc--;
        noerror = TRUE;
        while ((argc != 0) && (**ap == (CCharType) '-') && (noerror)) {
                cp = *ap;
                cp++;
                ap++;
                argc--;
                while ((*cp != (CCharType) 0) && (noerror)) {
                        switch (*cp) {

                        case 'c':  //comunity
                                argc--;
                                communityString = *ap++;
                                break;

                        case 'h':  //dst host ip
                                argc--;
                                lhostString = *ap++;
                                break;

                        case 'p':  //dst port
                                argc--;
                                lportString = *ap++;
                                break;

                        case 'v':  //SNMP version
                                argc--;
                                lversionString = *ap++;
                                break;

                        case 'e':  //enterprise OID
                                argc--;
                                lEOIDString = *ap++;
                                break;

                        default:
                                noerror = FALSE;
                                break;
                        }
                        cp++;
                }
        }

        if ((! noerror) || (argc <= 0)) {
                return ((int) usage ((CCharPtrType) argv [ 0 ]));
        }

        if (lhostString != (CCharPtrType) 0) {
                lhost = (u_long) hostAddress (lhostString);
                if (lhost == (u_long) -1) {
                        fprintf (stderr, "%s: Bad foreign host: %s\n",
                                argv [ 0 ], lhostString);
                        return (2);
                }
        }
        else {
                lhost = (u_long) 0;
        }

        if (lportString != (CCharPtrType) 0) {
                if (rdxDecodeAny (& number, lportString) < (CIntfType) 0) {
                        fprintf (stderr, "%s: Bad local port: %s\n",
                                argv [ 0 ], lportString);
                        return (2);
                }
                else {
                        lport = number;
                }
        }
        else 
                {
                lport = 162;  //default trap
                }

        if (communityString == (CCharPtrType) 0) {
                communityString = (CCharPtrType) "public";
        }
        
        if (lEOIDString == (CCharPtrType) 0) {
                lEOIDString = (CCharPtrType) defaultEOID;
        }
        
        cmdInit ();

        if (lversionString != (CCharPtrType) 0) {
                if ((lversionString[0]=='2')&&(lversionString[1]=='c')&&(lversionString[2]==0)) {
                        lversion = 1;
                } else if ((lversionString[0]=='1')&&(lversionString[1]==0)) {
                        lversion = 0;
                } else {
                        fprintf (stderr, "%s: Bad SNMP version number: %s\n",
                                argv [ 0 ], lversionString);
                        return (2);
                }; 
        } else {  //default snmpv1 trap
                lversion = 0;
        };

        s = socket (AF_INET, SOCK_DGRAM, 0);
        if (s < 0) {
                (void) perror ("socket");
                return (1);
        }

        result=-1;
        for (i=3000; ((i<3100)&&(result<0)); i++) {  //get a socket #
                sin = (struct sockaddr_in *) & salocal;
                bzero ((char *) sin, sizeof (salocal));
                sin->sin_family = AF_INET;
                sin->sin_addr.s_addr = 0;
                sin->sin_port = htons(i);

                result = bind (s, & salocal, sizeof (*sin));
                i++;
        };
        if (result<0) {
                (void) perror ("port");
                return (1);
        }

        communityId = apsNew ((ApsNameType) communityString,
                (ApsNameType) "trivial", (ApsGoodiesType) 0);

        sin = (struct sockaddr_in *) & saremote;
        salen = sizeof (saremote);
        bzero ((char *) sin, sizeof (saremote));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = lhost;
        sin->sin_port = htons(lport);

        //build req
        bzero ((char *) &req, sizeof (req));
        req.smpRequestCommunity = communityId;
	
        if (lversion==0) {  //v1 trap
                req.smpRequestCmd = smpCommandTrap;
                status = packTrapV1(argc, ap, &req, lEOIDString);
        } else {  //v2 trap
                status = req.smpRequestCmd = smpCommandTrapV2;
        };
        if (status==errBad) {
            printf("Error! SNMP Trap send failed\n");
            goto stop_trap;
        };

        udp = udpNew (s, sin->sin_addr.s_addr, sin->sin_port);
        smp = smpNew (udp, udpSend, myUpCall);
        status = smpRequest(smp, &req);
        smp = smpFree (smp);
        udp = udpFree (udp);

stop_trap:
        communityId = apsFree (communityId);

        return (close (s));
}


int     main (int argc, char **argv)
{

  /* signal handler */
  signal(SIGINT, done);
  signal(SIGTERM, done);
  signal(SIGHUP, done);

        exit (trapCommand (argc, argv));
}

