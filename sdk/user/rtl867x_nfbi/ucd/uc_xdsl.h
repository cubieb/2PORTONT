#ifndef INCLUDE_UC_XDSL_H
#define INCLUDE_UC_XDSL_H


// ADSL MODE
#define ADSL_MODE_T1413		0x0001
#define ADSL_MODE_GDMT		0x0002
#define ADSL_MODE_GLITE		0x0008
#define ADSL_MODE_ADSL2		0x0010
#define ADSL_MODE_ANXL		0x0020
#define ADSL_MODE_ADSL2P	0x0040
#define ADSL_MODE_ANXM		0x0080
#define ADSL_MODE_ANXB		0x0100
#define ADSL_MODE_GINP		0x0200
#define ADSL_MODE_VDSL2		0x0400


#define VDSL2_PROFILE_8A	0x0001
#define VDSL2_PROFILE_8B	0x0002
#define VDSL2_PROFILE_8C	0x0004
#define VDSL2_PROFILE_8D	0x0008
#define VDSL2_PROFILE_12A	0x0010
#define VDSL2_PROFILE_12B	0x0020
#define VDSL2_PROFILE_17A	0x0040
#define VDSL2_PROFILE_30A	0x0080

extern char uc_xdsl_drv_get(unsigned int id, void *data, unsigned int len);
extern int uc_xdsl_init(void);
extern int uc_xdsl_setup(void);
#endif /*INCLUDE_UC_XDSL_H*/

