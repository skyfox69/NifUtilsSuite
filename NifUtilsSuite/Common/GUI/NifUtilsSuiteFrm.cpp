/**
 *  file:   NifUtilsSuiteFrm.cpp
 *  class:  CNifUtilsSuiteFrame
 *
 *  Frame window
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "NifUtilsSuiteFrm.h"
#include "Common\Util\FDToolEntry.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\Configuration.h"
#include "Common\GUI\OptionsSheet.h"
#include "Common\GUI\AboutDialog.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\IfcLogMessageObject.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Tools\NifConvert\FormNifConvertView.h"
#include "Tools\ChunkMerge\FormChunkMergeView.h"
#include "Tools\ModelViewer\FormModelViewerView.h"
#include "Tools\ChunkExtract\FormChunkExtractView.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNCREATE(CNifUtilsSuiteFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CNifUtilsSuiteFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_TOOLS_NIFCONVERT,      &CNifUtilsSuiteFrame::SelectTabNifConvert)
	ON_COMMAND(ID_TOOLS_CHUNKMERGE,      &CNifUtilsSuiteFrame::SelectTabChunkMerge)
	ON_COMMAND(ID_TOOLS_MODELVIEWER,     &CNifUtilsSuiteFrame::SelectTabModelViewer)
	ON_COMMAND(ID_TOOLS_CHUNKEXTRACT,    &CNifUtilsSuiteFrame::SelectTabChunkExtract)
	ON_COMMAND(ID_OPTIONS_EDIT,          &CNifUtilsSuiteFrame::OnFileOptions)
	ON_COMMAND(ID_OPTIONS_SAVECURRENT,   &CNifUtilsSuiteFrame::OnOptionsSavecurrent)
	ON_COMMAND(ID_OPTIONS_SHOWTOOLTIPPS, &CNifUtilsSuiteFrame::OnOptionsShowtooltipps)
	ON_COMMAND(ID_HELP_ABOUT,            &CNifUtilsSuiteFrame::OnHelpAbout)
	ON_COMMAND(ID_OPTIONS_SAVEOPENVIEW,  &CNifUtilsSuiteFrame::OnOptionsSaveopenview)
END_MESSAGE_MAP()

//-----  STATICS  -------------------------------------------------------------
static SFDToolEntry toolList[] = { SFDToolEntry(RUNTIME_CLASS(CFormNifConvertView),   ID_TOOLS_NIFCONVERT,   " NifConvert",   5),
								   SFDToolEntry(RUNTIME_CLASS(CFormChunkMergeView),   ID_TOOLS_CHUNKMERGE,   " ChunkMerge",   6),
								   SFDToolEntry(RUNTIME_CLASS(CFormChunkExtractView), ID_TOOLS_CHUNKEXTRACT, " ChunkExtract", 11),
								   SFDToolEntry(RUNTIME_CLASS(CFormModelViewerView),  ID_TOOLS_MODELVIEWER,  " ModelViewer",  4),
								   SFDToolEntry(NULL, 0, "", 0)
								};

void logCallback(const int type, const char* pMessage)
{
	((CNifUtilsSuiteFrame*) AfxGetApp()->GetMainWnd())->LogMessage(type, pMessage);
}

//-----  CNifUtilsSuiteFrame()  -----------------------------------------------
CNifUtilsSuiteFrame::CNifUtilsSuiteFrame()
{}

//-----  ~CNifUtilsSuiteFrame()  ----------------------------------------------
CNifUtilsSuiteFrame::~CNifUtilsSuiteFrame()
{}

//-----  OnCreate()  ----------------------------------------------------------
int CNifUtilsSuiteFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return (CFrameWnd::OnCreate(lpCreateStruct) == -1) ? -1 : 0;
}

//-----  OnClose()  -----------------------------------------------------------
void CNifUtilsSuiteFrame::OnClose()
{
	Configuration*	pConfig(Configuration::getInstance());
	int				selView(m_wndTabBar.GetCurSel());

	//  ModelViewer is forbidden and doesn't make sense
	if (pConfig->_saveLastView && (selView < 3) && (selView != pConfig->_lastOpenView))
	{
		pConfig->_lastOpenView = selView;
		pConfig->write();
	}

	CFrameWnd::OnClose();
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CNifUtilsSuiteFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))	return FALSE;

	cs.cx = 646;
	cs.cy = 465;
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE | WS_MINIMIZEBOX | WS_SYSMENU;

	return TRUE;
}

//-----  OnCreateClient()  ----------------------------------------------------
BOOL CNifUtilsSuiteFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	Configuration*	pConfig(Configuration::getInstance());
	CMenu*			pMenu(GetMenu());
	CString			text;

	//  create tab-bar
	if (!m_wndTabBar.Create(this))		return FALSE;

	m_wndTabBar.SetMinTabWidth(90);

	// setup image list for bar
	m_wndTabBar.SetImageList(CFDResourceManager::getInstance()->getImageListOther());

	//  get 'Tools' menu entry
	pMenu = pMenu->GetSubMenu(1);

	for (short idx(0); ; ++idx)
	{
		//  reached end of list
		if (toolList[idx]._pRTClass == NULL)	break;

		// add tab to TabBar
		if (m_wndTabBar.AddView(toolList[idx]._pRTClass, CString(toolList[idx]._title.c_str()), toolList[idx]._resId, pContext) < 0)		continue;

		//  get first items text
		pMenu->GetMenuStringW(0, text, MF_BYPOSITION);

		if (text.IsEmpty())
		{
			pMenu->ModifyMenu(0, MF_STRING, toolList[idx]._cmdId, CString(toolList[idx]._title.c_str()));
		}
		else
		{
			pMenu->AppendMenu(MF_STRING, toolList[idx]._cmdId, CString(toolList[idx]._title.c_str()));
		}
	}  //  for (short idx(0); ; ++idx)

	//  set tool tipp usage
	pMenu = GetMenu()->GetSubMenu(2);
	pMenu->CheckMenuItem(ID_OPTIONS_SHOWTOOLTIPPS, (pConfig->_showToolTipps ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_OPTIONS_SAVEOPENVIEW,  (pConfig->_saveLastView  ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

	//  log messages from material loading
	vector<string>	userMessages(NifUtlMaterialList::getInstance()->getUserMessages());

	for (auto pIter=userMessages.begin(), pEnd=userMessages.end(); pIter != pEnd; ++pIter)
	{
		LogMessage(NCU_MSG_TYPE_INFO, pIter->c_str());
	}

	//  switch to last activated view
	::PostMessage(m_hWnd, WM_COMMAND , toolList[pConfig->_lastOpenView]._cmdId, 0);

	return TRUE;
}

//-----  SetActiveView()  -----------------------------------------------------
void CNifUtilsSuiteFrame::SetActiveView(CView* pViewNew, BOOL bNotify)
{
	SetWindowText(_T("NIFUtilsSuite (Beta) -") + m_wndTabBar.GetItemText());

	CFrameWnd::SetActiveView(pViewNew, bNotify);
}

//-----  SelectView()  --------------------------------------------------------
void CNifUtilsSuiteFrame::SelectView(int cmdId)
{
	for (int idx(0); ; ++idx)
	{
		//  reached end of list
		if (toolList[idx]._pRTClass == NULL)	break;

		//  matching type?
		if (toolList[idx]._cmdId == cmdId)
		{
			m_wndTabBar.SetCurSel(idx);
		}
	}
}

//-----  SelectTabNifConvert()  -----------------------------------------------
void CNifUtilsSuiteFrame::SelectTabNifConvert()
{
	SelectView(ID_TOOLS_NIFCONVERT);
}

//-----  SelectTabChunkMerge()  -----------------------------------------------
void CNifUtilsSuiteFrame::SelectTabChunkMerge()
{
	SelectView(ID_TOOLS_CHUNKMERGE);
}

//-----  SelectTabModelViewer()  ----------------------------------------------
void CNifUtilsSuiteFrame::SelectTabModelViewer()
{
	SelectView(ID_TOOLS_MODELVIEWER);
}

//-----  SelectTabChunkExtract()  ---------------------------------------------
void CNifUtilsSuiteFrame::SelectTabChunkExtract()
{
	SelectView(ID_TOOLS_CHUNKEXTRACT);
}

//-----  SelectTabModelViewer()  ----------------------------------------------
void CNifUtilsSuiteFrame::OnFileOptions()
{
	COptionsSheet	optSheet(_T("Settings"));
	INT_PTR			retVal  (optSheet.DoModal());

	if ((retVal == IDOK) || (retVal == ID_WIZFINISH))
	{
		Configuration::getInstance()->write();
		m_wndTabBar.BroadcastEvent(IBCE_CHANGED_SETTINGS);
	}
}

//-----  OnOptionsSavecurrent()  ----------------------------------------------
void CNifUtilsSuiteFrame::OnOptionsSavecurrent()
{
	Configuration::getInstance()->write();
}

//-----  OnOptionsShowtooltipps()  --------------------------------------------
void CNifUtilsSuiteFrame::OnOptionsShowtooltipps()
{
	CMenu*		pMenu(GetMenu());

	//  toggle tool tipps
	Configuration::getInstance()->_showToolTipps = !Configuration::getInstance()->_showToolTipps;
	Configuration::getInstance()->write();

	//  get 'Options' menu entry
	pMenu = pMenu->GetSubMenu(2);
	pMenu->CheckMenuItem(ID_OPTIONS_SHOWTOOLTIPPS, (Configuration::getInstance()->_showToolTipps ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

	m_wndTabBar.BroadcastEvent(IBCE_SET_TOOLTIPP);
}

//-----  OnOptionsSaveopenview()  ---------------------------------------------
void CNifUtilsSuiteFrame::OnOptionsSaveopenview()
{
	CMenu*		pMenu(GetMenu());

	//  toggle tool tipps
	Configuration::getInstance()->_saveLastView = !Configuration::getInstance()->_saveLastView;
	Configuration::getInstance()->write();

	//  get 'Options' menu entry
	pMenu = pMenu->GetSubMenu(2);
	pMenu->CheckMenuItem(ID_OPTIONS_SAVEOPENVIEW, (Configuration::getInstance()->_saveLastView ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
}

//-----  LogMessage()  --------------------------------------------------------
void CNifUtilsSuiteFrame::LogMessage(const int type, const char* pMessage, ...)
{
	//  decode var-args
	char	cbuffer[5000] = {0};
	va_list	ap;

	va_start(ap, pMessage);
	vsnprintf(cbuffer, 5000, pMessage, ap);
	va_end(ap);

	//  start logging
	Configuration*	pConfig(Configuration::getInstance());
	CHARFORMAT		charFormat = {0};
	CString			text (cbuffer);
	int				tType(type);

	//  special handling of type settings
	if (cbuffer[0] == '^')
	{
		tType = atoi(cbuffer+1);
		text  = (cbuffer+2);
	}

	//  early return on non enabled type
	if (!pConfig->_lvwLogActive[tType])		return;

	//  append of newline necessary?
	if (cbuffer[strlen(cbuffer) - 1] != '\n')
	{
		text += _T("\r\n");
	}

	//  character format
	charFormat.cbSize      = sizeof(charFormat);
	charFormat.dwMask      = CFM_COLOR;
	charFormat.dwEffects   = 0;
	charFormat.crTextColor = pConfig->_lvwColors[tType];

	//  broadcast to every tab
	m_wndTabBar.LogMessage(text, &charFormat);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CNifUtilsSuiteFrame::BroadcastEvent(WORD event, void* pParameter)
{
	return m_wndTabBar.BroadcastEvent(event, pParameter);
}

//-----  OnHelpAbout()  -------------------------------------------------------
void CNifUtilsSuiteFrame::OnHelpAbout()
{
	CAboutDialog	aboutDlg(_T("About NifUtilsSuite"));
	
	aboutDlg.DoModal();
}

