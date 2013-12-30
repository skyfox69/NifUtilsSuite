/**
 *  file:   FormNifConvertView.cpp
 *  class:  CFormNifConvertView
 *
 *  Form for NifConvert view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuiteDoc.h"
#include "NifUtilsSuite.h"
#include "Tools\NifConvert\FormNifConvertView.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifConvertUtility.h"
#include <afxbutton.h>

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,  "Open source in NifSkope"},
						                   {IDC_BT_NSCOPE_OUT, "Open result in NifSkope"},
						                   {IDC_BT_VIEW_IN,    "Display source on ModelView page"},
						                   {IDC_BT_VIEW_OUT,   "Display result on ModelView page"},
						                   {IDC_CK_UP_TANGENT, "Calculate normals and binormals in NiTriShapeData"},
						                   {IDC_CK_REORDER,    "Make sure properties of NiTriShape are in valid order"},
						                   {IDC_CK_END_DDS,    "Force texture names with ending .DDS"},
										   {IDC_CK_CLEAN_COLL, "Remove all known collision nodes from target NIF"},
						                   {IDC_RD_GEN_COL,    "Add default vertext color in case of set flag SLSF2_Vertex_Colors"},
						                   {IDC_RD_REM_FLAG,   "Remove flag SLSF2_Vertex_Colors in case of missing vertex colors"},
						                   {IDC_CB_TEXTURE,    "Define path to texture files written to BSShaderTextureSet"},
						                   {IDC_CB_TEMPLATE,   "Define NIF file used as template for converting"},
						                   {IDC_ED_FILE_IN,    "Path to source NIF-file to be converted"},
						                   {IDC_ED_FILE_OUT,   "Path to destination NIF-file"},
						                   {IDC_BT_FILE_IN,    "Choose source NIF-file to be converted"},
						                   {IDC_BT_FILE_OUT,   "Choose destination NIF-file"},
						                   {IDC_BT_TEMPLATE,   "Choose path to template files and scan recursively"},
						                   {IDC_BT_TEXTURE,    "Choose path to Skyrim and scan Data/Textures recursively"},
						                   {IDC_BT_RESET_FORM, "Reset form to default settings"},
						                   {IDC_BT_CONVERT,    "Convert NIF"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormNifConvertView, CFormView)

BEGIN_MESSAGE_MAP(CFormNifConvertView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,    &CFormNifConvertView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT,   &CFormNifConvertView::OnBnClickedBtViewOut)
	ON_BN_CLICKED(IDC_BT_TEMPLATE,   &CFormNifConvertView::OnBnClickedOpenSettings)
	ON_BN_CLICKED(IDC_BT_TEXTURE,    &CFormNifConvertView::OnBnClickedOpenSettings)
	ON_BN_CLICKED(IDC_RD_GEN_COL,    &CFormNifConvertView::OnBnClickedRdVtColor)
	ON_BN_CLICKED(IDC_RD_REM_FLAG,   &CFormNifConvertView::OnBnClickedRdVtColor)
	ON_BN_CLICKED(IDC_BT_FILE_IN,    &CFormNifConvertView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_OUT,   &CFormNifConvertView::OnBnClickedBtFileOut)
	ON_BN_CLICKED(IDC_BT_RESET_FORM, &CFormNifConvertView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_CONVERT,    &CFormNifConvertView::OnBnClickedBtConvert)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,  &CFormNifConvertView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_OUT, &CFormNifConvertView::OnBnClickedBtNscopeOut)
END_MESSAGE_MAP()

//-----  CFormChunkMergeView()  -----------------------------------------------
CFormNifConvertView::CFormNifConvertView()
	:	ToolsFormViewBase(CFormNifConvertView::IDD, LogMessageObject::NIFCONVERT)
{}

//-----  ~CFormChunkMergeView()  ----------------------------------------------
CFormNifConvertView::~CFormNifConvertView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormNifConvertView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,  _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_OUT, _fileNameOut);
	DDX_Text(pDX, IDC_CB_TEMPLATE, _template);
	DDX_Text(pDX, IDC_CB_TEXTURE,  _texturePath);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormNifConvertView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
//	ResizeParentToFit(FALSE);

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN)) ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT))->SetImage(pImageList->ExtractIcon(1));
/*
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE))->GetDlgItem(1001))->SetReadOnly();
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_TEXTURE)) ->GetDlgItem(1001))->SetReadOnly();
*/
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)  ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FLAGS)  ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_VCOLORS)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)  ->GetSafeHwnd(), _T(""), _T(""));

	pImageList = CFDResourceManager::getInstance()->getImageListNumbers();
	for (short i(1); i < 5; ++i)
	{
		((CStatic*) GetDlgItem(IDC_PC_NUM_0+i)) ->SetIcon(pImageList->ExtractIcon(i));
		((CStatic*) GetDlgItem(IDC_PC_HINT_0+i))->SetIcon(pImageList->ExtractIcon(i));
	}

	pImageList = CFDResourceManager::getInstance()->getImageListOther();
	((CMFCButton*) GetDlgItem(IDC_BT_CONVERT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_FORM)) ->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_IN))    ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_OUT))   ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_OUT)) ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_TEMPLATE))   ->SetImage(pImageList->ExtractIcon(3));
	((CMFCButton*) GetDlgItem(IDC_BT_TEXTURE))    ->SetImage(pImageList->ExtractIcon(3));

	GetDlgItem(IDC_BT_VIEW_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN) ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);

	//  prepare tool tips
	PrepareToolTips(glToolTiplist);

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  OnBnClickedBtViewIn()  -----------------------------------------------
void CFormNifConvertView::OnBnClickedBtViewIn()
{
	UpdateData(TRUE);
	ShowModel(_fileNameIn);
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormNifConvertView::OnBnClickedBtNscopeIn()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameIn);
}

//-----  OnBnClickedBtViewOut()  ----------------------------------------------
void CFormNifConvertView::OnBnClickedBtViewOut()
{
	UpdateData(TRUE);
	ShowModel(_fileNameOut);
}

//-----  OnBnClickedBtNscopeOut()  --------------------------------------------
void CFormNifConvertView::OnBnClickedBtNscopeOut()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameOut);
}

//-----  OnBnClickedOpenSettings()  -------------------------------------------
void CFormNifConvertView::OnBnClickedOpenSettings()
{
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_BT_OPTIONS_EDIT);
}

//-----  OnBnClickedRdVtColor()  ----------------------------------------------
void CFormNifConvertView::OnBnClickedRdVtColor()
{
	GetDlgItem(IDC_MFCCOLORBUTTON1)->EnableWindow((GetCheckedRadioButton(IDC_RD_REM_FLAG, IDC_RD_GEN_COL) == IDC_RD_GEN_COL));
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormNifConvertView::OnBnClickedBtFileIn()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameIn, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select input NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameIn = fileName;
		UpdateData(FALSE);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileOut()  ----------------------------------------------
void CFormNifConvertView::OnBnClickedBtFileOut()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameOut, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), true, _T("Select output NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameOut = fileName;
		UpdateData(FALSE);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
#endif
	GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormNifConvertView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormNifConvertView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			//  set settings from configuration
			Configuration*	pConfig (Configuration::getInstance());
			CComboBox*		pCBox   ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE));
			set<string>		tDirList(pConfig->getDirListTemplates());
			CString			tmpPath (pConfig->_ncDefaultTemplate.c_str());

			//- templates
			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "Adding templates");
			pCBox->ResetContent();
			for (auto pIter(tDirList.begin()), pEnd(tDirList.end()); pIter != pEnd; ++pIter)
			{
				if (pIter->rfind(".nif") == string::npos)		continue;
				pCBox->AddString(CString(pIter->c_str()));
				LogMessageObject::LogMessage(NCU_MSG_TYPE_SUB_INFO, "added: %s", pIter->c_str());
			}
			pCBox->SelectString(-1, CString(tmpPath));
			pCBox->GetWindowText(tmpPath);
			if (tmpPath.IsEmpty())		pCBox->SetCurSel(0);
			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "templates found: %d", tDirList.size());

			//- textures
			pCBox    = (CComboBox*) GetDlgItem(IDC_CB_TEXTURE);
			tDirList = pConfig->getDirListTextures();
			tmpPath  = pConfig->_ncDefaultTexture.c_str();

			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "Adding texture paths");
			pCBox->ResetContent();
			for (auto pIter(tDirList.begin()), pEnd(tDirList.end()); pIter != pEnd; ++pIter)
			{
				pCBox->AddString(CString(pIter->c_str()));
				LogMessageObject::LogMessage(NCU_MSG_TYPE_SUB_INFO, "added: %s", pIter->c_str());
			}
			pCBox->SelectString(-1, CString(tmpPath));
			pCBox->GetWindowText(tmpPath);
			if (tmpPath.IsEmpty())		pCBox->SetCurSel(0);
			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "texture paths found: %d", tDirList.size());

			//- vertex handling
			if (pConfig->_ncVtFlagsRemove == 0)
			{
				((CButton*) GetDlgItem(IDC_RD_REM_FLAG))->SetCheck(BST_CHECKED);
				((CButton*) GetDlgItem(IDC_RD_GEN_COL)) ->SetCheck(BST_UNCHECKED);
				GetDlgItem(IDC_MFCCOLORBUTTON1)->EnableWindow(FALSE);
			}
			else
			{
				((CButton*) GetDlgItem(IDC_RD_REM_FLAG))->SetCheck(BST_UNCHECKED);
				((CButton*) GetDlgItem(IDC_RD_GEN_COL)) ->SetCheck(BST_CHECKED);
				GetDlgItem(IDC_MFCCOLORBUTTON1)->EnableWindow(TRUE);
			}

			((CMFCColorButton*) GetDlgItem(IDC_MFCCOLORBUTTON1))->SetColor((COLORREF) pConfig->_ncDefColor);

			//- various flags
			((CButton*) GetDlgItem(IDC_CK_UP_TANGENT))->SetCheck(pConfig->_ncUpTangent   ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_REORDER)   )->SetCheck(pConfig->_ncReorderProp ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_END_DDS)   )->SetCheck(pConfig->_ncForceDDS    ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_CLEAN_COLL))->SetCheck(pConfig->_ncRemoveColl  ? BST_CHECKED : BST_UNCHECKED);

			break;
		}

		case IBCE_SET_TOOLTIPP:
		{
			_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
			break;
		}
	}  //  switch (event)

	return TRUE;
}

//-----  OnBnClickedBtConvert()  ----------------------------------------------
void CFormNifConvertView::OnBnClickedBtConvert()
{
	Configuration*		pConfig (Configuration::getInstance());
	NifConvertUtility	ncUtility;
	unsigned short		ncReturn(NCU_OK);

	//  set wait pointer
	BeginWaitCursor();

	//  store data
	UpdateData(TRUE);

	//  build full texture path and set to utility
	ncUtility.setTexturePath(string("textures") + CStringA(_texturePath).GetString());
	ncUtility.setSkyrimPath (pConfig->getPathTextures());

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  set flags
	ncUtility.setVertexColorHandling((VertexColorHandling) (GetCheckedRadioButton(IDC_RD_REM_FLAG, IDC_RD_GEN_COL) - IDC_RD_REM_FLAG));
	ncUtility.setUpdateTangentSpace (((CButton*) GetDlgItem(IDC_CK_UP_TANGENT))->GetCheck() != FALSE);
	ncUtility.setReorderProperties  (((CButton*) GetDlgItem(IDC_CK_REORDER)   )->GetCheck() != FALSE);
	ncUtility.setForceDDS           (((CButton*) GetDlgItem(IDC_CK_END_DDS)   )->GetCheck() != FALSE);
	ncUtility.setCleanTreeCollision (((CButton*) GetDlgItem(IDC_CK_CLEAN_COLL))->GetCheck() != FALSE);

	//  convert NIF
	ncReturn = ncUtility.convertShape(CStringA(_fileNameIn).GetString(), CStringA(_fileNameOut).GetString(), pConfig->getPathTemplates() + "\\" + CStringA(_template).GetString());

	//  decode result
	if (ncReturn == NCU_OK)
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "NIF converted successfully");
#ifndef NUS_LIGHT
		GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(TRUE);
#endif
		GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(TRUE);

		if (pConfig->_autoSetPath)
		{
			((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_AUTO_SET_PATH, (void*) CStringA(_fileNameOut).GetString());
		}
	}
	else
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "NifConverter returned code: %d", ncReturn);
	}
	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");

	//  end wait pointer
	EndWaitCursor();
}

