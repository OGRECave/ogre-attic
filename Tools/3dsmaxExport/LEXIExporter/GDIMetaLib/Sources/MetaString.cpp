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

#include "stdafx.h"

namespace GDI
{

MetaString::MetaString()
{

}
MetaString::~MetaString()
{

}

// Set defaults on a data object from a meta object
void MetaString::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_STRING)
	{			
		pData->SetString(pszMetaID, pMetaKey->GetString("Default"));					
	}			
}

void MetaString::OnCreated()
{
	m_pEditCtrl=new Edit();
	HWND hWnd=::CreateWindow("EDIT", "", ES_LEFT|WS_VISIBLE|WS_BORDER|WS_CHILD|WS_TABSTOP, 0, m_iHeight, m_iWidth, m_iHeight, m_hWnd, NULL, NULL, NULL);
	m_pEditCtrl->Attach(hWnd);
	m_iHeight+=m_iHeight;
	::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);		
}

void MetaString::UpdateData(CDDObject *pData)
{
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_STRING)
	{
		m_pEditCtrl->SetWindowText(pData->GetString(m_sMetaID.c_str()));
	} else
	{
		const char *pszDefault=m_pMetaKey->GetString("Default");
		m_pEditCtrl->SetWindowText(pszDefault);
		pData->SetString(m_sMetaID.c_str(), pszDefault);
	}
}

void MetaString::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);
	if(!hdc) return;
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());
		
	RECT rLabel;
	::GetClientRect(m_hWnd, &rLabel);		
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	

	rLabel.bottom-=m_iHeight/2;

	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);
	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	
	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaString::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pEditCtrl->SetWindowPos(NULL, rClient.left, m_iHeight/2, m_iWidth, m_iHeight/2, SWP_NOZORDER);
}

void MetaString::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pEditCtrl->m_hWnd)
	{
		if(iCode==EN_CHANGE)
		{
			std::string sText=m_pEditCtrl->GetWindowText();			
			m_pOwner->GetData()->SetString(m_sMetaID.c_str(), sText.c_str());			
		}
	}
}

// Enable/disable control
void MetaString::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pEditCtrl->EnableWindow(bEnabled);
}

}