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
//  Max Exporter - GDI ListBox
//
/////////////////////////////////////////////////

#include "stdafx.h"

//

namespace GDI {

//

ListBox::ListBox()
{
}

ListBox::~ListBox()
{
}

// Get Count
int ListBox::GetCount()
{
	return SendMessage(LB_GETCOUNT, 0,0);
}

// Get Item Data 
long ListBox::GetItemData(int iIndex) const
{
	return (long)SendMessage(LB_GETITEMDATA, iIndex, 0);
}

// Get Item Data Ptr
void* ListBox::GetItemDataPtr(int iIndex) const
{
	return (void*)SendMessage(LB_GETITEMDATA, iIndex, 0);
}

// Get Item Height
int ListBox::GetItemHeight() const
{
	return SendMessage(LB_GETITEMHEIGHT, 0 ,0);
}

// Get Item Rect
bool ListBox::GetItemRect(int iIndex, RECT* pRect) const
{
	return SendMessage(LB_GETITEMRECT, iIndex, (LPARAM)pRect) ? true : false;;
}

// Item From Point
int ListBox::GetItemFromPoint(POINT &pt) const
{
	return SendMessage(LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
}

// Set Item Data
void ListBox::SetItemData(int iIndex, long data)
{
	SendMessage(LB_SETITEMDATA, iIndex, (LPARAM)data);
}

// Get Cur Sel
int ListBox::GetCurSel()
{
	return SendMessage(LB_GETCURSEL, 0,0);
}

// Set Cur Sel
void ListBox::SetCurSel(int iIndex)
{
	SendMessage(LB_SETCURSEL, iIndex, 0);
}

//
// Add String
int ListBox::AddString(const char* pszText)
{
	return SendMessage(LB_ADDSTRING, 0, (LPARAM)pszText);
}

// Reset Content
void ListBox::ResetContent()
{
	SendMessage(LB_RESETCONTENT, 0,0);
}


} // namespace GDI

//

