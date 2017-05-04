/*
 *	Definitions for the private skb data pool handlers.
 */

#ifndef _SAR_FASTSKB_H
#define _SAR_FASTSKB_H

#include <linux/types.h>
#include <asm/system.h>

//#define DATA_POOL_SIZE 64
#define DATA_POOL_SIZE 256

extern u8 *private_data_pool[DATA_POOL_SIZE];
extern int pvt_data_pool_producer;
extern int pvt_data_pool_consumer;
extern int enable_sar_fastskb;
int flush_skb_data_bfr(void);

// Get data buffer from pool
static inline u8 *get_skb_data_bfr(void)
{
	//jim add context protection to avoid...
	unsigned long flags;
	int tmp;
	u8 *data;	
	if (!enable_sar_fastskb)
		return 0;
	local_irq_save(flags);
	tmp=(pvt_data_pool_consumer+1)&(DATA_POOL_SIZE-1);
	data=NULL;
	if (tmp!=pvt_data_pool_producer) {
		data = private_data_pool[tmp];
		private_data_pool[tmp] = 0;
		pvt_data_pool_consumer = tmp;
	};
	local_irq_restore(flags);	
	return data;
}

// Return data buffer to pool
static inline int put_skb_data_bfr(u8 *data)
{
	unsigned long flags;
	int tmp;
	int rtn_val;

	if (!enable_sar_fastskb)
		return 0;
	
	local_irq_save(flags);
	tmp=(pvt_data_pool_producer+1)&(DATA_POOL_SIZE-1);
	rtn_val=0;
	if (tmp!=pvt_data_pool_consumer) {
		private_data_pool[pvt_data_pool_producer] = data;
		pvt_data_pool_producer = tmp;
		rtn_val = 1;
	};
	local_irq_restore(flags);

	return rtn_val;
}

#endif	/* _SAR_FASTSKB_H */
