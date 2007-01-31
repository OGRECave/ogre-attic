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

#ifndef __NDS_LexiExporter_GDI_TreeCtrl__
#define __NDS_LexiExporter_GDI_TreeCtrl__

//

namespace GDI {

//

class TreeCtrl : public Window 
{
public:

	TreeCtrl();
	virtual ~TreeCtrl();

public:

	// Get Bk Color
	COLORREF GetBkColor() const;

	// Get Check
	bool GetCheck(HTREEITEM hItem) const;

	// Get Child Item
	HTREEITEM GetChildItem(HTREEITEM hItem) const;

	// Get Count
	unsigned int GetCount() const;

	// Get Drop Hilight Item
	HTREEITEM GetDropHilightItem() const;

	// Get Edit Control
//		Edit* GetEditControl() const;

	// Get First Visible Item
	HTREEITEM GetFirstVisibleItem() const;

	// Get Image List
//		ImageList* GetImageList(unsigned int iImageList) const;

	// Get Indent
	unsigned int GetIndent() const;

	// Get Insert Mark Color
	COLORREF GetInsertMarkColor() const;

	// Get Item
	bool GetItem(TVITEM* pItem) const;

	// Get Item Data
	DWORD_PTR GetItemData(HTREEITEM hItem) const;

	// Get Item Height
	int GetItemHeight() const;

	// Get Item Image
	bool GetItemImage(HTREEITEM hItem, int& iImage, int& iSelectedImage) const;

	// Get Item Rect
	bool GetItemRect(HTREEITEM hItem, RECT* pRect, bool bTextOnly) const;

	// Get Item State
	unsigned int GetItemState(HTREEITEM hItem, unsigned int nStateMask) const;

	// Get Item Text
	std::string GetItemText(HTREEITEM hItem) const;

	// Get Line Color
	COLORREF GetLineColor() const;

	// Get Next Item
	HTREEITEM GetNextItem(HTREEITEM hItem, unsigned int nCode) const;

	// Get Next Sibling Item
	HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const;

	// Get Next Visible Item
	HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const;

	// Get Parent Item
	HTREEITEM GetParentItem(HTREEITEM hItem) const;

	// Get Prev Sibling Item
	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const;

	// Get Prev Visible Item
	HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const;

	// Get Root Item
	HTREEITEM GetRootItem() const;

	// Get Scroll Time
	unsigned int GetScrollTime() const;

	// Get Selected Item
	HTREEITEM GetSelectedItem() const;

	// Get Text Color
	COLORREF GetTextColor() const;

	// Get Visible Count
	unsigned int GetVisibleCount() const;

	// Item Has Children
	bool ItemHasChildren(HTREEITEM hItem) const;

	// Set Bk Color
	COLORREF SetBkColor(COLORREF clr);

	// Set Check
	bool SetCheck(HTREEITEM hItem, bool bCheck = true);

	// Set Image List
	HIMAGELIST SetImageList(HIMAGELIST hImageList, int iImageListType);

	// Set Indent
	void SetIndent(unsigned int iIndent);

	// Set Insert Mark
	bool SetInsertMark(HTREEITEM hItem, bool bAfter = true);

	// Set Insert Mark Color
	COLORREF SetInsertMarkColor(COLORREF clrNew);

	// Set Item
	bool SetItem(TVITEM* pItem);
	bool SetItem(HTREEITEM hItem, unsigned int nMask, const char* pszItem, int iImage, int iSelectedImage, unsigned int nState, unsigned int nStateMask, LPARAM lParam);

	// Set Item Data
	bool SetItemData(HTREEITEM hItem, DWORD_PTR dwData);

	// Set Item Height
	int SetItemHeight(int iHeight);

	// Set Item Image
	bool SetItemImage(HTREEITEM hItem, int iImage, int iSelectedImage);

	// Set Item State
	bool SetItemState(HTREEITEM hItem, unsigned int nState, unsigned int nStateMask);

	// Set Item Text
	bool SetItemText(HTREEITEM hItem, const char* pszItem);

	// Set Line Color
	COLORREF SetLineColor(COLORREF clrNew = CLR_DEFAULT);

	// Set Scroll Time
	unsigned int SetScrollTime(unsigned int iScrollTime);

	// Set Text Color
	COLORREF SetTextColor(COLORREF clr);

	// Create Drag Image
//		ImageList* CreateDragImage(HTREEITEM hItem);

	// Delete All Items
	bool DeleteAllItems();

	// Delete Item
	bool DeleteItem(HTREEITEM hItem);

	// Edit Label
//		Edit* EditLabel(HTREEITEM hItem);

	// Ensure Visible
	bool EnsureVisible(HTREEITEM hItem);

	// Expand
	bool Expand(HTREEITEM hItem, unsigned int nCode);

	// Hit Test
//		HTREEITEM HitTest(CPoint pt, unsigned int* pFlags = NULL) const;
	HTREEITEM HitTest(TVHITTESTINFO* pHitTestInfo) const;

	// Insert Item
	HTREEITEM InsertItem(const TVINSERTSTRUCT* pInsertStruct);
	HTREEITEM InsertItem(unsigned int nMask, const char* pszItem, int iImage, int iSelectedImage, unsigned int nState, unsigned int nStateMask, LPARAM lParam, HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM InsertItem(const char* pszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM InsertItem(const char* pszItem, int iImage, int iSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	// Select
	bool Select(HTREEITEM hItem, unsigned int nCode);

	// Select Drop Target
	bool SelectDropTarget(HTREEITEM hItem);

	// Select Item
	bool SelectItem(HTREEITEM hItem);

	// Select Set First Visible
	bool SelectSetFirstVisible(HTREEITEM hItem);

	// Sort Children
	bool SortChildren(HTREEITEM hItem);

	// Sort Children CB
	bool SortChildrenCB(LPTVSORTCB pSort);
};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_TreeCtrl__