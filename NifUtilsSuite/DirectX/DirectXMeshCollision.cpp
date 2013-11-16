/**
 *  file:   DirectXMeshCollision.h
 *  class:  DirectXMeshCollision
 *
 *  Class representing single collision mesh for drawing in DirectX
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "DirectX\DirectXMeshCollision.h"

//-----  DEFINES  -------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX_COLOR		D3DFVF_XYZ | D3DFVF_DIFFUSE

//-----  DirectXMeshModel()  --------------------------------------------------
DirectXMeshCollision::DirectXMeshCollision(D3DXMATRIX transform,
								   D3DCustomVertex* pBufferV,
								   const unsigned int countV,
								   unsigned short* pBufferI,
								   const unsigned int countI,
								   DWORD wireframeColor)
	:	DirectXMesh   (),
		_pVBuffer     (NULL),
		_pIBuffer     (NULL),
		_pVertices    (pBufferV),
		_pIndices     (pBufferI),
		_countVertices(countV),
		_countIndices (countI),
		_primitiveType(D3DPT_TRIANGLELIST)
{
	_transform      = transform;
	_wireframeColor = wireframeColor;
}

//-----  ~DirectXMeshModel()  -------------------------------------------------
DirectXMeshCollision::~DirectXMeshCollision()
{
	if (_pVBuffer  != NULL)		_pVBuffer->Release();
	if (_pIBuffer  != NULL)		_pIBuffer->Release();
	if (_pVertices != NULL)		delete[] _pVertices;
	if (_pIndices  != NULL)		delete[] _pIndices;
}

//-----  SetWireframeColor()  -------------------------------------------------
DWORD DirectXMeshCollision::SetWireframeColor(const DWORD color)
{
	DWORD	oldColor(_wireframeColor);

	//  set new color
	_wireframeColor = color;

	//  wireframe mode?
	if ((_renderMode == DXRM_WIREFRAME) && (_pVBuffer != NULL))
	{
		_forceNoRender = true;		//  disable rendering

		_pVBuffer->Release();
		_pVBuffer = NULL;

		_forceNoRender = false;		//  enable rendering

	}  //  if (_renderMode == DXRM_WIREFRAME)

	return oldColor;
}

//-----  IncreaseRenderMode()  ------------------------------------------------
DirectXRenderMode DirectXMeshCollision::IncreaseRenderMode()
{
	DirectXRenderMode	oldMode   (GetRenderMode());
	int					renderMode(oldMode + 1);

	if (renderMode >= DXRM_SOLID)		renderMode = 0;

	SetRenderMode((DirectXRenderMode) renderMode);
	return oldMode;
}

//-----  SetRenderMode()  -----------------------------------------------------
DirectXRenderMode DirectXMeshCollision::SetRenderMode(const DirectXRenderMode renderMode)
{
	//  check allowed modes
	if (renderMode >= 2)		return _renderMode;

	return DirectXMesh::SetRenderMode(renderMode);
}

//-----  SetPrimitiveType()  --------------------------------------------------
D3DPRIMITIVETYPE DirectXMeshCollision::SetPrimitiveType(const D3DPRIMITIVETYPE type)
{
	D3DPRIMITIVETYPE	oldType(_primitiveType);

	_primitiveType = type;
	return oldType;
}

//-----  Render()  ------------------------------------------------------------
bool DirectXMeshCollision::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if ((_renderMode == DXRM_NONE) || _forceNoRender)		return true;

	//  create DX parameters if not existing
	if (_pVBuffer == NULL)
	{
		//  vertices
		D3DCustomVertex*	pVModel(NULL);

		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
		_pVBuffer->Lock(0, 0, (void**)&pVModel, 0);
		for (unsigned int i(0); i < _countVertices; ++i)
		{
			pVModel[i]._x     = _pVertices[i]._x;
			pVModel[i]._y     = _pVertices[i]._y;
			pVModel[i]._z     = _pVertices[i]._z;
			pVModel[i]._color = _wireframeColor;
		}
		_pVBuffer->Unlock();

		//  indices
		if (_pIBuffer == NULL)
		{
			unsigned short*		pIModel(NULL);

			pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
			_pIBuffer->Lock(0, 0, (void**)&pIModel, 0);
			memcpy(pIModel, _pIndices, _countIndices*sizeof(unsigned short));
			_pIBuffer->Unlock();
		}
	}  //  if (_pVBuffer == NULL)

	//  wireframe?
	if (_renderMode == DXRM_WIREFRAME)
	{
		pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
		pd3dDevice->SetTexture          (0, NULL);														//  no texture
		pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);								//  disable alpha blending
		pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);							//  forced wireframe
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);										//  disable light
		pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertex));						//  set vertices source
		pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style

		unsigned int	countIndices (_countIndices);
		unsigned int	countVertices(_countVertices);

		switch (_primitiveType)
		{
			case D3DPT_TRIANGLELIST:
			{
				countIndices  = _countIndices/3;
				countVertices = _countVertices;
				break;
			}

			case D3DPT_LINELIST:
			{
				countIndices  = _countIndices/2;
				countVertices = _countVertices;
				break;
			}
		}

		pd3dDevice->DrawIndexedPrimitive(_primitiveType, 0, 0, countVertices, 0, countIndices);			//  render
	}

	return true;
}
