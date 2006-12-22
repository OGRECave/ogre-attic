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
//  Max Exporter - GDI ComboBox
//
/////////////////////////////////////////////////

#include "stdafx.h"

//

namespace GDI {

//

ComboBox::ComboBox()
{
}

ComboBox::~ComboBox()
{
}

//

void ComboBox::Clear()
{
	SendMessage(WM_CLEAR, 0, 0);
}

//

void ComboBox::Copy()
{
	SendMessage(WM_COPY, 0, 0);
}

//

void ComboBox::Cut()
{
	SendMessage(WM_CUT, 0, 0);
}

//

bool ComboBox::GetComboBoxInfo(COMBOBOXINFO* pCBI) const
{
	return ::GetComboBoxInfo(m_hWnd, pCBI) ? true : false;
}

//

unsigned int ComboBox::GetCount() const
{
	return (unsigned int)SendMessage(CB_GETCOUNT, 0, 0);
}

//

int ComboBox::GetCurSel() const
{
	return (int)SendMessage(CB_GETCURSEL, 0, 0);
}

//

void ComboBox::GetDroppedControlRect(RECT* pRect) const
{
	SendMessage(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)pRect);
}

//

bool ComboBox::GetDroppedState() const
{
	return SendMessage(CB_GETDROPPEDSTATE, 0, 0) ? true : false;
}

//

unsigned int ComboBox::GetDroppedWidth() const
{
	return (unsigned int)SendMessage(CB_GETDROPPEDWIDTH, 0, 0);
}

//

unsigned int ComboBox::GetEditSel() const
{
	return (unsigned int)SendMessage(CB_GETEDITSEL, 0, 0);
}

//

bool ComboBox::GetExtendedUI() const
{
	return SendMessage(CB_GETEXTENDEDUI, 0, 0) ? true : false;
}

//

unsigned int ComboBox::GetHorizontalExtent() const
{
	return (unsigned int)SendMessage(CB_GETHORIZONTALEXTENT, 0, 0);
}

//

DWORD_PTR ComboBox::GetItemData(int iIndex) const
{
	return SendMessage(CB_GETITEMDATA, iIndex, 0);
}

//

void* ComboBox::GetItemDataPtr(int iIndex) const
{
	return (void*)GetItemData(iIndex);
}

//

unsigned int ComboBox::GetItemHeight(int iIndex) const
{
	return (unsigned int)SendMessage(CB_GETITEMHEIGHT, iIndex, 0L);
}

//

unsigned int ComboBox::GetLBText(int iIndex, char* pszText) const
{
	return (unsigned int)SendMessage(CB_GETLBTEXT, iIndex, (LPARAM)pszText);
}

void ComboBox::GetLBText(int iIndex, std::string& sString) const
{
	char text[4096];
	GetLBText(iIndex, text);
	sString = text;
}

//

unsigned int ComboBox::GetLBTextLen(int iIndex) const
{
	return (unsigned int)SendMessage(CB_GETLBTEXTLEN, iIndex, 0);
}

//

LCID ComboBox::GetLocale() const
{
	return (LCID)SendMessage(CB_GETLOCALE, 0, 0);
}

//

int ComboBox::GetTopIndex() const
{
	return (int)SendMessage(CB_GETTOPINDEX, 0, 0);
}

//

bool ComboBox::LimitText(unsigned int iMaxChars)
{
	return SendMessage(CB_LIMITTEXT, iMaxChars, 0) ? true : false;
}

//

void ComboBox::Paste()
{
	SendMessage(WM_PASTE, 0, 0);
}

//

unsigned int ComboBox::SetCurSel(int iSelect)
{
	return (unsigned int)SendMessage(CB_SETCURSEL, iSelect, 0);
}

//

unsigned int ComboBox::SetDroppedWidth(unsigned int iWidth)
{
	return (unsigned int)SendMessage(CB_SETDROPPEDWIDTH, iWidth, 0);
}

//

bool ComboBox::SetEditSel(int iStartChar, int iEndChar)
{
	return SendMessage(CB_SETEDITSEL, 0, MAKELONG(iStartChar, iEndChar)) ? true : false;
}

//

bool ComboBox::SetExtendedUI(bool bExtended)
{
	return SendMessage(CB_SETEXTENDEDUI, bExtended, 0) != CB_ERR ? true : false;
}

//

void ComboBox::SetHorizontalExtent(unsigned int iExtent)
{
	SendMessage(CB_SETHORIZONTALEXTENT, iExtent, 0);
}

//

bool ComboBox::SetItemData(int iIndex, DWORD_PTR dwItemData)
{
	return SendMessage(CB_SETITEMDATA, iIndex, (LPARAM)dwItemData) != CB_ERR ? true : false;
}

//

bool ComboBox::SetItemDataPtr(int iIndex, void* pData)
{
	return SetItemData(iIndex, (DWORD_PTR)pData);
}

//

bool ComboBox::SetItemHeight(int iIndex, unsigned int iItemHeight)
{
	return SendMessage(CB_SETITEMHEIGHT, iIndex, MAKELONG(iItemHeight, 0)) != CB_ERR ? true : false;
}

//

LCID ComboBox::SetLocale(LCID nNewLocale)
{
	return (LCID)SendMessage(CB_SETLOCALE, (WPARAM)nNewLocale, 0);
}

//

bool ComboBox::SetTopIndex(int iIndex)
{
	return SendMessage(CB_SETTOPINDEX, iIndex, 0) != CB_ERR ? true : false;
}

//

void ComboBox::ShowDropDown(bool bShowIt)
{
	SendMessage(CB_SHOWDROPDOWN, bShowIt, 0);
}

//

int ComboBox::AddString(const char* pszString)
{
	return (int)SendMessage(CB_ADDSTRING, 0, (LPARAM)pszString);
}

//

int ComboBox::DeleteString(int iIndex)
{
	return (int)SendMessage(CB_DELETESTRING, iIndex, 0);
}

//

int ComboBox::Dir(unsigned int attr, const char* pszWildCard)
{
	return (int)SendMessage(CB_DIR, attr, (LPARAM)pszWildCard);
}

//

int ComboBox::FindString(int iStartAfter, const char* pszString) const
{
	return (int)SendMessage(CB_FINDSTRING, iStartAfter, (LPARAM)pszString);
}

//

int ComboBox::FindStringExact(int iIndexStart, const char* pszFind) const
{
	return (int)SendMessage(CB_FINDSTRINGEXACT, iIndexStart, (LPARAM)pszFind);
}

//

int ComboBox::InsertString(int iIndex, const char* pszString)
{
	return (int)SendMessage(CB_INSERTSTRING, iIndex, (LPARAM)pszString);
}

//

void ComboBox::ResetContent()
{
	SendMessage(CB_RESETCONTENT, 0, 0);
}

//

int ComboBox::SelectString(int iStartAfter, const char* pszString)
{
	return (int)SendMessage(CB_SELECTSTRING, iStartAfter, (LPARAM)pszString);
}

//

} // namespace GDI

//

