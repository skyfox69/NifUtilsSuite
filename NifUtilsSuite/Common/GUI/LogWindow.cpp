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
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
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

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CLogWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	Configuration*	pConfig(Configuration::getInstance());

	if (!CDialog::PreCreateWindow(cs))	return FALSE;

	if (pConfig->_logPosX >= 0)
	{
		cs.x  = pConfig->_logPosX;
		cs.y  = pConfig->_logPosY;
		cs.cx = pConfig->_logWidth;
		cs.cy = pConfig->_logHeight;
	}

	return TRUE;
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CLogWindow::OnInitDialog()
{
	Configuration*	pConfig(Configuration::getInstance());

	CDialog::OnInitDialog();

	if (pConfig->_logPosX >= 0)
	{
		MoveWindow(pConfig->_logPosX, pConfig->_logPosY, pConfig->_logWidth, pConfig->_logHeight);
	}

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
void CLogWindow::OnDestroy()
{
	Configuration*	pConfig(Configuration::getInstance());
	RECT			tRect = {0};

	GetWindowRect(&tRect);
	pConfig->_logPosX   = tRect.left;
	pConfig->_logPosY   = tRect.top;
	pConfig->_logWidth  = (tRect.right - tRect.left);
	pConfig->_logHeight = (tRect.bottom - tRect.top);
	pConfig->write();

	CDialog::OnDestroy();
}

//-----  OnSize()  ------------------------------------------------------------
void CLogWindow::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRichEditCtrl*	pLogView((CRichEditCtrl*) GetDlgItem(IDC_RE_LOG));

	if ((pLogView != NULL) && (pLogView->GetSafeHwnd() != 0))
	{
		pLogView->MoveWindow(0, 0, cx, cy);
	}
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

