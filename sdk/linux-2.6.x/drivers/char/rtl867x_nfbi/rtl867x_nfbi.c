/*
 *  RTL867X NFBI char driver
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *  
 *  History
 *  2011.12.22:	This version is initial version. For RLE0437 test chip. Merge from AP
 */
/*================================================================*/
/* Include Files */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include "rtl867x_nfbi.h"
#include "bspchip.h"
#include "nfbi_gpiosmi.h"



/*================================================================*/
#define NFBI_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef NFBI_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
//#define PDEBUG(fmt, args...) printk( KERN_DEBUG DRIVER_NAME": " fmt, ## args)
#define PDEBUG(fmt, args...) printk( KERN_ERR DRIVER_NAME": " fmt, ## args)
#else
/* This one for user space */
//#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#define PDEBUG(fmt, args...) printf(fmt, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }


/*================================================================
 * 0437, 0513A: max 75Mhz CLK, undef USE_RISING_EDGE, define _MDIO_PATCH_
 * 0513B: 200Mhz CLK, define USE_RISING_EDGE, undef _MDIO_PATCH_
 *================================================================*/
#define USE_RISING_EDGE
#define SET_INT_LEVEL
#ifdef SET_INT_LEVEL
//CMD bit[2]: 1=high, 0=low when int occurs
#define INT_LEVEL		BIT(2)
#define INT_LEVEL_DEF	BIT(2)
#endif /*SET_INT_LEVEL*/

//#define DEBUG_MDIO_RW
#ifdef DEBUG_MDIO_RW
#define dgb_mdio_printk	printk
#else
#define dgb_mdio_printk(...) while(0){}
#endif //DEBUG_MDIO_RW

//#define _MDIO_PATCH_
#if !defined(_MDIO_PATCH_) && !defined(NFBI_GPIO_SIMULATE)
#define NFBI_USE_MEM_DWORD
#endif /*NFBI_GPIO_SIMULATE*/

//use delay instead of checking CMD:busy bit
//for test access to slave's memory when nfbi read() doesn't work
//#define _NFBI_MEMIO_USE_DELAY_

/*================================================================*/
#define RESET_RX_STATE { \
		priv->state = STATE_RX_INIT; \
		priv->rx_flag = 0; \
		priv->data_in.len = 0; \
}


/*================================================================*/
/* Local Variables */
#ifndef NFBI_GPIO_SIMULATE
static int nfbi_io=NFBI_BASE; //base address of RLE0437 CPU
static int nfbi_irq=BSP_MDIO_IRQ;
#endif /*NFBI_GPIO_SIMULATE*/
static unsigned char nfbi_phyaddr; 
static spinlock_t nfbi_lock = SPIN_LOCK_UNLOCKED;
static struct nfbi_dev_priv *ndev_priv=NULL;


/*================================================================*/
/*debug msg channel*/
static int nrx_msg_dbg=0;
static int ntx_msg_dbg=0;
#define NRX_MSG_PRT(format, args...)	if(nrx_msg_dbg) printk(format , ## args)
#define NTX_MSG_PRT(format, args...)	if(ntx_msg_dbg) printk(format , ## args)
static struct proc_dir_entry *nfbi_proc_root = NULL ;
#define NPROC_NAME_DIR	"nfbi"
#define NPROC_NAME_RX	"nrx_msg_dbg"
#define NPROC_NAME_TX	"ntx_msg_dbg"


/*================================================================*/
/* Functions */
static int is_checksum_ok(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}

static unsigned char append_checksum(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}


static void nfbi_dump_data( unsigned char *msg, unsigned char *p, int len )
{
	int i;

	printk( "\n\n(%s) nfbi_dump_data start, len=%d\n", msg?msg:(unsigned char *)"", len );
	for(i=0; i<len; i++)
	{
		printk( "%02x ", p[i] );
		if(i && (i&0x7)==0x7) printk( "  " );
		if(i && (i&0xf)==0xf) printk( "\n" );
	}
	printk( "\n(%s) nfbi_dump_data done\n\n", msg?msg:(unsigned char *)"" );
	

	return;
	
}

static int kill_proc(pid_t pid, int sig, int priv)
{
        return kill_proc_info(sig, (void *)(long)(priv != 0), pid);
}


/////////////////////////////////////////////////////////////////////////////////////
static int nfbi_proc_read_tx(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size=0;
	size=sprintf(buf, "%d", ntx_msg_dbg);
	*eof=1;
    return size;
}

static int nfbi_proc_write_tx(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[count];

	memset( tmp, 0, sizeof(tmp) );
	if( copy_from_user(tmp, buffer, count) ) 
		return -EFAULT;
	if(tmp[0]=='0') 
		ntx_msg_dbg=0;
	else
		ntx_msg_dbg=1;
    return count;
}

static int nfbi_proc_read_rx(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size=0;
	size=sprintf(buf, "%d", nrx_msg_dbg);
	*eof=1;
    return size;
}

static int nfbi_proc_write_rx(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[count];

	memset( tmp, 0, sizeof(tmp) );
	if( copy_from_user(tmp, buffer, count) ) 
		return -EFAULT;
	if(tmp[0]=='0') 
		nrx_msg_dbg=0;
	else
		nrx_msg_dbg=1;
    return count;
}

static void __exit nfbi_remove_proc(void)
{
	remove_proc_entry( NPROC_NAME_RX, nfbi_proc_root );
	remove_proc_entry( NPROC_NAME_TX, nfbi_proc_root );
	remove_proc_entry( NPROC_NAME_DIR, NULL );
	nfbi_proc_root=NULL;
}

static int __init nfbi_create_proc(void)
{
	struct proc_dir_entry *new_entry;

	nfbi_proc_root=proc_mkdir(NPROC_NAME_DIR, NULL);
	if(nfbi_proc_root==NULL)
	{
		printk( "create mdio proc root failed!\n" );
		return -1;
	}

	new_entry=create_proc_entry(NPROC_NAME_TX, 0644, nfbi_proc_root);
	if(new_entry==NULL)
	{
		printk( "create proc tx_msg_dbg failed!\n" );
		nfbi_remove_proc();
		return -1;
	}
	new_entry->write_proc=nfbi_proc_write_tx;
	new_entry->read_proc=nfbi_proc_read_tx;

	new_entry=create_proc_entry(NPROC_NAME_RX, 0644, nfbi_proc_root);
	if(new_entry==NULL)
	{
		printk( "create proc rx_msg_dbg failed!\n" );
		nfbi_remove_proc();
		return -1;
	}
	new_entry->write_proc=nfbi_proc_write_rx;
	new_entry->read_proc=nfbi_proc_read_rx;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////


/*================================================================*/
/*Slave xDSL status*/
static unsigned char slv_adslup=0;
unsigned char getSlvADSLLinkStatus(void)
{
	return slv_adslup;
}

static void SlvAdslLinkUp(void)
{
	slv_adslup=1;
	printk("!!!!!!!!!! Slave ADSL link up !!!!!!!!!!\n");
}

static void SlvAdslLinkDown(void)
{
	slv_adslup=0;
	printk("!!!!!!!!!! Slave ADSL link down !!!!!!!!!!\n");
}

/*================================================================*/



/*================================================================*/
#ifdef NFBI_GPIO_SIMULATE
static int nfbi867x_mdio_read(unsigned char phyid,unsigned char regaddr, unsigned int *data, int isdword)
{
	unsigned short p;
	dgb_mdio_printk( "\n%s\n", __FUNCTION__ );
	NFBI_smiRead(phyid,regaddr,&p);
	*data=(unsigned int)p;
	dgb_mdio_printk( "read phy=0x%02x, reg=0x%02x, data=0x%08x\n",
			phyid, regaddr,*data);
	return 0;
}

static int nfbi867x_mdio_write(unsigned char phyid,unsigned char regaddr, unsigned int data, int isdword)
{
	dgb_mdio_printk( "\n%s\n", __FUNCTION__ );
#ifdef SET_INT_LEVEL
	if(regaddr==NFBI_REG_CMD) 
		data=(data&~INT_LEVEL)|INT_LEVEL_DEF;
#endif /*SET_INT_LEVEL*/
	NFBI_smiWrite(phyid,regaddr,(unsigned short)data);
	dgb_mdio_printk( "write phy=0x%02x, reg=0x%02x, data=0x%08x\n",
			phyid, regaddr, data);
	return 0;
}

#else /*NFBI_GPIO_SIMULATE*/

//return 0: ok,  -1: failed
static int nfbi867x_mdio_read(unsigned char phyid,unsigned char regaddr, unsigned int *data, int isdword)
{
	volatile unsigned int vmar, vmcr, vmdr;
	int timeout = MDIO_TIMEOUT;
	int first_time;

	dgb_mdio_printk( "\n%s\n", __FUNCTION__ );
	if(data) *data=0;

	vmar = ((phyid&0x1f)<<11)|((regaddr&0x1f)<<6);
	vmcr = BIT(15);//read, single word
	if(isdword) vmcr |= BIT(13); //double word
#ifdef USE_RISING_EDGE
	vmcr |= BIT(12);
#endif /*USE_RISING_EDGE*/
	dgb_mdio_printk( "set NFBI_MAR(%08x)=%08x\n", NFBI_MAR, vmar );
	dgb_mdio_printk( "set NFBI_MCR(%08x)=%08x\n", NFBI_MCR, vmcr );
	REG32(NFBI_MAR) = vmar;
	REG32(NFBI_MCR) = vmcr;

	first_time=0;
	while (--timeout) 
	{
		vmcr = REG32(NFBI_MCR);
		if( (vmcr&BIT(14))==0 ) 
		{
			dgb_mdio_printk( "get NFBI_MCR=%08x, busy=%08x\n", vmcr, vmcr&BIT(14) );
			break;
		}else{
			if(first_time==0) 
			{
				dgb_mdio_printk( "get NFBI_MCR=%08x, busy=%08x\n", vmcr, vmcr&BIT(14) );
				first_time++;
			}
		}
	}
	if (timeout==0)
	{
		printk("Timeout! read phy=%x reg=%x fail!\n",phyid, regaddr );
		return -1;
	}

	vmdr=REG32(NFBI_MDR);
	dgb_mdio_printk( "get NFBI_MDR(%08x)=%08x\n", NFBI_MDR, vmdr );
	if(data) *data=vmdr;

	return 0;
}

//return 0: ok,  -1: failed
static int nfbi867x_mdio_write(unsigned char phyid,unsigned char regaddr, unsigned int data, int isdword)
{
	volatile unsigned int vmar, vmcr, vmdr;
	int timeout = MDIO_TIMEOUT;
	int first_time;

	dgb_mdio_printk( "\n%s\n", __FUNCTION__ );
#ifdef SET_INT_LEVEL
	if(regaddr==NFBI_REG_CMD) 
		data=(data&~INT_LEVEL)|INT_LEVEL_DEF;
#endif /*SET_INT_LEVEL*/
	vmar = ((phyid&0x1f)<<11)|((regaddr&0x1f)<<6);
	vmdr = data;
	vmcr = 0;//write, single word
	if(isdword) vmcr |= BIT(13); //double word
#ifdef USE_RISING_EDGE
	vmcr |= BIT(12);
#endif /*USE_RISING_EDGE*/
	dgb_mdio_printk( "set NFBI_MAR(%08x)=%08x\n", NFBI_MAR, vmar );
	dgb_mdio_printk( "set NFBI_MDR(%08x)=%08x\n", NFBI_MDR, vmdr );
	dgb_mdio_printk( "set NFBI_MCR(%08x)=%08x\n", NFBI_MCR, vmcr );
	REG32(NFBI_MAR) = vmar;
	REG32(NFBI_MDR) = vmdr;
	REG32(NFBI_MCR) = vmcr;

	first_time=0;
	while (--timeout) 
	{
		vmcr = REG32(NFBI_MCR);
		if( (vmcr&BIT(14))==0 ) 
		{
			dgb_mdio_printk( "get NFBI_MCR=%08x, busy=%08x\n", vmcr, vmcr&BIT(14) );
			break;
		}else{
			if(first_time==0) 
			{
				dgb_mdio_printk( "get NFBI_MCR=%08x, busy=%08x\n", vmcr, vmcr&BIT(14) );
				first_time++;
			}
		}
	}
	if (timeout==0)
	{
		printk("Timeout! write phy=%x reg=%x fail!\n",phyid, regaddr );
		return -1;
	}

	return 0;
}
#endif /*NFBI_GPIO_SIMULATE*/


/*================================================================*/
#ifdef _MDIO_PATCH_
static int nfbi867x_mdio_patch(unsigned char phyid, unsigned char reg)
{
	int ret;
	//if( (reg==NFBI_REG_DH)||(reg==NFBI_REG_DL) )
	if(reg!=NFBI_REG_PHYID1)
	{
		unsigned int phyval;
		dgb_mdio_printk( "\nnfbi867x_mdio_patch: read PHYID1\n" );
		ret = nfbi867x_mdio_read( phyid, NFBI_REG_PHYID1, &phyval, 0 );
	}else
		ret = 0;

	return ret;
}
#else /*_MDIO_PATCH_*/
#define nfbi867x_mdio_patch(...) while(0){}
#endif /*_MDIO_PATCH_*/

static int nfbi867x_mdio_read_one_word(unsigned char phyid,unsigned char regaddr, unsigned short *data)
{
	int ret;
	unsigned int b;
	ret = nfbi867x_mdio_read(phyid, regaddr, &b, 0 );
	if( (ret==0)&&data ) *data= (unsigned short)b;
	nfbi867x_mdio_patch(phyid, regaddr);
	return ret;
}

static int nfbi867x_mdio_write_one_word(unsigned char phyid,unsigned char regaddr, unsigned short data)
{
	int ret;
	ret = nfbi867x_mdio_write( phyid, regaddr, (unsigned int)data, 0 );
	nfbi867x_mdio_patch(phyid, regaddr);
	return ret;
}

#ifdef NFBI_USE_MEM_DWORD
static int nfbi867x_mdio_read_dword(unsigned char phyid,unsigned char regaddr, unsigned int *data)
{
	int ret;
	ret = nfbi867x_mdio_read(phyid, regaddr, data, 1 );
	nfbi867x_mdio_patch(phyid, regaddr);
	return ret;
}

static int nfbi867x_mdio_write_dword(unsigned char phyid,unsigned char regaddr, unsigned int data)
{
	int ret;
	ret = nfbi867x_mdio_write( phyid, regaddr, data, 1 );
	nfbi867x_mdio_patch(phyid, regaddr);
	return ret;
}
#endif  /*NFBI_USE_MEM_DWORD*/


/*================================================================*/
static int rtl_mdio_write(unsigned short reg, unsigned short data)
{
    unsigned long flags;
	int ret;	

    spin_lock_irqsave(&nfbi_lock, flags);
	ret = nfbi867x_mdio_write_one_word( nfbi_phyaddr, (unsigned char)reg, data );
	spin_unlock_irqrestore(&nfbi_lock, flags);
	return ret;
}

#if 0
static int rtl_mdio_mask_write(unsigned short reg, unsigned short mask, unsigned short data)
{
    unsigned long flags;
	unsigned short regval;
	int ret;
	
    spin_lock_irqsave(&nfbi_lock, flags);
	ret = nfbi867x_mdio_read_one_word( nfbi_phyaddr, (unsigned char)reg, &regval );
	if(ret<0)
	{
		spin_unlock_irqrestore(&nfbi_lock, flags);
		return ret;
	}
	data = (regval&(~mask)) | (data&mask);
	ret = nfbi867x_mdio_write_one_word( nfbi_phyaddr, (unsigned char)reg, data );
	spin_unlock_irqrestore(&nfbi_lock, flags);
	return ret;
}
#endif

static int rtl_mdio_read(unsigned short reg, unsigned short *pdata)
{
    unsigned long flags;
	int ret;

    spin_lock_irqsave(&nfbi_lock, flags);
	ret = nfbi867x_mdio_read_one_word( nfbi_phyaddr,(unsigned char)reg, pdata );
    spin_unlock_irqrestore(&nfbi_lock, flags);    
	return ret;
}

#ifdef NFBI_USE_MEM_DWORD
static int rtl_mdio_write_dword(unsigned short reg, unsigned int data)
{
    unsigned long flags;
	int ret;	

    spin_lock_irqsave(&nfbi_lock, flags);
	ret = nfbi867x_mdio_write_dword( nfbi_phyaddr, (unsigned char)reg, data );
	spin_unlock_irqrestore(&nfbi_lock, flags);
	return ret;
}

static int rtl_mdio_read_dword(unsigned short reg, unsigned int *pdata)
{
    unsigned long flags;
	int ret;

    spin_lock_irqsave(&nfbi_lock, flags);
	ret = nfbi867x_mdio_read_dword( nfbi_phyaddr,(unsigned char)reg, pdata );
    spin_unlock_irqrestore(&nfbi_lock, flags);    
	return ret;
}
#endif /*NFBI_USE_MEM_DWORD*/

/*================================================================*/
//for bonding: set/get FDCR/SDCR
int nfbi_set_FDCR(unsigned short data)
{
	return rtl_mdio_write(NFBI_REG_FDCR, data);
}
int nfbi_get_FDCR(unsigned short *pdata)
{
	return rtl_mdio_read(NFBI_REG_FDCR, pdata);
}

int nfbi_set_SDCR(unsigned short data)
{
	return rtl_mdio_write(NFBI_REG_SDCR, data);
}
int nfbi_get_SDCR(unsigned short *pdata)
{
	return rtl_mdio_read(NFBI_REG_SDCR, pdata);
}



/*================================================================*/
static void nfbi_handle_put_rx_char(struct nfbi_priv *priv, unsigned char tmp)
{
	if( priv->data_in.len<NFBI_MAX_BUFSIZE )
	{
		priv->data_in.buf[priv->data_in.len]=tmp;
		priv->data_in.len++;
		NRX_MSG_PRT( "%02x ", tmp );
	}else{
		NRX_MSG_PRT( "%s(): buffer is full, skip %02x byte!\n", __FUNCTION__, tmp );
	}

	return;
}

static void nfbi_handle_next_rx_char(struct nfbi_priv *priv, unsigned char tmp)
{
	if(priv->rx_flag&NRX_ESC)
	{
		priv->rx_flag=priv->rx_flag&(~NRX_ESC);
		NRX_MSG_PRT( "(rx_U)" );
		tmp=tmp^NFBI_PPP_TRANS;
		nfbi_handle_put_rx_char(priv, tmp);
	}else{
		if(tmp==NFBI_PPP_ESCAPE)
		{
			priv->rx_flag=priv->rx_flag|NRX_ESC;
			NRX_MSG_PRT( "(rx_E)" );
		}else{
			nfbi_handle_put_rx_char(priv, tmp);
		}
	}

	return;
}

static void nfbi_process_rx_msg(struct nfbi_priv *priv, unsigned short data)
{
	unsigned char *p=(unsigned char *)&data;
	unsigned char tmp;
	int i;

	for(i=0; i<2; i++)
	{
		tmp=p[i];
		if(tmp==NFBI_PPP_FLAG)
		{
			if(priv->data_in.len>0)
			{
				priv->rx_flag=priv->rx_flag|NRX_END;
				NRX_MSG_PRT( "(rx_end)\n" );
				if(nrx_msg_dbg)
					nfbi_dump_data( "rxdone", priv->data_in.buf, priv->data_in.len );
				if (!is_checksum_ok(priv->data_in.buf, priv->data_in.len)) 
				{
					printk("rx cheksum error!\n");
					ndev_priv->rx_checksum_errors++;

					RESET_RX_STATE;
					priv->rx_flag=NRX_START;
					NRX_MSG_PRT( "\n(rx_start)" );
				}else{
					priv->data_in.len -= 1; // substract checksum length
					priv->state = STATE_RX_FINISHED;
					return; /*the 2nd byte may be skip handling*/
				}
			}else{
				RESET_RX_STATE;
				priv->rx_flag=NRX_START;
				NRX_MSG_PRT( "\n(rx_start)" );
			}
		}else{
			nfbi_handle_next_rx_char(priv, tmp);
			if(priv->state!=STATE_RX_WAIT_DATA)
				priv->state=STATE_RX_WAIT_DATA;
		}
	}

	return;
}

static unsigned char nfbi_get_next_tx_char(struct nfbi_priv *priv)
{
	unsigned char tmp;
	unsigned char ret;

	if( (priv->tx_cmd_transmitting_len==0)&&(priv->tx_flag==0) )
	{
		ret=NFBI_PPP_FLAG;
		priv->tx_flag = NTX_START;
		NTX_MSG_PRT( "\n(start)" );
	}else if(priv->tx_cmd_transmitting_len>=priv->data_out.len)
	{
		ret=NFBI_PPP_FLAG;
		priv->tx_flag = priv->tx_flag | NTX_END;
		NTX_MSG_PRT( "(end)\n" );
	}else{
		tmp=priv->data_out.buf[priv->tx_cmd_transmitting_len];
		if( (tmp==NFBI_PPP_FLAG) || (tmp==NFBI_PPP_ESCAPE) )
		{
			if(priv->tx_flag&NTX_ESC)
			{
				ret=tmp^NFBI_PPP_TRANS;
				priv->tx_flag = priv->tx_flag & (~NTX_ESC);
				priv->tx_cmd_transmitting_len++;
				NTX_MSG_PRT( "(U)" );
			}else{
				ret=NFBI_PPP_ESCAPE;
				priv->tx_flag = priv->tx_flag | NTX_ESC;
				NTX_MSG_PRT( "(E)" );
			}
		}else{
			ret=tmp;
			priv->tx_cmd_transmitting_len++;
		}
	}

	return ret;
}

static void nfbi_transmit_msg(struct nfbi_priv *priv)
{
	unsigned short data;
	unsigned char *p=(unsigned char *)&data;

	if(priv->data_out.len<=0)
	{
		NTX_MSG_PRT( "%s:%d> out.len<=0 return\n", __FUNCTION__, __LINE__ );
		return;
	}
	if(priv->tx_flag&NTX_END)
	{
		NTX_MSG_PRT( "%s:%d> tx done return\n", __FUNCTION__, __LINE__ );
		return;
	}

	p[0]=nfbi_get_next_tx_char(priv);
	NTX_MSG_PRT( "%02x ", p[0] );
	p[1]=nfbi_get_next_tx_char(priv);
	NTX_MSG_PRT( "%02x ", p[1] );

	ndev_priv->tx_msg_is_fetched = 0;
    rtl_mdio_write(NFBI_REG_SCR, data);
    ndev_priv->tx_words++;
}

static int nfbi_indicate_evt(struct evt_msg *evt)
{
	int size;

	//handle dsllink flag
	if( (evt->id==1) && (evt->status&IP_DSLLINK) )
	{
		if(evt->value&BM_DSLLINK)
			SlvAdslLinkUp();
		else
			SlvAdslLinkDown();
	}
	

	if (ndev_priv->hcd_pid == -1)
	    return 0;
	    
	size = CIRC_SPACE(ndev_priv->evt_que_head, ndev_priv->evt_que_tail, EV_QUE_MAX);
	if (size == 0) 
	{
		PDEBUG("Indication queue full, drop event!\n");
        //send SIGUSR1 signal to notify the host control deamon process
		if (ndev_priv->hcd_pid != -1)
	       kill_proc(ndev_priv->hcd_pid, SIGUSR1, 1);

		return -1;
	}
	
	ndev_priv->ind_evt_que[ndev_priv->evt_que_head].id = evt->id;
	ndev_priv->ind_evt_que[ndev_priv->evt_que_head].status = evt->status;
	ndev_priv->ind_evt_que[ndev_priv->evt_que_head].value = evt->value;
	ndev_priv->evt_que_head = (ndev_priv->evt_que_head + 1) & (EV_QUE_MAX - 1);

    //send SIGUSR1 signal to notify the host control deamon process
	if (ndev_priv->hcd_pid != -1)
		kill_proc(ndev_priv->hcd_pid, SIGUSR1, 1);
	
	return 0;
}

static int nfbi_retrieve_evt(struct evt_msg *evt)
{
	if (CIRC_CNT(ndev_priv->evt_que_head, ndev_priv->evt_que_tail, EV_QUE_MAX) > 0) 
	{ 
		// more than one evt pending
	    evt->id = ndev_priv->ind_evt_que[ndev_priv->evt_que_tail].id;
	    evt->status = ndev_priv->ind_evt_que[ndev_priv->evt_que_tail].status;
	    evt->value = ndev_priv->ind_evt_que[ndev_priv->evt_que_tail].value;
		ndev_priv->evt_que_tail = (ndev_priv->evt_que_tail + 1) & (EV_QUE_MAX - 1);
	}else{
        evt->id = 0;
	    evt->status = 0;
	    evt->value = 0;
	}
	return 0;
}



#if 0
static void nfbi_dump_interrupt(unsigned short status, unsigned short mask)
{
	if(status)
	{
		printk("%s: status=%x mask=%x\n", __FUNCTION__, status, mask);
		if(status&IP_CHECKSUM_DONE)
			printk( "IP_CHECKSUM_DONE\n" );
		if(status&IP_CHECKSUM_OK)
			printk( "IP_CHECKSUM_OK\n" );
		if(status&IP_DSLLINK)
			printk( "IP_DSLLINK\n" );
		if(status&IP_UASR_CHANGE)
			printk( "IP_UASR_CHANGE\n" );
		if(status&IP_ALLSOFTWARE_READY)
			printk( "IP_ALLSOFTWARE_READY\n" );
		if(status&IP_BOOTCODE_READY)
			printk( "IP_BOOTCODE_READY\n" );
		if(status&IP_PREVMSG_FETCH)
			printk( "IP_PREVMSG_FETCH\n" );
		if(status&IP_NEWMSG_COMING)
			printk( "IP_NEWMSG_COMING\n" );
		if(status&IP_NEED_BOOTCODE)
			printk( "IP_NEED_BOOTCODE\n" );
	}
}
#else
#define nfbi_dump_interrupt(...) while(0){}
#endif

static irqreturn_t nfbi_interrupt(int irq, void *dev_id)
{
	unsigned short status, data, mask;
	struct evt_msg evt;

	//printk("nfbi_interrupt: irq=%d\n", irq);
	if (!ndev_priv->ready) 
	{
	    return IRQ_HANDLED;
	}


#ifdef SET_INT_LEVEL
	{
		unsigned short data_cmd=0;
		rtl_mdio_read(NFBI_REG_CMD, &data_cmd);
		if( (data_cmd&INT_LEVEL)!=INT_LEVEL_DEF )
		{
			printk("nfbi_interrupt: set INT_LEVEL_DEF=0x%04x\n", (unsigned short)INT_LEVEL_DEF);
			//printk("nfbi_interrupt: old data_cmd=0x%04x\n", data_cmd);
			data_cmd=(data_cmd&~INT_LEVEL)|INT_LEVEL_DEF;
			//printk("nfbi_interrupt: new data_cmd=0x%04x\n", data_cmd);
			rtl_mdio_write(NFBI_REG_CMD, data_cmd);
		}
	}
#endif /*SET_INT_LEVEL*/

	    
	rtl_mdio_read(NFBI_REG_ISR, &status);
	rtl_mdio_read(NFBI_REG_IMR, &mask);
	nfbi_dump_interrupt(status,mask);
	status &= mask;
	while (status) 
	{
		//printk("status=%x mask=%x\n", status, mask);
		rtl_mdio_write(NFBI_REG_ISR, status);

		if (status & IP_PREVMSG_FETCH) 
		{
		    ndev_priv->tx_msg_is_fetched = 1;
			wake_up_interruptible(&ndev_priv->wq);
		}
		if (status & IP_NEWMSG_COMING) 
		{
		    ndev_priv->rx_msg_is_coming = 1;
			wake_up_interruptible(&ndev_priv->wq);
		}
		if( status & IP_STATUS_MASK_NO_MSG ) 
		{
		    evt.id = 1;
		    evt.status = status & IP_STATUS_MASK_NO_MSG;
		    rtl_mdio_read(NFBI_REG_SYSSR, &data);
		    evt.value = data;
		    nfbi_indicate_evt(&evt);
		}
		
		if( status & ~(IP_STATUS_MASK_ALL) ) 
		{
			PDEBUG("Got satus=0x%x, not supported yet!\n", (unsigned int)status);
		}
	    
	    rtl_mdio_read(NFBI_REG_ISR, &status);
		nfbi_dump_interrupt(status,mask);
	    status &= mask;
	}

	//printk("end nfbi_interrupt\n");
	return IRQ_HANDLED;
}

static int nfbi_open(struct inode *inode, struct file *filp)
{
    struct nfbi_priv *priv;

	//printk("%s: major=%d, minor=%d\n", __FUNCTION__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
	//printk("filp=%p\n", filp);
	priv = (struct nfbi_priv *)kmalloc(sizeof(struct nfbi_priv), GFP_KERNEL);
	if(!priv)
	{
	    //printk(KERN_ERR DRIVER_NAME": unable to kmalloc for nfbi_priv\n");
		return -ENOMEM;
	}
	memset((void *)priv, 0, sizeof (struct nfbi_priv));
	priv->state = STATE_TX_INIT;
    priv->retransmit_count = NFBI_RETRANSMIT_COUNT_DEFAULT;
	priv->tx_cmdword_interval = 0;
	priv->interrupt_timeout = NFBI_INTERRUPT_TIMEOUT_DEFAULT;
    priv->response_timeout = NFBI_RESPONSE_TIMEOUT_DEFAULT;
	filp->private_data = priv;

	/* Not use in linux-2.6.x*/
	//MOD_INC_USE_COUNT;
	return 0;;
}

static int nfbi_release(struct inode *inode, struct file *filp)
{
	//printk("%s: major=%d, minor=%d\n", __FUNCTION__,  MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
    kfree((struct nfbi_priv *)filp->private_data);

	/* Not use in linux-2.6.x*/
	//MOD_DEC_USE_COUNT;
	return 0;
}


static void nfbi_timer_fn(unsigned long arg)
{
    ndev_priv->timer_expired = 1;
    wake_up_interruptible(&ndev_priv->wq);
}

static ssize_t nfbi_read (struct file *filp, char *buf, size_t count, loff_t *offset)
{
	struct nfbi_priv *priv = (struct nfbi_priv *)filp->private_data;

	if (!buf)
		return 0;

	if (priv->state != STATE_RX_FINISHED) 
	{
		PDEBUG("To read status frame, but not in a valid state!\n");
		return 0;
	}

	if (copy_to_user((void *)buf, (void *)priv->data_in.buf, priv->data_in.len)) 
	{
		PDEBUG("copy_to_user() error!\n");
		count = -EFAULT;
	}else 
		count = priv->data_in.len;

	return count;
}


static ssize_t nfbi_write (struct file *filp, const char *buf, size_t count, loff_t *offset)
{
	struct nfbi_priv *priv = (struct nfbi_priv *)filp->private_data;
	unsigned short data;
	int retransmit_count;

	if (!buf)
		return 0;
	if ( priv->state!=STATE_TX_INIT ) 
	{
		if( (priv->state!=STATE_TX_INIT)&&(priv->state!=STATE_RX_FINISHED) ) 
			NTX_MSG_PRT("Transmit status, but not in valid state. Reset state!\n");
		priv->state = STATE_TX_INIT;
	}
	if (count > NFBI_BUFSIZE) 
	{
		PDEBUG("Tx size too big [%d]!\n", count);
		return 0;
	}
	if (copy_from_user((void *)priv->data_out.buf, (void *)buf, count)) 
	{
		PDEBUG("copy_from_user() error!\n");		
		return -EFAULT;
	}
	priv->data_out.buf[count]=append_checksum(priv->data_out.buf, count);
	priv->data_out.len = count + 1;
	if(ntx_msg_dbg) 
		nfbi_dump_data( "txdump", priv->data_out.buf, priv->data_out.len );

	if (down_interruptible(&ndev_priv->sem))
		return -ERESTARTSYS;

    ndev_priv->filp = filp;
    ndev_priv->tx_command_frames++;
    retransmit_count = priv->retransmit_count;
    //transmit command frame
retransmit:
	priv->tx_cmd_transmitting_len = 0;
	priv->tx_flag=0;
	priv->state = STATE_TX_IN_PROGRESS;
    nfbi_transmit_msg(priv);
	do{
		//PDEBUG("\"%s\"(pid %i) writing: going to sleep\n", current->comm, current->pid);
	    /* register the timer */
	    ndev_priv->timer_expired = 0; /* if timer expired, wake up processes in the wait queue */
	    ndev_priv->timer.data = 0;
	    ndev_priv->timer.function = nfbi_timer_fn;
	    ndev_priv->timer.expires = jiffies + priv->interrupt_timeout; /* in jiffies */
	    add_timer(&ndev_priv->timer);
		if (wait_event_interruptible(ndev_priv->wq, (ndev_priv->tx_msg_is_fetched || ndev_priv->timer_expired))) 
		{
			priv->state = STATE_TX_INIT;
			del_timer(&ndev_priv->timer);
			ndev_priv->filp = NULL;
			ndev_priv->tx_stop_by_signals++;
		    up(&ndev_priv->sem); /* release the lock */
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		}
		if (ndev_priv->timer_expired) //timeout
		{ 
	        priv->state = STATE_TX_INIT;
	        ndev_priv->tx_interupt_timeouts++;
	        if (retransmit_count > 0) 
			{
	            retransmit_count--;
	            ndev_priv->tx_retransmit_counts++;
	            printk(".\n");
	            goto retransmit;
	        }
	        ndev_priv->filp = NULL;
	        up(&ndev_priv->sem); // release the lock
			return -ETIME;
		}else
		    del_timer(&ndev_priv->timer);
	
    	if(priv->tx_flag&NTX_END)
		    priv->state = STATE_RX_INIT;
        else
		{
            if (priv->tx_cmdword_interval > 0) 
			{
               mdelay(priv->tx_cmdword_interval*10);
            }
            nfbi_transmit_msg(priv);
        }
	} while (priv->state != STATE_RX_INIT);
    ndev_priv->tx_done_command_frames++;

    
    //receive status frame
    RESET_RX_STATE;
	while (priv->state != STATE_RX_FINISHED) 
	{
		//PDEBUG("\"%s\"(pid %i) reading: going to sleep\n", current->comm, current->pid);
		/* register the timer */
	    ndev_priv->timer_expired = 0; /* if timer expired, wake up processes in the wait queue */
	    ndev_priv->timer.data = 0;
	    ndev_priv->timer.function = nfbi_timer_fn;
	    if (priv->state == STATE_RX_INIT)
            ndev_priv->timer.expires = jiffies + priv->response_timeout; /* in jiffies */
	    else /* STATE_RX_WAIT_LEN or STATE_RX_WAIT_DATA*/
	        ndev_priv->timer.expires = jiffies + priv->interrupt_timeout; /* in jiffies */

	    add_timer(&ndev_priv->timer);
		if (wait_event_interruptible(ndev_priv->wq, (ndev_priv->rx_msg_is_coming || ndev_priv->timer_expired))) 
		{
			priv->state = STATE_TX_INIT;
			del_timer(&ndev_priv->timer);
			ndev_priv->rx_stop_by_signals++;
		    up(&ndev_priv->sem); /* release the lock */
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		}
	    if (ndev_priv->timer_expired) //timeout
		{ 
	        if (priv->state == STATE_RX_INIT)
	            ndev_priv->rx_response_timeouts++;
	        else
	            ndev_priv->rx_interupt_timeouts++;
	        priv->state = STATE_TX_INIT;
	        if (retransmit_count > 0) 
			{
	            retransmit_count--;
	            ndev_priv->tx_retransmit_counts++;
	            printk("+\n");
	            goto retransmit;
	        }
	        up(&ndev_priv->sem); // release the lock
	        PDEBUG("ETIME=%d\n", ETIME);
			return -ETIME;
		}else
		    del_timer(&ndev_priv->timer);

        ndev_priv->rx_msg_is_coming = 0;
		rtl_mdio_read(NFBI_REG_RSR, &data);
        ndev_priv->rx_words++;
		ndev_priv->rx_status_time = jiffies;
	    nfbi_process_rx_msg(priv, data);
	}

    ndev_priv->rx_status_frames++;
    ndev_priv->filp = NULL;
    up(&ndev_priv->sem);    

	return count;
}


/*================================================================*/
/* Description :
 *   char *buf : byte array should be big endian image for RTL8197B
 */
static int _nfbi_mem_write(int addr, int len, char *buf)
{
    int ret, i;
    unsigned short val;
    char tmp[4];

    if (0 != rtl_mdio_write(NFBI_REG_CMD, 0x0000)) //write mode
        return -1;
#ifdef NFBI_USE_MEM_DWORD
    if (0 != rtl_mdio_write_dword(NFBI_REG_ADDH, addr))
        return -1;
#else  /*NFBI_USE_MEM_DWORD*/
    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
#endif /*NFBI_USE_MEM_DWORD*/



    if (len >= 4) 
	{
    	for (i=0; i<len; i+=4) 
		{
#ifdef NFBI_USE_MEM_DWORD
      	    if (0 != rtl_mdio_write_dword(NFBI_REG_DH, *(unsigned int *)(buf+i)))
                return -1;
#else  /*NFBI_USE_MEM_DWORD*/
      	    if (0 != rtl_mdio_write(NFBI_REG_DH, *(unsigned short *)(buf+i))) //data H
                return -1;
            if (0 != rtl_mdio_write(NFBI_REG_DL, *(unsigned short *)(buf+i+2))) //data L
                return -1;
#endif /*NFBI_USE_MEM_DWORD*/



#ifdef CHECK_NFBI_BUSY_BIT
#ifdef _NFBI_MEMIO_USE_DELAY_
			udelay(100);//mdelay(1);
#else
            // check NFBI hardware status
        	do {
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
#endif /*CHECK_NFBI_BUSY_BIT*/

        }
	}
	
	if ( (len%4) )
	{
	    memset(tmp, 0, 4);
		memcpy(tmp, (buf+(len/4)*4),(len%4));

#ifdef NFBI_USE_MEM_DWORD
		if (0 != rtl_mdio_write_dword(NFBI_REG_DH, *(unsigned int *)tmp))
            return -1;
#else  /*NFBI_USE_MEM_DWORD*/
		if (0 != rtl_mdio_write(NFBI_REG_DH, *(unsigned short *)tmp)) //data H
            return -1;
        if (0 != rtl_mdio_write(NFBI_REG_DL, *(unsigned short *)(tmp+2))) //data L
            return -1;
#endif /*NFBI_USE_MEM_DWORD*/



#ifdef CHECK_NFBI_BUSY_BIT
#ifdef _NFBI_MEMIO_USE_DELAY_
		udelay(100);//mdelay(1);
#else
        // check NFBI hardware status
        do {
            ret = rtl_mdio_read(NFBI_REG_CMD, &val);
            if (ret != 0)
                return -1;
        } while (val&BM_BUSY); //wait busy bit to zero
#endif
#endif /*CHECK_NFBI_BUSY_BIT*/

    }
    return 0;
}

static int _nfbi_mem_read(int addr, int len, char *buf)
{
    int ret, i;
    unsigned short val;
    char tmp[4];

    if (0 != rtl_mdio_write(NFBI_REG_CMD, 0x8000)) //read mode
        return -1;
#ifdef NFBI_USE_MEM_DWORD
    if (0 != rtl_mdio_write_dword(NFBI_REG_ADDH, addr))
        return -1;
#else  /*NFBI_USE_MEM_DWORD*/
    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
#endif /*NFBI_USE_MEM_DWORD*/



    if (len >= 4) 
	{
    	for (i=0; i<len; i+=4) 
		{
#ifdef NFBI_USE_MEM_DWORD
      	    if (0 != rtl_mdio_read_dword(NFBI_REG_DH, (unsigned int *)(buf+i)))
                return -1;
#else  /*NFBI_USE_MEM_DWORD*/
      	    if (0 != rtl_mdio_read(NFBI_REG_DH, (unsigned short *)(buf+i))) //data H
                return -1;
            if (0 != rtl_mdio_read(NFBI_REG_DL, (unsigned short *)(buf+i+2))) //data L
                return -1;
#endif /*NFBI_USE_MEM_DWORD*/



#ifdef CHECK_NFBI_BUSY_BIT
#ifdef _NFBI_MEMIO_USE_DELAY_
			udelay(100);//mdelay(1);
#else
            // check NFBI hardware status
        	do{
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
#endif /*CHECK_NFBI_BUSY_BIT*/
        }
	}
	
	if ( (len%4) )
	{
#ifdef NFBI_USE_MEM_DWORD
        if (0 != rtl_mdio_read_dword(NFBI_REG_DH, (unsigned int *)tmp))
            return -1;
#else  /*NFBI_USE_MEM_DWORD*/
        if (0 != rtl_mdio_read(NFBI_REG_DH, (unsigned short *)tmp)) //data H
            return -1;
        if (0 != rtl_mdio_read(NFBI_REG_DL, (unsigned short *)(tmp+2))) //data L
            return -1;
#endif /*NFBI_USE_MEM_DWORD*/



#ifdef CHECK_NFBI_BUSY_BIT
#ifdef _NFBI_MEMIO_USE_DELAY_
			udelay(100);//mdelay(1);
#else
        // check NFBI hardware status
        do {
            ret = rtl_mdio_read(NFBI_REG_CMD, &val);
            if (ret != 0)
                return -1;
        } while (val&BM_BUSY); //wait busy bit to zero
#endif
#endif /*CHECK_NFBI_BUSY_BIT*/

        memcpy((buf+(len/4)*4), tmp, (len%4));
    }
    return 0;
}


static int nfbi_mem_write(int addr, int len, char *buf)
{
    if (len > NFBI_MAX_BULK_MEM_SIZE)
        return -1;
	return _nfbi_mem_write(addr,len,buf);
}


static int nfbi_mem_read(int addr, int len, char *buf)
{
    if (len > NFBI_MAX_BULK_MEM_SIZE)
        return -1;
	return _nfbi_mem_read(addr,len,buf);
}

static void nfbi_hw_reset(int type)
{
#if 1
    int ret;
    unsigned short val;

	//printk( "\nTodo: nfbi_hw_reset(%d). Try CMD_SystemRst instead\n", type );
	//currently, there is no hw reset, try CMD_SystemRst instead.
	rtl_mdio_write(NFBI_REG_CMD, BM_SYSTEMRST);
	do {
		ret = rtl_mdio_read(NFBI_REG_CMD, &val);
		if (ret != 0)
			break;
	} while (val&BM_SYSTEMRST); //wait BM_SYSTEMRST bit to zero

	return;
#else
    switch(type) {
      case 0:  //pull low
        Set_NFBI_RESET_L();
        break;
      case 1:  //pull high
        Set_NFBI_RESET_H();
        break;
      case 2: //hardware reset
#ifndef HOST_IS_PANABOARD
	// In 8198 platform, interrupt will come very soon after pull-low reset pin.
	// When the interrupt occurs, the hw reset sequence will be interleaved by ISR
	// and that would cause the reset action unable to compete.
	// Therefore, it's necessary to disable interrupt before hardware reset.
	REG32(PABIMR_REG) = REG32(PABIMR_REG) & 0x3fffffff; //set B7 falling interrupt(bit31,30), 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
#endif
        Set_NFBI_RESET_L();
        mdelay(400); //To reset RTL8197B CPU, the RESETn pin must be pull-low at least 350 ms.
        Set_NFBI_RESET_H();
#ifdef HOST_IS_PANABOARD
        /* Ebina add for Micrel Isolation */
        avev3_mdio_write(avev3_dev, 0x1, MII_BMCR, 0x3400);
#endif
#ifndef HOST_IS_PANABOARD
	// enable interrupt after hardware reset
	REG32(PABIMR_REG) = REG32(PABIMR_REG) | (0x01 <<30); //set B7 falling interrupt(bit31,30), 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
#endif
        break;
    }
#endif
}

#ifdef USE_RISING_EDGE
static void nfbi_set_slave_edge(void)
{
	unsigned char phyaddr;

	//set RR=0x1, nfbi_phyaddr is not ready and max 3 nfbi slaves
	for (phyaddr=1; phyaddr<4; phyaddr++)
	{
		//printk( "phyaddr=%u\n", phyaddr );
		nfbi867x_mdio_write_one_word( phyaddr, NFBI_REG_RR, 0x1 );
	}

	return;
}
#endif /*USE_RISING_EDGE*/

static unsigned char nfbi_probephyaddr(void)
{
	int i;
	unsigned short ret_val=0;
	int reg = NFBI_REG_PHYID2; //PHYID2
	unsigned char phyaddr;

	//for (i=0; i<32; i++) 
	for (i=1; i<4; i++) 
	{
		phyaddr=i;
		if( nfbi867x_mdio_read_one_word( phyaddr&0x1f, reg&0x1f, &ret_val )<0 )
			printk("Timeout! Probe PHY Addr=%x fail!\n",phyaddr);

		printk("Probe PHY ADDR=0x%x  val=0x%x\n",phyaddr, ret_val&0xffff);
		if (((ret_val&0xffff)==NFBI_REG_PHYID2_DEFAULT) ||
			((ret_val&0xffff)==NFBI_REG_PHYID2_DEFAULT2)|| 
			((ret_val&0xffff)==NFBI_REG_PHYID2_DEFAULT3)|| 
			((ret_val&0xffff)==NFBI_REG_PHYID2_DEFAULT4)) 
		{	
			printk("Pass! Probe PHY ADDR = %d\n", phyaddr);
			ndev_priv->ready = 1;
			return phyaddr;
		}
	}

	//if(i==32)
	if(i==4)
	    printk("Error! NFBI maybe not connect!\n");
	
	ndev_priv->ready = 0;
	return 0xff;
}

static void dump_private_data(void)
{
	int i;
	struct nfbi_priv *priv;
    
    printk("ready=%d\n", ndev_priv->ready);
    printk("nfbi_phyaddr=%d\n", nfbi_phyaddr);
	printk("hcd_pid=%d\n", ndev_priv->hcd_pid);
	printk("evt_que_head=%d\n", ndev_priv->evt_que_head);
    printk("evt_que_tail=%d\n", ndev_priv->evt_que_tail);
    printk("evt_que count=%d\n", CIRC_CNT(ndev_priv->evt_que_head, ndev_priv->evt_que_tail, EV_QUE_MAX));
    printk("evt_que space=%d\n", CIRC_SPACE(ndev_priv->evt_que_head, ndev_priv->evt_que_tail, EV_QUE_MAX));
	printk("tx_msg_is_fetched=%d\n", ndev_priv->tx_msg_is_fetched);
    printk("rx_msg_is_coming=%d\n", ndev_priv->rx_msg_is_coming);
	printk("ndev_priv->filp:%p\n", ndev_priv->filp);
	
	if (ndev_priv->filp != NULL) 
	{
	    priv = (struct nfbi_priv *)ndev_priv->filp->private_data;
	    printk("retransmit_count=%d (in 10ms)\n", priv->retransmit_count);
		printk("tx_cmdword_interval:%d (in 10ms)\n", priv->tx_cmdword_interval);
	    printk("interrupt_timeout=%d (in 10ms)\n", priv->interrupt_timeout);
		printk("response_timeout:%d (in 10ms)\n", priv->response_timeout);
    	printk("state:");
	    	switch(priv->state) 
			{
	    	case STATE_TX_INIT:
	    	  	printk("STATE_TX_INIT\n");
	    	  	break;
	    	case STATE_TX_IN_PROGRESS:
	    	  	printk("STATE_TX_IN_PROGRESS\n");
	    	  	break;
	        case STATE_RX_INIT:
	    	  	printk("STATE_RX_INIT\n");
	    	  	break;
	    	case STATE_RX_WAIT_DATA:
	    	  	printk("STATE_RX_WAIT_DATA\n");
	    	  	break;
	    	case STATE_RX_FINISHED:
	    	  	printk("STATE_RX_FINISHED\n");
	    	  	break;
	    	}
        printk("Tx:\n");
		printk("tx_flag: ");
			if(priv->tx_flag&NTX_START) printk("NTX_START ");
			if(priv->tx_flag&NTX_END) printk("NTX_END ");
			if(priv->tx_flag&NTX_ESC) printk("NTX_ESC ");
			printk("\n");
    	printk("data_out:");
	    	for(i=0;i<priv->data_out.len;i++) {
	    		printk("%02x ", priv->data_out.buf[i]);
	    	}
	    	printk("\n");
    	printk("tx_cmd_transmitting_len=%d\n", priv->tx_cmd_transmitting_len);
    
    	printk("Rx:\n");
		printk("rx_flag: ");
			if(priv->rx_flag&NRX_START) printk("NRX_START ");
			if(priv->rx_flag&NRX_END) printk("NRX_END ");
			if(priv->rx_flag&NRX_ESC) printk("NRX_ESC ");
			printk("\n");
    	printk("data_in:");
    	for(i=0;i<priv->data_in.len;i++) {
    		printk("%02x ", priv->data_in.buf[i]);
    	}
    	printk("\n");
    }
    printk("===============================================\n");
    printk("Statistics:\n");
    printk("tx_command_frames=%d\n", ndev_priv->tx_command_frames);
    printk("tx_done_command_frames=%d\n", ndev_priv->tx_done_command_frames);
    printk("tx_retransmit_counts=%d\n", ndev_priv->tx_retransmit_counts);
    printk("tx_words=%d\n", ndev_priv->tx_words);
    printk("tx_interupt_timeouts=%d\n", ndev_priv->tx_interupt_timeouts);
    printk("tx_stop_by_signals=%d\n", ndev_priv->tx_stop_by_signals);
    printk("rx_status_frames=%d\n", ndev_priv->rx_status_frames);
   	printk("rx_status_time=%lx jiffies=%lx (%d sec)\n", ndev_priv->rx_status_time, jiffies, (int)(jiffies-ndev_priv->rx_status_time)/HZ);
    printk("rx_words=%d\n", ndev_priv->rx_words);
    printk("rx_response_timeouts=%d\n", ndev_priv->rx_response_timeouts);
    printk("rx_interupt_timeouts=%d\n", ndev_priv->rx_interupt_timeouts);
    printk("rx_stop_by_signals=%d\n", ndev_priv->rx_stop_by_signals);
    printk("rx_not_1st_word_errors=%d\n", ndev_priv->rx_not_1st_word_errors);
    printk("rx_1st_byte_errors=%d\n", ndev_priv->rx_1st_byte_errors);
    printk("rx_cmdid_not_match_errors=%d\n", ndev_priv->rx_cmdid_not_match_errors);
    printk("rx_reset_by_sync_bit_errors=%d\n", ndev_priv->rx_reset_by_sync_bit_errors);
    printk("rx_checksum_errors=%d\n", ndev_priv->rx_checksum_errors);
}


/*================================================================*/
#if 0 /*test*/
#include <linux/syscalls.h>
void nfbi_write_file_test(void)
{
	int fd;
	int read_bytes;
	mm_segment_t       old_fs;
	char buf[8*1024];
	unsigned int kerneladdr=0x00400000;
	//struct stat ffstat;
	int buf_size=sizeof(buf);
	unsigned long start=jiffies,end;
	

	printk( "start jiffies=%u, %d\n", start, sizeof(start) );

	old_fs = get_fs();
	set_fs(KERNEL_DS);	
	if ((fd = sys_open("/etc/fw/linux.bin", O_RDONLY, 0)) < 0)
	{
		printk( "open /etc/fw/linux.bin failed\n" );
		set_fs(old_fs);
		return;
	}

	read_bytes = sys_read(fd, buf, buf_size);
	while(read_bytes)
	{
		if(read_bytes!=buf_size)
			printk( "read: %d\n", read_bytes );
		_nfbi_mem_write( kerneladdr, read_bytes, buf );
		kerneladdr = kerneladdr+read_bytes;
		read_bytes = sys_read(fd, buf, buf_size);
	}	
	sys_close(fd);
	set_fs(old_fs);

	end=jiffies;
	printk( "done jiffies=%u, diff=%u\n", end, end-start);
	return;
		
}
#endif

static void nfbi_private_command(int type)
{
    switch(type) 
	{
	case 0: //pull low
	case 1: //pull high
	case 2: //hardware reset
		nfbi_hw_reset(type);
		break;
	case 3:
		#if 1
		nfbi_probephyaddr();
		#else
		{
			unsigned short v1,v2;
			v1=0x54;
			nfbi_set_FDCR(v1);
			nfbi_get_FDCR(&v2);
			printk("test FDCR set 0x%02x, get 0x%02x\n", v1, v2);
			v1=0xae;
			nfbi_set_SDCR(v1);
			nfbi_get_SDCR(&v2);
			printk("test SDCR set 0x%02x, get 0x%02x\n", v1, v2);
		}
		#endif
		break;
	case 4: //dump private data
		//nfbi_write_file_test();
		dump_private_data();
		break;
	case 5: //reset event queue
		ndev_priv->evt_que_head = 0;
		ndev_priv->evt_que_tail = 0;
		break;
    }
}


/*
 * The ioctl() implementation
 * Refer to the book "Linux Device Drivers" by Alessandro Rubini and Jonathan Corbet, 
 * published by O'Reilly & Associates.
 */
static int nfbi_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int retval = 0, tmp;
	unsigned short val;
	struct nfbi_bulk_mem_param *pbulkmem;
	struct nfbi_mem32_param mem32_param;
	struct evt_msg evt;
	struct nfbi_priv *priv;
    
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != NFBI_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > NFBI_IOCTL_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	/*
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
	*/
    
	switch(cmd) 
	{
//private command
	case NFBI_IOCTL_PRIV_CMD:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
	        nfbi_private_command(tmp);
	    }
	    break;


//read/write phy reg
	case NFBI_IOCTL_REGREAD:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
            /* The register address to be read is stored at the high word of tmp. */
    	    retval = rtl_mdio_read((tmp >> 16) & 0xffff, &val); //@Pana_TBD
            if (retval == 0) 
			{
				/* 
				* stored register address at the high word of tmp and
				* value at the low word of tmp, and then copy to user space
				*/
                tmp = (tmp & 0xffff0000) | val;
        		retval = put_user(tmp, (int *)arg);
    	    }
	    }
		break;
	case NFBI_IOCTL_REGWRITE:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
			/* 
			* The register address is stored at the high word of tmp.
			* The value to be written is stored at the low word of tmp.
			*/
    	    retval = rtl_mdio_write((tmp >> 16) & 0xffff, tmp&0xffff); //@Pana_TBD
    	}
		break;


//read/write slave's memory 
	case NFBI_IOCTL_MEM32_WRITE:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        if (retval == 0)
	        retval = nfbi_mem_write(mem32_param.addr, sizeof(int), (char *)&(mem32_param.val));
        break;
	case NFBI_IOCTL_MEM32_READ:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
        if (retval == 0)
	        retval = nfbi_mem_read(mem32_param.addr, sizeof(int), (char *)&(mem32_param.val));
        if (retval == 0)
	        retval = copy_to_user((struct nfbi_mem32_param *)arg, &mem32_param, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        break;
	case NFBI_IOCTL_BULK_MEM_WRITE:
        pbulkmem  = (struct nfbi_bulk_mem_param *)kmalloc(sizeof(struct nfbi_bulk_mem_param), GFP_KERNEL);
	    if(!pbulkmem) 
		{
    	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc\n");
    		return -ENOMEM;
    	}
    	retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, sizeof(struct nfbi_bulk_mem_param));
	    //printk("pbulkmem->addr=%x pbulkmem->len=%d\n", pbulkmem->addr, pbulkmem->len);
        if (retval == 0)
	        retval = nfbi_mem_write(pbulkmem->addr, pbulkmem->len, pbulkmem->buf);
	    kfree(pbulkmem);
	    //printk(">");
        break;
	case NFBI_IOCTL_BULK_MEM_READ:
        pbulkmem  = (struct nfbi_bulk_mem_param *)kmalloc(sizeof(struct nfbi_bulk_mem_param), GFP_KERNEL);
	    if(!pbulkmem) 
		{
    	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc\n");
    		return -ENOMEM;
    	}
    	//retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, sizeof(struct nfbi_bulk_mem_param));
    	retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, 8);
    	if (retval == 0) 
		{
    	    //printk("pbulkmem->addr=%x pbulkmem->len=%d\n", pbulkmem->addr, pbulkmem->len);
	        retval = nfbi_mem_read(pbulkmem->addr, pbulkmem->len, pbulkmem->buf);
	    }
	    if (retval == 0)
	        retval = copy_to_user((struct nfbi_bulk_mem_param *)arg, pbulkmem, sizeof(struct nfbi_bulk_mem_param));
	    kfree(pbulkmem);
	    //printk("<");
        break;


//read/write local memory 
	case NFBI_IOCTL_EW:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        if (retval == 0)
            REG32(mem32_param.addr) = mem32_param.val;
        break;
	case NFBI_IOCTL_DW:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
        if (retval == 0)
            mem32_param.val = REG32(mem32_param.addr);
        if (retval == 0)
	        retval = copy_to_user((struct nfbi_mem32_param *)arg, &mem32_param, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        break;


// get/set parameters for message channel
	case NFBI_IOCTL_TX_CMDWORD_INTERVAL:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
	        priv = (struct nfbi_priv *)filp->private_data;
	        if (tmp & 0x10000) //set
	            priv->tx_cmdword_interval = tmp & 0xffff; 
	        else{ //get
	            tmp = priv->tx_cmdword_interval; 
	            retval = put_user(tmp, (int *)arg);
	        }
	    }
        break;
	case NFBI_IOCTL_INTERRUPT_TIMEOUT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
	        priv = (struct nfbi_priv *)filp->private_data;
	        if (tmp & 0x10000)//set
	            priv->interrupt_timeout = tmp & 0xffff; 
	        else {//get
	            tmp = priv->interrupt_timeout; 
	            retval = put_user(tmp, (int *)arg);
	        }
	    }
        break;
	case NFBI_IOCTL_RETRANSMIT_COUNT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
	        priv = (struct nfbi_priv *)filp->private_data;
            priv->retransmit_count = tmp;
            //PDEBUG("priv->retransmit_count=%d\n", priv->retransmit_count);
	    }
        break;
	case NFBI_IOCTL_RESPONSE_TIMEOUT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
	        priv = (struct nfbi_priv *)filp->private_data;
            priv->response_timeout = tmp;
            //PDEBUG("priv->response_timeout=%d\n", priv->response_timeout);
	    }
        break;


//others
	case NFBI_IOCTL_HCD_PID:
	    if (0 == get_user(tmp, (int *)arg)) 
		{
	        ndev_priv->hcd_pid = tmp;
	        //PDEBUG("hcd_pid=%d\n", ndev_priv->hcd_pid);
	    }
        break;
	case NFBI_IOCTL_GET_EVENT:
		nfbi_retrieve_evt(&evt);
        retval = copy_to_user((struct evt_msg *)arg, &evt, sizeof(struct evt_msg));
        break;
	case NFBI_IOCTL_MDIO_PHYAD:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) 
		{
            nfbi_phyaddr = (unsigned char)tmp;
            if ((nfbi_phyaddr==1)|| 
				(nfbi_phyaddr==2)||
				(nfbi_phyaddr==3) )
                ndev_priv->ready = 1;
            else
                ndev_priv->ready = 0;
            //PDEBUG("nfbi_phyaddr=%d\n", nfbi_phyaddr);
	    }
        break;
	default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

	return retval;
}


static struct file_operations nfbi_fops = 
{
	owner:		THIS_MODULE,
	read:		nfbi_read,
	write:		nfbi_write,
	ioctl:		nfbi_ioctl,
	open:		nfbi_open,
	release:	nfbi_release,
};

static void __exit nfbi_exit(void)
{
	nfbi_remove_proc();
#ifndef NFBI_GPIO_SIMULATE
    free_irq(nfbi_irq, ndev_priv);
#endif /*NFBI_GPIO_SIMULATE*/
	unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
    if(ndev_priv) kfree(ndev_priv);
    ndev_priv = NULL;
}

static int __init nfbi_init(void)
{
#ifdef NFBI_GPIO_SIMULATE
	printk( "%s: use gpio as mdc/mdio\n", __FUNCTION__ );
#else /*NFBI_GPIO_SIMULATE*/
	printk( "%s: CHIPID REG32(0xb8000024)=0x%08x\n", __FUNCTION__, REG32(0xb8000024) );
	printk( "%s: REG32(0xb800010c)=0x%08x (bit 2)\n", __FUNCTION__, REG32(0xb800010c) );

	//set Reg.IOPDCR: bit8 gpiob_drvsel, bit 7 gpioa_drvsel
	REG32(0xb8000110) = REG32(0xb8000110) | 0x180;
	printk( "%s: REG32(0xb8000110)=0x%08x (bit8,bit7)\n", __FUNCTION__, REG32(0xb8000110) );
	
	// Set Bonding Master mode
	REG32(BSP_MISC_PINMUX)=(REG32(BSP_MISC_PINMUX)&(~BSP_BD_SLV_MODE))|(BSP_GPON_BD|BSP_BD_MST_MODE);
	printk( "%s: set bonding master mode\n", __FUNCTION__ );

	#ifdef USE_RISING_EDGE
	nfbi_set_slave_edge();
	#endif /*USE_RISING_EDGE*/
#endif /*NFBI_GPIO_SIMULATE*/


	ndev_priv = (struct nfbi_dev_priv *)kmalloc(sizeof (struct nfbi_dev_priv), GFP_KERNEL);
	if(!ndev_priv) 
	{
	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc for nfbi_dev_priv\n");
		return -ENOMEM;
	}
	memset((void *)ndev_priv, 0, sizeof (struct nfbi_dev_priv));
	ndev_priv->hcd_pid = -1;
	init_waitqueue_head(&(ndev_priv->wq));
	init_timer(&ndev_priv->timer);
	init_MUTEX(&ndev_priv->sem);

	if (register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &nfbi_fops)) 
	{
		printk(KERN_ERR DRIVER_NAME": unable to get major %d\n", DRIVER_MAJOR);
		kfree(ndev_priv);
		return -EIO;
	}

    // set the pre-selected phy address for MDIO address or probe the correct one
#if 0
	nfbi_phyaddr = DEFAULT_MDIO_PHYAD;
	ndev_priv->ready = 1;
#else
	nfbi_phyaddr = nfbi_probephyaddr();
#endif


	if(ndev_priv->ready)
	{
#ifdef SET_INT_LEVEL
		unsigned short val;
		rtl_mdio_read(NFBI_REG_CMD, &val);
		printk( "read NFBI Reg.CMD=0x%04x\n", val );
		rtl_mdio_write(NFBI_REG_CMD, INT_LEVEL_DEF);
		rtl_mdio_read(NFBI_REG_CMD, &val);
		printk( "read NFBI Reg.CMD=0x%04x\n", val );
#endif /*SET_INT_LEVEL*/
	
	    rtl_mdio_write(NFBI_REG_IMR, 0);
#ifndef NFBI_GPIO_SIMULATE
		if (request_irq(nfbi_irq, nfbi_interrupt, IRQF_DISABLED, DRIVER_NAME, (void *)ndev_priv)) 
		{	    
			printk(KERN_ERR DRIVER_NAME": IRQ %d is not free.\n", nfbi_irq);
			kfree(ndev_priv);
			return -1;
		}
#endif /*NFBI_GPIO_SIMULATE*/
	}


#ifdef NFBI_GPIO_SIMULATE
	printk(KERN_INFO DRIVER_NAME" driver "DRIVER_VER"\n");
#else /*NFBI_GPIO_SIMULATE*/
	printk(KERN_INFO DRIVER_NAME" driver "DRIVER_VER" at %X (Interrupt %d)\n", nfbi_io, nfbi_irq);
#endif /*NFBI_GPIO_SIMULATE*/

	nfbi_create_proc();

	return 0;
}

module_init(nfbi_init);
module_exit(nfbi_exit);

MODULE_DESCRIPTION("Driver for RTL867X NFBI");
MODULE_LICENSE("none-GPL");

