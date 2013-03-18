/**
 *  file:   DirectXMeshModel.h
 *  class:  DirectXMeshModel
 *
 *  Class representing single mesh of model for drawing in DirectX
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  NifUtiliy includes
#include "DirectX\DirectXMesh.h"

//-----  CLASS  ---------------------------------------------------------------
class DirectXMeshModel : public DirectXMesh
{
	public:
		struct D3DCustomVertex
		{
			float		_x;
			float		_y;
			float		_z;
			DWORD		_color;
		};

		struct D3DCustomVertexFull
		{
			float		_x;
			float		_y;
			float		_z;
			D3DVECTOR	_normal;
			DWORD		_color;
			float		_u;
			float		_v;
		};

	protected:
		LPDIRECT3DVERTEXBUFFER9	_pVBuffer;		//  VertexBuffer to hold vertices
		LPDIRECT3DINDEXBUFFER9	_pIBuffer;		//  IndexBuffer to hold face indexes
		LPDIRECT3DTEXTURE9		_pTexture;		//  TextureBuffer
		D3DCustomVertexFull*	_pVertices;		//  raw vertex buffer
		unsigned short*			_pIndices;		//  raw index buffer
		string					_textureName;	//  path of texture image
		unsigned int			_countVertices;	//  number of vertices
		unsigned int			_countIndices;	//  number of indices
		bool					_isBillboard;	//  represents billboard type mesh
		bool					_doubleSided;	//  render both sides

	public:
								DirectXMeshModel(D3DXMATRIX transform,
												 D3DMATERIAL9 material,
												 D3DCustomVertexFull* pBufferV,
												 const unsigned int countV,
												 unsigned short* pBufferI,
												 const unsigned int countI,
												 string textureName,
												 DirectXAlphaState* pAlpha,
												 const bool isBillboard,
												 DWORD wireframeColor,
												 const bool doubleSided);
		virtual					~DirectXMeshModel();

		virtual	bool				GetDoubleSided() const	{ return _doubleSided; }
		virtual bool				SetDoubleSided(const bool doubleSided);

		//  IfcDirectXRenderObject
		virtual	bool				Render            (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
		virtual	DWORD				SetWireframeColor (const DWORD color);
		virtual	DirectXRenderMode	SetRenderMode(const DirectXRenderMode renderMode);
		virtual	DirectXRenderMode	IncreaseRenderMode();
};
