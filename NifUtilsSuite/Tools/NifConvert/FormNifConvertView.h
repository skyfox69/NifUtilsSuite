/**
 *  file:   FormNifConvertView.h
 *  class:  CFormNifConvertView
 *
 *  Form for NifConvert view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\GUI\ToolsFormViewBase.h"

//-----  CLASS  ---------------------------------------------------------------
class CFormNifConvertView : public ToolsFormViewBase
{
	private:
		CString						_fileNameIn;
		CString						_fileNameOut;
		CString						_texturePath;
		CString						_template;

		enum{ IDD = IDD_FORM_NIFCONVERT };

	protected:
									CFormNifConvertView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();
	
		afx_msg void				OnBnClickedBtViewIn();
		afx_msg void				OnBnClickedBtViewOut();
		afx_msg void				OnBnClickedOpenSettings();
		afx_msg void				OnBnClickedRdVtColor();
		afx_msg void				OnBnClickedBtFileIn();
		afx_msg void				OnBnClickedBtFileOut();
		afx_msg void				OnBnClickedBtResetForm();
		afx_msg void				OnBnClickedBtConvert();
		afx_msg void				OnBnClickedBtNscopeIn();
		afx_msg void				OnBnClickedBtNscopeOut();

		DECLARE_DYNCREATE(CFormNifConvertView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormNifConvertView();

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormNifConvertView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
