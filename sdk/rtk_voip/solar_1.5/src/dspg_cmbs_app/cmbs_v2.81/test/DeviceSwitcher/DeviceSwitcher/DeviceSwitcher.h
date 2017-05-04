// DeviceSwitcher.h : main header file for the DeviceSwitcher DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDeviceSwitcherApp
// See DeviceSwitcher.cpp for the implementation of this class
//

class CDeviceSwitcherApp : public CWinApp
{
public:
	CDeviceSwitcherApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
