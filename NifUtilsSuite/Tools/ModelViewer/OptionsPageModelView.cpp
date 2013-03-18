/**
 *  file:   OptionsPageModelView.h
 *  class:  COptionsPageModelView
 *
 *  property page for DirectX model viewer
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Tools\ModelViewer\OptionsPageModelView.h"
#include "Common\Util\Configuration.h"
#include "Common\Util\FDCLibHelper.h"

//-----  DEFINES  -------------------------------------------------------------
IMPLEMENT_DYNAMIC(COptionsPageModelView, CPropertyPage)

BEGIN_MESSAGE_MAP(COptionsPageModelView, CPropertyPage)
END_MESSAGE_MAP()

//-----  COptionsPageGeneral()  -----------------------------------------------
COptionsPageModelView::COptionsPageModelView(CWnd* pParent /*=NULL*/)
	: CPropertyPage(COptionsPageModelView::IDD)
{
	Configuration*	pConfig(Configuration::getInstance());

	_defColorBack.SetColor(argb2bgr(pConfig->_mvDefBackColor));
	_defColorWire.SetColor(argb2bgr(pConfig->_mvDefWireColor));
	_defColorColl.SetColor(argb2bgr(pConfig->_mvDefCollColor));
	_showAxes      = pConfig->_mvShowAxes      ? 1 : 0;
	_showModel     = pConfig->_mvShowModel     ? 1 : 0;
	_showCollision = pConfig->_mvShowCollision ? 1 : 0;
	_showGrid      = pConfig->_mvShowGrid      ? 1 : 0;
	_alterRows     = pConfig->_mvAlterRows     ? 1 : 0;
	_forceDDS      = pConfig->_mvForceDDS      ? 1 : 0;
	_doubleSided   = pConfig->_mvDoubleSided   ? 1 : 0;
}

//-----  ~COptionsPageGeneral()  ----------------------------------------------
COptionsPageModelView::~COptionsPageModelView()
{}

//-----  DoDataExchange()  ----------------------------------------------------
void COptionsPageModelView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Check  (pDX, IDC_CK_END_DDS,        _forceDDS);
	DDX_Check  (pDX, IDC_CK_SHOW_GRID,      _showGrid);
	DDX_Check  (pDX, IDC_CK_VW_AXES,        _showAxes);
	DDX_Check  (pDX, IDC_CK_VW_MODEL,       _showModel);
	DDX_Check  (pDX, IDC_CK_VW_COLLISION,   _showCollision);
	DDX_Check  (pDX, IDC_CK_ROW_BACKGROUND, _alterRows);
	DDX_Check  (pDX, IDC_CK_DOUBLE_SIDED,   _doubleSided);
	DDX_Control(pDX, IDC_BT_COL_BACKGROUND, _defColorBack);
	DDX_Control(pDX, IDC_BT_COL_COLLISION,  _defColorColl);
	DDX_Control(pDX, IDC_BT_COL_WIREFRAME,  _defColorWire);
	DDX_Control(pDX, IDC_VLB_TEXT_PATH,     _vsListBox);
	DDX_Control(pDX, IDC_CB_LOD,            _levelLOD);
}

//-----  OnWizardFinish()  ----------------------------------------------------
BOOL COptionsPageModelView::OnWizardFinish()
{
	OnOK();
	return CPropertyPage::OnWizardFinish();
}

//-----  OnOK()  --------------------------------------------------------------
void COptionsPageModelView::OnOK()
{
	Configuration*	pConfig(Configuration::getInstance());

	UpdateData(TRUE);

	pConfig->_mvShowAxes      = (_showAxes == 1);
	pConfig->_mvShowModel     = (_showModel == 1);
	pConfig->_mvShowCollision = (_showCollision == 1);
	pConfig->_mvShowGrid      = (_showGrid == 1);
	pConfig->_mvAlterRows     = (_alterRows == 1);
	pConfig->_mvForceDDS      = (_forceDDS == 1);
	pConfig->_mvDoubleSided   = (_doubleSided == 1);
	pConfig->_mvDefBackColor  = bgr2argb(_defColorBack.GetColor());
	pConfig->_mvDefWireColor  = bgr2argb(_defColorWire.GetColor());
	pConfig->_mvDefCollColor  = bgr2argb(_defColorColl.GetColor());
	pConfig->_mvTexturePathList.clear();
	for (int i(0), count(_vsListBox.GetCount()); i < count; ++i)
	{
		pConfig->_mvTexturePathList.push_back(CStringA(_vsListBox.GetItemText(i)).GetString());
	}
	pConfig->_mvDefLOD = _levelLOD.GetCurSel();
}

//-----  OnInitDialog()  ------------------------------------------------------
BOOL COptionsPageModelView::OnInitDialog()
{
	Configuration*	pConfig(Configuration::getInstance());

	CPropertyPage::OnInitDialog();

	for (auto pIter=pConfig->_mvTexturePathList.begin(), pEnd=pConfig->_mvTexturePathList.end(); pIter != pEnd; ++pIter)
	{
		_vsListBox.AddItem(CString(pIter->c_str()));
	}
	_levelLOD.SetCurSel(pConfig->_mvDefLOD);

	return TRUE;
}

//-----  OnSetActive()  -------------------------------------------------------
BOOL COptionsPageModelView::OnSetActive()
{
	if (!Configuration::getInstance()->_hasFile)
	{
		CPropertySheet*	pSheet((CPropertySheet*) GetParent());

		pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	}

	return CPropertyPage::OnSetActive();
}
