/**
 *  file:   AboutPage3.h
 *  class:  CAboutPage3
 *
 *  general about page
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class CAboutPage3 : public CPropertyPage
{
	private:
		enum { IDD = IDD_PAGE_ABOUT3 };

	protected:
		CFont					_myFont;

		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual	BOOL			OnInitDialog();
		virtual BOOL			OnSetActive();
		virtual	void			OnOK();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(CAboutPage3)

	public:
								CAboutPage3(CWnd* pParent = NULL);
		virtual					~CAboutPage3();
};
