// DeviceSwitcherDlg.h : header file
//

#pragma once

#include <mmsystem.h>
#include "Resource.h"
#define MYWM_NOTIFYICON		(WM_USER+2)

#define IDR_MENU_COMMAND_0				130
#define IDR_MENU_COMMAND_1				131
#define IDR_MENU_COMMAND_2				132
#define IDR_MENU_COMMAND_3				133
#define IDR_MENU_COMMAND_4				134
#define IDR_MENU_COMMAND_5				135
#define IDR_MENU_COMMAND_6				136
#define IDR_MENU_COMMAND_7				137
#define IDR_MENU_COMMAND_8				138
#define IDR_MENU_COMMAND_9				139
#define IDR_MENU_COMMAND_EXIT			140
#define IDT_HIDE_TIMER					141


#define IDS_REGISTRY_PATH		(TEXT("Software\\Microsoft\\Multimedia\\Sound Mapper"))
#define IDS_REGISTRY_PLAYBACK	(TEXT("Playback"))
#define IDS_REGISTRY_RECORD		(TEXT("Record"))
#define IDS_MENU_EXIT			(TEXT("Exit"))

// CDeviceSwitcherDlg dialog
class CDeviceSwitcherDlg : public CDialog
{
// Construction
public:
	CDeviceSwitcherDlg(CWnd* pParent = NULL);	// standard constructor
	~CDeviceSwitcherDlg();
// Dialog Data
	enum { IDD = IDD_DEVICESWITCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON	m_hIcon;
	CMenu	m_trayMenu;
	UINT	m_nTimer;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	void ShowTrayMenu(BOOL bShow, int x, int y);
	DECLARE_MESSAGE_MAP()
private:
	int m_nNumMixers;
	bool InitLinesList(void);
public:
	bool TrayMessage(DWORD dwMessage);
	virtual BOOL DestroyWindow();
	virtual	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuItem(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
