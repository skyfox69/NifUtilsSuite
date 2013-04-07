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
	_genNormals   = pConfig->_ceGenNormals   ? 1 : 0;
	_scaleToModel = pConfig->_ceScaleToModel ? 1 : 0;
	_saveAs20207  = pConfig->_ceSaveAs20207  ? 1 : 0;
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageChunkExtract::~COptionsPageChunkExtract()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageChunkExtract::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RD_NAME_MAT,       _nameHandling);
	DDX_Check(pDX, IDC_CK_GEN_NORMALS,    _genNormals);
	DDX_Check(pDX, IDC_CK_SCALE_TO_MODEL, _scaleToModel);
	DDX_Check(pDX, IDC_CK_SAVEAS_20207,   _saveAs20207);
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
	pConfig->_ceGenNormals   = (_genNormals   == 1);
	pConfig->_ceScaleToModel = (_scaleToModel == 1);
	pConfig->_ceSaveAs20207  = (_saveAs20207  == 1);
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
