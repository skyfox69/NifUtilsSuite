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
#include "Common\Nif\NifUtlMaterial.h"
#include "Tools\NifConvert\FormNifConvertView.h"
#include "Tools\ChunkMerge\FormChunkMergeView.h"
#include "Tools\ChunkExtract\FormChunkExtractView.h"
#include "Tools\BlenderPrepare\FormBlenderPrepareView.h"
#ifndef NUS_LIGHT
#include "Tools\ModelViewer\FormModelViewerView.h"
#endif

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNCREATE(CNifUtilsSuiteFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CNifUtilsSuiteFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_TOOLS_NIFCONVERT,      &CNifUtilsSuiteFrame::SelectTabNifConvert)
	ON_COMMAND(ID_TOOLS_CHUNKMERGE,      &CNifUtilsSuiteFrame::SelectTabChunkMerge)
	ON_COMMAND(ID_TOOLS_MODELVIEWER,     &CNifUtilsSuiteFrame::SelectTabModelViewer)
	ON_COMMAND(ID_TOOLS_CHUNKEXTRACT,    &CNifUtilsSuiteFrame::SelectTabChunkExtract)
	ON_COMMAND(ID_TOOLS_BLENDERPREPARE,  &CNifUtilsSuiteFrame::SelectTabBlenderPrepare)
	ON_COMMAND(ID_OPTIONS_EDIT,          &CNifUtilsSuiteFrame::OnFileOptions)
	ON_COMMAND(ID_BT_OPTIONS_EDIT,       &CNifUtilsSuiteFrame::OnButtonOptions)
	ON_COMMAND(ID_OPTIONS_SAVECURRENT,   &CNifUtilsSuiteFrame::OnOptionsSavecurrent)
	ON_COMMAND(ID_OPTIONS_SHOWTOOLTIPPS, &CNifUtilsSuiteFrame::OnOptionsShowtooltipps)
	ON_COMMAND(ID_HELP_ABOUT,            &CNifUtilsSuiteFrame::OnHelpAbout)
	ON_COMMAND(ID_OPTIONS_SAVEOPENVIEW,  &CNifUtilsSuiteFrame::OnOptionsSaveopenview)
	ON_COMMAND(ID_OPTIONS_SHOWLOGWINDOW, &CNifUtilsSuiteFrame::OnOptionsShowlogwindow)
END_MESSAGE_MAP()

//-----  STATICS  -------------------------------------------------------------
static SFDToolEntry toolList[] = { SFDToolEntry(RUNTIME_CLASS(CFormNifConvertView),     ID_TOOLS_NIFCONVERT,     " NifConvert",      5),
								   SFDToolEntry(RUNTIME_CLASS(CFormChunkMergeView),     ID_TOOLS_CHUNKMERGE,     " ChunkMerge",      6),
								   SFDToolEntry(RUNTIME_CLASS(CFormChunkExtractView),   ID_TOOLS_CHUNKEXTRACT,   " ChunkExtract",   11),
								   SFDToolEntry(RUNTIME_CLASS(CFormBlenderPrepareView), ID_TOOLS_BLENDERPREPARE, " BlenderPrepare", 15),
#ifndef NUS_LIGHT
								   SFDToolEntry(RUNTIME_CLASS(CFormModelViewerView),    ID_TOOLS_MODELVIEWER,    " ModelViewer",     4),
#endif
								   SFDToolEntry(NULL, 0, "", 0)
								};

void logCallback(const int type, const char* pMessage)
{
	((CNifUtilsSuiteFrame*) AfxGetApp()->GetMainWnd())->LogMessage(type, pMessage);
}

bool visCallback(NifCollisionUtility* pCollUtil)
{
	return ((CNifUtilsSuiteFrame*) AfxGetApp()->GetMainWnd())->VisualTriangeWinding(pCollUtil);
}

//-----  CNifUtilsSuiteFrame()  -----------------------------------------------
CNifUtilsSuiteFrame::CNifUtilsSuiteFrame()
	:	LogMessageObject(LogMessageObject::COMMON),
		m_pLogWindow    (NULL)
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
	RECT			tRect = {0};

	//  ModelViewer is forbidden and doesn't make sense
	if (pConfig->_saveLastView && (selView < 4) && (selView != pConfig->_lastOpenView))
	{
		pConfig->_lastOpenView = selView;
	}

	GetWindowRect(&tRect);
	pConfig->_framePosX = tRect.left;
	pConfig->_framePosY = tRect.top;
	pConfig->write();

	//  destroy log window if open
	if (m_pLogWindow != NULL)		m_pLogWindow->DestroyWindow();

	CFrameWnd::OnClose();
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CNifUtilsSuiteFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	Configuration*	pConfig(Configuration::getInstance());

	if (!CFrameWnd::PreCreateWindow(cs))	return FALSE;

	cs.cx = 646;
	cs.cy = 490;
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE | WS_MINIMIZEBOX | WS_SYSMENU;
	if (pConfig->_framePosX >= 0)
	{
		cs.x = pConfig->_framePosX;
		cs.y = pConfig->_framePosY;
	}

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
	pMenu->CheckMenuItem(ID_OPTIONS_SHOWLOGWINDOW, (pConfig->_showLogWindow ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
	pMenu->CheckMenuItem(ID_OPTIONS_SAVEOPENVIEW,  (pConfig->_saveLastView  ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

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

//-----  SelectTabBlenderPrepare()  -------------------------------------------
void CNifUtilsSuiteFrame::SelectTabBlenderPrepare()
{
	SelectView(ID_TOOLS_BLENDERPREPARE);
}

//-----  OnFileOptions()  -----------------------------------------------------
void CNifUtilsSuiteFrame::OnFileOptions()
{
	CRuntimeClass*	pClass(m_wndTabBar.GetItemView(m_wndTabBar.GetCurSel())->GetRuntimeClass());
	int				cmdId (-1);

	//  get matching id
	for (short idx(0); ; ++idx)
	{
		//  reached end of list
		if (toolList[idx]._pRTClass == NULL)	break;

		if (toolList[idx]._pRTClass == pClass)
		{
			cmdId = toolList[idx]._cmdId;
			break;
		}
	}  //  for (short idx(0); ; ++idx)

	COptionsSheet	optSheet(_T("Settings"));
	INT_PTR			retVal  (optSheet.DoModal(cmdId));

	if ((retVal == IDOK) || (retVal == ID_WIZFINISH))
	{
		Configuration::getInstance()->write();
		m_wndTabBar.BroadcastEvent(IBCE_CHANGED_SETTINGS);
	}
}

//-----  OnButtonOptions()  ---------------------------------------------------
void CNifUtilsSuiteFrame::OnButtonOptions()
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

//-----  OnOptionsShowlogwindow()  --------------------------------------------
void CNifUtilsSuiteFrame::OnOptionsShowlogwindow()
{
	CMenu*			pMenu  (GetMenu());
	Configuration*	pConfig(Configuration::getInstance());

	//  toggle show log-window
	pConfig->_showLogWindow = !Configuration::getInstance()->_showLogWindow;
	pConfig->write();

	//  get 'Options' menu entry
	pMenu = pMenu->GetSubMenu(2);
	pMenu->CheckMenuItem(ID_OPTIONS_SHOWLOGWINDOW, (pConfig->_showLogWindow ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

	//  display log-window
	if (pConfig->_showLogWindow)
	{
		//  already existing?
		if (m_pLogWindow != NULL)
		{
			m_pLogWindow->SetForegroundWindow();
		}
		else
		{
			//  create new one
			m_pLogWindow = new CLogWindow(this);
			m_pLogWindow->Create(IDD_LOG_WINDOW, GetDesktopWindow());
			m_pLogWindow->ShowWindow(SW_SHOW);
		}
	}
	else if (m_pLogWindow != NULL)
	{
		//  destroy existing log-window
		m_pLogWindow->DestroyWindow();
	}
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

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CNifUtilsSuiteFrame::BroadcastEvent(WORD event, void* pParameter)
{
	BOOL	retVal(FALSE);

	switch (event)
	{
		case IBCE_LOG_WINDOW_KILLED:
		{
			CMenu*	pMenu(GetMenu());

			//  toggle show log-window
			Configuration::getInstance()->_showLogWindow = false;

			//  get 'Options' menu entry
			pMenu = pMenu->GetSubMenu(2);
			pMenu->CheckMenuItem(ID_OPTIONS_SHOWLOGWINDOW, MF_UNCHECKED | MF_BYCOMMAND);

			m_pLogWindow = NULL;
			retVal       = TRUE;
			break;
		}

		default:
		{
			retVal = m_wndTabBar.BroadcastEvent(event, pParameter);
		}
	}
	return retVal;
}

//-----  OnHelpAbout()  -------------------------------------------------------
void CNifUtilsSuiteFrame::OnHelpAbout()
{
	CAboutDialog	aboutDlg(_T("About NifUtilsSuite"));
	
	aboutDlg.DoModal();
}

//-----  ShowLogWindow()  -----------------------------------------------------
void CNifUtilsSuiteFrame::ShowLogWindow()
{
	//  create LogWindow if enabled
	if (Configuration::getInstance()->_showLogWindow)
	{
		m_pLogWindow = new CLogWindow(this);
		m_pLogWindow->Create(IDD_LOG_WINDOW, GetDesktopWindow());
		m_pLogWindow->ShowWindow(SW_SHOW);

		//  log messages from material loading
		vector<string>	userMessages(NifUtlMaterialList::getInstance()->getUserMessages());

		for (auto pIter=userMessages.begin(), pEnd=userMessages.end(); pIter != pEnd; ++pIter)
		{
			LogMessage(NCU_MSG_TYPE_INFO, pIter->c_str());
		}

		//  dump exisitng log buffer
		DumpLogBuffer();
	}
}

//-----  VisualTriangeWinding()  ----------------------------------------------
bool CNifUtilsSuiteFrame::VisualTriangeWinding(NifCollisionUtility* pCollUtil)
{


	return true;
}