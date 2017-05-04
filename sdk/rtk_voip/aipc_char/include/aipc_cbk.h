#ifndef __AIPC_CBK_H__
#define __AIPC_CBK_H__

#define MAX_HANDLE_CNT 	(SIZE_2CPU_HIQ+SIZE_2CPU_LOWQ)
#ifndef MAX_HANDLE_CNT
#error "Need to define MAX_HANDLE_CNT"
#endif

extern int aipc_exe_callback( u32_t cmd , void *data );

//callback function
typedef int ( *aipc_callback )( u32_t cmd , void *data );


typedef struct {
	u32_t			cmd;
	aipc_callback	do_mgr;
} aipc_entry_t;

#define AIPC_MGR_BASE  (1)

/*
*	IPC Interrupt callback table index
*/
enum {
	AIPC_MGR_2CPU_MBOX_RECV = (AIPC_MGR_BASE+1),
	AIPC_MGR_2CPU_EVENT_RECV ,
	AIPC_MGR_2DSP_MBOX_RECV  ,
	AIPC_MGR_2DSP_EVENT_RECV ,
	AIPC_MGR_MAX
};

#endif

