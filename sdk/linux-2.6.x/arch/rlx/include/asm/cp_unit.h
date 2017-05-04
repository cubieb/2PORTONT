#ifndef _ASM_CP_UNIT_H
#define _ASM_CP_UNIT_H


extern int _system_idle_wait;
extern int _count_cpu_idle;
extern int _count_cycle_in_while;

#define _asm_enable_cp3() ({  \
    __asm__  (                \
	"   mfc0	$8, $12		\n" \
	"   la		$9, 0x80000000	\n" \
	"   or		$8, $9		\n" \
	"   mtc0	$8, $12		\n" \
    );                              	\
})

//= rlxregs.h read_c0_count()
#define _asm_get_cp0_count() ({ \
    u32 __ret=0;                \
    __asm__ __volatile__ (      \
    "   mfc0    %0, $9  "       \
    : "=r"(__ret)               \
    );                          \
    __ret;                      \
})

#define _asm_freeze_cp3_events() ({ \
    u32 __event=0;                  \
    __asm__ __volatile__ (          \
    "   cfc3    %0, $0      \n"     \
    "   ctc3 	$0, $0      \n"     \
    : "=r"(__event)                 \
    );                              \
    __event;                        \
})

//stop counting and return the events
#define _asm_resume_cp3_events(events) ({   \
    u32 __events=(events);                  \
    __asm__ __volatile__ (                  \
    "   ctc3    %0, $0   \n"                \
    : : "r"(__events)                       \
    );                                      \
})

// You can't call this macro before do freeze_cp3_events
#define __asm_get_cp3_event_counter(reg_lo, reg_hi) ({  \
    u64 __ret=0;                                        \
    u32 __hi_value=0, __lo_value=0;                     \
    __asm__ __volatile__ (                              \
	"	mfc3	%1,	$" #reg_hi "\n"		\
	"	mfc3	%0,	$" #reg_lo "\n"         \
    : "=r"(__lo_value), "=r"(__hi_value)                \
    );                                                  \
    __ret=__hi_value;                                   \
    __ret<<32 | __lo_value;                             \
})

// You can't call this macro before do freeze_cp3_events
#define __asm_get_cp3_event_counter_lo(reg_lo) ({ \
    u32 __lo_value=0;                           \
    __asm__ __volatile__ (                      \
	"	mfc3	%0,	$" #reg_lo "    \n"         \
    : "=r"(__lo_value)                          \
    );                                          \
    __lo_value;                                 \
})

#define __asm_clear_cp3_event_counter(reg_lo, reg_hi) ({  \
    __asm__ __volatile__ (                              \
	"	mtc3	$0,	$" #reg_hi "\n"         \
	"	mtc3	$0,	$" #reg_lo "\n"         \
    );                                                  \
})

#define get_all_cp3_counters(con0, con1, con2, con3) ({  \
    u32 __org_events=0;                                 \
    __org_events=_asm_freeze_cp3_events();              \
    con0=__asm_get_cp3_event_counter(8, 9);             \
    con1=__asm_get_cp3_event_counter(10, 11);           \
    con2=__asm_get_cp3_event_counter(12, 13);           \
    con3=__asm_get_cp3_event_counter(14, 15);           \
    _asm_resume_cp3_events(__org_events);               \
})

#define get_all_cp3_counters_lo(con0, con1, con2, con3) ({ \
    u32 __org_events=0;                             \
    __org_events=_asm_freeze_cp3_events();          \
    con0=__asm_get_cp3_event_counter_lo(8);         \
    con1=__asm_get_cp3_event_counter_lo(10);        \
    con2=__asm_get_cp3_event_counter_lo(12);        \
    con3=__asm_get_cp3_event_counter_lo(14);        \
    _asm_resume_cp3_events(__org_events);           \
})

#define clear_all_cp3_event_counter() ({    \
    __asm_clear_cp3_event_counter(8, 9);    \
    __asm_clear_cp3_event_counter(10, 11);  \
    __asm_clear_cp3_event_counter(12, 13);  \
    __asm_clear_cp3_event_counter(14, 15);  \
})

#endif /* _ASM_CP_UNIT_H */

