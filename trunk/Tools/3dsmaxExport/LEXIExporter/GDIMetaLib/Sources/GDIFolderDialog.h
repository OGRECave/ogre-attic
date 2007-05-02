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

#ifndef __GDI_FolderDialog__
#define __GDI_FolderDialog__

#include <shlobj.h>

//

namespace GDI {

//

class FolderDialog {

	public:

		// Constructor/Destructor
		FolderDialog(const char* pszFolderName = NULL, const char* pszTitle = NULL, Window* pParentWnd = NULL);
		virtual ~FolderDialog();

		// Call Dialog
		int DoModal();

		// Get selected folder
		std::string GetFolderName() const;

		BROWSEINFO m_bi;

	protected:

		void OnInitDialog();
		void OnSelChanged(ITEMIDLIST* pIdl);
		void CallbackFunction(HWND hWnd, UINT uMsg,	LPARAM lParam);

		void EnableOK(bool bEnable=true);
		void SetSelection(LPCTSTR pszSelection);

		std::string m_sInitialFolderName;
		std::string m_sFinalFolderName;

		char m_strDisplayName[MAX_PATH];
		char m_strPath[MAX_PATH];

		HWND m_hDialogBox;

		static int CALLBACK BrowseDirectoryCallback(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

};

//

} // namespace GDI

//

#endif // __GDI_FolderDialog__