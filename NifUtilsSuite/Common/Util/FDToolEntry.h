/**
 *  file:   FDToolEntry.h
 *  struct: SFDToolEntry
 *
 *  struct holding data about runtime classes used as view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include <string>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
struct SFDToolEntry
{
	CRuntimeClass*		_pRTClass;
	UINT_PTR			_cmdId;
	string				_title;
	UINT				_resId;

						SFDToolEntry(CRuntimeClass* pRTClass, UINT_PTR cmdId, const char* pTitle, UINT resId);
	virtual				~SFDToolEntry();
};

//-----  INLINES --------------------------------------------------------------
//-----  FDToolEntry ----------------------------------------------------------
inline SFDToolEntry::SFDToolEntry(CRuntimeClass* pRTClass, UINT_PTR cmdId, const char* pTitle, UINT resId)
	:	_pRTClass(pRTClass),
		_cmdId   (cmdId),
		_title   (pTitle),
		_resId   (resId)
{}

//-----  ~FDToolEntry ---------------------------------------------------------
inline SFDToolEntry::~SFDToolEntry()
{}
