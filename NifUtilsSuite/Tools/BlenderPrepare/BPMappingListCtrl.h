/**
 *  file:   BPMappingListCtrl.h
 *  class:  CBPMappingListCtrl
 *
 *  BPMappingListCtrl for defining mapping between body parts
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include <string>

//-----  DEFINES  -------------------------------------------------------------
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class CBPMappingComboBox : public CComboBox
{
	protected:
		unsigned short		_row;
		unsigned short		_col;
		unsigned short		_initial;
		bool				_pressedEscape;

		virtual	BOOL		PreTranslateMessage(MSG* pMsg);
		afx_msg	void		OnNcDestroy();
		afx_msg void		OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void		OnKillFocus(CWnd* pNewWnd);
		afx_msg void		OnCloseup();
		afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void		OnSize(UINT nType, int cx, int cy);

		DECLARE_MESSAGE_MAP()

	public:
							CBPMappingComboBox(const unsigned short row, const unsigned short col, const unsigned short initial);
		virtual				~CBPMappingComboBox();
};

//-----  CLASS  ---------------------------------------------------------------
class CBPMappingListCtrl : public CListCtrl
{
	protected:
		string				_emptyMessage;
		bool				_toggleRowBackground;

		afx_msg	void		OnPaint     ();
		afx_msg	void		OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg BOOL		OnClickEx(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg BOOL		OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void		OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);

		virtual int			OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
		virtual	bool		CellRectFromPoint(CPoint& point, RECT& cellrect, int& row, int& col) const;

		DECLARE_MESSAGE_MAP()

	public:
							CBPMappingListCtrl();
		virtual				~CBPMappingListCtrl();

		virtual	void		SetEmptyMessage(const string message);
		virtual bool		SetToggleRowBackground(const bool toggle);
};
