/**
 *  file:   NifPrepareUtility.cpp
 *  class:  NifPrepareUtility
 *
 *  Utilities preparing NIF files to get importable into Blender
 */

//-----  INCLUDES  ------------------------------------------------------------
//  Common includes
#include "Common\Nif\NifPrepareUtility.h"
#include "Common\Util\DefLogMessageTypes.h"

//  Niflib includes
#include "niflib.h"
#include "obj/NiTriShape.h"
#include "obj/BSInvMarker.h"
#include "obj/NiExtraData.h"
#include "obj/BSShaderPPLightingProperty.h"
#include "obj/BSDismemberSkinInstance.h"
#include "obj/BSShaderTextureSet.h"

//-----  DEFINES  -------------------------------------------------------------
//  used namespaces
using namespace Niflib;

/*---------------------------------------------------------------------------*/
NifPrepareUtility::NifPrepareUtility()
	:	_logCallback    (NULL),
		_remBSInvMarker (true),
		_remBSProperties(true)
{}

/*---------------------------------------------------------------------------*/
NifPrepareUtility::~NifPrepareUtility()
{}

/*---------------------------------------------------------------------------*/
NiNodeRef NifPrepareUtility::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot)
{
	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str());

	//  NiNode as root
	if (DynamicCast<NiNode>(pRootTree) != NULL)
	{
		pRootInput = DynamicCast<NiNode>(pRootTree);
		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiNode.");
	}
	//  NiTriShape as root
	else if (DynamicCast<NiTriShape>(pRootTree) != NULL)
	{
		//  create faked root
		pRootInput = new NiNode();

		//  add root as child
		pRootInput->AddChild(DynamicCast<NiAVObject>(pRootTree));

		//  mark faked root node
		fakedRoot = true;

		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiTriShape.");
	}

	//  no known root type found
	if (pRootInput == NULL)
	{
		logMessage(NCU_MSG_TYPE_WARNING, logPreText + " root has unhandled type: " + pRootTree->GetType().GetTypeName());
	}

	return pRootInput;
}


/*---------------------------------------------------------------------------*/
NiNodeRef NifPrepareUtility::parse4Blender(NiNodeRef pNode)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());
	list<NiExtraDataRef>	extraList(pNode->GetExtraData());

	//  parse extra data for BSInvMarker
	for (auto pIter(extraList.begin()), pEnd(extraList.end()); pIter != pEnd; pIter++)
	{
		if (_remBSInvMarker && (DynamicCast<BSInvMarker>(*pIter) != NULL))
		{
			pNode->RemoveExtraData(*pIter);
		}
	}

	//  unlink children
	pNode->ClearChildren();

	//  iterate over children
	for (auto pIter(childList.begin()), pEnd(childList.end()); pIter != pEnd; pIter++)
	{
		//  NiTriShape => remove BSLightingShaderProperty
		if (DynamicCast<NiTriShape>(*pIter) != NULL)
		{
			if (_remBSProperties)
			{
				NiTriShapeRef	pShape (DynamicCast<NiTriShape>(*pIter));

				//  remove properties (Bethesda uses max. 2)
				pShape->SetBSProperty(0, NULL);
				pShape->SetBSProperty(1, NULL);
			}

			//  add shape to node
			pNode->AddChild(*pIter);
		}
		//  BSInvMarker => remove whole object
		else if (_remBSInvMarker && (DynamicCast<BSInvMarker>(*pIter) != NULL))
		{
			//  skip entry => do not add to final list
		}
		//  NiNode (and derived classes?) => iterate subnodes
		else if (DynamicCast<NiNode>(*pIter) != NULL)
		{
			pNode->AddChild(&(*parse4Blender(DynamicCast<NiNode>(*pIter))));
		}
	}  //  for (auto pIter(childList.begin()), pEnd(childList.end()); pIter != pEnd; pIter++)

	return pNode;
}

/*---------------------------------------------------------------------------*/
unsigned int NifPrepareUtility::prepareArmorBlender(string fileNameSrc, string fileNameDst)
{
	NiNodeRef	pRootInput (NULL);
	NiNodeRef	pRootOutput(NULL);
	bool		fakedRoot  (false);

	//  test on existing file names
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "Source:  "      + (fileNameSrc.empty() ? "- none -" : fileNameSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  " + (fileNameDst.empty() ? "- none -" : fileNameDst));

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameSrc + "' as input");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  parse tree
	pRootOutput = parse4Blender(pRootInput);

	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, NifInfo(VER_20_2_0_7, 11, 34));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
NiNodeRef NifPrepareUtility::parse4Armor(NiNodeRef pNode, BSLightingShaderPropertyRef pShaderTmpl)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());

	//  unlink children
	pNode->ClearChildren();

	//  iterate over children
	for (auto pIter(childList.begin()), pEnd(childList.end()); pIter != pEnd; pIter++)
	{
		//  NiTriShape => remodel BSLightingShaderProperty
		if (DynamicCast<NiTriShape>(*pIter) != NULL)
		{
			NiTriShapeRef				pShape  (DynamicCast<NiTriShape>(*pIter));
			BSDismemberSkinInstanceRef	pSkin   (DynamicCast<BSDismemberSkinInstance>(pShape->GetSkinInstance()));
			vector<NiPropertyRef>		propList(pShape->GetProperties());

			//  part of skin data? => modify skin code
			if (pSkin != NULL)
			{
				vector<BodyPartList>	bPartList(pSkin->GetPartitions());

				for (auto pIter(bPartList.begin()), pEnd(bPartList.end()); pIter != pEnd; pIter++)
				{
					if (_bodyPartMap.count(pIter->bodyPart) > 0)
					{
						pIter->bodyPart = (BSDismemberBodyPartType) _bodyPartMap[pIter->bodyPart];
					}
				}  //  for (auto pIter(bPartList.begin()), pEnd(bPartList.end()); pIter != pEnd; pIter++)

				//  set modified parts
				pSkin->SetPartitions(bPartList);

			}  //  if (pSkin != NULL)

			//  remove all properties
			pShape->ClearProperties();

			//  create new BSLightingShaderProperty if template given
			if (pShaderTmpl != NULL)
			{
				//  check properties
				for (auto pIterProp(propList.begin()), pEnd(propList.end()); pIterProp != pEnd; pIterProp++)
				{
					//  convert BSShaderPPLightingProperty to BSLightingShaderProperty
					if (DynamicCast<BSShaderPPLightingProperty>(*pIterProp) != NULL)
					{
						BSShaderPPLightingProperty*	pPProp(DynamicCast<BSShaderPPLightingProperty>(*pIterProp));
						BSLightingShaderProperty*	pLProp(new BSLightingShaderProperty());

						//  move texture set
						pLProp->SetTextureSet(pPProp->GetTextureSet());
						pPProp->SetTextureSet(NULL);

						pLProp->SetShaderFlags1       (pShaderTmpl->GetShaderFlags1());
						pLProp->SetShaderFlags2       (pShaderTmpl->GetShaderFlags2());
						pLProp->SetEmissiveMultiple   (pShaderTmpl->GetEmissiveMultiple());
						pLProp->SetEmissiveColor      (pShaderTmpl->GetEmissiveColor());
						pLProp->SetLightingEffect1    (pShaderTmpl->GetLightingEffect1());
						pLProp->SetLightingEffect2    (pShaderTmpl->GetLightingEffect2());
						pLProp->SetEnvironmentMapScale(pShaderTmpl->GetEnvironmentMapScale());
						pLProp->SetSkyrimShaderType   (pShaderTmpl->GetSkyrimShaderType());
						pLProp->SetSpecularColor      (pShaderTmpl->GetSpecularColor());
						pLProp->SetSpecularStrength   (pShaderTmpl->GetSpecularStrength());
						pLProp->SetTextureClampMode   (pShaderTmpl->GetTextureClampMode());
						pLProp->SetUnknownFloat2      (pShaderTmpl->GetUnknownFloat2());
						pLProp->SetGlossiness         (pShaderTmpl->GetGlossiness());

						//  add property to shape
						pShape->SetBSProperty(0, pLProp);
					}
				}  //  for (auto pIterProp(propList.begin()), pEnd(propList.end()); pIterProp != pEnd; pIterProp++)
			}  //  if (pShaderTmpl != NULL)

			//  add shape to node
			pNode->AddChild(*pIter);
		}
		//  NiNode (and derived classes?) => iterate subnodes
		else if (DynamicCast<NiNode>(*pIter) != NULL)
		{
			pNode->AddChild(&(*parse4Armor(DynamicCast<NiNode>(*pIter), pShaderTmpl)));
		}
	}  //  for (auto pIter(childList.begin()), pEnd(childList.end()); pIter != pEnd; pIter++)

	return pNode;
}

/*---------------------------------------------------------------------------*/
unsigned int NifPrepareUtility::prepareBlenderArmor(string fileNameSrc, string fileNameDst, string fileNameTmpl)
{
	NiNodeRef						pRootInput (NULL);
	NiNodeRef						pRootOutput(NULL);
	BSLightingShaderPropertyRef		pShaderTempl(NULL);
	vector<NiObjectRef>				blockList;
	bool							fakedRoot  (false);

	//  test on existing file names
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "Source:  "      + (fileNameSrc.empty()  ? "- none -" : fileNameSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  " + (fileNameDst.empty()  ? "- none -" : fileNameDst));
	logMessage(NCU_MSG_TYPE_INFO, "Template:  "    + (fileNameTmpl.empty() ? "- none -" : fileNameTmpl));

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameSrc + "' as input");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get first BSLightingShaderProperty from template
	blockList = ReadNifList(fileNameTmpl);
	for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)
	{
		if (DynamicCast<BSLightingShaderProperty>(*pIter) != NULL)
		{
			pShaderTempl = DynamicCast<BSLightingShaderProperty>(*pIter);
			break;
		}
	}  //  for (auto pIter=blockList.begin(), pEnd=blockList.end(); pIter != pEnd; ++pIter)

	//  parse tree
	pRootOutput = parse4Armor(pRootInput, pShaderTempl);

	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
void NifPrepareUtility::setRemoveBSInvMarker(const bool doRemove)
{
	_remBSInvMarker = doRemove;
}

/*---------------------------------------------------------------------------*/
void NifPrepareUtility::setRemoveBSProperties(const bool doRemove)
{
	_remBSProperties = doRemove;
}

/*---------------------------------------------------------------------------*/
void NifPrepareUtility::setBodyPartMap(map<unsigned short, unsigned short> bodyPartMap)
{
	_bodyPartMap = bodyPartMap;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifPrepareUtility::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
void NifPrepareUtility::setLogCallback(void (*logCallback) (const int type, const char* pMessage))
{
	_logCallback = logCallback;
}

/*---------------------------------------------------------------------------*/
void NifPrepareUtility::logMessage(int type, string text)
{
	//  add message to internal storages
	switch (type)
	{
		default:
		case NCU_MSG_TYPE_INFO:
		case NCU_MSG_TYPE_WARNING:
		case NCU_MSG_TYPE_ERROR:
		{
			_userMessages.push_back(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE:
		{
//			_usedTextures.insert(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE_MISS:
		{
//			_newTextures.insert(text);
			break;
		}
	}

	//  send message to callback if given
	if (_logCallback != NULL)
	{
		(*_logCallback)(type, text.c_str());
	}
}
