/**
 *  file:   AboutDialog.h
 *  class:  CAboutDialog
 *
 *  About Dialog class
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\AboutDialog.h"
#include "Common\GUI\AboutPage1.h"
#include "Common\GUI\AboutPage2.h"
#include "Common\GUI\AboutPage3.h"
#include "version.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CAboutDialog, CPropertySheet)

BEGIN_MESSAGE_MAP(CAboutDialog, CPropertySheet)
END_MESSAGE_MAP()

//-----  CAboutDialog()  ------------------------------------------------------
CAboutDialog::CAboutDialog(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{}

//-----  CAboutDialog()  ------------------------------------------------------
CAboutDialog::CAboutDialog(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{}

//-----  ~CAboutDialog()  -----------------------------------------------------
CAboutDialog::~CAboutDialog()
{}

//-----  DoModal()  -----------------------------------------------------------
INT_PTR CAboutDialog::DoModal()
{
	CAboutPage1		page1;
	CAboutPage2		page2;
	CAboutPage3		page3;

	AddPage(&page1);
	AddPage(&page2);
	AddPage(&page3);

	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	SetTitle(_T("About NIFUtilsSuite (Beta)  [") + CString(MASTER_FILE_VERSION_STR) + _T("]"), 0);

	return CPropertySheet::DoModal();
}
