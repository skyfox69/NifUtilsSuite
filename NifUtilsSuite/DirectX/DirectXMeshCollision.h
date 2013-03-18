/**
 *  file:   DirectXMeshCollision.h
 *  class:  DirectXMeshCollision
 *
 *  Class representing single collision mesh for drawing in DirectX
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  NifUtiliy includes
#include "DirectX\DirectXMesh.h"

//-----  CLASS  ---------------------------------------------------------------
class DirectXMeshCollision : public DirectXMesh
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
		D3DCustomVertex*		_pVertices;		//  raw vertex buffer
		unsigned short*			_pIndices;		//  raw index buffer
		unsigned int			_countVertices;	//  number of vertices
		unsigned int			_countIndices;	//  number of indices

	public:
								DirectXMeshCollision(D3DXMATRIX transform,
													 D3DCustomVertex* pBufferV,
													 const unsigned int countV,
													 unsigned short* pBufferI,
													 const unsigned int countI,
													 DWORD wireframeColor);
		virtual					~DirectXMeshCollision();

		//  IfcDirectXRenderObject
		virtual	bool				Render            (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
		virtual	DWORD				SetWireframeColor (const DWORD color);
		virtual	DirectXRenderMode	SetRenderMode     (const DirectXRenderMode renderMode);
		virtual	DirectXRenderMode	IncreaseRenderMode();
};
