#ifndef __RTK_TR142_H__
#define __RTK_TR142_H__

#include <rtk/qos.h>
#include <omci_dm_sd.h>

#ifdef CONFIG_SDK_KERNEL_LINUX
enum {
	TR142_LOG_MOD_NONE		= 0,
	TR142_LOG_MOD_QOS 		= (1 << 0),
	TR142_LOG_MOD_MCAST		= (1 << 1),
	TR142_LOG_MOD_ALL			= (TR142_LOG_MOD_QOS |
								TR142_LOG_MOD_MCAST)
};

enum {
	TR142_LOG_LEVEL_OFF = 0,
	TR142_LOG_LEVEL_ERROR,
	TR142_LOG_LEVEL_WARNING,
	TR142_LOG_LEVEL_INFO,
	TR142_LOG_LEVEL_DEBUG,
};

typedef struct {
	uint32 log_module;
	uint32 log_level;
	//others
} rtk_tr142_control_t;

extern rtk_tr142_control_t g_rtk_tr142_ctrl;

/* Macro */
#define TR142_LOG(module, level, fmt, arg...) \
    do { if ((g_rtk_tr142_ctrl.log_module & module) && \
		(g_rtk_tr142_ctrl.log_level >= level)) { printk(fmt, ##arg); } } while (0);
#endif

typedef struct  rtk_tr142_qos_queue_conf_s
{
	uint8 enable;
	rtk_qos_queue_type_t type;
	uint32 weight;
} rtk_tr142_qos_queue_conf_t;

typedef struct  rtk_tr142_qos_queues_s
{
    rtk_tr142_qos_queue_conf_t queue[WAN_PONMAC_QUEUE_MAX];
} rtk_tr142_qos_queues_t;


/* IOCTL commands */
#define RTK_TR142_MACIG 'R'
#define RTK_TR142_IOCTL_GET_QOS_QUEUES _IOR(RTK_TR142_MACIG, 0, rtk_tr142_qos_queues_t)
#define RTK_TR142_IOCTL_SET_QOS_QUEUES _IOW(RTK_TR142_MACIG, 1, rtk_tr142_qos_queues_t)

/* Error Codes */
#define RTK_TR142_ERR_OK					0
#define RTK_TR142_ERR_WAN_INFO_NULL		-1		// for rtk_tr142_pon_wan_info_set()
#define RTK_TR142_ERR_NO_MEM				-2
#define RTK_TR142_ERR_WAN_IDX_NOT_FOUND	-3		// for rtk_tr142_pon_wan_info_del()
#define RTK_TR142_ERR_PONQ_UNAVAILABLE		-4

#endif //__RTK_TR142_H__

