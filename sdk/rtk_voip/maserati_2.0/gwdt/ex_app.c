#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

static void
usage(const char *name) {
    fprintf(stderr, "usage: %s -wdt <pipe-number> <timeout>\n", name);
    fprintf(stderr, "    or %s -wdt_fifo <fifo-name> <timeout>\n", name);
    fprintf(stderr, "    <timeout> should be between 0 and 86400\n");
    fprintf(stderr, "    <pipe-number> should be between 0 and 4096\n");
    exit(1);
}

int h=0;
int period=0;
char *wdt_fifo=NULL;
volatile int keep_kick=1;
volatile int to_leave=0;
static void
sigusr1_hand(int sig) {
    keep_kick=0;
}
static void
sigusr2_hand(int sig) {
    to_leave=1;
}


int
main(int argc, char *argv[]) {
    int kick_period;
    unsigned int t;
    int mypid=getpid();
    
    // parsing the argument passed into the app
    if (argc!=4) usage(argv[0]);
    if (strcmp(argv[1], "-wdt")==0) {
        // ex_app runs in pipe-mode.
        // the handle was passed into
        h=atoi(argv[2]);
        if ((h<0) || (h>4096)) {
            fprintf(stderr, "a strange pipe-number = %d\n", h);
            usage(argv[0]);
        }
    } else if (strcmp(argv[1], "-wdt_fifo")==0) {
        // ex_app runs in fifo-mode
        wdt_fifo=argv[2];
        // put the initialization of shared objects here
        sleep(2);
        // open the fifo in order of the succession of gwdt
        if ((h=open(wdt_fifo, O_WRONLY))<0) {
            fprintf(stderr, "can't open fifo '%s'\n", wdt_fifo);
            return 1;
        }
    } else usage(argv[0]);

    // compute the period to kick the watchdog
    period=atoi(argv[3]);
    if ((period<0) || (period>86400)) {
        fprintf(stderr, "an illegal timeout = %d\n", period);
        usage(argv[0]);
    }
    kick_period=(period*2)/5;
    t=time(NULL);
    fprintf(stderr, "[ex-app(%d)] kick_period=%d/period=%d with handle=%d at %u\n", 
        mypid, kick_period, period, h, t);
    
    // hook signals to debug
    signal(SIGUSR1, sigusr1_hand);
    signal(SIGUSR2, sigusr2_hand);
    
    // kick_period = 0 implies the timeout is infinite, so no kick is needed
    if (kick_period==0) {
        kick_period=1;
        keep_kick=0;
    }
    while(to_leave==0) {
        sleep(kick_period);
        t=time(NULL);
        if (keep_kick) {
            fprintf(stderr, "[ex-app(%d)] kick at %u\n", mypid, t);
            // kick the timer by writing a short string to the handle of pipe/fifo
            write(h, "123", 4);
        }
    }
    
    return 0;
}
