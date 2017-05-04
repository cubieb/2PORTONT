/*
  *   RTL867x MDIO char driver
  *
  *	Copyright (C)2012, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: rtl867x_mdio.c,v 1.2 2012/07/25 09:35:33 jiunming Exp $
  *
  */

/*================================================================*/
/* System Include Files */

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
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include "bspchip.h"
#include "rtl867x_mdio.h"

/*================================================================*/
#define KDB_ERR							// defined to print out error message
//#define KDB_MSG						// defined to print out debug message
//#define SIMULATION						// defined to do simuation

/*================================================================*/
#ifdef KDB_MSG
#define DEBUGK_OUT(fmt, args...) \
	printk("%s_%s: "fmt, DRIVER_NAME, __FUNCTION__, ## args)
#define ASSERT(expr) \
    if(!(expr)) {					\
			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
        __FILE__,__LINE__,#expr);		\
    }	
#else
#define DEBUGK_OUT(fmt, args...)
#define ASSERT(expr)	
#endif

#ifdef KDB_ERR
#define DEBUGK_ERR(fmt, args...) \
	printk("%s_%s_ERR: "fmt, DRIVER_NAME, __FUNCTION__, ## args)
#else
#define DEBUGK_ERR(fmt, args...)
#endif

/*================================================================*/
#define TIME_DIFF(a, b)	((a >= b)? (a - b):(0xffffffff - b + a + 1))
#define EVT_BUF_OFFSET	((int)(long *)&(((struct evt_msg *)0)->buf))
#define RESET_RX_STATE { \
		priv->rx_cmd_state = STATE_RX_INIT; \
		priv->rx_flag = 0; \
		priv->data_in.len = 0; \
		priv->rx_cmd_time = 0; \
}

/*================================================================*/
/* Local Variables */
static struct mdio_priv *dev_priv=NULL;


/*================================================================*/
/*debug msg channel*/
static int mrx_msg_dbg=0;
static int mtx_msg_dbg=0;
#define RX_MSG_PRT(format, args...)	if(mrx_msg_dbg) printk(format , ## args)
#define TX_MSG_PRT(format, args...)	if(mtx_msg_dbg) printk(format , ## args)
static struct proc_dir_entry *mdio_proc_root = NULL ;
#define MPROC_NAME_DIR	"mdio"
#define MPROC_NAME_RX	"mrx_msg_dbg"
#define MPROC_NAME_TX	"mtx_msg_dbg"

/*================================================================*/
/* Routine Implementations */

#ifdef SIMULATION
static unsigned char data_in[256];
static int data_in_len = 0, data_in_read_idx = 0;
static unsigned short data_out;
static int data_out_len = 0, msg_is_fetched = 0;
static unsigned long reg_scr =0, reg_isr=0; 

static unsigned long register_read_dw(int offset)
{
	unsigned long status = 0;
	unsigned short wdata = 0;

	if (offset == REG_ISR) {
		status = reg_isr;		
		
		if (data_in_len > data_in_read_idx) 
			status |= IP_NEWMSG;

		if (msg_is_fetched) {
			status |= IP_MSGFETCH;	
			msg_is_fetched = 0;
		}

		return status;			
	}	
	else if (offset == REG_RCR) {		
		ASSERT(data_in_len > data_in_read_idx);
		memcpy(&wdata, &data_in[data_in_read_idx], 2);				
		data_in_read_idx += 2;
		status |= wdata;			
		return status;			
	}
	else if (offset == REG_SYSCR) {		
		return reg_scr;		
	}	
	else {
		ASSERT(0);
		return status;	
	}	
}

static void register_write_dw(int offset, unsigned long data)
{
	if (offset ==	REG_SSR) {
		unsigned short wData = (unsigned short)data;
		
		memcpy(&data_out, &wData, 2);	
		data_out_len = 2;
	}	
	else if (offset ==	REG_ISR) {
		reg_isr &= ~data ;
	}
}

static int read_proc(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size = 0;

	if (data_out_len > 0) {	
		
fetch_again:		
		while (data_out_len > 0) {
			data_out_len = 0;
			size += sprintf(&buf[size], "%04x ", data_out);			
		}

		msg_is_fetched = 1;
		
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);			

		if (data_out_len > 0)
			goto fetch_again;		

		strcat(&buf[size++], "\n");	
	}

    return size;
}

static unsigned short _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {			
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				DEBUGK_ERR("error hex format [%x]!\n", *s);
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return (unsigned short)k;
}


static int write_proc(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[100];
	int len=count;
	unsigned short in_data;

	if (!memcmp(buffer, "cmd: ", 5)) {	
		buffer += 5;
		len -= 5;
		data_in_len = 0;
		while (len > 0) {
			memcpy(tmp, buffer, 4);
			tmp[4] = '\0';

			in_data = _atoi(tmp, 16);

			memcpy(&data_in[data_in_len], &in_data, 2);
			data_in_len += 2;
			len -= 5;
			buffer += 5;
		}

		data_in_read_idx = 0;
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);		
	}
	else if (!memcmp(buffer, "scr: ", 5)) {				
		buffer += 5;
		memcpy(tmp, buffer, 4);
		tmp[4] = '\0';
		reg_scr = _atoi(tmp, 16);
	}
	else if (!memcmp(buffer, "isr: ", 5)) {				
		buffer += 5;
		memcpy(tmp, buffer, 4);
		tmp[4] = '\0';
		reg_isr = _atoi(tmp, 16);
		
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);				
	}	
	else {
		printk("Invalid cmd!\n");		
	}
	
    return count;
}

#else  /*SIMULATION*/
#define register_read_dw(offset)		(REG32(offset))
#define register_write_dw(offset, data)	(REG32(offset)=data)
#endif /*SIMULATION*/


#ifdef KDB_MSG
static void inline debugk_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) 
	    DEBUGK_OUT("%s\n", label);	

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++) {   
        printk("\t");
        for (j=0; j<16; j++) 
            printk("%02x ", data[j + (i<<4)]);
        printk("\n");
    }

    if (block_remainder > 0) {    
        printk("\t");
        for (j=0; j<block_remainder; j++)
            printk("%02x ", data[j+(num_blocks<<4)]);
        printk("\n");
    }
}
#endif // KDB_MSG


/////////////////////////////////////////////////////////////////////////////////////
static int mdio_proc_read_tx(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size=0;
	size=sprintf(buf, "%d", mtx_msg_dbg);
	*eof=1;
    return size;
}

static int mdio_proc_write_tx(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[count];

	memset( tmp, 0, sizeof(tmp) );
	if( copy_from_user(tmp, buffer, count) ) 
		return -EFAULT;
	if(tmp[0]=='0') 
		mtx_msg_dbg=0;
	else
		mtx_msg_dbg=1;
    return count;
}

static int mdio_proc_read_rx(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size=0;
	size=sprintf(buf, "%d", mrx_msg_dbg);
	*eof=1;
    return size;
}

static int mdio_proc_write_rx(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[count];

	memset( tmp, 0, sizeof(tmp) );
	if( copy_from_user(tmp, buffer, count) ) 
		return -EFAULT;
	if(tmp[0]=='0') 
		mrx_msg_dbg=0;
	else
		mrx_msg_dbg=1;
    return count;
}


static void __exit mdio_remove_proc(void)
{
	remove_proc_entry( MPROC_NAME_RX, mdio_proc_root );
	remove_proc_entry( MPROC_NAME_TX, mdio_proc_root );
	remove_proc_entry( MPROC_NAME_DIR, NULL );
	mdio_proc_root=NULL;
#ifdef SIMULATION
	remove_proc_entry( "mdio_flag", NULL );
#endif /*SIMULATION*/
}

static int __init mdio_create_proc(void)
{
	struct proc_dir_entry *new_entry;

	mdio_proc_root=proc_mkdir(MPROC_NAME_DIR, NULL);
	if(mdio_proc_root==NULL)
	{
		printk( "create mdio proc root failed!\n" );
		return -1;
	}

	new_entry=create_proc_entry(MPROC_NAME_TX, 0644, mdio_proc_root);
	if(new_entry==NULL)
	{
		printk( "create proc tx_msg_dbg failed!\n" );
		mdio_remove_proc();
		return -1;
	}
	new_entry->write_proc=mdio_proc_write_tx;
	new_entry->read_proc=mdio_proc_read_tx;

	new_entry=create_proc_entry(MPROC_NAME_RX, 0644, mdio_proc_root);
	if(new_entry==NULL)
	{
		printk( "create proc rx_msg_dbg failed!\n" );
		mdio_remove_proc();
		return -1;
	}
	new_entry->write_proc=mdio_proc_write_rx;
	new_entry->read_proc=mdio_proc_read_rx;


#ifdef SIMULATION
	struct proc_dir_entry *res;
	res = create_proc_entry("mdio_flag", 0, NULL);
	if (res) 
	{
		res->read_proc = read_proc;
		res->write_proc = write_proc;
	}else{
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": unable to create /proc/mdio_flag\n");
		mdio_remove_proc();
		return -1;
	}		
#endif /*SIMULATION*/

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

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

static void mdio_dump_data( unsigned char *msg, unsigned char *p, int len )
{
	int i;

	printk( "\n\n(%s) mdio_dump_data start, len=%d\n", msg?msg:(unsigned char *)"", len );
	for(i=0; i<len; i++)
	{
		printk( "%02x ", p[i] );
		if(i && (i&0x7)==0x7) printk( "  " );
		if(i && (i&0xf)==0xf) printk( "\n" );
	}
	printk( "\n(%s) mdio_dump_data done\n\n", msg?msg:(unsigned char *)"" );
	

	return;
	
}


static int indicate_evt(struct mdio_priv *priv, int id, unsigned char *data, int data_len)
{
	int size;

	size = CIRC_SPACE(priv->evt_que_head, priv->evt_que_tail, EV_QUE_MAX);
	if (size == 0) 
	{
		DEBUGK_ERR("Indication queue full, drop event!\n");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
		if (priv->host_pid != -1)
			kill_proc(priv->host_pid, SIGUSR1, 1);
#else
		if (priv->host_pid != NULL)
			kill_pid(priv->host_pid, SIGUSR1, 1);
#endif

		return 0;
	}
	
	ASSERT(data_len <= MDIO_BUFSIZE);
		
	priv->ind_evt_que[priv->evt_que_head].id = id;
	priv->ind_evt_que[priv->evt_que_head].len = data_len;	
	memcpy(&priv->ind_evt_que[priv->evt_que_head].buf, data, data_len);
	priv->evt_que_head = (priv->evt_que_head + 1) & (EV_QUE_MAX - 1);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	if (priv->host_pid != -1)
		kill_proc(priv->host_pid, SIGUSR1, 1);
#else
	if (priv->host_pid != NULL)
		kill_pid(priv->host_pid, SIGUSR1, 1);
#endif

	return 1;
}

static int retrieve_evt(struct mdio_priv *priv, unsigned char *out)
{
	int len = 0;
	
	if (CIRC_CNT(priv->evt_que_head, priv->evt_que_tail, EV_QUE_MAX) > 0) 
	{   // more than one evt pending
		len = EVT_BUF_OFFSET + priv->ind_evt_que[priv->evt_que_tail].len;		
		memcpy(out, &priv->ind_evt_que[priv->evt_que_tail], len);
		priv->evt_que_tail = (priv->evt_que_tail + 1) & (EV_QUE_MAX - 1);		
	}
	return len;
}

static void nfbi_handle_put_rx_char(struct mdio_priv *priv, unsigned char tmp)
{
	if( priv->data_in.len<MDIO_MAX_BUFSIZE )
	{
		priv->data_in.buf[priv->data_in.len]=tmp;
		priv->data_in.len++;
		RX_MSG_PRT( "%02x ", tmp );
	}else{
		RX_MSG_PRT( "%s(): buffer is full, skip %02x byte!\n", __FUNCTION__, tmp );
	}

	return;
}

static void nfbi_handle_next_rx_char(struct mdio_priv *priv, unsigned char tmp)
{
	if(priv->rx_flag&NRX_ESC)
	{
		priv->rx_flag=priv->rx_flag&(~NRX_ESC);
		tmp=tmp^NFBI_PPP_TRANS;
		RX_MSG_PRT( "(rx_U)" );
		nfbi_handle_put_rx_char(priv, tmp);
	}else{
		if(tmp==NFBI_PPP_ESCAPE)
		{
			priv->rx_flag=priv->rx_flag|NRX_ESC;
			RX_MSG_PRT( "(rx_E)" );
		}else{
			nfbi_handle_put_rx_char(priv, tmp);
		}
	}

	return;
}


static void process_rx_cmd(struct mdio_priv *priv, unsigned short data)
{
	unsigned char *p=(unsigned char *)&data;
	unsigned char tmp;
	int i;
	
#if 0
	if( priv->rx_cmd_time &&
		(priv->rx_cmd_state==STATE_RX_WAIT_DATA) &&
		(priv->cmd_timeout && (TIME_DIFF(jiffies, priv->rx_cmd_time)>priv->cmd_timeout) ) ) 
	{
		DEBUGK_ERR("Rx cmd timeout [%ld][0x%x], discard pending cmd!\n", TIME_DIFF(jiffies, priv->rx_cmd_time), data);
		RESET_RX_STATE;
		return;
	}
#endif
	
	priv->rx_cmd_time = jiffies;
	for(i=0; i<2; i++)
	{
		tmp=p[i];
		if(tmp==NFBI_PPP_FLAG)
		{
			if(priv->data_in.len>0)
			{
				priv->rx_flag=priv->rx_flag|NRX_END;
				RX_MSG_PRT( "(rx_end)\n" );
				if(mrx_msg_dbg) 
					mdio_dump_data( "rxdone", priv->data_in.buf, priv->data_in.len );
				if (!is_checksum_ok(priv->data_in.buf, priv->data_in.len)) 
				{
					printk("rx cheksum error!\n");
					RESET_RX_STATE;
					priv->rx_flag=NRX_START;
					RX_MSG_PRT( "\n(rx_start)" );
				}else{
					priv->data_in.len -= 1; // substract checksum length
					indicate_evt(priv, IND_CMD_EV, priv->data_in.buf, priv->data_in.len);
					RESET_RX_STATE;
					return; /*the 2nd byte may be skip handling*/
				}
			}else{
				RESET_RX_STATE;
				priv->rx_flag=NRX_START;
				RX_MSG_PRT( "\n(rx_start)" );
			}
		}else{
			nfbi_handle_next_rx_char(priv, tmp);
			if(priv->rx_cmd_state!=STATE_RX_WAIT_DATA)
				priv->rx_cmd_state=STATE_RX_WAIT_DATA;
		}
	}

	return;
}


static unsigned char nfbi_get_next_tx_char(struct mdio_priv *priv)
{
	unsigned char tmp;
	unsigned char ret;

	if( (priv->tx_status_transmitting_len==0)&&(priv->tx_flag==0) )
	{
		ret=NFBI_PPP_FLAG;
		priv->tx_flag = NTX_START;
		TX_MSG_PRT( "\n(start)" );
	}else if(priv->tx_status_transmitting_len>=priv->data_out.len)
	{
		ret=NFBI_PPP_FLAG;
		priv->tx_flag = priv->tx_flag | NTX_END;
		TX_MSG_PRT( "(end)\n" );
	}else{
		tmp=priv->data_out.buf[priv->tx_status_transmitting_len];
		if( (tmp==NFBI_PPP_FLAG) || (tmp==NFBI_PPP_ESCAPE) )
		{
			if(priv->tx_flag&NTX_ESC)
			{
				ret=tmp^NFBI_PPP_TRANS;
				priv->tx_flag = priv->tx_flag & (~NTX_ESC);
				priv->tx_status_transmitting_len++;
				TX_MSG_PRT( "(U)" );
			}else{
				ret=NFBI_PPP_ESCAPE;
				priv->tx_flag = priv->tx_flag | NTX_ESC;
				TX_MSG_PRT( "(E)" );
			}
		}else{
			ret=tmp;
			priv->tx_status_transmitting_len++;
		}
	}

	return ret;
}


static void transmit_msg(struct mdio_priv *priv)
{
	unsigned short data;
	unsigned char *p=(unsigned char *)&data;

	if(priv->data_out.len<=0)
	{
		TX_MSG_PRT( "%s:%d> out.len<=0 return\n", __FUNCTION__, __LINE__ );
		return;
	}
	if(priv->tx_flag&NTX_END)
	{
		TX_MSG_PRT( "%s:%d> tx done return\n", __FUNCTION__, __LINE__ );
		return;
	}


	p[0]=nfbi_get_next_tx_char(priv);
	TX_MSG_PRT( "%02x ", p[0] );
	p[1]=nfbi_get_next_tx_char(priv);	
	TX_MSG_PRT( "%02x ", p[1] );
	register_write_dw(REG_SSR, (unsigned long)data);


	if(priv->tx_flag&NTX_END)
	{
		priv->tx_status_state = STATE_TX_INIT;
	}else{
		priv->tx_status_state = STATE_TX_IN_PROGRESS;
	}
}

//static void mdio_interrupt(int irq, void *dev_id, struct pt_regs *regs)
static irqreturn_t mdio_interrupt(int irq, void *dev_id)
{
	struct mdio_priv *priv = (struct mdio_priv *)dev_id;
	unsigned long status, data;

	while (1) 
	{
		status = register_read_dw(REG_ISR);
		if (!status) break;
		register_write_dw(REG_ISR, status);	// clear interrupt
		//printk("%s: status=%x\n", __FUNCTION__, status);
		if (status & IP_NEWMSG) 
		{
			data = register_read_dw(REG_RCR);			
			//printk( "event=IP_NEWMSG, REG_RCR=0x%08x\n", data );
			process_rx_cmd(priv, (unsigned short)data);					
		}
		if (status & IP_MSGFETCH) 
		{
			//printk( "event=IP_MSGFETCH\n" );
			transmit_msg(priv);	
		}
		if (status & NEEDED_IRQ_MASK_NO_MSG) 
		{
			unsigned long data_tmp;
			if( status&IP_ENUTPPHY )
			{
				//data_tmp = register_read_dw(REG_SYSCR);
				//printk( "event=IP_ENUTPPHY, REG_SYSCR=0x%08x\n", data_tmp );
			}

			if( status&IP_UACR_CHANGE)
			{
				//data_tmp = register_read_dw(REG_UACR);
				//printk( "event=IP_UACR_CHANGE, REG_UACR=0x%08x\n", data_tmp );
			}

			if( status&IP_FDCR_CHANGE)
			{
				data_tmp = register_read_dw(REG_FDCR);
				//printk( "event=IP_FDCR_CHANGE, REG_FDCR=0x%08x\n", data_tmp );
				REG32(0xB8A8582c)=data_tmp;
				//printk( "new 0xB8A8582c=0x%08x\n", REG32(0xB8A8582c) );
			}

			if( status&IP_SDCR_CHANGE)
			{
				data_tmp = register_read_dw(REG_SDCR);
				//printk( "event=IP_SDCR_CHANGE, REG_SDCR=0x%08x\n", data_tmp );					
				REG32(0xB8A85830)=data_tmp;
				//printk( "new 0xB8A85830=0x%08x\n", REG32(0xB8A85830) );
			}

			if( status&IP_KernelcodeReady )
			{
				//data_tmp = register_read_dw(REG_SYSCR);
				//printk( "event=IP_KernelcodeReady, REG_SYSCR=0x%08x\n", data_tmp );
			}

#if 0
			data = register_read_dw(REG_SYSCR);
			data &= NEEDED_IRQ_MASK_NO_MSG;
			indicate_evt(priv, IND_SYSCTL_EV, (unsigned char *)&data, sizeof(data));
#endif
		}
		if (status  & ~(NEEDED_IRQ_MASK))
		{
			DEBUGK_ERR("Got satus=0x%x, not supported yet!\n", (unsigned int)status);
		}
	}

	return IRQ_HANDLED;	
}


/*called by AdslLinkUp(), AdslLinkDown(), and mdio_private_command()*/
void mdio_set_dsllink_bit(int val)
{
    unsigned long flags;
    
    if (dev_priv == NULL)
        return;
    spin_lock_irqsave(&dev_priv->reglock, flags);
	if (val)
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_DSLLink;
	else
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_DSLLink);
	spin_unlock_irqrestore(&dev_priv->reglock, flags);
}


//fixme, test only
enum UTPRegNameTest
{
	URT_UACR=0,
	URT_FDCR,
	URT_SDCR,
	URT_NUM
};
unsigned int UTPRegTest[URT_NUM]={0};

static void mdio_reg_poll_timer(unsigned long task_priv)
{
	struct mdio_priv *priv = (struct mdio_priv *)task_priv;


    if (!priv->poll_timer_up)
        return;

	if (priv->reg_UACR_write != REG32(REG_UACR))
	{
		//printk("1. priv->reg_UACR_write=%x\n", priv->reg_UACR_write);
		priv->reg_UACR_write = REG32(REG_UACR);
		UTPRegTest[URT_UACR] = priv->reg_UACR_write;
		//printk("2. priv->reg_UACR_write=%x\n", priv->reg_UACR_write);
	}

	if (priv->reg_FDCR_write != REG32(REG_FDCR)) 
	{
		//printk("1. priv->reg_FDCR_write=%x\n", priv->reg_FDCR_write);
		priv->reg_FDCR_write = REG32(REG_FDCR);
		//printk("2. priv->reg_FDCR_write=%x\n", priv->reg_FDCR_write);
		UTPRegTest[URT_FDCR] = priv->reg_FDCR_write;
	}
	
	if (priv->reg_SDCR_write != REG32(REG_SDCR)) 
	{
		//printk("1. priv->reg_SDCR_write=%x\n", priv->reg_SDCR_write);
		priv->reg_SDCR_write = REG32(REG_SDCR);
		//printk("2. priv->reg_SDCR_write=%x\n", priv->reg_SDCR_write);
		UTPRegTest[URT_SDCR] = priv->reg_SDCR_write;
	}

	mod_timer(&priv->reg_poll_timer, jiffies + priv->phy_reg_poll_time);

}

static int mdio_open(struct inode *inode, struct file *filp)
{
	filp->private_data = dev_priv;
	//MOD_INC_USE_COUNT;
	return 0;
}

static int mdio_close(struct inode *inode, struct file *filp)
{
	//MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t mdio_read (struct file *filp, char *buf, size_t count, loff_t *offset)
{
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;

	if (!buf) return 0;
	count = retrieve_evt(priv, buf);
	return count;
}

static ssize_t mdio_write (struct file *filp, const char *buf, size_t count, loff_t *offset)
{
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;

	if (!buf) 
	{
		DEBUGK_ERR("buf = NULL!\n");		
		goto ret;
	}

	if (count > MDIO_BUFSIZE )
	{
		DEBUGK_ERR("write length too big!\n");		
		count = -EFAULT;
		goto ret;
	}

	if (priv->tx_status_state != STATE_TX_INIT) 
	{
		//DEBUGK_ERR("Transmit status, but not in valid state [%d]. Reset state!\n", priv->tx_status_state);		
		priv->tx_status_state = STATE_TX_INIT;
	}		
	
	if (copy_from_user((void *)priv->data_out.buf, buf, count)) 
	{
		DEBUGK_ERR("copy_from_user() error!\n");		
		count = -EFAULT;
		goto ret;		
	}
	
#ifdef KDB_MSG
	debugk_out("write data", priv->data_out.buf, count);
#endif

	priv->data_out.buf[count]=append_checksum(priv->data_out.buf, count);
	priv->data_out.len = count + 1;
	if(mtx_msg_dbg)
		mdio_dump_data( "txdump", priv->data_out.buf, priv->data_out.len);
	
	priv->tx_status_transmitting_len = 0;
	priv->tx_status_state = STATE_TX_INIT;
	priv->tx_flag=0;
	transmit_msg(priv);
	
ret:
	return count;
}

static void dump_private_data(void)
{
	printk("cmd_timeout=%d\n", dev_priv->cmd_timeout);
	printk("poll_timer_up=%d\n", dev_priv->poll_timer_up);
	printk("phy_reg_poll_time=%d\n", dev_priv->phy_reg_poll_time);

	printk("\nevt_que_head=%d\n", dev_priv->evt_que_head);
	printk("evt_que_tail=%d\n", dev_priv->evt_que_tail);
	printk("evt_que count=%d\n", CIRC_CNT(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
	printk("evt_que space=%d\n", CIRC_SPACE(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
	
	printk("\ntx_status_state:");
	switch(dev_priv->tx_status_state) 
	{
    	  case STATE_TX_INIT:
    	  	printk("STATE_TX_INIT\n");
    	  	break;
    	  case STATE_TX_IN_PROGRESS:
    	  	printk("STATE_TX_IN_PROGRESS\n");
    	  	break;
	}
	printk("tx_status_transmitting_len=%d\n", dev_priv->tx_status_transmitting_len);
    
	printk("rx_cmd_state:");
	switch(dev_priv->rx_cmd_state) 
	{
          case STATE_RX_INIT:
    	  	printk("STATE_RX_INIT\n");
    	  	break;
    	  case STATE_RX_WAIT_DATA:
    	  	printk("STATE_RX_WAIT_DATA\n");
    	  	break;
    	  case STATE_RX_WAIT_DAEMON:
    	  	printk("STATE_RX_WAIT_DAEMON\n");
    	  	break;
	}
	printk("rx_cmd_time=%lx jiffies=%lx (%d sec)\n", dev_priv->rx_cmd_time, jiffies, (int)(jiffies-dev_priv->rx_cmd_time)/HZ);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	printk("\nhost_pid=%d\n", dev_priv->host_pid);
#else
	printk("\nhost_pid=0x%p\n", dev_priv->host_pid);
#endif	

	printk("\nreg_UACR_write=0x%04lx\n", dev_priv->reg_UACR_write);
	printk("reg_FDCR_write=0x%04lx\n", dev_priv->reg_FDCR_write);
	printk("reg_SDCR_write=0x%04lx\n", dev_priv->reg_SDCR_write);

}

void mdio_private_command(int type)
{
    switch(type) {
      case 0: //stop reg poll timer
        dev_priv->poll_timer_up = 0;
        break;
      case 1: //start reg poll timer
        dev_priv->poll_timer_up = 1;
        mod_timer(&dev_priv->reg_poll_timer, jiffies + dev_priv->phy_reg_poll_time);
        break;
      case 2:
        dump_private_data();
        break;
      case 3: //dsl link up
        mdio_set_dsllink_bit(1);
        break;
      case 4: //dsl link down
        mdio_set_dsllink_bit(0);
        break;
    }
}

static int mdio_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    unsigned long flags;
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;
	int val, retval = 0;
	unsigned char bval;
	struct mdio_mem32_param mem_param;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != MDIO_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > MDIO_IOCTL_MAXNR) return -ENOTTY;

	switch (cmd) 
	{
	case MDIO_IOCTL_PRIV_CMD:
		retval = get_user(val, (int *)arg);
		if (retval == 0) 
		{
			mdio_private_command(val);
		}
		break;
	case MDIO_IOCTL_SET_HOST_PID:
		retval = copy_from_user((void *)&val, (void *)arg, 4);
		if (retval == 0) 
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
			priv->host_pid = val;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
			priv->host_pid = find_pid_ns(val,0);
#else
			// TODO : Check in stable 2.6.27
			priv->host_pid = task_pid(find_task_by_vpid(val));
			//priv->host_pid = task_pid(current);
#endif
			DEBUGK_OUT("set pid=%d\n", val);
			//printk("set pid=%d priv->host_pid=%p\n", val, priv->host_pid);
		}
		spin_lock_irqsave(&priv->reglock, flags);
		REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_AllSoftwareReady;
		spin_unlock_irqrestore(&priv->reglock, flags);
		break;			
	case MDIO_IOCTL_SET_CMD_TIMEOUT:
		retval = copy_from_user((void *)&bval, (void *)arg, 1);
		if (retval == 0) 
		{
			priv->cmd_timeout = (int)bval;				
			DEBUGK_OUT("set cmd_timeout=%d\n", priv->cmd_timeout); 
		}
		break;			
	case MDIO_IOCTL_SET_PHY_POLL_TIME:
		retval = copy_from_user( (void *)&bval, (void *)arg, 1);
		if (retval == 0) 
		{
			priv->phy_reg_poll_time = (int)bval;
			mod_timer(&priv->reg_poll_timer, jiffies + priv->phy_reg_poll_time);
			DEBUGK_OUT("set poll_time=%d\n", priv->phy_reg_poll_time); 
		}
		break;			
	case MDIO_IOCTL_READ_MEM:
		retval = copy_from_user( (void *)&mem_param.addr, (void *)arg, 4);
		if (retval == 0) 
		{
			mem_param.val =	READ_MEM32(mem_param.addr);				
			retval = copy_to_user((void *)arg, (void *)&mem_param.val, 4);
			DEBUGK_OUT("read_mem: addr=0x%x, data=0x%x\n", (int)mem_param.addr, (int)mem_param.val); 
		}
		break;
	case MDIO_IOCTL_WRITE_MEM:
		retval = copy_from_user((void *)&mem_param, (void *)arg, sizeof(mem_param));
		if (retval == 0) 
		{
			WRITE_MEM32(mem_param.addr, mem_param.val);
			DEBUGK_OUT("write_mem: addr=0x%x, data=0x%x\n", (int)mem_param.addr, (int)mem_param.val);
		}			
		break;
	case MDIO_IOCTL_READ_SCR:
		val = register_read_dw(REG_SYSCR);			
		retval = copy_to_user((void *)arg, (void *)&val, sizeof(val));			
		DEBUGK_OUT("read_src src=0x%x\n", val); 
		break;
	default:  /* redundant, as cmd was checked against MAXNR */
		DEBUGK_ERR("Invalid ioctl cmd [0x%x]!\n", cmd);
		return -ENOTTY;
	}
	
	return retval;
}


static struct file_operations mdio_fops = {
		read:		mdio_read, 
		write:		mdio_write,
		ioctl:		mdio_ioctl,		
		open:		mdio_open,
		release:	mdio_close,
};

static int __init mdio_init(void)
{
    struct mdio_priv *priv;	

#ifndef CONFIG_RTL8672_PTM
	//refer to ptm.c
	//Enable SACHEM module only (no PTM)
	REG32(BSP_IP_SEL) = REG32(BSP_IP_SEL) | BSP_EN_SACHEM;
	printk( "Enable SACHEM module 0x%08x\n", REG32(BSP_IP_SEL) );
#endif /*CONFIG_RTL8672_PTM*/

	printk(KERN_INFO DRIVER_NAME" driver "DRIVER_VER" at %x (Interrupt %d)\n", NFBI_BASE, BSP_NFBI_IRQ);
	priv  = (struct mdio_priv *)kmalloc(sizeof (struct mdio_priv), GFP_KERNEL);
	if(!priv) return -ENOMEM;
	memset((void *)priv, 0, sizeof (struct mdio_priv));
	priv->reglock = SPIN_LOCK_UNLOCKED;
	
    //force the poll time to update the register
	priv->reg_UACR_write = 0xf0000;
	priv->reg_FDCR_write = 0xf0000;
	priv->reg_SDCR_write = 0xf0000;
	init_timer(&priv->reg_poll_timer);
	priv->reg_poll_timer.data = (unsigned long)priv;
	priv->reg_poll_timer.function = mdio_reg_poll_timer;
	priv->poll_timer_up = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	priv->host_pid = -1;
#else
	priv->host_pid = NULL;
#endif
	dev_priv = priv;

	if (register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &mdio_fops))
	{
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": unable to get major %d\n", DRIVER_MAJOR);
		kfree(dev_priv);	
		dev_priv = NULL;
		return -EIO;
	}

	REG32(REG_IMR) = NEEDED_IRQ_MASK;
	if (request_irq(BSP_NFBI_IRQ, mdio_interrupt, IRQF_DISABLED, DRIVER_NAME, (void *)priv)) 
	{
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": IRQ %d is not free.\n", BSP_NFBI_IRQ);
		unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
		kfree(dev_priv);	
		dev_priv = NULL;
		return -1;
	}

	mdio_create_proc();
	return 0;
}

static void __exit mdio_exit(void)
{
	mdio_remove_proc();
	REG32(REG_IMR) = 0;
	free_irq(BSP_NFBI_IRQ, dev_priv);
	del_timer_sync(&dev_priv->reg_poll_timer);
	unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
	kfree(dev_priv);	
	dev_priv = NULL;
}


/*================================================================*/
module_init(mdio_init);
module_exit(mdio_exit);

MODULE_DESCRIPTION("Driver for RTL867x MDC/MDIO");
MODULE_LICENSE("none-GPL");

