/**
 *  file:   DirectXObjConverter.cpp
 *  class:  DirectXObjConverter
 *
 *  Class converting a OBJ model into DirectX readable one
 */

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <sstream>
#include <fstream>

//  NifUtils includes
#include "DirectX\DirectXObjConverter.h"
#include "DirectX\DirectXMeshModel.h"

//-----  DEFINES  -------------------------------------------------------------


//-----  DirectXNifConverter()  -----------------------------------------------
DirectXObjConverter::DirectXObjConverter()
	:	_defWireframeColor(0x00FFFFFF),
		_showModel        (true)
{}

//-----  ~DirectXNifConverter()  ----------------------------------------------
DirectXObjConverter::~DirectXObjConverter()
{}

//-----  SetShowModel()  ------------------------------------------------------
bool DirectXObjConverter::SetShowModel(const bool doShow)
{
	bool	oldShow(_showModel);

	_showModel = doShow;
	return oldShow;
}

//-----  SetDefaultWireframeColor()  ------------------------------------------
DWORD DirectXObjConverter::SetDefaultWireframeColor(const DWORD color)
{
	DWORD	oldColor(_defWireframeColor);

	_defWireframeColor = color;
	return oldColor;
}

/*---------------------------------------------------------------------------*/
unsigned int DirectXObjConverter::getGeometryFromObjFile(string fileName, vector<DirectXMesh*>& meshList)
{
	char					cBuffer[1000] = {0};
	char*					pChar  (NULL);
	ifstream				inFile;
	vector<Vector3>			vecVertices;
	vector<Triangle>		vecTriangles;
	vector<Vector3>			vecNormals;
	Vector3					tVector;
	Triangle				tTriangle;
	unsigned int			faceOffset(1);
	short					idx(0);
	short					objIdx (1);
	bool					hasFace(false);

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
			//  append model to list
			meshList.push_back(createMesh(vecVertices, vecTriangles, vecNormals, objIdx));

			//  set new offset for face vertices
			faceOffset += vecVertices.size();

			//  reset existing face flag
			hasFace = false;

			//  reset lists
			vecVertices.clear();
			vecTriangles.clear();
			vecNormals.clear();
		}

		//  vertex?
		if (_strnicmp(cBuffer, "v ", 2) == 0)
		{
			//  get vector from line
			sscanf(cBuffer, "v %f %f %f", &(tVector.x), &(tVector.y), &(tVector.z));

			//  scale final vertex
			tVector *= 0.0143f;

			//  add vertex to array
			vecVertices.push_back(tVector);
		}
		//  vertex nrmals?
		if (_strnicmp(cBuffer, "vn ", 2) == 0)
		{
			//  get vector from line
			sscanf(cBuffer, "vn %f %f %f", &(tVector.x), &(tVector.y), &(tVector.z));

			//  scale final vertex
			tVector *= 0.0143f;

			//  add vertex to array
			vecNormals.push_back(tVector);
		}
		//  face?
		else if (_strnicmp(cBuffer, "f ", 2) == 0)
		{
			//  get triangle idx from line
			for (idx=0, pChar=strchr(cBuffer, ' '); ((pChar != NULL) && (idx < 3)); ++idx, pChar = strchr(pChar, ' '))
			{
				tTriangle[idx] = atoi(++pChar) - faceOffset;
			}

			//  add to array
			vecTriangles.push_back(tTriangle);

			//  mark existing face
			hasFace = true;
		}
	}  //  while (inFile.good())

	//  existing last/only face? => create new geometry
	if (hasFace)
	{
		//  add geometry to result array
		meshList.push_back(createMesh(vecVertices, vecTriangles, vecNormals, objIdx));

	}  //  if (hasFace)

	//  close file
	inFile.close();

	return meshList.size();
}

//-----  ConvertModel()  ------------------------------------------------------
DirectXMesh* DirectXObjConverter::createMesh(vector<Vector3>& vecVertices, vector<Triangle>& vecTriangles, vector<Vector3>& vecNormals, short& objIdx)
{
	D3DMATERIAL9	material = { 0 };
	D3DXMATRIX		transform;
	string			name;

	//  - normals
	if (vecNormals.size() != vecVertices.size())
	{
		unsigned int	size(vecVertices.size());

		//  initialize normal vector
		vecNormals.resize(size);

		//  for each face
		for (auto pIter=vecTriangles.begin(), pEnd=vecTriangles.end(); pIter != pEnd; ++pIter)
		{
			Vector3		faceNormal((vecVertices[pIter->v2] - vecVertices[pIter->v1]).CrossProduct((vecVertices[pIter->v3] - vecVertices[pIter->v1])));

			//  add values to normals
			vecNormals[pIter->v1] += faceNormal;
			vecNormals[pIter->v2] += faceNormal;
			vecNormals[pIter->v3] += faceNormal;
		}

		//  normalize normals
		for (unsigned int idx(0); idx < size; ++idx)
		{
			vecNormals[idx] = vecNormals[idx].Normalized();
		}
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
		pBufVertices[i]._color    = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pBufVertices[i]._u        = 0.0f;
		pBufVertices[i]._v        = 0.0f;
	}

	//  - material
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	//  create new model
	pNewModel = new DirectXMeshModel(transform,
										material,
										pBufVertices,
										countV,
										pBufIndices,
										countI,
										"",
										NULL,
										false,
										_defWireframeColor,
										true
									);

	//  set visibility
	if (!_showModel)	pNewModel->SetRenderMode(DXRM_NONE);

	//  own block for local variables
	{
		stringstream	sStream;

		sStream << "Block-ID: " << objIdx;
		name = sStream.str();
	}

	//  add model view data
	pNewModel->SetNifData(name, "OBJ-Definition", objIdx);

	return pNewModel;
}

//-----  ConvertModel()  ------------------------------------------------------
bool DirectXObjConverter::ConvertModel(const string fileName, vector<DirectXMesh*>& meshList)
{
#if 0
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

	//  RootCollisionNode
	if (DynamicCast<bhkCollisionObject>(pRootInput->GetCollisionObject()) != NULL)
	{
		//  set collision flag
		_isCollision = true;

		//  recurse sub-tree
		getGeometryFromCollisionObject(DynamicCast<bhkCollisionObject>(pRootInput->GetCollisionObject()), meshList, transformAry, NULL);

		//  reset collision flag
		_isCollision = false;
	}
#endif
	return true;
}

