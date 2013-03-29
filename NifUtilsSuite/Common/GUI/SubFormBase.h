/**
 *  file:   SubFormBase.h
 *  class:  CSubFormBase
 *
 *  Base class for dialogs used as sub forms
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\IfcBroadcastObject.h"
#include <string>

//-----  DEFINES  -------------------------------------------------------------
//  used namespaces
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CSubFormBase : public CDialogEx, public IfcBroadcastObject
{
	protected:
		CToolTipCtrl			_toolTipCtrl;

		virtual	BOOL			PreTranslateMessage(MSG* pMsg);
		virtual	BOOL			OnInitDialog();
		afx_msg	HBRUSH			OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

		DECLARE_DYNAMIC(CSubFormBase)
		DECLARE_MESSAGE_MAP()

	public:
								CSubFormBase(UINT idTemplate, CWnd* pParent = NULL);
		virtual					~CSubFormBase();

		virtual	unsigned short	PerformAction(const string fileNameIn, const string fileNameOut) = 0;
};
