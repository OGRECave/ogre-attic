/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Lasse Tassing

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
#ifndef __NDS_LexiExporter_AddMultipleDialog__
#define __NDS_LexiExporter_AddMultipleDialog__

//
class CAddMultipleDlg : public GDI::Dialog 
{
public:
	// Struct to hold information about each item in listbox (custom drawed)
	struct SListItem
	{
		INode* pNode;
		SClass_ID NodeClassID;
		std::string sNodeName;
		int	ExportType;
		bool bCanEdit;
	};

	CAddMultipleDlg(Window* pParent, CExporterPropertiesDlg *pOwner, CExportObject *pRootObject);
	~CAddMultipleDlg();

	// If dialog returns IDOK, a list of export items can be retrieved from this function
	vector<SListItem*>	GetExportItems();

protected:
	virtual void OnInitDialog();

	// Custom drawed listbox
	GDI::ListBox	m_ListBox;	

	// Dropdown listbox
	GDI::ListBox	m_TypePopup;
	SListItem*		m_pEditItem;

	// Parent ExportObject
	CExportObject* m_pRootObject;

	// 
	CExporterPropertiesDlg *m_pOwner;

	// Icons for item list
	HIMAGELIST m_hImageList;
	std::map<SClass_ID, unsigned int> m_ImageListMap;

	// Selected items
	vector<CAddMultipleDlg::SListItem*> m_lSelectedItems;

	// Original Client Rectangle (initialize in OnInitDialog())
	RECT	m_OrgClientRect;

	// Windows callback function
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Invoked when each item in the listbox should be redrawed
	INT_PTR OnDrawItem(LPDRAWITEMSTRUCT pCD);

	// Handle size changes
	void OnSize();

	//
	void MakeResult();

	//
	void CloseTypePopup();

	//
	void OnTypeSelChange();

	// Called when user clicks on listbox
	void OnChildClick(POINT &pt);

	// Generate list of selected MAX objects and add them to listbox as SListItem(s)
	void PopulateView(void);
};

//

#endif // __NDS_LexiExporter_SelectNodeDialog__