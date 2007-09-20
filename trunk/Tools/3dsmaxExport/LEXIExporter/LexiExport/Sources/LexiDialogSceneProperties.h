/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn
Lasse Tassing
Marty Rabens

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef __NDS_LexiExporter_ScenePropertiesDialog__
#define __NDS_LexiExporter_ScenePropertiesDialog__

//

class CScenePropertiesDlg : public GDI::Dialog, public IDDNotify
{	
public:
	CScenePropertiesDlg(Window* pParent);	
	~CScenePropertiesDlg();

	// Initialize controls from object
	void	Init(CDDObject *pMeta, const char *pszDefExt);	
	void	SetInstance(CDDObject *pData, CExportObject* pObj);

private:
	void	OnInitDialog();
	void	OnSize();	
	void	BrowseOutput();
	void	OnFileNameChange();
	void	OnNameChange();
	void	BrowseNode();
	void	OnChanged(const CDDObject *pInstance, const char *pszKey);

	std::string	m_sDefExt;

	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	CExportObject* m_pObj;
	CDDObject	*m_pData;

	HWND m_hMetaWnd;
	GDI::MetaControl* m_pMetaCtrl;

	RECT	m_OrgClientRect;
};

//

#endif // __NDS_LexiExporter_ScenePropertiesDialog__