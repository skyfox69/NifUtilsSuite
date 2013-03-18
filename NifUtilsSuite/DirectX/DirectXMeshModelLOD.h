/**
 *  file:   DirectXMeshModelLOD.h
 *  class:  DirectXMeshModelLOD
 *
 *  Class representing single mesh of model havinf LOD for drawing in DirectX
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  NifUtiliy includes
#include "DirectX\DirectXMeshModel.h"

//-----  CLASS  ---------------------------------------------------------------
class DirectXMeshModelLOD : public DirectXMeshModel
{
	protected:
		unsigned int			_lodLevel[3];	//  number of triangles per level

	public:
								DirectXMeshModelLOD(D3DXMATRIX transform,
													D3DMATERIAL9 material,
													D3DCustomVertexFull* pBufferV,
													const unsigned int countV,
													unsigned short* pBufferI,
													const unsigned int countI,
													string textureName,
													DirectXAlphaState* pAlpha,
													const bool isBillboard,
													DWORD wireframeColor,
													const bool doubledSided,
													const unsigned int* pLodLevel);
		virtual					~DirectXMeshModelLOD();

		//  IfcDirectXRenderObject
		virtual	bool				Render            (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
		virtual	unsigned int		SetLODRenderLevel (const unsigned int lodLevel);
};
