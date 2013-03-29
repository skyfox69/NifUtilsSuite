/**
 *  file:   FDSubFormList.cpp
 *  struct: CFDSubFormList
 *
 *  Representation of a list of CDialogs used as sub forms
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\FDSubFormList.h"
#include "Common\GUI\SubFormBase.h"
#include "resource.h"

//-----  CFDSubFormList()  ----------------------------------------------------
CFDSubFormList::CFDSubFormList()
	:	_posLeft  (0),
		_posTop   (0),
		_activeIdx(-1)
{}

//-----  ~CFDSubFormList()  ---------------------------------------------------
CFDSubFormList::~CFDSubFormList()
{
	for (auto pIter=begin(); pIter != end();)
	{
		(*pIter)->DestroyWindow();
		delete *pIter;
		pIter = erase(pIter);
	}
}

//-----  SetPosition()  -------------------------------------------------------
void CFDSubFormList::SetPosition(const CRect& rect)
{
	_posLeft = rect.left;
	_posTop  = rect.top;
}

//-----  CreateSubForm()  -----------------------------------------------------
bool CFDSubFormList::CreateSubForm(const UINT id, CWnd* pParentWnd, CSubFormBase* pDlg)
{
	CRect	tRect;

	//  create dialog
	if (pDlg == NULL)		return false;
	pDlg->Create(id, pParentWnd);

	//  position dialog
	pDlg->GetWindowRect(&tRect);
	pDlg->SetWindowPos(pParentWnd, _posLeft-tRect.left, _posTop-tRect.top, tRect.Width(), tRect.Height(), SWP_NOZORDER);

	//  add dialog to list
	push_back(pDlg);

	return true;
}

//-----  ShowSubForm()  -------------------------------------------------------
bool CFDSubFormList::ShowSubForm(const unsigned int index)
{
	//  check bounds
	if ((index < 0) || (index > size()))	return false;

	//  undisplay actual form if set
	if (_activeIdx != -1)
	{
		at(_activeIdx)->ShowWindow(SW_HIDE);
	}

	//  display new form
	_activeIdx = index;

	CDialogEx*	pDlg(at(_activeIdx));

	pDlg->ShowWindow(SW_SHOW);
	pDlg->Invalidate();
	pDlg->RedrawWindow();

	return true;
}

//-----  GetActIndex()  -------------------------------------------------------
int CFDSubFormList::GetActIndex()
{
	return _activeIdx;
}

//-----  GetActiveDlg()  ------------------------------------------------------
CSubFormBase* CFDSubFormList::GetActiveDlg()
{
	return at(_activeIdx);
}
