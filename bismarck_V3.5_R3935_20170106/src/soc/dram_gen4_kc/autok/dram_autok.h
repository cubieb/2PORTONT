#ifndef _DRAM_AUTOK_H_
#define _DRAM_AUTOK_H_

#include <dram/memcntlr.h>
#include <cg/cg.h>

/*********************************************/
/***** Settings from /project/xxx/parameter.c *****/
/*********************************************/
#define PROJ_PARA_DDR2_DRAM_ODT_VALUE         (meminfo.cntlr_opt->ddr2_odt_val)
#define PROJ_PARA_DDR3_DRAM_RTT_WR_VALUE      (meminfo.cntlr_opt->rtt_wr_ohm)
#define PROJ_PARA_DDR3_DRAM_RTT_NOM_VALUE     (meminfo.cntlr_opt->rtt_nom_ohm)
#define PROJ_PARA_PREFERED_DRAM_DRIV_STRENGTH (meminfo.cntlr_opt->normal_drv)
#define PROJ_PARA_USE_ANALOG_DELAY_LINE       (!(meminfo.cntlr_opt->dq_dll_type))
#define PROJ_PARA_PARALLEL_BANK_EN (meminfo.cntlr_opt->parallel_bank_en)

#define PROJ_PARA_ZQ_AUTO_CALI     (meminfo.cntlr_opt->zq_cal_method)
#define PROJ_PARA_CNTLR_ZPROG      (meminfo.register_set->ddzqpcr.f.zprog)
#define PROJ_PARA_CNTLR_ODT_CLK    (meminfo.cntlr_opt->cntlr_odt_clk)
#define PROJ_PARA_CNTLR_ODT_ADR    (meminfo.cntlr_opt->cntlr_odt_adr)
#define PROJ_PARA_CNTLR_ODT_DQ     (meminfo.cntlr_opt->cntlr_odt_dq)
#define PROJ_PARA_CNTLR_OCD_CLK    (meminfo.cntlr_opt->cntlr_ocd_clk)
#define PROJ_PARA_CNTLR_OCD_ADR    (meminfo.cntlr_opt->cntlr_ocd_adr)
#define PROJ_PARA_CNTLR_OCD_DQ     (meminfo.cntlr_opt->cntlr_ocd_dq)

#define PROJ_PARA_DQS0_GROUP_DELAY (meminfo.cntlr_opt->dqs0_group_tap)
#define PROJ_PARA_DQS1_GROUP_DELAY (meminfo.cntlr_opt->dqs1_group_tap)
#define PROJ_PARA_DQS0_HCLK        (meminfo.cntlr_opt->dqs0_en_hclk)
#define PROJ_PARA_DQS1_HCLK        (meminfo.cntlr_opt->dqs1_en_hclk)
#define PROJ_PARA_DQS0_EN_TAP      (meminfo.cntlr_opt->dqs0_en_tap)
#define PROJ_PARA_DQS1_EN_TAP      (meminfo.cntlr_opt->dqs1_en_tap)


#define MEMCTL_ZQ_CALI_PASS		(0)
#define MEMCTL_ZQ_CALI_FAIL		(1)

#define IS_DDR_SDRAM		(1)
#define IS_DDR2_SDRAM		(2)
#define IS_DDR3_SDRAM		(3)

/* predefined configurations*/
#define CONFIG_DRAM_WR_NS 15
#define CONFIG_DRAM_RTP_NS 8
#define CONFIG_DRAM_WTR_NS 9
#define CONFIG_DRAM_REFI_NS 7800
#define CONFIG_DRAM_RP_NS 15
#define CONFIG_DRAM_RCD_NS 15
#define CONFIG_DRAM_RRD_NS 10
#define CONFIG_DRAM_FAWG_NS 50
#define CONFIG_DRAM_RFC_NS 128
#define CONFIG_DRAM_RAS_NS 45


/* Registers */
#define MCR		(0xB8001000)
#define DCR		(0xB8001004)
#define DTR		(0xB8001008)
#define DTR0	(0xB8001008)
#define DTR1	(0xB800100c)
#define DTR2	(0xB8001010)
#define DMCR	(0xB800101C)
#define DACCR	(0xB8001500)
#define DACSPCR	(DACCR+0x4)
#define SOCPNR	(0xB80010FC)
#define SYSREG_MCKG_FREQ_DIV_REG        (0xB8000224)
#define SYSREG_SYSCLK_CONTROL_REG       (0xB8000200)


/* Field start bit definition */
#define MCR_D_INIT_TRIG_FD_S	(14)
#define MCR_OCP1_RBF_MASK_EN	(8)
#define MCR_OCP0_RBF_MASK_EN	(7)
#define MCR_OCP1_RBF_F_DIS	(6)
#define MCR_OCP0_RBF_F_DIS	(5)
#define DTR0_CAS_FD_S		(28)
#define DTR0_WR_FD_S 		(24)
#define DTR0_CWL_FD_S 		(20)
#define DTR0_RTP_FD_S 		(16)
#define DTR0_WTR_FD_S 		(12)
#define DTR0_REFI_FD_S 		(8)
#define DTR0_REFI_UNIT_FD_S (4)
#define DTR0_T_CAS_PHY_S    (0)
#define DTR0_T_CAS_PHY_S    (0)
#define DTR1_RP_FD_S 		(24)
#define DTR1_RCD_FD_S 		(16)
#define DTR1_RRD_FD_S 		(8)
#define DTR1_FAWG_FD_S 		(0)
#define DTR2_RFC_FD_S 		(20)
#define DTR2_RAS_FD_S 		(12)
#define DCR_DBUSWID_FD_S	(24)

#define MCR_D_INIT_TRIG_MASK	(0x1 << MCR_D_INIT_TRIG_FD_S)
#define MCR_RBF_MAS			(0xF << MCR_OCP0_RBF_F_DIS)
#define DTR0_T_CAS_PHY_MASK (0xF << DTR0_T_CAS_PHY_S)
#define DTR0_REFI_MASK 		(0xF << DTR0_REFI_FD_S)
#define DTR0_REFI_UNIT_MASK 	(0xF << DTR0_REFI_UNIT_FD_S)
#define DTR0_CAS_MASK		(0xF << DTR0_CAS_FD_S)
#define DTR0_WR_MASK 		(0xF << DTR0_WR_FD_S)
#define DTR0_CWL_MASK 		(0xF << DTR0_CWL_FD_S)
#define DTR0_RTP_MASK 		(0xF << DTR0_RTP_FD_S)
#define DTR0_WTR_MASK 		(0xF << DTR0_WTR_FD_S)
#define DTR1_RP_MASK 		(0xFF << DTR1_RP_FD_S)
#define DTR1_RCD_MASK 		(0xFF << DTR1_RCD_FD_S)
#define DTR1_RRD_MASK 		(0xFF << DTR1_RRD_FD_S)
#define DTR1_FAWG_MASK 		(0xFF << DTR1_FAWG_FD_S)
#define DTR2_RFC_MASK 		(0xFFF << DTR2_RFC_FD_S)
#define DTR2_RAS_MASK 		(0x3F << DTR2_RAS_FD_S)
#define DCR_DBUSWID_MASK	(0xF << DCR_DBUSWID_FD_S)

/*DMCR register related definition*/
#define DMCR_MRS_BUSY		(0x80000000)
#define DMCR_MR_MODE_EN		(0x00100000)
#define DMCR_MRS_MODE_MR	(0x00000000)
#define DMCR_MRS_MODE_EMR1	(0x00010000)
#define DMCR_MRS_MODE_EMR2	(0x00020000)
#define DMCR_MRS_MODE_EMR3	(0x00030000)
#define DMCR_DIS_DRAM_REF_FD_S	(24)
#define DMCR_DIS_DRAM_REF_MASK	(1 << DMCR_DIS_DRAM_REF_FD_S)
#define DMCR_MR_MODE_EN_FD_S	(20)
#define DMCR_MR_MODE_EN_MASK	(1 << DMCR_MR_MODE_EN_FD_S)


/* DDR Mode register related definition */
#define DDR1_MR_BURST_2		(0x00000001)
#define DDR1_MR_BURST_4		(0x00000002)
#define DDR1_MR_BURST_8		(0x00000003)
#define DDR1_MR_BURST_SEQ	(0x00000000)
#define DDR1_MR_BURST_INTER	(0x00000008)
#define DDR1_MR_CAS_2 		(0x00000020)
#define DDR1_MR_CAS_3 		(0x00000030)
#define DDR1_MR_CAS_25 		(0x00000060)
#define DDR1_MR_OP_NOR		(0x00000000)
#define DDR1_MR_OP_RST_DLL	(0x00000100)
#define DDR1_MR_OP_TEST		(0x00000080)

#define DDR1_EMR1_DLL_EN	(0x00000000)
#define DDR1_EMR1_DLL_DIS	(0x00000001)
#define DDR1_EMR1_DRV_NOR	(0x00000000)
#define DDR1_EMR1_DRV_WEAK	(0x00000002)

/* DDR2 Mode register related definition */
#define DDR2_MR_BURST_4 	(0x00000002)
#define DDR2_MR_BURST_8 	(0x00000003)
#define DDR2_MR_BURST_SEQ 	(0x00000000)
#define DDR2_MR_BURST_INTER 	(0x00000008)
#define DDR2_MR_CAS_2 		(0x00000020)
#define DDR2_MR_CAS_3 		(0x00000030)
#define DDR2_MR_CAS_4 		(0x00000040)
#define DDR2_MR_CAS_5 		(0x00000050)
#define DDR2_MR_CAS_6 		(0x00000060)
#define DDR2_MR_CAS_7 		(0x00000070)
#define DDR2_MR_TM_NOR 		(0x00000000)
#define DDR2_MR_TM_TEST		(0x00000080)
#define DDR2_MR_DLL_RESET_YES	(0x00000100)
#define DDR2_MR_DLL_RESET_NO	(0x00000000)
#define DDR2_MR_WR_2		(0x00000200)
#define DDR2_MR_WR_3		(0x00000400)
#define DDR2_MR_WR_4		(0x00000600)
#define DDR2_MR_WR_5		(0x00000800)
#define DDR2_MR_WR_6		(0x00000A00)
#define DDR2_MR_WR_7		(0x00000C00)
#define DDR2_MR_PD_FAST		(0x00000000)
#define DDR2_MR_PD_SLOW		(0x00001000)
#define DDR2_EMR1_DLL_EN	(0x00000000)
#define DDR2_EMR1_DLL_DIS	(0x00000001)
#define DDR2_EMR1_DIC_FULL	(0x00000000)
#define DDR2_EMR1_DIC_REDUCE	(0x00000002)
#define DDR2_EMR1_RTT_DIS	(0x00000000)
#define DDR2_EMR1_RTT_75	(0x00000004)
#define DDR2_EMR1_RTT_150	(0x00000040)
#define DDR2_EMR1_RTT_50	(0x00000044)
#define DDR2_EMR1_ADD_0		(0x00000000)
#define DDR2_EMR1_ADD_1		(0x00000008)
#define DDR2_EMR1_ADD_2		(0x00000010)
#define DDR2_EMR1_ADD_3		(0x00000018)
#define DDR2_EMR1_ADD_4		(0x00000020)
#define DDR2_EMR1_ADD_5		(0x00000028)
#define DDR2_EMR1_OCD_EX	(0x00000000)
#define DDR2_EMR1_OCD_D1	(0x00000080)
#define DDR2_EMR1_OCD_D0	(0x00000100)
#define DDR2_EMR1_OCD_AD	(0x00000200)
#define DDR2_EMR1_OCD_DEF	(0x00000380)
#define DDR2_EMR1_QOFF_EN	(0x00000000)
#define DDR2_EMR1_QOFF_DIS	(0x00001000)
#define DDR2_EMR1_NDQS_EN	(0x00000000)
#define DDR2_EMR1_NDQS_DIS	(0x00000400)
#define DDR2_EMR1_RDQS_EN	(0x00000800)
#define DDR2_EMR1_RDQS_DIS	(0x00000000)
#define DDR2_EMR2_HTREF_EN	(0x00000080)
#define DDR2_EMR2_HTREF_DIS	(0x00000000)
#define DDR2_EMR2_DCC_DIS	(0x00000000)
#define DDR2_EMR2_DCC_EN	(0x00000008)
#define DDR2_EMR2_PASELF_FULL	(0x00000000)

/* DDR3 Mode register related definition */
#define DDR3_MR_BURST_8 		(0x00000000)
#define DDR3_MR_BURST_BC4OR8 		(0x00000001)
#define DDR3_MR_BURST_BC4 		(0x00000002)
#define DDR3_MR_READ_BURST_NIBBLE 	(0x00000000)
#define DDR3_MR_READ_BURST_INTER 	(0x00000008)
#define DDR3_MR_CAS_5 			(0x00000010)
#define DDR3_MR_CAS_6	 		(0x00000020)
#define DDR3_MR_CAS_7	 		(0x00000030)
#define DDR3_MR_CAS_8	 		(0x00000040)
#define DDR3_MR_CAS_9	 		(0x00000050)
#define DDR3_MR_CAS_10	 		(0x00000060)
#define DDR3_MR_CAS_11	 		(0x00000070)
#define DDR3_MR_TM_NOR	 		(0x00000000)
#define DDR3_MR_TM_TEST			(0x00000080)
#define DDR3_MR_DLL_RESET_YES		(0x00000100)
#define DDR3_MR_DLL_RESET_NO		(0x00000000)
#define DDR3_MR_WR_5			(0x00000200)
#define DDR3_MR_WR_6			(0x00000400)
#define DDR3_MR_WR_7			(0x00000600)
#define DDR3_MR_WR_8			(0x00000800)
#define DDR3_MR_WR_9			(0x00000A00)
#define DDR3_MR_WR_10			(0x00000C00)
#define DDR3_MR_WR_12			(0x00000E00)
#define DDR3_MR_PD_FAST			(0x00001000)
#define DDR3_MR_PD_SLOW			(0x00000000)
#define DDR3_EMR1_DLL_EN		(0x00000000)
#define DDR3_EMR1_DLL_DIS		(0x00000001)
#define DDR3_EMR1_DIC_RZQ_DIV_6		(0x00000000)
#define DDR3_EMR1_DIC_RZQ_DIV_7		(0x00000002)
#define DDR3_EMR1_RTT_NOM_DIS		(0x00000000)
#define DDR3_EMR1_RTT_NOM_RZQ_DIV4	(0x00000004)
#define DDR3_EMR1_RTT_NOM_RZQ_DIV2	(0x00000040)
#define DDR3_EMR1_RTT_NOM_RZQ_DIV6	(0x00000044)
#define DDR3_EMR1_RTT_NOM_RZQ_DIV12	(0x00000200)
#define DDR3_EMR1_RTT_NOM_RZQ_DIV8	(0x00000204)
#define DDR3_EMR1_ADD_0			(0x00000000)
#define DDR3_EMR1_ADD_CL_RD1		(0x00000008)
#define DDR3_EMR1_ADD_CL_RD2		(0x00000010)
#define DDR3_EMR1_WRITE_LEVEL_DIS	(0x00000000)
#define DDR3_EMR1_WRITE_LEVEL_EN	(0x00000080)
#define DDR3_EMR1_TDQS_DIS		(0x00000000)
#define DDR3_EMR1_TDQS_EN		(0x00000800)
#define DDR3_EMR1_QOFF_DIS		(0x00001000)
#define DDR3_EMR1_QOFF_EN		(0x00000000)
#define DDR3_EMR2_PASR_FULL		(0x00000000)
#define DDR3_EMR2_PASR_HALF_L		(0x00000001)
#define DDR3_EMR2_PASR_QUA		(0x00000002)
#define DDR3_EMR2_PASR_8TH_L		(0x00000003)
#define DDR3_EMR2_PASR_3_QUA		(0x00000004)
#define DDR3_EMR2_PASR_HALF_H		(0x00000005)
#define DDR3_EMR2_PASR_8TH_H		(0x00000007)
#define DDR3_EMR2_CWL_5			(0x00000000)
#define DDR3_EMR2_CWL_6			(0x00000008)
#define DDR3_EMR2_CWL_7			(0x00000010)
#define DDR3_EMR2_CWL_8			(0x00000018)
#define DDR3_EMR2_ASR_EN		(0x00000040)
#define DDR3_EMR2_ASR_DIS		(0x00000000)
#define DDR3_EMR2_SRT_NOR		(0x00000000)
#define DDR3_EMR2_SRT_EXT		(0x00000080)
#define DDR3_EMR2_RTT_WR_DIS		(0x00000000)
#define DDR3_EMR2_RTT_WR_RZQ_DIV_4	(0x00000200)
#define DDR3_EMR2_RTT_WR_RZQ_DIV_2	(0x00000400)
#define DDR3_EMR3_MPR_LOC_PRE_PAT	(0x00000000)
#define DDR3_EMR3_MPR_OP_NOR		(0x00000000)
#define DDR3_EMR3_MPR_OP_DATA		(0x00000004)

#ifndef SECTION_AUTOK
#define SECTION_AUTOK
#endif


/* vvv ZQCS / ZQCL for RTL8685SB vvv */
#define D3ZQCCR					(0xb8001080)
#define ZQ_LONG_TRI				(1<<31)
#define ZQ_LONG_TRI_BUSY		(0x80000000)
#define ZQ_SHORT_EN				(1<<30)
#define T_ZQCS					(0x7F)
#define AC_SILEN_PERIOD_EN		(1<<31)
#define AC_SILEN_PERIOD_UNIT	(0x0<<16)
#define AC_SILEN_PERIOD			(0x0<<8)
/* ^^^ ZQCS / ZQCL for RTL8685SB ^^^ */


//Extern Functions for autoK
extern int memctlc_ZQ_calibration(unsigned int auto_cali_value);
extern u32_t dram_autok(void);

#endif
