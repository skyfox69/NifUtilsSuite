/**
 *  file:   SubFormBlenderArmor.cpp
 *  class:  CSubFormBlenderArmor
 *
 *  Sub form for converting Blender importable to Skyrim armor
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "afxdialogex.h"
#include "NifUtilsSuite.h"
#include "Tools\BlenderPrepare\SubFormBlenderArmor.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDResourceManager.h"
#include "Common\Util\LogMessageObject.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Common\Nif\NifPrepareUtility.h"
#include "gen\enums.h"

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_CB_TEMPLATE, "Define NIF file used as template for converting"},
						                   {IDC_BT_TEMPLATE, "Choose path to template files and scan recursively"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNAMIC(CSubFormBlenderArmor, CSubFormBase)

BEGIN_MESSAGE_MAP(CSubFormBlenderArmor, CSubFormBase)
	ON_BN_CLICKED(IDC_BT_TEMPLATE, &CSubFormBlenderArmor::OnBnClickedOpenSettings)
END_MESSAGE_MAP()

using namespace Niflib;

//-----  CSubFormBlenderArmor()  ----------------------------------------------
CSubFormBlenderArmor::CSubFormBlenderArmor(CWnd* pParent /*=NULL*/)
	:	CSubFormBase(CSubFormBlenderArmor::IDD, pParent)
{}

//-----  ~CSubFormBlenderArmor()  ---------------------------------------------
CSubFormBlenderArmor::~CSubFormBlenderArmor()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CSubFormBlenderArmor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LV_MAPPING, _listView);
	DDX_Text(pDX, IDC_CB_TEMPLATE,   _template);
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CSubFormBlenderArmor::OnInitDialog()
{
	CSubFormBase::OnInitDialog();

	//  initialize GUI (icons, texts, read-only)
	CImageList*	pImageList(CFDResourceManager::getInstance()->getImageListOther());

	((CMFCButton*) GetDlgItem(IDC_BT_TEMPLATE))->SetImage(pImageList->ExtractIcon(3));

	//  initialize list view
	_listView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB | LVS_EX_GRIDLINES);
	_listView.InsertColumn(0, _T("Blender-BodyPart"), LVCFMT_LEFT, 145, 0);
	_listView.InsertColumn(1, _T("Skyrim-BodyPart"),  LVCFMT_LEFT, 152, 1);
	_listView.SetEmptyMessage("No body parts defined");
	_listView.EnableToolTips(TRUE);

	//  prepare tool tips
	if (_toolTipCtrl.Create(this, TTS_USEVISUALSTYLE | TTS_BALLOON))
	{
		for (short i(0); glToolTiplist[i]._uid != -1; ++i)
		{
			_toolTipCtrl.AddTool(GetDlgItem(glToolTiplist[i]._uid), CString(glToolTiplist[i]._text.c_str()));
		}

		_toolTipCtrl.Activate(Configuration::getInstance()->_showToolTipps);
	}

	return TRUE;
}

//-----  BroadcastEvent()  ----------------------------------------------------
BOOL CSubFormBlenderArmor::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			LogMessageObject*	pLogObj(dynamic_cast<LogMessageObject*>(GetParent()));
			Configuration*		pConfig(Configuration::getInstance());
			CComboBox*			pCBox   ((CComboBox*) GetDlgItem(IDC_CB_TEMPLATE));
			set<string>			tDirList(pConfig->getDirListTemplates());
			CString				tmpPath (pConfig->_bpBADefaultTemplate.c_str());

			//- templates
			if (pLogObj != NULL)	pLogObj->LogMessage(NCU_MSG_TYPE_INFO, "Adding templates");
			pCBox->ResetContent();
			for (auto pIter(tDirList.begin()), pEnd(tDirList.end()); pIter != pEnd; ++pIter)
			{
				if (pIter->rfind(".nif") == string::npos)		continue;
				pCBox->AddString(CString(pIter->c_str()));
				if (pLogObj != NULL)	pLogObj->LogMessage(NCU_MSG_TYPE_SUB_INFO, "added: %s", pIter->c_str());
			}
			pCBox->SelectString(-1, CString(tmpPath));
			pCBox->GetWindowText(tmpPath);
			if (tmpPath.IsEmpty())		pCBox->SetCurSel(0);
			if (pLogObj != NULL)	pLogObj->LogMessage(NCU_MSG_TYPE_INFO, "templates found: %d", tDirList.size());

			//- mapping
			FillListView();

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

//-----  FillListView()  ------------------------------------------------------
void CSubFormBlenderArmor::FillListView()
{
	NifUtlMaterialList*						pMatList(NifUtlMaterialList::getInstance());
	map<unsigned short, unsigned short>&	bpMap(Configuration::getInstance()->_bpBAMapping);
	int										idx  (0);

	//  empty listview
	_listView.DeleteAllItems();

	//  initialize body part list if not done yet
	if (bpMap.empty())
	{
		map<unsigned short, string>&	defBPMap(pMatList->getBodyPartMap());

		for (auto pIter=defBPMap.begin(), pEnd=defBPMap.end(); pIter != pEnd; ++pIter)
		{
			if (pIter->first > BP_BRAIN)	break;

			bpMap[pIter->first] = SBP_32_BODY;
		}
	}  //  if (bpList.empty())

	//  populate list view
	for (auto pIter=bpMap.begin(), pEnd=bpMap.end(); pIter != pEnd; ++pIter, ++idx)
	{
		_listView.InsertItem (idx, CString(pMatList->getBodyPartName(pIter->first).c_str()));
		_listView.SetItemText(idx, 1, CString(pMatList->getBodyPartName(pIter->second).c_str()));
		_listView.SetItemData(idx, (DWORD_PTR) ((pIter->second << 16) | pIter->first));
	}
}

//-----  OnBnClickedOpenSettings()  -------------------------------------------
void CSubFormBlenderArmor::OnBnClickedOpenSettings()
{
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_BT_OPTIONS_EDIT);
}

//-----  PerformAction()  -----------------------------------------------------
unsigned short CSubFormBlenderArmor::PerformAction(const string fileNameIn, const string fileNameOut)
{
	NifPrepareUtility						ncUtility;
	map<unsigned short, unsigned short>		bodyPartMap;
	int										mapping(0);

	//  store data
	UpdateData(TRUE);

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  get body part mapping from list
	for (int idx(0); idx < _listView.GetItemCount(); ++idx)
	{
		mapping = _listView.GetItemData(idx);
		bodyPartMap[mapping & 0x0000ffff] = mapping >> 16;
	}

	//  set map
	ncUtility.setBodyPartMap(bodyPartMap);

	//  prepare NIF
	return ncUtility.prepareArmorBlender(fileNameIn, fileNameOut);
}
