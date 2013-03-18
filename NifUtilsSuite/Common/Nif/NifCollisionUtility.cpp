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
#include "obj/bhkCompressedMeshShape.h"
#include "obj/rootcollisionnode.h"

//  Havok includes
#include "Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h"
#include "Physics/Collide/Shape/Compound/Collection/CompressedMesh/hkpCompressedMeshShapeBuilder.h"
#include "Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h"
#include "Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h"

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
		_generateNormals   (true)
{}

/*---------------------------------------------------------------------------*/
NifCollisionUtility::~NifCollisionUtility()
{}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromTriShape(NiTriShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		hkGeometry						tmpGeo;
		vector<Vector3>					vertices (pData->GetVertices());
		vector<Triangle>				triangles(pData->GetTriangles());
		hkArray<hkVector4>&				vertAry  (tmpGeo.m_vertices);
		hkArray<hkGeometry::Triangle>&	triAry   (tmpGeo.m_triangles);
		Vector3							tVector;
		unsigned int					material (_defaultMaterial);

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
			tVector *= 0.0143f;

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

	}  //  if (pData != NULL)

	return geometryMap.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::getGeometryFromNode(NiNodeRef pNode, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());

	//  add own translation to list
	transformAry.push_back(pNode->GetLocalTransform());

	//  iterate over children
	for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryMap, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryMap, transformAry);
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
	NiNodeRef				pRootInput     (NULL);
	vector<NiAVObjectRef>	srcChildList;
	vector<Matrix44>		transformAry;
	vector<hkGeometry>		geometryMapColl;
	vector<hkGeometry>		geometryMapShape;
	bool					fakedRoot      (false);

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName, "collSource", fakedRoot)) == NULL)
	{
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  add own transform to list
	//  BUGFIX:  don't consider transform of root node for collision 'cause used for both: collision AND shapes
	//transformAry.push_back(pRootInput->GetLocalTransform());

	//  iterate over source nodes and get geometry
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryMapShape, transformAry);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(&(*DynamicCast<RootCollisionNode>(*ppIter)), geometryMapColl, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryMapShape, transformAry);
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
NiNodeRef NifCollisionUtility::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot)
{
	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str());

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

	return pRootInput;
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::addCollision(string fileNameCollSrc, string fileNameNifDst, string fileNameCollTmpl)
{
	NiNodeRef				pRootInput   (NULL);
	NiNodeRef				pRootTemplate(NULL);
	bhkCollisionObjectRef	pCollNodeTmpl(NULL);
	vector<hkGeometry>		geometryMap;
	vector<NiAVObjectRef>	srcChildList;
	bool					fakedRoot    (false);

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
	if ((pRootInput = getRootNodeFromNifFile(fileNameNifDst, "target", fakedRoot)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameNifDst + "' as template");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  create and add collision node to target
	pRootInput->SetCollisionObject(createCollNode(geometryMap, pCollNodeTmpl, pRootInput));

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  iterate over source nodes and remove possible old-style collision node
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  RootCollisionNode
		if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			pRootInput->RemoveChild(*ppIter);
		}
	}  //  for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ....

	//  write modified nif file
	WriteNifTree((const char*) fileNameNifDst.c_str(), pRootInput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
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
	mci.m_enableChunkSubdivision = true;
	pMoppCode   = hkpMoppUtility::buildCode(pCompMesh, mci);
	pMoppBvTree = new hkpMoppBvTreeShape(pCompMesh, pMoppCode);
	hkpMeshWeldingUtility::computeWeldingInfo(pCompMesh, pMoppBvTree, hkpWeldingUtility::WELDING_TYPE_TWO_SIDED);
	//----  Havok  ----  END

	//----  Merge  ----  START
	hkArray<hkpCompressedMeshShape::Chunk>  chunkListHvk;
	vector<bhkCMSDChunk>                    chunkListNif = pData->GetChunks();
	vector<Niflib::byte>                    tByteVec;
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

	//  copy mopp data
	tByteVec.resize(pMoppBvTree->m_moppDataSize);
	tByteVec[0] = pMoppBvTree->m_moppData[pMoppBvTree->m_moppDataSize - 1];
	for (hkUint32 i(0); i < (pMoppBvTree->m_moppDataSize - 1); ++i)
	{
		tByteVec[i+1] = pMoppBvTree->m_moppData[i];
	}
	pMoppShape->SetMoppCode(tByteVec);

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
void NifCollisionUtility::setGenerateNormals(const bool genNormals)
{
	_generateNormals = genNormals;
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
unsigned int NifCollisionUtility::getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeDataRef pShape, vector<NifChunkData>& chunkDataList)
{
	vector<bhkCMSDChunk>		chunkGeoList(pShape->GetChunks());
	vector<bhkCMSDMaterial>		chunkMatList(pShape->GetChunkMaterials());
	unsigned short				idxChunk(0);

	//  for each chunk of shape
	for (auto pIter=chunkGeoList.begin(), pEnd=chunkGeoList.end(); pIter != pEnd; ++pIter, ++idxChunk)
	{
		NifChunkData	tmpCData;
		unsigned int	offI(0);

		//  set index and material
		tmpCData._index    = idxChunk;
		tmpCData._material = chunkMatList[pIter->materialIndex].skyrimMaterial;

		//  get vertices from chunk
		for (unsigned int idx(0); idx < pIter->numVertices; idx += 3)
		{
			tmpCData._vertices.push_back(Vector3((pIter->vertices[idx  ] / 1000.0f) + pIter->translation.x,
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
					tmpCData._triangles.push_back(Triangle(pIter->indices[idxI], pIter->indices[idxI+1], pIter->indices[idxI+2]));
				}
				else
				{
					tmpCData._triangles.push_back(Triangle(pIter->indices[idxI+2], pIter->indices[idxI+1], pIter->indices[idxI]));
				}
			}
			
			//  set next index
			offI += cntI;

		}  //  for (unsigned int idxS(0); idxS < pIter->numStrips; ++idxS)

		//  get faces defined by indices only
		for (unsigned int idxI(offI); idxI < pIter->numIndices; idxI += 3, offI += 3)
		{
			tmpCData._triangles.push_back(Triangle(pIter->indices[idxI], pIter->indices[idxI+1], pIter->indices[idxI+2]));
		}

		//  generate normals if set
		if (_generateNormals)
		{
			unsigned int	size(tmpCData._vertices.size());

			//  initialize normal vector
			tmpCData._normals.resize(size);

			//  for each face
			for (auto pIter=tmpCData._triangles.begin(), pEnd=tmpCData._triangles.end(); pIter != pEnd; ++pIter)
			{
				Vector3		faceNormal((tmpCData._vertices[pIter->v2] - tmpCData._vertices[pIter->v1]).CrossProduct((tmpCData._vertices[pIter->v3] - tmpCData._vertices[pIter->v1])));

				//  add values to normals
				tmpCData._normals[pIter->v1] += faceNormal;
				tmpCData._normals[pIter->v2] += faceNormal;
				tmpCData._normals[pIter->v3] += faceNormal;
			}

			//  normalize normals
			for (unsigned int idx(0); idx < size; ++idx)
			{
				tmpCData._normals[idx] = tmpCData._normals[idx].Normalized();
			}
		}  //  if (_generateNormals)

		// append geometry to list
		chunkDataList.push_back(tmpCData);

	}  //  for (auto pIter=chunkGeoList.begin(), pEnd=chunkGeoList.end(); pIter != pEnd; ++pIter)

	//  decode BigTris
	vector<bhkCMSDBigTris>		tBTriVec   (pShape->GetBigTris());
	vector<Vector4>				tBVecVec   (pShape->GetBigVerts());
	unsigned int				idxBigStart(chunkDataList.size());

	//  if existing
	if (!tBVecVec.empty() && !tBTriVec.empty())
	{
		NifChunkData*						pTmpCData(NULL);
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

		//  generate normals if set
		if (_generateNormals)
		{
			NifChunkData*	pTmpCData(NULL);

			for (unsigned int idx(idxBigStart); idx < chunkDataList.size(); ++idx)
			{
				pTmpCData = &chunkDataList[idx];

				unsigned int	size(pTmpCData->_vertices.size());

				//  initialize normal vector
				pTmpCData->_normals.resize(size);

				//  for each face
				for (auto pIter=pTmpCData->_triangles.begin(), pEnd=pTmpCData->_triangles.end(); pIter != pEnd; ++pIter)
				{
					Vector3		faceNormal((pTmpCData->_vertices[pIter->v2] - pTmpCData->_vertices[pIter->v1]).CrossProduct((pTmpCData->_vertices[pIter->v3] - pTmpCData->_vertices[pIter->v1])));

					//  add values to normals
					pTmpCData->_normals[pIter->v1] += faceNormal;
					pTmpCData->_normals[pIter->v2] += faceNormal;
					pTmpCData->_normals[pIter->v3] += faceNormal;
				}

				//  normalize normals
				for (unsigned int idx(0); idx < size; ++idx)
				{
					pTmpCData->_normals[idx] = pTmpCData->_normals[idx].Normalized();
				}
			}  //  for (unsigned int idx(idxBigStart); idx < chunkDataList.size(); ++idx)
		}  //  if (_generateNormals)
	}  //  if (!tBVecVec.empty() && !tBTriVec.empty())

	return chunkDataList.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifCollisionUtility::extractChunks(string fileNameCollSrc, string fileNameDstNif, string fileNameDstObj)
{
	bool					fakedRoot(false);
	vector<NiObjectRef>		blockList(ReadNifList(fileNameCollSrc));
	vector<NifChunkData>	chunkDataList;

	//  test on existing file names
	if (fileNameCollSrc.empty())								return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDstNif.empty() && fileNameDstObj.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "CollSource:  " + (fileNameCollSrc.empty() ? "- none -" : fileNameCollSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Name NIF:    " + (fileNameDstNif.empty()  ? "- none -" : fileNameDstNif));
	logMessage(NCU_MSG_TYPE_INFO, "Name OBJ:    " + (fileNameDstObj.empty()  ? "- none -" : fileNameDstObj));

	//  find collision node (bhkCompressedMeshData)
	for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)
	{
		if (DynamicCast<bhkCompressedMeshShapeData>(*pIter) != NULL)
		{
			getGeometryFromCompressedMeshShape(DynamicCast<bhkCompressedMeshShapeData>(*pIter), chunkDataList);
		}
	}  //  for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)

	//  check extracted chunks
	if (chunkDataList.empty())
	{
		logMessage(NCU_MSG_TYPE_INFO, "No chunks found");
	}
	else  //  if (chunkDataList.empty())
	{
		//  save OBJ file?
		if (!fileNameDstObj.empty())
		{
			logMessage(NCU_MSG_TYPE_INFO, "Exporting OBJ to " + fileNameDstObj);
			if (writeChunkDataAsObj(fileNameDstObj, chunkDataList))
			{
				logMessage(NCU_MSG_TYPE_INFO, "Done successfully");
			}
			else
			{
				logMessage(NCU_MSG_TYPE_ERROR, "Error while exporting to " + fileNameDstObj);
			}
		}  //  if (!fileNameDstObj.empty())

		//  save NIF file?
		if (!fileNameDstNif.empty())
		{
			logMessage(NCU_MSG_TYPE_INFO, "Exporting NIF to " + fileNameDstNif);
			if (writeChunkDataAsNif(fileNameDstNif, chunkDataList))
			{
				logMessage(NCU_MSG_TYPE_INFO, "Done successfully");
			}
			else
			{
				logMessage(NCU_MSG_TYPE_ERROR, "Error while exporting to " + fileNameDstObj);
			}
		}  //  if (!fileNameDstNif.empty())
	}  //  else [if (chunkDataList.empty())]

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
bool NifCollisionUtility::writeChunkDataAsObj(string fileName, vector<NifChunkData>& chunkDataList)
{
	ofstream		oStr   (fileName.c_str(), ios::out | ios::trunc);
	unsigned int	offFace(1);
	bool			retVal (false);

	if (oStr.is_open())
	{
		//  write header
		oStr << "# exported with NifUtilsSuite::ChunkExtract" << endl;
		oStr.precision(14);

		//  for each chunk
		for (auto pIterC=chunkDataList.begin(), pEndC=chunkDataList.end(); pIterC != pEndC; ++pIterC)
		{
			//  write chunk header
			oStr << endl << "# Chunk: " << pIterC->_index << endl;
			oStr << "# Material: " << pIterC->_material << ", " << _materialList.getMaterialDefName(pIterC->_material) << ", " << _materialList.getMaterialName(pIterC->_material) << endl;

			//  write vertices
			for (auto pIterV=pIterC->_vertices.begin(), pEndV=pIterC->_vertices.end(); pIterV != pEndV; ++pIterV)
			{
				oStr << fixed << "v " << pIterV->x << " " << pIterV->y << " " << pIterV->z << endl;
			}

			//  write normals if existing
			if (!pIterC->_normals.empty())
			{
				for (auto pIterN=pIterC->_vertices.begin(), pEndN=pIterC->_vertices.end(); pIterN != pEndN; ++pIterN)
				{
					oStr << fixed << "vn " << pIterN->x << " " << pIterN->y << " " << pIterN->z << endl;
				}
			}

			//  write faces
			for (auto pIterT=pIterC->_triangles.begin(), pEndT=pIterC->_triangles.end(); pIterT != pEndT; ++pIterT)
			{
				oStr << "f " << pIterT->v1+offFace << "/" << pIterT->v1+offFace << "/" << pIterT->v1+offFace << " "
					         << pIterT->v2+offFace << "/" << pIterT->v2+offFace << "/" << pIterT->v2+offFace << " "
					         << pIterT->v3+offFace << "/" << pIterT->v3+offFace << "/" << pIterT->v3+offFace << " "
							 << endl;
			}

			//  increment offset
			offFace += pIterC->_vertices.size();

		}  //  for (auto pIterC=chunkDataList.begin(), pEndC=chunkDataList.end(); pIterC != pEndC; ++pIterC)

		//  flush and close file
		oStr.flush();
		oStr.close();

		//  mark success
		retVal = true;

	}  //  if (oStr.is_open())

	return retVal;
}

/*---------------------------------------------------------------------------*/
bool NifCollisionUtility::writeChunkDataAsNif(string fileName, vector<NifChunkData>& chunkDataList)
{
	NiNodeRef	pRootNode(new NiNode);

	//  for each chunk
	for (auto pIterC=chunkDataList.rbegin(), pEndC=chunkDataList.rend(); pIterC != pEndC; ++pIterC)
	{
		//  create NiTriShape
		NiTriShapeRef	pShape(new NiTriShape);

		//  set name
		if (_cmHandling == NCU_CM_MATERIAL)
		{
			pShape->SetName(_materialList.getMaterialDefName(pIterC->_material));
		}
		else
		{
			stringstream	sStream;

			sStream << "Chunk " << pIterC->_index;
			pShape->SetName(sStream.str());
		}

		//  set flags
		pShape->SetFlags(14);		//  fix value

		//  create NiTriShapeData
		NiTriShapeDataRef	pData(new NiTriShapeData);

		//  add vertices
		pData->SetVertices(pIterC->_vertices);

		//  add faces
		pData->SetTriangles(pIterC->_triangles);

		//  add normals
		pData->SetNormals(pIterC->_normals);

		//  set flags
		pData->SetConsistencyFlags(CT_STATIC);

		//  add data to shape
		pShape->SetData(pData);

		//  add shape to node
		pRootNode->AddChild(DynamicCast<NiAVObject>(pShape));

	}  //  for (auto pIterC=chunkDataList.begin(), pEndC=chunkDataList.end(); pIterC != pEndC; ++pIterC)

	//  write nif to file using Skyrim version
	WriteNifTree((const char*) fileName.c_str(), pRootNode, NifInfo(VER_20_2_0_7, 12, 83));

	return true;
}
