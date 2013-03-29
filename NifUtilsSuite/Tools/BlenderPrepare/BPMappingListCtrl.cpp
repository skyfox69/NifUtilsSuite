/**
 *  file:   BPMappingListCtrl.cpp
 *  class:  CBPMappingListCtrl
 *
 *  BPMappingListCtrl for defining mapping between body parts
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "Tools\BlenderPrepare\BPMappingListCtrl.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "gen/enums.h"
#include "resource.h"

using namespace Niflib;

//-----  DEFINES  -------------------------------------------------------------
BEGIN_MESSAGE_MAP(CBPMappingListCtrl, CListCtrl)
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT_EX(NM_CLICK,               &CBPMappingListCtrl::OnClickEx)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,             &CBPMappingListCtrl::OnCustomDraw)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT,          &CBPMappingListCtrl::OnEndlabeledit)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CBPMappingListCtrl::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CBPMappingListCtrl::OnToolTipText)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CBPMappingComboBox, CComboBox)
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, &CBPMappingComboBox::OnCloseup)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----  CBPMappingListCtrl()  ------------------------------------------------
CBPMappingListCtrl::CBPMappingListCtrl()
	:	CListCtrl           (),
		_toggleRowBackground(true)
{}

//-----  ~CBPMappingListCtrl()  -----------------------------------------------
CBPMappingListCtrl::~CBPMappingListCtrl()
{}

//-----  SetEmptyMessage()  ---------------------------------------------------
void CBPMappingListCtrl::SetEmptyMessage(const string message)
{
	_emptyMessage = message;
}

//-----  OnPaint()  -----------------------------------------------------------
void CBPMappingListCtrl::OnPaint() 
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
void CBPMappingListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
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
			pLVCD->clrTextBk = (_toggleRowBackground && ((pLVCD->nmcd.dwItemSpec % 2) == 1)) ? 0x00F4F4F4 : 0xFF000000;
			*pResult = CDRF_NEWFONT;
			break;
		}

		default:
		{
			*pResult = CDRF_DODEFAULT;
		}
	}
}

//-----  OnClickEx()  ---------------------------------------------------------
BOOL CBPMappingListCtrl::OnClickEx(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW*		pLV(reinterpret_cast<NM_LISTVIEW*>(pNMHDR));
	LVHITTESTINFO	hitTestInfo = { 0 };
	
	hitTestInfo.pt = pLV->ptAction;
	SubItemHitTest(&hitTestInfo);

	if (hitTestInfo.iItem >= 0)
	{
		if (hitTestInfo.iSubItem == 1)
		{
			CRect			rect;
			CRect			rectCl;
			int				offset(0);
			unsigned short	first (GetItemData(hitTestInfo.iItem) & 0x0000ffff);
			unsigned short	second((GetItemData(hitTestInfo.iItem) >> 16) & 0x0000ffff);

			if (EnsureVisible(hitTestInfo.iItem, TRUE))
			{
				GetSubItemRect(hitTestInfo.iItem, hitTestInfo.iSubItem, LVIR_BOUNDS, rect);
				GetClientRect (rectCl);

				if (((offset + rect.left) < 0) || ((offset + rect.right) > rectCl.right))
				{
					CSize	size(offset + rect.left, 0);

					Scroll(size);
					rect.left -= size.cx;
				}

				rect.left += offset;
				rect.right = (int) (1.6 * (rect.left + GetColumnWidth(hitTestInfo.iSubItem)));
				//if (rect.right > rectCl.right)	rect.right = rectCl.right;
				rect.bottom += 12 * rect.Height();

				CBPMappingComboBox*		pCCombo(new CBPMappingComboBox(hitTestInfo.iItem, hitTestInfo.iSubItem, second));

				pCCombo->Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL | CBS_NOINTEGRALHEIGHT, rect, this, 3212);
				pCCombo->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
				pCCombo->ShowDropDown();

			}  //  if (EnsureVisible(hitTestInfo.iItem, TRUE))
		}  //  if (hitTestInfo.iSubItem == 1)
	}  //  if (hitTestInfo.iItem >= 0)

	*pResult = 0;

	return FALSE;
}

//-----  OnEndlabeledit()  ----------------------------------------------------
void CBPMappingListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO*	plvDispInfo((LV_DISPINFO*) pNMHDR);
 	LV_ITEM*		plvItem    (&plvDispInfo->item);

	if ((plvItem->iItem != -1) && (plvItem->pszText))
	{
		SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
		SetItemData(plvItem->iItem, plvItem->lParam);
		
		// this will invoke an ItemChanged handler in parent
		if (plvItem->iSubItem != 0)
		{
			CString	tString(GetItemText(plvItem->iItem, 0));
			SetItemText( plvItem->iItem, 0, tString.GetString());
		}
	}

	*pResult = 0;
}

//-----  OnToolHitTest()  -----------------------------------------------------
int CBPMappingListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
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
bool CBPMappingListCtrl::CellRectFromPoint(CPoint& point, RECT& cellrect, int& row, int& col) const
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

//-----  OnToolTipText()  -----------------------------------------------------
BOOL CBPMappingListCtrl::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	//  automated created tooltips
	if (pNMHDR->idFrom == 0)		return FALSE;

	int				row    (((pNMHDR->idFrom - 1) >> 10) & 0x003fffff);
	int				col    ((pNMHDR->idFrom - 1) & 0x000003ff);
	CString			tipText(GetItemText(row, col));

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
bool CBPMappingListCtrl::SetToggleRowBackground(const bool toggle)
{
	bool	oldToggle(_toggleRowBackground);

	_toggleRowBackground = toggle;
	return oldToggle;
}



//-----  CBPMappingComboBox()  ------------------------------------------------
CBPMappingComboBox::CBPMappingComboBox(const unsigned short row, const unsigned short col, const unsigned short initial)
	:	CComboBox     (),
		_row          (row),
		_col          (col),
		_initial      (initial),
		_pressedEscape(false)
{}

//-----  ~CBPMappingComboBox()  -----------------------------------------------
CBPMappingComboBox::~CBPMappingComboBox()
{}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL CBPMappingComboBox::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	   )
	{
		::TranslateMessage(pMsg);
		::DispatchMessage (pMsg);
		return TRUE;
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

//-----  OnNcDestroy()  -------------------------------------------------------
void CBPMappingComboBox::OnNcDestroy() 
{
	CComboBox::OnNcDestroy();
	delete this;
}

//-----  OnChar()  ------------------------------------------------------------
void CBPMappingComboBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_ESCAPE) || (nChar == VK_RETURN))
	{
		_pressedEscape = (nChar == VK_ESCAPE);
		GetParent()->SetFocus();
		return;
	}

	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

//-----  OnKillFocus()  -------------------------------------------------------
void CBPMappingComboBox::OnKillFocus(CWnd* pNewWnd) 
{
	CString			str;	
	LV_DISPINFO		lvDispinfo = { 0 };
	int				nIndex(GetCurSel());

	CComboBox::OnKillFocus(pNewWnd);

	GetWindowText(str);

	// Send Notification to parent of ListView ctrl	
	lvDispinfo.hdr.hwndFrom    = GetParent()->m_hWnd;
	lvDispinfo.hdr.idFrom      = GetDlgCtrlID();
	lvDispinfo.hdr.code        = LVN_ENDLABELEDIT;
	lvDispinfo.item.mask       = LVIF_TEXT | LVIF_PARAM;
	lvDispinfo.item.iItem      = _row;
	lvDispinfo.item.iSubItem   = _col;
	lvDispinfo.item.pszText    = _pressedEscape ? NULL : LPTSTR((LPCTSTR)str);
	lvDispinfo.item.cchTextMax = str.GetLength();
	lvDispinfo.item.lParam     = (GetItemData(GetCurSel()) << 16) | _row;

	int	test = (GetItemData(GetCurSel()) << 16) | _row;

	if (nIndex != CB_ERR)
	{
		GetParent()->GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&lvDispinfo);
	}
	PostMessage(WM_CLOSE);
}

//-----  OnCloseup()  ---------------------------------------------------------
void CBPMappingComboBox::OnCloseup() 
{
	GetParent()->SetFocus();
}

//-----  OnCreate()  ----------------------------------------------------------
int CBPMappingComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)	return -1;
	
	SetFont(GetParent()->GetFont());

	map<unsigned short, string>		bpMap(NifUtlMaterialList::getInstance()->getBodyPartMap());
	int								idx  (0);

	for (auto pIter=bpMap.begin(), pEnd=bpMap.end(); pIter != pEnd; ++pIter)
	{
		if (pIter->first <= BP_BRAIN)	continue;

		AddString(CString(pIter->second.c_str()));
		SetItemData(idx++, pIter->first);
	}
	SelectString(-1, CString(bpMap[_initial].c_str()));

	SetFocus();
	return 0;
}

//-----  OnSize()  ------------------------------------------------------------
void CBPMappingComboBox::OnSize(UINT nType, int cx, int cy) 
{
	CComboBox::OnSize(nType, cx, cy);
}