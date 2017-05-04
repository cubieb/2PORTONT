#ifndef _APOLLO_RAW_DOT1X_H_
#define _APOLLO_RAW_DOT1X_H_

#include <dal/apollo/raw/apollo_raw.h>


typedef enum apollo_raw_dot1x_unAuthAct_e
{
    RAW_DOT1X_UNAUTH_DROP = 0,
    RAW_DOT1X_UNAUTH_TRAP,
    RAW_DOT1X_UNAUTH_GVLAN,
    RAW_DOT1X_UNAUTH_END
}apollo_raw_dot1x_unAuthAct_t;



typedef enum apollo_raw_dot1x_opDir_e
{
	RAW_DOT1X_DIRECT_BOTH = 0,
    RAW_DOT1X_DIRECT_IN,
    RAW_DOT1X_DIRECT_END
}apollo_raw_dot1x_opDir_t;


typedef enum apollo_raw_dot1x_authState_e
{
    RAW_DOT1X_UNAUTHORIZED = 0,
	RAW_DOT1X_AUTHORIZED ,
    RAW_DOT1X_AUTH_END
}apollo_raw_dot1x_authState_t;




extern int32 apollo_raw_dot1x_guestVidx_set(uint32 index);
extern int32 apollo_raw_dot1x_guestVidx_get(uint32 *pIndex);
extern int32 apollo_raw_dot1x_trapPri_set(uint32 pri);
extern int32 apollo_raw_dot1x_trapPri_get(uint32 *pri);
extern int32 apollo_raw_dot1x_guestVlanOpdir_set(uint32 Enabled);
extern int32 apollo_raw_dot1x_guestVlanOpdir_get(uint32 *pEnabled);
extern int32 apollo_raw_dot1x_macBaseVlanOpdir_set(apollo_raw_dot1x_opDir_t opdir);
extern int32 apollo_raw_dot1x_macBaseVlanOpdir_get(apollo_raw_dot1x_opDir_t *pOpdir);
extern int32 apollo_raw_dot1x_macBasedEnable_set(uint32 port, uint32 enable);
extern int32 apollo_raw_dot1x_macBasedEnable_get(uint32 port, uint32 *pEnable);
extern int32 apollo_raw_dot1x_portBasedEnable_get(uint32 port, uint32 *pEnable);
extern int32 apollo_raw_dot1x_portBasedEnable_set(uint32 port, uint32 enable);
extern int32 apollo_raw_dot1x_portAuthState_set(uint32 port, apollo_raw_dot1x_authState_t auth);
extern int32 apollo_raw_dot1x_portAuthState_get(uint32 port, apollo_raw_dot1x_authState_t *pAuth);
extern int32 apollo_raw_dot1x_portBaseVlanOpdir_get(uint32 port, apollo_raw_dot1x_opDir_t *pOpdir);
extern int32 apollo_raw_dot1x_portBaseVlanOpdir_set(uint32 port, apollo_raw_dot1x_opDir_t opdir);
extern int32 apollo_raw_dot1x_unauthAct_get(uint32 port, apollo_raw_dot1x_unAuthAct_t *pProc);
extern int32 apollo_raw_dot1x_unauthAct_set(uint32 port, apollo_raw_dot1x_unAuthAct_t proc);


#endif /*_APOLLO_RAW_DOT1X_H_*/

