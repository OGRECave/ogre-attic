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

MetaFloat::MetaFloat()
{

}
MetaFloat::~MetaFloat()
{

}

// Set defaults on a data object from a meta object
void MetaFloat::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_FLOAT)
	{			
		// Get default value as either float or int
		if(pMetaKey->GetKeyType("Default")==DD_INT)
			pData->SetFloat(pszMetaID, (float)pMetaKey->GetInt("Default"));			
		else
			pData->SetFloat(pszMetaID, pMetaKey->GetFloat("Default"));
	}			
}

void MetaFloat::OnCreated()
{
	m_pEditCtrl=new Edit();
	HWND hWnd=::CreateWindow("EDIT", "", ES_RIGHT|WS_VISIBLE|WS_BORDER|WS_CHILD|WS_TABSTOP, 0, 0, g_iSmallEditWidth, m_iHeight, m_hWnd, NULL, NULL, NULL);
	m_pEditCtrl->Attach(hWnd);
	::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);

	m_bEnableSlider=m_pMetaKey->GetBool("EnableSlider",true);

	if(	m_bEnableSlider )
	{
		m_iHeight+=g_iSliderHeight;

		// Get min value as either float or int
		if(m_pMetaKey->GetKeyType("MinValue")==DD_INT)
			m_fMin=(float)m_pMetaKey->GetInt("MinValue",0);
		else
			m_fMin=m_pMetaKey->GetFloat("MinValue", 0);

		// Get max value as either float or int
		if(m_pMetaKey->GetKeyType("MaxValue")==DD_INT)
			m_fMax=(float)m_pMetaKey->GetInt("MaxValue",0);
		else
			m_fMax=m_pMetaKey->GetFloat("MaxValue", 100.0f);	

		char buffer[25];
		_snprintf_s(buffer, 25, "%.2f", m_fMin);
		m_sMin=buffer;	
		_snprintf_s(buffer, 25, "%.2f", m_fMax);
		m_sMax=buffer;
		
		::GetClientRect(m_hWnd, &m_rSlider);	
		m_rSlider.top+=g_iDefaultCtrlHeight;

		m_AlphaFiller.Create(0xFFFFFF, m_hWnd);
	}
}

void MetaFloat::UpdateData(CDDObject *pData)
{	
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_FLOAT)
	{
		m_fValue=pData->GetFloat(m_sMetaID.c_str());
	} else
	{
		// Get default value as either float or int
		if(m_pMetaKey->GetKeyType("Default")==DD_INT)
			m_fValue=(float)m_pMetaKey->GetInt("Default", 0);
		else
			m_fValue=m_pMetaKey->GetFloat("Default", 0);

		// Set data 
		pData->SetFloat(m_sMetaID.c_str(), m_fValue);
	}

	if(	m_bEnableSlider )
	{
		// Bound check
		if (m_fValue < m_fMin) m_fValue = m_fMin;
		else if (m_fValue > m_fMax) m_fValue = m_fMax;
	}

	char buffer[25];
	_snprintf_s(buffer, 25, "%.2f", m_fValue);
	m_pEditCtrl->SetWindowText(buffer);
}

void MetaFloat::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);
	if(!hdc) return;
		
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());
	::GetClientRect(m_hWnd, &m_rSlider);		
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	

	RECT rLabel;
	memcpy(&rLabel, &m_rSlider, sizeof(RECT));
	rLabel.bottom=g_iDefaultCtrlHeight;
	rLabel.right-=g_iSmallEditWidth;
	m_rSlider.top+=g_iDefaultCtrlHeight;

	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);

	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	if(	m_bEnableSlider )
	{
		TRIVERTEX        vert[2] ;
		GRADIENT_RECT    gRect;
		vert [0] .x      = m_rSlider.left;
		vert [0] .y      = m_rSlider.top;
		vert [0] .Red    = 0x6300;
		vert [0] .Green  = 0x7F00;
		vert [0] .Blue   = 0x8E00;
		vert [0] .Alpha  = 0x0000;

		vert [1] .x      = m_rSlider.right;
		vert [1] .y      = m_rSlider.bottom; 
		vert [1] .Red    = 0x9900;
		vert [1] .Green  = 0xAA00;
		vert [1] .Blue   = 0xBE00;
		vert [1] .Alpha  = 0x0000;

		if(!m_bEnabled)
		{
			COLORREF cFace=GET_SYSTEM_COLOR(COLOR_3DFACE);
			vert[0].Red=(vert[0].Red+(GetRValue(cFace)<<8))/2;
			vert[1].Red=(vert[1].Red+(GetRValue(cFace)<<8))/2;

			vert[0].Green=(vert[0].Green+(GetGValue(cFace)<<8))/2;
			vert[1].Green=(vert[1].Green+(GetGValue(cFace)<<8))/2;

			vert[0].Blue=(vert[0].Blue+(GetBValue(cFace)<<8))/2;
			vert[1].Blue=(vert[1].Blue+(GetBValue(cFace)<<8))/2;
		}

		gRect.UpperLeft  = 0;
		gRect.LowerRight = 1;
		::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

		// Draw min/max values
		::DrawText(hdc, m_sMin.c_str(), -1, &m_rSlider, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		::DrawText(hdc, m_sMax.c_str(), -1, &m_rSlider, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);	

		float fRange = m_fMax - m_fMin;
		int iXPos = (int)(((m_fValue - m_fMin) * (m_rSlider.right-g_iSliderHandleWidth)) / fRange);
		if (iXPos < 1) iXPos = 1;
		else if (iXPos + g_iSliderHandleWidth > m_rSlider.right - 1)
			iXPos = (m_rSlider.right - 1) - g_iSliderHandleWidth;

		m_AlphaFiller.Fill(hdc, iXPos, m_rSlider.top+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);
	}

	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaFloat::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pEditCtrl->SetWindowPos(NULL, rClient.right-g_iSmallEditWidth, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

bool MetaFloat::OnMouseDown(int iFlags, int iX, int iY)
{	
	return iY>g_iDefaultCtrlHeight && m_bEnabled;
}

bool MetaFloat::OnMouseMove(int iFlags, int iX, int iY)
{
	if(	m_bEnableSlider )
	{
		if(m_bEnabled && ((iFlags&MK_LBUTTON)==MK_LBUTTON && (iY>g_iDefaultCtrlHeight || m_bHasCapture)))
		{
			if(!m_bHasCapture)
			{
				::SetCapture(m_hWnd);
				m_bHasCapture=true;
			}

			RECT rClient;		
			::GetClientRect(m_hWnd, &rClient);

			double fRatio = (double)((iX - (g_iSliderHandleWidth / 2)) - rClient.left) / (double)(rClient.right - g_iSliderHandleWidth);
			m_fValue = (float)(((m_fMax - m_fMin) * fRatio)) + m_fMin;
			if (m_fValue < m_fMin) m_fValue = m_fMin;
			else if (m_fValue > m_fMax) m_fValue = m_fMax;		
			
			char buffer[25];
			_snprintf_s(buffer, 25, "%.2f", m_fValue);
			m_pEditCtrl->SetWindowText(buffer);
			
			return true;
		}
		if(m_bHasCapture)
		{
			::ReleaseCapture();
			m_bHasCapture=false;
		}
	}
	return false;
}

void MetaFloat::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pEditCtrl->m_hWnd)
	{
		if(iCode==EN_CHANGE)
		{
			std::string sText=m_pEditCtrl->GetWindowText();
			char *pTemp;
			m_fValue=(float)strtod(sText.c_str(), &pTemp);
			m_pOwner->GetData()->SetFloat(m_sMetaID.c_str(), m_fValue);
			InvalidateRect(m_hWnd, &m_rSlider, false);
		}
	}
}

// Enable/disable control
void MetaFloat::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pEditCtrl->EnableWindow(bEnabled);
}

}