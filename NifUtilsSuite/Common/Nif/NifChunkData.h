/**
 *  file:   NifChunkData.h
 *  struct: NifChunkData
 *
 *  data structure describing chunk data
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include <vector>

#include "nif_math.h"

//-----  DEFINES  -------------------------------------------------------------
using namespace std;
using namespace Niflib;

struct NifChunkData
{
	vector<Vector3>		_vertices;
	vector<Vector3>		_normals;
	vector<Triangle>	_triangles;
	unsigned int		_material;
	unsigned short		_index;
};
