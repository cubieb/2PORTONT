/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * Purpose : Definition those register command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) register
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <common/debug/mem.h>
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <ioal/io_mii.h>
#include <hal/chipdef/allreg.h>
#include <hal/mac/reg.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#include <rtk/gpon.h>
#include <diag_str.h>

/*
 * gpon set serial-number <STRING:vendor_id> <UINT:serial_number>
 */
cparser_result_t
cparser_cmd_gpon_set_serial_number_vendor_id_serial_number(
    cparser_context_t *context,
    char * *vendor_id_ptr,
    uint32_t  *serial_number_ptr)
{
	rtk_gpon_serialNumber_t sn;
	int32       			ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((strlen(*vendor_id_ptr) != RTK_GPON_VENDOR_ID_LEN),CPARSER_ERR_INVALID_PARAMS);

	osal_memcpy(sn.vendor,*vendor_id_ptr,RTK_GPON_VENDOR_ID_LEN);
	sn.specific[0] = ((*serial_number_ptr)>>24)&0xFF;
	sn.specific[1] = ((*serial_number_ptr)>>16)&0xFF;
	sn.specific[2] = ((*serial_number_ptr)>>8)&0xFF;
	sn.specific[3] = ((*serial_number_ptr)>>0)&0xFF;

	DIAG_UTIL_ERR_CHK(rtk_gpon_serialNumber_set(&sn), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_serial_number_vendor_id_serial_number */

/*
 * gpon get serial-number
 */
cparser_result_t
cparser_cmd_gpon_get_serial_number(
    cparser_context_t *context)
{
	rtk_gpon_serialNumber_t sn;
	char                    tmp[RTK_GPON_VENDOR_ID_LEN+1];
	int32       			ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_serialNumber_get(&sn), ret);

	osal_memcpy(tmp,sn.vendor,RTK_GPON_VENDOR_ID_LEN);
	tmp[RTK_GPON_VENDOR_ID_LEN] = '\0';
    diag_util_mprintf("serial number: %s 0x%02x%02x%02x%02x\n\r",
					  tmp, sn.specific[0],sn.specific[1],sn.specific[2],sn.specific[3]);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_serial_number */

/*
 * gpon set password <STRING:password>
 */
cparser_result_t
cparser_cmd_gpon_set_password_password(
    cparser_context_t *context,
    char * *password_ptr)
{
	rtk_gpon_password_t pwd;
	int32       		ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((strlen(*password_ptr) != RTK_GPON_PASSWORD_LEN),CPARSER_ERR_INVALID_PARAMS);

	osal_memcpy(pwd.password,*password_ptr,RTK_GPON_PASSWORD_LEN);

	DIAG_UTIL_ERR_CHK(rtk_gpon_password_set(&pwd), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_password_password */

/*
 * gpon get password
 */
cparser_result_t
cparser_cmd_gpon_get_password(
    cparser_context_t *context)
{
	rtk_gpon_password_t pwd;
	char                tmp[RTK_GPON_PASSWORD_LEN+1];
	int32       		ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_password_get(&pwd), ret);

	osal_memcpy(tmp,pwd.password,RTK_GPON_PASSWORD_LEN);
	tmp[RTK_GPON_PASSWORD_LEN] = '\0';
	diag_util_mprintf("password: %s \n\r",tmp);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_password */

/*
 * gpon set active-timer to1 <UINT:to1_timer> to2 <UINT:to2_timer>
 */
cparser_result_t
cparser_cmd_gpon_set_active_timer_to1_to1_timer_to2_to2_timer(
    cparser_context_t *context,
    uint32_t  *to1_timer_ptr,
    uint32_t  *to2_timer_ptr)
{
	rtk_gpon_onu_activation_para_t 	para;
	int32							ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	para.to1_timer = *to1_timer_ptr;
	para.to2_timer = *to2_timer_ptr;
	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_ONU_ACTIVATION,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_active_timer_to1_to1_timer_to2_to2_timer */

/*
 * gpon get active-timer
 */
cparser_result_t
cparser_cmd_gpon_get_active_timer(
    cparser_context_t *context)
{
	rtk_gpon_onu_activation_para_t 	para;
	int32							ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_ONU_ACTIVATION,&para), ret);

	diag_util_mprintf("to1 timer: %d ms\n\r",para.to1_timer);
	diag_util_mprintf("to2 timer: %d ms\n\r",para.to2_timer);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_active_timer */

/*
 * gpon set ds-laser ( opt_los | cdr_los ) state ( enable | disable ) polarity ( high | low )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_laser_opt_los_cdr_los_state_enable_disable_polarity_high_low(
    cparser_context_t *context)
{
	rtk_gpon_laser_para_t 	para;
	int32                   state, polar;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_LASER,&para), ret);

	if ('e' == TOKEN_CHAR(5,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(5,0))
		state = 0;

	if ('h' == TOKEN_CHAR(7,0))
		polar = 1;
	else if ('l' == TOKEN_CHAR(7,0))
		polar = 0;

	if ('o' == TOKEN_CHAR(3,0))
	{
		para.laser_optic_los_en = state;
		para.laser_optic_los_polar = polar;
	}
	else if ('c' == TOKEN_CHAR(3,0))
	{
		para.laser_cdr_los_en = state;
		para.laser_cdr_los_polar = polar;
	}

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_LASER,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_laser_opt_los_cdr_los_state_enable_disalbe_polarity_high_low */

/*
 * gpon set ds-laser los_holdover ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_laser_los_holdover_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_laser_para_t 	para;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_LASER,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.laser_los_filter_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.laser_los_filter_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_LASER,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_laser_los_holdover_enable_disable */

/*
 * gpon set ds-phy descramble ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_phy_descramble_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_ds_physical_para_t para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_PHY,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.ds_scramble_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.ds_scramble_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_PHY,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_phy_descramble_enable_disable */

/*
 * gpon set ds-phy fec-state ( enable | disable ) fec-threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_gpon_set_ds_phy_fec_state_enable_disable_fec_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    rtk_gpon_ds_physical_para_t para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_PHY,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.ds_fec_bypass = 0;
	else if ('d' == TOKEN_CHAR(4,0))
		para.ds_fec_bypass = 1;

	para.ds_fec_thrd = *threshold_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_PHY,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_phy_fec_state_enable_disable_fec_threshold_threshold */

/*
 * gpon set ds-ploam ( drop-crc-error | filter-onuid | accept-broadcast ) ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_ploam_drop_crc_error_filter_onuid_accept_broadcast_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_ds_ploam_para_t 	para;
	int32 						state;
	int32					 	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_PLOAM,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		state = 0;

	if ('d' == TOKEN_CHAR(3,0))
		para.ds_ploam_drop_crc_err = state;
	else if ('f' == TOKEN_CHAR(3,0))
		para.ds_ploam_onuid_filter = state;
	else if ('a' == TOKEN_CHAR(3,0))
		para.ds_ploam_broadcast_accpt = state;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_PLOAM,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_ploam_drop_crc_error_filter_onuid_accept_broadcast_enable_disable */

/*
 * gpon set ds-bwmap ( drop-crc-error | filter-onuid | strict-plen ) ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_bwmap_drop_crc_error_filter_onuid_strict_plen_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_ds_bwMap_para_t 	para;
	int32 						state;
	int32					 	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_BWMAP,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		state = 0;

	if ('d' == TOKEN_CHAR(3,0))
		para.ds_bwmap_crc_chk = state;
	else if ('f' == TOKEN_CHAR(3,0))
		para.ds_bwmap_onuid_filter = state;
    else if ('s' == TOKEN_CHAR(3,0))
		para.ds_bwmap_plend_mode = state;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_BWMAP,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_bwmap_drop_crc_error_filter_onuid_strict_plen_enable_disable */

/*
 * gpon set ds-gem assembly-threshold <UINT:threshold>
 */
cparser_result_t
cparser_cmd_gpon_set_ds_gem_assembly_threshold_threshold(
    cparser_context_t *context,
    uint32_t  *threshold_ptr)
{
    rtk_gpon_ds_gem_para_t 	para;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*threshold_ptr>31),CPARSER_ERR_INVALID_PARAMS);

	para.assemble_timer = *threshold_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_GEM,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_gem_assembly_threshold_threshold */

/*
 * gpon set ds-eth drop-crc-error ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_ds_eth_drop_crc_error_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_ds_eth_para_t	para;
	int32 					state;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_ETH,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.ds_eth_crc_chk = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.ds_eth_crc_chk = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_ETH,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_eth_drop_crc_error_enable_disable */

/*
 * gpon set ds-eth pti-pettern <UINT:pettern> pti-mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_gpon_set_ds_eth_pti_pettern_pettern_pti_mask_mask(
    cparser_context_t *context,
    uint32_t  *pettern_ptr,
    uint32_t  *mask_ptr)
{
    rtk_gpon_ds_eth_para_t	para;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*pettern_ptr>0x7),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*mask_ptr>0x7),CPARSER_ERR_INVALID_PARAMS);

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_ETH,&para), ret);

	para.ds_eth_pti_ptn = *pettern_ptr;
	para.ds_eth_pti_mask = *mask_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_ETH,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_eth_pti_pettern_pettern_pti_mask_mask */

/*
 * gpon set ds-omci pti-pettern <UINT:pettern> pti-mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_gpon_set_ds_omci_pti_pettern_pettern_pti_mask_mask(
    cparser_context_t *context,
    uint32_t  *pettern_ptr,
    uint32_t  *mask_ptr)
{
    rtk_gpon_ds_omci_para_t	para;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*pettern_ptr>0x7),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*mask_ptr>0x7),CPARSER_ERR_INVALID_PARAMS);

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_OMCI,&para), ret);

	para.ds_omci_pti_ptn = *pettern_ptr;
	para.ds_omci_pti_mask = *mask_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_DS_OMCI,&para), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_ds_omci_pti_pettern_pettern_pti_mask_mask */

/*
 * gpon get ( ds-laser | ds-phy | ds-ploam | ds-bwmap | ds-gem | ds-eth | ds-omci )
 */
cparser_result_t
cparser_cmd_gpon_get_ds_laser_ds_phy_ds_ploam_ds_bwmap_ds_gem_ds_eth_ds_omci(
    cparser_context_t *context)
{
    rtk_gpon_laser_para_t 		laser;
	rtk_gpon_ds_physical_para_t	phy;
	rtk_gpon_ds_ploam_para_t	ploam;
	rtk_gpon_ds_bwMap_para_t	bwmap;
	rtk_gpon_ds_gem_para_t		gem;
	rtk_gpon_ds_eth_para_t		eth;
	rtk_gpon_ds_omci_para_t		omci;
	int32						ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	if (0 == strcmp( "ds-laser",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_LASER,&laser), ret);
		diag_util_mprintf("OPT LOS state: %s \n\r",(laser.laser_optic_los_en==1)?"enable":"disable");
		diag_util_mprintf("OPT LOS polarity: %s \n\r",(laser.laser_optic_los_polar==1)?"high":"low");
		diag_util_mprintf("CDR LOS state: %s \n\r",(laser.laser_cdr_los_en==1)?"enable":"disable");
		diag_util_mprintf("CDR LOS polarity: %s \n\r",(laser.laser_cdr_los_polar==1)?"high":"low");
		diag_util_mprintf("LOS holdover: %s \n\r",(laser.laser_los_filter_en==1)?"enable":"disable");
	}
	else if (0 == strcmp( "ds-phy",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_PHY,&phy), ret);
		diag_util_mprintf("Descramble: %s \n\r",(phy.ds_scramble_en==1)?"enable":"disable");
		diag_util_mprintf("FEC state: %s \n\r",(phy.ds_fec_bypass==0)?"enable":"disable");
		diag_util_mprintf("FEC threshold: %d \n\r",phy.ds_fec_thrd);
	}
	else if (0 == strcmp( "ds-ploam",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_PLOAM,&ploam), ret);
		diag_util_mprintf("Accept broadcast: %s \n\r",(ploam.ds_ploam_broadcast_accpt==1)?"enable":"disable");
		diag_util_mprintf("Drop CRC error: %s \n\r",(ploam.ds_ploam_drop_crc_err==1)?"enable":"disable");
		diag_util_mprintf("Filter ONU-ID: %s \n\r",(ploam.ds_ploam_onuid_filter==1)?"enable":"disable");
	}
	else if (0 == strcmp( "ds-bwmap",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_BWMAP,&bwmap), ret);
		diag_util_mprintf("Drop CRC error: %s \n\r",(bwmap.ds_bwmap_crc_chk==1)?"enable":"disable");
		diag_util_mprintf("Filter ONU-ID: %s \n\r",(bwmap.ds_bwmap_onuid_filter==1)?"enable":"disable");
        diag_util_mprintf("Strict PLENd: %s \n\r",(bwmap.ds_bwmap_plend_mode==1)?"enable":"disable");
	}
	else if (0 == strcmp( "ds-gem",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_GEM,&gem), ret);
		diag_util_mprintf("Assembly threshold: %d \n\r",gem.assemble_timer);
	}
	else if (0 == strcmp( "ds-eth",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_ETH,&eth), ret);
		diag_util_mprintf("Drop CRC error: %s \n\r",(eth.ds_eth_crc_chk==1)?"enable":"disable");
		diag_util_mprintf("PTI pattern: 0x%x \n\r",eth.ds_eth_pti_ptn);
		diag_util_mprintf("PTI mask: 0x%x \n\r",eth.ds_eth_pti_mask);
	}
	else if (0 == strcmp( "ds-omci",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_DS_OMCI,&omci), ret);
		diag_util_mprintf("PTI pattern: 0x%x \n\r",omci.ds_omci_pti_ptn);
		diag_util_mprintf("PTI mask: 0x%x \n\r",omci.ds_omci_pti_mask);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_ds_laser_ds_phy_ds_ploam_ds_bwmap_ds_gem_ds_eth_ds_omci */

/*
 * gpon set us-laser on-offset <UINT:on_offset> off-offset <UINT:off_offset>
 */
cparser_result_t
cparser_cmd_gpon_set_us_laser_on_offset_on_offset_off_offset_off_offset(
    cparser_context_t *context,
    uint32_t  *on_offset_ptr,
    uint32_t  *off_offset_ptr)
{
    rtk_gpon_laser_para_t 	para;
	int32                   state, polar;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*on_offset_ptr > 55),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*off_offset_ptr > 55),CPARSER_ERR_INVALID_PARAMS);

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_LASER,&para), ret);

	para.laser_us_on = *on_offset_ptr;
	para.laser_us_off = *off_offset_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_LASER,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_up_laser_on_offset_on_offset_off_offset_off_offset */

/*
 * gpon set us-phy scramble ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_us_phy_scramble_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_us_physical_para_t	para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.us_scramble_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.us_scramble_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_phy_scramble_enable_disable */

/*
 * gpon set us-phy burst_polarity ( high | low )
 */
cparser_result_t
cparser_cmd_gpon_set_us_phy_burst_polarity_high_low(
    cparser_context_t *context)
{
    rtk_gpon_us_physical_para_t	para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	if ('h' == TOKEN_CHAR(4,0))
		para.us_burst_en_polar = 1;
	else if ('l' == TOKEN_CHAR(4,0))
		para.us_burst_en_polar = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_phy_burst_polarity_high_low */

/*
 * gpon set us-phy auto_sstart ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_us_phy_auto_sstart_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_us_physical_para_t	para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.small_sstart_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.small_sstart_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_phy_auto_sstart_enable_disable */

/*
 * gpon set us-phy suppress_laser ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_us_phy_suppress_laser_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_us_physical_para_t	para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.suppress_laser_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.suppress_laser_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_PHY,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_phy_suppress_laser_enable_disable */

/*
 * gpon set us-ploam state ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_us_ploam_state_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_us_ploam_para_t	para;
	int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PLOAM,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.us_ploam_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.us_ploam_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_PLOAM,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_ploam_state_enable_disable */

/*
 * gpon set us-dbr state ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_us_dbr_state_enable_disable(
    cparser_context_t *context)
{
    rtk_gpon_us_dbr_para_t	para;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_DBR,&para), ret);

	if ('e' == TOKEN_CHAR(4,0))
		para.us_dbru_en = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		para.us_dbru_en = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_set(RTK_GPON_PARA_TYPE_US_DBR,&para), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_us_dbr_state_enable_disable */

/*
 * gpon get ( us-laser | us-phy | us-ploam | us-dbr )
 */
cparser_result_t
cparser_cmd_gpon_get_us_laser_us_phy_us_ploam_us_dbr(
    cparser_context_t *context)
{
    rtk_gpon_laser_para_t 		laser;
	rtk_gpon_us_physical_para_t phy;
	rtk_gpon_us_ploam_para_t	ploam;
	rtk_gpon_us_dbr_para_t		dbr;
	int32						ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	if (0 == strcmp( "us-laser",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_LASER,&laser), ret);
		diag_util_mprintf("Laser-on offset: %d \n\r",laser.laser_us_on);
		diag_util_mprintf("Laser-off offset: %d \n\r",laser.laser_us_off);
	}
	else if (0 == strcmp( "us-phy",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PHY,&phy), ret);
		diag_util_mprintf("Scramble: %s \n\r",(phy.us_scramble_en==1)?"enable":"disable");
		diag_util_mprintf("burst-enable polarity: %s \n\r",(phy.us_burst_en_polar==1)?"high":"low");
		diag_util_mprintf("Auto-sstart: %s \n\r",(phy.small_sstart_en==1)?"enable":"disable");
		diag_util_mprintf("Suppressing laser: %s \n\r",(phy.suppress_laser_en==1)?"enable":"disable");
	}
	else if (0 == strcmp( "us-ploam",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_PLOAM,&ploam), ret);
		diag_util_mprintf("PLOAMu state: %s \n\r",(ploam.us_ploam_en==1)?"enable":"disable");
	}
	else if (0 == strcmp( "us-dbr",TOKEN_STR(2)))
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_parameter_get(RTK_GPON_PARA_TYPE_US_DBR,&dbr), ret);
		diag_util_mprintf("DBRu state: %s \n\r",(dbr.us_dbru_en==1)?"enable":"disable");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_us_laser_us_phy_us_ploam_us_dbr */

/*
 * gpon activate init-state ( o1 | o7 )
 */
cparser_result_t
cparser_cmd_gpon_activate_init_state_o1_o7(
    cparser_context_t *context)
{
    rtk_gpon_initialState_t	state;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if (0 == strcmp( "o1",TOKEN_STR(3)))
		state = RTK_GPONMAC_INIT_STATE_O1;
	else if (0 == strcmp( "o7",TOKEN_STR(3)))
		state = RTK_GPONMAC_INIT_STATE_O7;

	DIAG_UTIL_ERR_CHK(rtk_gpon_activate(state), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_activate_init_state_o1_o7 */

/*
 * gpon deactivate
 */
cparser_result_t
cparser_cmd_gpon_deactivate(
    cparser_context_t *context)
{
	int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_deActivate(), ret);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_deactivate */

/*
 * gpon get onu-state
 */
cparser_result_t
cparser_cmd_gpon_get_onu_state(
    cparser_context_t *context)
{
    rtk_gpon_fsm_status_t	state;
	int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_ponStatus_get(&state), ret);
	diag_util_mprintf("ONU state: %s \n\r", gpon_dbg_fsm_status_str(state));

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_onu_state */

/*
 * gpon add tcont alloc-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_add_tcont_alloc_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_tcont_ind_t	tcont_id;
	rtk_gpon_tcont_attr_t	tcont_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_OUTPUT_INIT();

	tcont_id.alloc_id = *id_ptr;
	tcont_id.type = RTK_GPON_TCONT_TYPE_3;

	DIAG_UTIL_ERR_CHK(rtk_gpon_tcont_create(&tcont_id, &tcont_attr), ret);
	diag_util_mprintf("Add alloc_id %d, return TCONT_id=%d \n\r", tcont_id.alloc_id,tcont_attr.tcont_id);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_add_tcont_tcont_id_tcont_id_alloc_id_alloc_id */

/*
 * gpon del tcont alloc-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_del_tcont_alloc_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_tcont_ind_t	tcont_id;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);

	tcont_id.alloc_id = *id_ptr;

	DIAG_UTIL_ERR_CHK(rtk_gpon_tcont_destroy(&tcont_id), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_del_tcont_tcont_id_tcont_id */

/*
 * gpon get tcont alloc-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_get_tcont_alloc_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_tcont_ind_t	tcont_id;
	rtk_gpon_tcont_attr_t	tcont_attr;
	int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);

	tcont_id.alloc_id = *id_ptr;
	DIAG_UTIL_ERR_CHK(rtk_gpon_tcont_get(&tcont_id, &tcont_attr), ret);
	diag_util_mprintf("alloc_id: %d, TCONT_id %d: \n\r",tcont_id.alloc_id,tcont_attr.tcont_id);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_tcont_tcont_id_id */

/*
 * gpon add ds-flow flow-id <UINT:id> gem-port <UINT:gem> ether { multicast } { aes }
 */
cparser_result_t
cparser_cmd_gpon_add_ds_flow_flow_id_id_gem_port_gem_ether_multicast_aes(
    cparser_context_t *context,
    uint32_t  *id_ptr,
    uint32_t  *gem_ptr)
{
    rtk_gpon_dsFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*gem_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);

	flow_attr.type = RTK_GPON_FLOW_TYPE_ETH;
	flow_attr.gem_port_id = *gem_ptr;

	if (TOKEN_NUM() < 9)
	{
		flow_attr.multicast = 0;
		flow_attr.aes_en = 0;
	}
	else if (TOKEN_NUM() == 9)
	{
		if ('m' == TOKEN_CHAR(8,0))
		{
			flow_attr.multicast = 1;
			flow_attr.aes_en = 0;
		}
		else if ('a' == TOKEN_CHAR(8,0))
		{
			flow_attr.multicast = 0;
			flow_attr.aes_en = 1;
		}
	}
	else
	{
		flow_attr.multicast = 1;
		flow_attr.aes_en = 1;
	}

	DIAG_UTIL_ERR_CHK(rtk_gpon_dsFlow_set(*id_ptr, &flow_attr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_add_ds_flow_flow_id_id_gem_port_gem_ether_multicast_aes */

/*
 * gpon add ds-flow flow-id <UINT:id> gem-port <UINT:gem> omci  { aes }
 */
cparser_result_t
cparser_cmd_gpon_add_ds_flow_flow_id_id_gem_port_gem_omci_aes(
    cparser_context_t *context,
    uint32_t  *id_ptr,
    uint32_t  *gem_ptr)
{
    rtk_gpon_dsFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*gem_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);

	osal_memset(&flow_attr,0,sizeof(rtk_gpon_dsFlow_attr_t));

	flow_attr.type = RTK_GPON_FLOW_TYPE_OMCI;
	flow_attr.gem_port_id = *gem_ptr;
	flow_attr.multicast = 0;

	if (TOKEN_NUM() < 9)
		flow_attr.aes_en = 0;
	else
		flow_attr.aes_en = 1;

	DIAG_UTIL_ERR_CHK(rtk_gpon_dsFlow_set(*id_ptr, &flow_attr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_add_ds_flow_flow_id_id_gem_port_gem_omci_aes */

/*
 * gpon del ds-flow flow-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_del_ds_flow_flow_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_dsFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);

	osal_memset(&flow_attr,0,sizeof(rtk_gpon_dsFlow_attr_t));

	flow_attr.gem_port_id = RTK_GPON_GEMPORT_ID_NOUSE;

	DIAG_UTIL_ERR_CHK(rtk_gpon_dsFlow_set(*id_ptr, &flow_attr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_del_ds_flow_flow_id_id */

/*
 * gpon get ds-flow flow-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_get_ds_flow_flow_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_dsFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);

	DIAG_UTIL_ERR_CHK(rtk_gpon_dsFlow_get(*id_ptr, &flow_attr), ret);
	diag_util_mprintf("flow_id  gem_port  type  multicast  AES\n\r");
	diag_util_mprintf("=======  ========  ====  =========  ===\n\r");
	diag_util_mprintf("%7d  %8d  %4s  %9s  %3s\n\r",
					  *id_ptr,
					  flow_attr.gem_port_id,
					  gpon_dbg_flow_type_str(flow_attr.type),
					  flow_attr.multicast?"yes":"no",
					  flow_attr.aes_en?"yes":"no");

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_ds_flow_flow_id_id */

/*
 * gpon add us-flow flow-id <UINT:id> gem-port <UINT:gem> ( ether | omci )
 */
cparser_result_t
cparser_cmd_gpon_add_us_flow_flow_id_id_gem_port_gem_ether_omci(
    cparser_context_t *context,
    uint32_t  *id_ptr,
    uint32_t  *gem_ptr)
{
    rtk_gpon_usFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);
	DIAG_UTIL_PARAM_RANGE_CHK((*gem_ptr > 4095),CPARSER_ERR_INVALID_PARAMS);

	osal_memset(&flow_attr,0,sizeof(rtk_gpon_usFlow_attr_t));

	flow_attr.gem_port_id = *gem_ptr;

	if ('e' == TOKEN_CHAR(7,0))
		flow_attr.type = RTK_GPON_FLOW_TYPE_ETH;
	else if ('o' == TOKEN_CHAR(7,0))
		flow_attr.type = RTK_GPON_FLOW_TYPE_OMCI;

	DIAG_UTIL_ERR_CHK(rtk_gpon_usFlow_set(*id_ptr, &flow_attr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_add_us_flow_flow_id_id_gem_port_gem_ether_omci */

/*
 * gpon del us-flow flow-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_del_us_flow_flow_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_usFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);

	osal_memset(&flow_attr,0,sizeof(rtk_gpon_usFlow_attr_t));

	flow_attr.gem_port_id = RTK_GPON_GEMPORT_ID_NOUSE;

	DIAG_UTIL_ERR_CHK(rtk_gpon_usFlow_set(*id_ptr, &flow_attr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_del_us_flow_flow_id_id */

/*
 * gpon get us-flow flow-id <UINT:id>
 */
cparser_result_t
cparser_cmd_gpon_get_us_flow_flow_id_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    rtk_gpon_usFlow_attr_t	flow_attr;
	int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > 127),CPARSER_ERR_INVALID_PARAMS);

	DIAG_UTIL_ERR_CHK(rtk_gpon_usFlow_get(*id_ptr, &flow_attr), ret);
	diag_util_mprintf("flow_id  gem_port  type\n\r");
	diag_util_mprintf("=======  ========  ====\n\r");
	diag_util_mprintf("%7d  %8d  %4s\n\r",
					  *id_ptr,
					  flow_attr.gem_port_id,
					  gpon_dbg_flow_type_str(flow_attr.type));

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_us_flow_flow_id_id */

/*
 * gpon set multicast-filter ( broadcast-pass | non-multicast-pass ) ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_multicast_filter_broadcast_pass_non_multicast_pass_enable_disable(
    cparser_context_t *context)
{
    int32	mode;
	int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if ('e' == TOKEN_CHAR(4,0))
		mode = 1;
	else if ('d' == TOKEN_CHAR(4,0))
		mode = 0;

	if ('b' == TOKEN_CHAR(3,0))
		DIAG_UTIL_ERR_CHK(rtk_gpon_broadcastPass_set(mode), ret);
	else if ('n' == TOKEN_CHAR(3,0))
        DIAG_UTIL_ERR_CHK(rtk_gpon_nonMcastPass_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_multicast_filter_broadcast_pass_non_multicast_pass_enable_disable */

/*
 * gpon set multicast-filter ( prefix-ip | prefix-ip6 ) <UINT:prefix>
 */
cparser_result_t
cparser_cmd_gpon_set_multicast_filter_prefix_ip_prefix_ip6_prefix(
    cparser_context_t *context,
	uint32_t  *prefix)
{
	uint32 	ip4_prefix, ip6_prefix, *tmp;
	int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_multicastAddrCheck_get(&ip4_prefix, &ip6_prefix), ret);

	if (0 == strcmp("prefix-ip",TOKEN_STR(3)))
		tmp = &ip4_prefix;
	else if (0 == strcmp("prefix-ip6",TOKEN_STR(3)))
		tmp = &ip6_prefix;

	*tmp = *prefix;

	DIAG_UTIL_ERR_CHK(rtk_gpon_multicastAddrCheck_set(ip4_prefix, ip6_prefix), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_multicast_filter_prefix_ip_prefix_ip6_prefix */

/*
 * gpon set multicast-filter ( force-ip | force-ip6 ) ( pass | drop | normal )
 */
cparser_result_t
cparser_cmd_gpon_set_multicast_filter_force_ip_force_ip6_pass_drop_normal(
    cparser_context_t *context)
{
	rtk_gpon_mc_force_mode_t 	ipv4, ipv6, *tmp;
    int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	DIAG_UTIL_ERR_CHK(rtk_gpon_mcForceMode_get(&ipv4, &ipv6), ret);

	if (0 == strcmp("force-ip",TOKEN_STR(3)))
		tmp = &ipv4;
	else if (0 == strcmp("force-ip6",TOKEN_STR(3)))
		tmp = &ipv6;

	if ('p' == TOKEN_CHAR(4,0))
		*tmp = RTK_GPON_MCFORCE_MODE_PASS;
	else if ('d' == TOKEN_CHAR(4,0))
		*tmp = RTK_GPON_MCFORCE_MODE_DROP;
	else if ('n' == TOKEN_CHAR(4,0))
		*tmp = RTK_GPON_MCFORCE_MODE_NORMAL;

	DIAG_UTIL_ERR_CHK(rtk_gpon_mcForceMode_set(ipv4, ipv6), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_multicast_filter_force_ip_force_ip6_pass_drop_normal */

/*
 * gpon set multicast-filter filter-mode ( include | exclude )
 */
cparser_result_t
cparser_cmd_gpon_set_multicast_filter_filter_mode_include_exclude(
    cparser_context_t *context)
{
	rtk_gpon_macTable_exclude_mode_t 	mode;
    int32								ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if ('i' == TOKEN_CHAR(4,0))
		mode = RTK_GPON_MACTBL_MODE_INCLUDE;
	else if ('e' == TOKEN_CHAR(4,0))
		mode = RTK_GPON_MACTBL_MODE_EXCLUDE;

	DIAG_UTIL_ERR_CHK(rtk_gpon_macFilterMode_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_multicast_filter_filter_mode_include_exclude */

/*
 * gpon get multicast-filter
 */
cparser_result_t
cparser_cmd_gpon_get_multicast_filter(
    cparser_context_t *context)
{
	int32 							 bc_pass, non_mc_pass;
	uint32 							 prefix_ip4, prefix_ip6;
	rtk_gpon_mc_force_mode_t 		 force_ip4, force_ip6;
	rtk_gpon_macTable_exclude_mode_t mode;
	char                             *force_str;
	int32							 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_broadcastPass_get(&bc_pass), ret);
    DIAG_UTIL_ERR_CHK(rtk_gpon_nonMcastPass_get(&non_mc_pass), ret);
	DIAG_UTIL_ERR_CHK(rtk_gpon_multicastAddrCheck_get(&prefix_ip4, &prefix_ip6), ret);
	DIAG_UTIL_ERR_CHK(rtk_gpon_mcForceMode_get(&force_ip4, &force_ip6), ret);
	rtk_gpon_macFilterMode_get(&mode);

    diag_util_mprintf("Broadcast bypass %s\n\r",(bc_pass?"enable":"disable"));
	diag_util_mprintf("Non-multicast bypass %s\n\r",(non_mc_pass?"enable":"disable"));
	diag_util_mprintf("Ipv4 prefix 0x%06x\n\r",prefix_ip4);
	diag_util_mprintf("Ipv6 prefix 0x%04x\n\r",prefix_ip6);
	diag_util_mprintf("Ipv4 force mode %s\n\r",gpon_dbg_mac_force_mode_str(force_ip4));
	diag_util_mprintf("Ipv6 force mode %s\n\r",gpon_dbg_mac_force_mode_str(force_ip6));
	diag_util_mprintf("Filter mode %s\n\r",gpon_dbg_mac_filter_mode_str(mode));

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_multicast_filter */

/*
 * gpon add multicast-filter-entry mac-address <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_gpon_add_multicast_filter_entry_mac_address_mac(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr)
{
    rtk_gpon_macTable_entry_t	entry;
    int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	osal_memcpy(entry.mac_addr, mac_ptr->octet, RTK_GPON_MACTABLE_ADDR_LEN);

	DIAG_UTIL_ERR_CHK(rtk_gpon_macEntry_add(&entry), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_add_multicast_filter_entry_mac_address_mac */

/*
 * gpon del multicast-filter-entry mac-address <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_gpon_del_multicast_filter_entry_mac_address_mac(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr)
{
    rtk_gpon_macTable_entry_t	entry;
    int32						ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	osal_memcpy(entry.mac_addr, mac_ptr->octet, RTK_GPON_MACTABLE_ADDR_LEN);

	DIAG_UTIL_ERR_CHK(rtk_gpon_macEntry_del(&entry), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_del_multicast_filter_entry_entry_mac_address_mac */

/*
 * gpon get multicast-filter-entry index <UINT:index>
 */
cparser_result_t
cparser_cmd_gpon_get_multicast_filter_entry_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	rtk_gpon_macTable_entry_t	entry;
	int32						ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr>255),CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_macEntry_get(*index_ptr,&entry), ret);
    diag_util_mprintf("Index | MAC Address\r\n");
	diag_util_mprintf("  %3d | %02x-%02x-%02x-%02x-%02x-%02x\r\n", *index_ptr,
					  entry.mac_addr[0],entry.mac_addr[1],entry.mac_addr[2],
					  entry.mac_addr[3],entry.mac_addr[4],entry.mac_addr[5]);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_multicast_filter_entry_index_index */

/*
 * gpon set rdi ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_rdi_enable_disable(
    cparser_context_t *context)
{
    int32	mode;
    int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if ('e' == TOKEN_CHAR(3,0))
		mode = 1;
	else if ('d' == TOKEN_CHAR(3,0))
		mode = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_rdi_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_rdi_enable_disable */

/*
 * gpon get rdi
 */
cparser_result_t
cparser_cmd_gpon_get_rdi(
    cparser_context_t *context)
{
	int32	mode;
    int32	ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_rdi_get(&mode), ret);

    diag_util_mprintf("RDI state: %s\n\r",mode?"enable":"disable");

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_rdi */

/*
 * gpon get alarm-status
 */
cparser_result_t
cparser_cmd_gpon_get_alarm_status(
    cparser_context_t *context)
{
    rtk_gpon_alarm_type_t	alarm;
	int32 					status;
    int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	for(alarm=RTK_GPON_ALARM_LOS;alarm<RTK_GPON_ALARM_MAX;alarm++)
	{
		DIAG_UTIL_ERR_CHK(rtk_gpon_alarmStatus_get(alarm,&status), ret);
		diag_util_mprintf("Alarm %s, status: %s\n\r",gpon_dbg_alarm_str(alarm),status?"occur":"clear");
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_alarm_status */

/*
 * gpon set tx-laser ( force-on | force-off | normal )
 */
cparser_result_t
cparser_cmd_gpon_set_tx_laser_force_on_force_off_normal(
    cparser_context_t *context)
{
    rtk_gpon_laser_status_t	state;
    int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if (0 == strcmp("force-on",TOKEN_STR(3)))
		state = RTK_GPON_LASER_STATUS_FORCE_ON;
	else if (0 == strcmp("force-off",TOKEN_STR(3)))
		state = RTK_GPON_LASER_STATUS_FORCE_OFF;
	else if (0 == strcmp("normal",TOKEN_STR(3)))
		state = RTK_GPON_LASER_STATUS_NORMAL;

	DIAG_UTIL_ERR_CHK(rtk_gpon_txForceLaser_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_tx_laser_force_on_force_off_normal */

/*
 * gpon set tx-force-idle ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_tx_force_idle_enable_disable(
    cparser_context_t *context)
{
    int32	state;
    int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

	if ('e' == TOKEN_CHAR(3,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(3,0))
		state = 0;

	DIAG_UTIL_ERR_CHK(rtk_gpon_txForceIdle_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_tx_force_idle_enable_disable */

/*
 * gpon get tx
 */
cparser_result_t
cparser_cmd_gpon_get_tx(
    cparser_context_t *context)
{
    rtk_gpon_laser_status_t	laser;
	int32 					state;
	int32					ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	DIAG_UTIL_ERR_CHK(rtk_gpon_txForceLaser_get(&laser), ret);
	diag_util_mprintf("TX laser mode: %s\n\r",gpon_dbg_mac_force_laser_str(laser));
	DIAG_UTIL_ERR_CHK(rtk_gpon_txForceIdle_get(&state), ret);
	diag_util_mprintf("TX force Idle: %s\n\r",state?"enable":"disable");

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_tx */

/*
 * gpon show ( version | dev-info | gtc | tcont | ds-flow | us-flow | multicast-filter-entry )
 */
cparser_result_t
cparser_cmd_gpon_show_version_dev_info_gtc_tcont_ds_flow_us_flow_multicast_filter_entry(
    cparser_context_t *context)
{
	int32	ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if (0 == strcmp("version",TOKEN_STR(2)))
		rtk_gpon_version_show();
	else if (0 == strcmp("dev-info",TOKEN_STR(2)))
		rtk_gpon_devInfo_show();
	else if (0 == strcmp("gtc",TOKEN_STR(2)))
		rtk_gpon_gtc_show();
	else if (0 == strcmp("tcont",TOKEN_STR(2)))
		rtk_gpon_tcont_show(0xFFFF);
	else if (0 == strcmp("ds-flow",TOKEN_STR(2)))
		rtk_gpon_dsFlow_show(0xFFFF);
	else if (0 == strcmp("us-flow",TOKEN_STR(2)))
		rtk_gpon_usFlow_show(0xFFFF);
	else if (0 == strcmp("mc-filter-entry",TOKEN_STR(2)))
		rtk_gpon_macTable_show();

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_show_version_dev_info_gtc_tcont_ds_flow_us_flow_multicast_filter_entry */

/*
 * gpon show counter global ( active | ds-phy | ds-plm | ds-bw | ds-gem | ds-eth | ds-omci | us-phy | us-dbr | us-plm | us-gem | us-eth | us-omci )
 */
cparser_result_t
cparser_cmd_gpon_show_counter_global_active_ds_phy_ds_plm_ds_bw_ds_gem_ds_eth_ds_omci_us_phy_us_dbr_us_plm_us_gem_us_eth_us_omci(
    cparser_context_t *context)
{
	rtk_gpon_global_performance_type_t 	global;
	int32								ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

    if (0 == strcmp("active",TOKEN_STR(4)))
		rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_ONU_ACTIVATION);
    else if ((0 == strcmp("ds-phy",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_PHY);
    else if ((0 == strcmp("ds-plm",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_PLOAM);
    else if ((0 == strcmp("ds-bw",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_BWMAP);
    else if ((0 == strcmp("ds-gem",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_GEM);
    else if ((0 == strcmp("ds-eth",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_ETH);
    else if ((0 == strcmp("ds-omci",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_DS_OMCI);
    else if ((0 == strcmp("us-phy",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_PHY);
    else if ((0 == strcmp("us-dbr",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_DBR);
    else if ((0 == strcmp("us-plm",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_PLOAM);
    else if ((0 == strcmp("us-gem",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_GEM);
    else if ((0 == strcmp("us-eth",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_ETH);
    else if ((0 == strcmp("us-omci",TOKEN_STR(4))))
        rtk_gpon_globalCounter_show(RTK_GPON_PMTYPE_US_OMCI);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_show_counter_global_active_ds_phy_ds_plm_ds_bw_ds_gem_ds_eth_ds_omci_us_phy_us_dbr_us_plm_us_gem_us_eth_us_omci */

/*
 * gpon show counter tcont <UINT:tcont_id>
 */
cparser_result_t
cparser_cmd_gpon_show_counter_tcont_tcont_id(
    cparser_context_t *context,
    uint32_t  *tcont_id_ptr)
{
	rtk_gpon_tcont_performance_type_t	tcont;
	int32								ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_PARAM_RANGE_CHK((*tcont_id_ptr>31), CPARSER_ERR_INVALID_PARAMS);

	for(tcont=RTK_GPON_PMTYPE_TCONT_GEM;tcont<RTK_GPON_PMTYPE_TCONT_MAX;tcont++)
		rtk_gpon_tcontCounter_show(*tcont_id_ptr,tcont);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_show_counter_tcont_tcont_id */

/*
 * gpon show counter flow <UINT:flow_id>
 */
cparser_result_t
cparser_cmd_gpon_show_counter_flow_flow_id(
    cparser_context_t *context,
    uint32_t  *flow_id_ptr)
{
	rtk_gpon_flow_performance_type_t	flow;
	int32								ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	DIAG_UTIL_PARAM_RANGE_CHK((*flow_id_ptr>127), CPARSER_ERR_INVALID_PARAMS);

	for(flow=RTK_GPON_PMTYPE_FLOW_DS_GEM;flow<RTK_GPON_PMTYPE_FLOW_MAX;flow++)
		rtk_gpon_flowCounter_show(*flow_id_ptr,flow);

	return CPARSER_OK;
}    /* end of cparser_cmd_gpon_show_counter_flow_flow_id */

/*
 * gpon initial
 */
cparser_result_t
cparser_cmd_gpon_initial(
    cparser_context_t *context,
    uint32_t  *irq_id_ptr)
{
    int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_gpon_initial(1), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_initial_irq_id */

/*
 * gpon deinitial
 */
cparser_result_t
cparser_cmd_gpon_deinitial(
    cparser_context_t *context)
{
    int32	ret = RT_ERR_FAILED;

	DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_gpon_deinitial(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_deinitial */

/*
 * gpon set debug ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_debug_enable_disable(
    cparser_context_t *context)
{
    int32 state;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if ('e' == TOKEN_CHAR(3,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(3,0))
		state = 0;

    DIAG_UTIL_ERR_CHK(rtk_gpon_debug_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_debug_enable_disable */

/*
 * gpon reg-get page <UINT:page> offset <UINT:offset>
 */
cparser_result_t
cparser_cmd_gpon_reg_get_page_page_offset_offset(
    cparser_context_t *context,
    uint32_t  *page_ptr,
    uint32_t  *offset_ptr)
{
    uint32 addr;
    uint32 value;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    addr = (*page_ptr << 12) | (*offset_ptr)*4 | 0x700000;
    DIAG_UTIL_ERR_CHK(ioal_mem32_read(addr, &value), ret);

    diag_util_printf("addr 0x%08x, value: 0x%08x\n\r",addr, value);

    return CPARSER_OK;
}   /* end of cparser_cmd_gpon_reg_get_page_page_offset_offset */

/*
 * gpon reg-set page <UINT:page> offset <UINT:offset> value <UINT:value>
 */
cparser_result_t
cparser_cmd_gpon_reg_set_page_page_offset_offset_value_value(
    cparser_context_t *context,
    uint32_t  *page_ptr,
    uint32_t  *offset_ptr,
    uint32_t  *value_ptr)
{
    uint32 addr;
    uint32 value;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    addr = (*page_ptr << 12) | (*offset_ptr)*4 | 0x700000;
    DIAG_UTIL_ERR_CHK(ioal_mem32_write(addr, *value_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_gpon_reg_set_page_page_offset_offset_value_value */

/*
 * gpon unit-test <UNIT:test_id>
 */
cparser_result_t
cparser_cmd_gpon_unit_test_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    int32 state;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_gpon_unit_test(*id_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_gpon_unit_test_id */

/*
 * gpon pkt-gen-buf item <UINT:item> da <MACADDR:dmac> sa <MACADDR:smac> vid <UINT:vid> pattern <UINT:pattern> length <UINT:length>
 */
cparser_result_t
cparser_cmd_gpon_pkt_gen_buf_item_item_da_dmac_sa_smac_vid_vid_pattern_pattern_length_length(
    cparser_context_t *context,
    uint32_t  *item_ptr,
    cparser_macaddr_t  *dmac_ptr,
    cparser_macaddr_t  *smac_ptr,
    uint32_t  *vid_ptr,
    uint32_t  *ptn_ptr,
    uint32_t  *length_ptr)
{
    uint8 *buf;
    uint32 offset=0;
    int32 i;

    DIAG_UTIL_PARAM_CHK();

    if((*item_ptr == 0) || (*item_ptr == 1))
    {
        DIAG_UTIL_PARAM_RANGE_CHK(((*length_ptr)>64), CPARSER_ERR_INVALID_PARAMS);
    }
    else if((*item_ptr == 2))
    {
        DIAG_UTIL_PARAM_RANGE_CHK(((*length_ptr)>2048), CPARSER_ERR_INVALID_PARAMS);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PARAM_RANGE_CHK((((*length_ptr)%4)!=0), CPARSER_ERR_INVALID_PARAMS);

    if(*length_ptr < 64)
        *length_ptr = 64;

    if((buf = (uint8*)osal_alloc(*length_ptr)) == NULL)
        return CPARSER_ERR_OUT_OF_RES;

    osal_memset(buf, 0x0, *length_ptr);


    /* DMAC*/
    osal_memcpy(&buf[offset], dmac_ptr->octet, 6);
    offset += 6;

    /* SMAC*/
    osal_memcpy(&buf[offset], smac_ptr->octet, 6);
    offset += 6;

    /* VLAN tag TPID 0x8100 */
    buf[offset++] = 0x81;
    buf[offset++] = 0x0;

    /* VID = gem*/
    buf[offset++] = (uint8)(((*vid_ptr)>>8)&0xF);
    buf[offset++] = (uint8)((*vid_ptr)&0xFF);

    while(offset <= ((*length_ptr)-4))
    {
        buf[offset++] = (uint8)(((*ptn_ptr)>>24)&0xFF);
        buf[offset++] = (uint8)(((*ptn_ptr)>>16)&0xFF);
        buf[offset++] = (uint8)(((*ptn_ptr)>>8)&0xFF);
        buf[offset++] = (uint8)((*ptn_ptr)&0xFF);
    }

#if 0 /*defined(FPGA_DEFINED)*/
    rtk_gpon_pktGen_buf_set(*item_ptr, buf, *length_ptr);
#endif
    osal_free(buf);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_pkt_gen_buf_item_item_da_dmac_sa_smac_vid_vid_pattern_ptn_length_length */

/*
 * gpon pkt_gen_cfg item <UINT:item> tcont <UINT:tcont> gem <UINT:gem> tx-length <UINT:length> { omci }
 */
cparser_result_t
cparser_cmd_gpon_pkt_gen_cfg_item_item_tcont_tcont_gem_gem_tx_length_length_omci(
    cparser_context_t *context,
    uint32_t  *item_ptr,
    uint32_t  *tcont_ptr,
    uint32_t  *gem_ptr,
    uint32_t  *length_ptr)
{
    int32 omci;

    DIAG_UTIL_PARAM_CHK();

    if(!((*item_ptr == 0) || (*item_ptr == 1) || (*item_ptr == 2)))
        return CPARSER_ERR_INVALID_PARAMS;

    if(TOKEN_NUM() <= 10)
        omci = 0;
    else
        omci = 1;

#if 0 /*#if defined(FPGA_DEFINED)*/
    rtk_gpon_pktGen_cfg_set(*item_ptr, *tcont_ptr, *length_ptr, *gem_ptr, omci);
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_pkt_gen_cfg_item_item_tcont_tcont_gem_gem_tx_length_length_omci */

/*
 * gpon omci_tx <UINT:data1> <UINT:data2> <UINT:data3> <UINT:data4> <UINT:data5> <UINT:data6> <UINT:data7> <UINT:data8> <UINT:data9> <UINT:data10> <UINT:data11>
 */
cparser_result_t
cparser_cmd_gpon_omci_tx_data1_data2_data3_data4_data5_data6_data7_data8_data9_data10_data11_data12(
    cparser_context_t *context,
    uint32_t  *data1_ptr,
    uint32_t  *data2_ptr,
    uint32_t  *data3_ptr,
    uint32_t  *data4_ptr,
    uint32_t  *data5_ptr,
    uint32_t  *data6_ptr,
    uint32_t  *data7_ptr,
    uint32_t  *data8_ptr,
    uint32_t  *data9_ptr,
    uint32_t  *data10_ptr,
    uint32_t  *data11_ptr,
    uint32_t  *data12_ptr)
{
    rtk_gpon_omci_msg_t omci;
    uint8_t *tmp_ptr;
    int32_t ret;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&omci, 0x0, sizeof(rtk_gpon_omci_msg_t));
    tmp_ptr = omci.msg;

    osal_memcpy(tmp_ptr, data1_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data2_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data3_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data4_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data5_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data6_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data7_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data8_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data9_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data10_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data11_ptr, sizeof(uint32_t));
    tmp_ptr += sizeof(uint32_t);
    osal_memcpy(tmp_ptr, data12_ptr, sizeof(uint32_t));


    DIAG_UTIL_ERR_CHK(rtk_gpon_omci_tx(&omci), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_omci_tx_data1_data2_data3_data4_data5_data6_data7_data8_data9_data10_data11 */

/*
 * gpon set auto-tcont ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_auto_tcont_enable_disable(
    cparser_context_t *context)
{
    int32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();


    if ('e' == TOKEN_CHAR(3,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(3,0))
		state = 0;

    DIAG_UTIL_ERR_CHK(rtk_gpon_autoTcont_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_auto_tcont_enable_disable */

/*
 * gpon get auto-tcont
 */
cparser_result_t
cparser_cmd_gpon_get_auto_tcont(
    cparser_context_t *context)
{
    int32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_gpon_autoTcont_get(&state), ret);

    diag_util_printf("GPON auto TCONT state: %s\n\r",diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_auto_tcont */

/*
 * gpon set auto-boh ( enable | disable )
 */
cparser_result_t
cparser_cmd_gpon_set_auto_boh_enable_disable(
    cparser_context_t *context)
{
    int32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();


    if ('e' == TOKEN_CHAR(3,0))
		state = 1;
	else if ('d' == TOKEN_CHAR(3,0))
		state = 0;

    DIAG_UTIL_ERR_CHK(rtk_gpon_autoBoh_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_auto_boh_enable_disable */

/*
 * gpon get auto-boh
 */
cparser_result_t
cparser_cmd_gpon_get_auto_boh(
    cparser_context_t *context)
{
    int32 state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_gpon_autoBoh_get(&state), ret);

    diag_util_printf("GPON auto BOH state: %s\n\r",diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_auto_boh */

/*
 * gpon set eqd-offset ( plus | minus ) <UINT:offset>
 */
cparser_result_t
cparser_cmd_gpon_set_eqd_offset_plus_minus_offset(
    cparser_context_t *context,
    uint32_t  *offset_ptr)
{
    int32 offset;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();


    if ('p' == TOKEN_CHAR(3,0))
		offset = *offset_ptr;
	else if ('m' == TOKEN_CHAR(3,0))
		offset = (0 - *offset_ptr);

    DIAG_UTIL_ERR_CHK(rtk_gpon_eqdOffset_set(offset), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_eqd_offset_plus_minus_offset */

/*
 * gpon get eqd-offset
 */
cparser_result_t
cparser_cmd_gpon_get_eqd_offset(
    cparser_context_t *context)
{
    int32 offset;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_gpon_eqdOffset_get(&offset), ret);

    diag_util_printf("GPON EQD offset: %d\n\r", offset);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_eqd_offset */



/*
 * gpon set aes-framecnt <UINT:framecnt>
 */
cparser_result_t
cparser_cmd_gpon_set_aes_framecnt_framecnt(
    cparser_context_t *context,
    uint32_t  *framecnt_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_gpon_aes_keyswitch_write(*framecnt_ptr), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_aes_framecnt_framecnt */

/*
 * gpon get aes-framecnt
 */
cparser_result_t
cparser_cmd_gpon_get_aes_framecnt(
    cparser_context_t *context)
{
    uint32 framecnt;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_gpon_aes_keyswitch_read(&framecnt), ret);
    diag_util_mprintf("\nAES Frame count = %d\n", framecnt);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_get_aes_framecnt */

/*
 * gpon set aes-key <UINT:byte0> <UINT:byte1> <UINT:byte2> <UINT:byte3> <UINT:byte4> <UINT:byte5> <UINT:byte6> <UINT:byte7> <UINT:byte8> <UINT:byte9> <UINT:byte10> <UINT:byte11> <UINT:byte12> <UINT:byte13> <UINT:byte14> <UINT:byte15>
 */
cparser_result_t
cparser_cmd_gpon_set_aes_key_byte0_byte1_byte2_byte3_byte4_byte5_byte6_byte7_byte8_byte9_byte10_byte11_byte12_byte13_byte14_byte15(
    cparser_context_t *context,
    uint32_t  *byte0_ptr,
    uint32_t  *byte1_ptr,
    uint32_t  *byte2_ptr,
    uint32_t  *byte3_ptr,
    uint32_t  *byte4_ptr,
    uint32_t  *byte5_ptr,
    uint32_t  *byte6_ptr,
    uint32_t  *byte7_ptr,
    uint32_t  *byte8_ptr,
    uint32_t  *byte9_ptr,
    uint32_t  *byte10_ptr,
    uint32_t  *byte11_ptr,
    uint32_t  *byte12_ptr,
    uint32_t  *byte13_ptr,
    uint32_t  *byte14_ptr,
    uint32_t  *byte15_ptr)
{
    uint8 key_array[16];
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if((*byte0_ptr > 0xFF) || (*byte1_ptr > 0xFF) || (*byte2_ptr > 0xFF) || (*byte3_ptr > 0xFF) ||
       (*byte4_ptr > 0xFF) || (*byte5_ptr > 0xFF) || (*byte6_ptr > 0xFF) || (*byte7_ptr > 0xFF) ||
       (*byte8_ptr > 0xFF) || (*byte9_ptr > 0xFF) || (*byte10_ptr > 0xFF) || (*byte11_ptr > 0xFF) ||
       (*byte12_ptr > 0xFF) || (*byte13_ptr > 0xFF) || (*byte14_ptr > 0xFF) || (*byte15_ptr > 0xFF) )
    {
        return CPARSER_ERR_INVALID_PARAMS;
    }

    key_array[0]    = (uint8)(*byte0_ptr & 0xFF);
    key_array[1]    = (uint8)(*byte1_ptr & 0xFF);
    key_array[2]    = (uint8)(*byte2_ptr & 0xFF);
    key_array[3]    = (uint8)(*byte3_ptr & 0xFF);
    key_array[4]    = (uint8)(*byte4_ptr & 0xFF);
    key_array[5]    = (uint8)(*byte5_ptr & 0xFF);
    key_array[6]    = (uint8)(*byte6_ptr & 0xFF);
    key_array[7]    = (uint8)(*byte7_ptr & 0xFF);
    key_array[8]    = (uint8)(*byte8_ptr & 0xFF);
    key_array[9]    = (uint8)(*byte9_ptr & 0xFF);
    key_array[10]   = (uint8)(*byte10_ptr & 0xFF);
    key_array[11]   = (uint8)(*byte11_ptr & 0xFF);
    key_array[12]   = (uint8)(*byte12_ptr & 0xFF);
    key_array[13]   = (uint8)(*byte13_ptr & 0xFF);
    key_array[14]   = (uint8)(*byte14_ptr & 0xFF);
    key_array[15]   = (uint8)(*byte15_ptr & 0xFF);

    DIAG_UTIL_ERR_CHK(apollo_raw_gpon_aes_keyword_write(key_array), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_gpon_set_aes_key_byte0_byte1_byte2_byte3_byte4_byte5_byte6_byte7_byte8_byte9_byte10_byte11_byte12_byte13_byte14_byte15 */


