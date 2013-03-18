/**
 *  file:   NifUtilsSuiteDoc.cpp
 *  class:  CNifUtilsSuiteDoc
 *
 *  'pseudo' document for SDI
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuiteDoc.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNCREATE(CNifUtilsSuiteDoc, CDocument)

//-----  CNifUtilsSuiteDoc()  -------------------------------------------------
CNifUtilsSuiteDoc::CNifUtilsSuiteDoc()
{}

//-----  ~CNifUtilsSuiteDoc()  ------------------------------------------------
CNifUtilsSuiteDoc::~CNifUtilsSuiteDoc()
{}

//-----  OnNewDocument()  -----------------------------------------------------
BOOL CNifUtilsSuiteDoc::OnNewDocument()
{
	return CDocument::OnNewDocument();
}

//-----  Serialize()  ---------------------------------------------------------
void CNifUtilsSuiteDoc::Serialize(CArchive& ar)
{}
