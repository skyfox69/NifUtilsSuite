/**
 *  file:   DirectXMeshAxes.cpp
 *  class:  DirectXMeshAxes
 *
 *  Class representing coordinate axes for drawing in DirectX
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "DirectX\DirectXMeshAxes.h"

//-----  DEFINES  -------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX_COLOR		D3DFVF_XYZ | D3DFVF_DIFFUSE

static DirectXMeshAxes::D3DCustomVertex		vertPts[] = {{-500.0f,   0.0f,   0.0f, 0x000000},
														 {  -1.0f,   0.0f,   0.0f, 0x000000},
														 {   1.0f,   0.0f,   0.0f, 0x000000},
														 { 500.0f,   0.0f,   0.0f, 0x000000},
														 { 450.0f,  30.0f,  30.0f, 0x000000},
														 { 450.0f, -30.0f,  30.0f, 0x000000},
														 { 450.0f, -30.0f, -30.0f, 0x000000},
														 { 450.0f,  30.0f, -30.0f, 0x000000}
														};

static unsigned int vertIdx[] = {0, 1, 2, 3, 4, 3, 5, 3, 6, 3, 7, 3};

//-----  DirectXMeshAxes()  ---------------------------------------------------
DirectXMeshAxes::DirectXMeshAxes()
	:	DirectXMesh   (),
		_pVBuffer     (NULL),
		_pIBuffer     (NULL),
		_countVertices(24),
		_countIndices (36)
{
	SetNifData("Coordinate System", "internal", -1);
}

//-----  ~DirectXMeshAxes()  --------------------------------------------------
DirectXMeshAxes::~DirectXMeshAxes()
{
	if (_pVBuffer != NULL)		_pVBuffer->Release();
	if (_pIBuffer != NULL)		_pIBuffer->Release();
}

//-----  IncreaseRenderMode()  ------------------------------------------------
DirectXRenderMode DirectXMeshAxes::IncreaseRenderMode()
{
	DirectXRenderMode	oldMode   (GetRenderMode());
	int					renderMode(oldMode + 1);

	if (renderMode >= DXRM_SOLID)	renderMode = 0;

	SetRenderMode((DirectXRenderMode) renderMode);
	return oldMode;
}

//-----  SetRenderMode()  -----------------------------------------------------
DirectXRenderMode DirectXMeshAxes::SetRenderMode(const DirectXRenderMode renderMode)
{
	//  check allowed modes
	if (renderMode >= DXRM_SOLID)		return _renderMode;

	return DirectXMesh::SetRenderMode(renderMode);
}

//-----  Render()  ------------------------------------------------------------
bool DirectXMeshAxes::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if ((_renderMode == DXRM_NONE) || _forceNoRender)		return true;

	//  create DX parameters if not existing
	if (_pVBuffer == NULL)
	{
		D3DCustomVertex*	pVAxis(NULL);
		unsigned short*		pIAxis(NULL);

		//  vertices
		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
		_pVBuffer->Lock(0, 0, (void**)&pVAxis, 0);

		for (short i(0); i < 8; ++i)
		{
			//  x-axis
			pVAxis[i]._x     = vertPts[i]._x;
			pVAxis[i]._y     = vertPts[i]._y;
			pVAxis[i]._z     = vertPts[i]._z;
			pVAxis[i]._color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);

			//  y-axis
			pVAxis[i+8]._x     = vertPts[i]._y;
			pVAxis[i+8]._y     = vertPts[i]._x;
			pVAxis[i+8]._z     = vertPts[i]._z;
			pVAxis[i+8]._color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);

			//  z-axis
			pVAxis[i+16]._x     = vertPts[i]._z;
			pVAxis[i+16]._y     = -vertPts[i]._y;
			pVAxis[i+16]._z     = vertPts[i]._x;
			pVAxis[i+16]._color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
		}

		_pVBuffer->Unlock();

		//  indices
		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIAxis, 0);

		for (short i(0); i < 12; ++i)
		{
			//  x-axis
			pIAxis[i] = vertIdx[i];

			//  y-axis
			pIAxis[i+12] = vertIdx[i] + 8;

			//  z-axis
			pIAxis[i+24] = vertIdx[i] + 16;
		}

		_pIBuffer->Unlock();

	}  //  if (_pVBuffer != NULL)

	//  render mesh
	pd3dDevice->SetTexture          (0, NULL);													//  no texture
	pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);							//  disable alpha blending
	pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);						//  forced wireframe
	pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);									//  disable light
	pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);								//  set world transformation
	pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertex));					//  set vertices source
	pd3dDevice->SetIndices          (_pIBuffer);												//  set indices source
	pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);								//  set vertex style
	pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, _countIndices, 0, _countIndices/2);	//  render

	return true;
}
