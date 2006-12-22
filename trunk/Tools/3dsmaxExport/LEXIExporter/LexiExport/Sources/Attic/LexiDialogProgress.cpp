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

#define FONT_FACE				"Courier New"
#define FONT_SIZE				9
#define FONT_COLOR_BACK			RGB(255, 255, 255)
#define FONT_COLOR_INFO			RGB(0, 0, 0)
#define FONT_COLOR_WARNING		RGB(0, 0, 255)
#define FONT_COLOR_ERROR		RGB(255, 0, 0)

//

CExportProgressDlg::CExportProgressDlg(Window* pParent) : Dialog(IDD_DIALOG_EXPORTPROGRESS, DlgProc, pParent)
{
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

		case WM_CLOSE:
		{
			// Allow user to abort progress?
		}	break;

		case WM_SIZE:
		{
			if(::IsWindow(pThis->m_hWnd))
			{
				RECT rc;
				pThis->GetClientRect(rc);
				pThis->GetDlgItem(IDC_OUTPUT)->MoveWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
			}
		}	break;
	}

	return 0;
}

//

void CExportProgressDlg::OnInitDialog()
{
	RECT rc;
	GetClientRect(rc);
	GetDlgItem(IDC_OUTPUT)->MoveWindow(0, 0, rc.right - rc.left, rc.bottom - rc.top, true);

	memset(&m_CharFormat, 0, sizeof(CHARFORMAT));
	m_CharFormat.cbSize = sizeof(CHARFORMAT);
	m_CharFormat.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_CHARSET;
	m_CharFormat.dwEffects = CFE_PROTECTED;
	m_CharFormat.yHeight = FONT_SIZE * 20;
	m_CharFormat.bPitchAndFamily = DEFAULT_PITCH;
	m_CharFormat.bCharSet = DEFAULT_CHARSET;
	strcpy(m_CharFormat.szFaceName, FONT_FACE);
	GetDlgItem(IDC_OUTPUT)->SendMessage(EM_SETBKGNDCOLOR, 0, FONT_COLOR_BACK);

	CenterWindow();
}

//

void CExportProgressDlg::Output(const char* pszText, unsigned int iLevel)
{
	Window* pCtrl = GetDlgItem(IDC_OUTPUT);

	COLORREF nColor = FONT_COLOR_INFO;
	if(iLevel == 1) nColor = FONT_COLOR_WARNING;
	else if(iLevel == 2) nColor = FONT_COLOR_ERROR;

	char strText[4096];
	sprintf(strText, "%s\r\n", pszText);

	int iTotal = pCtrl->SendMessage(WM_GETTEXTLENGTH, 0, 0);

	CHARRANGE range;
	range.cpMin = iTotal;
	range.cpMax = iTotal + 1;
	pCtrl->SendMessage(EM_EXSETSEL, 0, (LPARAM)&range);

	m_CharFormat.crTextColor = nColor;
	pCtrl->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_CharFormat);

	pCtrl->SendMessage(EM_REPLACESEL, 0, (LPARAM)strText);

	pCtrl->SendMessage(EM_SCROLLCARET, 0, 0);

	//

	MSG winmsg;
	while(PeekMessage(&winmsg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&winmsg);
		DispatchMessage(&winmsg);
	}
}

//

