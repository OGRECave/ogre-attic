/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#include "StdAfx.h"

namespace GDI {
AlphaFiller::AlphaFiller()
{
}
AlphaFiller::~AlphaFiller()
{
	::DeleteDC(m_hFillDC);
	::DeleteObject(m_hDCMemory);
}

// Create alpha filler, with a DC compatible with either window or other DC
bool AlphaFiller::Create(COLORREF FillColor, HWND hWindow)
{
	HDC hDC=::GetDC(hWindow);
	Create(FillColor, hDC);
	::ReleaseDC(hWindow, hDC);
	return true;
}
bool AlphaFiller::Create(COLORREF FillColor, HDC hCompDC)
{
	m_hFillDC=::CreateCompatibleDC(hCompDC);
	m_hDCMemory=::CreateCompatibleBitmap(m_hFillDC, 32, 32);
	::SelectObject(m_hFillDC, m_hDCMemory);
	SetFillColor(FillColor);

	m_BlendFunc.BlendOp=AC_SRC_OVER;
	m_BlendFunc.BlendFlags=0;
	m_BlendFunc.SourceConstantAlpha=255;
	m_BlendFunc.AlphaFormat=0;
	return true;
}

// Set new fill color
void AlphaFiller::SetFillColor(COLORREF FillColor)
{
	RECT r;
	r.top=r.left=0;
	r.bottom=r.right=32;
	HBRUSH hSolid=::CreateSolidBrush(FillColor);
	::FillRect(m_hFillDC, &r, hSolid);
	::DeleteObject(hSolid);
}

// Fill with a specified alpha opacity
void AlphaFiller::Fill(HDC hDest, int iX, int iY, int iWidth, int iHeight, BYTE bAlpha)
{
	m_BlendFunc.SourceConstantAlpha=bAlpha;
	::AlphaBlend(hDest, iX, iY, iWidth, iHeight, m_hFillDC, 0,0, 32, 32, m_BlendFunc);
}

}