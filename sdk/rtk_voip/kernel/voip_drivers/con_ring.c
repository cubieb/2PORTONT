#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include "bspchip.h"
#endif

#include "rtk_voip.h"
#include "voip_types.h"
#include "snd_mux_slic.h"
#include "con_ring.h"

extern voip_con_t *get_voip_con_ptr( uint32 cch );
static con_mrc_t mrc[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = {0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0}};

void MultiRingCadenceEnable_con(const voip_con_t *p_con, unsigned int enable, 
	unsigned int on1, unsigned int off1, unsigned int on2, unsigned int off2,
	unsigned int on3, unsigned int off3, unsigned int on4, unsigned int off4)
{
	con_mrc_t *p_mrc = p_con ->mrc;

	p_mrc->bMulti_rc = enable;
	p_mrc->vMrc_on1 = on1;
	p_mrc->vMrc_off1 = off1;
	p_mrc->vMrc_on2 = on2;
	p_mrc->vMrc_off2 = off2;
	p_mrc->vMrc_on3 = on3;
	p_mrc->vMrc_off3 = off3;
	p_mrc->vMrc_on4 = on4;
	p_mrc->vMrc_off4 = off4;	
	p_mrc->set_ready_flag = 1;	//true : already set	
	
	if ( (on1!=0) && (off1!=0))
	{
		p_mrc->vMrc_num = 1;

		if ( (on2!=0) && (off2!=0))
		{
			p_mrc->vMrc_num = 2;

			if ( (on3!=0) && (off3!=0))
			{
				p_mrc->vMrc_num = 3;

				if ( (on4!=0) && (off4!=0))
				{
					p_mrc->vMrc_num = 4;
				}
			}
		}
	}
}

uint32 MultiRingCadenceEnableCheck_con(const voip_con_t *p_con)
{
	con_mrc_t *p_mrc = p_con ->mrc;

	return p_mrc->bMulti_rc;
}

void MultiRingStart_con(const voip_con_t *p_con)
{
	con_mrc_t *p_mrc = p_con ->mrc;
	voip_snd_t *p_snd = p_con ->snd_ptr;

	if (1 == p_mrc->bMulti_rc)
	{
		// check hook status if on-hook
		if ( 0 == p_snd ->fxs_ops ->SLIC_Get_Hook_Status(p_snd, 1) )
		{
			p_mrc->bMrc_ring_flag = 1;
			p_mrc->vMrc_hook_state = 1;
			PRINT_MSG("MultiRingStart, ch%d\n", p_con ->cch);
		}
		else
			PRINT_R("%s: warning, SLIC is off-hook, can NOT Ringing, ch%d\n", __FUNCTION__, p_con ->cch);
	}
}

void MultiRingStop_con(const voip_con_t *p_con)
{
	con_mrc_t *p_mrc = p_con ->mrc;

	if (1 == p_mrc->bMulti_rc)
	{
		p_mrc->bMrc_ring_flag = 0;
		p_mrc->bMrc_off1_flag = 0;
		p_mrc->bMrc_off2_flag = 0;
		p_mrc->bMrc_off3_flag = 0;
		p_mrc->bMrc_off4_flag = 0;
		p_mrc->bMrc_prev_ring_flag = 0;
		p_mrc->vMrc_step = 0;
		p_mrc->vMrc_hook_state = 0;
	}
	PRINT_MSG("MultiRingStop, ch%d\n", p_con ->cch);
}

uint32 MultiRingStatusCheck_con(const voip_con_t *p_con)
{
	con_mrc_t *p_mrc = p_con ->mrc;

	return 	p_mrc->bMrc_ring_flag;
}
uint32 MultiRingReadyFlagCheck_con(uint32 cch)
{
	const voip_con_t * const p_con = get_voip_con_ptr(cch);
        con_mrc_t *p_mrc = p_con ->mrc;
        return  p_mrc->set_ready_flag;
}
uint32 MultiRingSetReadyFlag_con(uint32 cch, uint32 ready_flag)
{
	const voip_con_t * const p_con = get_voip_con_ptr(cch);
        con_mrc_t *p_mrc = p_con ->mrc;
        p_mrc->set_ready_flag = ready_flag;
}
uint32 MultiRingHookStateCheck_con(const voip_con_t *p_con)
{
	con_mrc_t *p_mrc = p_con ->mrc;
        return  p_mrc->vMrc_hook_state;
}
uint32 MultiRingSetHookState_con(const voip_con_t *p_con, uint32 hook_state)
{
	con_mrc_t *p_mrc = p_con ->mrc;
	p_mrc->vMrc_hook_state = hook_state;
}

uint32 MultiRingOffCheck_con(const voip_con_t *p_con)
{
	int max, max2;
	con_mrc_t *p_mrc = p_con ->mrc;

	if (p_mrc->vMrc_num == 1)
	{
		return 	p_mrc->bMrc_off1_flag;
		//PRINT_G("off-1 is max\n");
	}
	else if (p_mrc->vMrc_num == 2)
	{
		if (p_mrc->vMrc_off1 >=  p_mrc->vMrc_off2)
		{
			//PRINT_G("off-1 is max\n");
			return p_mrc->bMrc_off1_flag;
		}
		else
		{
			//PRINT_G("off-2 is max\n");
			return p_mrc->bMrc_off2_flag;
		}
	}
	else if (p_mrc->vMrc_num == 3)
	{
		max = p_mrc->vMrc_off1;

		if (p_mrc->vMrc_off2 >= max)		// off2 > off1
		{
			if (p_mrc->vMrc_off2 >=  p_mrc->vMrc_off3)	// off2 > off3
			{
				//PRINT_G("off-2 is max\n");
				return p_mrc->bMrc_off2_flag;		// off2 is max.
			}
		}

		if (p_mrc->vMrc_off3 >= max)	// off3 > off1
		{
			if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off2)	// off3 > off2
			{
				//PRINT_G("off-3 is max\n");
				return p_mrc->bMrc_off3_flag;		// off3 is max.
			}
		}

		// off1 is max.
		//PRINT_G("off-1 is max\n");
		return p_mrc->bMrc_off1_flag;
	}
	else if (p_mrc->vMrc_num == 4)
	{
		max = p_mrc->vMrc_off1;

		if (p_mrc->vMrc_off2 >= max)		// off2 > off1
		{
			if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off2)	// off3 > off2
			{
				if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off4)	// off3 > (off1, off2, off4)
				{
					//PRINT_G("off-3 is max\n");
					return p_mrc->bMrc_off3_flag;			// off3 is max.
				}
				else						// off4 > (off1, off3, off2)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;			// off4 is max.
				}
			}
			else						// off3 < off2
			{
				if (p_mrc->vMrc_off2 >=  p_mrc->vMrc_off4)	// off2 > (off1, off3, off4)
				{
					//PRINT_G("off-2 is max\n");
					return p_mrc->bMrc_off2_flag;			// off2 is max.
				}
				else						// off4 > (off1, off2, off3)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;		// off4 is max.
				}
			}
		}
		else if (p_mrc->vMrc_off3 >= max)	// off3 > off1
		{
			if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off2)	// off3 > off2
			{
				if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off4)	// off3 > (off1, off2, off4)
				{
					//PRINT_G("off-3 is max\n");
					return p_mrc->bMrc_off3_flag;			// off3 is max.
				}
				else						// off4 > (off3, off2, off1)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;			// off4 is max.
				}
			}
			else						// off3 < off2
			{
				if (p_mrc->vMrc_off2 >=  p_mrc->vMrc_off4)	// off2 > (0ff3, off4, off1)
				{
					//PRINT_G("off-2 is max\n");
					return p_mrc->bMrc_off2_flag;			// off2 is max.
				}
				else						// off4 > (0ff2, off3, off1)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;			// off4 is max.
				}
			}
		}
		else if (p_mrc->vMrc_off4 >= max)	// off4 > off1
		{
			if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off2)	// off3 > off2
			{
				if (p_mrc->vMrc_off3 >=  p_mrc->vMrc_off4)	// off3 > (off4, off1, off2)
				{
					//PRINT_G("off-3 is max\n");
					return p_mrc->bMrc_off3_flag;			// off3 is max.
				}
				else						// off4 > (off3, off2, off1)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;			// off4 is max.
				}
			}
			else						// off3 < off2
			{
				if (p_mrc->vMrc_off2 >=  p_mrc->vMrc_off4)	// off2 > (off4, off3, off1)
				{
					//PRINT_G("off-2 is max\n");
					return p_mrc->bMrc_off2_flag;			// off2 is max.
				}
				else						// off4 > (off2, off3, off1)
				{
					//PRINT_G("off-4 is max\n");
					return p_mrc->bMrc_off4_flag;			// off4 is max.
				}
			}
		}
		else					// off1 is max.
		{
			//PRINT_G("off-1 is max\n");
			return p_mrc->bMrc_off1_flag;
		}
	}
}

void MultiRingCadenceProcess_con(const voip_con_t *p_con)
{

	con_mrc_t *p_mrc = p_con ->mrc;
	voip_snd_t *p_snd = p_con ->snd_ptr;

	if ( 0 == p_mrc->bMulti_rc){
		return;		
	}
	if ( (0 == p_mrc->bMrc_ring_flag) && ( p_mrc->bMrc_prev_ring_flag != 0) )
	{
		p_snd ->fxs_ops ->FXS_Ring(p_snd, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
		//FXS_Ring(cch, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
		p_mrc->vMrc_step = 0; 		// Go to Idel step
		//p_mrc->bMrc_prev_ring_flag = 0;	// ring off state
		PRINT_MSG("Stop Ring, ch%d\n", p_con ->cch);
	}
		
	
	if ( (1 == p_mrc->bMrc_ring_flag) && ( p_mrc->bMrc_prev_ring_flag != 1) )
	{
		p_mrc->vMrc_step = 1; 		// Go to Start Ring step
		//p_mrc->bMrc_prev_ring_flag = 1;	// ring on state
		PRINT_MSG("Start Ring, ch%d\n", p_con ->cch);
	}
	
	p_mrc->bMrc_prev_ring_flag = p_mrc->bMrc_ring_flag;
	
#if 0	// debug
	if (p_mrc->vMrc_step != 0)
		printk("%d, %d\n", p_mrc->vMrc_step, p_con ->cch);
#endif
	
	switch(p_mrc->vMrc_step)
	{
		case 0:	// Idel
			break;
		case 1:	// Start Ring
			p_snd ->fxs_ops ->FXS_Ring(p_snd, 1 + MRC_RING_CTRL_OFFSET);			// Start Ring
			//FXS_Ring(cch, 1 + MRC_RING_CTRL_OFFSET);
			p_mrc->bMrc_off1_flag = 0;
			p_mrc->bMrc_off2_flag = 0;
			p_mrc->bMrc_off3_flag = 0;
			p_mrc->bMrc_off4_flag = 0;
			p_mrc->vMrc_step++;
			p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
			break;
		case 2:	// Cad on1
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				p_snd ->fxs_ops ->FXS_Ring(p_snd, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
				//FXS_Ring(cch, 0 + MRC_RING_CTRL_OFFSET);
				p_mrc->bMrc_off1_flag = 1;
				p_mrc->vMrc_step++;
				p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off1;
			}
			break;
		case 3: // Cad off1
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				p_snd ->fxs_ops ->FXS_Ring(p_snd, 1 + MRC_RING_CTRL_OFFSET);			// Start Ring
				//FXS_Ring(cch, 1 + MRC_RING_CTRL_OFFSET);
				p_mrc->bMrc_off1_flag = 0;
				if (p_mrc->vMrc_on2 != 0)
				{
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on2;
				}
				else
				{
					p_mrc->vMrc_step--;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
				}
			}
			break;
		case 4: // Cad on2
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				if (p_mrc->vMrc_off2 != 0)
				{
					p_snd ->fxs_ops ->FXS_Ring(p_snd, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
					p_mrc->bMrc_off2_flag = 1;
					//FXS_Ring(cch, 0 + MRC_RING_CTRL_OFFSET);
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off2;
				}
				else
				{
					PRINT_R("Should not run here, step%d\n", p_mrc->vMrc_step);
					p_mrc->vMrc_step-=2;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
				}
			}
			break;
		case 5: // Cad off2
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				p_snd ->fxs_ops ->FXS_Ring(p_snd, 1 + MRC_RING_CTRL_OFFSET);			// Start Ring
				//FXS_Ring(cch, 1 + MRC_RING_CTRL_OFFSET);
				p_mrc->bMrc_off2_flag = 0;
				if (p_mrc->vMrc_on3 != 0)
				{
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on3;
				}
				else
				{
					p_mrc->vMrc_step = 2;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
				}
			}
			break;
		case 6: // Cad on3
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				if (p_mrc->vMrc_off3 != 0)
				{
					p_snd ->fxs_ops ->FXS_Ring(p_snd, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
					p_mrc->bMrc_off3_flag = 1;
					//FXS_Ring(cch, 0 + MRC_RING_CTRL_OFFSET);
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off3;
				}
				else
				{
					PRINT_R("Should not run here, step%d\n", p_mrc->vMrc_step);
					p_mrc->vMrc_step = 2;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off3;
				}
			}
			break;
		case 7: // Cad off3
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				p_snd ->fxs_ops ->FXS_Ring(p_snd, 1 + MRC_RING_CTRL_OFFSET);			// start Ring
				//FXS_Ring(cch, 1 + MRC_RING_CTRL_OFFSET);
				p_mrc->bMrc_off3_flag = 0;
				if (p_mrc->vMrc_on4 != 0)
				{
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on4;
				}
				else
				{
					p_mrc->vMrc_step = 2;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
				}
			}
			break;
		case 8: // Cad on4
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				if (p_mrc->vMrc_off4 != 0)
				{
					p_snd ->fxs_ops ->FXS_Ring(p_snd, 0 + MRC_RING_CTRL_OFFSET);			// Stop Ring
					p_mrc->bMrc_off4_flag = 1;
					//FXS_Ring(cch, 0 + MRC_RING_CTRL_OFFSET);
					p_mrc->vMrc_step++;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off4;
				}
				else
				{
					PRINT_R("Should not run here, step%d\n", p_mrc->vMrc_step);
					p_mrc->vMrc_step = 2;
					p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_off4;
				}
			}
			break;
		case 9: // Cad off4
			if (timetick_after(timetick, p_mrc->vMrc_time_out))
			{
				p_snd ->fxs_ops ->FXS_Ring(p_snd, 1 + MRC_RING_CTRL_OFFSET);			// start Ring
				//FXS_Ring(cch, 1 + MRC_RING_CTRL_OFFSET);
				p_mrc->bMrc_off4_flag = 0;
				p_mrc->vMrc_step = 2;
				p_mrc->vMrc_time_out = timetick + p_mrc->vMrc_on1;
                
			}
			break;
		default:
			PRINT_R("error mrc step = %d, ch%d\n", p_mrc->vMrc_step, p_con ->cch);
			break;
	}
			
}

void mrc_cch_init( voip_con_t voip_con[], int num )
{
	int i;
	voip_con_t *p_con;
	con_mrc_t *p_mrc;

	//printk("mrc_cch_init:\n");

	for( i = 0; i < num; i ++ )
	{
		//printk("sizeof(mrc)/sizeof(con_mrc_t) = %d\n", sizeof(mrc)/sizeof(con_mrc_t));

		if (i > (sizeof(mrc)/sizeof(con_mrc_t)-1))
			printk("error size in %s, line%d\n", __FUNCTION__, __LINE__);

		p_con = &voip_con[ i ];
		p_con ->mrc = (void*)(&mrc[i]);
		p_mrc = (con_mrc_t*) p_con ->mrc;

		//printk("cch = %d\n", p_con->cch);
		//printk("p_con = %p\n", p_con);
		//printk("p_mrc = %p\n", p_mrc);

		p_mrc->bMulti_rc = 0;
		p_mrc->vMrc_step = 0;
		p_mrc->vMrc_on1 = 2000;
		p_mrc->vMrc_off1 = 4000;
		p_mrc->vMrc_on2 = 0;
		p_mrc->vMrc_off2 = 0;
		p_mrc->vMrc_num = 0;
		p_mrc->bMrc_off1_flag = 0;
		p_mrc->bMrc_off2_flag = 0;
		p_mrc->bMrc_off3_flag = 0;
		p_mrc->bMrc_off4_flag = 0;
		p_mrc->bMrc_ring_flag = 0;
		p_mrc->bMrc_prev_ring_flag = 0;
		p_mrc->vMrc_hook_state = 0;
		p_mrc->set_ready_flag = 0;		
		p_mrc->vMrc_time_out = 0;
	}
}

