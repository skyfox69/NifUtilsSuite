/**
 *  file:   DirectXMesh.h
 *  class:  DirectXMesh
 *
 *  Class representing a mesh renderable in DirectX
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <string>

//  NifUtiliy includes
#include "DirectX\IfcDirectXRenderObject.h"
#include "DirectX\DirectXAlphaState.h"
#include "Common\Util\IfcModelListViewObject.h"

//-----  DEFINES  -------------------------------------------------------------
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class DirectXMesh : public IfcDirectXRenderObject, public IfcModelListViewObject
{
	protected:
		DirectXAlphaState*			_pAlpha;			//  parameters used for alpha blending
		D3DXMATRIX					_transform;			//  transformation to global coord system
		D3DMATERIAL9				_material;			//  material of object
		string						_nifName;			//  name of node in NIF tree
		string						_nifClass;			//  class name of node in NIF tree
		DirectXRenderMode			_renderMode;		//  render mode settings
		DWORD						_wireframeColor;	//  default color of wireframe
		unsigned int				_lodRenderLevel;	//  LOD level to be rendered
		int							_nifBlockNumber;	//  block number of node in NIF tree
		int							_lvIndex;			//  index of image in mode list view
		bool						_isSelected;		//  flag showing selection state
		bool						_forceNoRender;		//  internal flag disabling rendering

	public:
									DirectXMesh(DirectXAlphaState* pAlpha=NULL);
		virtual						~DirectXMesh();

		virtual	bool				SetSelected      (const bool selected);
		virtual	bool				IsSelected       () const	{ return _isSelected; }

		//  information about NIF
		virtual	void				SetNifData       (const string name, const string className, const int blockNumber);
		virtual	string				GetNifName       () const	{ return _nifName; }
		virtual	string				GetNifClass      () const	{ return _nifClass; }
		virtual	int					GetNifBlockNumber() const	{ return _nifBlockNumber; }

		//  IfcDirectXRenderObject
		virtual	DirectXRenderMode	SetRenderMode(const DirectXRenderMode renderMode);
		virtual DirectXRenderMode	GetRenderMode() const		{ return _renderMode; }
		virtual	DWORD				GetWireframeColor() const	{ return _wireframeColor; }
		virtual bool				HasAlphaBlending () const	{ return (_pAlpha != NULL); }

		//  IfcModelListViewObject
		virtual	int					GetImageIndex     () const	{ return _lvIndex; }
		virtual	void				SetImageIndex     (const int index);
		virtual	COLORREF			GetColorBackground() const;
		virtual	void				SetColorBackground(const COLORREF color);
		virtual	unsigned int		SetLODRenderLevel (const unsigned int lodLevel);
};
