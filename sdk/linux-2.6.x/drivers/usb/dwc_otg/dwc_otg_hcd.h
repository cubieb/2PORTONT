/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_hcd.h $
 * $Revision: 1.2 $
 * $Date: 2010/12/16 06:25:36 $
 * $Change: 762293 $
 *
 * Synopsys HS OTG Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 * 
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 * 
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================== */
#ifndef DWC_DEVICE_ONLY
#if !defined(__DWC_HCD_H__)
#define __DWC_HCD_H__

#include <linux/list.h>
#include <linux/usb.h>
#include <../drivers/usb/core/hcd.h>

struct lm_device;
struct dwc_otg_device;

#include "dwc_otg_cil.h"

/**
 * @file
 *
 * This file contains the structures, constants, and interfaces for
 * the Host Contoller Driver (HCD).
 *
 * The Host Controller Driver (HCD) is responsible for translating requests
 * from the USB Driver into the appropriate actions on the DWC_otg controller.
 * It isolates the USBD from the specifics of the controller by providing an
 * API to the USBD.
 */

//cathy
#include "dwc_otg_hcd_def.h"

/** Gets the dwc_otg_hcd from a struct usb_hcd */
static inline dwc_otg_hcd_t *hcd_to_dwc_otg_hcd(struct usb_hcd *hcd)
{
	return (dwc_otg_hcd_t *)(hcd->hcd_priv);
}

/** Gets the struct usb_hcd that contains a dwc_otg_hcd_t. */
static inline struct usb_hcd *dwc_otg_hcd_to_hcd(dwc_otg_hcd_t *dwc_otg_hcd)
{
	return container_of((void *)dwc_otg_hcd, struct usb_hcd, hcd_priv);
}

/** @name HCD Create/Destroy Functions */
/** @{ */
extern int __init dwc_otg_hcd_init(struct lm_device *_lmdev);
extern void dwc_otg_hcd_remove(struct lm_device *_lmdev);
/** @} */

/** @name Linux HC Driver API Functions */
/** @{ */

extern int dwc_otg_hcd_start(struct usb_hcd *hcd);
extern void dwc_otg_hcd_stop(struct usb_hcd *hcd);
extern int dwc_otg_hcd_get_frame_number(struct usb_hcd *hcd);
extern void dwc_otg_hcd_free(struct usb_hcd *hcd);
extern int dwc_otg_hcd_urb_enqueue(struct usb_hcd *hcd, 
				   /* struct usb_host_endpoint *ep, */
				   struct urb *urb, 
				   gfp_t mem_flags);
extern int dwc_otg_hcd_urb_dequeue(struct usb_hcd *hcd, 
/*				   struct usb_host_endpoint *ep,*/
				   struct urb *urb, int status);
extern void dwc_otg_hcd_endpoint_disable(struct usb_hcd *hcd,
					 struct usb_host_endpoint *ep);
extern irqreturn_t dwc_otg_hcd_irq(struct usb_hcd *hcd);
extern int dwc_otg_hcd_hub_status_data(struct usb_hcd *hcd, 
				       char *buf);
extern int dwc_otg_hcd_hub_control(struct usb_hcd *hcd, 
				   u16 typeReq, 
				   u16 wValue, 
				   u16 wIndex, 
				   char *buf, 
				   u16 wLength);

/** @} */

/** @name Transaction Execution Functions */
/** @{ */
extern dwc_otg_transaction_type_e dwc_otg_hcd_select_transactions(dwc_otg_hcd_t *_hcd);
extern void dwc_otg_hcd_queue_transactions(dwc_otg_hcd_t *_hcd,
					   dwc_otg_transaction_type_e _tr_type);
extern void dwc_otg_hcd_complete_urb(dwc_otg_hcd_t *_hcd, struct urb *_urb,
				     int _status);
/** @} */

/** @name Interrupt Handler Functions */
/** @{ */
extern int32_t dwc_otg_hcd_handle_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_sof_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_rx_status_q_level_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_np_tx_fifo_empty_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_perio_tx_fifo_empty_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_incomplete_periodic_intr(dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_port_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_conn_id_status_change_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_disconnect_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_hc_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_hc_n_intr (dwc_otg_hcd_t *_dwc_otg_hcd, uint32_t _num);
extern int32_t dwc_otg_hcd_handle_session_req_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
extern int32_t dwc_otg_hcd_handle_wakeup_detected_intr (dwc_otg_hcd_t *_dwc_otg_hcd);
//eason
extern void dwc_otg_async (dwc_otg_hcd_t *_dwc_otg_hcd, unsigned char wlan_close);

/** @} */


/** @name Schedule Queue Functions */
/** @{ */

/* Implemented in dwc_otg_hcd_queue.c */
extern dwc_otg_qh_t *dwc_otg_hcd_qh_create (dwc_otg_hcd_t *_hcd, struct urb *_urb);
extern void dwc_otg_hcd_qh_init (dwc_otg_hcd_t *_hcd, dwc_otg_qh_t *_qh, struct urb *_urb);
extern void dwc_otg_hcd_qh_free (dwc_otg_qh_t *_qh);
extern int dwc_otg_hcd_qh_add (dwc_otg_hcd_t *_hcd, dwc_otg_qh_t *_qh);
extern void dwc_otg_hcd_qh_remove (dwc_otg_hcd_t *_hcd, dwc_otg_qh_t *_qh);
extern void dwc_otg_hcd_qh_deactivate (dwc_otg_hcd_t *_hcd, dwc_otg_qh_t *_qh, int sched_csplit);

/** Remove and free a QH */
static inline void dwc_otg_hcd_qh_remove_and_free (dwc_otg_hcd_t *_hcd,
						   dwc_otg_qh_t *_qh)
{
	dwc_otg_hcd_qh_remove (_hcd, _qh);
	dwc_otg_hcd_qh_free (_qh);
}

/** Allocates memory for a QH structure.
 * @return Returns the memory allocate or NULL on error. */
static inline dwc_otg_qh_t *dwc_otg_hcd_qh_alloc (void)
{
	return (dwc_otg_qh_t *) kmalloc (sizeof(dwc_otg_qh_t), GFP_ATOMIC);
}

extern dwc_otg_qtd_t *dwc_otg_hcd_qtd_create (struct urb *urb);
extern void dwc_otg_hcd_qtd_init (dwc_otg_qtd_t *qtd, struct urb *urb);
extern int dwc_otg_hcd_qtd_add (dwc_otg_qtd_t *qtd, dwc_otg_hcd_t *dwc_otg_hcd);

/** Allocates memory for a QTD structure.
 * @return Returns the memory allocate or NULL on error. */
static inline dwc_otg_qtd_t *dwc_otg_hcd_qtd_alloc (void)
{
	return (dwc_otg_qtd_t *) kmalloc (sizeof(dwc_otg_qtd_t), GFP_ATOMIC);
}

/** Frees the memory for a QTD structure.  QTD should already be removed from
 * list.
 * @param[in] _qtd QTD to free.*/
static inline void dwc_otg_hcd_qtd_free (dwc_otg_qtd_t *_qtd)
{
	kfree (_qtd);
}

/** Removes a QTD from list.
 * @param[in] _qtd QTD to remove from list. */
static inline void dwc_otg_hcd_qtd_remove (dwc_otg_qtd_t *_qtd)
{
	unsigned long flags;
	local_irq_save (flags);
	list_del (&_qtd->qtd_list_entry);
	local_irq_restore (flags);
}

/** Remove and free a QTD */
static inline void dwc_otg_hcd_qtd_remove_and_free (dwc_otg_qtd_t *_qtd)
{
	dwc_otg_hcd_qtd_remove (_qtd);
	dwc_otg_hcd_qtd_free (_qtd);
}

/** @} */


/** @name Internal Functions */
/** @{ */
dwc_otg_qh_t *dwc_urb_to_qh(struct urb *_urb);
void dwc_otg_hcd_dump_frrem(dwc_otg_hcd_t *_hcd);
void dwc_otg_hcd_dump_state(dwc_otg_hcd_t *_hcd);
/** @} */

/** Gets the usb_host_endpoint associated with an URB. */
static inline struct usb_host_endpoint *dwc_urb_to_endpoint(struct urb *_urb)
{
	struct usb_device *dev = _urb->dev;
	int ep_num = usb_pipeendpoint(_urb->pipe);

	if (usb_pipein(_urb->pipe) || (0 == ep_num))
		return dev->ep_in[ep_num];
	else
		return dev->ep_out[ep_num];
}

/**
 * Gets the endpoint number from a _bEndpointAddress argument. The endpoint is
 * qualified with its direction (possible 32 endpoints per device).
 */
#define dwc_ep_addr_to_endpoint(_bEndpointAddress_) ((_bEndpointAddress_ & USB_ENDPOINT_NUMBER_MASK) | \
                                                     ((_bEndpointAddress_ & USB_DIR_IN) != 0) << 4)

/** Gets the QH that contains the list_head */
#define dwc_list_to_qh(_list_head_ptr_) (container_of(_list_head_ptr_,dwc_otg_qh_t,qh_list_entry))

/** Gets the QTD that contains the list_head */
#define dwc_list_to_qtd(_list_head_ptr_) (container_of(_list_head_ptr_,dwc_otg_qtd_t,qtd_list_entry))

/** Check if QH is non-periodic  */
#define dwc_qh_is_non_per(_qh_ptr_) ((_qh_ptr_->ep_type == USB_ENDPOINT_XFER_BULK) || \
                                     (_qh_ptr_->ep_type == USB_ENDPOINT_XFER_CONTROL))

/** High bandwidth multiplier as encoded in highspeed endpoint descriptors */
#define dwc_hb_mult(wMaxPacketSize) (1 + (((wMaxPacketSize) >> 11) & 0x03))

/** Packet size for any kind of endpoint descriptor */
#define dwc_max_packet(wMaxPacketSize) ((wMaxPacketSize) & 0x07ff)

/**
 * Returns true if _frame1 is less than or equal to _frame2. The comparison is
 * done modulo DWC_HFNUM_MAX_FRNUM. This accounts for the rollover of the
 * frame number when the max frame number is reached.
 */
static inline int dwc_frame_num_le(uint16_t _frame1, uint16_t _frame2)
{
	return ((_frame2 - _frame1) & DWC_HFNUM_MAX_FRNUM) <=
		(DWC_HFNUM_MAX_FRNUM >> 1);
}

/**
 * Returns true if _frame1 is greater than _frame2. The comparison is done
 * modulo DWC_HFNUM_MAX_FRNUM. This accounts for the rollover of the frame
 * number when the max frame number is reached.
 */
static inline int dwc_frame_num_gt(uint16_t _frame1, uint16_t _frame2)
{
	return (_frame1 != _frame2) &&
		(((_frame1 - _frame2) & DWC_HFNUM_MAX_FRNUM) <
		 (DWC_HFNUM_MAX_FRNUM >> 1));
}

/**
 * Increments _frame by the amount specified by _inc. The addition is done
 * modulo DWC_HFNUM_MAX_FRNUM. Returns the incremented value.
 */
static inline uint16_t dwc_frame_num_inc(uint16_t _frame, uint16_t _inc)
{
	return (_frame + _inc) & DWC_HFNUM_MAX_FRNUM;
}

static inline uint16_t dwc_full_frame_num (uint16_t _frame)
{
	return ((_frame) & DWC_HFNUM_MAX_FRNUM) >> 3;
}

static inline uint16_t dwc_micro_frame_num (uint16_t _frame)
{
	return (_frame) & 0x7;
}

#ifdef DEBUG
/**
 * Macro to sample the remaining PHY clocks left in the current frame. This
 * may be used during debugging to determine the average time it takes to
 * execute sections of code. There are two possible sample points, "a" and
 * "b", so the _letter argument must be one of these values.
 *
 * To dump the average sample times, read the "hcd_frrem" sysfs attribute. For
 * example, "cat /sys/devices/lm0/hcd_frrem".
 */
#define dwc_sample_frrem(_hcd, _qh, _letter) \
{ \
	hfnum_data_t hfnum; \
	dwc_otg_qtd_t *qtd; \
	qtd = list_entry(_qh->qtd_list.next, dwc_otg_qtd_t, qtd_list_entry); \
	if (usb_pipeint(qtd->urb->pipe) && _qh->start_split_frame != 0 && !qtd->complete_split) { \
		hfnum.d32 = dwc_read_reg32(&_hcd->core_if->host_if->host_global_regs->hfnum); \
		switch (hfnum.b.frnum & 0x7) { \
		case 7: \
			_hcd->hfnum_7_samples_##_letter++; \
			_hcd->hfnum_7_frrem_accum_##_letter += hfnum.b.frrem; \
			break; \
		case 0: \
			_hcd->hfnum_0_samples_##_letter++; \
			_hcd->hfnum_0_frrem_accum_##_letter += hfnum.b.frrem; \
			break; \
		default: \
			_hcd->hfnum_other_samples_##_letter++; \
			_hcd->hfnum_other_frrem_accum_##_letter += hfnum.b.frrem; \
			break; \
		} \
	} \
}
#else
#define dwc_sample_frrem(_hcd, _qh, _letter) 
#endif		
#endif
#endif /* DWC_DEVICE_ONLY */
