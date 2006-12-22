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

#ifndef __NDS_LexiExporter_GDI_ListCtrl__
#define __NDS_LexiExporter_GDI_ListCtrl__

//

namespace GDI {

//

class ListCtrl : public Window {

	public:

		ListCtrl();
		virtual ~ListCtrl();

	public:

		// Set extended styles
		void SetExtendedStyles(unsigned int nStyles, unsigned int nMask);

		// Insert column
		void InsertColumn(int iIndex, const char* pszText, unsigned int nAlignment, int iWidth);

		// Insert item
		int InsertItem(int iIndex, const char* pszText);

		// Delete item
		void DeleteItem(int iIndex);

		// Delete all items
		void DeleteAllItems();

		// Set item text
		void SetItemText(int iIndex, int iSubItem, const char* pszText);

		// Get item count
		int GetItemCount() const;

		// Get selected count
		int GetSelectedCount() const;

		// Get next item
		int GetNextItem(int iStart, unsigned int nFlags) const;

		// Item data
		void SetItemData(int iIndex, unsigned int nData);
		unsigned int GetItemData(int iIndex) const;

		// Set image list
		HIMAGELIST SetImageList(HIMAGELIST hImageList, unsigned int iType);

		// Set item image
		void SetItemImage(int iIndex, unsigned int iImageIndex);

		// Set item state
		bool SetItemState(int iIndex, unsigned int nState, unsigned int nMask);

};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_ListCtrl__