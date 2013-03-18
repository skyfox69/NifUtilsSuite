/**
 *  file:   OptionsPageChunkExtract.h
 *  class:  COptionsPageChunkExtract
 *
 *  property page for ChunkExtract options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\Configuration.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Tools\ChunkExtract\OptionsPageChunkExtract.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageChunkExtract, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageChunkExtract, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageChunkExtract::COptionsPageChunkExtract(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageChunkExtract::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	_nameHandling = pConfig->_ceNameHandling;
	_genNormals   = pConfig->_ceGenNormals ? 1 : 0;
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageChunkExtract::~COptionsPageChunkExtract()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageChunkExtract::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RD_NAME_MAT,    _nameHandling);
	DDX_Check(pDX, IDC_CK_GEN_NORMALS, _genNormals);
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageChunkExtract::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageChunkExtract::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	pConfig->_ceNameHandling = _nameHandling;
	pConfig->_ceGenNormals   = (_genNormals == 1);
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageChunkExtract::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageChunkExtract::OnSetActive()
{
	//  first run => initialize wizard mode
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	}

	return CPropertyPage::OnSetActive();
}
