/**
 *  file:   NifUtilsSuiteDoc.h
 *  class:  CNifUtilsSuiteDoc
 *
 *  'pseudo' document for SDI
 *
 */

#pragma once

//-----  CLASS  ---------------------------------------------------------------
class CNifUtilsSuiteDoc : public CDocument
{
	protected:
						CNifUtilsSuiteDoc();

		DECLARE_DYNCREATE(CNifUtilsSuiteDoc)

	public:
		virtual			~CNifUtilsSuiteDoc();

		virtual	BOOL	OnNewDocument();
		virtual	void	Serialize(CArchive& ar);
};
