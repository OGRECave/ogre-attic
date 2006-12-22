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
fastintmap<MetaGroup*>	MetaGroup::m_mGroupMap;

MetaGroup::MetaGroup()
{
	m_iWidth=0;
	m_iHeight=g_iGroupLabelHeight;
	m_hFont=0;
	m_hCtrlFont=0;
	m_hCtrlFontBold=0;
	m_hSeperatorPen=0;
}
MetaGroup::~MetaGroup()
{
	m_mGroupMap.erase((unsigned int)m_hWnd);
	ClearAll();
	if(m_hFont) ::DeleteObject(m_hFont);
	if(m_hCtrlFont) ::DeleteObject(m_hCtrlFont);
	if(m_hCtrlFontBold) ::DeleteObject(m_hCtrlFontBold);
	if(m_hSeperatorPen) ::DeleteObject(m_hSeperatorPen);	
}

bool MetaGroup::Create(MetaControl *pOwner, const char *pszTitle)
{
	// Hook data and create window
	m_pOwner=pOwner;
	m_sTitle=pszTitle;
	m_hWnd=::CreateWindow("MetaGroup", pszTitle, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, 0, 0, 100, 100, pOwner->m_hWnd, NULL, NULL, NULL);
	m_iHeight=g_iGroupLabelHeight;
	m_mGroupMap.map((unsigned int)m_hWnd, this);

	// Create fonts
	LOGFONT lFont;
	memset(&lFont, 0, sizeof(lFont));
	lFont.lfHeight=-g_iGroupFontHeight;
	strcpy_s(lFont.lfFaceName, LF_FACESIZE, g_pszGroupFont);
	m_hFont=::CreateFontIndirect(&lFont);

	lFont.lfHeight=-g_iCtrlFontHeight;
	strcpy_s(lFont.lfFaceName, LF_FACESIZE, g_pszCtrlFont);
	m_hCtrlFont=::CreateFontIndirect(&lFont);	

	lFont.lfWeight=700;
	m_hCtrlFontBold=::CreateFontIndirect(&lFont);	

	m_HeadColor1=GET_SYSTEM_COLOR(COLOR_3DSHADOW);
	m_HeadColor2=GET_SYSTEM_COLOR(COLOR_3DFACE);	
	m_HeadColor2=(((m_HeadColor1&0xFF)+(m_HeadColor2&0xFF))>>1)|
				 (((((m_HeadColor1>>8)&0xFF)+((m_HeadColor2>>8)&0xFF))>>1)<<8)|
				 (((((m_HeadColor1>>16)&0xFF)+((m_HeadColor2>>16)&0xFF))>>1)<<16);
				

	m_hSeperatorPen=::CreatePen(PS_SOLID, 1, GET_SYSTEM_COLOR(COLOR_3DDKSHADOW));
	m_bOpen=true;
	return true;
}

void MetaGroup::AddControl(MetaBaseCtrl *pCtrl)
{
	m_lControls.push_back(pCtrl);
	m_iHeight+=pCtrl->GetHeight()+1;
}

void MetaGroup::UpdateData(CDDObject *pNewData)
{
	for(unsigned i=0;i<m_lControls.size();i++)
	{
		// Re-evaluate conditions on object
		m_lControls[i]->UpdateData(pNewData);
	}
}

void MetaGroup::CheckConditions(const char *pszExcludeKey)
{
	for(unsigned i=0;i<m_lControls.size();i++)
	{
		// Check if we want to skip this control
		if(_stricmp(m_lControls[i]->GetMetaID(), pszExcludeKey)==0) continue;

		// Re-evaluate conditions on object
		m_lControls[i]->CheckConditions();
	}
}

void MetaGroup::ClearAll()
{
	// Iterate all controls, destroying and deleting them
	for(unsigned i=0;i<m_lControls.size();i++)
	{
		m_lControls[i]->Destroy();
		delete m_lControls[i];
	}
	m_lControls.clear();
}

void MetaGroup::DoLayout()
{
	// Layout group - make sure all controls are correctly aligned
	int iY=g_iGroupLabelHeight;

	if(m_bOpen)
	{		
		for(unsigned i=0;i<m_lControls.size();i++)
		{
			m_lControls[i]->MoveTo(g_iGroupCtrlIndent-1, iY);
			iY+=m_lControls[i]->GetHeight()+1;
		}		
	} else
	{
		iY++;
	}
	m_iHeight=iY;
}

void MetaGroup::OnCtrlHeightChange(MetaBaseCtrl *pCtrl)
{
	DoLayout();
	InvalidateRect(this->m_hWnd, NULL, false);
	m_pOwner->DoLayout();
}

void MetaGroup::OnPaint()
{
	// This function paints the group heading and space lines between controls
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);	
	if(!hdc) return;
	HGDIOBJ hOld=SelectObject(hdc, m_hFont);

	RECT rHeadrect, rClient;
	::GetClientRect(m_hWnd, &rClient);
	memcpy(&rHeadrect, &rClient, sizeof(RECT));
	rHeadrect.bottom=g_iGroupLabelHeight;

	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = rHeadrect.left;
	vert [0] .y      = rHeadrect.top;
	vert [0] .Red    = (COLOR16)((m_HeadColor1<<8)&0xFF00);	
	vert [0] .Green  = (COLOR16)((m_HeadColor1)&0xFF00);	
	vert [0] .Blue   = (COLOR16)((m_HeadColor1>>8)&0xFF00);
	vert [0] .Alpha  = 0x0000;

	vert [1] .x      = rHeadrect.right;
	vert [1] .y      = rHeadrect.bottom; 
	vert [1] .Red    = (COLOR16)((m_HeadColor2<<8)&0xFF00);	
	vert [1] .Green  = (COLOR16)((m_HeadColor2)&0xFF00);
	vert [1] .Blue   = (COLOR16)((m_HeadColor2>>8)&0xFF00);
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	::SetTextColor(hdc, 0);
	::SetBkMode(hdc, TRANSPARENT); 
	::DrawText(hdc, m_sTitle.c_str(), -1, &rHeadrect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);	
	rHeadrect.left+=2;
	if(m_bOpen)
		::DrawText(hdc, "-", -1, &rHeadrect, DT_SINGLELINE|DT_TOP|DT_LEFT);
	else
		::DrawText(hdc, "+", -1, &rHeadrect, DT_SINGLELINE|DT_TOP|DT_LEFT);

	HGDIOBJ hOldPen=::SelectObject(hdc, m_hSeperatorPen);
	int iY=g_iGroupLabelHeight;
	for(unsigned i=0;i<m_lControls.size();i++)
	{
		iY+=m_lControls[i]->GetHeight()+1;
		::MoveToEx(hdc, rClient.left, iY-1, NULL);
		::LineTo(hdc, rClient.right, iY-1);	
	}	

	if(!m_bOpen)
	{
		rClient.bottom--;
		::MoveToEx(hdc, rClient.left, rClient.bottom, NULL);
		::LineTo(hdc, rClient.right, rClient.bottom);
	} else
	{
		::MoveToEx(hdc, g_iGroupCtrlIndent-2, g_iGroupLabelHeight, NULL);
		::LineTo(hdc, g_iGroupCtrlIndent-2, rClient.bottom);

		::MoveToEx(hdc, rClient.right-1, g_iGroupLabelHeight, NULL);
		::LineTo(hdc, rClient.right-1, rClient.bottom);

		rClient.right=g_iGroupCtrlIndent-2;
		rClient.top=g_iGroupLabelHeight;
		::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rClient, NULL, 0, NULL);
	}

	::SelectObject(hdc, hOldPen);
	::SelectObject(hdc, hOld);
	
	::EndPaint(m_hWnd, &ps);	
}

void MetaGroup::OnMouseDown(WPARAM wParam, POINT &p)
{
	if(p.x<g_iGroupLabelHeight && p.y<g_iGroupLabelHeight)
	{
		m_bOpen^=1;	
		DoLayout();
		for(unsigned i=0;i<m_lControls.size();i++)
			m_lControls[i]->ShowWindow(m_bOpen ? SW_SHOW : SW_HIDE);
		m_pOwner->DoLayout();
	} else
	{
		this->ClientToScreen(p);
		m_pOwner->ScreenToClient(p);
		m_pOwner->OnMouseDown(wParam, p.x, p.y);
	}
}

void MetaGroup::SetWidth(int iWidth)
{
	m_iWidth=iWidth;	
	::SetWindowPos(m_hWnd, NULL, 0, 0, m_iWidth, m_iHeight, SWP_NOMOVE|SWP_NOZORDER);

	// Iterate child controls and re-position them
	for(unsigned i=0;i<m_lControls.size();i++)
		m_lControls[i]->SetWidth(m_iWidth-g_iGroupCtrlIndent);
}

void MetaGroup::SetPosition(int iYPos)
{
	::SetWindowPos(m_hWnd, NULL, 0, iYPos, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	::InvalidateRect(m_hWnd, NULL, false);
	for(unsigned i=0;i<m_lControls.size();i++)
		::InvalidateRect(m_lControls[i]->m_hWnd, NULL, false);
}
int	MetaGroup::GetHeight()
{
	return m_iHeight;
}

HFONT MetaGroup::GetCtrlFont()
{
	return m_hCtrlFont;
}

HFONT MetaGroup::GetCtrlFontBold()
{
	return m_hCtrlFontBold;
}

LRESULT CALLBACK MetaGroup::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MetaGroup *pCtrl;
	if(!m_mGroupMap.find((unsigned int)hWnd, pCtrl))
		return ::DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_LBUTTONDOWN:
		{	// Pass message to parent to enable scrolling
		POINT p;
		p.x=((signed short)LOWORD(lParam));
		p.y=((signed short)HIWORD(lParam));
		pCtrl->OnMouseDown(wParam, p);		
		} break;
	case WM_MOUSEMOVE:
		{	// Pass message to parent to enable scrolling
		POINT p;
		p.x=((signed short)LOWORD(lParam));
		p.y=((signed short)HIWORD(lParam));
		pCtrl->ClientToScreen(p);
		pCtrl->m_pOwner->ScreenToClient(p);
		pCtrl->m_pOwner->OnMouseMove(wParam, p.x, p.y);
		} break;		
	case WM_ERASEBKGND:
		break;
	case WM_PAINT:
		pCtrl->OnPaint();
		break;
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
}