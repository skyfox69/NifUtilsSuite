/**
 *  file:   DirectXMeshAxes.h
 *  class:  DirectXMeshAxes
 *
 *  Class representing coordinate axes for drawing in DirectX
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  NifUtiliy includes
#include "DirectX\DirectXMesh.h"

//-----  CLASS  ---------------------------------------------------------------
class DirectXMeshAxes : public DirectXMesh
{
	public:
		struct D3DCustomVertex
		{
			float		_x;
			float		_y;
			float		_z;
			DWORD		_color;
		};

	protected:
		LPDIRECT3DVERTEXBUFFER9	_pVBuffer;		//  VertexBuffer to hold vertices
		LPDIRECT3DINDEXBUFFER9	_pIBuffer;		//  IndexBuffer to hold face indexes
		unsigned int			_countVertices;	//  number of vertices
		unsigned int			_countIndices;	//  number of indices

	public:
								DirectXMeshAxes();
		virtual					~DirectXMeshAxes();

		//  IfcDirectXRenderObject
		virtual	bool				Render            (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
		virtual	DWORD				SetWireframeColor (const DWORD)	{ return 0; }
		virtual bool				HasAlphaBlending  () const		{ return false; }
		virtual	DirectXRenderMode	SetRenderMode     (const DirectXRenderMode renderMode);
		virtual	DirectXRenderMode	IncreaseRenderMode();
};
