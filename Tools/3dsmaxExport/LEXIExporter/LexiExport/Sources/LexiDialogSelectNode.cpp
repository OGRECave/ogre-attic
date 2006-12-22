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
#include "LexiDialogSelectNode.h"

#include "..\Res\resource.h"

//

CSelectNodeDlg::CSelectNodeDlg(Window* pParent, CExportObject* pObj) : Dialog(IDD_DIALOG_SELECTMAXNODE, DlgProc, pParent)
{
	m_pObj = pObj;

	char temp[256];
	sprintf(temp, "Select %s Node", m_pObj->GetTypeName());
	m_sTitle = temp;

	m_iNode = 0;

	m_hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 64);

	std::vector<CExportObject::Desc> objlist;
	CExportObject::EnumObjects(objlist);

	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_SCENEROOT)));

	m_ImageListMap[NULL] = 1;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_UNKNOWN)));

	m_ImageListMap[LIGHT_CLASS_ID] = 2;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LIGHT)));

	m_ImageListMap[GEOMOBJECT_CLASS_ID] = 3;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_MODEL)));

//	m_ImageListMap[IGAME_SPLINE] = 4;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_UNKNOWN)));

	m_ImageListMap[CAMERA_CLASS_ID] = 5;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_CAMERA)));

	m_ImageListMap[HELPER_CLASS_ID] = 6;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_MODEL)));

//	m_ImageListMap[IGAME_BONE] = 7;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_UNKNOWN)));

//	m_ImageListMap[IGAME_IKCHAIN] = 8;
	ImageList_AddIcon(m_hImageList, ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_UNKNOWN)));
}

CSelectNodeDlg::~CSelectNodeDlg()
{
	ImageList_Destroy(m_hImageList);
}

//

INT_PTR CALLBACK CSelectNodeDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSelectNodeDlg* pThis = (CSelectNodeDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CSelectNodeDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 1;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					pThis->EndDialog(IDOK);
					return 0;

				case IDCANCEL:
					pThis->EndDialog(IDCANCEL);
					return 0;
			}
		}

		case WM_CLOSE:
		{
			pThis->EndDialog(IDCANCEL);
		}	break;

		case WM_NOTIFY:
		{
			const NMHDR* pHdr = (const NMHDR*)lParam;
			switch(pHdr->idFrom)
			{
				case IDC_TREE:
					if(pHdr->code == TVN_SELCHANGED)
					{
						pThis->UpdateStuff();
					}
					break;
			}
		}	break;
	}

	return 0;
}

//

void CSelectNodeDlg::OnInitDialog()
{
	CenterWindow();

	SetWindowText(m_sTitle.c_str());

	//

	Bind(IDC_TREE, m_Tree);
	m_Tree.SetImageList(m_hImageList, TVSIL_NORMAL);
	m_Tree.SetBkColor(::GetSysColor(COLOR_WINDOW));

	//

	m_hRoot = m_Tree.InsertItem("<SceneRoot>", 0, 0, TVI_ROOT, TVI_LAST);
	m_Tree.SetItemData(m_hRoot, NULL);

	m_Tree.SelectItem(NULL);

	for(unsigned int x = 0; x < CExporter::GetMax()->GetRootNode()->NumberOfChildren(); x++)
	{
		BuildTree(CExporter::GetMax()->GetRootNode()->GetChildNode(x), m_hRoot);
	}

	ShaveTree();

	m_Tree.Expand(m_hRoot, TVE_EXPAND);

	UpdateStuff();
}

//

void CSelectNodeDlg::UpdateStuff()
{
	bool bOK = false;

	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if(hItem)
	{
		INode* pNode = (INode*)m_Tree.GetItemData(hItem);
		if(pNode)
		{
			SClass_ID nClass = GetClassIDFromNode(pNode);
			if(m_pObj->SupportsClass(nClass))
			{
				m_iNode = pNode->GetHandle();
				bOK = true;
			}
		}
		else
		{
			m_iNode = 0;
			bOK = true;
		}
	}

	GetDlgItem(IDOK)->EnableWindow(bOK);
}

//

void CSelectNodeDlg::BuildTree(INode* pNode, HTREEITEM hTreeItem)
{
	if(pNode->IsNodeHidden()) return;

	unsigned int iID = pNode->GetHandle();
	const char* pszName = pNode->GetName();

	unsigned int iIconIndex = m_ImageListMap[GetClassIDFromNode(pNode)];

	HTREEITEM hItem = m_Tree.InsertItem(pszName, iIconIndex, iIconIndex, hTreeItem, TVI_SORT);
	m_Tree.SetItemData(hItem, (unsigned int)pNode);

	int iNumChildren = pNode->NumberOfChildren();
	for(int x = 0; x < iNumChildren; x++)
	{
		BuildTree(pNode->GetChildNode(x), hItem);
	}

	if(iID == m_iNode)
	{
		m_Tree.EnsureVisible(hItem);
		m_Tree.SelectItem(hItem);
	}
}

void CSelectNodeDlg::ShaveTree()
{
	while(ShaveTree(m_hRoot));
}

bool CSelectNodeDlg::ShaveTree(HTREEITEM hItem)
{
	if(m_Tree.ItemHasChildren(hItem))
	{
		HTREEITEM hChild = m_Tree.GetNextItem(hItem, TVGN_CHILD);
		while(hChild)
		{
			if(ShaveTree(hChild)) return true;
			hChild = m_Tree.GetNextItem(hChild, TVGN_NEXT);
		}
	}
	else
	{
		INode* pNode = (INode*)m_Tree.GetItemData(hItem);
		if(pNode)
		{
			SClass_ID nClass = GetClassIDFromNode(pNode);
			if(!m_pObj->SupportsClass(nClass))
			{
				m_Tree.DeleteItem(hItem);
				return true;
			}
		}
	}

	return false;
}

//

