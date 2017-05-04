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
 * $Date: 2013-07-01 15:36:16 +0800 (Mon, 01 Jul 2013) $
 *
 * Purpose :
 *
 * Feature : 
 *
 */

#ifndef __EUROPA_CLI_H__
#define __EUROPA_CLI_H__

/*
 * Function Declaration
 */

static int europa_cli_delete(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_param_init(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_param_open(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_param_close(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_param_clear(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_param_set(
    int argc,
    char *argv[],
    FILE *fp);

static int _europa_load_data(
    unsigned char *ptr,
    FILE *fp,
    int *file_len); 
    
static int europa_cli_param_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_long_set(
    int argc,
    char *argv[],
    FILE *fp);   

static int europa_cli_all_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_ddmi_a0_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_ddmi_a2_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_a4_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_apd_lut_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_laser_lut_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_P0_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_P1_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorName_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorPN_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorRev_set(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_ddmi_a0_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_ddmi_a2_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_a4_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_apd_lut_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_laser_lut_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_P0P1_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorName_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorPN_get(
    int argc,
    char *argv[],
    FILE *fp);

static int europa_cli_VendorRev_get(
    int argc,
    char *argv[],
    FILE *fp);

#endif /* __EUROPA_CLI_H__ */

