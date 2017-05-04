/* == IDENTIFICATION ==========================================================
 *
 * Copyright (C) 2009, DSP Group Nuremberg
 *
 * System           : Vega Family
 * Component        : FP and PP
 * Module           : OS Extented
 * Unit             : Interface 
 * File             : cos00int.h
 *
 *
 * @(#)    %filespec: cos00int.h-2 %
 */

/* == HISTORY =================================================================
 *
 * Name      Date        V.     Action
 * --------------------------------------------------------------------
 *
 * tcmc_ore      3-Dec-2009 E27.6   Add OS00_GET_STATE_OF_PROCESS
 *                                  process or from interrupt
 * tcmc_ore     20-Aug-2009 E27.4   Remove H/LPRIO defines
 * tcmc_ore     19-Aug-2009 E27.3   Merged E27.2 and 14, added InvokeProcess()
 * tcmc_asa	04-May-2009 E27.2   Added p_os14/os10_PutMsgInstInfo part II
 * tcmc_asa	04-May-2009 E27.1   Added p_os14/os10_PutMsgInstInfo
 * yanivso	13-Sep-2009  14	New random generator in library.
 * tcmc_asn     23-Sep-2008  13 Declare G_u16_os00_SystemTime as volatile
 *               		to be compatible with RVDS (needed to compile
 *				VegaBB with RVDS)
 * tcmc_ore     16-Jul-2008  12 Add OS_EXTENSIONS_32BIT
 * tcmc_ore     24-Feb-2008  11 Guard OS_PRE_EMPTIVE switch against misuse.
 *                              Add p_os20_PrintKNLMsg
 * tcmc_ore     28-Oct-2004  10 Correct structure alignement for MemoryAdmin
 * tcmc_dwi     26-Apr-2004  9  Move NULL defintion to this file from 01 as it
 *                              is used in the interface
 * tcmc_ore      9-Dec-2003  8  Correct OS_EXTENSION switches for XS
 * tcmc_ore      2-Dec-2003  7  Make preemptive OS default except for !arm
 * tcmc_ore      4-Sep-2003  6  Restore SInstInfo procedure for OS_EXTENSIONS
 * tcmc_ore      2-Sep-2003  5  No SInstInfo procedure needed for OS_EXTENSIONS
 * tcmc_ore      2-Sep-2003  4  In FP we need >=15 instances, extra instance
 *                              byte needed.
 * tcmc_ore     28-Aug-2003  3  Added cast to (u8*) is MEMORY_BLOCK macro
 * tcmc_ore     26-Aug-2003  3  Add debug info to memory admin data
 * tcmc_ore     18-Aug-2003  2  Add OS_EXTENSIONS for D67
 * tcmc_ore     18-Aug-2003  1  Same as version 11 in basic OS
 * tcmc_ore     19-Feb-2003 11  PR183: Add TicksToNextTimeout in non-preemptive
 *                              OS 
 * tcmc_ore     31-Jan-2003 10  move MsgAdmin from cos04msg.h, add OS00_MSG_ACTIVE
 * tcmc_ore     28-Jan-2003  9  no PREEMPTIVE OS for VegaXS
 * tcmc_ore     23-Oct-2002     add p_os10_ForceState for WalkieTalkie
 * tcmc_ore     30-Aug-2001     add OS_TIMER_QUEUE_CHECK switch
 * tcmc_asa	02-Jul-2001	moved t_os00_Symbol to cos17dbg.h
 * tcmc_ore     10-May-2001     move ErrorInfo to cos09use.h, add HPrioScheduler
 * tcmc_asa	07-May-2001	removed u8_Reserved from t_os00_Symbol
 * tcmc_ore     24-Apr-2001     MsgActive Flag not needed
 * tcmc_ore     23-Apr-2001     remove p_os10_GetNextMsg from interface
 * tcmc_ore     20-Apr-2001     make OS preemptive
 * tcmc_asa     22-Jan-2001	moved _u8_os08_Log2File and 8 G_u8_os08_Trace2Term
 *				from cos08trc.h to cos00int.h
 * tcmc_asa	11-Dec-2000	added prototype for p_os14_GetNextMsg()
 * tcmc_ore     15-Nov-2000     Put global variables to strucure
 * tcmc_ore     02-Nov-2000     Created, based on os_proto.h
 */

/*MPM==========================================================================
 * 
 * ABSTRACT:
 *
 * This is the header file for the OS Interface                       
 *                                                                      
 * Global functions to be called by other modules:                       
 * ----------------------------------------------                       
 * - None                                                               
 *                                                                      
 * Module functions to be called by other units:           
 * ----------------------------------------------------------
 * - None
 *                                                                      
 * Module functions to be called only by this unit:       
 * --------------------------------------------------------------       
 * - None                                                               
 *                                                                      
 *
 */

#ifndef  cos00_h
# define cos00_h

# ifdef  cos10_c
#   define PUBLIC
# else
#   define PUBLIC extern
# endif

/* == DEFINITION OF COMPILER SWITCHES ===================================== */

/* This makes the pre-emptive OS default (needed for the online debug feature)
 * for all platforms except UNIX or if explicetly switched off via compiler
 * option 'NO_OS_PRE_EMPTIVE' (to be set in makefi
 */
#if !defined(NO_OS_PRE_EMPTIVE) && defined(__arm)
#ifndef OS_PRE_EMPTIVE
#define OS_PRE_EMPTIVE
#endif
#else
#ifdef OS_PRE_EMPTIVE
#error "Contradicting compile switches for OS_PRE_EMPTIVE"
#endif
#endif

/* This makes the extented OS default (needed for more than 16 process IDs)
 * for all platforms except for VegaXS or if explicetly switched off via
 * compiler option 'NO_OS_EXTENSIONS' (to be set in makefi
 */
#if !defined(VegaXS)
#ifndef NO_OS_EXTENSIONS
#define OS_EXTENSIONS
#if defined(VegaFB)
#define OS_EXTENSIONS_32BIT
#endif
#endif
#endif

#if !defined(OS_EXTENSIONS) && defined(OS_EXTENSIONS_32BIT)
#error "Illegal combination of VDSW OS compile switches"
#endif

#define OS_TIMER_QUEUE_CHECK

#ifndef   OS_DEBUG
/* DEBUG options (change only if OS Sources available !)
 * level 0: no debug                      
 * level 1: show sdl-transitions
 * level 2: enable sdl-debugger 
 * level 3: enable symbolic input in the sdl-debugger
 * level 4: enable messages with data,
 *          modifications of variables
 *          call of functions in the
 *          sdl-debugger
 * level 5: enable messages with special
 *          handled data in the sdl-debugger
 */
# ifdef   __arm
#  define OS_DEBUG               0
# else
#  define OS_DEBUG               5
# endif
#endif /* OS_DEBUG */

/* RUNTIME check options ( change only if OS Sources available !)
 * level 0: no checks
 * level 1: very important checks
 * level 2: all checks
 */
#ifndef   OS_RUNTIME_CHECK_LEVEL 
# ifdef   __arm
#  define OS_RUNTIME_CHECK_LEVEL 1
# else
#  define OS_RUNTIME_CHECK_LEVEL 2
# endif
#endif /* OS_RUNTIME_CHECK_LEVEL */


/* == DEFINITION OF MACROS ================================================= */
#ifndef NULL
  #define    NULL  ( ( void * )0 )
#endif

#define OS00_GET_STATE_OF_PROCESS(Process,Instance) G_u8_os09_StateTable[G_u8_os09_StateTableIndex[Process]+Instance]
/* == DEFINITION OF CONSTANTS ============================================== */
#define OS00_ILLEGAL_PROCESS 0xFF
#define OS00_LIFO_MSG 0x80

#ifdef OS_PRE_EMPTIVE
#define OS00_HPRIO_SCHEDULER 1
#define OS00_LPRIO_SCHEDULER 0
#endif
/* == DEFINITION OF ENUMERATIONS =========================================== */
    
/* == DECLARATION OF TYPES ================================================= */

typedef struct {
#ifdef OS_EXTENSIONS
  u8  u8_Process;    /* process to send a msg if timer expires */
#else
  u8  u8_ProcInst;  /* process to send a msg if timer expires */
#endif
  u8  u8_Event;     /* event to send and instance of receiver */
#ifdef OS_EXTENSIONS
  u8  u8_TInfo;      /* 8 bit information field available */
  u8  u8_Instance;
#endif
  u16 u16_Delay;    /* if zero, timer is not running          */
} t_st_os00_Timer ;


typedef struct {
#ifdef OS_EXTENSIONS
  u8   u8_Process;    /* process to send a msg if timer expires */
#else
  u8   u8_ProcInst;
#endif
  u8   u8_Event;
  u16  u16_Info;
#ifdef OS_EXTENSIONS
#ifdef OS_EXTENSIONS_32BIT
  u8   u8_Instance;
  u8   u8_Reserved[3];
  u8  *pu8_DataPtr;
#else
  u32  u32_DataPtrInst;
#endif
#endif  
} t_st_os00_Message ;       

typedef struct {
  u8   u8_Process;
  u8   u8_Instance;
  u8   u8_State;
  u8   u8_Event;
#ifdef OS_EXTENSIONS
  u8  *pu8_Data;
#endif
  u16 u16_XInfo;
  u16 u16_SInstInfo;
  u8   u8_SInst;
  u8   u8_Reserved;
}t_st_os00_Act;

  
typedef struct{
  t_st_os00_Message  *pst_MsgInPtr;   /* ptr to put next msg */
  t_st_os00_Message  *pst_MsgOutPtr;  /* ptr to read next msg */
  u8                 u8_MsgInQueue;   /* # msgs in queue */
#if OS_RUNTIME_CHECK_LEVEL >= 2
  u8                 u8_LIFOMsgInQueue;
  u8                 u8_Reserved[2];
#else
  u8                 u8_Reserved[3];
#endif
} t_st_os00_MsgAdmin;


#ifdef OS_EXTENSIONS

#if (OS_RUNTIME_CHECK_LEVEL >= 2) || (OS_DEBUG >= 2)
#define OS00_MEMORY_BLOCK(size,address)  {0,OS00_ILLEGAL_PROCESS,\
                                      OS00_ILLEGAL_PROCESS,0,0,size,(u8*)address}
#else
#define OS00_MEMORY_BLOCK(size,address)\
                                     {0,OS00_ILLEGAL_PROCESS,size,(u8*)address}
#endif

typedef struct{
  u8	u8_InUse;
  u8    u8_Reserved;
#if (OS_RUNTIME_CHECK_LEVEL >= 2) || (OS_DEBUG >= 2)
  u8	u8_Process;
  u8	u8_Instance;
  u16 	u16_RequestedSize;
#endif
  u16 	u16_Size;
  u8	*pu8_MemoryBlock;
} t_st_os00_MemoryAdmin;
#endif

#ifdef CP_ON_ARM
typedef u8    K_MSG_TYPE;
#endif
  
/* == DECLARATION OF GLOBAL DATA  ========================================== */
PUBLIC  t_st_os00_Act G_st_os00_Act;

PUBLIC struct {
  u8   u8_TimerActive;  /* at least one timer is running  */
  u8   u8_Reserved;		   		   
  u16 u16_TicksToNextTimeout; /* timer 'til next timer expires */ 
  u16 u16_Ticks;              /* timer ticks since last call to 
			       * os_guard_timers() */
}G_st_os00_RunControl;
  

PUBLIC          u8 G_u8_os00_Random[8];     /* for data encryption */

#if (defined (VegaBB) && defined (RVC_FOR_BB))
PUBLIC volatile u16 G_u16_os00_SystemTime;  /* timer ticks since systemstart */
#else
PUBLIC          u16 G_u16_os00_SystemTime;  /* timer ticks since systemstart */
#endif

PUBLIC volatile u8 G_u8_os00_SystemActive; /* combines os and drv activities */

PUBLIC t_st_os00_MsgAdmin G_st_os00_MsgAdmin;

#ifdef OS_PRE_EMPTIVE
PUBLIC t_st_os00_MsgAdmin G_st_os00_HPrioMsgAdmin;
#endif

#if OS_DEBUG
PUBLIC u8 G_u8_os08_Log2File;
PUBLIC u8 G_u8_os08_Trace2Term;
#endif /* OS_DEBUG */

/* == DEFINITION OF MACROS ================================================= */

#define OS00_MSG_ACTIVE  ((G_st_os00_MsgAdmin.u8_MsgInQueue) != 0)

/* The following makro enable or disable the pre-emptive task switching of the OS.
 * This should be used when global data is modified that is accessed from both,
 * low and high priority processes.
 */
#ifdef OS_PRE_EMPTIVE
PUBLIC u32 G_u32_os08_SoftIRQSave;
PUBLIC u32 G_u32_os08_SoftIRQSaveCount;  // for allowing nesting
#define OS00_DISABLE_SWITCH              {if(G_u32_os08_SoftIRQSaveCount++ == 0) {G_u32_os08_SoftIRQSave = \
                                            sys0_ICU.irq_enb & SYS0_ICU_INT_SOFT_IRQ;\
                                            sys0_ICU.irq_enbc = SYS0_ICU_INT_SOFT_IRQ;}}
#define OS00_ENABLE_SWITCH               {if(--G_u32_os08_SoftIRQSaveCount == 0) sys0_ICU.irq_enbs = G_u32_os08_SoftIRQSave;}
#else
#define OS00_DISABLE_SWITCH
#define OS00_ENABLE_SWITCH
#endif


/* Translator makros for the os functions */
#if defined (OLD_OS_RANDOM)
#define p_os10_Random                    p_os13_Random
#else
#define p_os10_Random() p_os31_RandGen(G_u8_os00_Random)
#endif
#define p_os10_StopTimer(W)                              p_os13_StopTimer(W)
#define p_os10_StopTimerForOtherProcess(P,I,W)           p_os13_StopTimerForOtherProcess(P,I,W)
#ifdef OS_EXTENSIONS
#define p_os10_StartTimer(W,X)                           p_os13_StartTimerInfo(W,X,0)
#define p_os10_StartTimerInfo(W,X,Z)                     p_os13_StartTimerInfo(W,X,Z)
#define p_os10_StartTimerForOtherProcess(P,I,W,X,Z)      p_os13_StartTimerInfoForOtherProcess(P,I,W,X,0)
#define p_os10_StartTimerInfoForOtherProcess(P,I,W,X,Z)  p_os13_StartTimerInfoForOtherProcess(P,I,W,X,Z)
#else
#define p_os10_StartTimer(W,X)                           p_os13_StartTimerInfo(W,X)
#define p_os10_StartTimerForOtherProcess(P,I,W,X)        p_os13_StartTimerInfoForOtherProcess(P,I,W,X)
#endif

#define p_os10_InvokeProcess(V,W,X,Y,Z)\
                                         p_os12_InvokeProcess(V,W,X,Y,Z)
#ifdef OS_EXTENSIONS
#define p_os10_PutMsgInfo(W,X,Y,Z)       p_os14_PutMsgInfoAndDataPtr(W,X,Y,Z,NULL)
#define p_os10_PutMsg(W,X,Y)             p_os14_PutMsgInfoAndDataPtr(W,X,Y,0,NULL)
#define p_os10_PutMsgHPrio(W,X,Y)        p_os14_PutMsgInfoAndDataPtr(W,(X|OS00_LIFO_MSG),Y,0,NULL)
#define p_os10_PutMsgDataPtr(V,W,X,Z)\
                                         p_os14_PutMsgInfoAndDataPtr(V,W,X,0,Z)
#define p_os10_PutMsgInfoAndDataPtr(V,W,X,Y,Z)\
                                         p_os14_PutMsgInfoAndDataPtr(V,W,X,Y,Z)
#else /* OS_EXTENSIONS */
#define p_os10_PutMsgInfo(W,X,Y,Z)       p_os14_PutMsgInfoAndDataPtr(W,X,Y,Z)
#define p_os10_PutMsg(W,X,Y)             p_os14_PutMsgInfoAndDataPtr(W,X,Y,0)
#define p_os10_PutMsgHPrio(W,X,Y)        p_os14_PutMsgInfoAndDataPtr(W,(X|OS00_LIFO_MSG),Y,0)
#endif /* OS_EXTENSIONS */
#define p_os10_PutMsgSInstInfo(W,X,Y,Z)  p_os14_PutMsgSInstInfo(W,X,Y,Z)
#define p_os10_PutMsgInstInfo(V,W,X,Y,Z) p_os14_PutMsgInstInfo(V,W,X,Y,Z)

#define p_os10_GuardTimers               p_os13_GuardTimers
#ifdef OS_PRE_EMPTIVE
#define p_os10_HPrioScheduler            p_os12_HPrioScheduler
#endif

#if defined(OS_EXTENSIONS) 
#define p_os10_AllocMemory(X)            p_os15_AllocMemory(X)
#define p_os10_FreeMemory(X)             p_os15_FreeMemory(X)
#endif

#ifdef CP_ON_ARM
#define p_os10_PrintKNLMsg(X,Y)          p_os20_PrintKNLMsg(X,Y)

#endif
/* == DECLARATION OF FUNCTION PROTOTYPES =================================== */
void p_os10_InitOS(void);

void p_os10_StopTimer(u8 u8_Event);
void p_os10_StopTimerForOtherProcess(u8 u8_Process, u8 u8_Instance, u8 u8_Event);
#ifdef OS_EXTENSIONS
void p_os10_StartTimerInfo(u16 u16_Time, u8 u8_Event, u8 u8_Info);
void p_os10_StartTimerInfoForOtherProcess(u8 u8_Process, u8 u8_Instance, u16 u16_Time, u8 u8_Event, u8 u8_Info);
#else
void p_os10_StartTimer(u16 u16_Time, u8 u8_Event);
void p_os10_StartTimerForOtherProcess(u8 u8_Process, u8 u8_Instance, u16 u16_Time, u8 u8_Event);
#endif

void p_os10_GuardTimers(void);
#ifdef OS_PRE_EMPTIVE
void p_os10_HPrioScheduler(void);
#endif

#ifdef OS_EXTENSIONS
void  p_os10_PutMsgInfoAndDataPtr(u8 u8_Process, u8 u8_Instance,
		                 u8 u8_Event, u16 u16_Info, u8 *pu8_Data);
void *p_os10_AllocMemory(u16 u16_Length);
void  p_os10_FreeMemory(void *pv_Memory);
#else
void  p_os10_PutMsgInfo(u8 u8_Process, u8 u8_Instance,
		                                   u8 u8_Event, u16 u16_Info);
#endif
void p_os10_PutMsgSInstInfo(u8 u8_Process,u8 u8_Instance,
			                           u8 u8_Event, u16 u16_Info);

/* This takes the parameter u8_SInst into the os00_Act, instead of the sender instance of
 * the process invoking this function here:
 */
void p_os10_PutMsgInstInfo(u8 u8_Process,u8 u8_Instance,
			                           u8 u8_Event, u16 u16_Info, u8 u8_SInst);

/* This allows to force the state of a given process to a defined state */
void p_os10_ForceState (u8 u8_Process, u8 u8_Instance, u8 u8_NewState);

void p_os10_InvokeProcess(u8 u8_Process, u8 u8_Instance, u8 u8_Event, u16 u16_Info, u8 u8_SInst );

#ifdef CP_ON_ARM
/* Print KNL message using OS_DEBUG */
void p_os20_PrintKNLMsg(void *v_Msg, u8 u8_ParameterLength);
#endif
     
#undef PUBLIC
#endif /* cos00_h */

/* == END OF MODULE =================================================== */
