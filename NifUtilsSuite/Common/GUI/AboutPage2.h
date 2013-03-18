/**
 *  file:   AboutPage2.h
 *  class:  CAboutPage2
 *
 *  general about page
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class CAboutPage2 : public CPropertyPage
{
	private:
		enum { IDD = IDD_PAGE_ABOUT2 };

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	void			OnOK();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CAboutPage2)

	public:
								CAboutPage2(CWnd* pParent = NULL);
		virtual					~CAboutPage2();
};
