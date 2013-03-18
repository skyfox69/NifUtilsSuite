/**
 *  file:   IfcModelListViewObject.h
 *  class:  IfcModelListViewObject
 *
 *  Abstract class defining interface as list view object for model list
 *
 */

#pragma once

//-----  INTERFACE  -----------------------------------------------------------
class IfcModelListViewObject
{
	public:
		//  index of image in image list
		virtual	int			GetImageIndex() const = 0;
		virtual	void		SetImageIndex(const int index) = 0;

		//  background color
		virtual	COLORREF	GetColorBackground() const = 0;
		virtual	void		SetColorBackground(const COLORREF color) = 0;
};
