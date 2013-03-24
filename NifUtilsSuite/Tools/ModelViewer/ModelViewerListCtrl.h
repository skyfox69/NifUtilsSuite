/**
 *  file:   ModelViewerListCtrl.h
 *  class:  CModelViewerListCtrl
 *
 *  ModelViewerListCtrl for displaying model in ListView
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include <string>

#include "DirectX\IfcDirectXRenderObject.h"

//-----  DEFINES  -------------------------------------------------------------
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CModelViewerListCtrl : public CListCtrl
{
	protected:
		string				_emptyMessage;
		bool				_toggleRowBackground;

		afx_msg	void		OnPaint     ();
		afx_msg	void		OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg BOOL		OnClickEx(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg	void		OnContextMenu(CWnd* pWnd, CPoint pos);
		afx_msg void		OnMmvModelviewWireframecolor();
		afx_msg void		OnMmvDisplayNone();
		afx_msg void		OnMmvDisplayWireframe();
		afx_msg void		OnMmvDisplaySolid();
		afx_msg void		OnMmvDisplayTexture();
		afx_msg BOOL		OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

		virtual	void		SetSelectedRenderMode(DirectXRenderMode renderMode);
		virtual int			OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
		virtual	bool		CellRectFromPoint(CPoint& point, RECT& cellrect, int& row, int& col) const;

		DECLARE_MESSAGE_MAP()

	public:
							CModelViewerListCtrl();
		virtual				~CModelViewerListCtrl();

		virtual	void		SetEmptyMessage(const string message);
		virtual bool		SetToggleRowBackground(const bool toggle);
};
