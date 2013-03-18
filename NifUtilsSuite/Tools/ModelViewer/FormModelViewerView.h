/**
 *  file:   FormModelViewerView.h
 *  class:  CFormModelViewerView
 *
 *  Form for ModelViewer view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Tools\ModelViewer\ModelViewerListCtrl.h"
#include "DirectX\DirectXView.h"
#include <vector>

//-----  DEFINES  -------------------------------------------------------------

//-----  CLASS  ---------------------------------------------------------------
class CFormModelViewerView : public CFormView, public IfcBroadcastObject
{
	private:
		enum{ IDD = IDD_FORM_MODELVIEWER };

	protected:
		CModelViewerListCtrl		_listView;
		CToolTipCtrl				_toolTipCtrl;
		CSliderCtrl					_levelLOD;
		CDirectXView				_directXView;

									CFormModelViewerView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();
		virtual	BOOL				PreTranslateMessage(MSG* pMsg);

		afx_msg	BOOL				OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
		afx_msg void				OnBnClickedBtVwFront();
		afx_msg void				OnBnClickedBtVwTop();
		afx_msg void				OnBnClickedBtVwSide();
		afx_msg void				OnBnClickedCkVwButton();
		afx_msg void				OnBnClickedBtResetView();
		afx_msg	void				OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg	void				OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

		virtual	void				LoadModel(const string fileName);
		virtual	void				FillListView(vector<DirectXMesh*>& meshList);
	
		DECLARE_DYNCREATE(CFormModelViewerView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormModelViewerView();
		virtual	BOOL				PreCreateWindow(CREATESTRUCT& cs);

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);
		afx_msg void OnBnClickedCkDoubleSided();
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormModelViewerView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
