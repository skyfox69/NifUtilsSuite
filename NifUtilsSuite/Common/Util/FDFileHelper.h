#pragma once

#include <set>
#include <string>

using namespace std;

class FDFileHelper
{
  private:
    static  void		splitFileName  (const CString path, CString& directory, CString& fileName, CString& extension);

  public:
    static  CString		getFileOrFolder(const CString fileName, CString filter, CString extension, bool saveDialog=false, bool selFolder=false, CString title=_T(""), DWORD addFlags=0);
    static  CString		getFile        (const CString fileName, CString filter, CString extension, bool saveDialog=false, CString title=_T(""), DWORD addFlags=0);
    static  CString		getFolder      (const CString fileName, CString filter, CString extension, bool saveDialog=false, CString title=_T(""));

	static	void		parseDirectory (const CString path, set<string>& directoryList, const bool doRelative=false, const bool doDirs=true, int relSize=0);
};
