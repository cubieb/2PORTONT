#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_params.h"
#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_proslic_type.h"
#include "si3226x.h"

// We consider diasy chain case only. 

#define CHAN_PER_DEVICE 		2	// one 3226x contains 2 FXS

#define TOTAL_NUM_OF_3226x_CH	( CONFIG_RTK_VOIP_SLIC_SI3226x_NR * 2 )
#define NUMBER_OF_PROSLIC_3226x	( TOTAL_NUM_OF_3226x_CH / CHAN_PER_DEVICE )
#if 1
#define NUMBER_OF_HWINTF_3226x	1
#define NUMBER_OF_CHAN_3226x		TOTAL_NUM_OF_3226x_CH
#else
// CS mode may use this 
#define NUMBER_OF_HWINTF_3226x	( TOTAL_NUM_OF_3226x_CH / CHAN_PER_DEVICE )
#endif

static ctrl_S gSpiGciObj_3226x[ NUMBER_OF_HWINTF_3226x ];
static ProslicContainer_t gProslicContainer_3226x[ TOTAL_NUM_OF_3226x_CH ];

static voip_snd_t snd_proslic_3226x[ TOTAL_NUM_OF_3226x_CH ];

#ifdef CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS_USE_SW
#if 1
static const uint32 * const pin_cs = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS - 1 ];
#else
static const uint32 pin_cs[] = {
	PIN_CS1, 
};

CT_ASSERT( ( sizeof( pin_cs ) / sizeof( pin_cs[ 0 ] ) ) >= NUMBER_OF_HWINTF_3226x );
#endif
#endif

static const proslic_args_t proslic_args_3226x;

static int SLIC_init_si3226x(int pcm_mode, int initonly)
{
	int i, sidx;
	int j;
	int i_size, i_device, i_channel;
	//rtl_spi_dev_t *spi_devs[ NUMBER_OF_HWINTF_3226x ];
	
	printk ("\n<<<<<<<<<<< Si3226x Driver Version %s >>>>>>>>>>\n", ProSLIC_Version());
	
	//if( !initonly ) {
	//	// init spi first, because reset pin will affect all SLIC 
	//	printk( "Preparing spi channel for SLIC...\n" );
	//	
	//	for( i = 0; i < NUMBER_OF_HWINTF_3226x; i ++ ) {
	//		spi_devs[ i ] = &gSpiGciObj_3226x[ i ].spi_dev;
	//	}
	//
	//	init_spi_channels( NUMBER_OF_HWINTF_3226x, spi_devs, pin_cs, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	//}
	
	for( i = 0, sidx = 0; i < NUMBER_OF_HWINTF_3226x; i ++ ) {
		
		printk( "--------------------------------------\n" );
		printk( "SLIC HW intf %d starting at %d CS=%08X\n", i, sidx, 
#ifdef CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS_USE_SW
			pin_cs[ i ] 
#else 
			CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS + i
#endif
		);
		
#ifdef CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS_USE_SW
		init_spi_pins( &gSpiGciObj_3226x[ i ].spi_dev, pin_cs[ i ], PIN_CLK, PIN_DO, PIN_DI);
#else
		_init_rtl_spi_dev_type_hw( &gSpiGciObj_3226x[ i ].spi_dev, 
									CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS + i );
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
                extern void slicInternalCS(int cs);
                slicInternalCS(CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS);
#endif
#endif
		
		i_size = NUMBER_OF_CHAN_3226x;
		i_device = NUMBER_OF_PROSLIC_3226x;
		i_channel = NUMBER_OF_CHAN_3226x;
		
		if( initonly )
			goto label_do_init_only;

		// create objs
		for( j = 0; j < i_size; j ++ ) {
			gProslicContainer_3226x[ sidx + j ].spiGciObj = &gSpiGciObj_3226x[ i ];			
		}
		
		proslic_alloc_objs( &gProslicContainer_3226x[ sidx ], 
			i_size, i_device, i_channel, SI3226X_TYPE );
				
		// init proslic 
label_do_init_only:
		
		proslic_init_user_objs( &gProslicContainer_3226x[ sidx ], i_size, 
						SI3226X_TYPE );
		
		proslic_init( &snd_proslic_3226x[ sidx ], &gProslicContainer_3226x[ sidx ], 
						i_size, 
						&proslic_args_3226x,
						pcm_mode );
		
		sidx += i_size;
	}
	
	return 0;
}

static void SLIC_reset_si3226x(voip_snd_t *this, int codec_law)
{
	// This function will cause system reset, if watch dog is enable!
	// Because calibration need mdelay(1900).
	unsigned long flags;
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Disable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  0xA5000000;
	restore_flags(flags);
	
	SLIC_init_si3226x(codec_law, 1 /* don't allocate */);
	
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Enable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
	restore_flags(flags);
}

static void SLIC_show_ID_si3226x( voip_snd_t *this )
{
	unsigned char reg_val, reg_len;
	
	reg_len = sizeof(reg_val);
	
	//reg_val = R_reg_dev(&spiGciObj ->spi_dev, i, 0);
	this ->fxs_ops ->SLIC_read_reg( this, 0, &reg_len, &reg_val );
	
	if (((reg_val&0x38)>>3) == 0)
		printk("Si3226x\n");
	else if (((reg_val&0x38)>>3) == 1)
		printk("Si3227\n");
	else if ((((reg_val&0x38)>>3) & 0x2) == 0x2 )
		printk("Si32268/69\n");
	else
		PRINT_R("Unknow SLIC\n");
}

static Si3226x_Ring_param_dump(Si3226x_Ring_Cfg* p)
{
	printk("0x%8X\n", p->rtper);
	printk("0x%8X\n", p->freq);
	printk("0x%8X\n", p->amp);
	printk("0x%8X\n", p->phas);
	printk("0x%8X\n", p->offset);
	printk("0x%8X\n", p->slope_ring);
    	printk("0x%8X\n", p->iring_lim);
    	printk("0x%8X\n", p->rtacth);
	printk("0x%8X\n", p->rtdcth);
	printk("0x%8X\n", p->rtacdb);
	printk("0x%8X\n", p->rtdcdb);
	printk("0x%8X\n", p->vov_ring_bat);
	printk("0x%8X\n", p->vov_ring_gnd);
    	printk("0x%8X\n", p->vbatr_expect);
	printk("0x%2X\n", p->talo);
	printk("0x%2X\n", p->tahi);
	printk("0x%2X\n", p->tilo);
	printk("0x%2X\n", p->tihi);
	printk("0x%8X\n", p->adap_ring_min_i);
    	printk("0x%8X\n", p->counter_iring_val);
	printk("0x%8X\n", p->counter_vtr_val);
    	printk("0x%8X\n", p->ar_const28);
    	printk("0x%8X\n", p->ar_const32);
   	printk("0x%8X\n", p->ar_const38);
    	printk("0x%8X\n", p->ar_const46);
	printk("0x%8X\n", p->rrd_delay);
	printk("0x%8X\n", p->rrd_delay2);
    	printk("0x%8X\n", p->vbat_track_min_rng);
	printk("0x%2X\n", p->ringcon);
    	printk("0x%2X\n", p->userstat);
	printk("0x%8X\n", p->vcm_ring);
    	printk("0x%8X\n", p->vcm_ring_fixed);
    	printk("0x%8X\n", p->delta_vcm);
    	printk("0x%8X\n", p->dcdc_rngtype);
    	printk("0x%8X\n", p->vov_dcdc_slope);
    	printk("0x%8X\n", p->vov_dcdc_os);
    	printk("0x%8X\n", p->vov_ring_bat_max);
}

static void SLIC_set_param_si3226x(voip_snd_t *this, unsigned int slic_type, unsigned int param_type, unsigned char* pParam, unsigned int param_size)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;

	extern int si3226x_ring_preset_size;
	extern int si3226x_dcfeed_preset_size;
	extern int si3226x_impedance_preset_size;
	extern Si3226x_Ring_Cfg Si3226x_Ring_Presets[];
	extern Si3226x_DCfeed_Cfg Si3226x_DCfeed_Presets[];
	extern Si3226x_Impedance_Cfg Si3226x_Impedance_Presets[];

	if ( slic_type == PROSLIC_TYPE_SI3226X )
	{
		switch (param_type)
		{
			case PROSLIC_PARAM_TYPE_RING:
				if ( param_size != (sizeof(Si3226x_Ring_Cfg)) )
				{
					PRINT_R("%s, line%d, size mis-match, %d != %d.\n", __FUNCTION__, __LINE__, param_size, sizeof(Si3226x_Ring_Cfg));
					return;
				}
				//Si3226x_Ring_param_dump((Si3226x_Ring_Cfg*)pParam);
				memcpy(&Si3226x_Ring_Presets[si3226x_ring_preset_size-1], pParam, param_size);
				ProSLIC_RingSetup(pSlic, si3226x_ring_preset_size-1);
				break;
			case PROSLIC_PARAM_TYPE_DCFEED:
				if ( param_size != (sizeof(Si3226x_DCfeed_Cfg)) )
				{
					PRINT_R("%s, line%d, size mis-match, %d != %d.\n", __FUNCTION__, __LINE__, param_size, sizeof(Si3226x_DCfeed_Cfg));
					return;
				}
				memcpy(&Si3226x_DCfeed_Presets[si3226x_dcfeed_preset_size-1], pParam, param_size);
				ProSLIC_DCFeedSetup(pSlic, si3226x_dcfeed_preset_size-1);
				break;
			case PROSLIC_PARAM_TYPE_IMPEDANCE:
				if ( param_size != (sizeof(Si3226x_Impedance_Cfg)) )
				{
					PRINT_R("%s, line%d, size mis-match, %d != %d.\n", __FUNCTION__, __LINE__, param_size, sizeof(Si3226x_Impedance_Cfg));
					return;
				}
				memcpy(&Si3226x_Impedance_Presets[si3226x_impedance_preset_size-1], pParam, param_size);
				ProSLIC_ZsynthSetup(pSlic, si3226x_impedance_preset_size-1);
				break;
			default:
				PRINT_R("%s doesn't support param type: %d\n", __FUNCTION__, param_type);
				break;
		}
	}
	else
		PRINT_R("%s doesn't support SLIC type: %d\n", __FUNCTION__, slic_type);

	PRINT_MSG("SLIC_set_param_proslic, ch = %d, slic_type = %d, param_type = %d, param_size = %d\n", this ->sch, slic_type, param_type, param_size);
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static snd_ops_fxs_t snd_proslic_3226x_ops;

static const proslic_args_t proslic_args_3226x = {
	.ring_setup_preset = 0,	// 20Hz, 48V
};

static int __init voip_snd_si3226x_init( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_proslic_fxs_ops;
	int i, sch;
	int TS_base;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t si3226x_defer;
#endif
	
	// 3226x override proslic base ops 
	snd_proslic_3226x_ops = snd_proslic_fxs_ops;
	
	snd_proslic_3226x_ops.SLIC_reset = SLIC_reset_si3226x;
	snd_proslic_3226x_ops.SLIC_show_ID = SLIC_show_ID_si3226x;
	snd_proslic_3226x_ops.SLIC_set_param = SLIC_set_param_si3226x,
	
	// get TS base 
	TS_base = get_snd_free_timeslot();	
	if( TS_base < 0 )
		TS_base = 0;
	
	// common port definition 
	for( i = 0, sch = 0; i < TOTAL_NUM_OF_3226x_CH; i ++, sch ++ ) {

		snd_proslic_3226x[ i ].sch = sch;
		snd_proslic_3226x[ i ].name = "si3226x";
		snd_proslic_3226x[ i ].snd_type = SND_TYPE_FXS;
		snd_proslic_3226x[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_proslic_3226x[ i ].TS1 = TS_base + i * 2;
#ifdef CONFIG_RTK_VOIP_SLIC_SI3226x_WIDEBAND
		snd_proslic_3226x[ i ].TS2 = ( TS_base + i * 2 >= 16 ? 0 : TS_base + ( i + 8 ) * 2 );
		snd_proslic_3226x[ i ].band_mode_sup = BAND_MODE_8K | BAND_MODE_16K;
#else
		snd_proslic_3226x[ i ].TS2 = 0;
		snd_proslic_3226x[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_proslic_3226x[ i ].snd_ops = ( const snd_ops_t * )&snd_proslic_3226x_ops;
		snd_proslic_3226x[ i ].priv = &gProslicContainer_3226x[ i ];
	}
	
	register_voip_snd( &snd_proslic_3226x[ 0 ], TOTAL_NUM_OF_3226x_CH );
	
	// SLIC init use ops 
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	si3226x_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_si3226x;
	si3226x_defer.p0 = law;
	si3226x_defer.p1 = 0;
	
	add_defer_initialization( &si3226x_defer );
#else
	SLIC_init_si3226x( law, 0 /* allocate */ );
#endif
	
	return 0;
}

voip_initcall_snd( voip_snd_si3226x_init );



#ifdef SLICDUMPREG
#define VTESTNAME5 "slicdumpreg3226x"
extern void SLIC_dump_proslic_ram(voip_snd_t *this);
extern void SLIC_dump_proslic_reg(voip_snd_t *this);
static int voip_add_test5_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

	return 1;
}

static int voip_add_test5_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{

	char tmp[128];
	unsigned int t1 = 0;

	if (count < 2)
		return -EFAULT;

//static void WriteReg_dev (rtl_spi_dev_t* pDev, uInt8 channel, uInt8 regAddr, uInt8 data)

	
	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &t1);
		if ( t1 == 0 ){		
			printk("dump ram\n");
			SLIC_dump_proslic_ram(&snd_proslic_3226x[0]);
		}else if ( t1 == 1 )
		{
			printk("dump reg\n");
			SLIC_dump_proslic_reg(&snd_proslic_3226x[0]);		
		}

	}

	return 1;
}

static int __init voip_proc_add_test5_init(void)
{
	struct proc_dir_entry *voip_add_test_proc;

	voip_add_test_proc = create_proc_entry( VTESTNAME5, 0644, NULL);
	if (voip_add_test_proc == NULL) {
		remove_proc_entry(VTESTNAME5, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
				VTESTNAME5);
		return -ENOMEM;
	}
	voip_add_test_proc->read_proc  = (read_proc_t *)voip_add_test5_read;
	voip_add_test_proc->write_proc  = (write_proc_t *)voip_add_test5_write;

	
	return 0;
}

static void __exit voip_proc_add_test5_exit( void )
{
	remove_proc_entry( VTESTNAME5, NULL );
}

module_init( voip_proc_add_test5_init );
module_exit( voip_proc_add_test5_exit );
#endif
