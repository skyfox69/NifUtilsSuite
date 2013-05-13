/**
 *  file:   DirectXNifConverter.cpp
 *  class:  DirectXNifConverter
 *
 *  Class converting a NIF model into DirectX readable one
 */

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <sstream>

//  NifUtils includes
#include "DirectX\DirectXNifConverter.h"
#include "DirectX\DirectXMeshModelLOD.h"
#include "DirectX\DirectXMeshCollision.h"
#include "Common\Util\FDCLibHelper.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifChunkData.h"

//  niflib includes
#include "niflib.h"
#include "obj/NiBillboardNode.h"
#include "obj/RootCollisionNode.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/bhkRigidBodyT.h"
#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkCompressedMeshShapeData.h"
#include "obj/BSLODTriShape.h"
#include "obj/NiTriStrips.h"
#include "obj/NiTriStripsData.h"
#include "obj/hkPackedNiTriStripsData.h"

//-----  DEFINES  -------------------------------------------------------------

//-----  DirectXNifConverter()  -----------------------------------------------
DirectXNifConverter::DirectXNifConverter()
	:	_defWireframeColor(0x00FFFFFF),
		_defCollisionColor(0x00FFFF00),
		_defAmbientColor  (0x00707070),
		_defDiffuseColor  (0x00E5E5E5),
		_defSpecularColor (0x00FFFFFF),
		_lodRenderLevel   (2),
		_isBillboard      (false),
		_isCollision      (false),
		_forceDDS         (false),
		_showModel        (true),
		_showCollision    (true),
		_doubleSided      (true)
{}

//-----  ~DirectXNifConverter()  ----------------------------------------------
DirectXNifConverter::~DirectXNifConverter()
{}

//-----  SetTexturePathList()  ------------------------------------------------
void DirectXNifConverter::SetTexturePathList(vector<string> texturePathList)
{
	_texturePathList = texturePathList;
}

//-----  SetForceDDS()  -------------------------------------------------------
bool DirectXNifConverter::SetForceDDS(const bool forceDDS)
{
	bool	oldForce(_forceDDS);

	_forceDDS = forceDDS;
	return oldForce;
}

//-----  SetShowModel()  ------------------------------------------------------
bool DirectXNifConverter::SetShowModel(const bool doShow)
{
	bool	oldShow(_showModel);

	_showModel = doShow;
	return oldShow;
}

//-----  SetShowCollision()  --------------------------------------------------
bool DirectXNifConverter::SetShowCollision(const bool doShow)
{
	bool	oldShow(_showCollision);

	_showCollision = doShow;
	return oldShow;
}

//-----  SetDefaultWireframeColor()  ------------------------------------------
DWORD DirectXNifConverter::SetDefaultWireframeColor(const DWORD color)
{
	DWORD	oldColor(_defWireframeColor);

	_defWireframeColor = color;
	return oldColor;
}

//-----  SetDefaultCollisionColor()  ------------------------------------------
DWORD DirectXNifConverter::SetDefaultCollisionColor(const DWORD color)
{
	DWORD	oldColor(_defCollisionColor);

	_defCollisionColor = color;
	return oldColor;
}

//-----  SetDefaultAmbientColor()  --------------------------------------------
DWORD DirectXNifConverter::SetDefaultAmbientColor(const DWORD color)
{
	DWORD	oldColor(_defAmbientColor);

	_defAmbientColor = color;
	return oldColor;
}

//-----  SetDefaultDiffuseColor()  --------------------------------------------
DWORD DirectXNifConverter::SetDefaultDiffuseColor(const DWORD color)
{
	DWORD	oldColor(_defDiffuseColor);

	_defDiffuseColor = color;
	return oldColor;
}

//-----  SetDefaultSpecularColor()  -------------------------------------------
DWORD DirectXNifConverter::SetDefaultSpecularColor(const DWORD color)
{
	DWORD	oldColor(_defSpecularColor);

	_defSpecularColor = color;
	return oldColor;
}

//-----  SetDefaultCollisionColor()  ------------------------------------------
unsigned int DirectXNifConverter::SetDefaultLODRenderLevel(const unsigned int level)
{
	unsigned int	oldLevel(_lodRenderLevel);

	_lodRenderLevel = level;
	return oldLevel;
}

//-----  SetDoubleSided()  ----------------------------------------------------
bool DirectXNifConverter::SetDoubleSided(const bool doubleSided)
{
	bool	oldSided(_doubleSided);

	_doubleSided = doubleSided;
	return oldSided;
}

//-----  getRootNodeFromNifFile()  --------------------------------------------
NiNodeRef DirectXNifConverter::getRootNodeFromNifFile(string fileName)
{
	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str());

	//  NiNode as root
	if (DynamicCast<NiNode>(pRootTree) != NULL)
	{
		pRootInput = DynamicCast<NiNode>(pRootTree);
	}
	//  NiTriShape as root
	else if (DynamicCast<NiTriShape>(pRootTree) != NULL)
	{
		//  create faked root
		pRootInput = new NiNode();

		//  add root as child
		pRootInput->AddChild(DynamicCast<NiAVObject>(pRootTree));
	}

	return pRootInput;
}

//-----  getGeometryFromNode()  -----------------------------------------------
unsigned int DirectXNifConverter::getGeometryFromNode(NiNodeRef pNode, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());

	//  check for NiBillboardNode
	_isBillboard = (DynamicCast<NiBillboardNode>(pNode) != NULL);

	//  add own translation to list
	transformAry.push_back(pNode->GetLocalTransform());

	//  iterate over children
	for (auto pIter=childList.begin(), pEnd=childList.end(); pIter != pEnd; ++pIter)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*pIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriBasedGeom>(*pIter), meshList, transformAry, pTmplAlphaProp);
		}
		//  BSLODTriShape
		else if (DynamicCast<BSLODTriShape>(*pIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriBasedGeom>(*pIter), meshList, transformAry, pTmplAlphaProp);
		}
		//  NiTriStrips
		else if (DynamicCast<NiTriStrips>(*pIter) != NULL)
		{
			getGeometryFromTriStrips(DynamicCast<NiTriBasedGeom>(*pIter), meshList, transformAry, pTmplAlphaProp);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*pIter) != NULL)
		{
			//  set collision flag
			_isCollision = true;

			//  recurse sub-tree
			getGeometryFromNode(DynamicCast<NiNode>(*pIter), meshList, transformAry, pTmplAlphaProp);

			//  reset collision flag
			_isCollision = false;
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*pIter) != NULL)
		{
			//  find NiAlphaProperty and use as template in sub-nodes
			if (DynamicCast<NiAlphaProperty>((DynamicCast<NiNode>(*pIter))->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
			{
				pTmplAlphaProp = DynamicCast<NiAlphaProperty>((DynamicCast<NiNode>(*pIter))->GetPropertyByType(NiAlphaProperty::TYPE));
			}

			getGeometryFromNode(DynamicCast<NiNode>(*pIter), meshList, transformAry, pTmplAlphaProp);
		}
	}  //  for (auto pIter=childList.begin(), pEnd=childList.end(); pIter != pEnd; ++pIter)

	//  RootCollisionNode
	if (DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()) != NULL)
	{
		//  set collision flag
		_isCollision = true;

		//  recurse sub-tree
		getGeometryFromCollisionObject(DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()), meshList, transformAry, NULL);

		//  reset collision flag
		_isCollision = false;
	}

	//  reset billboard flag
	_isBillboard = false;

	//  remove own translation from list
	transformAry.pop_back();

	return meshList.size();
}

//-----  getGeometryFromTriShape()  -------------------------------------------
unsigned int DirectXNifConverter::getGeometryFromTriShape(NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		vector<TexCoord>	vecTexCoords;
		vector<Vector3>		vecVertices (pData->GetVertices());
		vector<Triangle>	vecTriangles(pData->GetTriangles());
		vector<Vector3>		vecNormals  (pData->GetNormals());
		vector<Color4>		vecColors   (pData->GetColors());

		//  get uv set
		if (pData->GetUVSetCount() > 0)			vecTexCoords = pData->GetUVSet(0);

		//  get geometry from data
		getGeometryFromData(vecVertices, vecTriangles, vecNormals, vecColors, vecTexCoords, pShape, meshList, transformAry, pTmplAlphaProp);

	}  //  if (pData != NULL)

	return meshList.size();
}

//-----  getGeometryFromTriStrips()  ------------------------------------------
unsigned int DirectXNifConverter::getGeometryFromTriStrips(NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiTriStripsDataRef	pData(DynamicCast<NiTriStripsData>(pShape->GetData()));

	if (pData != NULL)
	{
		vector<TexCoord>	vecTexCoords;
		vector<Vector3>		vecVertices (pData->GetVertices());
		vector<Triangle>	vecTriangles(pData->GetTriangles());
		vector<Vector3>		vecNormals  (pData->GetNormals());
		vector<Color4>		vecColors   (pData->GetColors());

		//  get uv set
		if (pData->GetUVSetCount() > 0)			vecTexCoords = pData->GetUVSet(0);

		//  get geometry from data
		getGeometryFromData(vecVertices, vecTriangles, vecNormals, vecColors, vecTexCoords, pShape, meshList, transformAry, pTmplAlphaProp);

	}  //  if (pData != NULL)

	return meshList.size();
}

//-----  getGeometryFromData()  -----------------------------------------------
unsigned int DirectXNifConverter::getGeometryFromData(vector<Vector3>& vecVertices, vector<Triangle>& vecTriangles, vector<Vector3>& vecNormals, vector<Color4>& vecColors, vector<TexCoord>& vecTexCoords, NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	BSLODTriShapeRef		pShapeLOD    (DynamicCast<BSLODTriShape> (pShape));
	DirectXAlphaState*		pAlpha       (NULL);
	DirectXMesh*			pAddModel    (NULL);
	D3DMATERIAL9			material;
	vector<NiPropertyRef>	propList    (pShape->GetProperties());
	array<2, NiPropertyRef>	propListBS  (pShape->GetBSProperties());
	Matrix44				locTransform(pShape->GetLocalTransform());
	string					name        (pShape->GetName());
	string					type        (pShape->GetType().GetTypeName());
	string					baseTexture;
	unsigned int			texCoordSize(vecTexCoords.size());
	bool					hasMaterial (false);

	//  parse properties (old style)
	for (auto pIter=propList.begin(), pEnd=propList.end(); pIter != pEnd; ++pIter)
	{
		//  NiTexturingProperty
		if (DynamicCast<NiTexturingProperty>(*pIter) != NULL)
		{
			TexDesc		baseTex((DynamicCast<NiTexturingProperty>(*pIter))->GetTexture(BASE_MAP));

			baseTexture = CheckTextureName(baseTex.source->GetTextureFileName());
		}
		//  NiAlphaProperty
		else if (DynamicCast<NiAlphaProperty>(*pIter) != NULL)
		{
			pAlpha = DecodeAlphaProperty(DynamicCast<NiAlphaProperty>(*pIter));
		}
		//  NiMaterialProperty
		else if (DynamicCast<NiMaterialProperty>(*pIter) != NULL)
		{
			NiMaterialProperty*	pProp(DynamicCast<NiMaterialProperty>(*pIter));
			Color3				tColor;

			tColor = pProp->GetAmbientColor();		material.Ambient  = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
			tColor = pProp->GetDiffuseColor();		material.Diffuse  = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
			tColor = pProp->GetEmissiveColor();		material.Emissive = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
			tColor = pProp->GetSpecularColor();		material.Specular = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
													material.Power    = pProp->GetGlossiness();

			hasMaterial = true;
		}
	}  //  for (auto pIter=propList.begin(), pEnd=propList.end(); pIter != pEnd; ++pIter)

	//  parse properties (new style)
	for (short idx(0); idx < 2; ++idx)
	{
		//  BSLightingShaderProperty
		if (DynamicCast<BSLightingShaderProperty>(propListBS[idx]) != NULL)
		{
			BSShaderTextureSet*	pTexSet((DynamicCast<BSLightingShaderProperty>(propListBS[idx]))->GetTextureSet());

			if (pTexSet != NULL)
			{
				baseTexture = CheckTextureName(pTexSet->GetTexture(0));
			}
		}
		//  NiAlphaProperty
		else if (DynamicCast<NiAlphaProperty>(propListBS[idx]) != NULL)
		{
			pAlpha = DecodeAlphaProperty(DynamicCast<NiAlphaProperty>(propListBS[idx]));
		}
	}  //  for (short idx(0); idx < 2; ++idx)

	//  collected all data needed => convert to DirectX
	//  - transformation matrix
	for (auto pIter=transformAry.rbegin(), pEnd=transformAry.rend(); pIter != pEnd; ++pIter)
	{
		locTransform *= *pIter;
	}

	if (_isBillboard)
	{
		float		scale(locTransform.GetScale());
		Vector3		trans(locTransform.GetTranslation());

		locTransform = Matrix44(trans, Matrix33(), scale);
	}

	//  - indices
	unsigned int		countI     (vecTriangles.size()*3);
	unsigned short*		pBufIndices(new unsigned short[countI]);

	for (unsigned int i(0); i < countI; i+=3)
	{
		pBufIndices[i]   = vecTriangles[i/3].v1;
		pBufIndices[i+1] = vecTriangles[i/3].v2;
		pBufIndices[i+2] = vecTriangles[i/3].v3;
	}

	//  in case of non-collision
	if (!_isCollision)
	{
		//  - vertices
		unsigned int							countV      (vecVertices.size());
		DirectXMeshModel::D3DCustomVertexFull*	pBufVertices(new DirectXMeshModel::D3DCustomVertexFull[countV]);
		DirectXMeshModel*						pNewModel   (NULL);

		for (unsigned int i(0); i < countV; ++i)
		{
			pBufVertices[i]._x        = vecVertices[i].x;
			pBufVertices[i]._y        = vecVertices[i].y;
			pBufVertices[i]._z        = vecVertices[i].z;
			pBufVertices[i]._normal.x = vecNormals[i].x;
			pBufVertices[i]._normal.y = vecNormals[i].y;
			pBufVertices[i]._normal.z = vecNormals[i].z;
			pBufVertices[i]._color    = !vecColors.empty() ? D3DXCOLOR(vecColors[i].r, vecColors[i].g, vecColors[i].b, 1.0f) : D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			pBufVertices[i]._u        = (i < texCoordSize) ? vecTexCoords[i].u : 0.0f;
			pBufVertices[i]._v        = (i < texCoordSize) ? vecTexCoords[i].v : 0.0f;
		}

		//  - material
		if (!hasMaterial)
		{
			ZeroMemory(&material, sizeof(material));
			material.Ambient  = D3DXCOLOR(_defAmbientColor);
			material.Diffuse  = D3DXCOLOR(_defDiffuseColor);
			material.Specular = D3DXCOLOR(_defSpecularColor);
		}

		//  - alpha
		if ((pAlpha == NULL) && (pTmplAlphaProp != NULL))
		{
			pAlpha = DecodeAlphaProperty(pTmplAlphaProp);
		}

		//  create new model
		if (pShapeLOD == NULL)
		{
			pNewModel = new DirectXMeshModel(Matrix44ToD3DXMATRIX(locTransform),
																	material,
																	pBufVertices,
																	countV,
																	pBufIndices,
																	countI,
																	baseTexture,
																	pAlpha,
																	_isBillboard,
																	_defWireframeColor,
																	_doubleSided
																	);
		}
		else
		{
			unsigned int	lodLevel[3] = { 0 };

			lodLevel[0] = pShapeLOD->GetLODLevelSize(0);
			lodLevel[1] = pShapeLOD->GetLODLevelSize(1) + lodLevel[0];
			lodLevel[2] = pShapeLOD->GetLODLevelSize(2) + lodLevel[1];

			pNewModel = new DirectXMeshModelLOD(Matrix44ToD3DXMATRIX(locTransform),
																		material,
																		pBufVertices,
																		countV,
																		pBufIndices,
																		countI,
																		baseTexture,
																		pAlpha,
																		_isBillboard,
																		_defWireframeColor,
																		_doubleSided,
																		lodLevel
																	);

			//  seet default render LOD
			pNewModel->SetLODRenderLevel(_lodRenderLevel);
		}

		//  model view data
		if (type.empty())		type = "- unknown -";
		if (name.empty())
		{
			stringstream	sStream;

			sStream << "Block-ID: " << pShape->internal_block_number;
			name = sStream.str();
		}

		//  set visibility
		if (!_showModel)	pNewModel->SetRenderMode(DXRM_NONE);

		//  set model to add
		pAddModel = pNewModel;
	}
	else  //  if (!_isCollision)
	{
		//  - vertices
		unsigned int							countV      (vecVertices.size());
		DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

		for (unsigned int i(0); i < countV; ++i)
		{
			pBufVertices[i]._x     = vecVertices[i].x;
			pBufVertices[i]._y     = vecVertices[i].y;
			pBufVertices[i]._z     = vecVertices[i].z;
			pBufVertices[i]._color = _defCollisionColor;
		}

		//  create new model
		DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																		pBufVertices,
																		countV,
																		pBufIndices,
																		countI,
																		_defCollisionColor
																	);
		//  model view data
		if (type.empty())		type = "- unknown -";
		if (name.empty())		name = "Collision";

		//  set visibility
		if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

		//  set model to add
		pAddModel = pNewModel;

	}  //  else [if (!_isCollision)]

	//  add model view data
	pAddModel->SetNifData(name, type, pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pAddModel);

	return meshList.size();
}

//-----  getGeometryFromCollisionObject()  ------------------------------------
unsigned int DirectXNifConverter::getGeometryFromCollisionObject(bhkCollisionObjectRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	//  search for embedded shape
	bhkRigidBodyRef		pRBody(DynamicCast<bhkRigidBody>(pShape->GetBody()));
	if (pRBody == NULL)		return meshList.size(); 

	//  check on extra bhkMoppBvTreeShape
	bhkShapeRef		pTShape(pRBody->GetShape());
	if (DynamicCast<bhkMoppBvTreeShape>(pTShape))
	{
		pTShape = DynamicCast<bhkMoppBvTreeShape>(pTShape)->GetShape();
	}

	//  check type of collision mesh
	//bhkCompressedMeshShape
	if (DynamicCast<bhkCompressedMeshShape>(pTShape) != NULL)
	{
		getGeometryFromCompressedMeshShape(DynamicCast<bhkCompressedMeshShape>(pTShape), pShape->GetBody(), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkPackedNiTriStripsShape
	else if (DynamicCast<bhkPackedNiTriStripsShape>(pTShape) != NULL)
	{
		getGeometryFromPackedTriStripsShape(DynamicCast<bhkPackedNiTriStripsShape>(pTShape), pShape->GetBody(), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkConvexVerticesShape
	else if (DynamicCast<bhkConvexVerticesShape>(pTShape) != NULL)
	{
		getGeometryFromConvexVerticesShape(DynamicCast<bhkConvexVerticesShape>(pTShape), pShape->GetBody(), meshList, transformAry, pTmplAlphaProp);
	}






	return meshList.size();
}

//-----  getGeometryFromConvexVerticesShape()  --------------------------------
unsigned int DirectXNifConverter::getGeometryFromConvexVerticesShape(bhkConvexVerticesShapeRef pShape, NiObjectRef pBody, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	bhkRigidBodyT*		pRBody      (DynamicCast<bhkRigidBodyT>(pBody));
	vector<Vector3>		vecVertices (pShape->GetVertices());
	Matrix44			locTransform;
	float				factor      (7.0f);

	//  add RigidBody translation to list - if bhkRigidBodyT
	if (pRBody != NULL)
	{
		Vector4		tVec4 (pRBody->GetTranslation());
		Vector3		tVec3 (tVec4.x * factor, tVec4.y * factor, tVec4.z * factor);
		Matrix33	tMat33(QuaternionToMatrix33(pRBody->GetRotation()));
		Matrix44	tMat44(tVec3, tMat33, 1.0f);
	
		transformAry.push_back(tMat44);
	}

	//  - indices
	unsigned int		countI     (vecVertices.size() * (vecVertices.size() - 1));
	unsigned int		indexI     (0);
	unsigned short*		pBufIndices(new unsigned short[countI]);

	for (unsigned int i(1); i < vecVertices.size(); ++i)
	{
		for (unsigned int j(0); j < i; ++j)
		{
			pBufIndices[indexI++] = i;
			pBufIndices[indexI++] = j;
		}
	}

	//  - vertices
	unsigned int							countV      (vecVertices.size());
	DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

	for (unsigned int i(0); i < countV; ++i)
	{
		pBufVertices[i]._x     = vecVertices[i].x * factor;
		pBufVertices[i]._y     = vecVertices[i].y * factor;
		pBufVertices[i]._z     = vecVertices[i].z * factor;
		pBufVertices[i]._color = _defCollisionColor;
	}

	//  collected all data needed => convert to DirectX
	//  - transformation matrix
	for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
	{
		locTransform *= *pIterT;
	}

	//  create new model
	DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																	pBufVertices,
																	countV,
																	pBufIndices,
																	countI,
																	_defCollisionColor
																);
	//  modify primitive type
	pNewModel->SetPrimitiveType(D3DPT_LINELIST);

	//  set visibility
	if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

	//  add model view data
	pNewModel->SetNifData("unknown", "bhkConvexVertices", pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pNewModel);

	//  remove RigidBody translation from list - if set
	if (pRBody != NULL)
	{
		transformAry.pop_back();
	}

	return meshList.size();
}

//-----  getGeometryFromCompressedMeshShape()  --------------------------------
unsigned int DirectXNifConverter::getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeRef pShape, NiObjectRef pBody, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	bhkCompressedMeshShapeData*	pData(DynamicCast<bhkCompressedMeshShapeData>(pShape->GetData()));
	if (pData == NULL)		return meshList.size();

	//  hurray!!!
	bhkRigidBodyT*				pRBody      (DynamicCast<bhkRigidBodyT>(pBody));
	vector<bhkCMSDChunk>		chunkGeoList(pData->GetChunks());
	vector<bhkCMSDMaterial>		chunkMatList(pData->GetChunkMaterials());
	float						factor      (70.0f);
	unsigned short				idxChunk    (0);

	//  add RigidBody translation to list - if bhkRigidBodyT
	if (pRBody != NULL)
	{
		Vector4		tVec4 (pRBody->GetTranslation());
		Vector3		tVec3 (tVec4.x * factor, tVec4.y * factor, tVec4.z * factor);
		Matrix33	tMat33(QuaternionToMatrix33(pRBody->GetRotation()));
		Matrix44	tMat44(tVec3, tMat33, 1.0f);
	
		transformAry.push_back(tMat44);
	}

	//  for each chunk of shape
	for (auto pIter=chunkGeoList.begin(), pEnd=chunkGeoList.end(); pIter != pEnd; ++pIter, ++idxChunk)
	{
		vector<Vector3>		vecVertices;
		vector<Triangle>	vecTriangles;
		Matrix44			locTransform;
		unsigned int		offI(0);

		//  get vertices from chunk
		for (unsigned int idx(0); idx < pIter->numVertices; idx += 3)
		{
			vecVertices.push_back(Vector3((pIter->vertices[idx  ] / 1000.0f) + pIter->translation.x,
			                              (pIter->vertices[idx+1] / 1000.0f) + pIter->translation.y,
			                              (pIter->vertices[idx+2] / 1000.0f) + pIter->translation.z
			                             ));
		}

		//  get faces defined by strip groups
		for (unsigned int idxS(0); idxS < pIter->numStrips; ++idxS)
		{
			unsigned int	cntI(pIter->strips[idxS]);

			for (unsigned int idxI(offI), swap(0); idxI < (offI + cntI - 2); ++idxI, ++swap)
			{
				if ((swap % 2) == 0)
				{
					vecTriangles.push_back(Triangle(pIter->indices[idxI], pIter->indices[idxI+1], pIter->indices[idxI+2]));
				}
				else
				{
					vecTriangles.push_back(Triangle(pIter->indices[idxI+2], pIter->indices[idxI+1], pIter->indices[idxI]));
				}
			}
			
			//  set next index
			offI += cntI;

		}  //  for (unsigned int idxS(0); idxS < pIter->numStrips; ++idxS)

		//  get faces defined by indices only
		for (unsigned int idxI(offI); idxI < pIter->numIndices; idxI += 3, offI += 3)
		{
			vecTriangles.push_back(Triangle(pIter->indices[idxI], pIter->indices[idxI+1], pIter->indices[idxI+2]));
		}

		//  - indices
		unsigned int		countI     (vecTriangles.size()*3);
		unsigned short*		pBufIndices(new unsigned short[countI]);

		for (unsigned int i(0); i < countI; i+=3)
		{
			pBufIndices[i]   = vecTriangles[i/3].v1;
			pBufIndices[i+1] = vecTriangles[i/3].v2;
			pBufIndices[i+2] = vecTriangles[i/3].v3;
		}

		//  - vertices
		unsigned int							countV      (vecVertices.size());
		DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

		for (unsigned int i(0); i < countV; ++i)
		{
			pBufVertices[i]._x     = vecVertices[i].x * factor;
			pBufVertices[i]._y     = vecVertices[i].y * factor;
			pBufVertices[i]._z     = vecVertices[i].z * factor;
			pBufVertices[i]._color = _defCollisionColor;
		}

		//  collected all data needed => convert to DirectX
		//  - transformation matrix
		for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
		{
			locTransform *= *pIterT;
		}

		//  create new model
		DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																		pBufVertices,
																		countV,
																		pBufIndices,
																		countI,
																		_defCollisionColor
																	);
		//  set visibility
		if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

		//  add model view data
		stringstream	sStream;

		sStream << "Chunk #" << idxChunk << ": " << NifUtlMaterialList::getInstance()->getMaterialDefName(chunkMatList[pIter->materialIndex].skyrimMaterial);
		pNewModel->SetNifData(sStream.str(), "bhkCMSDChunk", pShape->internal_block_number);

		//  append model to list
		meshList.push_back(pNewModel);

	}  //  for (auto pIter=chunkGeoList.begin(), pEnd=chunkGeoList.end(); pIter != pEnd; ++pIter)

	//  decode BigTris
	vector<bhkCMSDBigTris>		tBTriVec(pData->GetBigTris());
	vector<Vector4>				tBVecVec(pData->GetBigVerts());

	//  if existing
	if (!tBVecVec.empty() && !tBTriVec.empty())
	{
		NifChunkData*						pTmpCData(NULL);
		vector<NifChunkData>				chunkDataList;
		map<unsigned int, unsigned int>		knownMaterials;

		//  for each face
		for (auto pIter=tBTriVec.begin(), pEnd=tBTriVec.end(); pIter != pEnd; ++pIter)
		{
			//  look for known material mesh
			if (knownMaterials.count(pIter->unknownInt1) <= 0)
			{
				NifChunkData	tmpCData;
			
				//  initialize data structure
				tmpCData._material = chunkMatList[pIter->unknownInt1].skyrimMaterial;
				tmpCData._index    = -1;

				//  add new mesh to known materials
				knownMaterials[pIter->unknownInt1] = chunkDataList.size();

				// append geometry to list
				chunkDataList.push_back(tmpCData);
			}

			//  get known mesh
			pTmpCData = &chunkDataList[knownMaterials[pIter->unknownInt1]];

			//  vertices
			short	idxP1(pTmpCData->_vertices.size());
			pTmpCData->_vertices.push_back(Vector3(tBVecVec[pIter->triangle1].x,
												   tBVecVec[pIter->triangle1].y,
												   tBVecVec[pIter->triangle1].z
												  ));

			short	idxP2(pTmpCData->_vertices.size());
			pTmpCData->_vertices.push_back(Vector3(tBVecVec[pIter->triangle2].x,
												   tBVecVec[pIter->triangle2].y,
												   tBVecVec[pIter->triangle2].z
												  ));

			short	idxP3(pTmpCData->_vertices.size());
			pTmpCData->_vertices.push_back(Vector3(tBVecVec[pIter->triangle3].x,
												   tBVecVec[pIter->triangle3].y,
												   tBVecVec[pIter->triangle3].z
												  ));

			//  triangle
			pTmpCData->_triangles.push_back(Triangle(idxP1, idxP2, idxP3));

		}  //  for (auto pIter=tBTriVec.begin(), pEnd=tBTriVec.end(); pIter != pEnd; ++pIter)

		//  convert each entry in chunkDataList to DirectX
		for (auto pIter=chunkDataList.begin(), pEnd=chunkDataList.end(); pIter != pEnd; ++pIter)
		{
			Matrix44	locTransform;

			//  - indices
			unsigned int		countI     (pIter->_triangles.size()*3);
			unsigned short*		pBufIndices(new unsigned short[countI]);

			for (unsigned int i(0); i < countI; i+=3)
			{
				pBufIndices[i]   = pIter->_triangles[i/3].v1;
				pBufIndices[i+1] = pIter->_triangles[i/3].v2;
				pBufIndices[i+2] = pIter->_triangles[i/3].v3;
			}

			//  - vertices
			unsigned int							countV      (pIter->_vertices.size());
			DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

			for (unsigned int i(0); i < countV; ++i)
			{
				pBufVertices[i]._x     = pIter->_vertices[i].x * factor;
				pBufVertices[i]._y     = pIter->_vertices[i].y * factor;
				pBufVertices[i]._z     = pIter->_vertices[i].z * factor;
				pBufVertices[i]._color = _defCollisionColor;
			}

			//  collected all data needed => convert to DirectX
			//  - transformation matrix
			for (vector<Matrix44>::iterator pIterT=transformAry.begin(); pIterT != transformAry.end(); ++pIterT)
			{
				locTransform *= *pIterT;
			}

			//  create new model
			DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																			pBufVertices,
																			countV,
																			pBufIndices,
																			countI,
																			_defCollisionColor
																		);
			//  set visibility
			if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

			//  add model view data
			stringstream	sStream;

			sStream << "BigTri: " << NifUtlMaterialList::getInstance()->getMaterialDefName(pIter->_material);
			pNewModel->SetNifData(sStream.str(), "bhkCMSDBigTris", pShape->internal_block_number);

			//  append model to list
			meshList.push_back(pNewModel);

		}  //  for (auto pIter=chunkDataList.begin(), pEnd=chunkDataList.end(); pIter != pEnd; ++pIter)
	}  //  if (!tBVecVec.empty() && !tBTriVec.empty())

	//  remove RigidBody translation from list - if set
	if (pRBody != NULL)
	{
		transformAry.pop_back();
	}

	return meshList.size();
}

//-----  getGeometryFromPackedTriStripsShape()  -------------------------------
unsigned int DirectXNifConverter::getGeometryFromPackedTriStripsShape(bhkPackedNiTriStripsShapeRef pShape, NiObjectRef pBody, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	hkPackedNiTriStripsData*	pData(DynamicCast<hkPackedNiTriStripsData>(pShape->GetData()));
	if (pData == NULL)		return meshList.size();

	//  hurray!!!
	bhkRigidBodyT*		pRBody      (DynamicCast<bhkRigidBodyT>(pBody));
	vector<Vector3>		vecVertices (pData->GetVertices());
	vector<Triangle>	vecTriangles(pData->GetTriangles());
	Matrix44			locTransform;
	float				factor      (7.0f);

	//  add RigidBody translation to list - if bhkRigidBodyT
	if (pRBody != NULL)
	{
		Vector4		tVec4 (pRBody->GetTranslation());
		Vector3		tVec3 (tVec4.x * factor, tVec4.y * factor, tVec4.z * factor);
		Matrix33	tMat33(QuaternionToMatrix33(pRBody->GetRotation()));
		Matrix44	tMat44(tVec3, tMat33, 1.0f);
	
		transformAry.push_back(tMat44);
	}

	//  - indices
	unsigned int		countI     (vecTriangles.size()*3);
	unsigned short*		pBufIndices(new unsigned short[countI]);

	for (unsigned int i(0); i < countI; i+=3)
	{
		pBufIndices[i]   = vecTriangles[i/3].v1;
		pBufIndices[i+1] = vecTriangles[i/3].v2;
		pBufIndices[i+2] = vecTriangles[i/3].v3;
	}

	//  - vertices
	unsigned int							countV      (vecVertices.size());
	DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

	for (unsigned int i(0); i < countV; ++i)
	{
		pBufVertices[i]._x     = vecVertices[i].x * factor;
		pBufVertices[i]._y     = vecVertices[i].y * factor;
		pBufVertices[i]._z     = vecVertices[i].z * factor;
		pBufVertices[i]._color = _defCollisionColor;
	}

	//  collected all data needed => convert to DirectX
	//  - transformation matrix
	for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
	{
		locTransform *= *pIterT;
	}

	//  create new model
	DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																	pBufVertices,
																	countV,
																	pBufIndices,
																	countI,
																	_defCollisionColor
																);
	//  set visibility
	if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

	//  add model view data
	pNewModel->SetNifData("unknown", "bhkPackedNiTriStrips", pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pNewModel);

	//  remove RigidBody translation from list - if set
	if (pRBody != NULL)
	{
		transformAry.pop_back();
	}

	return meshList.size();
}

//-----  ConvertModel()  ------------------------------------------------------
bool DirectXNifConverter::ConvertModel(const string fileName, vector<DirectXMesh*>& meshList)
{
	NiNodeRef			pRootInput(NULL);
	vector<Matrix44>	transformAry;

	//  test on existing file names
	if (fileName.empty())		return false;

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName)) == NULL)
	{
		return false;
	}

	//  parse geometry
	getGeometryFromNode(pRootInput, meshList, transformAry, NULL);

	return true;
}

//-----  BlendFuncToDXBlend()  ------------------------------------------------
void DirectXNifConverter::BlendFuncToDXBlend(const NiAlphaProperty::BlendFunc value, DWORD& dxBlend, DWORD& dxArg)
{
	switch (value)
	{
		case NiAlphaProperty::BF_SRC_ALPHA:
		case NiAlphaProperty::BF_SRC_ALPHA_SATURATE:
		{
			dxBlend = D3DBLEND_SRCALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_DST_ALPHA:
		{
			dxBlend = D3DBLEND_DESTALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_ONE_MINUS_SRC_ALPHA:
		{
			dxBlend = D3DBLEND_INVSRCALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_ONE_MINUS_DST_ALPHA:
		{
			dxBlend = D3DBLEND_INVDESTALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}
	}
}

//-----  Matrix44ToD3DXMATRIX()  ----------------------------------------------
D3DXMATRIX DirectXNifConverter::Matrix44ToD3DXMATRIX(const Matrix44& matrixIn)
{
	D3DXMATRIX	matrixOut;

	for (short idx(0); idx < 16; ++idx)
	{
		matrixOut((idx / 4), (idx % 4)) = matrixIn.rows[(idx / 4)][(idx % 4)];
	}

	return matrixOut;
}

//-----  QuaternionToMatrix44()  ----------------------------------------------
Matrix33 DirectXNifConverter::QuaternionToMatrix33(const QuaternionXYZW& quadIn)
{
	Matrix33	matrixOut;
	double		xx  (quadIn.x * quadIn.x);
	double		yy  (quadIn.y * quadIn.y);
	double		zz  (quadIn.z * quadIn.z);
	double		ww  (quadIn.w * quadIn.w);
	double		invs(1 / (xx + yy + zz + ww));
	double		tmp1(quadIn.x * quadIn.y);
	double		tmp2(quadIn.z * quadIn.w);

	matrixOut.rows[0][0] = (float) (( xx - yy - zz + ww) * invs);
	matrixOut.rows[1][1] = (float) ((-xx + yy - zz + ww) * invs);
	matrixOut.rows[2][2] = (float) ((-xx - yy + zz + ww) * invs);

	matrixOut.rows[1][0] = (float) (2.0f * (tmp1 + tmp2)*invs);
	matrixOut.rows[0][1] = (float) (2.0f * (tmp1 - tmp2)*invs);

	tmp1 = quadIn.x * quadIn.z;
	tmp2 = quadIn.y * quadIn.w;
	matrixOut.rows[2][0] = (float) (2.0f * (tmp1 - tmp2)*invs);
	matrixOut.rows[0][2] = (float) (2.0f * (tmp1 + tmp2)*invs);

	tmp1 = quadIn.y * quadIn.z;
	tmp2 = quadIn.x * quadIn.w;
	matrixOut.rows[2][1] = (float) (2.0f * (tmp1 + tmp2)*invs);
	matrixOut.rows[1][2] = (float) (2.0f * (tmp1 - tmp2)*invs);

	return matrixOut;
}

//-----  CheckTextureName()  --------------------------------------------------
string DirectXNifConverter::CheckTextureName(string texName)
{
	ifstream	inFile;
	string		baseTexture("");
	char		cBuffer[1000];
	char*		pStart(NULL);

	//  early return on empty texName
	if (texName.empty())	return baseTexture;

	//  check texture paths
	for (auto pIter=_texturePathList.begin(), pEnd=_texturePathList.end(); pIter != pEnd; ++pIter)
	{
		_snprintf(cBuffer, 1000, "%s\\%s", pIter->c_str(), texName.c_str());

		//  test for '\\'
		if ((pStart = strstr(cBuffer, "\\\\")) != NULL)
		{
			memmove(pStart, pStart+1, strlen(pStart));
		}
					
		//  test for 'texture\texture' (case insensitive)
		if ((pStart = (char*) strcasestr(cBuffer, "textures\\textures")) != NULL)
		{
			memmove(pStart, pStart+8, strlen(pStart));
		}

		//  set texture path
		baseTexture = cBuffer;

		//  test for forced dds ending
		if (_forceDDS)
		{
			string::size_type	result(baseTexture.rfind('.'));

			if (result != string::npos)		baseTexture.erase(result);
			baseTexture += ".dds";
		}

		//  test if texture file exists
		inFile.open(baseTexture.c_str());
		if (inFile)
		{
			inFile.close();
			break;
		}
	}  //  for (auto pIter=_texturePathList.begin(), pEnd=_texturePathList.end(); pIter != pEnd; ++pIter)

	return baseTexture;
}

//-----  DecodeAlphaProperty()  -----------------------------------------------
DirectXAlphaState* DirectXNifConverter::DecodeAlphaProperty(NiAlphaProperty* pProperty)
{
	DirectXAlphaState*	pAlpha(NULL);
	static const DWORD	blendMap[] = { D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLEND_SRCCOLOR, D3DBLEND_INVSRCCOLOR,
									   D3DBLEND_DESTCOLOR, D3DBLEND_INVDESTCOLOR, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA,
									   D3DBLEND_DESTALPHA, D3DBLEND_INVDESTALPHA, D3DBLEND_SRCALPHASAT, D3DBLEND_ONE,
									   D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_ONE,D3DBLEND_ONE
									 };
	static const DWORD	testMap[]  = { D3DCMP_ALWAYS, D3DCMP_LESS, D3DCMP_EQUAL, D3DCMP_LESSEQUAL, D3DCMP_GREATER, D3DCMP_NOTEQUAL, D3DCMP_GREATEREQUAL, D3DCMP_NEVER };

	//  early return on no input
	if (pProperty == NULL)			return NULL;

	unsigned short	flags(pProperty->GetFlags());

	pAlpha = new DirectXAlphaState;
	pAlpha->_enabledBlend = ((flags & 0x0001 ) != 0);
	pAlpha->_threshold    = pProperty->GetTestThreshold();
	pAlpha->_source       = blendMap[(flags >> 1) & 0x000f];
	pAlpha->_destination  = blendMap[(flags >> 5) & 0x000f];
	pAlpha->_argument     = D3DTA_TEXTURE;
	pAlpha->_enabledTest  = ((flags & (1 << 9)) != 0);
	pAlpha->_function     = testMap[(flags >> 10) & 0x0007];

	return pAlpha;
}
