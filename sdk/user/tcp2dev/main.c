#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <netdb.h>
#include    <sys/time.h> 
#include    <sys/types.h>
#include    <sys/ioctl.h>
#include    <arpa/inet.h>
#include    <string.h>
#include    <strings.h>
#include    <unistd.h>
#include    <stdlib.h>
#include    <fcntl.h>
#include    <sys/stat.h>
#include    <termios.h>

#include    "connect.h"
static inline int intmax(int a, int b) {return a>b?a:b;}
static inline int intmin(int a, int b) {return a<b?a:b;}
    
#define BUFF_SZ (4000)
typedef enum {
    TSE_UNDEFINED,
    TSE_TTY,
    TSE_DEV
} se_type_t;
typedef struct {
    int             handle;
    int             ins, del;
    char            *wbuf;
        // name==NULL: tcp socket
        // otherwise, 'name' means a device or a tty of an execution file
} single_end_t;
typedef struct {
    single_end_t    end[2]; // 0: TCP port, 1: tty or dev
    se_type_t       type;   // for end[1]
    const char      *name;  // for end[1]
    int             passive_sck;
    const char      *port_name;
    // with type==TTY
    pid_t           cpid;   // for end[1] 
    const char      *tty_name;
    const char      *pty_name;
} pair_end_t;

static void
_pe_init(pair_end_t *pe, const char *name, const char *port_name, char *wbuf0, char *wbuf1, se_type_t t) {
    bzero(pe, sizeof(pair_end_t));
    pe->passive_sck=-1;
    pe->port_name=port_name;
    pe->name=name;
    pe->type=t;
    pe->cpid=-1;
    pe->end[0].handle=pe->end[1].handle=-1;
    pe->end[0].wbuf=wbuf0;
    pe->end[1].wbuf=wbuf1;
}
static inline void
pe_init_tty(pair_end_t *pe, const char *name, const char *port_name, char *wbuf0, char *wbuf1, const char *tty_name, const char *pty_name) {
    _pe_init(pe, name, port_name, wbuf0, wbuf1, TSE_TTY);
    pe->tty_name=tty_name;
    pe->pty_name=pty_name;
}
static inline void
pe_init_dev(pair_end_t *pe, const char *name, const char *port_name, char *wbuf0, char *wbuf1) {
    _pe_init(pe, name, port_name, wbuf0, wbuf1, TSE_DEV);
}
static void
_se_close(single_end_t *se) {
    se->ins=se->del=0;
    if (se->handle>=0) {
        int h=se->handle;
        se->handle=-1;
        close(h);
    }
}
static inline void
se_close_tcp(pair_end_t *pe) {
    single_end_t *se=pe->end;
    _se_close(se);
}
static void
se_close_end(pair_end_t *pe) {
    single_end_t *se=pe->end+1;
    _se_close(se);

    // close the execution file on the tty
    if ((pe->type==TSE_TTY) && (pe->cpid>0)) {
        int cpid=pe->cpid;
        pe->cpid=-1;
        kill(cpid, SIGTERM);
        sleep(1);
        if (waitpid(cpid, NULL, WNOHANG)==0) {
            kill(cpid, SIGKILL);
            waitpid(cpid, NULL, 0);
        }
    }
}
static int
_pe_open_tty(pair_end_t *pe) {
    se_close_end(pe);
    const char *pty_name=pe->pty_name;
    int h=open(pty_name, O_RDWR|O_NONBLOCK);
    if (h<0) {
        perror(pty_name);
        return -1;
    }
    
    int cpid=fork();
    if (cpid==0) {
        close(h);
        const char *name=pe->name;
        const char *tty_name=pe->tty_name;

        if (chown(tty_name, getuid(), -1)<0) {
            perror("chown");
        }
        if (chmod(tty_name, S_IRUSR|S_IWUSR)<0) {
            perror("chmod");
        }
        int slave=open(tty_name, O_RDWR);
        if (slave<0) {
            perror(tty_name);
            return -1;
        }
        if (setsid()<0) {
            perror("setsid");
        }
        if (ioctl(slave, TIOCSCTTY, NULL)) {
            perror("ioctl(TIOCSCTTY)");
        }

        struct termios t;
        bzero(&t, sizeof(t));
        if (!tcgetattr(slave,  &t)) {
            t.c_cc[VERASE] = 0x08; // BACKSPACE
            tcsetattr(0, TCSANOW, &t);
        }
        
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave>STDERR_FILENO) close(slave);

        
        execl(name, name, NULL);
        // should never be here
        perror(name);
        return -1;
    }       
    pe->end[1].handle=h;
    pe->cpid=cpid;
    return h;
}
static int
_pe_open_dev(pair_end_t *pe) {
    se_close_end(pe);
    const char *name=pe->name;
    int h=open(name, O_RDWR|O_NONBLOCK);
    if (h<0) {
        perror(name);
        return -1;
    }
    pe->end[1].handle=h;
    return h;
}
static inline int
pe_open_end(pair_end_t *pe) {
    return (pe->type==TSE_TTY)?_pe_open_tty(pe):_pe_open_dev(pe);
}

static int
pe_tcp_accept(pair_end_t *pe, int handle) {
    if (pe->type==TSE_TTY) {
        if (_pe_open_tty(pe)<0) {
            close(handle);
            return -1;
        }
#if 0
        struct termios t;
        bzero(&t, sizeof(t));
        int h=pe->end[1].handle;
        tcgetattr(h, &t);
        t.c_lflag &= ~(ICANON|ISIG); //|ECHO|ECHOCTL|ECHOE|ECHOK|ECHONL|ECHOPRT);
        t.c_lflag |= IGNBRK;
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        tcsetattr(h, TCSANOW, &t);
#endif
    } else {
        if (_pe_open_dev(pe)<0) {
            close(handle);
            return -1;
        }
    }
    pe->end[0].handle=handle;
    return 0;
}
static inline int
se_bytes_in_buffer(single_end_t *se) {
    return (se->ins-se->del+BUFF_SZ)%BUFF_SZ;
}
static inline int
se_bytes_free(single_end_t *se) {
    return BUFF_SZ-se_bytes_in_buffer(se)-1;
}
static inline int
_se_bytes_to_write(int ins, int del) {
    return (del>ins)?(del-ins-1):(BUFF_SZ-del);
}

static int
se_insert(single_end_t *se, const char *src, int size) {
    int s=0, c;
    int ins=se->ins;
    int del=se->del;
    char *wbuf=se->wbuf;
    while (((c=_se_bytes_to_write(ins, del))>0)&&(size>0)) {
        c=intmin(size, c);
        memcpy(wbuf + ins, src, c);
        s+=c;
        size-=c;
        src+=c;
        ins=(ins+c)%BUFF_SZ;
    }
    se->ins=ins;
    return s;
}
static inline int
_se_bytes_to_read(int ins, int del) {
    return (ins>=del)?(ins-del):(BUFF_SZ-del);
}
static int
se_output_wbuf(single_end_t *se) {
    int c, d, s=0;
    int ins=se->ins;
    int del=se->del;
    int handle=se->handle;
    char *wbuf=se->wbuf;

    while ((c=_se_bytes_to_read(ins, del))>0) {
        d=write(handle, wbuf + del, c);
        if (d<=0) break;
        del=(del+d)%BUFF_SZ;
        s+=d;
        if (d!=c) break;
    }
    se->del=del;
    return s;
}
static int
pe_open_passivesocket(pair_end_t *pe) {
    if (pe->passive_sck>=0) close(pe->passive_sck);
    int ss=passivesocket(pe->port_name, "tcp", 1);
    if (ss<0) {
        char msg[64];
        sprintf(msg, "open passive socket %s", pe->port_name);
        perror(msg);
        ss=-1;
    }
    pe->passive_sck=ss;
    return ss;
}
static int
se_set_fds(single_end_t *se, int max, fd_set *rfds, fd_set *wfds) {
    int h=se->handle;
    if (h<0) return max;
    
    FD_SET(h, rfds);
    if (se_bytes_in_buffer(se)>0) FD_SET(h, wfds);
    return intmax(max, h);
}

static int
pe_set_fds(pair_end_t *pe, int max, fd_set *rfds, fd_set *wfds) {
    int psck=pe->passive_sck;
    if (psck<0) return max;

    int e0h=pe->end[0].handle;
    if (e0h>=0) {
        max=se_set_fds(pe->end, max, rfds, wfds);
        max=se_set_fds(pe->end+1, max, rfds, wfds);
        return max;
    } else {
        FD_SET(psck, rfds);
        return intmax(max, psck);
    } 
}

char local_buffer[BUFF_SZ];
static int
se_is_issue(pair_end_t *pe, int idx, fd_set *rfds, fd_set *wfds, fd_set *efds) {
    single_end_t *se=pe->end+idx;
    int h=se->handle;
    if (h<0) return 0;
    
    if (FD_ISSET(h, efds)) {
        se_close_tcp(pe);
        se_close_end(pe);
        return -1;
    }
    if (FD_ISSET(h, wfds)) {
        se_output_wbuf(se);
    }
    if (FD_ISSET(h, rfds)) {
        single_end_t *soe=pe->end+(idx^1);

        int c=se_bytes_free(soe);
        c=read(h, local_buffer, c);
        if (c>0) {
            //debug_display(local_buffer, c); // debug display
            se_insert(soe, local_buffer, c);
        } else {
            fprintf(stderr, (idx==0)?"peer closed\n":"tty or device closed\n");
            se_close_tcp(pe);
            se_close_end(pe);
            return -1;
        }
    }
    
    return 0;
}
static void
pe_is_issue(pair_end_t *pe, fd_set *rfds, fd_set *wfds, fd_set *efds) {
    int psck=pe->passive_sck;
    if (psck<0) return;
        
    if (se_is_issue(pe, 0, rfds, wfds, efds)<0) return;
    if (se_is_issue(pe, 1, rfds, wfds, efds)<0) return;
    
    // passive socket
    if (FD_ISSET(psck, rfds)) {
        int peer=accept(psck, NULL, NULL);
        if (peer<0) {
            pe->passive_sck=-1;
            perror("accept");
            close(psck);
        } else {
            if (pe->end[0].handle<0) {
                pe_tcp_accept(pe, peer);
            } else {
                fprintf(stderr, "internal error: should not accept twice for single passive socket %s, close the second one.\n", pe->port_name);
                close(peer);
            }
        }
    }
    if (FD_ISSET(psck, wfds)) {
        pe->passive_sck=-1;
        char msg[64];
        sprintf(msg, "passive socket %s", pe->port_name);
        perror(msg);
        close(psck);
    }
}
static void
pe_check_tty_peer(pair_end_t *pe) {
    if (pe->type!=TSE_TTY) return;
    if (pe->cpid>=0) {
        if (waitpid(pe->cpid, NULL, WNOHANG)!=0) {
            fprintf(stderr, "%s: terminated\n", pe->name);
            _se_close(pe->end+1);
            se_close_tcp(pe);
            pe->cpid=-1;
        }
    }
}

static void
show_usage(char *name) {
    fprintf(stderr,"usage: %s <dev-name>:<tcp-port-number>\n", name);
    fprintf(stderr,"\tor %s -tty <tty-name>[,<pty-name>] <exec-file>:<tcp-port-number>\n", name);
}
static inline void
debug_display(char *s, int n) {
    s[n]='\0';
    while (*s!='\0') {
        char c=*(s++);
        if (c<32) printf("(%02x)", (unsigned int)(unsigned char)c);
        else printf("%c", c);
    }
    printf("[%d]\n", n);
}
static int
parameter_seperate(char **first, char **second, char *src, char seperator) {
    char *niddle=strchr(src, seperator);
    if (niddle==NULL) {
        *first=src;
        *second=NULL;
        return 1;
    } else {
        *first=src;
        *(niddle++)='\0';
        *second=niddle;
        return 2;
    }
}

#define MAX_PAIR        4
pair_end_t              pairs[MAX_PAIR];
int npair=0;

int 
main(int argc, char *argv[]) {
    char *name=NULL;
    char *port_name=NULL;
    char *tty_name=NULL;
    char *pty_name=NULL;
    int exec_tty=0;

    char **p=argv;
    while (*(++p)!=NULL) {
        if (**p=='-') {
            if (strcmp(*p, "-tty")==0) {
                if (*(++p)==NULL) {
                    show_usage(argv[0]);
                    return 1;
                }
                exec_tty=1;
                parameter_seperate(&tty_name, &pty_name, *p, ',');
                if (pty_name==NULL) {
                    int l=strlen(tty_name);
                    pty_name=alloca(l+1);
                    strcpy(pty_name, tty_name);
                    char *q=pty_name+l;
                    while (q!=pty_name) {
                        if (*q=='/') {++q; break;}
                        --q;
                    }
                    *q='p';
                }
            }
        } else {
            if (npair<MAX_PAIR) {
                parameter_seperate(&name, &port_name, *p, ':');
                if ((name==NULL)||(port_name==NULL)) {
                    show_usage(argv[0]);
                    return 1;
                }
                if ((exec_tty)&&(tty_name==NULL)) {
                    show_usage(argv[0]);
                    return 1;
                }
                
                pair_end_t *curr=pairs+(npair++);
                char *wbuf0=alloca(BUFF_SZ);
                char *wbuf1=alloca(BUFF_SZ);
                if ((wbuf0==NULL)||(wbuf1==NULL)) {
                    fprintf(stderr, "alloca failed\n");
                    return 1;
                }
                if (exec_tty) {
                    pe_init_tty(curr, name, port_name, wbuf0, wbuf1, tty_name, pty_name);
                } else {
                    pe_init_dev(curr, name, port_name, wbuf0, wbuf1);
                }
            }

            name=NULL;
            port_name=NULL;
            tty_name=NULL;
            pty_name=NULL;
            exec_tty=0;
        }
    }
    int i;
    for (i=0;i<npair;++i) 
        pe_open_passivesocket(pairs+i);
    
    fd_set read_fd, write_fd, error_fd;
    while (1) {
        unsigned int max=0;
        FD_ZERO(&read_fd);
        FD_ZERO(&write_fd);
        
        // set fds
        for (i=0;i<npair;++i) 
            max=pe_set_fds(pairs+i, max, &read_fd, &write_fd);
        error_fd=read_fd;

        // check if tty peer was killed
        for (i=0;i<npair;++i) 
            pe_check_tty_peer(pairs+i);

        // main select
        struct timeval wait_period={.tv_sec=3, .tv_usec=0};
        select(1+max, &read_fd, &write_fd, &error_fd, &wait_period);

        // check if any event happened
        for (i=0;i<npair;++i) 
            pe_is_issue(pairs+i, &read_fd, &write_fd, &error_fd);
    }
    
    return 0;
}


