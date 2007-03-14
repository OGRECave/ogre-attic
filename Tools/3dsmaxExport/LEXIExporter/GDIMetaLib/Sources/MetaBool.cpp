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

MetaBool::MetaBool()
{

}
MetaBool::~MetaBool()
{

}

void MetaBool::OnCreated()
{
	m_pCheckCtrl=new Button();
	HWND hWnd=::CreateWindow("BUTTON", "", BS_AUTOCHECKBOX|BS_CENTER|WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0, 0, m_iHeight, m_iHeight, m_hWnd, NULL, NULL, NULL);
	m_pCheckCtrl->Attach(hWnd);			
}

// Set defaults on a data object from a meta object
void MetaBool::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_BOOL)
	{	
		if(pMetaKey->GetKeyType("Default")==DD_INT)
			pData->SetBool(pszMetaID, pMetaKey->GetInt("Default",0)&1);			
		else
			pData->SetBool(pszMetaID, pMetaKey->GetBool("Default", false));
	}
}

// Data object changed - update as needed
void MetaBool::UpdateData(CDDObject *pNewData)
{
	SetDefaults(m_pMetaKey, m_sMetaID.c_str(), pNewData);
/*	bool bData;
	if(pNewData->GetKeyType(m_sMetaID.c_str())==DD_BOOL)
	{
		bData=pNewData->GetBool(m_sMetaID.c_str());
	} else
	{
		if(m_pMetaKey->GetKeyType("Default")==DD_INT)
			bData=m_pMetaKey->GetInt("Default",0)&1;			
		else
			bData=m_pMetaKey->GetBool("Default", false);
		pNewData->SetBool(m_sMetaID.c_str(), bData);
	}*/
	m_pCheckCtrl->SetCheck(pNewData->GetBool(m_sMetaID.c_str()));
}

void MetaBool::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);
	if(!hdc) return;
		
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());

	RECT rLabel;
	::GetClientRect(m_hWnd, &rLabel);		
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	
		
	rLabel.right-=m_iHeight;

	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);
	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	
	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaBool::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pCheckCtrl->SetWindowPos(NULL, rClient.right-m_iHeight, rClient.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

void MetaBool::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pCheckCtrl->m_hWnd)
	{
		if(iCode==BN_CLICKED)
		{
			m_pOwner->GetData()->SetBool(m_sMetaID.c_str(), m_pCheckCtrl->GetCheck()&1);
		}
	}
}

// Enable/disable control
void MetaBool::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pCheckCtrl->EnableWindow(bEnabled);
}

}