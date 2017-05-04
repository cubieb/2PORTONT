#ifndef _APOLLO_RAW_OAM_H_
#define _APOLLO_RAW_OAM_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/

typedef enum apollo_raw_oam_parserAct_e
{
    RAW_OAM_PARSER_FORWARD  = 0,
    RAW_OAM_PARSER_LOOPBACK,
    RAW_OAM_PARSER_DISCARD,   
    RAW_OAM_PARSER_ACTION_END    
} apollo_raw_oam_parserAct_t;

typedef enum apollo_raw_oam_muxAct_e
{
    RAW_OAM_MUX_FORWARD  = 0,
    RAW_OAM_MUX_DISCARD,  
    RAW_OAM_MUX_CPUONLY,    
    RAW_OAM_MUX_ACTION_END    
} apollo_raw_oam_muxAct_t;


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/



extern int32 apollo_raw_oam_parser_set(rtk_port_t port,  apollo_raw_oam_parserAct_t action);
extern int32 apollo_raw_oam_parser_get(rtk_port_t port,  apollo_raw_oam_parserAct_t *pAction);
extern int32 apollo_raw_oam_multiplexer_set(rtk_port_t port,  apollo_raw_oam_muxAct_t action);
extern int32 apollo_raw_oam_multiplexer_get(rtk_port_t port,  apollo_raw_oam_muxAct_t *pAction);
extern int32 apollo_raw_oam_priority_set(uint32 priority);
extern int32 apollo_raw_oam_priority_get(uint32 *pPriority);
extern int32 apollo_raw_oam_enable_set(rtk_enable_t state);
extern int32 apollo_raw_oam_enable_get(rtk_enable_t *pState);

#endif /*#ifndef _APOLLO_RAW_OAM_H_*/
