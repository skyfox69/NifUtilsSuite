/**
 *  file:   SubFormArmorBlender.cpp
 *  class:  CSubFormArmorBlender
 *
 *  Sub form for converting Skyrim armor to Blender importable
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "afxdialogex.h"
#include "NifUtilsSuite.h"
#include "Tools\BlenderPrepare\SubFormArmorBlender.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDToolTipText.h"
#include "Common\Nif\NifPrepareUtility.h"

extern void logCallback(const int type, const char* pMessage);

//-----  DEFINES  -------------------------------------------------------------
static SFDToolTipText	glToolTiplist[] = {{IDC_CK_OPTION_01, "Remove all BSInvMarker nodes from NIF tree"},
						                   {IDC_CK_OPTION_02, "Remove all BSProperties nodes from NiTriShapes"},
						                   {-1, ""}
						                  };

IMPLEMENT_DYNAMIC(CSubFormArmorBlender, CSubFormBase)

BEGIN_MESSAGE_MAP(CSubFormArmorBlender, CSubFormBase)
END_MESSAGE_MAP()

//-----  CSubFormArmorBlender()  ----------------------------------------------
CSubFormArmorBlender::CSubFormArmorBlender(CWnd* pParent /*=NULL*/)
	:	CSubFormBase(CSubFormArmorBlender::IDD, pParent)
{}

//-----  ~CSubFormArmorBlender()  ---------------------------------------------
CSubFormArmorBlender::~CSubFormArmorBlender()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void CSubFormArmorBlender::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL CSubFormArmorBlender::OnInitDialog()
{
	CSubFormBase::OnInitDialog();

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
BOOL CSubFormArmorBlender::BroadcastEvent(WORD event, void* pParameter)
{
	switch (event)
	{
		case IBCE_CHANGED_SETTINGS:
		{
			Configuration*	pConfig(Configuration::getInstance());

			//  various flags
			((CButton*) GetDlgItem(IDC_CK_OPTION_01))->SetCheck(pConfig->_bpABRemInvMarker ? BST_CHECKED : BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_CK_OPTION_02))->SetCheck(pConfig->_bpABRemBSProp    ? BST_CHECKED : BST_UNCHECKED);

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

//-----  PerformAction()  -----------------------------------------------------
unsigned short CSubFormArmorBlender::PerformAction(const string fileNameIn, const string fileNameOut)
{
	NifPrepareUtility	ncUtility;

	//  store data
	UpdateData(TRUE);

	//  set callback for log info
	ncUtility.setLogCallback(logCallback);

	//  set flags
	ncUtility.setRemoveBSInvMarker (((CButton*) GetDlgItem(IDC_CK_OPTION_01))->GetCheck() != FALSE);
	ncUtility.setRemoveBSProperties(((CButton*) GetDlgItem(IDC_CK_OPTION_02))->GetCheck() != FALSE);

	//  prepare NIF
	return ncUtility.prepareArmorBlender(fileNameIn, fileNameOut);
}
