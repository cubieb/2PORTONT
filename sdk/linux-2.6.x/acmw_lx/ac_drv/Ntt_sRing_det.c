#include <linux/kernel.h>
#include "rtk_voip.h"
#include "Ntt_sRing_det.h"

int ntt_sRing_on_pass[MAX_VOIP_CH_NUM] = {0};
int ntt_sRing_off_pass[MAX_VOIP_CH_NUM] = {0};
int ntt_sRing_on_cnt = 2;
int ntt_sRing_off_cnt = 1;

static int ntt_sRing_off_cont_low_lvl = 25 ;
static int ntt_sRing_off_cont_hi_lvl = 60 ;
static int ntt_sRing_on_cont_low_lvl = 25 ;
static int ntt_sRing_on_cont_hi_lvl = 60 ;

/*
   setting the NTT short ring OFF level  unit = 10 ms,
   EX: set_NTT_sRing_off_lvl(25, 60)
   600ms >  valid sRing off time >250ms
 */
void set_NTT_sRing_off_lvl(int low_lvl, int hi_lvl)
{
	ntt_sRing_off_cont_low_lvl = low_lvl;
	ntt_sRing_off_cont_hi_lvl = hi_lvl;
}

/*
   setting the NTT short ring ON level  unit = 10 ms,
   EX: set_NTT_sRing_on_lvl(25, 60)
   600ms >  valid sRing on time >250ms
 */
void set_NTT_sRing_on_lvl(int low_lvl, int hi_lvl)
{
	ntt_sRing_on_cont_low_lvl = low_lvl;
	ntt_sRing_on_cont_hi_lvl = hi_lvl;
}

/*
   setting the NTT short ring ON-OFF times:
   EX: set_NTT_onoff_times( 2, 1)
   
   sRing_on -> sRing_off -> sRing_on ; in the second ring_on 
   if enable NTT caller id det. DAA going to will OFF-HOOK, to recv the japan caller id.
 */
void set_NTT_onoff_times(int ring_on_cnt, int ring_off_cnt )
{
	ntt_sRing_on_cnt = ring_on_cnt;
	ntt_sRing_off_cnt = ring_off_cnt;
}


static int now_stat[MAX_VOIP_CH_NUM];
static int pre1_stat[MAX_VOIP_CH_NUM];
static int pre2_stat[MAX_VOIP_CH_NUM];
static int pre3_stat[MAX_VOIP_CH_NUM];

/*     oldest<- pre3_stat    pre2_stat	     pre1_stat     now_stat   -> newest */
/*  time line   0sec  		1sec   		2sec        3sec        sec is a unit time*/
/*   we treat  {RING_ON,     RING_OFF,        RING_ON     don't care}  is false alarm  */
/*   we treat  {RING_ON,     RING_OFF,        don't care   RING_ON  }  is false alarm  */
/*   we treat  {RING_OFF,    RING_ON ,        RING_OFF    don,t care}  is false alarm  */
/*   we treat  {RING_OFF,    RING_ON ,        don,t care    RING_OFF}  is false alarm  */

static int ntt_sRing_on_cont[MAX_VOIP_CH_NUM];
static int ntt_sRing_off_cont[MAX_VOIP_CH_NUM];

void NTT_sRing_det(int chid, int ring_stat)
{

	now_stat[chid] = ring_stat;

	if ( now_stat[chid] ==  pre3_stat[chid] )
	{
		pre1_stat[chid] = now_stat[chid];
		pre2_stat[chid] = now_stat[chid];
	}
	else if ( pre1_stat[chid] == pre3_stat[chid]  )
	{
		pre2_stat[chid] = pre1_stat[chid];
	}

	if (pre2_stat[chid] ==NTT_sRing_RINGON)
	{
		//printk("%d:",pre2_stat[chid]);
		if(ntt_sRing_on_cont[chid] < 32000 ) /* avoid over flow */
			ntt_sRing_on_cont[chid]++;

		if( (ntt_sRing_off_cont[chid] > ntt_sRing_off_cont_low_lvl ) && (ntt_sRing_off_cont[chid] < ntt_sRing_off_cont_hi_lvl)  )
		{
			ntt_sRing_off_pass[chid]++;
			//PRINT_MSG("+f=%d\n",ntt_sRing_off_cont[chid]);

		}
		else if (ntt_sRing_off_cont[chid] > 0)
		{
			ntt_sRing_off_pass[chid] =0;
			//PRINT_MSG("-f=%d\n",ntt_sRing_off_cont[chid]);
		}
		ntt_sRing_off_cont[chid] = 0;

	}
	else
	{
		//printk("-%d:",pre2_stat[chid]);
		if(ntt_sRing_off_cont[chid] < 32000 ) /* avoid over flow */
			ntt_sRing_off_cont[chid]++;

		if( (ntt_sRing_on_cont[chid] > 25 ) && (ntt_sRing_on_cont[chid] < 60)  )
		{
			ntt_sRing_on_pass[chid]++;
			//PRINT_MSG("+n%d\n",ntt_sRing_on_cont[chid]);

		}
		else if (ntt_sRing_on_cont[chid] > 0)
		{
			ntt_sRing_on_pass[chid] =0;
			//PRINT_MSG("-n=%d\n",ntt_sRing_on_cont[chid]);
		}
		ntt_sRing_on_cont[chid] = 0;

	}
	pre3_stat[chid] = pre2_stat[chid];
	pre2_stat[chid] = pre1_stat[chid];
	pre1_stat[chid] = now_stat[chid];
	
}



