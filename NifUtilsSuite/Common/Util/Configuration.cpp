/**
 *  file:   Configuration.h
 *  class:  Configuration
 *
 *  Configuration controller
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDFileHelper.h"
#include <fstream>
#include <sstream>

//-----  DEFINES  -------------------------------------------------------------
Configuration*	Configuration::_pInstance = NULL;

//-----  Configuration()  -----------------------------------------------------
Configuration::Configuration(const string fileName)
	:	_configName      (fileName),
		_matScanTag      ("SkyrimHavokMaterial"),
		_matScanName     ("SKY_HAV_"),
		_hasFile         (false),
		_ncVtFlagsRemove (0),
		_cmMatHandling   (0),
		_cmCollHandling  (1),
		_ceNameHandling  (0),
		_mvDefBackColor  (0xFF000000),
		_mvDefCollColor  (0xFFFF0000),
		_mvDefWireColor  (0xFFFFFF00),
		_mvDefAmbiColor  (0xFF707070),
		_mvDefDiffColor  (0xFFE5E5E5),
		_mvDefSpecColor  (0xFFFFFFFF),
		_ceSaveVersion   (NUS_USERVER_1134),
		_mvDefLOD        (2),
		_lastOpenView    (0),
		_ncUpTangent     (true),
		_ncReorderProp   (true),
		_ncForceDDS      (true),
		_ncRemoveColl    (true),
		_cmMergeColl     (true),
		_cmReorderTris   (true),
		_showToolTipps   (true),
		_saveLastView    (false),
		_ceGenNormals    (true),
		_ceScaleToModel  (true),
		_mvShowGrid      (true),
		_mvAlterRows     (true),
		_mvForceDDS      (false),
		_mvShowAxes      (true),
		_mvShowCollision (true),
		_mvShowModel     (true),
		_mvDoubleSided   (true),
		_mvAutoFocus     (true),
		_bpABRemInvMarker(true),
		_bpABRemBSProp   (true),
		_showLogWindow   (true)
{
	initLogView();
}

//-----  ~Configuration()  -----------------------------------------------------
Configuration::~Configuration()
{
	//if (!_hasFile)		write();
}

//-----  getInstance()  -------------------------------------------------------
Configuration* Configuration::getInstance()
{
	if (_pInstance == NULL)		_pInstance = new Configuration("DefaultConfig.xml");

	return _pInstance;
}

//-----  initInstance()  ------------------------------------------------------
bool Configuration::initInstance(const string fileName)
{
	if (_pInstance == NULL)		_pInstance = new Configuration(fileName);

	return _pInstance->read();
}

//-----  initLogView()  -------------------------------------------------------
void Configuration::initLogView()
{
	//  default log view settings
	_lvwLogActive.resize(10);
	_lvwLogActive[0x00] = true;
	_lvwLogActive[0x01] = true;
	_lvwLogActive[0x02] = true;
	_lvwLogActive[0x03] = true;
	_lvwLogActive[0x04] = true;
	_lvwLogActive[0x05] = true;
	_lvwLogActive[0x06] = true;
	_lvwLogActive[0x07] = true;
	_lvwLogActive[0x08] = false;

	_lvwColors.resize(10);
	_lvwColors[0x00] = RGB(0x00, 0x00, 0x00);
	_lvwColors[0x01] = RGB(0x00, 0xC0, 0xC0);
	_lvwColors[0x02] = RGB(0x00, 0xD0, 0x00);
	_lvwColors[0x03] = RGB(0x00, 0x60, 0x00);
	_lvwColors[0x04] = RGB(0xFF, 0xFF, 0x00);
	_lvwColors[0x05] = RGB(0xFF, 0x00, 0x00);
	_lvwColors[0x06] = RGB(0xA0, 0x00, 0xA0);
	_lvwColors[0x07] = RGB(0xC0, 0x50, 0xFF);
	_lvwColors[0x08] = RGB(0x50, 0x50, 0xFF);
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, map<unsigned short, unsigned short>& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	string			tString;
	unsigned int	offset(offsetIn);

	attribute.clear();

	while (readAttribute(content, tag, tString, offset, offset))
	{
		char*	pToken(strtok((char*) tString.c_str(), ","));
		unsigned short	tmp1((unsigned short) atoi(pToken));

		pToken = strtok(NULL, ",");
		unsigned short	tmp2((unsigned short) atoi(pToken));

		attribute[tmp1] = tmp2;
	}

	return !attribute.empty();
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, vector<unsigned int>& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	unsigned int	tInt;
	unsigned int	offset(offsetIn);

	attribute.clear();

	while (readAttribute(content, tag, tInt, offset, offset))
	{
		attribute.push_back(tInt);
	}

	return !attribute.empty();
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, vector<bool>& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	bool			tBool;
	unsigned int	offset(offsetIn);

	attribute.clear();

	while (readAttribute(content, tag, tBool, offset, offset))
	{
		attribute.push_back(tBool);
	}

	return !attribute.empty();
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, vector<string>& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	string			tString;
	unsigned int	offset(offsetIn);

	attribute.clear();

	while (readAttribute(content, tag, tString, offset, offset))
	{
		attribute.push_back(tString);
	}

	return !attribute.empty();
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, string& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	size_t	posStart(offsetIn);
	size_t	posEnd  (offsetIn);
	bool	isOK    (false);

	posStart = content.find(tag, posStart);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = content.substr(posStart, posEnd - posStart - 2);
			offsetOut = posEnd + tag.length();
			isOK      = true;
		}
	}

	return isOK;
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, int& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	size_t	posStart(offsetIn);
	size_t	posEnd  (offsetIn);
	bool	isOK    (false);

	posStart = content.find(tag, posStart);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = atoi(content.substr(posStart, posEnd - posStart - 2).c_str());
			offsetOut = posEnd + tag.length();
			isOK      = true;
		}
	}

	return isOK;
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, unsigned int& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	size_t	posStart(offsetIn);
	size_t	posEnd  (offsetIn);
	bool	isOK    (false);

	posStart = content.find(tag, posStart);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = strtoul(content.substr(posStart, posEnd - posStart - 2).c_str(), NULL, 0);
			offsetOut = posEnd + tag.length();
			isOK      = true;
		}
	}

	return isOK;
}

//-----  readAttribute()  -----------------------------------------------------
bool Configuration::readAttribute(const string& content, const string tag, bool& attribute, unsigned int& offsetOut, unsigned int offsetIn)
{
	size_t	posStart(offsetIn);
	size_t	posEnd  (offsetIn);
	bool	isOK    (false);

	posStart = content.find(tag, posStart);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = (atoi(content.substr(posStart, posEnd - posStart - 2).c_str()) == 1);
			offsetOut = posEnd + tag.length();
			isOK      = true;
		}
	}

	return isOK;
}

//-----  setPathTemplates()  --------------------------------------------------
void Configuration::setPathTemplates(const string path)
{
	_pathTemplates = path;
	_dirListTemplates.clear();
	FDFileHelper::parseDirectory(CString(_pathTemplates.c_str()), _dirListTemplates, false, false);
}

//-----  setPathTextures()  ---------------------------------------------------
void Configuration::setPathTextures(const string path)
{
	_pathTextures = path;
	if (_pathTextures.find("\\Data\\Textures") == string::npos)
	{
		_pathTextures += "\\Data\\Textures";
	}
	_dirListTextures.clear();
	_dirListTextures.insert("\\");
	FDFileHelper::parseDirectory(CString(_pathTextures.c_str()), _dirListTextures, true, true);
}

//-----  read()  --------------------------------------------------------------
bool Configuration::read()
{
	ifstream		iStr   (_configName.c_str());
	string			content;
	string			search;
	unsigned int	offset (0);

	//  file opened successfully
	if (iStr.is_open())
	{
		while (iStr.good())
		{
			//iStr >> content;
			getline(iStr, content);

			if (content.empty())	continue;

			//  fetch attributes
			readAttribute(content, "PathSkyrim>",              _pathTextures,      offset);
			readAttribute(content, "PathNifXML>",              _pathNifXML,        offset);
			readAttribute(content, "PathNifSkope>",            _pathNifSkope,      offset);
			readAttribute(content, "PathTemplate>",            _pathTemplates,     offset);
			readAttribute(content, "PathDefInput>",            _pathDefaultInput,  offset);
			readAttribute(content, "PathDefOutput>",           _pathDefaultOutput, offset);
			readAttribute(content, "SaveLastView>",            _saveLastView,      offset);
			readAttribute(content, "LastOpenView>",            _lastOpenView,      offset);
			readAttribute(content, "VertexColorHandling>",     _ncVtFlagsRemove,   offset);
			readAttribute(content, "DefaultVertexColor>",      _ncDefColor,        offset);
			readAttribute(content, "UpdateTangentSpace>",      _ncUpTangent,       offset);
			readAttribute(content, "ReorderProperties>",       _ncReorderProp,     offset);
			readAttribute(content, "DefaultTemplateNameNC>",   _ncDefaultTemplate, offset);
			readAttribute(content, "DefaultTextureNameNC>",    _ncDefaultTexture,  offset);
			readAttribute(content, "DefaultTemplateNameCM>",   _cmDefaultTemplate, offset);
			readAttribute(content, "MaterialHandling>",        _cmMatHandling,     offset);
			readAttribute(content, "CollisionHandling>",       _cmCollHandling,    offset);
			readAttribute(content, "MaterialSingleType>",      _cmMatSingleType,   offset);
			readAttribute(content, "MatScanTag>",              _matScanTag,        offset);
			readAttribute(content, "MatScanName>",             _matScanName,       offset);
			readAttribute(content, "MatScanPrefix>",           _matScanPrefix,     offset);
			readAttribute(content, "MatScanIgnore>",           _matScanIgnore,     offset);
			readAttribute(content, "LogVwEnabled>",            _lvwLogActive,      offset);
			readAttribute(content, "LogVwColor>",              _lvwColors,         offset);
			readAttribute(content, "ShowToolTipps>",           _showToolTipps,     offset);
			readAttribute(content, "ShowLogWindow>",           _showLogWindow,     offset);
			readAttribute(content, "NameHandling>",            _ceNameHandling,    offset);
			readAttribute(content, "GenNormals>",              _ceGenNormals,      offset);
			readAttribute(content, "ScaleToModel>",            _ceScaleToModel,    offset);
			readAttribute(content, "CeSaveVersion>",           _ceSaveVersion,     offset);
			readAttribute(content, "DefaultBackColor>",        _mvDefBackColor,    offset);
			readAttribute(content, "DefaultWireColor>",        _mvDefWireColor,    offset);
			readAttribute(content, "DefaultCollColor>",        _mvDefCollColor,    offset);
			readAttribute(content, "DefaultAmbiColor>",        _mvDefAmbiColor,    offset);
			readAttribute(content, "DefaultDiffColor>",        _mvDefDiffColor,    offset);
			readAttribute(content, "DefaultSpecColor>",        _mvDefSpecColor,    offset);
			readAttribute(content, "MvShowGrid>",              _mvShowGrid,        offset);
			readAttribute(content, "MvAlterRows>",             _mvAlterRows,       offset);
			readAttribute(content, "MvForceDDS>",              _mvForceDDS,        offset);
			readAttribute(content, "MvTexturePath>",           _mvTexturePathList, offset);
			readAttribute(content, "MvShowAxes>",              _mvShowAxes,        offset);
			readAttribute(content, "MvShowCollision>",         _mvShowCollision,   offset);
			readAttribute(content, "MvShowModel>",             _mvShowModel,       offset);
			readAttribute(content, "MvLevelOfDetail>",         _mvDefLOD,          offset);
			readAttribute(content, "MvDoubleSided>",           _mvDoubleSided,     offset);
			readAttribute(content, "MvAutoFocus>",             _mvAutoFocus,       offset);
			readAttribute(content, "BpABRemInvMarker>",        _bpABRemInvMarker,  offset);
			readAttribute(content, "BpABRemBSProp>",           _bpABRemBSProp,     offset);
			readAttribute(content, "BpBAMapping>",             _bpBAMapping,       offset);
			readAttribute(content, "DefaultTemplateNameBPBA>", _cmDefaultTemplate, offset);
			readAttribute(content, "NcForceDDS>",              _ncForceDDS,        offset);
			readAttribute(content, "NcRemoveColl>",            _ncRemoveColl,      offset);
			readAttribute(content, "CmMergeColl>",             _cmMergeColl,       offset);
			readAttribute(content, "CmReorderTris>",           _cmReorderTris,     offset);


		}  //  while (iStr.good())

		//  close file
		iStr.close();

		_hasFile = true;

		//  set default values on empty lists
		if (_matScanPrefix.empty())			_matScanPrefix.push_back("Material");
		if (_matScanIgnore.empty())			_matScanIgnore.push_back("Unknown");
		if (_lvwColors.empty())				initLogView();
		if (_lvwLogActive.empty())			initLogView();

		//  get additional directories depending on loaded settings
		setPathTemplates(_pathTemplates);
		setPathTextures (_pathTextures);

	}  //  if (oStr.is_open())

	return _hasFile;
}

//-----  write()  -------------------------------------------------------------
bool Configuration::write()
{
	ofstream	oStr(_configName.c_str(), ios::out | ios::trunc);
	bool		isOK(false);

	//  file opened successfully
	if (oStr.is_open())
	{
		oStr << "<Config><Common>";
		oStr << "<PathSkyrim>"            << _pathTextures            << "</PathSkyrim>";
		oStr << "<PathNifXML>"            << _pathNifXML              << "</PathNifXML>";
		oStr << "<PathNifSkope>"          << _pathNifSkope            << "</PathNifSkope>";
		oStr << "<PathTemplate>"          << _pathTemplates           << "</PathTemplate>";
		oStr << "<PathDefInput>"          << _pathDefaultInput        << "</PathDefInput>";
		oStr << "<PathDefOutput>"         << _pathDefaultOutput       << "</PathDefOutput>";
		oStr << "<ShowToolTipps>"         << _showToolTipps           << "</ShowToolTipps>";
		oStr << "<ShowLogWindow>"         << _showLogWindow           << "</ShowLogWindow>";
		oStr << "<SaveLastView>"          << _saveLastView            << "</SaveLastView>";
		oStr << "<LastOpenView>"          << _lastOpenView            << "</LastOpenView>";
		oStr << "</Common><NifConvert>";
		oStr << "<VertexColorHandling>"   << _ncVtFlagsRemove         << "</VertexColorHandling>";
		oStr << "<DefaultVertexColor>"    << _ncDefColor              << "</DefaultVertexColor>";
		oStr << "<UpdateTangentSpace>"    << (_ncUpTangent   ? 1 : 0) << "</UpdateTangentSpace>";
		oStr << "<ReorderProperties>"     << (_ncReorderProp ? 1 : 0) << "</ReorderProperties>";
		oStr << "<DefaultTemplateNameNC>" << _ncDefaultTemplate       << "</DefaultTemplateNameNC>";
		oStr << "<DefaultTextureNameNC>"  << _ncDefaultTexture        << "</DefaultTextureNameNC>";
		oStr << "<NcForceDDS>"            << _ncForceDDS              << "</NcForceDDS>";
		oStr << "<NcRemoveColl>"          << _ncRemoveColl            << "</NcRemoveColl>";
		oStr << "</NifConvert><ChunkMerge>";
		oStr << "<MaterialHandling>"      << _cmMatHandling           << "</MaterialHandling>";
		oStr << "<CollisionHandling>"     << _cmCollHandling          << "</CollisionHandling>";
		oStr << "<MaterialSingleType>"    << _cmMatSingleType         << "</MaterialSingleType>";
		oStr << "<DefaultTemplateNameCM>" << _cmDefaultTemplate       << "</DefaultTemplateNameCM>";
		oStr << "<CmMergeColl>"           << _cmMergeColl             << "</CmMergeColl>";
		oStr << "<CmReorderTris>"         << _cmReorderTris           << "</CmReorderTris>";
		oStr << "</ChunkMerge><ChunkExtract>";
		oStr << "<NameHandling>"          << _ceNameHandling          << "</NameHandling>";
		oStr << "<GenNormals>"            << _ceGenNormals            << "</GenNormals>";
		oStr << "<ScaleToModel>"          << _ceScaleToModel          << "</ScaleToModel>";
		oStr << "<CeSaveVersion>"         << _ceSaveVersion           << "</CeSaveVersion>";
		oStr << "</ChunkExtract><BlenderPrepare>";
		oStr << "<BpABRemInvMarker>"      << _bpABRemInvMarker        << "</BpABRemInvMarker>";
		oStr << "<BpABRemBSProp>"         << _bpABRemBSProp           << "</BpABRemBSProp>";
		oStr << "<BpBAMappingList>";
		for (auto pIter=_bpBAMapping.begin(), pEnd=_bpBAMapping.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<BpBAMapping>" << pIter->first << "," << pIter->second << "</BpBAMapping>";
		}
		oStr << "</BpBAMappingList>";
		oStr << "<DefaultTemplateNameBPBA>" << _bpBADefaultTemplate   << "</DefaultTemplateNameBPBA>";


		oStr << "</BlenderPrepare><Materials>";
		oStr << "<MatScanTag>"            << _matScanTag              << "</MatScanTag>";
		oStr << "<MatScanName>"           << _matScanName             << "</MatScanName>";
		oStr << "<MatScanPrefixList>";
		for (auto pIter=_matScanPrefix.begin(), pEnd=_matScanPrefix.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<MatScanPrefix>" << *pIter << "</MatScanPrefix>";
		}
		oStr << "</MatScanPrefixList>";
		oStr << "<MatScanIgnoreList>";
		for (auto pIter=_matScanIgnore.begin(), pEnd=_matScanIgnore.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<MatScanIgnore>" << *pIter << "</MatScanIgnore>";
		}
		oStr << "</MatScanIgnoreList>";
		oStr << "</Materials><LogView>";
		oStr << "<LogVwEnabledList>";
		for (auto pIter=_lvwLogActive.begin(), pEnd=_lvwLogActive.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<LogVwEnabled>" << ((*pIter) ? 1 : 0)  << "</LogVwEnabled>";
		}
		oStr << "</LogVwEnabledList>";
		oStr << "<LogVwColorList>";
		for (auto pIter=_lvwColors.begin(), pEnd=_lvwColors.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<LogVwColor>" << *pIter << "</LogVwColor>";
		}
		oStr << "</LogVwColorList>";
		oStr << "</LogView><ModelViewer>";
		oStr << "<DefaultBackColor>"      << _mvDefBackColor          << "</DefaultBackColor>";
		oStr << "<DefaultWireColor>"      << _mvDefWireColor          << "</DefaultWireColor>";
		oStr << "<DefaultCollColor>"      << _mvDefCollColor          << "</DefaultCollColor>";
		oStr << "<DefaultAmbiColor>"      << _mvDefAmbiColor          << "</DefaultAmbiColor>";
		oStr << "<DefaultDiffColor>"      << _mvDefDiffColor          << "</DefaultDiffColor>";
		oStr << "<DefaultSpecColor>"      << _mvDefSpecColor          << "</DefaultSpecColor>";
		oStr << "<MvShowGrid>"            << _mvShowGrid              << "</MvShowGrid>";
		oStr << "<MvAlterRows>"           << _mvAlterRows             << "</MvAlterRows>";
		oStr << "<MvForceDDS>"            << _mvForceDDS              << "</MvForceDDS>";
		oStr << "<MvTexturePathList>";
		for (auto pIter=_mvTexturePathList.begin(), pEnd=_mvTexturePathList.end(); pIter != pEnd; ++pIter)
		{
			oStr << "<MvTexturePath>" << *pIter << "</MvTexturePath>";
		}
		oStr << "</MvTexturePathList>";
		oStr << "<MvShowAxes>"            << _mvShowAxes              << "</MvShowAxes>";
		oStr << "<MvShowCollision>"       << _mvShowCollision         << "</MvShowCollision>";
		oStr << "<MvShowModel>"           << _mvShowModel             << "</MvShowModel>";
		oStr << "<MvLevelOfDetail>"       << _mvDefLOD                << "</MvLevelOfDetail>";
		oStr << "<MvDoubleSided>"         << _mvDoubleSided           << "</MvDoubleSided>";
		oStr << "<MvAutoFocus>"           << _mvAutoFocus             << "</MvAutoFocus>";
		oStr << "</ModelViewer>";


		oStr << "</Config>";

		//  close file
		oStr.close();

		isOK     = true;
		_hasFile = true;

	}  //  if (oStr.is_open())

	return isOK;
}
