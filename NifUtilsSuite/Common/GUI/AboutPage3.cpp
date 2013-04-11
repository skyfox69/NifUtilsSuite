/**
 *  file:   CAboutPage3.cpp
 *  class:  CAboutPage3
 *
 *  general about page
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\AboutPage3.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(CAboutPage3, CPropertyPage)

BEGIN_MESSAGE_MAP(CAboutPage3, CPropertyPage)
END_MESSAGE_MAP()

//-----  CAboutPage2()  -------------------------------------------------------
CAboutPage3::CAboutPage3(CWnd* pParent /*=NULL*/)
	: CPropertyPage(CAboutPage3::IDD)
{}

//-----  ~CAboutPage2()  ------------------------------------------------------
CAboutPage3::~CAboutPage3()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CAboutPage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

}

//-----  OnOK()  --------------------------------------------------------------
void CAboutPage3::OnOK()
{}

//-----  OnSetActive()  -------------------------------------------------------
BOOL CAboutPage3::OnSetActive()
{
	CString		tString;

	tString  = "\r\n";
	tString += "Version 1.0.5 (Beta3)\r\n";
	tString += "=====================\r\n\r\n";
	tString += "* New:	ChunkExtract: add option 'scaling to model mesh'\r\n";
	tString += "* New:	ModelViewer: tooltips for listview in ModelViewer\r\n";
	tString += "* Chng:	Common: remove debug code\r\n";
	tString += "* Fix:	ModelViewer: wrong tranlation/rotation to collision meshes in case of bhkRidgidBody\r\n\r\n\r\n";
	tString += "Version 1.0.4 (Beta2)\r\n";
	tString += "=====================\r\n\r\n";
	tString += "* New:	Settings: show full path names as tooltip on general tab\r\n";
	tString += "* New:	ModelViewer: shortcut F4 for toggling collision display as in CreationKit\r\n";
	tString += "* New:	ModelViewer: button 'Reload Model' + shortcut F5 as in CreationKit\r\n";
	tString += "* New:	ModelViewer: reset to users defaults when loading new model\r\n";
	tString += "* Fix:	ModelViewer: toggling 'row background' in settings has no effect\r\n";
	tString += "* Fix:	ModelViewer: display material name used at each Chunk/BigTri\r\n";
	tString += "* Fix:	ModelViewer: some nested blocks rendered at wrong position\r\n";
	tString += "* Fix:	ModelViewer: crash when attempt to render NIF without UV set\r\n";
	tString += "* Fix:	ModelViewer: translation/rotation of 'bhkRidgidBodyT' is ignored\r\n\r\n\r\n";
	tString += "Version 1.0.3 (Beta1)\r\n";
	tString += "=====================\r\n\r\n";
	tString += "* New:	first initial release of NIFUtilsSuite as Beta\r\n";
	tString += "* New:	Common: versioning system\r\n";
	tString += "* New:	Common: remember last visible tab on exit and open there\r\n\r\n\r\n";
	tString += "Version 1.0.2 (Alpha3)\r\n";
	tString += "======================\r\n\r\n";
	tString += "* New:	ModelViewer: toggle 'force double sided' or 'single sided' mesh rendering\r\n";
	tString += "* Chng:	ModelViewer: reworked alpha blending and testing\r\n";
	tString += "* Fix:	Common: problem with space character in name of path to nif when opening NifSkope\r\n";
	tString += "* Fix:	ModelViewer: crash when getting NIF that has no UV set\r\n";
	tString += "* Fix:	ModelViewer: ticking 'Model' below the list don't turn-on nodes without texture\r\n";
	tString += "* Fix:	ModelViewer: supporting for BigTris and BigVerts in bhkCompressedMeshShapeData\r\n\r\n\r\n";
	tString += "Version 1.0.1 (Alpha2)\r\n";
	tString += "======================\r\n\r\n";
	tString += "* Fix:	Common: multiple crashes due to Niflib.dll => static linking works\r\n";
	tString += "* Fix:	Common: showing default input directory on output file browsers\r\n";
	tString += "* Fix:	ChunkExtract: typo 'Namining' in ChunkExtract hints\r\n\r\n\r\n";
	tString += "Version 1.0.0 (Alpha1)\r\n";
	tString += "======================\r\n\r\n";
	tString += "* New:	first initial release of NIFUtilsSuite as Alpha to limited audience\r\n";

	GetDlgItem(IDC_EDIT1)->SetWindowText(tString);

	return CPropertyPage::OnSetActive();
}
