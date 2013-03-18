/**
 *  file:   OptionsPageLogView.h
 *  class:  COptionsPageLogView
 *
 *  property page for log view options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageLogView : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_LOGVIEW };

				CMFCColorButton	_defColor[9];
				int				_isEnabled[9];

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageLogView)

	public:
								COptionsPageLogView(CWnd* pParent = NULL);
		virtual					~COptionsPageLogView();
};
