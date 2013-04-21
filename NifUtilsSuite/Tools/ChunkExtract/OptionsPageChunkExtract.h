/**
 *  file:   OptionsPageChunkExtract.h
 *  class:  COptionsPageChunkExtract
 *
 *  property page for ChunkExtract options
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"

//-----  CLASS  ---------------------------------------------------------------
class COptionsPageChunkExtract : public CPropertyPage
{
	private:
		enum { IDD = IDD_PROPPAGE_CHUNKEXTRACT };

				int				_nameHandling;
				int				_genNormals;
				int				_scaleToModel;
				int				_saveAs1134;

	protected:
		virtual	void			DoDataExchange(CDataExchange* pDX);
		virtual BOOL			OnSetActive();
		virtual	BOOL			OnInitDialog();
		virtual	void			OnOK();
		virtual	LRESULT			OnWizardNext();

		DECLARE_MESSAGE_MAP()
		DECLARE_DYNAMIC(COptionsPageChunkExtract)

	public:
								COptionsPageChunkExtract(CWnd* pParent = NULL);
		virtual					~COptionsPageChunkExtract();
};
