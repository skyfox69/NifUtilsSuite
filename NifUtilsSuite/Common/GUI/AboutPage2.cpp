/**
 *  file:   AboutPage2.cpp
 *  class:  CAboutPage2
 *
 *  general about page
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\AboutPage2.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CAboutPage2, CPropertyPage)

BEGIN_MESSAGE_MAP(CAboutPage2, CPropertyPage)
END_MESSAGE_MAP()

//-----  CAboutPage2()  -------------------------------------------------------
CAboutPage2::CAboutPage2(CWnd* pParent /*=NULL*/)
	: CPropertyPage(CAboutPage2::IDD)
{}

//-----  ~CAboutPage2()  ------------------------------------------------------
CAboutPage2::~CAboutPage2()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CAboutPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

}

//-----  OnOK()  --------------------------------------------------------------
void CAboutPage2::OnOK()
{}

//-----  OnSetActive()  -------------------------------------------------------
BOOL CAboutPage2::OnSetActive()
{
	CString		tString;

	tString  = "\r\nNIFUTILSSUITE LICENSE\r\n\r\nCopyright (c) 2012-2013, NIFUtilsSuite\r\nAll rights reserved.\r\n\r\n";
	tString += "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\r\n\r\n";
	tString += "* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\r\n\r\n";
	tString += "* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\r\n\r\n";
	tString += "* Neither the name of the NIFUtilsSuite project nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\r\n\r\n";
	tString += "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS ";
	tString += "FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, ";
	tString += "BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT ";
	tString += "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\r\n\r\n";
	tString += "CREDITS\r\n\r\nNifUtilsSuite uses Havok(R) for the generation of mopp code and collision data. (C)Copyright 1999-2008 Havok.com Inc. (and its Licensors). All Rights Reserved.  See www.havok.com for details.\r\n\r\n";
	tString += "NifUtilsSuite uses Niflib as model base, from http://niftools.sourceforge.net/wiki. See Niflib_LICENSE.txt for details and https://github.com/Alecu100/niflib for the cloned branch source.\r\n\r\n";
	tString += "NOTICE\r\n\r\nParts of NifSkope (i.e. included libraries) might be subject to other licenses. This license only applies to original NifUtilsSuite sources";

	GetDlgItem(IDC_EDIT1)->SetWindowText(tString);

	return CPropertyPage::OnSetActive();
}
