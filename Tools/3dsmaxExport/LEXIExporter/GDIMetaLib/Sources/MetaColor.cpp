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

MetaColor::MetaColor()
{
	m_bHasCapture=false;
	m_bIncludeAlpha=false;
	m_bShowSliders=false;
}
MetaColor::~MetaColor()
{

}

void MetaColor::OnCreated()
{
	m_pEditCtrl=new Edit();
	HWND hWnd=::CreateWindow("EDIT", "", ES_NUMBER|ES_RIGHT|WS_VISIBLE|WS_BORDER|WS_CHILD|WS_TABSTOP, 0, 0, g_iColorEditWidth, m_iHeight, m_hWnd, NULL, NULL, NULL);
	m_pEditCtrl->Attach(hWnd);
//	m_iHeight+=g_iSliderHeight*4;
	::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_pGroup->GetCtrlFont(), 0);

	m_bShowSliders=m_pMetaKey->GetBool("SlidersVisible", false);

	m_AlphaFiller.Create(0xFFFFFF, this->m_hWnd);

	// Create alpha gradient
	HDC hMainDC=::GetDC(this->m_hWnd);
	m_hAlphaDC=::CreateCompatibleDC(hMainDC);

	static UINT bAlphaPixels[256];
	for(unsigned i=0;i<256;i++)
		bAlphaPixels[i]=(i<<24)|(i)|(i<<8)|(i<<16);
	m_hAlphaMemory=::CreateBitmap(256, 1, 1, 32, bAlphaPixels);
	::SelectObject(m_hAlphaDC, m_hAlphaMemory);

	// Create checker
	static unsigned short bBrushBits[8];  // bitmap bits
	bBrushBits[0]=0x0F;
	bBrushBits[1]=0x0F;
	bBrushBits[2]=0x0F;
	bBrushBits[3]=0x0F;
	bBrushBits[4]=0xF0;
	bBrushBits[5]=0xF0;
	bBrushBits[6]=0xF0;
	bBrushBits[7]=0xF0;
	m_hBrushBitmap = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBrushBits); 
	m_hCheckerBrush = CreatePatternBrush(m_hBrushBitmap);

	m_BlendFunc.BlendOp=AC_SRC_OVER;
	m_BlendFunc.BlendFlags=0;
	m_BlendFunc.SourceConstantAlpha=0xFF;
	m_BlendFunc.AlphaFormat=AC_SRC_ALPHA;
}

void MetaColor::UpdateData(CDDObject *pData)
{	
	if(pData->GetKeyType(m_sMetaID.c_str())==DD_VEC4)
	{		
		m_Value=pData->GetVec4(m_sMetaID.c_str());
		m_bIncludeAlpha=true;
	} else if(pData->GetKeyType(m_sMetaID.c_str())==DD_VEC3)
	{
		CVec3 v=pData->GetVec3(m_sMetaID.c_str());
		m_Value.x=v.x;
		m_Value.y=v.y;
		m_Value.z=v.z;
		m_Value.w=0;
		m_bIncludeAlpha=false;
	} else 
	{
		if(m_pMetaKey->GetKeyType("Default")==DD_VEC4)
		{
			m_Value=m_pMetaKey->GetVec4("Default");
			m_bIncludeAlpha=true;
			// Set default data on object
			pData->SetVec4(m_sMetaID.c_str(), m_Value);
		} else
		{
			CVec3 v=m_pMetaKey->GetVec3("Default");
			m_Value.x=v.x;
			m_Value.y=v.y;
			m_Value.z=v.z;
			m_Value.w=0;
			m_bIncludeAlpha=false;
			// Set default data on object
			pData->SetVec3(m_sMetaID.c_str(), v);
		}		
	}		
	
	char buffer[25];
	if(m_bIncludeAlpha)
		_snprintf_s(buffer, 25, "%02X%02X%02X%02X", (int)(m_Value.w*255.0), (int)(m_Value.x*255.0), 
													(int)(m_Value.y*255.0), (int)(m_Value.z*255.0));
	else
		_snprintf_s(buffer, 25, "%02X%02X%02X", (int)(m_Value.x*255.0), (int)(m_Value.y*255.0), 
												(int)(m_Value.z*255.0));
	m_pEditCtrl->SetWindowText(buffer);

	UpdateHeight();
}

void MetaColor::MakeGrayed(TRIVERTEX &tVert)
{
	COLORREF cFace=GET_SYSTEM_COLOR(COLOR_3DFACE);
	tVert.Red=(tVert.Red+(GetRValue(cFace)<<10))/5;
	tVert.Green=(tVert.Green+(GetGValue(cFace)<<10))/5;
	tVert.Blue=(tVert.Blue+(GetBValue(cFace)<<10))/5;
}

void MetaColor::UpdateHeight()
{
	m_iHeight=g_iDefaultCtrlHeight;
	if(m_bShowSliders)
	{
		if(m_bIncludeAlpha)
			m_iHeight+=(g_iSliderHeight*4);
		else
			m_iHeight+=(g_iSliderHeight*3);
	}
}

void MetaColor::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc=::BeginPaint(m_hWnd, &ps);
	if(!hdc) return;
		
	HGDIOBJ hOld=::SelectObject(hdc, m_pGroup->GetCtrlFont());
	RECT rLabel, rSlideArea;
	::GetClientRect(m_hWnd, &rLabel);		
	::SetTextColor(hdc, GET_SYSTEM_COLOR(m_bEnabled ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
	::SetBkMode(hdc, TRANSPARENT); 	

	memcpy(&rSlideArea, &rLabel, sizeof(RECT));	
	rSlideArea.top+=g_iDefaultCtrlHeight;
	rLabel.bottom=g_iDefaultCtrlHeight;
	rLabel.right-=g_iColorEditWidth+g_iColorPreviewWidth;

	::SetBkColor(hdc, GET_SYSTEM_COLOR(COLOR_3DFACE));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);	

	rLabel.left+=2;
	if(m_bShowSliders)
		::DrawText(hdc, "-", -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
	else
		::DrawText(hdc, "+", -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	rLabel.left+=10;
	::DrawText(hdc, m_sCaption.c_str(), -1, &rLabel, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	rLabel.left=rLabel.right;
	rLabel.right+=g_iColorPreviewWidth;
	COLORREF rCurrentColor=RGB((BYTE)(m_Value.x*255.0), (BYTE)(m_Value.y*255.0), (BYTE)(m_Value.z*255.0));
	::SetBkColor(hdc, rCurrentColor);
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rLabel, NULL, 0, NULL);

	if(!m_bShowSliders)
	{
		SelectObject(hdc, hOld);
		::EndPaint(m_hWnd, &ps);
		return;
	}

	int iRValue=(int)(m_Value.x*65535.0);
	int iGValue=(int)(m_Value.y*65535.0);
	int iBValue=(int)(m_Value.z*65535.0);
	int iAValue=(int)(m_Value.w*65535.0);

	// Prepare to draw first slider (RED)
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert[0].x      = rSlideArea.left;
	vert[0].y      = rSlideArea.top;
	vert[0].Red    = 0x0000;
	vert[0].Green  = iGValue;
	vert[0].Blue   = iBValue;
	vert[0].Alpha  = 0xFFFF;

	vert[1].x      = rSlideArea.right;
	vert[1].y      = rSlideArea.top+g_iSliderHeight; 
	vert[1].Red    = 0xFFFF;
	vert[1].Green  = iGValue;
	vert[1].Blue   = iBValue;
	vert[1].Alpha  = 0xFFFF;	
	if(!m_bEnabled) 
	{
		MakeGrayed(vert[0]);
		MakeGrayed(vert[1]);
	}

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	// Prepare GREEN slider
	vert[0].Red    = iRValue;
	vert[0].Green  = 0;
	vert[0].Blue   = iBValue;
	vert[0].y+=g_iSliderHeight;
	vert[1].Red    = iRValue;
	vert[1].Green  = 0xFFFF;
	vert[1].Blue   = iBValue;
	vert[1].y+=g_iSliderHeight;
	if(!m_bEnabled) 
	{
		MakeGrayed(vert[0]);
		MakeGrayed(vert[1]);
	}
	::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	// Prepare BLUE slider
	vert[0].Red    = iRValue;
	vert[0].Green  = iGValue;
	vert[0].Blue   = 0;
	vert[0].y+=g_iSliderHeight;
	vert[1].Red    = iRValue;
	vert[1].Green  = iGValue;
	vert[1].Blue   = 0xFFFF;
	vert[1].y+=g_iSliderHeight;
	if(!m_bEnabled) 
	{
		MakeGrayed(vert[0]);
		MakeGrayed(vert[1]);
	}
	::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

	if(m_bIncludeAlpha)
	{
		// Draw ALPHA slider (really special case)
		RECT r;
		r.top=rSlideArea.top+(g_iSliderHeight*3);
		r.left=rSlideArea.left;
		r.right=rSlideArea.right;
		r.bottom=r.top+g_iSliderHeight;
		::SetBkColor(hdc, 0x4F4F4F);
		if(m_bEnabled) ::SetTextColor(hdc, 0xDFDFDF);
		::FillRect(hdc, &r, m_hCheckerBrush);
		
		static UINT bAlphaPixels[256];
		if(m_bEnabled)
		{
			for(unsigned i=0;i<256;i++)
				bAlphaPixels[i]=(i<<24)|RGB((BYTE)(m_Value.z*i), (BYTE)(m_Value.y*i), (BYTE)(m_Value.x*i));
		} else
		{
			float fAvg=(m_Value.z+m_Value.y+m_Value.x)/3;;
			for(unsigned i=0;i<256;i++)
				bAlphaPixels[i]=(i<<24)|RGB((BYTE)(fAvg*i), (BYTE)(fAvg*i), (BYTE)(fAvg*i));
		}
		::SetBitmapBits(m_hAlphaMemory, 256*4, bAlphaPixels);
		::AlphaBlend(hdc, r.left, r.top, r.right-r.left, g_iSliderHeight, m_hAlphaDC, 0, 0, 256, 1, m_BlendFunc);
	}
	
	int iSliderRange=rSlideArea.right-g_iSliderHandleWidth;
	
	// Calculate RED button position
	int iXPos = ((iRValue * iSliderRange) / 65536);
	if (iXPos < 1) iXPos = 1;
	else if (iXPos > iSliderRange) iXPos = iSliderRange;	
	m_AlphaFiller.Fill(hdc, iXPos, rSlideArea.top+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);	

	// Calculate GREEN button position
	iXPos = ((iGValue * iSliderRange) / 65536);
	if (iXPos < 1) iXPos = 1;
	else if (iXPos > iSliderRange) iXPos = iSliderRange;
	m_AlphaFiller.Fill(hdc, iXPos, rSlideArea.top+g_iSliderHeight+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);		

	// Calculate GREEN button position
	iXPos = ((iBValue * iSliderRange) / 65536);
	if (iXPos < 1) iXPos = 1;
	else if (iXPos > iSliderRange) iXPos = iSliderRange;
	m_AlphaFiller.Fill(hdc, iXPos, rSlideArea.top+(g_iSliderHeight*2)+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);		

	if(m_bIncludeAlpha)
	{
		// Calculate ALPHA button position
		iXPos = ((iAValue * iSliderRange) / 65536);
		if (iXPos < 1) iXPos = 1;
		else if (iXPos > iSliderRange) iXPos = iSliderRange;
		m_AlphaFiller.Fill(hdc, iXPos, rSlideArea.top+(g_iSliderHeight*3)+1, g_iSliderHandleWidth, g_iSliderHeight-2, 170);	
	}
	
	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps);
}

void MetaColor::OnLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_pEditCtrl->SetWindowPos(NULL, rClient.right-g_iColorEditWidth, 0, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

bool MetaColor::OnMouseDown(int iFlags, int iX, int iY)
{	
	if(iX<10 && iY<=g_iDefaultCtrlHeight && m_bEnabled)
	{
		m_bShowSliders^=1;
		UpdateHeight();
		InvalidateRect(this->m_hWnd, NULL, false);
		m_pGroup->OnCtrlHeightChange(this);
		return true;
	}
	return (iY>g_iDefaultCtrlHeight && m_bEnabled);
}

bool MetaColor::OnMouseDblClick(int iFlags, int iX, int iY)
{
	if(iY<=g_iDefaultCtrlHeight && m_bEnabled)
	{
		m_bShowSliders^=1;
		UpdateHeight();
		InvalidateRect(this->m_hWnd, NULL, false);
		m_pGroup->OnCtrlHeightChange(this);
		return true;
	}
	return false;
}

bool MetaColor::OnMouseMove(int iFlags, int iX, int iY)
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

		float fValue = (float)((iX - (g_iSliderHandleWidth / 2)) - rClient.left) / (float)(rClient.right - g_iSliderHandleWidth);
		if (fValue < 0) fValue = 0;
		else if (fValue > 1) fValue = 1;
				
		if(iY>=rClient.bottom) iY=rClient.bottom-1;
		int iAbsYSlider=(iY-g_iDefaultCtrlHeight)/g_iSliderHeight;
		if(GetAsyncKeyState(VK_MENU)&0x8000)
		{
			float fDelta=0;
			switch(iAbsYSlider)
			{
				case 0:
					fDelta=fValue-m_Value.x;break;
				case 1:
					fDelta=fValue-m_Value.y;break;
				case 2:
					fDelta=fValue-m_Value.z;break;
				case 3:
					fDelta=fValue-m_Value.w;break;
			}
			m_Value.x+=fDelta;
			if(m_Value.x>1) m_Value.x=1;
			else if(m_Value.x<0) m_Value.x=0;
			m_Value.y+=fDelta;
			if(m_Value.y>1) m_Value.y=1;
			else if(m_Value.y<0) m_Value.y=0;
			m_Value.z+=fDelta;
			if(m_Value.z>1) m_Value.z=1;
			else if(m_Value.z<0) m_Value.z=0;
			m_Value.w+=fDelta;
			if(m_Value.w>1) m_Value.w=1;
			else if(m_Value.w<0) m_Value.w=0;
		} else
		{			
			switch(iAbsYSlider)
			{
				case 0:
					m_Value.x=fValue;break;
				case 1:
					m_Value.y=fValue;break;
				case 2:
					m_Value.z=fValue;break;
				case 3:
					m_Value.w=fValue;break;
			}
		}
		
		InvalidateRect(m_hWnd, &rClient, false);

		// Set data on data object
		char buffer[25];
		if(m_bIncludeAlpha)
		{
			_snprintf_s(buffer, 25, "%02X%02X%02X%02X", (int)(m_Value.w*255.0), (int)(m_Value.x*255.0), 
														(int)(m_Value.y*255.0), (int)(m_Value.z*255.0));
			m_pOwner->GetData()->SetVec4(m_sMetaID.c_str(), m_Value);
		}
		else
		{
			_snprintf_s(buffer, 25, "%02X%02X%02X", (int)(m_Value.x*255.0), (int)(m_Value.y*255.0), 
													(int)(m_Value.z*255.0));
			m_pOwner->GetData()->SetVec3(m_sMetaID.c_str(), CVec3(m_Value.x, m_Value.y, m_Value.z));
		}
		
		m_pEditCtrl->SetWindowText(buffer);
		return true;
	}
	if(m_bHasCapture)
	{
		::ReleaseCapture();
		m_bHasCapture=false;
	}			
	return false;
}

void MetaColor::OnCommand(HWND hWnd, int iCode, int iID)
{
	if(hWnd==m_pEditCtrl->m_hWnd)
	{
		if(iCode==EN_CHANGE)
		{
			std::string sText=m_pEditCtrl->GetWindowText();
		}
	}
}

// Enable/disable control
void MetaColor::SetEnabled(bool bEnabled)
{
	MetaBaseCtrl::SetEnabled(bEnabled);
	m_pEditCtrl->EnableWindow(bEnabled);
}

}