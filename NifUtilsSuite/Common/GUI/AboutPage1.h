/**
 *  file:   AboutPage1.h
 *  class:  CAboutPage1
 *
 *  general about page
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class CAboutPage1 : public CPropertyPage
{
	private:
		enum { IDD = IDD_PAGE_ABOUT1 };

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	void			OnOK();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CAboutPage1)

	public:
								CAboutPage1(CWnd* pParent = NULL);
		virtual					~CAboutPage1();
};
