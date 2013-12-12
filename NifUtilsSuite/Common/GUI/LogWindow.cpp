/**
 *  file:   LogWindow.h
 *  class:  CLogWindow
 *
 *  Log Window class
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "resource.h"
#include "Common\GUI\LogWindow.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\Configuration.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CLogWindow, CDialog)

BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//-----  CLogWindow()  --------------------------------------------------------
CLogWindow::CLogWindow(CWnd* pParentWnd)
	:	CDialog (IDD_LOG_WINDOW, pParentWnd),
		_pParent(pParentWnd)
{}

//-----  ~CLogWindow()  -------------------------------------------------------
CLogWindow::~CLogWindow()
{}

//-----  PostNcDestroy()  -----------------------------------------------------
void CLogWindow::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	if (_pParent != NULL)
	{
		((CNifUtilsSuiteFrame*) _pParent)->BroadcastEvent(IBCE_LOG_WINDOW_KILLED);
	}

	delete this;
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CLogWindow::OnInitDialog()
{
	CDialog::OnInitDialog();

	//  initialize log view
	CRichEditCtrl*	pLogView((CRichEditCtrl*) GetDlgItem(IDC_RE_LOG));
	CHARFORMAT		cf = { 0 };

	cf.cbSize    = sizeof(cf);
	cf.dwMask    = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_PROTECTED;
	cf.dwEffects = 0;
	cf.yHeight   = 127;
	lstrcpy(cf.szFaceName, _T("Small Fonts"));

	pLogView->SetDefaultCharFormat(cf);
	pLogView->SetReadOnly         (TRUE);
	if (Configuration::getInstance()->_lvwLogActive[0])
	{
		pLogView->SetBackgroundColor(FALSE, Configuration::getInstance()->_lvwColors[0]);
	}

	return TRUE;
}

//-----  OnClose()  -----------------------------------------------------------
void CLogWindow::OnClose()
{
	if (_pParent != NULL)
	{
		((CNifUtilsSuiteFrame*) _pParent)->BroadcastEvent(IBCE_LOG_WINDOW_KILLED);
	}

	CDialog::OnClose();
}

//-----  LogMessage()  --------------------------------------------------------
void CLogWindow::LogMessage(const CString text, const CHARFORMAT* pFormat)
{
	CRichEditCtrl*	pLogView    ((CRichEditCtrl*) GetDlgItem(IDC_RE_LOG));
	int				lineCountOld(pLogView->GetLineCount());

	//  select  nothing, set format and append new text
	pLogView->SetSel(-1, -1);
	pLogView->SetSelectionCharFormat(*((CHARFORMAT*) pFormat));
	pLogView->ReplaceSel(text);

	//  scroll to end of text
	pLogView->LineScroll(pLogView->GetLineCount() - lineCountOld);
	pLogView->SetSel(-1, -1);
}

