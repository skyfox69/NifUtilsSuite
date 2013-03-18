/**
 *  file:   FormChunkExtractView.cpp
 *  class:  CFormChunkExtractView
 *
 *  Form for ChunkExtract view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Tools\ChunkExtract\FormChunkExtractView.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifCollisionUtility.h"
#include <afxbutton.h>

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,   "Open source in NifSkope"},
						                   {IDC_BT_NSCOPE_OUT,  "Open result in NifSkope"},
						                   {IDC_BT_VIEW_IN,     "Display source on ModelView page"},
						                   {IDC_BT_VIEW_OUT,    "Display collision model on ModelView page"},
						                   {IDC_BT_VIEW_OUT2,   "Display collision model on ModelView page"},
						                   {IDC_BT_FILE_IN,     "Choose source NIF-file to get collision data from"},
						                   {IDC_BT_FILE_OUT,    "Choose target NIF-file to store collision data as Nif"},
						                   {IDC_BT_FILE_OUT2,   "Choose target NIF-file to store collision data as Obj"},
						                   {IDC_RE_LOG,         "Some log output"},
						                   {IDC_BT_RESET_FORM,  "Reset form to default settings"},
						                   {IDC_BT_CONVERT,     "Export collision data to target(s)"},
						                   {IDC_ED_FILE_IN,     "Path to source NIF-file to get collision data from"},
						                   {IDC_ED_FILE_OUT,    "Path to target NIF-file to store collision data to"},
						                   {IDC_ED_FILE_OUT2,   "Path to target OBJ-file to store collision data to"},
						                   {IDC_RD_NAME_MAT,    "Use material name (defined in nif.xml) for naming NiTriShape"},
						                   {IDC_RD_NAME_CHUNK,  "Use index of chunk (Chunk_%d) for naming NiTriShape"},
						                   {IDC_CK_GEN_NORMALS, "Generate normals from vertices on export if possible"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormChunkExtractView, CFormView)

BEGIN_MESSAGE_MAP(CFormChunkExtractView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,        &CFormChunkExtractView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT,       &CFormChunkExtractView::OnBnClickedBtViewNif)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT2,      &CFormChunkExtractView::OnBnClickedBtViewObj)
	ON_BN_CLICKED(IDC_BT_FILE_IN,        &CFormChunkExtractView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_OUT,       &CFormChunkExtractView::OnBnClickedBtFileNif)
	ON_BN_CLICKED(IDC_BT_FILE_OUT2,      &CFormChunkExtractView::OnBnClickedBtFileObj)
	ON_BN_CLICKED(IDC_BT_RESET_FORM,     &CFormChunkExtractView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,      &CFormChunkExtractView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_OUT,     &CFormChunkExtractView::OnBnClickedBtNscopeNif)
	ON_BN_CLICKED(IDC_BT_CONVERT,        &CFormChunkExtractView::OnBnClickedBtConvert)
END_MESSAGE_MAP()

//-----  CFormChunkExtractView()  -----------------------------------------------
CFormChunkExtractView::CFormChunkExtractView()
	:	CFormView(CFormChunkExtractView::IDD),
		LogMessageObject()
{}

//-----  ~CFormChunkExtractView()  ----------------------------------------------
CFormChunkExtractView::~CFormChunkExtractView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormChunkExtractView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,   _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_OUT,  _fileNameNif);
	DDX_Text(pDX, IDC_ED_FILE_OUT2, _fileNameObj);
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CFormChunkExtractView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormChunkExtractView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN))  ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT)) ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT2))->SetImage(pImageList->ExtractIcon(1));

	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_COLLISION)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FLAGS)    ->GetSafeHwnd(), _T(""), _T(""));

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
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_OUT2))  ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_OUT)) ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));

	GetDlgItem(IDC_BT_VIEW_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN) ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(FALSE);

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

		_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
	}

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL CFormChunkExtractView::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CFormView::PreTranslateMessage(pMsg);
}

//-----  OnCtlColor()  --------------------------------------------------------
HBRUSH CFormChunkExtractView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH  hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_GBOX_FILES:
		case IDC_GBOX_COLLISION:
		case IDC_GBOX_FLAGS:
		case IDC_GBOX_HINTS:
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
void CFormChunkExtractView::OnBnClickedBtViewIn()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameIn).GetString());
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormChunkExtractView::OnBnClickedBtNscopeIn()
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

//-----  OnBnClickedBtViewNif()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtViewNif()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameNif).GetString());
}

//-----  OnBnClickedBtNscopeNif()  --------------------------------------------
void CFormChunkExtractView::OnBnClickedBtNscopeNif()
{
	string	cmdString(Configuration::getInstance()->_pathNifSkope);

	if (!cmdString.empty())
	{
		STARTUPINFO			startupInfo = {0};
		PROCESS_INFORMATION	processInfo = {0};
		stringstream		sStream;

		startupInfo.cb = sizeof(startupInfo);

		UpdateData(TRUE);

		sStream << "\"" << cmdString << "\" \"" << CStringA(_fileNameNif).GetString() << "\"";
		CreateProcess(CString(cmdString.c_str()).GetString(), (LPWSTR) CString(sStream.str().c_str()).GetString(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
	}
	else
	{
		AfxMessageBox(_T("You didn't specify the NifSkope\r\nexecuteable in options."), MB_OK | MB_ICONEXCLAMATION);
	}
}

//-----  OnBnClickedBtViewObj()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtViewObj()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameObj).GetString());
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileIn()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameIn);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultInput.empty())
	{
		fileName = CString(pConfig->_pathDefaultInput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select source NIF"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultInput.c_str()) + _T("\\") )))
	{
		_fileNameIn = fileName;
		UpdateData(FALSE);
	}
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileNif()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileNif()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameNif);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultOutput.empty())
	{
		fileName = CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), true, _T("Select target NIF"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\") )))
	{
		_fileNameNif = fileName;
		UpdateData(FALSE);
	}
}

//-----  OnBnClickedBtFileObj()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileObj()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameObj);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultOutput.empty())
	{
		fileName = CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Obj Files (*.obj)|*.obj||"), _T("obj"), true, _T("Select target OBJ"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\") )))
	{
		_fileNameObj = fileName;
		UpdateData(FALSE);
	}
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormChunkExtractView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormChunkExtractView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			Configuration*	pConfig(Configuration::getInstance());

			//  name flags
			int		selItem(pConfig->_ceNameHandling + IDC_RD_NAME_MAT);

			((CButton*) GetDlgItem(IDC_RD_NAME_MAT))  ->SetCheck(selItem == IDC_RD_NAME_MAT);
			((CButton*) GetDlgItem(IDC_RD_NAME_CHUNK))->SetCheck(selItem == IDC_RD_NAME_CHUNK);

			//  various flags
			((CButton*) GetDlgItem(IDC_CK_GEN_NORMALS))->SetCheck(pConfig->_ceGenNormals ? BST_CHECKED : BST_UNCHECKED);

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
void CFormChunkExtractView::LogMessage(const CString text, const CHARFORMAT* pFormat)
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
void CFormChunkExtractView::OnBnClickedBtConvert()
{
	Configuration*			pConfig  (Configuration::getInstance());
	NifCollisionUtility		ncUtility(*(NifUtlMaterialList::getInstance()));
	unsigned short			ncReturn (NCU_OK);

	//  store data
	UpdateData(TRUE);

	//  build full texture path and set to utility
	ncUtility.setSkyrimPath(pConfig->getPathTextures());

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  set flags
	ncUtility.setChunkNameHandling((ChunkNameHandling) (GetCheckedRadioButton(IDC_RD_NAME_MAT, IDC_RD_NAME_CHUNK) - IDC_RD_NAME_MAT));
	ncUtility.setGenerateNormals  (((CButton*) GetDlgItem(IDC_CK_GEN_NORMALS))->GetCheck() != FALSE);

	//  extract chunks from NIF
	ncReturn = ncUtility.extractChunks(CStringA(_fileNameIn).GetString(), CStringA(_fileNameNif).GetString(), CStringA(_fileNameObj).GetString());

	//  decode result
	if (ncReturn == NCU_OK)
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "Chunks extracted successfully");
		GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(!_fileNameObj.IsEmpty());
		GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(!_fileNameNif.IsEmpty());
		GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(!_fileNameNif.IsEmpty());
	}
	else
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "NifCollision returned code: %d", ncReturn);
		GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(FALSE);
		GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
		GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
	}
	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
}
