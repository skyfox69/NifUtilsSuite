/**
 *  file:   DirectXView.h
 *  class:  CDirectXView
 *
 *  Interface between MFC and DirectX9
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "DirectX\DirectXGraphics.h"

//-----  DEFINES  -------------------------------------------------------------

//-----  CLASS  ---------------------------------------------------------------
class CDirectXView : public CWnd, public CDirectXGraphics
{
	protected:
		UINT_PTR				_unpTimer;
		float					_posXLast;
		float					_posYLast;

		DECLARE_MESSAGE_MAP()

	public:
								CDirectXView();
		virtual					~CDirectXView();

		afx_msg	void			OnTimer     (UINT nIDEvent);
		afx_msg	void			OnPaint     ();
		afx_msg	int				OnCreate    (LPCREATESTRUCT lpCreateStruct);
		afx_msg	void			OnMouseMove (UINT nFlags, CPoint point);
		afx_msg	BOOL			OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
		afx_msg	void			OnLButtonDown(UINT nFlags, CPoint point);

		virtual	void			SetOwnTimer(UINT_PTR pTimer);
};
