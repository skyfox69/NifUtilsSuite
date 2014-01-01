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
	:	DirectXMesh   ()
{
	_transform      = transform;
	_wireframeColor = wireframeColor;
	_vecVertices.push_back(pBufferV);
	_vecIndices.push_back(pBufferI);
	_vecCountVertices.push_back(countV);
	_vecCountIndices.push_back(countI);
	_vecPrimitiveType.push_back(D3DPT_TRIANGLELIST);
}

//-----  DirectXMeshModel()  --------------------------------------------------
DirectXMeshCollision::DirectXMeshCollision(D3DXMATRIX transform,
									vector<D3DCustomVertex*> vecBufferV,
									vector<unsigned int> vecCountV,
									vector<unsigned short*> vecBufferI,
									vector<unsigned int> vecCountI,
									DWORD wireframeColor)
	:	DirectXMesh   (),
		_vecVertices     (vecBufferV),
		_vecIndices      (vecBufferI),
		_vecCountVertices(vecCountV),
		_vecCountIndices (vecCountI)
{
	_transform      = transform;
	_wireframeColor = wireframeColor;
	for (unsigned int idx(0); idx < _vecVertices.size(); ++idx)
	{
		_vecPrimitiveType.push_back(D3DPT_TRIANGLELIST);
	}
}

//-----  ~DirectXMeshModel()  -------------------------------------------------
DirectXMeshCollision::~DirectXMeshCollision()
{
	for (auto pIter=_vecVBuffer.begin(), pEnd=_vecVBuffer.end(); pIter != pEnd; ++pIter)
	{
		(*pIter)->Release();
	}
	for (auto pIter=_vecIBuffer.begin(), pEnd=_vecIBuffer.end(); pIter != pEnd; ++pIter)
	{
		(*pIter)->Release();
	}
	for (auto pIter=_vecVertices.begin(), pEnd=_vecVertices.end(); pIter != pEnd; ++pIter)
	{
		delete[] (*pIter);
		//  removeFromVector ??
	}
	for (auto pIter=_vecIndices.begin(), pEnd=_vecIndices.end(); pIter != pEnd; ++pIter)
	{
		delete[] (*pIter);
		//  removeFromVector ??
	}
}

//-----  SetWireframeColor()  -------------------------------------------------
DWORD DirectXMeshCollision::SetWireframeColor(const DWORD color)
{
	DWORD	oldColor(_wireframeColor);

	//  set new color
	_wireframeColor = color;

	//  wireframe mode?
	if ((_renderMode == DXRM_WIREFRAME) && (!_vecVBuffer.empty()))
	{
		_forceNoRender = true;		//  disable rendering

		for (auto pIter=_vecVBuffer.begin(), pEnd=_vecVBuffer.end(); pIter != pEnd; ++pIter)
		{
			(*pIter)->Release();
		}
		_vecVBuffer.clear();

		_forceNoRender = false;		//  enable rendering

	}  //  if (_renderMode == DXRM_WIREFRAME)

	return oldColor;
}

//-----  IncreaseRenderMode()  ------------------------------------------------
DirectXRenderMode DirectXMeshCollision::IncreaseRenderMode()
{
	DirectXRenderMode	oldMode   (GetRenderMode());
	int					renderMode(oldMode + 1);

	if (renderMode >= DXRM_TEXTURE)		renderMode = 0;

	SetRenderMode((DirectXRenderMode) renderMode);
	return oldMode;
}

//-----  SetRenderMode()  -----------------------------------------------------
DirectXRenderMode DirectXMeshCollision::SetRenderMode(const DirectXRenderMode renderMode)
{
	//  check allowed modes
	if (renderMode >= DXRM_TEXTURE)		return _renderMode;

	return DirectXMesh::SetRenderMode(renderMode);
}

//-----  SetPrimitiveType()  --------------------------------------------------
D3DPRIMITIVETYPE DirectXMeshCollision::SetPrimitiveType(const D3DPRIMITIVETYPE type)
{
	D3DPRIMITIVETYPE	oldType(_vecPrimitiveType[0]);

	_vecPrimitiveType.clear();
	for (unsigned int idx(0); idx < _vecVertices.size(); ++idx)
	{
		_vecPrimitiveType.push_back(type);
	}
	return oldType;
}

//-----  SetPrimitiveType()  --------------------------------------------------
D3DPRIMITIVETYPE DirectXMeshCollision::SetPrimitiveType(const vector<D3DPRIMITIVETYPE> vecType)
{
	D3DPRIMITIVETYPE	oldType(_vecPrimitiveType[0]);

	_vecPrimitiveType = vecType;
	return oldType;
}

//-----  Render()  ------------------------------------------------------------
bool DirectXMeshCollision::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if ((_renderMode == DXRM_NONE) || _forceNoRender)		return true;

	//  create DX parameters if not existing
	if (_vecVBuffer.empty())
	{
		for (unsigned int idxShape(0); idxShape < _vecVertices.size(); ++idxShape)
		{
			//  vertices
			LPDIRECT3DVERTEXBUFFER9		pVBuffer     (NULL);
			D3DCustomVertex*			pVModel      (NULL);
			D3DCustomVertex*			pVertices    (_vecVertices[idxShape]);
			unsigned int				countVertices(_vecCountVertices[idxShape]);

			pd3dDevice->CreateVertexBuffer(countVertices*sizeof(D3DCustomVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVBuffer, NULL);
			pVBuffer->Lock(0, 0, (void**)&pVModel, 0);
			for (unsigned int i(0); i < countVertices; ++i)
			{
				pVModel[i]._x     = pVertices[i]._x;
				pVModel[i]._y     = pVertices[i]._y;
				pVModel[i]._z     = pVertices[i]._z;
				pVModel[i]._color = _wireframeColor;
			}
			pVBuffer->Unlock();
			_vecVBuffer.push_back(pVBuffer);

			//  indices
			if (_vecIBuffer.size() < _vecVBuffer.size())
			{
				LPDIRECT3DINDEXBUFFER9	pIBuffer    (NULL);
				unsigned short*			pIModel     (NULL);
				unsigned short*			pIndices    (_vecIndices[idxShape]);
				unsigned int			countIndices(_vecCountIndices[idxShape]);

				pd3dDevice->CreateIndexBuffer(countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIBuffer, NULL);
				pIBuffer->Lock(0, 0, (void**)&pIModel, 0);
				memcpy(pIModel, pIndices, countIndices*sizeof(unsigned short));
				pIBuffer->Unlock();
				_vecIBuffer.push_back(pIBuffer);
			}
		}  // for (unsigned int idxShape(0); idxShape < _vesVertices.size(); ++idxShape)
	}  //  if (_vecVBuffer.empty())

	//  wireframe?
	if (_renderMode == DXRM_WIREFRAME)
	{
		pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
		pd3dDevice->SetTexture          (0, NULL);														//  no texture
		pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);								//  disable alpha blending
		pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);							//  forced wireframe
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);										//  disable light
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style

		for (unsigned int idxShape(0); idxShape < _vecVBuffer.size(); ++idxShape)
		{
			pd3dDevice->SetStreamSource     (0, _vecVBuffer[idxShape], 0, sizeof(D3DCustomVertex));		//  set vertices source
			pd3dDevice->SetIndices          (_vecIBuffer[idxShape]);									//  set indices source

			unsigned int	countIndices (_vecCountIndices[idxShape]);
			unsigned int	countVertices(_vecCountVertices[idxShape]);

			switch (_vecPrimitiveType[idxShape])
			{
				case D3DPT_TRIANGLELIST:
				{
						countIndices  = countIndices/3;
						countVertices = countVertices;
					break;
				}

				case D3DPT_LINELIST:
				{
						countIndices  = countIndices/2;
						countVertices = countVertices;
					break;
				}
			}

			pd3dDevice->DrawIndexedPrimitive(_vecPrimitiveType[idxShape], 0, 0, countVertices, 0, countIndices);		//  render

		}  //  for (unsigned int idxShape(0); idxShape < _vecVBuffer.size(); ++idxShape)
	}
	else if (_renderMode == DXRM_SOLID)
	{
		ZeroMemory(&_material, sizeof(_material));
		_material.Ambient  = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
		_material.Diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		_material.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		pd3dDevice->SetMaterial         (&_material);
		pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

		pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
		pd3dDevice->SetTexture          (0, NULL);														//  no texture
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style

		for (unsigned int idxShape(0); idxShape < _vecVBuffer.size(); ++idxShape)
		{
			pd3dDevice->SetStreamSource     (0, _vecVBuffer[idxShape], 0, sizeof(D3DCustomVertex));		//  set vertices source
			pd3dDevice->SetIndices          (_vecIBuffer[idxShape]);									//  set indices source

			unsigned int	countIndices (_vecCountIndices[idxShape]);
			unsigned int	countVertices(_vecCountVertices[idxShape]);

			switch (_vecPrimitiveType[idxShape])
			{
				case D3DPT_TRIANGLELIST:
				{
						countIndices  = countIndices/3;
						countVertices = countVertices;
					break;
				}

				case D3DPT_LINELIST:
				{
						countIndices  = countIndices/2;
						countVertices = countVertices;
					break;
				}
			}

			pd3dDevice->DrawIndexedPrimitive(_vecPrimitiveType[idxShape], 0, 0, countVertices, 0, countIndices);		//  render
		}

		ZeroMemory(&_material, sizeof(_material));
		_material.Ambient  = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
		_material.Diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		_material.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		pd3dDevice->SetMaterial         (&_material);

		for (unsigned int idxShape(0); idxShape < _vecVBuffer.size(); ++idxShape)
		{
			pd3dDevice->SetStreamSource     (0, _vecVBuffer[idxShape], 0, sizeof(D3DCustomVertex));		//  set vertices source
			pd3dDevice->SetIndices          (_vecIBuffer[idxShape]);									//  set indices source

			unsigned int	countIndices (_vecCountIndices[idxShape]);
			unsigned int	countVertices(_vecCountVertices[idxShape]);

			switch (_vecPrimitiveType[idxShape])
			{
				case D3DPT_TRIANGLELIST:
				{
						countIndices  = countIndices/3;
						countVertices = countVertices;
					break;
				}

				case D3DPT_LINELIST:
				{
						countIndices  = countIndices/2;
						countVertices = countVertices;
					break;
				}
			}

			pd3dDevice->DrawIndexedPrimitive(_vecPrimitiveType[idxShape], 0, 0, countVertices, 0, countIndices);		//  render
		}


	}

	return true;
}
