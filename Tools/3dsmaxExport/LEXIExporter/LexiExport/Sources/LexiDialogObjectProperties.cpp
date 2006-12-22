/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
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

#include "LexiStdAfx.h"
#include "LexiMaxExport.h"
#include "LexiDialogObjectProperties.h"
#include "LexiExportObject.h"
#include "LexiDialogSelectNode.h"

#include "..\Res\resource.h"

//

CObjectPropertiesDlg::CObjectPropertiesDlg(Window* pParent, CExportObject* pObj) : Dialog(IDD_DIALOG_OBJECTPROPERTIES, DlgProc, pParent)
{
	m_pObj = pObj;
}

//

INT_PTR CALLBACK CObjectPropertiesDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CObjectPropertiesDlg* pThis = (CObjectPropertiesDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CObjectPropertiesDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 0;

		case WM_CLOSE:
		{
			pThis->OnCancel();
			pThis->EndDialog(IDCANCEL);
		}	break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					pThis->OnOK();
					pThis->EndDialog(IDOK);
					break;

				case IDCANCEL:
					pThis->OnCancel();
					pThis->EndDialog(IDCANCEL);
					break;

				case IDC_BROWSENODE:
					pThis->BrowseNode();
					break;
			}

		}	return 1;
	}

	return 0;
}

//

void CObjectPropertiesDlg::OnInitDialog()
{
	SetWindowText(m_sTitle.c_str());

	GetDlgItem(IDC_GROUPBOX_TITLE)->SetWindowText(m_sGBTitle.c_str());
	GetDlgItem(IDC_NAME)->SetWindowText(m_sName.c_str());
	GetDlgItem(IDC_FILENAME)->SetWindowText(m_sFilename.c_str());

	if(m_iInitFromSelected)
	{
		m_iID = m_iInitFromSelected;
		ForceUpdateMembers();
	}
	else if(strcmp(m_sName.c_str(),"<unnamed>")==0)
	{
		BrowseNode();
	}

	GetDlgItem(IDC_NODE)->SetWindowText(GetNameFromID(m_iID));

	//

	RECT mainrc;
	GetClientRect(mainrc);
	ClientToScreen(mainrc);

	RECT childrc;
	GetDlgItem(IDC_OBJECT_CHILD)->GetClientRect(childrc);
	GetDlgItem(IDC_OBJECT_CHILD)->ClientToScreen(childrc);

	int iChildWidth = childrc.right - childrc.left;
	int iChildHeight = childrc.bottom - childrc.top;

	int iChildX = childrc.left - mainrc.left;
	int iChildY = childrc.top - mainrc.top;

	CDDObject* pMetaDesc = m_pObj->GetMetaDesc();
	CDDObject* pEditMeta = m_pObj->GetEditMeta();
	if(pMetaDesc)
	{
		m_hMetaWnd = ::CreateWindow("MetaControl", NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, iChildX, iChildY, iChildWidth, iChildHeight, m_hWnd, NULL, CExporter::m_hInstance, NULL);

		m_pMetaCtrl = new GDI::MetaControl;
		m_pMetaCtrl->Attach(m_hMetaWnd);
		m_pMetaCtrl->SetData(pEditMeta);
		m_pMetaCtrl->CreateFromMeta(pMetaDesc);

		//pMetaDesc->Release();
	}

	CenterWindow();

	//

	GotoDlgCtrl(IDC_NODE);
}

//

void CObjectPropertiesDlg::OnOK()
{
	m_sName = GetDlgItem(IDC_NAME)->GetWindowText();
	m_sFilename = GetDlgItem(IDC_FILENAME)->GetWindowText();

	//

	m_pMetaCtrl->Detach();
	::DestroyWindow(m_hMetaWnd);
	delete m_pMetaCtrl;
}

void CObjectPropertiesDlg::OnCancel()
{
	m_pMetaCtrl->Detach();
	::DestroyWindow(m_hMetaWnd);
	delete m_pMetaCtrl;
}

//

void CObjectPropertiesDlg::BrowseNode()
{
	CSelectNodeDlg dlg(this, m_pObj);
	dlg.m_iNode = m_iID;

	if(dlg.DoModal() != IDOK) return;

	m_iID = dlg.m_iNode;
	GetDlgItem(IDC_NODE)->SetWindowText(GetNameFromID(m_iID));

	ForceUpdateMembers();
}

//

void CObjectPropertiesDlg::ForceUpdateMembers()
{
	std::string sTemp = GetDlgItem(IDC_NAME)->GetWindowText();
	if(!sTemp.size() || !stricmp(sTemp.c_str(), "<unnamed>"))
	{
		sTemp = GetNameFromID(m_iID);
		RemoveIllegalChars(sTemp);
		GetDlgItem(IDC_NAME)->SetWindowText(sTemp.c_str());
	}

	sTemp = GetDlgItem(IDC_FILENAME)->GetWindowText();
	if(!sTemp.size() || !stricmp(sTemp.c_str(), "<unknown>"))
	{
		sTemp = GetNameFromID(m_iID);
		sTemp += ".";
		sTemp += m_pObj->GetDefaultFileExt();
		RemoveIllegalChars(sTemp);
		sTemp = "C:\\" + sTemp;
		GetDlgItem(IDC_FILENAME)->SetWindowText(sTemp.c_str());
	}
}

//

