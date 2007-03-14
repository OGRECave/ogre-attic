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

MetaInt::MetaInt()
{
	m_bHasCapture=false;
}
MetaInt::~MetaInt()
{

}

// Set defaults on a data object from a meta object
void MetaInt::SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData)
{
	if(pData->GetKeyType(pszMetaID)!=DD_INT)
	{			
		pData->SetInt(pszMetaID, pMetaKey->GetInt("Default"));					
	}			
}

void MetaInt::OnCreated()
{
	m_pEditCtrl=new Edit();
	HWND hWnd=::CreateWindow("EDIT", "", ES_NUMBER|ES_RIGHT|WS_VISIBLE|WS_BORDER|WS_CHILD|WS_TABSTOP, 0, 0, g_iSmallEditWidth, m_iHeight, m_hWnd, NULL, NULL, NULL);
	m_pEditCtrl->Attach(hWnd);
	
	::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);

	m_bEnableSlider=m_pMetaKey->GetBool("EnableSlider",true);

	if(	m_bEnableSlider )
	{
		m_iHeight+=g_iSliderHeight;

		m_iMin=m_pMetaKey->GetInt("MinValue", 0);
		m_iMax=m_pMetaKey->GetInt("MaxValue", 100);	
		m_iStep=m_pMetaKey->GetInt("Step", 1);

		char buffer[25];
		_snprintf_s(buffer, 25, "%d", m_iMin);
		m_sMin=buffer;	
		_snprintf_s(buffer, 25, "%d", m_iMax);
		m_sMax=buffer;

		m_AlphaFiller.Create(0xFFFFFF, m_hWnd);

		::GetClientRect(m_hWnd, &m_rSlider);	
		m_rSlider.top+=g_iDefaultCtrlHeight;
	}
}

void MetaInt::UpdateData(CDDObject *pData)
{	
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_INT)
	{
		m_iValue=pData->GetInt(m_sMetaID.c_str());
	} else
	{
		m_iValue=m_pMetaKey->GetInt("Default", 0);				

		// Set default data on object
		pData->SetInt(m_sMetaID.c_str(), m_iValue);
	}

	if(	m_bEnableSlider )
	{
		// Bound check
		if (m_iValue < m_iMin) m_iValue = m_iMin;
		else if (m_iValue > m_iMax) m_iValue = m_iMax;
	}

	char buffer[25];
	_snprintf_s(buffer, 25, "%d", m_iValue);
	m_pEditCtrl->SetWindowText(buffer);
}

void MetaInt::OnPaint()
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

		::DrawText(hdc, m_sMin.c_str(), -1, &m_rSlider, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		::DrawText(hdc, m_sMax.c_str(), -1, &m_rSlider, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);	

		int iRange = m_iMax - m_iMin;
		int iXPos = (((m_iValue-m_iMin) * (m_rSlider.right-g_iSliderHandleWidth)) / iRange);
		if (iXPos < 1) iXPos = 1;
		else if (iXPos + g_iSliderHandleWidth > m_rSlider.right - 1)
			iXPos = (m_rSlider.right - 1) - g_iSliderHandleWidth;

		m_AlphaFiller.Fill(hdc, iXPos, m_rSlider.top+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);
	} // if(m_bEnableSlider)

	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaInt::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pEditCtrl->SetWindowPos(NULL, rClient.right-g_iSmallEditWidth, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

bool MetaInt::OnMouseDown(int iFlags, int iX, int iY)
{	
	return (iY>g_iDefaultCtrlHeight && m_bEnabled);
}

bool MetaInt::OnMouseMove(int iFlags, int iX, int iY)
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
			int iNewValue= ((int)((m_iMax - m_iMin) * fRatio + 0.5)) + m_iMin + (m_iStep / 2);
			iNewValue /= m_iStep;
			iNewValue *= m_iStep;
			if (iNewValue < m_iMin) iNewValue = m_iMin;
			else if (iNewValue > m_iMax) iNewValue = m_iMax;		
			if(iNewValue!=m_iValue)
			{
				m_iValue=iNewValue;
				// Set data on data object				
				char buffer[25];
				_snprintf_s(buffer, 25, "%d", m_iValue);
				m_pEditCtrl->SetWindowText(buffer);	
			}
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

void MetaInt::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pEditCtrl->m_hWnd)
	{
		if(iCode==EN_CHANGE)
		{
			std::string sText=m_pEditCtrl->GetWindowText();
			char *pTemp;
			m_iValue=strtol(sText.c_str(), &pTemp, 10);
			m_pOwner->GetData()->SetInt(m_sMetaID.c_str(), m_iValue);
			if(	m_bEnableSlider )
				InvalidateRect(m_hWnd, &m_rSlider, false);
		}
	}
}

// Enable/disable control
void MetaInt::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pEditCtrl->EnableWindow(bEnabled);
}

}