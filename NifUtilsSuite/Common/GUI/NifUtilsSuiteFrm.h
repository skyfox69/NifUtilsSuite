/**
 *  file:   NifUtilsSuiteFrm.h
 *  class:  CNifUtilsSuiteFrame
 *
 *  Frame window
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "BCTabBarCtrl.h"
#include "LogWindow.h"

//-----  CLASS  ---------------------------------------------------------------
class CNifUtilsSuiteFrame : public CFrameWnd
{
	private:

	protected:
		CLogWindow*		m_pLogWindow;
		CStatusBar		m_wndStatusBar;
		CToolBar		m_wndToolBar;
		BCTabBarCtrl	m_wndTabBar;

						CNifUtilsSuiteFrame();
		virtual	BOOL	OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
		virtual	void	SelectView(int cmdId);

		afx_msg void	OnClose();
		afx_msg	int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg	void	OnFileOptions();
		afx_msg	void	OnButtonOptions();
		afx_msg void	OnOptionsSavecurrent();
		afx_msg void	OnOptionsShowtooltipps();
		afx_msg void	OnOptionsSaveopenview();
		afx_msg void	OnOptionsShowlogwindow();
		afx_msg	void	SelectTabNifConvert();
		afx_msg	void	SelectTabChunkMerge();
		afx_msg	void	SelectTabModelViewer();
		afx_msg	void	SelectTabChunkExtract();
		afx_msg	void	SelectTabBlenderPrepare();
		afx_msg void	OnHelpAbout();

		DECLARE_DYNCREATE(CNifUtilsSuiteFrame)
		DECLARE_MESSAGE_MAP()

	public:
		virtual			~CNifUtilsSuiteFrame();
		virtual	BOOL	PreCreateWindow(CREATESTRUCT& cs);
				void	SetActiveView(CView* pViewNew, BOOL bNotify=TRUE);

		virtual	void	LogMessage(const int type, const char* pMessage, ...);
		virtual	BOOL	BroadcastEvent(WORD event, void* pParameter=NULL);
};
