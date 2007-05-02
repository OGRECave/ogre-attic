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

///////////////////////////////////////////////////////////////////////////
//
//  FolderDialog
//
///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//

namespace GDI {

//

FolderDialog::FolderDialog(const char* pszFolderName, const char* pszTitle, Window* pParentWnd)
{
	if(pszFolderName == NULL) m_sInitialFolderName = "";
	else m_sInitialFolderName = pszFolderName;

	ZeroMemory(&m_bi, sizeof(BROWSEINFO));

	if(pParentWnd == NULL) m_bi.hwndOwner = NULL;
	else m_bi.hwndOwner = pParentWnd->m_hWnd;

	m_bi.pszDisplayName = m_strDisplayName;
	m_bi.lpfn = BrowseDirectoryCallback;
	m_bi.lParam = (LPARAM)this;

	if(pszTitle == NULL) m_bi.lpszTitle = "Select Folder.";
	else m_bi.lpszTitle = pszTitle;
}

FolderDialog::~FolderDialog()
{
}

//

int CALLBACK FolderDialog::BrowseDirectoryCallback(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	FolderDialog* pFd=(FolderDialog*)lpData;

	pFd->CallbackFunction(hWnd, uMsg, lParam);

	return 0;
}

//

void FolderDialog::CallbackFunction(HWND hWnd, UINT uMsg,	LPARAM lParam)
{
	m_hDialogBox = hWnd;

	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			OnInitDialog();
			break;

		case BFFM_SELCHANGED:
			OnSelChanged((ITEMIDLIST*)lParam);
			break;
	}
}

int FolderDialog::DoModal()
{
	int nReturn = IDOK;

	m_sFinalFolderName = m_sInitialFolderName;

	ITEMIDLIST* piid = NULL;

	piid = ::SHBrowseForFolder(&m_bi);
	if(piid && ::SHGetPathFromIDList(piid, m_strPath))
	{
		m_sFinalFolderName = m_strPath;
		nReturn = IDOK;
	}
	else
	{
		nReturn = IDCANCEL;
	}

	if(piid)
	{
		LPMALLOC lpMalloc;
		::SHGetMalloc(&lpMalloc);
		lpMalloc->Free(piid);
		lpMalloc->Release();
	}

	return nReturn;
}

std::string FolderDialog::GetFolderName() const
{
	return m_sFinalFolderName;
}

void FolderDialog::EnableOK(bool bEnable)
{
	::SendMessage(m_hDialogBox, BFFM_ENABLEOK, 0, (bEnable ? 1 : 0));
}

void FolderDialog::SetSelection(const char* pszSelection)
{
	::SendMessage(m_hDialogBox, BFFM_SETSELECTION, TRUE, (LPARAM)pszSelection);
}

void FolderDialog::OnInitDialog()
{
	HWND hTreeWnd = GetDlgItem(m_hDialogBox, 14145);
	if(hTreeWnd) SendMessage(hTreeWnd, TVM_SETBKCOLOR, 0, ::GetSysColor(COLOR_WINDOW));

	SetSelection(m_sInitialFolderName.c_str());
}

void FolderDialog::OnSelChanged(ITEMIDLIST* pIdl)
{
	::SHGetPathFromIDList(pIdl, m_strPath);
	m_sFinalFolderName = m_strPath;

	if(m_sFinalFolderName.size()) EnableOK();
	else EnableOK(false);
}

//

} // namespace GDI

//

