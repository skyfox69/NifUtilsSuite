/**
 *  file:   SubFormBlenderArmor.h
 *  class:  CSubFormBlenderArmor
 *
 *  Sub form for converting Blender importable to Skyrim armor
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\GUI\SubFormBase.h"
#include "Tools\BlenderPrepare\BPMappingListCtrl.h"

//-----  CLASS  ---------------------------------------------------------------
class CSubFormBlenderArmor : public CSubFormBase
{
	private:
		enum { IDD = IDD_SUB_BLENDER_ARMOR };

	protected:
		CBPMappingListCtrl			_listView;
		CString						_template;

		virtual	BOOL				OnInitDialog();
		virtual	void				DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		afx_msg void				OnBnClickedOpenSettings();

		virtual	void				FillListView();

		DECLARE_DYNAMIC(CSubFormBlenderArmor)
		DECLARE_MESSAGE_MAP()

	public:
									CSubFormBlenderArmor(CWnd* pParent = NULL);   // standard constructor
		virtual						~CSubFormBlenderArmor();

		//  CSubFormBase
		virtual	unsigned short		PerformAction(const string fileNameIn, const string fileNameOut);

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);
};
