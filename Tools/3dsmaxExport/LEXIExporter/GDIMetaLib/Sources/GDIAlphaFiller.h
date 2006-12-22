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

#ifndef __NDS_GDI_ALPHA_FILLER__
#define __NDS_GDI_ALPHA_FILLER__

//

namespace GDI 
{

//

class AlphaFiller
{
public:
	AlphaFiller();
	virtual ~AlphaFiller();

	// Create alpha filler, with a DC compatible with either window or other DC
	bool	Create(COLORREF FillColor, HWND hWindow);
	bool	Create(COLORREF FillColor, HDC hCompDC);

	// Set new fill color
	void	SetFillColor(COLORREF FillColor);

	// Fill with a specified alpha opacity
	void	Fill(HDC hDest, int iX, int iY, int iWidth, int iHeight, BYTE bAlpha);

private:
	HDC		m_hFillDC;
	HBITMAP	m_hDCMemory;
	BLENDFUNCTION m_BlendFunc;
};

//
} // namespace GDI

//
#endif