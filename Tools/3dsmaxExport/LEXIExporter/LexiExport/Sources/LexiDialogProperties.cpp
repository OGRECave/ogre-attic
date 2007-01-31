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
#include "LexiMaxExport.h"
#include "LexiDialogProperties.h"
#include "LexiDialogObjectProperties.h"

#include "..\Res\resource.h"

//

CExporterPropertiesDlg::CExporterPropertiesDlg(Window* pParent, Interface* pMax, IUtil* pMaxUtil, CExportObjectRoot *pRoot) : Dialog(IDD_DIALOG_EXPORT, DlgProc, pParent)
{
	m_pMax = pMax;
	m_pMaxUtil = pMaxUtil;

	m_pRoot=pRoot;	
	m_pCurrent=m_pRoot;

	m_bChanges = false;
	m_pCurrentEditWindow=0;
	m_pLastEditObject=0;

	// Create icon image list
	m_hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 64);

	// Enumerate all supported export types
	std::vector<CExportObject::Desc> objlist;
	CExportObject::EnumObjects(objlist);	

	for(unsigned int x = 0; x < objlist.size(); x++)
	{
		m_ImageListMap[objlist[x].m_pszType] = x;
		ImageList_AddIcon(m_hImageList, objlist[x].m_hIcon);

		// Construct a type stub. This is used for testing parent/child compability
		CDDObject *pTemp=new CDDObject();
		pTemp->SetString("Type", objlist[x].m_pszType);
		CExportObject *pStub=CExportObject::Construct(pTemp);
		if(pStub!=NULL) m_lTypeCache.push_back(pStub);
		pTemp->Release();
	}

	memset(&m_OrgClientRect, 0, sizeof(RECT));	
}

CExporterPropertiesDlg::~CExporterPropertiesDlg()
{
	ImageList_Destroy(m_hImageList);

	try 
	{
		for(unsigned int i=0;i<m_lTypeCache.size();i++)
			m_lTypeCache[i]->Release();
		m_lTypeCache.clear();
	} catch(...)
	{
	}
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

		case WM_SIZE:
		{
			pThis->OnSize();
		} break;

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *pInfo=(MINMAXINFO*)lParam;
			pInfo->ptMinTrackSize.x=680;
			pInfo->ptMinTrackSize.y=300;
		} break;
	
		case WM_CLOSE:
		{
			if(!pThis->OnValidate()) return 0;
			pThis->EndDialog(0);
		}	break;

		case WM_NOTIFY_MESSAGE_ID:
			pThis->OnDataChange();
			break;

		case WM_NOTIFY:
		{
			const NMHDR* pHdr = (const NMHDR*)lParam;
			switch(pHdr->idFrom)
			{
				case IDC_EXPORTTREE:	
					if(pHdr->code == TVN_SELCHANGED)
					{
						pThis->OnSelChange();
					}
					else if(pHdr->code == NM_CLICK)
					{
						TVHITTESTINFO ht = {0};

						GetCursorPos(&ht.pt);
						pThis->m_ExportTree.ScreenToClient(ht.pt);

						HTREEITEM hItem=pThis->m_ExportTree.HitTest(&ht);         
						if(hItem!=NULL && (TVHT_ONITEMSTATEICON & ht.flags))
						{
							pThis->OnCheckChange(hItem);
						}						
					}
					else if(pHdr->code == NM_RCLICK)
					{
						pThis->OnRClickTree();
					}
					else if(pHdr->code == TVN_KEYDOWN)
					{
						const NMTVKEYDOWN* pKey = (const NMTVKEYDOWN*)pHdr;
						if(pKey->wVKey == VK_DELETE)
						{
							pThis->OnConfigButtonRemove();
						} else if(pKey->wVKey==VK_SPACE)
						{
							pThis->OnCheckChange(pThis->m_ExportTree.GetSelectedItem());
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
					if(!pThis->OnValidate()) return 0;
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

				case IDC_LOADSELECTION:
					pThis->OnLoadSelection();
					break;

				case IDC_VIEWLOG:
					pThis->OnViewLog();
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
	Bind(IDC_EXPORTTREE, m_ExportTree);	
	m_ExportTree.SetImageList(m_hImageList, TVSIL_NORMAL);
	m_ExportTree.SetBkColor(::GetSysColor(COLOR_WINDOW));

	Bind(IDC_ADD, m_ButtonAdd);
	Bind(IDC_ADDSELECTED, m_ButtonAddSelected);
	Bind(IDC_REMOVE, m_ButtonRemove);
	Bind(IDC_SETTINGS, m_ButtonSettings);
	Bind(IDC_EXPORTSELECTED, m_ButtonExportSelected);
	Bind(IDC_EXPORT, m_ButtonExport);
	Bind(IDC_LOADSELECTION, m_ButtonLoadSelection);

	GetClientRect(m_OrgClientRect);

	//
	CenterWindow();	
	PopulateExportTree();

	//
	OnSelChange();
}

//
bool CExporterPropertiesDlg::OnValidate()
{
	if(m_pCurrent && m_pCurrentEditWindow)
	{
		m_pCurrent->CloseEditWindow();
		m_pCurrentEditWindow=NULL;
		m_pLastEditObject=0;
	}
	return true;
}

//
void CExporterPropertiesDlg::OnSize()
{
	// Check if have initialized dialog
	if(m_OrgClientRect.right==0) return;

	RECT rNewMain;
	GetClientRect(rNewMain);
	int iDeltaY=rNewMain.bottom-m_OrgClientRect.bottom;
	int iDeltaX=rNewMain.right-m_OrgClientRect.right;

	// Get original client rect in screen coordinates
	RECT rScreenRect;
	memcpy(&rScreenRect, &m_OrgClientRect, sizeof(RECT));
	ClientToScreen(rScreenRect);	

	// Begin resize/move of all the controls
	HDWP hdwp=::BeginDeferWindowPos(8);

	RECT rTemp;
	Window *pItem;
	// Move Add button
	m_ButtonAdd.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonAdd.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move AddSelected button
	m_ButtonAddSelected.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonAddSelected.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move Remove button
	m_ButtonRemove.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonRemove.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move LoadSelection button
	m_ButtonLoadSelection.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonLoadSelection.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move Settings button
	m_ButtonSettings.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonSettings.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move ExportSelected button
	m_ButtonExportSelected.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonExportSelected.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move Export button
	m_ButtonExport.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ButtonExport.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move Export button
	pItem=GetDlgItem(IDC_CLOSE);
	pItem->GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move Export button
	pItem=GetDlgItem(IDC_VIEWLOG);
	pItem->GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Resize export tree
	m_ExportTree.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ExportTree.m_hWnd, NULL, 0, 0, (rTemp.right-rTemp.left), (rTemp.bottom-rTemp.top)+iDeltaY, SWP_NOZORDER|SWP_NOMOVE);

	// Resize object property frame
	pItem=GetDlgItem(IDC_LIST_GROUP);
	pItem->GetWindowRect(rTemp);
	hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, 0, 0, (rTemp.right-rTemp.left), (rTemp.bottom-rTemp.top)+iDeltaY, SWP_NOZORDER|SWP_NOMOVE);
	
	::EndDeferWindowPos(hdwp);
	memcpy(&m_OrgClientRect, &rNewMain, sizeof(RECT));
	UpdateEditWindow();
}

void CExporterPropertiesDlg::UpdateEditWindow()
{
	if(m_pCurrentEditWindow==NULL) return;

	RECT rTemp;
	Window *pItem;
	pItem=GetDlgItem(IDC_LIST_GROUP);
	pItem->GetWindowRect(rTemp);	
	ScreenToClient(rTemp);
		
	m_pCurrentEditWindow->SetWindowPos(NULL, rTemp.right+10, rTemp.top, m_OrgClientRect.right-(rTemp.right+rTemp.left+10), rTemp.bottom, SWP_NOZORDER);
}

void CExporterPropertiesDlg::OnConfigButtonAdd()
{
	OnButtonAdd(&m_ButtonAdd, 0xFFFFFFFF);
}

void CExporterPropertiesDlg::OnConfigButtonAddSelected()
{
	unsigned int iSelCount = m_pMax->GetSelNodeCount();
	if(iSelCount != 1) return;

	OnButtonAdd(&m_ButtonAddSelected, m_pMax->GetSelNode(0)->GetHandle());
}

void CExporterPropertiesDlg::OnButtonAdd(GDI::Button* pButton, unsigned int iSelectedID)
{	
	// Create pop menu with all allowed types in
	HMENU hMenu = CreatePopupMenu();	

	for(unsigned int x = 0; x < m_lTypeCache.size(); x++) 
	{
		// Check if the export object type supports currently selected parent		
		if(m_lTypeCache[x]->SupportsParentType(m_pCurrent))
		{
			AppendMenu(hMenu, MF_STRING, x + 1024, m_lTypeCache[x]->GetTypeName());
		}
	}

	RECT rc;
	pButton->GetWindowRect(rc);

	int iCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_BOTTOMALIGN, rc.left, rc.top, NULL, m_hWnd, NULL);
	DestroyMenu(hMenu);

	// On abort, just bail out
	if(iCmd == 0) return;

	// Get currently selected tree control item (this would be parent item)
	HTREEITEM hParent=m_ExportTree.GetSelectedItem();
	if(hParent==NULL) hParent=m_hRootItem;

	InternalCreate(iCmd-1024, hParent, iSelectedID);
}

void CExporterPropertiesDlg::InternalCreate(int iTypeIndex, HTREEITEM hParent, unsigned int iSelectedID)
{
	// Create config object for ExportObject
	CDDObject *pNewConfig=new CDDObject();
	pNewConfig->SetString("Type", m_lTypeCache[iTypeIndex]->GetType());

	// Construct new ExportObject
	CExportObject* pObj = CExportObject::Construct(pNewConfig);
	if(!pObj) 
	{
		MessageBox("Failed to create new ExportObject", NDS_EXPORTER_TITLE, MB_ICONERROR);
		return;
	}

	// Setup parent
	pObj->SetParent(m_pCurrent);
	// Setup selected MAX Node
	pObj->SetMAXNodeID(iSelectedID);

	// Initial create function
	if(!pObj->OnCreate(this))
	{
		// Object would not create, release
		pObj->Release();
		return;
	}

	// Create new treecontrol item
	int iIcon=m_ImageListMap[pObj->GetType()];
	HTREEITEM hItem=m_ExportTree.InsertItem(pObj->GetName(), iIcon, iIcon, hParent);
	m_ExportTree.SetItemData(hItem, (DWORD_PTR)pObj);
	m_ExportTree.SetCheck(hItem, pObj->GetEnabled());
	m_ExportTree.SelectItem(hItem);
	m_ExportTree.EnsureVisible(hItem);

	//
	m_bChanges = true;
}

void CExporterPropertiesDlg::InternalRemove(HTREEITEM hItem)
{
	/*	char text[256];
	sprintf(text, "Remove selected item%s?", iNumSel > 1 ? "s" : "");

	int rs = MessageBox(text, NDS_EXPORTER_TITLE, MB_YESNO | MB_ICONWARNING);
	if(rs != IDYES) return;
*/

	CExportObject *pSel=(CExportObject*)m_ExportTree.GetItemData(hItem);
	HTREEITEM hNextSel=m_ExportTree.GetPrevSiblingItem(hItem);
	if(hNextSel==NULL) 
	{
		hNextSel=m_ExportTree.GetParentItem(hItem);
		if(hNextSel==NULL) hNextSel=m_hRootItem;
	}

	m_ExportTree.SelectItem(hNextSel);

	m_ExportTree.DeleteItem(hItem);
	pSel->Release();	
	m_bChanges = true;
}

void CExporterPropertiesDlg::GetSubItems(HTREEITEM hParent, std::vector<HTREEITEM> &lList)
{
	HTREEITEM hChild=m_ExportTree.GetChildItem(hParent);
	if(hChild==NULL) return;

	while(hChild!=NULL)
	{
		lList.push_back(hChild);
		GetSubItems(hChild, lList);	// Recurse
		hChild=m_ExportTree.GetNextSiblingItem(hChild);
	}
}

void CExporterPropertiesDlg::OnLoadSelection()
{
	// Create pop menu with all allowed types in
	HMENU hMenu = CreatePopupMenu();	

	for(unsigned int x = 0; x < m_lTypeCache.size(); x++) 
	{
		if(stricmp(m_lTypeCache[x]->GetType(), "root")==0) continue;

		std::string sText="All of type '";
		sText+=m_lTypeCache[x]->GetTypeName();
		sText+="'";

		AppendMenu(hMenu, MF_STRING, x + 1024, sText.c_str());
	}

	AppendMenu(hMenu, MF_STRING, 100, "All");	
	AppendMenu(hMenu, MF_STRING, 101, "None");

	RECT rc;
	m_ButtonLoadSelection.GetWindowRect(rc);

	int iCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_BOTTOMALIGN, rc.left, rc.top, NULL, m_hWnd, NULL);
	DestroyMenu(hMenu);

	std::vector<HTREEITEM> lAllItems;
	GetSubItems(TVI_ROOT, lAllItems);
	if(iCmd>=1024)
	{
		const char *pszType=m_lTypeCache[iCmd-1024]->GetType();

		for(unsigned i=0;i<lAllItems.size();i++)
		{
			CExportObject *pObj=(CExportObject*)m_ExportTree.GetItemData(lAllItems[i]);
			if(pObj==NULL || pObj==m_pRoot) continue;
			bool bCheck=stricmp(pObj->GetType(), pszType)==0;
			m_ExportTree.SetCheck(lAllItems[i], bCheck);
			pObj->SetEnabled(bCheck);
		}
	} else
	{
		switch(iCmd)
		{
			case 100:				
				for(unsigned i=0;i<lAllItems.size();i++)
				{
					CExportObject *pObj=(CExportObject*)m_ExportTree.GetItemData(lAllItems[i]);
					if(pObj==NULL || pObj==m_pRoot) continue;
					m_ExportTree.SetCheck(lAllItems[i], true);
					pObj->SetEnabled(true);
				}
				break;
			case 101:
				for(unsigned i=0;i<lAllItems.size();i++)
				{
					CExportObject *pObj=(CExportObject*)m_ExportTree.GetItemData(lAllItems[i]);
					if(pObj==NULL || pObj==m_pRoot) continue;
					m_ExportTree.SetCheck(lAllItems[i], false);
					pObj->SetEnabled(false);
				}
				break;
			// On abort, just bail out
			default:
				return;
		}
	}	
}

void CExporterPropertiesDlg::OnConfigButtonRemove()
{
	HTREEITEM hSel=m_ExportTree.GetSelectedItem();
	if(hSel==NULL || hSel==m_hRootItem) return;
	
	InternalRemove(hSel);	
}

void CExporterPropertiesDlg::OnConfigButtonSettings()
{
	MessageBox("Export settings", NDS_EXPORTER_TITLE, MB_ICONINFORMATION);
}

void CExporterPropertiesDlg::OnViewLog()
{
	CExporter::Get()->ShowLog();
}

void CExporterPropertiesDlg::OnConfigButtonExport()
{
	if(!OnValidate()) return;
	EndDialog(100);
}

void CExporterPropertiesDlg::OnConfigButtonExportSelected()
{
/*	// Check if have any top-level items selected
	bool bEnabledChildren=false;
	HTREEITEM hChild=m_ExportTree.GetChildItem(m_hRootItem);	
	while(hChild!=NULL)
	{
		if(m_ExportTree.GetCheck(hChild))
		{
			bEnabledChildren=true;
			break;
		}
		hChild=m_ExportTree.GetNextSiblingItem(hChild);		
	}
	if(*/
	if(!OnValidate()) return;
	EndDialog(101);
}

void CExporterPropertiesDlg::OnCheckChange(HTREEITEM hItem)
{
	m_pCurrent=(CExportObject*)m_ExportTree.GetItemData(hItem);
	if(m_pCurrent)
	{
		m_pCurrent->SetEnabled(!m_ExportTree.GetCheck(hItem));	
	}
}

//
void CExporterPropertiesDlg::OnSelChange()
{
	HTREEITEM hSel=m_ExportTree.GetSelectedItem();
	if(hSel==NULL) hSel=m_hRootItem;	
	
	// Check for root item
	if(hSel!=m_hRootItem)
	{
		m_ButtonRemove.EnableWindow(true);	// All objects other than root can be removed
		m_pCurrent=(CExportObject*)m_ExportTree.GetItemData(hSel);
		if(!m_pCurrent) m_pCurrent=m_pRoot;
	} else
	{
		m_ButtonRemove.EnableWindow(false);	// Root cannot be removed
		m_pCurrent=m_pRoot;
	}

	// Retrieve edit window for current selected control
	GDI::Window *pNew=m_pCurrent->GetEditWindow(this);
	if(pNew!=m_pCurrentEditWindow)
	{			
		// If another edit window was open, close it using the ExportObject
		// it came from
		if(m_pLastEditObject!=NULL && m_pCurrentEditWindow!=NULL) m_pLastEditObject->CloseEditWindow();		

		// Store the new edit window
		m_pCurrentEditWindow=pNew;

		if(m_pCurrentEditWindow!=NULL)
		{
			// Save the last ExportObject we have gotten Edit window from
			m_pLastEditObject=m_pCurrent;

			// Move edit window into place
			UpdateEditWindow();

			// Make sure the edit window is visible
			m_pCurrentEditWindow->ShowWindow(SW_SHOW);
		}		
	}

	INode *pNode=0;
	if(m_pMax->GetSelNodeCount() == 1)
	{
		pNode=m_pMax->GetSelNode(0);		
	}

	// Check if have any available types	
	bool bCanAdd=false, bCanAddSel=false;
	for(unsigned i=0;i<m_lTypeCache.size();i++)
	{
		if(m_lTypeCache[i]->SupportsParentType(m_pCurrent))
		{
			bCanAdd=true;
			if(m_pMax->GetSelNodeCount() == 1 && m_lTypeCache[i]->SupportsMAXNode(pNode))
				bCanAddSel=true;						
		}
	}
	m_ButtonAdd.EnableWindow(bCanAdd);
	m_ButtonAddSelected.EnableWindow(bCanAddSel);

	m_ButtonExport.EnableWindow(m_pRoot->HasChildren());
	m_ButtonExportSelected.EnableWindow(m_pRoot->HasChildren());
}

//
void CExporterPropertiesDlg::OnRClickTree()
{		
	POINT ptMouse;
	
	// Get cursor position
	if(!GetCursorPos(&ptMouse)) return;

	// Create hittest structure with local coordinates
	TVHITTESTINFO hitInfo;
	hitInfo.pt=ptMouse;
	m_ExportTree.ScreenToClient(hitInfo.pt);

	// Check if we hit an item
	HTREEITEM hSelItem=m_ExportTree.HitTest(&hitInfo);
	if(hSelItem==NULL) return;	
	
	m_pCurrent=(CExportObject*)m_ExportTree.GetItemData(hSelItem);
	if(!m_pCurrent) m_pCurrent=m_pRoot;
	
	// Create pop menu with all allowed types in
	HMENU hMenu = CreatePopupMenu();

	// If we add any "Add Xx" items, we must add a separator
	bool bHasSeparator=false;
	std::string sText;

	// We do not want to remove root item
	if(m_pCurrent!=m_pRoot)
	{
		// Add option to remove currently selected item
		sText="Remove ";
		sText+=m_pCurrent->GetName();
		AppendMenu(hMenu, MF_STRING, 512, sText.c_str());	
	} else
	{
		bHasSeparator=true;	// This will omit the separator
	}
	
	for(unsigned int x = 0; x < m_lTypeCache.size(); x++) 
	{
		// Check if the export object type supports currently selected parent		
		if(m_lTypeCache[x]->SupportsParentType(m_pCurrent))
		{
			// Check if an separator should be inserted
			if(!bHasSeparator)
			{
				AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
				bHasSeparator=true;
			}

			// Add option to create this type
			sText="Add ";
			sText+=m_lTypeCache[x]->GetTypeName();
			AppendMenu(hMenu, MF_STRING, x + 1024, sText.c_str());
		}
	}

	// Create menu
	int iCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_BOTTOMALIGN, ptMouse.x, ptMouse.y, NULL, m_hWnd, NULL);
	DestroyMenu(hMenu);

	// On abort, just bail out
	if(iCmd == 0) return;
	
	if(iCmd==512)
	{
		// Remove current item
		InternalRemove(hSelItem);
	} else
	{
		// Create new sub item
		InternalCreate(iCmd-1024, hSelItem, 0xFFFFFFFF);
	}
}

//
void CExporterPropertiesDlg::OnDataChange()
{
	std::vector<HTREEITEM> lAllItems;
	GetSubItems(TVI_ROOT, lAllItems);
	for(unsigned i=0;i<lAllItems.size();i++)
	{
		CExportObject *pInstance=(CExportObject*)m_ExportTree.GetItemData(lAllItems[i]);
		if(pInstance==NULL) return;

		m_ExportTree.SetItemText(lAllItems[i], pInstance->GetName());
	}

	m_bChanges = true;
}

//
void CExporterPropertiesDlg::PopulateExportTree()
{
	m_ExportTree.DeleteAllItems();		
	AddObjectToTree(m_pRoot, TVI_ROOT);
	m_ExportTree.Expand(m_hRootItem, TVE_EXPAND);
}

//
void CExporterPropertiesDlg::AddObjectToTree(CExportObject *pObject, HTREEITEM hParent)
{
	int iIcon=m_ImageListMap[pObject->GetType()];
	HTREEITEM hItem=m_ExportTree.InsertItem(pObject->GetName(), iIcon, iIcon, hParent);	
	m_ExportTree.SetItemData(hItem, (DWORD_PTR)pObject);

	if(hParent==TVI_ROOT) 
	{
		m_hRootItem=hItem;
		m_ExportTree.SetCheck(hItem, true);
	} else
	{
		m_ExportTree.SetCheck(hItem, pObject->GetEnabled());
	}
	
	std::vector<CExportObject*> lList=pObject->GetChildren();
	for(unsigned i=0;i<lList.size();i++)
	{
		AddObjectToTree(lList[i], hItem);
	}
}