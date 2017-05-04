/* == IDENTIFICATION ==========================================================
 *
 * Copyright (C) 2009, DSP Group Nuernberg 
 *
 * System           : Vega Family
 * Component        : FP and PP
 * Module           : OS
 * Unit             : Trace Facility
 * File             : cos08trc.h
 *
 *
 * @(#)    %filespec: cos08trc.h-2 %
 */

/* == HISTORY =================================================================
 *
 * Name         Date        V.  Action
 * --------------------------------------------------------------------
 * tcmc_ore     18-Feb-2009 4   add G_u8_os08_TraceDisable
 * tcmc_ore     26-Sep-2007 3   add VDSW_MIPS_TRACER
 * tcmc_ore     12-Jan-2004 2   add OS08_TRACE_POINT
 * tcmc_ore     04-Jul-2001 1   rework after code CC by ASA
 * tcmc_ore     27-Apr-2001     updated for preemptive OS
 * tcmc_asa     22-Jan-2001	moved G_u8_os08_Log2File and ~Trace2Term
 *				from cos08trc.h to cos00int.h
 * tcmc_ore     02-Nov-2000     Created
 */

/*MPM==========================================================================
 * 
 * ABSTRACT:
 *
 * This is the header file for the trace feature
 *
 *
 * Global functions to be called by other modules:        
 * -----------------------------------------------
 * - p_os18_TraceFunc (macro OS08_TRACE_FUNC)
 *
 * Module functions to be called by other units:        
 * ---------------------------------------------
 *
 * - p_os18_TracePSI
 * - p_os18_TraceExit
 *
 * Unit functions to be called only by this unit:        
 * ----------------------------------------------
 * - None 
 *
 */


#ifndef  cos08_h
# define cos08_h

# ifdef  cos18_c
#   define PUBLIC
# else
#   define PUBLIC extern
# endif

/* == DEFINITION OF CONSTANTS ============================================== */

#define OS08_IDLE_PROCESS_ID         0xFF

#ifdef OS08_TRACE

#define OS08_IF               0x81
#define OS08_CASE             0x82
#define OS08_TASK             0x83
#define OS08_OUTPUT           0x84
#define OS08_OUTIPC           0x85
#define OS08_TIMER            0x86
#define OS08_SDL_CALL         0x87
#define OS08_SDL_RET          0x88
#define OS08_PSI              0x89
#define OS08_EXIT             0x8A
#define OS08_EXCEPTION        0x8B
#define OS08_MSG_CONTENTS     0x8C
#define OS08_DUMP             0x8D
#define OS08_FUNCTION_CALL    0x8E
#define OS08_POINT            0x8F
#define OS08_MSG_PUT          0x90


#define GLOBAL_MASK             0x00
#define LOCAL_MASK              0x01

#define OS08_INVOKED_INSTANCE_MASK 0x80 /* set in tracePSI if called from InvokeProcess() */

#endif /* OS08_TRACE */

#ifdef VDSW_MIPS_TRACER
/* Note these values are harcoded in ssw01irq/fiq.o files*/
#define VDSW_PROCESS_SWITCH   0
#define IRQ_ENTER             1
#define IRQ_LEAVE             2
#define FIQ_ENTER             3
#define FIQ_LEAVE             4

#ifdef VDSW_MIPS_TRACER_PER_INSTANCE /* not default, not tested */
#define IRQ_ID       (OS09_LEN_OF_STATE_TABLE  )
#define FIQ_ID       (OS09_LEN_OF_STATE_TABLE+1)
#define IDLE_ID      (OS09_LEN_OF_STATE_TABLE+2)
#define OVHD_ID      (OS09_LEN_OF_STATE_TABLE+3)
#define LAST_ID      (OS09_LEN_OF_STATE_TABLE+4)
#define PROCESS_IDX  (G_u8_os09_StateTableIndex[G_st_os00_Act.u8_Process] + G_st_os00_Act.u8_Instance)
#else
#define IRQ_ID       (OS09_NR_OF_PROCESSES     )
#define FIQ_ID       (OS09_NR_OF_PROCESSES+1   )
#define IDLE_ID      (OS09_NR_OF_PROCESSES+2   )
#define OVHD_ID      (OS09_NR_OF_PROCESSES+3   )
#define LAST_ID      (OS09_NR_OF_PROCESSES+4   )
#define PROCESS_IDX  (G_st_os00_Act.u8_Process )
#endif
#endif /* VDSW_MIPS_TRACER */

/* == DEFINITION OF MACROS ================================================= */

/* == DEFINITION OF ENUMERATIONS =========================================== */
    
/* == DECLARATION OF TYPES ================================================= */
#ifdef OS08_TRACE

#pragma pack(push, 1)

typedef struct
{
  u8    u8_State;
  u8    u8_Event;
  u8    u8_Instance;
  u16  u16_XInfo;
  u16  u16_SystemTime;
} t_st_os08_PSI;

typedef struct
{
  u8    u8_Event;
  u8    u8_Instance;
  u8    u8_SrcProc;
  u8    u8_SrcInst;
  u16  u16_XInfo;
  u16  u16_SystemTime;
} t_st_os08_MsgPut;

typedef struct
{
  u8    u8_Event;
  u16  u16_XInfo;
  u16  u16_SystemTime;
} t_st_os08_TracePoint;

typedef struct
{
  u8  u8_State;
  u8  u8_Event;
  u8  u8_Time;
  u8  u8_NrOfBytes;
} t_st_os08_FuncCall;

typedef struct
{
  u8   u8_State;
  u8   u8_WasIdle;
  u16 u16_PDTime;
  u16 u16_SystemTime;
} t_st_os08_Exit;

typedef struct
{
  u8   u8_State;
  u8   u8_Event;
  u8   u8_Instance;
  u8   u8_Exception;
  u16 u16_SystemTime;
} t_st_os08_Exception;

typedef struct
{
  u8  u8_TraceType;
  u8  u8_Process;
  union  
  {
    t_st_os08_PSI          st_PSI;
    t_st_os08_MsgPut       st_MsgPut;
    t_st_os08_TracePoint   st_TracePoint;
    t_st_os08_FuncCall     st_FuncCall;
    t_st_os08_Exit         st_Exit;
    t_st_os08_Exception    st_Exception;
  }un_Payload;
} t_st_os08_TraceBuffer;

#pragma pack(pop)


# define OS08_SIZEOF_TRACE(t) (2+(sizeof t))
/* == DECLARATION OF GLOBAL DATA  ========================================== */

#ifdef OS08_TRACE
PUBLIC u8 G_u8_os08_TraceDisable; /* If set to 1, no data will be streamed */
#endif /* OS08_TRACE */
/* == DECLARATION OF GLOBALS     =========================================== */
/*                                                                           */

#ifdef PT_MODULE
PUBLIC u16  G_u16_os08_IdleCall;
PUBLIC u16  G_u16_os08_IdleTime;
#endif

#endif /* OS08_TRACE */

/* == DECLARATION OF FUNCTION PROTOTYPES =================================== */

#ifdef __arm
void p_os18_TracePSI(u32 u32_Invoked);
void p_os18_TraceExit(u32 u32_StateChange);
#else
#define p_os18_TracePSI(x)    /* nothing */
#define p_os18_TraceExit(x)   /* nothing */
#endif

#ifdef VDSW_MIPS_TRACER
void p_os18_MIPSTracer(u32 u32_What);
void p_os18_ShowMIPS(void);
#endif

#ifdef OS08_TRACE
#   define OS08_TRACE_FUNC(s,e,buf,n)   p_os18_TraceFunc(s,e,buf,n)
#   define OS08_TRACE_POINT(e,i)        p_os18_TracePoint(e,i)
#   define OS08_TRACE_MSG_PUT(p,i,e,x)  p_os18_TraceMsgPut(p,i,e,x)
void  p_os18_TraceFunc(u8 u8_State, u8 u8_Event, u8* pu8_Buffer, u8 u8_NrOfBytes);
void  p_os18_TracePoint( u8 u8_Event, u16 u16_Info );
void  p_os18_TraceMsgPut( u8 u8_Process, u8 u8_Instance, u8 u8_Event, u16 u16_Info );
#else
#   define OS08_TRACE_FUNC(s,e,buf,n)  /* nothing */
#   define OS08_TRACE_POINT(e,i)       /* nothing */
#   define OS08_TRACE_MSG_PUT(p,i,e,x)
#endif


#undef PUBLIC
#endif /* cos08_h */
/* == END OF MODULE ======================================================== */

