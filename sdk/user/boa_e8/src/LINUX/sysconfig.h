/*
 * sysconfig.h --- header file for configuration server
 * --- By Kaohj
 */


#ifndef INCLUDE_SYSCONFIG_H
#define INCLUDE_SYSCONFIG_H
#include "mib.h"
#include "mibtbl.h"
//#define		INCLUDE_DEFAULT_VALUE	1

int mib_lock(void); /* Update RAM setting to flash */
int mib_unlock(void); /* Update RAM setting to flash */
int mib_update_from_raw(unsigned char* ptr, int len); /* Write the specified setting to flash, this function will also check the length and checksum */
int mib_read_to_raw(CONFIG_DATA_T data_type, unsigned char* ptr, int len); /* Load flash setting to the specified pointer */
int mib_update(CONFIG_DATA_T data_type, CONFIG_MIB_T flag); /* Update RAM setting to flash */
int mib_read_header(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader); /* Load flash header */
int mib_load(CONFIG_DATA_T data_type, CONFIG_MIB_T flag); /* Load flash setting to RAM */
int mib_swap(int id, int id1);
int mib_get(int id, void *value);
int mib_set(int id, void *value);
#ifdef CONFIG_USER_CWMP_TR069
int mib_set_cwmp(int id, void *value);
#endif
int mib_set_flash(int id, void *value); // set mib value into system and flash
int mib_sys_to_default(CONFIG_DATA_T type);
int mib_flash_to_default(CONFIG_DATA_T type);
#if (defined VOIP_SUPPORT) && (defined CONFIG_USER_XMLCONFIG)
int mib_voip_to_default();
#endif
int mib_info_id(int id, mib_table_entry_T *info);
int mib_info_index(int index, mib_table_entry_T *info);
int mib_backup(CONFIG_MIB_T flag); /* backup the running mib into the backup buffer */
int mib_restore(CONFIG_MIB_T flag); /* restore the backup buffer into the running mib */
//TYPE_T mib_type(int id);
int mib_chain_total(int id);
int mib_chain_swap(int id, unsigned int recordNum1, unsigned int recordNum2);	/* cathy, to swap recordNum1 and recordNum2 of a chain */
int mib_chain_get(int id, unsigned int recordNum, void *ptr); /* get the specified chain record */
int mib_chain_add(int id, void* ptr); /* add chain record */
int mib_chain_add_flash(int id, void* ptr); /* add chain record into system and flash*/
int mib_chain_delete(int id, unsigned int recordNum); /* delete the specified chain record */
int mib_chain_clear(int id); /* clear chain record */
// for message logging
int mib_chain_update(int id, void* ptr, unsigned int recordNum); /* log updating the specified chain record */
int mib_chain_info_id(int id, mib_chain_record_table_entry_T *info);
int mib_chain_info_index(int index, mib_chain_record_table_entry_T *info);
int mib_chain_info_name(char *name, mib_chain_record_table_entry_T *info);
int mib_chain_desc_id(int id, mib_chain_member_entry_T *desc);
int mib_check_desc();	/* Check the consistency between chain records and their record descriptors */

int cmd_reboot(void);
int cmd_get_bootmode(void);
int cmd_killproc(unsigned int mask);
int cmd_upload(const char *filename, int offset, int imgFileSize);
#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
int cmd_check_image(const char *filename, int offset);
#endif
int cmd_start_autohunt(void);
int cmd_file2xml(const char *filename, const char *xmlname);
int cmd_xml2file(const char *xmlname, const char *filename);
#ifdef CONFIG_IPV6
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411) && defined(CONFIG_USER_RADVD)
int cmd_delegation(const char *filename);
#endif
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
int cmd_stop_delegation(const char *filename);
int cmd_get_PD_prefix_ip(void *value);
int cmd_get_PD_prefix_len();
#endif
#endif

#ifdef CONFIG_RTL8676_CHECK_WIFISTATUS
int cmd_wlan_delay_restart();
#endif

///added by ql
#ifdef	RESERVE_KEY_SETTING
int mib_retrive_table(int id);
int mib_retrive_chain(int id);
#endif
int mib_set_PPPoE(int cmd, void *value, unsigned int length);
#endif // INCLUDE_SYSCONFIG_H

int cmd_set_dns_config(const char *ifname);

#ifdef CONFIG_USER_XMLCONFIG
#define current_file "/var/config/lastgood.xml"
#define backup_file "/tmp/config.xml"
#endif	/*CONFIG_USER_XMLCONFIG*/

#ifdef CONFIG_REMOTE_CONFIGD
int rconfigd_init(void);
void rconfigd_exit(void);
int mib_info_id_via_rconfigd(int id, mib_table_entry_T * info);
int mib_get_via_rconfigd(int id, void *value);
int mib_set_via_rconfigd(int id, void *value);
int mib_update_via_rconfigd(CONFIG_DATA_T type, CONFIG_MIB_T flag);
int cmd_reboot_via_rconfigd(void);
int mib_chain_total_via_rconfigd(int id);
int mib_chain_info_id_via_rconfigd(int id, mib_chain_record_table_entry_T * info);
int mib_chain_get_via_rconfigd(int id, unsigned int recordNum, void *value);
int mib_chain_update_via_rconfigd(int id, void *ptr, unsigned int recordNum);
int mib_chain_add_via_rconfigd(int id, void *ptr);
int mib_chain_delete_via_rconfigd(int id, unsigned int recordNum);
int mib_chain_clear_via_rconfigd(int id);
int mib_flash_to_default_via_rconfigd(CONFIG_DATA_T type);
int cmd_check_image_via_rconfigd(const char *filename, int offset);
int cmd_upload_via_rconfigd(const char *filename, int offset, int imgFileSize);
#endif
