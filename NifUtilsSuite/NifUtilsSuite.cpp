/**
 *  file:   NifUtilsSuite.cpp
 *  class:  CNifUtilsSuiteApp
 *
 *  Main application
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"

#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Common/GUI/NifUtilsSuiteFrm.h"
#include "Common/Util/Configuration.h"
#include "Common/Nif/NifUtlMaterial.h"
#include "Common/Util/FDResourceManager.h"
#include "Tools/NifConvert/FormNifConvertView.h"

#include "Common/Base/hkBase.h"
#include "Common/Base/System/hkBaseSystem.h"
#include "Common/Base/Memory/System/Util/hkMemoryInitUtil.h"
#include "Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h"
#include "Common/Base/keycode.cxx"
#ifdef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_ANIMATION
#endif
#ifndef HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#endif
#include "Common/Base/Config/hkProductFeatures.cxx" 

//-----  GLOBALS  -------------------------------------------------------------
CNifUtilsSuiteApp	theApp;		//  The one and only CNifUtilsSuiteApp object

//-----  DEFINES  -------------------------------------------------------------
#define HK_MAIN_CALL		_cdecl
#define HK_MEMORY_USAGE		500000

#if defined( HK_ATOM )
extern "C" int __cdecl ADP_Close( void );
#endif

BEGIN_MESSAGE_MAP(CNifUtilsSuiteApp, CWinAppEx)
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

//-----  errorReport()  -------------------------------------------------------
static void HK_CALL errorReport(const char* msg, void* userArgGivenToInit)
{
	CString	tString("Havok System returns error:\r\n\r\n");

	AfxMessageBox(tString += msg);
}

//-----  CNifUtilsSuiteApp()  -------------------------------------------------
CNifUtilsSuiteApp::CNifUtilsSuiteApp()
{
	SetAppID(_T("NifUtilsSuite.AppID.NoVersion"));
}

//-----  ~CNifUtilsSuiteApp()  ------------------------------------------------
CNifUtilsSuiteApp::~CNifUtilsSuiteApp()
{
	delete NifUtlMaterialList::getInstance();
	delete CFDResourceManager::getInstance();
	delete Configuration::getInstance();
	CMFCVisualManager::DestroyInstance();
}

//-----  InitInstance()  ------------------------------------------------------
BOOL CNifUtilsSuiteApp::InitInstance()
{
	//  init common controls (since Win95)
	INITCOMMONCONTROLSEX	InitCtrls;

	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC  = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	//  init own instance
	CWinAppEx::InitInstance();

	//  no interaction with Win7 taskbar
	EnableTaskbarInteraction(FALSE);

	//  init RichEdit
	AfxInitRichEdit2();

	//  initialize configuration
	CStringA	configName;
	bool		hasConfig(false);

	GetModuleFileNameA(NULL, configName.GetBuffer(MAX_PATH), MAX_PATH);
	configName.ReleaseBuffer();
	configName.Replace(".exe", ".xml");
	hasConfig = Configuration::initInstance((const char*) configName);

	//  initialize Havok  (HK_MEMORY_USAGE bytes of physics solver buffer)
	hkMemoryRouter*		pMemoryRouter(hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(HK_MEMORY_USAGE)));
	hkBaseSystem::init(pMemoryRouter, errorReport);

	//  initialize material map
	Configuration*	pConfig(Configuration::getInstance());

	NifUtlMaterialList::initInstance(pConfig->_pathNifXML, pConfig->_matScanTag, pConfig->_matScanName);


	// Register the application's document templates.
	CSingleDocTemplate*	pDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME,
															RUNTIME_CLASS(CNifUtilsSuiteDoc),
															RUNTIME_CLASS(CNifUtilsSuiteFrame),
															RUNTIME_CLASS(CFormNifConvertView))
															);
	if (!pDocTemplate)		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo	cmdInfo;

	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))		return FALSE;

	//  Parse command line for special handling
	LPWSTR*		argv;
	int			argc(0);

	argv = CommandLineToArgvW(m_lpCmdLine, &argc);
	for (int idx(0); idx < argc; ++idx)
	{
		if (StrCmpW(argv[idx], _T("-ap")) == 0)
		{
			pConfig->_autoSetPath = true;
		}
	}
	LocalFree(argv);

	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	((CNifUtilsSuiteFrame*) m_pMainWnd)->ShowLogWindow();

	if (!hasConfig)
	{
		AfxMessageBox(L"Seems you're running NifUtilsSuite for the first time.\nYou're redirected to Settings now.");
		m_pMainWnd->PostMessage(WM_COMMAND, ID_OPTIONS_EDIT);
	}

	return TRUE;
}

//-----  InitInstance()  ------------------------------------------------------
int CNifUtilsSuiteApp::ExitInstance()
{
	//  cleanup Havok
	hkBaseSystem::quit();
	hkMemoryInitUtil::quit();

	return CWinAppEx::ExitInstance();
}
