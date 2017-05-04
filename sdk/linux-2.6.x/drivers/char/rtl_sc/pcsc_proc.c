#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>

#include "pcsc_if.h"
#include "pcsc_io.h"
#include "pcsc_reg.h"

#define PCSCNAME "pcsc01"
#define PCSCNAME2 "pcsc02"

#ifdef __KERNEL__
#define PCSCPROC_PRINT printk
#else
#define PCSCPROC_PRINT printf
#endif
#define FUNCLEN 32
#if 1
static void getData(int len);
static void pps_t0_tck(void);
static void record01(void);
static void record02(void);
static void readrecord01(void);
static void getResponse(void);
static void select6F3A(void);
static void select7F10(void);
static void select3F00(void);
static void rtksc_hwinit(void);
static void dumpreg(void);
static void enCHV(void);
static void selectMF(void);
#endif
void (*funcptr[FUNCLEN])() = {0, pps_t0_tck, record01, record02,
						readrecord01, getResponse, select6F3A, select7F10,
						select3F00, rtksc_hwinit, dumpreg, enCHV, selectMF};


// unsigned int sc_read(uint32_t  address);	
// int sc_write(uint32_t  address, uint32_t  value_ptr);
static void getData(int len){
	unsigned int data = 0x0;
	int i ;
	for ( i = 0 ; i < len ; i ++ ){
		data = sc_read(0xee00 + i);
		PCSCPROC_PRINT("%02x", data);
	}
	PCSCPROC_PRINT("\n");
}

static void pps_t0_tck(void){
	unsigned int data = 0x0;
	sc_write(0xee00, 0xff);
	sc_write(0xee01, 0x70);
	sc_write(0xee02, 0x96);
	sc_write(0xee03, 0x00);
	sc_write(0xee04, 0x00);
	sc_write(0xee05, 0x19);
	sc_write(ICC_CTL0, 0x00);
	sc_write(ICC_PPS_RQST_LEN, 0x06);
	sc_write(ICC_TRANSFER, 0x84);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_TIME_CTL_FLAG = %02x\n", data);
}

static void record01(void){
	unsigned int data = 0x0;
	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xf2);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x0d);
	
	sc_write(0xfd93, 0x82);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);	
}

static void record02(void){
	unsigned int data = 0x0;
	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xf2);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x23);
	
	sc_write(0xfd93, 0x82);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);	
}

static void readrecord01(void){
	unsigned int data = 0x0;
	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xB2);
	sc_write(0xfd87, 0x01);
	sc_write(0xfd88, 0x04);
	sc_write(0xfd89, 0x1E);
	
	sc_write(0xfd93, 0x82);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);	
}

static void getResponse(void){
	unsigned int data = 0x0;
	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xc0);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x0f);
	
	sc_write(0xfd93, 0x82);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);	
}

static void select6F3A(void){
	unsigned int data = 0x0;
	
	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xa4);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x02);
	sc_write(0xee00, 0x6F);
	sc_write(0xee01, 0x3A);
	sc_write(0xfd93, 0x81);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);		
}

static void select7F10(void){
	unsigned int data = 0x0;

	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xa4);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x02);
	sc_write(0xee00, 0x7F);
	sc_write(0xee01, 0x10);
	sc_write(0xfd93, 0x81);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);		
}

static void select3F00(void){
	unsigned int data = 0x0;

	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xa4);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x02);
	sc_write(0xee00, 0x3F);
	sc_write(0xee01, 0x00);
	sc_write(0xfd93, 0x81);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);		
}

static void selectMF(void){
	unsigned int data = 0x0;

	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0xa4);
	sc_write(0xfd87, 0x04);
	sc_write(0xfd88, 0x00);
	sc_write(0xfd89, 0x02);
	sc_write(0xee00, 0x4D);
	sc_write(0xee01, 0x46);
	sc_write(0xfd93, 0x81);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);		
}

static void enCHV(void){
	unsigned int data = 0x0;

	sc_write(0xfd85, 0xa0);
	sc_write(0xfd86, 0x28);
	sc_write(0xfd87, 0x00);
	sc_write(0xfd88, 0x01);
	sc_write(0xfd89, 0x08);
	sc_write(0xee00, 0x31);
	sc_write(0xee01, 0x32);
	sc_write(0xee02, 0x33);
	sc_write(0xee03, 0x34);	
	sc_write(0xee04, 0xff);	
	sc_write(0xee05, 0xff);	
	sc_write(0xee06, 0xff);	
	sc_write(0xee07, 0xff);		
	sc_write(0xfd93, 0x81);
	
	data = sc_read(ICC_STATUS2);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);
	
	data = sc_read(ICC_TIME_CTL_FLAG);
	PCSCPROC_PRINT("ICC_STATUS2 = %02x\n", data);		
}

static void rtksc_hwinit(void){
	unsigned int data = 0x0;
	
	printk("start smart-card hw init\n");

	sc_write(ICC_LDO_1, 0xfa);

	data = sc_read(ICC_LDO_1);
	printk("ICC_LDO_1 : 0x%02x\n", data);

	// clear interrupt bit	
	sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_EN|CARD_CD_INTERRUPT_FLAG);
	
	data = sc_read(ICC_SC_CD_INT);
	printk("ICC_SC_CD_INT : 0x%02x\n", data);	

	// set interrupt enable
	sc_write(ICC_SC_CD_INT, CARD_CD_INTERRUPT_EN);
	
	data = sc_read(ICC_SC_CD_INT);
	printk("ICC_SC_CD_INT : 0x%02x\n", data);
	
	// set pad pin
	sc_write(ICC_SC_PAD_PULLCTL, 0x2e);

	data = sc_read(ICC_SC_PAD_PULLCTL);
	printk("ICC_SC_PAD_PULLCTL : 0x%02x\n", data);

	// set output enable
	sc_write(ICC_PAD_OE, SC_PAD_OE);

	data = sc_read(ICC_PAD_OE);
	printk("ICC_PAD_OE : 0x%02x\n", data);

	// set asynchronous
	sc_write(ICC_SC_SEL, ASYNCHRONOUS_SC);

	data = sc_read(ICC_SC_SEL);
	printk("ICC_SC_SEL : 0x%02x\n", data);

	// set power control
	sc_write(ICC_PWR_CTL, POWER_CTL_ON | CLASSB_3);

	data = sc_read(ICC_PWR_CTL);
	//printk("ICC_PWR_CTL : 0x%02x\n", data);

	// set clock control
	sc_write(ICC_CLK_CTL, CLOCK_FREQ_3P75 | CLOCK_CTL_EN);

	data = sc_read(ICC_CLK_CTL);
	//printk("ICC_CLK_CTL : 0x%02x\n", data);

	// mask all error status
	sc_write(ICC_INTR_MASK, 0xFF);

	// enable all status interrupt except for CD
	sc_write(ICC_SC_MASK, ICC_END_MSK|ICC_TOTAL_MSK);
	
	// start transfer to get ATR
	sc_write(ICC_TRANSFER, START_TRANSFER);

	data = sc_read(ICC_TRANSFER);
	//printk("ICC_TRANSFER : 0x%02x\n", data);

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
	printk("ICC_SC_CD, 0x%x\n", data);
	if ( data & CARD_INSERT ){
		getData(10);
		//tasklet_hi_schedule(&atr_handler_tasklet);
		//SCIF_PRINT("reset ETU and get ATR\n");
		//resetETU();
		//sc_write(ICC_TRANSFER, START_TRANSFER);		
	}

	REG32(IO_MODE_EN) |= SC_CD_EN;
}


static void dumpreg(void){
	unsigned int data = 0x0;
	data = sc_read(ICC_PAD_OE);
	printk("ICC_PAD_OE : %04x:0x%02x\n", ICC_PAD_OE, data);	

	data = sc_read(ICC_SC_SEL);
	printk("ICC_SC_SEL : %04x:0x%02x\n", ICC_SC_SEL, data);	

	data = sc_read(ICC_SC_CD_INT);
	printk("ICC_SC_CD_INT : %04x:0x%02x\n", ICC_SC_CD_INT, data);	

	data = sc_read(ICC_SC_PAD_PULLCTL);
	printk("ICC_SC_PAD_PULLCTL : %04x:0x%02x\n", ICC_SC_PAD_PULLCTL, data);	
	
	data = sc_read(ICC_SC_BIST_EN);
	printk("ICC_SC_BIST_EN : %04x:0x%02x\n", ICC_SC_BIST_EN, data);	

	data = sc_read(ICC_PWR_CTL);
	printk("ICC_PWR_CTL : %04x:0x%02x\n", ICC_PWR_CTL, data);	

	data = sc_read(ICC_CLK_CTL);
	printk("ICC_CLK_CTL : %04x:0x%02x\n", ICC_CLK_CTL, data);	

	data = sc_read(ICC_RST_CTL);
	printk("ICC_RST_CTL : %04x:0x%02x\n", ICC_RST_CTL, data);	

	data = sc_read(ICC_CTL0);
	printk("ICC_CTL0 : %04x:0x%02x\n", ICC_CTL0, data);	

	data = sc_read(ICC_CTL1);
	printk("ICC_CTL1 : %04x:0x%02x\n", ICC_CTL1, data);	

	data = sc_read(ICC_T0_CLA);
	printk("ICC_T0_CLA : %04x:0x%02x\n", ICC_T0_CLA, data);	

	data = sc_read(ICC_T0_INS);
	printk("ICC_T0_INS : %04x:0x%02x\n", ICC_T0_INS, data);	
	
	data = sc_read(ICC_T0_P1);
	printk("ICC_T0_P1 : %04x:0x%02x\n", ICC_T0_P1, data);	

	data = sc_read(ICC_T0_P2);
	printk("ICC_T0_P2 : %04x:0x%02x\n", ICC_T0_P2, data);	

	data = sc_read(ICC_T0_P3);
	printk("ICC_T0_P3 : %04x:0x%02x\n", ICC_T0_P3, data);	

	data = sc_read(ICC_PPS_RQST_LEN);
	printk("ICC_PPS_RQST_LEN : %04x:0x%02x\n", ICC_PPS_RQST_LEN, data);	
	
	data = sc_read(ICC_T1_BLK_LEN0);
	printk("ICC_T1_BLK_LEN0 : %04x:0x%02x\n", ICC_T1_BLK_LEN0, data);	

	data = sc_read(ICC_T1_BLK_LEN1);
	printk("ICC_T1_BLK_LEN1 : %04x:0x%02x\n", ICC_T1_BLK_LEN1, data);	

	data = sc_read(ICC_ATR_LEN);
	printk("ICC_ATR_LEN : %04x:0x%02x\n", ICC_ATR_LEN, data);	

	data = sc_read(ICC_RESET_TCTL2_L);
	printk("ICC_RESET_TCTL2_L : %04x:0x%02x\n", ICC_RESET_TCTL2_L, data);	
	
	data = sc_read(ICC_RESET_TCTL2_H);
	printk("ICC_RESET_TCTL2_H : %04x:0x%02x\n", ICC_RESET_TCTL2_H, data);	

	data = sc_read(ICC_STATUS1);
	printk("ICC_STATUS1 : %04x:0x%02x\n", ICC_STATUS1, data);	

	data = sc_read(ICC_STATUS2);
	printk("ICC_STATUS2 : %04x:0x%02x\n", ICC_STATUS2, data);	

	data = sc_read(ICC_TIME_CTL_FLAG);
	printk("ICC_TIME_CTL_FLAG : %04x:0x%02x\n", ICC_TIME_CTL_FLAG, data);
	
	data = sc_read(ICC_TRANSFER);
	printk("ICC_TRANSFER : %04x:0x%02x\n", ICC_TRANSFER, data);	

	data = sc_read(ICC_ETU0);
	printk("ICC_ETU0 : %04x:0x%02x\n", ICC_ETU0, data);	

	data = sc_read(ICC_ETU1);
	printk("ICC_ETU1 : %04x:0x%02x\n", ICC_ETU1, data);	

	data = sc_read(ICC_GUARDTIME);
	printk("ICC_GUARDTIME : %04x:0x%02x\n", ICC_GUARDTIME, data);	
	
	data = sc_read(ICC_OPP_GUARDTIME);
	printk("ICC_OPP_GUARDTIME : %04x:0x%02x\n", ICC_OPP_GUARDTIME, data);	

	data = sc_read(ICC_T0_WWT0);
	printk("ICC_T0_WWT0 : %04x:0x%02x\n", ICC_T0_WWT0, data);	

	data = sc_read(ICC_T0_WWT1);
	printk("ICC_T0_WWT1 : %04x:0x%02x\n", ICC_T0_WWT1, data);	

	data = sc_read(ICC_T0_WWT2);
	printk("ICC_T0_WWT2 : %04x:0x%02x\n", ICC_T0_WWT2, data);

	data = sc_read(ICC_RESET_TCTL0);
	printk("ICC_RESET_TCTL0 : %04x:0x%02x\n", ICC_RESET_TCTL0, data);	

	data = sc_read(ICC_RESET_TCTL1);
	printk("ICC_RESET_TCTL1 : %04x:0x%02x\n", ICC_RESET_TCTL1, data);	

	data = sc_read(ICC_T0_STATUS);
	printk("ICC_T0_STATUS : %04x:0x%02x\n", ICC_T0_STATUS, data);	

	data = sc_read(ICC_T1_CWT0);
	printk("ICC_T1_CWT0 : %04x:0x%02x\n", ICC_T1_CWT0, data);	
	
	data = sc_read(ICC_T1_CWT1);
	printk("ICC_T1_CWT1 : %04x:0x%02x\n", ICC_T1_CWT1, data);	

	data = sc_read(ICC_T1_BWT0);
	printk("ICC_T1_BWT0 : %04x:0x%02x\n", ICC_T1_BWT0, data);	

	data = sc_read(ICC_T1_BWT1);
	printk("ICC_T1_BWT1 : %04x:0x%02x\n", ICC_T1_BWT1, data);	

	data = sc_read(ICC_T1_BWT2);
	printk("ICC_T1_BWT2 : %04x:0x%02x\n", ICC_T1_BWT2, data);	

	data = sc_read(ICC_IO_DRIVING);
	printk("ICC_IO_DRIVING : %04x:0x%02x\n", ICC_IO_DRIVING, data);	

	data = sc_read(SCB_CTL);
	printk("SCB_CTL : %04x:0x%02x\n", SCB_CTL, data);	

	data = sc_read(SCB_ADDR_OFFSET);
	printk("SCB_ADDR_OFFSET : %04x:0x%02x\n", SCB_ADDR_OFFSET, data);	
	
	data = sc_read(ICC_INTR_MASK);
	printk("ICC_INTR_MASK : %04x:0x%02x\n", ICC_INTR_MASK, data);

	data = sc_read(ICC_SC_MASK);
	printk("ICC_SC_MASK : %04x:0x%02x\n", ICC_SC_MASK, data);	
	
	data = sc_read(ICC_DBG_STATUS01);
	printk("ICC_DBG_STATUS01 : %04x:0x%02x\n", ICC_DBG_STATUS01, data);		
}

static int pcscfs_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	
	return 0;
}

static int pcscfs_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	int len = 0, func = 0;
	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d %d", &len, &func);
		printk("len%d, func%d\n", len, func);
		if ( func == 0 )
			getData(len);
		else 
			(*funcptr[func])();
	}
	return count;
}


int __init pcsc_proc_init(void)
{
	struct proc_dir_entry *pcsc_test_proc;

	pcsc_test_proc = create_proc_entry( PCSCNAME, 0644, NULL);
	if (pcsc_test_proc == NULL) {
		remove_proc_entry(PCSCNAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				PCSCNAME);
		return -ENOMEM;
	}
	pcsc_test_proc->read_proc  = (read_proc_t *)pcscfs_read;
	pcsc_test_proc->write_proc  = (write_proc_t *)pcscfs_write;

	
	return 0;
}

void __exit pcsc_proc_exit( void )
{
	remove_proc_entry( PCSCNAME, NULL );
}

module_init( pcsc_proc_init );
module_exit( pcsc_proc_exit );


static int pcscfs2_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	
	return 0;
}

extern int rtksc_init(void);
static int pcscfs2_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128], str;
	//int type = 0;// 0 :read , 1 : write
	int reg = 0, value = 0, data1 = 0x0;
	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%c 0x%x 0x%x", &str, &reg, &value);
		//printk("type%d, reg%04x, value%04x\n", type, reg, value);
		if ( str == 'r' ){ // read
			data1 = sc_read(reg);
			printk("Read  : reg0x%04x, value0x%04x\n", reg, data1);
		}else if ( str == 'w' ){ // write
			printk("Write : reg0x%04x, value0x%04x\n", reg, value);
			sc_write(reg, value);
		}else if ( str == 'i' ) {
			rtksc_init();
		}else if ( str == 'g' ) {
			getData(32);
		}
	}
	return count;
}

int __init pcsc2_proc_init(void)
{
	struct proc_dir_entry *pcsc_test_proc;

	pcsc_test_proc = create_proc_entry( PCSCNAME2, 0644, NULL);
	if (pcsc_test_proc == NULL) {
		remove_proc_entry(PCSCNAME2, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				PCSCNAME2);
		return -ENOMEM;
	}
	pcsc_test_proc->read_proc  = (read_proc_t *)pcscfs2_read;
	pcsc_test_proc->write_proc  = (write_proc_t *)pcscfs2_write;

	
	return 0;
}

void __exit pcsc2_proc_exit( void )
{
	remove_proc_entry( PCSCNAME2, NULL );
}

module_init( pcsc2_proc_init );
module_exit( pcsc2_proc_exit );

