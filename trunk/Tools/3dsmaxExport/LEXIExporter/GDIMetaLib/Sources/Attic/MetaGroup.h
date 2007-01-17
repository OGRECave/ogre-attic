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

#ifndef __NDS_META_GROUP__
#define __NDS_META_GROUP__

//
namespace GDI 
{

class MetaGroup : public Window
{
public:
	MetaGroup();
	~MetaGroup();

	bool	Create(MetaControl *pOwner, const char *pszTitle);

	void	AddControl(MetaBaseCtrl *pCtrl);
	void	UpdateData(CDDObject *pNewData);
	void	CheckConditions(const char *pszExcludeID);
	void	ClearAll();
	void	OnCtrlHeightChange(MetaBaseCtrl *pCtrl);
	void	DoLayout();	
	void	SetWidth(int iWidth);
	void	SetPosition(int iYPos);	
	int		GetHeight();
	HFONT	GetCtrlFont();
	HFONT	GetCtrlFontBold();

	faststring	GetTitle(void);

	// Window callback procedure
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	// Map HWND<->MetaBaseCtrl instances
	static fastintmap<MetaGroup*>	m_mGroupMap;
	
	void	OnPaint();
	void	OnMouseDown(WPARAM wParam, POINT &p);

	fastvector<MetaBaseCtrl*>	m_lControls;

	// Colors for the heading
	COLORREF	m_HeadColor1;
	COLORREF	m_HeadColor2;

	HFONT		m_hCtrlFont;
	HFONT		m_hCtrlFontBold;
	HFONT		m_hFont;
	HPEN		m_hSeperatorPen;
	bool		m_bOpen;		// Group currently in open state (displaying children)
	int			m_iWidth;
	int			m_iHeight;
	faststring	m_sTitle;
	MetaControl *m_pOwner;	
};

}
#endif