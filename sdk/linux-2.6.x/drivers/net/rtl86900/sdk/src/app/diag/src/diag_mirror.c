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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition those Mirror command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <dal/apollo/raw/apollo_raw_mirror.h>

/*
 * mirror set egress-mode ( all-pkt | mirrored-only )
 */
cparser_result_t
cparser_cmd_mirror_set_egress_mode_all_pkt_mirrored_only(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    if ('a' == TOKEN_CHAR(3,0))
        state = DISABLED;
    else if ('m' == TOKEN_CHAR(3,0))
        state = ENABLED;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_ERR_CHK(apollo_raw_mirrorIso_set(state), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_mirror_set_egress_mode_all_pkt_mirrored_only */

/*
 * mirror get egress-mode
 */
cparser_result_t
cparser_cmd_mirror_get_egress_mode(
    cparser_context_t *context)
{
    int32 ret;
    rtk_enable_t iso_state;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_mirrorIso_get(&iso_state), ret);

    diag_util_printf("\n Mirror egress mode: %s", diagStr_mirrorEgressMode[iso_state]);
    diag_util_printf("\n\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_mirror_get_egress_mode */

/*
 * mirror set mirroring <UINT:port> mirrored ( <PORT_LIST:ports> | none ) { rx-mirror } { tx-mirror }
 */
cparser_result_t
cparser_cmd_mirror_set_mirroring_port_mirrored_ports_none_rx_mirror_tx_mirror(
    cparser_context_t *context,
    uint32_t  *port_id_ptr,
    char * *port_ptr)
{
    int32 ret;
    raw_mirror_cfg_t mir_cfg;
    diag_portlist_t portlist;
    uint32 token;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&mir_cfg, 0x00, sizeof(raw_mirror_cfg_t));

    mir_cfg.monitor_port = *port_id_ptr;
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    mir_cfg.source_portmask.bits[0] = portlist.portmask.bits[0];
    if (TOKEN_NUM() == 8)
    {
        mir_cfg.mirror_rx = ENABLED;
        mir_cfg.mirror_tx = ENABLED;
    }
    else if (TOKEN_NUM() == 6)
    {
        mir_cfg.mirror_rx = DISABLED;
        mir_cfg.mirror_tx = DISABLED;
    }
    else
    {
        if ('r' == TOKEN_CHAR(6, 0))
            mir_cfg.mirror_rx = ENABLED;

        if ('t' == TOKEN_CHAR(6, 0))
            mir_cfg.mirror_tx = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_mirrorCfg_set(&mir_cfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_mirror_set_mirroring_port_id_mirrored_ports_none_rx_mirror_tx_mirror */

/*
 * mirror get mirroring
 */
cparser_result_t
cparser_cmd_mirror_get_mirroring(
    cparser_context_t *context)
{
    int32 ret;
    raw_mirror_cfg_t mir_cfg;
    uint8 portStr[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_mirrorCfg_get(&mir_cfg), ret);

    diag_util_printf("\n Monitor port: %d", mir_cfg.monitor_port);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &mir_cfg.source_portmask), ret);
    diag_util_printf("\n Mirroring portmask: %s", portStr);
    diag_util_printf("\n Mirror TX: %s", diagStr_enable[mir_cfg.mirror_tx]);
    diag_util_printf("\n Mirror RX: %s", diagStr_enable[mir_cfg.mirror_rx]);

    diag_util_printf("\n\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_mirror_get_mirroring */
