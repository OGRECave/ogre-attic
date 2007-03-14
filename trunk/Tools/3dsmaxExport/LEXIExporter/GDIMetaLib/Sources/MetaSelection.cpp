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

MetaSelection::MetaSelection()
{
}
MetaSelection::~MetaSelection()
{

}

// Set defaults on a data object from a meta object
void MetaSelection::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_INT)
	{			
		if(pMetaKey->GetKeyType("Default")==DD_STRING)
		{
			const char *pszString=pMetaKey->GetString("Default");
			vector<faststring> lStrings=pMetaKey->GetStringList("Strings");
			int iSel=0;
			for(unsigned i=0;i<lStrings.size();i++)
			{
				if(strcmp(lStrings[i].c_str(),pszString)==0)
				{
					iSel=i;
					break;
				}
			}
			pData->SetInt(pszMetaID, iSel);
		} else
		{
			pData->SetInt(pszMetaID, pMetaKey->GetInt("Default"));
		}
	}			
}

void MetaSelection::OnCreated()
{
	vector<faststring> lStrings=m_pMetaKey->GetStringList("Strings");

	m_pCombo=new ComboBox();
	HWND hWnd=::CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST|WS_VISIBLE|WS_CHILD, 0, 0, g_iSelectionWidth, g_iComboHeight, m_hWnd, NULL, NULL, NULL);
	m_pCombo->Attach(hWnd);	
	::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);

	RECT rmy;
	m_pCombo->GetClientRect(rmy);
	m_iHeight=rmy.bottom;
	
	for(unsigned i=0;i<lStrings.size();i++)
	{
		m_pCombo->AddString(lStrings[i].c_str());	
	}	
}

void MetaSelection::UpdateData(CDDObject *pData)
{
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_INT)
	{
		m_pCombo->SetCurSel(pData->GetInt(m_sMetaID.c_str()));
	} else
	{
		if(m_pMetaKey->GetKeyType("Default")==DD_STRING)
			m_pCombo->SelectString(0, m_pMetaKey->GetString("Default"));
		else if(m_pMetaKey->GetKeyType("Default")==DD_INT)
			m_pCombo->SetCurSel(m_pMetaKey->GetInt("Default"));

		// Set selection on data object
		pData->SetInt(m_sMetaID.c_str(), m_pCombo->GetCurSel());
	}
}

void MetaSelection::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);

	RECT rLabel;
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());
	::GetClientRect(m_hWnd, &rLabel);		
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	

	rLabel.right-=g_iSelectionWidth;

	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);

	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	
	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaSelection::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pCombo->SetWindowPos(NULL, rClient.right-g_iSelectionWidth, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

void MetaSelection::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pCombo->m_hWnd && iCode==CBN_SELCHANGE)
	{
		m_pOwner->GetData()->SetInt(m_sMetaID.c_str(), m_pCombo->GetCurSel());
	}
}

// Enable/disable control
void MetaSelection::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pCombo->EnableWindow(bEnabled);
}

}