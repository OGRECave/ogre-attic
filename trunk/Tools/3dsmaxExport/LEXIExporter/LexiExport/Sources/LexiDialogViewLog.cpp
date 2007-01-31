/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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
#include "LexiDialogViewLog.h"
#include "LexiExportObject.h"

#include "..\Res\resource.h"

//
CExportViewLogDlg::CExportViewLogDlg(Window* pParent, CMemoryLog *pLog) : Dialog(IDD_DIALOG_LOGVIEW, DlgProc, pParent)
{
	m_pLog=pLog;
}

CExportViewLogDlg::~CExportViewLogDlg()
{
	
}

//
INT_PTR CALLBACK CExportViewLogDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CExportViewLogDlg* pThis = (CExportViewLogDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CExportViewLogDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 0;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{			
				case IDOK:
					pThis->EndDialog(IDOK);
					break;
				case IDCANCEL:
				case IDC_CLOSE:
					pThis->EndDialog(IDCANCEL);					
					break;
				case IDC_SHOWDEBUG:
					pThis->OnShowDebug();
					break;
				case IDC_SHOWOGRE:
					pThis->OnShowOGRE();
					break;
			} 
		} break;

		case WM_CLOSE:
		{
			pThis->EndDialog(IDCANCEL);
			// Allow user to abort progress?
		}	break;

		case WM_SIZE:
		{
			pThis->OnSize();			
		}	break;
	}

	return 0;
}

//

void CExportViewLogDlg::OnInitDialog()
{
	Bind(IDC_LOGTREE, m_LogTree);
	Bind(IDC_SHOWDEBUG, m_ShowDebug);
	Bind(IDC_SHOWOGRE, m_ShowOGRE);

	//
	GetClientRect(m_OrgClientRect);

	CDDObject *pGlobalSettings=CExporter::Get()->GetGlobalSettings();
	m_bShowDebug=pGlobalSettings->GetBool("ShowDebug", false);
	m_ShowDebug.SetCheck(m_bShowDebug);
	m_bShowOGRE=pGlobalSettings->GetBool("ShowOGRE", false);
	m_ShowOGRE.SetCheck(m_bShowOGRE);

	HIMAGELIST hIcons=ImageList_Create(16, 16, ILC_COLOR32, 4, 1);	
	HANDLE hIcon=::LoadImage(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LOGDEBUG), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_SHARED);
	::ImageList_ReplaceIcon(hIcons, -1, (HICON)hIcon);
	hIcon=::LoadImage(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LOGINFO), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_SHARED);
	::ImageList_ReplaceIcon(hIcons, -1, (HICON)hIcon);
	hIcon=::LoadImage(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LOGWARNING), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_SHARED);
	::ImageList_ReplaceIcon(hIcons, -1, (HICON)hIcon);	
	hIcon=::LoadImage(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LOGERROR), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_SHARED);
	::ImageList_ReplaceIcon(hIcons, -1, (HICON)hIcon);	
	hIcon=::LoadImage(CExporter::m_hInstance, MAKEINTRESOURCE(IDI_ICON_LOGOGRE), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR|LR_SHARED);
	::ImageList_ReplaceIcon(hIcons, -1, (HICON)hIcon);	
	m_LogTree.SetImageList(hIcons, TVSIL_NORMAL);
	
	CenterWindow();
	Repopulate();
}

//
void CExportViewLogDlg::OnSize()
{
	// Check if have initialized dialog
	if(m_OrgClientRect.right==0) return;

	RECT rNewMain;
	GetClientRect(rNewMain);
	int iDeltaY=rNewMain.bottom-m_OrgClientRect.bottom;
	int iDeltaX=rNewMain.right-m_OrgClientRect.right;

	// Begin resize/move of all the controls
	HDWP hdwp=::BeginDeferWindowPos(20);
	RECT rTemp;

	// Resize tree output
	m_LogTree.GetWindowRect(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_LogTree.m_hWnd, NULL, 0, 0, (rTemp.right-rTemp.left)+iDeltaX, (rTemp.bottom-rTemp.top)+iDeltaY, SWP_NOZORDER|SWP_NOMOVE);

	// Move debug button	
	m_ShowDebug.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ShowDebug.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Move OGRE button	
	m_ShowOGRE.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_ShowOGRE.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

/*	// Move auto-close button	
	m_AutoClose.GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, m_AutoClose.m_hWnd, NULL, rTemp.left, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);*/

	// Move close button	
	GetDlgItem(IDC_CLOSE)->GetWindowRect(rTemp);
	ScreenToClient(rTemp);
	hdwp=::DeferWindowPos(hdwp, GetDlgItem(IDC_CLOSE)->m_hWnd, NULL, rTemp.left+iDeltaX, rTemp.top+iDeltaY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	::EndDeferWindowPos(hdwp);
	memcpy(&m_OrgClientRect, &rNewMain, sizeof(RECT));
}

void CExportViewLogDlg::OnShowDebug(void)
{
	m_bShowDebug=m_ShowDebug.GetCheck()!=0;
	CExporter::Get()->GetGlobalSettings()->SetBool("ShowDebug", m_bShowDebug);
	Repopulate();
}
void CExportViewLogDlg::OnShowOGRE(void)
{
	m_bShowOGRE=m_ShowOGRE.GetCheck()!=0;
	CExporter::Get()->GetGlobalSettings()->SetBool("ShowOGRE", m_bShowOGRE);
	Repopulate();
}

void CExportViewLogDlg::Repopulate(void)
{
	m_LogTree.DeleteAllItems();

	for(int i=0;i<m_pLog->m_lMessages.size();i++)
	{
		AddMessage(m_pLog->m_lMessages[i]);
	}
}

void CExportViewLogDlg::AddMessage(CMemoryLog::SMessage *pMessage)
{
	if(!m_bShowDebug && pMessage->iLevel==0) return;
	if(pMessage->iLevel==4 && !m_bShowOGRE) return;

	TVINSERTSTRUCT tInsert;
	tInsert.hParent=TVI_ROOT;
	tInsert.hInsertAfter=TVI_LAST;
	tInsert.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tInsert.item.pszText=(char*)pMessage->sMessage.c_str();
	tInsert.item.iImage=pMessage->iLevel;
	tInsert.item.iSelectedImage=pMessage->iLevel;
	m_LogTree.InsertItem(&tInsert);
}



//

