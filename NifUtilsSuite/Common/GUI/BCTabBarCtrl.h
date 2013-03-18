/**
 *  file:   BCTabBarCtrl.h
 *  class:  BCTabBarCtrl
 *
 *  ControlBar simulating TabCtrlBar
 *    inspired by CTabBarCtrl from www.CodeGuru.com
 */

#pragma once

//-----  CLASS  ---------------------------------------------------------------
class BCTabBarCtrl : public CControlBar
{
	private:
		CFrameWnd*						m_pParentFrame;
		int								m_cyDefault;
		BOOL							m_bSendInitialUpdate;
		CMap<UINT, UINT, UINT, UINT>	m_mapUsedID;

				int				SetInfo  (UINT uiType, int iItem, TCITEM *pTabCtrlItem);
				int				SetInfo  (UINT uiType, int iItem, TCITEMHEADER* pBarItem);
				int				QueryInfo(UINT uiType, int iItem, TCITEM *pTabCtrlItem);
				int				QueryInfo(UINT uiType, int iItem, TCITEMHEADER* pBarItem);

				void			EnableDocking(DWORD dwDockStyle);
				BOOL			GetItem(int iItem, TCITEMHEADER* pBarItem) const;
				UINT			GetViewTabID(CWnd* pWnd);
				CWnd*			GetViewFromItem(int iItem);
				BOOL			CreateView(CRuntimeClass *pViewClass, TCITEMHEADER* pBarItem, CCreateContext *pContext);

		virtual	void			DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual	void			OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	protected:
		afx_msg	void			OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
		afx_msg	void			OnNcPaint();
		afx_msg	void			OnDestroy();
		afx_msg	void			OnPaint();
		afx_msg	LRESULT			OnSizeParent(WPARAM wParam, LPARAM lParam);
		afx_msg	LRESULT			OnInitialUpdate(WPARAM wParam, LPARAM lParam);
		afx_msg	void			OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg	LRESULT			OnDoNothing(WPARAM wParam, LPARAM lParam);

		DECLARE_DYNAMIC(BCTabBarCtrl)
		DECLARE_MESSAGE_MAP()

	public:
								BCTabBarCtrl();
		virtual					~BCTabBarCtrl();

				BOOL			Create(CFrameWnd* pParentWnd);
				CWnd*			GetItemView(int iItem);
				BOOL			SetItemText(int iItem, CString csText);
				CString			GetItemText(int iItem=-1);
				int				SetItemImage(int iItem, UINT uiImage);
				BOOL			SetItemData(int iItem, LPARAM lParam);
				DWORD			GetItemState(int iItem, DWORD dwMask) const;
				BOOL			SetItemState(int iItem, DWORD dwMask, DWORD dwState);
				CImageList*		GetImageList() const;
				CImageList*		SetImageList(CImageList* pImageList);
				int				GetItemCount() const;
				BOOL			GetItem(int iItem, TCITEM* pTabCtrlItem) const;
				BOOL			SetItem(int iItem, TCITEM* pTabCtrlItem);
				BOOL			GetItemRect(int iItem, LPRECT lpRect) const;
				int				GetCurSel() const;
				int				SetCurSel(int iItem);
				void			SetCurFocus(int iItem);
				CSize			SetItemSize(CSize size);
				void			SetPadding(CSize size);
				CToolTipCtrl*	GetToolTips() const;
				void			SetToolTips(CToolTipCtrl* pWndTip);
				int				GetCurFocus() const;
				int				SetMinTabWidth(int cx);
				DWORD			GetExtendedStyle();
				DWORD			SetExtendedStyle(DWORD dwNewStyle, DWORD dwExMask = 0);
				void			AdjustRect(BOOL bLarger, LPRECT lpRect);
				void			RemoveImage(int iImage);
				int				HitTest(TCHITTESTINFO* pHitTestInfo) const;
				BOOL			HighlightItem(int idItem, BOOL fHighlight = TRUE);

				BOOL			RemoveView(int iItem);
				BOOL			RemoveAllViews();

				int				InsertView(CRuntimeClass *pViewClass, int iItem, LPCTSTR lpszItem, CCreateContext* pContext = NULL);
				int				InsertView(CRuntimeClass *pViewClass, int iItem, LPCTSTR lpszItem, int iImage, CCreateContext* pContext = NULL);
				int				InsertView(CRuntimeClass *pViewClass, UINT uiMask, int iItem, LPCTSTR lpszItem, int iImage, LPARAM lParam, CCreateContext* pContext = NULL);
				int				InsertView(CRuntimeClass *pViewClass, UINT uiMask, int iItem, LPCTSTR lpszItem, int iImage, LPARAM lParam, DWORD dwState, DWORD dwStateMask, CCreateContext* pContext = NULL);
				int				InsertView(CRuntimeClass *pViewClass, int iItem, TCITEM* pTabCtrlItem, CCreateContext* pContext = NULL);

				int				AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, CCreateContext* pContext = NULL);
				int				AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, int iImage, CCreateContext* pContext = NULL);
				int				AddView(CRuntimeClass *pViewClass, LPCTSTR lpszItem, int iImage, LPARAM lParam, CCreateContext* pContext = NULL);
				int				AddView(CRuntimeClass *pViewClass, UINT uiMask, LPCTSTR lpszItem, int iImage, LPARAM lParam, DWORD dwState, DWORD dwStateMask, CCreateContext* pContext = NULL);
				int				AddView(CRuntimeClass *pViewClass, TCITEM* pTabCtrlItem, CCreateContext* pContext = NULL);

		virtual	BOOL			OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

		virtual	BOOL			BroadcastEvent(WORD event, void* pParameter=NULL);
		virtual	void			LogMessage(const CString text, const CHARFORMAT* pFormat);
};

//-----  INLINES  -------------------------------------------------------------
inline CImageList* BCTabBarCtrl::GetImageList() const
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TCM_GETIMAGELIST, 0, 0L)); }
inline CImageList* BCTabBarCtrl::SetImageList(CImageList* pImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST)::SendMessage(m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)pImageList->GetSafeHandle())); }
inline int BCTabBarCtrl::GetItemCount() const
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETITEMCOUNT, 0, 0L); }
inline void BCTabBarCtrl::SetCurFocus(int iItem)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETCURFOCUS, (WPARAM)iItem, 0); }
inline BOOL BCTabBarCtrl::GetItemRect(int iItem, LPRECT lpRect) const
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_GETITEMRECT, iItem, (LPARAM)lpRect); }
inline int BCTabBarCtrl::GetCurSel() const
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L); }
inline int BCTabBarCtrl::HitTest(TCHITTESTINFO* pHitTestInfo) const
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_HITTEST, 0, (LPARAM) pHitTestInfo); }
inline void BCTabBarCtrl::AdjustRect(BOOL bLarger, LPRECT lpRect)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_ADJUSTRECT, bLarger, (LPARAM)lpRect); }
inline CSize BCTabBarCtrl::SetItemSize(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); return (CSize)::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(size.cx,size.cy)); }
inline void BCTabBarCtrl::RemoveImage(int iImage)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_REMOVEIMAGE, iImage, 0L); }
inline void BCTabBarCtrl::SetPadding(CSize size)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(size.cx, size.cy)); }
inline CToolTipCtrl* BCTabBarCtrl::GetToolTips() const
	{ ASSERT(::IsWindow(m_hWnd)); return (CToolTipCtrl*)CControlBar::FromHandle((HWND)::SendMessage(m_hWnd, TCM_GETTOOLTIPS, 0, 0L)); }
inline void BCTabBarCtrl::SetToolTips(CToolTipCtrl* pWndTip)
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)pWndTip->GetSafeHwnd(), 0L); }
inline int BCTabBarCtrl::GetCurFocus() const
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURFOCUS, 0, 0L); }
inline int BCTabBarCtrl::SetMinTabWidth(int cx)
	{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_SETMINTABWIDTH, 0, (LPARAM)cx); }
inline BOOL BCTabBarCtrl::HighlightItem(int idItem, BOOL fHighlight /*= TRUE*/)
	{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_HIGHLIGHTITEM, idItem, MAKELPARAM(fHighlight, 0)); }
inline DWORD BCTabBarCtrl::GetExtendedStyle()
	{ ASSERT(::IsWindow(m_hWnd)); return (DWORD) ::SendMessage(m_hWnd, TCM_GETEXTENDEDSTYLE, 0, 0); }
inline DWORD BCTabBarCtrl::SetExtendedStyle(DWORD dwNewStyle, DWORD dwExMask /*= 0*/)
	{ ASSERT(::IsWindow(m_hWnd)); return (DWORD) ::SendMessage(m_hWnd, TCM_SETEXTENDEDSTYLE, dwExMask, dwNewStyle); }
