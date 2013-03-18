/**
 *  file:   OptionsPagematerial.h
 *  class:  COptionsPagematerial
 *
 *  property page for material options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageMaterial : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_MATERIAL };

		CVSListBox				_vsLBIgnore;
		CVSListBox				_vsLBPrefix;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageMaterial)

	public:
								COptionsPageMaterial(CWnd* pParent = NULL);
		virtual					~COptionsPageMaterial();
};
