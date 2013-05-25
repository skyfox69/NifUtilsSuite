/**
 *  file:   NifConvertUtility.cpp
 *  class:  NifConvertUtility
 *
 *  Utilities converting NIF files from old to new format
 */

//-----  INCLUDES  ------------------------------------------------------------
//  Common includes
#include "Common\Nif\NifConvertUtility.h"
#include "Common\Util\DefLogMessageTypes.h"
#include <algorithm>
#include <string>

//  Niflib includes
#include "niflib.h"
#include "obj/NiExtraData.h"
#include "obj/NiTimeController.h"
#include "obj/NiTexturingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiVertexColorProperty.h"
#include "obj/NiTriStripsData.h"
#include "obj/bhkPackedNiTriStripsShape.h"
#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkRigidBody.h"
#include "obj/bhkCollisionObject.h"
#include "obj/hkPackedNiTriStripsData.h"
#include "obj/bhkNiTriStripsShape.h"
#include "obj/bhkConvexTransformShape.h"
#include "obj/bhkListShape.h"

//-----  DEFINES  -------------------------------------------------------------
//  used namespaces
using namespace Niflib;
using namespace std;

/*---------------------------------------------------------------------------*/
NifConvertUtility::NifConvertUtility()
	:	_vcDefaultColor    (1.0f, 1.0f, 1.0f, 1.0f),
		_vcHandling        (NCU_VC_REMOVE_FLAG),
		_updateTangentSpace(true),
		_reorderProperties (true),
		_forceDDS          (true),
		_cleanTreeCollision(true),
		_logCallback       (NULL)
{}

/*---------------------------------------------------------------------------*/
NifConvertUtility::~NifConvertUtility()
{}

/*---------------------------------------------------------------------------*/
NiNodeRef NifConvertUtility::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot)
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
NiNodeRef NifConvertUtility::convertNiNode(NiNodeRef pSrcNode, NiTriShapeRef pTmplNode, NiNodeRef pRootNode, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiNodeRef				pDstNode    (pSrcNode);
	bhkCollisionObjectRef	pCollObject (DynamicCast<bhkCollisionObject>(pSrcNode->GetCollisionObject()));
	vector<NiAVObjectRef>	srcShapeList(pDstNode->GetChildren());

	//  add local transformation to list
	transformAry.push_back(pDstNode->GetLocalTransform());

	//  find NiAlphaProperty and use as template in sub-nodes
	if (DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
	{
		pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE));
	}

	//  unlink protperties -> not used in new format
	pDstNode->ClearProperties();
	pDstNode->SetCollisionObject(NULL);

	//  shift extra data to new version
	pDstNode->ShiftExtraData(VER_20_2_0_7);

	//  unlink children
	pDstNode->ClearChildren();

	//  iterate over source nodes and convert using template
	for (auto  ppIter=srcShapeList.begin(), pEnd=srcShapeList.end(); ppIter != pEnd; ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*ppIter), pTmplNode, pTmplAlphaProp)));
		}
		//  NiTriStrips
		else if (DynamicCast<NiTriStrips>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiTriStrips(DynamicCast<NiTriStrips>(*ppIter), pTmplNode, pTmplAlphaProp)));
		}
		//  RootCollisionNode
		else if ((DynamicCast<RootCollisionNode>(*ppIter) != NULL) && _cleanTreeCollision)
		{
			//  ignore node
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*ppIter), pTmplNode, pRootNode, transformAry, pTmplAlphaProp)));
		}
	}

	//  add collision object in case of holding
	if (!_cleanTreeCollision && (pCollObject != NULL))
	{
		//  search for embedded shape
		bhkRigidBodyRef		pRBody(DynamicCast<bhkRigidBody>(pCollObject->GetBody()));

		if (pRBody != NULL)
		{
			pRBody->SetShape(convertCollShape(pRBody->GetShape()));
			pDstNode->SetCollisionObject(pCollObject);
		}
	}  //  if (!_cleanTreeCollision && (pCollObject != NULL))

	//  remove local transformation from array
	transformAry.pop_back();

	return pDstNode;
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTriShape(NiTriShapeRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	//  NiTriShape is moved from src to dest. It's unlinked in calling function
	NiTriShapeRef	pDstNode(pSrcNode);

	//  force some data in destination shape
	pDstNode->SetCollisionObject(NULL);  //  no collision object here
	pDstNode->ClearExtraData();
	pDstNode->SetFlags          (14);    //  ???

	//  return converted NiTriShape
	return convertNiTri(pDstNode, pTmplNode, pTmplAlphaProp);
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTriStrips(NiTriStripsRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiTriShapeRef			pDstNode   (new NiTriShape());
	NiTriShapeDataRef		pDstGeo    (new NiTriShapeData());
	NiTriStripsDataRef		pSrcGeo    (DynamicCast<NiTriStripsData>(pSrcNode->GetData()));
	vector<NiPropertyRef>	srcPropList(pSrcNode->GetProperties());

	//  copy NiTriStrips to NiTriShape
	pDstNode->SetCollisionObject(NULL);  //  no collision object here
	pDstNode->SetFlags          (14);    //  ???
	pDstNode->SetName           (pSrcNode->GetName());
	pDstNode->SetLocalTransform (pSrcNode->GetLocalTransform());
	pDstNode->SetData           (pDstGeo);

	//  move properties
	for (auto pIter=srcPropList.begin(), pEnd=srcPropList.end(); pIter != pEnd; ++pIter)
	{
		if (DynamicCast<NiVertexColorProperty>(*pIter) != NULL)
		{
			int	iii=0;
		}
		pDstNode->AddProperty(*pIter);
	}
	pSrcNode->ClearProperties();

	//  data node
	if (pSrcGeo != NULL)
	{
		pDstGeo->SetVertices    (pSrcGeo->GetVertices());
		pDstGeo->SetNormals     (pSrcGeo->GetNormals());
		pDstGeo->SetTriangles   (pSrcGeo->GetTriangles());
		pDstGeo->SetVertexColors(pSrcGeo->GetColors());
		pDstGeo->SetUVSetCount  (pSrcGeo->GetUVSetCount());
		for (short idx(0), max(pSrcGeo->GetUVSetCount()); idx < max; ++idx)
		{
			pDstGeo->SetUVSet(idx, pSrcGeo->GetUVSet(idx));
		}
	}  //  if (pSrcGeo != NULL)

	//  return converted NiTriShape
	return convertNiTri(pDstNode, pTmplNode, pTmplAlphaProp);
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTri(NiTriShapeRef pDstNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	BSLightingShaderPropertyRef	pTmplLShader(NULL);
	BSLightingShaderPropertyRef	pDstLShader (NULL);
	NiGeometryDataRef			pDstGeo     (pDstNode->GetData());
	vector<NiPropertyRef>		dstPropList (pDstNode->GetProperties());
	short						bsPropIdx   (0);
	bool						forceAlpha  (pTmplAlphaProp != NULL);
	bool						hasAlpha    (false);

	//  data node
	if (pDstGeo != NULL)
	{
		//  set flags
		if (pTmplNode->GetData() != NULL)
		{
			pDstGeo->SetConsistencyFlags(pTmplNode->GetData()->GetConsistencyFlags());  //  nessessary ???
		}

		//  update tangent space?
		if ((_updateTangentSpace) && (DynamicCast<NiTriShapeData>(pDstGeo) != NULL))
		{
			//  update tangent space
			if (updateTangentSpace(DynamicCast<NiTriShapeData>(pDstGeo)))
			{
				//  enable tangent space
				pDstGeo->SetTspaceFlag(0x10);
			}
		}  //  if (_updateTangentSpace)
	}  //  if (pDstGeo != NULL)

	//  properties - get them from template
	for (short index(0); index < 2; ++index)
	{
		//  BSLightingShaderProperty
		if (DynamicCast<BSLightingShaderProperty>(pTmplNode->GetBSProperty(index)) != NULL)
		{
			pTmplLShader = DynamicCast<BSLightingShaderProperty>(pTmplNode->GetBSProperty(index));
		}
		//  NiAlphaProperty
		else if (DynamicCast<NiAlphaProperty>(pTmplNode->GetBSProperty(index)) != NULL)
		{
			pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pTmplNode->GetBSProperty(index));
		}
	}  //  for (short index(0); index < 2; ++index)

	//  parse properties of destination node
	dstPropList = pDstNode->GetProperties();
	pDstNode->ClearProperties();

	for (auto ppIter=dstPropList.begin(), pEnd=dstPropList.end(); ppIter != pEnd; ppIter++)
	{
		//  NiAlphaProperty
		if (DynamicCast<NiAlphaProperty>(*ppIter) != NULL)
		{
			NiAlphaPropertyRef	pPropAlpha(DynamicCast<NiAlphaProperty>(*ppIter));

			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);

			//  set new property to node
			pDstNode->SetBSProperty(bsPropIdx++, &(*pPropAlpha));

			//  own alpha, reset forced alpha
			forceAlpha = false;

			//  mark alpha property
			hasAlpha = true;
		}
		//  NiTexturingProperty
		else if (DynamicCast<NiTexturingProperty>(*ppIter) != NULL)
		{
			char*						pTextPos (NULL);
			BSShaderTextureSetRef		pDstSText(new BSShaderTextureSet());
			TexDesc						baseTex  ((DynamicCast<NiTexturingProperty>(*ppIter))->GetTexture(BASE_MAP));
			string						texture  (baseTex.source->GetTextureFileName());
			string::size_type			result   (string::npos);

			//  clone shader property from template
			pDstLShader = cloneBSLightingShaderProperty(pTmplLShader);

			//  copy textures from template to copy
			pDstSText->SetTextures(pTmplLShader->GetTextureSet()->GetTextures());

			//  separate filename from path
			result = texture.rfind('\\');
			if (result == string::npos)			result  = texture.find_last_of('/');
			if (result != string::npos)			texture = texture.substr(result + 1);

			//  build texture name
			if (_forceDDS)
			{
				result = texture.rfind('.');
				if (result != string::npos)		texture.erase(result);
				texture += ".dds";
			}

			//  build full path
			texture = _pathTexture + texture;

			//  set new texture map
			pDstSText->SetTexture(0, texture);

			logMessage(NCU_MSG_TYPE_TEXTURE, string("Txt-Used: ") + texture);
			if (!checkFileExists(texture))
			{
				_newTextures.insert(string("Txt-Missed: ") + texture);
			}

			//  build normal map name
			result = texture.rfind('.');
			if (result == string::npos)
			{
				texture += "_n";
			}
			else
			{
				string	extension(texture.substr(result));

				texture.erase(result);
				texture += "_n" + extension;
			}

			//  set new normal map
			pDstSText->SetTexture(1, texture);

			if (!checkFileExists(texture))
			{
				_newTextures.insert(string("Txt-Missed: ") + texture);
			}

			//  add texture set to texture property
			pDstLShader->SetTextureSet(pDstSText);

			//  check for existing vertex colors
			if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ((pDstLShader->GetShaderFlags2() & Niflib::SLSF2_VERTEX_COLORS) != 0))
			{
				switch (_vcHandling)
				{
					case NCU_VC_REMOVE_FLAG:
					{
						pDstLShader->SetShaderFlags2((SkyrimShaderPropertyFlags2) (pDstLShader->GetShaderFlags2() & ~Niflib::SLSF2_VERTEX_COLORS));
						break;
					}

					case NCU_VC_ADD_DEFAULT:
					{
						pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));
						break;
					}
				}
			}  //  if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ...

			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);

			//  set new property to node
			pDstNode->SetBSProperty(bsPropIdx++, &(*pDstLShader));
		}
		//  NiMaterialProperty
		else if (DynamicCast<NiMaterialProperty>(*ppIter) != NULL)
		{
			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);
		}
	}  //  for (vector<NiPropertyRef>::iterator  ppIter = dstPropList.begin(); ppIter != dstPropList.end(); ppIter++)

	//  add forced NiAlphaProperty?
	if (forceAlpha)
	{
		NiAlphaPropertyRef	pPropAlpha(new NiAlphaProperty());

		//  set values from template
		pPropAlpha->SetFlags        (pTmplAlphaProp->GetFlags());
		pPropAlpha->SetTestThreshold(pTmplAlphaProp->GetTestThreshold());

		//  set new property to node
		pDstNode->SetBSProperty(bsPropIdx++, &(*pPropAlpha));

		//  mark alpha property
		hasAlpha = true;

	}  //  if (forceAlpha)

	//  add default vertex colors if alpha property and no colors
	if (hasAlpha && (pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0))
	{
		pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));

		//  force flag in BSLightingShaderProperty
		if (pDstLShader != NULL)
		{
			pDstLShader->SetShaderFlags2((SkyrimShaderPropertyFlags2) (pDstLShader->GetShaderFlags2() | Niflib::SLSF2_VERTEX_COLORS));
		}
	}

	//  reorder properties - only necessary in case of both are set
	if (_reorderProperties && (pDstNode->GetBSProperty(0) != NULL) && (pDstNode->GetBSProperty(1) != NULL))
	{
		NiPropertyRef	tProp01(pDstNode->GetBSProperty(0));
		NiPropertyRef	tProp02(pDstNode->GetBSProperty(1));

		//  make sure BSLightingShaderProperty comes before NiAlphaProperty - seems a 'must be'
		if ((tProp01->GetType().GetTypeName() == "NiAlphaProperty") &&
			(tProp02->GetType().GetTypeName() == "BSLightingShaderProperty")
		   )
		{
			pDstNode->SetBSProperty(0, tProp02);
			pDstNode->SetBSProperty(1, tProp01);
		}
	}  //  if (_reorderProperties)

	return  pDstNode;
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility::convertShape(string fileNameSrc, string fileNameDst, string fileNameTmpl)
{
	NiNodeRef				pRootInput     (NULL);
	NiNodeRef				pRootOutput    (NULL);
	NiNodeRef				pRootTemplate  (NULL);
	NiTriShapeRef			pNiTriShapeTmpl(NULL);
	NiCollisionObjectRef	pRootCollObject(NULL);
	vector<NiAVObjectRef>	srcChildList;
	vector<Matrix44>		transformAry;
	bool					fakedRoot      (false);

	//  test on existing file names
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "Source:  "      + (fileNameSrc.empty() ? "- none -" : fileNameSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Template:  "    + (fileNameTmpl.empty() ? "- none -" : fileNameTmpl));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  " + (fileNameDst.empty() ? "- none -" : fileNameDst));
	logMessage(NCU_MSG_TYPE_INFO, "Texture:  "     + (_pathTexture.empty() ? "- none -" : _pathTexture));

	//  initialize used texture list
	_usedTextures.clear();
	_newTextures.clear();

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameSrc + "' as input");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameTmpl + "' as template");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get shapes from template
	//  - shape root
	pNiTriShapeTmpl = DynamicCast<NiTriShape>(pRootTemplate->GetChildren().at(0));
	if (pNiTriShapeTmpl == NULL)
	{
		logMessage(NCU_MSG_TYPE_INFO, "Template has no NiTriShape.");
	}

	//  template root is used as root of output
	pRootOutput     = pRootTemplate;
	pRootCollObject = pRootInput->GetCollisionObject();
	srcChildList    = pRootInput->GetChildren();

	//  move data from input to output
	pRootInput ->SetCollisionObject(NULL);
	pRootOutput->SetCollisionObject(pRootCollObject);
	pRootOutput->SetLocalTransform(pRootInput->GetLocalTransform());
	pRootOutput->SetName(pRootInput->GetName());
	pRootOutput->ClearChildren();
	pRootInput->ClearChildren();

	//  move children
	for (auto pIter=srcChildList.begin(), pEnd=srcChildList.end(); pIter != pEnd; ++pIter)
	{
		pRootOutput->AddChild(*pIter);
	}

	//  iterate over source nodes and convert using template
	pRootOutput = convertNiNode(pRootOutput, pNiTriShapeTmpl, pRootOutput, transformAry);

	//  write missing textures to log - as block
	for (auto pIter=_newTextures.begin(), pEnd=_newTextures.end(); pIter != pEnd; ++pIter)
	{
		logMessage(NCU_MSG_TYPE_TEXTURE_MISS, *pIter);
	}

	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
bool NifConvertUtility::updateTangentSpace(NiTriShapeDataRef pDataObj)
{
	vector<Vector3>		vecVertices (pDataObj->GetVertices());
	vector<Vector3>		vecNormals  (pDataObj->GetNormals());
	vector<Triangle>	vecTriangles(pDataObj->GetTriangles());
	vector<TexCoord>	vecTexCoords;

	//  get first uv-set if available
	if (pDataObj->GetUVSetCount() > 0)		vecTexCoords = pDataObj->GetUVSet(0);

	//  check on valid input data
	if (vecVertices.empty() || vecTriangles.empty() || vecNormals.size() != vecVertices.size() || vecVertices.size() != vecTexCoords.size())
	{
		logMessage(NCU_MSG_TYPE_INFO, "UpdateTangentSpace: No vertices, normals, coords or faces defined.");
		return false;
	}

	//  prepare result vectors
	vector<Vector3>		vecTangents  = vector<Vector3>(vecVertices.size(), Vector3(0.0f, 0.0f, 0.0f));
	vector<Vector3>		vecBiNormals = vector<Vector3>(vecVertices.size(), Vector3(0.0f, 0.0f, 0.0f));

	for (unsigned int t(0); t < vecTriangles.size(); ++t)
	{
		Vector3		vec21(vecVertices[vecTriangles[t][1]] - vecVertices[vecTriangles[t][0]]);
		Vector3		vec31(vecVertices[vecTriangles[t][2]] - vecVertices[vecTriangles[t][0]]);
		TexCoord	txc21(vecTexCoords[vecTriangles[t][1]] - vecTexCoords[vecTriangles[t][0]]);
		TexCoord	txc31(vecTexCoords[vecTriangles[t][2]] - vecTexCoords[vecTriangles[t][0]]);
		float		radius(((txc21.u * txc31.v - txc31.u * txc21.v) >= 0.0f ? +1.0f : -1.0f));

		Vector3		sdir(( txc31.v * vec21[0] - txc21.v * vec31[0] ) * radius,
					     ( txc31.v * vec21[1] - txc21.v * vec31[1] ) * radius,
					     ( txc31.v * vec21[2] - txc21.v * vec31[2] ) * radius);
		Vector3		tdir(( txc21.u * vec31[0] - txc31.u * vec21[0] ) * radius,
					     ( txc21.u * vec31[1] - txc31.u * vec21[1] ) * radius,
					     ( txc21.u * vec31[2] - txc31.u * vec21[2] ) * radius);

		//  normalize
		sdir = sdir.Normalized();
		tdir = tdir.Normalized();

		for (int j(0); j < 3; ++j)
		{
			vecTangents [vecTriangles[t][j]] += tdir;
			vecBiNormals[vecTriangles[t][j]] += sdir;
		}
	}  //  for (unsigned int t(0); t < vecTriangles.size(); ++t)

	for (unsigned int i(0); i < vecVertices.size(); ++i)
	{
		Vector3&	n(vecNormals[i]);
		Vector3&	t(vecTangents [i]);
		Vector3&	b(vecBiNormals[i]);

		if ((t == Vector3()) || (b == Vector3()))
		{
			t[0] = n[1];
			t[1] = n[2];
			t[2] = n[0];

			b = n.CrossProduct(t);
		}
		else
		{
			t = t.Normalized();
			t = (t - n * n.DotProduct(t));
			t = t.Normalized();

			b = b.Normalized();
			b = (b - n * n.DotProduct(b));
			b = (b - t * t.DotProduct(b));
			b = b.Normalized();
		}
	}  //  for (unsigned int i(0); i < vecVertices.size(); ++i)

	//  set tangents and binormals to object
	pDataObj->SetBitangents(vecBiNormals);
	pDataObj->SetTangents  (vecTangents);

	return true;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setTexturePath(string pathTexture)
{
	_pathTexture = pathTexture;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setSkyrimPath(string pathSkyrim)
{
	_pathSkyrim = pathSkyrim;
	transform(_pathSkyrim.begin(), _pathSkyrim.end(), _pathSkyrim.begin(), ::tolower);

	size_t	pos(_pathSkyrim.rfind("\\data\\textures"));

	if (pos != string::npos)
	{
		_pathSkyrim.replace(pos, 14, "");
	}
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setVertexColorHandling(VertexColorHandling vcHandling)
{
	_vcHandling = vcHandling;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setDefaultVertexColor(Color4 defaultColor)
{
	_vcDefaultColor = defaultColor;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setUpdateTangentSpace(bool doUpdate)
{
	_updateTangentSpace = doUpdate;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setReorderProperties(bool doReorder)
{
	_reorderProperties = doReorder;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setForceDDS(bool doForce)
{
	_forceDDS = doForce;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setCleanTreeCollision(bool doClean)
{
	_cleanTreeCollision = doClean;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifConvertUtility::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
set<string>& NifConvertUtility::getUsedTextures()
{
	return _usedTextures;
}

/*---------------------------------------------------------------------------*/
set<string>& NifConvertUtility::getNewTextures()
{
	return _newTextures;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setLogCallback(void (*logCallback) (const int type, const char* pMessage))
{
	_logCallback = logCallback;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::logMessage(int type, string text)
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
			_usedTextures.insert(text);
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

/*---------------------------------------------------------------------------*/
bool NifConvertUtility::checkFileExists(string fileName)
{
	string		path   (_pathSkyrim + "\\Data\\" + fileName);
	ifstream	iStream(path.c_str());

	//  return existance of file
	return iStream.good();
}

/*---------------------------------------------------------------------------*/
BSLightingShaderPropertyRef NifConvertUtility::cloneBSLightingShaderProperty(BSLightingShaderPropertyRef pSource)
{
	BSLightingShaderPropertyRef		pDest(new BSLightingShaderProperty);
	BSShaderTextureSetRef			pTSet(pSource->GetTextureSet());

	//  force empty texture set to source (HACK)
	pSource->SetTextureSet(NULL);

	//  copy all members, even inaccessable ones (HACK)
	memcpy(pDest, pSource, sizeof(BSLightingShaderProperty));

	//  reset source texture set
	pSource->SetTextureSet(pTSet);

	return pDest;
}

/*---------------------------------------------------------------------------*/
bhkShapeRef NifConvertUtility::convertCollShape(bhkShapeRef pShape)
{
	bhkShapeRef		pShapeOut(pShape);

	//  check type of collision mesh
	//bhkMoppBvTreeShape
	if (DynamicCast<bhkMoppBvTreeShape>(pShape) != NULL)
	{
		DynamicCast<bhkMoppBvTreeShape>(pShape)->SetShape(convertCollShape(DynamicCast<bhkMoppBvTreeShape>(pShape)->GetShape()));
	}
	//bhkTransformShape
	else if (DynamicCast<bhkTransformShape>(pShape) != NULL)
	{
		DynamicCast<bhkTransformShape>(pShape)->SetShape(convertCollShape(DynamicCast<bhkTransformShape>(pShape)->GetShape()));
	}
	//bhkListShape
	else if (DynamicCast<bhkListShape>(pShape) != NULL)
	{
		bhkListShapeRef			pTShape(DynamicCast<bhkListShape>(pShape));
		vector<bhkShapeRef>		subShapes(pTShape->GetSubShapes());
		vector<bhkShapeRef>		newShapes;

		//  parse sub shape(s)
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
		{
			//  parse sub shape(s)
			newShapes.push_back(convertCollShape(*pIter));
		}

		//  set new shape list
		pTShape->SetSubShapes(newShapes);
	}
	//bhkPackedNiTriStripsShape
	else if (DynamicCast<bhkPackedNiTriStripsShape>(pShape) != NULL)
	{
		pShapeOut = convertCollPackedNiTriStrips(DynamicCast<bhkPackedNiTriStripsShape>(pShape));
	}

	return pShapeOut;
}

/*---------------------------------------------------------------------------*/
bhkShapeRef NifConvertUtility::convertCollPackedNiTriStrips(bhkPackedNiTriStripsShapeRef pShape)
{
	bhkShapeRef					pShapeOut(pShape);
	hkPackedNiTriStripsDataRef	pData    (DynamicCast<hkPackedNiTriStripsData>(pShape->GetData()));

	//  only convert on existing dsata
	if (pData != NULL)
	{
		vector<OblivionSubShape>	subShapes   (pShape->GetSubShapes());
		vector<Vector3>				vertices    (pData->GetVertices());
		vector<Vector3>				verticesTmp;
		vector<hkTriangle>			triangles   (pData->GetHavokTriangles());
		vector<Triangle>			trianglesTmp;
		hkTriangle&					triangle    (triangles[0]);
		Vector3						tVector;
		unsigned int				verOffset   (0);
		unsigned int				triIndex    (0);
		int							subIndex    (0);

		//  create new result shape
		bhkNiTriStripsShapeRef		pShapeTri(new bhkNiTriStripsShape());

		//  copy attributes
		pShapeTri->SetScale(pShape->GetScale());
		pShapeTri->SetNumStripsData(subShapes.size());

		//  convert each sub-shape
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter, ++subIndex)
		{
			NiTriStripsDataRef	pDataTri(new NiTriStripsData());

			//  reset arrays
			verticesTmp.clear();
			trianglesTmp.clear();

			//  get vertices
			for (unsigned int idx(verOffset), idxMax(verOffset+pIter->numVertices); idx < idxMax; ++idx)
			{
				tVector = vertices[idx];

				//  scale final vertex
				tVector *= 7.0f;

				//  add vertex to tmp. array
				verticesTmp.push_back(tVector);

			}  //  for (unsigned int idx(verOffset), idxMax(verOffset+pIter->numVertices); idx < idxMax; ++idx)

			//  get triangles
			for (; triIndex < triangles.size(); ++triIndex)
			{
				//  check vertex bounds
				triangle = triangles[triIndex];
				if ((triangle.triangle.v1 >= (pIter->numVertices + verOffset)) ||
					(triangle.triangle.v2 >= (pIter->numVertices + verOffset)) ||
					(triangle.triangle.v3 >= (pIter->numVertices + verOffset))
				   )
				{
					break;
				}
			
				trianglesTmp.push_back(Triangle(triangle.triangle.v1-verOffset, triangle.triangle.v2-verOffset, triangle.triangle.v3-verOffset));
			}

			//  fill data object
			pDataTri->SetVertices (verticesTmp);
			pDataTri->SetTriangles(trianglesTmp);

			//  add data to shape
			pShapeTri->SetStripsData(subIndex, pDataTri);

			//  increase vertex offset
			verOffset += pIter->numVertices;

		}  //  for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)

		//  set out shape
		pShapeOut = pShapeTri;

	}  //  if (pData != NULL)

	return pShapeOut;
}
