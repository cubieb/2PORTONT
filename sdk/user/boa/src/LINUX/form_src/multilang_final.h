typedef unsigned int size_t;
typedef int wchar_t;


typedef struct
  {
    int quot;
    int rem;
  } div_t;
typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;


__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;

extern size_t _stdlib_mb_cur_max (void) __attribute__ ((__nothrow__)) ;

extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;


__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;


extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;


extern float strtof (__const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern long double strtold (__const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;


extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;


typedef unsigned int __kernel_dev_t;
typedef unsigned long __kernel_ino_t;
typedef unsigned int __kernel_mode_t;
typedef unsigned long __kernel_nlink_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef long int __kernel_ipc_pid_t;
typedef int __kernel_uid_t;
typedef int __kernel_gid_t;
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef int __kernel_ptrdiff_t;
typedef long __kernel_time_t;
typedef long __kernel_suseconds_t;
typedef long __kernel_clock_t;
typedef long __kernel_daddr_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
typedef int __kernel_uid32_t;
typedef int __kernel_gid32_t;
typedef __kernel_uid_t __kernel_old_uid_t;
typedef __kernel_gid_t __kernel_old_gid_t;
typedef __kernel_dev_t __kernel_old_dev_t;
typedef long long __kernel_loff_t;
typedef struct {
 long val[2];
} __kernel_fsid_t;
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;
__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;
__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;
__extension__ typedef int __clockid_t;
__extension__ typedef void * __timer_t;
__extension__ typedef long int __blksize_t;
__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;
__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;
__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;
__extension__ typedef int __ssize_t;
typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;
__extension__ typedef int __intptr_t;
__extension__ typedef unsigned int __socklen_t;
typedef __kernel_ipc_pid_t __ipc_pid_t;
struct __sched_param
  {
    int __sched_priority;
  };
struct _pthread_fastlock
{
  long int __status;
  int __spinlock;
};
typedef struct _pthread_descr_struct *_pthread_descr;
typedef struct __pthread_attr_s
{
  int __detachstate;
  int __schedpolicy;
  struct __sched_param __schedparam;
  int __inheritsched;
  int __scope;
  size_t __guardsize;
  int __stackaddr_set;
  void *__stackaddr;
  size_t __stacksize;
} pthread_attr_t;
typedef long __pthread_cond_align_t;
typedef struct
{
  struct _pthread_fastlock __c_lock;
  _pthread_descr __c_waiting;
  char __padding[48 - sizeof (struct _pthread_fastlock)
   - sizeof (_pthread_descr) - sizeof (__pthread_cond_align_t)];
  __pthread_cond_align_t __align;
} pthread_cond_t;
typedef struct
{
  int __dummy;
} pthread_condattr_t;
typedef unsigned int pthread_key_t;
typedef struct
{
  int __m_reserved;
  int __m_count;
  _pthread_descr __m_owner;
  int __m_kind;
  struct _pthread_fastlock __m_lock;
} pthread_mutex_t;
typedef struct
{
  int __mutexkind;
} pthread_mutexattr_t;
typedef int pthread_once_t;
typedef struct _pthread_rwlock_t
{
  struct _pthread_fastlock __rw_lock;
  int __rw_readers;
  _pthread_descr __rw_writer;
  _pthread_descr __rw_read_waiting;
  _pthread_descr __rw_write_waiting;
  int __rw_kind;
  int __rw_pshared;
} pthread_rwlock_t;
typedef struct
{
  int __lockkind;
  int __pshared;
} pthread_rwlockattr_t;
typedef __volatile__ int pthread_spinlock_t;
typedef struct {
  struct _pthread_fastlock __ba_lock;
  int __ba_required;
  int __ba_present;
  _pthread_descr __ba_waiting;
} pthread_barrier_t;
typedef struct {
  int __pshared;
} pthread_barrierattr_t;
typedef unsigned long int pthread_t;
typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;
typedef __loff_t loff_t;
typedef __ino_t ino_t;
typedef __dev_t dev_t;
typedef __gid_t gid_t;
typedef __mode_t mode_t;
typedef __nlink_t nlink_t;
typedef __uid_t uid_t;
typedef __off_t off_t;
typedef __pid_t pid_t;
typedef __id_t id_t;
typedef __ssize_t ssize_t;
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;
typedef __key_t key_t;

typedef __time_t time_t;


typedef __clockid_t clockid_t;
typedef __timer_t timer_t;
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));
typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));
typedef int register_t __attribute__ ((__mode__ (__word__)));
typedef int __sig_atomic_t;
typedef struct {
 unsigned long __val[(128 / (8 * sizeof (unsigned long)))];
} __sigset_t;
typedef __sigset_t sigset_t;
struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
typedef __suseconds_t suseconds_t;
typedef long int __fd_mask;
typedef struct
  {
    __fd_mask __fds_bits[1024 / (8 * sizeof (__fd_mask))];
  } fd_set;
typedef __fd_mask fd_mask;

extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);

__extension__
static __inline unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
static __inline unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
static __inline unsigned long long int gnu_dev_makedev (unsigned int __major,
       unsigned int __minor)
     __attribute__ ((__nothrow__));
__extension__ static __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}
__extension__ static __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}
__extension__ static __inline unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}
typedef __blkcnt_t blkcnt_t;
typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;

__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
__extension__
extern u_quad_t strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern char *l64a (long int __n) __attribute__ ((__nothrow__)) ;
extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern long int random (void) __attribute__ ((__nothrow__));
extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__));
extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };
extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__));
extern double drand48 (void) __attribute__ ((__nothrow__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern long int lrand48 (void) __attribute__ ((__nothrow__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern long int mrand48 (void) __attribute__ ((__nothrow__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void srand48 (long int __seedval) __attribute__ ((__nothrow__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };
extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;
extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;


extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));
extern void free (void *__ptr) __attribute__ ((__nothrow__));

extern void cfree (void *__ptr) __attribute__ ((__nothrow__));

extern void *alloca (size_t __size) __attribute__ ((__nothrow__));

extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;
extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));
extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));


extern void _Exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));


extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern char *__secure_getenv (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__));
extern int clearenv (void) __attribute__ ((__nothrow__));
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern int system (__const char *__command) ;

extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) ;
typedef int (*__compar_fn_t) (__const void *, __const void *);

extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;
extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;


__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;

extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) ;
extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) ;
extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) ;
extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));
extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));

extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern int posix_openpt (int __oflag) ;



typedef struct __STDIO_FILE_STRUCT FILE;


typedef struct __STDIO_FILE_STRUCT __FILE;
typedef unsigned int wint_t;
typedef struct
{
 wchar_t __mask;
 wchar_t __wc;
} __mbstate_t;
struct sched_param
  {
    int __sched_priority;
  };

extern int clone (int (*__fn) (void *__arg), void *__child_stack,
    int __flags, void *__arg, ...) __attribute__ ((__nothrow__));

typedef unsigned long int __cpu_mask;
typedef struct
{
  __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;

extern int sched_setparam (__pid_t __pid, __const struct sched_param *__param)
     __attribute__ ((__nothrow__));
extern int sched_getparam (__pid_t __pid, struct sched_param *__param) __attribute__ ((__nothrow__));
extern int sched_setscheduler (__pid_t __pid, int __policy,
          __const struct sched_param *__param) __attribute__ ((__nothrow__));
extern int sched_getscheduler (__pid_t __pid) __attribute__ ((__nothrow__));
extern int sched_yield (void) __attribute__ ((__nothrow__));
extern int sched_get_priority_max (int __algorithm) __attribute__ ((__nothrow__));
extern int sched_get_priority_min (int __algorithm) __attribute__ ((__nothrow__));
extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t) __attribute__ ((__nothrow__));



typedef __clock_t clock_t;



struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long int tm_gmtoff;
  __const char *tm_zone;
};


struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
struct sigevent;

extern clock_t clock (void) __attribute__ ((__nothrow__));
extern time_t time (time_t *__timer) __attribute__ ((__nothrow__));
extern double difftime (time_t __time1, time_t __time0)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern time_t mktime (struct tm *__tp) __attribute__ ((__nothrow__));
extern size_t strftime (char *__restrict __s, size_t __maxsize,
   __const char *__restrict __format,
   __const struct tm *__restrict __tp) __attribute__ ((__nothrow__));


extern struct tm *gmtime (__const time_t *__timer) __attribute__ ((__nothrow__));
extern struct tm *localtime (__const time_t *__timer) __attribute__ ((__nothrow__));

extern struct tm *gmtime_r (__const time_t *__restrict __timer,
       struct tm *__restrict __tp) __attribute__ ((__nothrow__));
extern struct tm *localtime_r (__const time_t *__restrict __timer,
          struct tm *__restrict __tp) __attribute__ ((__nothrow__));

extern char *asctime (__const struct tm *__tp) __attribute__ ((__nothrow__));
extern char *ctime (__const time_t *__timer) __attribute__ ((__nothrow__));

extern char *asctime_r (__const struct tm *__restrict __tp,
   char *__restrict __buf) __attribute__ ((__nothrow__));
extern char *ctime_r (__const time_t *__restrict __timer,
        char *__restrict __buf) __attribute__ ((__nothrow__));
extern char *tzname[2];
extern void tzset (void) __attribute__ ((__nothrow__));
extern int daylight;
extern long int timezone;
extern int stime (__const time_t *__when) __attribute__ ((__nothrow__));
extern time_t timegm (struct tm *__tp) __attribute__ ((__nothrow__));
extern time_t timelocal (struct tm *__tp) __attribute__ ((__nothrow__));
extern int dysize (int __year) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern int nanosleep (__const struct timespec *__requested_time,
        struct timespec *__remaining);
extern int clock_getres (clockid_t __clock_id, struct timespec *__res) __attribute__ ((__nothrow__));
extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __attribute__ ((__nothrow__));
extern int clock_settime (clockid_t __clock_id, __const struct timespec *__tp)
     __attribute__ ((__nothrow__));
extern int timer_create (clockid_t __clock_id,
    struct sigevent *__restrict __evp,
    timer_t *__restrict __timerid) __attribute__ ((__nothrow__));
extern int timer_delete (timer_t __timerid) __attribute__ ((__nothrow__));
extern int timer_settime (timer_t __timerid, int __flags,
     __const struct itimerspec *__restrict __value,
     struct itimerspec *__restrict __ovalue) __attribute__ ((__nothrow__));
extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     __attribute__ ((__nothrow__));
extern int timer_getoverrun (timer_t __timerid) __attribute__ ((__nothrow__));




enum
{
  PTHREAD_CREATE_JOINABLE,
  PTHREAD_CREATE_DETACHED
};
enum
{
  PTHREAD_INHERIT_SCHED,
  PTHREAD_EXPLICIT_SCHED
};
enum
{
  PTHREAD_SCOPE_SYSTEM,
  PTHREAD_SCOPE_PROCESS
};
enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP
};
enum
{
  PTHREAD_PROCESS_PRIVATE,
  PTHREAD_PROCESS_SHARED
};
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_WRITER_NP
};
struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};
enum
{
  PTHREAD_CANCEL_ENABLE,
  PTHREAD_CANCEL_DISABLE
};
enum
{
  PTHREAD_CANCEL_DEFERRED,
  PTHREAD_CANCEL_ASYNCHRONOUS
};
extern int pthread_create (pthread_t *__restrict __threadp,
      __const pthread_attr_t *__restrict __attr,
      void *(*__start_routine) (void *),
      void *__restrict __arg) __attribute__ ((__nothrow__));
extern pthread_t pthread_self (void) __attribute__ ((__nothrow__));
extern int pthread_equal (pthread_t __thread1, pthread_t __thread2) __attribute__ ((__nothrow__));
extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));
extern int pthread_join (pthread_t __th, void **__thread_return);
extern int pthread_detach (pthread_t __th) __attribute__ ((__nothrow__));
extern int pthread_attr_init (pthread_attr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_attr_destroy (pthread_attr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
     int __detachstate) __attribute__ ((__nothrow__));
extern int pthread_attr_getdetachstate (__const pthread_attr_t *__attr,
     int *__detachstate) __attribute__ ((__nothrow__));
extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
           __const struct sched_param *__restrict
           __param) __attribute__ ((__nothrow__));
extern int pthread_attr_getschedparam (__const pthread_attr_t *__restrict
           __attr,
           struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__));
extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy)
     __attribute__ ((__nothrow__));
extern int pthread_attr_getschedpolicy (__const pthread_attr_t *__restrict
     __attr, int *__restrict __policy)
     __attribute__ ((__nothrow__));
extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
      int __inherit) __attribute__ ((__nothrow__));
extern int pthread_attr_getinheritsched (__const pthread_attr_t *__restrict
      __attr, int *__restrict __inherit)
     __attribute__ ((__nothrow__));
extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope)
     __attribute__ ((__nothrow__));
extern int pthread_attr_getscope (__const pthread_attr_t *__restrict __attr,
      int *__restrict __scope) __attribute__ ((__nothrow__));
extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
          void *__stackaddr) __attribute__ ((__nothrow__));
extern int pthread_attr_getstackaddr (__const pthread_attr_t *__restrict
          __attr, void **__restrict __stackaddr)
     __attribute__ ((__nothrow__));
extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
      size_t __stacksize) __attribute__ ((__nothrow__));
extern int pthread_attr_getstack (__const pthread_attr_t *__restrict __attr,
      void **__restrict __stackaddr,
      size_t *__restrict __stacksize) __attribute__ ((__nothrow__));
extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
          size_t __stacksize) __attribute__ ((__nothrow__));
extern int pthread_attr_getstacksize (__const pthread_attr_t *__restrict
          __attr, size_t *__restrict __stacksize)
     __attribute__ ((__nothrow__));
extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
      __const struct sched_param *__param)
     __attribute__ ((__nothrow__));
extern int pthread_getschedparam (pthread_t __target_thread,
      int *__restrict __policy,
      struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__));
extern int pthread_mutex_init (pthread_mutex_t *__restrict __mutex,
          __const pthread_mutexattr_t *__restrict
          __mutex_attr) __attribute__ ((__nothrow__));
extern int pthread_mutex_destroy (pthread_mutex_t *__mutex) __attribute__ ((__nothrow__));
extern int pthread_mutex_trylock (pthread_mutex_t *__mutex) __attribute__ ((__nothrow__));
extern int pthread_mutex_lock (pthread_mutex_t *__mutex) __attribute__ ((__nothrow__));
extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
        __const struct timespec *__restrict
        __abstime) __attribute__ ((__nothrow__));
extern int pthread_mutex_unlock (pthread_mutex_t *__mutex) __attribute__ ((__nothrow__));
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_mutexattr_getpshared (__const pthread_mutexattr_t *
      __restrict __attr,
      int *__restrict __pshared) __attribute__ ((__nothrow__));
extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
      int __pshared) __attribute__ ((__nothrow__));
extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
         __const pthread_condattr_t *__restrict
         __cond_attr) __attribute__ ((__nothrow__));
extern int pthread_cond_destroy (pthread_cond_t *__cond) __attribute__ ((__nothrow__));
extern int pthread_cond_signal (pthread_cond_t *__cond) __attribute__ ((__nothrow__));
extern int pthread_cond_broadcast (pthread_cond_t *__cond) __attribute__ ((__nothrow__));
extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
         pthread_mutex_t *__restrict __mutex);
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       __const struct timespec *__restrict
       __abstime);
extern int pthread_condattr_init (pthread_condattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_condattr_destroy (pthread_condattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_condattr_getpshared (__const pthread_condattr_t *
     __restrict __attr,
     int *__restrict __pshared) __attribute__ ((__nothrow__));
extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
     int __pshared) __attribute__ ((__nothrow__));
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
    __const pthread_rwlockattr_t *__restrict
    __attr) __attribute__ ((__nothrow__));
extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
           __const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__));
extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
           __const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__));
extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_getpshared (__const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pshared) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
       int __pshared) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_getkind_np (__const pthread_rwlockattr_t *__attr,
       int *__pref) __attribute__ ((__nothrow__));
extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
       int __pref) __attribute__ ((__nothrow__));
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared)
     __attribute__ ((__nothrow__));
extern int pthread_spin_destroy (pthread_spinlock_t *__lock) __attribute__ ((__nothrow__));
extern int pthread_spin_lock (pthread_spinlock_t *__lock) __attribute__ ((__nothrow__));
extern int pthread_spin_trylock (pthread_spinlock_t *__lock) __attribute__ ((__nothrow__));
extern int pthread_spin_unlock (pthread_spinlock_t *__lock) __attribute__ ((__nothrow__));
extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
     __const pthread_barrierattr_t *__restrict
     __attr, unsigned int __count) __attribute__ ((__nothrow__));
extern int pthread_barrier_destroy (pthread_barrier_t *__barrier) __attribute__ ((__nothrow__));
extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr) __attribute__ ((__nothrow__));
extern int pthread_barrierattr_getpshared (__const pthread_barrierattr_t *
        __restrict __attr,
        int *__restrict __pshared) __attribute__ ((__nothrow__));
extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
        int __pshared) __attribute__ ((__nothrow__));
extern int pthread_barrier_wait (pthread_barrier_t *__barrier) __attribute__ ((__nothrow__));
extern int pthread_key_create (pthread_key_t *__key,
          void (*__destr_function) (void *)) __attribute__ ((__nothrow__));
extern int pthread_key_delete (pthread_key_t __key) __attribute__ ((__nothrow__));
extern int pthread_setspecific (pthread_key_t __key,
    __const void *__pointer) __attribute__ ((__nothrow__));
extern void *pthread_getspecific (pthread_key_t __key) __attribute__ ((__nothrow__));
extern int pthread_once (pthread_once_t *__once_control,
    void (*__init_routine) (void));
extern int pthread_setcancelstate (int __state, int *__oldstate);
extern int pthread_setcanceltype (int __type, int *__oldtype);
extern int pthread_cancel (pthread_t __cancelthread);
extern void pthread_testcancel (void);
extern void _pthread_cleanup_push (struct _pthread_cleanup_buffer *__buffer,
       void (*__routine) (void *),
       void *__arg) __attribute__ ((__nothrow__));
extern void _pthread_cleanup_pop (struct _pthread_cleanup_buffer *__buffer,
      int __execute) __attribute__ ((__nothrow__));
extern int pthread_getcpuclockid (pthread_t __thread_id,
      __clockid_t *__clock_id) __attribute__ ((__nothrow__));
extern int pthread_sigmask (int __how,
       __const __sigset_t *__restrict __newmask,
       __sigset_t *__restrict __oldmask)__attribute__ ((__nothrow__));
extern int pthread_kill (pthread_t __threadid, int __signo) __attribute__ ((__nothrow__));
extern int pthread_atfork (void (*__prepare) (void),
      void (*__parent) (void),
      void (*__child) (void)) __attribute__ ((__nothrow__));
extern void pthread_kill_other_threads_np (void) __attribute__ ((__nothrow__));

typedef struct {
 __off_t __pos;
 __mbstate_t __mbstate;
 int __mblen_pending;
} __STDIO_fpos_t;
typedef struct {
 __off64_t __pos;
 __mbstate_t __mbstate;
 int __mblen_pending;
} __STDIO_fpos64_t;
typedef __off64_t __offmax_t;
struct __STDIO_FILE_STRUCT {
 unsigned short __modeflags;
 unsigned char __ungot_width[2];
 int __filedes;
 unsigned char *__bufstart;
 unsigned char *__bufend;
 unsigned char *__bufpos;
 unsigned char *__bufread;
 unsigned char *__bufgetc_u;
 unsigned char *__bufputc_u;
 struct __STDIO_FILE_STRUCT *__nextopen;
 wchar_t __ungot[2];
 __mbstate_t __state;
 int __user_locking;
 pthread_mutex_t __lock;
};
extern int __fgetc_unlocked(FILE *__stream);
extern int __fputc_unlocked(int __c, FILE *__stream);
extern FILE *__stdin;
extern FILE *__stdout;
typedef __builtin_va_list __gnuc_va_list;

typedef __STDIO_fpos_t fpos_t;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

extern int remove (__const char *__filename) __attribute__ ((__nothrow__));
extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));

extern int renameat (int __oldfd, __const char *__old, int __newfd,
       __const char *__new) __attribute__ ((__nothrow__));

extern FILE *tmpfile (void) ;
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__)) ;

extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__)) ;
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;

extern int fclose (FILE *__stream);
extern int fflush (FILE *__stream);

extern int fflush_unlocked (FILE *__stream);

extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes) ;
extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream) ;

extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__)) ;

extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__));
extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__));

extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__));
extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__));

extern int fprintf (FILE *__restrict __stream,
      __const char *__restrict __format, ...);
extern int printf (__const char *__restrict __format, ...);
extern int sprintf (char *__restrict __s,
      __const char *__restrict __format, ...) __attribute__ ((__nothrow__));
extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg);
extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);
extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));


extern int snprintf (char *__restrict __s, size_t __maxlen,
       __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));


extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) ;
extern int scanf (__const char *__restrict __format, ...) ;
extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));


extern int vfscanf (FILE *__restrict __s, __const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) ;
extern int vscanf (__const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (__const char *__restrict __s,
      __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__scanf__, 2, 0)));


extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);
extern int getchar (void);

extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
extern int fgetc_unlocked (FILE *__stream);

extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);
extern int putchar (int __c);

extern int fputc_unlocked (int __c, FILE *__stream);
extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);
extern int getw (FILE *__stream);
extern int putw (int __w, FILE *__stream);

extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     ;
extern char *gets (char *__s) ;


extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);
extern int puts (__const char *__s);
extern int ungetc (int __c, FILE *__stream);
extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) ;

extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) ;

extern int fseek (FILE *__stream, long int __off, int __whence);
extern long int ftell (FILE *__stream) ;
extern void rewind (FILE *__stream);

extern int fseeko (FILE *__stream, __off_t __off, int __whence);
extern __off_t ftello (FILE *__stream) ;

extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);
extern int fsetpos (FILE *__stream, __const fpos_t *__pos);


extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int ferror (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern void perror (__const char *__s);

extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern FILE *popen (__const char *__command, __const char *__modes) ;
extern int pclose (FILE *__stream);
extern char *ctermid (char *__s) __attribute__ ((__nothrow__));
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));
extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));


extern void *dlopen (__const char *__file, int __mode) __attribute__ ((__nothrow__));
extern int dlclose (void *__handle) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void *dlsym (void *__restrict __handle,
      __const char *__restrict __name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
extern char *dlerror (void) __attribute__ ((__nothrow__));

enum STR_TAG
{
 LANG_STR_NULL,
 LANG_SITE_CONTENTS,
 LANG_SPECIFIED_PC,
 LANG_STATUS,
 LANG_DEVICE,
 LANG_IPV6,
 LANG_LAN,
 LANG_WIZARD,
 LANG_WLAN,
 LANG_ADVANCED_SETTINGS,
 LANG_CERTIFICATION_INSTALLATION,
 LANG_WAN,
 LANG_ETHERNET_WAN,
 LANG_DSL_WAN,
 LANG_ATM_SETTINGS,
 LANG_ADSL_SETTINGS,
 LANG_3G_SETTINGS,
 LANG_VPN,
 LANG_MAC_FILTERING,
 LANG_PORT_FORWARDING,
 LANG_DOMAIN_BLOCKING,
 LANG_PARENTAL_CONTROL,
 LANG_CONNECTION_LIMIT,
 LANG_NAT_IP_FORWARDING,
 LANG_PORT_TRIGGERING,
 LANG_MLD_PROXY,
 LANG_MLD_SNOOPING,
 LANG_IGMP_PROXY,
 LANG_UPNP,
 LANG_RIP,
 LANG_LANDING_PAGE,
 LANG_VOIP,
 LANG_TONE,
 LANG_OTHER,
 LANG_ARP_TABLE,
 LANG_BRIDGING,
 LANG_ROUTING,
 LANG_PORT_MAPPING,
 LANG_CLASSIFICATION,
 LANG_QOS_QUEUE,
 LANG_TRAFFIC_SHAPING,
 LANG_REMOTE_ACCESS,
 LANG_DIAGNOSTICS,
 LANG_PACKET_DUMP,
 LANG_ADMIN,
 LANG_FINISH_MAINTENANCE,
 LANG_GPON_SETTINGS,
 LANG_OMCI_INFO,
 LANG_OMCI_OLT_MODE,
 LANG_OMCI_OLT_MODE_1,
 LANG_OMCI_OLT_MODE_2,
 LANG_OMCI_OLT_MODE_3,
 LANG_OMCI_OLT_MODE_4,
 LANG_OMCI_SW_VER1,
 LANG_OMCI_SW_VER2,
 LANG_OMCC_VER,
 LANG_OMCI_TM_OPT,
 LANG_OMCI_EQID,
 LANG_OMCI_ONT_VER,
 LANG_OMCI_VENDOR_ID,
 LANG_LOGOUT,
 LANG_SYSTEM_LOG,
 LANG_USER_ACCOUNT,
 LANG_PASSWORD,
 LANG_FIRMWARE_UPGRADE,
 LANG_TIME_ZONE,
 LANG_INTERFACE,
 LANG_ADSL,
 LANG_DEVICE_STATUS,
 LANG_PAGE_DESC_DEVICE_STATUS_SETTING,
 LANG_SYSTEM,
 LANG_ALIAS,
 LANG_UPTIME,
 LANG_DATE,
 LANG_TIME,
 LANG_FIRMWARE_VERSION,
 LANG_DSP_VERSION,
 LANG_DSP_SLAVE_VERSION,
 LANG_NAME_SERVERS,
 LANG_DEFAULT_GATEWAY,
 LANG_OPERATIONAL_STATUS,
 LANG_UPSTREAM_SPEED,
 LANG_DOWNSTREAM_SPEED,
 LANG_IP_ADDRESS,
 LANG_SUBNET_MASK,
 LANG_SERVER,
 LANG_MAC_ADDRESS,
 LANG_VPI_VCI,
 LANG_ENCAPSULATION,
 LANG_PROTOCOL,
 LANG_GATEWAY,
 LANG_DISCONNECT,
 LANG_CONNECT,
 LANG_DEVICE_NAME,
 LANG_LAN_PORT_STATUS,
 LANG_THIS_PAGE_SHOWS_THE_CURRENT_LAN_PORT_STATUS,
 LANG_THIS_PAGE_SHOWS_THE_CURRENT_SYSTEM_STATUS_OF_IPV6,
 LANG_IPV6_ADDRESS,
 LANG_IPV6_LINK_LOCAL_ADDRESS,
 LANG_PREFIX_DELEGATION,
 LANG_PREFIX,
 LANG_PAGE_DESC_PON_STATUS,
 LANG_STATUS_1,
 LANG_VENDOR_NAME,
 LANG_PART_NUMBER,
 LANG_TEMPERATURE,
 LANG_VOLTAGE,
 LANG_TX_POWER,
 LANG_RX_POWER,
 LANG_BIAS_CURRENT,
 LANG_LAN_INTERFACE_SETTINGS,
 LANG_PAGE_DESC_CONFIG_DEVICE_LAN_INTERFACE,
 LANG_SECONDARY_IP,
 LANG_DHCP_POOL,
 LANG_IGMP_SNOOPING,
 LANG_ETHERNET_TO_WIRELESS_BLOCKING,
 LANG_IPV6_ADDRESS_MODE,
 LANG_IPV6_PREFIX_LENGTH,
 LANG_VLAN_ID,
 LANG_IP_VERSION,
 LANG_MAC_BASED_TAG_DECISION,
 LANG_WLAN_BASIC_SETTINGS,
 LANG_PAGE_DESC_WLAN_BASIC_SETTING,
 LANG_DISABLE_WLAN_INTERFACE,
 LANG_BAND,
 LANG_MODE,
 LANG_SSID,
 LANG_CHANNEL_WIDTH,
 LANG_CONTROL_SIDEBAND,
 LANG_CHANNEL_NUMBER,
 LANG_RADIO_POWER_MW,
 LANG_STA_NUMBER,
 LANG_ASSOCIATED_CLIENTS,
 LANG_SHOW_ACTIVE_WLAN_CLIENTS,
 LANG_APPLY_CHANGES,
 LANG_RADIO_POWER,
 LANG_REGDOMAIN,
 LANG_MULTIPLE_AP,
 LANG_ACTIVE_WLAN_CLIENTS,
 LANG_THIS_TABLE_SHOWS_THE_MAC_ADDRESS,
 LANG_TX_PACKETS,
 LANG_RX_PACKETS,
 LANG_TX_RATE_MBPS,
 LANG_POWER_SAVING,
 LANG_EXPIRED_TIME_SEC,
 LANG_WLAN_ADVANCED_SETTINGS,
 LANG_PAGE_DESC_WLAN_ADVANCE_SETTING,
 LANG_FRAGMENT_THRESHOLD,
 LANG_RTS_THRESHOLD,
 LANG_BEACON_INTERVAL,
 LANG_DATA_RATE,
 LANG_AUTO,
 LANG_TX_OPERATION_RATE,
 LANG_TX_BASIC_RATE,
 LANG_DTIM_PERIOD,
 LANG_IAPP,
 LANG_PREAMBLE_TYPE,
 LANG_LONG_PREAMBLE,
 LANG_SHORT_PREAMBLE,
 LANG_BROADCAST,
 LANG_ENABLED,
 LANG_DISABLED,
 LANG_NOT_REGISTERED,
 LANG_REGISTERED,
 LANG_REGISTERING,
 LANG_REGISTER_FAIL,
 LANG_VOIP_RESTART,
 LANG_ERROR,
 LANG_RELAY_BLOCKING,
 LANG_PROTECTION,
 LANG_AGGREGATION,
 LANG_SHORT_GI,
 LANG_WMM_SUPPORT,
 LANG_WLAN_SECURITY_SETTINGS,
 LANG_PAGE_DESC_WLAN_SECURITY_SETTING,
 LANG_ENCRYPTION,
 LANG_AUTHENTICATION,
 LANG_OPEN_SYSTEM,
 LANG_SHARED_KEY,
 LANG_KEY_LENGTH,
 LANG_KEY_FORMAT,
 LANG_ENCRYPTION_KEY,
 LANG_AUTHENTICATION_MODE,
 LANG_CIPHER_SUITE,
 LANG_GROUP_KEY_UPDATE_TIMER,
 LANG_PRE_SHARED_KEY_FORMAT,
 LANG_PRE_SHARED_KEY,
 LANG_PORT,
 LANG_USE_LOCAL_AS_SERVER,
 LANG_IEEE_802_11W,
 LANG_SHA256,
 LANG_WLAN_ACCESS_CONTROL,
 LANG_PAGE_DESC_WLAN_ALLOW_DENY_LIST,
 LANG_ALLOW_LISTED,
 LANG_DENY_LISTED,
 LANG_ADD,
 LANG_RESET,
 LANG_CURRENT_ACCESS_CONTROL_LIST,
 LANG_DELETE_SELECTED,
 LANG_DELETE_ALL,
 LANG_SELECT,
 LANG_WI_FI_PROTECTED_SETUP,
 LANG_PAGE_DESC_WPS_SETTING,
 LANG_DISABLE_WPS,
 LANG_CONFIGURED,
 LANG_UNCONFIGURED,
 LANG_AUTO_LOCK_DOWN_STATE,
 LANG_LOCKED,
 LANG_UNLOCKED,
 LANG_UNLOCK,
 LANG_SELF_PIN_NUMBER,
 LANG_REGENERATE_PIN,
 LANG_CONFIGURATION,
 LANG_START_PIN,
 LANG_PUSH_BUTTON,
 LANG_START_PBC,
 LANG_CURRENT_KEY_INFO,
 LANG_KEY,
 LANG_CLIENT_PIN_NUMBER,
 LANG_WLAN_MULTIPLE_BSSID_SETTINGS,
 LANG_BLOCKING_BETWEEN_VAP,
 LANG_DISABLE,
 LANG_ENABLE,
 LANG_WLAN_STATUS,
 LANG_THIS_PAGE_SHOWS_THE_WLAN_CURRENT_STATUS,
 LANG_STATE,
 LANG_VIRTUAL_AP,
 LANG_REPEATER_INTERFACE,
 LANG_PAGE_DESC_CONFIGURE_PARAMETERS,
 LANG_WAN_MODE,
 LANG_CONNECTION_TYPE,
 LANG_ENABLE_VLAN,
 LANG_VLAN,
 LANG_CHANNEL_MODE,
 LANG_ENABLE_NAPT,
 LANG_ENABLE_IGMP,
 LANG_DEFAULT_ROUTE,
 LANG_ADMIN_STATUS,
 LANG_PPP_SETTINGS,
 LANG_TYPE,
 LANG_IDLE_TIME_SEC,
 LANG_AUTHENTICATION_METHOD,
 LANG_AC_NAME,
 LANG_SERVICE_NAME,
 LANG_OBTAIN_DEFAULT_GATEWAY_AUTOMATICALLY,
 LANG_USE_THE_FOLLOWING_DEFAULT_GATEWAY,
 LANG_USE_REMOTE_WAN_IP_ADDRESS,
 LANG_USE_WAN_INTERFACE,
 LANG_WAN_SETTING,
 LANG_ADDRESS_MODE,
 LANG_ENABLE_DHCPV6_CLIENT,
 LANG_REQUEST_OPTIONS,
 LANG_REQUEST_ADDRESS,
 LANG_REQUEST_PREFIX,
 LANG_DELETE,
 LANG_WAN_IP_SETTINGS,
 LANG_LOCAL,
 LANG_UNNUMBERED,
 LANG_802_1P_MARK,
 LANG_ENABLE_IGMP_PROXY,
 LANG_6RD_CONFIG,
 LANG_BOARD_ROUTER_V4_ADDRESS,
 LANG_6RD_IPV4_MASK_LEN,
 LANG_6RD_PREFIX_EX,
 LANG_6RD_PREFIX_LENGTH,
 LANG_PRIMARY,
 LANG_SECONDARY,
 LANG_DS_LITE_WAN_CONFIG,
 LANG_LOCAL_IPV6_ADDRESS,
 LANG_REMOTE_IPV6_END_POINT_ADDRESS,
 LANG_GATEWAY_IPV6_ADDRESS,
 LANG_ENABLE_QOS,
 LANG_PAGE_DESC_DSL_WAN_CONFIGURE,
 LANG_CURRENT_ATM_VC_TABLE,
 LANG_CURRENT_AUTO_PVC_TABLE,
 LANG_TABLE,
 LANG_MODIFY,
 LANG_REMOTE,
 LANG_ENABLE_AUTO_PVC_SEARCH,
 LANG_APPLY,
 LANG_PAGE_DESC_ATM_SETTING,
 LANG_UNDO,
 LANG_PAGE_DESC_3G_CONFIGURE,
 LANG_PIN_CODE,
 LANG_APN,
 LANG_DIAL_NUMBER,
 LANG_BACKUP_FOR_ADSL,
 LANG_BACKUP_TIMER_SEC,
 LANG_IDLE_TIME_MIN,
 LANG_NAPT,
 LANG_MTU,
 LANG_NONE,
 LANG_CONTINUOUS,
 LANG_CONNECT_ON_DEMAND,
 LANG_DHCP_SETTINGS,
 LANG_PAGE_DESC_CONFIGURE_DHCP_SERVER_RELAY,
 LANG_RELAY,
 LANG_PAGE_DESC_CONFIGURE_DHCP_SERVER_IP_ADDRESS,
 LANG_PAGE_DESC_ENABLE_DHCP_SERVER,
 LANG_IP_POOL_RANGE,
 LANG_SHOW_CLIENT,
 LANG_MAX_LEASE_TIME,
 LANG_SECONDS,
 LANG_MINUS_1_INDICATES_AN_INFINITE_LEASE,
 LANG_GATEWAY_ADDRESS,
 LANG_MAC_BASED_ASSIGNMENT,
 LANG_PORT_BASED_FILTER,
 LANG_PAGE_DESC_CONFIGURE_PORT_BASED_FILTERING,
 LANG_FILTER_DHCP_DISCOVER_PACKET,
 LANG_DNS_OPTION,
 LANG_USE_DNS_RELAY,
 LANG_SET_MANUALLY,
 LANG_PAGE_DESC_CONFIGURE_DNS_SERVER_IP_ADDRESSES,
 LANG_ATTAIN_DNS_AUTOMATICALLY,
 LANG_SET_DNS_MANUALLY,
 LANG_WAN_INTERFACE_BINDING,
 LANG_DYNAMIC,
 LANG_PAGE_DESC_CONFIGURE_DYNAMIC_DNS_ADDRESS,
 LANG_PROVIDER,
 LANG_HOSTNAME,
 LANG_SETTINGS,
 LANG_EMAIL,
 LANG_REMOVE,
 LANG_SERVICE,
 LANG_FILTERING,
 LANG_PAGE_DESC_DATA_PACKET_FILTER_TABLE,
 LANG_OUTGOING_DEFAULT_ACTION,
 LANG_DENY,
 LANG_ALLOW,
 LANG_INCOMING_DEFAULT_ACTION,
 LANG_DIRECTION,
 LANG_OUTGOING,
 LANG_INCOMING,
 LANG_RULE_ACTION,
 LANG_SOURCE,
 LANG_DESTINATION,
 LANG_CURRENT_FILTER_TABLE,
 LANG_SOURCE_PORT,
 LANG_DESTINATION_PORT,
 LANG_PAGE_DESC_LAN_TO_INTERNET_DATA_PACKET_FILTER_TABLE,
 LANG_DEFAULT_ACTION,
 LANG_PAGE_DESC_NAT_FIREWALL_FILTER_TABLE,
 LANG_BOTH,
 LANG_COMMENT,
 LANG_LOCAL_PORT,
 LANG_PUBLIC_PORT,
 LANG_ANY,
 LANG_CURRENT_PORT_FORWARDING_TABLE,
 LANG_REMOTE_HOST,
 LANG_APPLICATION,
 LANG_LOCAL_IP,
 LANG_LOCAL_PORT_FROM,
 LANG_LOCAL_PORT_TO,
 LANG_REMOTE_IP,
 LANG_REMOTE_PORT_FROM,
 LANG_REMOTE_PORT_TO,
 LANG_BLOCKING,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BLOCKED_FQDN_SUCH_AS_TW_YAHOO_COM_AND_FILTERED_KEYWORD_HERE_YOU_CAN_ADD_DELETE_FQDN_AND_FILTERED_KEYWORD,
 LANG_FQDN,
 LANG__TABLE,
 LANG_KEYWORD,
 LANG_KEYWORD_FILTERING_TABLE,
 LANG_FILTERED_KEYWORD,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BLOCKED_DOMAIN_HERE_YOU_CAN_ADD_DELETE_THE_BLOCKED_DOMAIN,
 LANG_DOMAIN,
 LANG_A_DEMILITARIZED_ZONE_IS_USED_TO_PROVIDE_INTERNET_SERVICES_WITHOUT_SACRIFICING_UNAUTHORIZED_ACCESS_TO_ITS_LOCAL_PRIVATE_NETWORK_TYPICALLY_THE_DMZ_HOST_CONTAINS_DEVICES_ACCESSIBLE_TO_INTERNET_TRAFFIC_SUCH_AS_WEB_HTTP_SERVERS_FTP_SERVERS_SMTP_E_MAIL_SERVERS_AND_DNS_SERVERS,
 LANG_DMZ_HOST,
 LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_MLD_PROXY,
 LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_VLAN_ON_LAN,
 LANG_WAN_INTERFACE,
 LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_MLD_SNOOPING,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_UPNP_THE_SYSTEM_ACTS_AS_A_DAEMON_WHEN_YOU_ENABLE_IT_AND_SELECT_WAN_INTERFACE_UPSTREAM_THAT_WILL_USE_UPNP,
 LANG_ENABLE_THE_RIP_IF_YOU_ARE_USING_THIS_DEVICE_AS_A_RIP_ENABLED_DEVICE_TO_COMMUNICATE_WITH_OTHERS_USING_THE_ROUTING_INFORMATION_PROTOCOL_THIS_PAGE_IS_USED_TO_SELECT_THE_INTERFACES_ON_YOUR_DEVICE_IS_THAT_USE_RIP_AND_THE_VERSION_OF_THE_PROTOCOL_USED,
 LANG_RECEIVE_MODE,
 LANG_SEND_MODE,
 LANG_RIP_CONFIG_TABLE,
 LANG_PAGE_DESC_MAC_TABLE_INFO,
 LANG_REFRESH,
 LANG_USER_LIST,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_BRIDGE_PARAMETERS_HERE_YOU_CAN_CHANGE_THE_SETTINGS_OR_VIEW_SOME_INFORMATION_ON_THE_BRIDGE_AND_ITS_ATTACHED_PORTS,
 LANG_AGEING_TIME,
 LANG_SPANNING_TREE,
 LANG_SHOW_MACS,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_ROUTING_INFORMATION_HERE_YOU_CAN_ADD_DELETE_IP_ROUTES,
 LANG_NEXT_HOP,
 LANG_METRIC,
 LANG_ADD_ROUTE,
 LANG_UPDATE,
 LANG_SHOW_ROUTES,
 LANG_STATIC_ROUTE_TABLE,
 LANG_THIS_PAGE_IS_USED_TO_ENABLE_DISABLE_MANAGEMENT_SERVICES_FOR_THE_LAN_AND_WAN,
 LANG_WAN_PORT,
 LANG_THIS_PAGE_BE_USED_TO_CONFIGURE_IPV6_ENABLE_DISABLE,
 LANG_IPV6_E,
 LANG_THIS_PAGE_IS_USED_TO_SETUP_THE_RADVD_S_CONFIGURATION_OF_YOUR_DEVICE,
 LANG_MAXRTRADVINTERVAL,
 LANG_MINRTRADVINTERVAL,
 LANG_ADVCURHOPLIMIT,
 LANG_ADVDEFAULTLIFETIME,
 LANG_ADVREACHABLETIME,
 LANG_ADVRETRANSTIMER,
 LANG_ADVLINKMTU,
 LANG_ADVSENDADVERT,
 LANG_ADVMANAGEDFLAG,
 LANG_ADVOTHERCONFIGFLAG,
 LANG_PREFIX_MODE,
 LANG_MANUAL,
 LANG_PREFIX_LENGTH,
 LANG_ADVVALIDLIFETIME,
 LANG_ADVPREFERREDLIFETIME,
 LANG_ADVONLINK,
 LANG_ADVAUTONOMOUS,
 LANG_ENABLE_ULA,
 LANG_ULA_PREFIX,
 LANG_ULA_PREFIX_LEN,
 LANG_ULA_PREFIX_VALID_TIME,
 LANG_ULA_PREFIX_PREFERED_TIME,
 LANG_RDNSS_1,
 LANG_RDNSS_2,
 LANG_ON,
 LANG_OFF,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_IPV6_STATIC_ROUTING_INFORMATION_HERE_YOU_CAN_ADD_DELETE_STATIC_IP_ROUTES,
 LANG_STATIC,
 LANG_ADD_DELETE_STATIC_IP_ROUTES,
 LANG_STATIC_IPV6_ROUTE_TABLE,
 LANG_INTERFACE_ID,
 LANG_OTHER_ADVANCED,
 LANG_HERE_YOU_CAN_SET_SOME_OTHER_ADVANCED_SETTINGS,
 LANG_IP_PASSTHROUGH,
 LANG_LEASE_TIME,
 LANG_ALLOW_LAN_ACCESS,
 LANG_PAGE_DESC_ICMP_DIAGNOSTIC,
 LANG_HOST_ADDRESS,
 LANG_GO,
 LANG_ATM_LOOPBACK_DIAGNOSTICS,
 LANG_CONNECTIVITY_VERIFICATION,
 LANG_CONNECTIVITY_VERIFICATION_IS_SUPPORTED_BY_THE_USE_OF_THE_ATM_OAM_LOOPBACK_CAPABILITY_FOR_BOTH_VP_AND_VC_CONNECTIONS_THIS_PAGE_IS_USED_TO_PERFORM_THE_VCC_LOOPBACK_FUNCTION_TO_CHECK_THE_CONNECTIVITY_OF_THE_VCC,
 LANG_FLOW_TYPE,
 LANG_F5_SEGMENT,
 LANG_F5_END_TO_END,
 LANG_LOOPBACK_LOCATION_ID,
 LANG_ADSL_TONE_DIAGNOSTICS,
 LANG_ADSL_TONE_DIAGNOSTICS_ONLY_ADSL2_2_SUPPORT_THIS_FUNCTION,
 LANG_DSL_TONE_DIAGNOSTICS,
 LANG_DSL_TONE_DIAGNOSTICS_ONLY_ADSL2_ADSL2_VDSL2_SUPPORT_THIS_FUNCTION,
 LANG_ADSL_SLAVE_TONE_DIAGNOSTICS,
 LANG_ADSL_SLAVE_TONE_DIAGNOSTICS_ONLY_ADSL2_2_SUPPORT_THIS_FUNCTION,
 LANG_DSL_SLAVE_TONE_DIAGNOSTICS,
 LANG_DSL_SLAVE_TONE_DIAGNOSTICS_ONLY_ADSL2_ADSL2_VDSL2_SUPPORT_THIS_FUNCTION,
 LANG_START,
 LANG_DOWNSTREAM,
 LANG_UPSTREAM,
 LANG_LOOP_ATTENUATION,
 LANG_SIGNAL_ATTENUATION,
 LANG_HLIN_SCALE,
 LANG_SNR_MARGIN,
 LANG_ATTAINABLE_RATE,
 LANG_OUTPUT_POWER,
 LANG_TONE_NUMBER,
 LANG_H_REAL,
 LANG_H_IMAGE,
 LANG_SNR,
 LANG_QLN,
 LANG_HLOG,
 LANG_ADSL_CONNECTION_DIAGNOSTICS,
 LANG_THE_DEVICE_IS_CAPABLE_OF_TESTING_YOUR_CONNECTION_THE_INDIVIDUAL_TESTS_ARE_LISTED_BELOW_IF_A_TEST_DISPLAYS_A_FAIL_STATUS_CLICK_GO_BUTTON_AGAIN_TO_MAKE_SURE_THE_FAIL_STATUS_IS_CONSISTENT,
 LANG_SELECT_THE_ADSL_CONNECTION,
 LANG_THIS_PAGE_IS_USED_TO_INFORM_ITMS_THAT_MAINTENANCE_IS_FINISHED_AND_THEN_ITMS_MAY_CHANGE_THIS_GATEWAY_S_PASSWORD,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_YOUR_GPON_NETWORK_ACCESS,
 LANG_LOID,
 LANG_LOID_PASSWORD,
 LANG_PLOAM_PASSWORD,
 LANG_SERIAL_NUMBER,
 LANG_COMMIT_AND_REBOOT,
 LANG_THIS_PAGE_IS_USED_TO_COMMIT_CHANGES_TO_SYSTEM_MEMORY_AND_REBOOT_YOUR_SYSTEM,
 LANG_BACKUP_AND_RESTORE_SETTINGS,
 LANG_CHOOSE_FILE,
 LANG_THIS_PAGE_ALLOWS_YOU_TO_BACKUP_CURRENT_SETTINGS_TO_A_FILE_OR_RESTORE_THE_SETTINGS_FROM_THE_FILE_WHICH_WAS_SAVED_PREVIOUSLY_BESIDES_YOU_COULD_RESET_THE_CURRENT_SETTINGS_TO_FACTORY_DEFAULT,
 LANG_BACKUP_SETTINGS_TO_FILE,
 LANG_BACKUP_DEFAULT_SETTINGS_TO_FILE,
 LANG_BACKUP_HARDWARE_SETTINGS_TO_FILE,
 LANG_BACKUP,
 LANG_RESTORE_SETTINGS_FROM_FILE,
 LANG_RESTORE,
 LANG_RESET_SETTINGS_TO_DEFAULT,
 LANG_PAGE_DESC_SET_ACCOUNT_PASSWORD,
 LANG_OLD_PASSWORD,
 LANG_NEW_PASSWORD,
 LANG_CONFIRMED_PASSWORD,
 LANG_PASSWORD_CONFIGURATION,
 LANG_PAGE_DESC_UPGRADE_FIRMWARE,
 LANG_UPGRADE,
 LANG_FIRMWARE_UPGRADING_PLEASE_WAIT,
 LANG_SYSTEM_RESTARTING_PLEASE_WAIT,
 LANG_PAGE_DESC_UPGRADE_NOTICE,
 LANG_PAGE_DESC_UPGRADE_CONFIRM,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_IP_ADDRESS_FOR_ACCESS_CONTROL_LIST_IF_ACL_IS_ENABLED_ONLY_THE_IP_ADDRESS_IN_THE_ACL_TABLE_CAN_ACCESS_CPE_HERE_YOU_CAN_ADD_DELETE_THE_IP_ADDRESS,
 LANG_CAPABILITY,
 LANG_YOU_CAN_MAINTAIN_THE_SYSTEM_TIME_BY_SYNCHRONIZING_WITH_A_PUBLIC_TIME_SERVER_OVER_THE_INTERNET,
 LANG_CURRENT_TIME,
 LANG_TIME_ZONE_SELECT,
 LANG_ENABLE_DAYLIGHT_SAVING_TIME,
 LANG_ENABLE_SNTP_CLIENT_UPDATE,
 LANG_MANUAL_SETTING,
 LANG_YES,
 LANG_NO,
 LANG_YEAR,
 LANG_MONTH,
 LANG_DAY,
 LANG_HOUR,
 LANG_MIN,
 LANG_SEC,
 LANG_192_5_41_41_NORTH_AMERICA,
 LANG_192_5_41_209_NORTH_AMERICA,
 LANG_130_149_17_8_EUROPE,
 LANG_203_117_180_36_ASIA_PACIFIC,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_TR_069_CPE_HERE_YOU_MAY_CHANGE_THE_SETTING_FOR_THE_ACS_S_PARAMETERS,
 LANG_PERIODIC_INFORM,
 LANG_PERIODIC_INFORM_INTERVAL,
 LANG_CONNECTION_REQUEST,
 LANG_PATH,
 LANG_CERTIFICATE_MANAGEMENT,
 LANG_CERTIFICATE_PASSWORD,
 LANG_CERTIFICATE,
 LANG_UPLOAD,
 LANG_TR069_DAEMON,
 LANG_ACS,
 LANG_URL,
 LANG_PARAMETER,
 LANG_INTERFACE_STATISITCS,
 LANG_PAGE_DESC_PACKET_STATISTICS_INFO,
 LANG_RESET_STATISTICS,
 LANG_ADSL_STATISTICS,
 LANG_DSL_STATISTICS,
 LANG_ADSL_SLAVE_STATISTICS,
 LANG_DSL_SLAVE_STATISTICS,
 LANG_ON_OFF,
 LANG_THIS_PAGE_IS_USED_TO_ENABLE_DISABLE_ALG_SERVICES,
 LANG_ENTRIES_IN_THIS_TABLE_ARE_USED_TO_RESTRICT_ACCESS_TO_INTERNET_FROM_YOUR_LOCAL_PCS_DEVICES_BY_MAC_ADDRESS_AND_TIME_INTERVAL_USE_OF_SUCH_FILTERS_CAN_BE_HELPFUL_FOR_PARENTS_TO_CONTROL_CHILDREN_S_USAGE_OF_INTERNET,
 LANG_SUN,
 LANG_MON,
 LANG_TUE,
 LANG_WED,
 LANG_THU,
 LANG_FRI,
 LANG_SAT,
 LANG_END,
 LANG_CONTROLLED_DAYS,
 LANG_START_BLOCKING_TIME,
 LANG_END_BLOCKING_TIME,
 LANG_CURRENT_PARENT_CONTROL_TABLE,
 LANG_MAX_PORTS,
 LANG_ENTRIES_IN_THIS_TABLE_ALLOW_YOU_TO_LIMIT_THE_NUMBER_OF_TCP_UDP_PORTS_USED_BY_INTERNAL_USERS,
 LANG_GLOBAL,
 LANG_0_FOR_NO_LIMIT,
 LANG_MAX_LIMITATION_PORTS,
 LANG_CURRENT_CONNECTION_LIMIT_TABLE,
 LANG_ENTRIES_IN_THIS_TABLE_ALLOW_YOU_TO_AUTOMATICALLY_REDIRECT_TRAFFIC_TO_A_SPECIFIC_MACHINE_BEHIND_THE_NAT_FIREWALL_THESE_SETTINGS_ARE_ONLY_NECESSARY_IF_YOU_WISH_TO_HOST_SOME_SORT_OF_SERVER_LIKE_A_WEB_SERVER_OR_MAIL_SERVER_ON_THE_PRIVATE_LOCAL_NETWORK_BEHIND_YOUR_GATEWAY_S_NAT_FIREWALL,
 LANG_EXTERNAL,
 LANG_CURRENT_NAT_IP_FORWARDING_TABLE,
 LANG_SELECT_GAME,
 LANG_GAME_RULES_LIST,
 LANG_CHECKED,
 LANG_PORT_TO_OPEN,
 LANG_ADDRESS_MAPPING_RULE,
 LANG_THIS_PAGE_IS_USED_TO_SET_AND_CONFIGURE_THE_ADDRESS_MAPPING_RULE_FOR_YOUR_DEVICE,
 LANG_THE_MAXIMUM_NUMBER_OF_ENTRIES_ARE_16,
 LANG_ONE_TO_ONE,
 LANG_MANY_TO_ONE,
 LANG_MANY_TO_MANY_OVERLOAD,
 LANG_ONE_TO_MANY,
 LANG_LOCAL_START_IP,
 LANG_LOCAL_END_IP,
 LANG_GLOBAL_START_IP,
 LANG_GLOBAL_END_IP,
 LANG_CURRENT_ADDRESS_MAPPING_TABLE,
 LANG_OUTBOUND_ITERFACE,
 LANG_TYPE_OF_SERVIE_TOS,
 LANG_PRECEDENCE,
 LANG_ENTRIES_IN_THIS_TABLE_ARE_USED_TO_ASSIGN_THE_PRECEDENCE_FOR_EACH_INCOMING_PACKET_IF_DISABLE_IP_QOS_TRAFFIC_SHAPING_WILL_BE_EFFECTIVE_OTHERWISE_TRAFFIC_SHAPING_IS_INVALID,
 LANG_POLICY,
 LANG_SPECIFY_TRAFFIC_CLASSIFICATION_RULES,
 LANG_SOURCE_MASK,
 LANG_DESTINATION_MASK,
 LANG_PHYSICAL_PORT,
 LANG_ASSIGN_PRIORITY_AND_OR_IP_PRECEDENCE_AND_OR_TYPE_OF_SERVICE_AND_OR_DSCP,
 LANG_OUTBOUND_PRIORITY,
 LANG_RULES,
 LANG_ENTRIES_IN_THIS_TABLE_ARE_USED_FOR_TRAFFIC_CONTROL,
 LANG_RATE,
 LANG_APPLY_TOTAL_BANDWIDTH_LIMIT,
 LANG_CONFIGURATION_OF_CLASSIFICATION_TABLE_FOR_IPQOS,
 LANG_NETMASK,
 LANG_CLASSIFICATION_RESULTS,
 LANG_CLASSQUEUE,
 LANG_DESCRIPTION,
 LANG_QUEUE_KEY,
 LANG_REMOVEQUEUE,
 LANG_SAVEANDREBOOT,
 LANG_QUEUE_CONFIG_LIST,
 LANG_QUEUE_DESCRIPTION,
 LANG_QUEUE_STATUS,
 LANG_CLICK_TO_SELECT,
 LANG_QUEUE_INTERFACE,
 LANG_QUEUE_PRIORITY,
 LANG_CANCEL,
 LANG_QUEUE_CONFIG,
 LANG_PAGE_DESC_CONFIGURE_QOS_POLICY,
 LANG_PRIO,
 LANG_WRR,
 LANG_QUEUE,
 LANG_PRIORITY,
 LANG_TOTAL_BANDWIDTH_LIMIT,
 LANG_PAGE_DESC_CLASSICY_QOS_RULE,
 LANG_PAGE_DESC_CLASSICY_QOS_RULE_EXTRA,
 LANG_CLASSIFICATION_RULES,
 LANG_MARK,
 LANG_ID,
 LANG_IP_QOS_RULE,
 LANG_NAME,
 LANG_MASK,
 LANG_EDIT,
 LANG_VERSION,
 LANG_802_1P,
 LANG_WANIF,
 LANG_RULE_DETAIL,
 LANG_CANCEL_TOTAL_BANDWIDTH_LIMIT,
 LANG_KB,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_PPTP_MODE_VPN,
 LANG_ACTION,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_L2TP_MODE_VPN,
 LANG_TUNNEL_AUTHENTICATION,
 LANG_TUNNEL_AUTHENTICATION_SECRET,
 LANG_PERSISTENT,
 LANG_DIAL_ON_DEMAND,
 LANG_CHALLENGE,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_IPIP_MODE_VPN,
 LANG_THIS_PAGE_ALLOWS_YOU_TO_INSTALL_ASU_AND_USER_CERTIFICATION_FOR_YOUR_DEVICE_PLEASE_NOTE_CERTIFICATION_TYPE_X_509_IS_SUPPORTED_AT_PRESENT,
 LANG_CERTIFICATION,
 LANG_WIRELESS_DISTRIBUTION_SYSTEM_USES_WIRELESS_MEDIA_TO_COMMUNICATE_WITH_OTHER_APS_LIKE_THE_ETHERNET_DOES_TO_DO_THIS_YOU_MUST_SET_THESE_APS_IN_THE_SAME_CHANNEL_AND_SET_MAC_ADDRESS_OF_OTHER_APS_WHICH_YOU_WANT_TO_COMMUNICATE_WITH_IN_THE_TABLE_AND_THEN_ENABLE_THE_WDS,
 LANG_SET_SECURITY,
 LANG_SHOW_STATISTICS,
 LANG_CURRENT_WDS_AP_LIST,
 LANG_WLAN_SITE_SURVEY,
 LANG_THIS_PAGE_PROVIDES_TOOL_TO_SCAN_THE_WIRELESS_NETWORK_IF_ANY_ACCESS_POINT_OR_IBSS_IS_FOUND_YOU_COULD_CHOOSE_TO_CONNECT_IT_MANUALLY_WHEN_CLIENT_MODE_IS_ENABLED,
 LANG_SIGNAL,
 LANG_CHANNEL,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_THE_BANDS_OF_YOUR_DEVICE,
 LANG_MODULATION,
 LANG_OPTION,
 LANG_NOTE_ONLY_ADSL_2_S_UPPORTS_ANNEXL,
 LANG_NOTE_ONLY_ADSL_2_2_SUPPORT_ANNEXM,
 LANG_TONE_MASK,
 LANG_PSD_MASK,
 LANG_PSD_MEASUREMENT_MODE,
 LANG_DSL_SETTINGS,
 LANG_THIS_PAGE_LET_USER_TO_MARK_THE_DESIGNATE_TONES_TO_BE_MASKED,
 LANG_MASK_ALL,
 LANG_UNMASK_ALL,
 LANG_CLOSE,
 LANG_THIS_PAGE_LET_USER_TO_SET_PSD_MASK,
 LANG_WIRELESS_BAND_MODE,
 LANG_THIS_PAGE_IS_USED_TO_SWITCH_802_11N_SINGLE_BAND_OR_DUAL_BAND_RADIO_FREQUENCY,
 LANG_SIGNLE_BAND,
 LANG_THIS_MODE_CAN_SUPPORT_SINGLE_MODE_BY_2X2,
 LANG_DUAL_BAND,
 LANG_THIS_MODE_CAN_SIMULTANEOUSLY_SUPPORT_802_11_A_B_G_N_WIRELESS_NETWORK_CONNECTION,
 LANG_THIS_PAGE_IS_USED_TO_SET_AND_CONFIGURE_THE_DYNAMIC_HOST_CONFIGURATION_PROTOCOL_MODE_FOR_YOUR_DEVICE_WITH_DHCP_IP_ADDRESSES_FOR_YOUR_LAN_ARE_ADMINISTERED_AND_DISTRIBUTED_AS_NEEDED_BY_THIS_DEVICE_OR_AN_ISP_DEVICE,
 LANG_IP_RANGE,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_UPPER_INTERFACE_SERVER_LINK_FOR_DHCPV6_RELAY,
 LANG_UPPER_INTERFACE,
 LANG_ENABLE_THE_DHCPV6_SERVER_IF_YOU_ARE_USING_THIS_DEVICE_AS_A_DHCPV6_SERVER_THIS_PAGE_LISTS_THE_IP_ADDRESS_POOLS_AVAILABLE_TO_HOSTS_ON_YOUR_LAN_THE_DEVICE_DISTRIBUTES_NUMBERS_IN_THE_POOL_TO_HOSTS_ON_YOUR_NETWORK_AS_THEY_REQUEST_INTERNET_ACCESS,
 LANG_VALID_LIFETIME,
 LANG_PREFERRED_LIFETIME,
 LANG_RENEW_TIME,
 LANG_REBIND_TIME,
 LANG_CLIENT,
 LANG_DOMAIN_SEARCH_TABLE,
 LANG_NAME_SERVER,
 LANG_NAME_SERVER_TABLE,
 LANG_AUTO_CONFIG_BY_PREFIX_DELEGATION_FOR_DHCPV6_SERVER,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_DHCPV6_SERVER_AND_DHCPV6_RELAY,
 LANG_IGMP_PROXY_ENABLES_THE_SYSTEM_TO_ISSUE_IGMP_HOST_MESSAGES_ON_BEHALF_OF_HOSTS_THAT_THE_SYSTEM_DISCOVERED_THROUGH_STANDARD_IGMP_INTERFACES_THE_SYSTEM_ACTS_AS_A_PROXY_FOR_ITS_HOSTS_WHEN_YOU_ENABLE_IT_BY_DOING_THE_FOLLOWS,
 LANG_ENABLE_IGMP_PROXY_ON_WAN_INTERFACE_UPSTREAM_WHICH_CONNECTS_TO_A_ROUTER_RUNNING_IGMP,
 LANG_ENABLE_IGMP_ON_LAN_INTERFACE_DOWNSTREAM_WHICH_CONNECTS_TO_ITS_HOSTS,
 LANG_PROXY_INTERFACE,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_TIME_INTERVAL_OF_LANDING_PAGE,
 LANG_TIME_INTERVAL,
 LANG_DIGITAL_MEDIA_SERVER_SETTINGS,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_YOUR_DIGITAL_MEDIA_SERVER,
 LANG_DIGITAL_MEDIA_SERVER,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_SNMP_HERE_YOU_MAY_CHANGE_THE_SETTINGS_FOR_SYSTEM_DESCRIPTION_TRAP_IP_ADDRESS_COMMUNITY_NAME_ETC,
 LANG_SYSTEM_DESCRIPTION,
 LANG_SYSTEM_CONTACT,
 LANG_SYSTEM_LOCATION,
 LANG_SYSTEM_OBJECT_ID,
 LANG_TRAP_IP_ADDRESS,
 LANG_COMMUNITY_NAME_READ_ONLY,
 LANG_COMMUNITY_NAME_WRITE_ONLY,
 LANG_TO_MANIPULATE_A_MAPPING_GROUP,
 LANG_SELECT_A_GROUP_FROM_THE_TABLE,
 LANG_SELECT_INTERFACES_FROM_THE_AVAILABLE_GROUPED_INTERFACE_LIST_AND_ADD_IT_TO_THE_GROUPED_AVAILABLE_INTERFACE_LIST_USING_THE_ARROW_BUTTONS_TO_MANIPULATE_THE_REQUIRED_MAPPING_OF_THE_PORTS,
 LANG_CLICK_APPLY_CHANGES_BUTTON_TO_SAVE_THE_CHANGES,
 LANG_NOTE_THAT_THE_SELECTED_INTERFACES_WILL_BE_REMOVED_FROM_THEIR_EXISTING_GROUPS_AND_ADDED_TO_THE_NEW_GROUP,
 LANG_GROUPED_INTERFACES,
 LANG_AVAILABLE_INTERFACES,
 LANG_DIFFERENTIATED_SERVICES,
 LANG_ENTRIES_IN_THIS_TABLE_ARE_USED_TO_ASSIGN_THE_PRECEDENCE_FOR_EACH_INCOMING_PACKET_BASED_ON_TCP_UDP_PORT_NUMBER_SOURCE_DESTINATION_IP_ADDRESS_SUBNET_MASKS,
 LANG_PHB_GROUPS,
 LANG_RATE_LIMIT,
 LANG_MAX_DESIRED_LATENCY,
 LANG_ETHERNET_LINK_SPEED_DUPLEX_MODE,
 LANG_HALF_MODE,
 LANG_FULL_MODE,
 LANG_AUTO_MODE,
 LANG_SET_THE_ETHERNET_LINK_SPEED_DUPLEX_MODE,
 LANG_THIS_PAGE_IS_USED_TO_SHOW_PRINTER_URL_S,
 LANG_PRINTER,
 LANG_ADD_IP_QOS_TRAFFIC_SHAPING_RULE,
 LANG_SOURCE_PREFIX_LENGTH,
 LANG_DESTINATION_PREFIX_LENGTH,
 LANG_UPLINK_RATE,
 LANG_THIS_PAGE_IS_USED_TO_START_OR_STOP_A_WIRESHARK_PACKET_CAPTURE,
 LANG_YOU_NEED_TO_RETURN_TO_THIS_PAGE_TO_STOP_IT,
 LANG_CLICK_HERE_FOR_THE_DOCUMENTATION_OF_THE_ADDITIONAL_ARGUMENTS,
 LANG_ADDITIONAL_ARGUMENTS,
 LANG_STOP,
 LANG_THIS_PAGE_IS_USED_TO_LOGOUT_FROM_THE_DEVICE,
 LANG_LOGIN_USER,
 LANG_INPUT_USERNAME_AND_PASSWORD,
 LANG_LOGIN,
 LANG_DEVICE_WEBSERVER,
 LANG_ADD_QOS_CLASSIFICATION_RULES,
 LANG_PLEASE_SELECT_IP_VERSION,
 LANG_THIS_PAGE_IS_USED_TO_ADD_A_IP_QOS_CLASSIFICATION_RULE,
 LANG_RULE,
 LANG_ASSIGN,
 LANG_PLEASE_SPECIFY,
 LANG_OR,
 LANG_WDS_SECURITY_SETTINGS,
 LANG_THIS_PAGE_ALLOWS_YOU_SETUP_THE_WLAN_SECURITY_FOR_WDS_WHEN_ENABLED_YOU_MUST_MAKE_SURE_EACH_WDS_DEVICE_HAS_ADOPTED_THE_SAME_ENCRYPTION_ALGORITHM_AND_KEY,
 LANG_THIS_TABLE_SHOWS_THE_MAC_ADDRESS_TRANSMISSION_RECEPTION_PACKET_COUNTERS_AND_STATE_INFORMATION_FOR_EACH_CONFIGURED_WDS_AP,
 LANG_TX_ERRORS,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_STATIC_IP_BASE_ON_MAC_ADDRESS_YOU_CAN_ASSIGN_DELETE_THE_STATIC_IP_THE_HOST_MAC_ADDRESS_PLEASE_INPUT_A_STRING_WITH_HEX_NUMBER_SUCH_AS,
 LANG_ASSIGNED_IP_ADDRESS,
 LANG_DELETE_ASSIGNED_IP,
 LANG_SYSTEM_DATE_AND_TIME_MODIFICATION,
 LANG_SET_THE_SYSTEM_DATE_AND_TIME,
 LANG_SUBMIT,
 LANG_THIS_PAGE_IS_USED_TO_ADD_USER_ACCOUNT_TO_ACCESS_THE_WEB_SERVER_OF_THE_DEVICE_EMPTY_USER_NAME_OR_PASSWORD_IS_NOT_ALLOWED,
 LANG_PRIVILEGE,
 LANG_USER,
 LANG_SUPPORT,
 LANG_ACTIVE_DHCP_CLIENTS,
 LANG_THIS_TABLE_SHOWS_THE_ASSIGNED_IP_ADDRESS_MAC_ADDRESS_AND_TIME_EXPIRED_FOR_EACH_DHCP_LEASED_CLIENT,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_IP_ADDRESS_RANGE_BASED_ON_DEVICE_TYPE,
 LANG_START_ADDRESS,
 LANG_END_ADDRESS,
 LANG_OPTION60,
 LANG_DEVICE_TYPE,
 LANG_PC,
 LANG_CAMERA,
 LANG_HGW,
 LANG_STB,
 LANG_PHONE,
 LANG_UNKNOWN,
 LANG_RESERVED_OPTION,
 LANG_OPTION_STRING,
 LANG_DNS1,
 LANG_DNS2,
 LANG_LOG_LEVEL,
 LANG_DISPLAY_LEVEL,
 LANG_SAVE_LOG_TO_FILE,
 LANG_SAVE,
 LANG_CLEAR_LOG,
 LANG_FACILITY,
 LANG_LEVEL,
 LANG_MESSAGE,
 LANG_IP_PRECEDENCE_PRIORITY_SETTINGS,
 LANG_THIS_PAGE_IS_USED_TO_CONFIG_IP_PRECEDENCE_PRIORITY,
 LANG_IP_PRECEDENCE_RULE,
 LANG_BRIDGE_FORWARDING_DATABASE,
 LANG_THIS_TABLE_SHOWS_A_LIST_OF_LEARNED_MAC_ADDRESSES_FOR_THIS_BRIDGE,
 LANG_IS_LOCAL,
 LANG_AGEING_TIMER,
 LANG_DOS_DENIAL_OF_SERVICE_ATTACK_WHICH_IS_LAUNCHED_BY_HACKER_AIMS_TO_PREVENT_LEGAL_USER_FROM_TAKING_NORMAL_SERVICES_IN_THIS_PAGE_YOU_CAN_CONFIGURE_TO_PREVENT_SOME_KINDS_OF_DOS_ATTACK,
 LANG_ENABLE_DOS_BLOCK,
 LANG_SELECT_ALL,
 LANG_CLEAR,
 LANG_ENABLE_SOURCE_IP_BLOCKING,
 LANG_BLOCK_INTERVAL,
 LANG_CATEGORY,
 LANG_AVAILABLE_RULES,
 LANG_APPLIED_RULES,
 LANG_PORT_FORWARDING_ADVANCE_TABLE,
 LANG_IP_ROUTE_TABLE,
 LANG_THIS_TABLE_SHOWS_A_LIST_OF_DESTINATION_ROUTES_COMMONLY_ACCESSED_BY_YOUR_NETWORK,
 LANG_PAGE_DESC_CHANGE_TO_UPGRADE_MODE,
 LANG_PAGE_DESC_WAIT_FOT_UPDATE_INFO,
 LANG_GPON_STATUS,
 LANG_ONU_STATE,
 LANG_TELNET,
 LANG_FTP,
 LANG_TFTP,
 LANG_HTTP,
 LANG_PING,
 LANG_RX_PKT,
 LANG_RX_ERR,
 LANG_RX_DROP,
 LANG_TX_PKT,
 LANG_TX_ERR,
 LANG_TX_DROP,
 LANG_PON_STATISTICS,
 LANG_BYTES_SENT,
 LANG_BYTES_RECEIVED,
 LANG_PACKETS_SENT,
 LANG_PACKETS_RECEIVED,
 LANG_UNICAST_PACKETS_SENT,
 LANG_UNICAST_PACKETS_RECEIVED,
 LANG_MULTICAST_PACKETS_SENT,
 LANG_MULTICAST_PACKETS_RECEIVED,
 LANG_BROADCAST_PACKETS_SENT,
 LANG_BROADCAST_PACKETS_RECEIVED,
 LANG_FEC_ERRORS,
 LANG_HEC_ERRORS,
 LANG_PACKETS_DROPPED,
 LANG_PAUSE_PACKETS_SENT,
 LANG_PAUSE_PACKETS_RECEIVED,
 LANG_UPPER,
 LANG_LOWER,
 LANG_WPA2_MIXED,
 LANG_DEFAULT_PROXY,
 LANG_SELECT_DEFAULT_PROXY,
 LANG_PROXY,
 LANG_DISPLAY_NAME,
 LANG_NUMBER,
 LANG_LOGIN_ID,
 LANG_PROXY_ADDR,
 LANG_PROXY_PORT,
 LANG_SIP_DOMAIN,
 LANG_REG_EXPIRE_SEC,
 LANG_OUTBOUND_PROXY,
 LANG_OUTBOUND_PROXY_ADDR,
 LANG_OUTBOUND_PROXY_PORT,
 LANG_ENABLE_SESSION_TIMER,
 LANG_SESSION_EXPIRE_SEC,
 LANG_REGISTER_STATUS,
 LANG_SIP_ADVANCED,
 LANG_SIP_PORT,
 LANG_MEDIA_PORT,
 LANG_DMTF_RELAY,
 LANG_DTMF_RFC2833_PAYLOAD_TYPE,
 LANG_DTMF_RFC2833_PACKET_INTERVAL,
 LANG_USE_DTMF_RFC2833_PT_AS_FAX_MODEM_RFC2833_PT,
 LANG_FAX_MODEM_RFC2833_PAYLOAD_TYPE,
 LANG_FAX_MODEM_RFC2833_PACKET_INTERVAL,
 LANG_SIP_INFO_DURATION_MS,
 LANG_CALL_WAITING,
 LANG_CALL_WAITING_CALLER_ID,
 LANG_REJECT_DIRECT_IP_CALL,
 LANG_SEND_CALLER_ID_HIDDEN,
 LANG_MSEC,
 LANG_MUST_BE_MULTIPLE_OF_10MSEC,
 LANG_FORWARD_MODE,
 LANG_IMMEDIATE_FORWARD_TO,
 LANG_IMMEDIATE_NUMBER,
 LANG_BUSY_FORWARD_TO,
 LANG_BUSY_NUMBER,
 LANG_NO_ANSWER_FORWARD_TO,
 LANG_NO_ANSWER_NUMBER,
 LANG_NO_ANSWER_TIME_SEC,
 LANG_SPEED_DIAL,
 LANG_POSITION,
 LANG_PHONE_NUMBER,
 LANG_REMOVE_SELECTED,
 LANG_REMOVE_ALL,
 LANG_ABBREVIATED_DIAL,
 LANG_ABBREVIATED_NAME,
 LANG_DIAL_PLAN,
 LANG_ENABLE_DIALPLAN,
 LANG_PSTN_ROUTING_PREFIX,
 LANG_PREFIX_LIST_DELIMITER,
 LANG_CODEC,
 LANG_RTP_REDUNDANT,
 LANG_FIRST_PRECEDENCE,
 LANG_PAYLOAD_TYPE,
 LANG_PACKETIZATION,
 LANG_MS,
 LANG_G726_PACKING_ORDER,
 LANG_G723_BIT_RATE,
 LANG_LEFT,
 LANG_RIGHT,
 LANG_QOS,
 LANG_CODEC_TYPE,
 LANG_FAX,
 LANG_FAX_MODEM_DETECTION_MODE,
 LANG_CUSTOMIZE_PARAMETERS,
 LANG_MAX_BUFFER,
 LANG_LOCAL_TCF,
 LANG_REMOTE_TCF,
 LANG_MAX_RATE,
 LANG_ECC_SIGNAL,
 LANG_ECC_DATA,
 LANG_SPOOFING,
 LANG_PACKET_DUPLICATE_NUM,
 LANG_VOIP_SECURITY,
 LANG_MAKE_SECRUITY_CALL,
 LANG_HOT_LINE,
 LANG_USE_HOT_LINE,
 LANG_HOT_LINE_NUMBER,
 LANG_ALWAYS,
 LANG_FROM,
 LANG_TO,
 LANG_OFF_HOOK_PASSWORD,
 LANG_ALARM,
 LANG_JITTER_BUFFER_CONTROL,
 LANG_THRESHOLD,
 LANG_SID_NOISE_LEVEL,
 LANG_DISABLE_CONFIGURATION,
 LANG_FIXED_NOISE_LEVEL,
 LANG_ADJUST_NOISE_LEVEL,
 LANG_VALUE,
 LANG_NOT_CHANGE,
 LANG_MEANS_NO_LIMIT_FOR_MAX,
 LANG_INTERVAL,
 LANG_FAX_MODEM_RFC2833_SUPPORT,
 LANG_ENABLE_FAX_MODEM_RFC2833_RELAY_FOR_TX,
 LANG_ENABLE_FAX_MODEM_INBAND_REMOVAL_FOR_TX,
 LANG_ENABLE_FAX_MODEM_TONE_PLAY_FOR_RX,
 LANG_REQUIRE_LEVEL,
 LANG_MAX_GAIN_UP,
 LANG_MAX_GAIN_DOWN,
 LANG_SPEAKER_AGC,
 LANG_MIC_AGC,
 LANG_CALLER_ID_MODE,
 LANG_FSK_DATE_AND_TIME_SYNC,
 LANG_REVERSE_POLARITY_BEFORE_CALLER_ID,
 LANG_SHORT_RING_BEFORE_CALLER_ID,
 LANG_DUAL_TONE_BEFORE_CALLER_ID,
 LANG_CALLER_ID_PRIOR_FIRST_RING,
 LANG_CALLER_ID_DTMF_START_DIGIT,
 LANG_CALLER_ID_DTMF_END_DIGIT,
 LANG_FLASH_TIME,
 LANG_SPACE,
 LANG_SPEAKER_VOICE_GAIN,
 LANG_MIC_VOICE_GAIN,
 LANG_MUTE,
 LANG_OPTIMIZATION_FACTOR,
 LANG_MIN_DELAY,
 LANG_MAX_DELAY,
 LANG_SELECT_COUNTRY,
 LANG_COUNTRY,
 LANG_SELECT_CUSTOM_TONE,
 LANG_CUSTOM_TONE,
 LANG_TONE_PARAMETERS,
 LANG_TONETYPE,
 LANG_TONECYCLE,
 LANG_CYCLE_NO,
 LANG_0_CYCLE_IS_CONTINUOUS,
 LANG_1_4_FOR_ADDITIVE_MODULATED_SUCC_SUCC_ADD_1_32_FOR_FOUR_FREQ_2_FOR_STEP_INC_TWO_STEP,
 LANG_TONENUM,
 LANG_FREQ,
 LANG_FREQ2_IS_SUPPORTED_WHEN_SUCC_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE,
 LANG_FREQ3_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE,
 LANG_GAIN,
 LANG_GAIN2_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE,
 LANG_GAIN3_IS_SUPPORTED_WHEN_FOUR_FREQ_STEP_INC_TWO_STEP_TONE_TYPE,
 LANG_RING_CADENCE_DETECTION_SETTING,
 LANG_CADENCE,
 LANG_SELECT_CADENCE,
 LANG_CUSTOM_CADENCE,
 LANG_CADENCE_ON,
 LANG_CADENCE_OFF,
 LANG_FUNCTION_KEY,
 LANG_MUST_BE,
 LANG_SWITCH_TO_PSTN,
 LANG_CALL_TRANSFER,
 LANG_DEFAULT,
 LANG_DIAL_OPTION,
 LANG_AUTO_DIAL_TIME,
 LANG_DIAL_OUT_BY_HASH_KEY,
 LANG_0_IS_DISABLE,
 LANG_PSTN_RELAY,
 LANG_AUTO_BYPASS_RELAY,
 LANG_WARNING_TONE,
 LANG_OFF_HOOK_ALARM,
 LANG_OFF_HOOK_ALARM_TIME,
 LANG_VOIP_TO_PSTN,
 LANG_ONE_STAGE_DIALING,
 LANG_TWO_STAGE_DIALING,
 LANG_PSTN_TO_VOIP,
 LANG_CALLER_ID_AUTO_DETECTION,
 LANG_CALLER_ID_DETECTION_MODE,
 LANG_FXS_PULSE_DIAL_DETECTION,
 LANG_INTERDIGIT_PAUSE_DURATION,
 LANG_FXO_PULSE_DIAL_GENERATION,
 LANG_PULSE_PER_SECOND,
 LANG_1_99_MSEC_FOR_10PPS_1_49_MSEC_FOR_20PPS,
 LANG_SIP_SETTING,
 LANG_SIP_PRACK,
 LANG_SIP_SERVER_RENDUNDACY,
 LANG_SIP_CLIR_ANONYMOUSE_FROM_HEADER,
 LANG_NON_SIP_INBOX_CALL,
 LANG_HOOK_FLASH_RELAY_SETTING,
 LANG_SIP_OPTIONS,
 LANG_OPTIONS_INTERVAL_TIME,
 LANG_MAKE_DURATION,
 LANG_DSCP_FLAG,
 LANG_SIP_DSCP,
 LANG_RTP_DSCP,
 LANG_EXPORT,
 LANG_IMPORT,
 LANG_OMCI,
 LANG_PAGE_DESC_WAIT_INFO,
 LANG_VLAN_SETTINGS,
 LANG_PAGE_DESC_CONFIGURE_VLAN_SETTINGS,
 LANG_TRANSPARENT_MODE,
 LANG_TAGGING_MODE,
 LANG_REMOTE_ACCESS_MODE,
 LANG_SPECIAL_CASE_MODE,
 LANG_VLAN_PRIORITY,
 LANG_ENABLE_BRIDGE,
 LANG_BRIDGE_MODE,
 LANG_ENABLE_PPPOE_PROXY,
 LANG_MAX_PROXY_USER,
 LANG_DHCP_OPTION_SETTINGS,
 LANG_ENABLE_DHCP_OPTION_60,
 LANG_VENDOR_ID,
 LANG_ENABLE_DHCP_OPTION_61,
 LANG_IAID,
 LANG_DUID,
 LANG_LINK_LAYER_ADDRESS_PLUSE_TIME,
 LANG_ENTERPRISE_NUMBER_AND_IDENTIFIER,
 LANG_ENTERPRISE_NUMBER,
 LANG_IDENTIFIER,
 LANG_LINKLAYER_ADDRESS,
 LANG_ENABLE_DHCP_OPTION_125,
 LANG_MANUFACTURER_OUI,
 LANG_PRODUCT_CLASS,
 LANG_MODEL_NAME,
 LANG_PORT_MASK,
 LANG_POOL,
 LANG_NAS_IDENTIFIER,
 LANG_128_BIT_KEY_PASSPHRASE,
 LANG_INDEX,
 LANG_LLID,
 LANG_MAC,
 LANG_TR064,
 LANG_FLAGS,
 LANG_REF,
 LANG_USE,
 LANG_BUNDLE_INSTALLATION,
 LANG_THIS_PAGE_ALLOWS_YOU_TO_INSTALL_NEW_BUNDLE,
 LANG_INSTALL_BUNDLE_FROM_FILE,
 LANG_ETHERNETOAM_Y_1731,
 LANG_HERE_YOU_CAN_CONFIGURE_ETHERNETOAM_Y_1731,
 LANG_INITIATING_END_SCRIPT_CONTENT,
 LANG_BUNDLE_MANAGEMENT,
 LANG_THIS_PAGE_IS_USED_TO_MANAGE_OSGI_BUNDLES,
 LANG_PAGE_DESC_ENTRIES_FILTER_SETTING,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_STATIC_IP_BASE_ON_MAC_ADDRESS_YOU_CAN_ASSIGN_DELETE_THE_STATIC_IP_THE_HOST_MAC_ADDRESS_PLEASE_INPUT_A_STRING_WITH_HEX_NUMBER_SUCH_AS_00_D0_59_C6_12_43,
 LANG_TABLE_2,
 LANG_WAN_MODE_SELECTION,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_WHICH_WAN_TO_USE_OF_YOUR_ROUTER,
 LANG_THIS_PAGE_LET_USER_TO_CONFIG_SAMBA,
 LANG_SERVER_STRING,
 LANG_F4_SEGMENT,
 LANG_F4_END_TO_END,
 LANG_USERNAME,
 LANG_FAST_BSS_TRANSITION_802_11R,
 LANG_THIS_PAGE_ALLOWS_YOU_TO_CHANGE_THE_SETTING_FOR_FAST_BSS_TRANSITION_802_11R,
 LANG_IEEE_802_11R,
 LANG_MOBILITY_DOMAIN_ID,
 LANG_SUPPORT_OVER_DS,
 LANG_SUPPORT_RESOURCE_REQUEST,
 LANG_KEY_EXPIRATION_TIMEOUT,
 LANG_REASSOCIATION_TIMEOUT,
 LANG_SUPPORT_KEY_PUSH,
 LANG_KEY_HOLDER_CONFIGURATION,
 LANG_CURRENT_KEY_HOLDER_INFORMATION,
 LANG_MAC_FILTERING_FOR_BRIDGE_MODE,
 LANG_CFM_802_1AG_STATUS,
 LANG_REMOTE_MEP,
 LANG_MD_NAME,
 LANG_MA_NAME,
 LANG_REMOTE_MEP_ID,
 LANG_SRC_MAC_ADDRESS,
 LANG_ETHERY_TYPE,
 LANG_IP_PROTOCOL,
 LANG_DHCP_OPTIONS,
 LANG_OPTION_60,
 LANG_OPTION_61,
 LANG_OPTION_125,
 LANG_DHCP_OPTIONS_61,
 LANG_DUID_TYPE,
 LANG_DUID_LLT,
 LANG_DUID_EN,
 LANG_DUID_LL,
 LANG_HARDWARE_TYPE,
 LANG_DHCP_OPTIONS_125,
 LANG_MAXIMUM,
 LANG_CFM_802_1AG_ACTION,
 LANG_PROCESSING,
 LANG_READY,
 LANG_PLEASE_SELECT_ONE_OF_FOLLOWING_ENTRY,
 LANG_MD_LEVEL,
 LANG_MEP_ID,
 LANG_CCM_STATUS,
 LANG_SEND_LBM,
 LANG_SEND_LTM,
 LANG_USE_MULTICAST,
 LANG_DESTINATION_MAC_ADDRESS,
 LANG_LBM_COUNT,
 LANG_RESULT,
 LANG_MULTI_LANGUAL_SETTINGS,
 LANG_PAGE_DESC_MULTI_LANGUAL,
 LANG_LANGUAGE_SELECT,
 LANG_UPDATE_SELECTED_LANGUAGE,
 LANG_EPON_SETTINGS,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAMETERS_FOR_EPON_NETWORK_ACCESS,
 LANG_LLID_MAC_MAPPING_TABLE,
 LANG_BSSID,
 LANG_SYSTEM_INITIAL_SCRIPT,
 LANG_SET_SCRIPTS_THAT_ARE_EXECUTED_IN_SYSTEM_INITIALING,
 LANG_SHOW_SCRIPT_CONTENT,
 LANG_DELETE_SCRIPT,
 LANG_FRAMEWORK_INFO,
 LANG_THIS_PAGE_SHOWS_THE_OSGI_FRAMWEORK_OF_THE_DEVICE,
 LANG_OSGI_FRAMEWORK_INFORMATION,
 LANG_FRAMEWORK_NAME,
 LANG_FRAMEWORK_VERSION,
 LANG_FRAMEWORK_STATUS,
 LANG_TUNNEL_AUTH,
 LANG_PPP_AUTH,
 LANG_WIRELESS_WAN,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_THE_PARAPETERS_FOR_WIRELESS_WAN_OF_YOUR_ROUTER,
 LANG_INITIATING_START_SCRIPT_CONTENT,
 LANG_NEXT_STEP,
 LANG_BACK,
 LANG_CFM_802_1AG_CONFIGURATION,
 LANG_THIS_PAGE_IS_USED_TO_CONFIGURE_IEEE_802_1AG_WHICH_IS_ALSO_KNOWN_AS_CONNECTIVITY_FAULT_MAMAGMENT,
 LANG_CCM_INTERVAL,
 LANG_CFM_INSTANCE_TABLE,
 LANG_ENABLE_CCM,
 LANG_VOIP_CALLHISTORY,
 LANG_FAX_DETECTION_MODE,
 LANG_INSTALL,
 LANG_DYNAMIC_DNS,
 LANG_DNS_SERVER,
 LANG_RADVD,
 LANG_DHCPV6,
 LANG_IPV6_ROUTING,
 LANG_IP_PORT_FILTERING,
 LANG_ALG,
 LANG_URL_BLOCKING,
 LANG_DMZ,
 LANG_NAT_RULE_CONFIGURATION,
 LANG_QOS_CLASSIFICATION,
 LANG_QOS_POLICY,
 LANG_PPTP,
 LANG_L2TP,
 LANG_IPIP,
 LANG_IPSEC,
 LANG_BASIC_SETTINGS,
 LANG_SECURITY,
 LANG_FAST_ROAMING,
 LANG_ACCESS_CONTROL,
 LANG_WDS,
 LANG_SITE_SURVEY,
 LANG_WPS,
 LANG_WAPI,
 LANG_PON,
 LANG_LAN_PORT,
 LANG_WLAN0_5GHZ,
 LANG_WLAN1_5GHZ,
 LANG_SERVICES,
 LANG_ADVANCE,
 LANG_STATISTICS,
 LANG_OSGI,
 LANG_WLAN0_2_4GHZ,
 LANG_WLAN1_2_4GHZ,
 LANG_FON_SPOT_SETTINGS,
 LANG_PON_WAN,
 LANG_PTM_WAN,
 LANG_VTUO_SETTINGS,
 LANG_DHCP,
 LANG_VLAN_ON_LAN,
 LANG_BRIDGE_GROUPING,
 LANG_DNS,
 LANG_FIREWALL,
 LANG_DMS,
 LANG_SAMBA,
 LANG_SNMP,
 LANG_DIFFSERV,
 LANG_IP_QOS,
 LANG_LINK_MODE,
 LANG_PRINT_SERVER,
 LANG_OTHERS,
 LANG_ATM_LOOPBACK,
 LANG_DSL_TONE,
 LANG_DSL_SLAVE_TONE,
 LANG_ADSL_CONNECTION,
 LANG_802_1AG,
 LANG_INIT_SCRIPTS,
 LANG_COMMIT_REBOOT,
 LANG_MULTI_LINGUAL_SETTINGS,
 LANG_BACKUP_RESTORE,
 LANG_DOS,
 LANG_ACL,
 LANG_TR_069,
 LANG_ETH_OAM,
 LANG_DSL,
 LANG_VTUO_DSL,
 LANG_DSL_SLAVE,
 LANG_PORT1,
 LANG_PORT2,
 LANG_FXO,
 LANG_RING,
 LANG_NETWORK,
 LANG_ATM_WAN,
 LANG_BAND_MODE,
 LANG_ACL_CONFIG,
 LANG_WIRELESS,
 LANG_SAVE_MULTI_LANG_ERR,
 LANG_RE_LOGIN_NOTICE,
 LANG_AFRICA_ABIDJAN,
 LANG_AFRICA_ACCRA,
 LANG_AFRICA_ADDIS_ABABA,
 LANG_AFRICA_ALGIERS,
 LANG_AFRICA_ASMARA,
 LANG_AFRICA_BAMAKO,
 LANG_AFRICA_BANGUI,
 LANG_AFRICA_BANJUL,
 LANG_AFRICA_BISSAU,
 LANG_AFRICA_BLANTYRE,
 LANG_AFRICA_BRAZZAVILLE,
 LANG_AFRICA_BUJUMBURA,
 LANG_AFRICA_CASABLANCA,
 LANG_AFRICA_CEUTA,
 LANG_AFRICA_CONAKRY,
 LANG_AFRICA_DAKAR,
 LANG_AFRICA_DAR_ES_SALAAM,
 LANG_AFRICA_DJIBOUTI,
 LANG_AFRICA_DOUALA,
 LANG_AFRICA_EL_AAIUN,
 LANG_AFRICA_FREETOWN,
 LANG_AFRICA_GABORONE,
 LANG_AFRICA_HARARE,
 LANG_AFRICA_JOHANNESBURG,
 LANG_AFRICA_KAMPALA,
 LANG_AFRICA_KHARTOUM,
 LANG_AFRICA_KIGALI,
 LANG_AFRICA_KINSHASA,
 LANG_AFRICA_LAGOS,
 LANG_AFRICA_LIBREVILLE,
 LANG_AFRICA_LOME,
 LANG_AFRICA_LUANDA,
 LANG_AFRICA_LUBUMBASHI,
 LANG_AFRICA_LUSAKA,
 LANG_AFRICA_MALABO,
 LANG_AFRICA_MAPUTO,
 LANG_AFRICA_MASERU,
 LANG_AFRICA_MBABANE,
 LANG_AFRICA_MOGADISHU,
 LANG_AFRICA_MONROVIA,
 LANG_AFRICA_NAIROBI,
 LANG_AFRICA_NDJAMENA,
 LANG_AFRICA_NIAMEY,
 LANG_AFRICA_NOUAKCHOTT,
 LANG_AFRICA_OUAGADOUGOU,
 LANG_AFRICA_PORTO_NOVO,
 LANG_AFRICA_SAO_TOME,
 LANG_AFRICA_TRIPOLI,
 LANG_AFRICA_TUNIS,
 LANG_AFRICA_WINDHOEK,
 LANG_AMERICA_ADAK,
 LANG_AMERICA_ANCHORAGE,
 LANG_AMERICA_ANGUILLA,
 LANG_AMERICA_ANTIGUA,
 LANG_AMERICA_ARAGUAINA,
 LANG_AMERICA_ARGENTINA_BUENOS_AIRES,
 LANG_AMERICA_ARGENTINA_CATAMARCA,
 LANG_AMERICA_ARGENTINA_CORDOBA,
 LANG_AMERICA_ARGENTINA_JUJUY,
 LANG_AMERICA_ARGENTINA_LA_RIOJA,
 LANG_AMERICA_ARGENTINA_MENDOZA,
 LANG_AMERICA_ARGENTINA_RIO_GALLEGOS,
 LANG_AMERICA_ARGENTINA_SALTA,
 LANG_AMERICA_ARGENTINA_SAN_JUAN,
 LANG_AMERICA_ARGENTINA_TUCUMAN,
 LANG_AMERICA_ARGENTINA_USHUAIA,
 LANG_AMERICA_ARUBA,
 LANG_AMERICA_ASUNCION,
 LANG_AMERICA_ATIKOKAN,
 LANG_AMERICA_BAHIA,
 LANG_AMERICA_BARBADOS,
 LANG_AMERICA_BELEM,
 LANG_AMERICA_BELIZE,
 LANG_AMERICA_BLANC_SABLON,
 LANG_AMERICA_BOA_VISTA,
 LANG_AMERICA_BOGOTA,
 LANG_AMERICA_BOISE,
 LANG_AMERICA_CAMBRIDGE_BAY,
 LANG_AMERICA_CAMPO_GRANDE,
 LANG_AMERICA_CANCUN,
 LANG_AMERICA_CARACAS,
 LANG_AMERICA_CAYENNE,
 LANG_AMERICA_CAYMAN,
 LANG_AMERICA_CHICAGO,
 LANG_AMERICA_CHIHUAHUA,
 LANG_AMERICA_COSTA_RICA,
 LANG_AMERICA_CUIABA,
 LANG_AMERICA_CURACAO,
 LANG_AMERICA_DANMARKSHAVN,
 LANG_AMERICA_DAWSON,
 LANG_AMERICA_DAWSON_CREEK,
 LANG_AMERICA_DENVER,
 LANG_AMERICA_DETROIT,
 LANG_AMERICA_DOMINICA,
 LANG_AMERICA_EDMONTON,
 LANG_AMERICA_EIRUNEPE,
 LANG_AMERICA_EL_SALVADOR,
 LANG_AMERICA_FORTALEZA,
 LANG_AMERICA_GLACE_BAY,
 LANG_AMERICA_GOOSE_BAY,
 LANG_AMERICA_GRAND_TURK,
 LANG_AMERICA_GRENADA,
 LANG_AMERICA_GUADELOUPE,
 LANG_AMERICA_GUATEMALA,
 LANG_AMERICA_GUAYAQUIL,
 LANG_AMERICA_GUYANA,
 LANG_AMERICA_HALIFAX,
 LANG_AMERICA_HAVANA,
 LANG_AMERICA_HERMOSILLO,
 LANG_AMERICA_INDIANA_INDIANAPOLIS,
 LANG_AMERICA_INDIANA_KNOX,
 LANG_AMERICA_INDIANA_MARENGO,
 LANG_AMERICA_INDIANA_PETERSBURG,
 LANG_AMERICA_INDIANA_TELL_CITY,
 LANG_AMERICA_INDIANA_VEVAY,
 LANG_AMERICA_INDIANA_VINCENNES,
 LANG_AMERICA_INDIANA_WINAMAC,
 LANG_AMERICA_INUVIK,
 LANG_AMERICA_IQALUIT,
 LANG_AMERICA_JAMAICA,
 LANG_AMERICA_JUNEAU,
 LANG_AMERICA_KENTUCKY_LOUISVILLE,
 LANG_AMERICA_KENTUCKY_MONTICELLO,
 LANG_AMERICA_LA_PAZ,
 LANG_AMERICA_LIMA,
 LANG_AMERICA_LOS_ANGELES,
 LANG_AMERICA_MACEIO,
 LANG_AMERICA_MANAGUA,
 LANG_AMERICA_MANAUS,
 LANG_AMERICA_MARIGOT,
 LANG_AMERICA_MARTINIQUE,
 LANG_AMERICA_MATAMOROS,
 LANG_AMERICA_MAZATLAN,
 LANG_AMERICA_MENOMINEE,
 LANG_AMERICA_MERIDA,
 LANG_AMERICA_MEXICO_CITY,
 LANG_AMERICA_MIQUELON,
 LANG_AMERICA_MONCTON,
 LANG_AMERICA_MONTERREY,
 LANG_AMERICA_MONTEVIDEO,
 LANG_AMERICA_MONTREAL,
 LANG_AMERICA_MONTSERRAT,
 LANG_AMERICA_NASSAU,
 LANG_AMERICA_NEW_YORK,
 LANG_AMERICA_NIPIGON,
 LANG_AMERICA_NOME,
 LANG_AMERICA_NORONHA,
 LANG_AMERICA_NORTH_DAKOTA_CENTER,
 LANG_AMERICA_NORTH_DAKOTA_NEW_SALEM,
 LANG_AMERICA_OJINAGA,
 LANG_AMERICA_PANAMA,
 LANG_AMERICA_PANGNIRTUNG,
 LANG_AMERICA_PARAMARIBO,
 LANG_AMERICA_PHOENIX,
 LANG_AMERICA_PORT_OF_SPAIN,
 LANG_AMERICA_PORT_AU_PRINCE,
 LANG_AMERICA_PORTO_VELHO,
 LANG_AMERICA_PUERTO_RICO,
 LANG_AMERICA_RAINY_RIVER,
 LANG_AMERICA_RANKIN_INLET,
 LANG_AMERICA_RECIFE,
 LANG_AMERICA_REGINA,
 LANG_AMERICA_RIO_BRANCO,
 LANG_AMERICA_SANTA_ISABEL,
 LANG_AMERICA_SANTAREM,
 LANG_AMERICA_SANTO_DOMINGO,
 LANG_AMERICA_SAO_PAULO,
 LANG_AMERICA_SCORESBYSUND,
 LANG_AMERICA_SHIPROCK,
 LANG_AMERICA_ST_BARTHELEMY,
 LANG_AMERICA_ST_JOHNS,
 LANG_AMERICA_ST_KITTS,
 LANG_AMERICA_ST_LUCIA,
 LANG_AMERICA_ST_THOMAS,
 LANG_AMERICA_ST_VINCENT,
 LANG_AMERICA_SWIFT_CURRENT,
 LANG_AMERICA_TEGUCIGALPA,
 LANG_AMERICA_THULE,
 LANG_AMERICA_THUNDER_BAY,
 LANG_AMERICA_TIJUANA,
 LANG_AMERICA_TORONTO,
 LANG_AMERICA_TORTOLA,
 LANG_AMERICA_VANCOUVER,
 LANG_AMERICA_WHITEHORSE,
 LANG_AMERICA_WINNIPEG,
 LANG_AMERICA_YAKUTAT,
 LANG_AMERICA_YELLOWKNIFE,
 LANG_ANTARCTICA_CASEY,
 LANG_ANTARCTICA_DAVIS,
 LANG_ANTARCTICA_DUMONTDURVILLE,
 LANG_ANTARCTICA_MACQUARIE,
 LANG_ANTARCTICA_MAWSON,
 LANG_ANTARCTICA_MCMURDO,
 LANG_ANTARCTICA_ROTHERA,
 LANG_ANTARCTICA_SOUTH_POLE,
 LANG_ANTARCTICA_SYOWA,
 LANG_ANTARCTICA_VOSTOK,
 LANG_ARCTIC_LONGYEARBYEN,
 LANG_ASIA_ADEN,
 LANG_ASIA_ALMATY,
 LANG_ASIA_ANADYR,
 LANG_ASIA_AQTAU,
 LANG_ASIA_AQTOBE,
 LANG_ASIA_ASHGABAT,
 LANG_ASIA_BAGHDAD,
 LANG_ASIA_BAHRAIN,
 LANG_ASIA_BAKU,
 LANG_ASIA_BANGKOK,
 LANG_ASIA_BEIRUT,
 LANG_ASIA_BISHKEK,
 LANG_ASIA_BRUNEI,
 LANG_ASIA_CHOIBALSAN,
 LANG_ASIA_CHONGQING,
 LANG_ASIA_COLOMBO,
 LANG_ASIA_DAMASCUS,
 LANG_ASIA_DHAKA,
 LANG_ASIA_DILI,
 LANG_ASIA_DUBAI,
 LANG_ASIA_DUSHANBE,
 LANG_ASIA_GAZA,
 LANG_ASIA_HARBIN,
 LANG_ASIA_HO_CHI_MINH,
 LANG_ASIA_HONG_KONG,
 LANG_ASIA_HOVD,
 LANG_ASIA_IRKUTSK,
 LANG_ASIA_JAKARTA,
 LANG_ASIA_JAYAPURA,
 LANG_ASIA_KABUL,
 LANG_ASIA_KAMCHATKA,
 LANG_ASIA_KARACHI,
 LANG_ASIA_KASHGAR,
 LANG_ASIA_KATHMANDU,
 LANG_ASIA_KOLKATA,
 LANG_ASIA_KRASNOYARSK,
 LANG_ASIA_KUALA_LUMPUR,
 LANG_ASIA_KUCHING,
 LANG_ASIA_KUWAIT,
 LANG_ASIA_MACAU,
 LANG_ASIA_MAGADAN,
 LANG_ASIA_MAKASSAR,
 LANG_ASIA_MANILA,
 LANG_ASIA_MUSCAT,
 LANG_ASIA_NICOSIA,
 LANG_ASIA_NOVOKUZNETSK,
 LANG_ASIA_NOVOSIBIRSK,
 LANG_ASIA_OMSK,
 LANG_ASIA_ORAL,
 LANG_ASIA_PHNOM_PENH,
 LANG_ASIA_PONTIANAK,
 LANG_ASIA_PYONGYANG,
 LANG_ASIA_QATAR,
 LANG_ASIA_QYZYLORDA,
 LANG_ASIA_RANGOON,
 LANG_ASIA_RIYADH,
 LANG_ASIA_SAKHALIN,
 LANG_ASIA_SAMARKAND,
 LANG_ASIA_SEOUL,
 LANG_ASIA_SHANGHAI,
 LANG_ASIA_SINGAPORE,
 LANG_ASIA_TAIPEI,
 LANG_ASIA_TASHKENT,
 LANG_ASIA_TBILISI,
 LANG_ASIA_THIMPHU,
 LANG_ASIA_TOKYO,
 LANG_ASIA_ULAANBAATAR,
 LANG_ASIA_URUMQI,
 LANG_ASIA_VIENTIANE,
 LANG_ASIA_VLADIVOSTOK,
 LANG_ASIA_YAKUTSK,
 LANG_ASIA_YEKATERINBURG,
 LANG_ASIA_YEREVAN,
 LANG_ATLANTIC_AZORES,
 LANG_ATLANTIC_BERMUDA,
 LANG_ATLANTIC_CANARY,
 LANG_ATLANTIC_CAPE_VERDE,
 LANG_ATLANTIC_FAROE,
 LANG_ATLANTIC_MADEIRA,
 LANG_ATLANTIC_REYKJAVIK,
 LANG_ATLANTIC_SOUTH_GEORGIA,
 LANG_ATLANTIC_ST_HELENA,
 LANG_ATLANTIC_STANLEY,
 LANG_AUSTRALIA_ADELAIDE,
 LANG_AUSTRALIA_BRISBANE,
 LANG_AUSTRALIA_BROKEN_HILL,
 LANG_AUSTRALIA_CURRIE,
 LANG_AUSTRALIA_DARWIN,
 LANG_AUSTRALIA_EUCLA,
 LANG_AUSTRALIA_HOBART,
 LANG_AUSTRALIA_LINDEMAN,
 LANG_AUSTRALIA_LORD_HOWE,
 LANG_AUSTRALIA_MELBOURNE,
 LANG_AUSTRALIA_PERTH,
 LANG_AUSTRALIA_SYDNEY,
 LANG_EUROPE_AMSTERDAM,
 LANG_EUROPE_ANDORRA,
 LANG_EUROPE_ATHENS,
 LANG_EUROPE_BELGRADE,
 LANG_EUROPE_BERLIN,
 LANG_EUROPE_BRATISLAVA,
 LANG_EUROPE_BRUSSELS,
 LANG_EUROPE_BUCHAREST,
 LANG_EUROPE_BUDAPEST,
 LANG_EUROPE_CHISINAU,
 LANG_EUROPE_COPENHAGEN,
 LANG_EUROPE_DUBLIN,
 LANG_EUROPE_GIBRALTAR,
 LANG_EUROPE_GUERNSEY,
 LANG_EUROPE_HELSINKI,
 LANG_EUROPE_ISLE_OF_MAN,
 LANG_EUROPE_ISTANBUL,
 LANG_EUROPE_JERSEY,
 LANG_EUROPE_KALININGRAD,
 LANG_EUROPE_KIEV,
 LANG_EUROPE_LISBON,
 LANG_EUROPE_LJUBLJANA,
 LANG_EUROPE_LONDON,
 LANG_EUROPE_LUXEMBOURG,
 LANG_EUROPE_MADRID,
 LANG_EUROPE_MALTA,
 LANG_EUROPE_MARIEHAMN,
 LANG_EUROPE_MINSK,
 LANG_EUROPE_MONACO,
 LANG_EUROPE_MOSCOW,
 LANG_EUROPE_OSLO,
 LANG_EUROPE_PARIS,
 LANG_EUROPE_PODGORICA,
 LANG_EUROPE_PRAGUE,
 LANG_EUROPE_RIGA,
 LANG_EUROPE_ROME,
 LANG_EUROPE_SAMARA,
 LANG_EUROPE_SAN_MARINO,
 LANG_EUROPE_SARAJEVO,
 LANG_EUROPE_SIMFEROPOL,
 LANG_EUROPE_SKOPJE,
 LANG_EUROPE_SOFIA,
 LANG_EUROPE_STOCKHOLM,
 LANG_EUROPE_TALLINN,
 LANG_EUROPE_TIRANE,
 LANG_EUROPE_UZHGOROD,
 LANG_EUROPE_VADUZ,
 LANG_EUROPE_VATICAN,
 LANG_EUROPE_VIENNA,
 LANG_EUROPE_VILNIUS,
 LANG_EUROPE_VOLGOGRAD,
 LANG_EUROPE_WARSAW,
 LANG_EUROPE_ZAGREB,
 LANG_EUROPE_ZAPOROZHYE,
 LANG_EUROPE_ZURICH,
 LANG_INDIAN_ANTANANARIVO,
 LANG_INDIAN_CHAGOS,
 LANG_INDIAN_CHRISTMAS,
 LANG_INDIAN_COCOS,
 LANG_INDIAN_COMORO,
 LANG_INDIAN_KERGUELEN,
 LANG_INDIAN_MAHE,
 LANG_INDIAN_MALDIVES,
 LANG_INDIAN_MAURITIUS,
 LANG_INDIAN_MAYOTTE,
 LANG_INDIAN_REUNION,
 LANG_PACIFIC_APIA,
 LANG_PACIFIC_AUCKLAND,
 LANG_PACIFIC_CHATHAM,
 LANG_PACIFIC_EFATE,
 LANG_PACIFIC_ENDERBURY,
 LANG_PACIFIC_FAKAOFO,
 LANG_PACIFIC_FIJI,
 LANG_PACIFIC_FUNAFUTI,
 LANG_PACIFIC_GALAPAGOS,
 LANG_PACIFIC_GAMBIER,
 LANG_PACIFIC_GUADALCANAL,
 LANG_PACIFIC_GUAM,
 LANG_PACIFIC_HONOLULU,
 LANG_PACIFIC_JOHNSTON,
 LANG_PACIFIC_KIRITIMATI,
 LANG_PACIFIC_KOSRAE,
 LANG_PACIFIC_KWAJALEIN,
 LANG_PACIFIC_MAJURO,
 LANG_PACIFIC_MARQUESAS,
 LANG_PACIFIC_MIDWAY,
 LANG_PACIFIC_NAURU,
 LANG_PACIFIC_NIUE,
 LANG_PACIFIC_NORFOLK,
 LANG_PACIFIC_NOUMEA,
 LANG_PACIFIC_PAGO_PAGO,
 LANG_PACIFIC_PALAU,
 LANG_PACIFIC_PITCAIRN,
 LANG_PACIFIC_PONAPE,
 LANG_PACIFIC_PORT_MORESBY,
 LANG_PACIFIC_RAROTONGA,
 LANG_PACIFIC_SAIPAN,
 LANG_PACIFIC_TAHITI,
 LANG_PACIFIC_TARAWA,
 LANG_PACIFIC_TONGATAPU,
 LANG_PACIFIC_TRUK,
 LANG_PACIFIC_WAKE,
 LANG_PACIFIC_WALLIS,
 LANG_LOID_STATUS_INIT,
 LANG_LOID_STATUS_SUCCESS,
 LANG_LOID_STATUS_ERROR,
 LANG_LOID_STATUS_PWDERR,
 LANG_LOID_STATUS_DUPLICATE,
 LANG_ONU_ID,
 LANG_LOID_STATUS,
 LANG_INVALID_IPV4_ADDR_SHOULD_NOT_EMPTY,
 LANG_INVALID_IPV4_ADDR_SHOULD_BE_DECIMAL_NUM,
 LANG_INVALID_IPV4_ADDR,
 LANG_INVALID_IPV4_ADDR_1ST_DIGIT,
 LANG_INVALID_IPV4_ADDR_2ND_DIGIT,
 LANG_INVALID_IPV4_ADDR_3RD_DIGIT,
 LANG_INVALID_IPV4_ADDR_4TH_DIGIT,
 LANG_INVALID_IPV4_SUBNET_SHOULD_NOT_EMPTY,
 LANG_INVALID_IPV4_SUBNET_SHOULD_BE_DECIMAL_NUM,
 LANG_INVALID_IPV4_SUBNET_DIGIT,
 LANG_INVALID_MAC_ADDR_SHOULD_NOT_EMPTY,
 LANG_INVALID_MAC_ADDR_NOT_COMPLETE,
 LANG_INVALID_MAC_ADDR,
 LANG_INVALID_MAC_ADDR_SHOULD_BE,
 LANG_CONFIRM_DELETE_ONE_ENTRY,
 LANG_CONFIRM_DELETE_ALL_ENTRY,
 LANG_CONFIRM_DELETE,
 LANG_INVALID_IPV6_PREFIX,
 LANG_CHANGE_SETTING_SUCCESSFULLY,
 LANG_TINVALID_IP,
 LANG_TINVALID_START_IP,
 LANG_TINVALID_END_IP,
 LANG_TBYTES,
 LANG_TPING_RECV,
 LANG_TPING_STAT,
 LANG_TTRANS_PKT,
 LANG_TRECV_PKT,
 LANG_TNO_CONN,
 LANG_TRECV_CELL_SUC,
 LANG_TRECV_CELL_FAIL,
 LANG_TADSL_DIAG_SUC,
 LANG_TADSL_DIAG_FAIL,
 LANG_TDOWNSTREAM,
 LANG_TUPSTREAM,
 LANG_TTONE_NUM,
 LANG_TLAN_CONN_CHK,
 LANG_TTEST_ETH_CONN,
 LANG_TTEST_ADSL_SYN,
 LANG_TTEST_OAM_F5_SEG,
 LANG_TTEST_OAM_F5_END,
 LANG_TTEST_OAM_F4_SEG,
 LANG_TTEST_OAM_F4_END,
 LANG_TADSL_CONN_CHK,
 LANG_TINT_CONN_CHK,
 LANG_TTEST_PPPS_CONN,
 LANG_TTEST_AUTH,
 LANG_TTEST_ASSIGNED_IP,
 LANG_TPING_PRI_DNSS,
 LANG_TBACK,
 LANG_TPING_DEF_GW,
 LANG_TADSL_DIAG_WAIT,
 LANG_TINVALID_DNS,
 LANG_TBRG_NOT_EXIST,
 LANG_TYES,
 LANG_TNO,
 LANG_TSVR_PORT,
 LANG_TDMZ_ERROR,
 LANG_STRWRONGIP,
 LANG_STRSETIPERROR,
 LANG_STRWRONGMASK,
 LANG_STRSETMASKERROR,
 LANG_STRGETDHCPMODEERROR,
 LANG_STRGETIPERROR,
 LANG_STRGETMASKERROR,
 LANG_STRINVALIDRANGE,
 LANG_STRSETSTARIPERROR,
 LANG_STRSETENDIPERROR,
 LANG_STRSETLEASETIMEERROR,
 LANG_STRSETDOMAINNAMEERROR,
 LANG_STRINVALIDGATEWAYERROR,
 LANG_STRSETGATEWAYERROR,
 LANG_STRSETDHCPMODEERROR,
 LANG_STRINVALIDRANGEPC,
 LANG_STRINVALIDRANGECMR,
 LANG_STRINVALIDRANGESTB,
 LANG_STRINVALIDRANGEPHN,
 LANG_STRINVALIDTYPERANGE,
 LANG_STROVERLAPRANGE,
 LANG_STRINVALIDOPCHADDR,
 LANG_STRINVALIDOPCHPORT,
 LANG_STRSETPCSTARTIPERROR,
 LANG_STRSETPCENDIPERROR,
 LANG_STRSETCMRSTARTIPERROR,
 LANG_STRSETCMRENDIPERROR,
 LANG_STRSETSTBSTARTIPERROR,
 LANG_STRSETSTBENDIPERROR,
 LANG_STRSETPHNSTARTIPERROR,
 LANG_STRSETPHNENDIPERROR,
 LANG_STRDELCHAINERROR,
 LANG_STRMODCHAINERROR,
 LANG_STRINVALDHCPSADDRESS,
 LANG_STRSETDHCPSERROR,
 LANG_STRSTATICIPEXIST,
 LANG_STRCONNECTEXIST,
 LANG_STRMAXVC,
 LANG_STRADDCHAINERROR,
 LANG_STRTABLEFULL,
 LANG_STRUSERNAMETOOLONG,
 LANG_STRUSERNAMEEMPTY,
 LANG_STRPASSTOOLONG,
 LANG_STRPASSEMPTY,
 LANG_STRINVALPPPTYPE,
 LANG_STRDROUTEEXIST,
 LANG_STRACNAME,
 LANG_STRSERVERNAME,
 LANG_STRINVALDHCP,
 LANG_STRINVALIP,
 LANG_STRIPADDRESSERROR,
 LANG_STRINVALGATEWAY,
 LANG_STRGATEWAYIPEMPTY,
 LANG_STRINVALMASK,
 LANG_STRMASKEMPTY,
 LANG_STRMRUERR,
 LANG_STRMAXNUMPPPOE,
 LANG_STRSELECTVC,
 LANG_STRGETCHAINERROR,
 LANG_STRSETCOMMUNITYROERROR,
 LANG_STRSETCOMMUNITYRWERROR,
 LANG_STRINVALTRAPIP,
 LANG_STRSETTRAPIPERROR,
 LANG_STRGETTONEERROR,
 LANG_WARNING_EMPTY_OLD_PASSWORD,
 LANG_WARNING_EMPTY_NEW_PASSWORD,
 LANG_WARNING_EMPTY_CONFIRMED_PASSWORD,
 LANG_WARNING_UNMATCHED_PASSWORD,
 LANG_WARNING_GET_PASSWORD,
 LANG_WARNING_SET_PASSWORD,
 LANG_WARNING_WRONG_PASSWORD,
 LANG_WARNING_WRONG_USER,
 LANG_STRDISBWLANERR,
 LANG_STRINVDTXPOWER,
 LANG_STRSETMIBTXPOWERR,
 LANG_STRINVDMODE,
 LANG_STRSETWPAWARN,
 LANG_STRSETWEPWARN,
 LANG_STRSETSSIDERR,
 LANG_STRINVDCHANNUM,
 LANG_STRSETCHANERR,
 LANG_STRINVDAUTHTYPE,
 LANG_STRSETBASERATEERR,
 LANG_STRSETOPERRATEERR,
 LANG_STRINVDBRODSSID,
 LANG_STRINVDPROTECTION,
 LANG_STRINVDAGGREGATION,
 LANG_STRINVDSHORTGI0,
 LANG_STRSETPROTECTIONERR,
 LANG_STRSETAGGREGATIONERR,
 LANG_STRSETSHORTGI0ERR,
 LANG_STRSETCHANWIDTHERR,
 LANG_STRSETCTLBANDERR,
 LANG_STRENABACCCTLERR,
 LANG_STRINVDMACADDR,
 LANG_STRADDACERRFORFULL,
 LANG_STRMACINLIST,
 LANG_STRADDLISTERR,
 LANG_STRDELLISTERR,
 LANG_STRFRAGTHRESHOLD,
 LANG_STRSETFRAGTHREERR,
 LANG_STRRTSTHRESHOLD,
 LANG_STRSETRTSTHREERR,
 LANG_STRINVDBEACONINTV,
 LANG_STRSETBEACONINTVERR,
 LANG_STRINVDPREAMBLE,
 LANG_STRSETPREAMBLEERR,
 LANG_STRINVDDTIMPERD,
 LANG_STRSETDTIMERR,
 LANG_STRSETLANWLANBLOKERR,
 LANG_STRGETMBSSIBTBLERR,
 LANG_STRGETMULTIAPTBLERR,
 LANG_STRNOTSUPTSSIDTYPE,
 LANG_STRNOSSIDTYPEERR,
 LANG_STRNOENCRYPTIONERR,
 LANG_STRSET8021XWARNING,
 LANG_STRSETWLANWEPERR,
 LANG_STRSETWPARADIUSWARN,
 LANG_STRINVDWPAAUTHVALUE,
 LANG_STRNOPSKFORMAT,
 LANG_STRINVDPSKFORMAT,
 LANG_STRSETWPAPSKFMATERR,
 LANG_STRINVDPSKVALUE,
 LANG_STRSETWPAPSKERR,
 LANG_STRSETREKEYTIMEERR,
 LANG_STRINVDRSPORTNUM,
 LANG_STRNOIPADDR,
 LANG_STRINVDRSIPVALUE,
 LANG_STRRSPWDTOOLONG,
 LANG_STRINVDRSRETRY,
 LANG_STRSETRSRETRYERR,
 LANG_STRINVDRSTIME,
 LANG_STRSETRSINTVLTIMEERR,
 LANG_STRINVDWEPKEYLEN,
 LANG_STRGETMBSSIBTBLUPDTERR,
 LANG_STRKEYLENMUSTEXIST,
 LANG_STRINVDKEYLEN,
 LANG_STRKEYTYPEMUSTEXIST,
 LANG_STRINVDKEYTYPE,
 LANG_STRSETWEPKEYTYPEERR,
 LANG_STRINVDKEY1LEN,
 LANG_STRINVDWEPKEY1,
 LANG_STRSETWEPKEY1ERR,
 LANG_STRSETENABLEERR,
 LANG_STRCOMMENTTOOLONG,
 LANG_STRGETENTRYNUMERR,
 LANG_STRERRFORTABLFULL,
 LANG_STRADDENTRYERR,
 LANG_STRDELRECORDERR,
 LANG_FILEOPENFAILED,
 LANG_FILEWITHWRONGSIG,
 LANG_STRSETCERPASSERROR,
 LANG_STRACSURLWRONG,
 LANG_STRSSLWRONG,
 LANG_STRSETACSURLERROR,
 LANG_STRSETUSERNAMEERROR,
 LANG_STRSETPASSERROR,
 LANG_STRSETINFORMENABLEERROR,
 LANG_STRSETINFORMINTERERROR,
 LANG_STRSETCONREQUSERERROR,
 LANG_STRSETCONREQPASSERROR,
 LANG_STRSETCWMPFLAGERROR,
 LANG_STRGETCWMPFLAGERROR,
 LANG_STRUPLOADERROR,
 LANG_STRMALLOCFAIL,
 LANG_STRARGERROR,
 LANG_TINVALID_DNS_MODE,
 LANG_TDNS_MIB_GET_ERROR,
 LANG_TINVALID_DNS_ADDRESS,
 LANG_TDNS_MIB_SET_ERROR,
 LANG_TEXCEED_MAX_RULES,
 LANG_TINVALID_SOURCE_IP,
 LANG_TINVALID_SOURCE_NETMASK,
 LANG_TINVALID_SOURCE_PORT,
 LANG_TINVALID_PVC_BANDWIDTH,
 LANG_TINVALID_DESTINATION_IP,
 LANG_TINVALID_DESTINATION_NETMASK,
 LANG_TINVALID_DESTINATION_PORT,
 LANG_TINVALID_SPEED,
 LANG_TADD_CHAIN_ERROR,
 LANG_TDELETE_CHAIN_ERROR,
 LANG_TUPDATE_MIB_ERROR,
 LANG_TMODIFY_MIB_ERROR,
 LANG_TSET_MIB_ERROR,
 LANG_TGET_MIB_ERROR,
 LANG_TINVALID_IF_IP,
 LANG_TINVALID_IF_MASK,
 LANG_TPROTOCOL_EMPTY,
 LANG_TINVALID_PORT_RANGE,
 LANG_TINVALID_RULE_ACTION,
 LANG_TINVALID_SOURCE_MAC,
 LANG_TINVALID_DEST_MAC,
 LANG_TOUTGOING_IPPFILTER,
 LANG_TINCOMING_IPPFILTER,
 LANG_TDENY_IPPFILTER,
 LANG_TALLOW_IPPFILTER,
 LANG_TINVALID_RULE,
 LANG_STRSETDOSSYSSYNFLOODERR,
 LANG_STRSETDOSSYSFINFLOODERR,
 LANG_STRSETDOSSYSUDPFLOODERR,
 LANG_STRSETDOSSYSICMPFLOODERR,
 LANG_STRSETDOSPIPSYNFLOODERR,
 LANG_STRSETDOSPIPFINFLOODERR,
 LANG_STRSETDOSPIPUDPFLOODERR,
 LANG_STRSETDOSPIPICMPFLOODERR,
 LANG_STRSETDOSIPBLOCKTIMEERR,
 LANG_STRSETDOSENABLEERR,
 LANG_TIP_ADDR,
 LANG_TDYNAMIC_IP,
 LANG_TSTATIC_IP,
 LANG_TINVALID_IP_NET,
 LANG_TSTRURLEXIST,
 LANG_TSTRKEYEXIST,
 LANG_TMAXURL,
 LANG_TMAXKEY,
 LANG_STRIPEXIST,
 LANG_STRMACEXIST,
 LANG_STRSETINTERFACEERROR,
 LANG_STRSETACLCAPERROR,
 LANG_STRSETNATSESSIONERROR,
 LANG_INVALID,
 LANG_A_DEFAULT_GATEWAY_HAS_TO_BE_SELECTED,
 LANG_VENDOR_ID_CANNOT_BE_EMPTY,
 LANG_INVALID_VENDOR_ID,
 LANG_IAID_CANNOT_BE_EMPTY,
 LANG_IAID_SHOULD_BE_A_NUMBER,
 LANG_ENTERPRISE_NUMBER_CANNOT_BE_EMPTY,
 LANG_ENTERPRISE_NUMBER_SHOULD_BE_A_NUMBER,
 LANG_DUID_IDENTIFIER_CANNOT_BE_EMPTY,
 LANG_INVALID_DUID_IDENTIFIER,
 LANG_MANUFACTURER_OUI_CANNOT_BE_EMPTY,
 LANG_INVALID_MANUFACTURER_OUI,
 LANG_PRODUCT_CLASS_CANNOT_BE_EMPTY,
 LANG_INVALID_PRODUCT_CLASS,
 LANG_MODEL_NAME_CANNOT_BE_EMPTY,
 LANG_INVALID_MODEL_NAME,
 LANG_SERIAL_NUMBER_CANNOT_BE_EMPTY,
 LANG_INVALID_SERIAL_NUMBER,
 LANG_VID_SHOULD_NOT_BE_EMPTY,
 LANG_PPP_USER_NAME_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PPP_USER_NAME,
 LANG_INVALID_PPP_USER_NAME,
 LANG_PPP_PASSWORD_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PPP_PASSWORD,
 LANG_INVALID_PPP_PASSWORD,
 LANG_INVALID_PPP_IDLE_TIME,
 LANG_PLEASE_INPUT_IPV6_ADDRESS_OR_SELECT_DHCPV6_CLIENT_OR_CLICK_SLAAC,
 LANG_PLEASE_SELECT_IANA_OR_IAPD,
 LANG_PLEASE_INPUT_IPV6_ADDRESS_AND_PREFIX_LENGTH,
 LANG_INVALID_IPV6_ADDRESS,
 LANG_INVALID_IPV6_PREFIX_LENGTH,
 LANG_INVALID_IPV6_GATEWAY_ADDRESS,
 LANG_INVALID_PRIMARY_IPV6_DNS_ADDRESS,
 LANG_INVALID_SECONDARY_IPV6_DNS_ADDRESS,
 LANG_INVALID_6RD_BOARD_ROUTER_V4IP_ADDRESS,
 LANG_INVALID_6RD_IPV4_MASK_LENGTH,
 LANG_INVALID_6RD_PREFIX_ADDRESS,
 LANG_INVALID_6RD_PREFIX_LENGTH,
 LANG_INVALID_DSLITELOCALIP_ADDRESS,
 LANG_INVALID_DSLITEREMOTEIP_ADDRESS,
 LANG_INVALID_DSLITEGATEWAY_ADDRESS,
 LANG_NO_LINK_SELECTED,
 LANG_INVALID_VPI_VALUE_VPI_SHOULD_NOT_BE_A_DECIMAL,
 LANG_INVALID_VPI_VALUE_YOU_SHOULD_SET_A_VALUE_BETWEEN_0_255,
 LANG_INVALID_VCI_VALUE_VCI_SHOULD_NOT_BE_A_DECIMAL,
 LANG_INVALID_VCI_VALUE_YOU_SHOULD_SET_A_VALUE_BETWEEN_0_65535,
 LANG_INVALID_VPI_VCI_VALUE,
 LANG_YOU_SHOULD_ENABLE_AUTO_PVC_SEARCH_FIRST,
 LANG_INVALID_VLAN_ID,
 LANG_ENTER_SOURCE_NETWORK_ID,
 LANG_ENTER_SUBNET_MASK,
 LANG_INVALID_SOURCE_VALUE,
 LANG_INVALID_IP_ADDRESS_VALUE,
 LANG_INVALID_SOURCE_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_223,
 LANG_INVALID_SOURCE_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_SOURCE_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_SOURCE_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_0_254,
 LANG_YOU_SELECT_NONE,
 LANG_YOU_SELECT_ONE_TO_ONE,
 LANG_YOU_SELECT_MANY_TO_ONE,
 LANG_LOCAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_LOCAL_END_IP,
 LANG_INVALID_LOCAL_IP_RANGE,
 LANG_YOU_SELECT_MANY_TO_MANY,
 LANG_GLOBAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_GLOBAL_END_IP,
 LANG_INVALID_GLOBAL_IP_RANGE,
 LANG_YOU_SELECT_MANY_ONE_TO_MANY,
 LANG_PLEASE_COMMIT_AND_REBOOT_THIS_SYSTEM_FOR_TAKE_EFFECT_THE_ADDRESS_MAPPING_RULE,
 LANG_ADSL_MODULATION_CANNOT_BE_EMPTY,
 LANG_PASSWORD_IS_NOT_MATCHED_PLEASE_TYPE_THE_SAME_PASSWORD_BETWEEN_NEW_AND_CONFIRMED_BOX,
 LANG_PASSWORD_CANNOT_BE_EMPTY_PLEASE_TRY_IT_AGAIN,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_USER_NAME_PLEASE_TRY_IT_AGAIN,
 LANG_INVALID_USER_NAME,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PASSWORD_PLEASE_TRY_IT_AGAIN,
 LANG_INVALID_PASSWORD,
 LANG_INVALID_VALUE_FOR_MAX_NET_DATA_RATE,
 LANG_INVALID_VALUE_FOR_MIN_NET_DATA_RATE,
 LANG_INVALID_VALUE_FOR_MAX_INTERLEAVE_DELAY,
 LANG_INVALID_VALUE_FOR_SOS_MIN_DATA_RATE,
 LANG_INVALID_VALUE_FOR_EFFECTIVE_THROUGHPUT,
 LANG_INVALID_VALUE_FOR_NET_DATA_RATE,
 LANG_INVALID_VALUE_FOR_SHINE_RATIO,
 LANG_INVALID_VALUE_FOR_LEFTR_THRESHOLD,
 LANG_INVALID_VALUE_FOR_MAX_DELAY,
 LANG_INVALID_VALUE_FOR_MIN_DELAY,
 LANG_INVALID_AGEING_TIME,
 LANG_IP_ADDRESS_CANNOT_BE_EMPTY_IT_SHOULD_BE_FILLED_WITH_4_DIGIT_NUMBERS_AS_XXX_XXX_XXX_XXX,
 LANG_MAX_LIMITATION_PORTS_SHOULD_BE_LOWER_THAN_GLOBAL_TCP_CONNECTION_LIMIT,
 LANG_MAX_LIMITATION_PORTS_SHOULD_BE_LOWER_THAN_GLOBAL_UDP_CONNECTION_LIMIT,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_1_254,
 LANG_PLEASE_ENTER_HOSTNAME_FOR_THIS_ACCOUNT,
 LANG_INVALID_HOST_NAME,
 LANG_PLEASE_ENTER_USERNAME_FOR_THIS_ACCOUNT,
 LANG_PLEASE_ENTER_PASSWORD_FOR_THIS_ACCOUNT,
 LANG_PLEASE_ENTER_EMAIL_FOR_THIS_ACCOUNT,
 LANG_INVALID_EMAIL,
 LANG_PLEASE_ENTER_KEY_FOR_THIS_ACCOUNT,
 LANG_INVALID_KEY,
 LANG_PLEASE_SELECT_AN_ENTRY_TO_MODIFY,
 LANG_PLEASE_SELECT_AN_ENTRY_TO_DELETE,
 LANG_DEVICE_NAME_CANNOT_BE_NULL,
 LANG_INVALID_DEVICE_NAME,
 LANG_INVALID_OPTION60_STRING,
 LANG_INVALID_OPTION_STRING,
 LANG_OPTION60_CANNOT_BE_NULL,
 LANG_INVALID_SOURCE_RANGE_OF_IP_ADDRESS_IT_SHOULD_BE_IN_IP_POOL_RANGE,
 LANG_INVALID_DESTINATION_RANGE_OF_IP_ADDRESS_IT_SHOULD_BE_IN_IP_POOL_RANGE,
 LANG_ENTER_STBS_DNS_VALUE,
 LANG_ENTER_STBS_OPCH_ADDRESS_VALUE,
 LANG_ENTER_STBS_OPCH_PORT_VALUE,
 LANG_INVALID_OPCH_PORT_IT_SHOULD_BE_1_65535,
 LANG_ENTER_OPTION_STRING_VALUE,
 LANG_INVALID_DOMAIN_NAME,
 LANG_PLEASE_INPUT_DHCP_IP_POOL_RANGE_,
 LANG_INVALID_DHCP_CLIENT_START_RANGE_IT_SHOULD_BE_1_254,
 LANG_INVALID_DHCP_CLIENT_START_ADDRESSIT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET_OF_CURRENT_IP_ADDRESS,
 LANG_PLEASE_INPUT_DHCP_IP_POOL_RANGE,
 LANG_INVALID_DHCP_CLIENT_END_ADDRESS_RANGE_IT_SHOULD_BE_1_254,
 LANG_INVALID_DHCP_CLIENT_END_RANGE_IT_SHOULD_BE_1_254,
 LANG_INVALID_DHCP_CLIENT_END_ADDRESSIT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET_OF_CURRENT_IP_ADDRESS,
 LANG_INVALID_DHCP_CLIENT_ADDRESS_RANGEENDING_ADDRESS_SHOULD_BE_GREATER_THAN_STARTING_ADDRESS,
 LANG_INVALID_IP_POOL_RANGE_LAN_IP_MUST_BE_EXCLUDED_FROM_DHCP_IP_POOL,
 LANG_PLEASE_INPUT_DHCP_LEASE_TIME,
 LANG_INVALID_DHCP_SERVER_LEASE_TIME_NUMBER,
 LANG_INVALID_DHCP_SERVER_LEASE_TIME,
 LANG_GATEWAY_ADDRESS_CANNOT_BE_EMPTY_IT_SHOULD_BE_FILLED_WITH_4_DIGIT_NUMBERS_AS_XXX_XXX_XXX_XXX,
 LANG_INVALID_GATEWAY_ADDRESS_VALUE_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_GATEWAY_ADDRESS_VALUE,
 LANG_INVALID_GATEWAY_ADDRESS_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_223,
 LANG_INVALID_GATEWAY_ADDRESS_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_GATEWAY_ADDRESS_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_GATEWAY_ADDRESS_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_1_254,
 LANG_ENTER_DNS_VALUE,
 LANG_START_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_2000_0200_10,
 LANG_INVALID_START_IP,
 LANG_END_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_2000_0200_20,
 LANG_INVALID_END_IP,
 LANG_PLEASE_INPUT_IP_PREFIX_LENGTH,
 LANG_PLEASE_INPUT_DHCP_DEFAULT_LEASE_TIME,
 LANG_INVALID_DHCP_SERVER_DEFAULT_LEASE_TIME_NUMBER,
 LANG_INVALID_DHCP_SERVER_DEFAULT_LEASE_TIME,
 LANG_PLEASE_INPUT_DHCP_PREFERED_LIFETIME,
 LANG_INVALID_DHCP_SERVER_PREFERED_LIFETIME_NUMBER,
 LANG_INVALID_DHCP_SERVER_PREFERED_LIFETIME,
 LANG_PLEASE_INPUT_DHCP_RENEW_TIME,
 LANG_PLEASE_INPUT_DHCP_REBIND_TIME,
 LANG_PLEASE_INPUT_DHCP_CLIENT_OUID,
 LANG_TRAFFIC_CLASSIFICATION_RULES_CAN_NOT_BE_EMPTY,
 LANG_INVALID_TOTAL_BANDWIDTH_LIMIT,
 LANG_TOTAL_BANDWIDTH_LIMIT_NOT_ASSIGNED,
 LANG_INVALID_CLASS_RATE_LIMIT,
 LANG_CLASS_RATE_LIMIT_NOT_ASSIGNED,
 LANG_CLASS_RATE_MUST_BETWEEN_100K_TO_TOTAL_BANDWIDTH_LIMIT_100K,
 LANG_INVALID_MAX_DESIRED_LATENCY,
 LANG_MAX_DESIRED_LATENCY_NOT_ASSIGNED,
 LANG_INVALID_POLICE_RATE,
 LANG_POLICE_RATE_NOT_ASSIGNED,
 LANG_POLICE_ACTION_NOT_ASSIGNED,
 LANG_POLICE_RATE_CAN_NOT_BE_GREATER_THAN_TOTAL_BANDWIDTH_LIMIT,
 LANG_INVALID_SOURCE_PORT,
 LANG_INVALID_SOURCE_PORT_NUMBER,
 LANG_INVALID_DESTINATION_PORT,
 LANG_INVALID_DESTINATION_PORT_NUMBER,
 LANG_AT_LEAST_ONE_PHB_HAS_TO_BE_SELETED,
 LANG_AT_LEAST_TWO_PHB_HAS_TO_BE_SELETED_FOR_AF_CLASS,
 LANG_IP_ADDRESS_IS_INVALID,
 LANG_THE_SECOND_DNSV4_NAME_SERVER_S_IP_IS_EMPTY,
 LANG_THE_FIRST_DNSV6_NAME_SERVER_S_IP_IS_EMPTY,
 LANG_INVALID_THE_FIRST_DNSV6_NAME_SERVER_S_IP,
 LANG_THE_SECOND_DNSV6_NAME_SERVER_S_IP_IS_EMPTY,
 LANG_INVALID_THE_SECOND_DNSV6_NAME_SERVER_S_IP,
 LANG_THE_THIRD_DNSV6_NAME_SERVER_S_IP_IS_EMPTY,
 LANG_NO_ROUTER_WAN_INTERFACE,
 LANG_PLEASE_ENTER_THE_BLOCKED_DOMAIN,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_DOMAIN_PLEASE_TRY_IT_AGAIN,
 LANG_INVALID_BLOCKED_DOMAIN,
 LANG_MUST_LARGER_THAN_5_PACKETS_SECOND,
 LANG_YOU_MUST_SELECT_AN_ENTRY_IN_TABLE,
 LANG_INVALID_MAC_ADDRESS,
 LANG_COUNT_CANNOT_BE_EMPTY,
 LANG_COUNT_SHOULD_BE_A_POSITIVE_NUMBER,
 LANG_INTERFACE_IS_DUPLICATED,
 LANG_MD_NAME_CANNOT_BE_EMPTY,
 LANG_INVALID_MD_NAME,
 LANG_MD_LEVEL_IS_DUPLICATED,
 LANG_MA_NAME_CANNOT_BE_EMPTY,
 LANG_INVALID_MA_NAME,
 LANG_MEP_INDEX_CANNOT_BE_EMPTY,
 LANG_MEP_INDEX_SHOULD_BE_A_NUMBER_BETWEEN_1_AND_8191,
 LANG_MYID_MUST_MORE_THAN_0,
 LANG_MEGID_MUST_HAVE_VALUE,
 LANG_INVALID_IP_ADDRESS_VALUE_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_223,
 LANG_EMPTY_LOCAL_IP_ADDRESS,
 LANG_INVALID_LOCAL_IP_ADDRESS_VALUE_,
 LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_1ST_DIGIT,
 LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_2ND_DIGIT,
 LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_3RD_DIGIT,
 LANG_INVALID_LOCAL_IP_ADDRESS_RANGE_IN_4TH_DIGIT,
 LANG_EMPTY_EXTERNAL_IP_ADDRESS,
 LANG_INVALID_EXTERNAL_IP_ADDRESS_VALUE_,
 LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_1ST_DIGIT,
 LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_2ND_DIGIT,
 LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_3RD_DIGIT,
 LANG_INVALID_EXTERNAL_IP_ADDRESS_RANGE_IN_4TH_DIGIT,
 LANG_FILTER_RULES_CAN_NOT_BE_EMPTY,
 LANG_INVALID_SOURCE_IPV6_START_ADDRESS,
 LANG_INVALID_SOURCE_IPV6_PREFIX_LENGTH_IT_MUST_BE_0_9,
 LANG_INVALID_SOURCE_IPV6_PREFIX_LENGTH_IT_MUST_BE_0_64,
 LANG_INVALID_SOURCE_IPV6_END_ADDRESS,
 LANG_INVALID_DESTINATION_IPV6_START_ADDRESS,
 LANG_INVALID_DESTINATION_IPV6_PREFIX_LENGTH_IT_MUST_BE_0_9,
 LANG_INVALID_DESTINATION_IPV6_PREFIX_LENGTH_IT_MUST_BE_0_64,
 LANG_INVALID_DESTINATION_IPV6_END_ADDRESS,
 LANG_INPUT_FILTER_RULE_IS_NOT_VALID,
 LANG_INVALID_SOURCE_IPV6_INTERFACE_ID_START_ADDRESS,
 LANG_INVALID_SOURCE_IP_ADDRESS,
 LANG_INVALID_SOURCE_IP_MASK,
 LANG_INVALID_DESTINATION_IP_ADDRESS,
 LANG_INVALID_DESTINATION_IP_ADDRESS_VALUE,
 LANG_INVALID_DESTINATION_IP_MASK,
 LANG_INPUT_MAC_ADDRESS,
 LANG_INPUT_MAC_ADDRESS_IS_NOT_COMPLETE_IT_SHOULD_BE_12_DIGITS_IN_HEX,
 LANG_INVALID_MAC_ADDRESS_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F_,
 LANG_LOCAL_SETTINGS_CANNOT_BE_EMPTY,
 LANG_INVALID_LOCAL_IP_ADDRESS,
 LANG_INVALID_REMOTE_IP_ADDRESS,
 LANG_INVALID_PORT_NUMBER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_PORT_NUMBER_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535,
 LANG_IP_ADDRESS_CANNOT_BE_EMPTY,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_1ST_DIGIT,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_2ND_DIGIT,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_3RD_DIGIT,
 LANG_INVALID_IP_ADDRESS_RANGE_IN_4TH_DIGIT,
 LANG_LOID_CANNOT_BE_EMPTY,
 LANG_LOID_PASSWORD_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_LOID_PASSWORD,
 LANG_INVALID_LOID_PASSWORD,
 LANG_PLOAM_PASSWORD_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PLOAM_PASSWORD,
 LANG_INVALID_PLOAM_PASSWORD,
 LANG_PLOAM_PASSWORD_SHOULD_BE_10_CHARACTERS,
 LANG_SELECTED_FILE_CANNOT_BE_EMPTY,
 LANG_PLEASE_ENTER_TUNNEL_NAME,
 LANG_INVALID_VALUE_IN_TUNNEL_NAME,
 LANG_INVALID_VALUE_IN_REMOTE_ADDRESS,
 LANG_INVALID_VALUE_IN_LOCAL_ADDRESS,
 LANG_TRAFFIC_CLASSIFICATION_RULES_CAN_T_BE_EMPTY,
 LANG_PLEASE_SELECT_QUEUE,
 LANG_PLEASE_ENTER_KEY,
 LANG_PLEASE_ENTER_HEXADECIMAL_NUMBER,
 LANG_NEITHER_ENCRYPT_NOR_AUTH_IS_SELECTED,
 LANG_SPI_SHOULD_BE_A_DIGIT_NUMBER,
 LANG_SPI_0_255_IS_RESERVED,
 LANG_PORT_SHOULD_BE_0_65535,
 LANG_PSK_LENGTH_SHOULD_LESS_THAN_128,
 LANG_DON_T_SELECT_A_ELEMENT,
 LANG_PLEASE_ENTER_L2TP_SERVER_ADDRESS,
 LANG_INVALID_VALUE_IN_SERVER_ADDRESS,
 LANG_PLEASE_ENTER_L2TP_TUNNEL_AUTHENTICATION_SECRET,
 LANG_INVALID_VALUE_IN_TUNNEL_AUTHENTICATION_SECRET,
 LANG_PLEASE_ENTER_L2TP_CLIENT_USERNAME,
 LANG_INVALID_VALUE_IN_USERNAME,
 LANG_PLEASE_ENTER_L2TP_CLIENT_PASSWORD,
 LANG_INVALID_VALUE_IN_PASSWORD,
 LANG_PLEASE_ENTER_L2TP_TUNNEL_IDLE_TIME,
 LANG_PLEASE_SELECT_DEFAULT_GATEWAY_FOR_DIAL_ON_DEMAND,
 LANG_PLEASE_ENTER_L2TP_TUNNEL_MTU,
 LANG_LENGTH_SHOULD_LESS_THEN_32,
 LANG_INVALID_CHARACTER_IN_TEXT_AERA,
 LANG_PLEASE_ENTER_PEER_START_ADDRESS,
 LANG_PLEASE_ENTER_LOCAL_ADDRESS,
 LANG_PLEASE_ENTER_L2TP_SERVER_RULE_NAME,
 LANG_PLEASE_ENTER_L2TP_TUNNEL_AUTHKEY,
 LANG_PLEASE_ENTER_L2TP_SERVER_USERNAME,
 LANG_PLEASE_ENTER_L2TP_SERVER_PASSWORD,
 LANG_PLEASE_ENTER_L2TP_CLIENT_RULE_NAME,
 LANG_PLEASE_ENTER_L2TP_CLIENT_MTU,
 LANG_PLEASE_INPUT_IPV6_ADDRESS,
 LANG_PLEASE_INPUT_IPV4_ADDRESS,
 LANG_PLEASE_COMMIT_AND_REBOOT_THIS_SYSTEM_FOR_TAKE_EFFECT_THE_LANDING_PAGE,
 LANG_INPUT_HOST_MAC_ADDRESS_IS_NOT_COMPLETE_IT_SHOULD_BE_17_DIGITS_IN_HEX,
 LANG_ENTER_HOST_MAC_ADDRES,
 LANG_INVALID_HOST_MAC_ADDRESS_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F_OR_A_F,
 LANG_INVALID_IP_ADDRESS_IT_SHOULD_BE_IN_IP_POOL_RANGE,
 LANG_INVALID_SOURCE_RANGE_IN_1ST_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_INVALID_SOURCE_RANGE_IN_2ND_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_INVALID_SOURCE_RANGE_IN_3RD_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_INVALID_SOURCE_RANGE_IN_4TH_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_INVALID_SOURCE_RANGE_IN_5RD_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_INVALID_SOURCE_RANGE_IN_6TH_HEX_NUMBER_OF_HOST_MAC_ADDRESS_IT_SHOULD_BE_0X00_0XFF,
 LANG_RULE_NAME_IS_INVALID,
 LANG_PLEASE_ASSIGN_USED_QUEUE,
 LANG_NEED_TO_SPECIFY_ONE_PARAMETER_AT_LEAST,
 LANG_INCORRECT_VLAN_ID_SHOULE_BE_1_4095,
 LANG_PLEASE_ASSIGN_IP_VERSION,
 LANG_INVALID_ETHERNET_TYPE,
 LANG_PROTOCOL_IS_RTP_CAN_NOT_SPECIFY_DST_PORT,
 LANG_SRC_IP_IS_INVALID,
 LANG_SRC_MASK_IS_INVALID,
 LANG_DST_IP_IS_INVALID,
 LANG_DST_MASK_IS_INVALID,
 LANG_INVALID_SOURCE_IPV6_ADDRESS,
 LANG_INVALID_SOURCE_IPV6_PREFIX_LENGTH,
 LANG_INVALID_DESTINATION_IPV6_ADDRESS,
 LANG_INVALID_DESTINATION_IPV6_PREFIX_LENGTH,
 LANG_INVALID_SRC_START_PORT,
 LANG_PROTOCOL_IS_NOT_TCP_UDP_CAN_NOT_SPECIFY_PORT,
 LANG_INVALID_SRC_END_PORT,
 LANG_INVALID_DST_START_PORT,
 LANG_INVALID_DST_END_PORT,
 LANG_INVALID_SRC_MAC,
 LANG_INVALID_DST_MAC,
 LANG_VENDORCLASS_SHOULD_NOT_BE_EMPTY,
 LANG_VENDORCLASS_IS_INVALID,
 LANG_OPT61_IAID_IS_INVALID,
 LANG_DUID_HW_TYPE_IS_INVALID,
 LANG_DUID_TIME_IS_INVALID,
 LANG_DUID_MAC_IS_INVALID,
 LANG_NEED_TO_FILL_VALUE,
 LANG_DUID_ENT_NUM_IS_INVALID,
 LANG_DUID_ENT_ID_IS_INVALID,
 LANG_OPT125_ENT_NUM_IS_INVALID,
 LANG_OPT125_MANUFACTURER_IS_INVALID,
 LANG_OPT125_PRODUCT_CLASS_IS_INVALID,
 LANG_OPT125_MODEL_IS_INVALID,
 LANG_OPT125_SERIAL_IS_INVALID,
 LANG_SHOULD_NOT_BE_EMPLTY,
 LANG_INVALID_TOTALBANDWIDTH_NUMBER,
 LANG_WAN_INTERFACE_NOT_ASSIGNED,
 LANG_SOURCE_IP_INVALID,
 LANG_DESTINATION_IP_INVALID,
 LANG_SOURCE_IP_MASK_INVALID,
 LANG_DESTINATION_IP_MASK_INVALID,
 LANG_SOURCE_PORT_INVALID,
 LANG_PLEASE_ASSIGN_TCP_UDP,
 LANG_DESTINATION_PORT_INVALID,
 LANG_UPLINK_RATE_INVALID,
 LANG_INVALID_LOOPBACK_LOCATION_ID,
 LANG_INVALID_LEASE_TIME,
 LANG_INPUT_USERNAME,
 LANG_LOCAL_START_IP_DOMAIN_IS_DIFFERENT_FROM_END_IP,
 LANG_INVALID_IP_RANGE,
 LANG_INVALID_MAC_ADDRESS_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F,
 LANG_INVALID_VALUE_OF_TIME_VALUE_PLEASE_INPUT_CORRENT_TIME_FORMAT_00_00_23_59,
 LANG_END_TIME_SHOULD_BE_LARGER,
 LANG_INVALID_VALUE_OF_TIME_VALUE_PLEASE_INPUT_CORRENT_TIME_FORMAT_EX_12_50,
 LANG_ENTER_HOST_ADDRESS,
 LANG_LOCAL_IP_ADDRESS_CANNOT_BE_EMPTY,
 LANG_RULE_CANNOT_BE_EMPTY_PLEASE_CHOOSE_ONE_GATEGORY_AND_SELECT_ONE_AVAILABLE_RULE_THEN_ADD_INTO_APPLIED_RULE,
 LANG_PLEASE_ENTER_PPTP_SERVER_ADDRESS,
 LANG_PLEASE_ENTER_PPTP_CLIENT_USERNAME,
 LANG_PLEASE_ENTER_PPTP_CLIENT_PASSWORD,
 LANG_PLEASE_ENTER_PPTP_SERVER_RULE_NAME,
 LANG_PLEASE_ENTER_PPTP_SERVER_USERNAME,
 LANG_PLEASE_ENTER_PPTP_SERVER_PASSWORD,
 LANG_PLEASE_ENTER_PPTP_CLIENT_RULE_NAME,
 LANG_INVALID_MAXRTRADVINTERVAL_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_MAXRTRADVINTERVAL_MUST_BE_NO_LESS_THAN_4_SECONDS_AND_NO_GREATER_THAN_1800_SECONDS,
 LANG_INVALID_MINRTRADVINTERVAL_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_MINRTRADVINTERVAL_MUST_BE_NO_LESS_THAN_3_SECONDS_AND_NO_GREATER_THAN_0_75_MAXRTRADVINTERVAL,
 LANG_INVALID_ADVCURHOPLIMIT_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ADVDEFAULTLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_ADVDEFAULTLIFETIME_MUST_BE_EITHER_ZERO_OR_BETWEEN_MAXRTRADVINTERVAL_AND_9000_SECONDS,
 LANG_INVALID_ADVREACHABLETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ADVRETRANSTIMER_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ADVLINKMTU_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_ADVLINKMTU_MUST_BE_EITHER_ZERO_OR_BETWEEN_1280_AND_1500,
 LANG_ULA_PREFIX_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_FC01,
 LANG_INVALID_ULA_PREFIX_IP,
 LANG_ULA_PREFIX_LENGTH_CANNOT_BE_EMPTY_FOR_EXAMPLE_64,
 LANG_INVALID_ULA_PREFIX_LENGTH_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_ULA_PREFIX_VALID_TIME_CANNOT_BE_EMPTY_VALID_RANGE_IS_600_4294967295,
 LANG_INVALID_ULAPREFIXVALIDTIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ULAPREFIXPREFEREDTIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_PREFIX_IP_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_3FFE_501_FFFF_100,
 LANG_INVALID_PREFIX_IP,
 LANG_PREFIX_LENGTH_CANNOT_BE_EMPTY,
 LANG_INVALID_PREFIX_LENGTH_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ADVVALIDLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_ADVPREFERREDLIFETIME_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_ADVVALIDLIFETIME_MUST_BE_GREATER_THAN_ADVPREFERREDLIFETIME,
 LANG_INVALID_RDNSS1_IP,
 LANG_INVALID_RDNSS2_IP,
 LANG_DUPLICATED_PORT_NUMBER,
 LANG_PORT_RANGE_CANNOT_BE_EMPTY_YOU_SHOULD_SET_A_VALUE_BETWEEN_1_65535,
 LANG_ENTER_DESTINATION_NETWORK_ID,
 LANG_INVALID_DESTINATION_VALUE,
 LANG_INVALID_DESTINATION_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_DESTINATION_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_DESTINATION_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_DESTINATION_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_0_254,
 LANG_INVALID_SUBNET_MASK_VALUE,
 LANG_INVALID_SUBNET_MASK_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_SUBNET_MASK_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_SUBNET_MASK_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_SUBNET_MASK_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_0_255,
 LANG_ENTER_NEXT_HOP_IP_OR_SELECT_A_GW_INTERFACE,
 LANG_INVALID_NEXT_HOP_VALUE,
 LANG_INVALID_NEXT_HOP_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_NEXT_HOP_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_NEXT_HOP_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_NEXT_HOP_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_1_254,
 LANG_THE_SPECIFIED_MASK_PARAMETER_IS_INVALID_DESTINATION_MASK_DESTINATION,
 LANG_INVALID_METRIC_RANGE_IT_SHOULD_BE_0_16,
 LANG_DESTNET_IS_NOT_VALID_IPV6_NET,
 LANG_DESTNET_IS_NOT_VALID_IPV6_HOST,
 LANG_NEXTHOP_IS_NOT_VALID_GLOBAL_AND_UNICAST_IPV6_ADDRESS,
 LANG_TRAP_IP_ADDRESS_CANNOT_BE_EMPTY_IT_SHOULD_BE_FILLED_WITH_4_DIGIT_NUMBERS_AS_XXX_XXX_XXX_XXX,
 LANG_INVALID_TRAP_IP_ADDRESS_VALUE,
 LANG_INVALID_TRAP_IP_ADDRESS_RANGE_IN_1ST_DIGIT_IT_SHOULD_BE_0_223,
 LANG_INVALID_TRAP_IP_ADDRESS_RANGE_IN_2ND_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_TRAP_IP_ADDRESS_RANGE_IN_3RD_DIGIT_IT_SHOULD_BE_0_255,
 LANG_INVALID_TRAP_IP_ADDRESS_RANGE_IN_4TH_DIGIT_IT_SHOULD_BE_1_254,
 LANG_INVALID_OBJECT_ID_VALUE_IT_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_OBJECT_ID_VALUE_IT_SHOULD_BE_FILL_WITH_OID_STRING_AS_1_3_6_1_4_1_X,
 LANG_INVALID_OBJECT_ID_VALUE_IT_SHOULD_BE_FILL_WITH_PREFIX_OID_STRING_AS_1_3_6_1_4_1,
 LANG_INVALID_SYSTEM_DESCRIPTION,
 LANG_INVALID_SYSTEM_CONTACT,
 LANG_INVALID_SYSTEM_NAME,
 LANG_INVALID_SYSTEM_LOCATION_,
 LANG_INVALID_COMMUNITY_NAME_READ_ONLY,
 LANG_INVALID_COMMUNITY_NAME_WRITE_ONLY,
 LANG_PLEASE_COMMIT_AND_REBOOT_THIS_SYSTEM_FOR_TAKE_EFFECT_THE_SYSTEM_LOG,
 LANG_LAN_IPV6_ADDRESS_CANNOT_BE_EMPTY_FORMAT_IS_IPV6_ADDRESS_FOR_EXAMPLE_3FFE_501_FFFF_100_1,
 LANG_INVALID_LAN_IPV6_IP,
 LANG_LAN_IPV6_ADDRESS_IPV6_PREFIX1_CANNOT_BE_EMPTY_VALID_NUMBER_IS_0_127,
 LANG_INVALID_LAN_IPV6_PREVIX,
 LANG_ACS_URL_CANNOT_BE_EMPTY,
 LANG_PLEASE_INPUT_PERIODIC_INTERVAL_TIME_,
 LANG_INTERVAL_SHOULD_BE_THE_DECIMAL_NUMBER_0_9,
 LANG_INVALID_PATH,
 LANG_PLEASE_INPUT_THE_PORT_NUMBER_FOR_CONNECTION_REQUEST_,
 LANG_INVALID_PORT_NUMBER_OF_CONNECTION_REQUEST_IT_SHOULD_BE_1_65535,
 LANG_IT_SHOULD_BE_IN_NUMBER_0_9,
 LANG_INVALID_MONTH_NUMBER_IT_SHOULD_BE_IN_NUMBER_1_9,
 LANG_INVALID_MONTH_SETTING,
 LANG_INVALID_DAY_SETTING,
 LANG_INVALID_HOUR_SETTING,
 LANG_INVALID_TIME_SETTING,
 LANG_INVALID_SERVER_STRING,
 LANG_PLEASE_ENTER_THE_BLOCKED_FQDN,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_FQDN_PLEASE_TRY_IT_AGAIN,
 LANG_INVALID_BLOCKED_FQDN,
 LANG_PLEASE_ENTER_THE_BLOCKED_KEYWORD,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_BLOCKED_KEYWORD_PLEASE_TRY_IT_AGAIN,
 LANG_INVALID_BLOCKED_KEYWORD,
 LANG_CONFLICT_USER_NAME,
 LANG_PASSWORD_IS_NOT_MATCHED_PLEASE_TYPE_THE_SAME_PASSWORD_BETWEEN__NEW_AND__CONFIRMED_BOX,
 LANG_INCORRECT_OLD_PASSWORD_PLEASE_TRY_IT_AGAIN,
 LANG_THE_ACCOUNT_CANNOT_BE_DELETED,
 LANG_VID_CANNOT_BE_EMPTY,
 LANG_VLAN_PRIORITY_CANNOT_BE_EMPTY,
 LANG_VID1_SHOULD_NOT_BE_EMPTY,
 LANG_VID1_SHOULD_BE_NUMBER,
 LANG_VID1_7_9_ARE_RESERVED,
 LANG_VID1_SHOULD_BE_0_4095,
 LANG_VID2_SHOULD_NOT_BE_EMPTY,
 LANG_VID2_SHOULD_BE_NUMBER,
 LANG_VID2_7_9_ARE_RESERVED,
 LANG_VID2_SHOULD_BE_0_4095,
 LANG_VID3_SHOULD_NOT_BE_EMPTY,
 LANG_VID3_SHOULD_BE_NUMBER,
 LANG_VID3_7_9_ARE_RESERVED,
 LANG_VID3_SHOULD_BE_0_4095,
 LANG_VID4_SHOULD_NOT_BE_EMPTY,
 LANG_VID4_SHOULD_BE_NUMBER,
 LANG_VID4_7_9_ARE_RESERVED,
 LANG_VID4_SHOULD_BE_0_4095,
 LANG_INVALID_PIN_CODE_VALUE,
 LANG_INVALID_APN_VALUE_APN_MUST_BE_A_STRING_WITHOUT_ANY_SPACE,
 LANG_INVALID_DIAL_NUMBER_VALUE_DIAL_NUMBER_MUST_BE_A_PHONE_NUMBER,
 LANG_USER_NAME_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_USER_NAME,
 LANG_PASSWORD_CANNOT_BE_EMPTY,
 LANG_CANNOT_ACCEPT_SPACE_CHARACTER_IN_PASSWORD,
 LANG_INVALID_IDLE_TIME,
 LANG_INVALID_MTU_VALUE_YOU_SHOULD_SET_A_VALUE_BETWEEN_65_1500,
 LANG_INVALID_VALUE_IT_SHOULD_BE_IN_DECIMAL_NUMBER_0_9,
 LANG_INVALID_VALUE_OF_FRAGMENT_THRESHOLD_INPUT_VALUE_SHOULD_BE_BETWEEN_256_2346_IN_DECIMAL,
 LANG_INVALID_VALUE_OF_RTS_THRESHOLD_INPUT_VALUE_SHOULD_BE_BETWEEN_0_2347_IN_DECIMAL,
 LANG_INVALID_VALUE_OF_BEACON_INTERVAL_INPUT_VALUE_SHOULD_BE_BETWEEN_20_1024_IN_DECIMAL,
 LANG_SSID_CANNOT_BE_EMPTY,
 LANG_INVALID_LIMIT_ASSOCIATED_CLIENT_NUMBER,
 LANG_INVALID_SSID,
 LANG_INVALID_MDID_2_OCTET_IDENTIFIER_AS_A_HEX_STRING,
 LANG_INVALID_HEX_NUMBER,
 LANG_INVALID_RANGE_OF_KEY_EXPIRATION_TIMEOUT_0_OR_1_65535_MINUTES,
 LANG_INVALID_RANGE_OF_REASSOCIATION_TIMEOUT_0_OR_1000_65535,
 LANG_INVALID_NAS_IDENTIFIER_1_48_CHARACTERS,
 LANG_EXCEED_MAX_NUMBER_OF_KH_ENTRY_FOR_SELECTED_SSID,
 LANG_WHITE_SPACE_IS_NOT_ALLOWED_IN_NAS_IDENTIFIER_FIELD_,
 LANG_INVALID_KEY_LENGTH_SHOULD_BE_16_OCTETS_OR_1_32_CHARACTERS,
 LANG_WE_CAN_NOT_PROCESS_AP_THE_WINDOWS_WILL_BE_CLOSED,
 LANG_ERROR_NOT_SUPPORTED_WPA2_CIPHER_SUITE,
 LANG_ERROR_NOT_SUPPORTED_WPA_CIPHER_SUITE,
 LANG_ERROR_NOT_SUPPORTED_ENCRYPT,
 LANG_UPDATE_FIRMWARE_FAILED,
 LANG_FILE_NAME_CAN_NOT_BE_EMPTY,
 LANG_PLEASE_INPUT_MAC_ADDRESS,
 LANG_KEY_REGOUP_TIME_SHOUD_IN_300_31536000,
 LANG_NUMBER_OF_PACKETS_SHOULD_GREAT_THAN_1048576,
 LANG_NUMBER_OF_PACKETS_TOO_BIG,
 LANG_INVALID_COMMENT,
 LANG_THE_ENCRYPTION_KEY_YOU_SELECTED_AS_THE__TX_DEFAULT_KEY_CANNOT_BE_BLANK,
 LANG_INVALID_LENGTH_OF_KEY_CHARACTERS,
 LANG_INVALID_LENGTH_OF_WEP_KEY_VALUE,
 LANG_INVALID_KEY_VALUE_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F,
 LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_64_CHARACTERS,
 LANG_INVALID_PRE_SHARED_KEY_VALUE_IT_SHOULD_BE_IN_HEX_NUMBER_0_9_OR_A_F,
 LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_SET_AT_LEAST_8_CHARACTERS,
 LANG_PRE_SHARED_KEY_VALUE_SHOULD_BE_LESS_THAN_64_CHARACTERS,
 LANG_NOT_ALLOWED_FOR_THE_CLIENT_MODE,
 LANG_INVALID_LENGTH_OF_KEY_VALUE,
 LANG_RADIUS_SERVER_PORT_NUMBER_CANNOT_BE_EMPTY_IT_SHOULD_BE_A_DECIMAL_NUMBER_BETWEEN_1_65535,
 LANG_INVALID_PORT_NUMBER_OF_RADIUS_SERVER_IT_SHOULD_BE_A_DECIMAL_NUMBER_BETWEEN_1_65535,
 LANG_WARNING_SECURITY_IS_NOT_SETTHIS_MAY_BE_DANGEROUS,
 LANG_INFO_WPS_WILL_BE_DISABLED_WHEN_USING_WEP,
 LANG_WPA_CIPHER_SUITE_CAN_NOT_BE_EMPTY,
 LANG_CAN_NOT_SELECT_TKIP_AND_AES_IN_THE_SAME_TIME,
 LANG_INFO_WPS_WILL_BE_DISABLED_WHEN_USING_WPA_ONLY,
 LANG_WPA2_CIPHER_SUITE_CAN_NOT_BE_EMPTY,
 LANG_INFO_WPS_WILL_BE_DISABLED_WHEN_USING_TKIP_ONLY,
 LANG_IN_THE_CLIENT_MODE_YOU_CAN_T_SELECT_TKIP_AND_AES_IN_THE_SAME_TIME,
 LANG_INVALID_PRE_SHARED_KEY,
 LANG_INVALID_ENROLLEE_PIN_LENGTH_THE_DEVICE_PIN_IS_USUALLY_FOUR_OR_EIGHT_DIGITS_LONG,
 LANG_INVALID_ENROLLEE_PIN_ENROLLEE_PIN_MUST_BE_NUMERIC_DIGITS,
 LANG_INVALID_PIN_LENGTH_THE_DEVICE_PIN_IS_USUALLY_FOUR_OR_EIGHT_DIGITS_LONG,
 LANG_INVALID_PIN_THE_DEVICE_PIN_MUST_BE_NUMERIC_DIGITS,
 LANG_INVALID_PIN_CHECKSUM_ERRO,
 LANG_WPS2_0_DOES_NOT_SUPPORT_THIS_ENCRYPTION_MODE,
 LANG_OMCI_SW_VER1_CANNOT_BE_EMPTY,
 LANG_OMCI_SW_VER2_CANNOT_BE_EMPTY,
 LANG_OMCI_EQID_CANNOT_BE_EMPTY,
 LANG_OMCI_ONT_VER_CANNOT_BE_EMPTY,
 LANG_DO_YOU_CONFIRM_TO_LOGOUT,
 LANG_USER_ACCOUNT_IS_EMPTY_NDO_YOU_WANT_TO_DISABLE_THE_PASSWORD_PROTECTION,
 LANG_DO_YOU_REALLY_WANT_TO_MASK_ALL_TONES,
 LANG_DO_YOU_REALLY_WANT_TO_UNMASK_ALL_TONES,
 LANG_DOMAIN_STRING_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_DOMAIN,
 LANG_DO_YOU_CONFIRM_THE_MAINTENANCE_IS_OVER,
 LANG_DO_YOU_REALLY_WANT_TO_UPGRADE_THE_FIRMWARE,
 LANG_ARE_YOU_SURE_TO_DELETE_THE_SCRIPT_FILE,
 LANG_IT_NEEDS_REBOOTING_TO_CHANGE_WAN_MODE,
 LANG_DO_YOU_REALLY_WANT_TO_COMMIT_THE_CURRENT_SETTINGS,
 LANG_DO_YOU_REALLY_WANT_TO_RESET_THE_CURRENT_SETTINGS_TO_DEFAULT,
 LANG_DO_YOU_REALLY_WANT_TO_RESET_THE_CURRENT_SETTINGS_TO_FACTORY_DEFAULT,
 LANG_DO_YOU_REALLY_WANT_TO_DELETE_THE_SELECTED_ENTRY,
 LANG_DO_YOU_REALLY_WANT_TO_DELETE_THE_ALL_ENTRIES,
 LANG_ARE_YOU_SURE_YOU_WANT_TO_DELETE,
 LANG_FQDN_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_FQDN,
 LANG_KEYWORD_IS_EMPTY_NPLEASE_ENTER_THE_BLOCKED_KEYWORD,
 LANG_CHECKSUM_FAILED_USE_PIN_ANYWAY,
 LANG_INVALID_PIN_CODE,
 LANG_INVALID_BACKUP_TIMER_VALUE,
 LANG_ERROR_CANT_RESOLVE,
 LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE,
 LANG_MAXIMUM_RULES_EXCEEDED,
 LANG_ERROR_NO_ADDRESS_MAP_TYPE,
 LANG_INVALID_ADDRESS_MAP_TYPE,
 LANG_INVALID_VALUE,
 LANG_AUTO_PVC_SEARCH_TABLE_WAS_FULL,
 LANG_CAN_NOT_OPEN_TMP_CERT,
 LANG_CERIFICATION_INSTALL_SUCCESS,
 LANG_CONNECT_FAILED_1,
 LANG_CONNECT_REQUEST_TIMEOUT,
 LANG_CONNECT_TIMEOUT,
 LANG_DEFAULT_GW_HAS_ALREADY_EXIST,
 LANG_DEFAULT_GW_HAS_ALREADY_EXIST_FOR_IPIP_VPN,
 LANG_DEFAULT_GW_HAS_ALREADY_EXIST_FOR_L2TP_VPN,
 LANG_DEFAULT_GW_HAS_ALREADY_EXIST_FOR_PPTP_VPN,
 LANG_DELETE_KEYWORD_FILTER_CHAIN_RECORD_ERROR,
 LANG_DELETE_URL_CHAIN_RECORD_ERROR,
 LANG_ENCRYPT_VALUE_NOT_VALID,
 LANG_ENTER_VCI_0_65535,
 LANG_ENTER_VPI_0_255,
 LANG_ERROR_ADD_KEYWORD_FILTERING_CHAIN_RECORD,
 LANG_ERROR_ADD_URL_CHAIN_RECORD,
 LANG_ERROR_COMMENT_LENGTH_TOO_LONG,
 LANG_ERROR_INVALID_PSK_VALUE,
 LANG_ERROR_NO_MAC_ADDRESS_TO_SET,
 LANG_GET_JOIN_RESULT_FAILED,
 LANG_GET_WAN_INTERFACE_FAILED,
 LANG_GET_WRONG_BAND_MODE_VALUE,
 LANG_INTERNAL_ERROR_SET_WAN_MODE_FAILED,
 LANG_INVALID_AUTH_KEY_ID_0_255,
 LANG_INVALID_AUTH_KEY_LENGTH,
 LANG_INVALID_AUTH_KEY_VALUE_0_9A_FA_F,
 LANG_INVALID_AUTH_TYPE,
 LANG_INVALID_CDVT,
 LANG_INVALID_DETECT_MULT_1_255,
 LANG_INVALID_DNSV4_1_IP_ADDRESS_VALUE,
 LANG_INVALID_DNSV4_2_IP_ADDRESS_VALUE,
 LANG_INVALID_DSCP_0_63,
 LANG_INVALID_ENTERPRISE_NUMBER,
 LANG_INVALID_ETHERNET_PRIORITY_0_7,
 LANG_INVALID_IP_ADDRESS_IT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET,
 LANG_INVALID_IP_SUBNET_MASK_COMBINATION,
 LANG_INVALID_MBS,
 LANG_INVALID_MIN_ECHO_RX_INTERVAL,
 LANG_INVALID_MIN_RX_INTERVAL,
 LANG_INVALID_MIN_TX_INTERVAL,
 LANG_INVALID_PCR,
 LANG_INVALID_QOS,
 LANG_INVALID_SCR,
 LANG_INVALID_VCI,
 LANG_INVALID_VPI,
 LANG_INVALID_VPI_AND_VCI_00,
 LANG_INVALID_WEP_KEY_FORMAT_VALUE,
 LANG_INVALID_WEP_KEY_LENGTH,
 LANG_IPIP_VPN_INTERFACE_HAS_ALREADY_EXIST,
 LANG_JOIN_REQUEST_FAILED,
 LANG_L2TP_VPN_ACCOUNT_HAS_ALREADY_EXIST,
 LANG_L2TP_VPN_INTERFACE_HAS_ALREADY_EXIST,
 LANG_NO_RS_PORT_NUMBER,
 LANG_ONLY_SUPPORT_BOTH_OR_SINGLE_MODE_SWITCH,
 LANG_PLEASE_REFRESH_AGAIN,
 LANG_PPTP_VPN_ACCOUNT_HAS_ALREADY_EXIST,
 LANG_PPTP_VPN_INTERFACE_HAS_ALREADY_EXIST,
 LANG_PSK_LENGTH_ERROR_TOO_LONG,
 LANG_SCAN_REQUEST_TIMEOUT,
 LANG_SET_BAND_ERROR,
 LANG_SET_CONNECTION_REQUEST_PATH_ERROR,
 LANG_SET_CONNECTION_REQUEST_PORT_ERROR,
 LANG_SET_REGDOMAIN_ERROR,
 LANG_SET_UPNP_BINDED_WAN_INTERFACE_INDEX_ERROR,
 LANG_SET_UPNP_ERROR,
 LANG_SET_URL_CAPABILITY_ERROR,
 LANG_SET_WAN_INTERFACE_FAILED,
 LANG_SITE_SURVEY_REQUEST_FAILED,
 LANG_START_TR069_FAIL,
 LANG_SWAP_WLAN_MIB_FAILED,
 LANG_UPLOAD_FAILED,
 LANG_VLAN_ID_0__9_IS_RESERVED,
 LANG_VLAN_ID_DOES_NOT_MATCH_LAN_VLAN_ID_D,
 LANG_WARNING_WPA2_MIXED_ENCRYPTION_IS_NOT_SUPPORTED_IN_CLIENT_MODE_CHANGE_TO_WPA2_ENCRYPTION,
 LANG_CHANGE_SETTING_SUCCESSFULLY_DO_NOT_REBOOT,
 LANG_ERROR_NO_R0KH_R1KH_KEY_TO_SET,
 LANG_INVALID_LINK_MODE_VALUE,
 LANG_SORRY_THE_ACCOUNT_CANNOT_BE_DELETED,
 LANG_ERROR_USER_ALREADY_EXISTS,
 LANG_ERROR_INPUT_OLD_USER_PASSWORD_ERROR,
 LANG_ERROR_RESTORE_CONFIG_FILE_FAILED_UPLOADED_FILE_SIZE_OUT_OF_CONSTRAINT,
 LANG_ERROR_FLASH_READ_FAIL,
 LANG_ALLOCATE_BUFFER_FAILED,
 LANG_ERROR_FIND_THE_START_AND_END_OF_THE_UPLOAD_FILE_FAILED,
 LANG_ERROR_RESTORE_CONFIG_FILE_FAILED_INVALID_CONFIG_FILE,
 LANG_FLASH_ERROR,
 LANG_OK,
 LANG_S_SHOULD_BE_0_63,
 LANG_TD_S_NOT_A_NUMBER,
 LANG_PROHIBIT_ADMINISTRATORS_PASSWORD_NOT_SET,
 LANG_SYSTEM_LOG_NOT_EXISTS,
 LANG_SET_TIME_ERROR,
 LANG_PVC_CONFIGURATION_ERROR,
 LANG_IOCTL_ATM_OAM_LB_START_FAILED,
 LANG_IOCTL_ATM_OAM_LB_STATUS_FAILED,
 LANG_IOCTL_ATM_OAM_LB_STOP_FAILED,
 LANG_S_S_SUCCEED,
 LANG_S_S_FAILED,
 LANG_S_INSTALL_SUCCEED,
 LANG_S_INSTALL_FAILED,
 LANG_PING_SOCKET_CREATE_ERROR,
 LANG_UNKNOWN_ADDRESS_TYPE_ONLY_AF_INET_IS_CURRENTLY_SUPPORTED,
 LANG_ENTRY_ALREADY_EXISTS,
 LANG_GET_CHAIN_RECORD_ERROR,
 LANG_EF_CLASS_EXISTED_PLEASE_DELETE_EXISTED_ENTRY_FIRST,
 LANG_AF_CLASS_D_EXISTED_PLEASE_DELETE_EXISTED_ENTRY_FIRST,
 LANG_ERROR_CANT_RESOLVE_DEST_S,
 LANG_ERROR_INVALID_SUBNET_MASK_S,
 LANG_ERROR_CANT_RESOLVE_MASK_S,
 LANG_ERROR_METRIC,
 LANG_ERROR_IFNAME_ERROR_S,
 LANG_ERROR_CANT_RESOLVE_NEXT_THOP_S,
 LANG_ERROR_CANT_RESOLVE_NEXT_HOP_S,
 LANG_THIS_ROUTE_EXISTED_IN_MIB_ALREADY,
 LANG_SET_RIP_RECEIVE_MODE_ERROR,
 LANG_SET_RIP_SEND_MODE_ERROR,
 LANG_SET_RIP_ERROR,
 LANG_INVALID_PPP_ACTION,
 LANG_GET_LLIDENTRYNUM_ERROR,
 LANG_INVALID_IP_ADDRESS_IT_SHOULD_BE_SET_WITHIN_THE_CURRENT_SUBNET,
 LANG_SHOULD_BE,
 LANG_FORMAT,
 LANG_SHOULD_BE_SAME_SUBNET,
 LANG_NO_SET_ERROR,
 LANG_ADDR_CONFLICT,
 LANG_GAME_NAME_CONFLICT,
 LANG_AT_LEAST_SELECT_ONE,
 LANG_DEVICE_IS_ALREADY_EXIST_IN_TABLE,
 LANG_DESTINATION_IP_ADDRESS_SHOULD_BE_LARGER_THAN_SOURCE_IP_ADDRESS,
 LANG_ERROR_CAN_NOT_FIND_TR069_PCOCESS,
 LANG_ERROR_BAD_PASSWORD,
 LANG_ERROR_INVALID_USERNAME,
 LANG_ERROR_YOU_HAVE_LOGINED_ERROR_THREE_TIMES_PLEASE_RELOGIN_1_MINUTE_LATER,
 LANG_ERROR_ANOTHER_USER_HAVE_LOGINED_IN_USING_THIS_ACCOUNTONLY_ONE_USER_CAN_LOGIN_USING_THIS_ACCOUNT_AT_THE_SAME_TIME,
 LANG_ERROR_YOU_HAVE_LOGINED_PLEASE_LOGOUT_AT_FIRST_AND_THEN_LOGIN,
 LANG_ERROR_WEB_AUTHENTICATION_ERRORPLEASE_CLOSE_THIS_WINDOW_AND_REOPEN_YOUR_WEB_BROWSER_TO_LOGIN,
 LANG_ERROR_LOGOUT_FAILEDPERHAPS_YOU_HAVE_LOGOUTED,
 LANG_LOGOUT_ERROR,
 LANG_ERROR_SETTING,
 LANG_OK_START_TO_INFORM_ITMS_THAT_MAINTENANCE_IS_OVER,
 LANG_OK_LOGIN_SUCCESSFULLY,
 LANG_OK_LOGOUT_SUCCESSFULLY,
 LANG_LOGOUT_SUCCESSFULLY,
 LANG_TRAFFIC_CONTROLLING_QUEUE_IS_FULL_YOU_MUST_DELETE_SOME_ONE,
 LANG_WRONG_SETTING_IS_FOUND,
 LANG_ERROR_HAPPENED_WHEN_DELETING_RULE_D,
 LANG_FAILED_TO_DELETE_INITIATINGSTART_SCRIPT_FILE,
 LANG_UNEXPECTED_ERROR,
 LANG_FAILED_TO_DELETE_INITIATINGEND_SCRIPT_FILE,
 LANG_THE_INITIATINGSTART_SCRIPT_FILE_WAS_DELETED,
 LANG_THE_INITIATINGEND_SCRIPT_FILE_WAS_DELETED,
 LANG_YOU_CANNOT_ADD_ONE_NEW_RULE_WHEN_QUEUE_IS_FULL,
 LANG_ADDQOSRULEWRONG_ARGUMENT_FORMAT,
 LANG_UPDATEQOSRULE_WRONG_ARGUMENT_FORMAT,
 LANG_WRONG_OPERATION_HAPPENED_YOU_ONLY_AND_ADD_OR_EDIT_QOS_RULE_IN_THIS_PAGE,
 LANG_SYSTEM_DATE_HAS_BEEN_MODIFIED_SUCCESSFULLY_PLEASE_REFLESH_YOUR_STATUS_PAGE,
 LANG_READ_SITE_SURVEY_STATUS_FAILED,
 LANG_TAG_MAX
};
extern unsigned char g_language_state;
extern unsigned char g_language_state_prev;
extern const char **strtbl;
extern const char *multilang (int tag);
