#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_SELECT_SECOND 30


typedef struct {
    unsigned int    enable;
    char            **app_cmd;
    pid_t           pid;
    int             handle;     // -1 implies not running
    char            *fifo;      // the name of the fifo, NULL implies pipe is used
    unsigned int    last_kick;  // in unix-time in second
    unsigned int    period;     // the amount seconds to fire
    
} app_t;
static int num_app=0;
static app_t *all_app=NULL;
static char _handle_string[16];
static unsigned int relaunch_second=5;

static char *pidfile = "/var/run/voip_gwdt.pid";



// debug
#ifdef GWDT_DEBUG
void __dump_app(app_t *a);  
void __dump_all_app(void); 
#endif

// memory allocation
#define PAGE_SZ     4096
#define MMN_SZ      64
#define MAX_UNIT    (PAGE_SZ-MMN_SZ)
#define PRELOC_SZ   (PAGE_SZ/2-MMN_SZ)
static char mm_prelocated[PRELOC_SZ];
static unsigned int mm_size=PRELOC_SZ;
static unsigned int mm_tail=0;
static char *mm_free=mm_prelocated;
#define mm_avaliable (mm_size-mm_tail)
char *
gwdt_malloc(unsigned int sz) {
    char *r=NULL;
    if (sz==0) return NULL;
    
    sz = (sz+3) & (~0x03);
    if (sz>mm_avaliable) {
        if (sz>=MAX_UNIT) return malloc(sz);
        mm_size += MMN_SZ;
        mm_size *= 2;
        mm_size -= MMN_SZ;
        if ((mm_free=malloc(mm_size))==NULL) {
            fprintf(stderr, "GWDT: memory allocation fail\n");
#ifdef SYSLOG
			syslog(LOG_LOCAL1|LOG_INFO, "GWDT: memory allocation fail\n");
#endif				
            exit(1);
            return NULL;
        }
        mm_tail=0;
    }
    r = mm_free+mm_tail;
    mm_tail+=sz;
    return r;
}


#if 0 

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;
	printf("pidfile %s\n",pidfile);

	if(pidfile == NULL)
		return -1;
	
	printf("pidfile %s\n",pidfile);
	
	pid_fd = open(pidfile, O_CREAT | O_TRUNC| O_WRONLY, 0644);
#if 0	
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);
#endif
	printf("pid_fd is %x\n",pid_fd);
	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0){
		printf("pid_fd fail \n");
		return;
	}
	printf("pidfile_write_release\n");
	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
//	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

#endif

// log
static void
gwdt_log(const char *s) {
}
// sleep
static void
msec_sleep(unsigned int msec) {
    struct timeval tv;
    tv.tv_sec=msec/1000;
    tv.tv_usec=(msec%1000)*1000;
    select(0, NULL, NULL, NULL, &tv);
}
// parsing
static char *script=NULL;
int
count_number_of_app(void) {
    int count=0;
    int skip=0;
    char *p=script;
    while(*p!='\0') {
        while(1) {
            if ((*p=='.')&&(skip==0)) {
                if ((p[1]=='a') && (p[2]=='p') && (p[3]=='p')) {
                    ++count;
                    p+=3;
                }
                skip=1;
            } else if (*p=='#') {
                skip=1;
            } else if ((*p=='\n')||(*p=='\0')) {
                skip=0;
                ++p;
                break;
            }
            ++p;
        }
    }
    return count;
}

static char *
_parsing_dup_token(char **src) {
    int n;
    char *r, *t;
    char *tail;
    
    // skip leading space
    while((**src==' ')||(**src=='\t')) ++(*src);
    tail=*src;
    // count token size
    while ((*tail!=' ')&&(*tail!='\t')&&(*tail!='\n')) ++tail;
    if ((n=tail-*src)==0) return NULL;
    // allocate space
    t=r=gwdt_malloc(n+1);
    // copy
    while(*src!=tail) *(t++)=*((*src)++);
    *t='\0';
    
    return r;
}
static unsigned int
_parsing_number(char **src) {
    unsigned int n;
    char *head,c;
    
    // skip leading space
    while((**src==' ')||(**src=='\t')) ++(*src);
    head=*src;
    // count token size
    while ((**src!=' ')&&(**src!='\t')&&(**src!='\n')) ++(*src);
    if ((*src-head)==0) return 0;
    c=**src;
    **src='\0';
    n=strtoul(head, 0, 0);
    **src=c;
    
    return n;
}



static int lineno=1;
static unsigned int current_timeout=0;
static app_t *current_app=NULL;
static char **
parsing_app(unsigned int lvl, char *parsing) {
    char **r, *p;
    char *e, c;
    unsigned int n;
    char buf[16];
    
    while((*parsing==' ')||(*parsing=='\t')) ++parsing;
    if (*parsing=='\n') {
        unsigned int sz=(lvl+1)*sizeof(char*);
        r=(char**)gwdt_malloc(sz);
        bzero(r, sz);
        return r;
    }
    e=parsing;
    while((*e!=' ')&&(*e!='\t')&&(*e!='\n')) ++e;
    c=*e;
    *e='\0';
    if ((parsing[0]=='$')&&(parsing[1]=='t')) {
        sprintf(buf, "%d", current_timeout);
        n=strlen(buf);
        p=gwdt_malloc(n+1);
        strcpy(p, buf);        
    } else if ((parsing[0]=='$')&&(parsing[1]=='f')) {
        char *f=current_app->fifo;
        if (f!=NULL) {
            p=f;
        } else {
            fprintf(stderr, "fifo is not defined but $f is used in line %d\n", lineno);
            p="1";
        }
    } else if ((parsing[0]=='$')&&(parsing[1]=='p')) {
        if (current_app->fifo==NULL) {
            p=_handle_string;
        } else {
            fprintf(stderr, "fifo is defined but $p is used in line %d\n", lineno);
            p="no_pipe_exist";
        }
    } else {
        n=strlen(parsing);
        p=gwdt_malloc(n+1);
        strcpy(p, parsing);
    }
    *e=c;
    r=parsing_app(lvl+1, e);
    r[lvl]=p;
    return r;
}
int
parsing_script(const char *fn) {
    int h;
    struct stat ss;
    off_t fsz;
    char *parsing;
    int skip_rest_of_line;
    char **r;
	char line[100];
    
    // open script file
    if ((h=open(fn, O_RDONLY))<0) {
        fprintf(stderr, "GWDT:can't open %s as <app-script>\n", fn);
#ifdef SYSLOG
		syslog(LOG_LOCAL1|LOG_INFO, "GWDT: can't open %s as <app-script>\n", fn);
#endif		
        return -1;
    }
    
    // allocate memory for script
    if (fstat(h, &ss)!=0) {
        perror("fstat");
        return -1;
    }
    fsz=ss.st_size;
    if ((script=(char*)malloc(fsz+10))==NULL) {
        perror("malloc");
        return -1;
    }
    
    
    // read whole script
    if (read(h, script, fsz)!=fsz) {
        fprintf(stderr, "fail in reading %s\n", fn);
        return -1;
    }
    script[fsz]='\n';
    script[fsz+1]='\0';
    close(h);
    
    // guess how many app in script
    num_app=count_number_of_app();
    if ((all_app=(app_t*)malloc(sizeof(num_app)*num_app))==NULL) {
        perror("malloc");
        return -1;
    }
    bzero(all_app, sizeof(num_app)*num_app);
    
    // parsing script
    current_app=all_app;
    parsing=script;
    while(1) {
        skip_rest_of_line=0;
        if (*parsing=='\0') break;
        // skip leading space
        while((*parsing==' ')||(*parsing=='\t')) ++parsing;
        
        // first character
        switch (*parsing) {
            case '\n': 
                ++lineno;
            case '\r': 
                ++parsing;
                break;
            case '.':
                ++parsing;
                if ((*parsing=='t')&&(strncmp(parsing, "timeout", 7)==0)) {
                    parsing+=7;
                    current_timeout=_parsing_number(&parsing);
                    skip_rest_of_line=1;
                    break;                    
                } else if ((*parsing=='f')&&(strncmp(parsing, "fifo", 4)==0)) {
                    parsing+=4;
                    current_app->fifo=_parsing_dup_token(&parsing);
                    skip_rest_of_line=1;
                    break;
                } else if ((*parsing=='r')&&(strncmp(parsing, "relaunch", 8)==0)) {
                    parsing+=8;
                    relaunch_second=_parsing_number(&parsing);
                    skip_rest_of_line=1;
                    break;
                } else if ((*parsing=='a')&&(strncmp(parsing, "app", 3)==0)) {
                    parsing+=3;
                    r=parsing_app(0, parsing);
                    current_app->app_cmd=r;
                    current_app->period=current_timeout;
                    current_app->handle=-1;
                    current_app->pid=-1;
                    if (access(r[0], X_OK)==0)
                        current_app->enable=1;
                    else
                        fprintf(stderr, "%s is not found, or not executable in line %d\n", r[0], lineno);
                    ++current_app;
                    skip_rest_of_line=1;
                    break;
                }
            default:
                fprintf(stderr, "GWDT: syntax error in line %d\n", lineno);
#ifdef SYSLOG
		syslog(LOG_LOCAL1|LOG_INFO, "GWDT: syntax error in line %d\n", lineno);
#endif				
            case '#':
                skip_rest_of_line=1;
                break;
        }
        if (skip_rest_of_line) {
            while(*parsing!='\n') ++parsing;
            if (*parsing=='\n') {
                ++lineno;
                ++parsing;
            }
        }
    }
    
//    __dump_all_app();
   
	 
    // free memory
    free(script);
	sprintf(line,"echo %d >/var/run/voip_gwdt.pid",getpid());
	//fprintf(stderr, "%s\n", line);
	system(line);

    return 0;
}

static void
gwdt_kill_app(app_t *a, int sig) {
    if (a->pid<0) return ;
    if (kill(a->pid, 0)==0) {
        if (waitpid(a->pid, NULL, WNOHANG)==a->pid) {
            a->pid=-1;
        } else {
            #ifdef GWDT_DEBUG
            unsigned int now=time(NULL);
            fprintf(stderr, "killing %s(%d) with sig=%d at %u\n", a->app_cmd[0], a->pid, sig, now);
            #endif
            kill(a->pid, sig);
        }
    } else {
        a->pid=-1;
    }
}
static void
gwdt_kill_all(int sig) {
    app_t *a;
    #ifdef GWDT_DEBUG
    fprintf(stderr, "gwdt kill all %d\n", sig);
    #endif
    for (a=all_app; a!=current_app; ++a) {
        if (a->enable==0) continue;
        if (a->handle>=0) {
            close(a->handle);
            a->handle=-1;   // to restart ASAP
            a->last_kick=0;
        }
        gwdt_kill_app(a, sig);
    }
}
static unsigned int
lunch_app(app_t *a) {
    unsigned int now=time(NULL);
    unsigned int d;
    pid_t pid;
    int p[2], f;
    
    d=a->last_kick+relaunch_second;
    if (d>now) return d-now;
    
    a->last_kick=now;
    gwdt_kill_app(a, SIGTERM);
    if (a->pid>=0) return relaunch_second;
    
    fprintf(stderr, "launch %s at %u\n", a->app_cmd[0], now);
    if (a->fifo==NULL) {
        if (pipe(p)!=0) {
            perror("pipe");
            return relaunch_second;
        }
        sprintf(_handle_string, "%d", p[1]);
        if ((pid=fork())<0) {
            perror("fork");
            return relaunch_second;
        }
        if (pid==0) {
            close(p[0]);
            if (execv(a->app_cmd[0], a->app_cmd)<0) {
                perror("execv");
                exit(0);
            }
        } else {
            msec_sleep(50);    // reduce the possiblity of copy-on-write
            close(p[1]);
            a->handle=p[0];
            a->pid=pid;
        }
    } else {
        unlink(a->fifo);
        if (mkfifo(a->fifo, 0666) == -1) {
            // fifo can't be created, give up the app
            fprintf(stderr, "mkfifo %s failed\n", a->fifo);
            a->enable=0;
            return MAX_SELECT_SECOND;
        }

        if ((pid=fork())<0) {
            perror("fork");
            return relaunch_second;
        }
        if (pid==0) {
            if (execv(a->app_cmd[0], a->app_cmd)<0) {
                perror("execv");
                exit(0);
            }
        } else {
            msec_sleep(50);    // reduce the possiblity of copy-on-write
            a->pid=pid;
            if ((f=open(a->fifo, O_RDONLY))<0) {
                // open fifo fails, restart again
                return relaunch_second;
            } else {
                a->handle=f;
            }
        }
    }
    return MAX_SELECT_SECOND;
}
static volatile enum {
    GWDTST_NORMAL=0,
    GWDTST_KILLALL,
    GWDTST_STOP,
} gwdt_stat=GWDTST_NORMAL;
static void
main_loop(void) {
    fd_set gdwt_fdset;
    struct timeval gdwt_tv;
    app_t *a;
    unsigned int now;
    unsigned int shortest_wait_time;
    int max_handle_num;
    unsigned int deadline, t;
    
    while(gwdt_stat!=GWDTST_STOP) {
        gwdt_stat=GWDTST_NORMAL;
        while (gwdt_stat==GWDTST_NORMAL) {
            shortest_wait_time=MAX_SELECT_SECOND;
            
            // check if any app should restart, if so, lunch it (again)
            for (a=all_app; a!=current_app; ++a) {
                if ((a->enable)&&(a->handle<0)) {
                    t=lunch_app(a);
                    if (shortest_wait_time>t) shortest_wait_time=t;
                }
            }
            
            // compute how long to sleep
            now=time(NULL);
            FD_ZERO(&gdwt_fdset);
            max_handle_num=-1;
            for (a=all_app; a!=current_app; ++a) {
                if ((a->enable)&&(a->handle>=0)&&(a->period>0)) {
                    deadline=a->last_kick+a->period;
                    if (deadline<=now) {
                        // this app expired, don't sleep
                        shortest_wait_time=0;
                        break;
                    } else {
                        FD_SET(a->handle, &gdwt_fdset);
                        if (max_handle_num<a->handle) max_handle_num=a->handle;
                        deadline -= now;
                        if (shortest_wait_time>deadline)
                            shortest_wait_time=deadline;
                    }
                }
            }
            
            // select
            if (max_handle_num>=0) {
                gdwt_tv.tv_sec=shortest_wait_time;
                gdwt_tv.tv_usec=0;
                select(max_handle_num+1, &gdwt_fdset, NULL, NULL, &gdwt_tv);
            }
            
            // check if any app kicks wdt, or if any app is timeout
            now=time(NULL);
            for (a=all_app; a!=current_app; ++a) {
                if ((a->enable)&&(a->handle>=0)) {
                    if (FD_ISSET(a->handle, &gdwt_fdset)) {
                        if (read(a->handle, _handle_string, sizeof(_handle_string))<=0) {
                            // the connection was broken
                            close(a->handle);
                            a->handle=-1;   // to restart ASAP
                            a->last_kick=0;
                            fprintf(stderr, "GWDT: connection broken %s(%d) at %u\n", a->app_cmd[0], a->pid, now);
#ifdef SYSLOG
							syslog(LOG_LOCAL1|LOG_INFO, "GWDT: connection broken %s(%d) at %u\n",a->app_cmd[0], a->pid, now);
#endif
                        } else {
                            // this app kicked the timer
                            a->last_kick=now;
                        }
                    } else {
                        // this app didn't kick the timer
                        if (a->period>0) {
                            deadline=a->last_kick+a->period;
                            if (deadline<=now) {
                                // this app is timeout
                                close(a->handle);
                                a->handle=-1;   // to restart ASAP
                                a->last_kick=0;
                                fprintf(stderr, "GWDT: timeout %s(%d) at %u\n", a->app_cmd[0], a->pid, now);
#ifdef SYSLOG
								syslog(LOG_LOCAL1|LOG_INFO, "GWDT: timeout %s(%d) at %u\n",a->app_cmd[0], a->pid, now);
#endif								
                            }
                        }
                    }
                }
            } //for (a=all_app; a!=current_app; ++a)
        } // while (gwdt_stat==GWDTST_NORMAL)
        
        // kill all app
        gwdt_kill_all(SIGTERM);
        sleep(1);
    }
}


static void
killall_singal_handle(int sig) {
    gwdt_stat=GWDTST_KILLALL;
    //raise(SIGALRM);
}
static void
stop_singal_handle(int sig) {
    gwdt_stat=GWDTST_STOP;
    //raise(SIGALRM);
    
}
#ifdef GWDT_DEBUG
static void
debug_internal(int sig) {
    __dump_all_app();
}
#endif
int
main(int argc, char *argv[]) {
	
	int pid_fd;
	FILE *fp;
	pid_t pid;
			

    if (argc==1) {
        fprintf(stderr, "usage: %s <app-script>\n", argv[0]);
        return 1;
    }
    if (parsing_script(argv[1])) 
        return 1;
        
    // hook singal handlers
    signal(SIGTERM, stop_singal_handle);
    signal(SIGINT, stop_singal_handle);
    signal(SIGHUP, killall_singal_handle);
    #ifdef GWDT_DEBUG
    signal(SIGUSR1, killall_singal_handle);
    signal(SIGUSR2, debug_internal);
    #endif
#if 0 //eric
	if ((fp = fopen(pidfile, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if (sscanf(line, "%d", &pid)) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
	}

	pid_fd = pidfile_acquire(pidfile);
	printf("*** pid_fd is %x\n",pid_fd);
	if (pid_fd < 0){
		printf("pidfile_acquire fail \n");
		return 0;
	}
	pidfile_write_release(pid_fd);
		
	fp = fopen(pidfile,"w+");     /* open file pointer */
	printf("gwdt pid is %s\n",getpid());
	fprintf(fp,"%d\n",getpid());
	fclose(fp);
#endif


    // enter main loop
    main_loop();
    
    // kill existed process (second kill)
    sleep(1);
    gwdt_kill_all(SIGKILL);
    return 0;
}
#ifdef GWDT_DEBUG
void 
__dump_app(app_t *a) {
    char **v=a->app_cmd;
    int kill_0=0;
    fprintf(stderr, "================(%p)\n", a);
    fprintf(stderr, "    enable=%d\n", a->enable);
    if (a->pid>=0) {
        kill_0=kill(a->pid, 0);
        fprintf(stderr, "    pid=%d, kill_0=%d\n", a->pid, kill_0);
    } else {
        fprintf(stderr, "    pid=%d\n", a->pid);
    }
    fprintf(stderr, "    handle=%d\n", a->handle);
    fprintf(stderr, "    fifo=%s\n", (a->fifo==NULL)?"NULL":a->fifo);
    fprintf(stderr, "    period=%d\n", a->period);
    fprintf(stderr, "    last_kick=%d\n", a->last_kick);
    while (*v!=NULL) {
        fprintf(stderr, "        []=%s(%p)\n", (*v==NULL)?"NULL":*v, *v);
        ++v;
    }
}
void 
__dump_all_app(void) {
    app_t *a=all_app;
    int i=0;
    
    fprintf(stderr, "re-launch = %d\n", relaunch_second);
    while ((a!=current_app)&&(i<num_app)) {
        __dump_app(a++);
        i++;
    }
    fprintf(stderr, "================\n");
    fprintf(stderr, "number of app = %d\n", num_app);
}
#endif
