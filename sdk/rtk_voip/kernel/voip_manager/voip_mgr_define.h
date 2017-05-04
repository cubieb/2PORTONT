#ifndef __VOIP_MGR_DEFINE_H__
#define __VOIP_MGR_DEFINE_H__

#include "rtk_voip.h"
#include "voip_types.h"

// MGR flags for IPC (host=bit0-7; host/dsp=bit8-16)
#define MF_NONE		0x0000	// none 
#define MF_AUTOFW	0x0001	// bit 0= (1) host auto forward or ignore below 
#define MF_CHANNEL	0x0002	// bit 1= (0) no channel (1) with specified channel 
#define MF_FETCH	0x0004	// bit 2= (0) no fetch (1) fetch values in structure 
#define MF_BODY		0x0008	// bit 3= (0) no function body (1) with function body 
#define MF_EVENTCMD	0x0100	// bit 8= (1) event type command 
#define MF_SNDCMD	0x0200	// bit 9= (1) snd command (host IPC use)

typedef uint32	mgr_flags_t;

// voip_mgr_table 
typedef int ( *p_do_mgr_t )( int cmd, void *user, unsigned int len, unsigned short seq_no );

typedef struct {
	int			cmd;
	p_do_mgr_t	do_mgr;
	const char *cmd_name;
	uint32		type_size;
	uint32		field_size;
	uint32		field_offset;
	mgr_flags_t	flags;
} voip_mgr_entry_t;

// copy_from_user / copy_to_user wrapper 
//   return value: number of bytes that can't be copied (==0 ok; >0 fail; <0 fail)
#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#define COPY_FROM_USER( dst, src, len )	\
		{	\
			if (copy_from_user( dst, src, len ))	\
				return -EVOIP_IOCTL_COPY_FROM_USER_ERR;	\
		}
#define COPY_TO_USER( dst, src, len, cmd, seq )		copy_to_user( dst, src, len )
#define NO_COPY_TO_USER( cmd, seq )					( 0 )
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
#define COPY_FROM_USER( dst, src, len )	\
		{	\
			if (copy_from_user( dst, src, len ))	\
				return -EVOIP_IOCTL_COPY_FROM_USER_ERR;	\
		}
#define COPY_TO_USER( dst, src, len, cmd, seq )		copy_to_user( dst, src, len )
#define NO_COPY_TO_USER( cmd, seq )					( 0 )
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
#define COPY_FROM_USER( dst, src, len )				({ memcpy( dst, src, len ); 0; })
#define COPY_TO_USER( dst, src, len, cmd, seq )		\
				(ipcSentResponsePacket(cmd, seq, src, len))
#define NO_COPY_TO_USER( cmd, seq )					\
				(ipcSentResponsePacket(cmd, seq, NULL, 0))	
#endif

#endif /* __VOIP_MGR_DEFINE_H__ */

