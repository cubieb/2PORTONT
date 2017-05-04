/*
 *      Headler file of Realtek STP
 *
 *      $Id: rtl_alias.h,v 1.6 2012/05/15 06:19:43 kaohj Exp $
 */

#ifndef _RTL_ALIAS_H
#define _RTL_ALIAS_H
#include "rtl_types.h"

#define CMD_CMP     1
#define CMD_NCMP    2
#define RTL_DEV_NAME_NUM(name,num)	name#num


///////////////////////////////////////////////////////////////////
#ifndef CONFIG_RTL_ALIASNAME

#define ALIASNAME_VC   "vc" 
#define ALIASNAME_BR   "br" 
#define ALIASNAME_NAS  "nas"
#define ALIASNAME_ETH  "eth"
#define ALIASNAME_MWNAS  "nas0_"
#define ALIASNAME_ELAN_PREFIX  "eth0."
#define ALIASNAME_WLAN  "wlan"
#define ALIASNAME_PPP  "ppp"
#define ORIGINATE_NUM 2
//#define ALIASNAME_ETH0  RTL_DEV_NAME_NUM(ALIASNAME_ETH,0)
//#define ALIASNAME_BR0 RTL_DEV_NAME_NUM(ALIASNAME_BR,0)
//#define ALIASNAME_WLAN0_VAP RTL_DEV_NAME_NUM(ALIASNAME_WLAN,0-vap)
//#define ALIASNAME_NAS0 RTL_DEV_NAME_NUM(ALIASNAME_NAS,0)

#else

#define ALIASNAME_VC   CONFIG_ALIASNAME_VC//"vc" 
#define ALIASNAME_BR   CONFIG_ALIASNAME_BR//"br" 
#define ALIASNAME_NAS  CONFIG_ALIASNAME_NAS//"nas"
#define ALIASNAME_ETH  CONFIG_ALIASNAME_ETH//"eth"
#define ALIASNAME_MWNAS  CONFIG_ALIASNAME_MWNAS//"nas0_"
#define ALIASNAME_ELAN_PREFIX  CONFIG_ALIASNAME_ELAN_PREFIX//"eth0."
#define ALIASNAME_WLAN  CONFIG_ALIASNAME_WLAN//"wlan"
#define ALIASNAME_PPP  CONFIG_ALIASNAME_PPP//"ppp"
#define ORIGINATE_NUM CONFIG_ORIGINATE_NUM
/*
#define ALIASNAME_VC   "vc" 
#define ALIASNAME_BR   "br" 
#define ALIASNAME_NAS  "nas"
#define ALIASNAME_ETH   "eth"
#define ALIASNAME_WLAN  "wlan"
#define ALIASNAME_PPP  "ppp"
*/


#endif



///////////////////////////////////////////////////////////////////

#define ALIASNAME_NAS0  RTL_DEV_NAME_NUM(ALIASNAME_NAS,0)//"nas0"
//#define ALIASNAME_NAS0  CONFIG_ALIASNAME_NAS//"nas0"

#define ALIASNAME_BR0   RTL_DEV_NAME_NUM(ALIASNAME_BR,0)//"br0"  
//#define ALIASNAME_BR0   CONFIG_ALIASNAME_BR//"br0"  

#define ALIASNAME_BR1   RTL_DEV_NAME_NUM(ALIASNAME_BR,1)//"br1"  
#define ALIASNAME_VC0   RTL_DEV_NAME_NUM(ALIASNAME_VC,0)//"vc0" 
//#define ALIASNAME_VC0   CONFIG_ALIASNAME_VC//"vc0" 



#define ALIASNAME_ETH0  RTL_DEV_NAME_NUM(ALIASNAME_ETH,0)//eth0
//#define ALIASNAME_ELAN_PREFIX   RTL_DEV_NAME_NUM(ALIASNAME_ETH0,.)//eth0.
//#define ALIASNAME_ETH0  CONFIG_ALIASNAME_ETH//eth0

#define ALIASNAME_ETH1  RTL_DEV_NAME_NUM(ALIASNAME_ETH,1)//eth1

#define ALIASNAME_WLAN0  RTL_DEV_NAME_NUM(ALIASNAME_WLAN,0)//"wlan0"
//#define ALIASNAME_WLAN0 CONFIG_ALIASNAME_WLAN
#define ALIASNAME_VAP   "-vap" 
#define ALIASNAME_WLAN0_VAP  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0,-vap)//"wlan0-vap"
#define ALIASNAME_WLAN0_MSH0  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0,-msh0)//"wlan0-msh0"
#define ALIASNAME_WLAN0_VAP0  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,0)//"wlan0-vap0"
#define ALIASNAME_WLAN0_VAP1  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,1)//"wlan0-vap1"
#define ALIASNAME_WLAN0_VAP2  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,2)//"wlan0-vap2"
#define ALIASNAME_WLAN0_VAP3  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,3)//"wlan0-vap3"







static inline void TOKEN_NUM(char *inputStr,int *outInt)
{
    *outInt=inputStr[strlen(inputStr)-1]-'0';
   // printk("port :%d\n",*outInt);
}


static inline int32 alias_name_are_eq(char *orig_name,char *alias1,char *alias2)
{

int32 rtnValue=1;              
rtnValue=strncmp(orig_name,alias1,strlen(alias1));

if(rtnValue)
    rtnValue=strncmp(orig_name,alias2,strlen(alias2));    
       
#if 0//def CONFIG_RTL_ALIASNAME_DEBUG               
    if(rtnValue!=strncmp(orig_name,cmp_name,strlen(cmp_name)))
    {
        printk("====CONFIG_RTL_ALIASNAME_DEBUG====\n\ncmp_name : %s\norig_name :%s\n",cmp_name,orig_name);
                printk("rtnValue %d\n",rtnValue); 
    }
#endif    
    return !rtnValue; 
                         
}

static inline int32 alias_name_is_eq(int cmd,char *orig_name,char *alias_name )
{
    if(cmd==CMD_CMP)
        return !strcmp(orig_name,alias_name);    
    else
        return !strncmp(orig_name,alias_name,strlen(alias_name));                             
}


#endif // _RTL_ALIAS_H
