#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_feature.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_debug.h"
#include "con_register.h"

/* DAA NUM and Type*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT

// we assert SLIC_CH_NUM in GET_FEATURE ioctl 
#define RTK_VOIP_DAA_TYPE_FEATURE			REAL_DAA_NEGO
#endif //!CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
#else // !CONFIG_RTK_VOIP_DRIVERS_FXO
#define RTK_VOIP_DAA_TYPE_FEATURE			NO_DAA
#endif
				
/* Platform anf Platform Type */
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186) && (CONFIG_RTK_VOIP_DRIVERS_PCM8186V)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8186 | PLATFORM_TYPE_8186V)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186) && (CONFIG_RTK_VOIP_DRIVERS_PCM8186PV)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8186 | PLATFORM_TYPE_8186PV)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_865x | PLATFORM_TYPE_8651)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_867x | PLATFORM_TYPE_8671)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_867x | PLATFORM_TYPE_8672)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_867x | PLATFORM_TYPE_8676)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_865x | PLATFORM_TYPE_865xC)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_89xx | PLATFORM_TYPE_8972B_8982B)
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_89xx | PLATFORM_TYPE_89xxC)
#elif defined (CONFIG_RTK_VOIP_PLATFORM_8686) && defined (CONFIG_RTK_VOIP_SOC_8686_CPU0)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8686 | PLATFORM_TYPE_8686_CPU0)
#elif defined (CONFIG_RTK_VOIP_PLATFORM_8686) && defined (CONFIG_RTK_VOIP_SOC_8686_CPU1)
#define	RTK_VOIP_PLATFORM_FEATURE	(PLATFORM_8686 | PLATFORM_TYPE_8686_CPU1)
#endif

/* IVR */
#if defined (CONFIG_RTK_VOIP_IVR)
#define	RTK_VOIP_IVR_FEATURE		IVR_SUPPORT
#else
#define	RTK_VOIP_IVR_FEATURE		0x0
#endif

/* One ARM Router */
#if defined (CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER)
#define RTK_VOIP_ONE_ARM_ROUTER_FEATURE ONE_ARM_ROUTER_SUPPORT
#else
#define RTK_VOIP_ONE_ARM_ROUTER_FEATURE 0x0
#endif

/* G729 */
#if defined (CONFIG_RTK_VOIP_G729AB)
#define  RTK_VOIP_G729_FEATURE		CODEC_G729_SUPPORT
#else
#define  RTK_VOIP_G729_FEATURE		0x0
#endif

/* G723 */
#if defined (CONFIG_RTK_VOIP_G7231)
#define  RTK_VOIP_G723_FEATURE		CODEC_G723_SUPPORT
#else
#define  RTK_VOIP_G723_FEATURE		0x0
#endif

/* G726 */
#if defined (CONFIG_RTK_VOIP_G726)
#define  RTK_VOIP_G726_FEATURE		CODEC_G726_SUPPORT
#else
#define  RTK_VOIP_G726_FEATURE		0x0
#endif

/* GSM-FR */
#if defined (CONFIG_RTK_VOIP_GSMFR)
#define  RTK_VOIP_GSMFR_FEATURE		CODEC_GSMFR_SUPPORT
#else
#define  RTK_VOIP_GSMFR_FEATURE		0x0
#endif

/* AMR */
#if defined (CONFIG_RTK_VOIP_AMR_NB)
#define  RTK_VOIP_AMR_FEATURE		CODEC_AMR_SUPPORT
#else
#define  RTK_VOIP_AMR_FEATURE		0x0
#endif

/* iLBC */
#if defined (CONFIG_RTK_VOIP_ILBC)
#define  RTK_VOIP_iLBC_FEATURE		CODEC_iLBC_SUPPORT
#else
#define  RTK_VOIP_iLBC_FEATURE		0x0
#endif

/* T.38 */
#if defined (CONFIG_RTK_VOIP_T38)
#define  RTK_VOIP_T38_FEATURE		CODEC_T38_SUPPORT
#else
#define  RTK_VOIP_T38_FEATURE		0x0
#endif

/* G.722 */
#if defined (CONFIG_RTK_VOIP_G722)
#define  RTK_VOIP_G722_FEATURE		CODEC_G722_SUPPORT
#else
#define  RTK_VOIP_G722_FEATURE		0x0
#endif

/* G.711.1 */
#if defined (CONFIG_RTK_VOIP_G7111)
#define  RTK_VOIP_G7111_FEATURE		CODEC_G7111_SUPPORT
#else
#define  RTK_VOIP_G7111_FEATURE		0x0
#endif

/* SPEEX_NB */
#if defined (CONFIG_RTK_VOIP_SPEEX_NB)
#define  RTK_VOIP_SPEEX_NB_FEATURE	CODEC_SPEEX_NB_SUPPORT
#else
#define  RTK_VOIP_SPEEX_NB_FEATURE	0x0
#endif

#define RTK_VOIP_CODEC_FEATURE	( \
								RTK_VOIP_G729_FEATURE	| \
								RTK_VOIP_G723_FEATURE	| \
								RTK_VOIP_G726_FEATURE	| \
								RTK_VOIP_GSMFR_FEATURE	| \
								RTK_VOIP_AMR_FEATURE	| \
								RTK_VOIP_iLBC_FEATURE 	| \
								RTK_VOIP_T38_FEATURE	| \
								RTK_VOIP_G722_FEATURE	| \
								RTK_VOIP_G7111_FEATURE	| \
								RTK_VOIP_SPEEX_NB_FEATURE \
								)

/* VoIP Middleware*/
#ifdef CONFIG_AUDIOCODES_VOIP
#define	RTK_VOIP_MW_REALTEK_FEATURE			0x0
#define	RTK_VOIP_MW_AUDIOCODES_FEATURE		VOIP_MW_AUDIOCODES
#else
#define	RTK_VOIP_MW_REALTEK_FEATURE			VOIP_MW_REALTEK
#define	RTK_VOIP_MW_AUDIOCODES_FEATURE		0x0
#endif

/* VoIP IPC Arch */
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#define	RTK_VOIP_IPC_ARCH_FEATURE		IPC_ARCH_VOIP_SUPPORT
#else
#define	RTK_VOIP_IPC_ARCH_FEATURE		0x0
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
#define	RTK_VOIP_IPC_ARCH_TYPE_FEATURE	IPC_ARCH_TYPE_ETHERNET
#else
#define	RTK_VOIP_IPC_ARCH_TYPE_FEATURE	IPC_ARCH_TYPE_COPROCESSOR
#endif
#else
#define RTK_VOIP_IPC_ARCH_TYPE_FEATURE	0
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define	RTK_VOIP_IPC_ARCH_ROLE_FEATURE	IPC_ARCH_ROLE_IS_HOST
#else
#define	RTK_VOIP_IPC_ARCH_ROLE_FEATURE	IPC_ARCH_ROLE_IS_DSP
#endif
#else
#define RTK_VOIP_IPC_ARCH_ROLE_FEATURE	0
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
/* Ethernet DSP Device Number */
#define RTK_VOIP_DSP_DEVICE_NUM_FEATURE		( ( CONFIG_RTK_VOIP_DSP_DEVICE_NR << DSP_DEVICE_NUM_SHIFT ) & DSP_DEVICE_NUM_MASK )
#else
#define RTK_VOIP_DSP_DEVICE_NUM_FEATURE		0
#endif

VoipFeature_t gVoipFeature = 0;


void __init voip_con_main_init_feature( voip_con_t voip_con[], int num )
{
	int i;
	voip_snd_t *p_snd;
	VoipFeature_t feature = 0;
	uint32 slic = 0, daa = 0, dect = 0, phone = 0;
	
	// platform 
	feature |= RTK_VOIP_PLATFORM_FEATURE;
	
	// misc 
	feature |= RTK_VOIP_ONE_ARM_ROUTER_FEATURE;
	
	// channel
	for( i = 0; i < num; i ++ ) {
		
		p_snd = voip_con[ i ].snd_ptr;
		
		if( p_snd == NULL )
			continue;
		
		switch( p_snd ->snd_type ) {
		case SND_TYPE_FXS:
			slic ++;
			break;
			
		case SND_TYPE_DAA:
		case SND_TYPE_VDAA:
			daa ++;
			break;
			
		case SND_TYPE_DECT:
			dect ++;
			break;
			
		case SND_TYPE_AC:
			//phone = 1;
			slic ++;	// old design, we should modify it later 
			break;
		
		default:
			break;
		}
		
	}
	
	feature |= ( ( slic  << SLIC_NUM_SHIFT  ) & SLIC_NUM_MASK );
	feature |= ( ( daa   << DAA_NUM_SHIFT   ) & DAA_NUM_MASK  );
	feature |= ( ( dect  << DECT_NUM_SHIFT  ) & DECT_NUM_MASK );
	feature |= ( ( phone << PHONE_NUM_SHIFT ) & PHONE_NUM_MASK );
	
	feature |= RTK_VOIP_DAA_TYPE_FEATURE;
	
	// dsp
	feature |= RTK_VOIP_IVR_FEATURE;
	feature |= RTK_VOIP_MW_REALTEK_FEATURE;
	feature |= RTK_VOIP_MW_AUDIOCODES_FEATURE;
	
	// arch
	feature |= RTK_VOIP_IPC_ARCH_FEATURE;
	feature |= RTK_VOIP_IPC_ARCH_TYPE_FEATURE;
	feature |= RTK_VOIP_IPC_ARCH_ROLE_FEATURE;
	feature |= RTK_VOIP_DSP_DEVICE_NUM_FEATURE;
	
	// codec 
	feature |= RTK_VOIP_CODEC_FEATURE;
	
	gVoipFeature = feature;
	
	BOOT_MSG("Get RTK VoIP Feature.\n");
}

// --------------------------------------------------------
// proc 
// --------------------------------------------------------
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_feature_read_proc(struct seq_file *f, void *v)
{
#if ( ( PLATFORM_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x1C ) ||	\
		( ( PLATFORM_TYPE_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x03 )
#error "Re-write platform_str[]"
#endif
	
		const char * const platform_str[] = {
			"8186-8186V",		//8186 SoC - 8186V
			"8186-8186PV",		//8186 SoC - 8186PV
			"8186-RESERVED1",
			"8186-RESERVED2",
			"865x-8651",		//865x Soc - 8651
			"865x-865xC",		//865x Soc - 865xC
			"865x-RESERVED1",
			"865x-RESERVED2",
			"867x-8671",		//867x Soc - 8671
			"867x-8672",		//867x Soc - 8672
			"867x-8676",		//867x Soc - 8676
			"867x-RESERVED1",
			"89xx-8972B_8982B", //89xx Soc - 8972B or 8982B
			"89xx-89xxC",		//89xx Soc - 89xxC	
			"89xx-RESERVED1",	//89xx Soc	
			"89xx-RESERVED2",	//89xx Soc	
			"8686-CPU0",	//8686 SoC - CPU0
			"8686-CPU1",	//8686 SoC - CPU1
			"8686-RESERVED1",
			"8686-RESERVED2",
			"Noname5-0",
			"Noname5-1",
			"Noname5-2",
			"Noname5-3",
			"Noname6-0",
			"Noname6-1",
			"Noname6-2",
			"Noname6-3",
			"Noname7-0",
			"Noname7-1",
			"Noname7-2",
			"Noname7-3",
		};
		
		int i;
		uint8 *pch;
	if( f->index ) {	/* In our case, we write out all data at once. */
		return 0;
	}
	
	// dump feature 	
	seq_printf( f, "gVoipFeature = " );
	
	for( pch = ( uint8 * )&gVoipFeature, i = 0; i < sizeof( gVoipFeature ); i ++ )
	{
		seq_printf( f, "%02X ", *pch ++ );
	}
	
	seq_printf( f, "\n" );
	
	// human readable info
	seq_printf( f, "Platform: %s (%X)\n", 
				platform_str[ ( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) ], 
				( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) );
	
	seq_printf( f, "One Arm Router: %u\n", !!( gVoipFeature & ONE_ARM_ROUTER_SUPPORT ) );
	// ------------- 32 bits boundary 
	
	seq_printf( f, "Channel:\n" );
	
	seq_printf( f, "\tIP phone: %u\n", RTK_VOIP_PHONE_NUM( gVoipFeature ) );
	seq_printf( f, "\tDECT: %u\n", RTK_VOIP_DECT_NUM( gVoipFeature ) );
	seq_printf( f, "\tSLIC: %u\n", RTK_VOIP_SLIC_NUM( gVoipFeature ) );
	seq_printf( f, "\tDAA:	%u (type=%u)\n", RTK_VOIP_DAA_NUM( gVoipFeature ), ( uint32 )( ( gVoipFeature & DAA_TYPE_MASK ) >> DAA_TYPE_SHIFT ) );
	
	seq_printf( f, "DSP:\n" );
	seq_printf( f, "\tIVR: %u\n", !!( gVoipFeature & IVR_SUPPORT ) );
	seq_printf( f, "\tMW: %s %s\n", 
			( RTK_VOIP_MW_CHK_IS_REALTEK( gVoipFeature ) ? "Realtek" : "" ), 
			( RTK_VOIP_MW_CHK_IS_AUDIOCODES( gVoipFeature ) ? "Audiocodes" : "" ) );
	
	seq_printf( f, "Arch:\n" );
	seq_printf( f, "\tIPC: %s\n", ( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ? "Yes" : "Standalone" ) );
	seq_printf( f, "\t\tType: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP( gVoipFeature ) ? "Ethernet DSP" : "Coprocessor DSP" ) :
				"NA" ) );
	seq_printf( f, "\t\tRole: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_HOST( gVoipFeature ) ? "Host" : "DSP" ) :
				"NA" ) );
	seq_printf( f, "\tDSP Number: %u (IPC host only)\n", RTK_VOIP_DSP_DEVICE_NUMBER( gVoipFeature ) );
	
	seq_printf( f, "Codec:\n" );
	seq_printf( f, "\tG.711.1:	%u\n", !!( gVoipFeature & CODEC_G7111_SUPPORT ) );
	seq_printf( f, "\tG.722:	%u\n", !!( gVoipFeature & CODEC_G722_SUPPORT ) );
	seq_printf( f, "\tG.723:	%u\n", !!( gVoipFeature & CODEC_G723_SUPPORT ) );
	seq_printf( f, "\tG.726:	%u\n", !!( gVoipFeature & CODEC_G726_SUPPORT ) );
	seq_printf( f, "\tG.729:	%u\n", !!( gVoipFeature & CODEC_G729_SUPPORT ) );
	seq_printf( f, "\tGSM-FR:	%u\n", !!( gVoipFeature & CODEC_GSMFR_SUPPORT ) );

	seq_printf( f, "\tAMR:	%u\n", !!( gVoipFeature & CODEC_AMR_SUPPORT ) );
	seq_printf( f, "\tiLBC: %u\n", !!( gVoipFeature & CODEC_iLBC_SUPPORT ) );
	seq_printf( f, "\tT.38: %u\n", !!( gVoipFeature & CODEC_T38_SUPPORT ) );
	seq_printf( f, "\tSpeex-NB: %u\n", !!( gVoipFeature & CODEC_SPEEX_NB_SUPPORT ) );

	return 0;

}
#else
static int voip_feature_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#if ( ( PLATFORM_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x1C ) ||	\
	( ( PLATFORM_TYPE_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x03 )
#error "Re-write platform_str[]"
#endif

	const char * const platform_str[] = {
		"8186-8186V",		//8186 SoC - 8186V
		"8186-8186PV",		//8186 SoC - 8186PV
		"8186-RESERVED1",
		"8186-RESERVED2",
		"865x-8651",		//865x Soc - 8651
		"865x-865xC",		//865x Soc - 865xC
		"865x-RESERVED1",
		"865x-RESERVED2",
		"867x-8671",		//867x Soc - 8671
		"867x-8672",		//867x Soc - 8672
		"867x-8676",		//867x Soc - 8676
		"867x-RESERVED1",
		"89xx-8972B_8982B",	//89xx Soc - 8972B or 8982B
		"89xx-89xxC",		//89xx Soc - 89xxC	
		"89xx-RESERVED1",	//89xx Soc	
		"89xx-RESERVED2",	//89xx Soc	
		"8686-CPU0", 	//8686 SoC - CPU0
		"8686-CPU1", 	//8686 SoC - CPU1
		"8686-RESERVED1",
		"8686-RESERVED2",
		"Noname5-0",
		"Noname5-1",
		"Noname5-2",
		"Noname5-3",
		"Noname6-0",
		"Noname6-1",
		"Noname6-2",
		"Noname6-3",
		"Noname7-0",
		"Noname7-1",
		"Noname7-2",
		"Noname7-3",
	};
	
	int n = 0;
	int i;
	uint8 *pch;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	// dump feature 	
	n += sprintf( buf + n, "gVoipFeature = " );
	
	for( pch = ( uint8 * )&gVoipFeature, i = 0; i < sizeof( gVoipFeature ); i ++ )
	{
		n += sprintf( buf + n, "%02X ", *pch ++ );
	}
	
	n += sprintf( buf + n, "\n" );
	
	// human readable info
	n += sprintf( buf + n, "Platform: %s (%X)\n", 
				platform_str[ ( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) ], 
				( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) );
	
	n += sprintf( buf + n, "One Arm Router: %u\n", !!( gVoipFeature & ONE_ARM_ROUTER_SUPPORT ) );
	// ------------- 32 bits boundary 
	
	n += sprintf( buf + n, "Channel:\n" );

	n += sprintf( buf + n, "\tIP phone: %u\n", RTK_VOIP_PHONE_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tDECT: %u\n", RTK_VOIP_DECT_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tSLIC: %u\n", RTK_VOIP_SLIC_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tDAA:  %u (type=%u)\n", RTK_VOIP_DAA_NUM( gVoipFeature ), ( uint32 )( ( gVoipFeature & DAA_TYPE_MASK ) >> DAA_TYPE_SHIFT ) );
	
	n += sprintf( buf + n, "DSP:\n" );
	n += sprintf( buf + n, "\tIVR: %u\n", !!( gVoipFeature & IVR_SUPPORT ) );
	n += sprintf( buf + n, "\tMW: %s %s\n", 
			( RTK_VOIP_MW_CHK_IS_REALTEK( gVoipFeature ) ? "Realtek" : "" ), 
			( RTK_VOIP_MW_CHK_IS_AUDIOCODES( gVoipFeature ) ? "Audiocodes" : "" ) );
	
	n += sprintf( buf + n, "Arch:\n" );
	n += sprintf( buf + n, "\tIPC: %s\n", ( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ? "Yes" : "Standalone" ) );
	n += sprintf( buf + n, "\t\tType: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP( gVoipFeature ) ? "Ethernet DSP" : "Coprocessor DSP" ) :
				"NA" ) );
	n += sprintf( buf + n, "\t\tRole: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_HOST( gVoipFeature ) ? "Host" : "DSP" ) :
				"NA" ) );
	n += sprintf( buf + n, "\tDSP Number: %u (IPC host only)\n", RTK_VOIP_DSP_DEVICE_NUMBER( gVoipFeature ) );
	
	n += sprintf( buf + n, "Codec:\n" );
	n += sprintf( buf + n, "\tG.711.1:	%u\n", !!( gVoipFeature & CODEC_G7111_SUPPORT ) );
	n += sprintf( buf + n, "\tG.722:	%u\n", !!( gVoipFeature & CODEC_G722_SUPPORT ) );
	n += sprintf( buf + n, "\tG.723:	%u\n", !!( gVoipFeature & CODEC_G723_SUPPORT ) );
	n += sprintf( buf + n, "\tG.726:	%u\n", !!( gVoipFeature & CODEC_G726_SUPPORT ) );
	n += sprintf( buf + n, "\tG.729:	%u\n", !!( gVoipFeature & CODEC_G729_SUPPORT ) );
	n += sprintf( buf + n, "\tGSM-FR:	%u\n", !!( gVoipFeature & CODEC_GSMFR_SUPPORT ) );
	n += sprintf( buf + n, "\tAMR:	%u\n", !!( gVoipFeature & CODEC_AMR_SUPPORT ) );
	n += sprintf( buf + n, "\tiLBC:	%u\n", !!( gVoipFeature & CODEC_iLBC_SUPPORT ) );
	n += sprintf( buf + n, "\tT.38:	%u\n", !!( gVoipFeature & CODEC_T38_SUPPORT ) );
	n += sprintf( buf + n, "\tSpeex-NB:	%u\n", !!( gVoipFeature & CODEC_SPEEX_NB_SUPPORT ) );
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int feature_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_feature_read_proc, NULL);
}

struct file_operations proc_feature_fops = {
	.owner	= THIS_MODULE,
	.open	= feature_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,
//read:   voip_feature_read_proc
};
#endif


static int __init voip_feature_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_data( PROC_VOIP_DIR "/feature", 0, NULL, &proc_feature_fops, NULL );
#else
	create_proc_read_entry( PROC_VOIP_DIR "/feature", 0, NULL, voip_feature_read_proc, NULL );
#endif
	
	return 0;
}

static void __exit voip_feature_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/feature", NULL );
}

voip_initcall_proc( voip_feature_proc_init );
voip_exitcall( voip_feature_proc_exit );

