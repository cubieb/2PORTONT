/*!
*	\file		tcx_keyb.c
*	\brief		test command line generator for manual testing
*	\Author		kelbch
*
*	@(#)	%filespec: keyb_srv.c~11 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                \n
*	----------------------------------------------------------------------------\n
*   16-apr-09		kelbch		1.0		 Initialize \n
*   14-dec-09		sergiym		 ?		 Add start/stop log menu \n
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cmbs_platf.h"
#include "cmbs_api.h"
#include "cfr_ie.h"
#include "cfr_mssg.h"
#include "appcmbs.h"
#include "appsrv.h"
#include "appcall.h"
#include "cmbs_str.h"
#include "tcx_keyb.h"
#include "ListsApp.h"

/*****************************************************************************
*
*        Service Keyboard loop
*
******************************************************************************/
//		========== keyb_RFPISet  ===========
/*!
        \brief				 Set the RFPI of the CMBS Module

        \param[in,out]		 <none>

        \return				<none>

*/

void     keyb_ParamFlexGet( void )
{
    char  buffer[12];
    u16   u16_Location;

    printf( "Enter Location        (dec): " );
    tcx_gets( buffer, sizeof(buffer) );
    u16_Location = atoi( buffer );

    printf( "Enter Length (dec. max 512): " );
    tcx_gets( buffer, sizeof(buffer) );

    app_SrvParamAreaGet( CMBS_PARAM_AREA_TYPE_EEPROM, u16_Location, (u16)atoi(buffer), 1 );

}
void     keyb_RFPISet( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    char  buffer[12];
    u8    u8_RFPI[5];
    int   i,j=0;

    printf("\n");
    printf( "Enter RFPI: " );
    tcx_gets( buffer, sizeof(buffer) );

    for ( i=0; i< 10; i+=2 )
    {
        u8_RFPI[j] = app_ASC2HEX( buffer + i );
        j++;
    }

    printf("\n");
    app_SrvParamSet( CMBS_PARAM_RFPI, u8_RFPI, sizeof(u8_RFPI), 1 );

    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );

    printf("\n");
    printf( "Press Any Key!\n" );
    tcx_getch();

}

void     keyb_ECOModeSet( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    u8                   u8_Value;

    printf( "\nEnter 0 - to disable ECO mode, 1 - to enable ECO mode: " );

    switch ( tcx_getch() )
    {
    case '0':
        u8_Value = 0;
        break;

    case '1':
        u8_Value = 1;
        break;

    default:
        return;
    }

    app_ProductionParamSet( CMBS_PARAM_ECO_MODE, &u8_Value, sizeof(u8), 1 );

    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );

    printf( "\nPress Any Key!\n" );
    tcx_getch();

}


//		========== keyb_ChipSettingsSet ===========
/*!
        \brief				 set the tuning parameter of the chipset

        \param[in,out]		 <none>

        \return				<none>

*/

void     keyb_ChipSettingsSet( void )
{
    char  buffer[4];
    u8    u8_Value;
    ST_APPCMBS_CONTAINER st_Container;

    printf( "Enter RVBG: " );
    tcx_gets( buffer, sizeof(buffer) );
    if ( strlen(buffer) )
    {
        u8_Value = app_ASC2HEX(buffer);
        app_SrvParamSet( CMBS_PARAM_RVBG, &u8_Value, sizeof(u8_Value), 1 );

        appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
    }
    printf( "\nEnter RVREF: " );
    tcx_gets( buffer, sizeof(buffer) );
    if ( strlen(buffer) )
    {
        u8_Value = app_ASC2HEX(buffer);
        app_SrvParamSet( CMBS_PARAM_RVREF, &u8_Value, sizeof(u8_Value), 1 );

        appcmbs_WaitForContainer ( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
    }

    printf( "\nEnter RVTUN: " );
    tcx_gets( buffer, sizeof(buffer) );
    {
        u8_Value = app_ASC2HEX( buffer );
        app_SrvParamSet( CMBS_PARAM_RXTUN, &u8_Value, sizeof(u8_Value), 1 );

        appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
    }
}
//		========== keyb_ChipSettingsGet ===========
/*!
        \brief				 Shows the tuning parameter of the CMBS module

        \param[in,out]		 <none>

        \return				<none>

*/

void     keyb_ChipSettingsGet( void )
{
    u8    u8_Value[3];
    ST_APPCMBS_CONTAINER st_Container;

    app_SrvParamGet( CMBS_PARAM_RVBG, 1 );
    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
    u8_Value[0] = st_Container.ch_Info[0];

    app_SrvParamGet( CMBS_PARAM_RVREF, 1 );
    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
    u8_Value[1] = st_Container.ch_Info[0];

    app_SrvParamGet( CMBS_PARAM_RXTUN, 1 );
    appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
    u8_Value[2] = st_Container.ch_Info[0];

    tcx_appClearScreen();
    printf( "Chipset settings\n" );
    printf( "RVBG: %02x\n", u8_Value[0] );
    printf( "RVREF: %02x\n", u8_Value[1] );
    printf( "RVTUN: %02x\n", u8_Value[2] );
    printf( "Press Any Key!\n" );
    tcx_getch();
}
//		========== keypb_EEPromParamGet ===========
/*!
        \brief         Handle EEProm Settings get
      \param[in,ou]  <none>
        \return

*/

void keypb_param_test()
{
    int i;
    ST_APPCMBS_CONTAINER        st_Container;

    memset( &st_Container,0,sizeof(st_Container) );

    for ( i = 0; i <30; i++ )
    {
        app_SrvParamGet( CMBS_PARAM_RFPI, 1 );
        // wait for CMBS target message
        appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );

/*        memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );
        if(st_Resp.e_Response != CMBS_RESPONSE_OK)
        {
            printf("******** ERRROR CMBS_PARAM_RFPI!!!  on %d try ********\n",i);
        }
*/

        memset( &st_Container,0,sizeof(st_Container) );
        app_SrvParamGet( CMBS_PARAM_AUTH_PIN, 1 );
        // wait for CMBS target message
        appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_GET_RES, &st_Container );
/*        memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );
        if(st_Resp.e_Response != CMBS_RESPONSE_OK)
        {
            printf("******** ERRROR CMBS_PARAM_AUTH_PIN!!!  on %d try ********\n",i);
        }
*/        
        memset( &st_Container,0,sizeof(st_Container) );
        app_SrvFWVersionGet( 1 );
        // wait for CMBS target message
        appcmbs_WaitForContainer( CMBS_EV_DSR_FW_VERSION_GET_RES, &st_Container );
/*        memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );
        if(st_Resp.e_Response != CMBS_RESPONSE_OK)
        {
            printf("******** ERRROR CMBS_EV_DSR_FW_VERSION_GET_RES!!!  on %d try ********\n",i);
        }
*/        
        memset( &st_Container,0,sizeof(st_Container) );
        app_SrvLineSettingsGet(0xFF ,1);
        // wait for CMBS target message
        appcmbs_WaitForContainer( CMBS_EV_DSR_LINE_SETTINGS_LIST_GET_RES, &st_Container );
/*        memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );
        if(st_Resp.e_Response != CMBS_RESPONSE_OK)
        {
            printf("******** ERRROR CMBS_EV_DSR_LINE_SETTINGS_LIST_GET_RES!!!  on %d try ********\n",i);
        }
*/        
        memset( &st_Container,0,sizeof(st_Container) );
        app_SrvRegisteredHandsets(0xFF, 1);
        // wait for CMBS target message
        appcmbs_WaitForContainer( CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET_RES, &st_Container );
/*        memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );
        if(st_Resp.e_Response != CMBS_RESPONSE_OK)
        {
            printf("******** ERRROR CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET_RES!!!  on %d try ********\n",i);
        }
*/
    }
}

void  keypb_EEPromParamGet( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    int   nEvent;
//   int   i;

    memset( &st_Container,0,sizeof(st_Container) );

    printf( "Select EEProm Param:\n" );
    printf( "1 => RFPI\n" );
    printf( "2 => PIN\n" );
    printf( "3 => Chipset settings\n" );
/*
   printf( "4 => VBG\n" );
   printf( "5 => VREF\n" );
   printf( "6 => RXTUN\n" );
*/
    printf( "7 => TEST Mode\n" );
    printf( "8 => Master PIN\n" );
    printf( "9 => Flex EEprom get\n" );

    switch ( tcx_getch() )
    {
    case '1':
        app_SrvParamGet( CMBS_PARAM_RFPI, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '2':
        app_SrvParamGet( CMBS_PARAM_AUTH_PIN, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '3':
        keyb_ChipSettingsGet();
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        return;

    case '4':
        app_SrvParamGet( CMBS_PARAM_RVBG, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '5':
        app_SrvParamGet( CMBS_PARAM_RVREF, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '6':
        app_SrvParamGet( CMBS_PARAM_RXTUN, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '7':
        app_SrvParamGet( CMBS_PARAM_TEST_MODE, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '8':
        app_SrvParamGet( CMBS_PARAM_MASTER_PIN, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    case '9':
        keyb_ParamFlexGet();
        nEvent = CMBS_EV_DSR_PARAM_AREA_GET_RES;
        break;

    case '0':
        keypb_param_test();
        printf( "Press Any Key!\n" );
        tcx_getch();
        return;
        break;
    default:
        return;
    }
    // wait for CMBS target message
    appcmbs_WaitForContainer( nEvent, &st_Container );

    printf( "Press Any Key!\n" );
    tcx_getch();
}

//		========== keypb_ProductionParamGet ===========
/*!
        \brief         Handle Production Settings get
        \param[in,ou]  <none>
        \return

*/

void  keypb_ProductionParamGet( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    int   nEvent;

    memset( &st_Container,0,sizeof(st_Container) );

    printf( "\nSelect Production Param:\n" );
    printf( "1 => ECO mode\n" );

    switch ( tcx_getch() )
    {
    case '1':
        app_ProductionParamGet( CMBS_PARAM_ECO_MODE, 1 );
        nEvent = CMBS_EV_DSR_PARAM_GET_RES;
        break;

    default:
        return;
    }
    // wait for CMBS target message
    appcmbs_WaitForContainer( nEvent, &st_Container );

    printf( "\nPress Any Key!\n" );
    tcx_getch();
}

void  keyb_ParamFlexSet( void )
{
    char buffer[256];
    u8   u8_Data[128];
    u16  u16_Len = 0;
    u16  u16_Location, i, x;

    printf( "Enter Location        (dec): " );
    tcx_gets( buffer, sizeof(buffer) );
    u16_Location = atoi( buffer );

    printf( "Enter Length (dec. max 512): " );
    tcx_gets( buffer, sizeof(buffer) );
    u16_Len = atoi( buffer );

    printf( "Enter Data    (hexadecimal): " );
    tcx_gets( buffer, u16_Len * 2 );

    for ( i=0, x=0; i < u16_Len; i++ )
    {
        u8_Data[i] = app_ASC2HEX( buffer+x );
        x += 2;
    }

    app_SrvParamAreaSet( CMBS_PARAM_AREA_TYPE_EEPROM, u16_Location, u16_Len, u8_Data, 1 );
}


void     keyb_RxtnGpioConnect( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    u32 u32_GPIO = 0;

    printf( "Enter GPIO (0x00 -0x1B): " );
    scanf("%X", &u32_GPIO);

    appcmbs_PrepareRecvAdd ( 1 );
    cmbs_dsr_RxtunGpioConnect(g_cmbsappl.pv_CMBSRef, (u16)u32_GPIO);
    appcmbs_WaitForContainer ( CMBS_EV_DSR_GPIO_CONNECT_RES, &st_Container );
}

void     keyb_RxtnGpioDisconnect( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    appcmbs_PrepareRecvAdd ( 1 );
    cmbs_dsr_RxtunGpioDisconnect(g_cmbsappl.pv_CMBSRef);
    appcmbs_WaitForContainer ( CMBS_EV_DSR_GPIO_DISCONNECT_RES, &st_Container );
}

void     keyb_StartATETest( void )
{
    ST_IE_ATE_SETTINGS   st_AteSettings;
    u8                   u8_Ans;

    tcx_appClearScreen();
    printf( "ATE test:" );
    printf( "\nEnter slot type( 0 - slot, 1 - double slot, 2 - long slot): " );
    u8_Ans = tcx_getch();
    switch ( u8_Ans )
    {
    case '0':
        st_AteSettings.e_ATESlotType = CMBS_ATE_SLOT_TYPE;
        break;

    case '1':
        st_AteSettings.e_ATESlotType = CMBS_ATE_SLOT_TYPE_DOUBLE;
        break;

    case '2':
        st_AteSettings.e_ATESlotType = CMBS_ATE_SLOT_TYPE_LONG;
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }
    printf( "\nEnter ATE test type(0 - TX, 1 - continuous TX): " );
    u8_Ans = tcx_getch();
    switch ( u8_Ans )
    {
    case '0':
        st_AteSettings.e_ATEType = CMBS_ATE_TYPE_TX;
        break;

    case '1':
        st_AteSettings.e_ATEType = CMBS_ATE_TYPE_CONTINUOUS_TX;
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }
    printf( "\nEnter Instance(0..9): " );
    u8_Ans = tcx_getch();
    st_AteSettings.u8_Instance = u8_Ans - '0';
    printf( "\nEnter Carrier(0..9): " );
    u8_Ans = tcx_getch();
    st_AteSettings.u8_Carrier = u8_Ans - '0';
    printf( "\nEnter Ant(0,1): " );
    u8_Ans = tcx_getch();
    st_AteSettings.u8_Ant = u8_Ans - '0';
    if ( st_AteSettings.e_ATEType == CMBS_ATE_TYPE_TX )
    {
        printf( "\nEnter Slot(0..3): " );
        u8_Ans = tcx_getch();
        st_AteSettings.u8_Slot = u8_Ans - '0';
        printf( "\nEnter Pattern(0..4: [0,0x22,0xF0,0xDD,Fig41]): " );
        u8_Ans = tcx_getch();
        st_AteSettings.u8_Pattern = u8_Ans - '0';
        printf( "\nEnter Power Level (0,1 or 2): " );
        u8_Ans = tcx_getch();
        st_AteSettings.u8_PowerLevel = u8_Ans - '0';
        printf( "\nEnter Normal Preamble(y/n): " );
        u8_Ans = tcx_getch();
        switch ( u8_Ans )
        {
        case 'y':
        case 'Y':
            st_AteSettings.u8_NormalPreamble = 1;
            break;

        case 'n':
        case 'N':
            st_AteSettings.u8_NormalPreamble = 0;
            break;

        default:
            printf("\nError parameter !\n");
            return;
        }
    }

    cmbs_dsr_ATETestStart( g_cmbsappl.pv_CMBSRef, &st_AteSettings );
    printf("\nPress any key !\n");
    tcx_getch();
}

void     keyb_LeaveATETest( void )
{
    cmbs_dsr_ATETestLeave( g_cmbsappl.pv_CMBSRef );
    printf("\nPress any key !\n");
    tcx_getch();
}


void keyb_ListAccess_DeleteList(void)
{
    u8 u8_Ans;

    tcx_appClearScreen();

    printf("List Access Delete List\n");
    printf("========================\n\n");
    printf("select list:\n");
    printf("1: Missed calls List\n");
    printf("2: Outgoing calls List\n");
    printf("3: Incoming accepted calls List\n");
    printf("4: Line Settings List\n");
    printf("5: Contact List\n");

    printf("\n");

    u8_Ans = tcx_getch();

    switch ( u8_Ans )
    {
    case '1':
        {
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_MISSED_CALLS);

            List_DeleteAllEntries(LIST_TYPE_MISSED_CALLS);

            printf("Missed calls list cleared!");
        }
        break;

    case '2':
        {
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_OUTGOING_CALLS);

            List_DeleteAllEntries(LIST_TYPE_OUTGOING_CALLS);

            printf("Outgoing calls list cleared!");
        }
        break;

    case '3':
        {
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_INCOMING_ACCEPTED_CALLS);

            List_DeleteAllEntries(LIST_TYPE_INCOMING_ACCEPTED_CALLS);

            printf("Incoming Accepted calls list cleared!");
        }
        break;

    case '4':
        {
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_LINE_SETTINGS_LIST);

            List_DeleteAllEntries(LIST_TYPE_LINE_SETTINGS_LIST);

            printf("Line Settings list cleared!");
        }
        break;

    case '5':
        {
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_CONTACT_LIST);

            List_DeleteAllEntries(LIST_TYPE_CONTACT_LIST);

            printf("Contact list cleared!");
        }
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }
}

void keyb_ListAccess_DumpEntry(LIST_TYPE eListType, void* pv_Entry)
{
    const char *sTrue = "TRUE", *sFalse = "FALSE";

    switch ( eListType )
    {
    case LIST_TYPE_CONTACT_LIST:
        {
            stContactListEntry* pst_Entry = pv_Entry;
            const char *sNum1Type, *sNum2Type, *sWork = "WORK", *sMobile = "MOBILE", *sFixed = "FIXED";

            switch ( pst_Entry->cNumber1Type )
            {
            case NUM_TYPE_WORK:     sNum1Type = sWork;    break;
            case NUM_TYPE_MOBILE:   sNum1Type = sMobile;  break;
            default:                sNum1Type = sFixed;   break;
            }

            switch ( pst_Entry->cNumber2Type )
            {
            case NUM_TYPE_WORK:     sNum2Type = sWork;    break;
            case NUM_TYPE_MOBILE:   sNum2Type = sMobile;  break;
            default:                sNum2Type = sFixed;   break;
            }

            printf("Entry Id            = %d\n", pst_Entry->u32_EntryId);
            printf("Last Name           = %s\n", pst_Entry->sLastName);
            printf("First Name          = %s\n", pst_Entry->sFirstName);
            printf("Number 1            = %s\n", pst_Entry->sNumber1);
            printf("Number 1 Type       = %s\n", sNum1Type);
            printf("Number 1 Default    = %s\n", pst_Entry->bNumber1Default ? sTrue : sFalse);
            printf("Number 1 Own        = %s\n", pst_Entry->bNumber1Own ? sTrue : sFalse);
            printf("Number 2            = %s\n", pst_Entry->sNumber2);
            printf("Number 2 Type       = %s\n", sNum2Type);
            printf("Number 2 Default    = %s\n", pst_Entry->bNumber2Default ? sTrue : sFalse);
            printf("Number 2 Own        = %s\n", pst_Entry->bNumber2Own ? sTrue : sFalse);
            printf("Associated Melody   = %d\n", pst_Entry->u32_AssociatedMelody);
            printf("Line Id             = %d\n", pst_Entry->u32_LineId);
        }
        break;

    case LIST_TYPE_LINE_SETTINGS_LIST:
        {
            stLineSettingsListEntry* pst_Entry = pv_Entry;
            printf("Entry Id                            = %d\n", pst_Entry->u32_EntryId);
            printf("Line Name                           = %s\n", pst_Entry->sLineName);
            printf("Line Id                             = %d\n", pst_Entry->u32_LineId);
            printf("Attached Hs Mask                    = 0x%X\n", pst_Entry->u32_AttachedHsMask);
            printf("Dial Prefix                         = %s\n", pst_Entry->sDialPrefix);
            printf("FP Melody                           = %d\n", pst_Entry->u32_FPMelody);
            printf("FP Volume                           = %d\n", pst_Entry->u32_FPVolume);
            printf("Blocked Number                      = %s\n", pst_Entry->sBlockedNumber);
            printf("Multi-Call Enabled                  = %s\n", pst_Entry->bMultiCalls ? sTrue : sFalse);
            printf("Intrusion Enabled                   = %s\n", pst_Entry->bIntrusionCall ? sTrue : sFalse);
            printf("Permanent CLIR set                  = %s\n", pst_Entry->bPermanentCLIR ? sTrue : sFalse);
            printf("Permanent CLIR Activation code      = %s\n", pst_Entry->sPermanentCLIRActCode);
            printf("Permanent CLIR DeActivation code    = %s\n", pst_Entry->sPermanentCLIRDeactCode);
            printf("Call Fwd Unconditional Enabled      = %s\n", pst_Entry->bCallFwdUncond ? sTrue : sFalse);
            printf("Call Fwd Uncond Activation code     = %s\n", pst_Entry->sCallFwdUncondActCode);
            printf("Call Fwd Uncond DeActivation code   = %s\n", pst_Entry->sCallFwdUncondDeactCode);
            printf("Call Fwd Uncond Number              = %s\n", pst_Entry->sCallFwdUncondNum);
            printf("Call Fwd No Answer Enabled          = %s\n", pst_Entry->bCallFwdNoAns ? sTrue : sFalse);
            printf("Call Fwd No Ans Activation code     = %s\n", pst_Entry->sCallFwdNoAnsActCode);
            printf("Call Fwd No Ans DeActivation code   = %s\n", pst_Entry->sCallFwdNoAnsDeactCode);
            printf("Call Fwd No Answer Number           = %s\n", pst_Entry->sCallFwdNoAnsNum);
            printf("Call Fwd No Answer Timeout          = %d\n", pst_Entry->u32_CallFwdNoAnsTimeout);
            printf("Call Fwd Busy Enabled               = %s\n", pst_Entry->bCallFwdBusy ? sTrue : sFalse);
            printf("Call Fwd Busy Activation code       = %s\n", pst_Entry->sCallFwdBusyActCode);
            printf("Call Fwd Busy DeActivation code     = %s\n", pst_Entry->sCallFwdBusyDeactCode);
            printf("Call Fwd Busy Number                = %s\n", pst_Entry->sCallFwdBusyNum);
        }
        break;

    default:
        {
            stCallsListEntry* pst_Entry = pv_Entry;
            const char* sCallType;
            struct tm* pst_Time;
            pst_Time = gmtime(&(pst_Entry->t_DateAndTime));

            switch ( pst_Entry->cCallType )
            {
            case CALL_TYPE_MISSED:      sCallType = "Missed Call";      break;
            case CALL_TYPE_OUTGOING:    sCallType = "Outgoing Call";    break;
            default:                    sCallType = "Incoming Call";    break;
            }

            printf("Entry Id      = %d\n", pst_Entry->u32_EntryId);
            printf("Number        = %s\n", pst_Entry->sNumber);
            printf("Time - Year   = %d\n", pst_Time->tm_year + 1900);
            printf("Time - Month  = %d\n", pst_Time->tm_mon + 1);
            printf("Time - Day    = %d\n", pst_Time->tm_mday);
            printf("Time - Hour   = %d\n", pst_Time->tm_hour);
            printf("Time - Min    = %d\n", pst_Time->tm_min);
            printf("Time - Sec    = %d\n", pst_Time->tm_sec);
            printf("Line name     = %s\n", pst_Entry->sLineName);
            printf("Line ID       = %d\n", pst_Entry->u32_LineId);
            printf("Last name     = %s\n", pst_Entry->sLastName);
            printf("First name    = %s\n", pst_Entry->sFirstName);

            if ( eListType == LIST_TYPE_MISSED_CALLS || eListType == LIST_TYPE_ALL_INCOMING_CALLS )
            {
                printf("Read          = %s\n", pst_Entry->bRead ? sTrue : sFalse);
                printf("Num Of Calls  = %d\n", pst_Entry->u32_NumOfCalls);
            }
            else if ( eListType == LIST_TYPE_ALL_CALLS )
            {
                printf("Call Type     = %s\n", sCallType);
            }
        }
        break;
    }
}

void keyb_ListAccess_DumpList(void)
{
    u8 u8_Ans, pu8_Entry[LIST_ENTRY_MAX_SIZE];
    u32 u32_Count, u32_Index, pu32_Fields[FIELD_ID_MAX], u32_FieldsSize, u32_SortField, u32_NumOfEntries = 1;
    LIST_TYPE eListType;
    const char* sListName;

    tcx_appClearScreen();

    printf("List Access Display List Contents\n");
    printf("=================================\n\n");
    printf("select list:\n");
    printf("1: Contact List\n");
    printf("2: Line Settings List\n");
    printf("3: Missed calls List\n");
    printf("4: Outgoing Calls List\n");
    printf("5: Incoming Accepted Calls List\n");
    printf("6: All Calls List\n");
    printf("7: All Incoming Calls List\n");
    printf("\n");

    u8_Ans = tcx_getch();

    switch ( u8_Ans )
    {
    case '1':
        {
            eListType = LIST_TYPE_CONTACT_LIST;
            sListName = "Contacts";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_LAST_NAME;
            pu32_Fields[2] = FIELD_ID_FIRST_NAME;
            pu32_Fields[3] = FIELD_ID_CONTACT_NUM_1;
            pu32_Fields[4] = FIELD_ID_CONTACT_NUM_2;
            pu32_Fields[5] = FIELD_ID_ASSOCIATED_MELODY;
            pu32_Fields[6] = FIELD_ID_LINE_ID;
            u32_FieldsSize = 7;

            u32_SortField = FIELD_ID_LAST_NAME;
        }
        break;

    case '2':
        {
            eListType = LIST_TYPE_LINE_SETTINGS_LIST;
            sListName = "Line Settings";

            pu32_Fields[0]  = FIELD_ID_ENTRY_ID;
            pu32_Fields[1]  = FIELD_ID_LINE_NAME;
            pu32_Fields[2]  = FIELD_ID_LINE_ID;
            pu32_Fields[3]  = FIELD_ID_ATTACHED_HANDSETS;
            pu32_Fields[4]  = FIELD_ID_DIALING_PREFIX;
            pu32_Fields[5]  = FIELD_ID_FP_MELODY;
            pu32_Fields[6]  = FIELD_ID_FP_VOLUME;
            pu32_Fields[7]  = FIELD_ID_BLOCKED_NUMBER;
            pu32_Fields[8]  = FIELD_ID_MULTIPLE_CALLS_MODE;
            pu32_Fields[9]  = FIELD_ID_INTRUSION_CALL;
            pu32_Fields[10] = FIELD_ID_PERMANENT_CLIR;
            pu32_Fields[11] = FIELD_ID_CALL_FWD_UNCOND;
            pu32_Fields[12] = FIELD_ID_CALL_FWD_NO_ANSWER;
            pu32_Fields[13] = FIELD_ID_CALL_FWD_BUSY;
            u32_FieldsSize = 14;

            u32_SortField = FIELD_ID_LINE_ID;
        }
        break;

    case '3':
        {
            eListType = LIST_TYPE_MISSED_CALLS;
            sListName = "Missed Calls";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_NUMBER;            
            pu32_Fields[2] = FIELD_ID_LAST_NAME;         
            pu32_Fields[3] = FIELD_ID_FIRST_NAME;
            pu32_Fields[4] = FIELD_ID_DATE_AND_TIME;     
            pu32_Fields[5] = FIELD_ID_READ_STATUS;       
            pu32_Fields[6] = FIELD_ID_LINE_NAME;         
            pu32_Fields[7] = FIELD_ID_LINE_ID;           
            pu32_Fields[8] = FIELD_ID_NUM_OF_CALLS;
            u32_FieldsSize = 9;

            u32_SortField = FIELD_ID_DATE_AND_TIME;
        }
        break;

    case '4':
        {
            eListType = LIST_TYPE_OUTGOING_CALLS;
            sListName = "Outgoing Calls";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_NUMBER;            
            pu32_Fields[2] = FIELD_ID_LAST_NAME;         
            pu32_Fields[3] = FIELD_ID_FIRST_NAME;
            pu32_Fields[4] = FIELD_ID_DATE_AND_TIME;     
            pu32_Fields[5] = FIELD_ID_LINE_NAME;         
            pu32_Fields[6] = FIELD_ID_LINE_ID;           
            u32_FieldsSize = 7;

            u32_SortField = FIELD_ID_DATE_AND_TIME;
        }
        break;

    case '5':
        {
            eListType = LIST_TYPE_INCOMING_ACCEPTED_CALLS;
            sListName = "Incoming Accepted";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_NUMBER;            
            pu32_Fields[2] = FIELD_ID_LAST_NAME;         
            pu32_Fields[3] = FIELD_ID_FIRST_NAME;
            pu32_Fields[4] = FIELD_ID_DATE_AND_TIME;     
            pu32_Fields[5] = FIELD_ID_LINE_NAME;         
            pu32_Fields[6] = FIELD_ID_LINE_ID;           
            u32_FieldsSize = 7;

            u32_SortField = FIELD_ID_DATE_AND_TIME;
        }
        break;

    case '6':
        {
            eListType = LIST_TYPE_ALL_CALLS;
            sListName = "All Calls";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_CALL_TYPE;        
            pu32_Fields[2] = FIELD_ID_NUMBER;           
            pu32_Fields[3] = FIELD_ID_LAST_NAME;  
            pu32_Fields[4] = FIELD_ID_FIRST_NAME;
            pu32_Fields[5] = FIELD_ID_DATE_AND_TIME;    
            pu32_Fields[6] = FIELD_ID_LINE_NAME;        
            pu32_Fields[7] = FIELD_ID_LINE_ID;          
            u32_FieldsSize = 8;

            u32_SortField = FIELD_ID_DATE_AND_TIME;
        }
        break;

    case '7':
        {
            eListType = LIST_TYPE_ALL_INCOMING_CALLS;
            sListName = "All Incoming Calls";

            pu32_Fields[0] = FIELD_ID_ENTRY_ID;
            pu32_Fields[1] = FIELD_ID_NUMBER;            
            pu32_Fields[2] = FIELD_ID_LAST_NAME;         
            pu32_Fields[3] = FIELD_ID_FIRST_NAME;
            pu32_Fields[4] = FIELD_ID_DATE_AND_TIME;     
            pu32_Fields[5] = FIELD_ID_READ_STATUS;       
            pu32_Fields[6] = FIELD_ID_LINE_NAME;         
            pu32_Fields[7] = FIELD_ID_LINE_ID;           
            pu32_Fields[8] = FIELD_ID_NUM_OF_CALLS;
            u32_FieldsSize = 9;

            u32_SortField = FIELD_ID_DATE_AND_TIME;
        }
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }

    tcx_appClearScreen();

    printf("Contents of %s List:\n", sListName);
    printf("====================\n");

    /* Create if not exist */
    List_CreateList(eListType);

    List_GetCount(eListType, &u32_Count);
    printf("Total Num Of Entries = %d\n", u32_Count);

    for ( u32_Index = 0; u32_Index < u32_Count; ++u32_Index )
    {
        List_ReadEntries(eListType, u32_Index + 1, TRUE, MARK_LEAVE_UNCHANGED, pu32_Fields, u32_FieldsSize,
                         u32_SortField, FIELD_ID_INVALID, pu8_Entry, &u32_NumOfEntries);

        printf("Entry #%d of %d:\n", u32_Index + 1, u32_Count);
        printf("===================\n");
        keyb_ListAccess_DumpEntry(eListType, pu8_Entry);
        printf("**********************************************\n");
    }
}

void keyb_ListAccess_InsertEntry(void)
{
    u8 u8_Ans;
    char ps8_Buffer[10];
    stCallsListEntry         st_CallListEntry;
    stLineSettingsListEntry  st_LineSettingsListEntry;
    stContactListEntry       st_ContactEntry;
    time_t t_Time;

    memset(&st_CallListEntry,           0, sizeof(st_CallListEntry));
    memset(&st_LineSettingsListEntry,   0, sizeof(st_LineSettingsListEntry));

    tcx_appClearScreen();

    printf("List Access Insert Entry\n");
    printf("========================\n\n");
    printf("select list:\n");
    printf("1: Missed calls List\n");
    printf("2: Outgoing calls List\n");
    printf("3: Incoming accepted calls List\n");
    printf("4: Line Settings List\n");
    printf("5: Contact List\n");

    printf("\n");

    u8_Ans = tcx_getch();

    switch ( u8_Ans )
    {
    case '1':
        {
            u32 pu32_Fields[5], u32_FieldsNum = 4, u32_EntryId;
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_MISSED_CALLS);

            printf("Enter Number...\n");
            tcx_gets(st_CallListEntry.sNumber, sizeof(st_CallListEntry.sNumber));

            time(&t_Time);
            printf("Using current time as Date and Time...\n");
            st_CallListEntry.t_DateAndTime = t_Time;

            printf("Enter Line ID...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_CallListEntry.u32_LineId = atoi(ps8_Buffer);

            printf("Enter Number of calls...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_CallListEntry.u32_NumOfCalls = atoi(ps8_Buffer);

            pu32_Fields[0] = FIELD_ID_NUMBER;
            pu32_Fields[1] = FIELD_ID_DATE_AND_TIME;
            pu32_Fields[2] = FIELD_ID_LINE_ID;
            pu32_Fields[3] = FIELD_ID_NUM_OF_CALLS;

            List_InsertEntry(LIST_TYPE_MISSED_CALLS, &st_CallListEntry, pu32_Fields, u32_FieldsNum, &u32_EntryId);

            printf("Entry inserted. Entry Id = %d\n", u32_EntryId);
        }
        break;

    case '2':
    case '3':
        {
            u32 pu32_Fields[4], u32_FieldsNum = 3, u32_EntryId;
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList((u8_Ans == '2') ? LIST_TYPE_OUTGOING_CALLS : LIST_TYPE_INCOMING_ACCEPTED_CALLS);

            printf("Enter Number...\n");
            tcx_gets(st_CallListEntry.sNumber, sizeof(st_CallListEntry.sNumber));

            time(&t_Time);
            printf("Using current time as Date and Time...\n");
            st_CallListEntry.t_DateAndTime = t_Time;

            printf("Enter Line ID...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_CallListEntry.u32_LineId = atoi(ps8_Buffer);

            pu32_Fields[0] = FIELD_ID_NUMBER;
            pu32_Fields[1] = FIELD_ID_DATE_AND_TIME;
            pu32_Fields[2] = FIELD_ID_LINE_ID;

            List_InsertEntry((u8_Ans == '2') ? LIST_TYPE_OUTGOING_CALLS : LIST_TYPE_INCOMING_ACCEPTED_CALLS,
                             &st_CallListEntry, pu32_Fields, u32_FieldsNum, &u32_EntryId);

            printf("Entry inserted. Entry Id = %d\n", u32_EntryId);
        }
        break;

    case '4':
        {
            u32 pu32_Fields[13], u32_FieldsNum = 13, u32_EntryId;
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_LINE_SETTINGS_LIST);

            printf("Enter Line name...\n");
            tcx_gets(st_LineSettingsListEntry.sLineName, sizeof(st_LineSettingsListEntry.sLineName));

            printf("Enter Line ID...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.u32_LineId = atoi(ps8_Buffer);

            printf("Enter Attahced HS Mask (1 byte)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.u32_AttachedHsMask = atoi(ps8_Buffer);

            printf("Enter Dialing prefix...\n");
            tcx_gets(st_LineSettingsListEntry.sDialPrefix, sizeof(st_LineSettingsListEntry.sDialPrefix));

            printf("Enter FP Melody...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.u32_FPMelody = atoi(ps8_Buffer);

            printf("Enter FP Volume...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.u32_FPVolume = atoi(ps8_Buffer);

            printf("Enter Blocked telephone number...\n");
            tcx_gets(st_LineSettingsListEntry.sBlockedNumber, sizeof(st_LineSettingsListEntry.sBlockedNumber));

            printf("Enter Multicall enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bMultiCalls = atoi(ps8_Buffer);

            printf("Enter Intrusion enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bIntrusionCall = atoi(ps8_Buffer);

            printf("Enter Permanent CLIR enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bPermanentCLIR = atoi(ps8_Buffer);

            printf("Enter Permanent CLIR Activation code...\n");
            tcx_gets(st_LineSettingsListEntry.sPermanentCLIRActCode, sizeof(st_LineSettingsListEntry.sPermanentCLIRActCode));

            printf("Enter Permanent CLIR Deactivation code...\n");
            tcx_gets(st_LineSettingsListEntry.sPermanentCLIRDeactCode, sizeof(st_LineSettingsListEntry.sPermanentCLIRDeactCode));

            printf("Enter Call Fwd Uncond enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bCallFwdUncond = atoi(ps8_Buffer);

            printf("Enter Call Fwd Uncond Activation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdUncondActCode, sizeof(st_LineSettingsListEntry.sCallFwdUncondActCode));

            printf("Enter Call Fwd Uncond Deactivation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdUncondDeactCode, sizeof(st_LineSettingsListEntry.sCallFwdUncondDeactCode));

            printf("Enter Call Fwd Uncond Number...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdUncondNum, sizeof(st_LineSettingsListEntry.sCallFwdUncondNum));

            printf("Enter Call Fwd No Answer enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bCallFwdNoAns = atoi(ps8_Buffer);

            printf("Enter Call Fwd No Answer Activation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdNoAnsActCode, sizeof(st_LineSettingsListEntry.sCallFwdNoAnsActCode));

            printf("Enter Call Fwd No Answer Deactivation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdNoAnsDeactCode, sizeof(st_LineSettingsListEntry.sCallFwdNoAnsDeactCode));

            printf("Enter Call Fwd No Answer Number...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdNoAnsNum, sizeof(st_LineSettingsListEntry.sCallFwdNoAnsNum));

            printf("Enter Call Fwd No Answer Timeout [0..64]...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.u32_CallFwdNoAnsTimeout = atoi(ps8_Buffer);

            printf("Enter Call Fwd Busy enable (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_LineSettingsListEntry.bCallFwdBusy = atoi(ps8_Buffer);

            printf("Enter Call Fwd Busy Activation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdBusyActCode, sizeof(st_LineSettingsListEntry.sCallFwdBusyActCode));

            printf("Enter Call Fwd Busy Deactivation code...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdBusyDeactCode, sizeof(st_LineSettingsListEntry.sCallFwdBusyDeactCode));

            printf("Enter Call Fwd Busy Number...\n");
            tcx_gets(st_LineSettingsListEntry.sCallFwdBusyNum, sizeof(st_LineSettingsListEntry.sCallFwdBusyNum));

            pu32_Fields[0]  = FIELD_ID_LINE_NAME;
            pu32_Fields[1]  = FIELD_ID_LINE_ID;
            pu32_Fields[2]  = FIELD_ID_ATTACHED_HANDSETS;
            pu32_Fields[3]  = FIELD_ID_DIALING_PREFIX;
            pu32_Fields[4]  = FIELD_ID_FP_VOLUME;
            pu32_Fields[5]  = FIELD_ID_FP_MELODY;
            pu32_Fields[6]  = FIELD_ID_BLOCKED_NUMBER;
            pu32_Fields[7]  = FIELD_ID_MULTIPLE_CALLS_MODE;
            pu32_Fields[8]  = FIELD_ID_INTRUSION_CALL;
            pu32_Fields[9]  = FIELD_ID_PERMANENT_CLIR;
            pu32_Fields[10] = FIELD_ID_CALL_FWD_UNCOND;
            pu32_Fields[11] = FIELD_ID_CALL_FWD_NO_ANSWER;
            pu32_Fields[12] = FIELD_ID_CALL_FWD_BUSY;

            List_InsertEntry(LIST_TYPE_LINE_SETTINGS_LIST, &st_LineSettingsListEntry, pu32_Fields, u32_FieldsNum, &u32_EntryId);

            printf("Entry inserted. Entry Id = %d\n", u32_EntryId);
        }
        break;

    case '5':
        {
            u32 pu32_Fields[5], u32_FieldsNum = 5, u32_EntryId;
            tcx_appClearScreen();

            /* Create if not exist */
            List_CreateList(LIST_TYPE_CONTACT_LIST);

            printf("Enter Last name...\n");
            tcx_gets(st_ContactEntry.sLastName, sizeof(st_ContactEntry.sLastName));

            printf("Enter First name...\n");
            tcx_gets(st_ContactEntry.sFirstName, sizeof(st_ContactEntry.sFirstName));

            printf("Enter Number...\n");
            tcx_gets(st_ContactEntry.sNumber1, sizeof(st_ContactEntry.sNumber1));

            printf("Enter Number Type: 1=Fixed, 2=Mobile, 3=Work...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_ContactEntry.cNumber1Type = atoi(ps8_Buffer);

            printf("Enter Is Number Default (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_ContactEntry.bNumber1Default = atoi(ps8_Buffer);

            printf("Enter Is Number Own (0 / 1)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_ContactEntry.bNumber1Own = atoi(ps8_Buffer);

            printf("Enter Associated Melody (0-255)...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_ContactEntry.u32_AssociatedMelody = atoi(ps8_Buffer);

            printf("Enter Line ID...\n");
            tcx_gets(ps8_Buffer, sizeof(ps8_Buffer));
            st_ContactEntry.u32_LineId = atoi(ps8_Buffer);

            pu32_Fields[0]  = FIELD_ID_LAST_NAME;
            pu32_Fields[1]  = FIELD_ID_FIRST_NAME;
            pu32_Fields[2]  = FIELD_ID_CONTACT_NUM_1;
            pu32_Fields[3]  = FIELD_ID_ASSOCIATED_MELODY;
            pu32_Fields[4]  = FIELD_ID_LINE_ID;

            List_InsertEntry(LIST_TYPE_CONTACT_LIST, &st_ContactEntry, pu32_Fields, u32_FieldsNum, &u32_EntryId);

            printf("Entry inserted. Entry Id = %d\n", u32_EntryId);
        }
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }
}

void keyb_ListAccess( void )
{
    u8 u8_Ans;

    tcx_appClearScreen();

    printf("List Access Operation\n");
    printf("=====================\n\n");
    printf("select option:\n");
    printf("1: Insert Entry to List\n");
    printf("2: Delete List\n");
    printf("3: Display List contents\n");

    printf("\n");

    u8_Ans = tcx_getch();

    switch ( u8_Ans )
    {
    case '1':
        keyb_ListAccess_InsertEntry();
        break;

    case '2':
        keyb_ListAccess_DeleteList();
        break;

    case '3':
        keyb_ListAccess_DumpList();
        break;

    default:
        printf("\nError parameter !\n");
        return;
    }

    u8_Ans = tcx_getch();
    tcx_appClearScreen();
}





//		========== keypb_EEPromParamSet ===========
/*!
        \brief         Handle EEProm Settings set
      \param[in,ou]  <none>
        \return

*/
void  keypb_EEPromParamSet( void )
{
    char buffer[16];

    printf( "Select EEProm Param:\n" );
    printf( "1 => RFPI\n" );
    printf( "2 => PIN\n" );
    printf( "3 => Chipset Tunes\n" );
/*
   printf( "4 => VBG\n" );
   printf( "5 => VREF\n" );
   printf( "6 => RXTUN\n" );
*/
    printf( "4 => RXTUN GPIO connect\n" );
    printf( "5 => RXTUN GPIO disconnect\n" );

    printf( "7 => Test mode\n" );
    printf( "9 => Flex EEprom set\n" );
    printf( "0 => Reset EEprom\n" );


    switch ( tcx_getch() )
    {
    case '1':
        keyb_RFPISet();
/*
         printf("New RFPI value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RFPI, buffer, CMBS_PARAM_RFPI_LENGTH * 2 );
*/
        break;

    case '2':
        printf("\n");
        printf( "New PIN Code (default:ffff1590): " );
        tcx_gets( buffer, sizeof(buffer) );

        app_SrvPINCodeSet( buffer );
        printf( "Press Any Key!\n" );
        tcx_getch();
        break;

    case '3':
        keyb_ChipSettingsSet();
        break;
/*
      case '4':
         printf("New VBG value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RVBG, buffer, CMBS_PARAM_RVBG_LENGTH * 2 );
      break;

      case '5':
         printf("New VREF value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RVREF, buffer, CMBS_PARAM_RVREF_LENGTH * 2 );
      break;

      case '6':
         printf("New RXTUN value (hex): ");

         memset( buffer, 0, sizeof(buffer) );
         tcx_gets( buffer, sizeof(buffer) );
         app_SrvParamSet( CMBS_PARAM_RXTUN, buffer, CMBS_PARAM_RXTUN_LENGTH * 2 );
      break;
*/

    case '4':
        keyb_RxtnGpioConnect();
        break;

    case '5':
        keyb_RxtnGpioDisconnect();
        break;


    case '7':
        {
            ST_APPCMBS_CONTAINER st_Container;
            u8 u8_Value;

            printf( "New Test mode value (hex,00=>disabled,81=>TBR6): " );

            memset( buffer, 0, sizeof(buffer) );
            tcx_gets( buffer, sizeof(buffer) );

            if ( strlen(buffer) )
            {
                u8_Value = app_ASC2HEX(buffer);
                app_SrvParamSet( CMBS_PARAM_TEST_MODE, &u8_Value, sizeof(u8_Value), 1 );

                appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
            }
        }
        break;

    case '9':
        keyb_ParamFlexSet();
        break;

    case '0':
        {
            ST_APPCMBS_CONTAINER st_Container;
            u8 u8_Value = 0;

            printf( "Reset EEprom\n" );
            app_SrvParamSet( CMBS_PARAM_RESET_ALL, &u8_Value, sizeof(u8_Value), 1 );

            appcmbs_WaitForContainer( CMBS_EV_DSR_PARAM_SET_RES, &st_Container );
        }
        break;

    }
}

//		========== keypb_ProductionParamSet ===========
/*!
        \brief         Handle Production Settings set
        \param[in,ou]  <none>
        \return

*/
void  keypb_ProductionParamSet( void )
{
    printf( "Select EEProm Param:\n" );
    printf( "1 => ECO mode\n" );

    switch ( tcx_getch() )
    {
    case '1':
        keyb_ECOModeSet();
        break;

    default:
        return;
    }
}

//		========== keypb_HandsetPage ===========
/*!
        \brief         Page a handset or all handsets
      \param[in,ou]  <none>
        \return

*/
void  keypb_HandsetPage( void )
{
    char buffer[20];

    printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
    tcx_gets( buffer, sizeof(buffer) );

    app_SrvHandsetPage( buffer);

}

//      ========== keypb_HandsetStopPaging ===========
/*!
        \brief         Stop page handsets
      \param[in,ou]  <none>
        \return

*/
void  keypb_HandsetStopPaging( void )
{

    app_SrvHandsetStopPaging();

}


//		========== keypb_HandsetDelete ===========
/*!
        \brief         Delete a handset or all handsets
      \param[in,ou]  <none>
        \return

*/
void  keypb_HandsetDelete( void )
{
    char buffer[20];

    printf( "Enter handset mask ( e.g. 1,2,3,4 or none or all):\n" );
    tcx_gets( buffer, sizeof(buffer) );

    app_SrvHandsetDelete( buffer );

}

//		========== keypb_SYSRegistrationMode ===========
/*!
        \brief         Subscription on/off
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSRegistrationMode( void )
{
    printf( "1 => Registration open\n" );
    printf( "2 => Registration close\n" );

    switch ( tcx_getch() )
    {
    case '1':
        app_SrvSubscriptionOpen();
        break;
    case '2':
        app_SrvSubscriptionClose();
        break;
    }
}

//		========== keypb_SYSRegistrationRestart ===========
/*!
        \brief         Reboot CMBS Target
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSRegistrationRestart( void )
{
    app_SrvSystemReboot();
    printf( "Exit application\n" );
    //\todo cleanup
}

//		========== keypb_SYSFWVersionGet ===========
/*!
        \brief         Get current CMBS Target version
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSFWVersionGet( void )
{
    ST_APPCMBS_CONTAINER st_Container;
    PST_IE_FW_VERSION  p_IE;

    app_SrvFWVersionGet( TRUE );

    appcmbs_WaitForContainer( CMBS_EV_DSR_FW_VERSION_GET_RES, &st_Container );

    p_IE = (PST_IE_FW_VERSION)st_Container.ch_Info;

    printf( "FIRMWARE Version:\n " );
    switch ( p_IE->e_SwModule )
    {
    case CMBS_MODULE_CMBS:        printf( "CMBS Module " );     break;
    case CMBS_MODULE_DECT:        printf( "DECT Module " );     break;
    case CMBS_MODULE_DSP:         printf( "DSP Module " );      break;
    case CMBS_MODULE_EEPROM:      printf( "EEPROM Module " );   break;
    case CMBS_MODULE_USB:         printf( "USB Module " );      break;
    default:                      printf( "UNKNOWN Module " );
    }
    printf( "VER_%04x\n", p_IE->u16_FwVersion );

    printf( "Press Any Key!\n" );
    tcx_getch();
}


//		========== keypb_SysLogStart ===========
/*!
        \brief         Get current content of log buffer
      \param[in,ou]  <none>
        \return

*/
void  keypb_SysLogStart( void )
{
    app_SrvLogBufferStart();

    printf( "SysLog started\n" );
}

//		========== keypb_SysLogStop ===========
/*!
        \brief         Get current content of log buffer
      \param[in,ou]  <none>
        \return

*/
void  keypb_SysLogStop( void )
{
    app_SrvLogBufferStop();

    printf( "SysLog stopped\n" );
}

//		========== keypb_SysLogRead ===========
/*!
        \brief         Get current content of log buffer
      \param[in,ou]  <none>
        \return

*/
void  keypb_SysLogRead( void )
{
    ST_APPCMBS_CONTAINER st_Container;

    app_SrvLogBufferRead( TRUE );

    appcmbs_WaitForContainer( CMBS_EV_DSR_SYS_LOG, &st_Container );

    printf( "Press Any Key!\n" );
    tcx_getch();
}

/* == ALTDV == */

//      ========== keyb_SYSPowerOff ===========
/*!
        \brief         Power Off CMBS Target
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSPowerOff( void )
{
    app_SrvSystemPowerOff();
    printf("\n");
    printf( "Power Off CMBS Target\n" );
}

//      ========== keyb_RF_Control ===========
/*!
        \brief         Suspend/Resume RF on CMBS Target
        \param[in,ou]  <none>
        \return

*/
void keypb_RF_Control( void )
{
    printf("\n");
    printf( "1 => RF Suspend\n" );
    printf( "2 => RF Resume\n" );

    switch ( tcx_getch() )
    {
    case '1':
        app_SrvRFSuspend();
        break;
    case '2':
        app_SrvRFResume();
        break;
    }
}


//      ========== keypb_TurnOn_NEMo_mode ===========
/*!
        \brief         Turn On/Off NEMo mode on CMBS Target
        \param[in,ou]  <none>
        \return

*/
void keypb_Turn_On_Off_NEMo_mode(void)
{
    printf("\n");
    printf( "1 => Turn On NEMo mode\n" );
    printf( "2 => Turn Off NEMo mode\n" );

    switch ( tcx_getch() )
    {
    case '1':
        app_SrvTurnOnNEMo();
        break;
    case '2':
        app_SrvTurnOffNEMo();
        break;
    }
}

//      ========== keyb_RegisteredHandsets ===========
/*!
        \brief         Get List of subscribed handsets
        \param[in,ou]  <none>
        \return

*/
void  keypb_RegisteredHandsets( void )
{
    ST_APPCMBS_CONTAINER        st_Container;
    ST_IE_RESPONSE              st_Resp;
    u16                         u16_HsMask = 0;

    memset( &st_Container,0,sizeof(st_Container) );

    printf("\nEnter Mask of handsets in hex (FF for all): ");
    scanf("%hX", &u16_HsMask);

    app_SrvRegisteredHandsets( u16_HsMask, 1 );

    // wait for CMBS target message
    appcmbs_WaitForContainer( CMBS_EV_DSR_HS_SUBSCRIBED_LIST_GET_RES, &st_Container );

    memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );

    printf ( "\nSubscribed list get %s\n", st_Resp.e_Response == CMBS_RESPONSE_OK ? "Success": "Fail" );
    printf( "Press Any Key!\n" );
    tcx_getch();
}


//      ========== keyb_SetNewHandsetName ===========
/*!
        \brief         Set List of subscribed handsets
        \param[in,ou]  <none>
        \return

*/
void  keyb_SetNewHandsetName( void )
{
    u16     u16_HsID = 0;
    u8      u8_HsName[CMBS_HS_NAME_MAX_LENGTH+1];
    u16     u16_Name[CMBS_HS_NAME_MAX_LENGTH+1];
    u8      u8_Index = 0;
    u16     u16_NameSize = 0;

    memset (u8_HsName, 0, sizeof (u8_HsName));
    memset (u16_Name, 0, sizeof (u16_Name));

    printf("\nEnter handset ID : ");
    scanf("%hX", &u16_HsID);
    printf("\nEnter name (max 32 symbols): ");
    scanf("%s", u8_HsName);

    u16_NameSize = (u16)strlen((char*)u8_HsName) * 2;
    for ( u8_Index = 0; u8_Index < CMBS_HS_NAME_MAX_LENGTH; u8_Index++ )
    {
        u16_Name[u8_Index] = (u16)u8_HsName[u8_Index];
    }

    app_SrvSetNewHandsetName( u16_HsID, u16_Name, u16_NameSize, 1 );
}


//      ========== keypb_LineSettingsGet ===========
/*!
        \brief         Get List of Lines settings
        \param[in,ou]  <none>
        \return

*/
void  keypb_LineSettingsGet( void )
{
    ST_APPCMBS_CONTAINER        st_Container;
    ST_IE_RESPONSE              st_Resp;
    u16                         u16_LinesMask = 0;

    memset( &st_Container,0,sizeof(st_Container) );

    printf("\nEnter Mask of lines in hex (FF for all): ");
    scanf("%hX", &u16_LinesMask);

    app_SrvLineSettingsGet( u16_LinesMask, 1 );

    // wait for CMBS target message
    appcmbs_WaitForContainer( CMBS_EV_DSR_LINE_SETTINGS_LIST_GET_RES, &st_Container );

    memcpy( &st_Resp, &st_Container.ch_Info, st_Container.n_InfoLen );

    printf ( "\nLines settings list get %s\n", st_Resp.e_Response == CMBS_RESPONSE_OK ? "Success": "Fail" );
    printf( "Press Any Key!\n" );
    tcx_getch();
}


//      ========== keypb_LineSettingsSet ===========
/*!
        \brief         Set List of Lines settings
        \param[in,ou]  <none>
        \return

*/
void  keypb_LineSettingsSet( void )
{
    u16 u16_Line_Id = 0, u16_Temp;
    u8  u8_LineName[6];
    u8  u8_LineNameLen = 0;
    u8  u8_Index = 0;
    ST_IE_LINE_SETTINGS_LIST    st_LineSettingsList;
    ST_IE_LINE_SETTINGS_TYPE    st_LineSettingsType;

    memset( &st_LineSettingsList, 0, sizeof(st_LineSettingsList) );
    memset( &st_LineSettingsType, 0, sizeof(st_LineSettingsType) );

    printf("\nEnter line ID in hex: ");
    scanf("%hX", &u16_Line_Id);

    st_LineSettingsList.u8_Line_Id = (u8)u16_Line_Id;

    printf("\nEnter type of line setting: \n");
    printf( "1 => Set Line name\n" );
    printf( "2 => Set Handsets mask \n" );
    printf( "3 => Set Call Intrusion\n" );
    printf( "4 => Set Multiline Calls\n" );

    switch ( tcx_getch() )
    {
    case '1':
        st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_LINE_NAME;
        printf("\nEnter line name (max 5 symbols): ");
        scanf("%s", u8_LineName);
        u8_LineNameLen = (u8)strlen( (char*)u8_LineName );
        for ( u8_Index = 0; u8_Index < u8_LineNameLen; u8_Index++ )
        {
            st_LineSettingsList.u16_LineName[u8_Index] = (u16)u8_LineName[u8_Index];
        }
        st_LineSettingsList.u8_Line_NameLen = u8_LineNameLen * 2;
        break;

    case '2':
        st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_HS_MASK;
        printf("\nEnter Hs mask in hex:  ");
        scanf("%hX", &st_LineSettingsList.u16_Attached_HS);
        break;

    case '3':
        st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_CALL_INTRUSION;
        printf("\nEnter 0 - disable, 1 - enable: ");
        scanf("%hd", &u16_Temp);
        st_LineSettingsList.u8_Call_Intrusion = (u8)u16_Temp;
        break;

    case '4':
        st_LineSettingsType.e_LineSettingsType = CMBS_LINE_SETTINGS_TYPE_MULTIPLE_CALLS;
        printf("\nEnter 0 - disable, 1 - enable: ");
        scanf("%hd", &u16_Temp);
        st_LineSettingsList.u8_Multiple_Calls = (u8)u16_Temp;
        break;

    default:
        return;
    }

    app_SrvLineSettingsSet( &st_LineSettingsType, &st_LineSettingsList, 1 );
}


//		========== keypb_SYSTestModeGet ===========
/*!
        \brief         Get current CMBS Target test mode state
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSTestModeGet( void )
{
    app_SrvTestModeGet( TRUE );
}

//		========== keypb_SYSTestModeSet ===========
/*!
        \brief         enable TBR 6 mode
      \param[in,ou]  <none>
        \return

*/
void  keypb_SYSTestModeSet( void )
{
    app_SrvTestModeSet();
}

//		========== keyb_SRVLoop ===========
/*!
        \brief         keyboard loop of test application
      \param[in,ou]  <none>
        \return

*/
void  keyb_SRVLoop( void )
{
    int n_Keep = TRUE;

    while ( n_Keep )
    {
//      tcx_appClearScreen();
        printf( "-----------------------------\n" );
        printf( "Choose service:\n" );
        printf( "1 => EEProm  Param Get\n" );
        printf( "2 => EEProm  Param Set\n" );
        printf( "3 => Production  Param Get\n" );
        printf( "4 => Production  Param Set\n" );
        printf( "5 => Handset Page\n" );
        printf( "6 => Stop Paging\n" );
        printf( "7 => Handset Delete\n" );
        printf( "8 => System  Registration Mode\n" );
        printf( "9 => System  Restart\n" );
        printf( "A => System  FW Version Get\n" );
        printf( "B => System  Testmode Get\n" );
        printf( "C => System  Testmode Set\n" );
        printf( "D => SysLog  Start\n" );
        printf( "E => SysLog  Stop\n" );
        printf( "F => SysLog  Read\n" );
        printf( "G => List Of Registered Handsets\n" );
        printf( "H => Set new handset name\n" );
        printf( "I => RF Control\n" );
        printf( "J => Turn On/Off NEMo mode\n" );
        printf( "K => System  Power Off\n" );
        printf( "L => Get Line Settings\n" );
        printf( "M => Set Line Settings\n" );
        printf( "N => ATE test start\n" );
        printf( "O => ATE test leave\n" );
        printf( "P => List Access\n" );

        printf( "- - - - - - - - - - - - - - - \n" );
        printf( "q => Return to Interface Menu\n" );

        switch ( tcx_getch() )
        {
        case ' ':
            tcx_appClearScreen();
            break;

        case '1':
            keypb_EEPromParamGet();
            break;
        case '2':
            keypb_EEPromParamSet();
            break;
        case '3':
            keypb_ProductionParamGet();
            break;
        case '4':
            keypb_ProductionParamSet();
            break;
        case '5':
            keypb_HandsetPage();
            break;
        case '6':
            keypb_HandsetStopPaging();
            break;
        case '7':
            keypb_HandsetDelete();
            break;
        case '8':
            keypb_SYSRegistrationMode();
            break;
        case '9':
            keypb_SYSRegistrationRestart();
            break;
        case 'a':
        case 'A':
            keypb_SYSFWVersionGet();
            break;
        case 'b':
        case 'B':
            keypb_SYSTestModeGet();
            break;
        case 'c':
        case 'C':
            keypb_SYSTestModeSet();
            break;
        case 'd':
        case 'D':
            keypb_SysLogStart();
            break;
        case 'e':
        case 'E':
            keypb_SysLogStop();
            break;
        case 'f':
        case 'F':
            keypb_SysLogRead();
            break;
        case 'g':
        case 'G':
            keypb_RegisteredHandsets();
            break;
        case 'h':
        case 'H':
            keyb_SetNewHandsetName();
            break;
        case 'i':
        case 'I':
            keypb_RF_Control();
            break;
        case 'j':
        case 'J':
            keypb_Turn_On_Off_NEMo_mode();
            break;
        case 'k':
        case 'K':
            keypb_SYSPowerOff();
            break;
        case 'l':
        case 'L':
            keypb_LineSettingsGet();
            break;
        case 'm':
        case 'M':
            keypb_LineSettingsSet();
            break;

        case 'n':
        case 'N':
            keyb_StartATETest();
            break;
        case 'o':
        case 'O':
            keyb_LeaveATETest();
            break;

        case 'p':
        case 'P':
            keyb_ListAccess();
            break;

        case 'q':
            n_Keep = FALSE;
            break;

        }
    }
}

//*/
