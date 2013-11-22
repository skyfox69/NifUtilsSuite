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
#include <vector>

using namespace std;

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
		vector<LPDIRECT3DVERTEXBUFFER9>		_vecVBuffer;		//  VertexBuffer to hold vertices
		vector<LPDIRECT3DINDEXBUFFER9>		_vecIBuffer;		//  IndexBuffer to hold face indexes
		vector<D3DCustomVertex*>			_vecVertices;		//  raw vertex buffer
		vector<unsigned short*>				_vecIndices;		//  raw index buffer
		vector<unsigned int>				_vecCountVertices;	//  number of vertices
		vector<unsigned int>				_vecCountIndices;	//  number of indices
		vector<D3DPRIMITIVETYPE>			_vecPrimitiveType;	//  primitive type

	public:
								DirectXMeshCollision(D3DXMATRIX transform,
													 D3DCustomVertex* pBufferV,
													 const unsigned int countV,
													 unsigned short* pBufferI,
													 const unsigned int countI,
													 DWORD wireframeColor);

								DirectXMeshCollision(D3DXMATRIX transform,
													 vector<D3DCustomVertex*> vecBufferV,
													 const vector<unsigned int> vecCountV,
													 vector<unsigned short*> vecBufferI,
													 const vector<unsigned int> vecCountI,
													 DWORD wireframeColor);
		virtual					~DirectXMeshCollision();

		virtual	D3DPRIMITIVETYPE	SetPrimitiveType  (const D3DPRIMITIVETYPE type);
		virtual	D3DPRIMITIVETYPE	SetPrimitiveType  (const vector<D3DPRIMITIVETYPE> vecType);

		//  IfcDirectXRenderObject
		virtual	bool				Render            (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
		virtual	DWORD				SetWireframeColor (const DWORD color);
		virtual	DirectXRenderMode	SetRenderMode     (const DirectXRenderMode renderMode);
		virtual	DirectXRenderMode	IncreaseRenderMode();
};
