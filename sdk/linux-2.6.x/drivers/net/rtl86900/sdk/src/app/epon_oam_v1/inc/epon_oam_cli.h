/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 40647 $
 * $Date: 2013-07-01 15:36:16 +0800 (?±ä?, 01 ä¸ƒæ? 2013) $
 *
 * Purpose :
 *
 * Feature : 
 *
 */

#ifndef __EPON_OAM_CLI_H__
#define __EPON_OAM_CLI_H__

/*
 * Function Declaration
 */
static int epon_oam_cli_configOam_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_configMac_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_configEvent_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_configAutoReg_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_configHoldover_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_config_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_counter_clear(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_counter_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_dbgFlag_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_dbgFlag_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_dbgExt_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_dbgExt_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_failover_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_failover_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_oamInfoVenderOui_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_oamInfo_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_oamState_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_version_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);
static int epon_oam_cli_register_trigger(
    int argc,
    char *argv[],
    oam_cli_t *pCli);

static int epon_oam_cli_ctcLoid_get(
    int argc,
    char *argv[],
    oam_cli_t *pCli);

static int epon_oam_cli_ctcLoid_set(
    int argc,
    char *argv[],
    oam_cli_t *pCli);

#endif /* __EPON_OAM_CLI_H__ */

