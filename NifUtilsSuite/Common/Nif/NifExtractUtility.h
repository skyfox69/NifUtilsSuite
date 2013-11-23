/*!
 *  file:   NifExtractUtility.h
 *  class:  NifExtractUtility
 *
 *  Utilities extracting collision data from NIF files
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes
#include <set>

//  NifUtiliy includes
#include "Common\Nif\ChunkNameHandling.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifChunkData.h"

//  Niflib includes
#include "obj/bhkCompressedMeshShapeData.h"

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
class NifExtractUtility
{
public:
	/*!
	 * Default Constructor
	 */
	NifExtractUtility(NifUtlMaterialList& materialList);

	/*!
	 * Destructor
	 */
	virtual ~NifExtractUtility();

	/*!
	 * extract chunks from bhkCompressedMeshShapeData to file(s)
	 * 
	 * \param[in] fileNameCollSrc  filename of collision source. NIF ending
	 * \param[in] fileNameDstNif  filename of NIF, collision should be extracted to
	 * \param[in] fileNameDstObj  filename of NIF, collision should be written to
	 */
	virtual unsigned int extractChunks(string fileNameCollSrc, string fileNameDstNif, string fileNameDstObj);

	/*!
	 * set Skyrim path
	 * 
	 * \param[in] pathSkyrim  path to Skyrim base directory of textures
	*/
	virtual void setSkyrimPath(string pathSkyrim);

	/*!
	 * 
	 * \param[in] cmHandling  handling of naming chunks
	 */
	virtual void setChunkNameHandling(ChunkNameHandling cmHandling);

	/*!
	 * Set if face normals should be generated while exporting to OBJ
	 */
	virtual void setGenerateNormals(const bool genNormals);

	/*!
	 * Set if collision data should be resized to model size
	 */
	virtual void setScaleToModel(const bool doScale);

	/*!
	 * Set the version destination NIF should be saved to
	 */
	virtual void setSaveAsVersion(const unsigned int version);

	/*!
	 * Get list of user messages
	 */
	virtual vector<string>& getUserMessages();

	/*!
	 * Get list of used textures
	 */
	virtual set<string>& getUsedTextures();

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

	/*! path to Skyrim files */
	string _pathSkyrim;

	/*! reference to material list (injected) */
	NifUtlMaterialList& _materialList;

	/*! version NIF should be saved as */
	unsigned int _saveAsVersion;

	/*! handling of chunk names */
	ChunkNameHandling _cmHandling;

	/*! true if normals should be generated */
	bool _generateNormals;

	/*! true if collision data should be resized to model size */
	bool _scaleToModel;


	/*!
	* Log messages
	* 
	* \param[in] type  message type
	* \param[in] text  message text
	*/
	virtual void logMessage(int type, string text);

	/*!
	 * Extract geometry from bhkCompressedMeshShapeData
	 *
	 * \param[in] pShape  ptr. to bhkCompressedMeshShapeData chunks should be extracted from
	 * \param[out] chunkDataList  list of chunk geometries
	 */
	virtual unsigned int getGeometryFromCompressedMeshShape(bhkCompressedMeshShapeDataRef pShape, vector<NifChunkData>& chunkDataList);

	/*!
	 * Write chunk list of geometries into OBJ file
	 *
	 * \param[in] fileName  file name of destination file
	 * \param[in] chunkDataList  list of chunk geometries
	 */
	virtual bool writeChunkDataAsObj(string fileName, vector<NifChunkData>& chunkDataList);

	/*!
	 * Write chunk list of geometries into NIF file
	 *
	 * \param[in] fileName  file name of destination file
	 * \param[in] chunkDataList  list of chunk geometries
	 */
	virtual bool writeChunkDataAsNif(string fileName, vector<NifChunkData>& chunkDataList);
};
