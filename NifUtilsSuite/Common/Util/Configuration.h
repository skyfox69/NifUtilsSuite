/**
 *  file:   Configuration.h
 *  class:  Configuration
 *
 *  Configuration controller
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include <vector>
#include <set>
#include <map>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class Configuration
{
	private:
		static	Configuration*			_pInstance;
				set<string>				_dirListTemplates;
				set<string>				_dirListTextures;
				string					_pathTemplates;
				string					_pathTextures;

										Configuration(const string fileName);

		virtual	void					initLogView();

		virtual	bool					readAttribute(const string& content, const string tag, map<unsigned short, unsigned short>& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, vector<unsigned int>& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, vector<bool>& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, vector<string>& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, string& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, int& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, unsigned int& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);
		virtual	bool					readAttribute(const string& content, const string tag, bool& attribute, unsigned int& offsetOut, unsigned int offsetIn=0);

	public:
				string					_configName;
				string					_pathNifXML;
				string					_pathNifSkope;
				string					_pathDefaultInput;
				string					_pathDefaultOutput;
				string					_ncDefaultTemplate;
				string					_ncDefaultTexture;
				string					_cmDefaultTemplate;
				string					_bpBADefaultTemplate;
				string					_matScanTag;
				string					_matScanName;
				vector<string>			_matScanPrefix;
				vector<string>			_matScanIgnore;
				vector<string>			_mvTexturePathList;
				map<unsigned short, unsigned short>	_bpBAMapping;
				vector<unsigned int>	_lvwColors;
				vector<bool>			_lvwLogActive;
				unsigned int			_ncDefColor;
				unsigned int			_mvDefCollColor;
				unsigned int			_mvDefWireColor;
				unsigned int			_mvDefBackColor;
				unsigned int			_mvDefLOD;
				int						_ncVtFlagsRemove;
				int						_cmMatHandling;
				int						_cmCollHandling;
				int						_cmMatSingleType;
				int						_ceNameHandling;
				int						_lastOpenView;
				bool					_hasFile;
				bool					_ncUpTangent;
				bool					_ncReorderProp;
				bool					_showToolTipps;
				bool					_saveLastView;
				bool					_ceGenNormals;
				bool					_ceScaleToModel;
				bool					_ceSaveAs20207;
				bool					_mvShowGrid;
				bool					_mvAlterRows;
				bool					_mvForceDDS;
				bool					_mvShowAxes;
				bool					_mvShowCollision;
				bool					_mvShowModel;
				bool					_mvDoubleSided;
				bool					_bpABRemInvMarker;
				bool					_bpABRemBSProp;

		virtual							~Configuration();

		virtual	bool					read();
		virtual	bool					write();

		static	Configuration*			getInstance();
		static	bool					initInstance(const string fileName);

		virtual	set<string>&			getDirListTemplates()					{ return _dirListTemplates; }
		virtual	string					getPathTemplates()						{ return _pathTemplates; }
		virtual void					setPathTemplates(const string path);
		virtual	set<string>&			getDirListTextures()					{ return _dirListTextures; }
		virtual	string					getPathTextures()						{ return _pathTextures; }
		virtual void					setPathTextures(const string path);
};
