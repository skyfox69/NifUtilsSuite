/**
 *  file:   OptionsPageGeneral.h
 *  class:  COptionsPageGeneral
 *
 *  property page for general options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\OptionsPageGeneral.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Nif\NifUtlMaterial.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageGeneral, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_BT_PATH_NIFXML,    &COptionsPageGeneral::OnBnClickedBtPathNifxml)
	ON_BN_CLICKED(IDC_BT_PATH_SKYRIM,    &COptionsPageGeneral::OnBnClickedBtPathSkyrim)
	ON_BN_CLICKED(IDC_BT_PATH_TEMPLATES, &COptionsPageGeneral::OnBnClickedBtPathTemplates)
	ON_BN_CLICKED(IDC_BT_PATH_INPUT,     &COptionsPageGeneral::OnBnClickedBtPathInput)
	ON_BN_CLICKED(IDC_BT_PATH_OUTPUT,    &COptionsPageGeneral::OnBnClickedBtPathOutput)
	ON_BN_CLICKED(IDC_BT_PATH_NIFSKOPE,  &COptionsPageGeneral::OnBnClickedBtPathNifSkope)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageGeneral::COptionsPageGeneral(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageGeneral::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	_pathNifXML    = pConfig->_pathNifXML.c_str();			if (_pathNifXML.IsEmpty())		_pathNifXML = _T("nif.xml");
	_pathDefInput  = pConfig->_pathDefaultInput.c_str();
	_pathDefOutput = pConfig->_pathDefaultOutput.c_str();
	_pathNifSkope  = pConfig->_pathNifSkope.c_str();
	_pathTemplates = pConfig->getPathTemplates().c_str();
	_pathSkyrim    = pConfig->getPathTextures().c_str();	if (_pathSkyrim.IsEmpty())		_pathSkyrim = _T("TESV.EXE");
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageGeneral::~COptionsPageGeneral()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_PATH_SKYRIM,    _pathSkyrim);
	DDX_Text(pDX, IDC_ED_PATH_NIFXML,    _pathNifXML);
	DDX_Text(pDX, IDC_ED_PATH_TEMPLATES, _pathTemplates);
	DDX_Text(pDX, IDC_ED_PATH_INPUT,     _pathDefInput);
	DDX_Text(pDX, IDC_ED_PATH_OUTPUT,    _pathDefOutput);
	DDX_Text(pDX, IDC_ED_PATH_NIFSKOPE,  _pathNifSkope);
}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL COptionsPageGeneral::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);
	return CPropertyPage::PreTranslateMessage(pMsg);
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//Create the ToolTip control
	if( !m_ToolTip.Create(this))
	{
		TRACE0("Unable to create the ToolTip!");
	}
	else
	{
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_SKYRIM),    _pathSkyrim);
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_NIFXML),    _pathNifXML);
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_TEMPLATES), _pathTemplates);
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_INPUT),     _pathDefInput);
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_OUTPUT),    _pathDefOutput);
		m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_NIFSKOPE),  _pathNifSkope);

		m_ToolTip.Activate(TRUE);
		m_ToolTip.SetDelayTime(TTDT_INITIAL, 250);
	}

	return TRUE;
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageGeneral::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageGeneral::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	pConfig->_pathNifXML        = (const char*) CStringA(_pathNifXML).GetString();
	pConfig->_pathDefaultInput  = (const char*) CStringA(_pathDefInput).GetString();
	pConfig->_pathDefaultOutput = (const char*) CStringA(_pathDefOutput).GetString();
	pConfig->_pathNifSkope      = (const char*) CStringA(_pathNifSkope).GetString();
	pConfig->setPathTemplates((const char*) CStringA(_pathTemplates).GetString());
	pConfig->setPathTextures ((const char*) CStringA(_pathSkyrim).GetString());
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageGeneral::OnSetActive()
{
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT);
	}

	CImageList*	pImageList(CFDResourceManager::getInstance()->getImageListBrowse());
	
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_NIFXML))   ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_SKYRIM))   ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_TEMPLATES))->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_INPUT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_OUTPUT))   ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_PATH_NIFSKOPE)) ->SetImage(pImageList->ExtractIcon(1));

	return CPropertyPage::OnSetActive();
}

//-----  OnBnClickedBtPathNifxml()  -------------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathNifxml()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);
	_pathNifXML = FDFileHelper::getFile(_pathNifXML, _T("Nif-XML (nif.xml)|nif.xml||"), _T("xml"), false, _T("Please select Nif.xml file"), OFN_FILEMUSTEXIST);
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_NIFXML), _pathNifXML);

	//  re-initialize material list
	delete NifUtlMaterialList::getInstance();
	NifUtlMaterialList::initInstance(CStringA(_pathNifXML).GetString(), pConfig->_matScanTag, pConfig->_matScanName);
}

//-----  OnBnClickedBtPathSkyrim()  -------------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathSkyrim()
{
	UpdateData(TRUE);
	_pathSkyrim = FDFileHelper::getFolder(_pathSkyrim, _T("Data (Data-Dir)|*.*||"), _T("*"), false, _T("Please select SKYRIM executeable"));
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_SKYRIM), _pathSkyrim);
}

//-----  OnBnClickedBtPathTemplates()  ----------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathTemplates()
{
	UpdateData(TRUE);
	_pathTemplates = FDFileHelper::getFolder(_pathTemplates, _T("*.nif (*.nif)|*.nif||"), _T("nif"), false, _T("Please select template directory"));
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_TEMPLATES), _pathTemplates);
}

//-----  OnBnClickedBtPathInput()  --------------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathInput()
{
	UpdateData(TRUE);
	_pathDefInput = FDFileHelper::getFolder(_pathDefInput, _T("*.nif (*.nif)|*.nif||"), _T("nif"), false, _T("Please select default input directory"));
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_INPUT), _pathDefInput);
}

//-----  OnBnClickedBtPathOutput()  -------------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathOutput()
{
	UpdateData(TRUE);
	_pathDefOutput = FDFileHelper::getFolder(_pathDefOutput, _T("*.nif (*.nif)|*.nif||"), _T("nif"), false, _T("Please select default output directory"));
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_OUTPUT), _pathDefOutput);
}

//-----  OnBnClickedBtPathNifSkope()  -----------------------------------------
void COptionsPageGeneral::OnBnClickedBtPathNifSkope()
{
	UpdateData(TRUE);
	_pathNifSkope = FDFileHelper::getFile(_pathNifSkope, _T("NifSkope (NifSkope)|NifSkope.exe||"), _T("NifSkope.exe"), false, _T("Please select NifSkope executeable"), OFN_FILEMUSTEXIST);
	UpdateData(FALSE);
	m_ToolTip.AddTool(GetDlgItem(IDC_ED_PATH_NIFSKOPE), _pathNifSkope);
}

