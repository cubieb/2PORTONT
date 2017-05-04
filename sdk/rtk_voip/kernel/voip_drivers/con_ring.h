#ifndef __CON_RING_H__
#define __CON_RING_H__

#include "voip_types.h"
#include "voip_timer.h"
#include "con_mux.h"

#define MRC_RING_CTRL_OFFSET	2

typedef struct {
	uint32 bMulti_rc;
	uint32 vMrc_step;
	uint32 vMrc_on1;
	uint32 vMrc_off1;
	uint32 vMrc_on2;
	uint32 vMrc_off2;
	uint32 vMrc_on3;
	uint32 vMrc_off3;
	uint32 vMrc_on4;
	uint32 vMrc_off4;
	uint32 vMrc_num;
	uint32 bMrc_off1_flag;
	uint32 bMrc_off2_flag;
	uint32 bMrc_off3_flag;
	uint32 bMrc_off4_flag;
	uint32 bMrc_ring_flag;
	uint32 bMrc_prev_ring_flag;
	uint32 vMrc_hook_state;
	uint32 set_ready_flag;	//0 false 1 true
	timetick_t vMrc_time_out;
} con_mrc_t;

extern void MultiRingCadenceEnable_con(const voip_con_t *p_con, unsigned int enable, 
		unsigned int on1, unsigned int off1, unsigned int on2, unsigned int off2,
		unsigned int on3, unsigned int off3, unsigned int on4, unsigned int off4);
extern unsigned int MultiRingCadenceEnableCheck_con(const voip_con_t *p_con);
extern void MultiRingStart_con(const voip_con_t *p_con);
extern void MultiRingStop_con(const voip_con_t *p_con);
extern uint32 MultiRingStatusCheck_con(const voip_con_t *p_con);
extern uint32 MultiRingOffCheck_con(const voip_con_t *p_con);
extern uint32 MultiRingReadyFlagCheck_con(uint32 cch);
extern uint32 MultiRingSetReadyFlag_con(uint32 cch, uint32 ready_flag);
extern uint32 MultiRingHookStateCheck_con(const voip_con_t *p_con);
extern uint32 MultiRingSetHookState_con(const voip_con_t *p_con, uint32 hook_state);
extern void MultiRingCadenceProcess_con(const voip_con_t *p_con);
extern void mrc_cch_init( voip_con_t voip_con[], int num );

#endif
