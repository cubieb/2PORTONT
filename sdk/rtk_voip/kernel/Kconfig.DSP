# ---------------------------------------------------------------------
# DSP
		
comment "DSP options"
		
config AUDIOCODES_VOIP_
	bool 
	default y
	depends on RTK_VOIP_DRIVERS_PCM8981B_ || RTK_VOIP_DRIVERS_PCM8982B_ || RTK_VOIP_DRIVERS_PCM8984B_ ||	\
		RTK_VOIP_DRIVERS_PCM8961C_ || RTK_VOIP_DRIVERS_PCM8962C_ || RTK_VOIP_DRIVERS_PCM8964C_ ||	\
		RTK_VOIP_DRIVERS_PCM8672 || RTK_VOIP_DRIVERS_PCM8676
		
choice
	prompt "DSP package" 
	default AUDIOCODES_VOIP if AUDIOCODES_VOIP_
		
config REALTEK_VOIP
	bool "Realtek"
			
config AUDIOCODES_VOIP
	bool "Audiocodes"
	depends on AUDIOCODES_VOIP_

endchoice

config AUDIOCODES_VOIP_RELEASE
	bool
	default y
	depends on AUDIOCODES_VOIP
		
config	RTK_VOIP_IVR
	bool
	default y
	select RTK_VOIP_IVR_TEXT
	select RTK_VOIP_IVR_LINEAR_8K
	select RTK_VOIP_IVR_DEV

config	RTK_VOIP_IVR_DEV
	bool 

config	RTK_VOIP_IVR_TEXT
	bool 

config	RTK_VOIP_IVR_LINEAR_8K
	bool 

#config	RTK_VOIP_IVR_LINEAR_16K
#	bool "Linear 16kHz"

source "rtk_voip/Kconfig.DSP.codec"

