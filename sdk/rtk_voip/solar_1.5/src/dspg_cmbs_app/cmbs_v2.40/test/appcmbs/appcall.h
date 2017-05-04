/*!
*	\file			appcall.h
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: appcall.h-1 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************
*	COPYRIGHT DOSCH & AMAND RESEARCH GMBH & CO.KG
*	DOSCH & AMAND RESEARCH GMBH & CO.KG Confidential
*
*******************************************************************************/

#if	!defined( APPCALL_H )
#define	APPCALL_H

typedef enum
{
   E_APPCALL_AUTOMAT_MODE_OFF,      /*!< call object automat mode off */
   E_APPCALL_AUTOMAT_MODE_ON        /*!< call object automat mode on, incoming and outgoing calls shall be handled in simple statemachine */
}E_APPCALL_AUTOMAT_MODE;

/*! \brief exchange structure between CMBS API layer and upper application */
typedef  struct
{
   E_CMBS_IE_TYPE e_IE;          /*! IE type */
   char *         psz_Value;     /*! string value in case of upper -> CMBS API layer */
}ST_APPCALL_PROPERTIES, * PST_APPCALL_PROPERTIES;

/*! \brief CMBS API layer call states */
typedef enum
{
   E_APPCMBS_CALL_CLOSE,         /*!<  line is closed */
   
   E_APPCMBS_CALL_INC_PEND,      /*!<  CMBS target is informed of an incoming call*/
   E_APPCMBS_CALL_INC_RING,      /*!<  CMBS target let the handset ringing */
   
   E_APPCMBS_CALL_OUT_PEND,      /*!<  CMBS-API layer received a outgoing call establishment event */
   E_APPCMBS_CALL_OUT_PEND_DIAL, /*!<  Digits will be collected in CLD array, if Dialtone was switched on.
                                       it is automatically switched off, if line enters this state */
   E_APPCMBS_CALL_OUT_INBAND,    /*!<  The outgoing line is set-up to carry inband signalling, e.g. network tones */
   E_APPCMBS_CALL_OUT_PROC,      /*!<  The outgoing line is proceeding state */
   E_APPCMBS_CALL_OUT_RING,      /*!<  The outgoing line is in ringing state, if not the inband tone is available */
   
   E_APPCMBS_CALL_ACTIVE,        /*!<  The line is in active mode, media shall be transmitted after channel start */
   E_APPCMBS_CALL_RELEASE,       /*!<  The line is in release mode */
   
}E_APPCMBS_CALL;

/*! \brief CBS API layer media state */
typedef enum
{
   E_APPCMBS_MEDIA_CLOSE,        /*!< Media entity is closed */
   E_APPCMBS_MEDIA_PEND,         /*!< Media entity is prepared, codec negotiated and channel ID from Target available */
   E_APPCMBS_MEDIA_ACTIVE        /*!< Media entity is started to stream */
} E_APPCMBS_MEDIA;

/*! \brief  Line/Call node, it contains every important information of the connection */
typedef struct
{
   u32   u32_CallInstance;             /*!< call instance to identify the call on CMBS */
   u8    u8_LineId;                    /*!< line Id */
   ST_IE_CALLEDPARTY  st_CallerParty;  /*!< Caller Party, incoming call CLI, outgoing call Handset number */
   ST_IE_CALLERPARTY  st_CalledParty;  /*!< Called Party, incoming call ringing mask, outgoing call to be dialled number */
   ST_IE_CALLERPARTY  st_TmpParty;     /*!< further feature, temp party, e.g. call waiting active and the CMBS API layer has to restore connection*/
   char  ch_TmpParty[30];              /*!< buffer of temp party number */
   char  ch_CallerID[30];              /*!< buffer of caller party number */
   char  ch_CalledID[30];              /*!< buffer of called party number */
   u32   u32_ChannelID;                /*!< channel ID to identify the media connection on CMBS */
   E_CMBS_AUDIO_CODEC   e_Codec;       /*!< used codec */

   E_APPCMBS_CALL  e_Call;             /*!< line/call state */
   E_APPCMBS_MEDIA e_Media;            /*!< mediaaaa entity state */
} ST_CALL_OBJ, * PST_CALL_OBJ;

                                       /*! \brief max connection for CMBS API layer */
#define     APPCALL_CALLOBJ_MAX        4

#if defined( __cplusplus )
extern "C"
{
#endif

void        appcall_Initialize (void);
void        appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE e_Mode );
int         appcall_EstablishCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties );
int         appcall_ReleaseCall( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI );
int         appcall_AnswerCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI );
int         appcall_ProgressCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI );
int         appcall_DisplayCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, int n_Line, char * psz_CLI );
void        appcall_ResumeCall( int n_Line, char * psz_CLI );
void        appcall_HoldCall( int n_Line, char * psz_CLI );

void        appmedia_CallOBJTonePlay( char * psz_Value, int bo_On, int nLine, char * psz_Cli );
void        appmedia_CallOBJMediaStart( u32 u32_CallInstance, int n_Line, char * psz_CLI );
void        appmedia_CallOBJMediaStop( u32 u32_CallInstance, int n_Line, char * psz_CLI );
void        appmedia_CallObjMediaOffer( int n_Line, char ch_Audio );

void        appcall_InfoPrint(void);


void        app_PrintCallProgrInfo( E_CMBS_CALL_PROGRESS e_Progress );
void        app_PrintCallInfoType( E_CMBS_CALL_INFO_TYPE e_Type );
//void        app_PrintCallInfo( E_CMBS_CALL_INFO e_Info );

#if defined( __cplusplus )
}
#endif

#endif	//	APPCALL_H
//*/
