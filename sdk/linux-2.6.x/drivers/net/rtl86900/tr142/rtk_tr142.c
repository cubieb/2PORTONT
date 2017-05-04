#define __LINUX_KERNEL__

#include <linux/string.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/if_smux.h>

#include <rtk/init.h>
#include <hal/common/halctrl.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_liteRomeDriver.h>
#include <omci_dm_cb.h>
#include <rtk/rtk_tr142.h>

rtk_tr142_control_t g_rtk_tr142_ctrl;

rtk_gpon_schedule_info_t scheInfo;
#define WAN_PONMAC_QUEUE_ID_MAX (scheInfo.max_pon_queue)
omci_dmm_cb_t omci_cb;		//call back function for OMCI

static int major;
static char *rtk_tr142_dev_name = "rtk_tr142";
static dev_t rtk_tr142_dev = 0;
static struct cdev rtk_tr142_cdev;
static struct class *rtk_tr142_class = NULL;

static rtk_tr142_qos_queues_t queues;
static int is_queues_ready = 0;

typedef struct pon_wan_info_list_s
{
	omci_dm_pon_wan_info_t info;
	int acl_entry_idx[WAN_PONMAC_QUEUE_MAX];
	int swacl_entry_idx[WAN_PONMAC_QUEUE_MAX];
	int queue2ponqIdx[WAN_PONMAC_QUEUE_MAX];
	unsigned char ponq_used[WAN_PONMAC_QUEUE_MAX];
	int default_qos_queue;
	struct list_head list;
} pon_wan_info_list_t;
pon_wan_info_list_t pon_wan_list;

struct mutex update_qos_lock;

// Find the highest priority unused queue
static int get_next_ponq(pon_wan_info_list_t *node)
{
	int i;
	int qidx = -1;

	if(node == NULL)
		return -1;

	for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
	{
		if(node->ponq_used[i] == 0
			&& node->info.queueSts[i] >= 0
			&& node->info.queueSts[i] < WAN_PONMAC_QUEUE_ID_MAX)
		{
			if(qidx == -1 || node->info.queueSts[i] > node->info.queueSts[qidx])
				qidx = i;
		}
		else
			node->ponq_used[i] = 1;	//mark invalid pon queue
	}
	return qidx;
}

static int get_default_ponq(pon_wan_info_list_t *node)
{
	int i;
	int qidx = 0;

	if(node == NULL)
		return -1;

	for(i = 1 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
	{
		if(node->info.queueSts[i] >= 0
			&& node->info.queueSts[i] < WAN_PONMAC_QUEUE_ID_MAX)
		{
			if(node->info.queueSts[i] < node->info.queueSts[qidx])
				qidx = i;
		}
	}
	return qidx;
}

static int setup_flows(pon_wan_info_list_t *node)
{
	int i = 0;
	rtk_rg_aclFilterAndQos_t acl = {0};
	rtk_ponmac_queue_t ponq = {0};
	rtk_gpon_usFlow_attr_t flow_attr = {0};
	int ret = 0;

	if(node == NULL)
		return RTK_TR142_ERR_WAN_INFO_NULL;

	if(is_queues_ready)
	{
		int default_ponq;

		default_ponq = get_default_ponq(node);

		for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
		{
			int ponq_idx;

			if(queues.queue[i].enable == 0)
				continue;

			ponq_idx = node->queue2ponqIdx[i];

			if(ponq_idx == -1)
				break;

			// OMCI have mapped first queue & flow for us
			if(ponq_idx != 0)
			{
				flow_attr.gem_port_id = node->info.gemPortId;
				flow_attr.tcont_id = node->info.tcontId;
				flow_attr.type = RTK_GPON_FLOW_TYPE_ETH;
				ret = rtk_rg_gpon_usFlow_set(node->info.usFlowId[ponq_idx], &flow_attr);
				if(ret != 0)
				{
					TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
						"[tr142] rtk_rg_gpon_usFlow_set returns %d\n", ret);
				}

				ponq.schedulerId = node->info.tcontId;
				ponq.queueId = node->info.queueSts[ponq_idx];
				ret = rtk_rg_ponmac_flow2Queue_set(node->info.usFlowId[ponq_idx], &ponq);
				if(ret != 0)
				{
					TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
						"[tr142] rtk_rg_ponmac_flow2Queue_set returns %d\n", ret);
				}

				TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
					"[tr142] Map flow %d to queue %d\n", node->info.usFlowId[ponq_idx], node->info.queueSts[ponq_idx]);
			}

			memcpy(&acl, &node->info.rgAclEntry, sizeof(rtk_rg_aclFilterAndQos_t));
			if(acl.action_type == ACL_ACTION_TYPE_QOS)
			{
				// Default queue do not need to set internal priority
				//Default queue: last available pon queue
				// or last enabled user space QoS queue.
				if(ponq_idx != default_ponq && i != node->default_qos_queue)
				{
					TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
						"[tr142] Assign pri %d for flow %d on wanIdx %d, action_type = %d\n", 7-i, node->info.usFlowId[ponq_idx], node->info.wanIdx, acl.action_type);
					acl.filter_fields |= INTERNAL_PRI_BIT;
					acl.internal_pri = 7 - i;
				}
				acl.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
				acl.action_stream_id_or_llid = node->info.usFlowId[ponq_idx];
				ret = rtk_rg_aclFilterAndQos_add(&acl, &node->acl_entry_idx[ponq_idx]);
				if(ret != 0)
				{
					TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
						"[tr142] <%s:%d> rtk_rg_aclFilterAndQos_add error: %d\n", __FUNCTION__, __LINE__, ret);
				}
			}

			if(node->info.rgUsSwAclEntry.fwding_type_and_direction != 0)
			{
				memcpy(&acl, &node->info.rgUsSwAclEntry, sizeof(rtk_rg_aclFilterAndQos_t));
				if(acl.action_type == ACL_ACTION_TYPE_QOS)
				{
					if(ponq_idx != default_ponq || i != node->default_qos_queue)
					{
						TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
							"[tr142][SwAclEntry] Assign pri %d for flow %d on wanIdx %d, action_type = %d\n", 7-i, node->info.usFlowId[ponq_idx], node->info.wanIdx, acl.action_type);
						acl.filter_fields |= INTERNAL_PRI_BIT;
						acl.internal_pri = 7 - i;
					}

					acl.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
					acl.action_stream_id_or_llid = node->info.usFlowId[ponq_idx];
					ret = rtk_rg_aclFilterAndQos_add(&acl, &node->swacl_entry_idx[ponq_idx]);
					if(ret != 0)
					{
						TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
							"[tr142][SwAclEntry] <%s:%d> rtk_rg_aclFilterAndQos_add error: %d\n", __FUNCTION__, __LINE__, ret);
					}
				}
			}
		}
	}
	else
	{
		// QoS is disabled, use first pon queue and flow directly.
		memcpy(&acl, &node->info.rgAclEntry, sizeof(rtk_rg_aclFilterAndQos_t));
		if(acl.action_type == ACL_ACTION_TYPE_QOS)
		{
			TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
				"[tr142] Assign flow id %d on wanIdx %d, action_type = %d\n", node->info.usFlowId[0], node->info.wanIdx, acl.action_type);
			acl.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
			acl.action_stream_id_or_llid = node->info.usFlowId[0];
			ret = rtk_rg_aclFilterAndQos_add(&acl, &node->acl_entry_idx[0]);
			if(ret != 0 )
			{
				TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
					"[tr142] <%s:%d> rtk_rg_aclFilterAndQos_add error: %d\n", __FUNCTION__, __LINE__, ret);
			}
		}

		if(node->info.rgUsSwAclEntry.fwding_type_and_direction != 0)
		{
			memcpy(&acl, &node->info.rgUsSwAclEntry, sizeof(rtk_rg_aclFilterAndQos_t));
			if(acl.action_type == ACL_ACTION_TYPE_QOS)
			{
				TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
					"[tr142][SwAclEntry] Assign flow id %d on wanIdx %d, action_type = %d\n", node->info.usFlowId[0], node->info.wanIdx, acl.action_type);
				acl.qos_actions|= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
				acl.action_stream_id_or_llid = node->info.usFlowId[0];
				ret = rtk_rg_aclFilterAndQos_add(&acl, &node->swacl_entry_idx[0]);
				if(ret != 0)
				{
					TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
						"[tr142][SwAclEntry] <%s:%d> rtk_rg_aclFilterAndQos_add error: %d\n", __FUNCTION__, __LINE__, ret);
				}
			}
		}
	}

	return RTK_TR142_ERR_OK;
}

static int is_qos_enable(void)
{
	int i;

	for(i = 0 ; i< WAN_PONMAC_QUEUE_MAX ; i++)
	{
		if(queues.queue[i].enable)
			return 1;
	}
	return 0;
}


static int setup_ponmac_queue(pon_wan_info_list_t *node)
{
	int i;
	rtk_ponmac_queue_t ponq;
	int ponq_got = 0;
	int ponq_idx;

	if(is_queues_ready)
	{
		for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
		{
			if(queues.queue[i].enable == 0)
				continue;

			ponq_idx = get_next_ponq(node);
			if(ponq_idx == -1)
				break;

			node->default_qos_queue = i;

			ponq.schedulerId = node->info.tcontId;
			ponq.queueId = node->info.queueSts[ponq_idx];

			node->info.queueCfg[i].type = queues.queue[i].type;
			node->info.queueCfg[i].weight = queues.queue[i].weight;
			node->info.queueCfg[i].pir = 131071;

			rtk_ponmac_queue_add(&ponq, &node->info.queueCfg[i]);

			node->ponq_used[ponq_idx] = 1;
			node->queue2ponqIdx[i] = ponq_idx;
			ponq_got = 1;
		}
		if(ponq_got == 0)
			return RTK_TR142_ERR_PONQ_UNAVAILABLE;
	}
	else
	{
		node->ponq_used[0] = 1;
		node->queue2ponqIdx[0] = 0;
		node->default_qos_queue = 0;
	}

	TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
		"[tr142] Setup PON Queue Completed, default QoS queue: %d\n", node->default_qos_queue);
	return RTK_TR142_ERR_OK;
}

static void clear_pon_qos_conf_by_wan(pon_wan_info_list_t *wan_info)
{
	int i;
	rtk_gpon_usFlow_attr_t flow_attr = {0};

	TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_INFO,
		"[tr142] Clear wan node %p\n", wan_info);

	for(i=0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
	{
		if(wan_info->acl_entry_idx[i] >= 0)
		{
			TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
				"[tr142] Deleting acl entry %d\n", wan_info->acl_entry_idx[i]);
			rtk_rg_aclFilterAndQos_del(wan_info->acl_entry_idx[i]);
		}
		wan_info->acl_entry_idx[i] = -1;

		if(wan_info->swacl_entry_idx[i] >= 0)
		{
			TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
				"[tr142] Deleting sw acl entry %d\n", wan_info->swacl_entry_idx[i]);
			rtk_rg_aclFilterAndQos_del(wan_info->swacl_entry_idx[i]);
		}
		wan_info->swacl_entry_idx[i] = -1;

		// Do not clear first because OMCI will do this for us.
		if(i != 0)
		{
			rtk_rg_gpon_usFlow_get(wan_info->info.usFlowId[i], &flow_attr);
			if(flow_attr.gem_port_id != RTK_GPON_GEMPORT_ID_NOUSE)
			{
				flow_attr.gem_port_id = RTK_GPON_GEMPORT_ID_NOUSE;
				rtk_rg_gpon_usFlow_set(wan_info->info.usFlowId[i], &flow_attr);
			}
		}

		wan_info->queue2ponqIdx[i] = -1;
		wan_info->ponq_used[i] = 0;
	}

	wan_info->default_qos_queue = -1;
}

static void clear_pon_qos_conf(void)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;

	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		clear_pon_qos_conf_by_wan(node);
	}
}

static int update_pon_qos_by_wan(pon_wan_info_list_t *wan_info)
{
	int ret;

	if(wan_info == NULL)
		return RTK_TR142_ERR_WAN_INFO_NULL;

	clear_pon_qos_conf_by_wan(wan_info);

	ret = setup_ponmac_queue(wan_info);
	if(ret != RTK_TR142_ERR_OK)
	{
		TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
			"[tr142] setup_ponmac_queue failed. err = %d\n", ret);
		return ret;
	}

	ret = setup_flows(wan_info);

	return ret;
}

static void update_pon_qos(void)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;

	clear_pon_qos_conf();

	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		setup_ponmac_queue(node);
		setup_flows(node);
	}
}

int rtk_tr142_pon_wan_info_set(omci_dm_pon_wan_info_t *pon_wan_info)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;
	int updated = 0;
	int ret;

	if(pon_wan_info == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_ERROR,
			"[tr142] pon_wan_info is NULL, ignore it!\n");
		return RTK_TR142_ERR_WAN_INFO_NULL;
	}

	mutex_lock(&update_qos_lock);
	// Check we need to add a new one, or update existed one
	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		if(node->info.wanIdx == pon_wan_info->wanIdx)
		{
			// Update existed WAN
			memcpy(&node->info, pon_wan_info, sizeof(omci_dm_pon_wan_info_t));
			TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_INFO,
				"[tr142] %s Updating wan %d\n", __func__, pon_wan_info->wanIdx);
			updated = 1;
			break;
		}
	}

	if(!updated)
	{
		int i;

		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_INFO,
			"[tr142] %s Adding a new WAN %d\n", __func__, pon_wan_info->wanIdx);
		// Add a new one
		node = kmalloc(sizeof(pon_wan_info_list_t), GFP_KERNEL);
		if(node == NULL)
		{
			mutex_unlock(&update_qos_lock);
			return RTK_TR142_ERR_NO_MEM;
		}
		memset(node, 0, sizeof(pon_wan_info_list_t));

		// Initialization
		memcpy(&node->info, pon_wan_info, sizeof(omci_dm_pon_wan_info_t));
		for(i=0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
		{
			node->acl_entry_idx[i] = -1;
			node->swacl_entry_idx[i] = -1;
			node->queue2ponqIdx[i] = -1;
		}
		node->default_qos_queue = -1;

		list_add_tail(&(node->list), &(pon_wan_list.list));
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_DEBUG,
			"[tr142] Added WAN node %p\n", node);
	}

	ret = update_pon_qos_by_wan(node);
	if(!updated && ret != RTK_TR142_ERR_OK)
	{
		list_del(&(node->list));
		kfree(node);
	}
	mutex_unlock(&update_qos_lock);

	TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
		"[tr142] %s returns %d\n", __func__, ret);

	return ret;
}
//EXPORT_SYMBOL(rtk_tr142_pon_wan_info_set);


int rtk_tr142_pon_wan_info_del(unsigned int wanIdx)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;

	mutex_lock(&update_qos_lock);
	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		if(node->info.wanIdx == wanIdx)
		{
			clear_pon_qos_conf_by_wan(node);

			list_del(&node->list);
			kfree(node);
			mutex_unlock(&update_qos_lock);
			TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_INFO,
				"[tr142] wanIdx %d is deleted\n", wanIdx);
			return RTK_TR142_ERR_OK;
		}
	}
	mutex_unlock(&update_qos_lock);

	return RTK_TR142_ERR_WAN_IDX_NOT_FOUND;
}
//EXPORT_SYMBOL(rtk_tr142_pon_wan_info_del);


int rtk_tr142_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case RTK_TR142_IOCTL_GET_QOS_QUEUES:
		copy_to_user((char *)arg, (const void *)&queues, sizeof(rtk_tr142_qos_queues_t));
		break;

	case RTK_TR142_IOCTL_SET_QOS_QUEUES:
		{
			int i;
			mutex_lock(&update_qos_lock);
			copy_from_user((void *)&queues, (char *)arg, sizeof(rtk_tr142_qos_queues_t));
			for(i = 0 ; i< WAN_PONMAC_QUEUE_MAX ; i++)
			{
				TR142_LOG(TR142_LOG_MOD_QOS, TR142_LOG_LEVEL_DEBUG,
					"[tr142] queue %d: enable = %d, policy=%s, weight = %d\n",
					i, queues.queue[i].enable,
					(queues.queue[i].type == WFQ_WRR_PRIORITY) ? "WRR" : "STRICT",
					queues.queue[i].weight);
			}

			is_queues_ready = is_qos_enable();
			update_pon_qos();
			mutex_unlock(&update_qos_lock);
		}
		break;

	default:
		return -ENOTTY;
	}
	return 0;
}

static int proc_log_module_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int idx = 0;

	page[0] = '\0';
	idx = snprintf(page, PAGE_SIZE, "Realtek TR142 Log Modules: %d\n\n", g_rtk_tr142_ctrl.log_module);

	idx += snprintf(&page[idx], PAGE_SIZE - idx, "None: %d\n", TR142_LOG_MOD_NONE);
	idx += snprintf(&page[idx], PAGE_SIZE - idx, "QoS: %d\n", TR142_LOG_MOD_QOS);
	idx += snprintf(&page[idx], PAGE_SIZE - idx, "Multicast: %d\n", TR142_LOG_MOD_MCAST);
	idx += snprintf(&page[idx], PAGE_SIZE - idx, "ALL: %d\n", TR142_LOG_MOD_ALL);

	return strlen(page);
}

static int proc_log_module_write(struct file *filp, const char *buff,unsigned long len, void *data)
{
	char tmpbuf[64] = {0};
	int module;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		module = simple_strtol(tmpbuf, NULL, 10);

		g_rtk_tr142_ctrl.log_module = module;
	}

	return len;
}

static int proc_log_level_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int idx = 0;

	page[0] = '\0';
	idx = snprintf(page, PAGE_SIZE, "Realtek TR142 Log Level: %d\n\n", g_rtk_tr142_ctrl.log_level);

	idx += snprintf(&page[idx], PAGE_SIZE - idx, "Accept values: %d ~ %d\n"
		, TR142_LOG_LEVEL_OFF, TR142_LOG_LEVEL_DEBUG);

	return strlen(page);
}

static int proc_log_level_write(struct file *filp, const char *buff,unsigned long len, void *data)
{
	char tmpbuf[64] = {0};
	int level;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		level = simple_strtol(tmpbuf, NULL, 10);
		if(level < TR142_LOG_LEVEL_OFF || level > TR142_LOG_LEVEL_DEBUG)
		{
			TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
				"only accept %d ~ %d!\n", TR142_LOG_LEVEL_OFF, TR142_LOG_LEVEL_DEBUG);
			return -EFAULT;
		}

		g_rtk_tr142_ctrl.log_level = level;
	}

	return len;
}

static int proc_wan_info_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;
	int idx = 0;
	int cnt = 0;
	int i;

	page[0] = '\0';
	idx = snprintf(page, PAGE_SIZE, "Realtek TR142 Colleted WAN Info:\n");

	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "WAN[%d]:\n", cnt);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\twanIdx = %d\n", node->info.wanIdx);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\twanType = %d\n", node->info.wanType);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\tgemPortId = %d\n", node->info.gemPortId);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\ttcontId = %d\n", node->info.tcontId);

		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\tusFlowId =");
		for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX; i++)
			idx += snprintf(&page[idx], PAGE_SIZE - idx, " %d", node->info.usFlowId[i]);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\n");

		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\tqueueSts =");
		for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX; i++)
			idx += snprintf(&page[idx], PAGE_SIZE - idx, " %d", node->info.queueSts[i]);
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "\n");

		cnt++;
	}

	return strlen(page);
}


static int proc_queue_map_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	pon_wan_info_list_t *node = NULL, *tmp = NULL;
	int idx = 0;
	int i;

	page[0] = '\0';

	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		idx += snprintf(&page[idx], PAGE_SIZE - idx, "WAN Index %d:\n", node->info.wanIdx);
		if(is_queues_ready)
		{
			idx += snprintf(&page[idx], PAGE_SIZE - idx, "User Queue\tQueue Type\tStreamId\tGPON Queue\n");
			for(i = 0 ; i < WAN_PONMAC_QUEUE_MAX ; i++)
				idx += snprintf(&page[idx], PAGE_SIZE - idx, "Q%d(%s)\t%s\t\t%d\t\t%d\n", i+1,
				(queues.queue[i].enable) ? "Enabled" : "Disabled",
				(queues.queue[i].type == WFQ_WRR_PRIORITY) ? "WRR" : "STRICT",
				node->info.usFlowId[node->queue2ponqIdx[i]], node->info.queueSts[node->queue2ponqIdx[i]]);
			idx += snprintf(&page[idx], PAGE_SIZE - idx, "\n");
		}
		else
		{
			idx += snprintf(&page[idx], PAGE_SIZE - idx
				, "QoS is disabled, default queue: %d, streamId = %d\n\n"
				, node->info.queueSts[node->queue2ponqIdx[0]]
				, node->info.usFlowId[node->queue2ponqIdx[0]]);
		}
	}

	return strlen(page);
}


static struct proc_dir_entry *procfs;

static int rtk_tr142_init_procfs(void)
{
	struct proc_dir_entry *entry = NULL;

	/* create a directory */
	procfs = proc_mkdir(rtk_tr142_dev_name, NULL);
	if(procfs == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
			"Register /proc/%s failed\n", rtk_tr142_dev_name);
		return -ENOMEM;
	}

	entry = create_proc_entry("log_module", S_IFREG | S_IRWXU, procfs);
	if (entry == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
			"Register /proc/%s/log_module failed\n", rtk_tr142_dev_name);
		remove_proc_entry(rtk_tr142_dev_name, NULL);
		return -ENOMEM;
	}
	entry->read_proc = proc_log_module_read;
	entry->write_proc = proc_log_module_write;

	entry = create_proc_entry("log_level", S_IFREG | S_IRWXU, procfs);
	if (entry == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
			"Register /proc/%s/log_level failed\n", rtk_tr142_dev_name);
		remove_proc_entry(rtk_tr142_dev_name, NULL);
		return -ENOMEM;
	}
	entry->read_proc = proc_log_level_read;
	entry->write_proc = proc_log_level_write;

	entry = create_proc_read_entry("wan_info",  S_IRUGO, procfs, proc_wan_info_read, NULL);
	if (entry == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
			"Register /proc/%s/wan_info failed\n", rtk_tr142_dev_name);
		remove_proc_entry(rtk_tr142_dev_name, NULL);
		return -ENOMEM;
	}

	entry= create_proc_read_entry("queue_map",  S_IRUGO, procfs, proc_queue_map_read, NULL);
	if (entry == NULL)
	{
		TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
			"Register /proc/%s/queue_map failed\n", rtk_tr142_dev_name);
		remove_proc_entry(rtk_tr142_dev_name, NULL);
		return -ENOMEM;
	}
	return 0;
}


static struct file_operations rtk_tr142_fops =
{
	.ioctl = rtk_tr142_ioctl,
};

static int __init rtk_tr142_module_init(void)
{
	int err;

	//TBD default value for log, should be configurable
	g_rtk_tr142_ctrl.log_module = TR142_LOG_MOD_ALL;
	g_rtk_tr142_ctrl.log_level = TR142_LOG_LEVEL_INFO;

	rtk_gpon_scheInfo_get(&scheInfo);

	//create device node for ioctl
	err = alloc_chrdev_region(&rtk_tr142_dev, 0, 1, rtk_tr142_dev_name);
	if (err < 0)
	{
	        TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_ERROR,
				"alloc_chrdev_region() failed. err = %d\n", err);
	        return -1;
	}
	major = MAJOR(rtk_tr142_dev);

	rtk_tr142_class = class_create(THIS_MODULE, rtk_tr142_dev_name);
    if (rtk_tr142_class == NULL)
	{
		unregister_chrdev_region(rtk_tr142_dev, 1);
		return -1;
    }

	if (device_create(rtk_tr142_class, NULL, rtk_tr142_dev, NULL, rtk_tr142_dev_name) == NULL)
	{
		class_destroy(rtk_tr142_class);
		unregister_chrdev_region(rtk_tr142_dev, 1);
		return -1;
	}

	cdev_init(&rtk_tr142_cdev, &rtk_tr142_fops);

	if(cdev_add(&rtk_tr142_cdev, rtk_tr142_dev, 1) == -1)
	{
		device_destroy(rtk_tr142_class, rtk_tr142_dev);
		class_destroy(rtk_tr142_class);
		unregister_chrdev_region(rtk_tr142_dev, 1);
		return -1;
	}

	// Initialize data structures
	memset((void *)&queues, 0, sizeof(rtk_tr142_qos_queues_t));
	memset(&pon_wan_list, 0, sizeof(pon_wan_info_list_t));
	INIT_LIST_HEAD(&pon_wan_list.list);
	mutex_init(&update_qos_lock);

	err = rtk_tr142_init_procfs();

	// Register OMCI Dual Management API
	omci_cb.omci_dm_pon_wan_info_set = rtk_tr142_pon_wan_info_set;
	omci_cb.omci_dm_pon_wan_info_del = rtk_tr142_pon_wan_info_del;

	omci_dmm_cb_register(&omci_cb);

	TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_INFO,
		"Realtek TR-142 Module initialized. err = %d, max_ponq_id=%d\n", err, WAN_PONMAC_QUEUE_ID_MAX);
	return err;
}

static void __exit rtk_tr142_module_exit(void)
{
	pon_wan_info_list_t *node, *tmp = NULL;

	// remove /dev/rtk_tr142
	cdev_del(&rtk_tr142_cdev);
	device_destroy(rtk_tr142_class, rtk_tr142_dev);
	class_destroy(rtk_tr142_class);
	unregister_chrdev_region(rtk_tr142_dev, 1);

	// remove proc entries
	remove_proc_entry("wan_info", procfs);
	remove_proc_entry("queue_map", procfs);
	remove_proc_entry(rtk_tr142_dev_name, NULL);

	//unregister OMCI DM callback functions
	omci_dmm_cb_unregister();

	// free resources
	mutex_lock(&update_qos_lock);
	list_for_each_entry_safe(node, tmp, &(pon_wan_list.list), list)
	{
		clear_pon_qos_conf_by_wan(node);
		list_del(&node->list);
		kfree(node);
	}
	mutex_unlock(&update_qos_lock);
	mutex_destroy(&update_qos_lock);

	TR142_LOG(TR142_LOG_MOD_ALL, TR142_LOG_LEVEL_INFO,
		"Realtek TR-142 Module unregistered.\n");
}

module_init(rtk_tr142_module_init);
module_exit(rtk_tr142_module_exit);
MODULE_LICENSE("GPL");

