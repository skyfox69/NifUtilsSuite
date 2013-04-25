/**
 *  file:   OptionsPageNifConvert.h
 *  class:  COptionsPageNifConvert
 *
 *  property page for NifConvert options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\Configuration.h"
#include "Tools\NifConvert\OptionsPageNifConvert.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageNifConvert, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageNifConvert, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageNifConvert::COptionsPageNifConvert(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageNifConvert::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	_vertexColor = pConfig->_ncVtFlagsRemove;
	_upTangent   = pConfig->_ncUpTangent   ? 1 : 0;
	_reorderProp = pConfig->_ncReorderProp ? 1 : 0;
	_forceDDS    = pConfig->_ncForceDDS    ? 1 : 0;
	_removeColl  = pConfig->_ncRemoveColl  ? 1 : 0;
	_defColor.SetColor((COLORREF) pConfig->_ncDefColor);
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageNifConvert::~COptionsPageNifConvert()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageNifConvert::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio  (pDX, IDC_RD_REM_FLAG,   _vertexColor);
	DDX_Check  (pDX, IDC_CK_UP_TANGENT, _upTangent);
	DDX_Check  (pDX, IDC_CK_REORDER,    _reorderProp);
	DDX_Check  (pDX, IDC_CK_END_DDS,    _forceDDS);
	DDX_Check  (pDX, IDC_CK_CLEAN_COLL, _removeColl);
	DDX_Control(pDX, IDC_BT_VTCOLOR,    _defColor);
	DDX_Control(pDX, IDC_CB_TEMPLATE,   _defTemplate);
	DDX_Control(pDX, IDC_CB_TEXTURE,    _defTexture);
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageNifConvert::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageNifConvert::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());
	CString			tString;

	UpdateData(TRUE);

	pConfig->_ncVtFlagsRemove   = _vertexColor;
	pConfig->_ncUpTangent       = (_upTangent   == 1);
	pConfig->_ncReorderProp     = (_reorderProp == 1);
	pConfig->_ncForceDDS        = (_forceDDS    == 1);
	pConfig->_ncRemoveColl      = (_removeColl  == 1);
	pConfig->_ncDefColor        = _defColor.GetColor();
	_defTemplate.GetLBText(_defTemplate.GetCurSel(), tString);
	pConfig->_ncDefaultTemplate = CStringA(tString).GetString();
	_defTexture.GetLBText(_defTexture.GetCurSel(), tString);
	pConfig->_ncDefaultTexture  = CStringA(tString).GetString();
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageNifConvert::OnSetActive()
{
	Configuration*	pConfig(Configuration::getInstance());

	if (!pConfig->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	}

	_defTemplate.ResetContent();
	for (auto pIter=pConfig->getDirListTemplates().begin(), pEnd=pConfig->getDirListTemplates().end(); pIter != pEnd; ++pIter)
	{
		_defTemplate.AddString(CString(pIter->c_str()));
	}
	if (pConfig->_ncDefaultTemplate.empty())
	{
		_defTemplate.SetCurSel(0);
	}
	else
	{
		_defTemplate.SelectString(-1, CString(pConfig->_ncDefaultTemplate.c_str()));
	}

	_defTexture.ResetContent();
	for (auto pIter=pConfig->getDirListTextures().begin(), pEnd=pConfig->getDirListTextures().end(); pIter != pEnd; ++pIter)
	{
		_defTexture.AddString(CString(pIter->c_str()));
	}
	if (pConfig->_ncDefaultTexture.empty())
	{
		_defTexture.SetCurSel(0);
	}
	else
	{
		_defTexture.SelectString(-1, CString(pConfig->_ncDefaultTexture.c_str()));
	}

	return CPropertyPage::OnSetActive();
}
