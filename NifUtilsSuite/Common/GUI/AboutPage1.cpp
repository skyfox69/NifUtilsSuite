/**
 *  file:   AboutPage1.cpp
 *  class:  CAboutPage1
 *
 *  general about page
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\AboutPage1.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CAboutPage1, CPropertyPage)

BEGIN_MESSAGE_MAP(CAboutPage1, CPropertyPage)
END_MESSAGE_MAP()

//-----  CAboutPage1()  -------------------------------------------------------
CAboutPage1::CAboutPage1(CWnd* pParent /*=NULL*/)
	: CPropertyPage(CAboutPage1::IDD)
{}

//-----  ~CAboutPage1()  ------------------------------------------------------
CAboutPage1::~CAboutPage1()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CAboutPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

}

//-----  OnOK()  --------------------------------------------------------------
void CAboutPage1::OnOK()
{}

//-----  OnSetActive()  -------------------------------------------------------
BOOL CAboutPage1::OnSetActive()
{

	return CPropertyPage::OnSetActive();
}
