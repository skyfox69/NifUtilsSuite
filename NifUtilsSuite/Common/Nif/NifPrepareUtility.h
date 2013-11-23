/*!
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
	/*!
	 * Default Constructor
	 */
	NifPrepareUtility();

	/*!
	 * Destructor
	 */
	virtual ~NifPrepareUtility();

	/*!
	 * Prepare armor-NIF for import into Blender
	 *
	 * \param[in] fileNameSrc  file name of armor NIF
	 * \param[in] fileNameDst  file name of destination NIF
	 */
	virtual unsigned int prepareArmorBlender(string fileNameSrc, string fileNameDst);

	/*!
	 * Prepare NIF exproted from Blender as armor-NIF for Skyrim
	 *
	 * \param[in] fileNameSrc  file name of exported NIF
	 * \param[in] fileNameDst  file name of destination armor-NIF
	 * \param[in] fileNameTmpl  file name of NIF used as template for armor-NIF
	 */
	virtual unsigned int prepareBlenderArmor(string fileNameSrc, string fileNameDst, string fileNameTmpl);

	/*!
	 *  Set removement of BSInvMarker
	 */
	virtual void setRemoveBSInvMarker (const bool doRemove);

	/*!
	 *  Set removement of BSProperties
	 */
	virtual void setRemoveBSProperties(const bool doRemove);

	/*!
	 *  Set body part translation map
	 *
	 * \param[in] bodyPartMap  translation map of body parts
	 */
	virtual void setBodyPartMap(map<unsigned short, unsigned short> bodyPartMap);

	/*!
	 * Get list of user messages
	 */
	virtual vector<string>& getUserMessages();

	/*!
	 * Set callback function for logging info
	 */
	virtual void setLogCallback(void (*logCallback) (const int type, const char* pMessage));

protected:

	/*!	ptr. to logging callback function */
	void (*_logCallback) (const int, const char*);

	/*! body part translation map */
	map<unsigned short, unsigned short> _bodyPartMap;

	/*! log messages for user */
	vector<string> _userMessages;

	/*! remove BSInvMarker */
	bool _remBSInvMarker;

	/*! remove BSProperties */
	bool _remBSProperties;

	/*!
	 * Get NiNode from NIF-file
	 * 
	 * \param[in] fileName  path and name of NIF file
	 * \param[in] logPreText  text prepended to log output
	 * \param[in] fakedRoot  flag marking real root node or faked one
	 */
	virtual NiNodeRef getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot);

	/*!
	 * Parse NIF tree and transform for usage in Blender
	 *
	 * \param[in] pNode  root node to parse from
	 */
	virtual NiNodeRef parse4Blender(NiNodeRef pNode);

	/*!
	 * Parse NIF tree and transform into armor-NIF
	 *
	 * \param[in] pNode  root node to parse from
	 * \param[in] pShaderTmpl  template for use with BSLightingShaderProperty
	 */
	virtual NiNodeRef parse4Armor(NiNodeRef pNode, BSLightingShaderPropertyRef pShaderTmpl);

	/*!
	 * Log messages
	 * 
	 * \param[in] type  message type
	 * \param[in] text  message text
	 */
	virtual void logMessage(int type, string text);
};
