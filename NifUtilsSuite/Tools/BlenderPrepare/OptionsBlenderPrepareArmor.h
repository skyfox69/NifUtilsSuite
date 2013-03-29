/**
 *  file:   OptionsBlenderPrepareArmor.h
 *  class:  COptionsPageBlenderPrepareArmor
 *
 *  property page for BlenderPrepare armor options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Tools\BlenderPrepare\BPMappingListCtrl.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageBlenderPrepareArmor : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_BLENDPREPARMOR };

		CBPMappingListCtrl		_listView;
		CComboBox				_defTemplate;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();

		virtual void			FillListView();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageBlenderPrepareArmor)

	public:
								COptionsPageBlenderPrepareArmor(CWnd* pParent = NULL);
		virtual					~COptionsPageBlenderPrepareArmor();
};
