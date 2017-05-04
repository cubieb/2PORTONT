/****************************************************************************
*  Program/file: CLRSX.H
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LHJ
*
*  MODULE: WIN32SIM
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*
*  DESCRIPTION: Main terminal window for production test and communication
*               with unit.
*
*
*
*
*
****************************************************************************/

/****************************************************************************
*                                  PVCS info
*****************************************************************************

$Author: krammer $
$Date: 2012/08/17 08:26:37 $
$Revision: 1.1 $
$Modtime:   21 Feb 2003 15:11:46  $

*/

#ifndef __CLRSX_H__
#define __CLRSX_H__


// Trace stuff.
#define CLRSX_MAX_NR_LINES              200
#define CLRSX_MAX_WIDTH                  80

#define RMEM_READ_BLOCK_SIZE   32

class clRsxType
{
   private:
      char chScreen[CLRSX_MAX_NR_LINES][CLRSX_MAX_WIDTH];
      long lColor[CLRSX_MAX_NR_LINES];
      int iLastLine, iNumberOfLines;
      int Yoffset;
      int iFirstPos;
      int cxClient, cyClient, cxChar, cyChar;
      HWND hwndLocal;
      int iXPos;
   public:
      int iPrintBytes;
      clRsxType(void);    // Constructor.
      void ReceiveMail(uint8 *Mail);
      void Printf(char *format, ...);
      void ClearScreen(HWND hwnd);
      void HandleKey(char ch);
      void EnterTestMode(void);
      void ExitTestMode(void);
      void OnWM_KEYDOWN(HWND hwnd, WPARAM wParam);
      void OnWM_CHAR(char cKey);
      void OnWM_VSCROLL(HWND hwnd, WPARAM wParam);
      void OnWM_HSCROLL(HWND hwnd, WPARAM wParam);
      void OnWM_SIZE(HWND hwnd, LPARAM lParam);
      void OnWM_PAINT(HWND hwnd);
      void OnWM_CREATE(HWND hwnd);

      OPENFILENAME ofn;

      uint16 bFrequency;
      uint8 bFreqType; // 0 = CTU, 1 = Infineon

      uint8 bQuadrature;
      uint8 bModulation;
      uint8 bRfPower;
      HWND hFrequencyDlg;
      HWND hQuadratureDlg;
      HWND hModulationDlg;
      HWND hRfPowerDlg;

      uint8 bContTxCarrier;
      uint8 bContTxPll;
      uint8 bContTxPa;

      uint8 bContRxCarrier;
      uint8 bContRxPll;

      uint8 bFig31Carrier;
      uint8 bFig31Pll;

      uint8 bToggleCarrier1;
      uint8 bToggleCarrier2;
      uint8 bTogglePll;
      HWND  hAtsaTestDlg;
      HWND  hAtsiTestDlg;
      HWND  hAtsTestDlg;
};



#endif
