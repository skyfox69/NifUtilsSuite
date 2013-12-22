/**
 *  file:   ToolsFormViewBase.h
 *  class:  ToolsFormViewBase
 *
 *  Base class for tool views integrated in NifUtilsSuite
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\LogMessageObject.h"
#include "Common\Util\FDToolTipText.h"

//-----  CLASS  ---------------------------------------------------------------
class ToolsFormViewBase : public CFormView, public LogMessageObject, public IfcBroadcastObject
{
	protected:
		CToolTipCtrl				_toolTipCtrl;

									ToolsFormViewBase(UINT nIDTemplage, LogMessageObject::ClientType clientType);
		virtual						~ToolsFormViewBase();
		virtual	BOOL				PreTranslateMessage(MSG* pMsg);
	
		afx_msg	HBRUSH				OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

		virtual void				ShowModel      (CString fileName);
		virtual void				OpenNifSkope   (CString fileName);
		virtual	void				PrepareToolTips(SFDToolTipText* pToolTips);
		virtual	CString				GetFileName    (const CString fileNameIn, CString filter, CString extension, bool saveDialog=false, CString title=_T(""), DWORD addFlags=0);
};
