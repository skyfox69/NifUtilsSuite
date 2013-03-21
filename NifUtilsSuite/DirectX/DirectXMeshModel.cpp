/**
 *  file:   DirectXMeshModel.cpp
 *  class:  DirectXMeshModel
 *
 *  Class representing single mesh of model for drawing in DirectX
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "DirectX\DirectXMeshModel.h"

//-----  DEFINES  -------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX_COLOR		D3DFVF_XYZ | D3DFVF_DIFFUSE
#define D3DFVF_CUSTOMVERTEX_FULL		D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1

//-----  DirectXMeshModel()  --------------------------------------------------
DirectXMeshModel::DirectXMeshModel(D3DXMATRIX transform,
								   D3DMATERIAL9 material,
								   D3DCustomVertexFull* pBufferV,
								   const unsigned int countV,
								   unsigned short* pBufferI,
								   const unsigned int countI,
								   string textureName,
								   DirectXAlphaState* pAlpha,
								   const bool isBillboard,
								   DWORD wireframeColor,
								   const bool doubleSided)
	:	DirectXMesh   (pAlpha),
		_pTexture     (NULL),
		_pVBuffer     (NULL),
		_pIBuffer     (NULL),
		_pVertices    (pBufferV),
		_pIndices     (pBufferI),
		_textureName  (textureName),
		_countVertices(countV),
		_countIndices (countI),
		_isBillboard  (isBillboard),
		_doubleSided  (doubleSided)
{
	_transform      = transform;
	_material       = material;
	_wireframeColor = wireframeColor;
	_renderMode     = textureName.empty() ? DXRM_SOLID : DXRM_TEXTURE;
}

//-----  ~DirectXMeshModel()  -------------------------------------------------
DirectXMeshModel::~DirectXMeshModel()
{
	if (_pVBuffer  != NULL)		_pVBuffer->Release();
	if (_pIBuffer  != NULL)		_pIBuffer->Release();
	if (_pVertices != NULL)		delete[] _pVertices;
	if (_pIndices  != NULL)		delete[] _pIndices;
}

//-----  SetDoubleSided()  ----------------------------------------------------
bool DirectXMeshModel::SetDoubleSided(const bool doubleSided)
{
	bool	oldSided(_doubleSided);

	_doubleSided = doubleSided;
	return oldSided;
}

//-----  SetWireframeColor()  -------------------------------------------------
DWORD DirectXMeshModel::SetWireframeColor(const DWORD color)
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
DirectXRenderMode DirectXMeshModel::IncreaseRenderMode()
{
	DirectXRenderMode	oldMode   (GetRenderMode());
	int					renderMode(oldMode + 1);
	int					maxMode   (_textureName.empty() ? DXRM_SOLID : DXRM_TEXTURE);

	if (renderMode > maxMode)		renderMode = 0;

	SetRenderMode((DirectXRenderMode) renderMode);
	return oldMode;
}

//-----  SetRenderMode()  -----------------------------------------------------
DirectXRenderMode DirectXMeshModel::SetRenderMode(const DirectXRenderMode renderMode)
{
	DirectXRenderMode renderModeInt(renderMode);

	//  check allowed modes
	if (renderModeInt > (_textureName.empty() ? DXRM_SOLID : DXRM_TEXTURE))		renderModeInt = (DirectXRenderMode) (_textureName.empty() ? DXRM_SOLID : DXRM_TEXTURE);

	_forceNoRender = true;		//  disable rendering

	DirectXRenderMode	oldRender(DirectXMesh::SetRenderMode(renderModeInt));

	//  invalidate rendering data on change
	if ((oldRender != renderModeInt) && (_pVBuffer  != NULL))
	{
		_pVBuffer->Release();
		_pVBuffer = NULL;
	}

	_forceNoRender = false;		//  enable rendering

	return oldRender;
}

//-----  Render()  ------------------------------------------------------------
bool DirectXMeshModel::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if ((_renderMode == DXRM_NONE) || _forceNoRender)		return true;

	//  create DX parameters if not existing
	if (_pVBuffer == NULL)
	{
		//  vertices
		if (_renderMode == DXRM_WIREFRAME)
		{
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
		}
		else  //  if (_renderMode == DXRM_WIREFRAME)
		{
			D3DCustomVertexFull*	pVModel(NULL);

			pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertexFull), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
			_pVBuffer->Lock(0, 0, (void**)&pVModel, 0);
			memcpy(pVModel, _pVertices, _countVertices*sizeof(D3DCustomVertexFull));
			_pVBuffer->Unlock();

		}  //  else [if (_renderMode == DXRM_WIREFRAME)]

		//  indices
		if (_pIBuffer == NULL)
		{
			unsigned short*		pIModel(NULL);

			pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
			_pIBuffer->Lock(0, 0, (void**)&pIModel, 0);
			memcpy(pIModel, _pIndices, _countIndices*sizeof(unsigned short));
			_pIBuffer->Unlock();
		}

		//  texture
		if ((_renderMode == DXRM_TEXTURE) && !_textureName.empty())
		{
			if (_pAlpha != NULL)
			{
				D3DXCreateTextureFromFileEx(pd3dDevice,
											CString(_textureName.c_str()).GetString(),
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											NULL,
											D3DFMT_UNKNOWN,//D3DFMT_A8R8G8B8,
											D3DPOOL_MANAGED,
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											0xFF000000,
											NULL,
											NULL,
											&_pTexture);
			}
			else
			{
				D3DXCreateTextureFromFile(pd3dDevice, CString(_textureName.c_str()).GetString(), &_pTexture);
			}
		}  //  if ((_renderMode == DXRM_TEXTURE) && !_textureName.empty())
	}  //  if (_pVBuffer == NULL)

	//  wireframe?
	if (_renderMode == DXRM_WIREFRAME)
	{
		pd3dDevice->SetMaterial         (&_material);													//  set material
		pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
		pd3dDevice->SetTexture          (0, NULL);														//  no texture
		pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);								//  disable alpha blending
		pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);							//  forced wireframe
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);										//  disable light
		pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertex));						//  set vertices source
		pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);	//  render
	}
	else  //  if (_renderMode == DXRM_WIREFRAME)
	{
		//  display texture?
		if ((_renderMode == DXRM_TEXTURE) && (_pTexture != NULL))
		{
			pd3dDevice->SetTexture(0, _pTexture);
		}

		//  render both sides
		if (_doubleSided)
		{
			pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);									//  show both sides of face
		}
		else
		{
			pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);									//  show both sides of face
		}

		pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);											//  enable z buffer

		//  alpha properties?
		if (_pAlpha != NULL)
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, _pAlpha->_enabledBlend);					//  enable alpha blending
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, _pAlpha->_source);								//  source alpha
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, _pAlpha->_destination);							//  destination alpha
			pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, _pAlpha->_argument);					//  alpha source (diffuse/texture)
			pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, _pAlpha->_function);							//  set blend function
			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, _pAlpha->_threshold);							//  set threshold
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, _pAlpha->_enabledTest);					//  enable alpha testing
		}
		else
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);									//  disable alpha blending
			pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);									//  disable alpha testing
		}

		pd3dDevice->SetMaterial         (&_material);													//  set material
		pd3dDevice->SetRenderState		(D3DRS_FILLMODE, D3DFILL_SOLID);								//  solid fill
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, true);											//  enable light
		if (!_isBillboard)
		{
			pd3dDevice->SetTransform     (D3DTS_WORLD, &worldMatrix);									//  set world transformation
			pd3dDevice->MultiplyTransform(D3DTS_WORLD, &_transform);									//  transform local object into world
		}
		else
		{
			D3DXMATRIX		vMatrix;
			D3DXMATRIX		tMatrix;
			D3DXMATRIX		mMatrix;
			D3DXMATRIX		xMatrix;
			D3DXVECTOR3		vScale, vTranslation;
			D3DXQUATERNION	qRotation;

			D3DXMatrixDecompose(&vScale, &qRotation, &vTranslation, &_transform);

			pd3dDevice->GetTransform(D3DTS_VIEW, &vMatrix);
			D3DXMatrixTranspose(&tMatrix, &vMatrix);
			D3DXMatrixMultiply (&mMatrix, &_transform, &worldMatrix);

			tMatrix._41 = mMatrix._41;
			tMatrix._42 = mMatrix._42;
			tMatrix._43 = mMatrix._43;

			D3DXMatrixScaling(&xMatrix, vScale.x, vScale.y, vScale.z);
			tMatrix *= xMatrix;

			pd3dDevice->SetTransform    (D3DTS_WORLD, &tMatrix);										//  set world transformation
		}
		pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexFull));					//  set vertices source
		pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_FULL);										//  set vertex style
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);	//  render
		pd3dDevice->SetTexture          (0, NULL);

	}  //  else [if (_renderMode == DXRM_WIREFRAME)]

	return true;
}
