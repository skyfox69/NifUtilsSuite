/**
 *  file:   AboutDialog.h
 *  class:  CAboutDialog
 *
 *  About Dialog class
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include <string>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CAboutDialog : public CPropertySheet
{
	protected:
		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CAboutDialog)

	public:
								CAboutDialog(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
								CAboutDialog(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		virtual					~CAboutDialog();

		virtual	INT_PTR			DoModal();
};
