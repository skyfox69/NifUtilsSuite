/**
 *  file:   FDResourceManager.h
 *  struct: CFDResourceManager
 *
 *  The one and only class organizing resources
 *  (bitmaps, icons, ...)
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\FDResourceManager.h"
#include "resource.h"
#include <afxribbonres.h>

//-----  DEFINES  -------------------------------------------------------------
CFDResourceManager* CFDResourceManager::_pInstance = NULL;

//-----  CFDResourceManager  --------------------------------------------------
CFDResourceManager::CFDResourceManager()
{
	initInstance();
}

//-----  ~CFDResourceManager  -------------------------------------------------
CFDResourceManager::~CFDResourceManager()
{
	_pInstance = NULL;
}

//-----  getInstance()  -------------------------------------------------------
CFDResourceManager* CFDResourceManager::getInstance()
{
	if (_pInstance == NULL)
	{
		_pInstance = new CFDResourceManager();
	}

	return _pInstance;
}

//-----  initInstance()  ------------------------------------------------------
bool CFDResourceManager::initInstance()
{
	CBitmap	bmp;

	//  browse images
	_imgListBrowse.Create(16, 16, ILC_MASK | ILC_COLOR24, 0, 0);
	bmp.LoadBitmap(IDB_AFXBARRES_BROWSE32);
	_imgListBrowse.Add(&bmp, RGB(255, 0, 255));
	bmp.DeleteObject();

	//  number images
	_imgListNumbers.Create(16, 16, ILC_MASK | ILC_COLOR24, 1, 1);
	bmp.LoadBitmap(IDB_NUMBERS);
	_imgListNumbers.Add(&bmp, RGB(255, 0, 0));
	bmp.DeleteObject();

	//  other images
	_imgListOther.Create(16, 16, ILC_COLOR32, 1, 1);
	bmp.LoadBitmap(IDB_OTHER);
	_imgListOther.Add(&bmp, RGB(255, 0, 0));
	bmp.DeleteObject();

	//  other images (disabled)
	_imgListOtherDisabled.Create(16, 16, ILC_COLOR32, 1, 1);
	bmp.LoadBitmap(IDB_OTHER_DIS);
	_imgListOtherDisabled.Add(&bmp, RGB(255, 0, 0));
	bmp.DeleteObject();

	//  model viewer images
	_imgListModelView.Create(16, 16, ILC_COLOR32, 1, 1);
	bmp.LoadBitmap(IDB_MODEL_VIEWER);
	_imgListModelView.Add(&bmp, RGB(255, 0, 0));
	bmp.DeleteObject();

	return true;
}
