#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include "rtk_voip.h"
#include "voip_init.h"

#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_help_ipc.h"
#include "snd_zarlink_ioc_op.h"

#include "zarlink_int.h"
#include "zarlink_api.h"

#include "gpio.h"
#include "spi.h"
static voip_snd_t			snd_zarlink_le88601[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];
static rtl_spi_dev_t		spi_dev_le88601[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];
/*
** Realtek memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static RTKDevObj	  	  	DevObj_le88601[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR]; //
static RTKLineObj	  	  	LineObj_le88601[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR]; //2FXS

/*
** Application memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static Vp886DeviceObjectType  VpDevObj_le88601[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];
static Vp886LineObjectType    VpLineObj_le88601[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];

static VpDevCtxType           VpDevCtx_le88601[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];

static VpLineCtxType          VpLineCtx_le88601[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR];

#if 1
static const uint32 * const pin_cs_le88601 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_PIN_CS - 1 ];
#else
static uint32 pin_cs_le88601[] = {
#if !defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116 ) && !defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316 )
	PIN_CS1,
	PIN_CS2,
#endif
	PIN_CS3,
	PIN_CS4,
};

#define NUM_OF_PIN_CS_LE88601		( sizeof( pin_cs_le88601 ) / sizeof( pin_cs_le88601[ 0 ] ) )

CT_ASSERT( NUM_OF_PIN_CS_LE88601 >= CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR );
#endif

static int SLIC_init_le88601(int pcm_mode, int initonly)
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
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR; i ++ ) {
		PRINT_MSG( "le88601[%d] CS=%08X\n", i, CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_PIN_CS );
		#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI		
			init_spi_pins( &spi_dev_le88601[ i ], CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_PIN_CS + i, PIN_CLK, PIN_DO, PIN_DI);
		#else		
			_init_rtl_spi_dev_type_hw( &spi_dev_le88601[ i ], CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_PIN_CS);
			extern void slicInternalCS(int cs);
			slicInternalCS(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_PIN_CS);
		#endif
	}
	// Create Dev & Line Object 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR; i ++ ) {
		Ve886CreateDevObj( 
			DEV_FXSFXS, 0, 
			&DevObj_le88601[ i ], &LineObj_le88601[ i * 2 ],
			&spi_dev_le88601[ i ], VP_DEV_886_SERIES,
			&VpDevObj_le88601[ i ], &VpDevCtx_le88601[ i ],
			&VpLineObj_le88601[ i * 2 ], &VpLineCtx_le88601[ i * 2 ]);

		/* FXS */
		Ve886CreateLineObj( rtkGetNewChID(),   0, LINE_FXS, DevObj_le88601[ i ].pLine[0], 
							pcm_mode, snd_zarlink_le88601[ i * 2 ].TS1 );
							//pcm_mode, snd_zarlink_le88601[ i ].TS1 );
		/* FXS */
		Ve886CreateLineObj( rtkGetNewChID(), 1, LINE_FXS, DevObj_le88601[ i ].pLine[1], 
							pcm_mode, snd_zarlink_le88601[ i * 2 + 1 ].TS1 );
	}
	
label_do_init_only:

	/* ******************* */
	/* Init Zarlink API-II */
	/* ******************* */
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR; i ++ ) 
	{
		/* deviceId imply spi device id */
		pDev = &DevObj_le88601[ i ];
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

static void SLIC_reset_le88601( voip_snd_t *this, int codec_law )
{
	extern int law;	// FIXME: chmap 
	
	SLIC_init_le88601( law, 1 /* init only */ );
}

static void SLIC_show_ID_le88601( voip_snd_t *this )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	RTKDevObj * const pDev = pLine ->pDev;
	
	Ve886GetRev( pDev );
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------
__attribute__ ((section(".snd_desc_data")))
static snd_ops_fxs_t snd_le88601_fxs_ops;

static void __init fill_le88601_register_info( 
	voip_snd_t snd_zarlink_le88601[],
	int n_fxs, int m_daa, uint16 TS_base,
	RTKLineObj LineObj_le88601[] 
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

		snd_zarlink_le88601[ sch ].sch = sch;
		snd_zarlink_le88601[ sch ].name = "le88601";
		snd_zarlink_le88601[ sch ].snd_type = SND_TYPE_FXS;
		snd_zarlink_le88601[ sch ].bus_type_sup = BUS_TYPE_PCM;
		snd_zarlink_le88601[ sch ].TS1 = TS_base + sch * 2;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_WIDEBAND
		snd_zarlink_le88601[ sch ].TS2 = ( daa || TS_base + sch * 2 >= 16 ? 0 : TS_base + ( sch + 8 ) * 2 );
		snd_zarlink_le88601[ sch ].band_mode_sup = ( daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K );
#else
		snd_zarlink_le88601[ sch ].TS2 = 0;
		snd_zarlink_le88601[ sch ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_zarlink_le88601[ sch ].snd_ops = ( const snd_ops_t * )&snd_le88601_fxs_ops;
		snd_zarlink_le88601[ sch ].priv = &LineObj_le88601[ sch ];
		
		// DAA port  
		if( daa ) {
			snd_zarlink_le88601[ sch ].snd_type = SND_TYPE_DAA;
#if 0
			snd_zarlink_le88601[ sch ].snd_ops = ( const snd_ops_t * )&snd_zarlink_daa_ops;
#else
			printk( "No snd_ops for DAA!!\n" );
#endif
		}		
	}
}

static int __init voip_snd_zarlink_init_le88601( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_zarlink_fxs_ops;
	int i;//, sch, daa;
	int TS_base, TS_cur;
	int rtn;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t le88601_defer;
#endif
	
	// le88601 override proslic base ops 
	snd_le88601_fxs_ops = snd_zarlink_fxs_ops;

	snd_le88601_fxs_ops.SLIC_reset = SLIC_reset_le88601;
	snd_le88601_fxs_ops.SLIC_show_ID = SLIC_show_ID_le88601;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();
	if( TS_base < 0 )
		TS_base = 0;
	
	TS_cur = TS_base;
	// common port definition 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR; i ++ ) {
		
		fill_le88601_register_info( &snd_zarlink_le88601[ i * 2 ], 
				2 /* fxs */, 0 /* daa */, (uint16)TS_cur,
				&LineObj_le88601[ i * 2 ] );
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_OWNER_ALL_DSP
		if( fill_ipc_dsp_cpuid( &snd_zarlink_le88601[ i ], 
				1 /* fxs */, 0 /* daa */, 
				i, 
				SND_PER_DSPCPU( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR, 
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
	le88601_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_le88601;
	le88601_defer.p0 = law;
	le88601_defer.p1 = 0;
	
	add_defer_initialization( &le88601_defer );
	rtn = 0; //success
#else
	rtn = SLIC_init_le88601( law, 0 /* allocate */ );
#endif
	
	if ( rtn == FAILED ) /* init Device fail */
		return -1;

	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR; i ++ ) 
		register_voip_snd( &snd_zarlink_le88601[ i * 2 ], 2 );	

	return 0;
}

voip_initcall_snd( voip_snd_zarlink_init_le88601 );

// --------------------------------------------------------
// IOC support 
// --------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_IO
#define LE88601_IO_NUM		2

static voip_ioc_t le88601_ioc[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR * 2 ][ LE88601_IO_NUM ];
static zarlink_ioc_priv_t le88601_ioc_priv[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR * 2 ][ LE88601_IO_NUM ];

static int __init voip_ioc_zarlink_init_le88601( void )
{
	int i, j;
	
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88601_NR * 2; i ++ ) {
		for( j = 0; j < LE88601_IO_NUM; j ++ ) {
			
			voip_ioc_t * const p_ioc = &le88601_ioc[ i ][ j ];
			zarlink_ioc_priv_t * const p_ioc_priv = &le88601_ioc_priv[ i ][ j ];
			voip_snd_t * const p_snd = &snd_zarlink_le88601[ i ];
			
			p_ioc ->ioch = ( i << 8 ) | j;
			p_ioc ->name = "le88601";
			p_ioc ->ioc_type = ( j == 1 ? IOC_TYPE_RELAY : IOC_TYPE_LED );
			p_ioc ->pre_assigned_snd_ptr = ( j < 3 ? p_snd : NULL );
			p_ioc ->ioc_ops = ( p_ioc ->ioc_type == IOC_TYPE_RELAY ? &ioc_relay_ops_zarlink : &ioc_led_ops_zarlink );
			p_ioc ->priv = p_ioc_priv;
			
			p_ioc ->state_var = ( p_ioc ->ioc_type == IOC_TYPE_RELAY ? IOC_STATE_RELAY_CLOSE : IOC_STATE_LED_OFF );
			
			// NOTE: one 88601 has two lines, and our definition is:  
			//       First line uses IO1 and IO2, and 
			//       second line uses IO3 and IO4. 
			p_ioc_priv ->IO = zarlink_VPIO_list[ ( i & 0x01 ? 2 + j : j ) ];
			p_ioc_priv ->snd_priv = p_snd ->priv;
			
			InitializeZarlinkIO( p_ioc );
		}
		
		register_voip_ioc( &le88601_ioc[ i ][ 0 ], LE88601_IO_NUM );
	}
	
	return 0;
}

voip_initcall_ioc( voip_ioc_zarlink_init_le88601 );
#endif

#ifdef SLICDUMPREG
#define VDUMPREGNAME1 "slicdumpreg"

static int voip_dumpreg_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	
	return 0;
}

static int voip_dumpreg_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	int t1 = 0;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%x", &t1);
		printk("input = [%d]\n", t1);
		if ( t1 == 0 )
			rtkDumpReg(&DevObj_le88601[0]);
		else if ( t1 == 1 )
			Ve886GetRev( &DevObj_le88601[0]);
	}

	return count;
}

static int __init voip_proc_dumpreg_init(void)
{
	struct proc_dir_entry *voip_add_test_proc;

	voip_add_test_proc = create_proc_entry( VDUMPREGNAME1, 0644, NULL);
	if (voip_add_test_proc == NULL) {
		remove_proc_entry(VDUMPREGNAME1, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				VDUMPREGNAME1);
		return -ENOMEM;
	}
	voip_add_test_proc->read_proc  = (read_proc_t *)voip_dumpreg_read;
	voip_add_test_proc->write_proc  = (write_proc_t *)voip_dumpreg_write;

	
	return 0;
}

static void __exit voip_proc_dumpreg_exit( void )
{
	remove_proc_entry( VDUMPREGNAME1, NULL );
}

module_init( voip_proc_dumpreg_init );
module_exit( voip_proc_dumpreg_exit );
#endif
