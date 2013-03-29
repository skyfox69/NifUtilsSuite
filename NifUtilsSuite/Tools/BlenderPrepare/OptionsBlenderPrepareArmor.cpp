/**
 *  file:   OptionsBlenderPrepareArmor.h
 *  class:  COptionsPageBlenderPrepareArmor
 *
 *  property page for BlenderPrepare armor options
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\Configuration.h"
#include "Common\Nif\NifUtlMaterial.h"
#include "Tools\BlenderPrepare\OptionsBlenderPrepareArmor.h"
#include "gen/enums.h"

using namespace Niflib;

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageBlenderPrepareArmor, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageBlenderPrepareArmor, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageBlenderPrepareArmor()  -----------------------------------
COptionsPageBlenderPrepareArmor::COptionsPageBlenderPrepareArmor(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageBlenderPrepareArmor::IDD)
{}

//-----  ~COptionsPageBlenderPrepareArmor()  ----------------------------------
COptionsPageBlenderPrepareArmor::~COptionsPageBlenderPrepareArmor()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageBlenderPrepareArmor::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LV_MAPPING,  _listView);
	DDX_Control(pDX, IDC_CB_TEMPLATE, _defTemplate);
}

//-----  OnWizardNext()  ------------------------------------------------------
LRESULT COptionsPageBlenderPrepareArmor::OnWizardNext()
{
	OnOK();
	return CPropertyPage::OnWizardNext();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageBlenderPrepareArmor::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());
	CString			tString;
	int				mapping(0);

	UpdateData(TRUE);

	_defTemplate.GetLBText(_defTemplate.GetCurSel(), tString);
	pConfig->_bpBADefaultTemplate = CStringA(tString).GetString();

	for (int idx(0); idx < _listView.GetItemCount(); ++idx)
	{
		mapping = _listView.GetItemData(idx);
		pConfig->_bpBAMapping[mapping & 0x0000ffff] = mapping >> 16;
	}
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageBlenderPrepareArmor::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//  initialize list view
	_listView.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB | LVS_EX_GRIDLINES);
	_listView.InsertColumn(0, _T("Blender-BodyPart"), LVCFMT_LEFT, 145, 0);
	_listView.InsertColumn(1, _T("Skyrim-BodyPart"),  LVCFMT_LEFT, 148, 1);
	_listView.SetEmptyMessage("No body parts defined");
	_listView.EnableToolTips(TRUE);
	FillListView();

	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageBlenderPrepareArmor::OnSetActive()
{
	Configuration*	pConfig(Configuration::getInstance());

	//  first run => initialize wizard mode
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	}

	_defTemplate.ResetContent();
	for (auto pIter=pConfig->getDirListTemplates().begin(), pEnd=pConfig->getDirListTemplates().end(); pIter != pEnd; ++pIter)
	{
		_defTemplate.AddString(CString(pIter->c_str()));
	}
	if (pConfig->_bpBADefaultTemplate.empty())
	{
		_defTemplate.SetCurSel(0);
	}
	else
	{
		_defTemplate.SelectString(-1, CString(pConfig->_bpBADefaultTemplate.c_str()));
	}

	return CPropertyPage::OnSetActive();
}

//-----  FillListView()  ------------------------------------------------------
void COptionsPageBlenderPrepareArmor::FillListView()
{
	NifUtlMaterialList*						pMatList(NifUtlMaterialList::getInstance());
	map<unsigned short, unsigned short>&	bpMap(Configuration::getInstance()->_bpBAMapping);
	int										idx  (0);

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
