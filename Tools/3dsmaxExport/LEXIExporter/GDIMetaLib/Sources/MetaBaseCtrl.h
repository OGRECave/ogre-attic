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

#ifndef __NDS_META_BASE_CONTROL__
#define __NDS_META_BASE_CONTROL__

//
namespace GDI 
{

	class MetaBaseCtrl : public Window
	{
	public:
		MetaBaseCtrl();
		~MetaBaseCtrl();

		// Create meta control
		virtual bool Create(MetaControl *pOwner, 
							MetaGroup	*pGroup,
							const char	*pszMetaID,
							const CDDObject *pMetaKey);
		virtual void Destroy();

		// Data object changed - update data on control
		// Note: This is also called when control is created
		virtual void UpdateData(CDDObject *pData) {};

		// Windows messages called from MetaControl parent
		virtual void OnCommand(HWND hWnd, int iCode, int iID) {};
		virtual bool OnMouseDblClick(int iFlags, int iX, int iY) { return false; };
		virtual bool OnMouseDown(int iFlags, int iX, int iY) { return false; };
		virtual bool OnMouseMove(int iFlags, int iX, int iY) { return false; };

		// Callback - called when control has been created
		virtual void OnCreated() {};

		// Callback - called when control should redraw itself
		virtual void OnPaint();

		// Called from owning group control when control should update its layout
		virtual void OnLayout() {};		

		// Retrieve height of control
		virtual int	 GetHeight();

		// Place control. Coordinates are client space of owning group control
		virtual void MoveTo(int iX, int iY);

		// Set new width of control
		virtual void SetWidth(int iWidth);

		// Enable/disable control
		virtual void SetEnabled(bool bEnabled);

		// Evaluate conditions. This function will call SetEnabled() is needed.
		virtual void CheckConditions();

		// Get owner handle
		MetaControl* GetOwner();

		// Get ID in meta data
		const char * GetMetaID();

	protected:
		// Base data
		faststring	m_sMetaID;
		faststring	m_sCaption;
		faststring	m_sHelp;
		const CDDObject *m_pMetaKey;

		// Enable flag, updated in SetEnabled()
		bool	m_bEnabled;		

		// Position and size of the control
		int		m_iX;
		int		m_iY;
		int		m_iHeight;
		int		m_iWidth;		

		// Owner references
		MetaControl *m_pOwner;
		MetaGroup	*m_pGroup;
	};

}
#endif