/**
 *  file:   IfcDirectXRenderObject.h
 *  class:  IfcDirectXRenderObject
 *
 *  Abstract class defining interface for DirectX renderable object
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "d3dx9.h"

//-----  DEFINES  -------------------------------------------------------------
enum DirectXRenderMode { DXRM_NONE=0, DXRM_WIREFRAME, DXRM_SOLID, DXRM_TEXTURE };

//-----  INTERFACE  -----------------------------------------------------------
class IfcDirectXRenderObject
{
	public:
		//  prepare and render to DirectX9
		virtual	bool				Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix) = 0;

		//  allow rendering of object
		virtual	DirectXRenderMode	SetRenderMode(const DirectXRenderMode renderMode) = 0;
		virtual DirectXRenderMode	GetRenderMode() const = 0;
		virtual	DirectXRenderMode	IncreaseRenderMode() = 0;

		//  set color used for wireframe rendering
		virtual	DWORD				SetWireframeColor(const DWORD color) = 0;
		virtual	DWORD				GetWireframeColor() const = 0;

		//  alpha blending
		virtual bool				HasAlphaBlending() const = 0;

		//  LOD level
		virtual	unsigned int		SetLODRenderLevel(const unsigned int lodLevel) = 0;
};
