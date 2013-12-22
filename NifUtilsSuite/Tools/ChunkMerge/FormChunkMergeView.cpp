/**
 *  file:   FormChunkMergeView.cpp
 *  class:  CFormChunkMergeView
 *
 *  Form for ChunkMerge view
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "NifUtilsSuite.h"
#include "NifUtilsSuiteDoc.h"
#include "Tools\ChunkMerge\FormChunkMergeView.h"
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\FDFileHelper.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifCollisionUtility.h"
#include "Common\Nif\MaterialTypeHandling.h"
#include <afxbutton.h>

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_NSCOPE_IN,      "Open target in NifSkope"},
						                   {IDC_BT_NSCOPE_COLL,    "Open collision model in NifSkope"},
						                   {IDC_BT_VIEW_IN,        "Display target on ModelView page"},
						                   {IDC_BT_VIEW_COLL,      "Display collision model on ModelView page"},
						                   {IDC_RD_COLL_CDATA,     "Use RootCollisionNode as source for collision shapes"},
						                   {IDC_RD_COLL_FBACK,     "Use RootCollisionNode as source for collision shapes, nif not existing: use NiTriShapes of model"},
						                   {IDC_RD_COLL_MESH,      "Use NiTriShapes of model as source for collision shapes"},
						                   {IDC_RD_MAT_SINGLE,     "Use single material for all collision data"},
						                   {IDC_RD_MAT_NITRISHAPE, "Use multiple materials defined by name of NiTriShapes for collision data (e.g. SKY_HAV_MAT_STONE)"},
						                   {IDC_RD_MAT_DEFINE,     "Use multiple materials defined by additional dialog for collision data"},
						                   {IDC_CB_TEMPLATE,       "Define NIF file used as template for creating collision data"},
						                   {IDC_CB_MAT_SINGLE,     "Define material used for all collision shapes"},
										   {IDC_RD_COLL_LOCAL,     "Replace local collision meshes instead of creating one collision mesh for whole model"},
										   {IDC_RD_COLL_GLOBAL,    "Create one collision mesh for whole model"},
						                   {IDC_ED_FILE_IN,        "Path to target NIF-file to add collision data to"},
						                   {IDC_ED_FILE_COLL,      "Path to NIF-file defining collision data"},
						                   {IDC_BT_FILE_IN,        "Choose target NIF-file to add collision data to"},
						                   {IDC_BT_FILE_COLL,      "Choose NIF-file defining collision data"},
						                   {IDC_BT_TEMPLATE,       "Choose path to template files and scan recursively"},
						                   {IDC_BT_RESET_FORM,     "Reset form to default settings"},
						                   {IDC_BT_CONVERT,        "Add collision data to target"},
										   {IDC_CK_REORDER_TRIS,   "Reorder triangles of bhk(Packed)NiTriStrips facing outward of model"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormChunkMergeView, CFormView)

BEGIN_MESSAGE_MAP(CFormChunkMergeView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_VIEW_IN,        &CFormChunkMergeView::OnBnClickedBtViewIn)
	ON_BN_CLICKED(IDC_BT_VIEW_COLL,      &CFormChunkMergeView::OnBnClickedBtViewColl)
	ON_BN_CLICKED(IDC_BT_TEMPLATE,       &CFormChunkMergeView::OnBnClickedOpenSettings)
	ON_BN_CLICKED(IDC_RD_MAT_SINGLE,     &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_RD_MAT_DEFINE,     &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_RD_MAT_NITRISHAPE, &CFormChunkMergeView::OnBnClickedRadioCollMat)
	ON_BN_CLICKED(IDC_BT_FILE_IN,        &CFormChunkMergeView::OnBnClickedBtFileIn)
	ON_BN_CLICKED(IDC_BT_FILE_COLL,      &CFormChunkMergeView::OnBnClickedBtFileColl)
	ON_BN_CLICKED(IDC_BT_RESET_FORM,     &CFormChunkMergeView::OnBnClickedBtResetForm)
	ON_BN_CLICKED(IDC_BT_NSCOPE_IN,      &CFormChunkMergeView::OnBnClickedBtNscopeIn)
	ON_BN_CLICKED(IDC_BT_NSCOPE_COLL,    &CFormChunkMergeView::OnBnClickedBtNscopeColl)
	ON_BN_CLICKED(IDC_BT_CONVERT,        &CFormChunkMergeView::OnBnClickedBtConvert)
	ON_BN_CLICKED(IDC_RD_COLL_GLOBAL,    &CFormChunkMergeView::OnBnClickedRdCollGlobal)
	ON_BN_CLICKED(IDC_RD_COLL_LOCAL,     &CFormChunkMergeView::OnBnClickedRdCollLocal)
END_MESSAGE_MAP()

//-----  CFormChunkMergeView()  -----------------------------------------------
CFormChunkMergeView::CFormChunkMergeView()
	:	ToolsFormViewBase(CFormChunkMergeView::IDD, LogMessageObject::CHUNKMERGE)
{}

//-----  ~CFormChunkMergeView()  ----------------------------------------------
CFormChunkMergeView::~CFormChunkMergeView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormChunkMergeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_ED_FILE_IN,   _fileNameIn);
	DDX_Text(pDX, IDC_ED_FILE_COLL, _fileNameColl);
	DDX_Text(pDX, IDC_CB_TEMPLATE,  _template);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormChunkMergeView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList   (CFDResourceManager::getInstance()->getImageListBrowse());
	CImageList*	pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_IN))  ->SetImage(pImageList->ExtractIcon(1));
	((CMFCButton*) GetDlgItem(IDC_BT_FILE_COLL))->SetImage(pImageList->ExtractIcon(1));
/*
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE))  ->GetDlgItem(1001))->SetReadOnly();
	((CEdit*) ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE))->GetDlgItem(1001))->SetReadOnly();
*/
	::SetWindowTheme(GetDlgItem(IDC_GBOX_FILES)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HINTS)    ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_COLLISION)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_MATERIAL) ->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_GBOX_HANDLING) ->GetSafeHwnd(), _T(""), _T(""));

	pImageList = CFDResourceManager::getInstance()->getImageListNumbers();
	for (short i(1); i < 6; ++i)
	{
		((CStatic*) GetDlgItem(IDC_PC_NUM_0+i)) ->SetIcon(pImageList->ExtractIcon(i));
		((CStatic*) GetDlgItem(IDC_PC_HINT_0+i))->SetIcon(pImageList->ExtractIcon(i));
	}

	pImageList = CFDResourceManager::getInstance()->getImageListOther();
	((CMFCButton*) GetDlgItem(IDC_BT_CONVERT))    ->SetImage(pImageList->ExtractIcon(0));
	((CMFCButton*) GetDlgItem(IDC_BT_MAT_DEFINE)) ->SetImage(pImageList->ExtractIcon(7),  true, NULL, pImageListDis->ExtractIcon(7));
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_FORM)) ->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_IN))    ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_VIEW_COLL))  ->SetImage(pImageList->ExtractIcon(4),  true, NULL, pImageListDis->ExtractIcon(4));
	((CMFCButton*) GetDlgItem(IDC_BT_TEMPLATE))   ->SetImage(pImageList->ExtractIcon(3));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_IN))  ->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));
	((CMFCButton*) GetDlgItem(IDC_BT_NSCOPE_COLL))->SetImage(pImageList->ExtractIcon(10), true, NULL, pImageListDis->ExtractIcon(10));

	GetDlgItem(IDC_BT_VIEW_IN)     ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_IN)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_VIEW_COLL)   ->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_NSCOPE_COLL) ->EnableWindow(FALSE);
	GetDlgItem(IDC_RD_COLL_LOCAL)  ->EnableWindow(FALSE);
	GetDlgItem(IDC_RD_COLL_GLOBAL) ->EnableWindow(FALSE);
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(FALSE);

	//  prepare tool tips
	PrepareToolTips(glToolTiplist);

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);

	//  disable 'defining materials' until working
	GetDlgItem(IDC_RD_MAT_DEFINE)->EnableWindow(FALSE);
}

//-----  OnBnClickedBtViewIn()  -----------------------------------------------
void CFormChunkMergeView::OnBnClickedBtViewIn()
{
	UpdateData(TRUE);
	ShowModel(_fileNameIn);
}

//-----  OnBnClickedBtNscopeIn()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtNscopeIn()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameIn);
}

//-----  OnBnClickedBtViewColl()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtViewColl()
{
	UpdateData(TRUE);
	ShowModel(_fileNameColl);
}

//-----  OnBnClickedBtNscopeColl()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedBtNscopeColl()
{
	UpdateData(TRUE);
	OpenNifSkope(_fileNameColl);
}

//-----  OnBnClickedOpenSettings()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedOpenSettings()
{
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_BT_OPTIONS_EDIT);
}

//-----  OnBnClickedRadioCollMat()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedRadioCollMat()
{
	int		selItem(GetCheckedRadioButton(IDC_RD_MAT_SINGLE, IDC_RD_MAT_DEFINE));

	GetDlgItem(IDC_CB_MAT_SINGLE)->EnableWindow(selItem == IDC_RD_MAT_SINGLE);
	GetDlgItem(IDC_BT_MAT_DEFINE)->EnableWindow(selItem == IDC_RD_MAT_DEFINE);
}

//-----  OnBnClickedBtFileIn()  -----------------------------------------------
void CFormChunkMergeView::OnBnClickedBtFileIn()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameIn, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select input NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameIn = fileName;
		UpdateData(FALSE);
		GetDlgItem(IDC_RD_COLL_GLOBAL) ->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_RD_COLL_LOCAL)  ->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(_fileNameIn == _fileNameColl);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_IN)  ->EnableWindow(!_fileNameIn.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_IN)->EnableWindow(!_fileNameIn.IsEmpty());
}

//-----  OnBnClickedBtFileColl()  ---------------------------------------------
void CFormChunkMergeView::OnBnClickedBtFileColl()
{
	UpdateData(TRUE);

	CString		fileName(GetFileName(_fileNameColl, _T("Nif Files (*.nif)|*.nif||"), _T("nif"), false, _T("Select collision NIF")));

	if (!fileName.IsEmpty())
	{
		_fileNameColl = fileName;
		UpdateData(FALSE);
		GetDlgItem(IDC_RD_COLL_GLOBAL) ->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_RD_COLL_LOCAL)  ->EnableWindow(_fileNameIn == _fileNameColl);
		GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(_fileNameIn == _fileNameColl);
	}
#ifndef NUS_LIGHT
	GetDlgItem(IDC_BT_VIEW_COLL)  ->EnableWindow(!_fileNameColl.IsEmpty());
#endif
	GetDlgItem(IDC_BT_NSCOPE_COLL)->EnableWindow(!_fileNameColl.IsEmpty());
}

//-----  OnBnClickedBtResetForm()  --------------------------------------------
void CFormChunkMergeView::OnBnClickedBtResetForm()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormChunkMergeView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			//  set settings from configuration
			Configuration*	pConfig (Configuration::getInstance());
			CComboBox*		pCBox   ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE));
			set<string>		tDirList(pConfig->getDirListTemplates());
			CString			tmpPath (pConfig->_cmDefaultTemplate.c_str());

			//- templates
			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "Adding templates");
			pCBox->ResetContent();
			for (auto pIter(tDirList.begin()), pEnd(tDirList.end()); pIter != pEnd; ++pIter)
			{
				if (pIter->rfind(".nif") == string::npos)		continue;
				pCBox->AddString(CString(pIter->c_str()));
				LogMessageObject::LogMessage(NCU_MSG_TYPE_SUB_INFO, "added: %s", pIter->c_str());
			}
			pCBox->SelectString(-1, CString(tmpPath));
			pCBox->GetWindowText(tmpPath);
			if (tmpPath.IsEmpty())		pCBox->SetCurSel(0);
			LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "templates found: %d", tDirList.size());

			//- materials
			pCBox = (CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE);
			map<string, NifUtlMaterial>		matMap(NifUtlMaterialList::getInstance()->getMaterialMap());
			short							tIdx  (0);
			short							selIdx(0);

			//  add materials to combo box
			for (auto pIter(matMap.begin()), pEnd(matMap.end()); pIter != pEnd; ++pIter, ++tIdx)
			{
				pCBox->InsertString  (tIdx, CString(pIter->second._name.c_str()));
				pCBox->SetItemDataPtr(tIdx, (void*) (pIter->second._code));

				if (pIter->second._code == pConfig->_cmMatSingleType)
				{
					selIdx = tIdx;
				}
			}
			pCBox->SetCurSel(selIdx);

			//  material flags
			int		selItem(pConfig->_cmMatHandling + IDC_RD_MAT_SINGLE);

			((CButton*) GetDlgItem(IDC_RD_MAT_SINGLE))    ->SetCheck(selItem == IDC_RD_MAT_SINGLE);
			((CButton*) GetDlgItem(IDC_RD_MAT_NITRISHAPE))->SetCheck(selItem == IDC_RD_MAT_NITRISHAPE);
			((CButton*) GetDlgItem(IDC_RD_MAT_DEFINE))    ->SetCheck(selItem == IDC_RD_MAT_DEFINE);
			GetDlgItem(IDC_CB_MAT_SINGLE)->EnableWindow(selItem == IDC_RD_MAT_SINGLE);
			GetDlgItem(IDC_BT_MAT_DEFINE)->EnableWindow(selItem == IDC_RD_MAT_DEFINE);

			//  collision flags
			selItem = pConfig->_cmCollHandling + IDC_RD_COLL_CDATA;
			((CButton*) GetDlgItem(IDC_RD_COLL_CDATA))->SetCheck(selItem == IDC_RD_COLL_CDATA);
			((CButton*) GetDlgItem(IDC_RD_COLL_FBACK))->SetCheck(selItem == IDC_RD_COLL_FBACK);
			((CButton*) GetDlgItem(IDC_RD_COLL_MESH)) ->SetCheck(selItem == IDC_RD_COLL_MESH);

			//  other flags
			((CButton*) GetDlgItem(IDC_RD_COLL_LOCAL))  ->SetCheck(pConfig->_cmMergeColl   ? BST_UNCHECKED : BST_CHECKED);
			((CButton*) GetDlgItem(IDC_RD_COLL_GLOBAL)) ->SetCheck(pConfig->_cmMergeColl   ? BST_CHECKED   : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_REORDER_TRIS))->SetCheck(pConfig->_cmReorderTris ? BST_CHECKED   : BST_UNCHECKED);

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
void CFormChunkMergeView::OnBnClickedBtConvert()
{
	Configuration*			pConfig  (Configuration::getInstance());
	CComboBox*				pCBox    ((CComboBox*) GetDlgItem(IDC_CB_MAT_SINGLE));
	NifCollisionUtility		ncUtility(*(NifUtlMaterialList::getInstance()));
	map<int, unsigned int>	materialMap;
	unsigned short			ncReturn (NCU_OK);

	//  set wait pointer
	BeginWaitCursor();

	//  store data
	UpdateData(TRUE);

	//  build full texture path and set to utility
	ncUtility.setSkyrimPath(pConfig->getPathTextures());

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  get material handling
	MaterialTypeHandling	matHandling((MaterialTypeHandling) (GetCheckedRadioButton(IDC_RD_MAT_SINGLE, IDC_RD_MAT_DEFINE) - IDC_RD_MAT_SINGLE));

	//  set default material
	materialMap[-1] = ((unsigned int) pCBox->GetItemDataPtr(pCBox->GetCurSel()));

	//  add additional materials in case of special handling
	if (matHandling == NCU_MT_MATMAP)
	{
		//  do something special
	}

	//  set flags
	ncUtility.setCollisionNodeHandling((CollisionNodeHandling) (GetCheckedRadioButton(IDC_RD_COLL_CDATA, IDC_RD_COLL_MESH) - IDC_RD_COLL_CDATA));
	ncUtility.setMaterialTypeHandling (matHandling, materialMap);
	ncUtility.setDefaultMaterial      (materialMap[-1]);
	ncUtility.setMergeCollision       (((CButton*) GetDlgItem(IDC_RD_COLL_GLOBAL)) ->GetCheck() == TRUE);
	ncUtility.setReorderTriangles     (((CButton*) GetDlgItem(IDC_CK_REORDER_TRIS))->GetCheck() == TRUE);

	//  add collision data to nif
	ncReturn = ncUtility.addCollision(CStringA(_fileNameColl).GetString(), CStringA(_fileNameIn).GetString(), pConfig->getPathTemplates() + "\\" + CStringA(_template).GetString());

	//  decode result
	if (ncReturn == NCU_OK)
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_SUCCESS, "Collision data added successfully");
	}
	else
	{
		LogMessageObject::LogMessage(NCU_MSG_TYPE_ERROR, "NifCollision returned code: %d", ncReturn);
	}
	LogMessageObject::LogMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");

	//  end wait pointer
	EndWaitCursor();
}

//-----  OnBnClickedRdCollGlobal()  -------------------------------------------
void CFormChunkMergeView::OnBnClickedRdCollGlobal()
{
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(FALSE);
}

//-----  OnBnClickedRdCollLocal()  --------------------------------------------
void CFormChunkMergeView::OnBnClickedRdCollLocal()
{
	GetDlgItem(IDC_CK_REORDER_TRIS)->EnableWindow(TRUE);
}
