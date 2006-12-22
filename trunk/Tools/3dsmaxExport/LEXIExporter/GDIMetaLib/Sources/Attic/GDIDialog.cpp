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

Dialog::Dialog(unsigned int iTemplate, DLGPROC fnProc, Window* pParent)
{
	m_iTemplate = iTemplate;
	m_fnProc = fnProc;
	m_hParent = pParent ? pParent->m_hWnd : NULL;
}

Dialog::~Dialog()
{
}

//

bool Dialog::Create(unsigned int nTemplateID, Window* pParent)
{
	HWND hWnd = CreateDialogParam(GetCurrentInstance(), MAKEINTRESOURCE(nTemplateID), pParent->m_hWnd, m_fnProc, (LPARAM)this);
	if(!hWnd) return false;

	return true;
}

//

int Dialog::DoModal()
{
	return DialogBoxParam(GetCurrentInstance(), MAKEINTRESOURCE(m_iTemplate), m_hParent, m_fnProc, (LPARAM)this);
}

//

void Dialog::EndDialog(int iResult)
{
	::EndDialog(m_hWnd, iResult);
}

//

void Dialog::Bind(unsigned int nID, Window& ctl)
{
	ctl.Attach(::GetDlgItem(m_hWnd, nID));
}

//

void Dialog::GotoDlgCtrl(unsigned int nID)
{
	::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)::GetDlgItem(m_hWnd, nID), 1);
}

//

} // namespace GDI

//

