/**
 *  file:   LogWindow.h
 *  class:  CLogWindow
 *
 *  Log Window class
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include <string>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CLogWindow : public CDialog
{
	private:
		CWnd*						_pParent;

	protected:
		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CLogWindow)

		virtual	void				PostNcDestroy();
		virtual	BOOL				OnInitDialog();
		afx_msg	void				OnDestroy();
		afx_msg	void				OnSize(UINT nType, int cx, int cy);

	public:
									CLogWindow(CWnd* pParentWnd = NULL);
		virtual						~CLogWindow();
		virtual	BOOL				PreCreateWindow(CREATESTRUCT& cs);

		virtual	void				LogMessage(const CString text, const CHARFORMAT* pFormat);
};
