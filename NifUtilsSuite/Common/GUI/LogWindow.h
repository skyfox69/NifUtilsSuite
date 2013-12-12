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
#include "Common\Util\LogMessageObject.h"

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CLogWindow : public CDialog, public LogMessageObject
{
	private:
		CWnd*						_pParent;

	protected:
		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CLogWindow)

		virtual	void				PostNcDestroy();
		virtual	BOOL				OnInitDialog();
		afx_msg	void				OnClose();

	public:
									CLogWindow(CWnd* pParentWnd = NULL);
		virtual						~CLogWindow();

		//  IfcLogMessageObject
		virtual	void				LogMessage(const CString text, const CHARFORMAT* pFormat);
};
