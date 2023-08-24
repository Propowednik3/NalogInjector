// NalogInjector.h : main header file for the NALOGINJECTOR application
//

#if !defined(AFX_NALOGINJECTOR_H__12C781B8_28C3_4DDA_840E_70E640BE0AB5__INCLUDED_)
#define AFX_NALOGINJECTOR_H__12C781B8_28C3_4DDA_840E_70E640BE0AB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNalogInjectorApp:
// See NalogInjector.cpp for the implementation of this class
//

class CNalogInjectorApp : public CWinApp
{
public:
	CNalogInjectorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNalogInjectorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNalogInjectorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NALOGINJECTOR_H__12C781B8_28C3_4DDA_840E_70E640BE0AB5__INCLUDED_)
