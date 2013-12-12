/**
 *  file:   DirectXNifConverter.cpp
 *  class:  DirectXNifConverter
 *
 *  Class converting a NIF model into DirectX readable one
 */

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <sstream>
#include "Common\Util\FDCLibHelper.h"

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
#include "obj/bhkConvexTransformShape.h"
#include "obj/bhkListShape.h"

//-----  DEFINES  -------------------------------------------------------------

//-----  DirectXNifConverter()  -----------------------------------------------
DirectXNifConverter::DirectXNifConverter()
	:	_defWireframeColor(0x00FFFFFF),
		_defCollisionColor(0x00FFFF00),
		_defAmbientColor  (0x00707070),
		_defDiffuseColor  (0x00E5E5E5),
		_defSpecularColor (0x00FFFFFF),
		_factor           (70.0f),
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
	NifInfo			nifInfo;

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str(), &nifInfo);

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

	//  check NIF versions
	unsigned int	nifVersion(nifInfo.version);
	unsigned int	nifUserVer(nifInfo.userVersion);
	
	if (nifInfo.creator == "NifConvert")
	{
		vector<string>		tokenList;

		if (strexplode(nifInfo.exportInfo2.c_str(), ";", tokenList) >= 2)
		{
			nifVersion = atol(tokenList[0].c_str());
			nifUserVer = atol(tokenList[1].c_str());
		}
	}

	_factor = ((nifVersion >= VER_20_2_0_7) && (nifUserVer >= 12)) ? 71.0f : 7.1f;

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

	//  use default normals if not given
	//  (does this make sense???)
	if (vecNormals.empty())
	{
		for (unsigned int tmpCnt(0); tmpCnt < vecVertices.size(); ++tmpCnt)
		{
			vecNormals.push_back(vecVertices[tmpCnt].Normalized());
		}
	}

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

			//  check for min/max bounds
			if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
			if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
			if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
			if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
			if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
			if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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

			//  check for min/max bounds
			if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
			if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
			if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
			if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
			if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
			if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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

	//  add RigidBody translation to list - if bhkRigidBodyT
	bhkRigidBodyT*		pRBodyT(DynamicCast<bhkRigidBodyT>(pRBody));
	if (pRBodyT != NULL)
	{
		Vector4		tVec4 (pRBodyT->GetTranslation());
		Vector3		tVec3 (tVec4.x * _factor, tVec4.y * _factor, tVec4.z * _factor);
		Matrix33	tMat33(QuaternionToMatrix33(pRBodyT->GetRotation()));
		Matrix44	tMat44(tVec3, tMat33, 1.0f);
	
		transformAry.push_back(tMat44);
	}

	//  get geometry from meshes/lists
	getGeometryFromCollisionShape(pRBody->GetShape(), meshList, transformAry, pTmplAlphaProp);

	//  remove RigidBody translation from list - if set
	if (pRBodyT != NULL)
	{
		transformAry.pop_back();
	}

	return meshList.size();
}

//-----  getGeometryFromCollisionShape()  -------------------------------------
unsigned int DirectXNifConverter::getGeometryFromCollisionShape(bhkShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	//  check type of collision mesh
	//bhkMoppBvTreeShape
	if (DynamicCast<bhkMoppBvTreeShape>(pShape) != NULL)
	{
		getGeometryFromCollisionShape(DynamicCast<bhkMoppBvTreeShape>(pShape)->GetShape(), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkCompressedMeshShape
	else if (DynamicCast<bhkCompressedMeshShape>(pShape) != NULL)
	{
		getGeometryFromCompressedMeshShape(DynamicCast<bhkCompressedMeshShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkPackedNiTriStripsShape
	else if (DynamicCast<bhkPackedNiTriStripsShape>(pShape) != NULL)
	{
		getGeometryFromPackedTriStripsShape(DynamicCast<bhkPackedNiTriStripsShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkNiTriStripsShape
	else if (DynamicCast<bhkNiTriStripsShape>(pShape) != NULL)
	{
		getGeometryFromTriStripsShape(DynamicCast<bhkNiTriStripsShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkConvexVerticesShape
	else if (DynamicCast<bhkConvexVerticesShape>(pShape) != NULL)
	{
		getGeometryFromConvexVerticesShape(DynamicCast<bhkConvexVerticesShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkBoxShape
	else if (DynamicCast<bhkBoxShape>(pShape) != NULL)
	{
		getGeometryFromBoxShape(DynamicCast<bhkBoxShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkSphereShape
	else if (DynamicCast<bhkSphereShape>(pShape) != NULL)
	{
		getGeometryFromSphereShape(DynamicCast<bhkSphereShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkCapsuleShape
	else if (DynamicCast<bhkCapsuleShape>(pShape) != NULL)
	{
		getGeometryFromCapsuleShape(DynamicCast<bhkCapsuleShape>(pShape), meshList, transformAry, pTmplAlphaProp);
	}
	//bhkTransformShape
	else if (DynamicCast<bhkTransformShape>(pShape) != NULL)
	{
		Matrix44	tTrans(DynamicCast<bhkTransformShape>(pShape)->GetTransform().Transpose());
		Vector4		tVec4 (tTrans.GetTranslation());
		Vector3		tVec3 (tVec4.x * _factor, tVec4.y * _factor, tVec4.z * _factor);
		Matrix33	tMat33(tTrans.GetRotation());
		Matrix44	tMat44(tVec3, tMat33, tTrans.GetScale());
	
		//  add own translation to list
		transformAry.push_back(tMat44);

		//  parse sub shape(s)
		getGeometryFromCollisionShape(DynamicCast<bhkTransformShape>(pShape)->GetShape(), meshList, transformAry, pTmplAlphaProp);

		//  remove own translation from list
		transformAry.pop_back();
	}
	//bhkListShape
	else if (DynamicCast<bhkListShape>(pShape) != NULL)
	{
		vector<bhkShapeRef>		subShapes(DynamicCast<bhkListShape>(pShape)->GetSubShapes());

		//  parse sub shape(s)
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
		{
			//  parse sub shape(s)
			getGeometryFromCollisionShape(*pIter, meshList, transformAry, pTmplAlphaProp);
		}
	}

	return meshList.size();
}

//-----  getGeometryFromCapsuleShape()  ----------------------------------------
unsigned int DirectXNifConverter::getGeometryFromCapsuleShape(bhkCapsuleShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	vector<D3DPRIMITIVETYPE>	vecPrimTypes;
	Matrix44					locTransform;
	Vector3						point1  (pShape->GetFirstPoint() * _factor);
	Vector3						point2  (pShape->GetSecondPoint() * _factor);
	Vector3						distance(point2 - point1);
	Vector3						normal  (distance.Normalized());
	Vector3						x       (normal.y, normal.z, normal.x);
	Vector3						y       (normal.CrossProduct(x));
	Vector3						tmpVec;
	Vector3						radVec;
	float						radius  (pShape->GetRadius() * _factor);
	float						f       (0.0f);
	float						radSeg  (0.0f);
	int							subDiv  (8);

	x = normal.CrossProduct(y);
	x *= radius;
	y *= radius;

	//  - vertices
	vector<DirectXMeshCollision::D3DCustomVertex>	tmpVertices[4];
	vector<DirectXMeshCollision::D3DCustomVertex*>	vecVertices;
	vector<unsigned short*>							vecIndices;
	vector<unsigned int>							vecCntVertices;
	vector<unsigned int>							vecCntIndices;
	DirectXMeshCollision::D3DCustomVertex			tVertex;
	
	for (int i(0); i <= subDiv*2; ++i)
	{
		tmpVec         = point1 + distance/2 + x * sin(PI / subDiv * i) + y * cos(PI / subDiv * i);
		tVertex._x     = tmpVec.x;
		tVertex._y     = tmpVec.y;
		tVertex._z     = tmpVec.z;
		tVertex._color = _defCollisionColor;
		tmpVertices[0].push_back(tVertex);

		tmpVec         = point1 + x * sin(PI / subDiv * i) + y * cos(PI / subDiv * i);
		tVertex._x     = tmpVec.x;
		tVertex._y     = tmpVec.y;
		tVertex._z     = tmpVec.z;
		tVertex._color = _defCollisionColor;
		tmpVertices[1].push_back(tVertex);

		tmpVec         = point2 + x * sin(PI / subDiv * i) + y * cos(PI / subDiv * i);
		tVertex._x     = tmpVec.x;
		tVertex._y     = tmpVec.y;
		tVertex._z     = tmpVec.z;
		tVertex._color = _defCollisionColor;
		tmpVertices[1].push_back(tVertex);
	}

	for (int j(-subDiv); j <= subDiv; ++j)
	{
		f      = PI * float(j) / float(subDiv * 2);
		radSeg = sin(f);
		radVec = normal * radius * cos(f);
		for (int i(0); i <= subDiv*2; ++i)
		{
			tmpVec         = point1 - radVec + x * sin(PI / subDiv * i) * radSeg + y * cos(PI / subDiv * i) * radSeg;
			tVertex._x     = tmpVec.x;
			tVertex._y     = tmpVec.y;
			tVertex._z     = tmpVec.z;
			tVertex._color = _defCollisionColor;
			tmpVertices[2].push_back(tVertex);

			tmpVec         = point2 + radVec + x * sin(PI / subDiv * i) * radSeg + y * cos(PI / subDiv * i) * radSeg;
			tVertex._x     = tmpVec.x;
			tVertex._y     = tmpVec.y;
			tVertex._z     = tmpVec.z;
			tVertex._color = _defCollisionColor;
			tmpVertices[3].push_back(tVertex);
		}


	}  //  for (int j(-subDiv); j <= subDiv; ++j)

	//  - build parameter lists
	for (short idx(0); idx < 4; ++idx)
	{
		//  vertices
		unsigned int							countV      (tmpVertices[idx].size());
		DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

		//memcpy(pBufVertices, &tmpVertices[idx], countV * sizeof(DirectXMeshCollision::D3DCustomVertex));
		for (unsigned int ii(0); ii < countV; ++ii)
		{
			pBufVertices[ii] = tmpVertices[idx][ii];

			//  check for min/max bounds
			if (pBufVertices[ii]._x < _pointMin.x)		_pointMin.x = pBufVertices[ii]._x;
			if (pBufVertices[ii]._x > _pointMax.x)		_pointMax.x = pBufVertices[ii]._x;
			if (pBufVertices[ii]._y < _pointMin.y)		_pointMin.y = pBufVertices[ii]._y;
			if (pBufVertices[ii]._y > _pointMax.y)		_pointMax.y = pBufVertices[ii]._y;
			if (pBufVertices[ii]._z < _pointMin.z)		_pointMin.z = pBufVertices[ii]._z;
			if (pBufVertices[ii]._z > _pointMax.z)		_pointMax.z = pBufVertices[ii]._z;
		}
		vecVertices.push_back(pBufVertices);
		vecCntVertices.push_back(countV);

		//  indices
		unsigned int		countI     (tmpVertices[idx].size());
		unsigned short*		pBufIndices(new unsigned short[countI]);

		for (unsigned short ii(0); ii < countI; ++ii)
		{
			pBufIndices[ii] = ii;
		}
		vecIndices.push_back(pBufIndices);
		vecCntIndices.push_back(countI);

	}  //  for (short idx(0); idx < 3; ++idx)

	//  - primitive types
	vecPrimTypes.push_back(D3DPT_LINESTRIP);
	vecPrimTypes.push_back(D3DPT_LINELIST);
	vecPrimTypes.push_back(D3DPT_LINESTRIP);
	vecPrimTypes.push_back(D3DPT_LINESTRIP);

	//  collected all data needed => convert to DirectX
	//  - transformation matrix
	for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
	{
		locTransform *= *pIterT;
	}

	//  create new model
	DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																	vecVertices,
																	vecCntVertices,
																	vecIndices,
																	vecCntIndices,
																	_defCollisionColor
																);
	//  modify primitive type
	pNewModel->SetPrimitiveType(vecPrimTypes);

	//  set visibility
	if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

	//  add model view data
	pNewModel->SetNifData("unknown", "bhkCapsuleShape", pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pNewModel);

	return meshList.size();
}

//-----  getGeometryFromSphereShape()  ----------------------------------------
unsigned int DirectXNifConverter::getGeometryFromSphereShape(bhkSphereShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	Matrix44	locTransform;
	Vector3			center;
	Vector3			tmpVec;
	Vector3			radVec;
	float			radius(pShape->GetRadius() * _factor);
	float			f     (0.0f);
	float			radSeg(0.0f);
	int				subDiv(8);

	//  - vertices
	vector<DirectXMeshCollision::D3DCustomVertex>	tmpVertices[3];
	vector<DirectXMeshCollision::D3DCustomVertex*>	vecVertices;
	vector<unsigned short*>							vecIndices;
	vector<unsigned int>							vecCntVertices;
	vector<unsigned int>							vecCntIndices;
	DirectXMeshCollision::D3DCustomVertex			tVertex;

	for (int j(-subDiv); j <= subDiv; ++j)
	{
		f      = PI * float(j) / float(subDiv);
		radSeg = radius * sin(f);

		for (int i(0); i <= subDiv*2; ++i)
		{
			radVec = center + Vector3(0, 0, radius * cos(f));
			tmpVec         = (Vector3(sin(PI / subDiv * i), cos(PI / subDiv * i), 0) * radSeg + radVec);
			tVertex._x     = tmpVec.x;
			tVertex._y     = tmpVec.y;
			tVertex._z     = tmpVec.z;
			tVertex._color = _defCollisionColor;
			tmpVertices[0].push_back(tVertex);

			radVec = center + Vector3(0, radius * cos(f), 0);
			tmpVec         = (Vector3(sin(PI / subDiv * i), 0, cos(PI / subDiv * i)) * radSeg + radVec);
			tVertex._x     = tmpVec.x;
			tVertex._y     = tmpVec.y;
			tVertex._z     = tmpVec.z;
			tVertex._color = _defCollisionColor;
			tmpVertices[1].push_back(tVertex);

			radVec = center + Vector3(radius * cos(f), 0, 0);
			tmpVec         = (Vector3(0, sin(PI / subDiv * i), cos(PI / subDiv * i)) * radSeg + radVec);
			tVertex._x     = tmpVec.x;
			tVertex._y     = tmpVec.y;
			tVertex._z     = tmpVec.z;
			tVertex._color = _defCollisionColor;
			tmpVertices[2].push_back(tVertex);
		}
	}  //  for (int j(-subDiv); j <= subDiv; ++j)

	//  - build parameter lists
	for (short idx(0); idx < 3; ++idx)
	{
		//  vertices
		unsigned int							countV      (tmpVertices[idx].size());
		DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

		//memcpy(pBufVertices, &tmpVertices[idx], countV * sizeof(DirectXMeshCollision::D3DCustomVertex));
		for (unsigned int ii(0); ii < countV; ++ii)
		{
			pBufVertices[ii] = tmpVertices[idx][ii];

			//  check for min/max bounds
			if (pBufVertices[ii]._x < _pointMin.x)		_pointMin.x = pBufVertices[ii]._x;
			if (pBufVertices[ii]._x > _pointMax.x)		_pointMax.x = pBufVertices[ii]._x;
			if (pBufVertices[ii]._y < _pointMin.y)		_pointMin.y = pBufVertices[ii]._y;
			if (pBufVertices[ii]._y > _pointMax.y)		_pointMax.y = pBufVertices[ii]._y;
			if (pBufVertices[ii]._z < _pointMin.z)		_pointMin.z = pBufVertices[ii]._z;
			if (pBufVertices[ii]._z > _pointMax.z)		_pointMax.z = pBufVertices[ii]._z;
		}
		vecVertices.push_back(pBufVertices);
		vecCntVertices.push_back(countV);

		//  indices
		unsigned int		countI     (tmpVertices[idx].size());
		unsigned short*		pBufIndices(new unsigned short[countI]);

		for (unsigned short ii(0); ii < countI; ++ii)
		{
			pBufIndices[ii] = ii;
		}
		vecIndices.push_back(pBufIndices);
		vecCntIndices.push_back(countI);

	}  //  for (short idx(0); idx < 3; ++idx)

	//  collected all data needed => convert to DirectX
	//  - transformation matrix
	for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
	{
		locTransform *= *pIterT;
	}

	//  create new model
	DirectXMeshCollision*	pNewModel = new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform),
																	vecVertices,
																	vecCntVertices,
																	vecIndices,
																	vecCntIndices,
																	_defCollisionColor
																);
	//  modify primitive type
	pNewModel->SetPrimitiveType(D3DPT_LINESTRIP);

	//  set visibility
	if (!_showCollision)	pNewModel->SetRenderMode(DXRM_NONE);

	//  add model view data
	pNewModel->SetNifData("unknown", "bhkSphereShape", pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pNewModel);

	return meshList.size();
}

//-----  getGeometryFromBoxShape()  -------------------------------------------
unsigned int DirectXNifConverter::getGeometryFromBoxShape(bhkBoxShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	Vector3		dimensions(pShape->GetDimensions());
	Matrix44	locTransform;

	//  - indices
	unsigned int		countI     (24);
	unsigned short*		pBufIndices(new unsigned short[countI]);

	pBufIndices[0]  = 0;
	pBufIndices[1]  = 1;
	pBufIndices[2]  = 0;
	pBufIndices[3]  = 2;
	pBufIndices[4]  = 0;
	pBufIndices[5]  = 4;
	pBufIndices[6]  = 1;
	pBufIndices[7]  = 3;
	pBufIndices[8]  = 1;
	pBufIndices[9]  = 5;
	pBufIndices[10] = 2;
	pBufIndices[11] = 3;
	pBufIndices[12] = 2;
	pBufIndices[13] = 6;
	pBufIndices[14] = 3;
	pBufIndices[15] = 7;
	pBufIndices[16] = 4;
	pBufIndices[17] = 5;
	pBufIndices[18] = 4;
	pBufIndices[19] = 6;
	pBufIndices[20] = 5;
	pBufIndices[21] = 7;
	pBufIndices[22] = 6;
	pBufIndices[23] = 7;

	//  - vertices
	unsigned int							countV      (8);
	DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

	//  +x, +y, +z
	pBufVertices[0]._x =  dimensions.x * _factor;
	pBufVertices[0]._y =  dimensions.y * _factor;
	pBufVertices[0]._z =  dimensions.z * _factor;
	//  +x, +y, -z
	pBufVertices[1]._x =  dimensions.x * _factor;
	pBufVertices[1]._y =  dimensions.y * _factor;
	pBufVertices[1]._z = -dimensions.z * _factor;
	//  +x, -y, +z
	pBufVertices[2]._x =  dimensions.x * _factor;
	pBufVertices[2]._y = -dimensions.y * _factor;
	pBufVertices[2]._z =  dimensions.z * _factor;
	//  +x, -y, -z
	pBufVertices[3]._x =  dimensions.x * _factor;
	pBufVertices[3]._y = -dimensions.y * _factor;
	pBufVertices[3]._z = -dimensions.z * _factor;
	//  -x, +y, +z
	pBufVertices[4]._x = -dimensions.x * _factor;
	pBufVertices[4]._y =  dimensions.y * _factor;
	pBufVertices[4]._z =  dimensions.z * _factor;
	//  -x, +y, -z
	pBufVertices[5]._x = -dimensions.x * _factor;
	pBufVertices[5]._y =  dimensions.y * _factor;
	pBufVertices[5]._z = -dimensions.z * _factor;
	//  -x, -y, +z
	pBufVertices[6]._x = -dimensions.x * _factor;
	pBufVertices[6]._y = -dimensions.y * _factor;
	pBufVertices[6]._z =  dimensions.z * _factor;
	//  -x, -y, -z
	pBufVertices[7]._x = -dimensions.x * _factor;
	pBufVertices[7]._y = -dimensions.y * _factor;
	pBufVertices[7]._z = -dimensions.z * _factor;

	for (unsigned int i(0); i < countV; ++i)
	{
		pBufVertices[i]._color = _defCollisionColor;
	
		//  check for min/max bounds
		if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
		if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
		if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
		if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
		if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
		if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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
	pNewModel->SetNifData("unknown", "bhkBoxShape", pShape->internal_block_number);

	//  append model to list
	meshList.push_back(pNewModel);

	return meshList.size();
}

//-----  getGeometryFromConvexVerticesShape()  --------------------------------
unsigned int DirectXNifConverter::getGeometryFromConvexVerticesShape(bhkConvexVerticesShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	vector<Vector3>		vecVertices (pShape->GetVertices());
	Matrix44			locTransform;

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
		pBufVertices[i]._x     = vecVertices[i].x * _factor;
		pBufVertices[i]._y     = vecVertices[i].y * _factor;
		pBufVertices[i]._z     = vecVertices[i].z * _factor;
		pBufVertices[i]._color = _defCollisionColor;
	
		//  check for min/max bounds
		if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
		if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
		if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
		if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
		if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
		if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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

	return meshList.size();
}

//-----  getGeometryFromCompressedMeshShape()  --------------------------------
unsigned int DirectXNifConverter::getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	bhkCompressedMeshShapeData*	pData(DynamicCast<bhkCompressedMeshShapeData>(pShape->GetData()));
	if (pData == NULL)		return meshList.size();

	//  hurray!!!
	vector<bhkCMSDChunk>		chunkGeoList(pData->GetChunks());
	vector<bhkCMSDMaterial>		chunkMatList(pData->GetChunkMaterials());
	unsigned short				idxChunk    (0);

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
			pBufVertices[i]._x     = vecVertices[i].x * _factor;
			pBufVertices[i]._y     = vecVertices[i].y * _factor;
			pBufVertices[i]._z     = vecVertices[i].z * _factor;
			pBufVertices[i]._color = _defCollisionColor;
	
			//  check for min/max bounds
			if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
			if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
			if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
			if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
			if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
			if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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
				pBufVertices[i]._x     = pIter->_vertices[i].x * _factor;
				pBufVertices[i]._y     = pIter->_vertices[i].y * _factor;
				pBufVertices[i]._z     = pIter->_vertices[i].z * _factor;
				pBufVertices[i]._color = _defCollisionColor;
	
				//  check for min/max bounds
				if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
				if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
				if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
				if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
				if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
				if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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

	return meshList.size();
}

//-----  getGeometryFromPackedTriStripsShape()  -------------------------------
unsigned int DirectXNifConverter::getGeometryFromPackedTriStripsShape(bhkPackedNiTriStripsShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	hkPackedNiTriStripsData*	pData(DynamicCast<hkPackedNiTriStripsData>(pShape->GetData()));
	if (pData == NULL)		return meshList.size();

	//  hurray!!!
	vector<Vector3>				vecVertices (pData->GetVertices());
	vector<Triangle>			vecTriangles(pData->GetTriangles());
	vector<Triangle>			tmpTriangles;
	vector<OblivionSubShape>	subShapes   (pShape->GetSubShapes());
	Matrix44					locTransform;
	unsigned int				vertOff     (0);
	unsigned int				triIndex    (0);
	unsigned int				subShapeIdx (0);

	//  get sub shapes
	if (subShapes.empty())		subShapes = pData->GetSubShapes();

	//  - transformation matrix
	for (auto pIterT=transformAry.rbegin(), pEndT=transformAry.rend(); pIterT != pEndT; ++pIterT)
	{
		locTransform *= *pIterT;
	}

	//  for each sub shape
	for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter, ++subShapeIdx)
	{
		//  - vertices
		unsigned int							countV      (pIter->numVertices);
		DirectXMeshCollision::D3DCustomVertex*	pBufVertices(new DirectXMeshCollision::D3DCustomVertex[countV]);

		for (unsigned int i(0); i < countV; ++i)
		{
			pBufVertices[i]._x     = vecVertices[i+vertOff].x * _factor;
			pBufVertices[i]._y     = vecVertices[i+vertOff].y * _factor;
			pBufVertices[i]._z     = vecVertices[i+vertOff].z * _factor;
			pBufVertices[i]._color = _defCollisionColor;
	
			//  check for min/max bounds
			if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
			if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
			if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
			if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
			if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
			if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
		}

		//  - indices
		for (tmpTriangles.clear(); triIndex < vecTriangles.size(); ++triIndex)
		{
			//  check vertex bounds
			if ((vecTriangles[triIndex].v1 >= (pIter->numVertices + vertOff)) ||
				(vecTriangles[triIndex].v2 >= (pIter->numVertices + vertOff)) ||
				(vecTriangles[triIndex].v3 >= (pIter->numVertices + vertOff))
				)
			{
				break;
			}

			tmpTriangles.push_back(vecTriangles[triIndex]);
		}

		unsigned int		countI     (tmpTriangles.size()*3);
		unsigned short*		pBufIndices(new unsigned short[countI]);

		for (unsigned int i(0); i < countI; i+=3)
		{
			pBufIndices[i]   = tmpTriangles[i/3].v1 - vertOff;
			pBufIndices[i+1] = tmpTriangles[i/3].v2 - vertOff;
			pBufIndices[i+2] = tmpTriangles[i/3].v3 - vertOff;
		}

		//  collected all data needed => convert to DirectX
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

		sStream << "SubShape # " << subShapeIdx;
		pNewModel->SetNifData(sStream.str(), "bhkPackedNiTriStrips", pShape->internal_block_number);

		//  append model to list
		meshList.push_back(pNewModel);

		//  increase vertices offset
		vertOff += countV;

	}  //  for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)

	return meshList.size();
}

//-----  getGeometryFromTriStripsShape()  -------------------------------------
unsigned int DirectXNifConverter::getGeometryFromTriStripsShape(bhkNiTriStripsShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	int		numData(pShape->GetNumStripsData());
	if	(numData <= 0)		return meshList.size();

	//  for each strips data
	for (int idxData(0); idxData < numData; ++idxData)
	{
		NiTriStripsDataRef		pData(pShape->GetStripsData(idxData));
		if (pData == NULL)		continue;

		vector<Vector3>		vecVertices (pData->GetVertices());
		vector<Triangle>	vecTriangles(pData->GetTriangles());
		Matrix44			locTransform;

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
			pBufVertices[i]._x     = vecVertices[i].x;
			pBufVertices[i]._y     = vecVertices[i].y;
			pBufVertices[i]._z     = vecVertices[i].z;
			pBufVertices[i]._color = _defCollisionColor;
	
			//  check for min/max bounds
			if (pBufVertices[i]._x < _pointMin.x)		_pointMin.x = pBufVertices[i]._x;
			if (pBufVertices[i]._x > _pointMax.x)		_pointMax.x = pBufVertices[i]._x;
			if (pBufVertices[i]._y < _pointMin.y)		_pointMin.y = pBufVertices[i]._y;
			if (pBufVertices[i]._y > _pointMax.y)		_pointMax.y = pBufVertices[i]._y;
			if (pBufVertices[i]._z < _pointMin.z)		_pointMin.z = pBufVertices[i]._z;
			if (pBufVertices[i]._z > _pointMax.z)		_pointMax.z = pBufVertices[i]._z;
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

		sStream << "TriStripData #" << idxData << ": " << NifUtlMaterialList::getInstance()->getMaterialDefName(pData->GetSkyrimMaterial());
		pNewModel->SetNifData(sStream.str(), "bhkNiTriStrips", pShape->internal_block_number);

		//  append model to list
		meshList.push_back(pNewModel);

	}  //  for (int idxData(0); idxData < numData; ++idxData)

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

//-----  GetBoundingBox()  ----------------------------------------------------
Vector3 DirectXNifConverter::GetBoundingBox()
{
	return _pointMax - _pointMin;
}
