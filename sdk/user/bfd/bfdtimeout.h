#ifndef _BFDTIMEOUT_H_
#define _BFDTIMEOUT_H_

#include <sys/time.h>

struct bfd_callout 
{
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct bfd_callout	*c_next;
};

void bfd_timeout(void (*func) __P((void *)), void *arg, struct timeval time, struct bfd_callout *handle);
void bfd_untimeout(struct bfd_callout *handle);
void bfd_calltimeout(void);
int bfd_timeleft(struct bfd_callout *handle, struct timeval *tvp);

#endif /*_BFDTIMEOUT_H_*/
