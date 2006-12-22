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
//  Max Exporter - GDI Window
//
/////////////////////////////////////////////////

#ifndef __NDS_LexiExporter_GDI_Window__
#define __NDS_LexiExporter_GDI_Window__

//

namespace GDI {

//

class Window {

	public:

		Window();
		virtual ~Window();

	public:

		HWND m_hWnd;

	public:

		static void Cleanup();

		static Window* GetMapping(HWND hWnd);
		static Window* AddMapping(Window* pWnd, HWND hWnd);
		static void RemoveMapping(Window* pWnd);
		static void RemoveMapping(HWND hWnd);

	public:

		// Get temporary pointer from HWND
		static Window* FromHandle(HWND hWnd);

		// Attach/Detach handle
		void Attach(HWND hWnd);
		void Detach();

		// Destroy window
		void DestroyWindow();

		// Get dialog item
		Window* GetDlgItem(int iItem) const;

		// Enable/Disable window
		void EnableWindow(bool bEnable);

		// Window text
		void SetWindowText(const char* pszText);
		std::string GetWindowText() const;

		// Get rect
		void GetWindowRect(RECT& rect) const;
		void GetClientRect(RECT& rect) const;

		// Move window
		void MoveWindow(int x, int y, int w, int h, bool bRepaint);
		bool SetWindowPos(const Window* pWndInsertAfter, int x, int y, int cx, int cy, unsigned int nFlags);

		// Messagebox
		int MessageBox(const char* pszText, const char* pszCaption, unsigned int nType) const;

		// Send message
		int SendMessage(unsigned int Msg, WPARAM wParam, LPARAM lParam) const;

		// Center window
		void CenterWindow(Window* pParent = NULL);

		// Style
		unsigned int GetStyle() const;

		// Show window
		void ShowWindow(unsigned int nShow);

		// Coord conversion
		void ScreenToClient(POINT& point) const;
		void ScreenToClient(RECT& rect) const;
		void ClientToScreen(POINT& point) const;
		void ClientToScreen(RECT& rect) const;

};

//

} // namespace GDI

//

#endif // __NDS_LexiExporter_GDI_Window__