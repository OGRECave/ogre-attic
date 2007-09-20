/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn,
Lasse Tassing

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

#include "LexiStdAfx.h"
#include "LexiMaxExport.h"
#include "LexiDialogNodeAnimationProperties.h"
#include "LexiExportObject.h"
#include "LexiDialogSelectNode.h"

#include "..\Res\resource.h"

//

CNodeAnimationPropertiesDlg::CNodeAnimationPropertiesDlg(Window* pParent) : Dialog(IDD_DIALOG_NODEANIMPROPERTIES, DlgProc, pParent)
{
	memset(&m_OrgClientRect, 0, sizeof(RECT));	
	m_pMetaCtrl = NULL;
	m_pObj = NULL;
	m_pData = NULL;
}

CNodeAnimationPropertiesDlg::~CNodeAnimationPropertiesDlg()
{
	m_pMetaCtrl->Detach();
	::DestroyWindow(m_hMetaWnd);
	delete m_pMetaCtrl;
}

//

void CNodeAnimationPropertiesDlg::Init(CDDObject* pMeta, const char* pszDefExt)
{
	m_sDefExt = pszDefExt;

	if(m_pMetaCtrl == NULL)
	{
		RECT mainrc;
		GetClientRect(mainrc);
		ClientToScreen(mainrc);

		RECT childrc;

		GetDlgItem(IDC_OBJECT_CHILD_NODEANIM)->GetClientRect(childrc);
		GetDlgItem(IDC_OBJECT_CHILD_NODEANIM)->ClientToScreen(childrc);

		int iChildWidth = childrc.right - childrc.left;
		int iChildHeight = childrc.bottom - childrc.top;

		int iChildX = childrc.left - mainrc.left;
		int iChildY = childrc.top - mainrc.top;

		m_hMetaWnd = ::CreateWindow("MetaControl", NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, iChildX, iChildY, iChildWidth, iChildHeight, m_hWnd, NULL, CExporter::m_hInstance, NULL);

		m_pMetaCtrl = new GDI::MetaControl;
		m_pMetaCtrl->Attach(m_hMetaWnd);
		m_pMetaCtrl->SetDataNotify(this);
	}

	// Setup meta data and object data
	m_pMetaCtrl->CreateFromMeta(pMeta);			

	// Select default control	
	GotoDlgCtrl(IDC_NAME);
}

//

void CNodeAnimationPropertiesDlg::SetInstance(CDDObject* pData, CExportObject* pObj)
{
	m_pData = pData;
	m_pObj = pObj;

	// Setup defaults
	GetDlgItem(IDC_NAME)->SetWindowText(pData->GetString("Name", "<unnamed>"));
	GetDlgItem(IDC_FILENAME)->SetWindowText(pData->GetString("Filename", "<unknown>"));

	m_pMetaCtrl->SetData(m_pData);
}

//

INT_PTR CALLBACK CNodeAnimationPropertiesDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CNodeAnimationPropertiesDlg* pThis = (CNodeAnimationPropertiesDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CNodeAnimationPropertiesDlg*)lParam;
			Window::AddMapping(pThis, hWnd);		
			pThis->OnInitDialog();
		} return 0;

		case WM_SIZE:
		{
			pThis->OnSize();
		} break;		

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_NAME:
					if(HIWORD(wParam) == EN_CHANGE) pThis->OnNameChange();
					break;

				case IDC_FILENAME:
					if(HIWORD(wParam) == EN_CHANGE) pThis->OnFilenameChange();
					break;

				case IDC_BROWSEFILE:
					pThis->BrowseFilename();
					break;
			}
		} break;
	}

	return 0;
}

//

void CNodeAnimationPropertiesDlg::OnInitDialog()
{
	GetClientRect(m_OrgClientRect);
}

//

void CNodeAnimationPropertiesDlg::OnSize()
{
	// Check if have initialized dialog
	if(m_OrgClientRect.right == 0) return;

	RECT rNewMain;
	GetClientRect(rNewMain);
	int iDeltaY = rNewMain.bottom - m_OrgClientRect.bottom;
	int iDeltaX = rNewMain.right - m_OrgClientRect.right;

	// Get original client rect in screen coordinates
	RECT rScreenRect;
	memcpy(&rScreenRect, &m_OrgClientRect, sizeof(RECT));
	ClientToScreen(rScreenRect);	

	// Begin resize/move of all the controls
	HDWP hdwp = ::BeginDeferWindowPos(8);

	RECT rTemp;
	Window* pItem;
	m_pMetaCtrl->GetWindowRect(rTemp);
	hdwp = ::DeferWindowPos(hdwp, m_pMetaCtrl->m_hWnd, NULL, 0, 0, (rTemp.right - rTemp.left) + iDeltaX, (rTemp.bottom - rTemp.top) + iDeltaY, SWP_NOZORDER | SWP_NOMOVE);	

	// Resize object property frame
	pItem = GetDlgItem(IDC_GROUPBOX_TITLE);
	pItem->GetWindowRect(rTemp);
	hdwp = ::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, 0, 0, (rTemp.right - rTemp.left) + iDeltaX, (rTemp.bottom - rTemp.top) + iDeltaY, SWP_NOZORDER | SWP_NOMOVE);

	if(iDeltaX != 0)
	{
		// Move browse filename button
		pItem=GetDlgItem(IDC_BROWSEFILE);
		pItem->GetWindowRect(rTemp);
		ScreenToClient(rTemp);
		hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, rTemp.left+iDeltaX, rTemp.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

		// Resize name window
		pItem = GetDlgItem(IDC_NAME);
		pItem->GetWindowRect(rTemp);
		hdwp = ::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, 0, 0, (rTemp.right - rTemp.left) + iDeltaX, (rTemp.bottom - rTemp.top), SWP_NOZORDER | SWP_NOMOVE);

		// Resize filename window
		pItem=GetDlgItem(IDC_FILENAME);
		pItem->GetWindowRect(rTemp);
		hdwp=::DeferWindowPos(hdwp, pItem->m_hWnd, NULL, 0, 0, (rTemp.right-rTemp.left)+iDeltaX, (rTemp.bottom-rTemp.top), SWP_NOZORDER|SWP_NOMOVE);
	}
	
	::EndDeferWindowPos(hdwp);
	memcpy(&m_OrgClientRect, &rNewMain, sizeof(RECT));
}

//

void CNodeAnimationPropertiesDlg::OnNameChange()
{
	const std::string& sValue = GetDlgItem(IDC_NAME)->GetWindowText();
	m_pData->SetString("Name", sValue.c_str());
}

void CNodeAnimationPropertiesDlg::OnFilenameChange()
{
	const std::string& sValue = GetDlgItem(IDC_FILENAME)->GetWindowText();
	m_pData->SetString("Filename", sValue.c_str());
}

//

void CNodeAnimationPropertiesDlg::OnChanged(const CDDObject* pInstance, const char* pszKey)
{
	::SendMessage(m_hParent, WM_NOTIFY_MESSAGE_ID, 0, 0);
}

//

void CNodeAnimationPropertiesDlg::BrowseFilename()
{	
	// We need to have a valid instance
	if(!m_pObj || !m_pData) return;
	OPENFILENAME ofn;

	// Allocate filename buffer and fill out default
	char *pNameBuffer=new char[_MAX_PATH];
	std::string sDefault=GetDlgItem(IDC_FILENAME)->GetWindowText();
	strcpy(pNameBuffer, sDefault.c_str());

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = m_hWnd; 
	ofn.lpstrFilter = m_sDefExt.c_str(); 
	ofn.lpstrFile= pNameBuffer; 
	ofn.nMaxFile = _MAX_PATH; 
	ofn.lpstrInitialDir = (LPSTR)NULL; 
	ofn.Flags = OFN_OVERWRITEPROMPT; 
	ofn.lpstrTitle = "Save As..."; 

	if(GetSaveFileName(&ofn)==TRUE)
	{
		GetDlgItem(IDC_FILENAME)->SetWindowText(pNameBuffer);
	}	
}

//

