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

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CAboutPage3::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	_myFont.CreatePointFont(76, _T("Courier New"));
	GetDlgItem(IDC_EDIT1)->SetFont(&_myFont);

	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL CAboutPage3::OnSetActive()
{
	CString		tString;

	tString  = "\r\n";
	tString += "Version 1.2.0\r\n";
	tString += "=============\r\n\r\n";
	tString += "* New:  Common: added ReferenceGuide documentation\r\n";
	tString += "* New:	Common: (Beta) support simple collision meshes allowed in Skyrim (e.g. bhkBoxShape, bhkNiTriStripsShape)\r\n";
	tString += "* New:  Common: support NiTriStrips next to NiTriShape\r\n";
	tString += "* New:  Common: adapt to newest nifxml/niflib\r\n";
	tString += "* New:  NifConvert: optional remove all collision nodes from NIF tree\r\n";
	tString += "* New:  NifConvert: re-use tangents and bitangents from NiBinaryExtraData\r\n";
	tString += "* New:  ChunkMerge: in-place collision nodes vs. global collision structure\r\n";
	tString += "* New:  ChunkMerge: detect and report NIF version\r\n";
	tString += "* New:  ModelViewer: add more colors (e.g. background, ambient)\r\n";
	tString += "* New:  ModelViewer: display (bhk(Packed)NiTriStrips)\r\n";
	tString += "* Fix:  Common: use correct version info in resource data\r\n";
	tString += "* Fix:  NifConvert: remove all NiMaterialProperty objects\r\n";
	tString += "* Fix:  ModelViewer: display wireframe of simple collision data in correct size\r\n";
	tString += "* Fix:  ModelViewer: display vertex colors in case of existing texture\r\n";
	tString += "* Fix:  ModelViewer: occasional crash when showing model\r\n\r\n\r\n";
	tString += "Version 1.1.2 (internal)\r\n";
	tString += "========================\r\n\r\n";
	tString += "* Fix:	ChunkMerge: correct handling of buildType and MOPP data (fields swapped)\r\n\r\n\r\n";
	tString += "Version 1.1.1\r\n";
	tString += "=============\r\n\r\n";
	tString += "* Chng:	ChunkExtract: handling of version number to be saved to\r\n";
	tString += "* Fix:	NifConvert: crash when reorder non existing properties\r\n";
	tString += "* Fix:	NifConvert: crash on missing UV-Sets\r\n";
	tString += "* Fix:	NifConvert: bad texture filename when having no externsion\r\n\r\n\r\n";
	tString += "Version 1.1.0\r\n";
	tString += "=============\r\n\r\n";
	tString += "* New:	BlenderPrepare: (Beta) added new tool to prepare armor-NIFs from Skyrim to Blender and vicy versa\r\n";
	tString += "* New:	Common: add new page on AboutDialog showing change-log\r\n";
	tString += "* New:	Common: open settings dialog on matching page\r\n";
	tString += "* New:	ChunkExtract: add flag saving NIF as version 20.2.0.7 (UserVersion 11, UserVerion2 34)\r\n";
	tString += "* Chng:	Common: make UI descriptions more informative\r\n";
	tString += "* Chng:	Common: different prerequisites for binaries and development environment\r\n";
	tString += "* Fix:	ModelViewer: use correct translation/rotation for bhkRidgidBodyT\r\n\r\n\r\n";
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
