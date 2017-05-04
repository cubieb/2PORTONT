#ifndef _REGISTER_MAP_H_
#define _REGISTER_MAP_H_
#include <reg_map_util.h>
//#include <cmu/cmu.h>
#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/platform/9602C/cmu/cmu.h"

/*-----------------------------------------------------
    Hardware Interface: 
 ----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31_26:6; //0
		unsigned int ck25m_en:1; //0
		unsigned int uart0b2_en:1; //0
		unsigned int i2c_slv_en:1; //0
		unsigned int pps_sel:1; //0
		unsigned int sc_cd_en:1; //0
		unsigned int pps_en_0:1; //0
		unsigned int oem_en_0:1; //0
		unsigned int dying_en:1; //0
		unsigned int slic_pcm_en:1; 
		unsigned int slic_zsi_en:1; //0
		unsigned int slic_isi_en:1; //0
		unsigned int i2c_en:2; //0
		unsigned int no_use12_10:3; //0
		unsigned int mdx_m_en:2; //0
		unsigned int mdx_s_en:1; //0
		unsigned int spi_en:1; //0
		unsigned int uart0_en:2; //0
		unsigned int no_use3_0:4; //0
	} f;
	unsigned int v;
} IO_MODE_EN_T;
#define IO_MODE_ENrv (*((regval)0xbb023018))
#define RMOD_IO_MODE_EN(...) rset(IO_MODE_EN, IO_MODE_ENrv, __VA_ARGS__)


/*-----------------------------------------------------
 from System Clock control
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31:25; //0
		unsigned int one_ejtag_sel:1; //0
		unsigned int sys_cpu2_en:1; //0
		unsigned int cf_cksel_lx:1; //1  
		unsigned int cf_ckse_ocp1:1; //1
		unsigned int cf_ckse_ocp0:1; //1  
		unsigned int rdy_for_pathch:1; //0
		unsigned int soc_init_rdy:1; //0
	} f;
	unsigned int v;
} SYS_STATUS_T;
#define SYS_STATUSrv (*((regval)0xb8000044))
#define SYS_STATUSdv (0x0000004C)
#define RMOD_SYS_STATUS(...) rset(SYS_STATUS, SYS_STATUSrv, __VA_ARGS__)
#define RIZS_SYS_STATUS(...) rset(SYS_STATUS, 0, __VA_ARGS__)
#define RFLD_SYS_STATUS(fld) (*((const volatile SYS_STATUS_T *)0xb8000044)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ddrckm90_tap:5; //0
		unsigned int mbz_1:3; //0
		unsigned int ddrckm_tap:5; //0
		unsigned int mbz_2:2; //0
		unsigned int no_use05:1; //0
		unsigned int ddrck_phs_sel:5; //0
	} f;
	unsigned int v;
} DRCKO_T;
#define DRCKOrv (*((regval)0xb800021c))
#define DRCKOdv (0x00000000)
#define RMOD_DRCKO(...) rset(DRCKO, DRCKOrv, __VA_ARGS__)
#define RIZS_DRCKO(...) rset(DRCKO, 0, __VA_ARGS__)
#define RFLD_DRCKO(fld) (*((const volatile DRCKO_T *)0xb800021c)).f.fld




/*-----------------------------------------------------
 Extraced from file_IP_ENABLE.xml
-----------------------------------------------------*/
    typedef union {
    struct {
            unsigned int usbphy1:1;
            unsigned int usbphy0:1;
            unsigned int nfbi_clk_sel:2;
            unsigned int pcm_spd_mode:1;
            unsigned int voip_int_sel:2;
            unsigned int dbg_grp_sel:5;
            unsigned int dbg_sub_grp_sel:4;
            unsigned int en_nandfsf_ctrl:1;
            unsigned int en_nfbi:1;
            unsigned int en_sata:1;
            unsigned int en_voipfft:1;
            unsigned int en_voipacc:1;
            unsigned int en_ipsec:1;
            unsigned int en_gdma1:1;
            unsigned int en_gdma0:1;
            unsigned int en_pcie0:1;
            unsigned int en_pcie1:1;
            unsigned int en_p0usbhost:2;
            unsigned int en_p1usbhost:1;
            unsigned int en_pcm:1;
            unsigned int en_gmac:1;
            unsigned int en_prei_voip:1;
        }f;
        unsigned int v;
    } IP_EN_CTRL_T;
#define IP_EN_CTRLrv (*((regval)0xb8000600))
#define IP_EN_CTRLdv (0x08001b07)
#define RMOD_IP_EN_CTRL(...) rset(IP_EN_CTRL, IP_EN_CTRLrv, __VA_ARGS__)
#define RIZS_IP_EN_CTRL(...) rset(IP_EN_CTRL, 0, __VA_ARGS__)
#define RFLD_IP_EN_CTRL(fld) (*((const volatile IP_EN_CTRL_T *)0xb8000600)).f.fld



typedef union {
	struct {
		unsigned int mbz_31_28:4; //0
		unsigned int mvref0_pd:1; //2
		unsigned int mvref0_pdref:1; //0
		unsigned int mvref0_porsel:2; //2
		unsigned int mbz_23_21:3; //0
        unsigned int pow:1;
        unsigned int bin_phs_sel:5;
        unsigned int pfd_div:6;
        unsigned int sc:3;
        unsigned int sel_ipump:4;
        unsigned int en_enewpump:1;
        unsigned int en_test:1;
	} f;
	unsigned int v;
} ANA_DLL0_T;
#define ANA_DLL0rv (*((regval)0xb8000610))
#define ANA_DLL0dv (0x02148a5e)
#define RMOD_ANA_DLL0(...) rset(ANA_DLL0, ANA_DLL0rv, __VA_ARGS__)
#define RIZS_ANA_DLL0(...) rset(ANA_DLL0, 0, __VA_ARGS__)
#define RFLD_ANA_DLL0(fld) (*((const volatile ANA_DLL0_T *)0xb8000610)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_28:4; //0
		unsigned int mvref1_pd:1; //2
		unsigned int mvref1_pdref:1; //0
		unsigned int mvref1_porsel:2; //2
		unsigned int mbz_23_21:3; //0        
        unsigned int pow:1;
        unsigned int bin_phs_sel:5;
        unsigned int pfd_div:6;
        unsigned int sc:3;
        unsigned int sel_ipump:4;
        unsigned int en_enewpump:1;
        unsigned int en_test:1;
	} f;
	unsigned int v;
} ANA_DLL1_T;
#define ANA_DLL1rv (*((regval)0xb8000614))
#define ANA_DLL1dv (0x02148a5e)
#define RMOD_ANA_DLL1(...) rset(ANA_DLL1, ANA_DLL1rv, __VA_ARGS__)
#define RIZS_ANA_DLL1(...) rset(ANA_DLL1, 0, __VA_ARGS__)
#define RFLD_ANA_DLL1(fld) (*((const volatile ANA_DLL1_T *)0xb8000614)).f.fld



/*-----------------------------------------------------
 Extraced from file_UART.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int rbr_thr_dll:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_PBR_THR_DLL_T;
#define UART_PBR_THR_DLLrv (*((regval)0xb8002000))
#define UART_PBR_THR_DLLdv (0x00000000)
#define RMOD_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, UART_PBR_THR_DLLrv, __VA_ARGS__)
#define RIZS_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, 0, __VA_ARGS__)
#define RFLD_UART_PBR_THR_DLL(fld) (*((const volatile UART_PBR_THR_DLL_T *)0xb8002000)).f.fld

typedef union {
	struct {
		unsigned int ier_dlm:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_IER_DLM_T;
#define UART_IER_DLMrv (*((regval)0xb8002004))
#define UART_IER_DLMdv (0x00000000)
#define RMOD_UART_IER_DLM(...) rset(UART_IER_DLM, UART_IER_DLMrv, __VA_ARGS__)
#define RIZS_UART_IER_DLM(...) rset(UART_IER_DLM, 0, __VA_ARGS__)
#define RFLD_UART_IER_DLM(fld) (*((const volatile UART_IER_DLM_T *)0xb8002004)).f.fld

typedef union {
	struct {
		unsigned int iir_fcr_1:2; //3
		unsigned int mbz_0:2; //0
		unsigned int iir_fcr_0:4; //1
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_IIR_FCR_T;
#define UART_IIR_FCRrv (*((regval)0xb8002008))
#define UART_IIR_FCRdv (0xc1000000)
#define RMOD_UART_IIR_FCR(...) rset(UART_IIR_FCR, UART_IIR_FCRrv, __VA_ARGS__)
#define RIZS_UART_IIR_FCR(...) rset(UART_IIR_FCR, 0, __VA_ARGS__)
#define RFLD_UART_IIR_FCR(fld) (*((const volatile UART_IIR_FCR_T *)0xb8002008)).f.fld

typedef union {
	struct {
		unsigned int dlab:1; //0
		unsigned int brk:1; //0
		unsigned int eps:2; //0
		unsigned int pen:1; //0
		unsigned int stb:1; //0
		unsigned int wls_1:1; //1
		unsigned int wls_0:1; //1
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LCR_T;
#define UART_LCRrv (*((regval)0xb800200c))
#define UART_LCRdv (0x03000000)
#define RMOD_UART_LCR(...) rset(UART_LCR, UART_LCRrv, __VA_ARGS__)
#define RIZS_UART_LCR(...) rset(UART_LCR, 0, __VA_ARGS__)
#define RFLD_UART_LCR(fld) (*((const volatile UART_LCR_T *)0xb800200c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int lxclk_sel:1; //0
		unsigned int afe:1; //0
		unsigned int loop:1; //0
		unsigned int out2:1; //0
		unsigned int out1:1; //0
		unsigned int rts:1; //0
		unsigned int dtr:1; //0
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_MCR_T;
#define UART_MCRrv (*((regval)0xb8002010))
#define UART_MCRdv (0x00000000)
#define RMOD_UART_MCR(...) rset(UART_MCR, UART_MCRrv, __VA_ARGS__)
#define RIZS_UART_MCR(...) rset(UART_MCR, 0, __VA_ARGS__)
#define RFLD_UART_MCR(fld) (*((const volatile UART_MCR_T *)0xb8002010)).f.fld

typedef union {
	struct {
		unsigned int rfe:1; //0
		unsigned int temt:1; //1
		unsigned int thre:1; //1
		unsigned int bi:1; //0
		unsigned int fe:1; //0
		unsigned int pe:1; //0
		unsigned int oe:1; //0
		unsigned int dr:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LSR_T;
#define UART_LSRrv (*((regval)0xb8002014))
#define UART_LSRdv (0x60000000)
#define RMOD_UART_LSR(...) rset(UART_LSR, UART_LSRrv, __VA_ARGS__)
#define RIZS_UART_LSR(...) rset(UART_LSR, 0, __VA_ARGS__)
#define RFLD_UART_LSR(fld) (*((const volatile UART_LSR_T *)0xb8002014)).f.fld

typedef union {
	struct {
		unsigned int dcts:1; //0
		unsigned int ddsr:1; //0
		unsigned int teri:1; //0
		unsigned int ddcd:1; //1
		unsigned int cts:1; //0
		unsigned int dsr:1; //0
		unsigned int ri:1; //0
		unsigned int dcd:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_MSR_T;
#define UART_MSRrv (*((regval)0xb8002018))
#define UART_MSRdv (0x10000000)
#define RMOD_UART_MSR(...) rset(UART_MSR, UART_MSRrv, __VA_ARGS__)
#define RIZS_UART_MSR(...) rset(UART_MSR, 0, __VA_ARGS__)
#define RFLD_UART_MSR(fld) (*((const volatile UART_MSR_T *)0xb8002018)).f.fld

#if 0 //FIX ME: the bit-field
/*-----------------------------------------------------
 Extraced from file_INTR_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int tmo_ie:1; //0
		unsigned int voipacc_ie:1; //0
		unsigned int spi_ie:1; //0
		unsigned int ptm_ie:1; //0
		unsigned int gmac1_ie:1; //0
		unsigned int gmac0_ie:1; //0
		unsigned int no_use25:3; //0
		unsigned int pon_ip:1; //0
		unsigned int gdma1_ie:1; //0
		unsigned int gdma0_ie:1; //0
		unsigned int no_use19:2; //0
		unsigned int pcm_ie:1; //0
		unsigned int no_use16:4; //0
		unsigned int peripheral_ie:1; //0
		unsigned int no_use11:3; //0
		unsigned int switch_ie:1; //0
		unsigned int no_use07:4; //0
		unsigned int fftacc_ie:1; //0
		unsigned int no_use02:3; //0
	} f;
	unsigned int v;
} GIMR0_T;
#define GIMR0rv (*((regval)0xb8003000))
#define GIMR0dv (0x00000000)
#define RMOD_GIMR0(...) rset(GIMR0, GIMR0rv, __VA_ARGS__)
#define RIZS_GIMR0(...) rset(GIMR0, 0, __VA_ARGS__)
#define RFLD_GIMR0(fld) (*((const volatile GIMR0_T *)0xb8003000)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_ie:1; //0
		unsigned int lbctmom1_ie:1; //0
		unsigned int lbctmom0_ie:1; //0
		unsigned int lbctmos3_ie:1; //0
		unsigned int lbctmos2_ie:1; //0
		unsigned int lbctmos1_ie:1; //0
		unsigned int lbctmos0_ie:1; //0
		unsigned int no_use24:1; //0
		unsigned int ocpto0_ie:1; //0
		unsigned int no_use22:1; //0
		unsigned int ocp0_cpu1_ila_ip:1; //0
		unsigned int uart3_ie:1; //0
		unsigned int uart2_ie:1; //0
		unsigned int uart1_ie:1; //0
		unsigned int uart0_ie:1; //0
		unsigned int tc5_ie:1; //0
		unsigned int tc4_ie:1; //0
		unsigned int tc3_ie:1; //0
		unsigned int tc2_ie:1; //0
		unsigned int tc1_ie:1; //0
		unsigned int tc0_ie:1; //0
		unsigned int gpio1_ie:1; //0
		unsigned int gpio0_ie:1; //0
		unsigned int wdt_ph2to_ie:1; //0
		unsigned int wdt_ph1to_ie:1; //0
		unsigned int tc5_dly_int_ie:1; //0
		unsigned int tc4_dly_int_ie:1; //0
		unsigned int tc3_dly_int_ie:1; //0
		unsigned int tc2_dly_int_ie:1; //0
		unsigned int tc1_dly_int_ie:1; //0
		unsigned int tc0_dly_int_ie:1; //0
		unsigned int no_use00:1; //0
	} f;
	unsigned int v;
} GIMR1_T;
#define GIMR1rv (*((regval)0xb8003004))
#define GIMR1dv (0x00000000)
#define RMOD_GIMR1(...) rset(GIMR1, GIMR1rv, __VA_ARGS__)
#define RIZS_GIMR1(...) rset(GIMR1, 0, __VA_ARGS__)
#define RFLD_GIMR1(fld) (*((const volatile GIMR1_T *)0xb8003004)).f.fld

typedef union {
	struct {
		unsigned int tmo_ip:1; //0
		unsigned int voipacc_ip:1; //0
		unsigned int spi_ip:1; //0
		unsigned int xsi_ip:1; //0
		unsigned int gmac1_ip:1; //0
		unsigned int gmac0_ip:1; //0
		unsigned int mbz_0:3; //0
		unsigned int pon_ip:1; //0
		unsigned int gdma1_ip:1; //0
		unsigned int gdma0_ip:1; //0
		unsigned int mbz_1:2; //0
		unsigned int pcm1_ip:1; //0
		unsigned int mbz_2:4; //0
		unsigned int peripheral_ip:1; //0
		unsigned int mbz_3:3; //0
		unsigned int switch_ip:1; //0
		unsigned int mbz_4:4; //0
		unsigned int fftacc_ip:1; //0
		unsigned int mbz_5:3; //0
	} f;
	unsigned int v;
} GISR0_T;
#define GISR0rv (*((regval)0xb8003008))
#define GISR0dv (0x00000000)
#define RMOD_GISR0(...) rset(GISR0, GISR0rv, __VA_ARGS__)
#define RIZS_GISR0(...) rset(GISR0, 0, __VA_ARGS__)
#define RFLD_GISR0(fld) (*((const volatile GISR0_T *)0xb8003008)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_ip:1; //0
		unsigned int lbctmom1_ip:1; //0
		unsigned int lbctmom0_ip:1; //0
		unsigned int lbctmos3_ip:1; //0
		unsigned int lbctmos2_ip:1; //0
		unsigned int lbctmos1_ip:1; //0
		unsigned int lbctmos0_ip:1; //0
		unsigned int ocpto1_ip:1; //0
		unsigned int ocpto0_ip:1; //0
		unsigned int ocp1_cpu0_ila_ip:1; //0
		unsigned int ocp0_cpu1_ila_ip:1; //0
		unsigned int uart3_ip:1; //0
		unsigned int uart2_ip:1; //0
		unsigned int uart1_ip:1; //0
		unsigned int uart0_ip:1; //0
		unsigned int tc5_ip:1; //0
		unsigned int tc4_ip:1; //0
		unsigned int tc3_ip:1; //0
		unsigned int tc2_ip:1; //0
		unsigned int tc1_ip:1; //0
		unsigned int tc0_ip:1; //0
		unsigned int gpio1_ip:1; //0
		unsigned int gpio0_ip:1; //0
		unsigned int wdt_ph2to_ip:1; //0
		unsigned int wdt_ph1to_ip:1; //0
		unsigned int tc5_dly_int_ip:1; //0
		unsigned int tc4_dly_int_ip:1; //0
		unsigned int tc3_dly_int_ip:1; //0
		unsigned int tc2_dly_int_ip:1; //0
		unsigned int tc1_dly_int_ip:1; //0
		unsigned int tc0_dly_int_ip:1; //0
		unsigned int mbz_0:1; //0
	} f;
	unsigned int v;
} GISR1_T;
#define GISR1rv (*((regval)0xb800300c))
#define GISR1dv (0x00000000)
#define RMOD_GISR1(...) rset(GISR1, GISR1rv, __VA_ARGS__)
#define RIZS_GISR1(...) rset(GISR1, 0, __VA_ARGS__)
#define RFLD_GISR1(fld) (*((const volatile GISR1_T *)0xb800300c)).f.fld

typedef union {
	struct {
		unsigned int lbctmom2_rs:4; //0
		unsigned int lbctmom1_rs:4; //0
		unsigned int lbctmom0_rs:4; //0
		unsigned int lbctmos3_rs:4; //0
		unsigned int lbctmos2_rs:4; //0
		unsigned int lbctmos1_rs:4; //0
		unsigned int lbctmos0_rs:4; //0
		unsigned int no_use03:4; //0
	} f;
	unsigned int v;
} IRR0_T;
#define IRR0rv (*((regval)0xb8003010))
#define IRR0dv (0x00000000)
#define RMOD_IRR0(...) rset(IRR0, IRR0rv, __VA_ARGS__)
#define RIZS_IRR0(...) rset(IRR0, 0, __VA_ARGS__)
#define RFLD_IRR0(fld) (*((const volatile IRR0_T *)0xb8003010)).f.fld

typedef union {
	struct {
		unsigned int ocpto0_rs:4; //0
		unsigned int no_use27:4; //0
		unsigned int ocp0_cpu1_ila_rs:4; //0
		unsigned int voipacc_rs:4; //0
		unsigned int spi_rs:4; //0
		unsigned int xsi_rs:4; //0
		unsigned int gmac1_rs:4; //0
		unsigned int gmac0_rs:4; //0
	} f;
	unsigned int v;
} IRR1_T;
#define IRR1rv (*((regval)0xb8003014))
#define IRR1dv (0x00000000)
#define RMOD_IRR1(...) rset(IRR1, IRR1rv, __VA_ARGS__)
#define RIZS_IRR1(...) rset(IRR1, 0, __VA_ARGS__)
#define RFLD_IRR1(fld) (*((const volatile IRR1_T *)0xb8003014)).f.fld

typedef union {
	struct {
		unsigned int no_use31:12; //0
		unsigned int pon_rs:4; //0
		unsigned int gdma1_rs:4; //0
		unsigned int gdma0_rs:4; //0
		unsigned int no_use07:8; //0
	} f;
	unsigned int v;
} IRR2_T;
#define IRR2rv (*((regval)0xb8003018))
#define IRR2dv (0x00000000)
#define RMOD_IRR2(...) rset(IRR2, IRR2rv, __VA_ARGS__)
#define RIZS_IRR2(...) rset(IRR2, 0, __VA_ARGS__)
#define RFLD_IRR2(fld) (*((const volatile IRR2_T *)0xb8003018)).f.fld

typedef union {
	struct {
		unsigned int pcm_rs:4; //0
		unsigned int no_use27:16; //0
		unsigned int uart3_rs:4; //0
		unsigned int uart2_rs:4; //0
		unsigned int uart1_rs:4; //0
	} f;
	unsigned int v;
} IRR3_T;
#define IRR3rv (*((regval)0xb800301c))
#define IRR3dv (0x00000000)
#define RMOD_IRR3(...) rset(IRR3, IRR3rv, __VA_ARGS__)
#define RIZS_IRR3(...) rset(IRR3, 0, __VA_ARGS__)
#define RFLD_IRR3(fld) (*((const volatile IRR3_T *)0xb800301c)).f.fld

typedef union {
	struct {
		unsigned int uart0_rs:4; //0
		unsigned int tc5_rs:4; //0
		unsigned int tc4_rs:4; //0
		unsigned int tc3_rs:4; //0
		unsigned int tc2_rs:4; //0
		unsigned int tc1_rs:4; //0
		unsigned int tc0_rs:4; //0
		unsigned int gpio1_rs:4; //0
	} f;
	unsigned int v;
} IRR4_T;
#define IRR4rv (*((regval)0xb8003020))
#define IRR4dv (0x00000000)
#define RMOD_IRR4(...) rset(IRR4, IRR4rv, __VA_ARGS__)
#define RIZS_IRR4(...) rset(IRR4, 0, __VA_ARGS__)
#define RFLD_IRR4(fld) (*((const volatile IRR4_T *)0xb8003020)).f.fld

typedef union {
	struct {
		unsigned int gpio0_rs:4; //0
		unsigned int switch_rs:4; //0
		unsigned int no_use23:4; //0
		unsigned int wdog_ph2to_rs:4; //0
		unsigned int wdog_ph1to_rs:4; //0
		unsigned int no_use11:4; //0
		unsigned int fftacc_rs:4; //0
		unsigned int no_use03:4; //0
	} f;
	unsigned int v;
} IRR5_T;
#define IRR5rv (*((regval)0xb8003024))
#define IRR5dv (0x00000000)
#define RMOD_IRR5(...) rset(IRR5, IRR5rv, __VA_ARGS__)
#define RIZS_IRR5(...) rset(IRR5, 0, __VA_ARGS__)
#define RFLD_IRR5(fld) (*((const volatile IRR5_T *)0xb8003024)).f.fld
#endif

/*-----------------------------------------------------
 Extraced from file_DLY_INTC.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC0_DLY_INTR_T;
#define TC0_DLY_INTRrv (*((regval)0xb8003080))
#define TC0_DLY_INTRdv (0x00000000)
#define RMOD_TC0_DLY_INTR(...) rset(TC0_DLY_INTR, TC0_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC0_DLY_INTR(...) rset(TC0_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC0_DLY_INTR(fld) (*((const volatile TC0_DLY_INTR_T *)0xb8003080)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC1_DLY_INTR_T;
#define TC1_DLY_INTRrv (*((regval)0xb8003084))
#define TC1_DLY_INTRdv (0x00000000)
#define RMOD_TC1_DLY_INTR(...) rset(TC1_DLY_INTR, TC1_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC1_DLY_INTR(...) rset(TC1_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC1_DLY_INTR(fld) (*((const volatile TC1_DLY_INTR_T *)0xb8003084)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC2_DLY_INTR_T;
#define TC2_DLY_INTRrv (*((regval)0xb8003088))
#define TC2_DLY_INTRdv (0x00000000)
#define RMOD_TC2_DLY_INTR(...) rset(TC2_DLY_INTR, TC2_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC2_DLY_INTR(...) rset(TC2_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC2_DLY_INTR(fld) (*((const volatile TC2_DLY_INTR_T *)0xb8003088)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC3_DLY_INTR_T;
#define TC3_DLY_INTRrv (*((regval)0xb800308c))
#define TC3_DLY_INTRdv (0x00000000)
#define RMOD_TC3_DLY_INTR(...) rset(TC3_DLY_INTR, TC3_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC3_DLY_INTR(...) rset(TC3_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC3_DLY_INTR(fld) (*((const volatile TC3_DLY_INTR_T *)0xb800308c)).f.fld

typedef union {
	struct {
		unsigned int iti_trig:1; //0
		unsigned int no_use30:27; //0
		unsigned int delayed_ip_sel:4; //0
	} f;
	unsigned int v;
} TC4_DLY_INTR_T;
#define TC4_DLY_INTRrv (*((regval)0xb8003090))
#define TC4_DLY_INTRdv (0x00000000)
#define RMOD_TC4_DLY_INTR(...) rset(TC4_DLY_INTR, TC4_DLY_INTRrv, __VA_ARGS__)
#define RIZS_TC4_DLY_INTR(...) rset(TC4_DLY_INTR, 0, __VA_ARGS__)
#define RFLD_TC4_DLY_INTR(fld) (*((const volatile TC4_DLY_INTR_T *)0xb8003090)).f.fld


/*-----------------------------------------------------
 Extraced from file_TIMER.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC0DATA_T;
#define TC0DATArv (*((regval)0xb8003200))
#define TC0DATAdv (0x00000000)
#define RMOD_TC0DATA(...) rset(TC0DATA, TC0DATArv, __VA_ARGS__)
#define RIZS_TC0DATA(...) rset(TC0DATA, 0, __VA_ARGS__)
#define RFLD_TC0DATA(fld) (*((const volatile TC0DATA_T *)0xb8003200)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC0CNTR_T;
#define TC0CNTRrv (*((regval)0xb8003204))
#define TC0CNTRdv (0x00000000)
#define RMOD_TC0CNTR(...) rset(TC0CNTR, TC0CNTRrv, __VA_ARGS__)
#define RIZS_TC0CNTR(...) rset(TC0CNTR, 0, __VA_ARGS__)
#define RFLD_TC0CNTR(fld) (*((const volatile TC0CNTR_T *)0xb8003204)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC0CTRL_T;
#define TC0CTRLrv (*((regval)0xb8003208))
#define TC0CTRLdv (0x00000000)
#define RMOD_TC0CTRL(...) rset(TC0CTRL, TC0CTRLrv, __VA_ARGS__)
#define RIZS_TC0CTRL(...) rset(TC0CTRL, 0, __VA_ARGS__)
#define RFLD_TC0CTRL(fld) (*((const volatile TC0CTRL_T *)0xb8003208)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC0INTR_T;
#define TC0INTRrv (*((regval)0xb800320c))
#define TC0INTRdv (0x00000000)
#define RMOD_TC0INTR(...) rset(TC0INTR, TC0INTRrv, __VA_ARGS__)
#define RIZS_TC0INTR(...) rset(TC0INTR, 0, __VA_ARGS__)
#define RFLD_TC0INTR(fld) (*((const volatile TC0INTR_T *)0xb800320c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC1DATA_T;
#define TC1DATArv (*((regval)0xb8003210))
#define TC1DATAdv (0x00000000)
#define RMOD_TC1DATA(...) rset(TC1DATA, TC1DATArv, __VA_ARGS__)
#define RIZS_TC1DATA(...) rset(TC1DATA, 0, __VA_ARGS__)
#define RFLD_TC1DATA(fld) (*((const volatile TC1DATA_T *)0xb8003210)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC1CNTR_T;
#define TC1CNTRrv (*((regval)0xb8003214))
#define TC1CNTRdv (0x00000000)
#define RMOD_TC1CNTR(...) rset(TC1CNTR, TC1CNTRrv, __VA_ARGS__)
#define RIZS_TC1CNTR(...) rset(TC1CNTR, 0, __VA_ARGS__)
#define RFLD_TC1CNTR(fld) (*((const volatile TC1CNTR_T *)0xb8003214)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC1CTRL_T;
#define TC1CTRLrv (*((regval)0xb8003218))
#define TC1CTRLdv (0x00000000)
#define RMOD_TC1CTRL(...) rset(TC1CTRL, TC1CTRLrv, __VA_ARGS__)
#define RIZS_TC1CTRL(...) rset(TC1CTRL, 0, __VA_ARGS__)
#define RFLD_TC1CTRL(fld) (*((const volatile TC1CTRL_T *)0xb8003218)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC1INTR_T;
#define TC1INTRrv (*((regval)0xb800321c))
#define TC1INTRdv (0x00000000)
#define RMOD_TC1INTR(...) rset(TC1INTR, TC1INTRrv, __VA_ARGS__)
#define RIZS_TC1INTR(...) rset(TC1INTR, 0, __VA_ARGS__)
#define RFLD_TC1INTR(fld) (*((const volatile TC1INTR_T *)0xb800321c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC2DATA_T;
#define TC2DATArv (*((regval)0xb8003220))
#define TC2DATAdv (0x00000000)
#define RMOD_TC2DATA(...) rset(TC2DATA, TC2DATArv, __VA_ARGS__)
#define RIZS_TC2DATA(...) rset(TC2DATA, 0, __VA_ARGS__)
#define RFLD_TC2DATA(fld) (*((const volatile TC2DATA_T *)0xb8003220)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC2CNTR_T;
#define TC2CNTRrv (*((regval)0xb8003224))
#define TC2CNTRdv (0x00000000)
#define RMOD_TC2CNTR(...) rset(TC2CNTR, TC2CNTRrv, __VA_ARGS__)
#define RIZS_TC2CNTR(...) rset(TC2CNTR, 0, __VA_ARGS__)
#define RFLD_TC2CNTR(fld) (*((const volatile TC2CNTR_T *)0xb8003224)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC2CTRL_T;
#define TC2CTRLrv (*((regval)0xb8003228))
#define TC2CTRLdv (0x00000000)
#define RMOD_TC2CTRL(...) rset(TC2CTRL, TC2CTRLrv, __VA_ARGS__)
#define RIZS_TC2CTRL(...) rset(TC2CTRL, 0, __VA_ARGS__)
#define RFLD_TC2CTRL(fld) (*((const volatile TC2CTRL_T *)0xb8003228)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC2INTR_T;
#define TC2INTRrv (*((regval)0xb800322c))
#define TC2INTRdv (0x00000000)
#define RMOD_TC2INTR(...) rset(TC2INTR, TC2INTRrv, __VA_ARGS__)
#define RIZS_TC2INTR(...) rset(TC2INTR, 0, __VA_ARGS__)
#define RFLD_TC2INTR(fld) (*((const volatile TC2INTR_T *)0xb800322c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC3DATA_T;
#define TC3DATArv (*((regval)0xb8003230))
#define TC3DATAdv (0x00000000)
#define RMOD_TC3DATA(...) rset(TC3DATA, TC3DATArv, __VA_ARGS__)
#define RIZS_TC3DATA(...) rset(TC3DATA, 0, __VA_ARGS__)
#define RFLD_TC3DATA(fld) (*((const volatile TC3DATA_T *)0xb8003230)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC3CNTR_T;
#define TC3CNTRrv (*((regval)0xb8003234))
#define TC3CNTRdv (0x00000000)
#define RMOD_TC3CNTR(...) rset(TC3CNTR, TC3CNTRrv, __VA_ARGS__)
#define RIZS_TC3CNTR(...) rset(TC3CNTR, 0, __VA_ARGS__)
#define RFLD_TC3CNTR(fld) (*((const volatile TC3CNTR_T *)0xb8003234)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC3CTRL_T;
#define TC3CTRLrv (*((regval)0xb8003238))
#define TC3CTRLdv (0x00000000)
#define RMOD_TC3CTRL(...) rset(TC3CTRL, TC3CTRLrv, __VA_ARGS__)
#define RIZS_TC3CTRL(...) rset(TC3CTRL, 0, __VA_ARGS__)
#define RFLD_TC3CTRL(fld) (*((const volatile TC3CTRL_T *)0xb8003238)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC3INTR_T;
#define TC3INTRrv (*((regval)0xb800323c))
#define TC3INTRdv (0x00000000)
#define RMOD_TC3INTR(...) rset(TC3INTR, TC3INTRrv, __VA_ARGS__)
#define RIZS_TC3INTR(...) rset(TC3INTR, 0, __VA_ARGS__)
#define RFLD_TC3INTR(fld) (*((const volatile TC3INTR_T *)0xb800323c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC4DATA_T;
#define TC4DATArv (*((regval)0xb8003240))
#define TC4DATAdv (0x00000000)
#define RMOD_TC4DATA(...) rset(TC4DATA, TC4DATArv, __VA_ARGS__)
#define RIZS_TC4DATA(...) rset(TC4DATA, 0, __VA_ARGS__)
#define RFLD_TC4DATA(fld) (*((const volatile TC4DATA_T *)0xb8003240)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC4CNTR_T;
#define TC4CNTRrv (*((regval)0xb8003244))
#define TC4CNTRdv (0x00000000)
#define RMOD_TC4CNTR(...) rset(TC4CNTR, TC4CNTRrv, __VA_ARGS__)
#define RIZS_TC4CNTR(...) rset(TC4CNTR, 0, __VA_ARGS__)
#define RFLD_TC4CNTR(fld) (*((const volatile TC4CNTR_T *)0xb8003244)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC4CTRL_T;
#define TC4CTRLrv (*((regval)0xb8003248))
#define TC4CTRLdv (0x00000000)
#define RMOD_TC4CTRL(...) rset(TC4CTRL, TC4CTRLrv, __VA_ARGS__)
#define RIZS_TC4CTRL(...) rset(TC4CTRL, 0, __VA_ARGS__)
#define RFLD_TC4CTRL(fld) (*((const volatile TC4CTRL_T *)0xb8003248)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC4INTR_T;
#define TC4INTRrv (*((regval)0xb800324c))
#define TC4INTRdv (0x00000000)
#define RMOD_TC4INTR(...) rset(TC4INTR, TC4INTRrv, __VA_ARGS__)
#define RIZS_TC4INTR(...) rset(TC4INTR, 0, __VA_ARGS__)
#define RFLD_TC4INTR(fld) (*((const volatile TC4INTR_T *)0xb800324c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int data:28; //0
	} f;
	unsigned int v;
} TC5DATA_T;
#define TC5DATArv (*((regval)0xb8003250))
#define TC5DATAdv (0x00000000)
#define RMOD_TC5DATA(...) rset(TC5DATA, TC5DATArv, __VA_ARGS__)
#define RIZS_TC5DATA(...) rset(TC5DATA, 0, __VA_ARGS__)
#define RFLD_TC5DATA(fld) (*((const volatile TC5DATA_T *)0xb8003250)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:4; //0
		unsigned int value:28; //0
	} f;
	unsigned int v;
} TC5CNTR_T;
#define TC5CNTRrv (*((regval)0xb8003254))
#define TC5CNTRdv (0x00000000)
#define RMOD_TC5CNTR(...) rset(TC5CNTR, TC5CNTRrv, __VA_ARGS__)
#define RIZS_TC5CNTR(...) rset(TC5CNTR, 0, __VA_ARGS__)
#define RFLD_TC5CNTR(fld) (*((const volatile TC5CNTR_T *)0xb8003254)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TC5CTRL_T;
#define TC5CTRLrv (*((regval)0xb8003258))
#define TC5CTRLdv (0x00000000)
#define RMOD_TC5CTRL(...) rset(TC5CTRL, TC5CTRLrv, __VA_ARGS__)
#define RIZS_TC5CTRL(...) rset(TC5CTRL, 0, __VA_ARGS__)
#define RFLD_TC5CTRL(fld) (*((const volatile TC5CTRL_T *)0xb8003258)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:11; //0
		unsigned int ie:1; //0
		unsigned int mbz_1:3; //0
		unsigned int ip:1; //0
		unsigned int mbz_2:16; //0
	} f;
	unsigned int v;
} TC5INTR_T;
#define TC5INTRrv (*((regval)0xb800325c))
#define TC5INTRdv (0x00000000)
#define RMOD_TC5INTR(...) rset(TC5INTR, TC5INTRrv, __VA_ARGS__)
#define RIZS_TC5INTR(...) rset(TC5INTR, 0, __VA_ARGS__)
#define RFLD_TC5INTR(fld) (*((const volatile TC5INTR_T *)0xb800325c)).f.fld

typedef union {
	struct {
		unsigned int wdt_kick:1; //0
		unsigned int mbz_0:31; //0
	} f;
	unsigned int v;
} WDTCNTRR_T;
#define WDTCNTRRrv (*((regval)0xb8003260))
#define WDTCNTRRdv (0x00000000)
#define RMOD_WDTCNTRR(...) rset(WDTCNTRR, WDTCNTRRrv, __VA_ARGS__)
#define RIZS_WDTCNTRR(...) rset(WDTCNTRR, 0, __VA_ARGS__)
#define RFLD_WDTCNTRR(fld) (*((const volatile WDTCNTRR_T *)0xb8003260)).f.fld

typedef union {
	struct {
		unsigned int ph1_ip:1; //0
		unsigned int ph2_ip:1; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} WDTINTRR_T;
#define WDTINTRRrv (*((regval)0xb8003264))
#define WDTINTRRdv (0x00000000)
#define RMOD_WDTINTRR(...) rset(WDTINTRR, WDTINTRRrv, __VA_ARGS__)
#define RIZS_WDTINTRR(...) rset(WDTINTRR, 0, __VA_ARGS__)
#define RFLD_WDTINTRR(fld) (*((const volatile WDTINTRR_T *)0xb8003264)).f.fld

typedef union {
	struct {
		unsigned int wdt_e:1; //0
		unsigned int wdt_clk_sc:2; //0
		unsigned int mbz_0:2; //0
		unsigned int ph1_to:5; //0
		unsigned int mbz_1:2; //0
		unsigned int ph2_to:5; //0
		unsigned int mbz_2:13; //0
		unsigned int wdt_reset_mode:2; //0
	} f;
	unsigned int v;
} WDT_CTRL_T;
#define WDT_CTRLrv (*((regval)0xb8003268))
#define WDT_CTRLdv (0x00000000)
#define RMOD_WDT_CTRL(...) rset(WDT_CTRL, WDT_CTRLrv, __VA_ARGS__)
#define RIZS_WDT_CTRL(...) rset(WDT_CTRL, 0, __VA_ARGS__)
#define RFLD_WDT_CTRL(fld) (*((const volatile WDT_CTRL_T *)0xb8003268)).f.fld

/*-----------------------------------------------------
 Extraced from file_GPIO.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31:32; //0
	} f;
	unsigned int v;
} GPIO_NO_USE_0004_T;
#define GPIO_NO_USE_0004rv (*((regval)0xb8003304))
#define GPIO_NO_USE_0004dv (0x00000000)
#define RMOD_GPIO_NO_USE_0004(...) rset(GPIO_NO_USE_0004, GPIO_NO_USE_0004rv, __VA_ARGS__)
#define RIZS_GPIO_NO_USE_0004(...) rset(GPIO_NO_USE_0004, 0, __VA_ARGS__)
#define RFLD_GPIO_NO_USE_0004(fld) (*((const volatile GPIO_NO_USE_0004_T *)0xb8003304)).f.fld

typedef union {
	struct {
		unsigned int drc_d:8; //0
		unsigned int drc_c:8; //0
		unsigned int drc_b:8; //0
		unsigned int drc_a:8; //0
	} f;
	unsigned int v;
} PABCD_DIR_T;
#define PABCD_DIRrv (*((regval)0xb8003308))
#define PABCD_DIRdv (0x00000000)
#define RMOD_PABCD_DIR(...) rset(PABCD_DIR, PABCD_DIRrv, __VA_ARGS__)
#define RIZS_PABCD_DIR(...) rset(PABCD_DIR, 0, __VA_ARGS__)
#define RFLD_PABCD_DIR(fld) (*((const volatile PABCD_DIR_T *)0xb8003308)).f.fld

typedef union {
	struct {
		unsigned int pd_d:8; //0
		unsigned int pd_c:8; //0
		unsigned int pd_b:8; //0
		unsigned int pd_a:8; //0
	} f;
	unsigned int v;
} PABCD_DAT_T;
#define PABCD_DATrv (*((regval)0xb800330c))
#define PABCD_DATdv (0x00000000)
#define RMOD_PABCD_DAT(...) rset(PABCD_DAT, PABCD_DATrv, __VA_ARGS__)
#define RIZS_PABCD_DAT(...) rset(PABCD_DAT, 0, __VA_ARGS__)
#define RFLD_PABCD_DAT(fld) (*((const volatile PABCD_DAT_T *)0xb800330c)).f.fld

typedef union {
	struct {
		unsigned int ips_d:8; //0
		unsigned int ips_c:8; //0
		unsigned int ips_b:8; //0
		unsigned int ips_a:8; //0
	} f;
	unsigned int v;
} PABCD_ISR_T;
#define PABCD_ISRrv (*((regval)0xb8003310))
#define PABCD_ISRdv (0x00000000)
#define RMOD_PABCD_ISR(...) rset(PABCD_ISR, PABCD_ISRrv, __VA_ARGS__)
#define RIZS_PABCD_ISR(...) rset(PABCD_ISR, 0, __VA_ARGS__)
#define RFLD_PABCD_ISR(fld) (*((const volatile PABCD_ISR_T *)0xb8003310)).f.fld

typedef union {
	struct {
		unsigned int pb7_im:2; //0
		unsigned int pb6_im:2; //0
		unsigned int pb5_im:2; //0
		unsigned int pb4_im:2; //0
		unsigned int pb3_im:2; //0
		unsigned int pb2_im:2; //0
		unsigned int pb1_im:2; //0
		unsigned int pb0_im:2; //0
		unsigned int pa7_im:2; //0
		unsigned int pa6_im:2; //0
		unsigned int pa5_im:2; //0
		unsigned int pa4_im:2; //0
		unsigned int pa3_im:2; //0
		unsigned int pa2_im:2; //0
		unsigned int pa1_im:2; //0
		unsigned int pa0_im:2; //0
	} f;
	unsigned int v;
} PAB_IMR_T;
#define PAB_IMRrv (*((regval)0xb8003314))
#define PAB_IMRdv (0x00000000)
#define RMOD_PAB_IMR(...) rset(PAB_IMR, PAB_IMRrv, __VA_ARGS__)
#define RIZS_PAB_IMR(...) rset(PAB_IMR, 0, __VA_ARGS__)
#define RFLD_PAB_IMR(fld) (*((const volatile PAB_IMR_T *)0xb8003314)).f.fld

typedef union {
	struct {
		unsigned int pd7_im:2; //0
		unsigned int pd6_im:2; //0
		unsigned int pd5_im:2; //0
		unsigned int pd4_im:2; //0
		unsigned int pd3_im:2; //0
		unsigned int pd2_im:2; //0
		unsigned int pd1_im:2; //0
		unsigned int pd0_im:2; //0
		unsigned int pc7_im:2; //0
		unsigned int pc6_im:2; //0
		unsigned int pc5_im:2; //0
		unsigned int pc4_im:2; //0
		unsigned int pc3_im:2; //0
		unsigned int pc2_im:2; //0
		unsigned int pc1_im:2; //0
		unsigned int pc0_im:2; //0
	} f;
	unsigned int v;
} PCD_IMR_T;
#define PCD_IMRrv (*((regval)0xb8003318))
#define PCD_IMRdv (0x00000000)
#define RMOD_PCD_IMR(...) rset(PCD_IMR, PCD_IMRrv, __VA_ARGS__)
#define RIZS_PCD_IMR(...) rset(PCD_IMR, 0, __VA_ARGS__)
#define RFLD_PCD_IMR(fld) (*((const volatile PCD_IMR_T *)0xb8003318)).f.fld

typedef union {
	struct {
		unsigned int no_use31:32; //0
	} f;
	unsigned int v;
} GPIO_NO_USE_0020_T;
#define GPIO_NO_USE_0020rv (*((regval)0xb8003320))
#define GPIO_NO_USE_0020dv (0x00000000)
#define RMOD_GPIO_NO_USE_0020(...) rset(GPIO_NO_USE_0020, GPIO_NO_USE_0020rv, __VA_ARGS__)
#define RIZS_GPIO_NO_USE_0020(...) rset(GPIO_NO_USE_0020, 0, __VA_ARGS__)
#define RFLD_GPIO_NO_USE_0020(fld) (*((const volatile GPIO_NO_USE_0020_T *)0xb8003320)).f.fld

typedef union {
	struct {
		unsigned int drc_h:8; //0
		unsigned int drc_g:8; //0
		unsigned int drc_f:8; //0
		unsigned int drc_e:8; //0
	} f;
	unsigned int v;
} PEFGH_DIR_T;
#define PEFGH_DIRrv (*((regval)0xb8003324))
#define PEFGH_DIRdv (0x00000000)
#define RMOD_PEFGH_DIR(...) rset(PEFGH_DIR, PEFGH_DIRrv, __VA_ARGS__)
#define RIZS_PEFGH_DIR(...) rset(PEFGH_DIR, 0, __VA_ARGS__)
#define RFLD_PEFGH_DIR(fld) (*((const volatile PEFGH_DIR_T *)0xb8003324)).f.fld

typedef union {
	struct {
		unsigned int ph_h:8; //0
		unsigned int ph_g:8; //0
		unsigned int ph_f:8; //0
		unsigned int ph_e:8; //0
	} f;
	unsigned int v;
} PEFGH_DAT_T;
#define PEFGH_DATrv (*((regval)0xb8003328))
#define PEFGH_DATdv (0x00000000)
#define RMOD_PEFGH_DAT(...) rset(PEFGH_DAT, PEFGH_DATrv, __VA_ARGS__)
#define RIZS_PEFGH_DAT(...) rset(PEFGH_DAT, 0, __VA_ARGS__)
#define RFLD_PEFGH_DAT(fld) (*((const volatile PEFGH_DAT_T *)0xb8003328)).f.fld

typedef union {
	struct {
		unsigned int ips_h:8; //0
		unsigned int ips_g:8; //0
		unsigned int ips_f:8; //0
		unsigned int ips_e:8; //0
	} f;
	unsigned int v;
} PEFGH_ISR_T;
#define PEFGH_ISRrv (*((regval)0xb800332c))
#define PEFGH_ISRdv (0x00000000)
#define RMOD_PEFGH_ISR(...) rset(PEFGH_ISR, PEFGH_ISRrv, __VA_ARGS__)
#define RIZS_PEFGH_ISR(...) rset(PEFGH_ISR, 0, __VA_ARGS__)
#define RFLD_PEFGH_ISR(fld) (*((const volatile PEFGH_ISR_T *)0xb800332c)).f.fld

typedef union {
	struct {
		unsigned int pf7_im:2; //0
		unsigned int pf6_im:2; //0
		unsigned int pf5_im:2; //0
		unsigned int pf4_im:2; //0
		unsigned int pf3_im:2; //0
		unsigned int pf2_im:2; //0
		unsigned int pf1_im:2; //0
		unsigned int pf0_im:2; //0
		unsigned int pe7_im:2; //0
		unsigned int pe6_im:2; //0
		unsigned int pe5_im:2; //0
		unsigned int pe4_im:2; //0
		unsigned int pe3_im:2; //0
		unsigned int pe2_im:2; //0
		unsigned int pe1_im:2; //0
		unsigned int pe0_im:2; //0
	} f;
	unsigned int v;
} PEF_IMR_T;
#define PEF_IMRrv (*((regval)0xb8003330))
#define PEF_IMRdv (0x00000000)
#define RMOD_PEF_IMR(...) rset(PEF_IMR, PEF_IMRrv, __VA_ARGS__)
#define RIZS_PEF_IMR(...) rset(PEF_IMR, 0, __VA_ARGS__)
#define RFLD_PEF_IMR(fld) (*((const volatile PEF_IMR_T *)0xb8003330)).f.fld

typedef union {
	struct {
		unsigned int ph7_im:2; //0
		unsigned int ph6_im:2; //0
		unsigned int ph5_im:2; //0
		unsigned int ph4_im:2; //0
		unsigned int ph3_im:2; //0
		unsigned int ph2_im:2; //0
		unsigned int ph1_im:2; //0
		unsigned int ph0_im:2; //0
		unsigned int pg7_im:2; //0
		unsigned int pg6_im:2; //0
		unsigned int pg5_im:2; //0
		unsigned int pg4_im:2; //0
		unsigned int pg3_im:2; //0
		unsigned int pg2_im:2; //0
		unsigned int pg1_im:2; //0
		unsigned int pg0_im:2; //0
	} f;
	unsigned int v;
} PGH_IMR_T;
#define PGH_IMRrv (*((regval)0xb8003334))
#define PGH_IMRdv (0x00000000)
#define RMOD_PGH_IMR(...) rset(PGH_IMR, PGH_IMRrv, __VA_ARGS__)
#define RIZS_PGH_IMR(...) rset(PGH_IMR, 0, __VA_ARGS__)
#define RFLD_PGH_IMR(fld) (*((const volatile PGH_IMR_T *)0xb8003334)).f.fld

/*-----------------------------------------------------
 Extraced from file_OB_TO_MONT.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int err_indcat:1; //0
		unsigned int to_ctrl_thr:4; //7
		unsigned int mbz_0:26; //0
	} f;
	unsigned int v;
} OCP_TO_CTRL_T;
#define OCP_TO_CTRLrv (*((regval)0xb8005100))
#define OCP_TO_CTRLdv (0x9c000000)
#define RMOD_OCP_TO_CTRL(...) rset(OCP_TO_CTRL, OCP_TO_CTRLrv, __VA_ARGS__)
#define RFLD_OCP_TO_CTRL(fld) (*((const volatile OCP_TO_CTRL_T *)0xb8005100)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} OCP_TO_INTR_T;
#define OCP_TO_INTRrv (*((regval)0xb8005104))
#define OCP_TO_INTRdv (0x00000000)
#define RMOD_OCP_TO_INTR(...) rset(OCP_TO_INTR, OCP_TO_INTRrv, __VA_ARGS__)
#define RFLD_OCP_TO_INTR(fld) (*((const volatile OCP_TO_INTR_T *)0xb8005104)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} OCP_TO_MONT_ADDR_T;
#define OCP_TO_MONT_ADDRrv (*((regval)0xb8005108))
#define OCP_TO_MONT_ADDRdv (0x00000000)
#define RMOD_OCP_TO_MONT_ADDR(...) rset(OCP_TO_MONT_ADDR, OCP_TO_MONT_ADDRrv, __VA_ARGS__)
#define RIZS_OCP_TO_MONT_ADDR(...) rset(OCP_TO_MONT_ADDR, 0, __VA_ARGS__)
#define RFLD_OCP_TO_MONT_ADDR(fld) (*((const volatile OCP_TO_MONT_ADDR_T *)0xb8005108)).f.fld


/*-----------------------------------------------------
 LX_P Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LXP_TO_CTRL_T;
#define LXP_TO_CTRLrv (*((regval)0xb8005200))
#define LXP_TO_CTRLdv (0xF0000000)
#define RMOD_LXP_TO_CTRL(...) rset(LXP_TO_CTRL, LXP_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LXP_TO_CTRL(fld) (*((const volatile LXP_TO_CTRL_T *)0xb8005200)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LXP_TO_INTR_T;
#define LXP_TO_INTRrv (*((regval)0xb8005204))
#define LXP_TO_INTRdv (0x00000000)
#define RMOD_LXP_TO_INTR(...) rset(LXP_TO_INTR, LXP_TO_INTRrv, __VA_ARGS__)
#define RFLD_LXP_TO_INTR(fld) (*((const volatile LXP_TO_INTR_T *)0xb8005204)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LXP_TO_MONT_ADDR_T;
#define LXP_TO_MONT_ADDRrv (*((regval)0xb8005208))
#define LXP_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LXP_TO_MONT_ADDR(...) rset(LXP_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LXP_TO_MONT_ADDR(fld) (*((const volatile LXP_TO_MONT_ADDR_T *)0xb8005208)).f.fld


/*-----------------------------------------------------
 Mater LX_0 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX0_M_TO_CTRL_T;
#define LX0_M_TO_CTRLrv (*((regval)0xb8005210))
#define LX0_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX0_M_TO_CTRL(...) rset(LX0_M_TO_CTRL, LX0_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_CTRL(fld) (*((const volatile LX0_M_TO_CTRL_T *)0xb8005210)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX0_M_TO_INTR_T;
#define LX0_M_TO_INTRrv (*((regval)0xb8005214))
#define LX0_M_TO_INTRdv (0x00000000)
#define RMOD_LX0_M_TO_INTR(...) rset(LX0_M_TO_INTR, LX0_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_INTR(fld) (*((const volatile LX0_M_TO_INTR_T *)0xb8005214)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX0_M_TO_MONT_ADDR_T;
#define LX0_M_TO_MONT_ADDRrv (*((regval)0xb8005218))
#define LX0_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX0_M_TO_MONT_ADDR(...) rset(LX0_M_TO_MONT_ADDR, LX0_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_MONT_ADDR(fld) (*((const volatile LX0_M_TO_MONT_ADDR_T *)0xb8005218)).f.fld


/*-----------------------------------------------------
 Slave LX_0 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX0_S_TO_CTRL_T;
#define LX0_S_TO_CTRLrv (*((regval)0xb8005220))
#define LX0_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX0_S_TO_CTRL(...) rset(LX0_S_TO_CTRL, LX0_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_CTRL(fld) (*((const volatile LX0_S_TO_CTRL_T *)0xb8005220)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX0_S_TO_INTR_T;
#define LX0_S_TO_INTRrv (*((regval)0xb8005224))
#define LX0_S_TO_INTRdv (0x00000000)
#define RMOD_LX0_S_TO_INTR(...) rset(LX0_S_TO_INTR, LX0_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_INTR(fld) (*((const volatile LX0_S_TO_INTR_T *)0xb8005224)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX0_S_TO_MONT_ADDR_T;
#define LX0_S_TO_MONT_ADDRrv (*((regval)0xb8005228))
#define LX0_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX0_S_TO_MONT_ADDR(...) rset(LX0_S_TO_MONT_ADDR, LX0_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_MONT_ADDR(fld) (*((const volatile LX0_S_TO_MONT_ADDR_T *)0xb8005228)).f.fld


/*-----------------------------------------------------
 Mater LX_1 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX1_M_TO_CTRL_T;
#define LX1_M_TO_CTRLrv (*((regval)0xb8005230))
#define LX1_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX1_M_TO_CTRL(...) rset(LX1_M_TO_CTRL, LX1_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_CTRL(fld) (*((const volatile LX1_M_TO_CTRL_T *)0xb8005230)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX1_M_TO_INTR_T;
#define LX1_M_TO_INTRrv (*((regval)0xb8005234))
#define LX1_M_TO_INTRdv (0x00000000)
#define RMOD_LX1_M_TO_INTR(...) rset(LX1_M_TO_INTR, LX1_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_INTR(fld) (*((const volatile LX1_M_TO_INTR_T *)0xb8005234)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX1_M_TO_MONT_ADDR_T;
#define LX1_M_TO_MONT_ADDRrv (*((regval)0xb8005238))
#define LX1_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX1_M_TO_MONT_ADDR(...) rset(LX1_M_TO_MONT_ADDR, LX1_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_MONT_ADDR(fld) (*((const volatile LX1_M_TO_MONT_ADDR_T *)0xb8005238)).f.fld


/*-----------------------------------------------------
 Slave LX_1 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX1_S_TO_CTRL_T;
#define LX1_S_TO_CTRLrv (*((regval)0xb8005240))
#define LX1_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX1_S_TO_CTRL(...) rset(LX1_S_TO_CTRL, LX1_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_CTRL(fld) (*((const volatile LX1_S_TO_CTRL_T *)0xb8005240)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX1_S_TO_INTR_T;
#define LX1_S_TO_INTRrv (*((regval)0xb8005244))
#define LX1_S_TO_INTRdv (0x00000000)
#define RMOD_LX1_S_TO_INTR(...) rset(LX1_S_TO_INTR, LX1_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_INTR(fld) (*((const volatile LX1_S_TO_INTR_T *)0xb8005244)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX1_S_TO_MONT_ADDR_T;
#define LX1_S_TO_MONT_ADDRrv (*((regval)0xb8005248))
#define LX1_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX1_S_TO_MONT_ADDR(...) rset(LX1_S_TO_MONT_ADDR, LX1_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_MONT_ADDR(fld) (*((const volatile LX1_S_TO_MONT_ADDR_T *)0xb8005248)).f.fld



/*-----------------------------------------------------
 Mater LX2 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX2_M_TO_CTRL_T;
#define LX2_M_TO_CTRLrv (*((regval)0xb8005250))
#define LX2_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX2_M_TO_CTRL(...) rset(LX2_M_TO_CTRL, LX2_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_CTRL(fld) (*((const volatile LX2_M_TO_CTRL_T *)0xb8005250)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX2_M_TO_INTR_T;
#define LX2_M_TO_INTRrv (*((regval)0xb8005254))
#define LX2_M_TO_INTRdv (0x00000000)
#define RMOD_LX2_M_TO_INTR(...) rset(LX2_M_TO_INTR, LX2_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_INTR(fld) (*((const volatile LX2_M_TO_INTR_T *)0xb8005254)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX2_M_TO_MONT_ADDR_T;
#define LX2_M_TO_MONT_ADDRrv (*((regval)0xb8005258))
#define LX2_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX2_M_TO_MONT_ADDR(...) rset(LX2_M_TO_MONT_ADDR, LX2_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_MONT_ADDR(fld) (*((const volatile LX2_M_TO_MONT_ADDR_T *)0xb8005258)).f.fld


/*-----------------------------------------------------
 Slave LX2 Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX2_S_TO_CTRL_T;
#define LX2_S_TO_CTRLrv (*((regval)0xb8005260))
#define LX2_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX2_S_TO_CTRL(...) rset(LX2_S_TO_CTRL, LX2_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_CTRL(fld) (*((const volatile LX2_S_TO_CTRL_T *)0xb8005260)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX2_S_TO_INTR_T;
#define LX2_S_TO_INTRrv (*((regval)0xb8005264))
#define LX2_S_TO_INTRdv (0x00000000)
#define RMOD_LX2_S_TO_INTR(...) rset(LX2_S_TO_INTR, LX2_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_INTR(fld) (*((const volatile LX2_S_TO_INTR_T *)0xb8005264)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX2_S_TO_MONT_ADDR_T;
#define LX2_S_TO_MONT_ADDRrv (*((regval)0xb8005268))
#define LX2_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX2_S_TO_MONT_ADDR(...) rset(LX2_S_TO_MONT_ADDR, LX2_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_MONT_ADDR(fld) (*((const volatile LX2_S_TO_MONT_ADDR_T *)0xb8005268)).f.fld


/*-----------------------------------------------------
   LX_PBO_USW Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX_PBO_USW_TO_CTRL_T;
#define LX_PBO_USW_TO_CTRLrv (*((regval)0xb8005270))
#define LX_PBO_USW_TO_CTRLdv (0xF0000000)
#define RMOD_LX_PBO_USW_TO_CTRL(...) rset(LX_PBO_USW_TO_CTRL, LX_PBO_USW_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX_PBO_USW_TO_CTRL(fld) (*((const volatile LX_PBO_USW_TO_CTRL_T *)0xb8005270)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX_PBO_USW_TO_INTR_T;
#define LX_PBO_USW_TO_INTRrv (*((regval)0xb8005274))
#define LX_PBO_USW_TO_INTRdv (0x00000000)
#define RMOD_LX_PBO_USW_TO_INTR(...) rset(LX_PBO_USW_TO_INTR, LX_PBO_USW_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX_PBO_USW_TO_INTR(fld) (*((const volatile LX_PBO_USW_TO_INTR_T *)0xb8005274)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX_PBO_USW_TO_MONT_ADDR_T;
#define LX_PBO_USW_TO_MONT_ADDRrv (*((regval)0xb8005278))
#define LX_PBO_USW_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX_PBO_USW_TO_MONT_ADDR(...) rset(LX_PBO_USW_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX_PBO_USW_TO_MONT_ADDR(fld) (*((const volatile LX_PBO_USW_TO_MONT_ADDR_T *)0xb8005278)).f.fld


/*-----------------------------------------------------
   LX_PBO_USR Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX_PBO_USR_TO_CTRL_T;
#define LX_PBO_USR_TO_CTRLrv (*((regval)0xb8005280))
#define LX_PBO_USR_TO_CTRLdv (0xF0000000)
#define RMOD_LX_PBO_USR_TO_CTRL(...) rset(LX_PBO_USR_TO_CTRL, LX_PBO_USR_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX_PBO_USR_TO_CTRL(fld) (*((const volatile LX_PBO_USR_TO_CTRL_T *)0xb8005280)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX_PBO_USR_TO_INTR_T;
#define LX_PBO_USR_TO_INTRrv (*((regval)0xb8005284))
#define LX_PBO_USR_TO_INTRdv (0x00000000)
#define RMOD_LX_PBO_USR_TO_INTR(...) rset(LX_PBO_USR_TO_INTR, LX_PBO_USR_TO_INTRrv, __VA_ARGS__)
#define RIZS_LX_PBO_USR_TO_INTR(...) rset(LX_PBO_USR_TO_INTR, 0, __VA_ARGS__)

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX_PBO_USR_TO_MONT_ADDR_T;
#define LX_PBO_USR_TO_MONT_ADDRrv (*((regval)0xb8005288))
#define LX_PBO_USR_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX_PBO_USR_TO_MONT_ADDR(...) rset(LX_PBO_USR_TO_MONT_ADDR, LX_PBO_USR_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX_PBO_USR_TO_MONT_ADDR(fld) (*((const volatile LX_PBO_USR_TO_MONT_ADDR_T *)0xb8005288)).f.fld


/*-----------------------------------------------------
   LX_PBO_DSW Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX_PBO_DSW_TO_CTRL_T;
#define LX_PBO_DSW_TO_CTRLrv (*((regval)0xb8005290))
#define LX_PBO_DSW_TO_CTRLdv (0xF0000000)
#define RMOD_LX_PBO_DSW_TO_CTRL(...) rset(LX_PBO_DSW_TO_CTRL, LX_PBO_DSW_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSW_TO_CTRL(fld) (*((const volatile LX_PBO_DSW_TO_CTRL_T *)0xb8005290)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX_PBO_DSW_TO_INTR_T;
#define LX_PBO_DSW_TO_INTRrv (*((regval)0xb8005294))
#define LX_PBO_DSW_TO_INTRdv (0x00000000)
#define RMOD_LX_PBO_DSW_TO_INTR(...) rset(LX_PBO_DSW_TO_INTR, LX_PBO_DSW_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSW_TO_INTR(fld) (*((const volatile LX_PBO_DSW_TO_INTR_T *)0xb8005294)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX_PBO_DSW_TO_MONT_ADDR_T;
#define LX_PBO_DSW_TO_MONT_ADDRrv (*((regval)0xb8005298))
#define LX_PBO_DSW_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX_PBO_DSW_TO_MONT_ADDR(...) rset(LX_PBO_DSW_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSW_TO_MONT_ADDR(fld) (*((const volatile LX_PBO_DSW_TO_MONT_ADDR_T *)0xb8005298)).f.fld



/*-----------------------------------------------------
   LX_PBO_DSR Bus Timeout Monitor
-----------------------------------------------------*/
typedef union {
    struct {
        unsigned int to_ctrl_en:1; //1
        unsigned int to_ctrl_thr:3; //7
        unsigned int mbz_0:28; //0
    } f;
    unsigned int v;
} LX_PBO_DSR_TO_CTRL_T;
#define LX_PBO_DSR_TO_CTRLrv (*((regval)0xb80052A0))
#define LX_PBO_DSR_TO_CTRLdv (0xF0000000)
#define RMOD_LX_PBO_DSR_TO_CTRL(...) rset(LX_PBO_DSR_TO_CTRL, LX_PBO_DSR_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSR_TO_CTRL(fld) (*((const volatile LX_PBO_DSR_TO_CTRL_T *)0xb80052A0)).f.fld

typedef union {
    struct {
        unsigned int to_ip:2; //0
        unsigned int mbz_0:30; //0
    } f;
    unsigned int v;
} LX_PBO_DSR_TO_INTR_T;
#define LX_PBO_DSR_TO_INTRrv (*((regval)0xb80052A4))
#define LX_PBO_DSR_TO_INTRdv (0x00000000)
#define RMOD_LX_PBO_DSR_TO_INTR(...) rset(LX_PBO_DSR_TO_INTR, LX_PBO_DSR_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSR_TO_INTR(fld) (*((const volatile LX_PBO_DSR_TO_INTR_T *)0xb80052A4)).f.fld

typedef union {
    struct {
        unsigned int to_addr:32; //0
    } f;
    unsigned int v;
} LX_PBO_DSR_TO_MONT_ADDR_T;
#define LX_PBO_DSR_TO_MONT_ADDRrv (*((regval)0xb80052A8))
#define LX_PBO_DSR_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX_PBO_DSR_TO_MONT_ADDR(...) rset(LX_PBO_DSR_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX_PBO_DSR_TO_MONT_ADDR(fld) (*((const volatile LX_PBO_DSR_TO_MONT_ADDR_T *)0xb80052A8)).f.fld



/*-----------------------------------------------------
 Extraced from file_BUS_TRAF_GEN.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int poll:1; //0
		unsigned int bus_traf_gen:2; //0
		unsigned int func:4; //0
		unsigned int mbz_0:1; //0
		unsigned int swap_type:1; //0
		unsigned int ent_size:2; //0
		unsigned int mbz_1:10; //0
		unsigned int dis_check_ddr:1; //0
		unsigned int debug_sel:1; //0
		unsigned int burst_size:2; //0
		unsigned int enough_thr:2; //0
		unsigned int dst_low_thr:2; //0
		unsigned int src_low_th:2; //0
	} f;
	unsigned int v;
} GDMA_CTRL_T;
#define GDMA_CTRLrv (*((regval)0xb800a000))
#define GDMA_CTRLdv (0x00000000)
#define RMOD_GDMA_CTRL(...) rset(GDMA_CTRL, GDMA_CTRLrv, __VA_ARGS__)
#define RIZS_GDMA_CTRL(...) rset(GDMA_CTRL, 0, __VA_ARGS__)
#define RFLD_GDMA_CTRL(fld) (*((const volatile GDMA_CTRL_T *)0xb800a000)).f.fld

typedef union {
	struct {
		unsigned int comp_ie:1; //0
		unsigned int mbz_0:2; //0
		unsigned int need_cpu_ie:1; //0
		unsigned int rd_pkt_ie:1; //0
		unsigned int err_rchk_ie:1; //0
		unsigned int err_rprd_ie:1; //0
		unsigned int err_rrp_ie:1; //0
		unsigned int wr_pkt_ie:1; //0
		unsigned int mbz_1:1; //0
		unsigned int err_wprd_ie:1; //0
		unsigned int err_wrp_ie:1; //0
		unsigned int mbz_2:20; //0
	} f;
	unsigned int v;
} GDMA_IM_T;
#define GDMA_IMrv (*((regval)0xb800a004))
#define GDMA_IMdv (0x00000000)
#define RMOD_GDMA_IM(...) rset(GDMA_IM, GDMA_IMrv, __VA_ARGS__)
#define RIZS_GDMA_IM(...) rset(GDMA_IM, 0, __VA_ARGS__)
#define RFLD_GDMA_IM(fld) (*((const volatile GDMA_IM_T *)0xb800a004)).f.fld

typedef union {
	struct {
		unsigned int comp_ip:1; //0
		unsigned int mbz_0:2; //0
		unsigned int need_cpu_ip:1; //0
		unsigned int rd_pkt_ip:1; //0
		unsigned int err_rchk_ip:1; //0
		unsigned int err_rprd_ip:1; //0
		unsigned int err_rrp_ip:1; //0
		unsigned int wr_pkt_ip:1; //0
		unsigned int mbz_1:1; //0
		unsigned int err_wprd_ip:1; //0
		unsigned int err_wrp_ip:1; //0
		unsigned int mbz_2:20; //0
	} f;
	unsigned int v;
} GDMA_IS_T;
#define GDMA_ISrv (*((regval)0xb800a008))
#define GDMA_ISdv (0x00000000)
#define RMOD_GDMA_IS(...) rset(GDMA_IS, GDMA_ISrv, __VA_ARGS__)
#define RIZS_GDMA_IS(...) rset(GDMA_IS, 0, __VA_ARGS__)
#define RFLD_GDMA_IS(fld) (*((const volatile GDMA_IS_T *)0xb800a008)).f.fld

typedef union {
	struct {
		unsigned int icvl:32; //0
	} f;
	unsigned int v;
} GDMA_ICVL_T;
#define GDMA_ICVLrv (*((regval)0xb800a00c))
#define GDMA_ICVLdv (0x00000000)
#define RMOD_GDMA_ICVL(...) rset(GDMA_ICVL, GDMA_ICVLrv, __VA_ARGS__)
#define RIZS_GDMA_ICVL(...) rset(GDMA_ICVL, 0, __VA_ARGS__)
#define RFLD_GDMA_ICVL(fld) (*((const volatile GDMA_ICVL_T *)0xb800a00c)).f.fld

typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int no_use30:8; //0
		unsigned int rec_err:1; //0
		unsigned int prec:1; //0
		unsigned int infinite:1; //0
		unsigned int no_use19:2; //0
		unsigned int burst_len:2; //0
		unsigned int no_use15:3; //0
		unsigned int pkt_len:8; //0
		unsigned int no_use04:5; //0
	} f;
	unsigned int v;
} BTG_WRITE_CTRL_T;
#define BTG_WRITE_CTRLrv (*((regval)0xb800a100))
#define BTG_WRITE_CTRLdv (0x00000000)
#define RMOD_BTG_WRITE_CTRL(...) rset(BTG_WRITE_CTRL, BTG_WRITE_CTRLrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_CTRL(...) rset(BTG_WRITE_CTRL, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_CTRL(fld) (*((const volatile BTG_WRITE_CTRL_T *)0xb800a100)).f.fld

typedef union {
	struct {
		unsigned int iter:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ITER_T;
#define BTG_WRITE_ITERrv (*((regval)0xb800a104))
#define BTG_WRITE_ITERdv (0x00000000)
#define RMOD_BTG_WRITE_ITER(...) rset(BTG_WRITE_ITER, BTG_WRITE_ITERrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ITER(...) rset(BTG_WRITE_ITER, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ITER(fld) (*((const volatile BTG_WRITE_ITER_T *)0xb800a104)).f.fld

typedef union {
	struct {
		unsigned int resp_time:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_RESP_TIME_T;
#define BTG_WRITE_RESP_TIMErv (*((regval)0xb800a108))
#define BTG_WRITE_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_WRITE_RESP_TIME(...) rset(BTG_WRITE_RESP_TIME, BTG_WRITE_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_WRITE_RESP_TIME(...) rset(BTG_WRITE_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_RESP_TIME(fld) (*((const volatile BTG_WRITE_RESP_TIME_T *)0xb800a108)).f.fld

typedef union {
	struct {
		unsigned int period:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_PERI_T;
#define BTG_WRITE_PERIrv (*((regval)0xb800a10c))
#define BTG_WRITE_PERIdv (0x00000000)
#define RMOD_BTG_WRITE_PERI(...) rset(BTG_WRITE_PERI, BTG_WRITE_PERIrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_PERI(...) rset(BTG_WRITE_PERI, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_PERI(fld) (*((const volatile BTG_WRITE_PERI_T *)0xb800a10c)).f.fld

typedef union {
	struct {
		unsigned int base_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ADDR_T;
#define BTG_WRITE_ADDRrv (*((regval)0xb800a110))
#define BTG_WRITE_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_ADDR(...) rset(BTG_WRITE_ADDR, BTG_WRITE_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ADDR(...) rset(BTG_WRITE_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ADDR(fld) (*((const volatile BTG_WRITE_ADDR_T *)0xb800a110)).f.fld

typedef union {
	struct {
		unsigned int addr_msk:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_ADDR_MASK_T;
#define BTG_WRITE_ADDR_MASKrv (*((regval)0xb800a114))
#define BTG_WRITE_ADDR_MASKdv (0x00000000)
#define RMOD_BTG_WRITE_ADDR_MASK(...) rset(BTG_WRITE_ADDR_MASK, BTG_WRITE_ADDR_MASKrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_ADDR_MASK(...) rset(BTG_WRITE_ADDR_MASK, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_ADDR_MASK(fld) (*((const volatile BTG_WRITE_ADDR_MASK_T *)0xb800a114)).f.fld

typedef union {
	struct {
		unsigned int dec_gap:1; //0
		unsigned int no_use30:15; //0
		unsigned int addr_gap:16; //0
	} f;
	unsigned int v;
} BTG_WRITE_GAP_T;
#define BTG_WRITE_GAPrv (*((regval)0xb800a118))
#define BTG_WRITE_GAPdv (0x00000000)
#define RMOD_BTG_WRITE_GAP(...) rset(BTG_WRITE_GAP, BTG_WRITE_GAPrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_GAP(...) rset(BTG_WRITE_GAP, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_GAP(fld) (*((const volatile BTG_WRITE_GAP_T *)0xb800a118)).f.fld

typedef union {
	struct {
		unsigned int cur_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_CURR_ADDR_T;
#define BTG_WRITE_CURR_ADDRrv (*((regval)0xb800a11c))
#define BTG_WRITE_CURR_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_CURR_ADDR(...) rset(BTG_WRITE_CURR_ADDR, BTG_WRITE_CURR_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_CURR_ADDR(...) rset(BTG_WRITE_CURR_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_CURR_ADDR(fld) (*((const volatile BTG_WRITE_CURR_ADDR_T *)0xb800a11c)).f.fld

typedef union {
	struct {
		unsigned int resp_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_RESP_FAIL_ADDR_T;
#define BTG_WRITE_RESP_FAIL_ADDRrv (*((regval)0xb800a120))
#define BTG_WRITE_RESP_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_RESP_FAIL_ADDR(...) rset(BTG_WRITE_RESP_FAIL_ADDR, BTG_WRITE_RESP_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_RESP_FAIL_ADDR(...) rset(BTG_WRITE_RESP_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_RESP_FAIL_ADDR(fld) (*((const volatile BTG_WRITE_RESP_FAIL_ADDR_T *)0xb800a120)).f.fld

typedef union {
	struct {
		unsigned int peri_addr:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_PERI_FAIL_ADDR_T;
#define BTG_WRITE_PERI_FAIL_ADDRrv (*((regval)0xb800a124))
#define BTG_WRITE_PERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_WRITE_PERI_FAIL_ADDR(...) rset(BTG_WRITE_PERI_FAIL_ADDR, BTG_WRITE_PERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_WRITE_PERI_FAIL_ADDR(...) rset(BTG_WRITE_PERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_PERI_FAIL_ADDR(fld) (*((const volatile BTG_WRITE_PERI_FAIL_ADDR_T *)0xb800a124)).f.fld

typedef union {
	struct {
		unsigned int max_resp_time:32; //0
	} f;
	unsigned int v;
} BTG_WRITE_MAX_RESP_TIME_T;
#define BTG_WRITE_MAX_RESP_TIMErv (*((regval)0xb800a12c))
#define BTG_WRITE_MAX_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_WRITE_MAX_RESP_TIME(...) rset(BTG_WRITE_MAX_RESP_TIME, BTG_WRITE_MAX_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_WRITE_MAX_RESP_TIME(...) rset(BTG_WRITE_MAX_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_WRITE_MAX_RESP_TIME(fld) (*((const volatile BTG_WRITE_MAX_RESP_TIME_T *)0xb800a12c)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_WRITE_IN_VEC_T;
#define BTC_WRITE_IN_VECrv (*((regval)0xb800a130))
#define BTC_WRITE_IN_VECdv (0x00000000)
#define RMOD_BTC_WRITE_IN_VEC(...) rset(BTC_WRITE_IN_VEC, BTC_WRITE_IN_VECrv, __VA_ARGS__)
#define RIZS_BTC_WRITE_IN_VEC(...) rset(BTC_WRITE_IN_VEC, 0, __VA_ARGS__)
#define RFLD_BTC_WRITE_IN_VEC(fld) (*((const volatile BTC_WRITE_IN_VEC_T *)0xb800a130)).f.fld

typedef union {
	struct {
		unsigned int enable:1; //0
		unsigned int no_use30:5; //0
		unsigned int chk_mode:2; //0
		unsigned int no_use23:1; //0
		unsigned int rec_err:1; //0
		unsigned int prec:1; //0
		unsigned int infinite:1; //0
		unsigned int no_use19:2; //0
		unsigned int burst_len:2; //0
		unsigned int no_use15:3; //0
		unsigned int pkt_len:8; //0
		unsigned int no_use04:5; //0
	} f;
	unsigned int v;
} BTG_READ_CTRL_T;
#define BTG_READ_CTRLrv (*((regval)0xb800a200))
#define BTG_READ_CTRLdv (0x00000000)
#define RMOD_BTG_READ_CTRL(...) rset(BTG_READ_CTRL, BTG_READ_CTRLrv, __VA_ARGS__)
#define RIZS_BTG_READ_CTRL(...) rset(BTG_READ_CTRL, 0, __VA_ARGS__)
#define RFLD_BTG_READ_CTRL(fld) (*((const volatile BTG_READ_CTRL_T *)0xb800a200)).f.fld

typedef union {
	struct {
		unsigned int iter:32; //0
	} f;
	unsigned int v;
} BTG_READ_ITER_T;
#define BTG_READ_ITERrv (*((regval)0xb800a204))
#define BTG_READ_ITERdv (0x00000000)
#define RMOD_BTG_READ_ITER(...) rset(BTG_READ_ITER, BTG_READ_ITERrv, __VA_ARGS__)
#define RIZS_BTG_READ_ITER(...) rset(BTG_READ_ITER, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ITER(fld) (*((const volatile BTG_READ_ITER_T *)0xb800a204)).f.fld

typedef union {
	struct {
		unsigned int resp_time:32; //0
	} f;
	unsigned int v;
} BTG_READ_RESP_TIME_T;
#define BTG_READ_RESP_TIMErv (*((regval)0xb800a208))
#define BTG_READ_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_READ_RESP_TIME(...) rset(BTG_READ_RESP_TIME, BTG_READ_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_READ_RESP_TIME(...) rset(BTG_READ_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_READ_RESP_TIME(fld) (*((const volatile BTG_READ_RESP_TIME_T *)0xb800a208)).f.fld

typedef union {
	struct {
		unsigned int period:32; //0
	} f;
	unsigned int v;
} BTG_READ_PERI_T;
#define BTG_READ_PERIrv (*((regval)0xb800a20c))
#define BTG_READ_PERIdv (0x00000000)
#define RMOD_BTG_READ_PERI(...) rset(BTG_READ_PERI, BTG_READ_PERIrv, __VA_ARGS__)
#define RIZS_BTG_READ_PERI(...) rset(BTG_READ_PERI, 0, __VA_ARGS__)
#define RFLD_BTG_READ_PERI(fld) (*((const volatile BTG_READ_PERI_T *)0xb800a20c)).f.fld

typedef union {
	struct {
		unsigned int base_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_ADDR_T;
#define BTG_READ_ADDRrv (*((regval)0xb800a210))
#define BTG_READ_ADDRdv (0x00000000)
#define RMOD_BTG_READ_ADDR(...) rset(BTG_READ_ADDR, BTG_READ_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_ADDR(...) rset(BTG_READ_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ADDR(fld) (*((const volatile BTG_READ_ADDR_T *)0xb800a210)).f.fld

typedef union {
	struct {
		unsigned int addr_msk:32; //0
	} f;
	unsigned int v;
} BTG_READ_ADDR_MASK_T;
#define BTG_READ_ADDR_MASKrv (*((regval)0xb800a214))
#define BTG_READ_ADDR_MASKdv (0x00000000)
#define RMOD_BTG_READ_ADDR_MASK(...) rset(BTG_READ_ADDR_MASK, BTG_READ_ADDR_MASKrv, __VA_ARGS__)
#define RIZS_BTG_READ_ADDR_MASK(...) rset(BTG_READ_ADDR_MASK, 0, __VA_ARGS__)
#define RFLD_BTG_READ_ADDR_MASK(fld) (*((const volatile BTG_READ_ADDR_MASK_T *)0xb800a214)).f.fld

typedef union {
	struct {
		unsigned int dec_gap:1; //0
		unsigned int no_use30:15; //0
		unsigned int addr_gap:16; //0
	} f;
	unsigned int v;
} BTG_READ_GAP_T;
#define BTG_READ_GAPrv (*((regval)0xb800a218))
#define BTG_READ_GAPdv (0x00000000)
#define RMOD_BTG_READ_GAP(...) rset(BTG_READ_GAP, BTG_READ_GAPrv, __VA_ARGS__)
#define RIZS_BTG_READ_GAP(...) rset(BTG_READ_GAP, 0, __VA_ARGS__)
#define RFLD_BTG_READ_GAP(fld) (*((const volatile BTG_READ_GAP_T *)0xb800a218)).f.fld

typedef union {
	struct {
		unsigned int cur_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_CURR_ADDR_T;
#define BTG_READ_CURR_ADDRrv (*((regval)0xb800a21c))
#define BTG_READ_CURR_ADDRdv (0x00000000)
#define RMOD_BTG_READ_CURR_ADDR(...) rset(BTG_READ_CURR_ADDR, BTG_READ_CURR_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_CURR_ADDR(...) rset(BTG_READ_CURR_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_CURR_ADDR(fld) (*((const volatile BTG_READ_CURR_ADDR_T *)0xb800a21c)).f.fld

typedef union {
	struct {
		unsigned int resp_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_RESP_FAIL_ADDR_T;
#define BTG_READ_RESP_FAIL_ADDRrv (*((regval)0xb800a220))
#define BTG_READ_RESP_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_RESP_FAIL_ADDR(...) rset(BTG_READ_RESP_FAIL_ADDR, BTG_READ_RESP_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_RESP_FAIL_ADDR(...) rset(BTG_READ_RESP_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_RESP_FAIL_ADDR(fld) (*((const volatile BTG_READ_RESP_FAIL_ADDR_T *)0xb800a220)).f.fld

typedef union {
	struct {
		unsigned int peri_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_PERI_FAIL_ADDR_T;
#define BTG_READ_PERI_FAIL_ADDRrv (*((regval)0xb800a224))
#define BTG_READ_PERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_PERI_FAIL_ADDR(...) rset(BTG_READ_PERI_FAIL_ADDR, BTG_READ_PERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_PERI_FAIL_ADDR(...) rset(BTG_READ_PERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_PERI_FAIL_ADDR(fld) (*((const volatile BTG_READ_PERI_FAIL_ADDR_T *)0xb800a224)).f.fld

typedef union {
	struct {
		unsigned int chk_addr:32; //0
	} f;
	unsigned int v;
} BTG_READ_VERI_FAIL_ADDR_T;
#define BTG_READ_VERI_FAIL_ADDRrv (*((regval)0xb800a228))
#define BTG_READ_VERI_FAIL_ADDRdv (0x00000000)
#define RMOD_BTG_READ_VERI_FAIL_ADDR(...) rset(BTG_READ_VERI_FAIL_ADDR, BTG_READ_VERI_FAIL_ADDRrv, __VA_ARGS__)
#define RIZS_BTG_READ_VERI_FAIL_ADDR(...) rset(BTG_READ_VERI_FAIL_ADDR, 0, __VA_ARGS__)
#define RFLD_BTG_READ_VERI_FAIL_ADDR(fld) (*((const volatile BTG_READ_VERI_FAIL_ADDR_T *)0xb800a228)).f.fld

typedef union {
	struct {
		unsigned int max_resp_time:32; //0
	} f;
	unsigned int v;
} BTG_READ_MAX_RESP_TIME_T;
#define BTG_READ_MAX_RESP_TIMErv (*((regval)0xb800a22c))
#define BTG_READ_MAX_RESP_TIMEdv (0x00000000)
#define RMOD_BTG_READ_MAX_RESP_TIME(...) rset(BTG_READ_MAX_RESP_TIME, BTG_READ_MAX_RESP_TIMErv, __VA_ARGS__)
#define RIZS_BTG_READ_MAX_RESP_TIME(...) rset(BTG_READ_MAX_RESP_TIME, 0, __VA_ARGS__)
#define RFLD_BTG_READ_MAX_RESP_TIME(fld) (*((const volatile BTG_READ_MAX_RESP_TIME_T *)0xb800a22c)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC0_T;
#define BTC_READ_IN_VEC0rv (*((regval)0xb800a230))
#define BTC_READ_IN_VEC0dv (0x00000000)
#define RMOD_BTC_READ_IN_VEC0(...) rset(BTC_READ_IN_VEC0, BTC_READ_IN_VEC0rv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC0(...) rset(BTC_READ_IN_VEC0, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC0(fld) (*((const volatile BTC_READ_IN_VEC0_T *)0xb800a230)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC1_T;
#define BTC_READ_IN_VEC1rv (*((regval)0xb800a234))
#define BTC_READ_IN_VEC1dv (0x00000000)
#define RMOD_BTC_READ_IN_VEC1(...) rset(BTC_READ_IN_VEC1, BTC_READ_IN_VEC1rv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC1(...) rset(BTC_READ_IN_VEC1, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC1(fld) (*((const volatile BTC_READ_IN_VEC1_T *)0xb800a234)).f.fld

typedef union {
	struct {
		unsigned int in_vec:32; //0
	} f;
	unsigned int v;
} BTC_READ_IN_VEC_IDX_T;
#define BTC_READ_IN_VEC_IDXrv (*((regval)0xb800a238))
#define BTC_READ_IN_VEC_IDXdv (0x00000000)
#define RMOD_BTC_READ_IN_VEC_IDX(...) rset(BTC_READ_IN_VEC_IDX, BTC_READ_IN_VEC_IDXrv, __VA_ARGS__)
#define RIZS_BTC_READ_IN_VEC_IDX(...) rset(BTC_READ_IN_VEC_IDX, 0, __VA_ARGS__)
#define RFLD_BTC_READ_IN_VEC_IDX(fld) (*((const volatile BTC_READ_IN_VEC_IDX_T *)0xb800a238)).f.fld

/*-----------------------------------------------------
 Extraced from file_MEM_SRAM.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_ADDR_T;
#define CPU_SRAM_SEG0_ADDRrv (*((regval)0xb8004000))
#define CPU_SRAM_SEG0_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, CPU_SRAM_SEG0_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_ADDR(fld) (*((const volatile CPU_SRAM_SEG0_ADDR_T *)0xb8004000)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_SIZE_T;
#define CPU_SRAM_SEG0_SIZErv (*((regval)0xb8004004))
#define CPU_SRAM_SEG0_SIZEdv (0x00000008)
#define RMOD_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, CPU_SRAM_SEG0_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_SIZE(fld) (*((const volatile CPU_SRAM_SEG0_SIZE_T *)0xb8004004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_BASE_T;
#define CPU_SRAM_SEG0_BASErv (*((regval)0xb8004008))
#define CPU_SRAM_SEG0_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, CPU_SRAM_SEG0_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_BASE(fld) (*((const volatile CPU_SRAM_SEG0_BASE_T *)0xb8004008)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_ADDR_T;
#define CPU_SRAM_SEG1_ADDRrv (*((regval)0xb8004010))
#define CPU_SRAM_SEG1_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, CPU_SRAM_SEG1_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_ADDR(fld) (*((const volatile CPU_SRAM_SEG1_ADDR_T *)0xb8004010)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_SIZE_T;
#define CPU_SRAM_SEG1_SIZErv (*((regval)0xb8004014))
#define CPU_SRAM_SEG1_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, CPU_SRAM_SEG1_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_SIZE(fld) (*((const volatile CPU_SRAM_SEG1_SIZE_T *)0xb8004014)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_BASE_T;
#define CPU_SRAM_SEG1_BASErv (*((regval)0xb8004018))
#define CPU_SRAM_SEG1_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, CPU_SRAM_SEG1_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_BASE(fld) (*((const volatile CPU_SRAM_SEG1_BASE_T *)0xb8004018)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_ADDR_T;
#define CPU_SRAM_SEG2_ADDRrv (*((regval)0xb8004020))
#define CPU_SRAM_SEG2_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, CPU_SRAM_SEG2_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_ADDR(fld) (*((const volatile CPU_SRAM_SEG2_ADDR_T *)0xb8004020)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_SIZE_T;
#define CPU_SRAM_SEG2_SIZErv (*((regval)0xb8004024))
#define CPU_SRAM_SEG2_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, CPU_SRAM_SEG2_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_SIZE(fld) (*((const volatile CPU_SRAM_SEG2_SIZE_T *)0xb8004024)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_BASE_T;
#define CPU_SRAM_SEG2_BASErv (*((regval)0xb8004028))
#define CPU_SRAM_SEG2_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, CPU_SRAM_SEG2_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_BASE(fld) (*((const volatile CPU_SRAM_SEG2_BASE_T *)0xb8004028)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_ADDR_T;
#define CPU_SRAM_SEG3_ADDRrv (*((regval)0xb8004030))
#define CPU_SRAM_SEG3_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, CPU_SRAM_SEG3_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_ADDR(fld) (*((const volatile CPU_SRAM_SEG3_ADDR_T *)0xb8004030)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_SIZE_T;
#define CPU_SRAM_SEG3_SIZErv (*((regval)0xb8004034))
#define CPU_SRAM_SEG3_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, CPU_SRAM_SEG3_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_SIZE(fld) (*((const volatile CPU_SRAM_SEG3_SIZE_T *)0xb8004034)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_BASE_T;
#define CPU_SRAM_SEG3_BASErv (*((regval)0xb8004038))
#define CPU_SRAM_SEG3_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, CPU_SRAM_SEG3_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_BASE(fld) (*((const volatile CPU_SRAM_SEG3_BASE_T *)0xb8004038)).f.fld
#endif
