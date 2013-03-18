/**
 *  file:   OptionsPageMaterial.h
 *  class:  COptionsPageMaterial
 *
 *  property page for material options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\OptionsPageMaterial.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Nif\NifUtlMaterial.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageMaterial, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageMaterial, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageMaterial::COptionsPageMaterial(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageMaterial::IDD)
{}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageMaterial::~COptionsPageMaterial()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageMaterial::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VLB_IGN_MATERIAL, _vsLBIgnore);
	DDX_Control(pDX, IDC_VLB_IGN_PREFIX,   _vsLBPrefix);
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageMaterial::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageMaterial::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	pConfig->_matScanIgnore.clear();
	for (int i(0), count(_vsLBIgnore.GetCount()); i < count; ++i)
	{
		pConfig->_matScanIgnore.push_back(CStringA(_vsLBIgnore.GetItemText(i)).GetString());
	}

	pConfig->_matScanPrefix.clear();
	for (int i(0), count(_vsLBPrefix.GetCount()); i < count; ++i)
	{
		pConfig->_matScanPrefix.push_back(CStringA(_vsLBPrefix.GetItemText(i)).GetString());
	}
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageMaterial::OnInitDialog()
{
	Configuration*	pConfig(Configuration::getInstance());

	CPropertyPage::OnInitDialog();

	for (auto pIter=pConfig->_matScanIgnore.begin(), pEnd=pConfig->_matScanIgnore.end(); pIter != pEnd; ++pIter)
	{
		_vsLBIgnore.AddItem(CString(pIter->c_str()));
	}

	for (auto pIter=pConfig->_matScanPrefix.begin(), pEnd=pConfig->_matScanPrefix.end(); pIter != pEnd; ++pIter)
	{
		_vsLBPrefix.AddItem(CString(pIter->c_str()));
	}

	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageMaterial::OnSetActive()
{
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT);
	}

	return CPropertyPage::OnSetActive();
}

