/**
 *  file:   NifCollisionUtility.cpp
 *  class:  NifCollisioUtility
 *
 *  Utilities adding collision data to NIF files
 */

//-----  INCLUDES  ------------------------------------------------------------
//  Common includes
#include <fstream>
#include <sstream>

#include "Common\Nif\NifCollisionUtility.h"
#include "Common\Util\DefLogMessageTypes.h"

//  Niflib includes
#include "niflib.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTriStripsData.h"
#include "obj/bhkCompressedMeshShape.h"
#include "obj/rootcollisionnode.h"
#include "obj/bhkTransformShape.h"
#include "obj/bhkListShape.h"
#include "obj/bhkBoxShape.h"
#include "obj/bhkCapsuleShape.h"
#include "obj/bhkSphereShape.h"
#include "obj/bhkPackedNiTriStripsShape.h"
#include "obj/bhkConvexVerticesShape.h"
#include "obj/hkPackedNiTriStripsData.h"
#include "obj/bhkNiTriStripsShape.h"

//  Havok includes
#include "Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h"
#include "Physics/Collide/Shape/Compound/Collection/CompressedMesh/hkpCompressedMeshShapeBuilder.h"
#include "Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h"
#include "Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h"
#include "Physics/Collide/Shape/Compound/Collection/ExtendedMeshShape/hkpExtendedMeshShape.h"

//-----  DEFINES  -------------------------------------------------------------
//  used namespaces
using namespace Niflib;

/*---------------------------------------------------------------------------*/
NifCollisionUtility::NifCollisionUtility(NifUtlMaterialList& materialList)
	:	_cnHandling        (NCU_CN_FALLBACK),
		_mtHandling        (NCU_MT_SINGLE),
		_logCallback       (NULL),
		_materialList      (materialList),
		_defaultMaterial   (0),
		_mergeCollision    (true),
		_reorderTriangles  (true),
		_nifVersion        (0)
{}

/*---------------------------------------------------------------------------*/
NifCollisionUtility::~NifCollisionUtility()
{}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::addCollision(string fileNameCollSrc, string fileNameNifDst, string fileNameCollTmpl)
{
	NiNodeRef				pRootInput   (NULL);
	NiNodeRef				pRootTemplate(NULL);
	bhkCollisionObjectRef	pCollNodeTmpl(NULL);
	vector<hkGeometry>		geometryMap;
	vector<NiAVObjectRef>	srcChildList;
	bool					fakedRoot    (false);
	unsigned int			nifVersion   (0);

	//  test on existing file names
	if (fileNameCollSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameNifDst.empty())			return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameCollTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  reset material mapping in case of material from NiTriShape name
	if (_mtHandling == NCU_MT_NITRISHAPE_NAME)
	{
		_mtMapping.clear();
	}

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "CollSource:  "   + (fileNameCollSrc.empty() ? "- none -" : fileNameCollSrc));
	logMessage(NCU_MSG_TYPE_INFO, "CollTemplate:  " + (fileNameCollTmpl.empty() ? "- none -" : fileNameCollTmpl));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  "  + (fileNameNifDst.empty() ? "- none -" : fileNameNifDst));

	//  check supported NIF version
	nifVersion = GetNifVersion(fileNameNifDst);
	if (nifVersion != VER_20_2_0_7)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "NIF version of destination file '" + fileNameCollTmpl + "' not supported.");
		return NCU_ERROR_WRONG_NIF_VERSION;
	}

	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameCollTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameCollTmpl + "' as template");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get shapes from template
	//  - collision root
	pCollNodeTmpl = DynamicCast<bhkCollisionObject>(pRootTemplate->GetCollisionObject());
	if (pCollNodeTmpl == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Template has no bhkCollisionObject.");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get root node from destination
	if ((pRootInput = getRootNodeFromNifFile(fileNameNifDst, "target", fakedRoot, false)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameNifDst + "' as template");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  test on collision source having collision data
	//    CollisionNodeHandling::NCU_CN_FALLBACK should be osolete after this point
	if (!collSourceHasCollNodes(fileNameCollSrc))
	{
		//  fallback to create single collision from model data
		_mergeCollision = true;
		_cnHandling     = CollisionNodeHandling(NCU_CN_SHAPES);
	}
	else if (_cnHandling == NCU_CN_FALLBACK)
	{
		//  collision node existing
		_cnHandling = CollisionNodeHandling(NCU_CN_COLLISION);
	}

	//  output != collision source => force merge
	_mergeCollision = (_mergeCollision || (fileNameCollSrc != fileNameNifDst));

	//  replace each collision node
	if (!_mergeCollision)
	{
		vector<hkGeometry>		geometryMapCollLocal;

		logMessage(NCU_MSG_TYPE_INFO, "!!! Using replace-node-method due to target is source !!!");

		//  replace each collision node with new style one
		parseTreeCollision(pRootInput, fileNameCollTmpl, geometryMapCollLocal);
	}
	//  create combined collision node
	else
	{
		//  get geometry data
		switch (fileNameCollSrc[fileNameCollSrc.size() - 3])
		{
			//  from OBJ file
			case 'O':
			case 'o':
			{
				logMessage(NCU_MSG_TYPE_INFO, "Getting geometry from OBJ.");
				getGeometryFromObjFile(fileNameCollSrc, geometryMap);
				break;
			}
			//  from NIF file
			case 'N':
			case 'n':
			{
				logMessage(NCU_MSG_TYPE_INFO, "Getting geometry from NIF.");
				getGeometryFromNifFile(fileNameCollSrc, geometryMap);
				break;
			}
			//  from 3DS file
			case '3':
			{
				//  would be nice ;-)
				logMessage(NCU_MSG_TYPE_INFO, "Getting geometry from 3DS.");
				break;
			}
		}  //  switch (fileNameCollSrc[fileNameCollSrc.size() - 3])

		//  early break on missing geometry data
		if (geometryMap.size() <= 0)
		{
			logMessage(NCU_MSG_TYPE_ERROR, "Can't get geometry from input file.");
			return NCU_ERROR_CANT_GET_GEOMETRY;
		}

		//  create collision node
		bhkCollisionObjectRef	pCollNodeDest(createCollNode(geometryMap, pCollNodeTmpl, pRootInput));

		//  remove all collision sub nodes
		cleanTreeCollision(pRootInput);

		//  add collision node to target
		pRootInput->SetCollisionObject(pCollNodeDest);
	}

	//  write modified nif file
	WriteNifTree((const char*) fileNameNifDst.c_str(), pRootInput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setSkyrimPath(string pathSkyrim)
{
	_pathSkyrim = pathSkyrim;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setCollisionNodeHandling(CollisionNodeHandling cnHandling)
{
	_cnHandling = cnHandling;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setChunkNameHandling(ChunkNameHandling cmHandling)
{
	_cmHandling = cmHandling;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setMaterialTypeHandling(MaterialTypeHandling mtHandling, map<int, unsigned int>& mtMapping)
{
	_mtHandling = mtHandling;
	_mtMapping  = mtMapping;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setDefaultMaterial(unsigned int defaultMaterial)
{
	_defaultMaterial = defaultMaterial;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifCollisionUtility::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
set<string>& NifCollisionUtility::getUsedTextures()
{
	return _usedTextures;
}

/*---------------------------------------------------------------------------*/
set<string>& NifCollisionUtility::getNewTextures()
{
	return _newTextures;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setLogCallback(void (*logCallback) (const int type, const char* pMessage))
{
	_logCallback = logCallback;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setMergeCollision(const bool doMerge)
{
	_mergeCollision = doMerge;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::setReorderTriangles(const bool doReorder)
{
	_reorderTriangles = doReorder;
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromTriShape(NiTriShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		getGeometryFromShapeData(pData->GetVertices(), pData->GetTriangles(), &(*pShape), geometryMap, transformAry);
	}

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromTriStrips(NiTriStripsRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	NiTriStripsDataRef	pData(DynamicCast<NiTriStripsData>(pShape->GetData()));

	if (pData != NULL)
	{
		getGeometryFromShapeData(pData->GetVertices(), pData->GetTriangles(), &(*pShape), geometryMap, transformAry);
	}

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromShapeData(vector<Vector3>& vertices, vector<Triangle>& triangles, NiTriBasedGeomRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	hkGeometry						tmpGeo;
	hkArray<hkVector4>&				vertAry (tmpGeo.m_vertices);
	hkArray<hkGeometry::Triangle>&	triAry  (tmpGeo.m_triangles);
	Vector3							tVector;
	float							factor  (0.0143f);
	unsigned int					material(_defaultMaterial);

	//@TODO:  modify factor depending on _nifVersion if necessary

	//  add local transformation to list
	transformAry.push_back(pShape->GetLocalTransform());

	//  clear arrays
	vertAry.clear();
	triAry.clear();

	//  get vertices
	for (unsigned int idx(0); idx < vertices.size(); ++idx)
	{
		//  get vertex
		tVector = vertices[idx];

		//  transform vertex to global coordinates
		for (int t((int) (transformAry.size())-1); t >= 0; --t)
		{
			tVector = transformAry[t] * tVector;
		}

		//  scale final vertex
		tVector *= factor;

		//  add vertex to tmp. array
		vertAry.pushBack(hkVector4(tVector.x, tVector.y, tVector.z));

	}  //  for (unsigned int idx(0); idx < vertices.size(); ++idx)

	//  map material to geometry/triangles
	switch (_mtHandling)
	{
		case NCU_MT_SINGLE:				//  one material for all
		{
			material = _mtMapping[-1];
			break;
		}

		case NCU_MT_MATMAP:				//  material defined by node id
		{
			material = _mtMapping[pShape->internal_block_number];
			break;
		}

		case NCU_MT_NITRISHAPE_NAME:	//  material defined by node name
		{
			material = _materialList.getMaterialCode(pShape->GetName());
			break;
		}
	}

	//  get triangles
	for (unsigned int idx(0); idx < triangles.size(); ++idx)
	{
		hkGeometry::Triangle	tTri;

		tTri.set(triangles[idx].v1, triangles[idx].v2, triangles[idx].v3, material);
		triAry.pushBack(tTri);
	}

	//  add geometry to result array
	geometryMap.push_back(tmpGeo);

	//  remove local transformation from array
	transformAry.pop_back();

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
bhkShapeRef NifCollisionUtility::getGeometryFromCollShape(bhkShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	bhkShapeRef		pShapeOut(pShape);







	return pShapeOut;
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromPackedNiTriStrips(bhkPackedNiTriStripsShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	hkPackedNiTriStripsDataRef	pData (DynamicCast<hkPackedNiTriStripsData>(pShape->GetData()));
	float						factor(0.1f);

	//@TODO:  modify factor depending on _nifVersion if necessary

	if (pData != NULL)
	{
		hkGeometry						tmpGeo;
		hkArray<hkVector4>&				vertAry  (tmpGeo.m_vertices);
		hkArray<hkGeometry::Triangle>&	triAry   (tmpGeo.m_triangles);
		vector<OblivionSubShape>		subShapes(pData->GetSubShapes());
		vector<Vector3>					vertices (pData->GetVertices());
		vector<hkTriangle>				triangles(pData->GetHavokTriangles());
		hkTriangle&						triangle (triangles[0]);
		Vector3							tVector;
		unsigned int					verOffset(0);
		unsigned int					triIndex (0);
		unsigned int					material (_defaultMaterial);

		//  convert each sub-shape
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
		{
			//  reset arrays
			vertAry.clear();
			triAry.clear();

			//  get vertices
			for (unsigned int idx(verOffset), idxMax(verOffset+pIter->numVertices); idx < idxMax; ++idx)
			{
				tVector = vertices[idx];

				//  scale final vertex
				tVector *= factor;

				//  add vertex to tmp. array
				vertAry.pushBack(hkVector4(tVector.x, tVector.y, tVector.z));

			}  //  for (unsigned int idx(verOffset), idxMax(verOffset+pIter->numVertices); idx < idxMax; ++idx)

			//  get triangles
			for (; triIndex < triangles.size(); ++triIndex)
			{
				//  check vertex bounds
				triangle = triangles[triIndex];
				if ((triangle.triangle.v1 >= (pIter->numVertices + verOffset)) ||
					(triangle.triangle.v2 >= (pIter->numVertices + verOffset)) ||
					(triangle.triangle.v3 >= (pIter->numVertices + verOffset))
				   )
				{
					break;
				}
			
				hkGeometry::Triangle	tTri;

				tTri.set(triangle.triangle.v1-verOffset, triangle.triangle.v2-verOffset, triangle.triangle.v3-verOffset, material);
				triAry.pushBack(tTri);

			}  //  for (; triIndex < triangles.size(); ++triIndex)

			//  re-order triangles points to match same winding
			if (_reorderTriangles)
			{
				reorderTriangles(triAry);
			}

			//  add geometry to result array
			geometryMap.push_back(tmpGeo);

			//  increase vertex offset
			verOffset += pIter->numVertices;

		}  //  for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
	}  //  if (pData != NULL)

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromNode(NiNodeRef pNode, vector<hkGeometry>& geometryMap, vector<hkGeometry>& geometryMapColl, vector<Matrix44>& transformAry)
{
	bhkCollisionObjectRef	pCollObject(DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()));
	vector<NiAVObjectRef>	childList  (pNode->GetChildren());

	//  add own translation to list
	transformAry.push_back(pNode->GetLocalTransform());

	//  get geometry from collision object
	if (pCollObject != NULL)
	{
		//  search for embedded shape
		bhkRigidBodyRef		pRBody(DynamicCast<bhkRigidBody>(pCollObject->GetBody()));

		if (pRBody != NULL)
		{
			getGeometryFromCollShape(pRBody->GetShape(), geometryMapColl, transformAry);
		}
	}  //  if (pCollObject != NULL)

	//  iterate over children
	for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryMap, transformAry);
		}
		//  NiTriStrips
		else if (DynamicCast<NiTriStrips>(*ppIter) != NULL)
		{
			getGeometryFromTriStrips(DynamicCast<NiTriStrips>(*ppIter), geometryMap, transformAry);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(&(*DynamicCast<RootCollisionNode>(*ppIter)), geometryMapColl, geometryMapColl, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryMap, geometryMapColl, transformAry);
		}
	}  //  for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)

	//  remove own translation from list
	transformAry.pop_back();

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromObjFile(string fileName, vector<hkGeometry>& geometryMap)
{
	hkGeometry::Triangle*			pTri   (NULL);
	char*							pChar  (NULL);
	char							cBuffer[1000] = {0};
	hkGeometry						tmpGeo;
	hkArray<hkVector4>&				vertAry(tmpGeo.m_vertices);
	hkArray<hkGeometry::Triangle>&	triAry (tmpGeo.m_triangles);
	ifstream						inFile;
	Vector3							tVector;
	int								tIntAry[3];
	unsigned int					faceOffset(1);
	unsigned int					material  (_defaultMaterial);
	int								idxObject (1);
	short							idx(0);
	bool							hasFace(false);

	//  empty filename => early return
	if (fileName.empty())		return 0;

	//  open file
	inFile.open(fileName.c_str(), ifstream::in);

	//  process file
	while (inFile.good())
	{
		//  read line
		inFile.getline(cBuffer, 1000);

		//  check begin of new face
		if (hasFace && (_strnicmp(cBuffer, "f ", 2) != 0))
		{
			//  add geometry to result array
			geometryMap.push_back(tmpGeo);

			//  set new offset for face vertices
			faceOffset += vertAry.getSize();

			//  reset existing face flag
			hasFace = false;

			//  reset geometry and material
			tmpGeo.clear();
			material = _defaultMaterial;
		}

		//  vertex?
		if (_strnicmp(cBuffer, "v ", 2) == 0)
		{
			//  get vector from line
			sscanf(cBuffer, "v %f %f %f", &(tVector.x), &(tVector.y), &(tVector.z));

			//  scale final vertex
			tVector *= 0.0143f;

			//  add vertex to array
			vertAry.pushBack(hkVector4(tVector.x, tVector.y, tVector.z));
		}
		//  face?
		else if (_strnicmp(cBuffer, "f ", 2) == 0)
		{
			//  get triangle idx from line
			for (idx=0, pChar=strchr(cBuffer, ' '); ((pChar != NULL) && (idx < 3)); ++idx, pChar = strchr(pChar, ' '))
			{
				tIntAry[idx] = atoi(++pChar) - faceOffset;
			}

			//  create new triangle and add to array
			hkGeometry::Triangle	tTri;

			tTri.set(tIntAry[0], tIntAry[1], tIntAry[2], material);
			triAry.pushBack(tTri);

			//  mark existing face
			hasFace = true;
		}
		//  material?
		else if (_strnicmp(cBuffer, "usemtl ", 7) == 0)
		{
			//  get material from material table
			material = _materialList.getMaterialCode(cBuffer+7);

			//  unknown material => use default
			if (material == 0)		material = _defaultMaterial;
		}
	}  //  while (inFile.good())

	//  existing last/only face? => create new geometry
	if (hasFace)
	{
		//  add geometry to result array
		geometryMap.push_back(tmpGeo);

	}  //  if (hasFace)

	//  close file
	inFile.close();

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromNifFile(string fileName, vector<hkGeometry>& geometryMap)
{
	NiNodeRef				pRootInput (NULL);
	bhkCollisionObjectRef	pCollObject(NULL);
	vector<NiAVObjectRef>	srcChildList;
	vector<Matrix44>		transformAry;
	vector<hkGeometry>		geometryMapColl;
	vector<hkGeometry>		geometryMapShape;
	bool					fakedRoot  (false);

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName, "collSource", fakedRoot, true)) == NULL)
	{
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get geometry from collision object
	pCollObject = DynamicCast<bhkCollisionObject>(pRootInput->GetCollisionObject());
	if (pCollObject != NULL)
	{
		//  search for embedded shape
		bhkRigidBodyRef		pRBody(DynamicCast<bhkRigidBody>(pCollObject->GetBody()));

		if (pRBody != NULL)
		{
			getGeometryFromCollShape(pRBody->GetShape(), geometryMapColl, transformAry);
		}
	}  //  if (pCollObject != NULL)

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  iterate over source nodes and get geometry
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryMapShape, transformAry);
		}
		//  NiTriStrips
		else if (DynamicCast<NiTriStrips>(*ppIter) != NULL)
		{
			getGeometryFromTriStrips(DynamicCast<NiTriStrips>(*ppIter), geometryMapShape, transformAry);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(&(*DynamicCast<RootCollisionNode>(*ppIter)), geometryMapColl, geometryMapColl, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryMapShape, geometryMapColl, transformAry);
		}
	}

	//  which geomertry should be used?
	if ((_cnHandling == NCU_CN_COLLISION) || (_cnHandling == NCU_CN_FALLBACK))
	{
		geometryMap.swap(geometryMapColl);
	}
	else if (_cnHandling == NCU_CN_SHAPES)
	{
		geometryMap.swap(geometryMapShape);
	}
	if ((_cnHandling == NCU_CN_FALLBACK) && (geometryMap.size() <= 0))
	{
		geometryMap.swap(geometryMapShape);
	}

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
NiNodeRef NifCollisionUtility::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot, bool setVersion)
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
		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiNode.");
	}
	//  NiTriShape as root
	else if (DynamicCast<NiTriShape>(pRootTree) != NULL)
	{
		//  create faked root
		pRootInput = new NiNode();

		//  add root as child
		pRootInput->AddChild(DynamicCast<NiAVObject>(pRootTree));

		//  mark faked root node
		fakedRoot = true;

		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiTriShape.");
	}

	//  no known root type found
	if (pRootInput == NULL)
	{
		logMessage(NCU_MSG_TYPE_WARNING, logPreText + " root has unhandled type: " + pRootTree->GetType().GetTypeName());
	}

	//  get nif version of collision source (special handling for NifConvert)
	if (setVersion)
	{
		_nifVersion = (nifInfo.creator == "NifConvert") ? atol(nifInfo.exportInfo2.c_str()) : nifInfo.version;
	}

	return pRootInput;
}

/*---------------------------------------------------------------------------*/
bool NifCollisionUtility::collSourceHasCollNodes(string fileNameCollSrc)
{
	vector<NiObjectRef>		blockList  (ReadNifList(fileNameCollSrc));
	bool					hasCollNode(false);

	//  find collision node (bhkCompressedMeshData)
	for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)
	{
		if (DynamicCast<RootCollisionNode>(*pIter) != NULL)
		{
			hasCollNode = true;
			break;
		}
		else if (DynamicCast<bhkCollisionObject>(*pIter) != NULL)
		{
			hasCollNode = true;
			break;
		}
	}  //  for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)

	return hasCollNode;
}

/*---------------------------------------------------------------------------*/
bhkCollisionObjectRef NifCollisionUtility::createCollNode(vector<hkGeometry>& geometryMap, bhkCollisionObjectRef pTmplNode, NiNodeRef pRootNode)
{
	//  template collision node will be output collision node. it's unlinked from root in calling function
	bhkCollisionObjectRef	pDstNode(pTmplNode);

	//  parse collision node subtrees and correct targets
	bhkRigidBodyRef		pRigidBody(DynamicCast<bhkRigidBody>(pDstNode->GetBody()));

	//  bhkRigidBody found
	if (pRigidBody != NULL)
	{
		bhkMoppBvTreeShapeRef	pTmplMopp(DynamicCast<bhkMoppBvTreeShape>(pRigidBody->GetShape()));

		//  bhkMoppBvTreeShape found
		if (pTmplMopp != NULL)
		{
			bhkCompressedMeshShapeRef	pTmplCShape(DynamicCast<bhkCompressedMeshShape>(pTmplMopp->GetShape()));

			//  bhkCompressedMeshShape found
			if (pTmplCShape != NULL)
			{
				bhkCompressedMeshShapeDataRef	pData(pTmplCShape->GetData());

				//  bhkCompressedMeshShapeData found
				if (pData != NULL)
				{
					//  fill in Havok data into Nif structures
					injectCollisionData(geometryMap, pTmplMopp, pData, pRigidBody);

					//  set new target
					pTmplCShape->SetTarget(pRootNode);

				}  //  if (pData != NULL)
			}  //  if (pTmplCShape != NULL)
		}  //  if (pTmplMopp != NULL)
	}  //  if (pRigidBody != NULL)

	//  remove target from destination node
	pDstNode->SetTarget(NULL);

	return pDstNode;
}

/*---------------------------------------------------------------------------*/
bool NifCollisionUtility::injectCollisionData(vector<hkGeometry>& geometryMap, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData, bhkRigidBodyRef pRigidBody)
{
	if (pMoppShape == NULL)   return false;
	if (pData      == NULL)   return false;
	if (geometryMap.empty())  return false;

	//----  Havok  ----  START
	hkpCompressedMeshShape*					pCompMesh  (NULL);
	hkpMoppCode*							pMoppCode  (NULL);
	hkpMoppBvTreeShape*						pMoppBvTree(NULL);
	hkpCompressedMeshShapeBuilder			shapeBuilder;
	hkpMoppCompilerInput					mci;
	vector<int>								geometryIdxVec;
	vector<bhkCMSDMaterial>					tMtrlVec;
	SkyrimHavokMaterial						material   (SKY_HAV_MAT_STONE);
	int										subPartId  (0);
	int										tChunkSize (0);

	//  initialize shape Builder
	shapeBuilder.m_stripperPasses = 5000;

	//  create compressedMeshShape
	pCompMesh = shapeBuilder.createMeshShape(0.001f, hkpCompressedMeshShape::MATERIAL_SINGLE_VALUE_PER_CHUNK);

	//  add geometries to compressedMeshShape
	for (vector<hkGeometry>::iterator geoIter = geometryMap.begin(); geoIter != geometryMap.end(); geoIter++)
	{
		size_t		matIdx(0);

		//  determine material index
		material = (SkyrimHavokMaterial) geoIter->m_triangles[0].m_material;

		//  material already known?
		for (matIdx=0; matIdx < tMtrlVec.size(); ++matIdx)
		{
			if (tMtrlVec[matIdx].skyrimMaterial == material)		break;
		}

		//  add new material?
		if (matIdx >= tMtrlVec.size())
		{
			bhkCMSDMaterial		tChunkMat;

			//  create single material
			tChunkMat.skyrimMaterial = material;
			tChunkMat.skyrimLayer    = pRigidBody->GetSkyrimLayer();

			//  add material to list
			tMtrlVec.push_back(tChunkMat);
		}

		//  set material index to each triangle of geometry
		for (int idx(0); idx < geoIter->m_triangles.getSize(); ++idx)
		{
			geoIter->m_triangles[idx].m_material = matIdx;
		}

		//  add geometry to shape
		subPartId = shapeBuilder.beginSubpart(pCompMesh);
		shapeBuilder.addGeometry(*geoIter, hkMatrix4::getIdentity(), pCompMesh);
		shapeBuilder.endSubpart(pCompMesh);
		shapeBuilder.addInstance(subPartId, hkMatrix4::getIdentity(), pCompMesh);

	}  //  for (vector<hkGeometry>::iterator geoIter = geometryMap.begin(); geoIter != geometryMap.end(); geoIter++)

	//  create welding info
	mci.m_enableChunkSubdivision = false;  //  PC version
	pMoppCode   = hkpMoppUtility::buildCode(pCompMesh, mci);
	pMoppBvTree = new hkpMoppBvTreeShape(pCompMesh, pMoppCode);
	hkpMeshWeldingUtility::computeWeldingInfo(pCompMesh, pMoppBvTree, hkpWeldingUtility::WELDING_TYPE_TWO_SIDED);
	//----  Havok  ----  END

	//----  Merge  ----  START
	hkArray<hkpCompressedMeshShape::Chunk>  chunkListHvk;
	vector<bhkCMSDChunk>                    chunkListNif = pData->GetChunks();
	vector<Vector4>                         tVec4Vec;
	vector<bhkCMSDBigTris>                  tBTriVec;
	vector<bhkCMSDTransform>                tTranVec;
	map<unsigned int, bhkCMSDMaterial>		tMtrlMap;
	short                                   chunkIdxNif(0);

	//  --- modify MoppBvTree ---
	//  set origin
	pMoppShape->SetMoppOrigin(Vector3(pMoppBvTree->getMoppCode()->m_info.m_offset(0), pMoppBvTree->getMoppCode()->m_info.m_offset(1), pMoppBvTree->getMoppCode()->m_info.m_offset(2)));

	//  set scale
	pMoppShape->SetMoppScale(pMoppBvTree->getMoppCode()->m_info.getScale());

	//  set build Type
	pMoppShape->SetBuildType(MoppDataBuildType((Niflib::byte) pMoppCode->m_buildType));

	//  copy mopp data
	pMoppShape->SetMoppCode(vector<Niflib::byte>(pMoppBvTree->m_moppData, pMoppBvTree->m_moppData+pMoppBvTree->m_moppDataSize));

	//  set boundings
	pData->SetBoundsMin(Vector4(pCompMesh->m_bounds.m_min(0), pCompMesh->m_bounds.m_min(1), pCompMesh->m_bounds.m_min(2), pCompMesh->m_bounds.m_min(3)));
	pData->SetBoundsMax(Vector4(pCompMesh->m_bounds.m_max(0), pCompMesh->m_bounds.m_max(1), pCompMesh->m_bounds.m_max(2), pCompMesh->m_bounds.m_max(3)));

	//  resize and copy bigVerts
	pData->SetNumBigVerts(pCompMesh->m_bigVertices.getSize());
	tVec4Vec = pData->GetBigVerts();
	tVec4Vec.resize(pData->GetNumBigVerts());
	for (unsigned int idx(0); idx < pData->GetNumBigVerts(); ++idx)
	{
		tVec4Vec[idx].x = pCompMesh->m_bigVertices[idx](0);
		tVec4Vec[idx].y = pCompMesh->m_bigVertices[idx](1);
		tVec4Vec[idx].z = pCompMesh->m_bigVertices[idx](2);
		tVec4Vec[idx].w = pCompMesh->m_bigVertices[idx](3);
	}
	pData->SetBigVerts(tVec4Vec);

	//  resize and copy bigTris
	pData->SetNumBigTris(pCompMesh->m_bigTriangles.getSize());
	tBTriVec = pData->GetBigTris();
	tBTriVec.resize(pData->GetNumBigTris());
	for (unsigned int idx(0); idx < pData->GetNumBigTris(); ++idx)
	{
		tBTriVec[idx].triangle1     = pCompMesh->m_bigTriangles[idx].m_a;
		tBTriVec[idx].triangle2     = pCompMesh->m_bigTriangles[idx].m_b;
		tBTriVec[idx].triangle3     = pCompMesh->m_bigTriangles[idx].m_c;
		tBTriVec[idx].unknownInt1   = pCompMesh->m_bigTriangles[idx].m_material;
		tBTriVec[idx].unknownShort1 = pCompMesh->m_bigTriangles[idx].m_weldingInfo;
	}
	pData->SetBigTris(tBTriVec);

	//  resize and copy transform data
	pData->SetNumTransforms(pCompMesh->m_transforms.getSize());
	tTranVec = pData->GetChunkTransforms();
	tTranVec.resize(pData->GetNumTransforms());
	for (unsigned int idx(0); idx < pData->GetNumTransforms(); ++idx)
	{
		tTranVec[idx].translation.x = pCompMesh->m_transforms[idx].m_translation(0);
		tTranVec[idx].translation.y = pCompMesh->m_transforms[idx].m_translation(1);
		tTranVec[idx].translation.z = pCompMesh->m_transforms[idx].m_translation(2);
		tTranVec[idx].translation.w = pCompMesh->m_transforms[idx].m_translation(3);
		tTranVec[idx].rotation.x    = pCompMesh->m_transforms[idx].m_rotation(0);
		tTranVec[idx].rotation.y    = pCompMesh->m_transforms[idx].m_rotation(1);
		tTranVec[idx].rotation.z    = pCompMesh->m_transforms[idx].m_rotation(2);
		tTranVec[idx].rotation.w    = pCompMesh->m_transforms[idx].m_rotation(3);
	}
	pData->SetChunkTransforms(tTranVec);

	//  set material list
	pData->SetChunkMaterials(tMtrlVec);

	//  get chunk list from mesh
	chunkListHvk = pCompMesh->m_chunks;

	// resize nif chunk list
	chunkListNif.resize(chunkListHvk.getSize());

	//  for each chunk
	for (hkArray<hkpCompressedMeshShape::Chunk>::iterator pCIterHvk = pCompMesh->m_chunks.begin(); pCIterHvk != pCompMesh->m_chunks.end(); pCIterHvk++)
	{
		//  get nif chunk
		bhkCMSDChunk&	chunkNif = chunkListNif[chunkIdxNif];

		//  set offset => translation
		chunkNif.translation.x = pCIterHvk->m_offset(0);
		chunkNif.translation.y = pCIterHvk->m_offset(1);
		chunkNif.translation.z = pCIterHvk->m_offset(2);
		chunkNif.translation.w = pCIterHvk->m_offset(3);

		//  force flags to fixed values
		chunkNif.materialIndex  = pCIterHvk->m_materialInfo;
		chunkNif.unknownShort1  = 65535;
		chunkNif.transformIndex = pCIterHvk->m_transformIndex;

		//  vertices
		chunkNif.numVertices = pCIterHvk->m_vertices.getSize();
		chunkNif.vertices.resize(chunkNif.numVertices);
		for (unsigned int i(0); i < chunkNif.numVertices; ++i)
		{
			chunkNif.vertices[i] = pCIterHvk->m_vertices[i];
		}

		//  indices
		chunkNif.numIndices = pCIterHvk->m_indices.getSize();
		chunkNif.indices.resize(chunkNif.numIndices);
		for (unsigned int i(0); i < chunkNif.numIndices; ++i)
		{
			chunkNif.indices[i] = pCIterHvk->m_indices[i];
		}

		//  strips
		chunkNif.numStrips = pCIterHvk->m_stripLengths.getSize();
		chunkNif.strips.resize(chunkNif.numStrips);
		for (unsigned int i(0); i < chunkNif.numStrips; ++i)
		{
			chunkNif.strips[i] = pCIterHvk->m_stripLengths[i];
		}

		//  welding
		chunkNif.numIndices2 = pCIterHvk->m_weldingInfo.getSize();
		chunkNif.indices2.resize(chunkNif.numIndices2);
		for (unsigned int i(0); i < chunkNif.numIndices2; ++i)
		{
			chunkNif.indices2[i] = pCIterHvk->m_weldingInfo[i];
		}

		//  next chunk
		++chunkIdxNif;

	}  //  for (hkArray<hkpCompressedMeshShape::Chunk>::iterator pCIterHvk = 

	//  set modified chunk list to compressed mesh shape data
	pData->SetChunks(chunkListNif);
	//----  Merge  ----  END

	return true;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::logMessage(int type, string text)
{
	//  add message to internal storages
	switch (type)
	{
		default:
		case NCU_MSG_TYPE_INFO:
		case NCU_MSG_TYPE_WARNING:
		case NCU_MSG_TYPE_ERROR:
		{
			_userMessages.push_back(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE:
		{
			_usedTextures.insert(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE_MISS:
		{
//			_newTextures.insert(text);
			break;
		}
	}

	//  send message to callback if given
	if (_logCallback != NULL)
	{
		(*_logCallback)(type, text.c_str());
	}
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::cleanTreeCollision(NiNodeRef pNode)
{
	vector<NiAVObjectRef>	srcChildList(pNode->GetChildren());		//  children of node

	//  remove collision object (new style [>= Oblivion])
	pNode->SetCollisionObject(NULL);

	//  iterate over source nodes and remove possible old-style [Morrowind] collision node
	for (auto  ppIter=srcChildList.begin(), pEnd=srcChildList.end(); ppIter != pEnd; ppIter++)
	{
		//  RootCollisionNode
		if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			pNode->RemoveChild(*ppIter);
		}
		//  NiNode
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			cleanTreeCollision(DynamicCast<NiNode>(*ppIter));
		}
		//  other children
		else
		{
			(*ppIter)->SetCollisionObject(NULL);
		}
	}  //  for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ....
}

/*---------------------------------------------------------------------------*/
bool NifCollisionUtility::parseTreeCollision(NiNodeRef pNode, string fileNameCollTmpl, vector<hkGeometry>& geometryMapColl)
{
	vector<NiAVObjectRef>	srcChildList (pNode->GetChildren());	//  get list of children from input node
	bool					haveCollision(false);

	//  check for own collision object
	if (DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()) != NULL)
	{
		bhkCollisionObjectRef	pCollObj(DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()));
		bhkRigidBodyRef			pRBody  (DynamicCast<bhkRigidBody>      (pCollObj->GetBody()));

		//  check on valid body
		if (pRBody != NULL)
		{
			array<7, unsigned short>	unknown7(pRBody->GetUnknown7Shorts());

			//  force SKYRIM values
			unknown7[3] = 0;
			unknown7[4] = 0;
			unknown7[5] = 1;
			unknown7[6] = 65535;
			pRBody->SetUnknown7Shorts(unknown7);
			pRBody->SetTranslation(pRBody->GetTranslation() * 0.1f);

			pRBody->SetShape(parseCollisionShape(fileNameCollTmpl, pRBody->GetShape(), pRBody));

		}  //  if (pRBody != NULL)

		haveCollision |= true;

	}  //  if (DynamicCast<bhkCollisionObject>(pNode->GetCollisionObject()) != NULL)

	//  iterate over source nodes and get geometry
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  RootCollisionNode
		if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			NiNodeRef				pRootTemplate(DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameCollTmpl.c_str())));
			vector<hkGeometry>		geometryMapCollLocal;
			vector<Matrix44>		transformAryLocal;

			//  get template
			if (pRootTemplate != NULL)
			{
				bhkCollisionObjectRef	pCollNodeTmpl(DynamicCast<bhkCollisionObject>(pRootTemplate->GetCollisionObject()));

				if (pCollNodeTmpl != NULL)
				{
					//  get collision data from sub-nodes
					getGeometryFromNode(&(*DynamicCast<RootCollisionNode>(*ppIter)), geometryMapCollLocal, geometryMapCollLocal, transformAryLocal);

					//  replace collision object
					pNode->SetCollisionObject(createCollNode(geometryMapCollLocal, pCollNodeTmpl, pNode));
					pNode->RemoveChild(*ppIter);

					haveCollision |= true;

				}  //  if (pCollNodeTmpl == NULL)
			}  //  if (pRootTemplate != NULL)
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			haveCollision |= parseTreeCollision(DynamicCast<NiNode>(*ppIter), fileNameCollTmpl, geometryMapColl);
		}
	}  //  for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)

	return haveCollision;
}

/*---------------------------------------------------------------------------*/
bhkShapeRef NifCollisionUtility::parseCollisionShape(string fileNameCollTmpl, bhkShapeRef pShape, bhkRigidBodyRef pRigidBody, bhkMoppBvTreeShapeRef pMoppShape)
{
	bhkShapeRef		pShapeOut(pShape);
	float			factor   (0.1f);

	//@TODO:  modify factor depending on _nifVersion if necessary


	//  check type of collision mesh
	//bhkMoppBvTreeShape
	if (DynamicCast<bhkMoppBvTreeShape>(pShape) != NULL)
	{
		bhkMoppBvTreeShapeRef	pMoppBvTreeShape(DynamicCast<bhkMoppBvTreeShape>(pShape));

		pMoppBvTreeShape->SetShape(parseCollisionShape(fileNameCollTmpl, pMoppBvTreeShape->GetShape(), pRigidBody, pMoppBvTreeShape));
		//!!  we don't generate new MOPP data here because only
		//!!  sizes of sub-shapes were changed, not layout
	}
	//bhkTransformShape
	else if (DynamicCast<bhkTransformShape>(pShape) != NULL)
	{
		bhkTransformShapeRef	pShapeRef(DynamicCast<bhkTransformShape>(pShape));
		Matrix44				transform(pShapeRef->GetTransform());

		transform[0][3] *= factor;
		transform[1][3] *= factor;
		transform[2][3] *= factor;
		pShapeRef->SetTransform(transform);
		pShapeRef->SetShape(parseCollisionShape(fileNameCollTmpl, pShapeRef->GetShape(), pRigidBody, pMoppShape));
		pShapeOut = pShapeRef;
	}
	//bhkListShape
	else if (DynamicCast<bhkListShape>(pShape) != NULL)
	{
		bhkListShapeRef			pTShape(DynamicCast<bhkListShape>(pShape));
		vector<bhkShapeRef>		subShapes(pTShape->GetSubShapes());
		vector<bhkShapeRef>		newShapes;

		//  parse sub shape(s)
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
		{
			//  parse sub shape(s)
			newShapes.push_back(parseCollisionShape(fileNameCollTmpl, *pIter, pRigidBody, pMoppShape));
		}

		//  set new shape list
		pTShape->SetSubShapes(newShapes);
	}
	//bhkPackedNiTriStripsShape
	else if (DynamicCast<bhkPackedNiTriStripsShape>(pShape) != NULL)
	{
		pShapeOut = convertCollPackedNiTriStrips(DynamicCast<bhkPackedNiTriStripsShape>(pShape), pMoppShape, pRigidBody, fileNameCollTmpl);
		//!!  we might have to generate new MOPP data here
		//!!  because layout of sub-shapes was changed, too
	}
	//bhkBoxShape
	else if (DynamicCast<bhkBoxShape>(pShape) != NULL)
	{
		bhkBoxShapeRef	pShapeRef(DynamicCast<bhkBoxShape>(pShape));

		pShapeRef->SetDimensions(pShapeRef->GetDimensions() * factor);
		pShapeOut = pShapeRef;
	}
	//bhkCapsuleShape
	else if (DynamicCast<bhkCapsuleShape>(pShape) != NULL)
	{
		bhkCapsuleShapeRef	pShapeRef(DynamicCast<bhkCapsuleShape>(pShape));

		pShapeRef->SetFirstPoint (pShapeRef->GetFirstPoint()  * factor);
		pShapeRef->SetSecondPoint(pShapeRef->GetSecondPoint() * factor);
		pShapeRef->SetRadius     (pShapeRef->GetRadius()      * factor);
		pShapeRef->SetRadius1    (pShapeRef->GetRadius1()     * factor);
		pShapeRef->SetRadius2    (pShapeRef->GetRadius2()     * factor);
		pShapeOut = pShapeRef;
	}
	//bhkSphereShape
	else if (DynamicCast<bhkSphereShape>(pShape) != NULL)
	{
		bhkSphereShapeRef	pShapeRef(DynamicCast<bhkSphereShape>(pShape));

		pShapeRef->SetRadius(pShapeRef->GetRadius() * factor);
		pShapeOut = pShapeRef;
	}
	//bhkConvexVerticesShape
	else if (DynamicCast<bhkConvexVerticesShape>(pShape) != NULL)
	{
		bhkConvexVerticesShapeRef	pShapeRef(DynamicCast<bhkConvexVerticesShape>(pShape));
		vector<Vector3>				vertList (pShapeRef->GetVertices());

		for (auto pIter=vertList.begin(), pEnd=vertList.end(); pIter != pEnd; ++pIter)
		{
			*pIter *= factor;
		}
		pShapeRef->SetVertices(vertList);
		pShapeOut = pShapeRef;
	}

	return pShapeOut;
}

/*---------------------------------------------------------------------------*/
bhkShapeRef NifCollisionUtility::convertCollPackedNiTriStrips(bhkPackedNiTriStripsShapeRef pShape, bhkMoppBvTreeShapeRef pMoppShape, bhkRigidBodyRef pRigidBody, string fileNameCollTmpl)
{
	bhkShapeRef			pShapeOut(pShape);
	vector<hkGeometry>	geometryMap;
	vector<Matrix44>	transformAry;

	//  get geometries from PackedNiTriStrips
	getGeometryFromPackedNiTriStrips(pShape, geometryMap, transformAry);

	//  geometry found? => create collision data
	if (!geometryMap.empty())
	{
		NiNodeRef	pRootTemplate(DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameCollTmpl.c_str())));

		//  get template
		if (pRootTemplate != NULL)
		{
			bhkCollisionObjectRef	pCollNodeTmpl(DynamicCast<bhkCollisionObject>(pRootTemplate->GetCollisionObject()));

			//  bhkCollisionObject found
			if (pCollNodeTmpl != NULL)
			{
				//  parse collision node subtrees and correct targets
				bhkRigidBodyRef		pRigidBody(DynamicCast<bhkRigidBody>(pCollNodeTmpl->GetBody()));

				//  bhkRigidBody found
				if (pRigidBody != NULL)
				{
					bhkMoppBvTreeShapeRef	pTmplMopp(DynamicCast<bhkMoppBvTreeShape>(pRigidBody->GetShape()));

					//  bhkMoppBvTreeShape found
					if (pTmplMopp != NULL)
					{
						bhkCompressedMeshShapeRef	pTmplCShape(DynamicCast<bhkCompressedMeshShape>(pTmplMopp->GetShape()));

						//  bhkCompressedMeshShape found
						if (pTmplCShape != NULL)
						{
							bhkCompressedMeshShapeDataRef	pData(pTmplCShape->GetData());

							//  bhkCompressedMeshShapeData found
							if (pData != NULL)
							{
								//  set bhkCompressedMeshShape to existing bhkMoppBvTreeShape
								pTmplMopp->SetShape (NULL);
								pMoppShape->SetShape(pTmplCShape);

								//  fill in Havok data into Nif structures
								injectCollisionData(geometryMap, pMoppShape, pData, pRigidBody);

								//  set result shape
								pShapeOut = pTmplCShape;

							}  //  if (pData != NULL)
						}  //  if (pTmplCShape != NULL)
					}  //  if (pTmplMopp != NULL)
				}  //  if (pRigidBody != NULL)
			}  //  if (pCollNodeTmpl == NULL)
		}  //  if (pRootTemplate != NULL)
	}  //  if (!geometryMap.empty())

	return pShapeOut;
}

/*---------------------------------------------------------------------------*/
void NifCollisionUtility::reorderTriangles(hkArray<hkGeometry::Triangle>& srcAry)
{
	int		length  (srcAry.getSize() - 1);
	short	idxGroup(0);

	//  for each triangle in list
	for (int idx(0); idx < length; ++idx)
	{
		hkGeometry::Triangle&	dstTri(srcAry[idx]);
		hkGeometry::Triangle&	srcTri(srcAry[idx+1]);
		int						vertsDst[3] = {dstTri.m_a, dstTri.m_b, dstTri.m_c};
		int						vertsSrc[3] = {srcTri.m_a, srcTri.m_b, srcTri.m_c};
		short					idxDst(0);
		short					idxSrc(0);
		short					cntPts(0);

		//  find common edge
		for (; idxDst < 3; ++idxDst)
		{
			for (idxSrc=0; idxSrc < 3; ++idxSrc)
			{
				if (vertsSrc[idxSrc] == vertsDst[idxDst])	break;
			}

			if (idxSrc > 2)
			{
				vertsDst[idxDst] = -1;
			}
		}

		for (idxDst=0; idxDst < 3; ++idxDst)
		{
			if (vertsDst[idxDst] != -1)		++cntPts;
		}

		//  swap two vertices in case of uneven triangle in group having common edge with precessor
		if ((cntPts == 2) && ((idxGroup % 2) == 1))
		{
			int	tmp(srcTri.m_a);

			srcTri.m_a = srcTri.m_c;
			srcTri.m_c = tmp;
		}
		//  no common edge => start new group
		else if (cntPts != 2)
		{
			idxGroup = 0;
		}

		//  increase index in group
		++idxGroup;

	}  //  for (int idxRes(0); idxRes < length; ++idxRes)
}
