#if __KERNEL__
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/smp_lock.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#else
#include<stdio.h>
#include<string.h>
#endif
#include "pcsc_if.h"
#include "pcsc_io.h"
#include "pcsc_ioctl.h"
#include "pcsc_atr.h"
#include "pcsc_reg.h"
#include "pcsc_fs.h"
#include "pcsc_util.h"
#include "pcsc_timer.h"
#include "common/rt_type.h"
#include "common/type.h"
#include "common/error.h"
#include "rtk/intr.h"
#include "rtk/irq.h"

static struct class *rtksc_class;
static char *version = "Realtek SmartCard Reader v1.0";
static char gStatus = 0;
static int datasession = 0;

#ifdef __KERNEL__
#define SCIF_PRINT printk
#else
#define SCIF_PRINT printf
#endif

#define SC_DEV_MAJOR	240
#define CCID_DRIVER_BULK_DEFAULT_TIMEOUT        (150*HZ)
#define CCID_DRIVER_ASYNC_POWERUP_TIMEOUT       (35*HZ)
#define CCID_DRIVER_MINIMUM_TIMEOUT             (3*HZ)
#define READ_WRITE_BUFFER_SIZE 512
#define POLL_LOOP_COUNT                         1000
#define POLL_PERIOD                             msecs_to_jiffies(10)

#if 0
#define MY_MACIG 'G'
#define SC_IOCTL_READ	_IOR(MY_MACIG, 0, int)
#define SC_IOCTL_WRITE	_IOW(MY_MACIG, 1, int)
#define SC_IOCTL_GET_IMEI	_IOR(MY_MACIG, 2, int)
#define SC_IOCTL_GET_SN	_IOR(MY_MACIG, 3, int)
#define SC_IOCTL_WRITE	_IOW(MY_MACIG, 1, sizeof(T0cmd))
#endif

SCStruct *pSC;
struct tasklet_struct atr_handler_tasklet;

void setSystemStatus(char status){
	gStatus = status;
}

char getSystemStatus(void){
	return gStatus;
}

char checkCardIsPlugged(void){
	return pSC->isInserted;
}

void addSession(void){
	datasession ++;
	gStatus = 0;
}

void delSession(void){
	datasession --;
}

char getSession(void){
	return datasession;
}

static ssize_t rtksc_read(struct file *filp, char __user *buf,
                        size_t count, loff_t *ppos){
	unsigned char *databuf;
	
	if (down_interruptible(&pSC->sem))
			return -ERESTARTSYS;
	
	databuf = kzalloc(count, GFP_KERNEL);
	
	getBufData(databuf, count);
	copy_to_user(buf , databuf , count);

	kfree(databuf);
	
	up(&pSC->sem);
	return count;
}

static ssize_t rtksc_write(struct file *filp, const char __user *buf,
                         size_t count, loff_t *ppos){
//	int i = 0;
	unsigned char *databuf;
	T0cmd t0cmd;

	if (down_interruptible(&pSC->sem))
			return -ERESTARTSYS;

	databuf = kzalloc(count, GFP_KERNEL);
	
	copy_from_user(databuf, buf, count);
	
	delSession();
	
	memcpy(&t0cmd, &buf[0], sizeof(t0cmd));
	
	//printk("sizeof(T0cmd) = %d\n", sizeof(T0cmd));
#if 0	
	for ( i = 0 ; i < count ; i ++ ){
		printk("%02x", databuf[i]);
	}
	printk("\n");
#endif	
	if ( count > sizeof(T0cmd) ){
		
		setBufData(&databuf[0], count-sizeof(t0cmd) );
	}
	
	if ( t0cmd.Ins == SELECT )
		T0selectFS(&t0cmd);
	else if ( t0cmd.Ins == STATUS )
		T0getStatus(&t0cmd);
	else if ( t0cmd.Ins == GETRESPONSE )
		T0getResponse(&t0cmd);
	else if ( t0cmd.Ins == READRECORD || t0cmd.Ins == READBINARY )
		T0read(&t0cmd);
	else if ( t0cmd.Ins == UPDATERECORD || t0cmd.Ins == UPDATEBINARY)
		T0write(&t0cmd);	
	else if ( t0cmd.Ins == SEEK )
		T0read(&t0cmd);
	else if ( t0cmd.Ins == INCREASE )
		T0write(&t0cmd);
	else if ( t0cmd.Ins == VERIFYCHV )
		T0write(&t0cmd);
	else if ( t0cmd.Ins == CHANGECHV )
		T0write(&t0cmd);
	else if ( t0cmd.Ins == ENABLECHV )
		T0write(&t0cmd);			
	else if ( t0cmd.Ins == DISABLECHV )	
		T0write(&t0cmd);
	else if ( t0cmd.Ins == UNBLOCKCHV )	
		T0write(&t0cmd);
	else if ( t0cmd.Ins == INVALIDATE )	
		T0read(&t0cmd);
	else if ( t0cmd.Ins == REHABILITATE )
		T0read(&t0cmd);
	else if ( t0cmd.Ins == RUNGSMALORITHM )	
		T0write(&t0cmd);
	else if ( t0cmd.Ins == SLEEP )
		T0read(&t0cmd);
	else if ( t0cmd.Ins == TERMINALPROFILE )
		T0write(&t0cmd);
	else if ( t0cmd.Ins == ENVELOPE )
		T0write(&t0cmd);
	else if ( t0cmd.Ins == FETCH )
		T0read(&t0cmd);
	else if ( t0cmd.Ins == TERMINALRESPONSE )
		T0write(&t0cmd);
		
	kfree(databuf);
	
	up(&pSC->sem);
	return count;
}

static unsigned int rtksc_poll(struct file *filp, poll_table *wait){
	unsigned int mask = 0;
	printk("rtksc rtksc_poll\n");
    return mask;
}

static int rtksc_open(struct inode *inode, struct file *filp){
	int ret = 0;
	//printk("rtksc open\n");
	lock_kernel();	
	if ( pSC->open ){
		ret = -EBUSY;
		goto out;		
	}
	pSC->open = 1;
out:
	unlock_kernel();	
	return ret;
}

static int rtksc_close(struct inode *inode, struct file *filp){
	//printk("rtksc close\n");
	pSC->open = 0;
	return 0;
}

int rtksc_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg) {
	char buf[200];
	int len = 200;
	char data = 0x0;
	T0cmd t0cmd;
	
	switch(cmd) {
	case SC_IOCTL_GETSW:
		//printk("READ_IOCTL\n");
		memset(buf, 0x0, 200);
		getSW(&buf[0]);
		copy_to_user((char *)arg, buf, 2);
		break;
	
	case SC_IOCTL_WRITE:
		printk("WRITE_IOCTL\n");
		copy_from_user(buf, (char *)arg, len);
		break;
	
	case SC_IOCTL_SETCMD:
		//printk("SET CMD\n");
		copy_from_user(&t0cmd, (T0cmd *)arg, sizeof(T0cmd));
		if ( t0cmd.Ins == SELECT )
			T0selectFS(&t0cmd);
		else if ( t0cmd.Ins == STATUS )
			T0getStatus(&t0cmd);
		else if ( t0cmd.Ins == GETRESPONSE )
			T0getResponse(&t0cmd);
		else if ( t0cmd.Ins == READRECORD || t0cmd.Ins == READBINARY )
			T0read(&t0cmd);
		else if ( t0cmd.Ins == UPDATERECORD || t0cmd.Ins == UPDATEBINARY)
			T0write(&t0cmd);	
		else if ( t0cmd.Ins == SEEK )
			T0read(&t0cmd);
		else if ( t0cmd.Ins == INCREASE )
			T0write(&t0cmd);
		else if ( t0cmd.Ins == VERIFYCHV )
			T0write(&t0cmd);
		else if ( t0cmd.Ins == CHANGECHV )
			T0write(&t0cmd);
		else if ( t0cmd.Ins == ENABLECHV )
			T0write(&t0cmd);			
		else if ( t0cmd.Ins == DISABLECHV )	
			T0write(&t0cmd);
		else if ( t0cmd.Ins == UNBLOCKCHV )	
			T0write(&t0cmd);
		else if ( t0cmd.Ins == INVALIDATE )	
			T0read(&t0cmd);
		else if ( t0cmd.Ins == REHABILITATE )
			T0read(&t0cmd);
		else if ( t0cmd.Ins == RUNGSMALORITHM )	
			T0write(&t0cmd);
		else if ( t0cmd.Ins == SLEEP )
			T0read(&t0cmd);
		else if ( t0cmd.Ins == TERMINALPROFILE )
			T0write(&t0cmd);
		else if ( t0cmd.Ins == ENVELOPE )
			T0write(&t0cmd);
		else if ( t0cmd.Ins == FETCH )
			T0read(&t0cmd);
		else if ( t0cmd.Ins == TERMINALRESPONSE )
			T0write(&t0cmd);
		
		break;
		
	case SC_IOCTL_CHECKCARD:
		printk("Card%d\n", pSC->isInserted);
		copy_to_user((char *)arg, &pSC->isInserted, 1);
		break;
		
	case SC_IOCTL_GETSYSTEMSTATUS:
		data = getSystemStatus();
		//printk("get system status : 0x%02x\n", data);		
		copy_to_user((char *)arg, &data, 1);
		break;
		
	case SC_IOCTL_CHECKATRPPS:
		if ( pSC->getATRPPS != 0x3 )
			data = 0x0;
		else
			data = 0x1;
		copy_to_user((char *)arg, &data, 1);
		break;
	
	case SC_IOCTL_DOWARMRESET:
		resetETU();
		sc_write(ICC_TRANSFER, START_TRANSFER);	
		break;
		
	default:
		printk("ENOTTY\n");
		return -ENOTTY;
	}
	return len;
}

static void getData(int len){
	unsigned int data = 0x0;
	int i ;
	for ( i = 0 ; i < len ; i ++ ){
		data = sc_read(0xee00 + i);
		SCIF_PRINT("%02x", data);
	}
	SCIF_PRINT("\n");
}

//int sc_read(uint32_t  address);
//int sc_write(uint32_t  address, uint32_t  value_ptr);
static void rtksc_hwinit(void){
	unsigned int data = 0x0;
	
	printk("start smart-card hw init\n");

	REG32(IO_MODE_EN) |= SC_CD_EN;
	printk("0x%x : 0x%x\n", IO_MODE_EN, REG32(IO_MODE_EN));

	sc_write(ICC_LDO_1, 0xfa);

	data = sc_read(ICC_LDO_1);
	printk("ICC_LDO_1 [0x%x]: 0x%02x\n", ICC_LDO_1, data);

	// clear interrupt bit	
	sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_EN|CARD_CD_INTERRUPT_FLAG);
	
	data = sc_read(ICC_SC_CD_INT);
	printk("ICC_SC_CD_INT [0x%x]: 0x%02x\n", ICC_SC_CD_INT, data);	

	// set interrupt enable
	sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_EN);
	
	data = sc_read(ICC_SC_CD_INT);
	printk("ICC_SC_CD_INT [0x%x]: 0x%02x\n", ICC_SC_CD_INT, data);
	
	// set pad pin
	sc_write(ICC_SC_PAD_PULLCTL, 0x2e);

	data = sc_read(ICC_SC_PAD_PULLCTL);
	printk("ICC_SC_PAD_PULLCTL [0x%x]: 0x%02x\n", ICC_SC_PAD_PULLCTL, data);

	// set output enable
	sc_write(ICC_PAD_OE, SC_PAD_OE);

	data = sc_read(ICC_PAD_OE);
	printk("ICC_PAD_OE [0x%x]: 0x%02x\n", ICC_PAD_OE, data);

	// set asynchronous
	sc_write(ICC_SC_SEL, ASYNCHRONOUS_SC);

	data = sc_read(ICC_SC_SEL);
	printk("ICC_SC_SEL [0x%x]: 0x%02x\n", ICC_SC_SEL, data);

	// set power control
	sc_write(ICC_PWR_CTL, POWER_CTL_ON | CLASSB_3);

	data = sc_read(ICC_PWR_CTL);
	printk("ICC_PWR_CTL [0x%x]: 0x%02x\n", ICC_PWR_CTL, data);

	// set clock control
	sc_write(ICC_CLK_CTL, CLOCK_FREQ_3P75 | CLOCK_CTL_EN);
	data = sc_read(ICC_CLK_CTL);
	printk("ICC_CLK_CTL [0x%x]: 0x%02x\n", ICC_CLK_CTL, data);

	// mask all error status
	sc_write(ICC_INTR_MASK, 0xFF);
	data = sc_read(ICC_INTR_MASK);	
	printk("ICC_INTR_MASK [0x%x]: 0x%02x\n", ICC_INTR_MASK, data);

	// enable all status interrupt except for CD
	sc_write(ICC_SC_MASK, ICC_END_MSK|ICC_TOTAL_MSK);
	data = sc_read(ICC_SC_MASK);	
	printk("ICC_SC_MASK [0x%x]: 0x%02x\n", ICC_SC_MASK, data);	
	
	// start transfer to get ATR
	sc_write(ICC_TRANSFER, START_TRANSFER);
	data = sc_read(ICC_TRANSFER);
	printk("ICC_TRANSFER [0x%x]: 0x%02x\n", ICC_TRANSFER, data);

#if 0	
	data = sc_read(ICC_SC_MASK);
	data = data & ~ICC_END_STATUS;
	//SCIF_PRINT("1 : ICC_SC_MASK : 0x%02x\n", data);
	sc_write(ICC_SC_MASK, data);
	sc_write(ICC_STATUS2, 0x0);
	sc_write(ICC_TIME_CTL_FLAG, 0x0);
	//sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_FLAG|sc_read(ICC_SC_CD_INT));	
#endif	
	data = sc_read(ICC_SC_CD);
	SCIF_PRINT("ICC_SC_CD, 0x%x\n", data);
//	if ( data & CARD_INSERT ){
	if ( ! (data & CARD_INSERT) ){		
		getData(10);
		//tasklet_hi_schedule(&atr_handler_tasklet);
		//SCIF_PRINT("reset ETU and get ATR\n");
		//resetETU();
		//sc_write(ICC_TRANSFER, START_TRANSFER);		
	}

}

static int rtksc_swinit(void){
	//memset(gSC, 0x0, sizeof(SCStruct));
	pSC = kzalloc(sizeof(SCStruct), GFP_KERNEL);
	if ( !pSC ){
		printk("[%s] kzalloc pSC fail [%p]\n", __FUNCTION__, pSC);
		return -1;	
	}
	pSC->gATR = kzalloc(sizeof(ATRStruct), GFP_KERNEL);
	if ( ! pSC->gATR ){
		printk("[%s] kzalloc gATR fail [%p]\n", __FUNCTION__, pSC->gATR);
		return -1;	
	}
	
	return 0;
}

static void rtksc_swuninit(void){
	kfree(pSC->gATR);
	kfree(pSC);
}

int checkErrorStatus(void){
	unsigned int data = 0x0;
	unsigned char err = 0;
	int status = 0, status2 = 0;

	//printk("check error status, 0xfd91 : [0x%x], 0xfd92 : [0x%x], 0xfd93 : [0x%x]\n", sc_read(ICC_STATUS2), sc_read(ICC_TIME_CTL_FLAG), sc_read(ICC_TRANSFER));
	data = sc_read(ICC_STATUS2);
	if ( data & PROCEDURE_ERR ){
		SCIF_PRINT("ICC_STATUS2, 0x%x\n", data);
		SCIF_PRINT("PROCEDURE_ERR\n");
		status |= SC_PROCEDURE_ERR;
	}
	
	if ( data & ICC_TIMEOUT_FLAG ){
		//SCIF_PRINT("ICC_STATUS2, 0x%x\n", data);
		//SCIF_PRINT("ICC_TIMEOUT_FLAG\n");
		status |= SC_ICC_TIMEOUT_FLAG;
		err = 1;
		data = sc_read(ICC_TIME_CTL_FLAG);
		if ( data & ICC_RESET_ATR_DURATION_TIMEOUT ){
			SCIF_PRINT("ICC_RESET_ATR_DURATION_TIMEOUT\n");
		}
		if ( data & ICC_RESET_RECEIVE_TIMEOUT ){
			SCIF_PRINT("ICC_RESET_RECEIVE_TIMEOUT\n");
		}
		if ( data & ICC_T1_BWT_TIMEOUT_FLAG ){
			SCIF_PRINT("ICC_T1_BWT_TIMEOUT_FLAG\n");
		}
		if ( data & ICC_T1_CWT_TIMEOUT_FLAG ){
			SCIF_PRINT("ICC_T1_CWT_TIMEOUT_FLAG\n");
		}
		if ( data & ICC_T0_WWT_TIMEOUT_FLAG ){
			SCIF_PRINT("ICC_T0_WWT_TIMEOUT_FLAG\n");			
		}
		status2 = data;
	}
	
	data = sc_read(ICC_STATUS2);
	//SCIF_PRINT("ICC_STATUS2, 0x%x\n", data);
	if ( data & CRC_ERR ){
		err = 1;
		SCIF_PRINT("CRC_ERR\n");
		status |= SC_CRC_ERR;
	}
	if ( data & LRC_ERR ){
		err = 1;
		SCIF_PRINT("LRC_ERR\n");
		status |= SC_LRC_ERR;
	}
	if ( data & TCK_ATRLEN_ERR ){
		err = 1;
		SCIF_PRINT("TCK_ATRLEN_ERR\n");
		status |= SC_TCK_ATRLEN_ERR;
	}
	if ( data & PCK_TS_ERR ){
		err = 1;
		SCIF_PRINT("PCK_TS_ERR\n");
		status |= SC_PCK_TS_ERR;
	}
	if ( data & PARITY_ERR ){
		err = 1;
		SCIF_PRINT("PARITY_ERR\n");
		status |= SC_PARITY_ERR;
	}	

	data = sc_read(ICC_TRANSFER);
	//SCIF_PRINT("ICC_TRANSFER, 0x%x\n", data);

	return status;
}

int checkTransStatus(void){
	unsigned int data = 0x0, count = 0;
	data = sc_read(ICC_TRANSFER);
	while ( ! (data & END_TRNASFER)){
//		if ( data & END_TRNASFER )
//			break;
		count ++;
		if ( count > 50000 )
			return 0;
	}

	printk("checkTransStatus == count %d\n", count);
	if ( count > 50000 )
		return 0;
	else
		return 1;
}

void ATRPPS_handler(unsigned long dummy){
	unsigned char ppslen = 0;
	unsigned int atrlen = 0;
	int ret = 0;
	ATRStruct *gATR = &pSC->gATR;
	unsigned int data = 0x0;

	if ( (ret = checkTransStatus()) == 0 ){
		checkErrorStatus();
		return;
	}

	data = sc_read(ICC_TRANSFER);
	printk("ICC_TRANSFER 0xfd73 : 0x%x\n", data);

	
	if ( !(pSC->getATRPPS & 0x1) ){
		
		SCIF_PRINT("enter ATR/PPS tasklet\n");

		// Get ATR length
		atrlen = sc_read(ICC_ATR_LEN);
		if ( atrlen == 0 ){
			SCIF_PRINT("ATR length is invalid, %d\n", atrlen);
			checkErrorStatus();
			return;		
		}

		if ( (ret = analyzeATR(atrlen, gATR)) == -1){
			SCIF_PRINT("Analyse ATR fail\n");
			checkErrorStatus();
			return;
		}
		
		genPPS();
		pSC->getATRPPS |= 0x1;
	}else if ( !(pSC->getATRPPS & 0x2) ){
		// check PPS response
		SCIF_PRINT("Get PPS response\n");
		
		ppslen = sc_read(ICC_STATUS1);
		if ( ppslen == 0 ){
			SCIF_PRINT("PPS response length is invalid, %d\n", ppslen);
			checkErrorStatus();
			return;		
		}	
			
		if ( (ret == analyzePPS(ppslen)) == -1){
			SCIF_PRINT("Analyze PPS fail\n");
			checkErrorStatus();
			return;
		}
		
		pSC->getATRPPS |= 0x2;
	}
}

/*every 10ms to check interrupt pin GPIO23
  interrupt pin is low active
 */
//static void checkInterrupt(void){
//static irq_handler_t checkInterrupt(int irq, void *dev_instance){
void checkInterrupt(void){

	unsigned int data = 0x0, count = 0, ret = 0;
	unsigned char err = 0, getatrpps = 0;
	int status = 0, status2 = 0;
	//#define GPIO_STATUS	( ( volatile unsigned int * )0xbb0000e4)
	//#define INTRPIN	(1<<23)	//GPIO23
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    if((ret = rtk_intr_imr_set(INTR_TYPE_SMARTCARD, DISABLED)) != RT_ERR_OK)
    {
    	printk("imr set fail, ret %d\n", ret);
		goto INTR_END; 
    }	

	//if( !(*GPIO_STATUS & INTRPIN) ){	// interrupt occur
		//SCIF_PRINT("Interrupt occur, 0x%x\n", *GPIO_STATUS);
#if 0
	if ( ! pSC->isInited ){
		data = sc_read(ICC_SC_CD);
		if ( !(data & CARD_INSERT ) ){
			pSC->isInserted = 1;
			resetETU();
			sc_write(ICC_TRANSFER, START_TRANSFER);
		}
	}else
#endif	
	{

		data = sc_read(ICC_SC_CD_INT);
		if ( data & CARD_CD_INTERRUPT_FLAG ){ // card insert
			SCIF_PRINT("ICC_SC_CD_INT, 0x%x\n", data);
			data = sc_read(ICC_SC_CD);
	//			if ( data & CARD_INSERT ){
			if ( ! (data & CARD_INSERT) ){				
				SCIF_PRINT("ICC_SC_CD, 0x%x\n", data);
				pSC->isInserted = 1;	
				if ( !(pSC->getATRPPS & 0x1) && pSC->isInserted ){ // not get ATR
					SCIF_PRINT("reset ETU and get ATR\n");
					resetETU();
					sc_write(ICC_TRANSFER, START_TRANSFER);
					getatrpps = 1;
				}
			}else{
				pSC->getATRPPS = 0;
				pSC->isInserted = 0;
			}
		}else{	// not insert card interrupt
			data = sc_read(ICC_SC_CD);
	//			if ( ! (data & CARD_INSERT) ){
			if ( (!pSC->isInited) && !(data & CARD_INSERT) ){ // first boot and card is already in the slot
				resetETU();
				sc_write(ICC_TRANSFER, START_TRANSFER);
				pSC->isInserted = 1;
			}else if ( data & CARD_INSERT ){				
				printk("[%s][%d]\n", __FUNCTION__, __LINE__);
				pSC->isInserted = 0;
				pSC->getATRPPS = 0;
			}
		}
	}

	if ( !((pSC->getATRPPS & 0x2)>>1) && pSC->isInserted ) // not get PPS
		getatrpps = 1;
	//else
		//SCIF_PRINT("pSC->getATRPPS0x%02x, pSC->isInserted0x%02x\n", (pSC->getATRPPS & 0x2)>>1, pSC->isInserted);		
	
	if ( ! pSC->isInserted )
		goto INTR_END;
	
	//SCIF_PRINT("0 : ICC_SC_MASK 0x%02x\n", sc_read(ICC_SC_MASK));
	while( !(sc_read(ICC_SC_MASK) & ICC_END_STATUS) && count < 10000 ){
		//SCIF_PRINT("ICC_SC_MASK 0x%02x\n", sc_read(ICC_SC_MASK));
		count ++;
	};
	//SCIF_PRINT("1 : ICC_SC_MASK 0x%02x\n", sc_read(ICC_SC_MASK));
		
	status = checkErrorStatus();
	
	if ( getatrpps && !status ){
		tasklet_hi_schedule(&atr_handler_tasklet);
		//getData(10);
	}else if ( pSC->getATRPPS == 3 && !status ){
		if (getSession() == 0){
			//SCIF_PRINT("setTrasnferFinish\n");
			status |= SC_DATAFINISH;
			setSystemStatus(status);
		}else{
			SCIF_PRINT("\n\n other case...\n\n");
		}
	}

	if ( ! pSC->isInited ){
		tasklet_hi_schedule(&atr_handler_tasklet);
		pSC->isInited = 1;
	}

INTR_END:		
	data = sc_read(ICC_SC_MASK);
	data = data & ~ICC_END_STATUS;
	//SCIF_PRINT("1 : ICC_SC_MASK : 0x%02x\n", data);
	sc_write(ICC_SC_MASK, data);
	sc_write(ICC_STATUS2, 0x0);
	sc_write(ICC_TIME_CTL_FLAG, 0x0);
	sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_FLAG|sc_read(ICC_SC_CD_INT));

    if((ret = rtk_intr_ims_clear(INTR_TYPE_SMARTCARD)) != RT_ERR_OK)
    {
    	printk("imr set fail, ret %d\n", ret);
    }		
    if((ret = rtk_intr_imr_set(INTR_TYPE_SMARTCARD, ENABLED)) != RT_ERR_OK)
    {
    	printk("imr set fail, ret %d\n", ret);
    }		
}

static const struct file_operations sc_fops = {
        .owner          = THIS_MODULE,
        .read           = rtksc_read,
        .write          = rtksc_write,
        .open           = rtksc_open,
        .release        = rtksc_close,
        .poll           = rtksc_poll,
		.ioctl			= rtksc_ioctl,
};

//static int __init rtksc_init(void)
int rtksc_init(void)
{
	int ret = 0;
	dev_t dev = 0;
	struct device *pdevice;
	
	printk(KERN_INFO "%s\n", version);
	
	if ( (ret = rtksc_swinit()) == -1 )	{
		printk("create smartcard structure fail\n");
		return -1;
	}	
	
	if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0){  //$cat /proc/devices
		printk(KERN_WARNING MODULE_NAME ": could not create alloc char region\n");		
		return -1;
	}
	
	pSC->major = MAJOR(dev);
	pSC->minor = MINOR(dev);	
	
	printk("smartcard: register chrdev(%d,%d)\n", pSC->major, pSC->minor);
	
	rtksc_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(rtksc_class)){    //$ls /sys/class
		printk(KERN_WARNING MODULE_NAME	": could not create class\n");		
		unregister_chrdev_region(dev, 1);
		return PTR_ERR(rtksc_class);;
	}
	
	pdevice = device_create(rtksc_class, NULL , MKDEV(pSC->major, pSC->minor) , NULL , DEVICE_NAME);
	
	if (IS_ERR(pdevice)){ //$ls /dev/
		class_destroy(rtksc_class);
		unregister_chrdev_region(dev, 1);
		return -1;
	}
	
	cdev_init(&pSC->cdev, &sc_fops);
	
	pSC->cdev.owner = THIS_MODULE;
	pSC->cdev.ops = &sc_fops;
	
	ret = cdev_add (&pSC->cdev, MKDEV(pSC->major, pSC->minor), 1);
	if (ret){
		device_destroy(rtksc_class, dev);
		class_destroy(rtksc_class);
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	//ret = request_irq(SIM_IRQ, checkInterrupt, IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER, 
	//DEVICE_NAME, pSC);// NULL OK	

	rtksc_hwinit();

	if((ret = rtk_irq_isr_register(INTR_TYPE_SMARTCARD, checkInterrupt)) != RT_ERR_OK)
	{
		printk("register irq fail, ret = %d\n", ret);
		return -1;
	}

    if((ret = rtk_intr_imr_set(INTR_TYPE_SMARTCARD, ENABLED)) != RT_ERR_OK)
    {
		printk("set imr fail, ret = %d\n", ret);    
		return -1;
    }
	
	init_MUTEX(&pSC->sem);
	
	tasklet_init(&atr_handler_tasklet, ATRPPS_handler, (unsigned long)NULL);
	
	return 0;
}

static void __exit rtksc_exit(void)
{
	printk(KERN_INFO MODULE_NAME ": unloading\n");
	tasklet_kill(&atr_handler_tasklet);
#if 0		
        unregister_chrdev(SC_DEV_MAJOR, DEVICE_NAME);
        class_destroy(rtksc_class);
#else
	/* cleanup_module is never called if registering failed */
	rtksc_swuninit();

	unregister_chrdev_region(MKDEV(pSC->major, pSC->minor), 1);

	device_destroy(rtksc_class, MKDEV(pSC->major, pSC->minor));
	class_destroy(rtksc_class);
	
#endif	
}


//module_init(rtksc_init);
//module_exit(rtksc_exit);
//late_initcall(rtksc_init);
__exitcall(rtksc_exit);

MODULE_LICENSE("GPL");

