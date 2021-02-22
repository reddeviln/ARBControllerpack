// UAEControllerpack.h : main header file for the UAEControllerpack DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "UAEControllerpack2.h"


// CUAEControllerpackApp
// See UAEControllerpack.cpp for the implementation of this class
//

class CUAEControllerpackApp : public CWinApp
{
public:
	CUAEControllerpackApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
