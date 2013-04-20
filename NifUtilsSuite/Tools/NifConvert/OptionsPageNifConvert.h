/**
 *  file:   OptionsPageNifConvert.h
 *  class:  COptionsPageNifConvert
 *
 *  property page for NifConvert options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageNifConvert : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_NIFCONVERT };

				CMFCColorButton	_defColor;
				CComboBox		_defTemplate;
				CComboBox		_defTexture;
				int				_vertexColor;
				int				_upTangent;
				int				_reorderProp;
				int				_forceDDS;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageNifConvert)

	public:
								COptionsPageNifConvert(CWnd* pParent = NULL);
		virtual					~COptionsPageNifConvert();
};
