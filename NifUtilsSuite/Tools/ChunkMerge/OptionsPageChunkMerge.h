/**
 *  file:   OptionsPageChunkMerge.h
 *  class:  COptionsPageChunkMerge
 *
 *  property page for ChunkMerge options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageChunkMerge : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_CHUNKMERGE };

				int				_matHandling;
				int				_colHandling;
				int				_matSingle;
				int				_mergeCollision;
				int				_reorderTriangles;
				int				_wndHandling;
				int				_visual;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();
		afx_msg	void			OnCbnSelchangeCbMatSingle();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageChunkMerge)

	public:
								COptionsPageChunkMerge(CWnd* pParent = NULL);
		virtual					~COptionsPageChunkMerge();
};
