#ifndef _NTT_SRING_DET
#define _NTT_SRING_DET

#define NTT_sRing_RINGON 1
#define NTT_sRing_RINGOFF 0

extern int ntt_sRing_on_pass[];
extern int ntt_sRing_off_pass[];
extern int ntt_sRing_on_cnt;
extern int ntt_sRing_off_cnt;

/*
   setting the NTT short ring OFF level  unit = 10 ms,
   EX: set_NTT_sRing_off_lvl(25, 60)
   600ms >  valid sRing off time >250ms
 */
void set_NTT_sRing_off_lvl(int low_lvl, int hi_lvl);

/*
   setting the NTT short ring ON level  unit = 10 ms,
   EX: set_NTT_sRing_on_lvl(25, 60)
   600ms >  valid sRing on time >250ms
 */
void set_NTT_sRing_on_lvl(int low_lvl, int hi_lvl);

/*
   setting the NTT short ring ON-OFF times:
   EX: set_NTT_onoff_times( 2, 1)
   
   sRing_on -> sRing_off -> sRing_on ; in the second ring_on 
   if enable NTT caller id det. DAA going to will OFF-HOOK, to recv the japan caller id.
 */
void set_NTT_onoff_times(int ring_on_cnt, int ring_off_cnt );
void NTT_sRing_det(int chid, int ring_stat);

#endif
