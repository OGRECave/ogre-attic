/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#ifndef __NDS_LexiExporter_Dialog_ExporterProperties__
#define __NDS_LexiExporter_Dialog_ExporterProperties__

//

class CExporterPropertiesDlg : public GDI::Dialog
{
public:
	CExporterPropertiesDlg(Window* pParent, Interface* pMax, IUtil* pMaxUtil, CExportObjectRoot *pRoot);
	~CExporterPropertiesDlg();

public:

	bool m_bChanges;		

	void PopulateExportTree();
//	void AddToExportTree(unsigned int iConfigIndex);
//	void UpdateItemInList(int iIndex);

	// Accessible for AddMultiple Dialog
	Interface* m_pMax;
	IUtil* m_pMaxUtil;
	std::vector<CExportObject*> m_lTypeCache;

protected:
	void	OnInitDialog();
	bool	OnValidate();
	void	OnSize();	
	void	OnConfigButtonAdd();
	void	OnConfigButtonAddSelected();
	void	OnConfigButtonRemove();	
	void	OnLoadSelection();	
	void	OnConfigButtonSettings();
	void	OnConfigButtonExport();
	void	OnConfigButtonExportSelected();
	void	OnViewLog();
	void	OnSelChange();
	void	OnCheckChange(HTREEITEM hItem);
	void	OnRClickTree();
	void	OnDataChange();
	void	OnButtonAdd(GDI::Button* pButton, unsigned int iSelectedID);

	void	AddObjectToTree(CExportObject *pObject, HTREEITEM hParent);
	void	InternalCreate(int iTypeIndex, HTREEITEM hParent, unsigned int iSelectedID);
	void	InternalRemove(HTREEITEM hItem);
	void	UpdateEditWindow();
	void	GetSubItems(HTREEITEM hParent, std::vector<HTREEITEM> &lList);

private:
	// Image list for icons in the treectrl
	HIMAGELIST m_hImageList;
	std::map<std::string, unsigned int> m_ImageListMap;	

	// UI controls
	GDI::Button		m_ButtonAdd;
	GDI::Button		m_ButtonAddSelected;
	GDI::Button		m_ButtonRemove;
	GDI::Button		m_ButtonSettings;
	GDI::Button		m_ButtonExport;
	GDI::Button		m_ButtonLoadSelection;
	GDI::Button		m_ButtonExportSelected;
	GDI::TreeCtrl	m_ExportTree;
	GDI::Window		*m_pCurrentEditWindow;	

	CExportObjectRoot	*m_pRoot;	
	HTREEITEM			m_hRootItem;

	// Currently selected ExportObject
	CExportObject*		m_pCurrent;

	// Last object to have edit control displayed
	CExportObject*		m_pLastEditObject;

	// Original Client Rectangle (initialize in OnInitDialog())
	RECT	m_OrgClientRect;

	//
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

//

#endif // __NDS_LexiExporter_Dialog_ExporterProperties__