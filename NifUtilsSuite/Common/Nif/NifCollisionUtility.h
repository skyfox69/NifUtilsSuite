/*!
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
#include "obj/bhkPackedNiTriStripsShape.h"

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
#define   NCU_ERROR_WRONG_NIF_VERSION       0x07

//  used namespaces
using namespace Niflib;
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class NifCollisionUtility
{
protected:
	struct NCUTriangle
	{
		vector<int>			_neightbours;
		bool				_windingOk;

							NCUTriangle() : _windingOk(false) {};
		virtual				~NCUTriangle() {};
	};

	struct NCUBorder
	{
		vector<int>			_neightbours;
		int					_index;

							NCUBorder(int index, vector<int> neightbours);
		virtual				~NCUBorder();
	};

public:
	/*!
	 * Default Constructor
	 */
	NifCollisionUtility(NifUtlMaterialList& materialList);

	/*!
	 * Destructor
	 */
	virtual ~NifCollisionUtility();

	/*!
	 * add collision nodes to shape
	 * 
	 * \param[in] fileNameCollSrc  filename of collision source. NIF/OBJ ending
	 * \param[in] fileNameNifDst  filename of NIF, collision info should be added to
	 * \param[in] fileNameCollTmpl  path and name of Nif file used as template
	 */
	virtual unsigned int addCollision(string fileNameCollSrc, string fileNameNifDst, string fileNameCollTmpl);

	/*!
	 * set Skyrim path
	 * 
	 * \param[in] pathSkyrim  path to Skyrim base directory
	 * of textures
	 */
	virtual void setSkyrimPath(string pathSkyrim);

	/*!
	 * set handling of collision nodes
	 *
	 * \param[in] cnHandling  handling of collision node strategy
	 */
	virtual void setCollisionNodeHandling(CollisionNodeHandling cnHandling);

	/*!
	 * set handling of chunk names
	 *
	 * \param[in] cmHandling  handling of naming chunks
	 */
	virtual void setChunkNameHandling(ChunkNameHandling cmHandling);

	/*!
	 * set handling of material types and material map
	 *
	 * \param[in] mtHandling  handling of material type
	 * \param[in] mtMap  map of material types: key=NodeID, value=MatType
	 */
	virtual void setMaterialTypeHandling(MaterialTypeHandling mtHandling, map<int, unsigned int>& mtMapping);

	/*!
	 * set default material for all nodes
	 *
	 * \param[in] defaultMaterial  default material if none given
	 */
	virtual void setDefaultMaterial(unsigned int defaultMaterial);

	/*!
	 * Get list of user messages
	 */
	virtual vector<string>& getUserMessages();

	/*!
	 * Get list of used textures
	 */
	virtual set<string>& getUsedTextures();

	/*!
	 * Get list of non existing textures
	 */
	virtual set<string>& getNewTextures();

	/*!
	 * Set callback function for logging info
	 */
	virtual void setLogCallback(void (*logCallback) (const int type, const char* pMessage));

	/*!
	 * Set generation of one or individual collision nodes
	 */
	virtual void setMergeCollision(const bool doMerge);

	/*!
	 * Set if triangles should be reordered matching outward
	 */
	virtual void setReorderTriangles(const bool doReorder);

protected:

	/*!	ptr. to logging callback function */
	void (*_logCallback) (const int, const char*);

	/*! mapping of material per NiTriShape; key: nif node number, value: material id */
	map<int, unsigned int> _mtMapping;

	/*! log messages for user */
	vector<string> _userMessages;

	/*! list of used textures */
	set<string> _usedTextures;

	/*! list of non existing textures */
	set<string> _newTextures;

	/*! path to Skyrim files */
	string _pathSkyrim;

	/*! reference to material list (injected) */
	NifUtlMaterialList& _materialList;

	/*! handling of material type */
	MaterialTypeHandling _mtHandling;

	/*! handling of collision nodes */
	CollisionNodeHandling _cnHandling;

	/*! handling of chunk names */
	ChunkNameHandling _cmHandling;

	/*! default material */
	unsigned int _defaultMaterial;

	/*! NIF version of actual loaded destination NIF (internal) */
	unsigned _nifVersion;

	/*! generate one global collision or re-use multiple ones on sub trees */
	bool _mergeCollision;

	/*! reorder triangles to face outwards (bhkPackedNiTriStrips only */
	bool _reorderTriangles;


	/*!
	 * Get geometry from NiTriShape
	 * 
	 * \param[in] pShape  ptr. to NiTriShape
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations are inserted in
	 */
	virtual unsigned int getGeometryFromTriShape(NiTriShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from NiTriStrips
	 * 
	 * \param[in] pShape  ptr. to NiTriStrips
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations are inserted in
	 */
	virtual unsigned int getGeometryFromTriStrips(NiTriStripsRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from NiTriBasedGeom
	 * 
	 * \param[in] vertices  reference to vertices
	 * \param[in] triangles  reference to triangles
	 * \param[in] pShape  ptr. to NiTriBasedGeom
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations
	 */
	virtual unsigned int getGeometryFromShapeData(vector<Vector3>& vertices, vector<Triangle>& triangles, NiTriBasedGeomRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from bhkShape
	 * 
	 * \param[in] pShape  ptr. to bhkShape
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations are inserted in
	 */
	virtual bhkShapeRef getGeometryFromCollShape(bhkShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from bhkPackedNiTriStripsShape
	 * 
	 * \param[in] pShape  ptr. to bhkPackedNiTriStripsShape
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations are inserted in
	 */
	virtual unsigned int getGeometryFromPackedNiTriStrips(bhkPackedNiTriStripsShapeRef pShape, vector<hkGeometry>& geometryMap, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from NiNode
	 * 
	 * \param[in] pNode  ptr. to root NiNode
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 * \param[in] transformAry  reference to transformations
	 */
	virtual unsigned int getGeometryFromNode(NiNodeRef pNode, vector<hkGeometry>& geometryMap, vector<hkGeometry>& geometryMapColl, vector<Matrix44>& transformAry);

	/*!
	 * Get geometry from OBJ-file
	 * 
	 * \param[in] fileName  path and name of OBJ file
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 */
	virtual unsigned int getGeometryFromObjFile(string fileName, vector<hkGeometry>& geometryMap);

	/*!
	 * Get geometry from NIF-file
	 * 
	 * \param[in] fileName  path and name of NIF file
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 */
	virtual unsigned int getGeometryFromNifFile(string fileName, vector<hkGeometry>& geometryMap);

	/*!
	 * Get NiNode from NIF-file
	 * 
	 * \param[in] fileName  path and name of NIF file
	 * \param[in] logPreText  text prepended to log output
	 * \param[out] fakedRoot  flag marking real root node or faked one
	 * \param[in] setVersion  flag remembering version of NIF in internal attribute
	 */
	virtual NiNodeRef getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot, bool setVersion);

	/*!
	  * Check wether NIF has collision nodes or not
	  * 
	  * \param[in] fileNameCollSrc  filename of collision source. NIF/OBJ ending
	  */
	virtual bool collSourceHasCollNodes(string fileNameCollSrc);

	/*!
	 * Create bhkCollisionObject from template and geometry
	 * 
	 * \param[in] geometryAry  reference to vector of hkGeometry
	 * \param[in] tmplNode  Template bhkCollisionObject
	 * \param[in] rootNode  Root node of NIF tree
	 */
	virtual bhkCollisionObjectRef createCollNode(vector<hkGeometry>& geometryMap, bhkCollisionObjectRef pTmplNode, NiNodeRef pRootNode);

	/*!
	 * Create HAVOK specific collision data and inject into model
	 * 
	 * \param[in] geometryAry  reference to vector of hkGeometry
	 * \param[in] pMoppShape  MoppBvTreeShape to inject MoppCode into
	 * \param[in] pData  CompressedMeshShapeData getting chunks and tris
	 * \param[in] pRigidBody  bhkRigidBody for layer
	 */
	virtual bool injectCollisionData(vector<hkGeometry>& geometryMap, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData, bhkRigidBodyRef pRigidBody);

	/*!
	 * Log messages
	 * 
	 * \param[in] type  message type
	 * \param[in] text  message text
	 */
	virtual void logMessage(int type, string text);

	/*!
	 * Remove all collision nodes from NiNode
	 * 
	 * \param[in] pNode  ptr. to root NiNode
	 */
	virtual void cleanTreeCollision(NiNodeRef pNode);

	/*!
	 * Get geometry from NiNode
	 * 
	 * \param[in] pNode  ptr. to root NiNode
	 * \param[in] fileNameCollTmpl  path and name of Nif file used as template
	 * \param[out] geometryAry  reference to vector of hkGeometry read geometries are inserted in
	 */
	virtual bool parseTreeCollision(NiNodeRef pNode, string fileNameCollTmpl, vector<hkGeometry>& geometryMapColl);

	/*!
	 * Parse NIF tree for collision nodes
	 * 
	 * \param[in] fileNameCollTmpl  path and name of Nif file used as template
	 * \param[in] pShape  root node of tree/branch
	 * \param[in] pRigidBody  ptr. to bhkRigidBody in case of parsing sub-tree of bhkRigidBody
	 * \param[in] pMoppShape  ptr. to bhkMoppBvTreeShape in case of parsing sub-tree of bhkMoppBvTreeShape
	 */
	virtual bhkShapeRef parseCollisionShape(string fileNameCollTmpl, bhkShapeRef pShape, bhkRigidBodyRef pRigidBody, bhkMoppBvTreeShapeRef pMoppShape=NULL);

	/*!
	 * Convert bhkPackedNiTriStripsShape to bhkCompressedMeshShape and modify MoppData
	 * 
	 * \param[in] pShape  ptr. to bhkPackedNiTriStripsShape to convert
	 * \param[in] pMoppShape  ptr. to bhkMoppBvTreeShape in case of parsing sub-tree of bhkMoppBvTreeShape
	 * \param[in] pRigidBody  ptr. to bhkRigidBody in case of parsing sub-tree of bhkRigidBody
	 * \param[in] fileNameCollTmpl  path and name of Nif file used as template
	 */
	virtual bhkShapeRef convertCollPackedNiTriStrips(bhkPackedNiTriStripsShapeRef pShape, bhkMoppBvTreeShapeRef pMoppShape, bhkRigidBodyRef pRigidBody, string fileNameCollTmpl);

	/*!
	 * Reorder winding of triangles to face outward
	 * 
	 * \param[in/out] srcAry  reference to vector of triangles (hkGeometry::Triangle)
	 * \param[in] vertAry  reference to vector of vertices (hkVector4)
	 */
	virtual void reorderTriangles(hkArray<hkGeometry::Triangle>& srcAry, hkArray<hkVector4>& vertAry);

	/*!
	 * Reorder winding of triangles using adjacent triangles defined by borders
	 * 
	 * \param[in/out] srcAry  reference to vector of triangles (hkGeometry::Triangle)
	 * \param[in/out] triangleList  reference to internal triangle definition list
	 * \param[in/out] borderList  reference to internal border definition list
	 * \param[in/out] border  reference to border being processed
	 */
	virtual void reorderTriangleBorders(hkArray<hkGeometry::Triangle>& srcAry, vector<NCUTriangle>& triangleList, vector<NCUBorder>& borderList, NCUBorder& border);

	/*!
	 * Try getting winding of single triangle
	 * 
	 * \param[in/out] triIndex  index of triangle to get winding for
	 * \param[in/out] srcAry  reference to vector of triangles (hkGeometry::Triangle)
	 * \param[in] vertAry  reference to vector of vertices (hkVector4)
	 */
	virtual bool getTriangleWinding(const int triIndex, hkArray<hkGeometry::Triangle>& srcAry, hkArray<hkVector4>& vertAry);
};
