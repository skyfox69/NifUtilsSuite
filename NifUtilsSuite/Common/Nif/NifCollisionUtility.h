/**
 *  file:   NifCollisionUtility.h
 *  class:  NifCollisioUtility
 *
 *  Utilities adding collision data to NIF files
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <set>

//  NifUtiliy includes
#include "Common\Nif\CollisionNodeHandling.h"
#include "Common\Nif\MaterialTypeHandling.h"
#include "Common\Nif\ChunkNameHandling.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifChunkData.h"

//  Niflib includes
#include "obj/nitrishape.h"
#include "obj/bhkcollisionobject.h"
#include "obj/bsfadenode.h"
#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkCompressedMeshShapeData.h"
#include "obj/bhkRigidBody.h"
#include "obj/NiTriStrips.h"

//  Havok includes
#include "Common/Base/Types/Geometry/hkGeometry.h"

//-----  DEFINES  -------------------------------------------------------------
//  return codes
#define   NCU_OK                            0x00
#define   NCU_ERROR_MISSING_FILE_NAME       0x01
#define   NCU_ERROR_MISSING_TEXTURE_NAME    0x02
#define   NCU_ERROR_CANT_OPEN_INPUT         0x03
#define   NCU_ERROR_CANT_OPEN_TEMPLATE      0x04
#define   NCU_ERROR_CANT_OPEN_OUTPUT        0x05
#define   NCU_ERROR_CANT_GET_GEOMETRY       0x06

//  used namespaces
using namespace Niflib;
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class NifCollisionUtility
{
public:
	/**
		* Default Constructor
		*/
	NifCollisionUtility(NifUtlMaterialList& materialList);

	/**
		* Destructor
		*/
	virtual ~NifCollisionUtility();

	/**
		* add collision nodes to shape
		* 
		* @param fileNameCollSrc    in: filename of collision source. NIF/OBJ ending
		* @param fileNameNifDst    in: filename of NIF, collision info should be added to
		* @param fileNameCollTmpl    in: path and name of Nif file used as template
		*/
	virtual unsigned int addCollision(string fileNameCollSrc, string fileNameNifDst, string fileNameCollTmpl);

	/**
		* set Skyrim path
		* 
		* @param pathSkyrim    in: path to Skyrim base directory
		* of textures
		*/
	virtual void setSkyrimPath(string pathSkyrim);

	/**
		* 
		* @param cnHandling    in: handling of collision node strategy
		*/
	virtual void setCollisionNodeHandling(CollisionNodeHandling cnHandling);

	/**
		* 
		* @param cmHandling    in: handling of naming chunks
		*/
	virtual void setChunkNameHandling(ChunkNameHandling cmHandling);

	/**
		* 
		* @param mtHandling    in: handling of material type
		* @param mtMap         in: map of material types: key=NodeID, value=MatType
		*/
	virtual void setMaterialTypeHandling(MaterialTypeHandling mtHandling, map<int, unsigned int>& mtMapping);

	/**
	* @param defaultMaterial   in: default material if none given
	*/
	virtual void setDefaultMaterial(unsigned int defaultMaterial);

	/**
		* Get list of user messages
		*/
	virtual vector<string>& getUserMessages();

	/**
		* Get list of used textures
		*/
	virtual set<string>& getUsedTextures();

	/**
		* Get list of non existing textures
		*/
	virtual set<string>& getNewTextures();

	/**
		* Set callback function for logging info
		*/
	virtual void setLogCallback(void (*logCallback) (const int type, const char* pMessage));


	virtual unsigned int extractChunks(string fileNameCollSrc, string fileNameDstNif, string fileNameDstObj);

	virtual void setGenerateNormals(const bool genNormals);

	virtual void setScaleToModel(const bool doScale);

	virtual void setSaveAsVersion(const unsigned int version);

	virtual void setMergeCollision(const bool doMerge);

protected:

	void (*_logCallback) (const int, const char*);

	/**
		* path to Skyrim files
		*/
	string _pathSkyrim;

	/**
		* log messages for user
		*/
	vector<string> _userMessages;

	/**
		* list of used textures
		*/
	set<string> _usedTextures;

	/**
		* list of non existing textures
		*/
	set<string> _newTextures;

	/**
		* handling of collision nodes
		*/
	CollisionNodeHandling _cnHandling;

	/**
		* mapping of material per NiTriShape; key: nif node number, value: material id
		*/
	map<int, unsigned int> _mtMapping;

	/**
		* handling of material type
		*/
	MaterialTypeHandling _mtHandling;

	/**
		* default material
		*/
	unsigned int _defaultMaterial;

	/**
		* reference to material list (injected)
		*/
	NifUtlMaterialList& _materialList;

	ChunkNameHandling _cmHandling;

	bool _generateNormals;

	bool _scaleToModel;

	bool _mergeCollision;

	bool _replaceCollision;

	unsigned int _saveAsVersion;

	/**
	* Get geometry from NiTriShape
	* 
	* @param pShape    in: ptr. to NiTriShape
	* @param geometryAry    out: reference to vector of hkGeometry read geometries
	* are inserted in
	*/
	virtual unsigned int getGeometryFromTriShape(NiTriShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/**
	* Get geometry from NiTriStrips
	* 
	* @param pShape    in: ptr. to NiTriStrips
	* @param geometryAry    out: reference to vector of hkGeometry read geometries
	* are inserted in
	*/
	virtual unsigned int getGeometryFromTriStrips(NiTriStripsRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	virtual unsigned int getGeometryFromShapeData(vector<Vector3>& vertices, vector<Triangle>& triangles, float factor, NiTriBasedGeomRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	virtual unsigned int getGeometryFromCollObject(bhkCollisionObjectRef pModel, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/**
	* Get geometry from NiNode
	* 
	* @param pNode    in: ptr. to root NiNode
	* @param geometryAry    out: reference to vector of hkGeometry read geometries
	* are inserted in
	*/
	virtual unsigned int getGeometryFromNode(NiNodeRef pNode, vector<hkGeometry>& geometryMap, vector<hkGeometry>& geometryMapColl, vector<Matrix44>& transformAry);

	/**
	* Get geometry from OBJ-file
	* 
	* @param fileName    in: path and name of OBJ file
	* @param geometryAry    out: reference to vector of hkGeometry read geometries
	* are inserted in
	*/
	virtual unsigned int getGeometryFromObjFile(string fileName, vector<hkGeometry>& geometryMap);

	/**
	* Get geometry from NIF-file
	* 
	* @param fileName    in: path and name of NIF file
	* @param geometryAry    out: reference to vector of hkGeometry read geometries
	* are inserted in
	*/
	virtual unsigned int getGeometryFromNifFile(string fileName, vector<hkGeometry>& geometryMap);

	/**
	* Get NiNode from NIF-file
	* 
	* @param fileName    in: path and name of NIF file
	* @param logPreText    in: text prepended to log output
	* @param fakedRoot    out: flag marking real root node or faked one
	*/
	virtual NiNodeRef getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot);

	/**
	* Create bhkCollisionObject from template and geometry
	* 
	* @param geometryAry    in: reference to vector of hkGeometry
	* @param tmplNode    in: Template bhkCollisionObject
	* @param rootNode    in: Root node of NIF tree
	*/
	virtual bhkCollisionObjectRef createCollNode(vector<hkGeometry>& geometryMap, bhkCollisionObjectRef pTmplNode, NiNodeRef pRootNode);

	/**
	* Create HAVOK specific collision data and inject into model
	* 
	* @param geometryAry    in: reference to vector of hkGeometry
	* @param pMoppShape    in: MoppBvTreeShape to inject MoppCode into
	* @param pData    in: CompressedMeshShapeData getting chunks and tris
	* @param pRigidBody    in: bhkRigidBody for layer
	*/
	virtual bool injectCollisionData(vector<hkGeometry>& geometryMap, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData, bhkRigidBodyRef pRigidBody);

	/**
	* Log messages
	* 
	* @param type    in: message type
	* @param text    in: message text
	*/
	virtual void logMessage(int type, string text);

	virtual unsigned int getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeDataRef pShape, vector<NifChunkData>& chunkDataList);

	virtual bool writeChunkDataAsObj(string fileName, vector<NifChunkData>& chunkDataList);

	virtual bool writeChunkDataAsNif(string fileName, vector<NifChunkData>& chunkDataList);

	virtual void cleanTreeCollision(NiNodeRef pNode);
};
