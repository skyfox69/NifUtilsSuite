/**
 *  file:   ModelViewerListCtrl.cpp
 *  class:  CModelViewerListCtrl
 *
 *  ModelViewerListCtrl for displaying model in ListView
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "Tools\ModelViewer\ModelViewerListCtrl.h"
#include "DirectX\DirectXMesh.h"
#include "resource.h"

//-----  DEFINES  -------------------------------------------------------------
BEGIN_MESSAGE_MAP(CModelViewerListCtrl, CListCtrl)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT_EX(NM_CLICK,              &CModelViewerListCtrl::OnClickEx)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,            &CModelViewerListCtrl::OnCustomDraw)
	ON_COMMAND(ID_MMV_MODELVIEW_WIREFRAMECOLOR, &CModelViewerListCtrl::OnMmvModelviewWireframecolor)
	ON_COMMAND(ID_MMV_DISPLAY_NONE,             &CModelViewerListCtrl::OnMmvDisplayNone)
	ON_COMMAND(ID_MMV_DISPLAY_WIREFRAME,        &CModelViewerListCtrl::OnMmvDisplayWireframe)
	ON_COMMAND(ID_MMV_DISPLAY_SOLID,            &CModelViewerListCtrl::OnMmvDisplaySolid)
	ON_COMMAND(ID_MMV_DISPLAY_TEXTURE,          &CModelViewerListCtrl::OnMmvDisplayTexture)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CModelViewerListCtrl::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CModelViewerListCtrl::OnToolTipText)
END_MESSAGE_MAP()

//-----  CModelViewerListCtrl()  ----------------------------------------------
CModelViewerListCtrl::CModelViewerListCtrl()
	:	CListCtrl           (),
		_toggleRowBackground(true)
{}

//-----  ~CModelViewerListCtrl()  ---------------------------------------------
CModelViewerListCtrl::~CModelViewerListCtrl()
{}

//-----  SetEmptyMessage()  ---------------------------------------------------
void CModelViewerListCtrl::SetEmptyMessage(const string message)
{
	_emptyMessage = message;
}

//-----  OnPaint()  -----------------------------------------------------------
void CModelViewerListCtrl::OnPaint() 
{
	Default();

	//Draw empty list message
	if ((GetItemCount() <= 0) && !_emptyMessage.empty())
	{
		CHeaderCtrl*	pHC     (GetHeaderCtrl());
		CDC*			pDC     (GetDC());
		CRect			rc;
		COLORREF		crText  (RGB(0, 0, 255));
		int				nSavedDC(pDC->SaveDC());
		int				oldMode (0);

		GetWindowRect(&rc);
		ScreenToClient(&rc);

		if (pHC != NULL)
		{
			CRect	rcH;

			pHC->GetItemRect(0, &rcH);
			rc.top += rcH.bottom;
		}
		rc.top += 10;

		pDC->SetTextColor(crText);
		oldMode = pDC->SetBkMode(TRANSPARENT);
		
		pDC->SelectStockObject(ANSI_VAR_FONT);
		pDC->DrawText         (CString(_emptyMessage.c_str()), -1, rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
		pDC->SetBkMode        (oldMode);
		pDC->RestoreDC        (nSavedDC);
		ReleaseDC(pDC);
	}
}

//-----  OnCustomDraw()  ------------------------------------------------------
void CModelViewerListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW*		pLVCD(reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR));
	
	switch (pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		}

		case CDDS_ITEMPREPAINT:
		{
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		}

		case (CDDS_ITEMPREPAINT| CDDS_SUBITEM):
		{
			int				row(pLVCD->nmcd.dwItemSpec);
			int				col(pLVCD->iSubItem);
			DirectXMesh*	pMesh((DirectXMesh*) GetItemData(row));

			switch (col)
			{
				case 0:
				{
					SetItem(row, col, LVIF_IMAGE, NULL, (int) pMesh->GetRenderMode(), 0, 0, 0 );
					pLVCD->clrTextBk = (_toggleRowBackground && ((row % 2) == 1)) ? 0x00F4F4F4 : 0xFF000000;
					*pResult = CDRF_DODEFAULT;
					break;
				}

				case 1:
				{
					pLVCD->clrTextBk = pMesh->GetColorBackground();
					*pResult = CDRF_NEWFONT;
					break;
				}

				default:
				{
					pLVCD->clrTextBk = (_toggleRowBackground && ((row % 2) == 1)) ? 0x00F4F4F4 : 0xFF000000;
					*pResult = CDRF_NEWFONT;
					break;
				}
			}  //  switch (col)

			break;
		}

		default:
		{
			*pResult = CDRF_DODEFAULT;
		}
	}


}

//-----  OnNMClickLvNodes()  --------------------------------------------------
BOOL CModelViewerListCtrl::OnClickEx(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW*		pLV(reinterpret_cast<NM_LISTVIEW*>(pNMHDR));
	LVHITTESTINFO	hitTestInfo = { 0 };
	
	hitTestInfo.pt = pLV->ptAction;
	SubItemHitTest(&hitTestInfo);

	if (hitTestInfo.iItem >= 0)
	{
		DirectXMesh*	pMesh((DirectXMesh*) GetItemData(hitTestInfo.iItem));

		switch (hitTestInfo.iSubItem)
		{
			case 0:
			{
				pMesh->IncreaseRenderMode();
				SetItem(hitTestInfo.iItem, hitTestInfo.iSubItem, LVIF_IMAGE, NULL, (int) pMesh->GetRenderMode(), 0, 0, 0 );
				break;
			}

			case 1:
			{
				CHOOSECOLOR			cc = { 0 };
				static	COLORREF	cusCol[16];

				cc.lStructSize  = sizeof(cc);
				cc.hwndOwner    = GetSafeHwnd();
				cc.lpCustColors = cusCol;
				cc.rgbResult    = pMesh->GetColorBackground();
				cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
				if (ChooseColor(&cc))
				{
					pMesh->SetColorBackground(cc.rgbResult);
				}

				break;
			}
		}  //  switch (hitTestInfo.iSubItem)
	}

	*pResult = 0;

	return FALSE;
}

//-----  OnContextMenu()  -----------------------------------------------------
void CModelViewerListCtrl::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if ((GetItemCount() > 0) && (GetSelectedCount() > 0))
	{
		CMenu*	pPopUpMenu(NULL);
		CMenu	menuBar;

		menuBar.LoadMenu(IDR_MENU_MODELVIEW);

		pPopUpMenu = menuBar.GetSubMenu(0);
		pPopUpMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, this);
	}
}

//-----  OnMmvModelviewWireframecolor()  --------------------------------------
void CModelViewerListCtrl::OnMmvModelviewWireframecolor()
{
	if ((GetItemCount() > 0) && (GetSelectedCount() > 0))
	{
		CHOOSECOLOR			cc = { 0 };
		static	COLORREF	cusCol[16];
		COLORREF			tColor(0xFFFFFF);

		cc.lStructSize  = sizeof(cc);
		cc.hwndOwner    = GetSafeHwnd();
		cc.lpCustColors = cusCol;
		cc.rgbResult    = tColor;
		cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
		if (ChooseColor(&cc))
		{
			DirectXMesh*	pMesh (NULL);
			POSITION		posItm(GetFirstSelectedItemPosition());
			int				selIdx(-1);

			while (posItm != NULL)
			{
				selIdx = GetNextSelectedItem(posItm);
				if (selIdx >= 0)
				{
					pMesh = (DirectXMesh*) GetItemData(selIdx);
					if (pMesh != NULL)
					{
						pMesh->SetColorBackground(cc.rgbResult);
						Update(selIdx);
					}
				}  //  if (selIdx >= 0)
			}  //  while (posItm != NULL)
		}  //  if (ChooseColor(&cc))
	}  //  if ((GetItemCount() > 0) && (GetSelectedCount() > 0))
}

//-----  SetSelectedRenderMode()  ---------------------------------------------
void CModelViewerListCtrl::SetSelectedRenderMode(DirectXRenderMode renderMode)
{
	if ((GetItemCount() > 0) && (GetSelectedCount() > 0))
	{
		DirectXMesh*	pMesh (NULL);
		POSITION		posItm(GetFirstSelectedItemPosition());
		int				selIdx(-1);

		while (posItm != NULL)
		{
			selIdx = GetNextSelectedItem(posItm);
			if (selIdx >= 0)
			{
				pMesh = (DirectXMesh*) GetItemData(selIdx);
				if (pMesh != NULL)
				{
					pMesh->SetRenderMode(renderMode);
					SetItem(selIdx, 0, LVIF_IMAGE, NULL, (int) pMesh->GetRenderMode(), 0, 0, 0 );
				}
			}  //  if (selIdx >= 0)
		}  //  while (posItm != NULL)
	}  //  if ((GetItemCount() > 0) && (GetSelectedCount() > 0))
}

//-----  OnMmvDisplayNone()  --------------------------------------------------
void CModelViewerListCtrl::OnMmvDisplayNone()
{
	SetSelectedRenderMode(DXRM_NONE);
}

//-----  OnMmvDisplayNone()  --------------------------------------------------
void CModelViewerListCtrl::OnMmvDisplayWireframe()
{
	SetSelectedRenderMode(DXRM_WIREFRAME);
}

//-----  OnMmvDisplayNone()  --------------------------------------------------
void CModelViewerListCtrl::OnMmvDisplaySolid()
{
	SetSelectedRenderMode(DXRM_SOLID);
}

//-----  OnMmvDisplayNone()  --------------------------------------------------
void CModelViewerListCtrl::OnMmvDisplayTexture()
{
	SetSelectedRenderMode(DXRM_TEXTURE);
}

//-----  OnToolHitTest()  -----------------------------------------------------
int CModelViewerListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	int		row(0);
	int		col(0);
	RECT	cellrect = { 0 };

	if (!CellRectFromPoint(point, cellrect, row, col))
	{
		return -1;
	}

	pTI->hwnd     = m_hWnd;
	pTI->uId      = (UINT)((row<<10)+(col&0x3ff)+1);
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->rect     = cellrect;

	return pTI->uId;
}

//-----  CellRectFromPoint()  -------------------------------------------------
bool CModelViewerListCtrl::CellRectFromPoint(CPoint& point, RECT& cellrect, int& row, int& col) const
{
	CHeaderCtrl*	pHeader    ((CHeaderCtrl*) GetDlgItem(0));
	CRect			rect;
	int				columnCount(pHeader->GetItemCount());
	int				bottom     (0);

	// Get the top and bottom row visible
	row    = GetTopIndex();
	bottom = row + GetCountPerPage();

	if (bottom > GetItemCount())		bottom = GetItemCount();
	
	//  for each visible rows
	for(; row <= bottom; ++row)
	{
		GetItemRect(row, &rect, LVIR_BOUNDS);

		if (rect.PtInRect(point))
		{
			// Now find the column
			for (col=0; col < columnCount; ++col)
			{
				// Getting column width
				int		colwidth(GetColumnWidth(col));

				if ((point.x >= rect.left) && (point.x <= (rect.left + colwidth)))
				{
					RECT	rectClient = { 0 };

					GetClientRect(&rectClient);

					rect.right = rect.left + colwidth;
					if (rect.right > rectClient.right)		rect.right = rectClient.right;

					cellrect = rect;
					return true;

				}  //  if ((point.x >= rect.left) && (point.x <= (rect.left + colwidth)))

				rect.left += colwidth;

			}  //  for (col=0; col < columnCount; ++col)
		}  //  if (rect.PtInRect(point))
	}  //  for(; row <= bottom; ++row)

	return false;
}

//-----  OnMmvDisplayNone()  --------------------------------------------------
BOOL CModelViewerListCtrl::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	//  automated created tooltips
	if (pNMHDR->idFrom == 0)		return FALSE;

	int		row    (((pNMHDR->idFrom - 1) >> 10) & 0x003fffff);
	int		col    ((pNMHDR->idFrom - 1) & 0x000003ff);
	CString	tipText(GetItemText(row, col));

	//  set text for wide or ascii character set
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		_wcstombsz(((TOOLTIPTEXTA*)pNMHDR)->szText, tipText, 80);
	}
	else
	{
		lstrcpyn(((TOOLTIPTEXTW*)pNMHDR)->szText, tipText, 80);
	}

	//  mark final result OK
	*pResult = 0;

	return TRUE;
}

//-----  SetToggleRowBackground()  --------------------------------------------
bool CModelViewerListCtrl::SetToggleRowBackground(const bool toggle)
{
	bool	oldToggle(_toggleRowBackground);

	_toggleRowBackground = toggle;
	return oldToggle;
}

