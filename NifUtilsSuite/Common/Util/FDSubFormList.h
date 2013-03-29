/**
 *  file:   FDSubFormList.h
 *  struct: CFDSubFormList
 *
 *  Representation of a list of CDialogs used as sub forms
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include <vector>

using namespace std;

class CSubFormBase;

//-----  CLASS  ---------------------------------------------------------------
class CFDSubFormList : public vector<CSubFormBase*>
{
	protected:
		int						_posLeft;
		int						_posTop;
		int						_activeIdx;

	public:
								CFDSubFormList();
		virtual					~CFDSubFormList();

		virtual	bool			CreateSubForm(const UINT id, CWnd* pParentWnd, CSubFormBase* pDlg);
		virtual	bool			ShowSubForm  (const unsigned int index);
		virtual	void			SetPosition  (const CRect& rect);
		virtual	int				GetActIndex  ();
		virtual	CSubFormBase*	GetActiveDlg ();
};
