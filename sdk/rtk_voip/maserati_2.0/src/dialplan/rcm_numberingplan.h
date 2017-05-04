#ifndef __NUMBERINGPLAN_H__
#define __NUMBERINGPLAN_H__

#include "uglib.h"
#include "linphonecore.h"


//#include "Voip_types.h"

//feature key entries.
#define PrefixlistEntries 30

//facility define , follow TR-104 
typedef enum 
{
    CA_ACTIVATE 		= 0 ,
    X_CA_DEACTIVATE 	= 1 ,
    CFU_ACTIVATE 		= 2 ,
    CFU_DEACTIVATE 		= 3 ,
    CFB_ACTIVATE 		= 4,
    CFB_DEACTIVATE 		= 5 ,
    CFNR_ACTIVATE 		= 6 ,
    CFNR_DEACTIVATE 	= 7 ,
    CW_ACTIVATE 		= 8 ,
    CW_DEACTIVATE 		= 9 ,
    X_MCID_ACTIVATE	 	= 10 ,
    OCB_ACTIVATE 		= 11,
    OCB_DEACTIVATE 		= 12 ,
    X_DND_ACTIVATE 		= 13 ,
    X_DND_DEACTIVATE 	= 18,
    AA_REGISTER 		= 19,
    X_AA_ACTIVATE 		= 20,
    AA_ERASE 			= 21,
    X_HOTLINE_ACTIVATE 	= 22,
    X_HOTLINE_DEACTIVATE= 23,
	X_ALARM_ACTIVATE 	= 24,
	X_ALARM_DEACTIVATE 	= 25,
	X_ABSENT_ACTIVATE 	= 26,
	X_ABSENT_DEACTIVATE = 27,
	X_CFU_STATUS		=28,
	X_CFNR_STATUS		=29,
	X_CFB_STATUS=		30,
	X_CW_STATUS=		31
} FacilityActionEvent;


// define prefixinfo struct
typedef struct _prefixinfo
{
	char Prefixrange[40];			/* phone feature key , ex: *33# */
	uint8 PrefixMinNumberOfDigits;	/* minimum number of allowable digits */
	uint8 PrefixMaxNumberOfDigits;	/* maximum number of allowable digits , 	
									   Once the number of input digits reaches this value 
									   an outgoing request should be initiated. */

	FacilityActionEvent FacilityAction; /* string of Facility Action */
	gboolean remove_feature_key;		/* if match prefixrange, outgoing request must  remove Prefixrange */
	gboolean remove_pound_key;			/* if last digits is # key, outgoing request must remove it. */
	char featurename[60];				/* feature name for log */

}prefixinfo_t;




extern prefixinfo_t prefixinfolist[PrefixlistEntries]; //create phone feature key list

extern int rcm_checkPrefixKey( LinphoneCore *lc,SIGNSTATE s_sign, guint32 ssid, char *src_ptr);
extern int rcm_checkInternalPhoneBook( LinphoneCore *lc,SIGNSTATE s_sign, guint32 ssid, char *src_ptr);




#endif /* __NUMBERINGPLAN_H__ */
