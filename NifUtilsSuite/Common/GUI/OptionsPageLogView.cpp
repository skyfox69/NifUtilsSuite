/**
 *  file:   OptionsPageLogView.cpp
 *  class:  COptionsPageLogView
 *
 *  property page for log view options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\OptionsPageLogView.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageLogView, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageLogView, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageLogView::COptionsPageLogView(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageLogView::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	for (short i(0); i < 9; ++i)
	{
		_isEnabled[i] = pConfig->_lvwLogActive[i] ? 1 : 0;
		_defColor [i].SetColor((COLORREF) pConfig->_lvwColors[i]);
	}
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageLogView::~COptionsPageLogView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageLogView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	for (short i(0); i < 9; ++i)
	{
		DDX_Check  (pDX, IDC_CB_COL_BACKGROUND+i, _isEnabled[i]);
		DDX_Control(pDX, IDC_BT_COL_BACKGROUND+i, _defColor[i]);
	}
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageLogView::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageLogView::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	for (short i(0); i < 9; ++i)
	{
		pConfig->_lvwLogActive[i] = (_isEnabled[i] == 1);
		pConfig->_lvwColors   [i] = _defColor[i].GetColor();
	}
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageLogView::OnInitDialog()
{
	Configuration*	pConfig(Configuration::getInstance());

	CPropertyPage::OnInitDialog();


	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageLogView::OnSetActive()
{
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT);
	}

	return CPropertyPage::OnSetActive();
}

