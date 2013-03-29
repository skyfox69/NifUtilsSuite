/**
 *  file:   FormBlenderPrepareView.h
 *  class:  CFormBlenderPrepareView
 *
 *  Form for BlenderPrepare view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\LogMessageObject.h"
#include "Common\Util\FDSubFormList.h"
#include <vector>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CFormBlenderPrepareView : public CFormView, public LogMessageObject, public IfcBroadcastObject
{
	private:
		CFDSubFormList				_subFormList;
		CToolTipCtrl				_toolTipCtrl;
		CString						_fileNameIn;
		CString						_fileNameOut;
		CString						_template;
		int							_actDirection;
		int							_actTool;

		enum{ IDD = IDD_FORM_BLENDERPREPARE };

	protected:
									CFormBlenderPrepareView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();
		virtual	BOOL				PreTranslateMessage(MSG* pMsg);

		afx_msg	HBRUSH				OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void				OnBnClickedBtViewIn();
		afx_msg void				OnBnClickedBtViewOut();
		afx_msg void				OnBnClickedBtFileIn();
		afx_msg void				OnBnClickedBtFileOut();
		afx_msg void				OnBnClickedBtResetForm();
		afx_msg void				OnBnClickedBtNscopeIn();
		afx_msg void				OnBnClickedBtNscopeOut();
		afx_msg void				OnBnClickedBtConvert();
		afx_msg void				OnBnClickedRdDirection();

		DECLARE_DYNCREATE(CFormBlenderPrepareView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormBlenderPrepareView();
		virtual	BOOL				PreCreateWindow(CREATESTRUCT& cs);

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);

		//  IfcLogMessageObject
		virtual	void				LogMessage(const CString text, const CHARFORMAT* pFormat);
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormBlenderPrepareView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
