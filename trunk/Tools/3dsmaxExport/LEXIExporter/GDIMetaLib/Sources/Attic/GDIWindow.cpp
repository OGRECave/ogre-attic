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

#include "stdafx.h"

//

namespace GDI {

//

Window::Window()
{
	m_hWnd = NULL;
}

Window::~Window()
{
}

//

class Mapping {

	public:

		Window* m_pWnd;
		bool m_bDynamic;

};

typedef std::map<HWND, Mapping> HPMap;
static HPMap m_HPMap;

Window* Window::GetMapping(HWND hWnd)
{
	HPMap::const_iterator it = m_HPMap.find(hWnd);
	return it != m_HPMap.end() ? it->second.m_pWnd : NULL;
}

Window* Window::AddMapping(Window* pWnd, HWND hWnd)
{
	bool bDynamic = false;

	if(!pWnd)
	{
		pWnd = new Window;
		bDynamic = true;
	}

	Mapping m;
	m.m_pWnd = pWnd;
	m.m_bDynamic = bDynamic;
	m_HPMap[hWnd] = m;

	pWnd->m_hWnd = hWnd;

	return pWnd;
}

void Window::RemoveMapping(Window* pWnd)
{
	RemoveMapping(pWnd->m_hWnd);
}

void Window::RemoveMapping(HWND hWnd)
{
	HPMap::iterator it = m_HPMap.find(hWnd);
	if(it == m_HPMap.end()) return;

	const Mapping& m = it->second;
	if(m.m_bDynamic)
	{
		Window* pWnd = m.m_pWnd;
		pWnd->m_hWnd = NULL;
		delete pWnd;
	}

	m.m_pWnd->m_hWnd = NULL;

	m_HPMap.erase(it);
}

//

void Window::Cleanup()
{
	for(HPMap::const_iterator pit = m_HPMap.begin(); pit != m_HPMap.end(); pit++)
	{
		const Mapping& m = pit->second;
		if(m.m_bDynamic)
		{
			Window* pWnd = m.m_pWnd;
			pWnd->m_hWnd = NULL;
			delete pWnd;
		}
	}

	m_HPMap.clear();
}

//

Window* Window::FromHandle(HWND hWnd)
{
	if(!hWnd) return NULL;
	Window* pWnd = GetMapping(hWnd);
	return pWnd ? pWnd : AddMapping(NULL, hWnd);
}

//

void Window::Attach(HWND hWnd)
{
	if(m_hWnd) throw;
	if(GetMapping(hWnd)) throw;
	AddMapping(this, hWnd);
}

void Window::Detach()
{
	if(!m_hWnd) throw;
	RemoveMapping(this);
}

//

void Window::DestroyWindow()
{
	::DestroyWindow(m_hWnd);
}

//

void Window::EnableWindow(bool bEnable)
{
	::EnableWindow(m_hWnd, bEnable ? TRUE : FALSE);
}

//

Window* Window::GetDlgItem(int iItem) const
{
	return Window::FromHandle(::GetDlgItem(m_hWnd, iItem));
}

//

void Window::SetWindowText(const char* pszText)
{
	::SetWindowText(m_hWnd, pszText);
}

std::string Window::GetWindowText() const
{
	char temp[32768] = { 0 };
	::GetWindowText(m_hWnd, temp, 32766);
	return temp;
}

//

void Window::GetWindowRect(RECT& rect) const
{
	::GetWindowRect(m_hWnd, &rect);
}

void Window::GetClientRect(RECT& rect) const
{
	::GetClientRect(m_hWnd, &rect);
}

//

void Window::MoveWindow(int x, int y, int w, int h, bool bRepaint)
{
	::MoveWindow(m_hWnd, x, y, w, h, bRepaint ? TRUE : FALSE);
}

bool Window::SetWindowPos(const Window* pWndInsertAfter, int x, int y, int cx, int cy, unsigned int nFlags)
{
	return ::SetWindowPos(m_hWnd, pWndInsertAfter ? pWndInsertAfter->m_hWnd : NULL, x, y, cx, cy, nFlags) ? true : false;
}

//

int Window::MessageBox(const char* pszText, const char* pszCaption, unsigned int nType) const
{
	return ::MessageBox(m_hWnd, pszText, pszCaption, nType);
}

//

int Window::SendMessage(unsigned int Msg, WPARAM wParam, LPARAM lParam) const
{
	return ::SendMessage(m_hWnd, Msg, wParam, lParam);
}

//

unsigned int Window::GetStyle() const
{
	return ::GetWindowLong(m_hWnd, GWL_STYLE);
}

//

void Window::CenterWindow(Window* pParent)
{
	DWORD nStyle = GetStyle();
	HWND hWndCenter = pParent ? pParent->m_hWnd : NULL;

	if(!pParent)
	{
		if(nStyle & WS_CHILD) hWndCenter = ::GetParent(m_hWnd);
		else hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
	}

	RECT rcDlg;
	GetWindowRect(rcDlg);

	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;

	if(!(nStyle & WS_CHILD))
	{
		if(hWndCenter)
		{
			unsigned int nAlternateStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
			if(!(nAlternateStyle & WS_VISIBLE) || (nAlternateStyle & WS_MINIMIZE)) hWndCenter = NULL;
		}

 		MONITORINFO mi;
		mi.cbSize = sizeof(mi);

		if(!hWndCenter)
		{
			HWND hwDefault = GetDesktopWindow();//AfxGetMainWnd()->GetSafeHwnd();

			GetMonitorInfo(MonitorFromWindow(hwDefault, MONITOR_DEFAULTTOPRIMARY), &mi);
			rcCenter = mi.rcWork;
			rcArea = mi.rcWork;
		}
		else
		{
			::GetWindowRect(hWndCenter, &rcCenter);
			GetMonitorInfo(MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST), &mi);
			rcArea = mi.rcWork;
		}
	}
	else
	{
		hWndParent = ::GetParent(m_hWnd);

		::GetClientRect(hWndParent, &rcArea);
		::GetClientRect(hWndCenter, &rcCenter);
		::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	int iDlgWidth = rcDlg.right - rcDlg.left;
	int iDlgHeight = rcDlg.bottom - rcDlg.top;

	int xLeft = ((rcCenter.left + rcCenter.right) >> 1) - (iDlgWidth >> 1);
	int yTop = ((rcCenter.top + rcCenter.bottom) >> 1) - (iDlgHeight >> 1);

	if(xLeft < rcArea.left) xLeft = rcArea.left;
	else if(xLeft + iDlgWidth > rcArea.right) xLeft = rcArea.right - iDlgWidth;

	if(yTop < rcArea.top) yTop = rcArea.top;
	else if(yTop + iDlgHeight > rcArea.bottom) yTop = rcArea.bottom - iDlgHeight;

	SetWindowPos(NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

//

void Window::ShowWindow(unsigned int nShow)
{
	::ShowWindow(m_hWnd, nShow);
}

//

void Window::ScreenToClient(POINT& point) const
{
	::ScreenToClient(m_hWnd, &point);
}

void Window::ScreenToClient(RECT& rect) const
{
	::ScreenToClient(m_hWnd, (LPPOINT)&rect);
	::ScreenToClient(m_hWnd, ((LPPOINT)&rect) + 1);
}

void Window::ClientToScreen(POINT& point) const
{
	::ClientToScreen(m_hWnd, &point);
}

void Window::ClientToScreen(RECT& rect) const
{
	::ClientToScreen(m_hWnd, (LPPOINT)&rect);
	::ClientToScreen(m_hWnd, ((LPPOINT)&rect) + 1);
}

//

} // namespace GDI

//

