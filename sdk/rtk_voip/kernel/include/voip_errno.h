#ifndef __VOIP_ERRNO_H__
#define __VOIP_ERRNO_H__

// This error number extend linux standard one, so we give 1024 
// to avoid conflict.  

#define EVOIP_BASE		1024

enum {
	EVOIP_RESEND_ERR = EVOIP_BASE,	// re-send 3 times fail 
	EVOIP_DUP_SEQ,		// receive duplicate seq 
	EVOIP_EVENT_EMPTY, 	// VoIP event is emtpy (user)
	EVOIP_EVENT_FULL, 	// VoIP event is full (kernel)
	EVOIP_EVENT_QUEUE_ERR,	// find event queue error  
	EVOIP_IPC_TX_ERR,	// ipc_pkt_tx_final fail
	EVOIP_IOCTL_NO_MGR_ERR,
	EVOIP_IOCTL_CMD_ERR,
	EVOIP_IOCTL_COPY_FROM_USER_ERR,
	EVOIP_IOCTL_COPY_TO_USER_ERR,
	EVOIP_IOCTL_PROCESS_ERR,
	EVOIP_IOCTL_CONFIG_RANGE_ERR,
	EVOIP_IOCTL_NOT_SUPPORT_ERR,
	EVOIP_IOCTL_SIZE_CHECK_ERR,
	EVOIP_IOCTL_CHID_CHECK_ERR,
	EVOIP_IOCTL_IPC_HOST_SIZE_CHECK_ERR,
	EVOIP_IOCTL_IPC_HOST_CHID_CHECK_ERR,
	EVOIP_IOCTL_IPC_HOST_GET_DSP_INFO_ERR,

};

#endif /* __VOIP_ERRNO_H__ */

