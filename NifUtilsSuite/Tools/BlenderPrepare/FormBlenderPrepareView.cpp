/**
 *  file:   FormBlenderPrepareView.cpp
 *  class:  CFormBlenderPrepareView
 *
 *  Form for BlenderPrepare view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Tools\BlenderPrepare\FormBlenderPrepareView.h"
#include "Tools\BlenderPrepare\SubFormArmorBlender.h"
#include "Tools\BlenderPrepare\SubFormBlenderArmor.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifCollisionUtility.h"
#include <afxbutton.h>

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,    "Open source in NifSkope"},
						                   {IDC_BT_NSCOPE_OUT,   "Open result in NifSkope"},
						                   {IDC_BT_VIEW_IN,      "Display source on ModelView page"},
						                   {IDC_BT_VIEW_OUT,     "Display result on ModelView page"},
						                   {IDC_ED_FILE_IN,      "Path to source NIF-file to be converted"},
						                   {IDC_ED_FILE_OUT,     "Path to destination NIF-file"},
						                   {IDC_BT_FILE_IN,      "Choose source NIF-file to be converted"},
						                   {IDC_BT_FILE_OUT,     "Choose destination NIF-file"},
						                   {IDC_RE_LOG,          "Some log output"},
						                   {IDC_BT_RESET_FORM,   "Reset form to default settings"},
						                   {IDC_BT_CONVERT,      "Process tools action"},
										   {IDC_RD_TO_BLENDER,   "Prepare NIF for Blender-Import"},
										   {IDC_RD_FROM_BLENDER, "Prepare NIF for use in Skyrim/NifSkope"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormBlenderPrepareView, CFormView)

BEGIN_MESSAGE_MAP(CFormBlenderPrepareView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,        &CFormBlenderPrepareView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT,       &CFormBlenderPrepareView::OnBnClickedBtViewOut)
	ON_BN_CLICKED(IDC_BT_FILE_IN,        &CFormBlenderPrepareView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_OUT,       &CFormBlenderPrepareView::OnBnClickedBtFileOut)
	ON_BN_CLICKED(IDC_BT_RESET_FORM,     &CFormBlenderPrepareView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,      &CFormBlenderPrepareView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_OUT,     &CFormBlenderPrepareView::OnBnClickedBtNscopeOut)
	ON_BN_CLICKED(IDC_BT_CONVERT,        &CFormBlenderPrepareView::OnBnClickedBtConvert)
	ON_BN_CLICKED(IDC_RD_TO_BLENDER,     &CFormBlenderPrepareView::OnBnClickedRdDirection)
	ON_BN_CLICKED(IDC_RD_FROM_BLENDER,   &CFormBlenderPrepareView::OnBnClickedRdDirection)
END_MESSAGE_MAP()

//-----  CFormBlenderPrepareView()  -------------------------------------------
CFormBlenderPrepareView::CFormBlenderPrepareView()
	:	CFormView(CFormBlenderPrepareView::IDD),
		LogMessageObject(),
		_actDirection   (IDC_RD_TO_BLENDER),
		_actTool        (IDC_RD_ARMOR)
{}

//-----  ~CFormBlenderPrepareView()  ------------------------------------------
CFormBlenderPrepareView::~CFormBlenderPrepareView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormBlenderPrepareView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,  _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_OUT, _fileNameOut);
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CFormBlenderPrepareView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormBlenderPrepareView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN)) ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT))->SetImage(pImageList->ExtractIcon(1));

	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_TOOLS)->GetSafeHwnd(), _T(""), _T(""));

	pImageList = CFDResourceManager::getInstance()->getImageListNumbers();
	for (short i(1); i < 5; ++i)
	{
		if (i != 3)
		{
			((CStatic*) GetDlgItem(IDC_PC_NUM_0+i))->SetIcon(pImageList->ExtractIcon(i));
		}
		((CStatic*) GetDlgItem(IDC_PC_HINT_0+i))->SetIcon(pImageList->ExtractIcon(i));
	}

	pImageList = CFDResourceManager::getInstance()->getImageListOther();
	((CMFCButton*) GetDlgItem(IDC_BT_CONVERT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_FORM)) ->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_IN))    ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_OUT))   ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_OUT)) ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));

	GetDlgItem(IDC_BT_VIEW_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN) ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);

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

	//  create initial sub form
	CRect	tRect;
	GetDlgItem(IDC_SUBFORM_BOX)->GetWindowRect(&tRect);

	_subFormList.SetPosition  (tRect);
	_subFormList.CreateSubForm(IDD_SUB_ARMOR_BLENDER, this, new CSubFormArmorBlender());
	_subFormList.CreateSubForm(IDD_SUB_BLENDER_ARMOR, this, new CSubFormBlenderArmor());
	_subFormList.ShowSubForm  (0);

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
BOOL CFormBlenderPrepareView::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CFormView::PreTranslateMessage(pMsg);
}

//-----  OnCtlColor()  --------------------------------------------------------
HBRUSH CFormBlenderPrepareView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH  hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_GBOX_FILES:
		case IDC_GBOX_TOOLS:
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
void CFormBlenderPrepareView::OnBnClickedBtViewIn()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameIn).GetString());
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtNscopeIn()
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

//-----  OnBnClickedBtViewOut()  ----------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtViewOut()
{
	// TODO: Add your control notification handler code here

	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(_fileNameOut).GetString());
}

//-----  OnBnClickedBtNscopeOut()  --------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtNscopeOut()
{
	string	cmdString(Configuration::getInstance()->_pathNifSkope);

	if (!cmdString.empty())
	{
		STARTUPINFO			startupInfo = {0};
		PROCESS_INFORMATION	processInfo = {0};
		stringstream		sStream;

		startupInfo.cb = sizeof(startupInfo);

		UpdateData(TRUE);

		sStream << "\"" << cmdString << "\" \"" << CStringA(_fileNameOut).GetString() << "\"";
		CreateProcess(CString(cmdString.c_str()).GetString(), (LPWSTR) CString(sStream.str().c_str()).GetString(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
	}
	else
	{
		AfxMessageBox(_T("You didn't specify the NifSkope\r\nexecuteable in options."), MB_OK | MB_ICONEXCLAMATION);
	}
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtFileIn()
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
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileOut()  ----------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtFileOut()
{
	UpdateData(TRUE);

	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(_fileNameOut);

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultOutput.empty())
	{
		fileName = CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), true, _T("Select target NIF"));

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultOutput.c_str()) + _T("\\") )))
	{
		_fileNameOut = fileName;
		UpdateData(FALSE);
	}
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormBlenderPrepareView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormBlenderPrepareView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			Configuration*	pConfig(Configuration::getInstance());

			//  tool
			((CButton*) GetDlgItem(IDC_RD_ARMOR))->SetCheck(_actTool == IDC_RD_ARMOR);

			//  direction
			((CButton*) GetDlgItem(IDC_RD_TO_BLENDER))  ->SetCheck(_actDirection == IDC_RD_TO_BLENDER);
			((CButton*) GetDlgItem(IDC_RD_FROM_BLENDER))->SetCheck(_actDirection == IDC_RD_FROM_BLENDER);
			GetDlgItem(IDC_BT_CONVERT)->SetWindowText((_actDirection == IDC_RD_TO_BLENDER) ? _T("Prep. Blender") : _T("Prep. Skyrim"));
/*
			//  name flags
			int		selItem(pConfig->_ceNameHandling + IDC_RD_NAME_MAT);

			((CButton*) GetDlgItem(IDC_RD_NAME_MAT))  ->SetCheck(selItem == IDC_RD_NAME_MAT);
			((CButton*) GetDlgItem(IDC_RD_NAME_CHUNK))->SetCheck(selItem == IDC_RD_NAME_CHUNK);

			//  various flags
			((CButton*) GetDlgItem(IDC_CK_GEN_NORMALS))   ->SetCheck(pConfig->_ceGenNormals   ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_SCALE_TO_MODEL))->SetCheck(pConfig->_ceScaleToModel ? BST_CHECKED : BST_UNCHECKED);
*/
			break;
		}

		case IBCE_SET_TOOLTIPP:
		{
			_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
			break;
		}
	}  //  switch (event)

	//  route event to sub forms
	for (auto pIter=_subFormList.begin(), pEnd=_subFormList.end(); pIter != pEnd; ++pIter)
	{
		if (dynamic_cast<IfcBroadcastObject*>(*pIter) != NULL)
		{
			dynamic_cast<IfcBroadcastObject*>(*pIter)->BroadcastEvent(event, pParameter);
		}
	}

	return TRUE;
}

//-----  LogMessage()  --------------------------------------------------------
void CFormBlenderPrepareView::LogMessage(const CString text, const CHARFORMAT* pFormat)
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
void CFormBlenderPrepareView::OnBnClickedBtConvert()
{
	CSubFormBase*	pActForm(_subFormList.GetActiveDlg());

	//  store data
	UpdateData(TRUE);

	//  call active sub from to perform its action
	if (pActForm != NULL)
	{
		unsigned short	ncReturn (pActForm->PerformAction(CStringA(_fileNameIn).GetString(), CStringA(_fileNameOut).GetString()));

		//  decode result
		if (ncReturn == NCU_OK)
		{
			LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "Action performed successfully");
#ifndef NUS_LIGHT
			GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(!_fileNameOut.IsEmpty());
#endif
			GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(!_fileNameOut.IsEmpty());
		}
		else
		{
			LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "Action returned code: %d", ncReturn);
			GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
			GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
		}
	}  //  if (pActForm != NULL)

	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
}

//-----  OnBnClickedRdDirection()  ----------------------------------------------
void CFormBlenderPrepareView::OnBnClickedRdDirection()
{
	int		selItem (GetCheckedRadioButton(IDC_RD_TO_BLENDER, IDC_RD_FROM_BLENDER));

	if (selItem != _actDirection)
	{
		_subFormList.ShowSubForm(((_actTool - IDC_RD_ARMOR) * 2) + ((selItem == IDC_RD_TO_BLENDER) ? 0 : 1));
		_actDirection = selItem;
		GetDlgItem(IDC_BT_CONVERT)->SetWindowText((_actDirection == IDC_RD_TO_BLENDER) ? _T("Prep. Blender") : _T("Prep. Skyrim"));
	}
}
