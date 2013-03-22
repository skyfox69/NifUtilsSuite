/**
 *  file:   OptionsSheet.h
 *  class:  COptionsSheet
 *
 *  Main property sheet for options/properties
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\OptionsSheet.h"
#include "Common\GUI\OptionsPageGeneral.h"
#include "Common\GUI\OptionsPageMaterial.h"
#include "Common\GUI\OptionsPageLogView.h"
#include "Tools\NifConvert\OptionsPageNifConvert.h"
#include "Tools\ChunkMerge\OptionsPageChunkMerge.h"
#include "Tools\ChunkExtract\OptionsPageChunkExtract.h"
#ifndef NUS_LIGHT
#include "Tools\ModelViewer\OptionsPageModelView.h"
#endif
#include "Common\Util\Configuration.h"
#include "Common\Nif\NifUtlMaterial.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(COptionsSheet, CPropertySheet)
END_MESSAGE_MAP()

//-----  COptionsSheet()  -----------------------------------------------------
COptionsSheet::COptionsSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{}

//-----  COptionsSheet()  -----------------------------------------------------
COptionsSheet::COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{}

//-----  ~COptionsSheet()  ----------------------------------------------------
COptionsSheet::~COptionsSheet()
{}

//-----  DoModal()  -----------------------------------------------------------
INT_PTR COptionsSheet::DoModal()
{
	COptionsPageGeneral			optPageGen;
	COptionsPageMaterial		optPageMat;
	COptionsPageNifConvert		optPageNif;
	COptionsPageChunkMerge		optPageCMg;
	COptionsPageChunkExtract	optPageCEx;
#ifndef NUS_LIGHT
	COptionsPageModelView		optPageMVw;
#endif
	COptionsPageLogView			optPageLVw;

	AddPage(&optPageGen);
	AddPage(&optPageLVw);
	AddPage(&optPageMat);
	AddPage(&optPageNif);
	AddPage(&optPageCMg);
	AddPage(&optPageCEx);
#ifndef NUS_LIGHT
	AddPage(&optPageMVw);
#endif

	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	if (!Configuration::getInstance()->_hasFile)
	{
		SetWizardMode();
	}

	INT_PTR	retVal(CPropertySheet::DoModal());

	if ((retVal == IDOK) || (retVal == ID_WIZFINISH))
	{
		//  it's done in Page::OnOK() now
	}
	else
	{
		//  re-initialize material list - it might be changed during options dialog
		Configuration*	pConfig(Configuration::getInstance());

		delete NifUtlMaterialList::getInstance();
		NifUtlMaterialList::initInstance(pConfig->_pathNifXML, pConfig->_matScanTag, pConfig->_matScanName);
	}

	return retVal;
}
