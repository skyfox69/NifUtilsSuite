/**
 *  file:   FormChunkMergeView.cpp
 *  class:  CFormChunkMergeView
 *
 *  Form for ChunkMerge view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Tools\ChunkMerge\FormChunkMergeView.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifCollisionUtility.h"
#include "Common\Nif\MaterialTypeHandling.h"
#include <afxbutton.h>

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,      "Open target in NifSkope"},
						                   {IDC_BT_NSCOPE_COLL,    "Open collision model in NifSkope"},
						                   {IDC_BT_VIEW_IN,        "Display target on ModelView page"},
						                   {IDC_BT_VIEW_COLL,      "Display collision model on ModelView page"},
						                   {IDC_RD_COLL_CDATA,     "Use RootCollisionNode as source for collision shapes"},
						                   {IDC_RD_COLL_FBACK,     "Use RootCollisionNode as source for collision shapes, nif not existing: use NiTriShapes of model"},
						                   {IDC_RD_COLL_MESH,      "Use NiTriShapes of model as source for collision shapes"},
						                   {IDC_RD_MAT_SINGLE,     "Use single material for all collision data"},
						                   {IDC_RD_MAT_NITRISHAPE, "Use multiple materials defined by name of NiTriShapes for collision data (e.g. SKY_HAV_MAT_STONE)"},
						                   {IDC_RD_MAT_DEFINE,     "Use multiple materials defined by additional dialog for collision data"},
						                   {IDC_CB_TEMPLATE,       "Define NIF file used as template for creating collision data"},
						                   {IDC_CB_MAT_SINGLE,     "Define material used for all collision shapes"},
										   {IDC_RD_COLL_LOCAL,     "Replace local collision meshes instead of creating one collision mesh for whole model"},
										   {IDC_RD_COLL_GLOBAL,    "Create one collision mesh for whole model"},
						                   {IDC_ED_FILE_IN,        "Path to target NIF-file to add collision data to"},
						                   {IDC_ED_FILE_COLL,      "Path to NIF-file defining collision data"},
						                   {IDC_BT_FILE_IN,        "Choose target NIF-file to add collision data to"},
						                   {IDC_BT_FILE_COLL,      "Choose NIF-file defining collision data"},
						                   {IDC_BT_TEMPLATE,       "Choose path to template files and scan recursively"},
						                   {IDC_RE_LOG,            "Some log output"},
						                   {IDC_BT_RESET_FORM,     "Reset form to default settings"},
						                   {IDC_BT_CONVERT,        "Add collision data to target"},
										   {IDC_CK_REORDER_TRIS,   "Reorder triangles of bhk(Packed)NiTriStrips facing outward of model"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormChunkMergeView, CFormView)

BEGIN_MESSAGE_MAP(CFormChunkMergeView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,        &CFormChunkMergeView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_COLL,      &CFormChunkMergeView::OnBnClickedBtViewColl)
	ON_BN_CLICKED(IDC_BT_TEMPLATE,       &CFormChunkMergeView::OnBnClickedOpenSettings)
	ON_BN_CLICKED(IDC_RD_MAT_SINGLE,     &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_RD_MAT_DEFINE,     &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_RD_MAT_NITRISHAPE, &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_BT_FILE_IN,        &CFormChunkMergeView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_COLL,      &CFormChunkMergeView::OnBnClickedBtFileColl)
	ON_BN_CLICKED(IDC_BT_RESET_FORM,     &CFormChunkMergeView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,      &CFormChunkMergeView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_COLL,    &CFormChunkMergeView::OnBnClickedBtNscopeColl)
	ON_BN_CLICKED(IDC_BT_CONVERT,        &CFormChunkMergeView::OnBnClickedBtConvert)
	ON_BN_CLICKED(IDC_RD_COLL_GLOBAL,    &CFormChunkMergeView::OnBnClickedRdCollGlobal)
	ON_BN_CLICKED(IDC_RD_COLL_LOCAL,     &CFormChunkMergeView::OnBnClickedRdCollLocal)
END_MESSAGE_MAP()

//-----  CFormChunkMergeView()  -----------------------------------------------
CFormChunkMergeView::CFormChunkMergeView()
	:	CFormView(CFormChunkMergeView::IDD),
		LogMessageObject()
{}

//-----  ~CFormChunkMergeView()  ----------------------------------------------
CFormChunkMergeView::~CFormChunkMergeView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormChunkMergeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,   _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_COLL, _fileNameColl);
	DDX_Text(pDX, IDC_CB_TEMPLATE,  _template);
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CFormChunkMergeView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormChunkMergeView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN))  ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_COLL))->SetImage(pImageList->ExtractIcon(1));
/*
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE))  ->GetDlgItem(1001))->SetReadOnly();
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE))->GetDlgItem(1001))->SetReadOnly();
*/
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_COLLISION)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_MATERIAL) ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HANDLING) ->GetSafeHwnd(), _T(""), _T(""));

	pImageList = CFDResourceManager::getInstance()->getImageListNumbers();
	for (short i(1); i < 6; ++i)
	{
		((CStatic*) GetDlgItem(IDC_PC_NUM_0+i)) ->SetIcon(pImageList->ExtractIcon(i));
		((CStatic*) GetDlgItem(IDC_PC_HINT_0+i))->SetIcon(pImageList->ExtractIcon(i));
	}

	pImageList = CFDResourceManager::getInstance()->getImageListOther();
	((CMFCButton*) GetDlgItem(IDC_BT_CONVERT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_MAT_DEFINE)) ->SetImage(pImageList->ExtractIcon(7),  true, NULL, pImageListDis->ExtractIcon(7));
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_FORM)) ->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_IN))    ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_COLL))  ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_TEMPLATE))   ->SetImage(pImageList->ExtractIcon(3));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_COLL))->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));

	GetDlgItem(IDC_BT_VIEW_IN)     ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_COLL)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_COLL) ->EnableWindow(FALSE);
	GetDlgItem(IDC_RD_COLL_LOCAL)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_RD_COLL_GLOBAL) ->EnableWindow(FALSE);
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(FALSE);

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

	//  prepare tool tips
	if (_toolTipCtrl.Create(this, TTS_USEVISUALSTYLE | TTS_BALLOON))
	{
		for (short i(0); glToolTiplist[i]._uid != -1; ++i)
		{
			_toolTipCtrl.AddTool(GetDlgItem(glToolTiplist[i]._uid), CString(glToolTiplist[i]._text.c_str()));
		}

		_toolTipCtrl.SetMaxTipWidth(260);
		_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
	}

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);

	//  disable 'defining materials' until working
	GetDlgItem(IDC_RD_MAT_DEFINE)->EnableWindow(FALSE);
}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL CFormChunkMergeView::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CFormView::PreTranslateMessage(pMsg);
}

//-----  OnCtlColor()  --------------------------------------------------------
HBRUSH CFormChunkMergeView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH  hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_GBOX_FILES:
		case IDC_GBOX_COLLISION:
		case IDC_GBOX_MATERIAL:
		case IDC_GBOX_HINTS:
		case IDC_GBOX_HANDLING:
		{
			pDC->SetTextColor(RGB(0,0,255));
			pDC->SetBkMode(TRANSPARENT);
			hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			break;
		}
	}

	return hbr;
}

//-----  OnBnClickedBtViewIn()  -----------------------------------------------
void CFormChunkMergeView::OnBnClickedBtViewIn()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameIn).GetString());
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtNscopeIn()
{
	string	cmdString(Configuration::getInstance()->_pathNifSkope);

	if (!cmdString.empty())
	{
		STARTUPINFO			startupInfo = {0};
		PROCESS_INFORMATION	processInfo = {0};
		stringstream		sStream;

		startupInfo.cb = sizeof(startupInfo);

		UpdateData(TRUE);

		sStream << "\"" << cmdString << "\" \"" << CStringA(_fileNameIn).GetString() << "\"";
		CreateProcess(CString(cmdString.c_str()).GetString(), (LPWSTR) CString(sStream.str().c_str()).GetString(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
	}
	else
	{
		AfxMessageBox(_T("You didn't specify the NifSkope\r\nexecuteable in options."), MB_OK | MB_ICONEXCLAMATION);
	}
}

//-----  OnBnClickedBtViewColl()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtViewColl()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameColl).GetString());
}

//-----  OnBnClickedBtNscopeColl()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedBtNscopeColl()
{
	string	cmdString(Configuration::getInstance()->_pathNifSkope);

	if (!cmdString.empty())
	{
		STARTUPINFO			startupInfo = {0};
		PROCESS_INFORMATION	processInfo = {0};
		stringstream		sStream;

		startupInfo.cb = sizeof(startupInfo);

		UpdateData(TRUE);

		sStream << "\"" << cmdString << "\" \"" << CStringA(_fileNameColl).GetString() << "\"";
		CreateProcess(CString(cmdString.c_str()).GetString(), (LPWSTR) CString(sStream.str().c_str()).GetString(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
	}
	else
	{
		AfxMessageBox(_T("You didn't specify the NifSkope\r\nexecuteable in options."), MB_OK | MB_ICONEXCLAMATION);
	}
}

//-----  OnBnClickedOpenSettings()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedOpenSettings()
{
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_BT_OPTIONS_EDIT);
}

//-----  OnBnClickedRadioCollMat()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedRadioCollMat()
{
	int		selItem(GetCheckedRadioButton(IDC_RD_MAT_SINGLE, IDC_RD_MAT_DEFINE));

	GetDlgItem(IDC_CB_MAT_SINGLE)->EnableWindow(selItem == IDC_RD_MAT_SINGLE);
	GetDlgItem(IDC_BT_MAT_DEFINE)->EnableWindow(selItem == IDC_RD_MAT_DEFINE);
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormChunkMergeView::OnBnClickedBtFileIn()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameIn);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultOutput.empty())
	{
		fileName = CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select input NIF"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\") )))
	{
		_fileNameIn = fileName;
		UpdateData(FALSE);
		GetDlgItem(IDC_RD_COLL_GLOBAL)->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_RD_COLL_LOCAL) ->EnableWindow(_fileNameIn == _fileNameColl);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileColl()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtFileColl()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameColl);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultInput.empty())
	{
		fileName = CString(pConfig->_pathDefaultInput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select input NIF"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultInput.c_str()) + _T("\\") )))
	{
		_fileNameColl = fileName;
		UpdateData(FALSE);
		GetDlgItem(IDC_RD_COLL_GLOBAL)->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_RD_COLL_LOCAL) ->EnableWindow(_fileNameIn == _fileNameColl);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_COLL)  ->EnableWindow(!_fileNameColl.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_COLL)->EnableWindow(!_fileNameColl.IsEmpty());
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormChunkMergeView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormChunkMergeView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			//  set settings from configuration
			Configuration*	pConfig (Configuration::getInstance());
			CComboBox*		pCBox   ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE));
			set<string>		tDirList(pConfig->getDirListTemplates());
			CString			tmpPath (pConfig->_cmDefaultTemplate.c_str());

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

			//- materials
			pCBox = (CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE);
			map<string, NifUtlMaterial>		matMap(NifUtlMaterialList::getInstance()->getMaterialMap());
			short							tIdx  (0);
			short							selIdx(0);

			//  add materials to combo box
			for (auto pIter(matMap.begin()), pEnd(matMap.end()); pIter != pEnd; ++pIter, ++tIdx)
			{
				pCBox->InsertString  (tIdx, CString(pIter->second._name.c_str()));
				pCBox->SetItemDataPtr(tIdx, (void*) (pIter->second._code));

				if (pIter->second._code == pConfig->_cmMatSingleType)
				{
					selIdx = tIdx;
				}
			}
			pCBox->SetCurSel(selIdx);

			//  material flags
			int		selItem(pConfig->_cmMatHandling + IDC_RD_MAT_SINGLE);

			((CButton*) GetDlgItem(IDC_RD_MAT_SINGLE))    ->SetCheck(selItem == IDC_RD_MAT_SINGLE);
			((CButton*) GetDlgItem(IDC_RD_MAT_NITRISHAPE))->SetCheck(selItem == IDC_RD_MAT_NITRISHAPE);
			((CButton*) GetDlgItem(IDC_RD_MAT_DEFINE))    ->SetCheck(selItem == IDC_RD_MAT_DEFINE);
			GetDlgItem(IDC_CB_MAT_SINGLE)->EnableWindow(selItem == IDC_RD_MAT_SINGLE);
			GetDlgItem(IDC_BT_MAT_DEFINE)->EnableWindow(selItem == IDC_RD_MAT_DEFINE);

			//  collision flags
			selItem = pConfig->_cmCollHandling + IDC_RD_COLL_CDATA;
			((CButton*) GetDlgItem(IDC_RD_COLL_CDATA))->SetCheck(selItem == IDC_RD_COLL_CDATA);
			((CButton*) GetDlgItem(IDC_RD_COLL_FBACK))->SetCheck(selItem == IDC_RD_COLL_FBACK);
			((CButton*) GetDlgItem(IDC_RD_COLL_MESH)) ->SetCheck(selItem == IDC_RD_COLL_MESH);

			//  other flags
			((CButton*) GetDlgItem(IDC_RD_COLL_LOCAL))  ->SetCheck(pConfig->_cmMergeColl   ? BST_UNCHECKED : BST_CHECKED);
			((CButton*) GetDlgItem(IDC_RD_COLL_GLOBAL)) ->SetCheck(pConfig->_cmMergeColl   ? BST_CHECKED   : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_REORDER_TRIS))->SetCheck(pConfig->_cmReorderTris ? BST_CHECKED   : BST_UNCHECKED);

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

//-----  LogMessage()  --------------------------------------------------------
void CFormChunkMergeView::LogMessage(const CString text, const CHARFORMAT* pFormat)
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

//-----  OnBnClickedBtConvert()  ----------------------------------------------
void CFormChunkMergeView::OnBnClickedBtConvert()
{
	Configuration*			pConfig  (Configuration::getInstance());
	CComboBox*				pCBox    ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE));
	NifCollisionUtility		ncUtility(*(NifUtlMaterialList::getInstance()));
	map<int, unsigned int>	materialMap;
	unsigned short			ncReturn (NCU_OK);

	//  store data
	UpdateData(TRUE);

	//  build full texture path and set to utility
	ncUtility.setSkyrimPath(pConfig->getPathTextures());

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  get material handling
	MaterialTypeHandling	matHandling((MaterialTypeHandling) (GetCheckedRadioButton(IDC_RD_MAT_SINGLE, IDC_RD_MAT_DEFINE) - IDC_RD_MAT_SINGLE));

	//  set default material
	materialMap[-1] = ((unsigned int) pCBox->GetItemDataPtr(pCBox->GetCurSel()));

	//  add additional materials in case of special handling
	if (matHandling == NCU_MT_MATMAP)
	{
		//  do something special
	}

	//  set flags
	ncUtility.setCollisionNodeHandling((CollisionNodeHandling) (GetCheckedRadioButton(IDC_RD_COLL_CDATA, IDC_RD_COLL_MESH) - IDC_RD_COLL_CDATA));
	ncUtility.setMaterialTypeHandling (matHandling, materialMap);
	ncUtility.setDefaultMaterial      (materialMap[-1]);
	ncUtility.setMergeCollision       (((CButton*) GetDlgItem(IDC_RD_COLL_GLOBAL)) ->GetCheck() == TRUE);
	ncUtility.setReorderTriangles     (((CButton*) GetDlgItem(IDC_CK_REORDER_TRIS))->GetCheck() == TRUE);

	//  add collision data to nif
	ncReturn = ncUtility.addCollision(CStringA(_fileNameColl).GetString(), CStringA(_fileNameIn).GetString(), pConfig->getPathTemplates() + "\\" + CStringA(_template).GetString());

	//  decode result
	if (ncReturn == NCU_OK)
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "Collision data added successfully");
	}
	else
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "NifCollision returned code: %d", ncReturn);
	}
	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
}

//-----  OnBnClickedRdCollGlobal()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedRdCollGlobal()
{
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(FALSE);
}

//-----  OnBnClickedRdCollLocal()  --------------------------------------------
void CFormChunkMergeView::OnBnClickedRdCollLocal()
{
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(TRUE);
}
