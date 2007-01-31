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

#ifndef __NDS_META_COLOR__
#define __NDS_META_COLOR__

//
namespace GDI 
{
class MetaColor : public MetaBaseCtrl
{
public:
	MetaColor();
	~MetaColor();	

private:
	// Data object changed - update data on control
	// Note: This is also called when control is created
	void	UpdateData(CDDObject *pData);
	void	OnCreated();
	void	OnPaint();
	void	OnLayout();	
	bool	OnMouseDown(int iFlags, int iX, int iY);
	bool	OnMouseDblClick(int iFlags, int iX, int iY);
	bool	OnMouseMove(int iFlags, int iX, int iY);
	void	OnCommand(HWND hWnd, int iCode, int iID);
	void	MakeGrayed(TRIVERTEX &tVert);
	void	UpdateHeight();

	// Enable/disable control
	void	SetEnabled(bool bEnabled);

	bool	m_bIncludeAlpha;
	bool	m_bHasCapture;
	bool	m_bShowSliders;
	CVec4	m_Value;
	AlphaFiller	m_AlphaFiller;
	int		m_iActiveSlider;

	//
	HDC		m_hAlphaDC;
	HBITMAP	m_hAlphaMemory;
	HBITMAP	m_hBrushBitmap;
	HBRUSH	m_hCheckerBrush;
	BLENDFUNCTION m_BlendFunc;

	Edit	*m_pEditCtrl;
};

}
#endif