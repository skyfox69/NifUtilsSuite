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
#include "Tools\ModelViewer\FormModelViewerView.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "DirectX\DirectXNifConverter.h"
#include "DirectX\DirectXObjConverter.h"
#include "DirectX\DirectXMeshAxes.h"
#include "DirectX\DirectXMeshCollision.h"
#include "DirectX\DirectXMeshModelLOD.h"
#include <afxbutton.h>
#include <algorithm>

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_BT_VW_FRONT,     "Switch to front view"},
						                   {IDC_BT_VW_TOP,       "Switch to top view"},
						                   {IDC_BT_VW_SIDE,      "Switch to side view"},
						                   {IDC_SL_LOD,          "Modify Level-Of-Detail"},
						                   {IDC_CK_VW_MODEL,     "Toggle display of model meshes"},
						                   {IDC_CK_VW_COLLISION, "Toggle display of collision wireframe"},
						                   {IDC_CK_VW_AXES,      "Toggle display of coordinate axes"},
						                   {IDC_BT_RESET_VIEW,   "Reset view to default settings"},
						                   {IDC_LV_NODES,        "List of displayed objects. Use multi-select and context menu for more options"},
										   {IDC_CK_DOUBLE_SIDED, "Toggle between one and two side texture rendering"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNCREATE(CFormModelViewerView, CFormView)

BEGIN_MESSAGE_MAP(CFormModelViewerView, CFormView)
	ON_WM_MOUSEWHEEL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BT_VW_FRONT,     &CFormModelViewerView::OnBnClickedBtVwFront)
	ON_BN_CLICKED(IDC_BT_VW_TOP,       &CFormModelViewerView::OnBnClickedBtVwTop)
	ON_BN_CLICKED(IDC_BT_VW_SIDE,      &CFormModelViewerView::OnBnClickedBtVwSide)
	ON_BN_CLICKED(IDC_CK_VW_MODEL,     &CFormModelViewerView::OnBnClickedCkVwButton)
	ON_BN_CLICKED(IDC_CK_VW_AXES,      &CFormModelViewerView::OnBnClickedCkVwButton)
	ON_BN_CLICKED(IDC_CK_VW_COLLISION, &CFormModelViewerView::OnBnClickedCkVwButton)
	ON_BN_CLICKED(IDC_BT_RESET_VIEW,   &CFormModelViewerView::OnBnClickedBtResetView)
	ON_BN_CLICKED(IDC_CK_DOUBLE_SIDED, &CFormModelViewerView::OnBnClickedCkDoubleSided)
END_MESSAGE_MAP()

//-----  CFormChunkMergeView()  -----------------------------------------------
CFormModelViewerView::CFormModelViewerView()
	:	CFormView(CFormModelViewerView::IDD)
{}

//-----  ~CFormChunkMergeView()  ----------------------------------------------
CFormModelViewerView::~CFormModelViewerView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CFormModelViewerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LV_NODES, _listView);
	DDX_Control(pDX, IDC_SL_LOD,   _levelLOD);
}

//-----  PreCreateWindow()  ---------------------------------------------------
BOOL CFormModelViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

//-----  OnInitialUpdate()  ---------------------------------------------------
void CFormModelViewerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	//  initialize GUI (icons, texts, read-only)
	Configuration*	pConfig      (Configuration::getInstance());
	CImageList*		pImageList   (CFDResourceManager::getInstance()->getImageListOther());
	CImageList*		pImageListDis(CFDResourceManager::getInstance()->getImageListOtherDis());
	
	((CMFCButton*) GetDlgItem(IDC_BT_RESET_VIEW))->SetImage(pImageList->ExtractIcon(2));
	((CMFCButton*) GetDlgItem(IDC_BT_VW_FRONT))  ->SetImage(pImageList->ExtractIcon(12), true, NULL, pImageListDis->ExtractIcon(12));
	((CMFCButton*) GetDlgItem(IDC_BT_VW_TOP))    ->SetImage(pImageList->ExtractIcon(14), true, NULL, pImageListDis->ExtractIcon(14));
	((CMFCButton*) GetDlgItem(IDC_BT_VW_SIDE))   ->SetImage(pImageList->ExtractIcon(13), true, NULL, pImageListDis->ExtractIcon(13));

	//  initialize list view
	DWORD	exStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB);

	if (pConfig->_mvShowGrid)		exStyle |= LVS_EX_GRIDLINES;

	_listView.SetExtendedStyle(exStyle);
	_listView.InsertColumn(0, _T("V"),             LVCFMT_LEFT,    22, 0);
	_listView.InsertColumn(1, _T("C"),             LVCFMT_CENTER,  22, 1);
	_listView.InsertColumn(2, _T("Type"),          LVCFMT_LEFT,    88, 2);
	_listView.InsertColumn(3, _T("Name/Material"), LVCFMT_LEFT,   120, 3);
	_listView.SetEmptyMessage("No object model loaded");
	_listView.SetImageList(CFDResourceManager::getInstance()->getImageListModelView(), LVSIL_SMALL);

	//  DirectX view
	CRect	rect;

	GetDlgItem(IDC_VIEW_MODEL)->GetWindowRect(rect);
	ScreenToClient(rect);

	_directXView.dxCreate   (rect, this, &_directXView);
	_directXView.SetOwnTimer(_directXView.SetTimer(1, 10, NULL));

	//  prepare tool tips
	if (_toolTipCtrl.Create(this, TTS_USEVISUALSTYLE | TTS_BALLOON))
	{
		for (short i(0); glToolTiplist[i]._uid != -1; ++i)
		{
			_toolTipCtrl.AddTool(GetDlgItem(glToolTiplist[i]._uid), CString(glToolTiplist[i]._text.c_str()));
		}

		_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
	}

	//  set settings from configuration
	BroadcastEvent(IBCE_CHANGED_SETTINGS);

	//  add default axes
	vector<DirectXMesh*>&	meshList(_directXView.dxGetMeshList());
	DirectXMeshAxes*		pMeshAxes(new DirectXMeshAxes);

	if (!pConfig->_mvShowAxes)		pMeshAxes->SetRenderMode(DXRM_NONE);
	meshList.push_back(pMeshAxes);
	FillListView(meshList);
}

//-----  PreTranslateMessage()  -----------------------------------------------
BOOL CFormModelViewerView::PreTranslateMessage(MSG* pMsg)
{
	if (Configuration::getInstance()->_showToolTipps)
	{
		_toolTipCtrl.RelayEvent(pMsg);
	}

    return CFormView::PreTranslateMessage(pMsg);
}

//-----  OnMouseWheel()  ------------------------------------------------------
BOOL CFormModelViewerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	CRect	rect;

	GetDlgItem(IDC_VIEW_MODEL)->GetWindowRect(rect);

	if ((point.x >= rect.left) &&
		(point.x <= rect.right) &&
		(point.y >= rect.top) &&
		(point.y <= rect.bottom)
	   )
	{
		return _directXView.OnMouseWheel(nFlags, zDelta, point);
	}

	return CFormView::OnMouseWheel(nFlags, zDelta, point);
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CFormModelViewerView::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			//  set settings from configuration
			Configuration*	pConfig(Configuration::getInstance());
			char			cBuffer[10] = { 0 };

			//- list view
			if (pConfig->_mvShowGrid)
			{
				_listView.SetExtendedStyle(_listView.GetExtendedStyle() | LVS_EX_GRIDLINES);
			}
			else
			{
				_listView.SetExtendedStyle(_listView.GetExtendedStyle() & ~LVS_EX_GRIDLINES);
			}

			//- view flags
			((CButton*) GetDlgItem(IDC_CK_VW_AXES)     )->SetCheck(pConfig->_mvShowAxes      ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_VW_MODEL)    )->SetCheck(pConfig->_mvShowModel     ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_VW_COLLISION))->SetCheck(pConfig->_mvShowCollision ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_DOUBLE_SIDED))->SetCheck(pConfig->_mvDoubleSided   ? BST_CHECKED : BST_UNCHECKED);

			//  update model view
			if (!_directXView.dxGetMeshList().empty())
			{
				vector<DirectXMesh*>&	meshList (_directXView.dxGetMeshList());
				bool					showAxes (((CButton*) GetDlgItem(IDC_CK_VW_AXES))     ->GetCheck() == BST_CHECKED);
				bool					showModel(((CButton*) GetDlgItem(IDC_CK_VW_MODEL))    ->GetCheck() == BST_CHECKED);
				bool					showColl (((CButton*) GetDlgItem(IDC_CK_VW_COLLISION))->GetCheck() == BST_CHECKED);

				for (auto pIter=meshList.begin(), pEnd=meshList.end(); pIter != pEnd; ++pIter)
				{
					if (dynamic_cast<DirectXMeshAxes*>(*pIter))
					{
						(*pIter)->SetRenderMode(showAxes ? DXRM_WIREFRAME : DXRM_NONE);
					}
					else if (dynamic_cast<DirectXMeshCollision*>(*pIter))
					{
						(*pIter)->SetRenderMode(showColl ? DXRM_WIREFRAME : DXRM_NONE);
						(*pIter)->SetWireframeColor(pConfig->_mvDefCollColor);
					}
					else if (dynamic_cast<DirectXMeshModel*>(*pIter))
					{
						(*pIter)->SetRenderMode(showModel ? DXRM_TEXTURE : DXRM_NONE);
						(*pIter)->SetWireframeColor(pConfig->_mvDefWireColor);
					}
				}  //  for (auto pIter=meshList.begin(), pEnd=meshList.end(); pIter != pEnd; ++pIter)

				_listView.RedrawItems(0, _listView.GetItemCount() - 1);
				_listView.UpdateWindow();

			}  //  if (!_directXView.dxGetMeshList().empty())

			//  initialize LOD level
			_levelLOD.SetRange(0, 2, TRUE);
			_levelLOD.SetPos(pConfig->_mvDefLOD);
			_snprintf(cBuffer, 10, "%d", pConfig->_mvDefLOD);
			GetDlgItem(IDC_ST_LOD)->SetWindowText(CString(cBuffer));

			break;
		}

		case IBCE_SHOW_MODEL:
		{
			if ((pParameter != NULL) && (*((const char*) pParameter) != 0))
			{
				LoadModel((const char*) pParameter);
			}
			else
			{
				AfxMessageBox(_T("Can't display model!\r\nReason: Missing file name"), MB_OK | MB_ICONERROR);
			}
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

//-----  LoadModel()  ---------------------------------------------------------
void CFormModelViewerView::LoadModel(const string fileName)
{
	Configuration*			pConfig  (Configuration::getInstance());
	DirectXMeshAxes*		pMeshAxes(NULL);
	vector<DirectXMesh*>&	meshList(_directXView.dxGetMeshList());
	string					fName   (fileName);

	//  clear old mesh
	_directXView.dxResetMeshList();

	//  add axes
	pMeshAxes = new DirectXMeshAxes;
	meshList.push_back(pMeshAxes);

	//  disable view of axes if wanted
	if (!pConfig->_mvShowAxes)		pMeshAxes->SetRenderMode(DXRM_NONE);

	//  detect type of source
	transform(fName.begin(), fName.end(), fName.begin(), ::tolower);

	//  NIF source
	if (fName.find(".nif") != string::npos)
	{
		DirectXNifConverter		dxConverter;

		//  set flags
		dxConverter.SetShowModel            (pConfig->_mvShowModel);
		dxConverter.SetShowCollision        (pConfig->_mvShowCollision);
		dxConverter.SetForceDDS             (pConfig->_mvForceDDS);
		dxConverter.SetDefaultWireframeColor(pConfig->_mvDefWireColor);
		dxConverter.SetDefaultCollisionColor(pConfig->_mvDefCollColor);
		dxConverter.SetTexturePathList      (pConfig->_mvTexturePathList);
		dxConverter.SetDoubleSided          (pConfig->_mvDoubleSided);

		//  convert model
		if (!dxConverter.ConvertModel(fileName, meshList))
		{
			AfxMessageBox(_T("Something went wrong while converting model"), MB_OK | MB_ICONERROR);
			return;
		}
	}
	//  OBJ source
	else if (fName.find(".obj") != string::npos)
	{
		DirectXObjConverter		dxConverter;

		//  set flags
		dxConverter.SetShowModel            (pConfig->_mvShowModel);
		dxConverter.SetDefaultWireframeColor(pConfig->_mvDefWireColor);

		//  convert model
		if (!dxConverter.ConvertModel(fileName, meshList))
		{
			AfxMessageBox(_T("Something went wrong while converting model"), MB_OK | MB_ICONERROR);
			return;
		}
	}
	//  unknown source
	else
	{
		AfxMessageBox(_T("Unknown source file format"), MB_OK | MB_ICONERROR);
		return;
	}

	//  fill model listview
	FillListView(meshList);
}

//-----  FillListView()  ------------------------------------------------------
void CFormModelViewerView::FillListView(vector<DirectXMesh*>& meshList)
{
	DirectXMesh*	pMesh    (NULL);
	bool			showAxes (((CButton*) GetDlgItem(IDC_CK_VW_AXES))     ->GetCheck() == BST_CHECKED);
	bool			showModel(((CButton*) GetDlgItem(IDC_CK_VW_MODEL))    ->GetCheck() == BST_CHECKED);
	bool			showColl (((CButton*) GetDlgItem(IDC_CK_VW_COLLISION))->GetCheck() == BST_CHECKED);

	_listView.DeleteAllItems();

	for (unsigned int idx(0); idx < meshList.size(); ++idx)
	{
		pMesh = meshList[idx];

		if (dynamic_cast<DirectXMeshAxes*>(pMesh))
		{
			(pMesh)->SetRenderMode(showAxes ? DXRM_WIREFRAME : DXRM_NONE);
		}
		else if (dynamic_cast<DirectXMeshCollision*>(pMesh))
		{
			(pMesh)->SetRenderMode(showColl ? DXRM_WIREFRAME : DXRM_NONE);
		}
		else if (dynamic_cast<DirectXMeshModel*>(pMesh))
		{
			(pMesh)->SetRenderMode(showModel ? DXRM_TEXTURE : DXRM_NONE);
		}

		_listView.InsertItem (LVIF_IMAGE, idx, NULL, 0, 0, pMesh->GetRenderMode(), NULL);
		_listView.SetItemText(idx, 2, CString(pMesh->GetNifClass().c_str()));
		_listView.SetItemText(idx, 3, CString(pMesh->GetNifName().c_str()));
		_listView.SetItemData(idx, (DWORD_PTR) pMesh);

	}  //  for (unsigned int idx(0); idx < _meshList.size(); ++idx)
}

//-----  OnBnClickedBtVwFront()  ----------------------------------------------
void CFormModelViewerView::OnBnClickedBtVwFront()
{
	_directXView.dxSetCameraPos(DX_CAM_POS_FRONT);
}

//-----  OnBnClickedBtVwTop()  ------------------------------------------------
void CFormModelViewerView::OnBnClickedBtVwTop()
{
	_directXView.dxSetCameraPos(DX_CAM_POS_TOP);
}

//-----  OnBnClickedBtVwSide()  -----------------------------------------------
void CFormModelViewerView::OnBnClickedBtVwSide()
{
	_directXView.dxSetCameraPos(DX_CAM_POS_SIDE);
}

//-----  OnBnClickedCkVwButton()  ---------------------------------------------
void CFormModelViewerView::OnBnClickedCkVwButton()
{
	DirectXMesh*			pMesh    (NULL);
	int						cntItems (_listView.GetItemCount());
	bool					showAxes (((CButton*) GetDlgItem(IDC_CK_VW_AXES))     ->GetCheck() == BST_CHECKED);
	bool					showModel(((CButton*) GetDlgItem(IDC_CK_VW_MODEL))    ->GetCheck() == BST_CHECKED);
	bool					showColl (((CButton*) GetDlgItem(IDC_CK_VW_COLLISION))->GetCheck() == BST_CHECKED);

	//  for each item
	for (int selIdx(0); selIdx < cntItems; ++selIdx)
	{
		pMesh  = (DirectXMesh*) _listView.GetItemData(selIdx);

		if (dynamic_cast<DirectXMeshAxes*>(pMesh))
		{
			pMesh->SetRenderMode(showAxes ? DXRM_WIREFRAME : DXRM_NONE);
		}
		else if (dynamic_cast<DirectXMeshCollision*>(pMesh))
		{
			pMesh->SetRenderMode(showColl ? DXRM_WIREFRAME : DXRM_NONE);
		}
		else if (dynamic_cast<DirectXMeshModel*>(pMesh))
		{
			pMesh->SetRenderMode(showModel ? DXRM_TEXTURE : DXRM_NONE);
		}

		_listView.SetItem(selIdx, 0, LVIF_IMAGE, NULL, (int) pMesh->GetRenderMode(), 0, 0, 0 );

	}  //  for (int selIdx(0); selIdx < cntItems; ++selIdx)
}

//-----  OnBnClickedBtResetView()  --------------------------------------------
void CFormModelViewerView::OnBnClickedBtResetView()
{
	BroadcastEvent(IBCE_CHANGED_SETTINGS);
	_directXView.dxSetCameraPos(DX_CAM_POS_FRONT);
	OnBnClickedCkVwButton();
}

//-----  OnHScroll()  ---------------------------------------------------------
void CFormModelViewerView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//  changed LOD?
	if (pScrollBar->GetDlgCtrlID() == IDC_SL_LOD)
	{
		vector<DirectXMesh*>&	meshList(_directXView.dxGetMeshList());
		int						posAct  (_levelLOD.GetPos());
		int						posMin  (_levelLOD.GetRangeMin());
		int						posMax  (_levelLOD.GetRangeMax());
		char					cBuffer[10] = { 0 };

		switch (nSBCode)
		{
			default:				return;
			case SB_LEFT:			{ posAct = posMin; break; }
			case SB_RIGHT:			{ posAct = posMax; break; }
			case SB_LINELEFT:		{ if (posAct > posMin)	--posAct; break; }
			case SB_LINERIGHT:		{ if (posAct < posMax)	++posAct; break; }
			case SB_PAGELEFT:
			case SB_PAGERIGHT:
			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:	{ posAct = nPos; break; }

		}  //  switch (nSBCode)

		//  set new position
		_levelLOD.SetPos(posAct);

		//  set new LOD level to meshes
		for (auto pIter=meshList.begin(), pEnd=meshList.end(); pIter != pEnd; ++pIter)
		{
			if (dynamic_cast<DirectXMeshModelLOD*>(*pIter))
			{
				(*pIter)->SetLODRenderLevel(posAct);
			}
		}  //  for (auto pIter=meshList.begin(), pEnd=meshList.end(); pIter != pEnd; ++pIter)

		//  update level display
		_snprintf(cBuffer, 10, "%d", posAct);
		GetDlgItem(IDC_ST_LOD)->SetWindowText(CString(cBuffer));

	}  //  if (pScrollBar->GetDlgCtrlID() == IDC_SL_LOD)
}

//-----  OnActivateView()  ----------------------------------------------------
void CFormModelViewerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		_directXView.SetOwnTimer(_directXView.SetTimer(1, 10, NULL));
	}
	else
	{
		_directXView.KillTimer  (1);
		_directXView.SetOwnTimer(0);
	}
	return CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//-----  OnBnClickedCkDoubleSided()  ------------------------------------------
void CFormModelViewerView::OnBnClickedCkDoubleSided()
{
	DirectXMeshModel*	pMesh      (NULL);
	int					cntItems   (_listView.GetItemCount());
	bool				doubleSided(((CButton*) GetDlgItem(IDC_CK_DOUBLE_SIDED))->GetCheck() == BST_CHECKED);

	//  for each item
	for (int selIdx(0); selIdx < cntItems; ++selIdx)
	{
		pMesh = dynamic_cast<DirectXMeshModel*>((DirectXMesh*) _listView.GetItemData(selIdx));

		if (pMesh != NULL)
		{
			pMesh->SetDoubleSided(doubleSided);
		}

	}  //  for (int selIdx(0); selIdx < cntItems; ++selIdx)
}
