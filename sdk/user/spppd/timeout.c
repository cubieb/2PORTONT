
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "timeout.h"

// Modified by Mason Yu
//#define MAINDEBUG(x)	printf(x)
#define MAINDEBUG	printf


static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */
void untimeout(struct callout *handle);

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, time, handle)
    void (*func) __P((void *));
    void *arg;
    int time;
    struct callout *handle;
{
    struct callout *p, **pp;

    // Kaohj --- protect from re-timeout for the same handle
    untimeout(handle);
    // Modified and Commented by Mason Yu     
    //MAINDEBUG(("Timeout %p:%p in %d seconds.", func, arg, time));
    //MAINDEBUG("Timeout %p:%p in %d seconds.", func, arg, time);
    
    handle->c_arg = arg;
    handle->c_func = func;
    gettimeofday(&timenow, NULL);
    handle->c_time.tv_sec = timenow.tv_sec + time;
    handle->c_time.tv_usec = timenow.tv_usec;
  
    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (handle->c_time.tv_sec < p->c_time.tv_sec
	    || (handle->c_time.tv_sec == p->c_time.tv_sec
		&& handle->c_time.tv_usec < p->c_time.tv_usec))
	    break;
    handle->c_next = p;
    *pp = handle;
}

/*
 * mtimeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of micro-seconds.
 */
void
mtimeout(func, arg, time, handle)
    void (*func) __P((void *));
    void *arg;
    int time;
    struct callout *handle;
{
    struct callout *p, **pp;

    untimeout(handle);
    
    handle->c_arg = arg;
    handle->c_func = func;
    gettimeofday(&timenow, NULL);
    handle->c_time.tv_sec = timenow.tv_sec + time/1000000;
    handle->c_time.tv_usec = timenow.tv_usec + time%1000000;
  
    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (handle->c_time.tv_sec < p->c_time.tv_sec
	    || (handle->c_time.tv_sec == p->c_time.tv_sec
		&& handle->c_time.tv_usec < p->c_time.tv_usec))
	    break;
    handle->c_next = p;
    *pp = handle;
}

/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(handle)
struct callout *handle;
{
    struct callout **copp, *freep;
  
    //MAINDEBUG(("Untimeout %p:%p.", func, arg));
  
    /*
     * Find first matching timeout and remove it from the list.
     */
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep == handle) {
	    *copp = freep->c_next;
	    break;
	}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
void
calltimeout()
{
    struct callout *p;

    while (callout != NULL) {
		p = callout;
		if (gettimeofday(&timenow, NULL) < 0)
		    //fatal("Failed to get time of day: %m");
	    	printf("Failed to get time of day: %m");
		if (!(p->c_time.tv_sec < timenow.tv_sec
		      || (p->c_time.tv_sec == timenow.tv_sec
			  && p->c_time.tv_usec <= timenow.tv_usec)))
	    	break;		/* no, it's not time yet */
		callout = p->c_next;
		(*p->c_func)(p->c_arg);
    }
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}

#if 0
void test_timeout(void *arg)
{
	printf("test timeout!!\n");
	timeout(&test_timeout, 0, 3);
}

void main(void)
{
struct	callout test_handle;
	timeout(&test_timeout, 0, 3, &test_handle);
	while(1)
		calltimeout();
}

#endif
