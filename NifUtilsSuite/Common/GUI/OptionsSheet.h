/**
 *  file:   OptionsSheet.h
 *  class:  COptionsSheet
 *
 *  Main property sheet for options/properties
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------


//-----  CLASS  ---------------------------------------------------------------
class COptionsSheet : public CPropertySheet
{
	protected:
		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsSheet)

	public:
								COptionsSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
								COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
		virtual					~COptionsSheet();

		virtual	INT_PTR			DoModal();
};
