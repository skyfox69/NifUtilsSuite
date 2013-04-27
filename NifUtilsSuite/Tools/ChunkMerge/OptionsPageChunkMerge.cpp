/**
 *  file:   OptionsPageChunkMerge.h
 *  class:  COptionsPageChunkMerge
 *
 *  property page for ChunkMerge options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\Configuration.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Tools\ChunkMerge\OptionsPageChunkMerge.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageChunkMerge, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageChunkMerge, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_CB_MAT_SINGLE, &COptionsPageChunkMerge::OnCbnSelchangeCbMatSingle)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageChunkMerge::COptionsPageChunkMerge(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageChunkMerge::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	_colHandling    = pConfig->_cmCollHandling;
	_matHandling    = pConfig->_cmMatHandling;
	_matSingle      = pConfig->_cmMatSingleType;
	_mergeCollision = pConfig->_cmMergeColl ? 0 : 1;
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageChunkMerge::~COptionsPageChunkMerge()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageChunkMerge::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RD_MAT_SINGLE, _matHandling);
	DDX_Radio(pDX, IDC_RD_COLL_CDATA, _colHandling);
	DDX_Check(pDX, IDC_CK_MERGE_COLL, _mergeCollision); 
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageChunkMerge::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageChunkMerge::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	pConfig->_cmCollHandling  = _colHandling;
	pConfig->_cmMatHandling   = _matHandling;
	pConfig->_cmMatSingleType = _matSingle;
	pConfig->_cmMergeColl     = (_mergeCollision == 0);
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageChunkMerge::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//  set combox read-only
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE))->GetDlgItem(1001))->SetReadOnly();

	return TRUE;
}

//-----  OnCbnSelchangeCbMatSingle()  -----------------------------------------
void COptionsPageChunkMerge::OnCbnSelchangeCbMatSingle()
{
	CComboBox*	pCBox ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE));

	_matSingle = (unsigned int) pCBox->GetItemDataPtr(pCBox->GetCurSel());
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageChunkMerge::OnSetActive()
{
	CComboBox*						pCBox ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE));
	map<string, NifUtlMaterial>		matMap(NifUtlMaterialList::getInstance()->getMaterialMap());
	short							tIdx  (0);
	short							selIdx(0);

	//  add materials to combo box
	for (auto pIter(matMap.begin()), pEnd(matMap.end()); pIter != pEnd; ++pIter, ++tIdx)
	{
		pCBox->InsertString  (tIdx, CString(pIter->second._name.c_str()));
		pCBox->SetItemDataPtr(tIdx, (void*) (pIter->second._code));

		if (pIter->second._code == _matSingle)
		{
			selIdx = tIdx;
		}
	}
	pCBox->SetCurSel(selIdx);

	//  first run => initialize wizard mode
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	}

	//  disable 'defining materials' until working
	GetDlgItem(IDC_RD_MAT_DEFINE)->EnableWindow(FALSE);

	return CPropertyPage::OnSetActive();
}
