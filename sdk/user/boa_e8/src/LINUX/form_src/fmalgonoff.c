/*
 * =====================================================================================
 *
 *       Filename:  fmalgonoff.c
 *
 *    Description:  control the on-off of ALG by the web
 *
 *        Version:  1.0
 *        Created:  08/16/07 15:54:05    
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ramen.Shen (Mr), ramen_shen@realsil.com.cn
 *        Company:  REALSIL Microelectronics Inc
 *
 * =====================================================================================
 */

/*-- System inlcude files --*/
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>
#include <linux/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
enum algtype{
#ifdef CONFIG_NF_CONNTRACK_FTP
    ALG_TYPE_FTP=0,
#endif
#ifdef CONFIG_NF_CONNTRACK_H323	
    ALG_TYPE_H323,
#endif
#ifdef CONFIG_NF_CONNTRACK_IRC
    ALG_TYPE_IRC,
 #endif
 #ifdef CONFIG_NF_CONNTRACK_RTSP
    ALG_TYPE_RTSP,
 #endif
 #ifdef CONFIG_NF_CONNTRACK_QUAKE3
    ALG_TYPE_QUAKE3,
 #endif
 #ifdef CONFIG_NF_CONNTRACK_CUSEEME
    ALG_TYPE_CUSEEME,
 #endif 
 #ifdef CONFIG_NF_CONNTRACK_L2TP
    ALG_TYPE_L2TP,
 #endif
 #ifdef CONFIG_NF_CONNTRACK_IPSEC
    ALG_TYPE_IPSEC,
 #endif
 #ifdef CONFIG_NF_CONNTRACK_SIP
    ALG_TYPE_SIP,
#endif
 #ifdef CONFIG_NF_CONNTRACK_PPTP
    ALG_TYPE_PPTP,  
#endif
    ALG_TYPE_MAX
};
struct {
	unsigned char id;
	unsigned int mibalgid;
	char* name;
}algTypeName[]={
#ifdef CONFIG_NF_CONNTRACK_FTP
{ALG_TYPE_FTP,MIB_IP_ALG_FTP,"ftp"},
#endif
#ifdef CONFIG_NF_CONNTRACK_H323
{ALG_TYPE_H323,MIB_IP_ALG_H323,"h323"},
#endif
#ifdef CONFIG_NF_CONNTRACK_IRC
{ALG_TYPE_IRC,MIB_IP_ALG_IRC,"irc"},
#endif
#ifdef CONFIG_NF_CONNTRACK_RTSP
{ALG_TYPE_RTSP,MIB_IP_ALG_RTSP,"rtsp"},
#endif
#ifdef CONFIG_NF_CONNTRACK_QUAKE3
{ALG_TYPE_QUAKE3,MIB_IP_ALG_QUAKE3,"quake3"},
#endif
#ifdef CONFIG_NF_CONNTRACK_CUSEEME
{ALG_TYPE_CUSEEME,MIB_IP_ALG_CUSEEME,"cuseeme"},
#endif
#ifdef CONFIG_NF_CONNTRACK_L2TP
{ALG_TYPE_L2TP,MIB_IP_ALG_L2TP,"l2tp"},
#endif
#ifdef CONFIG_NF_CONNTRACK_IPSEC
{ALG_TYPE_IPSEC,MIB_IP_ALG_IPSEC,"ipsec"},
#endif
#ifdef CONFIG_NF_CONNTRACK_SIP
{ALG_TYPE_SIP,MIB_IP_ALG_SIP,"sip"},
#endif
 #ifdef CONFIG_NF_CONNTRACK_PPTP
{ALG_TYPE_PPTP, MIB_IP_ALG_PPTP,"pptp"}, 
 #endif
{ALG_TYPE_MAX,0,NULL}
};
void formALGOnOff(request * wp, char *path, char *query)
{

	char	*str, *submitUrl;
	char tmpBuf[100];	
	char cmdstr[128]={0};
	//char cmdbuf[8]={0};
	unsigned char  algenable=0;
	str = boaGetVar(wp, "apply", "");
	if (str[0]) {
	int i=0;	
	for(i=0;i<ALG_TYPE_MAX;i++)
		{

		char algTypeStr[32]={0};
		snprintf(algTypeStr,sizeof(algTypeStr),"%s_algonoff",algTypeName[i].name);
		str=boaGetVar(wp, algTypeStr, "");
		if(str[0])
			{
			algenable=str[0]-'0';
			  snprintf(cmdstr,sizeof(cmdstr),"/proc/%s_algonoff",algTypeName[i].name);
			 FILE *fp=fopen(cmdstr,"w");
			 if(fp)
			 	{
			 	fwrite(str,sizeof(char),1,fp);
				fclose(fp);
			 	}
			if(!mib_set(algTypeName[i].mibalgid,(void*)&algenable))
				printf("%s mib set %d error!\n",__FUNCTION__,algTypeName[i].mibalgid);
			}		
		}			
 	}	
	
	// Mason Yu. alg_onoff_20101023
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	setupAlgOnOff();
		
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;	
}
void GetAlgTypes(request * wp)
{
int i = 0;
for(i=0;i<ALG_TYPE_MAX;i++)
{
	boaWrite(wp,"<tr>\n <td ><font size=2>%s</td>\n"
	"  <td ><input type=\"radio\" name=%s_algonoff value=1 >∆Ù”√ </td>\n"
	"     <td ><input type=\"radio\" name=%s_algonoff value=0 >πÿ±’ </td>\n " 	
	"</tr>\n",algTypeName[i].name,algTypeName[i].name,algTypeName[i].name);
}
return;
}
void CreatejsAlgTypeStatus(request * wp)
{
unsigned char  value=0;
int i =0;
for(i=0;i<ALG_TYPE_MAX;i++)
{

	mib_get(algTypeName[i].mibalgid,&value);	
	boaWrite(wp,"document.algof.%s_algonoff[%d].checked=true;\n",algTypeName[i].name,!value&0x01);	
}	
return;

}
void initAlgOnOff(request * wp)
{
	return;   
}
#endif

