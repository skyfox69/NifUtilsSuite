/**
 *  file:   NifPrepareUtility.h
 *  class:  NifPrepareUtility
 *
 *  Utilities preparing NIF files to get importable into Blender
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
//  common includes

//  NifUtiliy includes

//  Niflib includes
#include "obj/NiNode.h"
#include "obj/BSLightingShaderProperty.h"

//-----  DEFINES  -------------------------------------------------------------
//  return codes
#define   NCU_OK                            0x00
#define   NCU_ERROR_MISSING_FILE_NAME       0x01
#define   NCU_ERROR_MISSING_TEXTURE_NAME    0x02
#define   NCU_ERROR_CANT_OPEN_INPUT         0x03
#define   NCU_ERROR_CANT_OPEN_TEMPLATE      0x04
#define   NCU_ERROR_CANT_OPEN_OUTPUT        0x05

//  used namespaces
using namespace Niflib;
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class NifPrepareUtility
{

public:
	/**
		* Default Constructor
		*/
	NifPrepareUtility();

	/**
		* Destructor
		*/
	virtual ~NifPrepareUtility();

	virtual unsigned int prepareArmorBlender(string fileNameSrc, string fileNameDst);
	virtual unsigned int prepareBlenderArmor(string fileNameSrc, string fileNameDst, string fileNameTmpl);



	virtual void setRemoveBSInvMarker (const bool doRemove);
	virtual void setRemoveBSProperties(const bool doRemove);
	virtual void setBodyPartMap       (map<unsigned short, unsigned short> bodyPartMap);

	/**
		* Get list of user messages
		*/
	virtual vector<string>& getUserMessages();

	/**
		* Set callback function for logging info
		*/
	virtual void setLogCallback(void (*logCallback) (const int type, const char* pMessage));

protected:

	void (*_logCallback) (const int, const char*);

	bool _remBSInvMarker;
	bool _remBSProperties;

	/**
		* log messages for user
		*/
	vector<string> _userMessages;

	map<unsigned short, unsigned short> _bodyPartMap;


	/**
		* Get NiNode from NIF-file
		* 
		* @param fileName    in: path and name of NIF file
		* @param logPreText    in: text prepended to log output
		* @param fakedRoot    out: flag marking real root node or faked one
		*/
	virtual NiNodeRef getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot);


	virtual NiNodeRef parse4Blender(NiNodeRef pNode);
	virtual NiNodeRef parse4Armor  (NiNodeRef pNode, BSLightingShaderPropertyRef pShaderTmpl);

	/**
		* Log messages
		* 
		* @param type    in: message type
		* @param text    in: message text
		*/
	virtual void logMessage(int type, string text);
};
