// todel-1Dlg.cpp : implementation file
//

#include "stdafx.h"
//#include "DeviceSwitcher.h"
#include "DeviceSwitcherDlg.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDeviceSwitcherDlg::CDeviceSwitcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceSwitcherDlg::IDD, pParent)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nTimer = 0;
}

CDeviceSwitcherDlg::~CDeviceSwitcherDlg()
{
	//TrayMessage(NIM_DELETE);
}

void CDeviceSwitcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

//	DDX_Control(pDX, IDR_MAINFRAME, m_trayMenu);

}

BEGIN_MESSAGE_MAP(CDeviceSwitcherDlg, CDialog)
	ON_WM_QUERYDRAGICON()
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_0, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_1, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_2, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_3, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_4, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_5, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_6, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_7, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_8, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_9, &OnMenuItem)
	ON_UPDATE_COMMAND_UI(IDR_MENU_COMMAND_EXIT, &OnMenuItem)
	
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDeviceSwitcherDlg message handlers

BOOL CDeviceSwitcherDlg::OnInitDialog()
{
	InitLinesList();
	TrayMessage(NIM_ADD);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CDialog::OnInitDialog();
	m_nTimer = SetTimer(IDT_HIDE_TIMER,200,0);
	return FALSE;  // return TRUE  unless you set the focus to a control
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDeviceSwitcherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CDeviceSwitcherDlg::InitLinesList(void)
{
	if(m_trayMenu.CreatePopupMenu() == 0)
	{
		MessageBox(TEXT("menu create fail"));
		return false;
	}

	return true;
}

bool CDeviceSwitcherDlg::TrayMessage(DWORD dwMessage)
{
	CString sTip(_T("device switcher"));	

	NOTIFYICONDATA tnd;

	tnd.cbSize		= sizeof(NOTIFYICONDATA);
	tnd.hWnd		= m_hWnd;
	tnd.uID			= IDR_MAINFRAME;

	tnd.uFlags		= NIF_MESSAGE|NIF_ICON;

	tnd.uCallbackMessage	= MYWM_NOTIFYICON;

	
	VERIFY( tnd.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE (IDR_MAINFRAME)) );

	tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;

	lstrcpyn(tnd.szTip, (LPCTSTR)sTip, sizeof(tnd.szTip)/sizeof(tnd.szTip[0]) );
	
	Shell_NotifyIcon(dwMessage, &tnd);
	return true;
}

BOOL CDeviceSwitcherDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	TrayMessage( NIM_DELETE );	
	return CDialog::DestroyWindow();
}

void CDeviceSwitcherDlg::ShowTrayMenu(BOOL bShow, int x, int y)
{
	int nNumDevices = ::waveOutGetNumDevs();

	while(m_trayMenu.GetMenuItemCount())
		m_trayMenu.DeleteMenu(0,MF_BYPOSITION);

	CRegKey reg;
	CString strCurrentDevice;

	if(ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER,IDS_REGISTRY_PATH,KEY_READ))
	{
		TCHAR buffer[30];
		ULONG nChars;
		if (ERROR_SUCCESS == reg.QueryStringValue(IDS_REGISTRY_PLAYBACK,buffer,&nChars))
		{
			strCurrentDevice = buffer;
		}
		reg.Close();
	}



	WAVEOUTCAPS outCaps; 
	for(int i=0;i<nNumDevices;i++)
	{
		::ZeroMemory(&outCaps, sizeof(outCaps));
		if(::waveOutGetDevCaps(i, &outCaps, sizeof(outCaps)) == MMSYSERR_NOERROR)
		{
			if(strCurrentDevice.Compare(outCaps.szPname) == 0)
				m_trayMenu.AppendMenu(MF_CHECKED,IDR_MENU_COMMAND_0+i,outCaps.szPname);
			else
				m_trayMenu.AppendMenu(0,IDR_MENU_COMMAND_0+i,outCaps.szPname);
		}
	}
	//m_trayMenu.AppendMenu(0,IDR_MENU_COMMAND_EXIT,IDS_MENU_EXIT);
	m_trayMenu.TrackPopupMenu(0,x,y,this);
}

LRESULT CDeviceSwitcherDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if((message == MYWM_NOTIFYICON) &&(lParam == WM_RBUTTONDOWN))
	{
		POINT pt;
		::GetCursorPos(&pt);
		ShowTrayMenu(TRUE,pt.x, pt.y);
		return TRUE;
	}
	else if(message == 1234)
	{
		TrayMessage(NIM_DELETE);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
void CDeviceSwitcherDlg::OnMenuItem(CCmdUI *pCmdUI)
{

	if(pCmdUI->m_nID == IDR_MENU_COMMAND_EXIT)
	{
		OnCancel();
		return;
	}

	int nDevId = pCmdUI->m_nID - IDR_MENU_COMMAND_0;
	WAVEOUTCAPS outCaps; 
	WAVEINCAPS inCaps; 
	LONG res;

	// init structure before using
	::ZeroMemory(&outCaps, sizeof(outCaps));
	::ZeroMemory(&inCaps, sizeof(inCaps));
	
	if ( (::waveOutGetDevCaps(nDevId, &outCaps, sizeof(outCaps)) == MMSYSERR_NOERROR)
		&& (::waveInGetDevCaps(nDevId, &inCaps, sizeof(inCaps)) == MMSYSERR_NOERROR))
	{
		HKEY hKey = NULL;
		CRegKey reg;

		res = reg.Open(HKEY_CURRENT_USER,IDS_REGISTRY_PATH,KEY_WRITE);
		if(ERROR_SUCCESS == res)
		{
			reg.SetValue(outCaps.szPname,IDS_REGISTRY_PLAYBACK);
			reg.SetValue(inCaps.szPname,IDS_REGISTRY_RECORD);
			//write 
			reg.Close();
		}
		::PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, NULL);
	}
	else
	{
		// todo: error notify.
	}
}

void CDeviceSwitcherDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(m_nTimer != 0)
	{
		KillTimer(m_nTimer);
		ShowWindow(SW_HIDE);
	}
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}
