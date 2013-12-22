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
#include "Common\GUI\ToolsFormViewBase.h"

//-----  CLASS  ---------------------------------------------------------------
class CFormChunkMergeView : public ToolsFormViewBase
{
	private:
		CString						_fileNameIn;
		CString						_fileNameColl;
		CString						_template;

		enum{ IDD = IDD_FORM_CHUNKMERGE };

	protected:
									CFormChunkMergeView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();

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
		afx_msg void				OnBnClickedRdCollGlobal();
		afx_msg void				OnBnClickedRdCollLocal();

		DECLARE_DYNCREATE(CFormChunkMergeView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormChunkMergeView();

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormChunkMergeView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
