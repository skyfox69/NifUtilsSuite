/**
 *  file:   NifUtlMaterial.h
 *  class:  NifUtlMaterialList
 *
 *  data structures handling materials from nif.xml
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include <map>
#include <vector>

//-----  DEFINES  -------------------------------------------------------------
using namespace std;

//-----  CLASS  ---------------------------------------------------------------
struct NifUtlMaterial
{
	unsigned int	_code;
	string			_name;
	string			_defName;
};

//-----  CLASS  ---------------------------------------------------------------
class NifUtlMaterialList
{
	private:
		static	NifUtlMaterialList*				_pInstance;

												NifUtlMaterialList();

		virtual	void							initializeMaterialMap(const string pathToXML, const string matScanTag, const string matScanName);

	protected:
		vector<string>							_userMessages;
		map<string, NifUtlMaterial>				_materialMap;

	public:
		virtual									~NifUtlMaterialList();

				map<string, NifUtlMaterial>&	getMaterialMap    () { return _materialMap; }
				unsigned int					getMaterialCode   (string matDefName);
				string							getMaterialDefName(unsigned int material);
				string							getMaterialName   (unsigned int material);
				virtual vector<string>&			getUserMessages   () { return _userMessages; }
		virtual void							reset             ();

		static	NifUtlMaterialList*				getInstance();
		static	bool							initInstance(const string pathToXML, const string matScanTag, const string matScanName);
};
