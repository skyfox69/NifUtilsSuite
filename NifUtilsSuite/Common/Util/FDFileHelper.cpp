#include "StdAfx.h"
#include "Common\Util\FDFileHelper.h"

/*-------------------------------------------- splitFileName ---------------------------------------*/
void FDFileHelper::splitFileName(const CString path, CString& directory, CString& fileName, CString& extension)
{
  if (!path.IsEmpty())
  {
    char* pChar      (NULL);
    bool  isExtension(false);
    char  cBuffer[5000];

    sprintf_s(cBuffer, 5000, "%s", (const char*) CStringA(path).GetString());

    for (pChar = cBuffer + strlen(cBuffer) - 1; (pChar > cBuffer); --pChar)
    {
      if (*pChar == '.')
      {
        isExtension = true;
        extension   = pChar + 1;
        *pChar      = 0;
      }
      else if (*pChar == '\\')
      {
        fileName = pChar + 1;
        *pChar   = 0;
        break;
      }
    }  //  for (pChar = cBuffer + strlen(cBuffer) - 1; (pChar > cBuffer); --pChar)

    directory = cBuffer;

  }  //  if (!path.IsEmpty())
}

/*-------------------------------------------- getFileName -----------------------------------------*/
CString FDFileHelper::getFile(const CString fileName, CString filter, CString extension, bool saveDialog, CString title, DWORD addFlags)
{
	return getFileOrFolder(fileName, filter, extension, saveDialog, false, title, addFlags);
}

/*-------------------------------------------- getFolderName ---------------------------------------*/
CString FDFileHelper::getFolder(const CString fileName, CString filter, CString extension, bool saveDialog, CString title)
{
	return getFileOrFolder(fileName, filter, extension, saveDialog, true, title);
}

/*-------------------------------------------- getFileName -----------------------------------------*/
CString FDFileHelper::getFileOrFolder(const CString fileName, CString filter, CString extension, bool saveDialog, bool selFolder, CString title, DWORD addFlags)
{
  CString           dirName;
  CString           tmpName;
  CString           extName;
  DWORD             flags  (OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | addFlags);

  //  split path into parts
  splitFileName(fileName, dirName, tmpName, extName);

  //  set some flags and default values
  if (saveDialog)   flags |= OFN_OVERWRITEPROMPT;
  if (selFolder)
  {
    tmpName   = "SelectedFolder";
    extension = "";
  }

  //  create dialog
  CFileDialog   dlg(saveDialog?FALSE:TRUE, extension, tmpName, flags, filter);

  //  check on empty directory => use default one
  if (dirName.IsEmpty())
  {
    dirName = ".";
  }
  dlg.m_ofn.lpstrInitialDir = dirName;

  //  use title if given
  if (!title.IsEmpty())
  {
	  dlg.m_ofn.lpstrTitle = title.GetString();
  }

  //  open dialog
  if (dlg.DoModal() == IDOK)
  {
    //  choose return value
    if (!selFolder)   return dlg.GetPathName();

    splitFileName(dlg.GetPathName(), dirName, tmpName, extName);
    return dirName;
  }

  //  return old value
  return fileName;
}

/*-------------------------------------------- parseDirectory ------------------------------------------*/
void FDFileHelper::parseDirectory(const CString path, set<string>& directoryList, const bool doRelative, const bool doDirs, int relSize)
{
	CFileFind   finder;
	BOOL        result(FALSE);

	if (doRelative && (relSize == 0))		relSize = path.GetLength();

	result = finder.FindFile(path + _T("\\*.*"));

	while (result)
	{
		result = finder.FindNextFile();

		if (finder.IsDots())    continue;
		if (finder.IsDirectory() && doDirs)
		{
			CString   newDir(finder.GetFilePath());
			CString   tDir = newDir.Right(newDir.GetLength() - relSize);

			directoryList.insert(string(CStringA(tDir).GetString()) + "\\");

			parseDirectory(newDir, directoryList, doRelative, doDirs, relSize);
		}
		else if (!finder.IsDirectory() && !doDirs)
		{
			CString   newDir(finder.GetFilePath());
			CString   tDir = newDir.Right(newDir.GetLength() - path.GetLength() - 1);

			directoryList.insert(CStringA(tDir).GetString());
		}
	}
}