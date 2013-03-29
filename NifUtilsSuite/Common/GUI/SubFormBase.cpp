/**
 *  file:   SubFormBase.h
 *  class:  CSubFormBase
 *
 *  Base class for dialogs used as sub forms
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\SubFormBase.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDResourceManager.h"
#include "resource.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CSubFormBase, CDialogEx)

BEGIN_MESSAGE_MAP(CSubFormBase, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//-----  CSubFormBase()  ------------------------------------------------------
CSubFormBase::CSubFormBase(UINT idTemplate, CWnd* pParent /*=NULL*/)
	: CDialogEx(idTemplate, pParent)
{}

//-----  ~CSubFormBase()  -----------------------------------------------------
CSubFormBase::~CSubFormBase()
{}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CSubFormBase::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	((CStatic*) GetDlgItem(IDC_PC_NUM_3)) ->SetIcon(CFDResourceManager::getInstance()->getImageListNumbers()->ExtractIcon(3));

	::SetWindowTheme(GetDlgItem(IDC_GBOX_FLAGS)->GetSafeHwnd(), _T(""), _T(""));

	return TRUE;
}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL CSubFormBase::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CDialogEx::PreTranslateMessage(pMsg);
}

//-----  OnCtlColor()  --------------------------------------------------------
HBRUSH CSubFormBase::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH  hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_GBOX_FLAGS:
		{
			pDC->SetTextColor(RGB(0,0,255));
			pDC->SetBkMode(TRANSPARENT);
			hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			break;
		}
	}

	return hbr;
}

