/**
 *  file:   DirectXMesh.cpp
 *  class:  DirectXMesh
 *
 *  Class representing a mesh renderable in DirectX
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "DirectX\DirectXMesh.h"
#include "Common\Util\FDCLibHelper.h"

//-----  DEFINES  -------------------------------------------------------------


//-----  DirectXMesh()  -------------------------------------------------------
DirectXMesh::DirectXMesh(DirectXAlphaState* pAlpha/*=NULL*/)
	:	_pAlpha        (pAlpha),
		_renderMode    (DXRM_WIREFRAME),
		_wireframeColor(0xFFFFFFFF),
		_lodRenderLevel(0),
		_lvIndex       (-1),
		_isSelected    (false),
		_forceNoRender (false)
{
	memset(&_material, 0, sizeof(_material));
	_material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	_material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//-----  ~DirectXMesh()  ------------------------------------------------------
DirectXMesh::~DirectXMesh()
{
	if (_pAlpha != NULL)		delete _pAlpha;
}

//-----  SetSelected()  -------------------------------------------------------
bool DirectXMesh::SetSelected(const bool selected)
{
	bool	oldSelected(_isSelected);

	_isSelected = selected;
	return oldSelected;
}

//-----  SetNifData()  --------------------------------------------------------
void DirectXMesh::SetNifData(const string name, const string className, const int blockNumber)
{
	_nifName        = name;
	_nifClass       = className;
	_nifBlockNumber = blockNumber;
}

//-----  SetRenderMode()  -----------------------------------------------------
DirectXRenderMode DirectXMesh::SetRenderMode(const DirectXRenderMode renderMode)
{
	DirectXRenderMode	oldRender(_renderMode);

	_renderMode = renderMode;
	return oldRender;
}

//-----  SetImageIndex()  -----------------------------------------------------
void DirectXMesh::SetImageIndex(const int index)
{
	_lvIndex = index;
}

//-----  SetColorBackground()  ------------------------------------------------
void DirectXMesh::SetColorBackground(const COLORREF color)
{
	SetWireframeColor(bgr2argb(color));
}

//-----  GetColorBackground()  ------------------------------------------------
COLORREF DirectXMesh::GetColorBackground() const
{
	return argb2bgr(_wireframeColor);
}

//-----  SetLODRenderLevel()  -------------------------------------------------
unsigned int DirectXMesh::SetLODRenderLevel(const unsigned int lodLevel)
{
	return _lodRenderLevel;
}
