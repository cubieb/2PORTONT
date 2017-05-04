#define CID_TABLE_TYPE	unsigned char
#define CID_MDMF_LEN 41
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF 	0x0	/* FOR DTMF BASED CID */
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF 	0x1	/* FOR FSK BASED CID*/

typedef enum	TMSG_TYPE_t
{
	CALL_SET_UP_SDMF	= 0x04,  
	SDMF_VMWI		= 0x06,
	MDMF_VMWI		= 0x82,
	CALL_SET_UP_MDMF	= 0x80,
	NTT_CLIP		= 0x40,
	NTT_CIDCW		= 0x41,	
	DEFAULT			= 0x00
}TMSG_TYPE;

typedef enum TPARAM_TYPE_t
{
	DATE_AND_TIME			=1,
	NUMBER				=2,
	CALLED_ID			=3,
	NUM_ABS				=4,   
	NAME				=7,
	NAME_ABS			=8,
	VMWI_STATUS			=11,
	COMPLMNTRY_CALLING		=16,
	CALL_TYPE			=17,
	FIRST_CALLED			=18,
	NETWORK_STATUS			=19,
	FORWARD_CALL			=21,
	CALLING_USER			=22,
	REDIRECTING_NUM			=26, 
	ORIGIN_NUM_EXPAN		=33/*NTT*/, 
	EXTENTION_NET			=224,
	PARAM_TYPE_NUM
}TPARAM_TYPE;

typedef enum TNTT_PARAM_TYPE_t
{
	NTT_DATE_AND_TIME		=0x01,
	NTT_NUMBER			=0x02,
	NTT_NUM_ABS			=0x04,   
	NTT_NAME			=0x07,
	NTT_NAME_ABS			=0x08,
	NTT_DDI_NUM			=0x09,
	NTT_ORIGIN_NUM			=0x0B,
	//?? =0x13,	
	NTT_NUMBER_EXPAN		=0x21,
	NTT_DDI_NUM_EXPAN		=0x22,
	NTT_ORIGIN_NUM_EXPAN		=0x23,
	NTT_PUBLIC_MESSAGE		=0x24, 

}TNTT_PARAM_TYPE;

#if 1
#define	SDMF_LEN			15
#define	MDMF_LEN1			39
#define	MDMF_LEN2			16
#define	VMWI_SDMF_LEN		3
#define	VMWI_MDMF_LEN	3+MDMF_LEN1

#define	MDMF_ETSI_LEN1  	81
#define	MDMF_ETSI_LEN2  	118+9	   		//118
#define	MDMF_ETSI_LEN3  	70  				//MDMF_ETSI_LEN1-11

#define	VMWI_ETSI_LEN1  	42
#define	VMWI_ETSI_LEN2  	99

CID_TABLE_TYPE	_CID_MDMF1[39+2]=             //MDMF_Bellcore
{	// Message Type
	CALL_SET_UP_MDMF,
	// Message Length
	MDMF_LEN1,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// Paramater Type
	NUMBER,
	// Paramater Length
	10,
	// digits
	'0','1','2','3','4','5','6','7','0','1',
	// Paramater Type
	NAME,
	// Paramater Length
	15,
	// name
	'a','b','c','d','e','f','g','h','i','k','l','m','C','H','1'
};

CID_TABLE_TYPE	_CID_MDMF2[MDMF_LEN2+2]=             //MDMF_Bellcore
{	// Message Type
	CALL_SET_UP_MDMF,
	// Message Length
	MDMF_LEN2,
	//Parameter Type
	NAME_ABS,
	//Parameter Length
	1,
	//Reason
	'P', //'O',
	// Paramater Type
	NUM_ABS,
	// Paramater Length
	1,
	// Reason
	'O', //'P',
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0'
};

#if defined(ACL_CID_TEST)
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF 0x0
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF 0x1
#define CID_MDMF_LEN 41

#define ACL_CID_TEST_MACRO() \
{\
\
\
	extern int Ac49xSendCallerIdMessage(int Device, int Channel, Tac49xCallerIdAttr *pAttr, U8 *pMessage);\
\
	Tac49xCallerIdAttr callerIdAttr[2]={{0}};\
	Tac49xCallerIdAttr *pCallerIdAttr;\
	Tac49xCallerIdAsciMessage *pCallerIdAsciMessage={{0}};\
	Tac49xCallerIdAsciMessage callerIdAsciMessage[2];\
\
	pCallerIdAttr = &callerIdAttr[0];\
	pCallerIdAttr->Standard = CALLER_ID_STANDARD__ETSI;\
	pCallerIdAttr->ServiceType = CALLER_ID_SERVICE_TYPE__1_ON_HOOK;\
	pCallerIdAttr->EtsiOnhookMethod = ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;\
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;\
	pCallerIdAttr->Event = CALLER_ID_EVENT__NONE_EVENT;\
\
	pCallerIdAsciMessage = &callerIdAsciMessage[0];\
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;\
	pCallerIdAsciMessage->Title.Length.Value = (char)41;\
	memcpy(&pCallerIdAsciMessage->Data[0], (char *)&_CID_MDMF1[0], pCallerIdAsciMessage->Title.Length.Value);\
\
	pCallerIdAttr = &callerIdAttr[1];\
	pCallerIdAttr->Standard = CALLER_ID_STANDARD__TELCORDIA_BELLCORE;\
	pCallerIdAttr->ServiceType = CALLER_ID_SERVICE_TYPE__2_OFF_HOOK;\
	pCallerIdAttr->EtsiOnhookMethod = ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;\
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;\
	pCallerIdAttr->Event = CALLER_ID_EVENT__NONE_EVENT;\
\
	pCallerIdAsciMessage = &callerIdAsciMessage[1];\
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;\
	pCallerIdAsciMessage->Title.Length.Value = (char)18;\
	memcpy(&pCallerIdAsciMessage->Data[0], (char *)&_CID_MDMF2[0], pCallerIdAsciMessage->Title.Length.Value);\
\
\
	pCallerIdAsciMessage = &callerIdAsciMessage[channel];\
	pCallerIdAttr	= &callerIdAttr[channel];\
	Ac49xSendCallerIdMessage(0, channel, pCallerIdAttr, pCallerIdAsciMessage);\
\
	printk("CID Test <ch-%d>\n",channel);\
}
#endif

#else

/******************************************************************************
		Caller ID sample tables
 ******************************************************************************/
#define	MDMF_LEN		27
CID_TABLE_TYPE	OLD_MDMF[MDMF_LEN+2]=
{	// Message Type
	CALL_SET_UP_MDMF,
	// Message Length
	MDMF_LEN,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// Paramater Type
	NUMBER,
	// Paramater Length
	7,
	// digits
	'5', '3', '9' , '4' , '0' , '0' , '0' ,
	// Paramater Type
	NAME,
	// Paramater Length
	6,
	// name
	'A', 'B', 'C', 'D', 'E', 'F'
};


/******************************************************************************
		Caller ID sample tables
 ******************************************************************************/
#define	SDMF_LEN			15
#define	MDMF_LEN1			39
#define	MDMF_LEN2			16
#define	VMWI_SDMF_LEN		3
#define	VMWI_MDMF_LEN	3+MDMF_LEN1

#define	MDMF_ETSI_LEN1  	81
#define	MDMF_ETSI_LEN2  	118+9	   		//118
#define	MDMF_ETSI_LEN3  	70  				//MDMF_ETSI_LEN1-11

#define	VMWI_ETSI_LEN1  	42
#define	VMWI_ETSI_LEN2  	99

// CID messages
CID_TABLE_TYPE	CID_SDMF[SDMF_LEN+2]=              //SDMF_Bellcore
{	// Message Type
	CALL_SET_UP_SDMF,
	// Message Length
	SDMF_LEN,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// digits
	'5','3','9','4','0','0','0',
};

CID_TABLE_TYPE	CID_MDMF1[MDMF_LEN1+2]=             //MDMF_Bellcore
{	// Message Type
	CALL_SET_UP_MDMF,
	// Message Length
	MDMF_LEN1,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// Paramater Type
	NUMBER,
	// Paramater Length
	10,
	// digits
	'0','1','2','3','4','5','6','7','0','1',
	// Paramater Type
	NAME,
	// Paramater Length
	15,
	// name
	'a','b','c','d','e','f','g','h','i','k','l','m','C','H','1'
};


CID_TABLE_TYPE	CID_MDMF2[MDMF_LEN2+2]=             //MDMF_Bellcore
{	// Message Type
	CALL_SET_UP_MDMF,
	// Message Length
	MDMF_LEN2,
	//Parameter Type
	NAME_ABS,
	//Parameter Length
	1,
	//Reason
	'P', //'O',
	// Paramater Type
	NUM_ABS,
	// Paramater Length
	1,
	// Reason
	'O', //'P',
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0'
};


// VMWI messages
CID_TABLE_TYPE	VMWI_SDMF[VMWI_SDMF_LEN+2]=              //VMWI SDMF Bellcore
{	// Message Type
	SDMF_VMWI,
	// Message Length
	VMWI_SDMF_LEN,
	// Indicator On
	'A', 'B', 'C'
	// Indicator OFF
	//'o', 'o', 'o'
};

CID_TABLE_TYPE	VMWI_MDMF[VMWI_MDMF_LEN+2]=            //VMWI MDMF Bellcore
{	// Message Type
	MDMF_VMWI,
	// Message Length
	VMWI_MDMF_LEN,
	// Paramater Type
	VMWI_STATUS,
	// Paramater Length
	1,
	// Indicator on
	0xff,
	// Indicator off
	//0x00
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// Paramater Type
	NUMBER,
	// Paramater Length
	10,
	// digits
	'0','1','2','3','4','5','6','7','8','9',
	// Paramater Type
	NAME,
	// Paramater Length
	15,
	// name
	'a','b','c','d','e','f','g','h','i','k','l','m','n','o','j'
};



//ETSI
CID_TABLE_TYPE MDMF_ETSI1[MDMF_ETSI_LEN1+2]=
{   //Message Type
	CALL_SET_UP_MDMF,
	//Message Lenght
	MDMF_ETSI_LEN1,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',
	// Paramater Type
	NUMBER,
	// Paramater Length
	7,
	// digits
	'5', '3', '9' , '4' , '0' , '0' , '0' ,
	// Paramater Type
	NAME,
	// Paramater Length
	6,
	// name
	'A', 'B', 'C', 'D' ,'E', 'F',
	// Parameter Type
	CALLED_ID,
	// Paramater Length
	7,
	// digits
	'1', '2', '3','4','5','6','7',
	// Parameter Type
	COMPLMNTRY_CALLING,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',
	// Parameter Type
	CALL_TYPE,
	// Paramater Length
	1,
	// Voice Call
	1,
	// Parameter Type
	FIRST_CALLED,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',
	// Parameter Type
	NETWORK_STATUS,
	// Paramater Length
	1,
	// No messages
	1,
	// Parameter Type
	FORWARD_CALL,
	// Paramater Length
	1,
	// Forwarded call on busy
	1,
	// Parameter Type
	CALLING_USER,
	// Paramater Length
	1,
	// Mobile phone
	4,
	// Parameter Type
	REDIRECTING_NUM,
	//
	5,
	// digits
	'0','0','0','0','0',
	// Parameter Type
	EXTENTION_NET,
	// Paramater Length
	10,
	// Country Code
	'9','7','2',
	//Network Operator Code
	'1','2','3','4',
	//Version (operating coding)
	'1','1','1'
};




CID_TABLE_TYPE MDMF_ETSI2[MDMF_ETSI_LEN2+2]=
{   //Message Type
	CALL_SET_UP_MDMF,
	//Message Lenght
	MDMF_ETSI_LEN2,
	// Paramater Type
	NUMBER,
	// Paramater Length
	20,
	// digits
	'0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',
	// Paramater Type
	NAME,
	// Paramater Length
	30+9,
	// name
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e', 
	// Parameter Type
	COMPLMNTRY_CALLING,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',
	// Parameter Type
	CALL_TYPE,
	// Paramater Length
	1,
	// Voice Call
	1,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',

	// Parameter Type
	FIRST_CALLED,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',

	// Parameter Type
	FORWARD_CALL,
	// Paramater Length
	1,
	// Forwarded call on busy
	1,

	// Parameter Type
	CALLING_USER,
	// Paramater Length
	1,
	// Mobile phone
	4,

	// Parameter Type
	REDIRECTING_NUM,
	//
	5,
	// digits
	'0','0','0','0','0',


	// Parameter Type
	EXTENTION_NET,
	// Paramater Length
	10,
	// Country Code
	'9','7','2',
	//Network Operator Code
	'1','2','3','4',
	//Version (operating coding)
	'1','1','1',

	// Parameter Type
	NETWORK_STATUS,
	// Paramater Length
	1,
	// No messages
	1,

	// Parameter Type
	CALLED_ID,
	// Paramater Length
	7,
	// digits
	'1', '2', '3','4','5','6','7'

};




CID_TABLE_TYPE MDMF_ETSI3[MDMF_ETSI_LEN3+2]=
{   //Message Type
	CALL_SET_UP_MDMF,
	//Message Lenght
	MDMF_ETSI_LEN3,
	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',

	/*// Paramater Type
	NUMBER,
	// Paramater Length
	7,
	// digits
	'5', '3', '9' , '4' , '0' , '0' , '0' ,

	// Paramater Type
	NAME,
	// Paramater Length
	6,
	// name
	'A', 'B', 'C', 'r', 'o', 'n',*/ //-17

 
	//Parameter Type
	NAME_ABS,
	//Parameter Length
	1,
	//Reason
	'P', //'O',

	// Paramater Type
	NUM_ABS,
	// Paramater Length
	1,
	// Reason
	'O', //'P',      //+6

	// Parameter Type
	CALLED_ID,
	// Paramater Length
	7,
	// digits
	'1', '2', '3','4','5','6','7',

	// Parameter Type
	COMPLMNTRY_CALLING,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',

	// Parameter Type
	CALL_TYPE,
	// Paramater Length
	1,
	// Voice Call
	1,

	// Parameter Type
	FIRST_CALLED,
	// Paramater Length
	5,
	// digits
	'1', '2', '3','4','5',

	// Parameter Type
	NETWORK_STATUS,
	// Paramater Length
	1,
	// No messages
	1,

	// Parameter Type
	FORWARD_CALL,
	// Paramater Length
	1,
	// Forwarded call on busy
	1,

	// Parameter Type
	CALLING_USER,
	// Paramater Length
	1,
	// Mobile phone
	4,

	// Parameter Type
	REDIRECTING_NUM,
	//
	5,
	// digits
	'0','0','0','0','0',

 	// Parameter Type
	EXTENTION_NET,
	// Paramater Length
	10,
	// Country Code
	'9','7','2',
	//Network Operator Code
	'1','2','3','4',
	//Version (operating coding)
	'1','1','1'

};

CID_TABLE_TYPE VMWI_ETSI1[VMWI_ETSI_LEN1+2]=
{
	//Message Type
	MDMF_VMWI,
	// Message Lenght
	VMWI_ETSI_LEN1,

	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',

	// Paramater Type
	NUMBER,
	// Paramater Length
	7,
	// digits
	'5', '3', '9' , '4' , '0' , '0' , '0' ,

	// Paramater Type
	NAME,
	// Paramater Length
	6,
	// name
	'A', 'B', 'C', 'r', 'o', 'n',


	// Parameter Type
	NETWORK_STATUS,
	// Paramater Length
	1,
	// No messages
	0,


	// Parameter Type
	EXTENTION_NET,
	// Paramater Length
	10,
	// Country Code
	'9','7','2',
	//Network Operator Code
	'1','2','3','4',
	//Version (operating coding)
	'1','1','1'
};



CID_TABLE_TYPE VMWI_ETSI2[VMWI_ETSI_LEN2+2]=
{
	//Message Type
	MDMF_VMWI,
	// Message Lenght
	VMWI_ETSI_LEN2,

	// Paramater Type
	DATE_AND_TIME,
	// Paramater Length
	8,
	// Month
	'0', '5',
	// Day
	'2', '5',
	// Hour
	'1', '8',
	//Minute
	'0', '0',

	// Paramater Type
	NUMBER,
	// Paramater Length
	20,
	// digits
	'0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9',

	// Paramater Type
	NAME,
	// Paramater Length
	50,
	// name
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',
	'A','B','C','r','o','n','A','l','e','x',


    // Parameter Type
	NETWORK_STATUS,
	// Paramater Length
	1,
	// No messages
	0,


	// Parameter Type
	EXTENTION_NET,
	// Paramater Length
	10,
	// Country Code
	'9','7','2',
	//Network Operator Code
	'1','2','3','4',
	//Version (operating coding)
	'1','1','1'

};



#define NTT_LEN1           17				//message only
CID_TABLE_TYPE CID_NTT1[12+2]=
{
	////////----------Data-------//////////
	//Type of service
	0x41,
	//Message content length
	12,
	// Parameter Type-originating telephone number(0x02=ETSI)
	NUMBER,
	// Paramater Length
	10,
	//digits
	'0', '1', '2' , '3' , '4' , '5' , '6', '7', '8', '9',
};

#endif

