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

#include "LexiStdAfx.h"
#include "LexiMaxExport.h"
#include "LexiDialogProperties.h"
#include "LexiDialogAddMultiple.h"

#include "..\Res\resource.h"

//

CAddMultipleDlg::CAddMultipleDlg(Window* pParent, CExporterPropertiesDlg *pOwner, CExportObject *pRootObject) : Dialog(IDD_DIALOG_ADDMULTIPLE, DlgProc, pParent)
{		
	m_pOwner=pOwner;
	m_pRootObject=pRootObject;
	m_pEditItem=NULL;
	memset(&m_OrgClientRect, 0, sizeof(RECT));	

	m_hImageList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 64);

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

CAddMultipleDlg::~CAddMultipleDlg()
{
	ImageList_Destroy(m_hImageList);
}

// If dialog returns IDOK, a list of export items can be retrieved from this function
vector<CAddMultipleDlg::SListItem*> CAddMultipleDlg::GetExportItems()
{
	return m_lSelectedItems;
}

//

INT_PTR CALLBACK CAddMultipleDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CAddMultipleDlg* pThis = (CAddMultipleDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CAddMultipleDlg*)lParam;
			Window::AddMapping(pThis, hWnd);
			pThis->OnInitDialog();
		}	return 1;

		case WM_SIZE:
		{
			pThis->OnSize();
		} break;

		case WM_PARENTNOTIFY:
			if(LOWORD(wParam)==WM_LBUTTONDOWN && pThis->m_TypePopup.m_hWnd==NULL)
			{
				POINT pt;
				lParam=GetMessagePos();
				pt.x=LOWORD(lParam);
				pt.y=HIWORD(lParam);
				pThis->OnChildClick(pt);
			}
			return 0;

		case WM_LBUTTONDOWN:
			if(pThis->m_TypePopup.m_hWnd!=NULL)
			{
				pThis->CloseTypePopup();
			}
			break;

		case WM_COMMAND:
		{
			if((HWND)lParam==pThis->m_TypePopup.m_hWnd && pThis->m_TypePopup.m_hWnd!=NULL && HIWORD(wParam)==LBN_SELCHANGE)
			{
				pThis->OnTypeSelChange();
				
			}

			switch(LOWORD(wParam))
			{
				case IDOK:
					pThis->MakeResult();
					pThis->EndDialog(IDOK);
					return 0;

				case IDCANCEL:
					pThis->EndDialog(IDCANCEL);
					return 0;
			}
		} break;

		case WM_CLOSE:
		{
			pThis->EndDialog(IDCANCEL);
		}	break;

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT) lParam;
			lpMeasureItem->itemHeight=20;
		}break;

		case WM_DRAWITEM:	
			return pThis->OnDrawItem((LPDRAWITEMSTRUCT)lParam);
	}

	return 0;
}

// Handle size changes
void CAddMultipleDlg::OnSize()
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
	HDWP hdwp=::BeginDeferWindowPos(3);

	RECT rTemp;
	Window *pItem;

	// Move OK button
	pItem=GetDlgItem(IDOK);
	pItem->GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, rTemp.left+iDeltaX, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move OK button
	pItem=GetDlgItem(IDCANCEL);
	pItem->GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, rTemp.left+iDeltaX, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Resize export tree
	m_ListBox.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ListBox.m_hWnd, NULL, 0, 0, (rTemp.right-rTemp.left)+iDeltaX, (rTemp.bottom-rTemp.top)+iDeltaY, SWP_NOZORDER|SWP_NOMOVE);

	::EndDeferWindowPos(hdwp);
	InvalidateRect(m_ListBox.m_hWnd, NULL, false);
	memcpy(&m_OrgClientRect, &rNewMain, sizeof(RECT));
}

//
void CAddMultipleDlg::CloseTypePopup()
{
	// Remove popup listbox and re-enable the list control
	m_TypePopup.DestroyWindow();
	m_TypePopup.m_hWnd=NULL;
	m_pEditItem=NULL;
	m_ListBox.EnableWindow(true);
}

//
void CAddMultipleDlg::OnTypeSelChange()
{
	// If we have an valid edit item pointer, we set the value
	if(m_pEditItem!=NULL)
	{
		int iTypeSel=m_TypePopup.GetCurSel();
		m_pEditItem->ExportType=m_TypePopup.GetItemData(iTypeSel);
	}

	CloseTypePopup();		
}

//
void CAddMultipleDlg::MakeResult()
{
	// Just to make sure
	m_lSelectedItems.clear();

	// Iterate all items
	int iCount=m_ListBox.GetCount();
	for(int i=0;i<iCount;i++)
	{
		// Get item and skip it, if flagged -1
		SListItem* pItem=(SListItem*)m_ListBox.GetItemDataPtr(i);
		if(pItem->ExportType==-1) continue;

		m_lSelectedItems.push_back(pItem);
	}
}

INT_PTR CAddMultipleDlg::OnDrawItem(LPDRAWITEMSTRUCT pCD)
{
	if(pCD==NULL) return 0;

/*	// Select right background and textcolor, based on selection state
	if(pCD->itemState&ODS_SELECTED)
	{
		::SetBkColor(pCD->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
		::SetTextColor(pCD->hDC,::GetSysColor(COLOR_HIGHLIGHTTEXT));
	} else
	{	
		::SetBkColor(pCD->hDC, GetCustSysColor(COLOR_WINDOW));
		::SetTextColor(pCD->hDC,::GetSysColor(COLOR_WINDOWTEXT));
	}*/
	::SetBkColor(pCD->hDC, GetCustSysColor(COLOR_WINDOW));
	::SetTextColor(pCD->hDC,::GetSysColor(COLOR_WINDOWTEXT));

	// Fill background (this is the Micro$oft way of doing it..)
	::ExtTextOut(pCD->hDC, 0, 0, ETO_OPAQUE, &pCD->rcItem, NULL, 0, NULL);

	// Get item data
	SListItem *pItem=(SListItem *)pCD->itemData;
	if(pItem==NULL) return 0;

	// Calculate MAX node name text rectangle
	RECT rLeft=pCD->rcItem;	
	rLeft.right/=2;
	int iHeight=rLeft.bottom-rLeft.top;

	// Calculate exporttype frame and button size
	RECT rRight=rLeft;
	rRight.left+=rLeft.right;	
	RECT rButton=rRight;
	rButton.right+=rLeft.right;
	rButton.left=rButton.right-iHeight;
	rRight.right+=rLeft.right-iHeight;
	
	// Draw MAX node icon
	ImageList_Draw(m_hImageList, m_ImageListMap[pItem->NodeClassID], pCD->hDC, rLeft.left+1, rLeft.top+2, ILD_TRANSPARENT);
	rLeft.left+=20;

	// Draw MAX node name
	::DrawText(pCD->hDC, pItem->sNodeName.c_str() , -1, &rLeft, DT_LEFT|DT_VCENTER|DT_SINGLELINE);

	// Draw "combobox" frame
	::DrawEdge(pCD->hDC, &rRight, EDGE_SUNKEN, BF_ADJUST|BF_RECT);
	rRight.left++;	// Just make some space

	// Draw selected export type string
	if(pItem->ExportType>=0)
		::DrawText(pCD->hDC, m_pOwner->m_lTypeCache[pItem->ExportType]->GetTypeName() , -1, &rRight, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
	else
		::DrawText(pCD->hDC, "Do not add" , -1, &rRight, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
	
	// Draw combobox pushbutton in right state
	if(pItem==m_pEditItem)
		::DrawFrameControl(pCD->hDC, &rButton, DFC_SCROLL, DFCS_SCROLLCOMBOBOX|DFCS_PUSHED);
	else
	{
		if(pItem->bCanEdit)
			::DrawFrameControl(pCD->hDC, &rButton, DFC_SCROLL, DFCS_SCROLLCOMBOBOX);
		else
			::DrawFrameControl(pCD->hDC, &rButton, DFC_SCROLL, DFCS_SCROLLCOMBOBOX|DFCS_INACTIVE);
	}
	return 0;
}

void CAddMultipleDlg::OnChildClick(POINT &pt)
{	
	RECT r;
	m_ListBox.GetClientRect(r);	
	POINT pLocal=pt;
	m_ListBox.ScreenToClient(pLocal);

	// If we did not click on "combobox" button, we dont care
	if(pLocal.x<(r.right-20)) return;

	// Find out which item was clicked
	int iIndex=m_ListBox.GetItemFromPoint(pLocal);
	if(HIWORD(iIndex)) return;	// Bail, if we was outside ListBox area
	
	// Get selected item data
	m_pEditItem=(SListItem*)m_ListBox.GetItemDataPtr(iIndex);
	if(!m_pEditItem->bCanEdit) 
	{
		m_pEditItem=NULL;	// Abort edit
		return;
	}

	m_ListBox.GetItemRect(iIndex, &r);
	int iWidth=(r.right/2)-20;	
	r.left=r.right/2;

	m_ListBox.ClientToScreen(r);
	ScreenToClient(r);	

	if(m_TypePopup.m_hWnd!=NULL)
	{
		CloseTypePopup();
	} else
	{	
		HWND hListView=::CreateWindow("LISTBOX", "TypePopup", WS_BORDER|WS_CHILD|LBS_NOTIFY, r.left, r.bottom, iWidth, 150, m_hWnd, (HMENU)0x80, CExporter::m_hInstance, NULL);
		m_TypePopup.Attach(hListView);
		SetWindowFont(hListView, GetWindowFont(m_hWnd), false);

		LONG lStyle=GetWindowLong(hListView, GWL_EXSTYLE);
		lStyle |= WS_EX_TRANSPARENT;
		SetWindowLong(hListView, GWL_EXSTYLE, lStyle);				

		int iItemIndex=m_TypePopup.AddString("Do not add");
		m_TypePopup.SetItemData(iItemIndex, -1);
		if(m_pEditItem->ExportType==-1) m_TypePopup.SetCurSel(iItemIndex);
		
		for(unsigned int x = 0; x < m_pOwner->m_lTypeCache.size(); x++) 
		{
			// Check if the export object type supports currently selected parent		
			if(	m_pOwner->m_lTypeCache[x]->SupportsParentType(m_pRootObject) &&
				m_pOwner->m_lTypeCache[x]->SupportsMAXNode(m_pEditItem->pNode))
			{
				int iIndex=m_TypePopup.AddString( m_pOwner->m_lTypeCache[x]->GetTypeName());
				m_TypePopup.SetItemData(iIndex, x);
				if(x==m_pEditItem->ExportType) m_TypePopup.SetCurSel(iIndex);
			}
		}	
		int iTotal=m_TypePopup.GetCount();
		int iHeight=m_TypePopup.GetItemHeight();

		m_ListBox.EnableWindow(false);
		m_TypePopup.SetWindowPos(NULL, 0, 0, iWidth, (iTotal*iHeight)+5, SWP_NOMOVE|SWP_NOZORDER);
		::ShowWindow(hListView, SW_SHOW);		
		GotoDlgCtrl(0x80);
	}
}

//

void CAddMultipleDlg::OnInitDialog()
{
	CenterWindow();	

	m_TypePopup.m_hWnd=NULL;
	GetClientRect(m_OrgClientRect);

	//
	Bind(IDC_LIST, m_ListBox);	
	LONG lStyle=GetWindowLong(m_ListBox.m_hWnd, GWL_EXSTYLE);
	lStyle &= ~WS_EX_NOPARENTNOTIFY;
	SetWindowLong(m_ListBox.m_hWnd, GWL_EXSTYLE, lStyle);

	PopulateView();
}

void CAddMultipleDlg::PopulateView()
{
	int iCount=m_ListBox.GetCount();
	for(int i=0;i<iCount;i++)
	{
		SListItem* pItem=(SListItem*)m_ListBox.GetItemDataPtr(i);
		if(pItem!=NULL) delete pItem;
	}
	m_ListBox.ResetContent();

	iCount=m_pOwner->m_pMax->GetSelNodeCount();	
	for(int i=0;i<iCount;i++)
	{
		INode* pNode=m_pOwner->m_pMax->GetSelNode(i);
		if(pNode==NULL || pNode->IsNodeHidden()) continue;

		SListItem* pItem=new SListItem;
		pItem->pNode=pNode;
		pItem->sNodeName=pNode->GetName();
		pItem->NodeClassID=GetClassIDFromNode(pNode);
		pItem->ExportType=-1;

		int iTypeCount=0;
		for(unsigned int x = 0; x < m_pOwner->m_lTypeCache.size(); x++) 
		{
			// Check if the export object type supports currently selected parent		
			if(	m_pOwner->m_lTypeCache[x]->SupportsParentType(m_pRootObject) &&
				m_pOwner->m_lTypeCache[x]->SupportsMAXNode(pNode))
			{
				pItem->ExportType=x;
				iTypeCount++;
			}
		}	
		pItem->bCanEdit=iTypeCount>0;	// We can only edit if type count>0

		m_ListBox.AddString((const char*)pItem);		
	}
}

