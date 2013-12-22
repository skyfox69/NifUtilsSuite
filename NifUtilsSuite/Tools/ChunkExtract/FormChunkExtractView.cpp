/**
 *  file:   FormChunkExtractView.cpp
 *  class:  CFormChunkExtractView
 *
 *  Form for ChunkExtract view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Tools\ChunkExtract\FormChunkExtractView.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifExtractUtility.h"
#include <afxbutton.h>

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,      "Open source in NifSkope"},
						                   {IDC_BT_NSCOPE_OUT,     "Open result in NifSkope"},
						                   {IDC_BT_VIEW_IN,        "Display source on ModelView page"},
						                   {IDC_BT_VIEW_OUT,       "Display collision model on ModelView page"},
						                   {IDC_BT_VIEW_OUT2,      "Display collision model on ModelView page"},
						                   {IDC_BT_FILE_IN,        "Choose source NIF-file to get collision data from"},
						                   {IDC_BT_FILE_OUT,       "Choose target NIF-file to store collision data as Nif"},
						                   {IDC_BT_FILE_OUT2,      "Choose target NIF-file to store collision data as Obj"},
						                   {IDC_BT_RESET_FORM,     "Reset form to default settings"},
						                   {IDC_BT_CONVERT,        "Export collision data to target(s)"},
						                   {IDC_ED_FILE_IN,        "Path to source NIF-file to get collision data from"},
						                   {IDC_ED_FILE_OUT,       "Path to target NIF-file to store collision data to"},
						                   {IDC_ED_FILE_OUT2,      "Path to target OBJ-file to store collision data to"},
						                   {IDC_RD_NAME_MAT,       "Use material name (defined in nif.xml) for naming NiTriShape"},
						                   {IDC_RD_NAME_CHUNK,     "Use index of chunk (Chunk_%d) for naming NiTriShape"},
						                   {IDC_CK_GEN_NORMALS,    "Generate normals from vertices on export if possible"},
						                   {IDC_CK_SCALE_TO_MODEL, "Scale exported mesh to fit model size (x70.0)"},
						                   {IDC_CK_SAVEAS_20207,   "Save NIF as version 20.2.0.7 (UserV: 11, UserV2: 34) for Blender"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormChunkExtractView, CFormView)

BEGIN_MESSAGE_MAP(CFormChunkExtractView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,        &CFormChunkExtractView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT,       &CFormChunkExtractView::OnBnClickedBtViewNif)
	ON_BN_CLICKED(IDC_BT_VIEW_OUT2,      &CFormChunkExtractView::OnBnClickedBtViewObj)
	ON_BN_CLICKED(IDC_BT_FILE_IN,        &CFormChunkExtractView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_OUT,       &CFormChunkExtractView::OnBnClickedBtFileNif)
	ON_BN_CLICKED(IDC_BT_FILE_OUT2,      &CFormChunkExtractView::OnBnClickedBtFileObj)
	ON_BN_CLICKED(IDC_BT_RESET_FORM,     &CFormChunkExtractView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,      &CFormChunkExtractView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_OUT,     &CFormChunkExtractView::OnBnClickedBtNscopeNif)
	ON_BN_CLICKED(IDC_BT_CONVERT,        &CFormChunkExtractView::OnBnClickedBtConvert)
END_MESSAGE_MAP()

//-----  CFormChunkExtractView()  -----------------------------------------------
CFormChunkExtractView::CFormChunkExtractView()
	:	ToolsFormViewBase(CFormChunkExtractView::IDD, LogMessageObject::CHUNKEXTRACT)
{}

//-----  ~CFormChunkExtractView()  ----------------------------------------------
CFormChunkExtractView::~CFormChunkExtractView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormChunkExtractView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,   _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_OUT,  _fileNameNif);
	DDX_Text(pDX, IDC_ED_FILE_OUT2, _fileNameObj);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormChunkExtractView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN))  ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT)) ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_OUT2))->SetImage(pImageList->ExtractIcon(1));

	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_COLLISION)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FLAGS)    ->GetSafeHwnd(), _T(""), _T(""));

	pImageList = CFDResourceManager::getInstance()->getImageListNumbers();
	for (short i(1); i < 5; ++i)
	{
		((CStatic*) GetDlgItem(IDC_PC_NUM_0+i)) ->SetIcon(pImageList->ExtractIcon(i));
		((CStatic*) GetDlgItem(IDC_PC_HINT_0+i))->SetIcon(pImageList->ExtractIcon(i));
	}

	pImageList = CFDResourceManager::getInstance()->getImageListOther();
	((CMFCButton*) GetDlgItem(IDC_BT_CONVERT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_FORM)) ->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_IN))    ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_OUT))   ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_OUT2))  ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_OUT)) ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));

	GetDlgItem(IDC_BT_VIEW_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN) ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(FALSE);

	//  prepare tool tips
	PrepareToolTips(glToolTiplist);

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  OnBnClickedBtViewIn()  -----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtViewIn()
{
	UpdateData(TRUE);
	ShowModel(_fileNameIn);
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormChunkExtractView::OnBnClickedBtNscopeIn()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameIn);
}

//-----  OnBnClickedBtViewNif()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtViewNif()
{
	UpdateData(TRUE);
	ShowModel(_fileNameNif);
}

//-----  OnBnClickedBtNscopeNif()  --------------------------------------------
void CFormChunkExtractView::OnBnClickedBtNscopeNif()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameNif);
}

//-----  OnBnClickedBtViewObj()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtViewObj()
{
	UpdateData(TRUE);
	ShowModel(_fileNameObj);
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileIn()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameIn, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select input NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameIn = fileName;
		UpdateData(FALSE);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileNif()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileNif()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameNif, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), true, _T("Select target NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameNif = fileName;
		UpdateData(FALSE);
	}
}

//-----  OnBnClickedBtFileObj()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtFileObj()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameObj, _T("Obj Files (*.obj)|*.obj||"), _T("obj"), true, _T("Select target OBJ")));

	if (!fileName.IsEmpty())
	{
		_fileNameObj = fileName;
		UpdateData(FALSE);
	}
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormChunkExtractView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormChunkExtractView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			Configuration*	pConfig(Configuration::getInstance());

			//  name flags
			int		selItem(pConfig->_ceNameHandling + IDC_RD_NAME_MAT);

			((CButton*) GetDlgItem(IDC_RD_NAME_MAT))  ->SetCheck(selItem == IDC_RD_NAME_MAT);
			((CButton*) GetDlgItem(IDC_RD_NAME_CHUNK))->SetCheck(selItem == IDC_RD_NAME_CHUNK);

			//  various flags
			((CButton*) GetDlgItem(IDC_CK_GEN_NORMALS))   ->SetCheck(pConfig->_ceGenNormals   ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_SCALE_TO_MODEL))->SetCheck(pConfig->_ceScaleToModel ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_SAVEAS_20207)  )->SetCheck((pConfig->_ceSaveVersion == NUS_USERVER_1134)  ? BST_CHECKED : BST_UNCHECKED);

			break;
		}

		case IBCE_SET_TOOLTIPP:
		{
			_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
			break;
		}
	}  //  switch (event)

	return TRUE;
}

//-----  OnBnClickedBtConvert()  ----------------------------------------------
void CFormChunkExtractView::OnBnClickedBtConvert()
{
	Configuration*			pConfig  (Configuration::getInstance());
	NifExtractUtility		ncUtility(*(NifUtlMaterialList::getInstance()));
	unsigned short			ncReturn (NCU_OK);

	//  store data
	UpdateData(TRUE);

	//  build full texture path and set to utility
	ncUtility.setSkyrimPath(pConfig->getPathTextures());

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  set flags
	ncUtility.setChunkNameHandling((ChunkNameHandling) (GetCheckedRadioButton(IDC_RD_NAME_MAT, IDC_RD_NAME_CHUNK) - IDC_RD_NAME_MAT));
	ncUtility.setGenerateNormals  (((CButton*) GetDlgItem(IDC_CK_GEN_NORMALS))   ->GetCheck() != FALSE);
	ncUtility.setScaleToModel     (((CButton*) GetDlgItem(IDC_CK_SCALE_TO_MODEL))->GetCheck() != FALSE);
	ncUtility.setSaveAsVersion    ((((CButton*) GetDlgItem(IDC_CK_SAVEAS_20207)) ->GetCheck() != FALSE) ? NUS_USERVER_1134 : NUS_USERVER_1283);

	//  extract chunks from NIF
	ncReturn = ncUtility.extractChunks(CStringA(_fileNameIn).GetString(), CStringA(_fileNameNif).GetString(), CStringA(_fileNameObj).GetString());

	//  decode result
	if (ncReturn == NCU_OK)
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "Chunks extracted successfully");
#ifndef NUS_LIGHT
		GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(!_fileNameObj.IsEmpty());
		GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(!_fileNameNif.IsEmpty());
#endif
		GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(!_fileNameNif.IsEmpty());
	}
	else
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "NifCollision returned code: %d", ncReturn);
		GetDlgItem(IDC_BT_VIEW_OUT2) ->EnableWindow(FALSE);
		GetDlgItem(IDC_BT_VIEW_OUT)  ->EnableWindow(FALSE);
		GetDlgItem(IDC_BT_NSCOPE_OUT)->EnableWindow(FALSE);
	}
	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
}
