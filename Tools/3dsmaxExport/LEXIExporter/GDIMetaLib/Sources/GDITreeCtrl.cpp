/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

/////////////////////////////////////////////////
//
//  Max Exporter - GDI TreeCtrl
//
/////////////////////////////////////////////////

#include "stdafx.h"

//

namespace GDI {

//

TreeCtrl::TreeCtrl()
{
}

TreeCtrl::~TreeCtrl()
{
}

//

COLORREF TreeCtrl::GetBkColor() const
{
	return (COLORREF)SendMessage(TVM_GETBKCOLOR, 0, 0);
}

//

bool TreeCtrl::GetCheck(HTREEITEM hItem) const
{
	TVITEM item;
	item.mask = TVIF_HANDLE | TVIF_STATE;
	item.hItem = hItem;
	item.stateMask = TVIS_STATEIMAGEMASK;

	SendMessage(TVM_GETITEM, 0, (LPARAM)&item);

	return ((BOOL)(item.state >> 12) - 1) ? true : false;
}

//

HTREEITEM TreeCtrl::GetChildItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);
}

//

unsigned int TreeCtrl::GetCount() const
{
	return (unsigned int)SendMessage(TVM_GETCOUNT, 0, 0);
}

//

HTREEITEM TreeCtrl::GetDropHilightItem() const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
}

//

/*
Edit* TreeCtrl::GetEditControl() const
{
	return (Edit*)Window::FromHandle((HWND)SendMessage(TVM_GETEDITCONTROL, 0, 0));
}
*/

//

HTREEITEM TreeCtrl::GetFirstVisibleItem() const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0);
}

//

/*
ImageList* TreeCtrl::GetImageList(unsigned int iImageList) const
{
	return CImageList::FromHandle((HIMAGELIST)SendMessage(TVM_GETIMAGELIST, (UINT)nImageList, 0));
}
*/

//

unsigned int TreeCtrl::GetIndent() const
{
	return (unsigned int)SendMessage(TVM_GETINDENT, 0, 0);
}

//

COLORREF TreeCtrl::GetInsertMarkColor() const
{
	return (COLORREF)SendMessage(TVM_GETINSERTMARKCOLOR, 0, 0);
}

//

bool TreeCtrl::GetItem(TVITEM* pItem) const
{
	return SendMessage(TVM_GETITEM, 0, (LPARAM)pItem) ? true : false;
}

//

DWORD_PTR TreeCtrl::GetItemData(HTREEITEM hItem) const
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_PARAM;

	SendMessage(TVM_GETITEM, 0, (LPARAM)&item);

	return item.lParam;
}

//

int TreeCtrl::GetItemHeight() const
{
	return (int)SendMessage(TVM_GETITEMHEIGHT, 0, 0);
}

//

bool TreeCtrl::GetItemImage(HTREEITEM hItem, int& iImage, int& iSelectedImage) const
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	bool bOK = SendMessage(TVM_GETITEM, 0, (LPARAM)&item) ? true : false;
	if(bOK)
	{
		iImage = item.iImage;
		iSelectedImage = item.iSelectedImage;
	}

	return bOK;
}

//

bool TreeCtrl::GetItemRect(HTREEITEM hItem, RECT* pRect, bool bTextOnly) const
{
	*(HTREEITEM*)pRect = hItem;
	return SendMessage(TVM_GETITEMRECT, (WPARAM)bTextOnly, (LPARAM)pRect) ? true : false;
}

//

unsigned int TreeCtrl::GetItemState(HTREEITEM hItem, unsigned int nStateMask) const
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_STATE;
	item.stateMask = nStateMask;
	item.state = 0;

	SendMessage(TVM_GETITEM, 0, (LPARAM)&item);

	return item.state;
}

//

std::string TreeCtrl::GetItemText(HTREEITEM hItem) const
{
	char text[4096];

	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_TEXT;
	item.pszText = text;
	item.cchTextMax = 4096;

	SendMessage(TVM_GETITEM, 0, (LPARAM)&item);

	return text;
}

//

COLORREF TreeCtrl::GetLineColor() const
{
	return (COLORREF)SendMessage(TVM_GETLINECOLOR, 0, 0);
}

//

HTREEITEM TreeCtrl::GetNextItem(HTREEITEM hItem, unsigned int nCode) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, nCode, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetNextSiblingItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetNextVisibleItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetParentItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetPrevSiblingItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetPrevVisibleItem(HTREEITEM hItem) const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_PREVIOUSVISIBLE, (LPARAM)hItem);
}

//

HTREEITEM TreeCtrl::GetRootItem() const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_ROOT, 0);
}

//

unsigned int TreeCtrl::GetScrollTime() const
{
	return (unsigned int)SendMessage(TVM_GETSCROLLTIME, 0, 0);
}

//

HTREEITEM TreeCtrl::GetSelectedItem() const
{
	return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_CARET, 0);
}

//

COLORREF TreeCtrl::GetTextColor() const
{
	return (COLORREF)SendMessage(TVM_GETTEXTCOLOR, 0, 0);
}

//

unsigned int TreeCtrl::GetVisibleCount() const
{
	return (unsigned int)SendMessage(TVM_GETVISIBLECOUNT, 0, 0);
}

//

bool TreeCtrl::ItemHasChildren(HTREEITEM hItem) const
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_CHILDREN;

	SendMessage(TVM_GETITEM, 0, (LPARAM)&item);

	return item.cChildren ? true : false;
}

//

COLORREF TreeCtrl::SetBkColor(COLORREF clr)
{
	return (COLORREF)SendMessage(TVM_SETBKCOLOR, 0, (LPARAM)clr);
}

//

bool TreeCtrl::SetCheck(HTREEITEM hItem, bool bCheck)
{
	TVITEM item;
	item.mask = TVIF_HANDLE | TVIF_STATE;
	item.hItem = hItem;
	item.stateMask = TVIS_STATEIMAGEMASK;
	item.state = INDEXTOSTATEIMAGEMASK((bCheck ? 2 : 1));

	return SendMessage(TVM_SETITEM, 0, (LPARAM)&item) ? true : false;
}

//

HIMAGELIST TreeCtrl::SetImageList(HIMAGELIST hImageList, int iImageListType)
{
	return (HIMAGELIST)SendMessage(TVM_SETIMAGELIST, (UINT)iImageListType, (LPARAM)hImageList);
}

//

void TreeCtrl::SetIndent(unsigned int iIndent)
{
	SendMessage(TVM_SETINDENT, iIndent, 0);
}

//

bool TreeCtrl::SetInsertMark(HTREEITEM hItem, bool bAfter)
{
	return SendMessage(TVM_SETINSERTMARK, bAfter, (LPARAM)hItem) ? true : false;
}

//

COLORREF TreeCtrl::SetInsertMarkColor(COLORREF clrNew)
{
	return (COLORREF)SendMessage(TVM_SETINSERTMARKCOLOR, 0, (LPARAM)clrNew);
}

//

bool TreeCtrl::SetItem(TVITEM* pItem)
{
	return SendMessage(TVM_SETITEM, 0, (LPARAM)pItem) ? true : false;
}

bool TreeCtrl::SetItem(HTREEITEM hItem, unsigned int nMask, const char* pszItem, int iImage, int iSelectedImage, unsigned int nState, unsigned int nStateMask, LPARAM lParam)
{
	TVITEM item;
	item.hItem = hItem;
	item.mask = nMask;
	item.pszText = (char*)pszItem;
	item.iImage = iImage;
	item.iSelectedImage = iSelectedImage;
	item.state = nState;
	item.stateMask = nStateMask;
	item.lParam = lParam;

	return SendMessage(TVM_SETITEM, 0, (LPARAM)&item) ? true : false;
}

//

bool TreeCtrl::SetItemData(HTREEITEM hItem, DWORD_PTR dwData)
{
	return SetItem(hItem, TVIF_PARAM, NULL, 0, 0, 0, 0, (LPARAM)dwData);
}

//

int TreeCtrl::SetItemHeight(int iHeight)
{
	return (int)SendMessage(TVM_SETITEMHEIGHT, iHeight, 0);
}

//

bool TreeCtrl::SetItemImage(HTREEITEM hItem, int iImage, int iSelectedImage)
{
	return SetItem(hItem, TVIF_IMAGE | TVIF_SELECTEDIMAGE, NULL, iImage, iSelectedImage, 0, 0, NULL);
}

//

bool TreeCtrl::SetItemState(HTREEITEM hItem, unsigned int nState, unsigned int nStateMask)
{
	return SetItem(hItem, TVIF_STATE, NULL, 0, 0, nState, nStateMask, NULL);
}

//

bool TreeCtrl::SetItemText(HTREEITEM hItem, const char* pszItem)
{
	return SetItem(hItem, TVIF_TEXT, pszItem, 0, 0, 0, 0, NULL);
}

//

COLORREF TreeCtrl::SetLineColor(COLORREF clrNew)
{
	return (COLORREF)SendMessage(TVM_SETLINECOLOR, 0, (LPARAM)clrNew);
}

//

unsigned int TreeCtrl::SetScrollTime(unsigned int iScrollTime)
{
	return (unsigned int)SendMessage(TVM_SETSCROLLTIME, iScrollTime, 0);
}

//

COLORREF TreeCtrl::SetTextColor(COLORREF clr)
{
	return (COLORREF)SendMessage(TVM_SETTEXTCOLOR, 0, (LPARAM)clr);
}

//

/*
ImageList* TreeCtrl::CreateDragImage(HTREEITEM hItem)
{
	HIMAGELIST hImageList = (HIMAGELIST)::SendMessage(m_hWnd,
		TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItem);
	if (hImageList == NULL)
		return NULL;

	CImageList* pImageList = new CImageList;
	VERIFY(pImageList->Attach(hImageList));
}
*/

//

bool TreeCtrl::DeleteAllItems()
{
	return SendMessage(TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT) ? true : false;
}

//

bool TreeCtrl::DeleteItem(HTREEITEM hItem)
{
	return SendMessage(TVM_DELETEITEM, 0, (LPARAM)hItem) ? true : false;
}

//

/*
Edit* TreeCtrl::EditLabel(HTREEITEM hItem)
{
	return (Edit*)Window::FromHandle((HWND)SendMessage(TVM_EDITLABEL, 0, (LPARAM)hItem));
}
*/

//

bool TreeCtrl::EnsureVisible(HTREEITEM hItem)
{
	return SendMessage(TVM_ENSUREVISIBLE, 0, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::Expand(HTREEITEM hItem, unsigned int nCode)
{
	return SendMessage(TVM_EXPAND, nCode, (LPARAM)hItem) ? true : false;
}

//

/*
HTREEITEM TreeCtrl::HitTest(CPoint pt, unsigned int* pFlags = NULL) const
{
	TVHITTESTINFO hti;
	hti.pt = pt;
	HTREEITEM h = (HTREEITEM)SendMessage(TVM_HITTEST, 0,
		(LPARAM)&hti);
	if (pFlags != NULL)
		*pFlags = hti.flags;
	return h;
}
*/

HTREEITEM TreeCtrl::HitTest(TVHITTESTINFO* pHitTestInfo) const
{
	return (HTREEITEM)SendMessage(TVM_HITTEST, 0, (LPARAM)pHitTestInfo);
}

//

HTREEITEM TreeCtrl::InsertItem(const TVINSERTSTRUCT* pInsertStruct)
{
	return (HTREEITEM)SendMessage(TVM_INSERTITEM, 0, (LPARAM)pInsertStruct);
}

HTREEITEM TreeCtrl::InsertItem(unsigned int nMask, const char* pszItem, int iImage, int iSelectedImage, unsigned int nState, unsigned int nStateMask, LPARAM lParam, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = hInsertAfter;
	tvis.item.mask = nMask;
	tvis.item.pszText = (char*)pszItem;
	tvis.item.iImage = iImage;
	tvis.item.iSelectedImage = iSelectedImage;
	tvis.item.state = nState;
	tvis.item.stateMask = nStateMask;
	tvis.item.lParam = lParam;

	return (HTREEITEM)SendMessage(TVM_INSERTITEM, 0, (LPARAM)&tvis);
}

HTREEITEM TreeCtrl::InsertItem(const char* pszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(TVIF_TEXT, pszItem, 0, 0, 0, 0, 0, hParent, hInsertAfter);
}

HTREEITEM TreeCtrl::InsertItem(const char* pszItem, int iImage, int iSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, pszItem, iImage, iSelectedImage, 0, 0, 0, hParent, hInsertAfter);
}

//

bool TreeCtrl::Select(HTREEITEM hItem, unsigned int nCode)
{
	return SendMessage(TVM_SELECTITEM, nCode, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::SelectDropTarget(HTREEITEM hItem)
{
	return SendMessage(TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::SelectItem(HTREEITEM hItem)
{
	return SendMessage(TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::SelectSetFirstVisible(HTREEITEM hItem)
{
	return SendMessage(TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::SortChildren(HTREEITEM hItem)
{
	return SendMessage(TVM_SORTCHILDREN, 0, (LPARAM)hItem) ? true : false;
}

//

bool TreeCtrl::SortChildrenCB(LPTVSORTCB pSort)
{
	return SendMessage(TVM_SORTCHILDRENCB, 0, (LPARAM)pSort) ? true : false;
}

//

} // namespace GDI

//

