#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_proc.h"

#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_help_ipc.h"
#include "snd_zarlink_ioc_op.h"

#include "zarlink_int.h"
#include "zarlink_api.h"

#include "gpio.h"
#include "spi.h"
static voip_snd_t			snd_zarlink_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];
static rtl_spi_dev_t		spi_dev_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];
/*
** Realtek memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static RTKDevObj	  	  	DevObj_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR]; //
static RTKLineObj	  	  	LineObj_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR]; //2FXS

/*
** Application memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static Vp886DeviceObjectType  VpDevObj_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];
static Vp886LineObjectType    VpLineObj_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];

static VpDevCtxType           VpDevCtx_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];

static VpLineCtxType          VpLineCtx_le9661[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR];

#if 1
static const uint32 * const pin_cs_le9661 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_PIN_CS - 1 ];
#else
static uint32 pin_cs_le9661[] = {
#if !defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116 ) && !defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316 )
	PIN_CS1,
	PIN_CS2,
#endif
	PIN_CS3,
	PIN_CS4,
};

#define NUM_OF_PIN_CS_LE9661		( sizeof( pin_cs_le9661 ) / sizeof( pin_cs_le9661[ 0 ] ) )

CT_ASSERT( NUM_OF_PIN_CS_LE9661 >= CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR );
#endif

static int SLIC_init_le9661(int pcm_mode, int initonly)
{
	int i;
	RTKDevObj *pDev;
	int PcnType;
   	int rtn;
   	//int ch;

// setup pcm fs, slic reset
#ifdef CONFIG_RTK_VOIP_PLATFORM_8686
	extern void aipc_module_voip_set_pcm_fs(void);
	extern void aipc_module_voip_slic_reset(void);
	extern unsigned int soc_set_pcm_fs;
    if( soc_set_pcm_fs == 0 ){
		aipc_module_voip_set_pcm_fs();
#ifdef CONFIG_RTL8686_SLIC_RESET 
		aipc_module_voip_slic_reset();
#endif
		soc_set_pcm_fs = 1;
	}
#endif

	PRINT_MSG("================================================\n");
	PRINT_MSG("Zarlink API-II Lite Version %d.%d.%d\n", 
				VP_API_VERSION_MAJOR_NUM, 
				VP_API_VERSION_MINOR_NUM, 
				VP_API_VERSION_MINI_NUM);
			
	if( initonly )
		goto label_do_init_only;

	/* ******************* */
	/* Init Realtek obj    */
	/* ******************* */
	// setup GPIO for SPI 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) {
		PRINT_MSG( "le9661[%d] CS=%08X\n", i, CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_PIN_CS );
		#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI		
			init_spi_pins( &spi_dev_le9661[ i ], CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_PIN_CS + i, PIN_CLK, PIN_DO, PIN_DI);
		#else
			_init_rtl_spi_dev_type_hw( &spi_dev_le9661[ i ], CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_PIN_CS);
			extern void slicInternalCS(int cs);
			slicInternalCS(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_PIN_CS);
		#endif
	}
	// Create Dev & Line Object 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) {
		Ve886CreateDevObj( 
			DEV_FXS, 0, 
			&DevObj_le9661[ i ], &LineObj_le9661[ i ],
			&spi_dev_le9661[ i ], VP_DEV_887_SERIES,
			&VpDevObj_le9661[ i ], &VpDevCtx_le9661[ i ],
			&VpLineObj_le9661[ i ], &VpLineCtx_le9661[ i ]);

		/* FXS */
		Ve886CreateLineObj( rtkGetNewChID(),   0, LINE_FXS, DevObj_le9661[ i ].pLine[0], 
							pcm_mode, snd_zarlink_le9661[ i  ].TS1 );
		
	}
	
label_do_init_only:

	/* ******************* */
	/* Init Zarlink API-II */
	/* ******************* */
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) 
	{
		/* deviceId imply spi device id */
		pDev = &DevObj_le9661[ i ];
		PcnType = Ve886GetRev( pDev );

		if ( PcnType == FAILED ) {
			PRINT_R("Error (%d:x) %s Read version fail\n", i, __FUNCTION__);
			return FAILED;
		}

		/* Initialize API-2 device settings */
		rtn = zarlinkInitDevice( pDev );
		if ( rtn == FAILED ) {
			PRINT_R("Error (%d:x) %s\n", i, __FUNCTION__);
			return FAILED;
		}	
	}

	
	return SUCCESS;
}

static void SLIC_reset_le9661( voip_snd_t *this, int codec_law )
{
	extern int law;	// FIXME: chmap 
	
	SLIC_init_le9661( law, 1 /* init only */ );
}

static void SLIC_show_ID_le9661( voip_snd_t *this )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	RTKDevObj * const pDev = pLine ->pDev;
	
	Ve886GetRev( pDev );
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------
__attribute__ ((section(".snd_desc_data")))
static snd_ops_fxs_t snd_le9661_fxs_ops;

static void __init fill_le9661_register_info( 
	voip_snd_t snd_zarlink_le9661[],
	int n_fxs, int m_daa, uint16 TS_base,
	RTKLineObj LineObj_le9661[] 
	)
{
	// once call this function:
	//  - one control interface 
	//  - n fxs
	//  - m daa 
	int sch;
	int daa = 0;
	
	for( sch = 0; sch < n_fxs + m_daa; sch ++ ) {
	
		if( sch == n_fxs )
			daa = 1;

		snd_zarlink_le9661[ sch ].sch = sch;
		snd_zarlink_le9661[ sch ].name = "le9661";
		snd_zarlink_le9661[ sch ].snd_type = SND_TYPE_FXS;
		snd_zarlink_le9661[ sch ].bus_type_sup = BUS_TYPE_PCM;
		snd_zarlink_le9661[ sch ].TS1 = TS_base + sch * 2;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_WIDEBAND
		snd_zarlink_le9661[ sch ].TS2 = ( daa || TS_base + sch * 2 >= 16 ? 0 : TS_base + ( sch + 8 ) * 2 );
		snd_zarlink_le9661[ sch ].band_mode_sup = ( daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K );
#else
		snd_zarlink_le9661[ sch ].TS2 = 0;
		snd_zarlink_le9661[ sch ].band_mode_sup = BAND_MODE_8K;
#endif
	//	snd_zarlink_le9661[ 0 ].TS1 = 4;
	//	snd_zarlink_le9661[ 1 ].TS1 = 2;
		snd_zarlink_le9661[ sch ].snd_ops = ( const snd_ops_t * )&snd_le9661_fxs_ops;
		snd_zarlink_le9661[ sch ].priv = &LineObj_le9661[ sch ];
		
		// DAA port  
		if( daa ) {
			snd_zarlink_le9661[ sch ].snd_type = SND_TYPE_DAA;
#if 0
			snd_zarlink_le9661[ sch ].snd_ops = ( const snd_ops_t * )&snd_zarlink_daa_ops;
#else
			printk( "No snd_ops for DAA!!\n" );
#endif
		}		
	}
}

static int __init voip_snd_zarlink_init_le9661( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_zarlink_fxs_ops;
	int i;//, sch, daa;
	int TS_base, TS_cur;
	int rtn;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t le9661_defer;
#endif
	
	// le9661 override proslic base ops 
	snd_le9661_fxs_ops = snd_zarlink_fxs_ops;

	snd_le9661_fxs_ops.SLIC_reset = SLIC_reset_le9661;
	snd_le9661_fxs_ops.SLIC_show_ID = SLIC_show_ID_le9661;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();
	if( TS_base < 0 )
		TS_base = 0;
	
	TS_cur = TS_base;
	// common port definition 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) {
		
		fill_le9661_register_info( &snd_zarlink_le9661[ i * 2 ], 
				2 /* fxs */, 0 /* daa */, (uint16)TS_cur,
				&LineObj_le9661[ i * 2 ] );
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_OWNER_ALL_DSP
		if( fill_ipc_dsp_cpuid( &snd_zarlink_le9661[ i ], 
				1 /* fxs */, 0 /* daa */, 
				i, 
				SND_PER_DSPCPU( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR, 
								CONFIG_RTK_VOIP_DSP_DEVICE_NR ) ) )
		{
			TS_cur = TS_base;
		} else
#endif
		{
			TS_cur += 2;
		}
		
		//TS_base += 4;
	}
	
	// SLIC init use ops 
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	le9661_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_le9661;
	le9661_defer.p0 = law;
	le9661_defer.p1 = 0;
	
	add_defer_initialization( &le9661_defer );
	rtn = 0; //success
#else
	rtn = SLIC_init_le9661( law, 0 /* allocate */ );
#endif
	
	if ( rtn == FAILED ) /* init Device fail */
		return -1;

	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) 
		register_voip_snd( &snd_zarlink_le9661[ i * 2 ], 2 );	

	return 0;
}

voip_initcall_snd( voip_snd_zarlink_init_le9661 );

// --------------------------------------------------------
// IOC support 
// --------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_IO
#define LE9661_IO_NUM		1
//#define LE9661_IO_NUM		2

static voip_ioc_t le9661_ioc[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR ][ LE9661_IO_NUM ];
static zarlink_ioc_priv_t le9661_ioc_priv[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR ][ LE9661_IO_NUM ];

static int __init voip_ioc_zarlink_init_le9661( void )
{
	int i, j;
	
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE9661_NR; i ++ ) {
		for( j = 0; j < LE9661_IO_NUM; j ++ ) {
			
			voip_ioc_t * const p_ioc = &le9661_ioc[ i ][ j ];
			zarlink_ioc_priv_t * const p_ioc_priv = &le9661_ioc_priv[ i ][ j ];
			voip_snd_t * const p_snd = &snd_zarlink_le9661[ i ];
			
			p_ioc ->ioch = ( i << 8 ) | j;
			p_ioc ->name = "le9661";
			p_ioc ->ioc_type = ( j == 1 ? IOC_TYPE_RELAY : IOC_TYPE_LED );
			p_ioc ->pre_assigned_snd_ptr = ( j < 3 ? p_snd : NULL );
			p_ioc ->ioc_ops = ( p_ioc ->ioc_type == IOC_TYPE_RELAY ? &ioc_relay_ops_zarlink : &ioc_led_ops_zarlink );
			p_ioc ->priv = p_ioc_priv;
			
			p_ioc ->state_var = ( p_ioc ->ioc_type == IOC_TYPE_RELAY ? IOC_STATE_RELAY_CLOSE : IOC_STATE_LED_OFF );
			
			// NOTE: one 9661 has two lines, and our definition is:  
			//       First line uses IO1 and IO2, and 
			//       second line uses IO3 and IO4. 
			p_ioc_priv ->IO = zarlink_VPIO_list[ ( i & 0x01 ? 2 + j : j ) ];
			p_ioc_priv ->snd_priv = p_snd ->priv;
			
			InitializeZarlinkIO( p_ioc );
		}
		
		register_voip_ioc( &le9661_ioc[ i ][ 0 ], LE9661_IO_NUM );
	}
	
	return 0;
}

voip_initcall_ioc( voip_ioc_zarlink_init_le9661 );
#endif

#ifdef SLICDUMPREG


static int reset = 20;
#define VDUMPREGNAME1 "slicdumpreg"
#define VDUMPREGNAME2 "slicdumpreg2"
#define GPIORESET 33
static int voip_dumpreg_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	
	return 0;
}
static int voip_dumpreg_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	int t1 = 0, i = 0;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST
	int ret = 0;
#endif

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &t1);
		printk("input = [%d]\n", t1);
		if ( t1 == 0 ){
			rtkDumpReg(&DevObj_le9661[0]);
		}
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST			
		else if ( t1 == 1 )
		{			
			rtkPortDetect(&LineObj_le9661[0], 3, &ret);
			if ( ret )
				printk("phone is plugged.\n");
			else
				printk("no phone is plugged.\n");		
		}		
#endif			
		else if ( t1 == 3){
#if 0		
			printk("reset = [%d]\n", reset);
			printk("pull low gpio = [%x]\n", GPIORESET);
			_rtl_generic_setGpioDataBit(GPIORESET, 0);

			mdelay(reset);

			printk("pull high gpio = [%x]\n", GPIORESET);
			_rtl_generic_setGpioDataBit(GPIORESET, 1);
#endif			
			mdelay(50);		
			for ( i = 0; i < 20 ; i ++ )
				Ve886GetRev( &DevObj_le9661[0] );
		}
	}

	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int le9661_dumpreg_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_dumpreg_read, NULL);
}

static ssize_t le9661_dumpreg_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return voip_dumpreg_write(file, userbuf, count, NULL);
}

struct file_operations proc_le9661_dumpreg_fops = {
	.owner	= THIS_MODULE,
	.open	= le9661_dumpreg_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = le9661_dumpreg_write,	
//read:   voip_dumpreg_read,
//write:	voip_dumpreg_write
};
#endif

static int __init voip_proc_dumpreg_init(void)
{
	struct proc_dir_entry *voip_add_test_proc;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
		voip_add_test_proc = proc_create(VDUMPREGNAME1, 0644, NULL, &proc_le9661_dumpreg_fops);
		if (voip_add_test_proc == NULL) {
			remove_proc_entry(VDUMPREGNAME1, NULL);
			printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
					VDUMPREGNAME1);
			return -ENOMEM;
		}	
#else
	voip_add_test_proc = create_proc_entry( VDUMPREGNAME1, 0644, NULL);
	if (voip_add_test_proc == NULL) {
		remove_proc_entry(VDUMPREGNAME1, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				VDUMPREGNAME1);
		return -ENOMEM;
	}
	voip_add_test_proc->read_proc  = (read_proc_t *)voip_dumpreg_read;
	voip_add_test_proc->write_proc  = (write_proc_t *)voip_dumpreg_write;
#endif
	
	return 0;
}

static void __exit voip_proc_dumpreg_exit( void )
{
	remove_proc_entry( VDUMPREGNAME1, NULL );
}

module_init( voip_proc_dumpreg_init );
module_exit( voip_proc_dumpreg_exit );





static int voip_dumpreg2_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	
	return 0;
}
extern VpStatusType zarlinkled (RTKLineObj *pLine, int onoff );
static int voip_dumpreg2_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	int t1 = 0;
	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &t1);
		printk("input = [%d]\n", t1);
		reset = t1;
	}
	return count;
}

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int le9661_dumpreg2_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_dumpreg2_read, NULL);
}

static ssize_t le9661_dumpreg2_write(struct file *file, const char __user * userbuf, size_t count, loff_t * off) {
        return voip_dumpreg2_write(file, userbuf, count, NULL);
}

struct file_operations proc_le9661_dumpreg2_fops = {
	.owner	= THIS_MODULE,
	.open	= le9661_dumpreg2_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
	.write  = le9661_dumpreg2_write,
//read:   voip_dumpreg2_read,
//write:	voip_dumpreg2_write
};
#endif

static int __init voip_proc_dumpreg2_init(void)
{
	struct proc_dir_entry *voip_add_test_proc;

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
		voip_add_test_proc = proc_create(VDUMPREGNAME2, 0644, NULL, &proc_le9661_dumpreg2_fops);
		if (voip_add_test_proc == NULL) {
			remove_proc_entry(VDUMPREGNAME1, NULL);
			printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
					VDUMPREGNAME1);
			return -ENOMEM;
		}
	
#else
	voip_add_test_proc = create_proc_entry( VDUMPREGNAME2, 0644, NULL);
	if (voip_add_test_proc == NULL) {
		remove_proc_entry(VDUMPREGNAME2, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				VDUMPREGNAME2);
		return -ENOMEM;
	}
	voip_add_test_proc->read_proc  = (read_proc_t *)voip_dumpreg2_read;
	voip_add_test_proc->write_proc  = (write_proc_t *)voip_dumpreg2_write;
#endif
	
	return 0;
}

static void __exit voip_proc_dumpreg2_exit( void )
{
	remove_proc_entry( VDUMPREGNAME2, NULL );
}

module_init( voip_proc_dumpreg2_init );
module_exit( voip_proc_dumpreg2_exit );
#endif
