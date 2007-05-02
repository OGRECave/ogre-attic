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
#include "LexiDialogProgress.h"
#include "LexiExportObject.h"

#include "..\Res\resource.h"

//
CExportProgressDlg::CExportProgressDlg(Window* pParent) : Dialog(IDD_DIALOG_PROGRESS, DlgProc, pParent)
{
	m_iTotalGlobal=0;
	m_iGlobalProgress=0;
	m_bAbortRequest=false;
	m_bAbortFlag=false;
}

CExportProgressDlg::~CExportProgressDlg()
{
}

// Initialize global stepping
void CExportProgressDlg::InitGlobal(int iObjectCount)
{
	m_pGlobalProgress->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, iObjectCount+1));
	m_iGlobalProgress=0;
	m_iTotalGlobal=iObjectCount;
}

// Do a global step (once per ExportObject)
void CExportProgressDlg::GlobalStep()
{
	++m_iGlobalProgress;
	_snprintf_s(m_InfoBuffer, 1024, "Exporting %d of %d", m_iGlobalProgress, m_iTotalGlobal);
	m_pGlobalInfo->SetWindowText(m_InfoBuffer);
	m_pGlobalProgress->SendMessage(PBM_SETPOS, m_iGlobalProgress, 0);

	// Pump messages
	MSG winmsg;
	while(PeekMessage(&winmsg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&winmsg);
		DispatchMessage(&winmsg);
	}
}

// Initialize local progress
void CExportProgressDlg::InitLocal(int iStepCount)
{
	m_pLocalProgress->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, iStepCount+1));
	m_iLocalProgress=0;
}

// Do a local step
void CExportProgressDlg::LocalStep(const char *pszDesc)
{
	++m_iLocalProgress;	
	if(pszDesc!=NULL) m_pLocalInfo->SetWindowText(pszDesc);
	m_pLocalProgress->SendMessage(PBM_SETPOS, m_iLocalProgress, 0);

	// Pump messages
	MSG winmsg;
	while(PeekMessage(&winmsg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&winmsg);
		DispatchMessage(&winmsg);
	}
}

// Signal export done
void CExportProgressDlg::ExportDone(void)
{
	DestroyWindow();
	CProfiler::getSingletonPtr()->WriteLog("Profiler.log");
}

// Check if we want to abort current export
bool CExportProgressDlg::CheckAbort()
{
	if(m_bAbortFlag) return true;

	if(m_bAbortRequest)
	{
		int rs = MessageBox("Do you want to abort current export?", NDS_EXPORTER_TITLE, MB_YESNO | MB_ICONWARNING);
		if(rs != IDYES) 
		{
			m_bAbortRequest=false;
			return false;	
		}
		m_bAbortFlag=true;
		return true;
	}
	return false;
}

//
INT_PTR CALLBACK CExportProgressDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CExportProgressDlg* pThis = (CExportProgressDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CExportProgressDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 0;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_ABORT:
					pThis->OnAbort();
					break;
			}
		}	break;
	}

	return 0;
}

//

void CExportProgressDlg::OnInitDialog()
{		
	m_pGlobalInfo=GetDlgItem(IDC_EXPORTINFO);
	m_pGlobalProgress=GetDlgItem(IDC_GLOBALPROGRESS);
	m_pLocalInfo=GetDlgItem(IDC_LOCALINFO);
	m_pLocalProgress=GetDlgItem(IDC_LOCALPROGRESS);
	m_pGlobalInfo->SetWindowText("Counting...");
	m_pLocalInfo->SetWindowText("");
	CenterWindow();
}


void CExportProgressDlg::OnAbort()
{
	m_bAbortRequest=true;
}