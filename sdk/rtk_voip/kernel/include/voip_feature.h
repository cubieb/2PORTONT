#ifndef __VOIP_FEATURE_H
#define __VOIP_FEATURE_H

#include "rtk_voip.h"
#include "voip_types.h"

//*****************************************
//* VoIP Feature (64 bits)
//*****************************************
/*
 * Layout: 
 *
 * 63       55       47       39       31
 * +--------+--------+--------+--------+
 * |platform|  misc  |     codec       |
 * +--------+--------+--------+--------+
 * 
 * 31       23       15       7        0
 * +--------+--------+--------+--------+
 * |   DSP  |  ARCH  |   channel nr    |
 * +--------+--------+--------+--------+
 */

typedef uint64 VoipFeature_t;

#ifdef __KERNEL__
extern VoipFeature_t gVoipFeature;
#else
extern VoipFeature_t g_VoIP_Feature;	// defiend in voip_manager.c 
#endif

#define SHL32( x )		x ## 00000000ULL

/* Platform ( bit 61-63 )*/
#define PLATFORM_8186			SHL32( 0x00000000 )	//8186 SoC
#define PLATFORM_865x			SHL32( 0x20000000 )	//865x Soc
#define PLATFORM_867x			SHL32( 0x40000000 )	//867x Soc
#define PLATFORM_89xx			SHL32( 0x60000000 )	//89xx Soc
#define PLATFORM_8686			SHL32( 0x80000000 )	//8686 Soc
#define PLATFORM_RFU1			SHL32( 0xA0000000 )
#define PLATFORM_RFU2			SHL32( 0xC0000000 )
#define PLATFORM_RFU3			SHL32( 0xE0000000 )

#define PLATFORM_MASK			SHL32( 0xE0000000 )

/* Platform Type (8186) ( bit 59-60 )*/
#define PLATFORM_TYPE_8186V		SHL32( 0x00000000 )	//8186V 
#define PLATFORM_TYPE_8186PV	SHL32( 0x08000000 )	//8186PV
#define PLATFORM_TYPE3			SHL32( 0x10000000 )	//reserve
#define PLATFORM_TYPE4			SHL32( 0x18000000 )	//reserve

/* Platform Type (865x) ( bit 59-60 )*/
#define PLATFORM_TYPE_8651		SHL32( 0x00000000 )	//8651
#define PLATFORM_TYPE_865xC		SHL32( 0x08000000 )	//865xC
#define PLATFORM_TYPE3			SHL32( 0x10000000 )	//reserve
#define PLATFORM_TYPE4			SHL32( 0x18000000 )	//reserve

/* Platform Type (867x) ( bit 59-60 )*/
#define PLATFORM_TYPE_8671		SHL32( 0x00000000 )	//8671
#define PLATFORM_TYPE_8672		SHL32( 0x08000000 )	//8672
#define PLATFORM_TYPE_8676		SHL32( 0x10000000 )	//8676
#define PLATFORM_TYPE4			SHL32( 0x18000000 )	//reserve

/* Platform Type (89xx) ( bit 59-60 )*/
#define PLATFORM_TYPE_8972B_8982B	SHL32( 0x00000000 )	//8972B or 8982B
#define PLATFORM_TYPE_89xxC		SHL32( 0x08000000 )	//89xxC
#define PLATFORM_TYPE_89xxD		SHL32( 0x10000000 )	//89xxD
#define PLATFORM_TYPE4			SHL32( 0x18000000 )	//reserve

/* Platform Type (8686) ( bit 59-60 )*/
#define PLATFORM_TYPE_8686_CPU0	SHL32( 0x00000000 )	//CPU0
#define PLATFORM_TYPE_8686_CPU1	SHL32( 0x08000000 )	//CPU1
#define PLATFORM_TYPE3			SHL32( 0x10000000 )	//reserve
#define PLATFORM_TYPE4			SHL32( 0x18000000 )	//reserve

#define PLATFORM_TYPE_MASK		SHL32( 0x18000000 )

#define RTK_VOIP_PLATFORM_MASK	( PLATFORM_MASK | PLATFORM_TYPE_MASK )
#define RTK_VOIP_PLATFORM_SHIFT	59

/* Reserved bits 56-58 for platform */

/* One ARM Router ( bit 55 )*/
#define ONE_ARM_ROUTER_SUPPORT	SHL32( 0x00800000 )
//#define ONE_ARM_ROUTER_NOT_SUPPORT	~ONE_ARM_ROUTER_SUPPORT

/* Reserved bits 48-54 for misc */

/* Codec ( bit 32-47)*/
#define RTK_VOIP_CODEC_MASK		SHL32( 0x0000FFFF )

#define CODEC_G729_SUPPORT		SHL32( 0x00000001 )
#define CODEC_G729_NOT_SUPPORT	~CODEC_G729_SUPPORT

#define CODEC_G723_SUPPORT		SHL32( 0x00000002 )
#define CODEC_G723_NOT_SUPPORT	~CODEC_G723_SUPPORT

#define CODEC_G726_SUPPORT		SHL32( 0x00000004 )
#define CODEC_G726_NOT_SUPPORT	~CODEC_G726_SUPPORT

#define CODEC_GSMFR_SUPPORT		SHL32( 0x00000008 )
#define CODEC_GSMFR_NOT_SUPPORT	~CODEC_GSMFR_SUPPORT

#define CODEC_AMR_SUPPORT		SHL32( 0x00000010 )
#define CODEC_AMR_NOT_SUPPORT	~CODEC_AMR_SUPPORT

#define CODEC_iLBC_SUPPORT		SHL32( 0x00000020 )
#define CODEC_iLBC_NOT_SUPPORT	~CODEC_iLBC_SUPPORT

#define CODEC_T38_SUPPORT		SHL32( 0x00000040 )
#define CODEC_T38_NOT_SUPPORT	~CODEC_T38_SUPPORT

#define CODEC_G7111_SUPPORT		SHL32( 0x00000080 )
#define CODEC_G7111_NOT_SUPPORT	~CODEC_G7111_SUPPORT

#define CODEC_SPEEX_NB_SUPPORT	SHL32( 0x00000100 )
#define CODEC_SPEEX_NB_NOT_SUPPORT	~CODEC_SPEEX_NB_SUPPORT

#define CODEC_G722_SUPPORT		SHL32( 0x00000200 )
#define CODEC_G722_NOT_SUPPORT	~CODEC_G722_SUPPORT

/* IVR ( bit 31 )*/
#define IVR_SUPPORT			0x80000000
#define IVR_NOT_SUPPORT		~IVR_SUPPORT

/* VoIP MW - Realtek ( bit 30 )*/
#define VOIP_MW_REALTEK		0x40000000

/* VoIP MW - Audiocodes ( bit 29 )*/
#define VOIP_MW_AUDIOCODES	0x20000000

/* Reserved bit 24-28 for DSP */

/* IPC Arch (bit 23)*/
#define IPC_ARCH_VOIP_SUPPORT		0x00800000

/* IPC Arch type (bit 22)*/
#define IPC_ARCH_TYPE_ETHERNET		0x00000000
#define IPC_ARCH_TYPE_COPROCESSOR	0x00400000

#define IPC_ARCH_TYPE_MASK			0x00400000

/* IPC Arch role (bit 21)*/
#define IPC_ARCH_ROLE_IS_HOST		0x00000000
#define IPC_ARCH_ROLE_IS_DSP		0x00200000

#define IPC_ARCH_ROLE_MASK			0x00200000

/* Ethernet DSP Device Number( bit 18-20 ) - range 0-7 */
#define DSP_DEVICE_NUM_MASK		0x001C0000
#define DSP_DEVICE_NUM_SHIFT	18

/* Reserved bit 15-17 for arch */

/* DAA Type ( bit 14-15 )*/
#define NO_DAA				0x00000000	//No DAA
#define REAL_DAA			0x00004000	//No Negotiation
#define REAL_DAA_NEGO		0x00008000	//Negotiation
#define VIRTUAL_DAA			0x0000C000	//Virtual DAA

#define DAA_TYPE_MASK		0x0000C000
#define DAA_TYPE_SHIFT		14

/* Reserved bit 13 for channel */

/* Phone NUM ( bit 12 ) - range 0~1 */
#define PHONE_NUM_MASK		0x00001000
#define PHONE_NUM_SHIFT		12

/* DECT NUM ( bit 9-11 ) - range 0~7 */
#define DECT_NUM_MASK		0x00000E00
#define DECT_NUM_SHIFT		9

/* SLIC NUM ( bit 4-8 ) - range 0~31 */
#define SLIC_NUM_MASK		0x000001F0
#define SLIC_NUM_SHIFT		4

/* DAA NUM ( bit 0-3 ) - range 0~15 */
#define DAA_NUM_MASK		0x0000000F
#define DAA_NUM_SHIFT		0

//------- simulation -------

#if 0//def X86_SIMULATE

#define RTK_VOIP_SLIC_FEATURE     ( ( SLIC_CH_NUM << SLIC_NUM_SHIFT ) & SLIC_NUM_MASK )
#define RTK_VOIP_DECT_FEATURE     ( ( DECT_CH_NUM << DECT_NUM_SHIFT ) & DECT_NUM_MASK )
#define RTK_VOIP_DAA_FEATURE      ( ( ( DAA_CH_NUM << DAA_NUM_SHIFT ) & DAA_NUM_MASK ) |	\
									( DAA_CH_NUM ? REAL_DAA_NEGO : 0 ) )

#define RTK_VOIP_MW_FEATURE		( RTK_VOIP_MW_REALTEK_FEATURE | RTK_VOIP_MW_AUDIOCODES_FEATURE )

#define RTK_VOIP_ETHERNET_DSP_FEATURE	RTK_VOIP_IPC_ARCH_FEATURE

#define RTK_VOIP_FEATURE 	( \
							RTK_VOIP_SLIC_FEATURE			| \
							RTK_VOIP_DECT_FEATURE			| \
							RTK_VOIP_DAA_FEATURE			| \
							RTK_VOIP_PLATFORM_FEATURE 		| \
							RTK_VOIP_IVR_FEATURE			| \
							RTK_VOIP_ONE_ARM_ROUTER_FEATURE	| \
							RTK_VOIP_CODEC_FEATURE 			| \
							RTK_VOIP_MW_FEATURE	| \
							RTK_VOIP_DSP_DEVICE_NUM_FEATURE	| \
							RTK_VOIP_ETHERNET_DSP_FEATURE	\
							)
#endif

//------- feature macro -----------------------
#define RTK_VOIP_SLIC_NUM(f)	( unsigned int )		\
				( ( (f) & SLIC_NUM_MASK ) >> SLIC_NUM_SHIFT )
#define RTK_VOIP_DAA_NUM(f)		( unsigned int )		\
				( ( (f) & DAA_NUM_MASK ) >> DAA_NUM_SHIFT )
#define RTK_VOIP_DECT_NUM(f)	( unsigned int )		\
				( ( (f) & DECT_NUM_MASK ) >> DECT_NUM_SHIFT )
#define RTK_VOIP_PHONE_NUM(f)	( unsigned int )		\
				( ( (f) & PHONE_NUM_MASK ) >> PHONE_NUM_SHIFT )

#define RTK_VOIP_DECT_CH_OFFSET(f)	( unsigned int )0
#define RTK_VOIP_SLIC_CH_OFFSET(f)	( unsigned int )(RTK_VOIP_DECT_CH_OFFSET(f) + RTK_VOIP_DECT_NUM(f))
#define RTK_VOIP_DAA_CH_OFFSET(f)	( unsigned int )(RTK_VOIP_SLIC_CH_OFFSET(f) + RTK_VOIP_SLIC_NUM(f))

#define RTK_VOIP_IS_DECT_CH(ch, f)	( ( ( ch ) >= RTK_VOIP_DECT_CH_OFFSET( f ) ) && ( ( ch ) < RTK_VOIP_DECT_CH_OFFSET( f ) + RTK_VOIP_DECT_NUM( f ) ) )
#define RTK_VOIP_IS_SLIC_CH(ch, f)	( ( ( ch ) >= RTK_VOIP_SLIC_CH_OFFSET( f ) ) && ( ( ch ) < RTK_VOIP_SLIC_CH_OFFSET( f ) + RTK_VOIP_SLIC_NUM( f ) ) )
#define RTK_VOIP_IS_DAA_CH(ch, f)	( ( ( ch ) >= RTK_VOIP_DAA_CH_OFFSET( f ) )  && ( ( ch ) < RTK_VOIP_DAA_CH_OFFSET( f )  + RTK_VOIP_DAA_NUM( f ) ) )

#define RTK_VOIP_MW_CHK_IS_REALTEK(f)		((f) & VOIP_MW_REALTEK)
#define RTK_VOIP_MW_CHK_IS_AUDIOCODES(f)	((f) & VOIP_MW_AUDIOCODES)

#define	RTK_VOIP_PLATFORM_CMP(f1,f2)	(((f1) & ( PLATFORM_MASK | PLATFORM_TYPE_MASK ) ) == ((f2) & ( PLATFORM_MASK | PLATFORM_TYPE_MASK )))
//#define	RTK_VOIP_PLATFORM_CHECK(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == RTK_VOIP_PLATFORM_FEATURE)
#if ( RTK_VOIP_PLATFORM_SHIFT >= 32 && RTK_VOIP_PLATFORM_SHIFT < 64 )
#define RTK_VOIP_PLATFORM_CHK_IS8676(f)		((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_867x | PLATFORM_TYPE_8676)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS8672(f)		((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_867x | PLATFORM_TYPE_8672)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS865xC(f)	((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_865x | PLATFORM_TYPE_865xC)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS8972B(f)	((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_89xx | PLATFORM_TYPE_8972B_8982B)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS89xxC(f)	((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_89xx | PLATFORM_TYPE_89xxC)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS8686(f)		((uint32)(((f) & PLATFORM_MASK)>>32) == (uint32)((PLATFORM_8686)>>32))
#define RTK_VOIP_PLATFORM_CHK_IS89xxD(f)	((uint32)(((f) & RTK_VOIP_PLATFORM_MASK)>>32) == (uint32)((PLATFORM_89xx | PLATFORM_TYPE_89xxD)>>32))
#else
#define RTK_VOIP_PLATFORM_CHK_IS8676(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_867x | PLATFORM_TYPE_8676))
#define RTK_VOIP_PLATFORM_CHK_IS8672(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_867x | PLATFORM_TYPE_8672))
#define RTK_VOIP_PLATFORM_CHK_IS865xC(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_865x | PLATFORM_TYPE_865xC))
#define RTK_VOIP_PLATFORM_CHK_IS8972B(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_89xx | PLATFORM_TYPE_8972B_8982B))
#define RTK_VOIP_PLATFORM_CHK_IS89xxC(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_89xx | PLATFORM_TYPE_89xxC))
#define RTK_VOIP_PLATFORM_CHK_IS8686(f)		(((f) & PLATFORM_MASK) == (PLATFORM_8686))
#define RTK_VOIP_PLATFORM_CHK_IS89xxD(f)	(((f) & RTK_VOIP_PLATFORM_MASK) == (PLATFORM_89xx | PLATFORM_TYPE_89xxD))
#endif
//#define	RTK_VOIP_ROUTER_CHECK(f)	(!!((f) & ONE_ARM_ROUTER_SUPPORT) == !!RTK_VOIP_ONE_ARM_ROUTER_FEATURE)
//#define	RTK_VOIP_SLIC_CHECK(f)		(((f) & SLIC_NUM_MASK) == RTK_VOIP_SLIC_FEATURE)
//#define	RTK_VOIP_DECT_CHECK(f)		(((f) & RTK_VOIP_DECT_MASK) == RTK_VOIP_DECT_FEATURE)
//#define	RTK_VOIP_DAA_CHECK(f)		(((f) & RTK_VOIP_DAA_MASK) == RTK_VOIP_DAA_FEATURE)
//#define	RTK_VOIP_CODEC_CHECK(f)		(((f) & RTK_VOIP_CODEC_MASK) == RTK_VOIP_CODEC_FEATURE)
//#define	RTK_VOIP_MW_CHECK(f)		(((f) & RTK_VOIP_MW_MASK) == RTK_VOIP_MW_FEATURE)
//#define	RTK_VOIP_ETHERNET_DSP_CHECK(f)		(((f) & RTK_VOIP_ETHERNET_DSP_FEATURE) == RTK_VOIP_ETHERNET_DSP_FEATURE)
#define	RTK_VOIP_ROUTER_CMP(f1,f2)		(!!((f1) & ONE_ARM_ROUTER_SUPPORT) == !!((f2) & ONE_ARM_ROUTER_SUPPORT))
#define	RTK_VOIP_SLIC_CMP(f1,f2)		(RTK_VOIP_SLIC_NUM(f1) == RTK_VOIP_SLIC_NUM(f2))
#define	RTK_VOIP_DECT_CMP(f1,f2)		(RTK_VOIP_DECT_NUM(f1) == RTK_VOIP_DECT_NUM(f2))
#define	RTK_VOIP_DAA_CMP(f1,f2)			(RTK_VOIP_DAA_NUM(f1) == RTK_VOIP_DAA_NUM(f2))
#define	RTK_VOIP_CODEC_CMP(f1,f2)		(((f1) & RTK_VOIP_CODEC_MASK ) == ((f2) & RTK_VOIP_CODEC_MASK ))
#define	RTK_VOIP_MW_CMP(f1,f2)			(((f1) & ( VOIP_MW_REALTEK | VOIP_MW_AUDIOCODES )) ==	\
										 ((f2) & ( VOIP_MW_REALTEK | VOIP_MW_AUDIOCODES )))
#define	RTK_VOIP_CHECK_IS_IPC_ARCH(f)	((f) & IPC_ARCH_VOIP_SUPPORT)
#define RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP(f)	\
						(((f) & IPC_ARCH_TYPE_MASK) == IPC_ARCH_TYPE_ETHERNET )
#define RTK_VOIP_CHECK_IS_IPC_COPROCESSORDSP(f)	\
						(((f) & IPC_ARCH_TYPE_MASK) == IPC_ARCH_TYPE_COPROCESSOR )
#define RTK_VOIP_CHECK_IS_IPC_HOST(f)	\
						(((f) & IPC_ARCH_ROLE_MASK) == IPC_ARCH_ROLE_IS_HOST )
#define RTK_VOIP_ETHERNET_DSP_HOST_CHECK(f)							\
						(RTK_VOIP_CHECK_IS_IPC_ARCH(f) &&			\
						 RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP(f) &&	\
						 RTK_VOIP_CHECK_IS_IPC_HOST(f))

/* RTK_VOIP_CH_NUM(f) get the SLIC number + real DAA number */

//#define SLIC_NUM			(RTK_VOIP_SLIC_FEATURE + 1)
//#define MAX_SLIC_NUM			(SLIC_NUM_6 + 1)
//#define DECT_NUM			((RTK_VOIP_DECT_FEATURE >> 4) + 1)
//#define DAA_NUM				((RTK_VOIP_DAA_FEATURE >> 6) + 1)
//#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define RTK_VOIP_DSP_DEVICE_NUMBER(f)		( uint32 )(((f) & DSP_DEVICE_NUM_MASK) >> DSP_DEVICE_NUM_SHIFT )
//#else
//#define RTK_VOIP_DSP_DEVICE_NUMBER(f)		0
//#endif
#define RTK_VOIP_CH_NUM(f)	(RTK_VOIP_SLIC_NUM(f) + RTK_VOIP_DAA_NUM(f) +	\
							 RTK_VOIP_DECT_NUM(f) + RTK_VOIP_PHONE_NUM(f))
							 
#define RTK_VOIP_CH_NUM_CMP(f1,f2)	( RTK_VOIP_CH_NUM(f1) == RTK_VOIP_CH_NUM(f2) )

#endif
