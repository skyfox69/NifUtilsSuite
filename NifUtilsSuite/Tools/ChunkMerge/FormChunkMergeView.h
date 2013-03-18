/**
 *  file:   FormChunkMergeView.h
 *  class:  CFormChunkMergeView
 *
 *  Form for ChunkMerge view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\LogMessageObject.h"

//-----  CLASS  ---------------------------------------------------------------
class CFormChunkMergeView : public CFormView, public LogMessageObject, public IfcBroadcastObject
{
	private:
		CToolTipCtrl				_toolTipCtrl;
		CString						_fileNameIn;
		CString						_fileNameColl;
		CString						_template;

		enum{ IDD = IDD_FORM_CHUNKMERGE };

	protected:
									CFormChunkMergeView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();
		virtual	BOOL				PreTranslateMessage(MSG* pMsg);

		afx_msg	HBRUSH				OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void				OnBnClickedBtViewIn();
		afx_msg void				OnBnClickedBtViewColl();
		afx_msg void				OnBnClickedOpenSettings();
		afx_msg void				OnBnClickedBtFileIn();
		afx_msg void				OnBnClickedBtFileColl();
		afx_msg void				OnBnClickedBtResetForm();
		afx_msg void				OnBnClickedBtNscopeIn();
		afx_msg void				OnBnClickedBtNscopeColl();
		afx_msg void				OnBnClickedBtConvert();
		afx_msg void				OnBnClickedRadioCollMat();

		DECLARE_DYNCREATE(CFormChunkMergeView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormChunkMergeView();
		virtual	BOOL				PreCreateWindow(CREATESTRUCT& cs);

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);

		//  IfcLogMessageObject
		virtual	void				LogMessage(const CString text, const CHARFORMAT* pFormat);
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormChunkMergeView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
