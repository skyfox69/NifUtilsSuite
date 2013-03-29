/**
 *  file:   SubFormArmorBlender.h
 *  class:  CSubFormArmorBlender
 *
 *  Sub form for converting Skyrim armor to Blender importable
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\GUI\SubFormBase.h"

//-----  CLASS  ---------------------------------------------------------------
class CSubFormArmorBlender : public CSubFormBase
{
	private:
		enum { IDD = IDD_SUB_ARMOR_BLENDER };

	protected:
		virtual	BOOL				OnInitDialog();
		virtual	void				DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_DYNAMIC(CSubFormArmorBlender)
		DECLARE_MESSAGE_MAP()

	public:
									CSubFormArmorBlender(CWnd* pParent = NULL);   // standard constructor
		virtual						~CSubFormArmorBlender();

		//  CSubFormBase
		virtual	unsigned short		PerformAction(const string fileNameIn, const string fileNameOut);

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);
};
