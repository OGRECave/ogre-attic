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

#ifndef __NDS_LexiExporter_GDI_ComboBox__
#define __NDS_LexiExporter_GDI_ComboBox__

//

namespace GDI {

//

class ComboBox : public Window {

	public:

		ComboBox();
		virtual ~ComboBox();

	public:

		// Clear
		void Clear();

		// Copy
		void Copy();

		// Cut
		void Cut();

		// Get ComboBox Info
		bool GetComboBoxInfo(COMBOBOXINFO* pCBI) const;

		// GetCount
		unsigned int GetCount() const;

		// Get Cur Sel
		int GetCurSel() const;

		// Get Dropped Control Rect
		void GetDroppedControlRect(RECT* pRect) const;

		// Get Dropped State
		bool GetDroppedState() const;

		// Get Dropped Width
		unsigned int GetDroppedWidth() const;

		// Get Edit Sel
		unsigned int GetEditSel() const;

		// Get Extended UI
		bool GetExtendedUI() const;

		// Get Horizontal Extent
		unsigned int GetHorizontalExtent() const;

		// Get Item Data
		DWORD_PTR GetItemData(int iIndex) const;

		// Get Item Data Ptr
		void* GetItemDataPtr(int iIndex) const;

		// Get Item Height
		unsigned int GetItemHeight(int iIndex) const;

		// Get LB Text
		unsigned int GetLBText(int iIndex, char* pszText) const;
		void GetLBText(int iIndex, std::string& sString) const;

		// Get LB Text Len
		unsigned int GetLBTextLen(int iIndex) const;

		// Get Locale
		LCID GetLocale() const;

		// Get Top Index
		int GetTopIndex() const;

		// Limit Text
		bool LimitText(unsigned int iMaxChars);

		// Paste
		void Paste();

		// Set Cur Sel
		unsigned int SetCurSel(int iSelect);

		// Set Dropped Width
		unsigned int SetDroppedWidth(unsigned int iWidth);

		// Set Edit Sel
		bool SetEditSel(int iStartChar, int iEndChar);

		// Set Extended UI
		bool SetExtendedUI(bool bExtended = true);

		// Set Horizontal Extent
		void SetHorizontalExtent(unsigned int iExtent);

		// Set Item Data
		bool SetItemData(int iIndex, DWORD_PTR dwItemData);

		// Set Item Data Ptr
		bool SetItemDataPtr(int iIndex, void* pData);

		// Set Item Height
		bool SetItemHeight(int iIndex, unsigned int iItemHeight);

		// Set Locale
		LCID SetLocale(LCID nNewLocale);

		// Set Top Index
		bool SetTopIndex(int iIndex);

		// Show Drop Down
		void ShowDropDown(bool bShowIt = true);

		// Add String
		int AddString(const char* pszString);

		// Delete String
		int DeleteString(int iIndex);

		// Dir
		int Dir(unsigned int attr, const char* pszWildCard);

		// Find String
		int FindString(int iStartAfter, const char* pszString) const;

		// Find String Exact
		int FindStringExact(int iIndexStart, const char* pszFind) const;

		// Insert String
		int InsertString(int iIndex, const char* pszString);

		// Reset Content
		void ResetContent();

		// Select String
		int SelectString(int iStartAfter, const char* pszString);

};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_ComboBox__