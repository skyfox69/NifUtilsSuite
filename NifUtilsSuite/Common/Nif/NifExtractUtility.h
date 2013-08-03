/**
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
	/**
		* Default Constructor
		*/
	NifExtractUtility(NifUtlMaterialList& materialList);

	/**
		* Destructor
		*/
	virtual ~NifExtractUtility();

	/**
		* set Skyrim path
		* 
		* @param pathSkyrim    in: path to Skyrim base directory
		* of textures
		*/
	virtual void setSkyrimPath(string pathSkyrim);

	/**
		* 
		* @param cmHandling    in: handling of naming chunks
		*/
	virtual void setChunkNameHandling(ChunkNameHandling cmHandling);

	/**
		* Get list of user messages
		*/
	virtual vector<string>& getUserMessages();

	/**
		* Get list of used textures
		*/
	virtual set<string>& getUsedTextures();

	/**
		* Set callback function for logging info
		*/
	virtual void setLogCallback(void (*logCallback) (const int type, const char* pMessage));


	virtual unsigned int extractChunks(string fileNameCollSrc, string fileNameDstNif, string fileNameDstObj);

	virtual void setGenerateNormals(const bool genNormals);

	virtual void setScaleToModel(const bool doScale);

	virtual void setSaveAsVersion(const unsigned int version);

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
		* reference to material list (injected)
		*/
	NifUtlMaterialList& _materialList;

	ChunkNameHandling _cmHandling;

	bool _generateNormals;

	bool _scaleToModel;

	bool _mergeCollision;

	unsigned int _saveAsVersion;

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
};
