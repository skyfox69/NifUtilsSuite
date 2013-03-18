/**
 *  file:   BCBrowseVSListBox.h
 *  class:  BCBrowseVSListBox
 *
 *  VSListBox derived class for browsing file system
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\GUI\BCBrowseVSListBox.h"
#include "Common\Util\FDFileHelper.h"

//-----  DoDataExchange()  ----------------------------------------------------
void BCBrowseVSListBox::OnBrowse()
{
	CString	pathName;
	int nSel = GetSelItem();

	pathName = FDFileHelper::getFolder(GetItemText(nSel), _T("Texture (Texture-Dir)|*.*||"), _T("*"), false, _T("Please select texture directory"));

	if (nSel == GetCount()) // New item
	{
		nSel = AddItem(pathName);
		SelectItem(nSel);
	}
	else
	{
		SetItemText(nSel, pathName);
	}
}
