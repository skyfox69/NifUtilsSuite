/**
 *  file:   NifUtilsSuite.h
 *  class:  CNifUtilsSuiteApp
 *
 *  Main application
 *
 */

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"	// main symbols

//-----  CLASS  ---------------------------------------------------------------
class CNifUtilsSuiteApp : public CWinAppEx
{
	protected:
		DECLARE_MESSAGE_MAP()

	public:
						CNifUtilsSuiteApp();
		virtual			~CNifUtilsSuiteApp();

		virtual	BOOL	InitInstance();
		virtual	int		ExitInstance();
};

//-----  EXTERNALS  -----------------------------------------------------------
extern CNifUtilsSuiteApp theApp;
