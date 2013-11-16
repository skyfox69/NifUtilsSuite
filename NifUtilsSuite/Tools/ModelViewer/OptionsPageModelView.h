/**
 *  file:   OptionsPageModelView.h
 *  class:  COptionsPageModelView
 *
 *  property page for DirectX model viewer
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\GUI\BCBrowseVSListBox.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageModelView : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_MODELVIEW };

				BCBrowseVSListBox	_vsListBox;
				CMFCColorButton		_defColorBack;
				CMFCColorButton		_defColorColl;
				CMFCColorButton		_defColorWire;
				CMFCColorButton		_defColorAmbi;
				CMFCColorButton		_defColorDiff;
				CMFCColorButton		_defColorSpec;
				CComboBox			_levelLOD;
				int					_showAxes;
				int					_showModel;
				int					_showCollision;
				int					_showGrid;
				int					_alterRows;
				int					_forceDDS;
				int					_doubleSided;

	protected:
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual BOOL				OnSetActive();
		virtual	BOOL				OnInitDialog();
		virtual	void				OnOK();
		virtual BOOL				OnWizardFinish();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageModelView)

	public:
									COptionsPageModelView(CWnd* pParent = NULL);
		virtual						~COptionsPageModelView();
};
