/**
 *  file:   FDResourceManager.h
 *  struct: CFDResourceManager
 *
 *  The one and only class organizing resources
 *  (bitmaps, icons, ...)
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"

//-----  CLASS  ---------------------------------------------------------------
class CFDResourceManager
{
	private:
		static	CFDResourceManager*		_pInstance;
				CImageList				_imgListNumbers;
				CImageList				_imgListBrowse;
				CImageList				_imgListOther;
				CImageList				_imgListOtherDisabled;
				CImageList				_imgListModelView;

	protected:
										CFDResourceManager();
		virtual	bool					initInstance();

	public:
		virtual							~CFDResourceManager();

		static	CFDResourceManager*		getInstance();

		virtual	CImageList*				getImageListBrowse()		{ return &_imgListBrowse; }
		virtual	CImageList*				getImageListNumbers()		{ return &_imgListNumbers; }
		virtual	CImageList*				getImageListOther()			{ return &_imgListOther; }
		virtual	CImageList*				getImageListOtherDis()		{ return &_imgListOtherDisabled; }
		virtual	CImageList*				getImageListModelView()		{ return &_imgListModelView; }
};
