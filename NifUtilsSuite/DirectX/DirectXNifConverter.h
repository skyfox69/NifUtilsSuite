/**
 *  file:   DirectXNifConverter.h
 *  class:  DirectXNifConverter
 *
 *  Class converting a NIF model into DirectX readable one
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <string>
#include <vector>

//  DirectX includes
#include <d3dx9.h>

//  NifUtils includes
#include "obj/bsfadenode.h"
#include "obj/nitrishape.h"
#include "obj/NiAlphaProperty.h"
#include "obj/bhkCollisionObject.h"
#include "obj/bhkCompressedMeshShape.h"
#include "obj/bhkPackedNiTriStripsShape.h"
#include "gen/QuaternionXYZW.h"

//-----  DEFINES  -------------------------------------------------------------
using namespace std;
using namespace Niflib;

class DirectXMesh;
struct DirectXAlphaState;

//-----  CLASS  ---------------------------------------------------------------
class DirectXNifConverter
{
	protected:
		vector<string>				_texturePathList;
		DWORD						_defWireframeColor;
		DWORD						_defCollisionColor;
		DWORD						_defAmbientColor;
		DWORD						_defDiffuseColor;
		DWORD						_defSpecularColor;
		unsigned short				_lodRenderLevel;
		bool						_isBillboard;
		bool						_isCollision;
		bool						_forceDDS;
		bool						_showModel;
		bool						_showCollision;
		bool						_doubleSided;

		virtual	NiNodeRef			getRootNodeFromNifFile (string fileName);
		virtual	unsigned int		getGeometryFromNode    (NiNodeRef pNode, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		//  model
		virtual	unsigned int		getGeometryFromTriShape(NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		virtual	unsigned int		getGeometryFromTriStrips(NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		virtual	unsigned int		getGeometryFromData(vector<Vector3>& vecVertices, vector<Triangle>& vecTriangles, vector<Vector3>& vecNormals, vector<Color4>& vecColors, vector<TexCoord>& vecTexCoords, NiTriBasedGeomRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		//  collision
		virtual	unsigned int		getGeometryFromCollisionObject(bhkCollisionObjectRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		virtual	unsigned int		getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeRef pShape, NiObjectRef pBody, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);
		virtual	unsigned int		getGeometryFromPackedTriStripsShape(bhkPackedNiTriStripsShapeRef pShape, NiObjectRef pBody, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp);

		virtual	void				BlendFuncToDXBlend     (const NiAlphaProperty::BlendFunc value, DWORD& dxBlend, DWORD& dxArg);
		virtual D3DXMATRIX			Matrix44ToD3DXMATRIX   (const Matrix44& matrixIn);
		virtual	Matrix33			QuaternionToMatrix33   (const QuaternionXYZW& quadIn);
		virtual	string				CheckTextureName       (string texName);
		virtual	DirectXAlphaState*	DecodeAlphaProperty    (NiAlphaProperty* pProperty);

	public:
									DirectXNifConverter();
		virtual						~DirectXNifConverter();

		virtual	bool				ConvertModel(const string fileName, vector<DirectXMesh*>& meshList);

		virtual	void				SetTexturePathList      (vector<string> texturePathList);
		virtual	bool				SetForceDDS             (const bool forceDDS);
		virtual	bool				SetShowModel            (const bool doShow);
		virtual	bool				SetShowCollision        (const bool doShow);
		virtual	bool				SetDoubleSided          (const bool doubleSided);
		virtual	DWORD				SetDefaultWireframeColor(const DWORD color);
		virtual	DWORD				SetDefaultCollisionColor(const DWORD color);
		virtual	DWORD				SetDefaultAmbientColor  (const DWORD color);
		virtual	DWORD				SetDefaultDiffuseColor  (const DWORD color);
		virtual	DWORD				SetDefaultSpecularColor (const DWORD color);
		virtual	unsigned int		SetDefaultLODRenderLevel(const unsigned int level);
};
