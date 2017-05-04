#ifndef _CWMP_PORTING_H_
#define _CWMP_PORTING_H_

#include "libcwmp.h"

int port_init_userdata( struct cwmp_userdata *data );
int port_update_userdata( struct cwmp_userdata *data, int is_discon, int mib_id );
void port_save_reboot( struct cwmp_userdata *user, int reboot_flag );
void port_factoryreset_reboot(void);

//download & upload callback functions
int port_before_download( int file_type, char *target );
int port_after_download( int file_type, char *target );
int port_before_upload( int file_type, char *target );
int port_after_upload( int file_type, char *target );
int port_check_fw_header(char *buf, struct cwmp_userdata *data);

//notify_save
int port_notify_load_default();
int port_notify_save( char *name );

int port_before_session_start(struct cwmp_userdata *data);
int port_session_closed(struct cwmp_userdata *data);
int port_restart_wan(void);

#ifdef CONFIG_RTL8672_SAR
void port_setuppid(void);
#endif

//backup & restore config
int port_backup_config( void );
int port_restore_config( void );

#ifdef CONFIG_E8B
int port_is_dsl_mode_changed();	//return 1 if dsl mode is changed else return 0
int isTR069(char *name);
int isINTERNET(char *name);
#endif

int port_bindAddrOnTR069Interface(struct sockaddr_in *myaddr);
int port_get_tr069_ifname(char *ifname);
void port_acs_url_changed( struct cwmp_userdata *data );
int port_before_SetParameterValues(char *prmt_name);

void port_update_transfer_queue(struct node *root);

#endif /*#ifndef _CWMP_PORTING_H_*/
