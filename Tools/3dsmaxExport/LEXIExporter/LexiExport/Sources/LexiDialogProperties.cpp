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

#include "LexiStdAfx.h"
#include "LexiDialogProperties.h"
#include "LexiDialogObjectProperties.h"

#include "..\Res\resource.h"

//

CExporterPropertiesDlg::CExporterPropertiesDlg(Window* pParent, Interface* pMax, IUtil* pMaxUtil, ExportObjectList* pConfig, CDDObject* pSettings) : Dialog(IDD_DIALOG_EXPORT, DlgProc, pParent)
{
	m_pMax = pMax;
	m_pMaxUtil = pMaxUtil;

	m_pConfig = pConfig;
	m_pSettings = pSettings;

	m_bChanges = false;

	//

	m_hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 64);

	std::vector<CExportObject::Desc> objlist;
	CExportObject::EnumObjects(objlist);	

	for(unsigned int x = 0; x < objlist.size(); x++)
	{
		m_ImageListMap[objlist[x].m_pszType] = x;
		ImageList_AddIcon(m_hImageList, objlist[x].m_hIcon);
	}
}

CExporterPropertiesDlg::~CExporterPropertiesDlg()
{
	ImageList_Destroy(m_hImageList);
}

//

INT_PTR CALLBACK CExporterPropertiesDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CExporterPropertiesDlg* pThis = (CExporterPropertiesDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CExporterPropertiesDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 1;

		case WM_CLOSE:
		{
			pThis->EndDialog(0);
		}	break;

		case WM_NOTIFY:
		{
			const NMHDR* pHdr = (const NMHDR*)lParam;
			switch(pHdr->idFrom)
			{
				case IDC_LIST:
					if(pHdr->code == LVN_ITEMCHANGED)
					{
						pThis->UpdateStuff();
					}
					else if(pHdr->code == NM_DBLCLK)
					{
						pThis->OnConfigButtonProperties();
					}
					else if(pHdr->code == LVN_KEYDOWN)
					{
						const NMLVKEYDOWN* pKey = (const NMLVKEYDOWN*)pHdr;
						if(pKey->wVKey == VK_DELETE)
						{
							pThis->OnConfigButtonRemove();
						}
					}
					break;
			}
		}	break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					break;

				case IDCANCEL:
				case IDC_CLOSE:
					pThis->EndDialog(0);
					break;

				case IDC_ADD:
					pThis->OnConfigButtonAdd();
					break;

				case IDC_ADDSELECTED:
					pThis->OnConfigButtonAddSelected();
					break;

				case IDC_REMOVE:
					pThis->OnConfigButtonRemove();
					break;

				case IDC_PROPERTIES:
					pThis->OnConfigButtonProperties();
					break;

				case IDC_SETTINGS:
					pThis->OnConfigButtonSettings();
					break;

				case IDC_EXPORT:
					pThis->OnConfigButtonExport();
					break;

				case IDC_EXPORTSELECTED:
					pThis->OnConfigButtonExportSelected();
					break;
			}
		}	return 1;
	}

	return 0;
}

//

void CExporterPropertiesDlg::OnInitDialog()
{
	Bind(IDC_LIST, m_ItemList);
	m_ItemList.SetImageList(m_hImageList, LVSIL_SMALL);

	Bind(IDC_ADD, m_ButtonAdd);
	Bind(IDC_ADDSELECTED, m_ButtonAddSelected);
	Bind(IDC_REMOVE, m_ButtonRemove);
	Bind(IDC_PROPERTIES, m_ButtonProperties);
	Bind(IDC_SETTINGS, m_ButtonSettings);
	Bind(IDC_EXPORTSELECTED, m_ButtonExportSelected);
	Bind(IDC_EXPORT, m_ButtonExport);

	//

	CenterWindow();

	//

	RECT rc;
	m_ItemList.GetClientRect(rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int w1 = 106;
	int w2 = 115;
	int w0 = w - w1 - w2 - ::GetSystemMetrics(SM_CXVSCROLL);

	m_ItemList.SetExtendedStyles(LVS_EX_FULLROWSELECT, 0xffffffff);

	m_ItemList.InsertColumn(0, "Name", LVCFMT_LEFT, w0);
	m_ItemList.InsertColumn(1, "Type", LVCFMT_LEFT, w1);
	m_ItemList.InsertColumn(2, "Node", LVCFMT_LEFT, w2);

	PopulateItemList();

	//

	UpdateStuff();
}

//

void CExporterPropertiesDlg::OnConfigButtonAdd()
{
	OnButtonAdd(&m_ButtonAdd, 0);
}

void CExporterPropertiesDlg::OnConfigButtonAddSelected()
{
	unsigned int iSelCount = m_pMax->GetSelNodeCount();
	if(iSelCount != 1) return;

	OnButtonAdd(&m_ButtonAddSelected, m_pMax->GetSelNode(0)->GetHandle());
}

void CExporterPropertiesDlg::OnButtonAdd(GDI::Button* pButton, unsigned int iSelectedID)
{
	std::vector<CExportObject::Desc> objlist;
	CExportObject::EnumObjects(objlist);	

	HMENU hMenu = CreatePopupMenu();
	for(unsigned int x = 0; x < objlist.size(); x++) AppendMenu(hMenu, MF_STRING, x + 1024, objlist[x].m_pszTypeName);

	RECT rc;
	pButton->GetWindowRect(rc);

	int iCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_BOTTOMALIGN, rc.left, rc.top, NULL, m_hWnd, NULL);
	DestroyMenu(hMenu);

	if(iCmd == 0) return;

	//

	CExportObject* pObj = CExportObject::Construct(objlist[iCmd - 1024].m_pszType);
	if(!pObj) return;

	char strTitle[256];
	sprintf(strTitle, "Add %s", objlist[iCmd - 1024].m_pszTypeName);

	if(!pObj->Edit(this, strTitle, iSelectedID))
	{
		pObj->Release();
		return;
	}

	unsigned int iIndex = m_pConfig->size();

	m_pConfig->push_back(pObj);
	AddToItemList(iIndex);

	//

	while(m_ItemList.GetSelectedCount())
	{
		int iSelIndex = m_ItemList.GetNextItem(-1, LVNI_SELECTED);
		m_ItemList.SetItemState(iSelIndex, 0, LVIS_SELECTED);
	}

	m_ItemList.SetItemState(iIndex, LVIS_SELECTED, LVIS_SELECTED);

	//

	m_bChanges = true;
	UpdateStuff();
}

void CExporterPropertiesDlg::OnConfigButtonRemove()
{
	int iNumSel = m_ItemList.GetSelectedCount();
	if(!iNumSel) return;

	char text[256];
	sprintf(text, "Remove selected item%s?", iNumSel > 1 ? "s" : "");

	int rs = MessageBox(text, NDS_EXPORTER_TITLE, MB_YESNO | MB_ICONWARNING);
	if(rs != IDYES) return;

	while(m_ItemList.GetSelectedCount())
	{
		int iIndex = m_ItemList.GetNextItem(-1, LVNI_SELECTED);
		m_ItemList.DeleteItem(iIndex);

		CExportObject* pObj = (*m_pConfig)[iIndex];
		pObj->Release();

		m_pConfig->erase(m_pConfig->begin() + iIndex);
	}

	m_bChanges = true;
	UpdateStuff();
}

void CExporterPropertiesDlg::OnConfigButtonProperties()
{
	int iIndex = m_ItemList.GetNextItem(-1, LVNI_SELECTED);
	if(iIndex == -1) return;

	CExportObject* pObj = m_pConfig->at(iIndex);

	char strTitle[256];
	sprintf(strTitle, "%s Properties", pObj->GetTypeName());

	if(!pObj->Edit(this, strTitle, 0)) return;

	UpdateItemInList(iIndex);

	m_bChanges = true;
	UpdateStuff();
}

void CExporterPropertiesDlg::OnConfigButtonSettings()
{
	MessageBox("Export settings", NDS_EXPORTER_TITLE, MB_ICONINFORMATION);
}

void CExporterPropertiesDlg::OnConfigButtonExport()
{
	EndDialog(100);
}

void CExporterPropertiesDlg::OnConfigButtonExportSelected()
{
	m_SelectionList.clear();

	while(m_ItemList.GetSelectedCount())
	{
		int iSelIndex = m_ItemList.GetNextItem(-1, LVNI_SELECTED);
		m_SelectionList.push_back(iSelIndex);
		m_ItemList.SetItemState(iSelIndex, 0, LVIS_SELECTED);
	}

	EndDialog(101);
}

//

void CExporterPropertiesDlg::UpdateStuff()
{
	int iNumSel = m_ItemList.GetSelectedCount();
	m_ButtonRemove.EnableWindow(iNumSel ? true : false);
	m_ButtonProperties.EnableWindow(iNumSel == 1 ? true : false);
	m_ButtonExportSelected.EnableWindow(iNumSel ? true : false);

	int iNum = m_ItemList.GetItemCount();
	m_ButtonExport.EnableWindow(iNum ? true : false);

	bool bSelected = m_pMax->GetSelNodeCount() == 1 ? true : false;
	m_ButtonAddSelected.EnableWindow(bSelected);
}

//

void CExporterPropertiesDlg::PopulateItemList()
{
	m_ItemList.DeleteAllItems();

	unsigned int iCount = m_pConfig->size();
	for(unsigned int x = 0; x < iCount; x++) AddToItemList(x);
}

void CExporterPropertiesDlg::AddToItemList(unsigned int iConfigIndex)
{
	m_ItemList.InsertItem(m_ItemList.GetItemCount(), m_pConfig->at(iConfigIndex)->GetName());
	UpdateItemInList(iConfigIndex);
}

void CExporterPropertiesDlg::UpdateItemInList(int iIndex)
{
	const CExportObject* pObj = m_pConfig->at(iIndex);

	m_ItemList.SetItemText(iIndex, 0, pObj->GetName());
	m_ItemList.SetItemText(iIndex, 1, pObj->GetTypeName());
	m_ItemList.SetItemText(iIndex, 2, GetNameFromID(pObj->GetID()));
	m_ItemList.SetItemImage(iIndex, m_ImageListMap[pObj->GetType()]);
}

//

void CExporterPropertiesDlg::GetLastSelection(std::vector<unsigned int>& selectionlist) const
{
	unsigned int iCount = m_SelectionList.size();
	for(unsigned int x = 0; x < iCount; x++)
	{
		selectionlist.push_back(m_SelectionList[x]);
	}
}

//

