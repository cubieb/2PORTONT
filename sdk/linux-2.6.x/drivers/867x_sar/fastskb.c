/*
 *	Implementation for the private skb data pool handlers.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include "fastskb.h"

//#define FASTSKB_DEBUG

#define MODULENAME "fastskb"
#define PFX MODULENAME ": "

#ifdef FASTSKB_DEBUG
#define assert(expr) \
	if (!(expr)) {					\
		printk( "Assertion failed! %s,%s,%s,line=%d\n",	\
		#expr,__FILE__,__FUNCTION__,__LINE__);		\
	}
#define dprintk(fmt, args...)	do { printk(PFX fmt, ## args); } while (0)
#else
#define assert(expr) do {} while (0)
#define dprintk(fmt, args...)	do {} while (0)
#endif /* FASTSKB_DEBUG */

int enable_sar_fastskb=1;
u8 *private_data_pool[DATA_POOL_SIZE];
int pvt_data_pool_producer=0;
int pvt_data_pool_consumer=DATA_POOL_SIZE-1;

int flush_skb_data_bfr(void)
{
	int ptr;
	int count=0;
	
	dprintk("consumer=%d; producer=%d\n", pvt_data_pool_consumer, pvt_data_pool_producer);
	ptr = (pvt_data_pool_consumer+1)&(DATA_POOL_SIZE-1);
	while (ptr != pvt_data_pool_producer) {
		if (private_data_pool[ptr]==NULL)
			dprintk("Assertion failed! %s,%s,%s,line=%d\n",
			"private_data_pool[ptr]!=NULL",__FILE__,__FUNCTION__,__LINE__);
		else
			kfree(private_data_pool[ptr]);
		count++;
		ptr = (ptr+1)&(DATA_POOL_SIZE-1);
	}
	pvt_data_pool_producer=0;
	pvt_data_pool_consumer=DATA_POOL_SIZE-1;
	dprintk("free count = %d\n", count);
	return 0;
}

