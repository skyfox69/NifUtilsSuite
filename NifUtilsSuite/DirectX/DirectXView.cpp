/**
 *  file:   DirectXView.cpp
 *  class:  CDirectXView
 *
 *  Interface between MFC and DirectX9
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "DirectX\DirectXView.h"

//-----  DEFINES  -------------------------------------------------------------
BEGIN_MESSAGE_MAP(CDirectXView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

//-----  CDirectXView  --------------------------------------------------------
CDirectXView::CDirectXView()
{}

//-----  ~CDirectXView  -------------------------------------------------------
CDirectXView::~CDirectXView()
{}

//-----  OnPaint  -------------------------------------------------------------
void CDirectXView::OnPaint()
{
	ValidateRect(NULL);
}

//-----  OnTimer  -------------------------------------------------------------
void CDirectXView::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case 1:
		{
			dxBeginScene();
			dxRenderScene();
			dxEndScene();
			break;
		}

		default:
		{
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

//-----  OnMouseWheel  --------------------------------------------------------
BOOL CDirectXView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	_zoom += (float)0.001f * zDelta;

	if (_zoom < 0.02f)	_zoom = 0.02f;

	return FALSE;
}

//-----  OnMouseMove  ---------------------------------------------------------
void CDirectXView::OnMouseMove(UINT nFlags, CPoint point)
{
	int diffX = (int)(point.x - _posXLast);
	int diffY = (int)(point.y - _posYLast);

	_posXLast  = (float)point.x;
	_posYLast  = (float)point.y;

	// Left mouse button
	if (nFlags & MK_LBUTTON)
	{
		_rotX += (float)0.5f * diffY;

		if ((_rotX > 360.0f) || (_rotX < -360.0f))
		{
			_rotX = 0.0f;
		}

		_rotY += (float)0.5f * diffX;

		if ((_rotY > 360.0f) || (_rotY < -360.0f))
		{
			_rotY = 0.0f;
		}
	}

	// Right mouse button
	else if (nFlags & MK_RBUTTON)
	{
		_zoom -= (float)0.1f * diffY;
		if (_zoom < 0.02f)	_zoom = 0.02f;
	}

	// Middle mouse button
	else if (nFlags & MK_MBUTTON)
	{
		_posX += (float)0.05f * diffX;
		_posY -= (float)0.05f * diffY;
	}

	CWnd::OnMouseMove(nFlags, point);
}

//-----  OnLButtonDown()  -----------------------------------------------------
void CDirectXView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect	rect;

	GetWindowRect(rect);
	ScreenToClient(rect);

	if ((point.x >= rect.left) &&
		(point.x <= rect.right) &&
		(point.y >= rect.top) &&
		(point.y <= rect.bottom)
	   )
	{
		SetFocus();
	}
}

//-----  OnCreate  ------------------------------------------------------------
int CDirectXView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)	return -1;

	if (!dxCreateRenderingContext(GetSafeHwnd(), _rectOrig.Width(), _rectOrig.Height()))
	{
		dxShutdown();
		return false;
	}

	if (!dxInitScene())
	{
		dxShutdown();
		return false;
	}

	return 0;
}

//-----  SetOwnTimer  ---------------------------------------------------------
void CDirectXView::SetOwnTimer(UINT_PTR pTimer)
{
	_unpTimer = pTimer;
}