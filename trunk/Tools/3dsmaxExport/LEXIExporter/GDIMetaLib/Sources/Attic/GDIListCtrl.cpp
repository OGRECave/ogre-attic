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
//  Max Exporter - GDI ListCtrl
//
/////////////////////////////////////////////////

#include "stdafx.h"

//

namespace GDI {

//

ListCtrl::ListCtrl()
{
}

ListCtrl::~ListCtrl()
{
}

//

void ListCtrl::SetExtendedStyles(unsigned int nStyles, unsigned int nMask)
{
	SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, nStyles, nMask);
}

//

void ListCtrl::InsertColumn(int iIndex, const char* pszText, unsigned int nAlignment, int iWidth)
{
	LVCOLUMN clm;
	memset(&clm, 0, sizeof(clm));

	clm.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	clm.fmt = nAlignment;
	clm.cx = iWidth;
	clm.pszText = (char*)pszText;
	SendMessage(LVM_INSERTCOLUMN, iIndex, (LPARAM)&clm);
}

//

int ListCtrl::InsertItem(int iIndex, const char* pszText)
{
	LVITEM item;
	memset(&item, 0, sizeof(item));

	item.mask = LVIF_TEXT;
	item.iItem = iIndex;
	item.pszText = (char*)pszText;
	return SendMessage(LVM_INSERTITEM, 0, (LPARAM)&item);
}

void ListCtrl::DeleteItem(int iIndex)
{
	SendMessage(LVM_DELETEITEM, iIndex, 0);
}

void ListCtrl::DeleteAllItems()
{
	SendMessage(LVM_DELETEALLITEMS, 0, 0);
}

//

void ListCtrl::SetItemText(int iIndex, int iSubItem, const char* pszText)
{
	LVITEM item;
	memset(&item, 0, sizeof(item));

	item.mask = LVIF_TEXT;
	item.iSubItem = iSubItem;
	item.pszText = (char*)pszText;
	SendMessage(LVM_SETITEMTEXT, iIndex, (LPARAM)&item);
}

//

int ListCtrl::GetItemCount() const
{
	return SendMessage(LVM_GETITEMCOUNT, 0, 0);
}

//

int ListCtrl::GetSelectedCount() const
{
	return SendMessage(LVM_GETSELECTEDCOUNT, 0, 0);
}

//

int ListCtrl::GetNextItem(int iStart, unsigned int nFlags) const
{
	return SendMessage(LVM_GETNEXTITEM, iStart, nFlags);
}

//

void ListCtrl::SetItemData(int iIndex, unsigned int nData)
{
	LVITEM item;
	item.iItem = iIndex;
	item.mask = LVIF_PARAM;
	item.lParam = nData;
	SendMessage(LVM_SETITEM, 0, (LPARAM)&item);
}

unsigned int ListCtrl::GetItemData(int iIndex) const
{
	LVITEM item;
	item.iItem = iIndex;
	item.mask = LVIF_PARAM;
	SendMessage(LVM_GETITEM, 0, (LPARAM)&item);

	return item.lParam;
}

//

HIMAGELIST ListCtrl::SetImageList(HIMAGELIST hImageList, unsigned int iType)
{
	return (HIMAGELIST)SendMessage(LVM_SETIMAGELIST,  iType, (LPARAM)hImageList);
}

//

void ListCtrl::SetItemImage(int iIndex, unsigned int iImageIndex)
{
	LVITEM item;
	item.iItem = iIndex;
	item.iSubItem = 0;
	item.mask = LVIF_IMAGE;
	item.iImage = iImageIndex;
	SendMessage(LVM_SETITEM, 0, (LPARAM)&item);
}

//

bool ListCtrl::SetItemState(int iIndex, unsigned int nState, unsigned int nMask)
{
	LVITEM lvi;
	lvi.stateMask = nMask;
	lvi.state = nState;
	return SendMessage(LVM_SETITEMSTATE, iIndex, (LPARAM)&lvi) ? true : false;
}

//

} // namespace GDI

//

