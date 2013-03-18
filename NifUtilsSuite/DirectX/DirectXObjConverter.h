/**
 *  file:   DirectXObjConverter.h
 *  class:  DirectXObjConverter
 *
 *  Class converting a OBJ model into DirectX readable one
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <string>
#include <vector>

//  DirectX includes
#include <d3dx9.h>

//  Niflib includes
#include "nif_math.h"

//-----  DEFINES  -------------------------------------------------------------
using namespace std;
using namespace Niflib;

class DirectXMesh;

//-----  CLASS  ---------------------------------------------------------------
class DirectXObjConverter
{
	protected:
		vector<string>			_texturePathList;
		DWORD					_defWireframeColor;
		bool					_showModel;

		virtual	unsigned int	getGeometryFromObjFile(string fileName, vector<DirectXMesh*>& meshList);
		virtual	DirectXMesh*	createMesh(vector<Vector3>& vecVertices, vector<Triangle>& vecTriangles, vector<Vector3>& vecNormals, short& objIdx);

	public:
								DirectXObjConverter();
		virtual					~DirectXObjConverter();

		virtual	bool			ConvertModel(const string fileName, vector<DirectXMesh*>& meshList);

		virtual	bool			SetShowModel            (const bool doShow);
		virtual	DWORD			SetDefaultWireframeColor(const DWORD color);
};
