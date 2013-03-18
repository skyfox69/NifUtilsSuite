/**
 *  file:   DirectXMeshModelLOD.cpp
 *  class:  DirectXMeshModelLOD
 *
 *  Class representing single mesh of model havinf LOD for drawing in DirectX
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "DirectX\DirectXMeshModelLOD.h"

//-----  DEFINES  -------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX_COLOR		D3DFVF_XYZ | D3DFVF_DIFFUSE
#define D3DFVF_CUSTOMVERTEX_FULL		D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1

//-----  DirectXMeshModelLOD()  -----------------------------------------------
DirectXMeshModelLOD::DirectXMeshModelLOD(D3DXMATRIX transform,
										 D3DMATERIAL9 material,
										 D3DCustomVertexFull* pBufferV,
										 const unsigned int countV,
										 unsigned short* pBufferI,
										 const unsigned int countI,
										 string textureName,
										 DirectXAlphaState* pAlpha,
										 const bool isBillboard,
										 DWORD wireframeColor,
										 const bool doubleSided,
										 const unsigned int* pLodLevel)
	:	DirectXMeshModel(transform, material, pBufferV, countV, pBufferI, countI, textureName, pAlpha, isBillboard, wireframeColor, doubleSided)
{
	for (short idx(0); idx < 3; ++idx)
	{
		_lodLevel[idx] = pLodLevel[idx];
	}
}

//-----  ~DirectXMeshModelLOD()  ----------------------------------------------
DirectXMeshModelLOD::~DirectXMeshModelLOD()
{}

//-----  SetLODRenderLevel()  -------------------------------------------------
unsigned int DirectXMeshModelLOD::SetLODRenderLevel(const unsigned int lodLevel)
{
	DWORD	oldLevel(_lodRenderLevel);

	//  set new level
	_lodRenderLevel = lodLevel;

	//  existing buffer
	if (_pVBuffer != NULL)
	{
		_forceNoRender = true;		//  disable rendering

		_pVBuffer->Release();
		_pVBuffer = NULL;

		_pIBuffer->Release();
		_pIBuffer = NULL;

		_forceNoRender = false;		//  enable rendering

	}  //  if (_pVBuffer != NULL)

	return oldLevel;
}

//-----  Render()  ------------------------------------------------------------
bool DirectXMeshModelLOD::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if ((_renderMode == DXRM_NONE) || _forceNoRender)		return true;

	//  no rendering due to LOD level size
	if (_lodLevel[_lodRenderLevel] == 0)					return true;

	//  create DX parameters if not existing
	if (_pIBuffer == NULL)
	{
		unsigned short*		pIModel(NULL);

		//  set number of indices depending on LOD
		_countIndices = _lodLevel[_lodRenderLevel] * 3;

		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIModel, 0);
		memcpy(pIModel, _pIndices, _countIndices*sizeof(unsigned short));
		_pIBuffer->Unlock();
	}

	//  rendering done by parent
	return DirectXMeshModel::Render(pd3dDevice, worldMatrix);
}
