// DeviceSwitcher.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DeviceSwitcher.h"
#include "DeviceSwitcherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CDeviceSwitcherApp

BEGIN_MESSAGE_MAP(CDeviceSwitcherApp, CWinApp)
END_MESSAGE_MAP()


// CDeviceSwitcherApp construction

CDeviceSwitcherApp::CDeviceSwitcherApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDeviceSwitcherApp object

CDeviceSwitcherApp		theApp;
CWinThread*				pDlgThread = NULL;
CDeviceSwitcherDlg*		pWnd = NULL;


// CDeviceSwitcherApp initialization

BOOL CDeviceSwitcherApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

UINT __cdecl Thread4ModalDialogFunction( LPVOID pParam )
{
	CDeviceSwitcherDlg dlg;
	pWnd = &dlg;
	dlg.DoModal();
	return FALSE;
}


extern "C" void PASCAL EXPORT LoadDeviceSwitcher( void )
{
	pDlgThread = AfxBeginThread(Thread4ModalDialogFunction,NULL);
}

extern "C" void PASCAL EXPORT UnLoadDeviceSwitcher( void )
{
	if (pDlgThread)
	{
		NOTIFYICONDATA tnd;

		tnd.cbSize		= sizeof(NOTIFYICONDATA);
		tnd.hWnd		= pWnd->m_hWnd;
		tnd.uID			= IDR_MAINFRAME;

		tnd.uFlags		= NIF_MESSAGE|NIF_ICON;

		tnd.uCallbackMessage	= MYWM_NOTIFYICON;

		tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;

		Shell_NotifyIcon(NIM_DELETE, &tnd);
	}
}

