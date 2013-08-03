/**
 *  file:   NifExtractUtility.cpp
 *  class:  NifCollisioUtility
 *
 *  Utilities extracting collision data from NIF files
 */

//-----  INCLUDES  ------------------------------------------------------------
//  Common includes
#include <fstream>

#include "Common\Nif\NifExtractUtility.h"
#include "Common\Util\DefLogMessageTypes.h"

//  Niflib includes
#include "niflib.h"
#include "obj/NiNode.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriShapeData.h"

/*---------------------------------------------------------------------------*/
NifExtractUtility::NifExtractUtility(NifUtlMaterialList& materialList)
	:	_logCallback       (NULL),
		_materialList      (materialList),
		_generateNormals   (true),
		_scaleToModel      (true),
		_mergeCollision    (true)
{}

/*---------------------------------------------------------------------------*/
NifExtractUtility::~NifExtractUtility()
{}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setSkyrimPath(string pathSkyrim)
{
	_pathSkyrim = pathSkyrim;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setChunkNameHandling(ChunkNameHandling cmHandling)
{
	_cmHandling = cmHandling;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setGenerateNormals(const bool genNormals)
{
	_generateNormals = genNormals;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setScaleToModel(const bool doScale)
{
	_scaleToModel = doScale;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setSaveAsVersion(const unsigned int version)
{
	_saveAsVersion = version;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifExtractUtility::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
set<string>& NifExtractUtility::getUsedTextures()
{
	return _usedTextures;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::setLogCallback(void (*logCallback) (const int type, const char* pMessage))
{
	_logCallback = logCallback;
}

/*---------------------------------------------------------------------------*/
void NifExtractUtility::logMessage(int type, string text)
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
unsigned int NifExtractUtility::getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeDataRef pShape, vector<NifChunkData>& chunkDataList)
{
	vector<bhkCMSDChunk>		chunkGeoList(pShape->GetChunks());
	vector<bhkCMSDMaterial>		chunkMatList(pShape->GetChunkMaterials());
	float						scaleFactor (_scaleToModel ? 70.0f : 1.0f);
	unsigned short				idxChunk    (0);

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
			tmpCData._vertices.push_back(Vector3(((pIter->vertices[idx  ] / 1000.0f) + pIter->translation.x) * scaleFactor,
				                                 ((pIter->vertices[idx+1] / 1000.0f) + pIter->translation.y) * scaleFactor,
											     ((pIter->vertices[idx+2] / 1000.0f) + pIter->translation.z) * scaleFactor
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
			pTmpCData->_vertices.push_back(Vector3((tBVecVec[pIter->triangle1].x) * scaleFactor,
												   (tBVecVec[pIter->triangle1].y) * scaleFactor,
												   (tBVecVec[pIter->triangle1].z) * scaleFactor
												  ));

			short	idxP2(pTmpCData->_vertices.size());
			pTmpCData->_vertices.push_back(Vector3((tBVecVec[pIter->triangle2].x) * scaleFactor,
												   (tBVecVec[pIter->triangle2].y) * scaleFactor,
												   (tBVecVec[pIter->triangle2].z) * scaleFactor
												  ));

			short	idxP3(pTmpCData->_vertices.size());
			pTmpCData->_vertices.push_back(Vector3((tBVecVec[pIter->triangle3].x) * scaleFactor,
												   (tBVecVec[pIter->triangle3].y) * scaleFactor,
												   (tBVecVec[pIter->triangle3].z) * scaleFactor
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
unsigned int NifExtractUtility::extractChunks(string fileNameCollSrc, string fileNameDstNif, string fileNameDstObj)
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
bool NifExtractUtility::writeChunkDataAsObj(string fileName, vector<NifChunkData>& chunkDataList)
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
bool NifExtractUtility::writeChunkDataAsNif(string fileName, vector<NifChunkData>& chunkDataList)
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
	WriteNifTree((const char*) fileName.c_str(), pRootNode, NifInfo(VER_20_2_0_7, ((_saveAsVersion >> 16) & 0x0000FFFF), (_saveAsVersion & 0x0000FFFF)));

	return true;
}

