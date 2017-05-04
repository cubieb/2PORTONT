/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Define DS packet broadcaster
 *
 * Feature : Broadcast DS packet to lan interface
 */

/*
 * Include Files
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>

#include "hal/common/halctrl.h"
#include "rtk/switch.h"
#include "rtk/gponv2.h"
#include "rtk/svlan.h"

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif

#include "ds_pkt_bcaster.h"

/*
 * Data Declaration
 */
struct proc_dir_entry *g_procDir = NULL;
struct proc_dir_entry *g_procEntryGem = NULL;
struct proc_dir_entry *g_procEntryRule = NULL;


struct file_operations g_procEntryGem_fop;
struct file_operations g_procEntryRule_fop;

uint32              g_dsBcGemFlowId;
struct list_head    g_tagOperationHead;
uint32              g_chipId;
uint32              g_ponPort;
uint8               g_numOfStagTpid;

/*
 * Function Declaration
 */
extern int drv_nic_register_rxhook(int portmask, int priority, p2rfunc_t rx);
extern int drv_nic_unregister_rxhook(int portmask, int priority, p2rfunc_t rx);
extern int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo);

static int ds_pkt_bcaster_pkt_xmit_to_nic(
        uint8 *pData, uint32 len, uint32 txPortMask)
{
    struct tx_info  txInfo;

    memset(&txInfo, 0, sizeof(struct tx_info));

    txInfo.opts1.bit.keep = 1;
    txInfo.opts1.bit.dislrn = 1;
    txInfo.opts2.bit.cputag = 1;
    txInfo.opts3.bit.tx_portmask = txPortMask;

    return re8686_tx_with_Info(pData, len, &txInfo);
}

static int ds_pkt_bcaster_pkt_modify_and_xmit(
        uint8 *pData, uint32 len, uint16 *pSTag, uint16 *pCTag, tagOperation_t *pTagOp)
{
    struct vlan_ethhdr  *pVlanTag;
    uint16              vlanTci;
    uint8               *pBuf;
    uint8               *pBufOffset;
    uint32              remainedDataLen;
    int32               ret = RT_ERR_NOT_ALLOWED;

    pBuf = kmalloc(len, GFP_ATOMIC);
    if (unlikely(!pBuf))
    {
        printk(KERN_ERR "[dpbcaster] memory allocation fail, stop immediately\n");

        return RT_ERR_FAILED;
    }
    pBufOffset = pBuf;

    // copy eth header
    memcpy(pBufOffset, pData, ETH_ALEN * 2);
    pBufOffset += ETH_ALEN * 2;
    pData += ETH_ALEN * 2;

    // keep calculating remained len
    remainedDataLen = len - ETH_ALEN * 2;

    // deal with s-tag
    if (TAG_TREATMENT_MODE_NOP == pTagOp->tagTreatment.sTagMode)
    {
        if (pSTag)
        {
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
        }
    }
    else if (TAG_TREATMENT_MODE_REMOVE == pTagOp->tagTreatment.sTagMode)
    {
        if (pSTag)
        {
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
            len -= VLAN_HLEN;
        }
        else
        {
            printk(KERN_WARNING "[dpbcaster] remove stag while stag is not exist\n");
            goto free_buf;
        }
    }
    else if (TAG_TREATMENT_MODE_INSERT == pTagOp->tagTreatment.sTagMode)
    {
        if (pSTag)
        {
            printk(KERN_WARNING "[dpbcaster] insert stag while stag is exist\n");
            goto free_buf;
        }
        else
        {
            pVlanTag = (struct vlan_ethhdr *)pBuf;
            pVlanTag->h_vlan_proto = htons(pTagOp->tagTreatment.sTagField.tagTpid);
            memset(&vlanTci, 0, sizeof(vlanTci));

            switch (pTagOp->tagTreatment.sTagAction.priAction)
            {
                case TAG_TREATMENT_PRI_ACTION_INSERT:
                    vlanTci = m_set_tci_pri(vlanTci, pTagOp->tagTreatment.sTagField.tagPri);
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy pri from stag while stag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_OUTER:
                    if (pCTag)
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy pri from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_PRI_ACTION_DERIVE_FROM_DSCP:
                    printk(KERN_WARNING "[dpbcaster] copy pri from dscp is not supported\n");
                    goto free_buf;
                case TAG_TREATMENT_PRI_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] pri action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.sTagAction.deiAction)
            {
                case TAG_TREATMENT_DEI_ACTION_INSERT:
                    vlanTci = m_set_tci_dei(vlanTci, pTagOp->tagTreatment.sTagField.tagDei);
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy dei from stag while stag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_OUTER:
                    if (pCTag)
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy dei from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_DEI_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] dei action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.sTagAction.vidAction)
            {
                case TAG_TREATMENT_VID_ACTION_INSERT:
                    vlanTci = m_set_tci_vid(vlanTci, pTagOp->tagTreatment.sTagField.tagVid);
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy vid from stag while stag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_OUTER:
                    if (pCTag)
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy vid from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_VID_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] vid action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            pVlanTag->h_vlan_TCI = htons(vlanTci);
            pBufOffset += VLAN_HLEN;
            len += VLAN_HLEN;
        }
    }
    else if (TAG_TREATMENT_MODE_MODIFY == pTagOp->tagTreatment.sTagMode)
    {
        if (pSTag)
        {
            pVlanTag = (struct vlan_ethhdr *)pBuf;
            memcpy(&pVlanTag->h_vlan_proto, pSTag - 1, sizeof(pVlanTag->h_vlan_proto));
            memcpy(&vlanTci, pSTag, sizeof(vlanTci));

            switch (pTagOp->tagTreatment.sTagAction.priAction)
            {
                case TAG_TREATMENT_PRI_ACTION_INSERT:
                    vlanTci = m_set_tci_pri(vlanTci, pTagOp->tagTreatment.sTagField.tagPri);
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_INNER:
                    if (pCTag)
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy pri from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_OUTER:
                    vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pSTag));
                    break;
                case TAG_TREATMENT_PRI_ACTION_DERIVE_FROM_DSCP:
                    printk(KERN_WARNING "[dpbcaster] copy pri from dscp is not supported\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.sTagAction.deiAction)
            {
                case TAG_TREATMENT_DEI_ACTION_INSERT:
                    vlanTci = m_set_tci_dei(vlanTci, pTagOp->tagTreatment.sTagField.tagDei);
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_INNER:
                    if (pCTag)
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy dei from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_OUTER:
                    vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pSTag));
                    break;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.sTagAction.vidAction)
            {
                case TAG_TREATMENT_VID_ACTION_INSERT:
                    vlanTci = m_set_tci_vid(vlanTci, pTagOp->tagTreatment.sTagField.tagVid);
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_INNER:
                    if (pCTag)
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy vid from ctag while ctag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_OUTER:
                    vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pSTag));
                    break;
                default:
                    break;
            }

            pVlanTag->h_vlan_TCI = htons(vlanTci);
            pBufOffset += VLAN_HLEN;
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
        }
        else
        {
            printk(KERN_WARNING "[dpbcaster] modify stag while stag is not exist\n");
            goto free_buf;
        }
    }

    // deal with c-tag
    if (TAG_TREATMENT_MODE_NOP == pTagOp->tagTreatment.cTagMode)
    {
        if (pCTag)
        {
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
        }
    }
    else if (TAG_TREATMENT_MODE_REMOVE == pTagOp->tagTreatment.cTagMode)
    {
        if (pCTag)
        {
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
            len -= VLAN_HLEN;
        }
        else
        {
            printk(KERN_WARNING "[dpbcaster] remove ctag while ctag is not exist\n");
            goto free_buf;
        }
    }
    else if (TAG_TREATMENT_MODE_INSERT == pTagOp->tagTreatment.cTagMode)
    {
        if (pCTag)
        {
            printk(KERN_WARNING "[dpbcaster] insert ctag while ctag is exist\n");
            goto free_buf;
        }
        else
        {
            pVlanTag = (struct vlan_ethhdr *)pBuf;
            pVlanTag->h_vlan_proto = htons(ETH_P_8021Q);
            memset(&vlanTci, 0, sizeof(vlanTci));

            switch (pTagOp->tagTreatment.cTagAction.priAction)
            {
                case TAG_TREATMENT_PRI_ACTION_INSERT:
                    vlanTci = m_set_tci_pri(vlanTci, pTagOp->tagTreatment.cTagField.tagPri);
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy pri from ctag while ctag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pSTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy pri from stag while stag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_PRI_ACTION_DERIVE_FROM_DSCP:
                    printk(KERN_WARNING "[dpbcaster] copy pri from dscp is not supported\n");
                    goto free_buf;
                case TAG_TREATMENT_PRI_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] pri action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.cTagAction.deiAction)
            {
                case TAG_TREATMENT_DEI_ACTION_INSERT:
                    vlanTci = m_set_tci_dei(vlanTci, pTagOp->tagTreatment.cTagField.tagDei);
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy dei from ctag while ctag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pSTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy dei from stag while stag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_DEI_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] dei action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.cTagAction.vidAction)
            {
                case TAG_TREATMENT_VID_ACTION_INSERT:
                    vlanTci = m_set_tci_vid(vlanTci, pTagOp->tagTreatment.cTagField.tagVid);
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_INNER:
                    printk(KERN_WARNING "[dpbcaster] copy vid from ctag while ctag is not exist\n");
                    goto free_buf;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pSTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy vid from stag while stag is not exist\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_VID_ACTION_NOP:
                    printk(KERN_WARNING "[dpbcaster] vid action cannot be nop while inserting tag\n");
                    goto free_buf;
                default:
                    break;
            }

            pVlanTag->h_vlan_TCI = htons(vlanTci);
            pBufOffset += VLAN_HLEN;
            len += VLAN_HLEN;
        }
    }
    else if (TAG_TREATMENT_MODE_MODIFY == pTagOp->tagTreatment.cTagMode)
    {
        if (pCTag)
        {
            pVlanTag = (struct vlan_ethhdr *)pBuf;
            pVlanTag->h_vlan_proto = htons(ETH_P_8021Q);
            memcpy(&vlanTci, pCTag, sizeof(vlanTci));

            switch (pTagOp->tagTreatment.cTagAction.priAction)
            {
                case TAG_TREATMENT_PRI_ACTION_INSERT:
                    vlanTci = m_set_tci_pri(vlanTci, pTagOp->tagTreatment.cTagField.tagPri);
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_INNER:
                    if (pSTag)
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy pri from inner while it's single-tagged\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_PRI_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pSTag));
                    else
                        vlanTci = m_set_tci_pri(vlanTci, m_get_tci_pri(*pCTag));
                    break;
                case TAG_TREATMENT_PRI_ACTION_DERIVE_FROM_DSCP:
                    printk(KERN_WARNING "[dpbcaster] copy pri from dscp is not supported\n");
                    goto free_buf;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.cTagAction.deiAction)
            {
                case TAG_TREATMENT_DEI_ACTION_INSERT:
                    vlanTci = m_set_tci_dei(vlanTci, pTagOp->tagTreatment.cTagField.tagDei);
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_INNER:
                    if (pSTag)
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy dei from inner while it's single-tagged\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_DEI_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pSTag));
                    else
                        vlanTci = m_set_tci_dei(vlanTci, m_get_tci_dei(*pCTag));
                    break;
                default:
                    break;
            }

            switch (pTagOp->tagTreatment.cTagAction.vidAction)
            {
                case TAG_TREATMENT_VID_ACTION_INSERT:
                    vlanTci = m_set_tci_vid(vlanTci, pTagOp->tagTreatment.cTagField.tagVid);
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_INNER:
                    if (pSTag)
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pCTag));
                    else
                    {
                        printk(KERN_WARNING "[dpbcaster] copy vid from inner while it's single-tagged\n");
                        goto free_buf;
                    }
                    break;
                case TAG_TREATMENT_VID_ACTION_COPY_FROM_OUTER:
                    if (pSTag)
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pSTag));
                    else
                        vlanTci = m_set_tci_vid(vlanTci, m_get_tci_vid(*pCTag));
                    break;
                default:
                    break;
            }

            pVlanTag->h_vlan_TCI = htons(vlanTci);
            pBufOffset += VLAN_HLEN;
            pData += VLAN_HLEN;
            remainedDataLen -= VLAN_HLEN;
        }
        else
        {
            printk(KERN_WARNING "[dpbcaster] modify ctag while ctag is not exist\n");
            goto free_buf;
        }
    }

    // copy the rest of payload
    memcpy(pBufOffset, pData, remainedDataLen);

    // invoke transmission
    ret = ds_pkt_bcaster_pkt_xmit_to_nic(pBuf, len, pTagOp->dstPortMask);
    if (unlikely(ret < RT_ERR_OK))
        ret = RT_ERR_FAILED;

free_buf:
    kfree(pBuf);

    return ret;
}

int ds_pkt_bcaster_ds_bc_gem_pkt_rx(
        struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
    tagOperation_entry_t    *pCursor;
    tagOperation_t          *pTagOp;
    uint16                  *p1stTag;
    uint16                  *p2ndTag;
    uint16                  *pSTag;
    uint16                  *pCTag;
    rtk_enable_t            sTagTpidEn;
    uint32                  sTagTpid;
    uint8                   i;
    int32                   ret;

    // only deal with packet which comes from ds bc gem
    if (pRxInfo->opts2.bit.pon_stream_id == g_dsBcGemFlowId)
    {
        if (APOLLOMP_CHIP_ID == g_chipId)
        {
            // insert tag if GMAC RxVLAN de-tagging enabled and the tag is presented
            if (((*(volatile uint32 *)(0xB8012038)) & 0x4) && pRxInfo->opts2.bit.ctagva)
            {
                #if defined(CONFIG_KERNEL_2_6_30)
                if (unlikely(NULL == vlan_put_tag(skb, pRxInfo->opts2.bit.cvlan_tag)))
                #else
                if (unlikely(NULL == vlan_insert_tag_set_proto(skb, htons(ETH_P_8021Q), pRxInfo->opts2.bit.cvlan_tag)))
                #endif
                {
                    // insert vlan error!?
                    return RE8670_RX_CONTINUE;
                }
            }
        }

        // stop immediately for extremely short packet
        if (unlikely(skb->len < ETH_HLEN))
        {
            printk(KERN_ERR "[dpbcaster] skb->len < %d, stop immediately\n", ETH_HLEN);

            return RE8670_RX_STOP;
        }

        // assign pointers for tag recognition
        p1stTag = (uint16 *)(skb->data + ETH_ALEN * 2);
        if (unlikely(skb->len < VLAN_ETH_HLEN))
            p2ndTag = NULL;
        else
            p2ndTag = p1stTag + 2;

        // check if s-tag exists
        pSTag = NULL;
        for (i = 0; i < g_numOfStagTpid; i++)
        {
            if (RTL9601B_CHIP_ID != g_chipId)
            {
                if (RT_ERR_OK != rtk_svlan_tpidEnable_get(i, &sTagTpidEn))
                    continue;

                if (ENABLED != sTagTpidEn)
                    continue;
            }

            if (RT_ERR_OK != rtk_svlan_tpidEntry_get(i, &sTagTpid))
                continue;

            if (sTagTpid == *p1stTag)
            {
                pSTag = p1stTag + 1;

                break;
            }
        }

        // check if c-tag exists
        pCTag = NULL;
        if (unlikely(pSTag))
        {
            if (p2ndTag && ETH_P_8021Q == *p2ndTag)
                pCTag = p2ndTag + 1;
        }
        else
        {
            if (ETH_P_8021Q == *p1stTag)
                pCTag = p1stTag + 1;
        }

        list_for_each_entry(pCursor, &g_tagOperationHead, list)
        {
            pTagOp = &pCursor->data;

            // check s-tag mode
            if ((TAG_FILTER_MODE_NO_TAG == pTagOp->tagFilter.sTagMode && pSTag) ||
                    (TAG_FILTER_MODE_TAG == pTagOp->tagFilter.sTagMode && !pSTag))
                continue;

            // check s-tag mask and field
            if (TAG_FILTER_MODE_TAG == pTagOp->tagFilter.sTagMode)
            {
                if (TAG_FILTER_FIELD_PRI & pTagOp->tagFilter.sTagMask &&
                        pTagOp->tagFilter.sTagField.tagPri != m_get_tci_pri(*pSTag))
                    continue;

                if (TAG_FILTER_FIELD_DEI & pTagOp->tagFilter.sTagMask &&
                        pTagOp->tagFilter.sTagField.tagDei != m_get_tci_dei(*pSTag))
                    continue;

                if (TAG_FILTER_FIELD_VID & pTagOp->tagFilter.sTagMask &&
                        pTagOp->tagFilter.sTagField.tagVid != m_get_tci_vid(*pSTag))
                    continue;

                if (TAG_FILTER_FIELD_TPID & pTagOp->tagFilter.sTagMask &&
                        pTagOp->tagFilter.sTagField.tagTpid != *(pSTag - 1))
                    continue;
            }

            // check c-tag mode
            if ((TAG_FILTER_MODE_NO_TAG == pTagOp->tagFilter.cTagMode && pCTag) ||
                    (TAG_FILTER_MODE_TAG == pTagOp->tagFilter.cTagMode && !pCTag))
                continue;

            // check c-tag mask and field
            if (TAG_FILTER_MODE_TAG == pTagOp->tagFilter.cTagMode)
            {
                if (TAG_FILTER_FIELD_PRI & pTagOp->tagFilter.cTagMask &&
                        pTagOp->tagFilter.cTagField.tagPri != m_get_tci_pri(*pCTag))
                    continue;

                if (TAG_FILTER_FIELD_DEI & pTagOp->tagFilter.cTagMask &&
                        pTagOp->tagFilter.cTagField.tagDei != m_get_tci_dei(*pCTag))
                    continue;

                if (TAG_FILTER_FIELD_VID & pTagOp->tagFilter.cTagMask &&
                        pTagOp->tagFilter.cTagField.tagVid != m_get_tci_vid(*pCTag))
                    continue;

                if (TAG_FILTER_FIELD_TPID & pTagOp->tagFilter.cTagMask &&
                        pTagOp->tagFilter.cTagField.tagTpid != *(pCTag - 1))
                    continue;
            }

            ret = ds_pkt_bcaster_pkt_modify_and_xmit(
                    skb->data, skb->len, pSTag, pCTag, pTagOp);

            if (RT_ERR_OK == ret)
            {
                // successfully hit
                pCursor->hit++;
            }
            else
            {
                printk(KERN_ERR "[dpbcaster] pkt modify and xmit fail, ret = %d\n", ret);
            }
        }

        return RE8670_RX_STOP;
    }

    return RE8670_RX_CONTINUE;
}

int ds_pkt_bcaster_ds_bc_gem_flow_id_set(uint32 gemFlowId)
{
    g_dsBcGemFlowId = gemFlowId;

    return RT_ERR_OK;
}

int ds_pkt_bcaster_ds_tag_operation_add(tagOperation_t *pData, uint32 idx)
{
    tagOperation_entry_t    *pCursor;
    tagOperation_entry_t    *pNext;
    tagOperation_entry_t    *pEntry;

    if (!pData)
        return RT_ERR_FAILED;

    pEntry = kmalloc(sizeof(tagOperation_entry_t), GFP_KERNEL);
    if (!pEntry)
        return RT_ERR_FAILED;

    memcpy(&pEntry->data, pData, sizeof(tagOperation_t));
    pEntry->idx = idx;

    list_for_each_entry_safe(pCursor, pNext, &g_tagOperationHead, list)
    {
        // insert entry while keeping ascended order
        if (pCursor->idx > idx)
        {
            pCursor->list.prev->next = &pEntry->list;
            pEntry->list.next = &pCursor->list;
            pEntry->list.prev = pCursor->list.prev;
            pCursor->list.prev = &pEntry->list;

            return RT_ERR_OK;
        }
        // return fail if the entry exists
        else if (pCursor->idx == idx)
        {
            kfree(pEntry);

            return RT_ERR_ENTRY_EXIST;
        }
    }

    list_add_tail(&pEntry->list, &g_tagOperationHead);

    return RT_ERR_OK;
}

int ds_pkt_bcaster_ds_tag_operation_del(uint32 idx)
{
    tagOperation_entry_t    *pCursor;
    tagOperation_entry_t    *pNext;

    list_for_each_entry_safe(pCursor, pNext, &g_tagOperationHead, list)
    {
        // return fail if the entry not exists
        if (pCursor->idx > idx)
        {
            return RT_ERR_ENTRY_NOTFOUND;
        }
        else if (pCursor->idx == idx)
        {
            list_del(&pCursor->list);

            kfree(pCursor);

            return RT_ERR_OK;
        }
    }

    return RT_ERR_ENTRY_NOTFOUND;
}

int ds_pkt_bcaster_ds_tag_operation_flush(void)
{
    tagOperation_entry_t    *pCursor;
    tagOperation_entry_t    *pNext;

    list_for_each_entry_safe(pCursor, pNext, &g_tagOperationHead, list)
    {
        list_del(&pCursor->list);

        kfree(pCursor);
    }

    return RT_ERR_OK;
}

int ds_pkt_bcaster_ds_tag_operation_dump(void)
{
    tagOperation_entry_t    *pCursor;
    tagOperation_t          *pData;

    list_for_each_entry(pCursor, &g_tagOperationHead, list)
    {
        pData = &pCursor->data;

        printk("----------------------------------------");
        printk("----------------------------------------\n");
        printk(" %8s | %8s | %6s | %4s | %8s | %8s |",
            "Idx", "Hit", "S-Mode", "Mask", " VID/PRI", "TPID/DEI");
        printk(" %6s | %4s | %8s | %8s | %4s\n",
            "C-Mode", "Mask", " VID/PRI", "TPID/DEI", "PORT");

        printk(" %8x | %8x | %6u | %4x | %4u   %1u | %04x   %1u |",
            pCursor->idx,
            pCursor->hit,
            pData->tagFilter.sTagMode,
            pData->tagFilter.sTagMask,
            pData->tagFilter.sTagField.tagVid,
            pData->tagFilter.sTagField.tagPri,
            pData->tagFilter.sTagField.tagTpid,
            pData->tagFilter.sTagField.tagDei);
        printk(" %6u | %4x | %4u   %1u | %04x   %1u | %4x\n",
            pData->tagFilter.cTagMode,
            pData->tagFilter.cTagMask,
            pData->tagFilter.cTagField.tagVid,
            pData->tagFilter.cTagField.tagPri,
            pData->tagFilter.cTagField.tagTpid,
            pData->tagFilter.cTagField.tagDei,
            pData->dstPortMask);

        printk("  Treatment %6s | %10s | %10s | %8s |",
            "S-Mode", "VID Action", "PRI Action", " VID/PRI");
        printk(" %6s | %10s | %10s | %8s\n",
            "C-Mode", "VID Action", "PRI Action", " VID/PRI");

        printk("            %6u | %10u | %10u | %4u   %1u |",
            pData->tagTreatment.sTagMode,
            pData->tagTreatment.sTagAction.vidAction,
            pData->tagTreatment.sTagAction.priAction,
            pData->tagTreatment.sTagField.tagVid,
            pData->tagTreatment.sTagField.tagPri);
        printk(" %6u | %10u | %10u | %4u   %1u\n",
            pData->tagTreatment.cTagMode,
            pData->tagTreatment.cTagAction.vidAction,
            pData->tagTreatment.cTagAction.priAction,
            pData->tagTreatment.cTagField.tagVid,
            pData->tagTreatment.cTagField.tagPri);
    }

    return RT_ERR_OK;
}

static int gem_read_proc(struct seq_file *seq, void *v)
{
    int len;

    len = seq_printf(seq, "DS BC GEM Flow ID = %u\n", g_dsBcGemFlowId);

    return len;
}

static int gem_write_proc(
        struct file *file, const char *buffer, unsigned long count, void *data)
{
    uint64  tmpFlowId;
    uint8   tmpBuf[16];
    int     len;

    len = (count > 15) ? 15 : count;

    if (buffer && !copy_from_user(tmpBuf, buffer, len))
    {
        tmpFlowId = simple_strtoull(tmpBuf, NULL, 0);

        g_dsBcGemFlowId = (tmpFlowId > UINT_MAX) ? UINT_MAX : tmpFlowId;

        printk("DS BC GEM Flow ID = %u\n", g_dsBcGemFlowId);

        return count;
    }

    return -EFAULT;
}

static int gem_open_proc(struct inode *inode, struct file *file)
{
    return single_open(file, gem_read_proc, NULL);
}


static int rule_read_proc(struct seq_file *seq, void *v)
{
    int len;

    len = ds_pkt_bcaster_ds_tag_operation_dump();

    return len;
}

static int rule_open_proc(struct inode *inode, struct file *file)
{
    return single_open(file, rule_read_proc, NULL);
}


static void ds_pkt_bcaster_proc_init(void)
{
    g_procDir = proc_mkdir("ds_pkt_bcaster", NULL);
    if (!g_procDir)
    {
        printk(KERN_WARNING "create proc dir failed\n");
        return;
    }
	memset(&g_procEntryGem_fop, 0x0, sizeof(g_procEntryGem_fop));
	memset(&g_procEntryRule_fop, 0x0, sizeof(g_procEntryRule_fop));
	
	g_procEntryGem_fop.open = gem_open_proc;
	g_procEntryGem_fop.write = gem_write_proc;
	g_procEntryGem_fop.read = seq_read;
	g_procEntryGem_fop.llseek = seq_lseek;
	g_procEntryGem_fop.release = single_release;
	g_procEntryGem_fop.owner = THIS_MODULE;

	g_procEntryGem = proc_create("gem", 0, g_procDir, &g_procEntryGem_fop);
	if (!g_procEntryGem)
        printk(KERN_WARNING "create proc entry failed\n");

	g_procEntryRule_fop.open = rule_open_proc;
	g_procEntryRule_fop.read = seq_read;
	g_procEntryRule_fop.llseek = seq_lseek;
	g_procEntryRule_fop.release = single_release;
	g_procEntryRule_fop.owner = THIS_MODULE;

	g_procEntryRule = proc_create("rule", 0, g_procDir, &g_procEntryRule_fop);
    if (!g_procEntryRule)
        printk(KERN_WARNING "create proc entry failed\n");

}

static void ds_pkt_bcaster_proc_exit(void)
{
    if (g_procEntryGem)
    {
        remove_proc_entry("gem", g_procDir);
        g_procEntryGem = NULL;
    }

    if (g_procEntryRule)
    {
        remove_proc_entry("rule", g_procDir);
        g_procEntryRule = NULL;
    }

    if (g_procDir)
    {
        remove_proc_entry("ds_pkt_bcaster", NULL);
        g_procDir = NULL;
    }
}

int __init ds_pkt_bcaster_init(void)
{
    rtk_switch_devInfo_t    devInfo;
    int32                   ret;

    ret = rtk_switch_deviceInfo_get(&devInfo);
    if (RT_ERR_OK != ret)
    {
        printk(KERN_ERR "get device info fail!\n");

        return RT_ERR_FAILED;
    }

    g_chipId = devInfo.chipId;

    g_ponPort = 1 << HAL_GET_PON_PORT();

    g_numOfStagTpid = HAL_MAX_NUM_OF_SVLAN_TPID();

    g_dsBcGemFlowId = UINT_MAX;

    INIT_LIST_HEAD(&g_tagOperationHead);

    ret = drv_nic_register_rxhook(g_ponPort,
            RE8686_RXPRI_MUTICAST, ds_pkt_bcaster_ds_bc_gem_pkt_rx);
    if (RT_ERR_OK != ret)
    {
        printk(KERN_ERR "register packet rx callback fail!\n");

        return RT_ERR_FAILED;
    }

    // init proc
    ds_pkt_bcaster_proc_init();

    printk(KERN_NOTICE "ds packet broadcaster init ok!\n");

    return RT_ERR_OK;
}

void __exit ds_pkt_bcaster_exit(void)
{
    int32   ret;

    // exit proc
    ds_pkt_bcaster_proc_exit();

    g_dsBcGemFlowId = UINT_MAX;

    ds_pkt_bcaster_ds_tag_operation_flush();

    ret = drv_nic_unregister_rxhook(g_ponPort,
            RE8686_RXPRI_MUTICAST, ds_pkt_bcaster_ds_bc_gem_pkt_rx);
    if (RT_ERR_OK != ret)
    {
        printk(KERN_ERR "unregister packet rx callback fail!\n");

        return;
    }

    printk(KERN_NOTICE "ds packet broadcaster exit ok!\n");
}

/*
 * Symbol Definition
 */
EXPORT_SYMBOL(ds_pkt_bcaster_ds_bc_gem_flow_id_set);
EXPORT_SYMBOL(ds_pkt_bcaster_ds_tag_operation_add);
EXPORT_SYMBOL(ds_pkt_bcaster_ds_tag_operation_del);
EXPORT_SYMBOL(ds_pkt_bcaster_ds_tag_operation_flush);
EXPORT_SYMBOL(ds_pkt_bcaster_ds_tag_operation_dump);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek DS Packet Broadcasting Module");
MODULE_AUTHOR("Alvin Wu <alvinwu@realtek.com>");
module_init(ds_pkt_bcaster_init);
module_exit(ds_pkt_bcaster_exit);
