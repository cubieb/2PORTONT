/*
	A easy tool for measure the time difference or cp3 cycles.
	Usage: inssert the ts_start(), and ts_end() to the code you want to measure.
	Please make sure that ts_start(), and ts_end() are used one by one.
	I.E. the call flow MUST be start -> end -> start ->end, repeatedly.

	Macro TS_SIZE is the buffer size for save the measured results.
	If buffer is full, then the results will be dumped.

	If HZ=100, then the accuracy of measurement is 10ms unit.
	If HZ=1000, then the accuracy of measurement is 1ms unit.

	When macro CP3_MEASURE is defined, then the CPU cycle results will be count
	rather then the time difference.

	Auther: thlin@realtek.com
*/

#include "voip_timer.h"
#include "cp3_profile.h"

//#define CP3_MEASURE

#ifdef CP3_MEASURE
#define CP3_PERDUMP_CNT		10
#endif

#define TS_SIZE 50
timetick_t ts_dbg_start[TS_SIZE] = {[0 ... TS_SIZE-1] = 0};
timetick_t ts_dbg_end[TS_SIZE] = {[0 ... TS_SIZE-1] = 0};
timetick_t ts_dbg_flag[TS_SIZE] = {[0 ... TS_SIZE-1] = 0};
static int ts_idx = 0;

void ts_start(void)
{
    if (ts_dbg_flag[ts_idx] == 0)
    {
#ifdef CP3_MEASURE
		ProfileEnterPoint(PROFILE_INDEX_TEMP219);
#else
        ts_dbg_start[ts_idx] = timetick;
#endif
        ts_dbg_flag[ts_idx] = 1;
        //printk("<<%d\n", ts_dbg_start[ts_idx]);
    }
}

void ts_end(void)
{
    int i;

    if (ts_dbg_flag[ts_idx] == 1)
    {
#ifdef CP3_MEASURE
		ProfileExitPoint(PROFILE_INDEX_TEMP219);	
#else
        ts_dbg_end[ts_idx] = timetick;
#endif
        //printk(">>%d\n", ts_dbg_end[ts_idx]);
        ts_idx = (ts_idx + 1)%TS_SIZE;

#ifdef CP3_MEASURE
		ProfilePerDump(PROFILE_INDEX_TEMP219, CP3_PERDUMP_CNT);
		if (ts_idx == 0)
		{
			for (i=0; i<TS_SIZE; i++ )
                ts_dbg_flag[i] = 0;
		}
#else
        if (ts_idx == 0)
        {
            for (i=0; i<TS_SIZE; i++ )
            {
            	// dump the save results when buffer is full.
                printk("[%d]:%d\n", i, ts_dbg_end[i] - ts_dbg_start[i]);
                ts_dbg_flag[i] = 0;
                ts_dbg_start[i] = 0;
                ts_dbg_end[i] = 0;
            }
        }
#endif

    }
}

