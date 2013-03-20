/**
 *  file:   OptionsPageGeneral.h
 *  class:  COptionsPageGeneral
 *
 *  property page for general options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageGeneral : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_GENERAL };

				CToolTipCtrl	m_ToolTip;
				CString			_pathSkyrim;
				CString			_pathNifXML;
				CString			_pathTemplates;
				CString			_pathDefInput;
				CString			_pathDefOutput;
				CString			_pathNifSkope;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();
		virtual	BOOL			PreTranslateMessage(MSG* pMsg);
		afx_msg void			OnBnClickedBtPathNifxml();
		afx_msg void			OnBnClickedBtPathSkyrim();
		afx_msg void			OnBnClickedBtPathTemplates();
		afx_msg void			OnBnClickedBtPathInput();
		afx_msg void			OnBnClickedBtPathOutput();
		afx_msg void			OnBnClickedBtPathNifSkope();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageGeneral)

	public:
								COptionsPageGeneral(CWnd* pParent = NULL);
		virtual					~COptionsPageGeneral();
};
