/**
 *  file:   ToolsFormViewBase.h
 *  class:  ToolsFormViewBase
 *
 *  Base class for tool views integrated in NifUtilsSuite
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\GUI\ToolsFormViewBase.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDFileHelper.h"
#include <sstream>

using namespace std;

//-----  DEFINES  -------------------------------------------------------------

//-----  ToolsFormViewBase()  -------------------------------------------------
ToolsFormViewBase::ToolsFormViewBase(UINT nIDTemplage, LogMessageObject::ClientType clientType)
	:	CFormView       (nIDTemplage),
		LogMessageObject(clientType)
{}

//-----  ~ToolsFormViewBase()  ------------------------------------------------
ToolsFormViewBase::~ToolsFormViewBase()
{}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL ToolsFormViewBase::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CFormView::PreTranslateMessage(pMsg);
}

//-----  OnCtlColor()  --------------------------------------------------------
HBRUSH ToolsFormViewBase::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH  hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_GBOX_FILES:
		case IDC_GBOX_VCOLORS:
		case IDC_GBOX_FLAGS:
		case IDC_GBOX_HINTS:
		case IDC_GBOX_COLLISION:
		case IDC_GBOX_MATERIAL:
		case IDC_GBOX_HANDLING:
		case IDC_GBOX_TOOLS:
		case IDC_GBOX_TRIANGLES:
		{
			pDC->SetTextColor(RGB(0,0,255));
			pDC->SetBkMode(TRANSPARENT);
			hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			break;
		}
	}

	return hbr;
}

//-----  ShowModel()  ---------------------------------------------------------
void ToolsFormViewBase::ShowModel(CString fileName)
{
	//  switch to form ModelView
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_TOOLS_MODELVIEWER);

	//  load model
	((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->BroadcastEvent(IBCE_SHOW_MODEL, (void*) CStringA(fileName).GetString());
}

//-----  OpenNifSkope()  ------------------------------------------------------
void ToolsFormViewBase::OpenNifSkope(CString fileName)
{
	string	cmdString(Configuration::getInstance()->_pathNifSkope);

	if (!cmdString.empty())
	{
		STARTUPINFO			startupInfo = {0};
		PROCESS_INFORMATION	processInfo = {0};
		stringstream		sStream;

		startupInfo.cb = sizeof(startupInfo);

		sStream << "\"" << cmdString << "\" \"" << CStringA(fileName).GetString() << "\"";
		CreateProcess(CString(cmdString.c_str()).GetString(), (LPWSTR) CString(sStream.str().c_str()).GetString(), NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startupInfo, &processInfo);
	}
	else
	{
		AfxMessageBox(_T("You didn't specify the NifSkope\r\nexecuteable in options."), MB_OK | MB_ICONEXCLAMATION);
	}
}

//-----  PrepareToolTips()  ---------------------------------------------------
void ToolsFormViewBase::PrepareToolTips(SFDToolTipText* pToolTips)
{
	if (_toolTipCtrl.Create(this, TTS_USEVISUALSTYLE | TTS_BALLOON))
	{
		for (short i(0); pToolTips[i]._uid != -1; ++i)
		{
			_toolTipCtrl.AddTool(GetDlgItem(pToolTips[i]._uid), CString(pToolTips[i]._text.c_str()));
		}

		_toolTipCtrl.SetMaxTipWidth(260);
		_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
	}
}

//-----  GetFileName()  -------------------------------------------------------
CString ToolsFormViewBase::GetFileName(const CString fileNameIn, CString filter, CString extension, bool saveDialog, CString title, DWORD addFlags)
{
	Configuration*	pConfig (Configuration::getInstance());
	CString			fileName(fileNameIn);
	CString			fileNameOut;

	//  set default input dir if empty
	if (fileName.IsEmpty() && !pConfig->_pathDefaultInput.empty())
	{
		fileName = CString(pConfig->_pathDefaultInput.c_str()) + _T("\\");
	}

	//  get new input file
	fileName = FDFileHelper::getFile(fileName, filter, extension, saveDialog, title);

	if (!fileName.IsEmpty() && (fileName != (CString(pConfig->_pathDefaultInput.c_str()) + _T("\\") )))
	{
		fileNameOut = fileName;
	}

	return fileNameOut;
}