/**
 *  file:   BCTabBarCtrl.cpp
 *  class:  BCTabBarCtrl
 *
 *  ControlBar simulating TabCtrlBar
 *    inspired by CTabBarCtrl from www.CodeGuru.com
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include <afxpriv.h>
#include "Common\GUI\BCTabBarCtrl.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\IfcBroadcastObject.h"

//-----  CLASS  ---------------------------------------------------------------
typedef struct _BARTCITEMW
{
	_BARTCITEMW() 
		: lParam(NULL)
		, uiID(0)
		, pWnd(NULL)
	{
		ZeroMemory(&hdr, sizeof(TCITEMHEADERW));
	}
	
	_BARTCITEMW(TCITEMW& tcItem) 
	{
		hdr.cchTextMax = tcItem.cchTextMax; 
		hdr.iImage = tcItem.iImage; 
		hdr.lpReserved1 = tcItem.dwState; 
		hdr.lpReserved2 = tcItem.dwStateMask; 
		hdr.mask = tcItem.mask; 
		hdr.pszText = tcItem.pszText; 
		lParam = tcItem.lParam;
		
	}
	
	_BARTCITEMW(_BARTCITEMW& tcBarItem) 
	{
		*this = tcBarItem;
	}
	
	_BARTCITEMW& operator =(const _BARTCITEMW& tcBarItem)
	{
		hdr.cchTextMax = tcBarItem.hdr.cchTextMax; 
		hdr.iImage = tcBarItem.hdr.iImage; 
		hdr.lpReserved1 = tcBarItem.hdr.lpReserved1; 
		hdr.lpReserved2 = tcBarItem.hdr.lpReserved2; 
		hdr.mask = tcBarItem.hdr.mask; 
		hdr.pszText = tcBarItem.hdr.pszText; 
		
		lParam = tcBarItem.lParam;
		uiID = tcBarItem.uiID;
		pWnd = tcBarItem.pWnd;
		
		return *this;
	}
	
	TCITEMHEADERW hdr;
	LPARAM lParam;
	int uiID;
	CWnd* pWnd;
	
} BARTCITEMW, FAR *LPBARTCITEMW;

//-----  DEFINES  -------------------------------------------------------------
#define NC_FUDGE			3
#define CLIENT_FUDGE		10
#define FIRST_TAB_VIEW_ID	AFX_IDW_PANE_FIRST + 1
#define TCIF_ALL_BAR		(TCIF_TEXT | TCIF_IMAGE | TCIF_RTLREADING | TCIF_PARAM | TCIF_STATE)
#define	BARTCITEM			BARTCITEMW
#define	LPBARTCITEM			LPBARTCITEMW

IMPLEMENT_DYNAMIC(BCTabBarCtrl, CControlBar)

BEGIN_MESSAGE_MAP(BCTabBarCtrl, CControlBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_MESSAGE(WM_INITIALUPDATE, OnInitialUpdate)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	ON_MESSAGE(TCM_GETROWCOUNT, OnDoNothing)
	ON_MESSAGE(TCM_SETEXTENDEDSTYLE, OnDoNothing)
	ON_MESSAGE(TCM_GETEXTENDEDSTYLE, OnDoNothing)
END_MESSAGE_MAP()

//-----  BCTabBarCtrl()  ------------------------------------------------------
BCTabBarCtrl::BCTabBarCtrl() 
	: m_pParentFrame(NULL)
	, m_bSendInitialUpdate(FALSE)
	, m_cyDefault(0)
{}

//-----  ~BCTabBarCtrl()  -----------------------------------------------------
BCTabBarCtrl::~BCTabBarCtrl()
{
	DestroyWindow();
}

//-----  OnInitialUpdate()  ---------------------------------------------------
LRESULT BCTabBarCtrl::OnInitialUpdate(WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	
	SetFont(CFont::FromHandle(hFont));
	
	ASSERT(m_pParentFrame);
	ASSERT(IsWindow(*m_pParentFrame));
	
	int iSel = GetCurSel();
	ASSERT(iSel > -1);

	CWnd* pView = GetViewFromItem(iSel);
	
	pView->SetDlgCtrlID(AFX_IDW_PANE_FIRST);

	m_pParentFrame->RecalcLayout();

	m_pParentFrame->SetActiveView((CView*)pView);
	
	pView = m_pParentFrame->GetActiveView();

	m_bSendInitialUpdate = TRUE;

	CRect rectItem;
	GetItemRect(0, rectItem);

	m_cyDefault = rectItem.Height() + CLIENT_FUDGE;

	return 0;
}

//-----  OnSizeParent()  ------------------------------------------------------
LRESULT BCTabBarCtrl::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	
	CRect rectItem, rectClient(lpLayout->rect);
	
	GetItemRect(0, rectItem);
	
	rectClient.bottom = rectClient.top + rectItem.Height() + CLIENT_FUDGE;
	
	lpLayout->rect.top = rectClient.bottom;
	lpLayout->sizeTotal = rectClient.Size();
	
	
	if (lpLayout->hDWP != NULL)
		AfxRepositionWindow(lpLayout, m_hWnd, &rectClient);
	
	return 0;
}



//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	lpncsp->rgrc[0].right += 5;
	lpncsp->rgrc[0].left -= 1;
	CControlBar::OnNcCalcSize(bCalcValidRects, lpncsp);
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnNcPaint() 
{
	CRect rectCLient, rectNC;
	GetWindowRect(rectNC);
	
	rectNC.OffsetRect(-rectNC.left, -rectNC.top);
	
	CWindowDC dc(this);
	CPen pen;
	
	DWORD dwSysGray = GetSysColor(COLOR_BTNFACE);
	
	dc.DrawEdge(rectNC, EDGE_ETCHED, BF_TOP);
	
	rectNC.DeflateRect(0, 2, 0, 0);
	
	CBrush brush(dwSysGray);
	dc.FrameRect(rectNC, &brush);
	
	for(int iIndx = 0; iIndx < 2; iIndx++)
	{
		rectNC.DeflateRect(1, 1, 1, 1);
		dc.FrameRect(rectNC, &brush);
	}
	
	rectNC.DeflateRect(0, 1, 0, 2);
	pen.DeleteObject();
	
	pen.CreatePen(PS_GEOMETRIC | PS_SOLID, 1, dwSysGray);
	dc.SelectObject(&pen);
	
	dc.MoveTo(rectNC.left, rectNC.top);
	dc.LineTo(rectNC.right, rectNC.top);
}


//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnDestroy() 
{
	HIMAGELIST h = (HIMAGELIST)SendMessage(TCM_GETIMAGELIST);
	if (CImageList::FromHandlePermanent(h) != NULL)
		SendMessage(TCM_SETIMAGELIST, NULL, NULL);
	
	CControlBar::OnDestroy();
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::Create(CFrameWnd* pParentWnd)
{
	ASSERT_VALID(pParentWnd);   // must have a parent
	m_pParentFrame = pParentWnd;

	m_dwStyle = NULL;

	if (!CWnd::Create(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE/* | WS_BORDER*/| WS_DLGFRAME, CRect(0, 0, 0, 0), pParentWnd, AFX_IDC_TAB_CONTROL))
	{
		TRACE("Failed to create tab bar!\n");
		return FALSE;
	}

	LRESULT lres = ::SendMessage(m_hWnd, TCM_SETITEMEXTRA, 3 * sizeof(DWORD), 0);

	return TRUE;
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message != WM_DRAWITEM)
		return CControlBar::OnChildNotify(message, wParam, lParam, pResult);
								   /**** JohnCz ***/
	ASSERT(pResult == NULL);       // no return needed
	UNUSED(pResult); 
	
	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return TRUE;
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	/**** JohnCz ***/
	// must override for overdrawn
	ASSERT(FALSE);  
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::GetItem(int iItem, TCITEM* pTabCtrlItem) const
{ 
	ASSERT(::IsWindow(m_hWnd));

	BCTabBarCtrl *pBar = (BCTabBarCtrl*)this;
	return pBar->QueryInfo(TCM_GETITEM, iItem, pTabCtrlItem); 
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::GetItem(int iItem, TCITEMHEADER* pBarItem) const
{ 
	ASSERT(::IsWindow(m_hWnd));
	BARTCITEM *pItem = (BARTCITEM*)pBarItem;

	return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, iItem, (LPARAM)pBarItem); 
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::SetItem(int iItem, TCITEM* pTabCtrlItem)
{ 
	ASSERT(::IsWindow(m_hWnd)); 
	return SetInfo(TCM_SETITEM, iItem, pTabCtrlItem); 
}

//-----  OnSizeParent()  ------------------------------------------------------
DWORD BCTabBarCtrl::GetItemState(int iItem, DWORD dwMask) const
{
	ASSERT(::IsWindow(m_hWnd));
	
	BARTCITEM item;
	ZeroMemory(&item, sizeof(BARTCITEM));
	item.hdr.mask = TCIF_STATE;
	item.hdr.lpReserved1 = dwMask;

	VERIFY(::SendMessage(m_hWnd, TCM_GETITEM, (WPARAM)iItem, (LPARAM)&item));
	
	return item.hdr.lpReserved1;
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::SetItemState(int iItem, DWORD dwMask, DWORD dwState)
{
	ASSERT(::IsWindow(m_hWnd));
	
	BARTCITEM item;
	ZeroMemory(&item, sizeof(BARTCITEM));
	item.hdr.mask = TCIF_STATE;
	item.hdr.lpReserved1 = dwMask;
	
	return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, (WPARAM)iItem, (LPARAM)&item);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::SetCurSel(int iItem)
{ 
	ASSERT(::IsWindow(m_hWnd)); 

	int result = ::SendMessage(m_hWnd, TCM_SETCURSEL, iItem, 0L); 

	LRESULT	tRes;

	OnSelchange(NULL, &tRes);

	return result;
}

//////////////////////////////////////////////////////////////////////////
/**** JohnCz ***/
// InsertView overloads
//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::InsertView(CRuntimeClass *pViewClass, int iItem, LPCTSTR lpszItem, CCreateContext* pContext /*= NULL*/)
{ 
	ASSERT(::IsWindow(m_hWnd)); 
	
	return InsertView(pViewClass, TCIF_TEXT, iItem, lpszItem, 0, 0, 0, 0, pContext); 
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::InsertView(CRuntimeClass *pViewClass, int iItem, LPCTSTR lpszItem, 
							int iImage, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return InsertView(pViewClass, TCIF_TEXT | TCIF_IMAGE, iItem, lpszItem, iImage, 0, 0, 0, pContext); 
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::InsertView(CRuntimeClass *pViewClass, UINT uiMask, int iItem, 
							 LPCTSTR lpszItem, int iImage, LPARAM lParam, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));

	return InsertView(pViewClass, TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, iItem, lpszItem, 
						iItem, lParam, 0, 0, pContext);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::InsertView(CRuntimeClass *pViewClass, UINT uiMask, int iItem, 
							 LPCTSTR lpszItem, int iImage, LPARAM lParam, DWORD dwState, DWORD dwStateMask, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	
	TCITEM item;
	ZeroMemory(&item, sizeof(TCITEM));
	item.mask = uiMask;
	item.iImage = iImage;
	item.lParam = lParam;
	item.pszText = (LPTSTR)lpszItem;
	item.dwState = dwState;
	item.dwStateMask = dwStateMask;
	
	return InsertView(pViewClass, iItem, &item, pContext);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::InsertView(CRuntimeClass *pViewClass, int iItem, TCITEM* pTabCtrlItem, CCreateContext* pContext /*= NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	BARTCITEM item(*pTabCtrlItem);

	if(!CreateView(pViewClass, (TCITEMHEADER*)&item, pContext))
	{
		return FALSE;
	}
	
	return ::SendMessage(m_hWnd, TCM_INSERTITEM, iItem, (LPARAM)&item);
}
//////////////////////////////////////////////////////////////////////////
/**** JohnCz ***/
// AddView overloads
//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, CCreateContext* pContext/* = NULL*/)
{ 
	ASSERT(::IsWindow(m_hWnd));
	
	return AddView(pViewClass, TCIF_TEXT, lpszItem, 0, 0, 0, 0, pContext);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, int iImage, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	
	return AddView(pViewClass, TCIF_TEXT | TCIF_IMAGE, lpszItem, iImage, 0, 0, 0, pContext); 
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, 
						  int iImage, LPARAM lParam, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	
	return AddView(pViewClass, TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, lpszItem, iImage, lParam, 0, 0, pContext);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::AddView(CRuntimeClass *pViewClass, UINT uiMask, LPCTSTR lpszItem, 
						  int iImage, LPARAM lParam, DWORD dwState, DWORD dwStateMask, CCreateContext* pContext/* = NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));

	int iCount = GetItemCount();
	return InsertView(pViewClass, uiMask, iCount, lpszItem, iImage, lParam, dwState, 
						dwStateMask, pContext);
}

//-----  OnSizeParent()  ------------------------------------------------------
int BCTabBarCtrl::AddView(CRuntimeClass *pViewClass, TCITEM* pTabCtrlItem, CCreateContext* pContext /*= NULL*/)
{
	ASSERT(::IsWindow(m_hWnd));
	
	int iCount = GetItemCount();
	return InsertView(pViewClass, iCount, pTabCtrlItem, pContext); 
}
/**** JohnCz ***/
// AddView overloads END
//////////////////////////////////////////////////////////////////////////

/**** JohnCz ***/
// RemoveView contains certain guards to tell whenever attempted remove is not valid
//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::RemoveView(int iItem)
{ 
	ASSERT(::IsWindow(m_hWnd)); 
	
	int iCount = GetItemCount();

	/**** JohnCz ***/
	// if this fires, it is an attempt to remove view that does not exist or the last view
	ASSERT((iCount > iItem) || (iCount > 1) || (iItem > 0));
	
	// return FALSE to continue working
	if((iCount <= iItem) || (iCount < 2))
	{
		return FALSE;
	}
	
	BARTCITEM item;
	item.hdr.mask = TCIF_PARAM;
	VERIFY(GetItem(iItem, (TCITEMHEADER*)&item));

	ASSERT(item.pWnd);
	ASSERT(IsWindow(*item.pWnd));

	int iTabItem = -1;
	CWnd *pNewView = NULL;

	CWnd *pRemoveView = item.pWnd; // just to simplify 

	int iSetCurr = iItem + 1;

	/**** JohnCz ***/
	// this will set active tab either after or if tab does not exist before removed one
	if(AFX_IDW_PANE_FIRST == pRemoveView->GetDlgCtrlID())	//removing active?
	{
		/**** JohnCz ***/
		// to set active tab after removed
		pNewView = GetViewFromItem(iSetCurr); 

		// is it last tab? if so set tab before
		if(NULL == pNewView) 
		{
			iSetCurr = iItem - 1;
			// attempt to set active tab before	removved
			pNewView = GetViewFromItem(iSetCurr); 
		}
	}

	m_mapUsedID.RemoveKey(item.uiID);
	pRemoveView->DestroyWindow();

	/**** JohnCz ***/
	// we just removed active, set new view.
	if(pNewView) 
	{
		pNewView->SetDlgCtrlID(AFX_IDW_PANE_FIRST);
		pNewView->ShowWindow(SW_SHOW);
		SetCurSel(iSetCurr);
		m_pParentFrame->RecalcLayout();
		pNewView->SetFocus();
	}

	return (BOOL)::SendMessage(m_hWnd, TCM_DELETEITEM, iItem, 0L); 
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::RemoveAllViews()
{ 
	ASSERT(::IsWindow(m_hWnd)); 
	int iCount = GetItemCount();
	
	ASSERT(iCount > 1);	// 1 view must be left

	if(iCount < 2)
	{
		return FALSE;
	}

	BARTCITEM itemSave;
	BARTCITEM item;

	CString csBuffer;
	CString csItemText;

	/**** JohnCz ***/
	// kill all views except current
	for(int iIndx = 0; iIndx < iCount; iIndx++)
	{
		item.hdr.mask = TCIF_ALL_BAR;
		
		LPTSTR pBuf = csBuffer.GetBufferSetLength(MAX_PATH);
		item.hdr.cchTextMax = MAX_PATH;
		item.hdr.pszText = pBuf;
		
		GetItem(iIndx, (TCITEMHEADER*)&item);
		
		csBuffer.ReleaseBuffer();

		ASSERT(item.pWnd);

		if(AFX_IDW_PANE_FIRST == item.pWnd->GetDlgCtrlID())
		{
			/**** JohnCz ***/
			// csBuffer	has different pointer on each iteration
			// this will just copy pointer of csBuffer string to csItemText data member 
			// so it will point to the same string in itemSave 
			csItemText = csBuffer;

			/**** JohnCz ***/
			// save for default view, do not destroy
			itemSave = item;
			continue;
		}
		item.pWnd->DestroyWindow();
	}

	/**** JohnCz ***/
	// delete all items
	BOOL bResult = ::SendMessage(m_hWnd, TCM_DELETEALLITEMS, 0, 0); 

	if(!bResult)
	{
		ASSERT(bResult); // something is wrong not right TCM_DELETEALLITEMS failed
		return bResult;
	}

	m_mapUsedID.RemoveAll();

	/**** JohnCz ***/
	// insert first item to manage current view
	bResult &= (-1 < ::SendMessage(m_hWnd, TCM_INSERTITEM, 0, (LPARAM)&itemSave));
	
	m_mapUsedID.SetAt(itemSave.uiID, itemSave.uiID);

	/**** JohnCz ***/
	// TCM_INSERTITEM ignores state, we have to set item to set the same state
	itemSave.hdr.mask = TCIF_STATE;
	bResult &= ::SendMessage(m_hWnd, TCM_SETITEM, 0, (LPARAM)&itemSave);

	ASSERT(bResult);
	m_pParentFrame->RecalcLayout();

	return bResult;
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnPaint() 
{
	/**** JohnCz ***/
	// do default tab view painting, not a control bar painting
	Default();
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::EnableDocking(DWORD dwDockStyle)
{
	/**** JohnCz ***/
	// docking not supported 	
	ASSERT(FALSE); 
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int iSel = GetCurSel();

	ASSERT(m_pParentFrame);
	ASSERT(IsWindow(*m_pParentFrame));
	
	/**** JohnCz ***/
	// get current view
	CWnd *pViewCurrent = m_pParentFrame->GetDlgItem(AFX_IDW_PANE_FIRST);
	
	if (pViewCurrent != NULL)
	{
		ASSERT(pViewCurrent);
		ASSERT(IsWindow(*pViewCurrent));

		/**** JohnCz ***/
		// retrieve original ID
		UINT uiID = GetViewTabID(pViewCurrent);

		pViewCurrent->SetDlgCtrlID(uiID);
		pViewCurrent->ShowWindow(SW_HIDE);
	}

	/**** JohnCz ***/
	// get View for selected tab
	CWnd *pNewViewWnd = GetViewFromItem(iSel);
	ASSERT(pNewViewWnd);
	ASSERT(IsWindow(*pNewViewWnd));
	
	/**** JohnCz ***/
	// make view current
	pNewViewWnd->SetDlgCtrlID(AFX_IDW_PANE_FIRST);
	pNewViewWnd->ShowWindow(SW_SHOW);

	m_pParentFrame->RecalcLayout();
	((CNifUtilsSuiteFrame*) m_pParentFrame)->SetActiveView((CView*)pNewViewWnd);

	*pResult = 0;
}

//-----  OnSizeParent()  ------------------------------------------------------
void BCTabBarCtrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	Default();	
}

//-----  OnSizeParent()  ------------------------------------------------------
BOOL BCTabBarCtrl::CreateView(CRuntimeClass *pViewClass, TCITEMHEADER* pBarItem, CCreateContext *pContext)
{

	UINT uiNewID = FIRST_TAB_VIEW_ID; 
	UINT uiFound = 0;
	BARTCITEM* pItem = (BARTCITEM*)pBarItem;

	/**** JohnCz ***/
	// find valid ID >= FIRST_TAB_VIEW_ID
	while(m_mapUsedID.Lookup(uiNewID, uiFound))
	{
		uiNewID++;
	}

	/**** JohnCz ***/
	// Create View
	CCreateContext cntxt;
	cntxt.m_pCurrentDoc = pContext->m_pCurrentDoc;
	cntxt.m_pCurrentFrame = pContext->m_pCurrentFrame;
	cntxt.m_pLastView = pContext->m_pLastView;
	cntxt.m_pNewDocTemplate = pContext->m_pNewDocTemplate;
	cntxt.m_pNewViewClass = pViewClass;

	ASSERT(m_pParentFrame);
	ASSERT(IsWindow(*m_pParentFrame));

	CWnd *pWnd = m_pParentFrame->CreateView(&cntxt, uiNewID);
	
	if(NULL == pWnd)
	{
		TRACE("Failed to create view.");
		return FALSE;
	}
	pItem->hdr.mask |= TCIF_PARAM;
	pItem->pWnd = pWnd;
	pItem->uiID = uiNewID;

	m_mapUsedID.SetAt(uiNewID, uiNewID);

	if(m_bSendInitialUpdate)
	{
		pWnd->SendMessage(WM_INITIALUPDATE);
	}
	return TRUE;
}

//-----  GetViewTabID()  ------------------------------------------------------
UINT BCTabBarCtrl::GetViewTabID(CWnd* pWnd)
{
	CWnd*	pView   (NULL);
	int		iTabItem(-1);
	int		iCount  (GetItemCount());

	for(int iIndx(0); iIndx < iCount; ++iIndx)
	{
		BARTCITEM	barItem;

		barItem.hdr.mask = TCIF_PARAM;
		GetItem(iIndx, (TCITEMHEADER*)&barItem);

		if(barItem.pWnd == pWnd)
		{
			ASSERT(barItem.uiID > AFX_IDW_PANE_FIRST); 
			return	barItem.uiID;
		}
	}

	ASSERT(FALSE);
	return 0;
}

//-----  GetViewFromItem()  ---------------------------------------------------
CWnd* BCTabBarCtrl::GetViewFromItem(int iItem)
{
	BARTCITEM	barItem;

	barItem.hdr.mask = TCIF_PARAM;
	
	if (!GetItem(iItem, (TCITEMHEADER*)&barItem))
	{
		return NULL;
	}

	ASSERT(barItem.uiID > AFX_IDW_PANE_FIRST);
	return barItem.pWnd;
}

//-----  QueryInfo()  ---------------------------------------------------------
int BCTabBarCtrl::QueryInfo(UINT uiType, int iItem, TCITEMHEADER* pBarItem)
{
	return ::SendMessage(m_hWnd, uiType, iItem, (LPARAM)pBarItem);
}

//-----  QueryInfo()  ---------------------------------------------------------
int BCTabBarCtrl::QueryInfo(UINT uiType, int iItem, TCITEM *pTabCtrlItem)
{
	BARTCITEM	BarItem(*pTabCtrlItem);

	return QueryInfo(uiType, iItem, (TCITEMHEADER*)&BarItem);
}

//-----  SetInfo()  -----------------------------------------------------------
int BCTabBarCtrl::SetInfo(UINT uiType, int iItem, TCITEMHEADER* pBarItem)
{
	return ::SendMessage(m_hWnd, uiType, iItem, (LPARAM)pBarItem); 
}

//-----  SetInfo()  -----------------------------------------------------------
int BCTabBarCtrl::SetInfo(UINT uiType, int iItem, TCITEM *pTabCtrlItem)
{
	BARTCITEM	BarItem(*pTabCtrlItem);
	BOOL		bParam ((pTabCtrlItem->mask & TCIF_PARAM) == TCIF_PARAM);
	
	if (bParam)
	{
		BARTCITEM barItemQuerry;

		barItemQuerry.hdr.mask = TCIF_PARAM;
		
		VERIFY(GetItem(iItem, (TCITEMHEADER*)&barItemQuerry));
		
		BarItem.uiID = barItemQuerry.uiID;
		BarItem.pWnd = barItemQuerry.pWnd;
	}

	return SetInfo(uiType, iItem, (TCITEMHEADER*)&BarItem);
}

//-----  SetItemData()  -------------------------------------------------------
BOOL BCTabBarCtrl::SetItemData(int iItem, LPARAM lParam)
{
	BARTCITEM item;

	item.hdr.mask = TCIF_PARAM;
	item.lParam   = lParam;

	return (!QueryInfo(TCM_GETITEM, iItem, (TCITEMHEADER*)&item)) ? FALSE : SetInfo(TCM_SETITEM, iItem, (TCITEMHEADER*)&item);
}

//-----  SetItemImage()  ------------------------------------------------------
int BCTabBarCtrl::SetItemImage(int iItem, UINT uiImage)
{
	BARTCITEM item;

	item.hdr.mask   = TCIF_IMAGE;
	item.hdr.iImage = uiImage;

	return SetInfo(TCM_SETITEM, iItem, (TCITEMHEADER*)&item);
}

//-----  GetItemText()  -------------------------------------------------------
CString BCTabBarCtrl::GetItemText(int iItem)
{
	BARTCITEM	barItem;
	TCHAR		cBuffer[1000];

	if (iItem < 0)		iItem = GetCurSel();

	barItem.hdr.mask       = TCIF_TEXT;
	barItem.hdr.pszText    = cBuffer;
	barItem.hdr.cchTextMax = 1000;
	QueryInfo(TCM_GETITEM, iItem, (TCITEMHEADER*)&barItem);

	return cBuffer;
}

//-----  SetItemText()  -------------------------------------------------------
BOOL BCTabBarCtrl::SetItemText(int iItem, CString csText)
{
	BARTCITEM item;

	item.hdr.mask    = TCIF_TEXT;
	item.hdr.pszText = (LPTSTR)(LPCTSTR)csText;
	
	return SetInfo(TCM_SETITEM, iItem, (TCITEMHEADER*)&item);
}

//-----  GetItemView()  -------------------------------------------------------
CWnd* BCTabBarCtrl::GetItemView(int iItem)
{
	return GetViewFromItem(iItem);
}

//-----  OnDoNothing()  -------------------------------------------------------
LRESULT BCTabBarCtrl::OnDoNothing(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL BCTabBarCtrl::BroadcastEvent(WORD event, void* pParameter)
{
	CWnd*	pView  (NULL);
	int		cntItem(GetItemCount());

	for(int idx(0); idx < cntItem; ++idx)
	{
		BARTCITEM	barItem;

		barItem.hdr.mask = TCIF_PARAM;
		GetItem(idx, (TCITEMHEADER*)&barItem);

		if (dynamic_cast<IfcBroadcastObject*>(barItem.pWnd) != NULL)
		{
			dynamic_cast<IfcBroadcastObject*>(barItem.pWnd)->BroadcastEvent(event, pParameter);
		}
	}

	return TRUE;
}

