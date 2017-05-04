#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "bfdtimeout.h"

//#define MAINDEBUG(x)	printf(x)

static struct bfd_callout *pbfd_callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void bfd_timeout(
void (*func) __P((void *)),
void *arg,
struct timeval time,
struct bfd_callout *handle)
{
	struct bfd_callout *p = pbfd_callout;
	
	//MAINDEBUG(("Timeout %p:%p in %d seconds.", func, arg, time));	
	handle->c_arg = arg;
	handle->c_func = func;
	gettimeofday(&timenow, NULL);
	timeradd( &timenow, &time, &handle->c_time );
	
	/* try to find out the handle */
	while (p) 
	{
		if(p == handle) {
			return;
		}
		p = p->c_next;
	}
	
	/* put handle in the front */
	handle->c_next = pbfd_callout;
	pbfd_callout = handle;
}


/*
 * untimeout - Unschedule a timeout.
 */
void bfd_untimeout(struct bfd_callout *handle)
{
	struct bfd_callout **q, *p;	

	//MAINDEBUG(("Untimeout %p:%p.", func, arg));	
	/* Remove the entry matching timeout and remove it from the list. */
	for (q = &pbfd_callout; (p = *q); q = &p->c_next)
		if (p == handle) 
		{
			*q = p->c_next;
			break;
		}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
void bfd_calltimeout(void)
{
	struct bfd_callout *p = pbfd_callout;
	struct bfd_callout *q;
	
	gettimeofday(&timenow, NULL);	
	while (p) 
	{
		if((timenow.tv_sec > p->c_time.tv_sec) ||
			(timenow.tv_sec == p->c_time.tv_sec && timenow.tv_usec >= p->c_time.tv_usec)) 
		{
			(*p->c_func)(p->c_arg);
		}
		p = p->c_next;
	}
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
int bfd_timeleft(struct bfd_callout *handle, struct timeval *tvp)
{
	struct bfd_callout *p = pbfd_callout;

	if( (handle==NULL) || (tvp==NULL) )
		return -1;

	tvp->tv_sec = tvp->tv_usec = 0;
	/* try to find out the handle */
	while (p) 
	{
		if(p == handle) {
			break;
		}
		p = p->c_next;
	}
	//not found
	if(p==NULL) return -1;
	
	gettimeofday(&timenow, NULL);
	tvp->tv_sec = handle->c_time.tv_sec - timenow.tv_sec;
	tvp->tv_usec = handle->c_time.tv_usec - timenow.tv_usec;
	if (tvp->tv_usec < 0) {
		tvp->tv_usec += 1000000;
		tvp->tv_sec -= 1;
	}
	if (tvp->tv_sec < 0)
		tvp->tv_sec = tvp->tv_usec = 0;
	
	return 0;
}
