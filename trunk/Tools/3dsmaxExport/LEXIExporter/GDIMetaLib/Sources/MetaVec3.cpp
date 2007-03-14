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

MetaVec3::MetaVec3()
{
	m_bShowControls=false;
}
MetaVec3::~MetaVec3()
{

}

// Set defaults on a data object from a meta object
void MetaVec3::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_VEC3)
	{			
		pData->SetVec3(pszMetaID, pMetaKey->GetVec3("Default"));					
	}			
}

void MetaVec3::OnCreated()
{
	int iY=g_iDefaultCtrlHeight;

	for(int i=0;i<3;i++)
	{
		m_pEditCtrls[i]=new Edit();
		HWND hWnd=::CreateWindow("EDIT", "", ES_RIGHT|WS_VISIBLE|WS_BORDER|WS_CHILD|WS_TABSTOP, 0, 
								iY, g_iSmallEditWidth, g_iDefaultCtrlHeight, m_hWnd, NULL, NULL, NULL);
		::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);
		m_pEditCtrls[i]->Attach(hWnd);
		iY+=g_iDefaultCtrlHeight;
	}
}

void MetaVec3::UpdateData(CDDObject *pData)
{	
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_VEC3)
	{
		m_Value=pData->GetVec3(m_sMetaID.c_str());
	} else
	{
		// Get default value as either float or int
		m_Value=m_pMetaKey->GetVec3("Default");

		// Set data 
		pData->SetVec3(m_sMetaID.c_str(), m_Value);
	}

	char buffer[25];
	_snprintf_s(buffer, 25, "%.3f", m_Value.x);
	m_pEditCtrls[0]->SetWindowText(buffer);
	_snprintf_s(buffer, 25, "%.3f", m_Value.y);
	m_pEditCtrls[1]->SetWindowText(buffer);
	_snprintf_s(buffer, 25, "%.3f", m_Value.z);
	m_pEditCtrls[2]->SetWindowText(buffer);
}

void MetaVec3::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);
	if(!hdc) return;
		
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());
	RECT rLabel;
	::GetClientRect(m_hWnd, &rLabel);
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	

	rLabel.bottom=g_iDefaultCtrlHeight;
	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);

	rLabel.left+=2;
	if(m_bShowControls)
	{
		RECT rTempLabel;
		memcpy(&rTempLabel, &rLabel, sizeof(RECT));
		::DrawText(hdc, "-", -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		rTempLabel.left=0;
		rTempLabel.right-=g_iSmallEditWidth;
		rTempLabel.top=rLabel.bottom;
		rTempLabel.bottom+=g_iDefaultCtrlHeight*3;
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rTempLabel, NULL, 0, NULL);
		
		rTempLabel.bottom=g_iDefaultCtrlHeight*2;
		::DrawText(hdc, "X", -1, &rTempLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		rTempLabel.top+=g_iDefaultCtrlHeight;
		rTempLabel.bottom+=g_iDefaultCtrlHeight;
		::DrawText(hdc, "Y", -1, &rTempLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		rTempLabel.top+=g_iDefaultCtrlHeight;
		rTempLabel.bottom+=g_iDefaultCtrlHeight;
		::DrawText(hdc, "Z", -1, &rTempLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	}
	else
		::DrawText(hdc, "+", -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	rLabel.left+=10;

	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);	

	char buffer[50];
	_snprintf(buffer, 50, "{%.3f, %.3f, %.3f}", m_Value.x, m_Value.y, m_Value.z);
	::SelectObject(hdc, m_pGroup->GetCtrlFontBold());
	::DrawText(hdc, buffer, -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
	
	::SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaVec3::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	int iY=g_iDefaultCtrlHeight;
	for(int i=0;i<3;i++)
	{
		m_pEditCtrls[i]->SetWindowPos(NULL, rClient.right-g_iSmallEditWidth, iY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		iY+=g_iDefaultCtrlHeight;
	}
}
/*
bool MetaVec3::OnKeyDown(unsigned vKey, unsigned iFlags)
{
	if(!m_bShowControls) return false;

	if(vKey==VK_UP || vKey==VK_DOWN)
	{
		HWND hWnd=::GetFocus();
		for(int i=0;i<3;i++)
		{
			if(m_pEditCtrls[i]->m_hWnd==hWnd)
			{
				if(vKey==VK_UP)
					m_Value[i]--;
				else
					m_Value[i]++;

				char buffer[25];
				_snprintf_s(buffer, 25, "%.3f", m_Value.x);
				m_pEditCtrls[0]->SetWindowText(buffer);
				_snprintf_s(buffer, 25, "%.3f", m_Value.y);
				m_pEditCtrls[1]->SetWindowText(buffer);
				_snprintf_s(buffer, 25, "%.3f", m_Value.z);
				m_pEditCtrls[2]->SetWindowText(buffer);

				m_pOwner->GetData()->SetVec3(m_sMetaID.c_str(), m_Value);
				InvalidateRect(m_hWnd, NULL, false);
			}
		}
	}
	return false;
}*/

bool MetaVec3::OnMouseDown(int iFlags, int iX, int iY)
{	
	if(iX<10 && iY<=g_iDefaultCtrlHeight && m_bEnabled)
	{
		m_bShowControls^=1;
		for(int i=0;i<3;i++)
			m_pEditCtrls[i]->ShowWindow(m_bShowControls ? SW_SHOW : SW_HIDE);

		if(m_bShowControls)
			m_iHeight=g_iDefaultCtrlHeight*4;
		else
			m_iHeight=g_iDefaultCtrlHeight;

		InvalidateRect(this->m_hWnd, NULL, false);
		m_pGroup->OnCtrlHeightChange(this);
		return true;
	}
	return false;
}

void MetaVec3::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(iCode!=EN_CHANGE) return;

	for(int i=0;i<3;i++)
	{
		if(hWnd==m_pEditCtrls[i]->m_hWnd)
		{
			std::string sText=m_pEditCtrls[i]->GetWindowText();
			char *pTemp;
			float fValue=(float)strtod(sText.c_str(), &pTemp);
			m_Value[i]=fValue;			
			m_pOwner->GetData()->SetVec3(m_sMetaID.c_str(), m_Value);
			InvalidateRect(m_hWnd, NULL, false);
		}
	}	
}

// Enable/disable control
void MetaVec3::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	for(int i=0;i<3;i++)
		m_pEditCtrls[i]->EnableWindow(bEnabled);
}

}