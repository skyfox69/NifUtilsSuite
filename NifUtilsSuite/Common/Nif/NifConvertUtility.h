/*!
 *  file:   NifConvertUtility.h
 *  class:  NifConvertUtility
 *
 *  Utilities converting NIF files from old to new format
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <set>

//  NifUtiliy includes
#include "Common\Nif\VertexColorHandling.h"

//  Niflib includes
#include "obj/nitrishape.h"
#include "obj/rootcollisionnode.h"
#include "obj/bsfadenode.h"
#include "obj/nialphaproperty.h"
#include "obj/NiTriShapeData.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/NiTriStrips.h"
#include "obj/bhkShape.h"

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
class NifConvertUtility
{
public:
	/*!
	 * Default Constructor
	 */
	NifConvertUtility();

	/*!
	 * Destructor
	 */
	virtual ~NifConvertUtility();

	/*!
	 * Convert shape part of NIF to new format
	 * 
	 * \param[in] fileNameSrc  filename of NIF to be converted
	 * \param[in] fileNameDst  filename of NIF to be produced
	 * \param[in] fileNameTmpl  path and name of Nif-file used as template
	 */
	virtual unsigned int convertShape(string fileNameSrc, string fileNameDst, string fileNameTmpl);

	/*!
	 * set texture path used for re-locate textures
	 * 
	 * \param[in] pathTexture  path to texture base directory used for new location of textures
	 */
	virtual void setTexturePath(string pathTexture);

	/*!
	 * set path to Skyrim executeable
	 * 
	 * \param[in] pathSkyrim  path to Skyrim base directory of textures
	 */
	virtual void setSkyrimPath(string pathSkyrim);

	/*!
	 * Set handling of missing vertex color
	 *
	 * \param[in] vcHandling  handling of vertex colors when missing in source NIF
	 */
	virtual void setVertexColorHandling(VertexColorHandling vcHandling);

	/*!
	 * Set default vertex color
	 *
	 * \param[in] defaultColor  RGB color
	 */
	virtual void setDefaultVertexColor(Color4 defaultColor);

	/*!
	 * Set if normals and bi-normals should be (re-)generated
	 * 
	 * \param[in] doUpdate  true: update tangent space
	 */
	virtual void setUpdateTangentSpace(bool doUpdate);

	/*!
	 * Set if properties of NiTriShapes should be brought into 'right' order
	 *
	 * \param[in] doReorder  true: reorder NiTriShape properties
	 */
	virtual void setReorderProperties(bool doReorder);

	/*!
	 * Set if texture filenames should get .DDS ending
	 *
	 * \param[in] doForce  true: force texture names having DDS ending
	 */
	virtual void setForceDDS(bool doForce);

	/*!
	 * Set if all collision data should be removed from NIF
	 *
	 * \param[in] doClean  true: remove all collision nodes before saving
	 */
	virtual void setCleanTreeCollision(bool doClean);

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

protected:

	/*!	ptr. to logging callback function */
	void (*_logCallback) (const int, const char*);

	/*! log messages for user */
	vector<string> _userMessages;

	/*! list of used textures */
	set<string> _usedTextures;

	/*! list of non existing textures */
	set<string> _newTextures;

	/*! path to texture files */
	string _pathTexture;

	/*! path to Skyrim files */
	string _pathSkyrim;

	/*! default vertex color */
	Color4 _vcDefaultColor;

	/*! handling of vertex colors */
	VertexColorHandling _vcHandling;

	/*! update tangent space */
	bool _updateTangentSpace;

	/*! reorder NiTriShape properties */
	bool _reorderProperties;

	/*! force texture names having DDS ending */
	bool _forceDDS;

	/*! remove all collision data form tree */
	bool _cleanTreeCollision;


	/*!
	 * Get NiNode from NIF-file
	 * 
	 * \param[in] fileName  path and name of NIF file
	 * \param[in] logPreText  text prepended to log output
	 * \param[in] fakedRoot  flag marking real root node or faked one
	 */
	virtual NiNodeRef getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot, NifInfo* pNifInfo=NULL);

	/*!
	 * Convert NiNode and all known sub-nodes
	 * 
	 * \param[in] srcNode  root node of (sub-)tree to convert
	 * \param[in] tmplNode  template node for converion
	 * \param[in] rootNode  Root node of destination NIF tree
	 * \param[in] tmplAlphaProp  template of AlphaProperties when found
	 */
	virtual NiNodeRef convertNiNode(NiNodeRef pSrcNode, NiTriShapeRef pTmplNode, NiNodeRef pRootNode, NiAlphaPropertyRef pTmplAlphaProp = NULL);

	/*!
	 * Convert NiTriShape and properties/geometry
	 * 
	 * \param[in] srcNode  Source NiTriShape node
	 * \param[in] tmplNode  Template NiTriShape node
	 * \param[in] tmplAlphaProp  template of AlphaProperties when found
	 */
	virtual NiTriShapeRef convertNiTriShape(NiTriShapeRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp = NULL);

	/*!
	 * Convert NiTriStrips and properties/geometry
	 * 
	 * \param[in] srcNode  Source NiTriStrips node
	 * \param[in] tmplNode  Template NiTriShape node
	 * \param[in] tmplAlphaProp  template of AlphaProperties when found
	 */
	virtual NiTriShapeRef convertNiTriStrips(NiTriStripsRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp = NULL);

	/*!
	 * Base convert function for NiTriShapes and properties/geometry
	 * 
	 * \param[in] srcNode  Source NiTriStrips node
	 * \param[in] tmplNode  Template NiTriShape node
	 * \param[in] tmplAlphaProp  template of AlphaProperties when found
	 */
	virtual NiTriShapeRef convertNiTri(NiTriShapeRef pDstNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp);

	/*!
	 * Create tangent space data
	 * 
	 * \param[in] pDataObj  data object
	 */
	virtual bool updateTangentSpace(NiTriShapeDataRef pDataObj);

	/*!
	 * Clone BSLightingShaderProperty
	 * 
	 * \param[in] pSource  ptr. to source object
	 */
	virtual BSLightingShaderPropertyRef cloneBSLightingShaderProperty(BSLightingShaderPropertyRef pSource);

	/*!
	 * Log messages
	 * 
	 * \param[in] type  message type
	 * \param[in] text  message text
	 */
	virtual void logMessage(int type, string text);

	/*!
	 * Check of existence of file
	 * 
	 * \param[in] fileName  path and name of file to check
	 */
	virtual bool checkFileExists(string fileName);

	/*!
	 * Parse NIF tree for collision data
	 * 
	 * \param[in] pShape  ptr. to root node of (sub-)tree to parse
	 */
	virtual void parseCollisionTree(bhkShapeRef pShape);
};
