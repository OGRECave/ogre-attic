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

#ifndef __NDS_META_VEC3__
#define __NDS_META_VEC3__

//
namespace GDI 
{
class MetaVec3 : public MetaBaseCtrl
{
public:
	MetaVec3();
	~MetaVec3();	

	// Set defaults on a data object from a meta object
	static void SetDefaults(const CDDObject *pMetaKey, const char *pszMetaID, CDDObject *pData);

private:
	// Data object changed - update data on control
	// Note: This is also called when control is created
	void	UpdateData(CDDObject *pData);
	void	OnCreated();
	void	OnPaint();
	void	OnLayout();	
	bool	OnMouseDown(int iFlags, int iX, int iY);
//	bool	OnKeyDown(unsigned vKey, unsigned iFlags);
	void	OnCommand(HWND hWnd, int iCode, int iID);

	// Enable/disable control
	void	SetEnabled(bool bEnabled);

	CVec3	m_Value;
	bool	m_bShowControls;
	Edit	*m_pEditCtrls[3];	
};

}
#endif